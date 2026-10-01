# eZeus architecture map

Repository navigation, runtime relationships, and persistent-schema boundaries. Paths are repository-relative; original Zeus/Poseidon files remain external runtime inputs.

## Future agent reading rule

| Task | Read first |
| --- | --- |
| Building | `CONTENT_EXTENSION_MAP.md` building section, then its closest analogue |
| Resource | `CONTENT_EXTENSION_MAP.md` resource section, then its closest analogue |
| Save-sensitive change | This document's serialization map, then the relevant content section |

Do not reread the entire repository unless these documents prove insufficient.

## Runtime topology

```text
main.cpp -> eMainWindow -> eCampaign
                         -> eWorldBoard / eWorldCity
                         -> eGameBoard -> eBoardCity / eBoardPlayer
                                       -> tiles, buildings, characters, events
                                       -> pathfinding and resource movement
```

`main.cpp` initializes SDL, `eGameDir`, settings, texture packs, audio, and `eMainWindow`. `eCampaign` owns the world board, parent/colony game boards, and episodes. Each `eGameBoard` registers live simulation objects and owns the tile grid.

## Subsystem map

| Subsystem | Primary paths and symbols | Non-obvious relationship |
| --- | --- | --- |
| Startup/runtime | `main.cpp`; `emainwindow.{h,cpp}`; `egamedir.{h,cpp}` | `eGameDir` joins staged eZeus assets to the external original-game root |
| Campaign/world/board | `engine/ecampaign.*`; `engine/eworldboard.*`; `engine/eworldcity.*`; `engine/egameboard*`; `engine/eboardcity.*`; `engine/eboardplayer.*`; `engine/etile.*` | Campaign owns parent/colony boards; board registries connect simulation objects |
| Buildings | `buildings/ebuilding.{h,cpp}` (`eBuildingType`, `eBuilding`); `allbuildings.h`; concrete `e*.{h,cpp}` | `eBuilding` registers during construction; derived bases supply employment, patrol, production, storage, housing, and monument behavior |
| Placement | `widgets/ebuildingmode.{h,cpp}`; `widgets/egamemenu.cpp`; `widgets/egamewidgetpaint.cpp`; `widgets/egamewidgetbuild.cpp`; `engine/egameboard.cpp` (`canBuild*`, `build`, `buildBase`) | Preview and build switches are separate; `buildBase` assigns footprint/tile ownership and charges `engine/edifficulty.cpp` cost |
| Rendering | `buildings/ebuildingrenderer.*`; `widgets/etilepainter.*`; `textures/ebuildingtextures.*` | Special multi-tile renderers cover palace, gatehouse, stadium, and sanctuary stairs |
| Resources/production | `engine/eresourcetype.{h,cpp}`; `buildings/eresourcebuildingbase.*`; `buildings/eresourcebuilding.*`; `buildings/eresourcecollectbuilding*`; `buildings/eprocessingbuilding.*` | `eResourceType` combines single-bit IDs and aggregate masks; processors share timed input/output logic |
| Storage/distribution/trade | `buildings/ebuildingwithresource.*`; `buildings/estoragebuilding.*`; `buildings/egranary.*`; `buildings/ewarehouse.*`; `buildings/etradepost.*`; `buildings/epier.*`; `characters/ecarttransporter.*` | Buildings publish `eCartTask`s; carts search registered compatible resource buildings |
| Characters/actions | `characters/echaracterbase.*`; `characters/echaracter.*`; `characters/echaractercreator.cpp`; `characters/actions/echaracteraction.*`; `characters/actions/echaracteractioncreator.cpp` | Character and action type factories reconstruct nested saved state |
| Pathfinding | `characters/actions/emove*`; `epatrol*`; `walkable/*`; `engine/epathfinder*`; `epathboard.*`; `ethreadpool.*` | Actions serialize walkability/path state; board changes invalidate or recompute paths |
| Gods/heroes/monsters | `characters/gods/*`; `characters/heroes/*`; `characters/monsters/*`; matching `characters/actions/*`, `audio/*`, `textures/*` | Each family has separate type-to-character mappings plus combat, UI, sound, and event registration |
| Sanctuaries | `buildings/sanctuaries/esanctuary.*`; `esanctbuilding.*`; `esanctuaryblueprint.*`; `esanctuarytextures.*`; `sanctuaries/*.txt` | Text layouts create a monument plus linked element buildings; episode/city state limits availability/counts |
| Military | `characters/esoldier.*`; `esoldierbanner.*`; `spawners/ebanner.*`; `einvasionhandler.*`; `engine/emilitaryaid.*`; `ereinforcements.*` | Banners, invasion handlers, cities, buildings, and events share military state |
| Events | `gameEvents/egameevent.{h,cpp}`; `engine/egameevents.*`; `widgets/eventwidgets/*` | `eGameEventType` selects a factory; event graphs reconnect through serialized IO IDs |
| Scenarios/adventures | `engine/ecampaign.*`; `ecampaignreadpak.cpp`; `eepisode.*`; `eepisodegoal.*`; `pak/*`; `Adventures/*` | Native `.epak` handling is separate from original PAK import |
| UI | `widgets/*`; `datawidgets/*`; `infowidgets/*`; `eventwidgets/*`; `world*` | `eGameWidget` joins input, menus, overlays, info, construction, and painting |
| Audio/text/settings | `audio/*`; `esettings.*`; `enumbers.*`; `elanguage.*`; `text/*` | Audio reads the external root; XML/repository text supplies localized strings |
| Textures/sprites | `textures/egametextures.*`; category texture classes; `espriteloader.*`; `ebinaryimageloader.*`; `esplitbinary.h`; `spriteData/*`; `offsets/*`; `textureTemplates/*` | Sprite metadata crops images from `interface.e` or an enabled size-specific `i*.e` pack |
| Save/load | `emainwindow.cpp`; `fileIO/*`; `engine/*read.cpp`; `engine/*write.cpp`; per-object `read`/`write` | Positional binary streams use temporary IO IDs to repair pointers after object creation |

