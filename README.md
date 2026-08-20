# AnimSync Together

AnimSync Together is an experimental SKSE/CommonLibSSE-NG plugin for Skyrim Special Edition / Anniversary Edition designed to improve animation synchronization between Skyrim Together Reborn clients.

## Status

**v0.11.0 — Helmet Toggle GPMA state synchronization**

Inspection of Helmet Toggle 2 v3.6.1 showed that its OAR replacements are selected primarily from graph state rather than a PlayerCharacter-only condition. In particular, Helmet Toggle writes `iGPMAAnimationType` before sending `OffsetGPMA` and sends `OffsetGPMAStop` before resetting that variable to zero.

AnimSync v0.11.0 therefore upgrades its animation packet to version 2 and synchronizes the graph state required by Helmet Toggle:

- `iGPMAAnimationType`
- `iGPMAOffsetType`
- `OffsetGPMA`
- `OffsetGPMAStop`

On the sending PlayerCharacter, AnimSync captures both graph variables immediately before the graph input is processed. On the receiving STR proxy, AnimSync applies the transmitted variables before replaying `OffsetGPMA`, clears OAR condition state so the replacement is evaluated against the new graph state, then replays the input. For `OffsetGPMAStop`, AnimSync mirrors Helmet Toggle's ordering by replaying the stop first and resetting `iGPMAAnimationType` to zero afterwards.

This should allow Helmet Toggle's OAR replacement to make the same selection on the remote STR proxy as on the local player instead of falling back to the generic GPMA animation.

The OAR Animations API and clip-selection diagnostics from v0.10.0 remain enabled so the selected replacement can still be compared between local player and proxy.

Animations already reproduced natively by STR, such as normal locomotion, sneak, jump and furniture transitions, are not retransmitted by this milestone.

## Runtime dependency

- STRPluginMessagingAPI v0.8.2 or newer
- Helmet Toggle 2
- Open Animation Replacer with Animations API v1
- Offset Movement Animation
- Dynamic Armor Variants

## Logging

Logs are written to:

`Documents/My Games/Skyrim Special Edition/SKSE/AnimSyncTogether.log`

Useful v0.11.0 markers:

- `GPMAStateTx ... animationType=... offsetType=...`
- `AnimTxInput ... stateFlags=... animationType=... offsetType=...`
- `GPMAStateApply ... animationTypeApplied=true ...`
- `AnimRxInput ... event='OffsetGPMA' replay=true result=true ...`
- `GPMAStateReset ... iGPMAAnimationType ... value=0 result=true`
- `ClipSelection ... oarReplacement=... oarMod='...' oarSubMod='...' oarPath='...' oarVariant='...'`
- `HelmetToggleCompat: add spell ... result=true`

For a helmet unequip test, the expected local state is normally `iGPMAAnimationType=2`. The remote proxy should receive and apply the same value before `OffsetGPMA` is replayed.

## Build

Run `build_release.bat` to create:

`dist/AnimSyncTogether-v0.11.0.zip`

## License

MIT
