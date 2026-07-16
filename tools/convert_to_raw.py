#!/usr/bin/env python3
"""
Convert images (BMP/PNG/JPG) to the raw RGB565 format used by
RawLoader::rawDraw() / rawDrawDMA() / rawLoad() (see RawLoader.h/.cpp).

Output file format
------------------
  Bytes 0-1 : image width   (uint16_t, little-endian)
  Bytes 2-3 : image height  (uint16_t, little-endian)
  Bytes 4.. : width * height big-endian RGB565 pixels, top-to-bottom.

Big-endian storage = display-native order, so rawDraw()/rawDrawDMA() can push
the data directly without a byte-swap on the MCU.

Usage
-----
  # Convert a single file:
  python3 convert_to_raw.py photo.bmp photo.raw

  # Convert every BMP/PNG/JPG in a folder (output beside source files):
  python3 convert_to_raw.py images/

  # Convert folder, write output to a different folder (mirrors filenames,
  # swapping the extension to .raw):
  python3 convert_to_raw.py images/ sd_card/

  # Resize while converting (e.g. to exactly 80x80 icons):
  python3 convert_to_raw.py photo.bmp photo.raw --size 80 80

Requirements
------------
  pip install -r requirements.txt   (Pillow + numpy)

Performance note
-----------------
Pixel packing is vectorized with numpy instead of a per-pixel Python loop -
converting a few thousand images (e.g. a full SD card's worth of icons and
sprites) takes seconds rather than minutes.
"""

import argparse
import os
import struct
import sys

try:
    from PIL import Image
except ImportError:
    print("Pillow is not installed. Run: pip install -r requirements.txt")
    sys.exit(1)

try:
    import numpy as np
except ImportError:
    print("numpy is not installed. Run: pip install -r requirements.txt")
    sys.exit(1)

SUPPORTED_EXTS = {".bmp", ".png", ".jpg", ".jpeg"}


def to_raw_bytes(img: Image.Image) -> bytes:
    """Return the raw RGB565 payload (header + pixels) for *img*."""
    img = img.convert("RGB")
    w, h = img.size
    arr = np.asarray(img, dtype=np.uint16)  # shape (h, w, 3): R, G, B

    r = arr[:, :, 0]
    g = arr[:, :, 1]
    b = arr[:, :, 2]
    rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)  # (h, w) uint16

    header = struct.pack("<HH", w, h)
    pixels = rgb565.astype(">u2").tobytes()  # big-endian on the wire
    return header + pixels


def convert_file(src: str, dst: str, size=None) -> int:
    """Convert src to dst, returning the output file size in bytes."""
    img = Image.open(src)
    if size:
        img = img.resize(size, Image.LANCZOS)
    data = to_raw_bytes(img)
    with open(dst, "wb") as f:
        f.write(data)
    return len(data)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Convert images to raw RGB565 for RawLoader's rawDraw()/rawDrawDMA()"
    )
    parser.add_argument("source", help="Source image file or source folder")
    parser.add_argument(
        "dest",
        nargs="?",
        help="Output file (single conversion) or output folder (batch). "
             "Defaults to source location.",
    )
    parser.add_argument(
        "--size",
        nargs=2,
        type=int,
        metavar=("W", "H"),
        help="Resize image to W x H before converting",
    )
    args = parser.parse_args()

    resize = tuple(args.size) if args.size else None

    if os.path.isdir(args.source):
        out_dir = args.dest if args.dest else args.source
        os.makedirs(out_dir, exist_ok=True)
        converted = 0
        total_bytes = 0
        for fname in sorted(os.listdir(args.source)):
            ext = os.path.splitext(fname)[1].lower()
            if ext in SUPPORTED_EXTS:
                src = os.path.join(args.source, fname)
                dst = os.path.join(out_dir, os.path.splitext(fname)[0] + ".raw")
                size = convert_file(src, dst, size=resize)
                total_bytes += size
                converted += 1
        if converted == 0:
            print(f"No supported images found in '{args.source}'")
        else:
            print(f"Done - {converted} file(s) converted, {total_bytes / 1e6:.1f} MB total.")
    else:
        if not os.path.isfile(args.source):
            print(f"Error: '{args.source}' is not a file or directory.")
            sys.exit(1)
        dst = args.dest if args.dest else os.path.splitext(args.source)[0] + ".raw"
        size = convert_file(args.source, dst, size=resize)
        print(f"{args.source} -> {dst} ({size} bytes)")


if __name__ == "__main__":
    main()
