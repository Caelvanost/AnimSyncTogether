# v0.3.0 test procedure

This milestone validates AnimSync Together's integration with the STRPluginMessagingAPI v0.8.2 ProxyResolver. It does not replay or send animation events yet.

## Setup

1. Install STRPluginMessagingAPI v0.8.2 on both clients and make sure the server uses the required relay v3 resource.
2. Run `build_release.bat`.
3. Install `dist/AnimSyncTogether-v0.3.0.zip` with Vortex on both clients.
4. Launch both clients through the normal STR/SKSE setup.
5. Connect both players to the same STR server and place them in the same cell.

## Controlled test

Keep Player2 idle. On Player1 perform:

- sneak;
- unsneak;
- jump;
- sit down on a simple chair;
- get up.

Then optionally repeat in the opposite direction with Player1 idle and Player2 performing the sequence.

## Expected AnimSync log behavior

Each client should show:

- `STRPMClient: ProxyResolver listener registered`;
- a `proxy mapping added` line for the remote player;
- `AnimationProbe: sink attached ... remoteConnection=<non-zero>` for the remote proxy;
- normal local-player `AnimEvent` lines with `remoteConnection=0`;
- remote-proxy `AnimEvent` lines, when STR itself drives corresponding graph events, with `remoteConnection=<non-zero>`.

Example shape:

```text
STRPMClient: proxy mapping added connection=123456 formID=FF001234
AnimationProbe: sink attached actor=FF001234 ... localPlayer=false remoteConnection=123456 ...
AnimEvent actor=00000014 ... localPlayer=true remoteConnection=0 tag='JumpUp' ...
AnimEvent actor=FF001234 ... localPlayer=false remoteConnection=123456 tag='JumpUp' ...
```

## What this test determines

Compare Player1's local event sequence with the events observed on Player1's proxy in Player2's log. Events already reproduced by STR should not be retransmitted by AnimSync. Events consistently missing on the proxy become candidates for the future STRPM animation-message allow-list.

## Failure cases worth reporting

- ProxyResolver listener is unavailable.
- STRPM reports a mapping but AnimSync cannot look up the FormID.
- The proxy actor has no animation graph manager when the mapping arrives.
- The proxy mapping is added but no sink is attached.
- Mapping update/removal leaves an old sink attached.

Collect from both machines:

`Documents/My Games/Skyrim Special Edition/SKSE/AnimSyncTogether.log`
