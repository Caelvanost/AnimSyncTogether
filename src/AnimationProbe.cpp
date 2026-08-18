#include "AnimSyncTogether/AnimationProbe.h"
#include "AnimSyncTogether/ProxyResolver.h"

#include <SKSE/SKSE.h>

namespace AnimSyncTogether
{
    AnimationProbe* AnimationProbe::GetSingleton()
    {
        static AnimationProbe singleton;
        return std::addressof(singleton);
    }

    void AnimationProbe::Install()
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            SKSE::log::error("AnimationProbe: PlayerCharacter is not available");
            return;
        }

        player->AddAnimationGraphEventSink(this);
        SKSE::log::info("AnimationProbe installed on local PlayerCharacter");
    }

    RE::BSEventNotifyControl AnimationProbe::ProcessEvent(
        const RE::BSAnimationGraphEvent* event,
        RE::BSTEventSource<RE::BSAnimationGraphEvent>*)
    {
        if (!event || !event->holder) {
            return RE::BSEventNotifyControl::kContinue;
        }

        const auto* actor = event->holder->As<RE::Actor>();
        if (!actor) {
            return RE::BSEventNotifyControl::kContinue;
        }

        const auto identity = ProxyResolver::GetSingleton()->Describe(actor);

        SKSE::log::info(
            "AnimEvent actor={:08X} base={:08X} player={} playerLike={} tag='{}' payload='{}'",
            identity.formID,
            identity.baseFormID,
            identity.isPlayer,
            identity.isPlayerLike,
            event->tag.c_str(),
            event->payload.c_str());

        return RE::BSEventNotifyControl::kContinue;
    }
}
