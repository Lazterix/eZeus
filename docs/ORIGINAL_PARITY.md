# Original Zeus + Poseidon Parity Audit

- Baseline: main @ `ad40147d`
- Audit type: static/read-only
- Confidence: MEDIUM
- Original runtime comparison is still required for UNKNOWN items.
- Original proprietary assets remain external and are not stored in the repository.

## Executive Summary

The audit finds a substantial but incomplete reimplementation. Core city simulation, world-state handling, campaign import, gods/heroes/monsters, Poseidon resources, and much of save/load exist. The largest parity gaps are concentrated in the original gameplay shell and several persistence/control paths:

- Original File / Options / Help top menus are absent.
- The original categorized message system and browsable history are absent.
- Notifications are transient and event icons can remain indefinitely.
- Sound controls, scrolling-speed controls, Auto-Defend, balloon help, and warning preferences are absent.
- Autosave timing and controls do not match the original.
- Military command buttons and naval panels are substantially unwired.
- Several gameplay timers and subclass fields are not persisted correctly.
- High-resolution support is valuable but insufficiently validated and can fail silently.
- Three potentially serious stability/corruption paths were identified statically.
- The Orichalc/Black Marble zero-size gift divide-by-zero is fixed on main and is not an open P0.

This was a static, read-only audit. No executable comparison, build, save fixture, sanitizer run, or original-game runtime session was performed.

Evidence labels:

- **ORIGINAL**: Original manuals, original installation files, text databases, or asset inventory.
- **CODE**: Current `ad40147d` source.
- **PACK**: External eZeus runtime-pack metadata.
- **DOC**: Repository documentation.
- **INFERENCE**: Strong static inference requiring runtime reproduction.

Original manuals consulted externally:

- `D:\Steam\steamapps\common\Zeus + Poseidon\Zeus - Manual.pdf`
- `D:\Steam\steamapps\common\Zeus + Poseidon\Zeus - Poseidon - Manual.pdf`
- `D:\Steam\steamapps\common\Zeus + Poseidon\Zeus - Adventure_Editor - Manual.pdf`
- `D:\Steam\steamapps\common\Zeus + Poseidon\Zeus - Poseidon - Adventure_Editor - Manual.pdf`

## Estimated Original Parity

**Approximately 60–70% overall.**

Approximate subsystem estimates:

| Area | Estimated parity | Basis |
|---|---:|---|
| Core city simulation | 70–85% | Most systems are real implementations, but housing timers, walkers, distribution, and save omissions materially affect behavior. |
| Gods/heroes/monsters | 70–85% structural | Complete type/factory coverage; exact powers, balance, targeting, timing, audiovisual behavior remain unverified. |
| World/diplomacy | 70–80% structural | Broad event and relationship implementation; exact original formulas and edge cases are unknown. |
| Campaigns | 60–75% | Zeus/Poseidon PAK importer and progression exist; stock campaigns need systematic runtime validation. |
| Military/naval | 45–60% | Simulation paths exist, but command UI, Auto-Defend, naval presentation, and persistence are incomplete. |
| Original gameplay UI | 30–45% | Core panels exist; top menus and multiple original workflows are absent. |
| Messages/options/help | 20–35% | Major original systems are missing or architecturally incomplete. |
| Poseidon | 65–80% structural | Broad content presence; behavior, campaigns, editor controls, and several persistence paths need validation. |
| High-resolution modernization | 50–70% | Exposed and partially functional, but display validation, scaling, pack availability, reload, and layout are incomplete. |

This is a feature-surface estimate, not a percentage of source code.

## Restore

Major original functionality requiring restoration:

- Original gameplay File / Options / Help top-menu structure.
- New Adventure, Replay Current Episode, and Delete actions in the gameplay File menu.
- Full original message categories, full/condensed policy, history, dates, locations, and go-to behavior.
- Correct event-icon acknowledgement, expiration, and save/load behavior.
- Sound controls for music, narration, effects, ambient and mute.
- Original six-month autosave schedule and enable/disable setting.
- Simulation-speed UI and independent scrolling-speed setting.
- Auto-Defend and functional military command controls.
- In-game Help, balloon-help levels, and warning preferences.
- Housing water decay and periodic elite-house reevaluation.
- Complete walker/resource/peddler/trireme persistence.
- Poseidon campaign nationality control in the editor.
- Stock campaign and episode parity validation.

## Preserve

Useful modernization that should remain:

- Widescreen and resolutions above the original 800×600/1024×768 limits.
- 1080p, 1440p, ultrawide, and 4K ambitions.
- Multiple texture-size packs and scalable interface assets.
- Windowed/fullscreen selection.
- External original-game asset dependency rather than committing proprietary assets.
- SDL-based modern OS/runtime compatibility.
- Current save of pause, speed, camera position, view direction, tile size, and bookmarks.
- Direct import of original Zeus and Poseidon PAK adventures.
- Current modern editor and runtime improvements where they do not conflict with original behavior.
- Existing working custom content.

## Fix-Modernization

- Validate and safely apply high-resolution display modes.
- Replace height-only UI scaling with DPI/logical-scale-aware layout.
- Repair texture-pack persistence and same-resolution pack reload.
- Fix renderer/widget/static-texture shutdown ordering.
- Stop invalid-renderer text texture retry/log spam.
- Reject unsupported newer save versions instead of parsing them as the current positional schema.
- Persist newer runtime state such as in-flight cargo, trireme-away state, and chariot production progress.
- Correct the fourth god-voice path.
- Allow silent startup when an audio device is unavailable.
- Harden corrupt-save discriminator and pointer resolution.
- Correct gift-cap validation.

