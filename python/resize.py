import cv2
from pathlib import Path

def resize(w, h):
    # Input / output settings
    src_dir = Path("../badApple/original/")
    target_w = w
    target_h = h
    resolution_name = f"{target_w}x{target_h}"
    dst_dir = Path(f"../badApple/{resolution_name}/frames/")

    # Create output folder if needed
    dst_dir.mkdir(parents=True, exist_ok=True)

    # Collect jpg files in sorted order
    image_paths = sorted(src_dir.glob("*.jpg"))

    if not image_paths:
        print(f"No .jpg files found in {src_dir.resolve()}")
        raise SystemExit(1)

    print(f"Found {len(image_paths)} images.")
    print(f"Output directory: {dst_dir.resolve()}")

    for idx, img_path in enumerate(image_paths, start=1):
        img = cv2.imread(str(img_path), cv2.IMREAD_COLOR)
        if img is None:
            print(f"[WARN] Failed to read: {img_path}")
            continue

        resized = cv2.resize(img, (target_w, target_h), interpolation=cv2.INTER_AREA)

        out_path = dst_dir / img_path.name
        ok = cv2.imwrite(str(out_path), resized)
        if not ok:
            print(f"[WARN] Failed to write: {out_path}")
            continue

        if idx % 100 == 0 or idx == len(image_paths):
            print(f"Processed {idx}/{len(image_paths)}")

    print("Done.")

def write_to_txt():
    src_dir = Path("../badApple/96x72/frames")
    out_file = Path("../badApple/96x72/frames.txt")

    image_paths = sorted(src_dir.glob("*.jpg"))

    if not image_paths:
        print(f"No jpg files found in {src_dir.resolve()}")
        raise SystemExit(1)

    print(f"Found {len(image_paths)} frames")

    with out_file.open("w", encoding="utf-8") as f_out:
        for idx, img_path in enumerate(image_paths, start=1):
            img = cv2.imread(str(img_path), cv2.IMREAD_GRAYSCALE)
            if img is None:
                print(f"[WARN] Failed to read {img_path}")
                continue

            flat = img.flatten()
            line = " ".join(str(int(v)) for v in flat)
            f_out.write(line + "\n")

            if idx % 100 == 0 or idx == len(image_paths):
                print(f"Processed {idx}/{len(image_paths)}")

    print(f"Done. Wrote {out_file.resolve()}")

if __name__ == "__main__":
    #resize(72, 54)
    write_to_txt()