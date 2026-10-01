# Content extension map

Hand-written registration surfaces for extending existing content. Use the closest analogue and inspect all of its registrations before editing.

## Persistent-schema rules

**Serialized enum ordinals and positional fields are persistent file-format schema. Before changing IDs, ordering, map cardinality/order, or fields, inspect the reader, writer, factories, and `eFileFormat` branches. A compiling change can still corrupt old saves.**

- `eWriteStream` stores enums as signed 32-bit integers. Use stable explicit/append-only IDs; never insert or reorder persisted values without migration.
- Keep base/derived `write` and `read` calls symmetric and identically ordered. Version every payload change that must read old files.
- `eResourceType` values are explicit bits. Allocate a unique bit and update aggregate masks deliberately.
- Register new compiled/metadata files in `CMakeLists.txt`; keep `eZeus.pro` aligned when retaining qmake.

## Registration failure map

| Missed registration | Typical result |
| --- | --- |
| Type missing from reader/factory | Saved or scenario-created object cannot be reconstructed; null use may crash |
| Build mode/menu mapping omitted | Content exists but is invisible or unselectable |
| Preview/build switch mismatch | Preview differs from placement, or selection does nothing |
| Availability field/switch omitted | Scenario gating or persistence fails |
| Cost/name/info registration omitted | Incorrect cost/text/info behavior |
| Texture field/load chain omitted | Null texture, invisible object, or draw failure |
| Resource helper/mask omitted | Resource cannot complete naming, UI, transport, storage, or trade paths |
| Enum reordered or field added without migration | Old files reinterpret IDs or shift all following bytes |

## 1. New building using existing mechanics

**Closest analogues:** `eFountain` for an employing building with a standard walker; `eBench`/`eAestheticsBuilding` for a passive decorative building.

**Mandatory:**

- Stable `eBuildingType` in `buildings/ebuilding.h`; concrete class using the nearest base; `buildings/allbuildings.h`; build file lists.
- `fileIO/ebuildingreader.cpp` construction and `fileIO/ebuildingwriter.cpp` type/discriminator handling before virtual state.
- `widgets/ebuildingmode.{h,cpp}` mode/`toBuildingType`; matching `egamewidgetpaint.cpp` preview and `egamewidgetbuild.cpp` constructor/footprint.
- `widgets/egamemenu.cpp` button/icon/label/cost/mode; `engine/edifficulty.cpp` cost.
- Applicable `buildings/ebuilding.cpp` classifications, `sNameForBuilding`, and `sInfoText`.
- Category texture field/loader, `eGameTextures::loadX`, sprite/custom mapping, and interface icon.

**Conditional:** `buildings/eavailablebuildings.{h,cpp}` full read/write/query/start-episode path; `engine/ai/eaidistrict.cpp`, `eaibuilding.*`, `eaicityplan.*`; `engine/ecampaignreadpak.cpp`; specialized UI, overlays, rotations, child tiles, audio, or editor controls.

**Validation checklist:** enum ID is stable; reader/writer agree; UI mode maps to type; preview equals placed footprint; cost/name/info valid; texture non-null at all enabled sizes; availability persists; old save still loads; new save round-trips.

## 2. New production building

**Closest analogue:** `eOlivePress` over `eProcessingBuilding` for one input -> timed output. Use `eResourceBuilding`/`eResourceCollectBuilding` for direct extraction and inspect `eArmory`, `eChariotFactory`, or `eCorral` only when their special behavior matches.

**Mandatory:**

- All generic building points from section 1.
- Configure input/output, capacity/use, workers, footprint, and timing in the concrete constructor; use `enumbers.*` for externally tuned periods.
- `buildings/eprocessingbuilding.*`: confirm its take cart, `cartTasks`, effectiveness scaling, raw capacity, production accounting, and serialization fit the design.
- `engine/eresourcetype.*`: confirm both resources have transport sizes and are included in the intended aggregate masks.
- `buildings/estoragebuilding.*`, `egranary.*`, `ewarehouse.*`, `etradepost.*`, `epier.*`: verify the output can be accepted/distributed/traded as intended.
- `buildings/ebuilding.cpp::sInfoText`: input-empty, shutdown, and workforce states.
- `eAvailableBuildings` when scenarios gate the building.

**Save compatibility:** `eProcessingBuilding` already saves output cart/resource through `eResourceBuildingBase`, then input cart, raw count, and process time. Extra fields need symmetric, versioned serialization.

