#include "AnimSyncTogether/AnimationProbe.h"
#include "AnimSyncTogether/ProxyResolver.h"

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

        RE::BSAnimationGraphManagerPtr graphManager;
        const bool hasGraphManager = player->GetAnimationGraphManager(graphManager) && graphManager;
        if (!hasGraphManager) {
            SKSE::log::warning("AnimationProbe: player animation graph manager is not ready");
            return;
        }

        SKSE::log::info(
            "AnimationProbe: player graph manager ready with {} graph(s)",
            graphManager->graphs.size());

        const bool added = player->AddAnimationGraphEventSink(this);
        if (added) {
            SKSE::log::info("AnimationProbe: sink added to local PlayerCharacter");
        } else {
            SKSE::log::info("AnimationProbe: sink was already present or could not be added");
        }
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
