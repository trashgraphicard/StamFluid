import cv2
from pathlib import Path

def write_to_bin(src_dir, out_file):
    
    img_paths = sorted(src_dir.glob("*.png"))
    if not img_paths:
        print(f"No png files found in {src_dir.resolve()}")
        raise SystemExit(1)
    
    print(f"Found {len(img_paths)} frames")

    with out_file.open("wb") as f_out:
        for idx, img_path in enumerate(img_paths, start=1):
            img = cv2.imread(str(img_path), cv2.IMREAD_GRAYSCALE)
            if img is None:
                print(f"Failed to read {img_path}")
            flat = img.flatten().astype("uint8")
            f_out.write(flat.tobytes())

            if idx % 100 == 0 or idx == len(img_paths):
                print(f"Processed {idx}/{len(img_paths)}")
    
    print(f"Done. Wrote {out_file.resolve()}")

def main():
    src_dir = Path("../brush/thumb/128x128/v/")
    out_file = Path("../brush/thumb/128x128/v.bin")
    write_to_bin(src_dir, out_file)

if __name__ == "__main__":
    main()