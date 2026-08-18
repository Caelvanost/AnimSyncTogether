#pragma once

#include <RE/Skyrim.h>

namespace AnimSyncTogether
{
    struct ActorIdentity
    {
        RE::FormID formID{ 0 };
        RE::FormID baseFormID{ 0 };
        bool isPlayer{ false };
        bool isPlayerLike{ false };
    };

    class ProxyResolver final
    {
    public:
        static ProxyResolver* GetSingleton();

        [[nodiscard]] ActorIdentity Describe(const RE::Actor* actor) const;
        [[nodiscard]] bool IsCandidateProxy(const RE::Actor* actor) const;

    private:
        ProxyResolver() = default;
    };
}