**Validation checklist:** disabled/zero-worker building does not produce; input cart respects range; exact input is consumed; output count/statistics increment; output cart reaches accepted storage; full/no storage is safe; save during partial process/input/cart travel round-trips.

## 3. New resource

**Closest analogues:** `olives` for a raw input, `oliveOil` for a processed good, `sculpture` for non-four-unit transport/storage, and `orichalc` for a later appended resource.

**Mandatory:**

- `engine/eresourcetype.h`: allocate a unique explicit bit; update `food`, `allBasic`, `warehouse`, `tradePost`, or `allTransportable` only when membership is intended.
- `engine/eresourcetype.cpp`: `extractResourceTypes`, `typeName`, `typeLongName`, `icon`, `transportSize`, `defaultPrice`, and assumptions in `isSingleType`.
- Localized text sources used by the chosen name mapping and an interface icon field/load path.
- Producers/consumers and concrete storage/trade acceptance masks.
- `characters/ecarttransporter.cpp`: cart overlay/frame mapping if the resource should be visibly carried.
- Search all switches and maps over `eResourceType`, especially `engine/egameboard.cpp`, `engine/ecampaign.*`, `engine/eboardcity*`, storage/trade/info widgets, event values, goals, gifts/requests, and economy calculations.

**Conditional:** production chain, vendor/house consumption, world-city supply/demand, scenario editor selectors, event messages/goals, and audio.

**Save compatibility:** resource bits and aggregate masks are serialized in inventories, carts, prices, orders, exports, events, goals, and campaign data. Never reuse a bit. Map iteration/order and fixed assumptions in existing saves must be checked; a new campaign or board price entry can change positional serialization because `eCampaign::write/read` and `eGameBoard::write/read` currently iterate in-memory price maps without writing resource keys.

**Validation checklist:** unique single bit; extraction returns it once; aggregate membership correct; name/icon/price/transport size valid; producer-to-storage-to-trade path works; every selector can display it; old campaign/save price streams remain aligned or are migrated.

## 4. New production chain

**Closest analogue:** olives -> `eOlivePress` -> olive oil -> warehouse/trade/vendor. For worker-collected raw goods, inspect vine/olive tree + `eGrowersLodge`/`eGrowerAction`.

**Mandatory:**

- Complete sections 2 and 3 for every new building/resource.
- Define where raw input originates, which `eBuildingWithResource` owns it, which cart task moves it, who consumes it, and what happens when every capacity is full.
- Verify board supported-resource masks, storage acceptance, trade pricing/supply/demand, shutdown rules, production statistics, scenario availability, UI categories, and goals/events.
- Serialize every intermediate inventory, active worker/cart/action, and fractional production timer.

**Conditional:** farms/terrain fertility, resource gatherer actions/path policies, house/vendor consumption, sanctuary effects, imports-only chains, or military consumption.

**Validation checklist:** no resource creation/loss outside defined conversion; blocked endpoints recover; carts do not deadlock with mixed cargo; rates scale with employment; monthly/yearly accounting is correct; save at each stage round-trips.

## 5. New walker/character

**Closest analogue:** `eWaterDistributor` for the standard patrol stack; `eCartTransporter` for resource delivery; choose a specialist only if it needs a custom action.

**Mandatory:**

- `characters/echaracterbase.h`: stable `eCharacterType`.
- Concrete class and `characters/allcharacters.h`; build file lists for new files.
- `characters/echaractercreator.cpp` (`eCharacter::sCreate`) so saved characters load.
- Spawn owner: building, event, spawner, invasion, or scenario path; constructors register automatically with `eGameBoard`.
- Action selection. Reuse a serialized action when possible; otherwise add stable `eCharActionType`, factory case in `echaracteractioncreator.cpp`, and symmetric action serialization. New nested function/walkability/god-action types have their own enums/factories and the same ID rule.
- Texture fields/loaders in `textures/echaractertextures.*` and `textures/egametextures.*`, size-specific sprite metadata, and rendering behavior (`eBasicPatroler` or specialist override).
- `widgets/infowidgets/echaracterinfowidget.cpp` and sound dispatch when the character is user-visible.

**Conditional:** combat classification in `eCharacterBase`/`eCharacter`, board specialist registries, soldier/banner integration, resource cart overlays, god/hero/monster mappings.

**Save compatibility:** board writes character type then virtual payload; `eCharacter` serializes tile, service state, orientation, timing, current/paused action types and payloads. Owner references use IO IDs and post-load callbacks. Append IDs; version any new payload.

