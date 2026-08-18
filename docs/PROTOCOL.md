# AnimSync Together protocol notes

## Responsibility boundary

AnimSync Together does not discover Skyrim Together Reborn player proxies.

STRPluginMessagingAPI (STRPM) is responsible for identifying connected players and resolving the local proxy FormID associated with each remote player. AnimSync Together consumes that mapping and only performs animation-specific work on the actor returned by that FormID.

A proxy FormID is local to a client and must never be treated as a network-global player identifier.

## Animation message shape

Captured animation events will eventually be reduced to a transport-neutral message containing:

- source STR connection identity supplied by STRPM;
- animation graph tag;
- optional payload;
- monotonic sequence number;
- local timestamp.

The receiving client will use STRPM's player registry to resolve the source connection identity to the current local proxy FormID before replay.

## Planned progression

1. Capture and classify local animation graph events.
2. Filter noisy/redundant events.
3. Integrate STRPM player/proxy registry.
4. Compare events already reproduced by STR with events missing on remote proxies.
5. Add deduplication and loop prevention.
6. Send animation messages through STRPM.
7. Replay a small allow-list of deterministic animation events.
8. Extend to furniture transitions.
9. Expose integration hooks for OStim Together.

## Explicitly out of scope for AnimSync Together

- scanning `RE::ProcessLists` to discover STR proxies;
- identifying proxies from `0xFFxxxxxx` FormIDs;
- matching remote players by actor name;
- maintaining connected-player membership;
- implementing a separate LAN discovery or transport stack.
