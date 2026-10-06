from pathlib import Path
from PIL import Image
import shutil

ROOT = Path(r"X:\Projects\eZeus-Extended")
OVERRIDES = ROOT / "assets" / "overrides"

# Only operate on the lower part of each sprite where the pedestal/base outline lives.
BOTTOM_START_BY_RES = {
    "15": 0.58,
    "30": 0.58,
    "45": 0.58,
    "60": 0.58,
}

TARGET_FILES = [
    OVERRIDES / "15" / "poseidonStatue_0.png",
    OVERRIDES / "30" / "poseidonStatue_0.png",
    OVERRIDES / "45" / "poseidonStatue_0.png",
    OVERRIDES / "60" / "poseidonStatue_0.png",
    OVERRIDES / "60" / "poseidonStatue_1.png",
    OVERRIDES / "60" / "poseidonStatue_2.png",
    OVERRIDES / "60" / "poseidonStatue_3.png",
]

def brightness(rgb):
    r, g, b = rgb
    return 0.2126 * r + 0.7152 * g + 0.0722 * b

def is_outer_edge(alpha_mask, x, y, w, h):
    for dx, dy in ((-1,0), (1,0), (0,-1), (0,1)):
        nx, ny = x + dx, y + dy
        if nx < 0 or ny < 0 or nx >= w or ny >= h:
            return True
        if not alpha_mask[ny][nx]:
            return True
    return False

def fix_image(path: Path, bottom_start_ratio: float):
    if not path.exists():
        print(f"SKIP missing: {path}")
        return

    backup = path.with_suffix(path.suffix + ".base-outline-backup")
    if not backup.exists():
        shutil.copy2(path, backup)
        print(f"Backup: {backup}")

    img = Image.open(path).convert("RGBA")
    px = img.load()
    w, h = img.size
    start_y = int(h * bottom_start_ratio)

    alpha_mask = [[px[x, y][3] > 0 for x in range(w)] for y in range(h)]

    changed = 0

    # First pass: find suspicious dark outer-edge pixels around pedestal/base only.
    targets = []
    for y in range(start_y, h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a == 0:
                continue
            if not is_outer_edge(alpha_mask, x, y, w, h):
                continue

            cur_b = brightness((r, g, b))
            # Keep legitimate dark internal details; only the exterior rim is targeted.
            if cur_b > 105:
                continue

            targets.append((x, y, cur_b))

    # Second pass: repaint each target from a nearby interior pedestal colour.
    for x, y, cur_b in targets:
        best = None
        best_score = None

        for radius in range(1, 8):
            xmin = max(0, x - radius)
            xmax = min(w - 1, x + radius)
            ymin = max(start_y, y - radius)
            ymax = min(h - 1, y + radius)

            for sy in range(ymin, ymax + 1):
                for sx in range(xmin, xmax + 1):
                    rr, gg, bb, aa = px[sx, sy]
                    if aa == 0:
                        continue

                    # Source must be interior, not another edge pixel.
                    if is_outer_edge(alpha_mask, sx, sy, w, h):
                        continue

                    src_b = brightness((rr, gg, bb))
                    if src_b <= cur_b + 10:
                        continue

                    # Favor nearby pixels and similar warm marble/stone tones.
                    dist = abs(sx - x) + abs(sy - y)
                    warm_bias = abs(rr - gg) + abs(gg - bb)
                    score = (dist, warm_bias, -src_b)

                    if best is None or score < best_score:
                        best = (rr, gg, bb, a)
                        best_score = score

            if best is not None:
                break

        if best is not None:
            px[x, y] = best
            changed += 1

    img.save(path)
    print(f"Fixed: {path} | changed pixels: {changed}")

def main():
    print("=== Poseidon pedestal outer-outline cleanup ===")
    for path in TARGET_FILES:
        res = path.parent.name
        fix_image(path, BOTTOM_START_BY_RES.get(res, 0.58))
    print("Done.")

if __name__ == "__main__":
    main()
