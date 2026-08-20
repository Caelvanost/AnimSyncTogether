#include "AnimSyncTogether/AnimationInputProbe.h"

#include "AnimSyncTogether/AnimationClipProbe.h"
#include "AnimSyncTogether/STRPMClient.h"

namespace AnimSyncTogether
{
    namespace
    {
        bool IsHelmetInput(std::string_view eventName)
        {
            return eventName == "OffsetGPMA" || eventName == "OffsetGPMAStop";
        }
    }

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
        const std::string_view name = eventName.c_str();

        std::int32_t animationType = 0;
        std::int32_t offsetType = 0;
        bool hasAnimationType = false;
        bool hasOffsetType = false;

        if (IsHelmetInput(name)) {
            const RE::BSFixedString animationTypeVariable{ "iGPMAAnimationType" };
            const RE::BSFixedString offsetTypeVariable{ "iGPMAOffsetType" };
            hasAnimationType = holder && holder->GetGraphVariableInt(animationTypeVariable, animationType);
            hasOffsetType = holder && holder->GetGraphVariableInt(offsetTypeVariable, offsetType);

            SKSE::log::info(
                "GPMAStateTx actor=00000014 event='{}' animationTypePresent={} animationType={} offsetTypePresent={} offsetType={}",
                name,
                hasAnimationType,
                animationType,
                hasOffsetType,
                offsetType);
        }

        if (name == "OffsetGPMA") {
            AnimationClipProbe::ArmActor(0x14, "local OffsetGPMA");
        }

        const auto result = originalPlayerNotify_(holder, eventName);

        SKSE::log::info(
            "AnimInput actor=00000014 localPlayer=true event='{}' result={}",
            name,
            result);

        // Helmet Toggle writes iGPMAAnimationType/iGPMAOffsetType before
        // OffsetGPMA, then sends OffsetGPMAStop before resetting animation type.
        // Capture the state before the graph call and only transmit accepted inputs.
        if (result && IsHelmetInput(name)) {
            STRPMClient::GetSingleton()->SendAnimationEvent(
                name,
                {},
                animationType,
                hasAnimationType,
                offsetType,
                hasOffsetType);
        }

        return result;
    }
}