## Extend

Existing additions to retain but deprioritize:

| Addition | Class | Status |
|---|---|---|
| Salt resource | EXTEND | Existing |
| Salt Works | EXTEND | Existing |
| Laurel Garden | EXTEND | Existing |
| Further custom resources/buildings/mechanics | EXTEND | Deferred |

## P0

No open P0 was reproduced at runtime. Three source-confirmed paths meet the threshold for immediate P0-risk investigation.

| Finding | Class | Status | Assessment |
|---|---|---|---|
| Racing-horse negative texture index | FIX-MODERNIZATION | BROKEN | Signed modulo can produce a negative `aid`, followed by unchecked `race[aid]`. This is undefined behavior and a possible deterministic crash for affected angles. See `characters/eracinghorse.cpp:43`. |
| Newer save version continues loading | FIX-MODERNIZATION | BROKEN | Loader warns that the file is newer, then continues parsing it as the current positional schema. Shifted counts, enums, and references can cause corruption or crashes. See `emainwindow.cpp:281`. |
| Renderer/static-texture shutdown lifetime | FIX-MODERNIZATION | BROKEN | Window and renderer are destroyed before all widgets/static textures are guaranteed gone. Later texture destruction or painting can use an invalid renderer. See `emainwindow.cpp:37`. |
| Orichalc/Black Marble zero-size gift division | FIX-MODERNIZATION | PARITY—FIXED | Fixed by positive gift sizes, compile-time assertions, and a runtime `giftCount <= 0` guard. Historical P0 only; not open. |

## P1

| Finding | Class | Status |
|---|---|---|
| Common-house water never decays because its timer is never incremented | RESTORE | BROKEN |
| Elite housing does not periodically reevaluate evolution | RESTORE | PARTIAL |
| Gathered resources already removed from terrain are lost if saved before deposit | RESTORE | BROKEN |
| Original top menus are missing | RESTORE | MISSING |
| Original categorized/condensed message system and history are missing | RESTORE | MISSING |
| Messages and alert icons are not persisted | RESTORE | MISSING |
| Alert icons are not acknowledged or removed when clicked | RESTORE | BROKEN |
| Autosave is unconditional and annual rather than optional in January and July | RESTORE | BROKEN |
| Sound controls and persistent volumes are absent | RESTORE | MISSING |
| Auto-Defend is absent | RESTORE | MISSING |
| Military formation/tactics/special-command buttons are unwired | RESTORE | BROKEN |
| Trireme-away state is not saved | RESTORE | BROKEN |
| Some selectable high-resolution/fullscreen modes can fail silently | FIX-MODERNIZATION | BROKEN |
| Missing audio device aborts startup rather than selecting silent mode | FIX-MODERNIZATION | BROKEN |
| Fourth god-voice variant uses an incorrect relative path | FIX-MODERNIZATION | BROKEN |

## P2

| Finding | Class | Status |
|---|---|---|
| Active Agora peddlers lose their Agora owner after load | RESTORE | PARTIAL |
| Common-house culture service decays twice as fast as intended | RESTORE | BROKEN |
| Chariot partial-production time is not saved | FIX-MODERNIZATION | BROKEN |
| House-level clamp permits an index equal to the array size | FIX-MODERNIZATION | BROKEN |
| Gift capacity validation ignores the incoming gift count | FIX-MODERNIZATION | PARTIAL |
| Help content exists externally but no functional help browser is wired | RESTORE | MISSING |
| Scrolling speed is fixed and has no setting | RESTORE | MISSING |
| Difficulty is only available during episode introduction | RESTORE | PARTIAL |
| Navy and mercenary force panels are largely empty shells | RESTORE | PARTIAL |
| Editor Greek/Atlantean campaign-nationality control is commented out | RESTORE | MISSING |
| Prices are serialized positionally without resource keys | FIX-MODERNIZATION | PARTIAL |
| Save reference/factory reads trust IDs and runtime types | FIX-MODERNIZATION | PARTIAL |
| Message condensed-title loader writes the wrong field | RESTORE | BROKEN |
| Message second-action visibility is applied to the first button | RESTORE | BROKEN |
| Resolution-to-interface-scale mapping depends only on height | FIX-MODERNIZATION | PARTIAL |
| Texture-pack changes do not reload at unchanged resolution | FIX-MODERNIZATION | BROKEN |

## P3

| Finding | Class | Status |
|---|---|---|
| Elite-house horse overlays are disabled despite implemented sprite selection | RESTORE | MISSING |
| Original cursor treatment was not found | RESTORE | UNKNOWN |
| Tooltip coverage exists but is inconsistent and lacks balloon-help policy | RESTORE | PARTIAL |
| Several hover, highlight, keyboard-navigation, and frame-timing differences require visual comparison | RESTORE | UNKNOWN |
| Additional installed music tracks are not mapped; whether they belong in normal rotation is unknown | RESTORE | UNKNOWN |

## Core Gameplay