**Validation checklist:** spawn and destruction register/unregister once; path failure and return are safe; action factory covers every persisted action; texture exists for all action/orientation states; save during movement/interaction restores owner, tile, action, and target.

## 6. New god

**Closest analogue:** choose the existing god with the nearest powers and sanctuary behavior; no existing god is a small isolated extension.

**Mandatory:**

- `characters/gods/egodtype.h`, matching `eCharacterType`, concrete god class, `eGod::sCharacterToGodType`, `sGodToCharacterType`, `sCreateGod`, name/string list, timing/target helpers, texture loader, and the pairwise `sFightWinner` data in `characters/gods/egod.cpp`.
- God actions/help behavior in `characters/actions/egod*` and `characters/actions/godHelp/*`; add serialized action/factory types only if reuse is impossible.
- `textures/egodtextures.*`, `textures/egametextures.*`, sprite metadata; `audio/egodsounds.*`; messages/text.
- `eAvailableBuildings`: sanctuary state, serialized fixed-range loops, monuments, and god-keyed maps.
- God temple/build modes/menu/preview/build/name/cost/reader/writer cases; `buildings/sanctuaries/*` and a repository `sanctuaries/<god>.txt` layout if the god has a sanctuary.
- God selection/info widgets, events, quests, campaign friendly/enemy lists, and original PAK conversion if compatibility is required.

**Save compatibility:** `eGodType` is ordinal and several loops assume the inclusive range `aphrodite..zeus` or `0..zeus`. Appending after `zeus` does not automatically include the god; changing the range changes serialized fixed-count data. This needs an explicit format migration, not only an enum append.

**Validation checklist:** all god/god and god/content interactions are defined; selection lists and fixed loops include the new type; sanctuary availability persists; attack/help/event paths work; all sounds/textures/text exist; old saves retain exact god IDs and fixed-array alignment.

## 7. New hero

**Closest analogue:** use the hero whose combat style and hall requirements are nearest; `Atalanta` is the existing ranged special case, so prefer a non-ranged hero for a simpler first addition.

**Mandatory:**

- `characters/heroes/ehero.h` (`eHeroType`) plus matching `eCharacterType`.
- Concrete class and every mapping/factory/name/list helper in `characters/heroes/ehero.cpp`.
- `buildings/eheroshall.*`: hall type mappings, requirements, summoning, serialization; add `eBuildingType`/`eBuildingMode` and all building registration points for a new hall.
- `characters/actions/eheroaction.*`, combat/missile timing where applicable, textures, `audio/eherosounds.*`, messages and info/selection widgets.
- Campaign goals/events/quests and board-city `mSummonedHeroes` handling.

**Save compatibility:** both hero and character ordinal IDs are persisted; the hall is also a serialized building. All three mappings must remain stable.

**Validation checklist:** hall enable/build/summon cycle; requirement/status UI; factory and character mapping round trips; combat/monster pairing; hero/hall save before and after summon.

## 8. New monster

**Closest analogue:** choose by movement/combat environment; use a land melee monster for the smallest extension, not `Scylla`/`Kraken` water cases or a ranged special case.

**Mandatory:**

- `characters/monsters/emonstertype.h` plus matching `eCharacterType`.
- Concrete class and mappings/factory/name/list/combat helpers in `characters/monsters/emonster.*`; `eCharacter::sCreate` coverage follows from the character factory.
- `characters/actions/emonsteraction.*`, texture/sprite chain, `audio/emonstersounds.*`, messages and character info.
- Monster event value/selection widgets, `eMonsterUnleashedEvent`, `eMonsterInvasionEvent*`, board-city monster registry/maps, hero/god interactions, and scenario goal/import conversion.

**Save compatibility:** monster type, character type, event types/payloads, killed/spawned lists, and board-city monster-event maps are serialized.

**Validation checklist:** event and direct spawn; path policy; target/combat/death; hero pairing if any; selection/info/audio/art; save during event, movement, combat, and after death.

## 9. New sanctuary

**Closest analogue:** copy the existing god sanctuary size/layout family that matches the goal; `eSanctuary` and repository `sanctuaries/*.txt` are the shared mechanism.

**Mandatory:**

