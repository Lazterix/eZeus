from pathlib import Path
import shutil

ROOT = Path(r"X:\Projects\eZeus-Extended")
RUNTIME = Path(r"D:\Steam\steamapps\common\Zeus + Poseidon\.ezeus-dev-runtime\eZeus")

FILES = {
    "building_h": ROOT / "buildings" / "ebuilding.h",
    "building_cpp": ROOT / "buildings" / "ebuilding.cpp",
    "aest_h": ROOT / "buildings" / "eaestheticsbuilding.h",
    "aest_cpp": ROOT / "buildings" / "eaestheticsbuilding.cpp",
    "mode_h": ROOT / "widgets" / "ebuildingmode.h",
    "mode_cpp": ROOT / "widgets" / "ebuildingmode.cpp",
    "tex_h": ROOT / "textures" / "ebuildingtextures.h",
    "tex_cpp": ROOT / "textures" / "ebuildingtextures.cpp",
    "game_tex_h": ROOT / "textures" / "egametextures.h",
    "game_tex_cpp": ROOT / "textures" / "egametextures.cpp",
    "build_cpp": ROOT / "widgets" / "egamewidgetbuild.cpp",
    "paint_cpp": ROOT / "widgets" / "egamewidgetpaint.cpp",
    "menu_cpp": ROOT / "widgets" / "egamemenu.cpp",
    "reader_cpp": ROOT / "fileIO" / "ebuildingreader.cpp",
    "writer_cpp": ROOT / "fileIO" / "ebuildingwriter.cpp",
    "thread_cpp": ROOT / "engine" / "thread" / "ethreadbuilding.cpp",
    "difficulty_cpp": ROOT / "engine" / "edifficulty.cpp",
    "heat_cpp": ROOT / "buildings" / "eheatgetters.cpp",
}

for name, p in FILES.items():
    if not p.exists():
        raise SystemExit(f"MISSING {name}: {p}")

for res in (15, 30, 45, 60):
    p = ROOT / "spriteData" / f"perseusMonument{res}.h"
    if not p.exists():
        raise SystemExit(
            f"MISSING sprite data: {p}\n"
            "Extract the Perseus package into X:\\Projects\\eZeus-Extended first."
        )

ASSET_FILES = [
    ROOT / "assets" / "overrides" / "15" / "perseusMonument_0.png",
    ROOT / "assets" / "overrides" / "30" / "perseusMonument_0.png",
    ROOT / "assets" / "overrides" / "45" / "perseusMonument_0.png",
    ROOT / "assets" / "overrides" / "60" / "perseusMonument_0.png",
    ROOT / "assets" / "overrides" / "60" / "perseusMonument_1.png",
    ROOT / "assets" / "overrides" / "60" / "perseusMonument_2.png",
    ROOT / "assets" / "overrides" / "60" / "perseusMonument_3.png",
]
for p in ASSET_FILES:
    if not p.exists():
        raise SystemExit(f"MISSING asset: {p}")

def read(path):
    return path.read_text(encoding="utf-8-sig")

def write(path, text):
    backup = path.with_name(path.name + ".before-perseus-monument")
    if not backup.exists():
        shutil.copy2(path, backup)
        print(f"Backup: {backup}")
    path.write_text(text, encoding="utf-8")

def replace_once(path, old, new, label, already=None):
    text = read(path)
    if already and already in text:
        print(f"SKIP: {label}")
        return
    count = text.count(old)
    if count != 1:
        raise SystemExit(
            f"PATCH ERROR [{label}]: expected exactly 1 match in {path}, found {count}"
        )
    text = text.replace(old, new, 1)
    write(path, text)
    print(f"PATCHED: {label}")

def insert_before_once(path, marker, block, label, already):
    text = read(path)
    if already in text:
        print(f"SKIP: {label}")
        return
    count = text.count(marker)
    if count != 1:
        raise SystemExit(
            f"PATCH ERROR [{label}]: expected exactly 1 marker in {path}, found {count}"
        )
    text = text.replace(marker, block + marker, 1)
    write(path, text)
    print(f"PATCHED: {label}")

def insert_after_once(path, marker, block, label, already):
    text = read(path)
    if already in text:
        print(f"SKIP: {label}")
        return
    count = text.count(marker)
    if count != 1:
        raise SystemExit(
            f"PATCH ERROR [{label}]: expected exactly 1 marker in {path}, found {count}"
        )
    text = text.replace(marker, marker + block, 1)
    write(path, text)
    print(f"PATCHED: {label}")

