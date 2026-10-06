# Corrected Poseidon God Monument optional override

The previous pack targeted `poseidonStatues2`, which is NOT the god monument used by
`eGodMonument`. The actual monument collection is loaded as `poseidonStatue`.

Correct runtime files:

- `Textures/15/poseidonStatue_0.png`
- `Textures/30/poseidonStatue_0.png`
- `Textures/45/poseidonStatue_0.png`
- `Textures/60/poseidonStatue_0.png`
- `Textures/60/poseidonStatue_1.png`
- `Textures/60/poseidonStatue_2.png`
- `Textures/60/poseidonStatue_3.png`

The installer automatically moves the previous wrong-target
`poseidonStatues2_0.png` files out of the active runtime.

Packed originals (`i15.e`, `i30.e`, `i45.e`, `i60.e`) are NEVER modified.

Install:
`pwsh -ExecutionPolicy Bypass -File .\scripts\poseidon-monument-override.ps1 Install`

Status:
`pwsh -ExecutionPolicy Bypass -File .\scripts\poseidon-monument-override.ps1 Status`

Disable:
`pwsh -ExecutionPolicy Bypass -File .\scripts\poseidon-monument-override.ps1 Disable`

Enable:
`pwsh -ExecutionPolicy Bypass -File .\scripts\poseidon-monument-override.ps1 Enable`
