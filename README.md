# AnimSync Together

AnimSync Together is an experimental SKSE/CommonLibSSE-NG plugin for Skyrim Special Edition / Anniversary Edition designed to improve animation synchronization between Skyrim Together Reborn clients.

## Status

**v0.1.0 — animation probe scaffold**

The initial milestone focuses on observing local animation graph events, identifying relevant actors, and building the plumbing needed to map a local Skyrim Together player to the corresponding remote proxy.

No gameplay-facing synchronization is enabled yet.

## Initial goals

- Observe animation graph events on the local player.
- Keep the animation capture layer independent from IEDSync Together and OStim Together.
- Provide a reusable local-player / remote-proxy abstraction.
- Add a transport-facing message model without modifying Skyrim Together Reborn's native opcodes.
- Start with deterministic animation events before attempting furniture or OStim scene synchronization.
- Remain compatible with OAR/Pandora-based setups.

## Logging

After loading the game, logs are written to:

`Documents/My Games/Skyrim Special Edition/SKSE/AnimSyncTogether.log`

## Build

Requirements:

- Visual Studio 2022/2026 with C++ desktop development tools
- CMake
- vcpkg
- CommonLibSSE-NG

Configure with CMake and build the `Release` configuration.

## License

MIT