| Area | Class | Status | Assessment |
|---|---|---|---|
| Housing evolution | RESTORE | PARTIAL | Requirements, consumption and level changes exist; timer defects distort evolution. |
| Common housing | RESTORE | BROKEN | Water never expires; culture duration is effectively halved. |
| Elite housing | RESTORE | PARTIAL | Goods and requirements exist; periodic reevaluation and horse overlays are incomplete. |
| Desirability | RESTORE | PARTIAL | Appeal maps and house sampling exist; exact original radii/values need comparison. |
| Water | RESTORE | BROKEN | Distribution exists, but supplied common housing can retain water indefinitely. |
| Food | RESTORE | PARTIAL | Consumption/storage paths exist; exact balance and house timing remain unverified. |
| Fleece/oil/wine | RESTORE | PARTIAL | Household inventories and distribution exist; runtime parity unverified. |
| Workforce | RESTORE | PARTIAL | Weighted sector allocation is implemented. |
| Unemployment | RESTORE | PARTIAL | Workforce accounting exists; original thresholds/messages require comparison. |
| Taxes | RESTORE | PARTIAL | Tax coverage and collection persist. |
| Administration | RESTORE | PARTIAL | Administrative service paths exist; exact original effects remain unverified. |
| Maintenance | RESTORE | PARTIAL | Maintenance decay and risk effects are implemented. |
| Fire | RESTORE | PARTIAL | Fire risk, state, firefighters and persistence exist. |
| Collapse | RESTORE | PARTIAL | Collapse risk is implemented; exact probabilities unknown. |
| Disease/hygiene | RESTORE | PARTIAL | Plague and hygiene systems exist; water defect affects parity. |
| Culture | RESTORE | BROKEN | Services exist, but common-house culture decays twice. |
| Education | RESTORE | PARTIAL | Source, venue and patrol delivery exist. |
| Entertainment | RESTORE | PARTIAL | Venue and walker systems exist; detailed original coverage needs runtime tests. |
| Athletics | RESTORE | PARTIAL | Implemented through service/venue systems; timing parity unknown. |
| Philosophy | RESTORE | PARTIAL | Implemented through service/venue systems; timing parity unknown. |
| Agriculture | RESTORE | PARTIAL | Farm ripening and persistence exist. |
| Ranching | RESTORE | PARTIAL | Corral cattle and processing state persist. |
| Hunting/fishing | RESTORE | BROKEN | Gathering works, but collected cargo is lost across save/load. |
| Resource production | RESTORE | PARTIAL | Broad producer registration exists; mid-cycle collector persistence is incomplete. |
| Processing industries | RESTORE | PARTIAL | Input, output, carts and timers generally persist; chariot timer does not. |
| Storage/warehouses/granaries | RESTORE | PARTIAL | Orders, slots, capacities and carts serialize; exact original logistics need runtime validation. |
| Distribution | RESTORE | PARTIAL | Carts and walkers operate; active peddlers lose owner linkage after load. |
| Agora/vendors | RESTORE | PARTIAL | Spawn and distribution paths exist; loaded active peddlers can become ineffective. |
| Walkers | RESTORE | PARTIAL | Generic character/action persistence is broad, but subclass fields are omitted. |
| Transport | RESTORE | PARTIAL | Cart cargo and followers persist; deployed/naval edge cases remain. |
| Roads/roadblocks | RESTORE | PARTIAL | Terrain, road appearance and roadblock bits persist. |
| Bridges/crosswalks | RESTORE | PARTIAL | Implemented as terrain/placement behavior rather than ordinary persisted buildings. |
| Beautification | RESTORE | PARTIAL | Appeal contributions exist; detailed visual/radius parity unknown. |
| Monuments | RESTORE | PARTIAL | Construction, resources, stages and persistence exist; visual/staging parity requires runtime comparison. |

## Gods/Heroes/Monsters

All original registered entity types were found. No original Zeus/Poseidon entity was identified as wholly absent.

| Group | Entities confirmed present | Status |
|---|---|---|
| Gods | Aphrodite, Apollo, Ares, Artemis, Athena, Atlas, Demeter, Dionysus, Hades, Hephaestus, Hera, Hermes, Poseidon, Zeus | PARTIAL |
| Heroes | Achilles, Atalanta, Bellerophon, Hercules, Jason, Odysseus, Perseus, Theseus | PARTIAL |
| Monsters | Calydonian Boar, Cerberus, Chimera, Cyclops, Dragon, Echidna, Harpies, Hector, Hydra, Kraken, Maenads, Medusa, Minotaur, Scylla, Sphinx, Talos, Satyr | PARTIAL |

| Capability | Class | Status | Assessment |
|---|---|---|---|
| Presence/factories | RESTORE | PARITY | All listed entities map to concrete factories/classes. |
| Sanctuaries | RESTORE | PARTIAL | Every god has a sanctuary factory and monument texture path. |
| Summons/prerequisites | RESTORE | PARTIAL | Hero halls and god spawning have implemented requirements; exact values need comparison. |
| Blessings/curses/attacks | RESTORE | PARTIAL | Concrete actions exist for every god; power values, cooldowns and target priority are unverified. |
| God interactions | RESTORE | PARTIAL | Pairwise combat and attitude state exist. |
| Hero quests | RESTORE | PARTIAL | Quest registration, dispatch, fulfillment and hall state are real, serialized flows. |
| Monster behavior | RESTORE | PARTIAL | Land/water subtypes, god sender and hero slayer mappings exist; special behavior parity is unknown. |
| Animations | RESTORE | PARTIAL | Attack/bless/curse/disappear and entity sprite paths exist; frame timing requires runtime comparison. |
| Sounds | RESTORE | PARTIAL | Sound objects exist for all groups; one fourth-variant god path is broken. |
| Messages | RESTORE | BROKEN | Entity events exist, but the overarching message/notification system lacks original behavior. |
| Disappearance/death | RESTORE | PARTIAL | Actions exist; cleanup timing and edge cases require runtime tests. |
| Save/load | RESTORE | PARTIAL | Major action and attitude state is serialized; complete mid-action round-trip coverage is unproven. |
| Sanctuary spawn retry | FIX-MODERNIZATION | PARTIAL | Failed no-road spawn can leave `mGod` set and prevent retry. |

