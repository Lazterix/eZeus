from pathlib import Path
import shutil

ROOT = Path(r"X:\Projects\eZeus-Extended")
GAMEWIDGET = ROOT / "widgets" / "egamewidget.cpp"
PAINT = ROOT / "widgets" / "egamewidgetpaint.cpp"

for p in (GAMEWIDGET, PAINT):
    if not p.exists():
        raise SystemExit(f"MISSING: {p}")

def backup(p: Path, suffix: str):
    b = p.with_name(p.name + suffix)
    if not b.exists():
        shutil.copy2(p, b)
        print(f"Backup: {b}")

# ----------------------------------------------------------------------
# 1) Q / E while a building tool is active:
#    rotate the world/camera 90 degrees left/right so the placement
#    preview shows the corresponding directional sprite BEFORE placement.
# ----------------------------------------------------------------------
text = GAMEWIDGET.read_text(encoding="utf-8-sig")

if "BUILD PREVIEW ROTATION Q/E" not in text:
    marker = "} else if(k == SDL_Scancode::SDL_SCANCODE_LEFT) {"
    pos = text.find(marker)
    if pos == -1:
        raise SystemExit(
            "ERROR: could not locate LEFT-arrow branch in eGameWidget::keyPressEvent()."
        )

    insertion = r''' } else if((k == SDL_Scancode::SDL_SCANCODE_Q ||
              k == SDL_Scancode::SDL_SCANCODE_E) &&
              mGm->mode() != eBuildingMode::none) {
        // BUILD PREVIEW ROTATION Q/E
        // eZeus uses four pre-rendered directional sprites. Rotating the
        // world direction while a building tool is active lets us inspect
        // the correct N/W/S/E preview before placing the building.
        if(!mBoard) return true;

        const auto dir = mBoard->direction();
        const bool clockwise = k == SDL_Scancode::SDL_SCANCODE_E;
        eWorldDirection nextDir = dir;

        if(clockwise) {
            switch(dir) {
            case eWorldDirection::N:
                nextDir = eWorldDirection::W;
                break;
            case eWorldDirection::W:
                nextDir = eWorldDirection::S;
                break;
            case eWorldDirection::S:
                nextDir = eWorldDirection::E;
                break;
            case eWorldDirection::E:
                nextDir = eWorldDirection::N;
                break;
            }
        } else {
            switch(dir) {
            case eWorldDirection::N:
                nextDir = eWorldDirection::E;
                break;
            case eWorldDirection::E:
                nextDir = eWorldDirection::S;
                break;
            case eWorldDirection::S:
                nextDir = eWorldDirection::W;
                break;
            case eWorldDirection::W:
                nextDir = eWorldDirection::N;
                break;
            }
        }

        setWorldDirection(nextDir);
   '''

    backup(GAMEWIDGET, ".before-build-preview-rotation")
    text = text[:pos] + insertion + text[pos:]
    GAMEWIDGET.write_text(text, encoding="utf-8")
    print("PATCHED: Q/E build-preview rotation")
else:
    print("SKIP: Q/E build-preview rotation already present")

# ----------------------------------------------------------------------
# 2) Poseidon dev preview:
#    Ctrl+F9 deliberately places the monument without the auxiliary 4x4
#    eGodMonumentTile surround. Make the preview match the final placement.
# ----------------------------------------------------------------------
text = PAINT.read_text(encoding="utf-8-sig")

if "POSEIDON DEV PREVIEW: NO AUXILIARY TILES" not in text:
    start = text.find("        case eBuildingMode::aphroditeMonument:")
    if start == -1:
        raise SystemExit("ERROR: god-monument preview block start not found.")

    end = text.find("        case eBuildingMode::bench:", start)
    if end == -1:
        raise SystemExit("ERROR: god-monument preview block end not found.")

    region = text[start:end]

    loop_start = region.find("            for(int x = tminX; x < tmaxX; x++) {")
    if loop_start == -1:
        raise SystemExit("ERROR: auxiliary monument preview tile loop not found.")

    after_loop_marker = "\n\n            ebs.emplace_back(mHoverTX, mHoverTY, b1);"
    loop_end = region.find(after_loop_marker, loop_start)
    if loop_end == -1:
        raise SystemExit("ERROR: end of auxiliary monument preview tile loop not found.")

    loop = region[loop_start:loop_end]

    replacement = (
        "            // POSEIDON DEV PREVIEW: NO AUXILIARY TILES\n"
        "            // Ctrl+F9 forces Poseidon even when the scenario does not\n"
        "            // normally expose that build mode. Its real dev placement\n"
        "            // intentionally skips eGodMonumentTile, so the preview must\n"
        "            // skip those tiles too.\n"
        "            const bool devForcedPoseidon =\n"
        "                mode == eBuildingMode::poseidonMonument &&\n"
        "                !mBoard->supportsBuilding(mViewedCityId, mode);\n"
        "\n"
        "            if(!devForcedPoseidon) {\n"
    )

    replacement += "\n".join(
        ("    " + line if line.strip() else line)
        for line in loop.splitlines()
    )
    replacement += "\n            }"

    region = region[:loop_start] + replacement + region[loop_end:]
    new_text = text[:start] + region + text[end:]

    backup(PAINT, ".before-poseidon-preview-cleanup")
    PAINT.write_text(new_text, encoding="utf-8")
    print("PATCHED: Ctrl+F9 Poseidon preview no longer shows auxiliary tiles")
else:
    print("SKIP: Poseidon preview cleanup already present")

print()
print("SUCCESS")
print("Controls while a building tool is active:")
print("  Q = rotate preview/world 90 degrees one way")
print("  E = rotate preview/world 90 degrees the other way")
print("  Ctrl+F10 = existing dev world-rotation hotkey")
print()
print("Ctrl+F9 Poseidon preview now matches the final placement: no extra 4x4 tiles.")
