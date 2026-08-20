# AnimSync Together

AnimSync Together is an experimental SKSE/CommonLibSSE-NG plugin for Skyrim Special Edition / Anniversary Edition designed to improve animation synchronization between Skyrim Together Reborn clients.

## Status

**v0.11.4 — Helmet Toggle stale hand-marker cleanup**

Inspection of Helmet Toggle 2 v3.6.1 showed that its OAR replacements are selected primarily from graph state rather than a PlayerCharacter-only condition. In particular, Helmet Toggle writes `iGPMAAnimationType` before sending `OffsetGPMA`, and its equip/unequip families use that state to select the matching OAR replacement.

AnimSync v0.11.x synchronizes the graph state required by Helmet Toggle:

- `iGPMAAnimationType`
- `iGPMAOffsetType`
- `OffsetGPMA`
- `OffsetGPMAStop`

On the sending PlayerCharacter, AnimSync captures both graph variables immediately before the graph input is processed. On the receiving STR proxy, AnimSync applies the transmitted variables before replaying `OffsetGPMA`, clears OAR condition state so the replacement is evaluated against the new graph state, then replays the input.

v0.11.0 validated this approach in two-client testing: the remote STR proxy selected the same Helmet Toggle OAR replacement and the same effective animation duration as the local player.

v0.11.1 removed the temporary `HT_NPCSpellMonitor` injection introduced during earlier diagnostics and added local `iGPMAAnimationType` reset handling when a proxy emits its own `OffsetGPMAStop`.

v0.11.2 fixed the CommonLib constness issue in that proxy-output reset path by resolving a mutable `RE::Actor*` from the event actor's FormID before changing graph variables.

A local v0.11.3 test build attempted to force cleanup by replaying `AnimObjectUnequip` through `NotifyAnimationGraph`. Runtime logs showed `result=false` consistently. That confirms the earlier v0.5 finding: `AnimObjectUnequip` is an animation graph output, not a valid input, so replaying it cannot remove the stale hand object.

v0.11.3 logs also exposed the actual residual-object mechanism. Helmet Toggle ships an IED NodeMonitor named `Helmet on hand` that watches the `AnimObjectHelmetInvisible` marker under `AnimObjectR`. On an STR proxy, `AnimObjectHelmetInvisible` can be loaded/drawn again after `OffsetGPMAStop`, leaving IED convinced that the helmet is still being held.

v0.11.4 therefore cleans the scene graph directly instead of replaying an output event:

- when a remote STR proxy emits `OffsetGPMAStop`, AnimSync marks that proxy's GPMA sequence as stopped and queues removal of `AnimObjectHelmetInvisible` from the proxy's loaded 3D;
- when a fresh `AnimObjLoad(AnimObjectGPMA)` begins, the stopped state is cleared so the marker is allowed during the next legitimate animation;
- if `AnimObjLoad` or `AnimObjDraw` recreates `AnimObjectHelmetInvisible` after the stop, AnimSync queues another removal;
- cleanup is queued through the SKSE task interface so the scene graph is not mutated inside the animation-event dispatch itself;
- only STR proxies are affected; local players and ordinary NPCs are untouched.

The OAR Animations API and clip-selection diagnostics remain enabled so the selected replacement can still be compared between local player and proxy.

Animations already reproduced natively by STR, such as normal locomotion, sneak, jump and furniture transitions, are not retransmitted.

## Runtime dependency

- STRPluginMessagingAPI v0.8.2 or newer
- Helmet Toggle 2
- Open Animation Replacer with Animations API v1
- Offset Movement Animation
- Dynamic Armor Variants
- Immersive Equipment Displays for Helmet Toggle's hand display

## Logging

Logs are written to:

`Documents/My Games/Skyrim Special Edition/SKSE/AnimSyncTogether.log`

Useful v0.11.4 markers:

- `GPMAStateTx ... animationType=... offsetType=...`
- `AnimTxInput ... stateFlags=... animationType=... offsetType=...`
- `GPMAStateApply ... animationTypeApplied=true ...`
- `AnimRxInput ... event='OffsetGPMA' replay=true result=true ...`
- `GPMAStateReset ... source='network stop'`
- `GPMAStateAutoReset ... source='proxy graph output'`
- `GPMAHelmetMarkerCleanup ... removed=... remaining=... reason='proxy OffsetGPMAStop'`
- `GPMAHelmetMarkerCleanup ... reason='late AnimObjectHelmetInvisible after GPMA stop'`
- `ClipSelection ... oarReplacement=... oarMod='...' oarSubMod='...' oarPath='...' oarVariant='...'`

The old v0.11.3 marker `GPMAAnimObjectCleanup ... result=false` should no longer appear.

For a successful stale-marker cleanup, the useful result is normally:

`GPMAHelmetMarkerCleanup ... removed=1 remaining=false ...`

If the animation system recreates the marker after the initial stop cleanup, a second cleanup with reason `late AnimObjectHelmetInvisible after GPMA stop` should remove that late copy as well.

## Build

Run `build_release.bat` to create:

`dist/AnimSyncTogether-v0.11.4.zip`

## License

MIT
