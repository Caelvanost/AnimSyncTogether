#include "AnimSyncTogether/ProxyResolver.h"

namespace AnimSyncTogether
{
    ProxyResolver* ProxyResolver::GetSingleton()
    {
        static ProxyResolver singleton;
        return std::addressof(singleton);
    }

    ActorIdentity ProxyResolver::Describe(const RE::Actor* actor) const
    {
        ActorIdentity identity{};
        if (!actor) {
            return identity;
        }

        identity.formID = actor->GetFormID();

        if (const auto* base = actor->GetActorBase()) {
            identity.baseFormID = base->GetFormID();
        }

        const auto* player = RE::PlayerCharacter::GetSingleton();
        identity.isPlayer = actor == player;

        // STR remote players are represented by Actor proxies rather than the
        // local PlayerCharacter singleton. For v0.1.0 we deliberately keep the
        // heuristic broad and only expose diagnostics; no animation is replayed.
        identity.isPlayerLike = identity.isPlayer || IsCandidateProxy(actor);
        return identity;
    }

    bool ProxyResolver::IsCandidateProxy(const RE::Actor* actor) const
    {
        if (!actor || actor == RE::PlayerCharacter::GetSingleton()) {
            return false;
        }

        if (!actor->IsPlayerRef()) {
            return false;
        }

        return true;
    }
}
