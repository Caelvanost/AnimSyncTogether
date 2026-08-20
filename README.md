# AnimSync Together

AnimSync Together is an experimental SKSE/CommonLibSSE-NG plugin for Skyrim Special Edition / Anniversary Edition designed to improve animation synchronization between Skyrim Together Reborn clients.

## Status

**v0.8.0 — Helmet Toggle NPC/OAR compatibility probe**

The STRPM transport and proxy replay path are validated. In v0.7.0, `OffsetGPMA` reached the remote STR proxy, returned `true`, and produced the expected GPMA graph outputs (`AnimObjectUnequip`, `AnimObjLoad`, `AnimObjDraw`). The animation was still visually absent, which points to OAR selecting the generic GPMA behavior instead of Helmet Toggle 2's NPC animation replacement.

Helmet Toggle 2 documents that NPC/follower animations are enabled through monitor spells such as `HT_NPCSpellMonitor`. v0.8.0 therefore resolves `HT_NPCSpellMonitor` by EditorID and temporarily adds it to STR proxy actors while their STRPM mapping exists. AnimSync only removes the spell if AnimSync itself added it.

The synchronized behavior inputs remain:

- `OffsetGPMA`
- `OffsetGPMAStop`

Animations already reproduced natively by STR, such as sneak, jump and furniture transitions, are not retransmitted.

## Runtime dependency

- STRPluginMessagingAPI v0.8.2 or newer
- Helmet Toggle 2
- Open Animation Replacer
- Offset Movement Animation
- Dynamic Armor Variants

If `HT_NPCSpellMonitor` cannot be found, AnimSync logs a warning and leaves the proxy unchanged.

## Logging

Logs are written to:

`Documents/My Games/Skyrim Special Edition/SKSE/AnimSyncTogether.log`

Useful v0.8.0 markers:

- `AnimTxInput event='OffsetGPMA'`
- `AnimRxInput ... event='OffsetGPMA' replay=true result=true`
- `HelmetToggleCompat: add spell ... result=true`
- `HelmetToggleCompat: spell 'HT_NPCSpellMonitor' not found`
- `HelmetToggleCompat: remove spell ...`

## Build

Run `build_release.bat` to create:

`dist/AnimSyncTogether-v0.8.0.zip`

## License

MIT
