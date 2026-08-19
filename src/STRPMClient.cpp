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
                SKSE::log::error("STRPMClient: animation channel registration failed ({})", STRPM::ResultToString(result));
                return false;
            }
            channelRegistered_ = true;
            SKSE::log::info("STRPMClient: animation channel registered '{}'; replay disabled for input diagnostics", kAnimationChannel);
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
                SKSE::log::error("STRPMClient: registerListener failed ({})", STRPM::ResultToString(result));
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
        return it == formIDToConnection_.end() ? std::nullopt : std::optional<STRPM::ConnectionID>{ it->second };
    }

    bool STRPMClient::SendAnimationEvent(std::string_view tag, std::string_view payload)
    {
        if (!messaging_ || !channelRegistered_) {
            return false;
        }

        AnimationPacket packet{};
        CopyField(packet.tag, tag);
        CopyField(packet.payload, payload);
        const STRPM::Target target{ STRPM::TargetKind::kAllPlayers, 0, nullptr };
        const auto result = messaging_->send(kAnimationChannel, target, &packet, sizeof(packet), STRPM::kMessageReliable | STRPM::kMessageOrdered);
        if (result != STRPM::Result::kOk) {
            SKSE::log::warn("STRPMClient: animation send failed tag='{}' payload='{}' result={}", tag, payload, STRPM::ResultToString(result));
            return false;
        }

        SKSE::log::info("AnimTx tag='{}' payload='{}'", tag, payload);
        return true;
    }

    void STRPM_CALL STRPMClient::OnAnimationMessage(const STRPM::Message* message, void* userData)
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
        static_cast<STRPMClient*>(userData)->QueueAnimationMessage(message->sender.connectionID, tag, payload);
    }

    void STRPMClient::QueueAnimationMessage(STRPM::ConnectionID senderConnectionID, std::string tag, std::string payload)
    {
        if (auto* tasks = SKSE::GetTaskInterface()) {
            tasks->AddTask([this, senderConnectionID, tag = std::move(tag), payload = std::move(payload)]() {
                ApplyAnimationMessage(senderConnectionID, tag, payload);
            });
        }
    }

    void STRPMClient::ApplyAnimationMessage(STRPM::ConnectionID senderConnectionID, const std::string& tag, const std::string& payload)
    {
        STRPM::ProxyFormID formID{};
        const auto result = resolver_ ? resolver_->resolve(senderConnectionID, &formID) : STRPM::Result::kNotAvailable;
        auto* actor = result == STRPM::Result::kOk ? RE::TESForm::LookupByID<RE::Actor>(formID) : nullptr;
        SKSE::log::info(
            "AnimRx sender={} proxy={:08X} actor='{}' tag='{}' payload='{}' replay=false diagnostic=true",
            senderConnectionID,
            formID,
            actor ? actor->GetName() : "<missing>",
            tag,
            payload);
    }

    void STRPM_CALL STRPMClient::OnProxyMappingChanged(const STRPM::ProxyMappingEvent* event, void* userData)
    {
        if (event && userData) {
            static_cast<STRPMClient*>(userData)->QueueMappingEvent(*event);
        }
    }

    void STRPMClient::QueueMappingEvent(const STRPM::ProxyMappingEvent& event)
    {
        if (auto* tasks = SKSE::GetTaskInterface()) {
            tasks->AddTask([this, event]() { ApplyMappingEvent(event); });
        }
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
            SKSE::log::info("STRPMClient: proxy mapping {} connection={} formID={:08X}", event.type == STRPM::ProxyMappingEventType::kAdded ? "added" : "updated", event.connectionID, event.newFormID);
            probe->AttachActorByFormID(event.newFormID, "STRPM proxy mapping");
            break;
        case STRPM::ProxyMappingEventType::kRemoved:
            if (event.oldFormID != STRPM::kInvalidProxyFormID) {
                probe->DetachActorByFormID(event.oldFormID, "STRPM mapping removed");
                formIDToConnection_.erase(event.oldFormID);
            }
            connectionToFormID_.erase(event.connectionID);
            break;
        case STRPM::ProxyMappingEventType::kCleared:
            for (const auto& [formID, connectionID] : formIDToConnection_) {
                (void)connectionID;
                probe->DetachActorByFormID(formID, "STRPM mappings cleared");
            }
            connectionToFormID_.clear();
            formIDToConnection_.clear();
            break;
        }
    }
}
