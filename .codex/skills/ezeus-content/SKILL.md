---
name: ezeus-content
description: Implement or modify eZeus gameplay/content involving buildings (including aesthetic, decorative, or production buildings), resources and production chains, walkers and characters, gods, heroes, monsters, sanctuaries, scenarios or adventures, and game events; do not use for generic builds, documentation, or unrelated maintenance.
---

# eZeus Content

Extend gameplay/content with the smallest complete change while preserving the fork's architecture, save compatibility, and external-asset boundary.

## Workflow

1. Read only the relevant section of `docs/CONTENT_EXTENSION_MAP.md`.
2. Locate the closest existing analogous implementation. Read that analogue, not many siblings.
3. Before editing, briefly state:
   - the selected analogue;
   - expected files and symbols to touch;
   - known save/serialization impact;
   - asset requirement.
4. Trace only the lifecycle stages needed by the request:
   `registration/type -> creation/spawn -> simulation/action -> rendering/UI -> serialization`.
   Skip irrelevant stages.
5. Identify every affected surface that applies:
   - enums and numeric IDs;
   - factories and switch statements;
   - registries and maps;
   - UI or menu exposure;
   - texture or sprite mappings;
   - reader and writer serialization.
6. Implement the smallest coherent change using existing patterns.
7. Add or update tests where practical and valuable.
8. Use `docs/BUILD.md` for validation commands; do not duplicate a build workflow here.
9. Finish with relevant build/tests, `git diff --check`, and inspection of `git status --short` plus the complete relevant diff for unrelated changes.

## Save safety

Before changing serialized enums, IDs, or fields:

- Read the **Serialization map** section of `docs/ARCHITECTURE.md`.
- Preserve existing numeric values and order unless the format is intentionally versioned.
- Inspect both reader and writer paths and keep their ordering symmetric.
- Ensure every new serializable type has complete discriminator, factory, read, write, and reconstruction handling where applicable.

## Assets

- Distinguish proprietary Zeus/Poseidon assets from repository-owned custom art.
- Use the existing asset mechanism unless the task specifically concerns custom-asset infrastructure.
- Never copy proprietary assets into Git or modify the user's original game installation.

## Scope and context discipline

- Use targeted symbol and path searches and reuse the navigation documents.
- Do not rescan or reread the entire repository.
- Do not inspect `build/_deps`.
- Do not inspect the external original-game installation unless the task explicitly depends on original data.
- Expand investigation only when the documentation or selected analogue is insufficient.
- Do not refactor or reformat unrelated code, mass-rename symbols, update dependencies without necessity, create speculative abstractions, or invent a generalized content/mod framework.

## Completion report

Unless the user asks otherwise, report only:

1. changed files;
2. verification performed;
3. remaining limitation or blocker.
