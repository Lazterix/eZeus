from pathlib import Path
import runpy
import shutil

ROOT = Path(r"X:\Projects\eZeus-Extended")

BUILDING_H = ROOT / "buildings" / "ebuilding.h"
MODE_H = ROOT / "widgets" / "ebuildingmode.h"
INSTALLER = ROOT / "scripts" / "add-perseus-custom-monument.py"


def fail(msg: str):
    raise SystemExit(f"\nPATCH ERROR: {msg}")


def backup(path: Path):
    dst = path.with_name(path.name + ".before-perseus-enum-v2")
    if not dst.exists():
        shutil.copy2(path, dst)
        print(f"Backup: {dst}")


def append_enum_member(path: Path, enum_name: str, member: str, comment: str):
    if not path.exists():
        fail(f"missing file: {path}")

    text = path.read_text(encoding="utf-8-sig")

    start_token = f"enum class {enum_name}"
    start = text.find(start_token)
    if start < 0:
        fail(f"cannot find {start_token} in {path}")

    brace = text.find("{", start)
    if brace < 0:
        fail(f"cannot find opening brace for {enum_name} in {path}")

    # These enums contain no nested braces, so the first `};` after the
    # opening brace is the enum terminator.
    end = text.find("};", brace)
    if end < 0:
        fail(f"cannot find end of {enum_name} in {path}")

    body = text[brace + 1:end]

    if member in body:
        print(f"SKIP: {enum_name}::{member} already exists")
        return

    # Preserve all existing numeric enum values by APPENDING at the end.
    # Normalize the previous final enumerator so it has a comma.
    lines = body.splitlines()

    last = None
    for i in range(len(lines) - 1, -1, -1):
        if lines[i].strip():
            last = i
            break

    if last is None:
        fail(f"{enum_name} body is unexpectedly empty")

    previous = lines[last].rstrip()
    if not previous.endswith(","):
        lines[last] = previous + ","

    lines.append("")
    lines.append(f"    // {comment}")
    lines.append(f"    {member}")

    new_body = "\n".join(lines)
    if body.endswith("\n"):
        new_body += "\n"

    backup(path)
    text = text[:brace + 1] + new_body + text[end:]
    path.write_text(text, encoding="utf-8")

    print(f"PATCHED: {enum_name}::{member}")


print("=== PERSEUS INSTALLER v2 — ENUM REPAIR + CONTINUE ===")
print()

append_enum_member(
    BUILDING_H,
    "eBuildingType",
    "customMonument",
    "Custom/remastered monuments. Appended to preserve existing enum values."
)

append_enum_member(
    MODE_H,
    "eBuildingMode",
    "perseusMonument",
    "Always-available custom monument collection."
)

if not INSTALLER.exists():
    fail(
        f"missing original installer: {INSTALLER}\n"
        "Extract Perseus_Custom_Monument_v1.zip into the project first."
    )

print()
print("Enums repaired. Continuing with the full Perseus installer...")
print()

runpy.run_path(str(INSTALLER), run_name="__main__")
