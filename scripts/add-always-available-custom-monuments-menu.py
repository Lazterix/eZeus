from pathlib import Path
import shutil

ROOT = Path(r"X:\Projects\eZeus-Extended")
HEADER = ROOT / "widgets" / "egamemenu.h"
CPP = ROOT / "widgets" / "egamemenu.cpp"

for p in (HEADER, CPP):
    if not p.exists():
        raise SystemExit(f"MISSING: {p}")

def backup(path: Path, suffix: str):
    b = path.with_name(path.name + suffix)
    if not b.exists():
        shutil.copy2(path, b)
        print(f"Backup: {b}")

# ----------------------------------------------------------------------
# 1) eSPR can mark individual build-menu items as always available.
# ----------------------------------------------------------------------
h = HEADER.read_text(encoding="utf-8-sig")

if "bool fAlwaysAvailable = false;" not in h:
    old = "    int fCity = -1;\n};"
    new = "    int fCity = -1;\n    bool fAlwaysAvailable = false;\n};"

    if old not in h:
        raise SystemExit("ERROR: could not locate eSPR tail in egamemenu.h")

    backup(HEADER, ".before-always-available-monuments")
    h = h.replace(old, new, 1)
    HEADER.write_text(h, encoding="utf-8")
    print("PATCHED: eSPR.fAlwaysAvailable")
else:
    print("SKIP: eSPR.fAlwaysAvailable already exists")

# ----------------------------------------------------------------------
# 2) A submenu becomes visible if it contains at least one always-available item.
# ----------------------------------------------------------------------
c = CPP.read_text(encoding="utf-8-sig")

old_visibility = """                const bool s = showAllPossibleBuildings ||
                               mBoard.supportsBuilding(cid, c.fMode);"""

new_visibility = """                const bool s = c.fAlwaysAvailable ||
                               showAllPossibleBuildings ||
                               mBoard.supportsBuilding(cid, c.fMode);"""

if "const bool s = c.fAlwaysAvailable ||" not in c:
    if old_visibility not in c:
        raise SystemExit("ERROR: could not locate submenu visibility filter in egamemenu.cpp")

    backup(CPP, ".before-always-available-monuments")
    c = c.replace(old_visibility, new_visibility, 1)
    print("PATCHED: always-available child keeps submenu visible")
else:
    print("SKIP: submenu visibility already patched")

# ----------------------------------------------------------------------
# 3) openBuildWidget must not filter out always-available custom entries.
# ----------------------------------------------------------------------
old_filter = """        if(!mBoard->supportsBuilding(cid, c.fMode) &&
           !mShowAllPossibleBuildings) continue;"""

new_filter = """        if(!c.fAlwaysAvailable &&
           !mBoard->supportsBuilding(cid, c.fMode) &&
           !mShowAllPossibleBuildings) continue;"""

if "if(!c.fAlwaysAvailable &&" not in c:
    if old_filter not in c:
        raise SystemExit("ERROR: could not locate openBuildWidget availability filter")

    c = c.replace(old_filter, new_filter, 1)
    print("PATCHED: always-available items bypass scenario availability")
else:
    print("SKIP: build-widget filter already patched")

# ----------------------------------------------------------------------
# 4) Mark Poseidon Monument as the first always-available custom monument.
#    This makes the existing Monuments button appear under the top two
#    decoration buttons, even when the scenario provides no monuments.
# ----------------------------------------------------------------------
old_poseidon = (
    "eSPR{eBuildingMode::poseidonMonument, "
    "eLanguage::zeusText(198, 11)}"
)
new_poseidon = (
    "eSPR{eBuildingMode::poseidonMonument, "
    "eLanguage::zeusText(198, 11), 0, -1, true}"
)

if new_poseidon not in c:
    count = c.count(old_poseidon)
    if count != 1:
        raise SystemExit(
            f"ERROR: expected exactly one Poseidon monument menu entry, found {count}"
        )

    c = c.replace(old_poseidon, new_poseidon, 1)
    print("PATCHED: Poseidon Monument is always available")
else:
    print("SKIP: Poseidon Monument already marked always available")

CPP.write_text(c, encoding="utf-8")

print()
print("SUCCESS")
print("The existing Monuments button in the Aesthetics/Decorations panel")
print("will now stay visible because Poseidon is an always-available child.")
print("Click Monuments -> Poseidon to place it; Ctrl+F9 is no longer required.")
print()
print("Future custom monuments only need fAlwaysAvailable=true in m9spr.")
