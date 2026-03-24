import cv2
import numpy as np
from pathlib import Path

# These python code are vibe coded garbage, do not read

# =========================
# Config
# =========================
SRC_DIR = Path("../badApple/original/")
TARGET_W = 96
TARGET_H = 72

OUT_DIR = Path(f"../badApple/{TARGET_W}x{TARGET_H}")
OUT_SDF_BIN = OUT_DIR / "sdf.bin"
OUT_SDF_IMG_DIR = OUT_DIR / "signedDistanceFields"

WRITE_SDF_BIN = True
WRITE_SDF_DEBUG_IMAGES = True

# Threshold for binarizing source frames
THRESHOLD = 127

# Small epsilon to avoid division by zero
EPS = 1e-8


# =========================
# Helpers
# =========================
def ensure_dirs():
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_SDF_IMG_DIR.mkdir(parents=True, exist_ok=True)


def sorted_image_paths(src_dir: Path):
    exts = {".jpg", ".jpeg", ".png", ".bmp"}
    paths = [p for p in src_dir.iterdir() if p.is_file() and p.suffix.lower() in exts]
    paths.sort()
    return paths


def read_gray(path: Path):
    img = cv2.imread(str(path), cv2.IMREAD_GRAYSCALE)
    if img is None:
        raise RuntimeError(f"Failed to read image: {path}")
    return img


def compute_signed_distance_field(gray: np.ndarray, threshold: int = 127) -> np.ndarray:
    """
    Produces a signed distance field in the range [-1, 1].

    Convention:
      white / foreground  -> positive
      black / background  -> negative

    Steps:
      1. Threshold to binary mask
      2. Compute distance-to-background for foreground
      3. Compute distance-to-foreground for background
      4. signed = inside_dist - outside_dist
      5. Normalize per frame to [-1, 1]
    """
    # Foreground = white
    _, fg = cv2.threshold(gray, threshold, 255, cv2.THRESH_BINARY)

    # Distance inside white regions to nearest black pixel
    inside = cv2.distanceTransform(fg, cv2.DIST_L2, 5)

    # Distance inside black regions to nearest white pixel
    bg = cv2.bitwise_not(fg)
    outside = cv2.distanceTransform(bg, cv2.DIST_L2, 5)

    sdf = inside - outside

    max_abs = float(np.max(np.abs(sdf)))
    if max_abs < EPS:
        max_abs = 1.0

    sdf = (sdf / max_abs).astype(np.float32)
    return sdf


def resize_sdf(sdf: np.ndarray, dst_w: int, dst_h: int) -> np.ndarray:
    return cv2.resize(sdf, (dst_w, dst_h), interpolation=cv2.INTER_AREA).astype(np.float32)


def sdf_to_debug_image(sdf_small: np.ndarray) -> np.ndarray:
    """
    Debug image rule requested by user:
      values < 0 => black
      values > 1 => white
      values in [0,1] mapped linearly to [0,255]

    Since our SDF is already in [-1,1], this effectively means:
      negative values -> 0
      positive values -> scaled to 0..255
    """
    clamped = np.clip(sdf_small, 0.0, 1.0)
    img = (clamped * 255.0).astype(np.uint8)
    return img


# =========================
# Main
# =========================
def main():
    ensure_dirs()

    image_paths = sorted_image_paths(SRC_DIR)
    if not image_paths:
        raise RuntimeError(f"No input frames found in {SRC_DIR.resolve()}")

    print(f"Found {len(image_paths)} source frames")

    sdf_bin_file = open(OUT_SDF_BIN, "wb") if WRITE_SDF_BIN else None

    try:
        for idx, img_path in enumerate(image_paths, start=1):
            gray = read_gray(img_path)

            sdf = compute_signed_distance_field(gray, THRESHOLD)
            sdf_small = resize_sdf(sdf, TARGET_W, TARGET_H)

            if WRITE_SDF_BIN:
                sdf_bin_file.write(np.ascontiguousarray(sdf_small, dtype=np.float32).tobytes())

            if WRITE_SDF_DEBUG_IMAGES:
                debug = sdf_to_debug_image(sdf_small)
                out_img_path = OUT_SDF_IMG_DIR / f"{idx:05d}.png"
                ok = cv2.imwrite(str(out_img_path), debug)
                if not ok:
                    raise RuntimeError(f"Failed to write debug image: {out_img_path}")

            if idx % 100 == 0 or idx == len(image_paths):
                print(f"Processed {idx}/{len(image_paths)}")

        print("Done.")
        if WRITE_SDF_BIN:
            print(f"sdf.bin:    {OUT_SDF_BIN.resolve()}")
        if WRITE_SDF_DEBUG_IMAGES:
            print(f"debug imgs: {OUT_SDF_IMG_DIR.resolve()}")

    finally:
        if sdf_bin_file:
            sdf_bin_file.close()


if __name__ == "__main__":
    main()