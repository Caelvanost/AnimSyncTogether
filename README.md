# AnimSync Together

AnimSync Together is an experimental SKSE/CommonLibSSE-NG plugin for Skyrim Special Edition / Anniversary Edition designed to improve animation synchronization between Skyrim Together Reborn clients.

## Status

**v0.5.0 — first remote helmet replay**

Local animation capture, STRPM transport and STRPM ProxyResolver integration are working. v0.5.0 turns the validated Helmet Toggle 2 transport probe into the first gameplay-facing replay experiment.

The local player's `AnimObjLoad` event with payload `AnimObjectGPMA` is used as a stable marker that the helmet animation sequence has started. AnimSync sends one packet for that marker instead of forwarding every output event emitted by the graph.

On the receiving client, AnimSync resolves the authenticated STRPM sender to the current local proxy FormID and calls:

`NotifyAnimationGraph("AnimObjectUnequip")`

on that proxy on the game thread.

This intentionally replays the **behavior input event**, not the observed `AnimObjLoad` / `AnimObjDraw` output events. The proxy's own behavior/OAR setup is expected to produce the appropriate animation-object outputs locally.

Animations already reproduced natively by STR, such as sneak, jump and furniture transitions, are not retransmitted by AnimSync.

## Responsibilities

AnimSync Together owns animation capture, filtering, transport payloads, deduplication and replay. STRPluginMessagingAPI owns connected-player identity, proxy resolution and transport.

AnimSync Together does not scan `ProcessLists`, infer proxies from dynamic FormIDs, or match actors by name.

## Runtime dependency

v0.5.0 expects STRPluginMessagingAPI v0.8.2 or newer with ProxyResolver API v1 installed on both clients.

Helmet Toggle 2 / DAV / OAR remain responsible for the actual helmet animation and visibility behavior.

## Logging

Logs are written to:

`Documents/My Games/Skyrim Special Edition/SKSE/AnimSyncTogether.log`

Useful v0.5.0 markers:

- `AnimTx tag='AnimObjLoad' payload='AnimObjectGPMA'` — local helmet sequence trigger sent through STRPM;
- `AnimRx ... replay=true behaviorEvent='AnimObjectUnequip' result=true` — remote proxy accepted the behavior event;
- `result=false` — the proxy graph rejected or could not consume the behavior event.

If replay succeeds, the remote proxy should subsequently emit its own animation graph events for the helmet sequence. Those proxy events are logged but are never retransmitted, preventing loops.

## Build

Run `build_release.bat` to create:

`dist/AnimSyncTogether-v0.5.0.zip`

## License

MIT
