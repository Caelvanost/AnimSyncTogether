# AnimSync Together

AnimSync Together is an experimental SKSE/CommonLibSSE-NG plugin for Skyrim Special Edition / Anniversary Edition designed to improve animation synchronization between Skyrim Together Reborn clients.

## Status

**v0.2.0 — animation capture boundary**

Local animation graph capture is working. AnimSync Together is intentionally limited to animation-related responsibilities: capture, classification, filtering, transport payloads, deduplication, and replay.

Remote-player identity and STR proxy discovery are **not** implemented in AnimSync Together. They are delegated to STRPluginMessagingAPI (STRPM), which will expose the local proxy FormID associated with each connected player.

No gameplay-facing synchronization is enabled yet.

## Responsibilities

AnimSync Together owns:

- observing animation graph events;
- classifying and filtering useful animation events;
- building animation synchronization messages;
- deduplication and loop prevention;
- replaying approved animation events on an actor supplied by STRPM;
- later furniture and OStim-related animation synchronization.

STRPluginMessagingAPI owns:

- connected-player identity;
- mapping STR players to their local proxy actors;
- exposing the local proxy FormID for each relevant remote player;
- notifying consumers when that mapping appears, changes, or disappears;
- transport between connected clients.

AnimSync Together must not independently scan `ProcessLists` or guess proxy actors from dynamic FormIDs.

## Logging

After loading the game, logs are written to:

`Documents/My Games/Skyrim Special Edition/SKSE/AnimSyncTogether.log`

## Build

Requirements:

- Visual Studio 2022/2026 with C++ desktop development tools
- CMake
- vcpkg
- CommonLibSSE-NG

Run `build_release.bat` to create the Vortex-ready archive in `dist`.

## License

MIT
