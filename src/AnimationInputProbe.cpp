#include "AnimSyncTogether/AnimationInputProbe.h"

namespace AnimSyncTogether
{
    void AnimationInputProbe::Install()
    {
        if (installed_) {
            return;
        }

        REL::Relocation<std::uintptr_t> playerGraphVTable{ RE::VTABLE_PlayerCharacter[3] };
        originalPlayerNotify_ = playerGraphVTable.write_vfunc(0x1, PlayerNotifyAnimationGraph);
        installed_ = true;

        SKSE::log::info("AnimationInputProbe: PlayerCharacter NotifyAnimationGraph hook installed");
    }

    bool AnimationInputProbe::PlayerNotifyAnimationGraph(
        RE::IAnimationGraphManagerHolder* holder,
        const RE::BSFixedString& eventName)
    {
        const auto result = originalPlayerNotify_(holder, eventName);

        SKSE::log::info(
            "AnimInput actor=00000014 localPlayer=true event='{}' result={}",
            eventName.c_str(),
            result);

        return result;
    }
}
