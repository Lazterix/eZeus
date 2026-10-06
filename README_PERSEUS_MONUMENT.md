# Perseus Custom Monument v1

This package adds Perseus as a NEW always-available monument in eZeus.
It does not replace Poseidon or any packed original asset.

## Design

- Custom monument ID: 0
- Building mode: `eBuildingMode::perseusMonument`
- Building type: `eBuildingType::customMonument`
- Texture family: `perseusMonument`
- Footprint: 2x2
- Four directional sprites: N / W / S / E
- Q/E preview rotation: uses the project’s existing working world-direction rotation
- No `eGodMonumentTile` surround
- Terrain is rendered below the custom monument, preventing the black diamond/rim seen during the Poseidon integration
- Menu: Aesthetics / Decorations -> Monuments -> Perseus Monument
- Always available; no Ctrl+F9 required
- Packed originals remain untouched

## Install

Extract this ZIP directly into:

`X:\Projects\eZeus-Extended`

Allow it to merge folders. It adds:

- `assets/overrides/.../perseusMonument_*.png`
- `spriteData/perseusMonument15.h`
- `spriteData/perseusMonument30.h`
- `spriteData/perseusMonument45.h`
- `spriteData/perseusMonument60.h`
- `scripts/add-perseus-custom-monument.py`

Then run:

```powershell
cd X:\Projects\eZeus-Extended
python .\scripts\add-perseus-custom-monument.py
```

Build:

```powershell
cmake --build .\build --config Release --parallel
```

Copy the executable:

```powershell
Copy-Item `
  ".\build\Release\eZeus.exe" `
  "D:\Steam\steamapps\common\Zeus + Poseidon\.ezeus-dev-runtime\eZeus\Bin\eZeus.exe" `
  -Force
```

Launch:

```powershell
& "D:\Steam\steamapps\common\Zeus + Poseidon\.ezeus-dev-runtime\eZeus\Bin\eZeus.exe"
```

## In-game validation

1. Open Aesthetics / Decorations.
2. Open Monuments.
3. Select Perseus Monument.
4. Check preview has no extra 4x4 helper-tile surround.
5. Use Q/E before placement and verify all four authored views appear.
6. Place it and verify:
   - no black diamond/rim,
   - no extra pavement helper tiles,
   - removal works,
   - save/reload preserves Perseus,
   - Poseidon still works unchanged.

## Atlas geometry

The new Perseus atlas intentionally uses the proven Poseidon monument technical slot geometry.

15:
- `perseusMonument_0.png` = 236x118

30:
- `perseusMonument_0.png` = 472x236

45:
- `perseusMonument_0.png` = 708x354

60:
- frame 2 -> `perseusMonument_0.png` = 236x400
- frame 1 -> `perseusMonument_1.png` = 236x464
- frame 3 -> `perseusMonument_2.png` = 236x454
- frame 4 -> `perseusMonument_3.png` = 236x472

The source four-view artwork is kept as `Perseus-reference.png`.
