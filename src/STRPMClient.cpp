#include "AnimSyncTogether/STRPMClient.h"

#include "AnimSyncTogether/AnimationClipProbe.h"
#include "AnimSyncTogether/AnimationProbe.h"
#include "AnimSyncTogether/OARClient.h"

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
        constexpr std::uint32_t kAnimationPacketVersion = 2;
        constexpr std::uint32_t kStateHasGPMAAnimationType = 1u << 0;
        constexpr std::uint32_t kStateHasGPMAOffsetType = 1u << 1;

        struct AnimationPacket
        {
            std::uint32_t version{ kAnimationPacketVersion };
            std::uint32_t stateFlags{ 0 };
            std::int32_t gpmaAnimationType{ 0 };
            std::int32_t gpmaOffsetType{ 0 };
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
        OARClient::GetSingleton()->Initialize();

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
            SKSE::log::info(
                "STRPMClient: animation channel registered '{}'; GPMA state replay enabled packetVersion={}",
                kAnimationChannel,
                kAnimationPacketVersion);
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

    bool STRPMClient::SendAnimationEvent(
        std::string_view tag,
        std::string_view payload,
        std::int32_t gpmaAnimationType,
        bool hasGPMAAnimationType,
        std::int32_t gpmaOffsetType,
        bool hasGPMAOffsetType)
    {
        if (!messaging_ || !channelRegistered_) {
            return false;
        }

        AnimationPacket packet{};
        if (hasGPMAAnimationType) {
            packet.stateFlags |= kStateHasGPMAAnimationType;
            packet.gpmaAnimationType = gpmaAnimationType;
        }
        if (hasGPMAOffsetType) {
            packet.stateFlags |= kStateHasGPMAOffsetType;
            packet.gpmaOffsetType = gpmaOffsetType;
        }
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
                "STRPMClient: animation send failed event='{}' result={}",
                tag,
                STRPM::ResultToString(result));
            return false;
        }

        SKSE::log::info(
            "AnimTxInput event='{}' stateFlags=0x{:X} animationType={} offsetType={}",
            tag,
            packet.stateFlags,
            packet.gpmaAnimationType,
            packet.gpmaOffsetType);
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
            payload,
            packet->stateFlags,
            packet->gpmaAnimationType,
            packet->gpmaOffsetType);
    }

    void STRPMClient::QueueAnimationMessage(
        STRPM::ConnectionID senderConnectionID,
        std::string tag,
        std::string payload,
        std::uint32_t stateFlags,
        std::int32_t gpmaAnimationType,
        std::int32_t gpmaOffsetType)
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
            payload = std::move(payload),
            stateFlags,
            gpmaAnimationType,
            gpmaOffsetType]() {
            ApplyAnimationMessage(
                senderConnectionID,
                tag,
                payload,
                stateFlags,
                gpmaAnimationType,
                gpmaOffsetType);
        });
    }

    void STRPMClient::ApplyAnimationMessage(
        STRPM::ConnectionID senderConnectionID,
        const std::string& tag,
        const std::string& payload,
        std::uint32_t stateFlags,
        std::int32_t gpmaAnimationType,
        std::int32_t gpmaOffsetType)
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

        const RE::BSFixedString animationTypeVariable{ "iGPMAAnimationType" };
        const RE::BSFixedString offsetTypeVariable{ "iGPMAOffsetType" };

        if (tag == "OffsetGPMA") {
            bool animationTypeApplied = false;
            bool offsetTypeApplied = false;

            if ((stateFlags & kStateHasGPMAAnimationType) != 0) {
                animationTypeApplied = actor->SetGraphVariableInt(animationTypeVariable, gpmaAnimationType);
            }
            if ((stateFlags & kStateHasGPMAOffsetType) != 0) {
                offsetTypeApplied = actor->SetGraphVariableInt(offsetTypeVariable, gpmaOffsetType);
            }

            SKSE::log::info(
                "GPMAStateApply actor={:08X} event='OffsetGPMA' animationTypePresent={} animationType={} animationTypeApplied={} offsetTypePresent={} offsetType={} offsetTypeApplied={}",
                formID,
                (stateFlags & kStateHasGPMAAnimationType) != 0,
                gpmaAnimationType,
                animationTypeApplied,
                (stateFlags & kStateHasGPMAOffsetType) != 0,
                gpmaOffsetType,
                offsetTypeApplied);

            // The graph variables must exist before OAR evaluates the GPMA clip.
            OARClient::GetSingleton()->ClearConditionStateData(actor);
            AnimationClipProbe::ArmActor(formID, "remote OffsetGPMA with GPMA state");
        }

        const RE::BSFixedString inputEvent{ tag.c_str() };
        const bool replayed = actor->NotifyAnimationGraph(inputEvent);

        SKSE::log::info(
            "AnimRxInput sender={} proxy={:08X} actor='{}' event='{}' replay=true result={} stateFlags=0x{:X} animationType={} offsetType={}",
            senderConnectionID,
            formID,
            actor->GetName(),
            tag,
            replayed,
            stateFlags,
            gpmaAnimationType,
            gpmaOffsetType);

        if (tag == "OffsetGPMAStop") {
            // Helmet Toggle sends OffsetGPMAStop first and resets the animation
            // type afterwards. Mirror that ordering when the stop is transported.
            const bool reset = actor->SetGraphVariableInt(animationTypeVariable, 0);
            SKSE::log::info(
                "GPMAStateReset actor={:08X} variable='iGPMAAnimationType' value=0 result={} source='network stop'",
                formID,
                reset);
        }
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

            probe->AttachActorByFormID(event.newFormID, "STRPM proxy mapping");
            break;

        case STRPM::ProxyMappingEventType::kRemoved:
            if (event.oldFormID != STRPM::kInvalidProxyFormID) {
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
                probe->DetachActorByFormID(formID, "STRPM mappings cleared");
            }
            connectionToFormID_.clear();
            formIDToConnection_.clear();
            SKSE::log::info("STRPMClient: proxy mappings cleared");
            break;
        }
    }
}
