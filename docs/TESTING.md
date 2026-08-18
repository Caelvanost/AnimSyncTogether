# v0.2.0 test procedure

Until the STRPluginMessagingAPI player/proxy registry is available, AnimSync Together tests only local animation capture.

1. Run `build_release.bat`.
2. Install `dist/AnimSyncTogether-v0.2.0.zip` with Vortex on one test client.
3. Launch through the normal SKSE/STR setup and load a save.
4. Perform a controlled sequence on the local player:
   - idle
   - crouch / stand
   - jump
   - draw / sheath weapon
   - sit on a chair
   - get up
5. Collect `Documents/My Games/Skyrim Special Edition/SKSE/AnimSyncTogether.log`.

## Expected result

The plugin should log animation graph events for the local player only. It must not:

- scan `ProcessLists` for remote-player candidates;
- classify `0xFFxxxxxx` actors as STR proxies;
- listen for object-load events solely to discover proxies;
- alter remote actor positions;
- replay remote animations;
- send custom network messages yet.

Useful data at this stage:

- animation tag;
- animation payload;
- local actor FormID/base FormID;
- repeated/noisy events that should be filtered before networking.

## Next multiplayer test

After STRPM exposes its player/proxy registry, install AnimSync Together on both clients and verify that AnimSync receives the proxy FormID from STRPM and attaches its animation sink to that actor without performing its own proxy discovery.