print("=== PERSEUS CUSTOM MONUMENT INSTALLER ===")
print()

# eBuildingType: append, preserving every existing numeric value.
replace_once(
    FILES["building_h"],
    "    hippodromePiece,\n    crosswalk\n};",
    "    hippodromePiece,\n    crosswalk,\n\n    // Custom/remastered monuments. Appended to preserve old enum values.\n    customMonument\n};",
    "eBuildingType::customMonument",
    already="    customMonument\n};"
)

# eBuildingMode: append Perseus, preserving all existing values.
replace_once(
    FILES["mode_h"],
    "    hippodromePiece,\n    crosswalk\n};",
    "    hippodromePiece,\n    crosswalk,\n\n    // Always-available custom monument collection.\n    perseusMonument\n};",
    "eBuildingMode::perseusMonument",
    already="    perseusMonument\n};"
)

# Map mode -> generic custom monument building type.
replace_once(
    FILES["mode_cpp"],
    "    case eBuildingMode::crosswalk:\n        return eBuildingType::crosswalk;\n",
    "    case eBuildingMode::crosswalk:\n        return eBuildingType::crosswalk;\n"
    "    case eBuildingMode::perseusMonument:\n        return eBuildingType::customMonument;\n",
    "Perseus building-mode mapping",
    already="case eBuildingMode::perseusMonument:"
)

# Generic custom monument class.
custom_class = r'''class eCustomMonument : public eBuilding {
public:
    // id 0 = Perseus. Future custom monuments extend this id table.
    eCustomMonument(const int id,
                    eGameBoard& board, const eCityId cid);

    std::shared_ptr<eTexture>
    getTexture(const eTileSize size) const override;

    int id() const { return mId; }

private:
    const int mId;
};

'''
insert_before_once(
    FILES["aest_h"],
    "class eGodMonumentTile;\n",
    custom_class,
    "eCustomMonument class",
    already="class eCustomMonument : public eBuilding"
)

# Ensure board direction is available.
aest_cpp = read(FILES["aest_cpp"])
if '#include "engine/egameboard.h"' not in aest_cpp:
    marker = '#include "textures/egametextures.h"\n'
    if marker not in aest_cpp:
        raise SystemExit("PATCH ERROR: cannot find egametextures include in eaestheticsbuilding.cpp")
    aest_cpp = aest_cpp.replace(marker, marker + '#include "engine/egameboard.h"\n', 1)
    write(FILES["aest_cpp"], aest_cpp)
    print("PATCHED: egameboard include for custom monument direction")
else:
    print("SKIP: egameboard include already present")

custom_impl = r'''eCustomMonument::eCustomMonument(
        const int id,
        eGameBoard& board, const eCityId cid) :
    eBuilding(board, eBuildingType::customMonument, 2, 2, cid),
    mId(id) {
    switch(mId) {
    case 0:
        eGameTextures::loadPerseusMonument();
        break;
    default:
        break;
    }
}

std::shared_ptr<eTexture>
eCustomMonument::getTexture(const eTileSize size) const {
    const int sizeId = static_cast<int>(size);
    const auto& texs = eGameTextures::buildings()[sizeId];

    const eTextureCollection* coll = nullptr;
    switch(mId) {
    case 0:
        coll = &texs.fPerseusMonument;
        break;
    default:
        return nullptr;
    }

    int dirId = 0;
    switch(getBoard().direction()) {
    case eWorldDirection::N:
        dirId = 0;
        break;
    case eWorldDirection::W:
        dirId = 1;
        break;
    case eWorldDirection::S:
        dirId = 2;
        break;
    case eWorldDirection::E:
        dirId = 3;
        break;
    }

    return coll->getTexture(dirId);
}

'''
insert_before_once(
    FILES["aest_cpp"],
    "eGodMonument::eGodMonument(",
    custom_impl,
    "eCustomMonument implementation",
    already="eCustomMonument::eCustomMonument("
)

# Texture plumbing.
tex_h = read(FILES["tex_h"])
if "void loadPerseusMonument();" not in tex_h:
    marker = "    bool fPoseidonMonumentsLoaded = false;\n    void loadPoseidonMonuments();\n"
    if marker not in tex_h:
        raise SystemExit("PATCH ERROR: Poseidon monument loader declaration not found")
    tex_h = tex_h.replace(
        marker,
        marker +
        "    bool fPerseusMonumentLoaded = false;\n"
        "    void loadPerseusMonument();\n",
        1
    )
    write(FILES["tex_h"], tex_h)
    print("PATCHED: Perseus texture loader declaration")
