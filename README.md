# AnimSync Together

AnimSync Together is an experimental SKSE/CommonLibSSE-NG plugin for Skyrim Special Edition / Anniversary Edition designed to improve animation synchronization between Skyrim Together Reborn clients.

## Status

**v0.10.0 — OAR condition-cache refresh + replacement diagnostics**

The STRPM transport, proxy resolution and `OffsetGPMA` replay path are validated. v0.9.2 proved that the local PlayerCharacter and the STR proxy do not receive the same effective OAR replacement for `Animations\GPMAOffsetAnimation.hkx`:

- local PlayerCharacter: effective clip duration around 3.3 seconds;
- STR proxy: effective clip duration around 9.133 seconds and an internal `OffsetGPMAStop` roughly 0.2 seconds after activation.

`HT_NPCSpellMonitor` is present on the STR proxy, so v0.10.0 integrates the official Open Animation Replacer Animations API and explicitly clears OAR condition state for the proxy after AnimSync injects that spell and again immediately before a remote `OffsetGPMA` replay. This forces OAR to re-evaluate replacement conditions using the proxy's current state instead of potentially reusing condition state created before the monitor spell was added.

v0.10.0 also queries OAR's `GetCurrentReplacementAnimationInfo` API for each GPMA clip activation and logs the selected replacement's mod, submod, animation path and variant. This makes it possible to compare the exact Helmet Toggle replacement chosen for the local player and remote proxy.

The synchronized behavior inputs remain:

- `OffsetGPMA`
- `OffsetGPMAStop`

Animations already reproduced natively by STR, such as sneak, jump and furniture transitions, are not retransmitted.

## Runtime dependency

- STRPluginMessagingAPI v0.8.2 or newer
- Helmet Toggle 2
- Open Animation Replacer with Animations API v1
- Offset Movement Animation
- Dynamic Armor Variants

## Logging

Logs are written to:

`Documents/My Games/Skyrim Special Edition/SKSE/AnimSyncTogether.log`

Useful v0.10.0 markers:

- `OARClient: OAR Animations API v1 connected`
- `OARConditionCache: cleared actor=...`
- `ClipProbeArm actor=... reason='local OffsetGPMA'`
- `ClipProbeArm actor=... reason='remote OffsetGPMA'`
- `ClipSelection ... duration=... oarReplacement=... oarMod='...' oarSubMod='...' oarPath='...' oarVariant='...'`
- `GPMAState ... variable='iGPMAOffsetType' ...`
- `AnimTxInput event='OffsetGPMA'`
- `AnimRxInput ... event='OffsetGPMA' replay=true result=true`
- `HelmetToggleCompat: add spell ... result=true`

## Build

Run `build_release.bat` to create:

`dist/AnimSyncTogether-v0.10.0.zip`

## License

MIT
