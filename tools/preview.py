"""Convert the host renderer's PPM snapshots to PNG using the standard library."""
from pathlib import Path
import struct
import zlib


def chunk(kind, data):
    return (struct.pack(">I", len(data)) + kind + data +
            struct.pack(">I", zlib.crc32(kind + data) & 0xffffffff))


for path in (Path(__file__).resolve().parent.parent / "build/native").glob("*.ppm"):
    magic, dimensions, maximum, pixels = path.read_bytes().split(b"\n", 3)
    width, height = map(int, dimensions.split())
    assert magic == b"P6" and maximum == b"255"
    assert len(pixels) == width * height * 3
    stride = width * 3
    scanlines = b"".join(b"\0" + pixels[y * stride:(y + 1) * stride]
                         for y in range(height))
    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(scanlines))
    png += chunk(b"IEND", b"")
    path.with_suffix(".png").write_bytes(png)