## Representative traces

| Analogue | Registration and runtime path | Persistence and assets |
| --- | --- | --- |
| Fountain: basic employing/patrol building | `eBuildingType::fountain` -> `eBuildingMode::fountain` -> `egamemenu.cpp` -> preview/build switches -> `eGameBoard::build(..., 2, 2, ..., eFountain)`; `ePatrolBuildingBase::timeChanged` spawns `eWaterDistributor` with `ePatrolAction` | `eBuildingReader`/`eBuildingWriter` plus inherited patrol state; `eBuildingTextures::fFountain`/`fFountainOverlay`; `spriteData/fountain{15,30,45,60}.h`; no `eAvailableBuildings` gate |
| Olive Press: input-to-output processor | `eBuildingType`/mode/menu/preview/build/cost -> `eOlivePress`; constructor configures `eProcessingBuilding` with olives -> olive oil, 2x2, 12 workers, `eNumbers::sOlivePressProcessingPeriod`; take/give carts connect storage | `eAvailableBuildings::fOlivePress`; inherited resource state plus input cart, raw count, and process time; `fOlivePress`/`fOlivePressOverlay`; `spriteData/olivePress{15,30,45,60}.h` |
| Water Distributor: patrol walker | `eCharacterType::waterDistributor` -> `eCharacter::sCreate` in `echaractercreator.cpp` -> fountain generator -> board/tile registration -> `ePatrolAction`/`ePatrolMoveAction` -> `eBuilding::provide` | `eCharacter::write/read` persists tile, service, orientation, timing, and nested actions; fountain reference reconnects by IO ID; `eCharacterTextures::fWaterDistributor` via `eGameTextures::loadWaterDistributor` |

## Serialization map

**Persistent-schema rule: serialized enum ordinals and positional fields are file-format schema. Before changing an ID, enum order, map cardinality/order, or serialized field, inspect both writer and reader, every factory, and `eFileFormat` version handling. Never assume a compiling change preserves saves.**