## Military

| Area | Class | Status | Assessment |
|---|---|---|---|
| Army recruitment | RESTORE | PARTIAL | Housing, armor and horses determine force capacity. |
| Companies | RESTORE | PARTIAL | Banner selection and muster/home behavior exist. |
| Formations | RESTORE | BROKEN | Placement exists; rotation/tactics controls are unwired. |
| Deployment | RESTORE | PARTIAL | Rectangular selection and right-click formation placement exist. |
| Invasions | RESTORE | PARTIAL | Staged land/sea arrival, march, invasion and return are implemented. |
| Defense | RESTORE | PARTIAL | Fight, bribe and surrender callbacks exist. |
| Attack other cities | RESTORE | PARTIAL | Raid/conquest event paths and delayed results exist. |
| Naval systems/triremes | RESTORE | BROKEN | Construction and combat exist; force UI and away-state persistence are incomplete. |
| Poseidon military additions | RESTORE | PARTIAL | Archers, spearmen/chariots and Atlantean banners are registered. |
| Allies/rivals/colonies | RESTORE | PARTIAL | Aid, ally strikes, conquest and colony relationships exist. |
| Tribute | RESTORE | PARTIAL | Tribute scheduling/state exists; exact cadence unverified. |
| Bribery/surrender | RESTORE | PARTIAL | Both have implemented invasion callbacks. |
| Auto-Defend | RESTORE | MISSING | No working global option/control path found. |
| Combat commands | RESTORE | BROKEN | Several visible buttons have no press actions. |
| Battle resolution | RESTORE | UNKNOWN | Significant implementation exists, but original formulas were not runtime-compared. |

## World/Diplomacy

| Area | Class | Status | Assessment |
|---|---|---|---|
| World map | RESTORE | PARTIAL | City markers, nationality and map interaction exist. |
| Relations/allies/rivals/vassals | RESTORE | PARTIAL | Relationship and attitude states are implemented and serialized. |
| Colonies | RESTORE | PARTIAL | Parent/colony board and campaign relationships exist. |
| Gifts | RESTORE | PARTIAL | Sending/receiving exists; nominal three-gift cap ignores incoming count. |
| Requests/demands | RESTORE | PARTIAL | Reminders, overdue, compliance/refusal and consequences exist. |
| Tribute | RESTORE | PARTIAL | Tribute scheduling and payment events exist. |
| Trade/imports/exports | RESTORE | PARTIAL | Supply/demand, routes, water links and availability exist. |
| Trade-route creation | RESTORE | PARTIAL | Route/world state exists; original capacity and price behavior need comparison. |
| Conquest | RESTORE | PARTIAL | Ownership/vassalization and consequences are implemented. |
| Military expeditions | RESTORE | PARTIAL | Raid, conquest, reinforcement and ally-strike flows exist. |
| Delayed events | RESTORE | PARTIAL | Gifts, raids, strikes, aid and return events use scheduled delays. |
| Event responses | RESTORE | PARTIAL | Response callbacks are present; original probabilities and attitude deltas are unverified. |
| Gift crash | FIX-MODERNIZATION | PARITY—FIXED | Zero-size Orichalc/Black Marble failure is closed. |

## Campaigns

| Area | Class | Status | Assessment |
|---|---|---|---|
| Adventure selection | RESTORE | PARTIAL | Original PAK discovery/import exists; packaged runtime contains only repository adventures. |
| Zeus/Poseidon PAK parsing | RESTORE | PARTIAL | Versions 1.0 and 2.0, Atlantean campaigns, episodes, boards, events and goals are recognized. |
| Episode progression | RESTORE | PARTIAL | Parent/colony episodes and played-colony state persist. |
| Objectives | RESTORE | PARTIAL | Goal structures and serialization exist. |
| Victory/defeat | RESTORE | PARTIAL | Campaign state exists; every stock campaign path remains untested. |
| Episode introduction | RESTORE | PARTIAL | Introduction and difficulty selection exist. |
| Campaign narration | RESTORE | PARTIAL | Narration loading/playback paths exist; exact triggers need comparison. |
| Difficulty | RESTORE | PARTIAL | Implemented but not exposed through the original Options workflow. |
| Replay episode | RESTORE | PARTIAL | `autosave replay.ez` is produced; no File-menu action uses it. |
| Autosave replay snapshot | RESTORE | PARTIAL | Snapshot backend exists. |
| Campaign transitions | RESTORE | PARTIAL | Parent/colony transition state exists; alternate branch validation remains. |
| Scenario scripting/events | RESTORE | PARTIAL | Broad event types and delayed actions exist. |
| Campaign-specific mechanics | RESTORE | UNKNOWN | Requires a stock-campaign test matrix. |

## UI

