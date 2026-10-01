# Windows build and runtime cheat sheet

Verified MSVC/CMake Release workflow. Keep original Zeus/Poseidon data outside Git.

## Build

Prerequisites: Visual Studio 2022 C++ tools, CMake, and network access for the first dependency fetch.

```powershell
if (Test-Path -LiteralPath build) {
    Remove-Item -LiteralPath build -Recurse -Force
}
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
```

Expected executable: `build/Release/eZeus.exe`.

`CMakeLists.txt` explicitly lists sources and metadata. Add new compiled files there; keep `eZeus.pro` aligned when retaining the Qt/qmake path.

## Runtime inputs

| Source | Required contents | Bootstrap behavior |
| --- | --- | --- |
| Repository | `build/Release/eZeus.exe`, `Adventures/`, `fonts/`, `sanctuaries/`, `text/` | Staged under `<stage-root>/eZeus/` |
| `<runtime-assets-dir>` | `interface.e`, `Zeus_Text.xml`, `Zeus_MM.xml`, at least one `i15.e`/`i30.e`/`i45.e`/`i60.e`; optional `Bin/*.dll` | Copied into the stage |
| `<original-game-dir>` | `DATA/`, `Audio/`, `Model/`; optional `Adventures/`, `zeus.ico` | Never copied; referenced by `zeus_path.txt` |

Missing `DATA`, `Audio`, or `Model` fails preparation. Missing original `Adventures` or `zeus.ico` warns.

## Bootstrap parameters

Parameters override environment variables:

| Parameter | Environment variable | Value |
| --- | --- | --- |
| `-OriginalGameDir` | `EZEUS_ORIGINAL_GAME_DIR` | External Zeus + Poseidon root |
| `-RuntimeAssetsDir` | `EZEUS_RUNTIME_ASSETS_DIR` | Extracted eZeus release assets |
| `-StageRoot` | `EZEUS_STAGE_ROOT` | Parent of generated `eZeus/` tree |

Default stage root:

- Repository and original-game root on the same drive: `<repository>/.dev-runtime`
- Different drive: `<original-game-dir>/.ezeus-dev-runtime`

`StageRoot` must share a drive with `OriginalGameDir` and cannot equal it. `eGameDir::initialize` resolves the relative path in `eZeus/Bin/zeus_path.txt`.

Generated layout:

```text
<stage-root>/eZeus/
├── Bin/eZeus.exe
├── Bin/*.dll
├── Adventures/
├── Fonts/
├── Sanctuaries/
├── Text/
├── interface.e
├── i15.e / i30.e / i45.e / i60.e
├── Zeus_Text.xml
├── Zeus_MM.xml
└── zeus_path.txt
```

Preparation replaces staged repository resource directories and removes stale `i*.e` packs. It does not stage original `DATA`, `Audio`, `Model`, `Adventures`, or `Textures`.

## Prepare and run

```powershell
$env:EZEUS_ORIGINAL_GAME_DIR = '<original-game-dir>'
$env:EZEUS_RUNTIME_ASSETS_DIR = '<runtime-assets-dir>'

& .\scripts\dev-runtime.ps1 -Prepare
& .\scripts\dev-runtime.ps1 -Run
```

Pass `-StageRoot '<stage-root>'` or set `EZEUS_STAGE_ROOT` only when overriding the default. `-Run` prepares, validates, launches the staged executable, and returns its exit code.

## Validate bootstrap

```powershell
& .\scripts\dev-runtime.tests.ps1
```

Expected final output: `dev-runtime fixture tests passed.` The test uses synthetic temporary directories and does not inspect an original installation.

## Validate repository state

```powershell
git diff --check
git status --short
git diff --stat
```

`build/` and repository-local `.dev-runtime/` are ignored. Investigate any listed source, external asset, generated pack, executable, or DLL. Never add proprietary Zeus/Poseidon data.