| Layer | Entry points | Compatibility constraint |
| --- | --- | --- |
| Save file | `eMainWindow::saveGame/loadGame` | Header `"eZeus.ez"`, `eFileFormat::version`, widget settings, then campaign |
| Adventure | `eCampaign::save/load` | Header `"eZeus.epak"`; positional campaign graph plus adjacent text/numbers |
| Campaign/world | `eCampaign::write/read`; `eWorldBoard::write/read`; `eWorldCity::write/read` | Ordered boards, episodes, dates, price maps, cities, progress, references |
| Board/city | `eGameBoard::write/read`; `eBoardCity::write/read`; `eBoardPlayer::write/read` | Ordered vectors/maps and object type tags; temporary IO IDs assigned before write |
| Buildings | `eBuildingWriter::sWrite`; `eBuildingReader::sRead`; virtual `write/read` chain | Type precedes payload; reader/writer factory switches and base/derived field order must match |
| Characters/actions | `eCharacter::write/read`; `eCharacter::sCreate`; `eCharacterAction::sCreate` | Character, action, walkability, and nested function type IDs select factories |
| Resources/availability | Building payloads; `eAvailableBuildings::write/read`; campaign/board prices | Resource bits and fixed/positional availability fields persist directly; price maps omit resource keys |
| References | `eWriteStream::writeBuilding/writeCharacter/...`; matching `eReadStream` methods; `handlePostFuncs` | Numeric IO IDs resolve only after all objects exist |

- `eWriteStream` writes generic enums as signed 32-bit integers. Inserting or reordering serialized enum values reinterprets old data.
- `eResourceType` uses explicit bits through `1 << 23`; allocate a unique bit and update aggregate masks deliberately.
- `eFileFormat` currently defines `initial`, `settlerEmigrant`, and `cartTarget`. Only selected readers branch on `formatVersion()`; there is no tagged-field migration layer.
- Adding, deleting, or reordering positional fields without a versioned reader shifts all following bytes.
- Campaign and board price maps serialize values without resource keys; changing cardinality/order requires migration analysis.

## Asset boundaries

| Class | Paths/data | Active behavior |
| --- | --- | --- |
| Original proprietary data | External `DATA/`, `Audio/`, `Model/`, optional `Adventures/`, `zeus.ico`; `eGameDir::path` | Required by runtime systems; bootstrap never copies it |
| eZeus release assets | `interface.e`; `i15.e`/`i30.e`/`i45.e`/`i60.e`; `Zeus_Text.xml`; `Zeus_MM.xml` | `eBinaryImageLoader` resolves packed blobs through `esplitbinary.h` |
| Repository data | `spriteData/*`; `offsets/*`; `textureTemplates/*`; `sanctuaries/*.txt`; `text/*`; `fonts/*`; `Adventures/*` | Compiled or staged metadata/content |
| Repository-owned custom art | `assets/textures/` staged to runtime `Textures/`; file loading through `eGameDir::texturesDir()` | Laurel Garden loads one owned PNG and scales it for enabled tile sizes; missing art falls back safely |

Texture chain: concrete type -> category texture field -> `eGameTextures::loadX` -> category loader -> four size-specific `spriteData` arrays -> `eSpriteLoader` -> `eBinaryDataMap`. Interface icons use `eInterfaceTextures` separately.

## Verification priorities and bottlenecks

1. Golden numeric IDs for serialized content/action/event enums and complete mode/factory mappings.
2. `eResourceTypeHelpers` masks, extraction, names, icons, prices, and transport sizes.
3. `eAvailableBuildings` round trip, including Olive Press and sanctuary-derived state.
4. Board round trip with Fountain/Water Distributor/patrol action and Olive Press partial production/cart references.
5. Factory coverage for valid building, character, action, and event IDs, excluding documented legacy-null events.

Current automation is `scripts/dev-runtime.tests.ps1`; CMake defines no CTest target. Highest-risk boundaries are distributed content registration, sparse save migrations, resource single-bit/aggregate-mask mixing, fixed god ranges/pairwise data, inactive custom-art loading, and explicit source lists in both `CMakeLists.txt` and `eZeus.pro`.
