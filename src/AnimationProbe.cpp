#include "AnimSyncTogether/AnimationProbe.h"
#include "AnimSyncTogether/ProxyResolver.h"

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
            const auto identity = ProxyResolver::GetSingleton()->Describe(actor);
            SKSE::log::info(
                "AnimationProbe: sink attached actor={:08X} base={:08X} name='{}' player={} proxyCandidate={} graphs={} reason='{}'",
                identity.formID,
                identity.baseFormID,
                actor->GetName(),
                identity.isPlayer,
                ProxyResolver::GetSingleton()->IsCandidateProxy(actor),
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

    void AnimationProbe::ScanForProxyActors()
    {
        auto* processLists = RE::ProcessLists::GetSingleton();
        if (!processLists) {
            return;
        }

        std::unordered_set<RE::FormID> seen;
        auto scan = [&](const auto& handles) {
            for (const auto& handle : handles) {
                const auto actorPointer = handle.get();
                auto* actor = actorPointer.get();
                if (!ProxyResolver::GetSingleton()->IsCandidateProxy(actor)) {
                    continue;
                }

                const auto formID = actor->GetFormID();
                if (!seen.insert(formID).second) {
                    continue;
                }

                SKSE::log::info(
                    "AnimationProbe: dynamic actor candidate actor={:08X} base={:08X} name='{}'",
                    formID,
                    actor->GetActorBase() ? actor->GetActorBase()->GetFormID() : 0,
                    actor->GetName());

                AttachActor(actor, "process-list scan");
            }
        };

        scan(processLists->highActorHandles);
        scan(processLists->middleHighActorHandles);
        scan(processLists->middleLowActorHandles);
        scan(processLists->lowActorHandles);
    }

    void AnimationProbe::Install()
    {
        if (!objectLoadWatcherInstalled_) {
            if (auto* eventSource = RE::ScriptEventSourceHolder::GetSingleton()) {
                eventSource->AddEventSink<RE::TESObjectLoadedEvent>(this);
                objectLoadWatcherInstalled_ = true;
                SKSE::log::info("AnimationProbe: TESObjectLoadedEvent watcher installed");
            } else {
                SKSE::log::warn("AnimationProbe: ScriptEventSourceHolder is not available");
            }
        }

        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            SKSE::log::error("AnimationProbe: PlayerCharacter is not available");
            return;
        }

        AttachActor(player, "local player install");
        ScanForProxyActors();
    }

    RE::BSEventNotifyControl AnimationProbe::ProcessEvent(
        const RE::TESObjectLoadedEvent* event,
        RE::BSTEventSource<RE::TESObjectLoadedEvent>*)
    {
        if (!event || !event->loaded) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto* actor = RE::TESForm::LookupByID<RE::Actor>(event->formID);
        if (!ProxyResolver::GetSingleton()->IsCandidateProxy(actor)) {
            return RE::BSEventNotifyControl::kContinue;
        }

        SKSE::log::info(
            "AnimationProbe: TESObjectLoadedEvent proxy candidate actor={:08X} name='{}'",
            event->formID,
            actor->GetName());
        AttachActor(actor, "TESObjectLoadedEvent");

        return RE::BSEventNotifyControl::kContinue;
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
        const auto proxyCandidate = ProxyResolver::GetSingleton()->IsCandidateProxy(actor);

        SKSE::log::info(
            "AnimEvent actor={:08X} base={:08X} name='{}' player={} proxyCandidate={} tag='{}' payload='{}'",
            identity.formID,
            identity.baseFormID,
            actor->GetName(),
            identity.isPlayer,
            proxyCandidate,
            event->tag.c_str(),
            event->payload.c_str());

        if (identity.isPlayer) {
            ScanForProxyActors();
        }

        return RE::BSEventNotifyControl::kContinue;
    }
}
