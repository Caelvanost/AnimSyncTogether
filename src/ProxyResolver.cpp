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

        // v0.1.0 deliberately does not guess STR remote-player identity from
        // generic Actor flags. A wrong heuristic here would classify normal
        // NPCs as player proxies and contaminate all later animation tests.
        identity.isPlayerLike = identity.isPlayer;
        return identity;
    }

    bool ProxyResolver::IsCandidateProxy(const RE::Actor* actor) const
    {
        // Reserved for the STR-specific resolver introduced after we have
        // two-client diagnostic data and a stable proxy identifier.
        (void)actor;
        return false;
    }
}
