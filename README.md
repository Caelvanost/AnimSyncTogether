# AnimSync Together

AnimSync Together is an experimental SKSE/CommonLibSSE-NG plugin for Skyrim Special Edition / Anniversary Edition designed to improve animation synchronization between Skyrim Together Reborn clients.

## Status

**v0.9.2 — OAR clip-selection diagnostics (CommonLib flatrim compatibility)**

The STRPM transport, proxy resolution and behavior replay path are validated. `OffsetGPMA` reaches the remote STR proxy, returns `true`, and the proxy emits the expected GPMA graph events (`AnimObjectUnequip`, `AnimObjLoad`, `AnimObjDraw`).

v0.8.0 also confirmed that `HT_NPCSpellMonitor` can be resolved and added successfully to STR proxies, but the Helmet Toggle 2 animation is still visually absent.

A key runtime difference remains: the remote proxy falls back to a very short GPMA sequence while the local player runs the full Helmet Toggle animation. v0.9.x therefore hooks `hkbClipGenerator::Activate` after OAR has installed its own hook and logs the animation binding selected during a 1.5 second window around `OffsetGPMA`.

For each relevant clip activation AnimSync logs:

- the binding index before activation;
- the binding index selected after the chained activation hook;
- whether that binding index changed;
- the clip generator animation name;
- the resulting animation duration;
- the actor and whether it is the local player or an STR proxy.

v0.9.2 removes the diagnostic dependency on `hkbCharacterSetup`, `hkbCharacterData` and `hkbCharacterStringData`, because those internal headers are not shipped by the `commonlibsse-ng-flatrim` 3.5.3 package used by the project. The remaining diagnostics are sufficient to compare OAR selection between the local PlayerCharacter and the STR proxy without relying on unavailable Havok internals.

The synchronized behavior inputs remain:

- `OffsetGPMA`
- `OffsetGPMAStop`

Animations already reproduced natively by STR, such as sneak, jump and furniture transitions, are not retransmitted.

## Runtime dependency

- STRPluginMessagingAPI v0.8.2 or newer
- Helmet Toggle 2
- Open Animation Replacer
- Offset Movement Animation
- Dynamic Armor Variants

## Logging

Logs are written to:

`Documents/My Games/Skyrim Special Edition/SKSE/AnimSyncTogether.log`

Useful v0.9.2 markers:

- `ClipProbeArm actor=... reason='local OffsetGPMA'`
- `ClipProbeArm actor=... reason='remote OffsetGPMA'`
- `ClipSelection ... beforeIndex=... selectedIndex=... replaced=... clipName='...' duration=...`
- `GPMAState ... variable='iGPMAOffsetType' ...`
- `AnimTxInput event='OffsetGPMA'`
- `AnimRxInput ... event='OffsetGPMA' replay=true result=true`
- `HelmetToggleCompat: add spell ... result=true`

## Build

Run `build_release.bat` to create:

`dist/AnimSyncTogether-v0.9.2.zip`

## License

MIT
