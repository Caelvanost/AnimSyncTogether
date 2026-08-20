#pragma once

#include "STRPluginMessagingAPI/STRPluginMessagingAPI.h"

#include <RE/Skyrim.h>

#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace AnimSyncTogether
{
    class STRPMClient final
    {
    public:
        static STRPMClient* GetSingleton();

        bool Initialize();
        [[nodiscard]] bool IsAvailable() const noexcept;
        [[nodiscard]] std::optional<STRPM::ConnectionID> FindConnectionID(RE::FormID formID) const;
        bool SendAnimationEvent(
            std::string_view tag,
            std::string_view payload,
            std::int32_t gpmaAnimationType,
            bool hasGPMAAnimationType,
            std::int32_t gpmaOffsetType,
            bool hasGPMAOffsetType);

    private:
        STRPMClient() = default;

        static void STRPM_CALL OnProxyMappingChanged(
            const STRPM::ProxyMappingEvent* event,
            void* userData);

        static void STRPM_CALL OnAnimationMessage(
            const STRPM::Message* message,
            void* userData);

        void QueueMappingEvent(const STRPM::ProxyMappingEvent& event);
        void ApplyMappingEvent(const STRPM::ProxyMappingEvent& event);
        void QueueAnimationMessage(
            STRPM::ConnectionID senderConnectionID,
            std::string tag,
            std::string payload,
            std::uint32_t stateFlags,
            std::int32_t gpmaAnimationType,
            std::int32_t gpmaOffsetType);
        void ApplyAnimationMessage(
            STRPM::ConnectionID senderConnectionID,
            const std::string& tag,
            const std::string& payload,
            std::uint32_t stateFlags,
            std::int32_t gpmaAnimationType,
            std::int32_t gpmaOffsetType);
        void EnsureHelmetToggleNPCSpell(RE::Actor* actor);
        void RemoveInjectedHelmetToggleNPCSpell(RE::FormID formID);

        const STRPM::Interface* messaging_{ nullptr };
        const STRPM::ProxyResolverInterface* resolver_{ nullptr };
        STRPM::ListenerHandle animationListener_{};
        bool channelRegistered_{ false };
        bool listenerRegistered_{ false };
        std::unordered_map<STRPM::ConnectionID, RE::FormID> connectionToFormID_;
        std::unordered_map<RE::FormID, STRPM::ConnectionID> formIDToConnection_;
        std::unordered_set<RE::FormID> injectedHelmetToggleSpellActors_;
    };
}
