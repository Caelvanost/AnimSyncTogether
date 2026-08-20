#include "AnimSyncTogether/STRPMClient.h"

#include "AnimSyncTogether/AnimationProbe.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <span>
#include <string>

namespace AnimSyncTogether
{
    namespace
    {
        constexpr char kAnimationChannel[] = "caelvanost.animsynctogether.anim.v1";
        constexpr std::uint32_t kAnimationPacketVersion = 1;
        constexpr std::string_view kHelmetToggleNPCSpellEditorID = "HT_NPCSpellMonitor";

        struct AnimationPacket
        {
            std::uint32_t version{ kAnimationPacketVersion };
            std::array<char, 64> tag{};
            std::array<char, 128> payload{};
        };

        void CopyField(std::span<char> destination, std::string_view source)
        {
            if (destination.empty()) {
                return;
            }

            const auto count = (std::min)(source.size(), destination.size() - 1);
            std::memcpy(destination.data(), source.data(), count);
            destination[count] = '\0';
        }

        bool IsHelmetInput(std::string_view eventName)
        {
            return eventName == "OffsetGPMA" || eventName == "OffsetGPMAStop";
        }
    }

    STRPMClient* STRPMClient::GetSingleton()
    {
        static STRPMClient singleton;
        return std::addressof(singleton);
    }

    bool STRPMClient::Initialize()
    {
        if (!messaging_) {
            messaging_ = STRPM::LoadFromModule();
            if (!messaging_) {
                SKSE::log::warn("STRPMClient: messaging API is not available");
                return false;
            }
        }

        if (!channelRegistered_) {
            const auto result = messaging_->registerChannel(
                kAnimationChannel,
                &STRPMClient::OnAnimationMessage,
                this,
                &animationListener_);
            if (result != STRPM::Result::kOk) {
                SKSE::log::error(
                    "STRPMClient: animation channel registration failed ({})",
                    STRPM::ResultToString(result));
                return false;
            }
            channelRegistered_ = true;
            SKSE::log::info("STRPMClient: animation channel registered '{}'; input replay enabled", kAnimationChannel);
        }

        if (!resolver_) {
            resolver_ = STRPM::LoadProxyResolverFromModule();
            if (!resolver_) {
                SKSE::log::warn("STRPMClient: ProxyResolver API is not available");
                return false;
            }
        }

        if (!listenerRegistered_) {
            const auto result = resolver_->registerListener(&STRPMClient::OnProxyMappingChanged, this);
            if (result != STRPM::Result::kOk) {
                SKSE::log::error(
                    "STRPMClient: registerListener failed ({})",
                    STRPM::ResultToString(result));
                resolver_ = nullptr;
                return false;
            }

            listenerRegistered_ = true;
            SKSE::log::info("STRPMClient: ProxyResolver listener registered");
        }

        return true;
    }

    bool STRPMClient::IsAvailable() const noexcept
    {
        return messaging_ != nullptr && channelRegistered_ && resolver_ != nullptr && listenerRegistered_;
    }

