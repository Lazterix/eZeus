<h1 align="center">eZeus</h1>

eZeus is an open-source implementation of Zeus: Master of Olympus game

[Watch trailers](https://www.youtube.com/@eZeus-e)

Please note that eZeus is not an exact copy of the original game. There are some minor differences, most of which are accidental, as well as optional additional features (such as a new walker implementation and the ability to build two cities at once). eZeus supports the original game’s adventures, and the overall experience should be similar to the original.

## How To Play

To play eZeus you will need original game files (Zeus: Master of Olympus base game and Poseidon expansion).

Currently only English and Polish versions are supported due to font glyphs limitations.

Only windows binaries are provided. If you want to play on Linux you have to build eZeus yourself.

[You can download eZeus from GitHub](https://github.com/MaurycyLiebner/eZeus/releases)

## How To Build on macOS

You need these dependencies for x86 and ARM Builds: 
```
softwareupdate --install-rosetta
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
arch -x86_64 /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
brew install sdl2 sdl2_mixer sdl2_image sdl2_ttf git qt make dylibbundler fluid-synth
arch -x86_64 /usr/local/bin/brew install sdl2 sdl2_mixer sdl2_image sdl2_ttf git qt make dylibbundler fluid-synth
```
Clone the eZeus repository:
```
git clone --recurse-submodules https://github.com/MaurycyLiebner/eZeus
```
Than simply execute compile_apple.sh script. All is done automatic. At the End you should have 2 new folders. build_arm and build_x86. Follow the instructions at the end of the script.

Or you could DL ready to go Binaries straight forward here:
https://github.com/MacThings/eZeus/releases/tag/Zeus-Binaries

## How To Build on Linux

Make sure you have **git** installed on your system.

Clone the eZeus repository:
```
git clone --recurse-submodules https://github.com/MaurycyLiebner/eZeus
```

You will need to install following packages:

```
sudo apt install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev libsdl2-mixer-dev libnoise-dev g++
```

You will need Qt Creator (qt.io).

Open eZeus.pro in Qt Creator and use it to build eZeus.

Before running eZeus binary make sure to setup all files as specified in **How To Play** section and the release description. Put eZeus binary in (...)/Zeus and Poseidon/eZeus/Bin directory and run it.

## Windows development runtime

Build the Release configuration, then set the original game and extracted official eZeus runtime asset directories:

```powershell
$env:EZEUS_ORIGINAL_GAME_DIR = 'path\to\Zeus and Poseidon'
$env:EZEUS_RUNTIME_ASSETS_DIR = 'path\to\extracted eZeus release'
./scripts/dev-runtime.ps1 -Prepare
./scripts/dev-runtime.ps1 -Run
```

`-OriginalGameDir`, `-RuntimeAssetsDir`, and optional `-StageRoot` override `EZEUS_ORIGINAL_GAME_DIR`, `EZEUS_RUNTIME_ASSETS_DIR`, and `EZEUS_STAGE_ROOT`. Without a StageRoot override, the script uses the ignored repository `.dev-runtime/` when the repository and original game share a drive; otherwise it uses `<original game>/.ezeus-dev-runtime/`. StageRoot must be on the original game's drive because the current runtime interprets `zeus_path.txt` relative to `eZeus/Bin/`.

The original `DATA/` directory is required during startup. `Audio/` and `Model/` are required for full gameplay. Original `Adventures/` enables original campaigns, while `zeus.ico` is cosmetic; their absence produces warnings only. `Textures/` is not staged because current texture loading uses the `interface.e` and enabled `i*.e` binary packs. The runtime asset directory must contain `interface.e`, `Zeus_Text.xml`, `Zeus_MM.xml`, and at least one of `i15.e`, `i30.e`, `i45.e`, or `i60.e`.

## Authors

**Maurycy Liebner** - 2021 - 2025 - [MaurycyLiebner](https://github.com/MaurycyLiebner)