| Area | Class | Status | Assessment |
|---|---|---|---|
| Top bar | RESTORE | PARTIAL | Original-style background and city statistics exist; File/Options/Help menus do not. |
| Right-side control panel | RESTORE | PARTIAL | Present with build/world/army/minimap controls. |
| Build categories | RESTORE | PARTIAL | Broadly present; complete ordering and hover parity unverified. |
| Overlays | RESTORE | PARTIAL | Overlay infrastructure exists; complete original coverage was not verified. |
| Map/world controls | RESTORE | PARTIAL | Implemented, with military and help gaps. |
| Information windows | RESTORE | PARTIAL | Extensive current UI exists; exact layout/content parity unknown. |
| Dialogs | RESTORE | PARTIAL | Functional custom dialogs exist; original message semantics are absent. |
| Tooltips | RESTORE | PARTIAL | Selective tooltips exist; no balloon-help preference. |
| Cursors | RESTORE | UNKNOWN | No complete custom original cursor system was found. |
| Mouse behavior | RESTORE | PARTIAL | Selection, placement, minimap drag and scrolling exist. |
| Hotkeys | RESTORE | PARTIAL | Pause, rotation, speed, arrows and bookmarks exist; full original matrix unverified. |
| Speed | RESTORE | PARTIAL | Keyboard-only simulation speed; scrolling speed missing. |
| Pause | RESTORE | PARITY | Pause state and hotkey exist and are saved. |
| Selection | RESTORE | PARTIAL | Building, character and military selection exist. |
| Notifications | RESTORE | BROKEN | No original history/category model; icons stick and are not persisted. |
| Date/treasury/population | RESTORE | PARITY | All update on the current top bar. |
| Minimap | RESTORE | PARTIAL | Present; exact original behavior and aspect scaling unverified. |
| Hover states | RESTORE | PARTIAL | Many four-state assets exist; systematic parity not established. |
| Keyboard navigation | RESTORE | UNKNOWN | Partial hotkeys exist; menu/dialog keyboard navigation needs runtime comparison. |

## File Menu

| Item | Backend classification | Current status | Finding |
|---|---|---|---|
| New Adventure | BACKEND EXISTS | MISSING in gameplay | Available from title menu, not original gameplay File menu. |
| Replay Current Episode | PARTIAL BACKEND | PARTIAL | Replay snapshot is written but no user action loads it. |
| Load | BACKEND EXISTS | PARTIAL | Functional `.ez` picker/backend; original menu integration absent. |
| Save | BACKEND EXISTS | PARTIAL | Functional picker/backend; original workflow incomplete. |
| Delete | MISSING BACKEND | MISSING | No delete operation or UI found. |
| Leave Greece | BACKEND EXISTS | PARITY | Exit-to-main-menu path exists. |

## Options

| Option | Class | Status | Assessment |
|---|---|---|---|
| Display | PRESERVE / FIX-MODERNIZATION | PARTIAL | Resolution, fullscreen and pack choices exist; mode validation and scaling are incomplete. |
| Sound | RESTORE | MISSING | No volume or mute settings. |
| Simulation speed | RESTORE | PARTIAL | Hotkeys and save state exist; original Options UI absent. |
| Scrolling speed | RESTORE | MISSING | Arrow movement uses a fixed increment. |
| Difficulty | RESTORE | PARTIAL | Backend exists at episode introduction only. |
| Autosave | RESTORE | BROKEN | Always enabled and annual instead of optional every six months. |
| Auto-Defend | RESTORE | MISSING | No working setting/backend found. |
| Messages | RESTORE | MISSING | No category-based full/condensed preferences. |

## Messages

The original data defines 20 message-category IDs with default presentation modes. Current eZeus does not model those preferences.

| Capability | Class | Status | Assessment |
|---|---|---|---|
| Full messages | RESTORE | PARTIAL | Current UI always chooses the full variant. |
| Condensed messages | RESTORE | BROKEN | Not used as an original-style policy; condensed title loader is defective. |
| Categories | RESTORE | MISSING | Original category table exists externally but is not represented in settings. |
| Category preferences | RESTORE | MISSING | No full/condensed/off per-category state. |
| Message history | RESTORE | MISSING | Messages button is inert; no browsable record. |
| Dates | RESTORE | PARTIAL | Current displayed messages can receive date context; no durable history. |
| Locations | RESTORE | PARTIAL | Live callbacks can target locations; no stable saved record. |
| Go-to | RESTORE | PARTIAL | Alert clicks navigate to targets but do not acknowledge alerts. |
| Event icons | RESTORE | BROKEN | Maximum four; oldest is evicted only when a fifth arrives. |
| Dismissal | RESTORE | BROKEN | Clicking an icon does not remove it. |
| Icon lifetime | RESTORE | BROKEN | No expiry/acknowledgement lifecycle. |
| Save/load persistence | RESTORE | MISSING | History, queue, open dialog and icons are not serialized. |
| Queued messages | RESTORE | PARTIAL | In-memory deque exists while a dialog is active. |
| Modal behavior | RESTORE | PARTIAL | Full messages are modal; some incomplete initializers may create problematic dialogs. |
| Non-modal behavior | RESTORE | MISSING | Original condensed non-interrupting behavior is absent. |
| Warning messages | RESTORE | PARTIAL | Individual warnings exist; original warning preference is missing. |
| Architectural cause | RESTORE | BROKEN | UI stores live event pointers/callbacks rather than a durable notification record with stable IDs, category, date, location and acknowledgement state. |

## Audio

| Area | Class | Status | Assessment |
|---|---|---|---|
| Music | RESTORE | PARTIAL | Menu, normal, battle and victory tracks are mapped through `Mix_Music`. |
| Ambient audio | RESTORE | PARTIAL | Layered ambient paths exist. |
| Walker sounds | RESTORE | PARTIAL | Broad voice/effect registration exists. |
| Building sounds | RESTORE | PARTIAL | Broad effect vectors exist. |
| Gods/heroes/monsters | RESTORE | PARTIAL | All entity groups have audio objects; fourth god variant path is broken. |
| Battles | RESTORE | PARTIAL | Battle music/effects exist; exact original transitions unverified. |
| Campaign narration | RESTORE | PARTIAL | Supported through the music path; no independent volume bus. |
| UI sounds | RESTORE | PARTIAL | Some paths exist; exhaustive original UI-sound parity is unknown. |
| Music transitions | RESTORE | PARTIAL | Fade/halt/menu/random/battle transitions exist. |
| Volume controls | RESTORE | MISSING | No `Mix_Volume`/`Mix_VolumeMusic` settings integration. |
| Mute | RESTORE | MISSING | No persistent mute control. |
| Mixer buses | RESTORE | PARTIAL | Music/narration share the music path; effects/voices use mixer channels. No original-style user bus controls. |
| Missing files | RESTORE | UNKNOWN | The original installation contains extensive Audio content; exhaustive registered-path comparison was not completed. |
| Missing code | RESTORE | CONFIRMED | Controls, persistence and one path construction are missing/broken. |
| Audio-device absence | FIX-MODERNIZATION | BROKEN | Startup aborts rather than continuing silently. |

