# AnimSync Together

AnimSync Together is an experimental SKSE/CommonLibSSE-NG plugin for Skyrim Special Edition / Anniversary Edition designed to improve animation synchronization between Skyrim Together Reborn clients.

## Status

**v0.11.2 — Helmet Toggle GPMA state cleanup compile fix**

Inspection of Helmet Toggle 2 v3.6.1 showed that its OAR replacements are selected primarily from graph state rather than a PlayerCharacter-only condition. In particular, Helmet Toggle writes `iGPMAAnimationType` before sending `OffsetGPMA` and sends `OffsetGPMAStop` before resetting that variable to zero.

AnimSync v0.11.x synchronizes the graph state required by Helmet Toggle:

- `iGPMAAnimationType`
- `iGPMAOffsetType`
- `OffsetGPMA`
- `OffsetGPMAStop`

On the sending PlayerCharacter, AnimSync captures both graph variables immediately before the graph input is processed. On the receiving STR proxy, AnimSync applies the transmitted variables before replaying `OffsetGPMA`, clears OAR condition state so the replacement is evaluated against the new graph state, then replays the input.

v0.11.0 validated this approach in two-client testing: the remote STR proxy selected the same Helmet Toggle OAR replacement and the same effective animation duration as the local player.

v0.11.1 removes the temporary `HT_NPCSpellMonitor` injection introduced during earlier diagnostics. The monitor spell is not required once the GPMA graph state itself is synchronized, and leaving it on STR proxies could cause Helmet Toggle's NPC management logic to run in parallel with AnimSync.

v0.11.1 also handles GPMA clips that emit their own `OffsetGPMAStop` output before a corresponding local stop input can be transported. When an STR proxy emits `OffsetGPMAStop`, AnimSync resets that proxy's `iGPMAAnimationType` to zero locally. Output events remain diagnostic only and are never retransmitted over STRPM.

v0.11.2 fixes the CommonLib constness issue in that proxy-output reset path. Animation graph event holders are exposed as const through the event callback, so AnimSync now uses the event actor only to obtain its FormID and then resolves a mutable `RE::Actor*` with `LookupByID` before calling `SetGraphVariableInt`.

The OAR Animations API and clip-selection diagnostics remain enabled so the selected replacement can still be compared between local player and proxy.

Animations already reproduced natively by STR, such as normal locomotion, sneak, jump and furniture transitions, are not retransmitted.

## Runtime dependency

- STRPluginMessagingAPI v0.8.2 or newer
- Helmet Toggle 2
- Open Animation Replacer with Animations API v1
- Offset Movement Animation
- Dynamic Armor Variants

## Logging

Logs are written to:

`Documents/My Games/Skyrim Special Edition/SKSE/AnimSyncTogether.log`

Useful v0.11.2 markers:

- `GPMAStateTx ... animationType=... offsetType=...`
- `AnimTxInput ... stateFlags=... animationType=... offsetType=...`
- `GPMAStateApply ... animationTypeApplied=true ...`
- `AnimRxInput ... event='OffsetGPMA' replay=true result=true ...`
- `GPMAStateReset ... source='network stop'`
- `GPMAStateAutoReset ... source='proxy graph output'`
- `ClipSelection ... oarReplacement=... oarMod='...' oarSubMod='...' oarPath='...' oarVariant='...'`

There should no longer be any `HelmetToggleCompat: add spell ...` entries in v0.11.2 logs.

For a helmet unequip test, the expected local state is normally `iGPMAAnimationType=2`. The remote proxy should receive and apply the same value before `OffsetGPMA` is replayed, and should return to `iGPMAAnimationType=0` when the GPMA sequence stops.

## Build

Run `build_release.bat` to create:

`dist/AnimSyncTogether-v0.11.2.zip`

## License

MIT
