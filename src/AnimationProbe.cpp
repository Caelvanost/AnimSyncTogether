#include "AnimSyncTogether/AnimationProbe.h"

#include "AnimSyncTogether/STRPMClient.h"

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
            const auto connectionID = STRPMClient::GetSingleton()->FindConnectionID(formID);
            SKSE::log::info(
                "AnimationProbe: sink attached actor={:08X} base={:08X} name='{}' localPlayer={} remoteConnection={} graphs={} reason='{}'",
                formID,
                base ? base->GetFormID() : 0,
                actor->GetName(),
                actor == RE::PlayerCharacter::GetSingleton(),
                connectionID.value_or(0),
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

    bool AnimationProbe::AttachActorByFormID(RE::FormID formID, const char* reason)
    {
        if (formID == 0) {
            return false;
        }

        auto* actor = RE::TESForm::LookupByID<RE::Actor>(formID);
        if (!actor) {
            SKSE::log::info(
                "AnimationProbe: actor lookup failed formID={:08X} reason='{}'",
                formID,
                reason);
            return false;
        }

        return AttachActor(actor, reason);
    }

    void AnimationProbe::DetachActorByFormID(RE::FormID formID, const char* reason)
    {
        if (!attachedActors_.erase(formID)) {
            return;
        }

        if (auto* actor = RE::TESForm::LookupByID<RE::Actor>(formID)) {
            actor->RemoveAnimationGraphEventSink(this);
            SKSE::log::info(
                "AnimationProbe: sink detached actor={:08X} name='{}' reason='{}'",
                formID,
                actor->GetName(),
                reason);
        } else {
            SKSE::log::info(
                "AnimationProbe: detached stale actor formID={:08X} reason='{}'",
                formID,
                reason);
        }
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

        const auto formID = actor->GetFormID();
        const auto* base = actor->GetActorBase();
        const auto connectionID = STRPMClient::GetSingleton()->FindConnectionID(formID);
        SKSE::log::info(
            "AnimEvent actor={:08X} base={:08X} name='{}' localPlayer={} remoteConnection={} tag='{}' payload='{}'",
            formID,
            base ? base->GetFormID() : 0,
            actor->GetName(),
            actor == RE::PlayerCharacter::GetSingleton(),
            connectionID.value_or(0),
            event->tag.c_str(),
            event->payload.c_str());

        return RE::BSEventNotifyControl::kContinue;
    }
}