## Save/Load

The format is positional and schema-sensitive. See `docs/ARCHITECTURE.md`, section **Serialization map**.

| State | Status | Assessment |
|---|---|---|
| Board/terrain | PARTIAL | Broad dimensions, fog and tile state persist. |
| Buildings | PARTIAL | Extensive factory/state coverage; individual omissions remain. |
| Walkers | BROKEN | Generic state persists, but collector cargo and peddler ownership do not. |
| Production | PARTIAL | Most timers/resources persist; chariot-building progress does not. |
| Resources | BROKEN | Already-collected in-flight cargo is lost. |
| Campaign | PARTIAL | Campaign, episodes, colonies and events persist. |
| Diplomacy/world | PARTIAL | Cities, relationships, trade and world state persist. |
| Pending game events | PARTIAL | Many event payloads persist; complete coverage requires fixtures. |
| Invasions | PARTIAL | Invasion handler state persists. |
| Triremes | BROKEN | Wharf `mAbroad` state is omitted. |
| Heroes/gods/monsters | PARTIAL | Major action/attitude/quest state persists; full mid-action coverage unverified. |
| Messages | MISSING | No history, category preference, current message or queue persistence. |
| Notification icons | MISSING | No icon lifecycle persistence. |
| Speed/pause | PARITY | Both persist. |
| Camera/view | PARITY | Camera delta, direction, tile size and bookmarks persist. |
| Objectives | PARTIAL | Goal/campaign state exists. |
| Versioning | BROKEN | Only three versions; newer versions are warned about but still parsed. |
| Prices | PARTIAL | Values are keyless and depend on current map order/cardinality. |
| Pointer repair | PARTIAL | Deferred ID repair exists, but invalid IDs/types are largely trusted. |
| Possible corruption paths | BROKEN | Unsupported versions, corrupt counts/types, unchecked casts and invalid house levels can escape validation. |

## Editor

| Area | Class | Status | Assessment |
|---|---|---|---|
| Map editor | RESTORE | PARTIAL | Map, bitmap, terrain and save actions exist. |
| Adventure editor | RESTORE | PARTIAL | Parent map, colonies, episodes, world, goals and events are exposed. |
| City editing | RESTORE | PARTIAL | Broad city/building placement support exists. |
| Scenario settings | RESTORE | PARTIAL | Settings surfaces exist; exact original coverage unverified. |
| Events/triggers | RESTORE | PARTIAL | Numerous event types are implemented. |
| Objectives | RESTORE | PARTIAL | Goal editing exists. |
| Starting conditions | RESTORE | PARTIAL | Campaign/city setup exists; complete original matrix unverified. |
| World cities/trade | RESTORE | PARTIAL | World-map and city economic controls exist. |
| Invasions | RESTORE | PARTIAL | Event infrastructure exists; round-trip testing needed. |
| Gods/heroes/monsters | RESTORE | PARTIAL | Relevant entities are represented. |
| Greek/Atlantean campaign type | RESTORE | MISSING | Campaign nationality selector is commented out. |
| Testing workflow | RESTORE | UNKNOWN | Editor save/load and run-game round trips were not exercised. |

## Poseidon

| Area | Class | Status | Assessment |
|---|---|---|---|
| Atlantean nationality/systems | RESTORE | PARTIAL | Atlantean PAK detection, markers, troops and buildings exist. |
| Science | RESTORE | PARTIAL | Bibliotheke, University, Inventors Workshop, Observatory, Laboratory and Museum service chains exist. |
| Black Marble | RESTORE | PARTIAL | Resource bit, extraction, storage, pricing and monument use exist. |
| Orichalc | RESTORE | PARTIAL | Resource, refinery, storage, trade and monument use exist. |
| Chariots | RESTORE | BROKEN | Factory/troops exist; partial-production time is not saved. |
| Hippodrome/racing | RESTORE / FIX-MODERNIZATION | BROKEN | System exists; negative racing-horse texture index is a serious risk. |
| Poseidon monuments | RESTORE | PARTIAL | Monument and shrine families, resource handling and persistence exist. |
| Gods/heroes/monsters | RESTORE | PARTIAL | Poseidon-specific entities are registered; detailed behavior remains unverified. |
| Campaigns | RESTORE | PARTIAL | Poseidon 2.0 PAK parsing and Atlantean detection exist. |
| Expansion UI/world | RESTORE | PARTIAL | Atlantean markers and military art exist; naval UI and editor nationality are incomplete. |
| Expansion military | RESTORE | PARTIAL | Archers/chariots/frigates-related code exists; commands and naval state need repair. |
| Gift crash | FIX-MODERNIZATION | PARITY—FIXED | Orichalc and Black Marble gift sizes are positive and guarded. |

## Original Asset Recovery