else:
    print("SKIP: Perseus texture loader declaration")

tex_h = read(FILES["tex_h"])
if "eTextureCollection fPerseusMonument;" not in tex_h:
    marker = "    eTextureCollection fPoseidonMonuments;\n"
    if marker not in tex_h:
        raise SystemExit("PATCH ERROR: fPoseidonMonuments field not found")
    tex_h = tex_h.replace(
        marker,
        marker + "    eTextureCollection fPerseusMonument;\n",
        1
    )
    write(FILES["tex_h"], tex_h)
    print("PATCHED: fPerseusMonument texture collection")
else:
    print("SKIP: fPerseusMonument texture collection")

# Include spriteData.
tex_cpp = read(FILES["tex_cpp"])
if '#include "spriteData/perseusMonument15.h"' not in tex_cpp:
    marker = '#include "spriteData/poseidonStatue60.h"\n'
    if marker not in tex_cpp:
        raise SystemExit("PATCH ERROR: poseidonStatue60 include not found")
    block = (
        '\n#include "spriteData/perseusMonument15.h"\n'
        '#include "spriteData/perseusMonument30.h"\n'
        '#include "spriteData/perseusMonument45.h"\n'
        '#include "spriteData/perseusMonument60.h"\n'
    )
    tex_cpp = tex_cpp.replace(marker, marker + block, 1)
    write(FILES["tex_cpp"], tex_cpp)
    print("PATCHED: Perseus spriteData includes")
else:
    print("SKIP: Perseus spriteData includes")

replace_once(
    FILES["tex_cpp"],
    "    fPoseidonMonuments(renderer),\n",
    "    fPoseidonMonuments(renderer),\n"
    "    fPerseusMonument(renderer),\n",
    "fPerseusMonument renderer initialization",
    already="    fPerseusMonument(renderer),"
)

perseus_loader = r'''
void eBuildingTextures::loadPerseusMonument() {
    if(fPerseusMonumentLoaded) return;
    fPerseusMonumentLoaded = true;
    loadGodMonuments(ePerseusMonumentSpriteData15,
                     ePerseusMonumentSpriteData30,
                     ePerseusMonumentSpriteData45,
                     ePerseusMonumentSpriteData60,
                     "perseusMonument",
                     fPerseusMonument);
}

'''
insert_before_once(
    FILES["tex_cpp"],
    "void eBuildingTextures::loadHadesMonuments()",
    perseus_loader,
    "Perseus texture loader implementation",
    already="void eBuildingTextures::loadPerseusMonument()"
)

insert_after_once(
    FILES["game_tex_h"],
    "    static void loadPoseidonMonuments();\n",
    "    static void loadPerseusMonument();\n",
    "eGameTextures Perseus declaration",
    already="static void loadPerseusMonument();"
)

game_tex_impl = r'''
void eGameTextures::loadPerseusMonument() {
    loadTexture([](const int i) {
        auto& c = sBuildingTextures[i];
        c.loadPerseusMonument();
    });
}

'''
insert_before_once(
    FILES["game_tex_cpp"],
    "void eGameTextures::loadHadesMonuments()",
    game_tex_impl,
    "eGameTextures Perseus implementation",
    already="void eGameTextures::loadPerseusMonument()"
)

# Building semantics.
replace_once(
    FILES["building_cpp"],
    "    const bool r = bi >= min && bi <= max;\n    return r;\n}\n\nbool eBuilding::sHeroHall",
    "    const bool r = bi >= min && bi <= max;\n"
    "    return r || bt == eBuildingType::customMonument;\n"
    "}\n\nbool eBuilding::sHeroHall",
    "custom monument aesthetics semantics",
    already="return r || bt == eBuildingType::customMonument;"
)

# Specific runtime name for Perseus.
building_cpp = read(FILES["building_cpp"])
name_obj_pos = building_cpp.find("std::string eBuilding::sNameForBuilding(eBuilding* const b)")
name_type_pos = building_cpp.find("std::string eBuilding::sNameForBuilding(const eBuildingType type)")
segment = building_cpp[name_obj_pos:name_type_pos]
if "case eBuildingType::customMonument:" not in segment:
    marker = "    case eBuildingType::commemorative: {\n"
    pos = building_cpp.find(marker, name_obj_pos, name_type_pos)
    if pos == -1:
        raise SystemExit("PATCH ERROR: object-name commemorative case not found")
    block = (
        "    case eBuildingType::customMonument: {\n"
        "        const auto c = static_cast<eCustomMonument*>(b);\n"
        "        if(c->id() == 0) return \"Perseus Monument\";\n"
        "        return \"Custom Monument\";\n"
        "    } break;\n"
    )
    building_cpp = building_cpp[:pos] + block + building_cpp[pos:]
    write(FILES["building_cpp"], building_cpp)
    print("PATCHED: custom monument runtime name")
