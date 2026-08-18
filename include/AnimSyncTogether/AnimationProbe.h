#pragma once

#include <RE/Skyrim.h>

namespace AnimSyncTogether
{
    class AnimationProbe final : public RE::BSTEventSink<RE::BSAnimationGraphEvent>
    {
    public:
        static AnimationProbe* GetSingleton();

        void Install();

        RE::BSEventNotifyControl ProcessEvent(
            const RE::BSAnimationGraphEvent* event,
            RE::BSTEventSource<RE::BSAnimationGraphEvent>* source) override;

    private:
        AnimationProbe() = default;
    };
}
