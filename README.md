# AnimSync Together

AnimSync Together is an experimental SKSE/CommonLibSSE-NG plugin for Skyrim Special Edition / Anniversary Edition designed to improve animation synchronization between Skyrim Together Reborn clients.

## Status

**v0.12.0 — data-driven custom animation synchronization foundation**

v0.11.x validated the core approach with Helmet Toggle 2: custom animation replacement is often controlled by animation graph state that Skyrim Together Reborn does not reproduce on its remote proxies. Synchronizing the required graph variables before replaying a custom graph input allows Open Animation Replacer to select the same replacement on the remote client.

The Helmet Toggle path remains validated and retained in v0.12.0, including the stale `AnimObjectHelmetInvisible` scene-graph cleanup introduced in v0.11.4.

v0.12.0 generalizes the transport so compatibility with additional animation mods no longer has to be hardcoded into the DLL.

### Generic synchronization model

AnimSync now supports two ordered message kinds on the existing STRPM animation channel:

1. `GraphVariable`
   - bool
   - int
   - float
2. `AnimationEvent`

Both message kinds use the same reliable + ordered channel. When a local PlayerCharacter graph input occurs, AnimSync first captures all graph variables listed by active rule profiles. Only changed values are transmitted. If the graph input itself is listed as a synchronized event, that event is sent afterwards on the same channel.

The receiving client therefore processes:

```text
custom graph variables
        ↓
remote STR proxy
        ↓
OAR condition cache refresh
        ↓
custom animation event, if profiled
        ↓
OAR selects the replacement from synchronized state
```

Native Skyrim / STR animation inputs are not globally replayed. This is deliberate: locomotion, normal combat, furniture and other STR-owned behavior must not be double-driven by AnimSync.

## Rule profiles

Profiles are plain text files installed in:

```text
Data/SKSE/Plugins/AnimSyncTogether/Rules/
```

Files may use `.rule` or `.rules`.

Supported directives:

```text
# comment
event CustomAnimationEvent
bool bCustomVariable
int iCustomVariable
float fCustomVariable
```

- `event` opts one specific graph input into network replay.
- `bool`, `int` and `float` opt specific graph variables into synchronization.
- duplicate rules are ignored.
- conflicting types for the same variable are rejected and logged.

This means a new animation mod can be supported by adding a profile rather than rebuilding AnimSync Together.

### Helmet Toggle 2 profile

v0.12.0 packages:

```text
SKSE/Plugins/AnimSyncTogether/Rules/HelmetToggle2.rules
```

with:

```text
event OffsetGPMA
event OffsetGPMAStop
int iGPMAAnimationType
int iGPMAOffsetType
```

The previously validated GPMA state embedded in Helmet Toggle event packets is temporarily retained as a redundant compatibility path while the generic graph-variable transport is validated in two-client testing.

If external rule files are missing entirely, AnimSync falls back to the same built-in Helmet Toggle rules so the previously working behavior is not lost.

## Helmet Toggle compatibility history

- v0.11.0 synchronized `iGPMAAnimationType`, `iGPMAOffsetType`, `OffsetGPMA` and `OffsetGPMAStop`, allowing the remote proxy to select the same OAR replacement and animation duration as the local player.
- v0.11.1 removed the temporary `HT_NPCSpellMonitor` injection and reset stale GPMA graph state on proxy stop outputs.
- v0.11.2 fixed CommonLib constness in that reset path.
- v0.11.4 fixed the residual helmet-in-hand display by removing stale `AnimObjectHelmetInvisible` nodes from STR proxy scene graphs, including marker nodes recreated after `OffsetGPMAStop`.

## Runtime dependencies

Core:

- Skyrim Together Reborn
- SKSE
- STRPluginMessagingAPI v0.8.2 or newer

For OAR-driven custom animations:

- Open Animation Replacer

Additional dependencies remain specific to the animation mod being synchronized. For example Helmet Toggle also uses Offset Movement Animation, Dynamic Armor Variants and Immersive Equipment Displays.

## Logging

Logs are written to:

```text
Documents/My Games/Skyrim Special Edition/SKSE/AnimSyncTogether.log
```

Useful v0.12.0 markers:

```text
SyncRules: loading profile=...
SyncRules: initialized profiles=... events=... graphVariables=...
GraphVarTx name='...' type=... value=...
GraphVarRx ... name='...' type=... value=... applied=true
AnimInput ... event='...' result=true tracked=true
AnimTxInput event='...'
AnimRxInput ... event='...' replay=true result=true
```

Helmet Toggle diagnostics remain available:

```text
GPMAStateTx ...
GPMAStateApply ...
GPMAStateReset ...
GPMAStateAutoReset ...
GPMAHelmetMarkerCleanup ...
ClipSelection ...
```

For the packaged Helmet Toggle profile, startup should report at least:

```text
SyncRules: initialized profiles=1 events=2 graphVariables=2
```

During a helmet animation, the generic path should additionally show graph variable traffic such as:

```text
GraphVarTx name='iGPMAAnimationType' type=int value=2
GraphVarRx ... name='iGPMAAnimationType' type=int value=2 applied=true
```

while the existing GPMA event replay continues to work.

## Adding another animation mod

The intended workflow is:

1. inspect the mod's OAR conditions and behavior scripts/configuration;
2. identify custom graph variables that determine replacement selection;
3. identify custom graph events that are not already reproduced by STR;
4. add a small `.rules` profile;
5. compare local and remote `ClipSelection` logs.

Mods that only depend on custom graph variables but use vanilla/STR-owned behavior events may need variable rules only, with no `event` directive at all.

## Build

Run:

```text
build_release.bat
```

to create:

```text
dist/AnimSyncTogether-v0.12.0.zip
```

The archive includes the DLL and packaged rule profiles.

## License

MIT