else:
    print("SKIP: custom monument runtime name")

# Generic type name.
building_cpp = read(FILES["building_cpp"])
name_type_pos = building_cpp.find("std::string eBuilding::sNameForBuilding(const eBuildingType type)")
info_pos = building_cpp.find("void eBuilding::sInfoText", name_type_pos)
segment = building_cpp[name_type_pos:info_pos]
if "case eBuildingType::customMonument:" not in segment:
    marker = "    case eBuildingType::commemorative:\n        string = 119;\n        break;\n"
    if marker not in segment:
        raise SystemExit("PATCH ERROR: commemorative type-name marker not found")
    segment = segment.replace(
        marker,
        marker +
        "    case eBuildingType::customMonument:\n"
        "        return \"Custom Monument\";\n",
        1
    )
    building_cpp = building_cpp[:name_type_pos] + segment + building_cpp[info_pos:]
    write(FILES["building_cpp"], building_cpp)
    print("PATCHED: custom monument generic type name")
else:
    print("SKIP: custom monument generic type name")

# Info widget: reuse existing localized Perseus hero title.
building_cpp = read(FILES["building_cpp"])
info_pos = building_cpp.find("void eBuilding::sInfoText")
segment = building_cpp[info_pos:]
if "case eBuildingType::customMonument:" not in segment:
    marker = "    case eBuildingType::achillesHall:\n"
    pos = building_cpp.find(marker, info_pos)
    if pos == -1:
        raise SystemExit("PATCH ERROR: hero hall info marker not found")
    block = (
        "    case eBuildingType::customMonument:\n"
        "        group = 185;\n"
        "        titleString = 12; // Perseus\n"
        "        infoString = -1;\n"
        "        employmentInfoString = -1;\n"
        "        break;\n"
    )
    building_cpp = building_cpp[:pos] + block + building_cpp[pos:]
    write(FILES["building_cpp"], building_cpp)
    print("PATCHED: Perseus monument info title")
else:
    print("SKIP: Perseus monument info title")

# Save/load.
reader_block = r'''    case eBuildingType::customMonument: {
        int id;
        src >> id;
        b = e::make_shared<eCustomMonument>(id, board, cid);
    } break;

'''
insert_before_once(
    FILES["reader_cpp"],
    "    case eBuildingType::godMonument: {\n",
    reader_block,
    "custom monument save reader",
    already="case eBuildingType::customMonument:"
)

writer_block = r'''    case eBuildingType::customMonument: {
        const auto cm = static_cast<const eCustomMonument*>(b);
        dst << cm->id();
    } break;

'''
insert_before_once(
    FILES["writer_cpp"],
    "    case eBuildingType::godMonument: {\n",
    writer_block,
    "custom monument save writer",
    already="case eBuildingType::customMonument:"
)

insert_before_once(
    FILES["thread_cpp"],
    "        case eBuildingType::godMonument:\n",
    "        case eBuildingType::customMonument:\n",
    "thread building custom monument case",
    already="case eBuildingType::customMonument:"
)

insert_before_once(
    FILES["difficulty_cpp"],
    "    case eBuildingType::godMonument:\n",
    "    case eBuildingType::customMonument:\n",
    "custom monument zero construction cost",
    already="case eBuildingType::customMonument:"
)

# Appeal heat.
heat = read(FILES["heat_cpp"])
if "case eBuildingType::customMonument: return {35, 7};" not in heat:
    marker = "    case eBuildingType::godMonument: return {35, 7};\n"
    if marker not in heat:
        raise SystemExit("PATCH ERROR: godMonument heat getter marker not found")
    heat = heat.replace(
        marker,
        marker + "    case eBuildingType::customMonument: return {35, 7};\n",
        1
    )
    write(FILES["heat_cpp"], heat)
    print("PATCHED: custom monument appeal heat")
else:
    print("SKIP: custom monument appeal heat")

