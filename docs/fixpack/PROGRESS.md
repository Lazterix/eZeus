# Core Fixpack integration

## Scope and safety

Baseline: `55f5beb76381bbf5ed2e5eae536aeeab1aff0d8d`.
Worktree: `C:/Users/Laz/.codex/worktrees/adc0/eZeus-Extended`.
Branch: `codex/core-fixpack-integration`.
Original main and all other worktrees are outside the modification scope.
Pre-existing untracked `.serena/` is left untouched.
No pushes, merges, dependency downloads, runtime/asset/save modifications.

## Plan and gates

- [x] Verify baseline, isolated worktree, branch and archive integrity.
- [x] Inventory 89 base core patches and continuations 1–154; exclude base monument feature extra, Enhanced and Large City.
- [x] Review serialization, overlapping lifecycles, async callbacks, accounting and texture blending before integration.
- [x] Integrate base tiers A–C in manifest order, reconciling each patch against actual source.
- [x] Integrate continuations 1–25 in numbered order, using reviewed 8 and 18 only (patches 1–154 reconciled; 33 uses largest-remainder troop allocation).
- [x] Review complete combined source and add focused production-code regressions where practical.
- [x] Build Release and Debug offline with existing dependencies; run automated tests and available analysis.
- [x] Review final diff and commit logical validated batches; deliver limitations and manual gameplay checklist.

`ledger.json` records every archive entry, original identifier, hash, affected files and disposition. Release-validated entries are compiled source evidence only; patch application is not evidence of gameplay correctness.
Extracted review material lives in ignored `build/fixpack-audit/`; archive installers are never executed.

## Preflight evidence

All 42 supplied archive SHA256 values match. All 26 core ZIP CRC checks pass.
The base manifest has 36 tier-A, 43 tier-B, 10 tier-C core patches and one feature extra.
Canonical order: base manifest tiers A/B/C, numbered continuations 1–154.
The supplied prior audit reports static analysis only, with no compilation or gameplay proof.

## Decisions

- Preserve the existing save format. Review persistence patches for safe compatible portions; do not silently introduce version bumps or positional fields.
- Reuse installed dependency sources read-only where possible, with local build outputs and FetchContent disconnected. Never run the documented build-directory deletion example.
- User explicitly authorized autonomous integration; implement after audit without an additional plan-approval round.

## Validation status

Release source build passed after the interrupted-edit repair using `cmake --build build --config Release --target eZeus -- /m:1 /p:BuildInParallel=false /p:TrackFileAccess=false`.
The build emitted existing C4715 warnings in unrelated pre-existing fallback functions; no compile errors occurred. No running-game verification yet.

After continuation 6, the same serial Release build passed again and produced `build/Release/eZeus.exe`. This checkpoint includes the movement/pathfinding guards in patches 34–40; the build still emits the known unrelated C4715 warnings in fallback functions and dependency warnings, with no errors. No standalone compiler is installed for the archive's contract source, so its assertions remain source-reviewed rather than executed.

After reviewed continuation 8, the serial Release build passed again and produced `build/Release/eZeus.exe`. Patches 41–57 cover trade accounting, thread-pool completion/shutdown, storage aggregation, cart targeting, and collector lifetimes. The build still emits only the known unrelated C4715 warnings and dependency warnings; no runtime gameplay verification has been performed.

Continuation batches 9–17 are now source-integrated through patch 109. They cover processing/farm limits, fishery and urchin lifecycles, cattle/grower/hunter/shepherd callback lifetimes, cart invalidation, and save-load normalization that does not add serialized fields. Patches 105, 109, and the fishery/urchin state fixes were reconciled to preserve positive legacy stock and retry incomplete deliveries. The next checkpoint is a Release compile of this combined batch.

That Release compile passed and produced `build/Release/eZeus.exe`. It also includes the corrected continuation 21 patrol safeguards through patch 131: `epathfinder.h` was retained, asynchronous route generations are invalidated, endpoint/tile bounds are checked, and failed routes cannot spawn ghost patrolers.