- God/building IDs and build modes for the sanctuary; `eBuilding::sSanctuaryBuilding` range/classification assumptions.
- Layout parsing and lookup in `buildings/sanctuaries/esanctuary.*`; repository layout data in `sanctuaries/*.txt`.
- `engine/egameboard.cpp::buildSanctuary` element construction, dimensions/rotation, cost/material handling, and links among monument/element buildings.
- `buildings/eavailablebuildings.*`, episode `fMaxSanctuaries`, city registration, menu/selection/info, reader/writer, names/costs.
- Sanctuary/god textures, statues/overlays, and `eGameTextures` loaders.

**Conditional:** unique blessing/curse, warriors, resource gifts, special monument pieces, or new element types. New `eSanctEleType` values are serialized/layout-sensitive and require their own factory/build handling.

**Validation checklist:** parse every orientation/size; placement owns every tile exactly once; construction material/progress; completion registers god and availability effects; erase cleans child links; save incomplete and complete sanctuary.

## 10. New scenario/adventure

**Closest analogue:** repository-owned `Adventures/The Founding of Athens` or `Adventures/The Sands of Betrayal`; use `eCampaign`/`eEpisode` rather than changing the original PAK importer for native content.

**Mandatory:**

- `engine/ecampaign.*`: native `.epak`, adjacent text, optional `numbers.txt`, episode sequencing, parent/colony boards, world data, save/load.
- `engine/eepisode.*`: events by city, goals, available buildings, friendly gods, sanctuary limits, dates/funds.
- `engine/eepisodegoal.*`: use only supported goal types and payload meanings.
- Editor flow in `widgets/echoosegameeditmenu.cpp`, episode/city/world/event/goal widgets; verify native save/export produces the expected adventure directory.
- Repository `Adventures/<name>/<name>.epak` and `<name>.txt`; add only legally owned optional narration/numbers assets.

**Conditional:** `engine/ecampaignreadpak.cpp` and `pak/*` only for importing original-game scenario data or adding compatibility with an original binary record. Do not inspect or commit proprietary installations/files.

**Save compatibility:** native adventures and player saves share many serializers and enum IDs. A scenario containing a newly added type cannot load in an older executable; a new executable must preserve all old IDs/read layouts.

**Validation checklist:** glossary/listing; text keys; parent/colony transitions; goals/events/availability; world trade/cities; save/load in each episode; no external absolute path embedded.

## 11. New game event

**Closest analogue:** choose the smallest concrete event with the same payload/target pattern; simple value changes (`eWageChangeEvent`, supply/demand/price changes) are safer analogues than invasions or god/monster graphs.

**Mandatory:**

- `gameEvents/egameevent.h`: stable `eGameEventType`.
- Concrete event deriving from `eGameEvent` or the closest event-value base; symmetric `read`/`write` for subtype fields.
- `eGameEvent::sCreate` in `gameEvents/egameevent.cpp`; a missing case makes episode/save reconstruction return null.
- Event editor exposure in `widgets/eeventselectionwidget.cpp` and the matching `widgets/eventwidgets/*` configuration widget.
- Long name, trigger behavior, warnings/consequences, resource loading, message/UI behavior, and CMake/qmake file lists.
- Episode/city event serialization paths in `engine/eepisode.cpp`, `engine/egameevents.*`, and `eGameEvent::write/read` remain compatible.

**Conditional:** new trigger/value/resource/god/monster helper types only when existing event bases cannot model the behavior; original PAK parser mapping only for compatible original records.

**Save compatibility:** `eGameEventType` and branch are serialized before factory construction; event graphs reconnect by IO ID. Append a stable ID and version any payload change. The legacy warning enum values intentionally return null and must remain accounted for in completeness tests.

**Validation checklist:** editor creates correct subtype; trigger date/repeat/warnings; consequence/parent links; action affects intended city/board once; long name/message; copy via `makeCopy`; episode and save round trip.

## Recommended development sequence

| Slice | Scope | Proof/stop condition |
| --- | --- | --- |
| 1. Registration | Minimal 1x1 `eAestheticsBuilding` using the existing texture mechanism or a temporary existing asset mapping. No new mechanics, resource, or asset loader. | Stable type registration, menu, placement, rendering, board ownership, availability, save/load, and build/test workflow all pass |
| 2. Custom art | Smallest clean repository-owned custom-art loading/staging path, used by a custom aesthetic building | All enabled texture sizes load without proprietary additions; registration behavior from slice 1 remains unchanged |
| 3. Economy | One new resource plus one single-stage production chain | Resource IDs, masks, storage, distribution, trade, UI, production accounting, and serialization boundaries are covered |
