#include "AnimSyncTogether/AnimationProbe.h"

namespace AnimSyncTogether
{
    AnimationProbe* AnimationProbe::GetSingleton()
    {
        static AnimationProbe singleton;
        return std::addressof(singleton);
    }

    bool AnimationProbe::AttachActor(RE::Actor* actor, const char* reason)
    {
        if (!actor) {
            return false;
        }

        const auto formID = actor->GetFormID();
        if (attachedActors_.contains(formID)) {
            return true;
        }

        RE::BSAnimationGraphManagerPtr graphManager;
        const bool hasGraphManager = actor->GetAnimationGraphManager(graphManager) && graphManager;
        if (!hasGraphManager) {
            SKSE::log::info(
                "AnimationProbe: actor {:08X} '{}' has no graph manager yet ({})",
                formID,
                actor->GetName(),
                reason);
            return false;
        }

        const bool added = actor->AddAnimationGraphEventSink(this);
        if (added) {
            attachedActors_.insert(formID);
            const auto* base = actor->GetActorBase();
            SKSE::log::info(
                "AnimationProbe: sink attached actor={:08X} base={:08X} name='{}' localPlayer={} graphs={} reason='{}'",
                formID,
                base ? base->GetFormID() : 0,
                actor->GetName(),
                actor == RE::PlayerCharacter::GetSingleton(),
                graphManager->graphs.size(),
                reason);
            return true;
        }

        SKSE::log::info(
            "AnimationProbe: sink already present or unavailable actor={:08X} name='{}' reason='{}'",
            formID,
            actor->GetName(),
            reason);
        return false;
    }

    void AnimationProbe::Install()
    {
        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            SKSE::log::error("AnimationProbe: PlayerCharacter is not available");
            return;
        }

        AttachActor(player, "local player install");
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

        const auto* base = actor->GetActorBase();
        SKSE::log::info(
            "AnimEvent actor={:08X} base={:08X} name='{}' localPlayer={} tag='{}' payload='{}'",
            actor->GetFormID(),
            base ? base->GetFormID() : 0,
            actor->GetName(),
            actor == RE::PlayerCharacter::GetSingleton(),
            event->tag.c_str(),
            event->payload.c_str());

        return RE::BSEventNotifyControl::kContinue;
    }
}
