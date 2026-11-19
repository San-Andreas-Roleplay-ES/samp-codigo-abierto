"""Build the SCBA dial, needle and status sprites in the mask's existing RenderWare dictionary."""

import argparse
import math
import re
import struct
import zlib
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
CONFIG = ROOT / "gamemodes/features/scba/config.inc"
MASK = ROOT / "models/scba/mask.txd"
AA = 4
SCALE_MARKS = 40


def gauge_settings():
    source = CONFIG.read_text(encoding="cp1252")
    return {
        key: float(value)
        for key, value in re.findall(r"^#define SCBA_GAUGE_(\w+) \(([\d.]+)\)$", source, re.MULTILINE)
    }


def point(cx, cy, radius, angle):
    radians = math.radians(angle)
    return cx + radius * math.cos(radians), cy + radius * math.sin(radians)


class Canvas:
    def __init__(self, size):
        self.image = Image.new("RGBA", (size * AA, size * AA))
        self.pen = ImageDraw.Draw(self.image)

    def polygon(self, points, colour):
        self.pen.polygon([(round(x * AA), round(y * AA)) for x, y in points], fill=colour)

    def line(self, points, colour, width=1):
        self.pen.line(
            [(round(x * AA), round(y * AA)) for x, y in points],
            fill=colour,
            width=max(1, round(width * AA)),
            joint="curve",
        )

    def circle(self, x, y, radius, colour):
        self.pen.ellipse(
            tuple(round(p * AA) for p in (x - radius, y - radius, x + radius, y + radius)), fill=colour
        )

    def plate(self, left, top, right, bottom, corner, colour):
        self.polygon(
            [
                (left + corner, top),
                (right - corner, top),
                (right, top + corner),
                (right, bottom - corner),
                (right - corner, bottom),
                (left + corner, bottom),
                (left, bottom - corner),
                (left, top + corner),
            ],
            colour,
        )

    def finish(self, size):
        return self.image.resize((size, size), Image.Resampling.LANCZOS)


def numeral(canvas, x, y, number):
    strokes = {
        "0": [(1, 0), (4, 0), (5, 1), (5, 7), (4, 8), (1, 8), (0, 7), (0, 1), (1, 0)],
        "1": [(0, 2), (2, 0), (2, 8)],
        "5": [(5, 0), (0, 0), (0, 4), (4, 4), (5, 5), (5, 7), (4, 8), (0, 8)],
    }
    start = x - (len(number) * 7 - 2) / 2
    for digit in number:
        canvas.line([(start + a, y - 4 + b) for a, b in strokes[digit]], "#d6d2bd", 1.65)
        start += 7


def face(settings):
    size = int(settings["ART_SIZE"])
    c = Canvas(size)
    x, y = settings["DIAL_X"], settings["DIAL_Y"]
    # One radar-style rim and two compact readouts leave the corners transparent.
    c.plate(27, 170, 229, 217, 7, (9, 9, 8, 225))
    c.line([(128, 177), (128, 209)], "#56544a", 1)
    left, top = settings["STATUS_LEFT"], settings["STATUS_TOP"]
    c.plate(left, top, left + settings["STATUS_WIDTH"], top + settings["STATUS_HEIGHT"], 4, (11, 11, 10, 208))

    def bezel(radius, colour):
        c.polygon([point(x, y, radius, i * 12 + 6) for i in range(30)], colour)

    bezel(84, "#080807")
    bezel(79, "#bcb9a6")
    bezel(76.8, "#141513")
    bezel(73.5, "#34392e")
    c.polygon([point(x, y, 73.3, i * 12 + 6) for i in range(15)] + [(x, y)], "#30342c")
    start, sweep = settings["START_ANGLE"], settings["SWEEP_ANGLE"]
    angles = [start + sweep * i / 100 for i in range(101)]
    c.line([point(x, y, 66, a) for a in angles], "#8e9484", 1)
    for i in range(SCALE_MARKS + 1):
        a = start + sweep * i / SCALE_MARKS
        length = 9.5 if i % 10 == 0 else 6 if i % 5 == 0 else 3
        c.line([point(x, y, 65, a), point(x, y, 65 - length, a)], "#ebe8d8", 2.6 if i % 10 == 0 else 1.35)
    for fraction, label in ((0, "0"), (0.5, "50"), (1, "100")):
        px, py = point(x, y, 45, start + sweep * fraction)
        numeral(c, px, py, label)
    return c.finish(size)


def needle(settings, frame):
    size = 128
    c = Canvas(size)
    angle = settings["START_ANGLE"] + settings["SWEEP_ANGLE"] * frame / settings["NEEDLE_STEPS"]
    cx = cy = size / 2
    length = 61 * size / settings["DIAL_SIZE"]
    tip = point(cx, cy, length, angle)
    left = point(cx, cy, 3.3, angle - 90)
    right = point(cx, cy, 3.3, angle + 90)
    tail = point(cx, cy, 10, angle + 180)
    c.polygon([(x + 1.1, y + 1.5) for x, y in [tail, left, tip, right]], (20, 26, 22, 100))
    c.polygon([tail, left, tip, right], "#a16529")
    c.polygon([left, tip, (cx, cy)], "#f3cb72")
    c.polygon(
        [point(cx, cy, length * 0.72, angle - 2), tip, point(cx, cy, length * 0.72, angle + 2)], "#fff0bc"
    )
    c.circle(cx, cy, 5.1, "#100f0b")
    c.circle(cx, cy, 3.6, "#cfad62")
    c.circle(cx - 0.5, cy - 0.5, 1.3, "#f2e5bc")
    return c.finish(size)


