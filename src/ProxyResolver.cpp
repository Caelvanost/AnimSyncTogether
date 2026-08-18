#include "AnimSyncTogether/ProxyResolver.h"

namespace AnimSyncTogether
{
    namespace
    {
        constexpr RE::FormID kDynamicFormMask = 0xFF000000;
    }

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
        identity.isPlayerLike = identity.isPlayer || IsCandidateProxy(actor);
        return identity;
    }

    bool ProxyResolver::IsCandidateProxy(const RE::Actor* actor) const
    {
        if (!actor || actor == RE::PlayerCharacter::GetSingleton() || actor->IsPlayerRef()) {
            return false;
        }

        return (actor->GetFormID() & kDynamicFormMask) == kDynamicFormMask;
    }
}