Continuations 18–25, patches 110–154, are now reconciled in the ledger. Save-state normalizations clamp only invalid negative values and do not add fields, reorder values, or bump the file format. The patrol, firefighter, collector, artisan, hippodrome, ranch/enclosure, sanctuary, and monument fixes were combined with their earlier lifecycle safeguards. Patch 127 retains exactly one required `engine/epathfinder.h` include.

The final Release project build passed after restoring the concrete `engine/egameboard.h` include required by chariot production accounting. `build/Release/eZeus.exe` and the linker tracking records were updated at 2026-10-09 13:59 UTC. The former project C4715 missing-return paths now have defined fallbacks or explicit unsupported-save errors; the Release project compilation emitted no C4715 diagnostics. Third-party dependency C4996 warnings remain outside this source tree.

Next action: run the required full Debug build, then the runtime-bootstrap fixture tests, final source/diff review, update all ledger outcomes from pending validation, and create the local integration commit. No safe game runtime or non-private save fixture has been prepared.

Debug validation was attempted twice. It is currently blocked before eZeus project compilation by generated SDL2 Debug dependency failures: MSVC C1902, `Program database manager mismatch`, reported for 196 SDL2 translation units, followed by MSB4181 in the SDL2_mixer project. This is a build-intermediate/PDB failure, not a source diagnostic. No generated build directories or PDB files were deleted because automatic cleanup is prohibited. The precise captured log is ignored at `build/fixpack-debug-sdl2-mixer.log`. The next action requiring a decision is to remove or regenerate only the affected generated Debug intermediates and rerun the Debug build; otherwise continue with fixture tests and final review while reporting Debug as blocked.

`scripts/dev-runtime.tests.ps1` was corrected and passed. Its prior cross-drive fixture used two paths on `C:` and therefore asserted the cross-volume stage location after the script correctly chose the same-volume location. The fixture now creates its repository fixture on a writable distinct volume while retaining all staged-content and invalid-stage-root assertions. It uses synthetic temporary files only and does not inspect game assets or saves.

## Final validation checkpoint

Release compilation passed for the complete integrated source. It produced `build/Release/eZeus.exe` after the chariot-factory include correction; no project C4715 diagnostics remained. The runtime-fixture test passed on same-volume and real cross-volume staging layouts.

Debug compilation is blocked by generated SDL2 dependency C1902 errors before eZeus can link. One verified ignored SDL2 Debug intermediate directory and one verified ignored eZeus Debug PDB were regenerated or removed; no Release output, source, Git metadata, save, asset, or untracked user data was deleted. C1902 persisted for SDL2 even with no active compiler/PDB server. MSBuild 17.14 and the selected v143 compiler, PDB core, and PDB server are all 14.44.35228; an isolated `/Zi /FS /MP` compiler probe succeeds. This is recorded as a local SDL2 Debug dependency/toolchain interaction, not a source defect. No additional cleanup or installation was attempted.

The ledger has no pending or unresolved entries: 201 integrated fixes and 39 corrected fixes are Release-validated, 3 patches are rejected for save-schema safety, and 1 is excluded from scope. Save compatibility remains source-reviewed: no field ordering, enum ordinal, or file-version changes were accepted. `git diff --check` reports no whitespace errors apart from CRLF conversion notices.

## Isolated Release QA checkpoint

The Release executable is staged at `X:/Projects/eZeus-Fixpack-QA/eZeus/Bin/eZeus.exe` with a matching SHA256 and required DLLs. Only private QA copies of `DATA`, `Audio`, and `Model` were copied from the read-only installation; no installed files or saves were modified. The staged process remained alive for 12 seconds with an exact path match and produced no startup output errors. GUI save/load interaction was unavailable because the Windows automation adapter exposed no targetable native window. A legacy `.ez` fixture was copied into QA but its load/save/reload round-trip remains unverified. Full details and the manual checklist are in `docs/fixpack/RELEASE-QA.md`.
