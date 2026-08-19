# v0.4.0 test procedure

This milestone validates the first AnimSync-specific STRPM animation channel. It transmits the helmet animation-object events identified as missing from STR's normal proxy animation stream, but does not replay them yet.

## Setup

1. Install STRPluginMessagingAPI v0.8.2 or newer on both clients.
2. Run `build_release.bat`.
3. Install `dist/AnimSyncTogether-v0.4.0.zip` with Vortex on both clients.
4. Connect both players to the same STR server and place them in the same cell.

## Controlled test

Keep Player2 idle. On Player1 / Kahel:

1. equip the helmet if needed;
2. trigger the helmet unequip / DAV display transition;
3. re-equip it;
4. repeat once if useful.

Normal sneak/jump/sit tests are optional; they should remain handled by STR and should not produce `AnimTx` entries.

## Expected Player1 log

For the helmet transition, expect local `AnimEvent` lines followed by `AnimTx` for allow-listed packets, especially:

```text
AnimTx tag='AnimObjLoad' payload='AnimObjectGPMA'
AnimTx tag='AnimObjDraw' payload='AnimObjectGPMA'
AnimTx tag='AnimObjLoad' payload='AnimObjectHelmetInvisible'
AnimTx tag='AnimObjDraw' payload='AnimObjectHelmetInvisible'
```

## Expected Player2 log

Expect the same packets to arrive through STRPM and resolve to Kahel's proxy:

```text
AnimRx sender=<ConnectionID> proxy=FF...... actor='Kahel' tag='AnimObjLoad' payload='...' replay=false
```

`replay=false` is expected in v0.4.0. The goal is to prove that the missing sequence and its payload survive transport intact and resolve to the correct proxy before implementing graph replay.

Collect `AnimSyncTogether.log` from both machines after the test.
