# AnimSync Together protocol notes

## v0.1.0

The first version does **not** send animation events over the network.

Captured local events are reduced to a future transport-neutral message shape:

- source actor identity
- animation graph tag
- optional payload
- monotonic sequence number
- local timestamp

The transport layer will be introduced only after actor/proxy identification is verified in Skyrim Together Reborn logs.

## Planned progression

1. Capture and classify local animation graph events.
2. Verify which events already propagate through STR and which do not.
3. Resolve remote-player proxy actors reliably.
4. Add deduplication and loop prevention.
5. Introduce a transport adapter.
6. Replay a small allow-list of deterministic animation events.
7. Extend to furniture transitions.
8. Expose integration hooks for OStim Together.
