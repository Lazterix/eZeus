# Core Fixpack Release QA

Date: 2026-10-09

Worktree: `C:/Users/Laz/.codex/worktrees/adc0/eZeus-Extended`

Branch: `codex/core-fixpack-integration`

Source-under-test commit: `a4f4825a7fa6264ba8af20eb850f9c780731303a`

## Isolated environment

The independently staged QA environment is:

`X:/Projects/eZeus-Fixpack-QA/eZeus`

The executable is `X:/Projects/eZeus-Fixpack-QA/eZeus/Bin/eZeus.exe`.
Its SHA256 is `024907B3C05C63927815A341A18B62DED31B885533FF252888F5C3C0561164D6`, matching `build/Release/eZeus.exe`.
The staged runtime contains `SDL2.dll`, `SDL2_image.dll`, `SDL2_mixer.dll`, and `SDL2_ttf.dll`, plus the required interface, text, and texture-pack files. `zeus_path.txt` points to the private QA copy at `..\..\OriginalGame\`.

Only `DATA`, `Audio`, and `Model` were copied from the read-only installation into `X:/Projects/eZeus-Fixpack-QA/OriginalGame`. Repository-owned runtime resources were staged by `scripts/dev-runtime.ps1`. No installed files, source assets, Git files, or saves were modified.

## Runtime smoke test

The staged executable was launched from its QA `Bin` directory. PID 7480 remained alive for 12 seconds with an exact executable-path match, and its empty stdout/stderr logs contained no startup error. It was then stopped by that exact PID. A second controlled launch (PID 3068) also initialized and was stopped after the Windows window-discovery attempt.

This verifies process startup and dependency resolution. It does not verify menu interaction, rendering correctness, simulation, or gameplay behavior. The available Windows automation adapter returned no targetable native app/window and did not expose its documented app-launch method, so file-menu interaction was unavailable in this session.

## Save compatibility

The legacy fixture `D:/Steam/steamapps/common/Zeus + Poseidon/eZeus-0.8.2-beta/Save/Laz/autosave replay.ez` was copied without modification to `X:/Projects/eZeus-Fixpack-QA/eZeus/Save/Laz/autosave replay.ez`. The copy has SHA256 `8D954A1827BA75BA60A53FF59A9DD68BB818D20D7020F1D3C62F106804649FD0`.

Loading, saving, and reloading this fixture remain unverified because the executable exposes the save operation through its GUI and has no command-line load mode. The original fixture was not opened or changed.

## Automated evidence

- Release project build: passed before QA staging.
- `scripts/dev-runtime.tests.ps1`: passed, including same-volume and real cross-volume staging assertions.
- `git diff --check`: clean at the committed integration checkpoint.
- Debug build: blocked before eZeus linking by generated SDL2 dependency C1902 (`Program database manager mismatch`).
- CTest target: none is defined; no additional automated gameplay suite is available.

## Manual gameplay checklist

Run from the staged QA executable and record each result:

- Start a new game and verify initialization, menus, resolution, and texture selection.
- Load the copied legacy save, save it under a new QA filename, restart, and reload it.
- Exercise residential buildings, immigration, food production, storage, and warehouse logistics.
- Exercise trade routes, trading posts, manufacturing, ships, combat, and resource accounting.
- Create fires and verify firefighter dispatch, return, and propagation behavior.
- Create and edit patrol routes, destroy/rebuild patrol buildings, and verify stale asynchronous routes do not spawn walkers.
- Build ranches, enclosures, horses, hippodromes, and chariot production; demolish and rebuild them.
- Construct sanctuaries and monuments, including custom monument textures with alpha transparency.
- Repeat representative actions in a large city while watching for leaks, stalls, crashes, and significant frame-time regressions.

## Readiness

The Release build is safely staged and independently starts from the QA directory. It is ready for isolated manual gameplay testing. Legacy save round-trips and gameplay fidelity remain unverified until that manual pass is performed. The Debug build remains a separate local SDL2/MSVC toolchain blocker; the least invasive next diagnostic is to reproduce the SDL2 Debug compile in a fresh, independently configured build directory using the same installed v143 toolchain, without changing the system installation or deleting additional artifacts.
