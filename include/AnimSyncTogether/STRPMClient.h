#pragma once

#include "STRPluginMessagingAPI/STRPluginMessagingAPI.h"

#include <RE/Skyrim.h>

#include <optional>
#include <unordered_map>

namespace AnimSyncTogether
{
    class STRPMClient final
    {
    public:
        static STRPMClient* GetSingleton();

        bool Initialize();
        [[nodiscard]] bool IsAvailable() const noexcept;
        [[nodiscard]] std::optional<STRPM::ConnectionID> FindConnectionID(RE::FormID formID) const;

    private:
        STRPMClient() = default;

        static void STRPM_CALL OnProxyMappingChanged(
            const STRPM::ProxyMappingEvent* event,
            void* userData);

        void QueueMappingEvent(const STRPM::ProxyMappingEvent& event);
        void ApplyMappingEvent(const STRPM::ProxyMappingEvent& event);

        const STRPM::ProxyResolverInterface* resolver_{ nullptr };
        bool listenerRegistered_{ false };
        std::unordered_map<STRPM::ConnectionID, RE::FormID> connectionToFormID_;
        std::unordered_map<RE::FormID, STRPM::ConnectionID> formIDToConnection_;
    };
}
