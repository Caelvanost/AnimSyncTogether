# v0.1.0 test procedure

1. Run `build_release.bat`.
2. Install `dist/AnimSyncTogether-v0.1.0.zip` with Vortex on both Skyrim Together Reborn clients.
3. Launch through the normal STR/SKSE setup.
4. Load both players into the same cell.
5. On Player1, perform a small controlled sequence:
   - idle
   - draw weapon
   - sheath weapon
   - jump
   - crouch / stand
   - activate a chair or other simple furniture
6. Repeat the same sequence on Player2.
7. Collect `Documents/My Games/Skyrim Special Edition/SKSE/AnimSyncTogether.log` from both machines.

## Expected v0.1.0 result

The plugin should only log animation graph events. It must not alter actor positions, play animations on remote actors, or send custom network messages yet.

The useful data for the next milestone is:

- event tag and payload
- local actor FormID/base FormID
- whether the actor is recognized as the local player
- whether a remote STR actor is detected as player-like
- which animation events are visible locally but missing remotely
