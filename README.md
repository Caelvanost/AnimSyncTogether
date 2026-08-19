# AnimSync Together

AnimSync Together is an experimental SKSE/CommonLibSSE-NG plugin for Skyrim Special Edition / Anniversary Edition designed to improve animation synchronization between Skyrim Together Reborn clients.

## Status

**v0.4.0 — STRPM animation transport probe**

Local animation capture and STRPM ProxyResolver integration are working. v0.4.0 adds the first dedicated AnimSync STRPM channel and forwards only the helmet-related animation-object events that were observed locally on Player1 but missing from the remote proxy on Player2.

Current allow-list:

- `AnimObjLoad` with payload `AnimObjectGPMA`;
- `AnimObjDraw` with payload `AnimObjectGPMA`;
- `AnimObjLoad` with payload `AnimObjectHelmetInvisible`;
- `AnimObjDraw` with payload `AnimObjectHelmetInvisible`.

Received packets are resolved through STRPM `ConnectionID -> local proxy FormID` and logged as `AnimRx`. **v0.4.0 does not replay them yet.** This is intentional: the payload is significant and must be preserved before choosing the correct replay injection point.

Animations already reproduced natively by STR, such as sneak, jump and furniture transitions, are not retransmitted by AnimSync.

## Responsibilities

AnimSync Together owns animation capture, filtering, transport payloads, deduplication and replay. STRPluginMessagingAPI owns connected-player identity, proxy resolution and transport.

AnimSync Together does not scan `ProcessLists`, infer proxies from dynamic FormIDs, or match actors by name.

## Runtime dependency

v0.4.0 expects STRPluginMessagingAPI v0.8.2 or newer with ProxyResolver API v1 installed on both clients.

## Logging

Logs are written to:

`Documents/My Games/Skyrim Special Edition/SKSE/AnimSyncTogether.log`

Useful v0.4.0 markers:

- `AnimTx` — allow-listed local animation packet sent through STRPM;
- `AnimRx` — packet received and resolved to the sender's local proxy FormID;
- `replay=false` — diagnostic transport milestone; no graph mutation yet.

## Build

Run `build_release.bat` to create:

`dist/AnimSyncTogether-v0.4.0.zip`

## License

MIT