    std::optional<STRPM::ConnectionID> STRPMClient::FindConnectionID(RE::FormID formID) const
    {
        const auto it = formIDToConnection_.find(formID);
        if (it == formIDToConnection_.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    bool STRPMClient::SendAnimationEvent(std::string_view tag, std::string_view payload)
    {
        if (!messaging_ || !channelRegistered_) {
            return false;
        }

        AnimationPacket packet{};
        CopyField(packet.tag, tag);
        CopyField(packet.payload, payload);

        const STRPM::Target target{
            STRPM::TargetKind::kAllPlayers,
            0,
            nullptr
        };

        const auto result = messaging_->send(
            kAnimationChannel,
            target,
            &packet,
            sizeof(packet),
            STRPM::kMessageReliable | STRPM::kMessageOrdered);

        if (result != STRPM::Result::kOk) {
            SKSE::log::warn(
                "STRPMClient: animation send failed tag='{}' payload='{}' result={}",
                tag,
                payload,
                STRPM::ResultToString(result));
            return false;
        }

        SKSE::log::info("AnimTxInput event='{}' payload='{}'", tag, payload);
        return true;
    }

    void STRPM_CALL STRPMClient::OnAnimationMessage(
        const STRPM::Message* message,
        void* userData)
    {
        if (!message || !userData || !message->data || message->size != sizeof(AnimationPacket)) {
            return;
        }

        const auto* packet = static_cast<const AnimationPacket*>(message->data);
        if (packet->version != kAnimationPacketVersion) {
            return;
        }

        const std::string tag(packet->tag.data(), strnlen_s(packet->tag.data(), packet->tag.size()));
        const std::string payload(packet->payload.data(), strnlen_s(packet->payload.data(), packet->payload.size()));
        static_cast<STRPMClient*>(userData)->QueueAnimationMessage(
            message->sender.connectionID,
            tag,
            payload);
    }

    void STRPMClient::QueueAnimationMessage(
        STRPM::ConnectionID senderConnectionID,
        std::string tag,
        std::string payload)
    {
        auto* tasks = SKSE::GetTaskInterface();
        if (!tasks) {
            SKSE::log::error("STRPMClient: SKSE task interface unavailable; dropping animation message");
            return;
        }

        tasks->AddTask([
            this,
            senderConnectionID,
            tag = std::move(tag),
            payload = std::move(payload)]() {
            ApplyAnimationMessage(senderConnectionID, tag, payload);
        });
    }

    void STRPMClient::ApplyAnimationMessage(
        STRPM::ConnectionID senderConnectionID,
        const std::string& tag,
        const std::string& payload)
    {
        (void)payload;

        if (!resolver_) {
            return;
        }

        STRPM::ProxyFormID formID{};
        const auto result = resolver_->resolve(senderConnectionID, &formID);
        if (result != STRPM::Result::kOk || formID == STRPM::kInvalidProxyFormID) {
            SKSE::log::info(
                "AnimRxInput unresolved sender={} event='{}' result={}",
                senderConnectionID,
                tag,
                STRPM::ResultToString(result));
            return;
        }

        auto* actor = RE::TESForm::LookupByID<RE::Actor>(formID);
        if (!actor) {
            SKSE::log::info(
                "AnimRxInput sender={} proxy={:08X} actor='<missing>' event='{}' replay=false",
                senderConnectionID,
                formID,
                tag);
            return;
        }

        if (!IsHelmetInput(tag)) {
            SKSE::log::info(
                "AnimRxInput sender={} proxy={:08X} actor='{}' event='{}' replay=false reason='not allow-listed'",
                senderConnectionID,
                formID,
                actor->GetName(),
                tag);
            return;
        }

        EnsureHelmetToggleNPCSpell(actor);

        const RE::BSFixedString inputEvent{ tag };
        const bool replayed = actor->NotifyAnimationGraph(inputEvent);

        SKSE::log::info(
            "AnimRxInput sender={} proxy={:08X} actor='{}' event='{}' replay=true result={}",
            senderConnectionID,
            formID,
            actor->GetName(),
            tag,
            replayed);
    }

    void STRPMClient::EnsureHelmetToggleNPCSpell(RE::Actor* actor)
    {
        if (!actor) {
            return;
        }

        auto* spell = RE::TESForm::LookupByEditorID<RE::SpellItem>(kHelmetToggleNPCSpellEditorID);
        if (!spell) {
            SKSE::log::warn(
                "HelmetToggleCompat: spell '{}' not found; OAR NPC conditions may remain unsatisfied",
                kHelmetToggleNPCSpellEditorID);
            return;
        }

        const auto formID = actor->GetFormID();
        if (actor->HasSpell(spell)) {
            SKSE::log::debug(
                "HelmetToggleCompat: proxy {:08X} '{}' already has spell '{}'",
                formID,
                actor->GetName(),
                kHelmetToggleNPCSpellEditorID);
            return;
        }

        const bool added = actor->AddSpell(spell);
        SKSE::log::info(
            "HelmetToggleCompat: add spell proxy={:08X} actor='{}' spell='{}' result={}",
            formID,
            actor->GetName(),
            kHelmetToggleNPCSpellEditorID,
            added);

        if (added) {
            injectedHelmetToggleSpellActors_.insert(formID);
        }
    }

    void STRPMClient::RemoveInjectedHelmetToggleNPCSpell(RE::FormID formID)
    {
        if (!injectedHelmetToggleSpellActors_.erase(formID)) {
            return;
        }

        auto* actor = RE::TESForm::LookupByID<RE::Actor>(formID);
        auto* spell = RE::TESForm::LookupByEditorID<RE::SpellItem>(kHelmetToggleNPCSpellEditorID);
        if (!actor || !spell) {
            SKSE::log::info(
                "HelmetToggleCompat: cleanup skipped proxy={:08X} actorPresent={} spellPresent={}",
                formID,
                actor != nullptr,
                spell != nullptr);
            return;
        }

        const bool removed = actor->RemoveSpell(spell);
        SKSE::log::info(
            "HelmetToggleCompat: remove spell proxy={:08X} actor='{}' spell='{}' result={}",
            formID,
            actor->GetName(),
            kHelmetToggleNPCSpellEditorID,
            removed);
    }

    void STRPM_CALL STRPMClient::OnProxyMappingChanged(
        const STRPM::ProxyMappingEvent* event,
        void* userData)
    {
        if (!event || !userData) {
            return;
        }

        static_cast<STRPMClient*>(userData)->QueueMappingEvent(*event);
    }

    void STRPMClient::QueueMappingEvent(const STRPM::ProxyMappingEvent& event)
    {
        auto* tasks = SKSE::GetTaskInterface();
        if (!tasks) {
            SKSE::log::error("STRPMClient: SKSE task interface unavailable; dropping proxy mapping event");
            return;
        }

        tasks->AddTask([this, event]() {
            ApplyMappingEvent(event);
        });
    }

    void STRPMClient::ApplyMappingEvent(const STRPM::ProxyMappingEvent& event)
    {
        auto* probe = AnimationProbe::GetSingleton();

        switch (event.type) {
        case STRPM::ProxyMappingEventType::kAdded:
        case STRPM::ProxyMappingEventType::kUpdated:
            if (event.oldFormID != STRPM::kInvalidProxyFormID && event.oldFormID != event.newFormID) {
                RemoveInjectedHelmetToggleNPCSpell(event.oldFormID);
                probe->DetachActorByFormID(event.oldFormID, "STRPM mapping replaced");
                formIDToConnection_.erase(event.oldFormID);
            }

            if (event.newFormID == STRPM::kInvalidProxyFormID) {
                return;
            }

            connectionToFormID_[event.connectionID] = event.newFormID;
            formIDToConnection_[event.newFormID] = event.connectionID;

            SKSE::log::info(
                "STRPMClient: proxy mapping {} connection={} formID={:08X}",
                event.type == STRPM::ProxyMappingEventType::kAdded ? "added" : "updated",
                event.connectionID,
                event.newFormID);

            if (auto* actor = RE::TESForm::LookupByID<RE::Actor>(event.newFormID)) {
                EnsureHelmetToggleNPCSpell(actor);
            }
            probe->AttachActorByFormID(event.newFormID, "STRPM proxy mapping");
            break;

        case STRPM::ProxyMappingEventType::kRemoved:
            if (event.oldFormID != STRPM::kInvalidProxyFormID) {
                RemoveInjectedHelmetToggleNPCSpell(event.oldFormID);
                probe->DetachActorByFormID(event.oldFormID, "STRPM mapping removed");
                formIDToConnection_.erase(event.oldFormID);
            }
            connectionToFormID_.erase(event.connectionID);
            SKSE::log::info(
                "STRPMClient: proxy mapping removed connection={} formID={:08X}",
                event.connectionID,
                event.oldFormID);
            break;

        case STRPM::ProxyMappingEventType::kCleared:
            for (const auto& [formID, connectionID] : formIDToConnection_) {
                (void)connectionID;
                RemoveInjectedHelmetToggleNPCSpell(formID);
                probe->DetachActorByFormID(formID, "STRPM mappings cleared");
            }
            connectionToFormID_.clear();
            formIDToConnection_.clear();
            injectedHelmetToggleSpellActors_.clear();
            SKSE::log::info("STRPMClient: proxy mappings cleared");
            break;
        }
    }
}
