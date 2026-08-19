#include "AnimSyncTogether/STRPMClient.h"

#include "AnimSyncTogether/AnimationProbe.h"

namespace AnimSyncTogether
{
    STRPMClient* STRPMClient::GetSingleton()
    {
        static STRPMClient singleton;
        return std::addressof(singleton);
    }

    bool STRPMClient::Initialize()
    {
        if (listenerRegistered_) {
            return true;
        }

        resolver_ = STRPM::LoadProxyResolverFromModule();
        if (!resolver_) {
            SKSE::log::warn("STRPMClient: ProxyResolver API is not available");
            return false;
        }

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
        return true;
    }

    bool STRPMClient::IsAvailable() const noexcept
    {
        return resolver_ != nullptr && listenerRegistered_;
    }

    std::optional<STRPM::ConnectionID> STRPMClient::FindConnectionID(RE::FormID formID) const
    {
        const auto it = formIDToConnection_.find(formID);
        if (it == formIDToConnection_.end()) {
            return std::nullopt;
        }
        return it->second;
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
