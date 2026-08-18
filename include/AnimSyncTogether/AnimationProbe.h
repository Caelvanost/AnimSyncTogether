#pragma once

#include <RE/Skyrim.h>

#include <unordered_set>

namespace AnimSyncTogether
{
    class AnimationProbe final :
        public RE::BSTEventSink<RE::BSAnimationGraphEvent>,
        public RE::BSTEventSink<RE::TESObjectLoadedEvent>
    {
    public:
        static AnimationProbe* GetSingleton();

        void Install();
        void ScanForProxyActors();

        RE::BSEventNotifyControl ProcessEvent(
            const RE::BSAnimationGraphEvent* event,
            RE::BSTEventSource<RE::BSAnimationGraphEvent>* source) override;

        RE::BSEventNotifyControl ProcessEvent(
            const RE::TESObjectLoadedEvent* event,
            RE::BSTEventSource<RE::TESObjectLoadedEvent>* source) override;

    private:
        AnimationProbe() = default;

        bool AttachActor(RE::Actor* actor, const char* reason);

        std::unordered_set<RE::FormID> attachedActors_;
        bool objectLoadWatcherInstalled_{ false };
    };
}
