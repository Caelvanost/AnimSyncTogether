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

        if (name == "OffsetGPMA") {
            AnimationClipProbe::ArmActor(0x14, "local OffsetGPMA");

            std::int32_t offsetType = 0;
            const RE::BSFixedString offsetTypeVariable{ "iGPMAOffsetType" };
            const bool hasOffsetType = holder && holder->GetGraphVariableInt(offsetTypeVariable, offsetType);
            SKSE::log::info(
                "GPMAState actor=00000014 localPlayer=true variable='iGPMAOffsetType' present={} value={}",
                hasOffsetType,
                offsetType);
        }

        const auto result = originalPlayerNotify_(holder, eventName);

        SKSE::log::info(
            "AnimInput actor=00000014 localPlayer=true event='{}' result={}",
            name,
            result);

        // v0.6.0 diagnostics established that OffsetGPMA is the actual graph
        // input that starts the helmet-removal sequence. OffsetGPMAStop ends it.
        // Only forward inputs that the local PlayerCharacter graph accepted.
        if (result && IsHelmetInput(name)) {
            STRPMClient::GetSingleton()->SendAnimationEvent(name, {});
        }

        return result;
    }
}