| Source pack | Sprite/index | Dimensions at 15/30/45/60 | Likely purpose | Currently mapped |
|---|---:|---|---|---|
| `interface.e`, `interfaceNewParts_0.png` | 1 | 419×15 / 838×30 / 1257×45 / 1676×60 | Gameplay top-bar background | YES |
| `interface.e`, `interfaceNewParts_0.png` | 2 | 93×384 / 186×768 / 279×1152 / 372×1536 | Right-side gameplay menu background | YES |
| `interface.e`, `interfaceNewParts_0.png` | 61–64 | 17² / 33² / 50² / 66² | Messages-button states | YES, but inert |
| `interface.e`, `interfaceNewParts_0.png` | 184–228 | Mostly 17² / 33² / 50² / 66² | Fire, disaster, disease, invasion, monster, god, hero and army-return alert icons | YES |
| `interface.e`, `interfaceNewParts_0.png` | 248–251 | 9×14 / 18×28 / 27×42 / 36×56 | Manual military-control states | YES, control unwired |
| `interface.e`, `interfaceNewParts_0.png` | 252–255 | 9×14 / 18×28 / 27×42 / 36×56 | Automatic military-control states | YES, control unwired |
| `interface.e`, `paneling_0.png` | 64–67 | 11² / 22² / 33² / 44² | Message location/exclamation control | YES |
| `interface.e`, `paneling_0.png` | 333–336 | About 11–12² / 22–24² / 33–36² / 44–48² | Help button states | YES, help missing |
| `interface.e`, `paneling_0.png` | 337–340 | 12² / 24² / 36² / 48² | OK/acknowledge control | YES |
| `i15.e` / `i30.e` / `i45.e` / `i60.e` | `interfaceSprites_0.png`, 42 sprites per scale | Scale-specific | Additional interface/game-scale pieces | Split-table mappings exist |
| `Zeus_Text.xml` | Groups 1–3 | N/A | File, Options and Help menu labels | External text present; UI not wired |
| `Zeus_MM.xml` | Help records | N/A | In-game help content | External content present; no browser wired |
| `Zeus event message categories.txt` | IDs 0–19 | N/A | Original category/presentation policy | External data present; not modeled |

Important conclusions:

- Original top-bar, message, alert, help, checkbox and military-control art already exists externally and much of it is already decoded.
- The main limitation is missing behavior and layout, not absence of all original art.
- Exact sprite indices for the original File/Options/Help dropdown frames, sliders, menu rows and hover treatments were not positively identified in this audit. They remain a targeted asset-mapping task.
- Runtime contains `interface.e`, `i15.e`, and `i30.e`.
- Runtime does **not** contain `i45.e` or `i60.e`, even though source split metadata supports them.
- No proprietary asset was copied or extracted into the repository.

## Modernization to Preserve

| Modernization | Status | Direction |
|---|---|---|
| High-resolution choices through 3840×2160 | PARTIAL | Preserve and validate. |
| Widescreen/ultrawide capability | PARTIAL | Preserve; remove aspect-ratio assumptions. |
| Scalable interface packs | PARTIAL | Preserve with guaranteed fallback. |
| Windowed/fullscreen choice | PARTIAL | Preserve; modernize transition handling. |
| External original asset directory | PARITY | Preserve licensing boundary. |
| Repository-owned custom artwork separation | PARITY | Preserve. |
| SDL-based modern runtime | PARTIAL | Preserve; fix lifecycle and silent-audio behavior. |
| Camera/speed/bookmark save state | PARITY | Preserve. |
| Direct original PAK import | PARTIAL | Preserve and regression-test. |
| Custom Salt/Salt Works/Laurel Garden | EXISTING | Preserve without prioritizing further expansion. |

## High-Resolution Findings

The current high-resolution feature is **FIX-MODERNIZATION / BROKEN-PARTIAL**, not a candidate for removal.

Confirmed failure categories:

1. **Unsupported display mode:** the UI exposes a fixed resolution list regardless of actual monitor/fullscreen support.
2. **Incomplete enumeration:** startup appends only each display's current mode rather than all supported modes.
3. **Silent SDL failure:** return values from window-size/fullscreen transitions are ignored.
4. **Exclusive fullscreen mismatch:** fullscreen uses `SDL_WINDOW_FULLSCREEN` without explicitly selecting and validating a compatible display mode.
5. **Resolution → scale mapping:** UI scale is selected only from vertical resolution.
6. **Missing packs:** runtime lacks `i45.e` and `i60.e`.
7. **Hard-coded dimensions:** top/right panels and many controls use absolute scale-multiplied coordinates.
8. **Aspect-ratio assumptions:** width changes do not independently influence interface scale or safe layout regions.
9. **Texture reload:** changing texture packs without changing resolution does not force a reload.
10. **Invalid persisted selection:** settings can write all texture-pack flags false before the in-memory fallback is applied.
11. **UI reconstruction:** resolution/fullscreen changes do not provide a clearly robust renderer/resource reconstruction boundary.
12. **Clipping/render targets:** likely at unusual ratios and scale combinations; requires runtime capture.
13. **Shutdown lifetime:** static textures/widgets can outlive their renderer.
14. **Error amplification:** failed text texture creation can retry and log repeatedly.

Recommended strategy: **C — a safer hybrid.**

- Enumerate actual display modes dynamically.
- Deduplicate and filter modes by minimum logical UI size and supported renderer limits.
- Keep a small validated safe list for windowed mode and recovery.
- Prefer borderless desktop fullscreen unless explicit exclusive-mode selection succeeds.
- Validate every SDL transition and revert on failure.
- Decouple logical UI scale from physical resolution.
- Select the closest available asset pack independently of layout scale.
- Guarantee `i15`/`i30` fallback if larger packs are absent.
- Rebuild renderer-dependent textures and layout at an explicit transition boundary.
- Add screenshot/layout validation for 800×600, 1024×768, 1080p, 1440p, ultrawide and 4K.