def status_caption(caption, settings):
    # Centre the visible capitals, independently of a client's font baseline and aspect ratio.
    width, height = 256, 32
    font = ImageFont.load_default(size=64)
    left, top, right, bottom = font.getbbox(caption, stroke_width=2)
    ink = Image.new("L", (right - left, bottom - top))
    ImageDraw.Draw(ink).text((-left, -top), caption, font=font, fill=255, stroke_width=2, stroke_fill=255)
    ink = ink.crop(ink.getbbox())
    glyph_height = 18
    aspect = (width / settings["STATUS_WIDTH"]) / (height / settings["STATUS_HEIGHT"])
    glyph_width = min(width - 16, round(ink.width / ink.height * glyph_height * aspect * 0.86))
    ink = ink.resize((glyph_width, glyph_height), Image.Resampling.LANCZOS)
    alpha = Image.new("L", (width, height))
    alpha.paste(ink, ((width - glyph_width) // 2, (height - glyph_height) // 2))
    sprite = Image.new("RGBA", (width, height), "white")
    sprite.putalpha(alpha)
    return sprite


def chunk(kind, data, version):
    return struct.pack("<III", kind, len(data), version) + data


def chunks(blob):
    offset = 0
    while offset < len(blob):
        kind, size, version = struct.unpack_from("<III", blob, offset)
        end = offset + 12 + size
        if end > len(blob):
            raise ValueError("Invalid RenderWare chunk size")
        yield kind, blob[offset + 12 : end], version
        offset = end


def argb4444(image):
    # Preserve the alpha edge of the pointer on the dark dial without block compression.
    nibbles = image.tobytes().translate(bytes((value + 8) // 17 for value in range(256)))
    pixels = bytearray(len(nibbles) // 2)
    for index, (red, green, blue, alpha) in enumerate(struct.iter_unpack("4B", nibbles)):
        pixels[2 * index] = (green << 4) | blue
        pixels[2 * index + 1] = (alpha << 4) | red
    return pixels


def native_texture(name, image, version, compact=True):
    width, height = image.size
    mips = []
    while True:
        mips.append(argb4444(image) if compact else image.tobytes("raw", "BGRA"))
        if image.size == (1, 1):
            break
        image = image.resize((max(1, image.width // 2), max(1, image.height // 2)), Image.Resampling.LANCZOS)
    raster_type, format_id, depth = (0x8300, 26, 16) if compact else (0x8500, 21, 32)
    header = struct.pack(
        "<IBBH32s32sIIHHBBBB",
        9,
        6,
        0x33,
        0,
        name.encode("ascii"),
        b"",
        raster_type,
        format_id,
        width,
        height,
        depth,
        len(mips),
        4,
        1,  # D3D9 alpha channel
    )
    raster = header + b"".join(struct.pack("<I", len(mip)) + mip for mip in mips)
    return chunk(21, chunk(1, raster, version) + chunk(3, b"", version), version)


def build(dictionary):
    ((kind, body, version),) = chunks(dictionary)
    if kind != 22:
        raise ValueError("Expected a RenderWare texture dictionary")
    entries = list(chunks(body))
    header, extension = entries[0], entries[-1]
    if header[0] != 1 or extension[0] != 3:
        raise ValueError("Unexpected dictionary layout")
    count, device = struct.unpack("<HH", header[1])
    if count != len(entries) - 2:
        raise ValueError("Texture count mismatch")
    textures = []
    for kind, data, ver in entries[1:-1]:
        if kind != 21:
            raise ValueError("Expected a native texture")
        raster = next(chunks(data))[1]
        name = raster[8:40].split(b"\0", 1)[0]
        if name != b"air_dial" and not re.fullmatch(rb"air_n\d\d|air_s_\w+", name):
            textures.append(chunk(kind, data, ver))
    settings = gauge_settings()
    textures.append(native_texture("air_dial", face(settings), version, compact=False))
    for i in range(1 + int(settings["NEEDLE_STEPS"])):
        textures.append(native_texture(f"air_n{i:02}", needle(settings, i), version))
    captions = re.findall(r'\{"(air_s_\w+)", "([^"]+)"', CONFIG.read_text(encoding="cp1252"))
    for name, caption in captions:
        textures.append(native_texture(name, status_caption(caption, settings), version))
    return chunk(
        22,
        chunk(1, struct.pack("<HH", len(textures), device), version)
        + b"".join(textures)
        + chunk(*extension),
        version,
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check", action="store_true", help="verify the shipped artwork without writing it"
    )
    args = parser.parse_args()
    original = MASK.read_bytes()
    updated = build(original)
    if args.check and updated != original:
        raise SystemExit("SCBA gauge textures need rebuilding")
    if not args.check and updated != original:
        candidate = MASK.with_suffix(".txd.tmp")
        candidate.write_bytes(updated)
        candidate.replace(MASK)
    print(f"Mask dictionary CRC 0x{zlib.crc32(updated):08X}")
