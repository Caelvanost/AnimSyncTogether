# AnimSync Together

AnimSync Together is an experimental SKSE/CommonLibSSE-NG plugin for Skyrim Special Edition / Anniversary Edition designed to improve animation synchronization between Skyrim Together Reborn clients.

## Status

**v0.3.0 — STRPM proxy registry integration**

Local animation graph capture is working. AnimSync Together now consumes the validated STRPluginMessagingAPI (STRPM) ProxyResolver and attaches its animation probe to remote-player proxy actors using the local FormIDs supplied by STRPM.

No gameplay-facing animation replay is enabled yet. v0.3.0 is a diagnostic multiplayer milestone used to compare local animation events with the events STR already generates on the corresponding remote proxy.

## Responsibilities

AnimSync Together owns:

- observing animation graph events;
- attaching animation sinks to actors explicitly supplied by STRPM;
- classifying and filtering useful animation events;
- building animation synchronization messages;
- deduplication and loop prevention;
- replaying approved animation events in later milestones;
- later furniture and OStim-related animation synchronization.

STRPluginMessagingAPI owns:

- connected-player identity;
- mapping STR players to their local proxy actors;
- exposing `ConnectionID -> local proxy FormID`;
- notifying consumers when that mapping appears, changes, disappears, or is cleared;
- transport between connected clients.

AnimSync Together does not scan `ProcessLists`, infer proxies from `0xFFxxxxxx`, or match actors by name.

## Runtime dependency

v0.3.0 expects STRPluginMessagingAPI v0.8.2 or newer with ProxyResolver API v1 installed on the client.

The consumer API header is vendored in `include/STRPluginMessagingAPI/` so AnimSync can compile independently; at runtime it dynamically loads `STRPluginMessagingAPI.dll`.

## Logging

After loading the game, logs are written to:

`Documents/My Games/Skyrim Special Edition/SKSE/AnimSyncTogether.log`

Proxy events include the STRPM `remoteConnection` value so local and remote logs can be correlated without relying on actor names.

## Build

Requirements:

- Visual Studio 2022/2026 with C++ desktop development tools
- CMake
- vcpkg
- CommonLibSSE-NG

Run `build_release.bat` to create:

`dist/AnimSyncTogether-v0.3.0.zip`

## License

MIT