Pure option A would preserve unsupported assumptions. Pure option B would expose arbitrary modes without ensuring the fixed-layout UI can render them safely.

## Unknown / Requires Original-Game Runtime Comparison

- Exact housing requirement values, service lifetimes and evolution timing.
- Original fire, collapse, disease and crime probability curves.
- Per-god blessing/curse magnitude, cooldown and targeting priority.
- Hero prerequisite values, quest timing and disappearance behavior.
- Monster targeting, special attacks, movement restrictions and cleanup.
- Formation, tactics and naval battle resolution.
- Auto-Defend semantics for land and sea invasions.
- Diplomacy attitude deltas, response probabilities and travel delays.
- Trade-route capacity and original price behavior.
- Every stock Zeus and Poseidon campaign path.
- Colony transitions, alternate episodes and defeat/replay behavior.
- Campaign narration trigger and interruption behavior.
- Editor save/load/run round trips.
- Exact original dropdown/slider sprite indices.
- Complete original audio-path manifest.
- Cursor art and cursor-state parity.
- Complete hotkey and keyboard-navigation parity.
- Visual alignment at ultrawide and 4K.
- Runtime reproduction of the racing-horse and shutdown-lifetime failures.
- Old-save compatibility across every historical eZeus save format.

## Top 30 Restoration Priorities

1. Reproduce and eliminate the racing-horse negative-index path.
2. Correct widget/texture/renderer/window shutdown ownership.
3. Stop invalid-renderer text retries and log spam.
4. Reject unsupported newer save versions safely.
5. Restore common-house water decay.
6. Persist in-flight collected resources.
7. Persist active peddler ownership.
8. Persist trireme-away state.
9. Persist chariot partial-production time.
10. Restore periodic elite-house evolution checks.
11. Build the original File / Options / Help top-menu shell.
12. Restore the categorized message-record model.
13. Add message-history browsing, dates, locations and go-to.
14. Implement alert acknowledgement, expiry and dismissal.
15. Persist durable messages and alerts through save/load.
16. Restore File-menu New Adventure, Replay Episode and Delete.
17. Restore sound controls and persistent mixer volumes.
18. Restore six-month autosave and its toggle.
19. Restore simulation-speed and scrolling-speed Options UI.
20. Restore Auto-Defend.
21. Wire military formations, tactics, rotation and special commands.
22. Complete naval/mercenary force panels.
23. Restore Help, balloon help and warning preferences.
24. Implement validated high-resolution/fullscreen transitions.
25. Correct texture-pack persistence and reload.
26. Fix fourth-variant god audio lookup.
27. Restore Greek/Atlantean editor campaign selection.
28. Validate all stock Zeus campaigns.
29. Validate all stock Poseidon campaigns and expansion mechanics.
30. Complete original cursor, hover, overlay and minor presentation parity.

## First 10 Implementation Tasks

1. Add a focused racing-horse angle/index regression test and normalize the index.
2. Establish explicit renderer-owned texture teardown; destroy widgets/textures before renderer and window.
3. Make text texture failures non-retrying after renderer invalidation and add a shutdown smoke test.
4. Reject save versions newer than the supported format before parsing any positional payload.
5. Add round-trip fixtures for collector cargo, peddler ownership, trireme-away state and chariot progress, then repair those omissions compatibly.
6. Fix common-house water/culture timers and elite-house periodic evolution with deterministic simulation tests.
7. Implement a resolution transition boundary: validated modes, rollback, DPI/logical scaling and pack fallback.
8. Restore the original top-menu shell using external interface art and existing localized text groups.
9. Complete File-menu actions, including safe Delete confirmation and Replay Current Episode.
10. Introduce a stable message-record model—category, presentation mode, date, location reference, acknowledgement and persistence—before wiring history and alert UI.

## Custom Extensions Should Resume When

Further custom content should resume only when:

- No open P0 stability or corruption paths remain.
- Core P1 housing and save omissions have regression coverage.
- File / Options / Help workflows are restored.
- Message history, categories, alerts and save/load persistence are functional.
- Sound, autosave, speed and Auto-Defend controls are restored.
- Military commands and naval persistence are functional.
- Zeus and Poseidon stock campaigns pass a representative smoke matrix.
- Save compatibility has versioned fixtures and explicit unsupported-version handling.
- 800×600, 1024×768, 1080p, 1440p, ultrawide and 4K have validated layout/display behavior.
- Original parity gaps are tracked in a maintained document with reproducible acceptance checks.

## Proposed `docs/ORIGINAL_PARITY.md` Structure

1. Purpose and evidence policy
2. Original-game sources and licensing boundary
3. Classification/status/severity definitions
4. Verified baseline and supported runtime assets
5. Executive parity scorecard
6. Core city gameplay matrix
7. Gods/heroes/monsters matrix
8. Military and naval matrix
9. World/diplomacy matrix
10. Campaign and episode matrix
11. Complete UI matrix
12. File menu
13. Options
14. Messages and notifications
15. Audio
16. Save/load and compatibility
17. Editors
18. Poseidon expansion
19. Original asset recovery map
20. Modernization preservation requirements
21. High-resolution compatibility matrix
22. Known defects and fixed regressions
23. Runtime-comparison backlog
24. Ordered restoration roadmap
25. Acceptance gates for renewed extension work