# Placement render fixes.
paint = read(FILES["paint_cpp"])
if "terrBt == eBuildingType::customMonument ||" not in paint:
    marker = "                    terrBt == eBuildingType::godMonument ||\n"
    if marker not in paint:
        raise SystemExit(
            "PATCH ERROR: working godMonument terrain-underlay rule not found.\n"
            "The Poseidon black-footprint fix must remain installed."
        )
    paint = paint.replace(
        marker,
        marker + "                    terrBt == eBuildingType::customMonument ||\n",
        1
    )
    write(FILES["paint_cpp"], paint)
    print("PATCHED: terrain under custom monuments")
else:
    print("SKIP: terrain under custom monuments")

paint = read(FILES["paint_cpp"])
if "ub->type() != eBuildingType::customMonument &&" not in paint:
    marker = "            if(ub->type() != eBuildingType::godMonument &&\n"
    if marker in paint:
        paint = paint.replace(
            marker,
            marker + "               ub->type() != eBuildingType::customMonument &&\n",
            1
        )
        write(FILES["paint_cpp"], paint)
        print("PATCHED: generic basement suppressed for custom monuments")
    else:
        print("NOTE: no godMonument basement suppression marker found; continuing.")
else:
    print("SKIP: custom monument basement suppression")

preview_block = r'''        case eBuildingMode::perseusMonument: {
            const auto b1 = e::make_shared<eCustomMonument>(
                                0, *mBoard, mViewedCityId);
            ebs.emplace_back(mHoverTX, mHoverTY, b1);
        } break;

'''
insert_before_once(
    FILES["paint_cpp"],
    "        case eBuildingMode::bench: {\n",
    preview_block,
    "Perseus placement preview",
    already="case eBuildingMode::perseusMonument:"
)

build_block = r'''        case eBuildingMode::perseusMonument: {
            r = mBoard->build(
                    mHoverTX, mHoverTY, 2, 2,
                    cid, pid, mEditorMode,
                    [this]() {
                        return e::make_shared<eCustomMonument>(
                            0, *mBoard, mViewedCityId);
                    });
        } break;

'''
insert_before_once(
    FILES["build_cpp"],
    "        case eBuildingMode::bench: {\n",
    build_block,
    "Perseus final placement",
    already="case eBuildingMode::perseusMonument:"
)

# Normal Monuments UI entry.
menu = read(FILES["menu_cpp"])
if "eBuildingMode::perseusMonument" not in menu:
    menu_h = read(ROOT / "widgets" / "egamemenu.h")
    if "bool fAlwaysAvailable = false;" not in menu_h:
        raise SystemExit(
            "PATCH ERROR: fAlwaysAvailable is missing from eSPR.\n"
            "Install the previously working custom-monuments menu patch first."
        )

    marker = (
        "                                     eSPR{eBuildingMode::poseidonMonument, "
        "eLanguage::zeusText(198, 11), 0, -1, true},\n"
    )
    if marker not in menu:
        marker2 = (
            "                                     eSPR{eBuildingMode::poseidonMonument, "
            "eLanguage::zeusText(198, 11)},\n"
        )
        if marker2 in menu:
            marker = marker2
        else:
            raise SystemExit("PATCH ERROR: Poseidon entry in m9spr not found")

    entry = (
        '                                     eSPR{eBuildingMode::perseusMonument, '
        '"Perseus Monument", 0, -1, true},\n'
    )
    menu = menu.replace(marker, marker + entry, 1)
    write(FILES["menu_cpp"], menu)
    print("PATCHED: Perseus added to always-available Monuments menu")
else:
    print("SKIP: Perseus already present in Monuments menu")

# Install loose PNGs into dev runtime.
for src in ASSET_FILES:
    rel = src.relative_to(ROOT / "assets" / "overrides")
    dst = RUNTIME / "Textures" / rel
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)
    print(f"INSTALLED: {rel}")

print()
print("=== SUCCESS ===")
print("Perseus is registered as custom monument id 0.")
print("It does NOT replace Poseidon or any packed original asset.")
print("Menu: Aesthetics/Decorations -> Monuments -> Perseus Monument")
print("Preview/final footprint: 2x2, no auxiliary god-monument tile ring.")
print("Q/E preview rotation uses all four Perseus directional sprites.")
print("Terrain-underlay rule prevents the Poseidon-style black footprint.")
print()
print("NEXT:")
print(r"  cd X:\Projects\eZeus-Extended")
print(r"  cmake --build .\build --config Release --parallel")
print(r"  Copy-Item .\build\Release\eZeus.exe 'D:\Steam\steamapps\common\Zeus + Poseidon\.ezeus-dev-runtime\eZeus\Bin\eZeus.exe' -Force")
