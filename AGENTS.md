# AGENTS.md

## Purpose and scope

This fork extends eZeus while preserving its existing architecture and compatibility with external Zeus: Master of Olympus + Poseidon runtime data.
Likely work includes buildings, decorative content, resources, production chains, walkers, gods, heroes, monsters, sanctuaries, scenarios, events, and engine improvements.
Original proprietary Zeus/Poseidon assets are external dependencies and must never be committed.

These are the authoritative navigation documents:

- `docs/ARCHITECTURE.md` — subsystem boundaries, runtime relationships, serialization, and assets.
- `docs/CONTENT_EXTENSION_MAP.md` — registration surfaces and validation by content type.
- `docs/BUILD.md` — supported build, development runtime, and validation workflow.

Use this file to choose what to read, not as a substitute for those documents.
Do not broadly rescan the repository or reread unrelated documentation.

## Mandatory gameplay/content workflow

1. Read only the relevant section of `docs/CONTENT_EXTENSION_MAP.md`.
2. Locate the closest existing analogous implementation.
3. Inspect only the concrete files and symbols required by that analogue.
4. Identify every affected ID, enum, registry, factory, and build-file entry.
5. Check serialization and save impact before changing persistent types or fields.
6. Make the smallest coherent implementation consistent with existing patterns.
7. Build and test the affected behavior.
8. Inspect `git status` and the diff for unrelated changes.

Expand investigation only when the documentation or closest analogue is insufficient.
Do not begin by rereading the entire repository.

## Context discipline

- Prefer targeted symbol and path searches.
- Reuse the navigation documents instead of rediscovering documented architecture.
- Do not repeatedly reread large files or dump source files into responses.
- Do not explain basic C++ or CMake concepts unless asked.
- Avoid speculative redesign and investigation unrelated to the requested behavior.
- Do not load `build/`, `build/_deps/`, or external original-game data unless the task directly requires it.
- Keep implementation reports concise.

## Save compatibility — critical

Serialized enum ordinals and positional fields are part of the persistent save schema.
Read `docs/ARCHITECTURE.md` section **Serialization map** before any save-sensitive change.

- Never reorder existing serialized enum values casually.
- Never insert into a persistent enum without understanding ordinal impact; preserve existing numeric IDs where possible.
- Before adding, deleting, or reordering serialized fields, inspect both writer and reader plus `eFileFormat` version handling.
- Keep base and derived read/write order symmetric.
- Every serializable content type needs complete read, write, discriminator, and factory handling where applicable.
- Treat resource bits, fixed ranges, map cardinality/order, and temporary IO-reference repair as schema-sensitive.
- A successful compile does not prove old saves remain compatible.

## Asset and licensing boundary

- Original Zeus/Poseidon `DATA`, `Audio`, `Model`, and other proprietary assets remain external.
- Never copy proprietary assets into Git or modify the user's original installation during normal development.
- The runtime bootstrap uses external `OriginalGameDir` and `RuntimeAssetsDir`; follow `docs/BUILD.md`.
- Keep repository-owned custom artwork clearly separated from proprietary assets.
- Do not invent a new asset framework unless the task specifically requires one.

## Scope and upstream discipline

- Keep diffs narrow and preserve upstream mergeability.
- Do not perform unrelated refactors, drive-by formatting, mass renames, or dependency upgrades.
- Do not introduce speculative abstractions or mechanics beyond the requested slice.
- Prefer one coherent feature or fix per branch where practical.
- Do not modify generated files or external runtime data as a shortcut.

## Task routing

- Building/content: relevant building section in `docs/CONTENT_EXTENSION_MAP.md`, then the closest analogue.
- Resource/production: resource and production sections, then the nearest complete chain.
- Walker/character: walker section, then an analogous character and action.
- God/hero/monster/sanctuary: the corresponding section, then the closest existing type.
- Scenario/event: the corresponding section and the smallest matching payload/target analogue.
- Save-sensitive: `docs/ARCHITECTURE.md` **Serialization map** first.
- Build/runtime: `docs/BUILD.md` first.

Do not read unrelated sections unless the selected path exposes a concrete dependency.

## Build and validation

Use the commands and runtime setup in `docs/BUILD.md`; do not improvise a second workflow.

- Build Release.
- Run relevant automated tests where present; currently, runtime bootstrap coverage is in `scripts/dev-runtime.tests.ps1`.
- Use the development runtime when behavior needs in-game verification.
- Run `git diff --check`.
- Inspect `git status --short` and the complete relevant diff before completion.
- Confirm no proprietary or unrelated generated files entered the worktree.

## Recommended initial development sequence

1. Minimal 1x1 aesthetic building using the existing texture mechanism.
2. Small repository-owned custom-art loading path plus a custom aesthetic building.
3. New resource plus a single-stage production chain.

This sequence is guidance, not an immutable roadmap. Each slice must meet its proof/stop condition in `docs/CONTENT_EXTENSION_MAP.md` before expanding scope.

## Completion report

Unless asked otherwise, report only:

1. Changed files.
2. Verification performed.
3. Remaining limitation or blocker, if any.

Do not produce a long implementation essay.
