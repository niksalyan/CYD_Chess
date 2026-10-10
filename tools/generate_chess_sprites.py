#!/usr/bin/env python3
"""Generate a C++ sprite atlas header from the chess piece sprite sheet.

Usage:
    python3 tools/generate_chess_sprites.py

Expected input:
- root/Chess_Pieces_Sprite.png
- 180x60 PNG with 6 columns x 2 rows

Each tile is assumed to be 30x30 pixels, ordered as:
    Row 0: K, Q, B, R, N, P
    Row 1: k, q, b, r, n, p

Output:
- root/ChessPieceAtlas.h
  with a generated 16-bit RGB565 sprite atlas in PROGMEM-friendly C++ syntax.

This is designed for the ESP32 + ILI9341 setup used by CYD_Chess.
"""

from pathlib import Path

try:
    from PIL import Image
except ImportError as exc:
    raise SystemExit(
        "Pillow is required. Install it with: pip install pillow"
    ) from exc

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "Chess_Pieces_Sprite.png"
OUT = ROOT / "ChessPieceAtlas.h"

TILE_W = 30
TILE_H = 30
COLUMNS = 6
ROWS = 2
TRANSPARENT = 0xF81F  # Magenta is used as a transparent marker in the generated data.


def rgb565(r: int, g: int, b: int) -> int:
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def convert_image_to_atlas() -> list[list[int]]:
    if not SRC.exists():
        raise FileNotFoundError(f"Sprite sheet not found: {SRC}")

    image = Image.open(SRC).convert("RGBA")
    width, height = image.size
    if width != COLUMNS * TILE_W or height != ROWS * TILE_H:
        raise ValueError(
            f"Sprite sheet must be {COLUMNS * TILE_W}x{ROWS * TILE_H}, "
            f"found {width}x{height}"
        )

    atlas = []
    for row in range(ROWS):
        for col in range(COLUMNS):
            tile = Image.new("RGBA", (TILE_W, TILE_H), (0, 0, 0, 0))
            crop = image.crop((col * TILE_W, row * TILE_H, (col + 1) * TILE_W, (row + 1) * TILE_H))
            tile.paste(crop, (0, 0))
            pixels = []
            for y in range(TILE_H):
                for x in range(TILE_W):
                    r, g, b, a = tile.getpixel((x, y))
                    if a < 128:
                        pixels.append(TRANSPARENT)
                    else:
                        pixels.append(rgb565(r, g, b))
            atlas.append(pixels)
    return atlas


def format_sprite_array(name: str, values: list[int], per_line: int = 12) -> str:
    lines = [f"const uint16_t {name}[{len(values)}] PROGMEM = {{"]
    for i in range(0, len(values), per_line):
        chunk = values[i:i + per_line]
        rendered = ", ".join(f"0x{v:04X}" for v in chunk)
        lines.append("  " + rendered + ",")
    lines.append("};")
    return "\n".join(lines)


def write_header(atlas: list[list[int]]) -> None:
    output = [
        "#ifndef CHESS_PIECE_ATLAS_H",
        "#define CHESS_PIECE_ATLAS_H",
        "",
        "#include <Arduino.h>",
        "",
        "#define CHESS_SPRITE_ATLAS_AVAILABLE 1",
        "#define CHESS_PIECE_ATLAS_TILE_SIZE 30",
        "#define CHESS_PIECE_ATLAS_COLUMNS 6",
        "#define CHESS_PIECE_ATLAS_ROWS 2",
        "#define CHESS_PIECE_ATLAS_COUNT 12",
        "",
        "// Auto-generated from Chess_Pieces_Sprite.png",
        "// Layout: row 0 = White: K Q B R N P",
        "//         row 1 = Black: k q b r n p",
        "",
    ]

    # Generate individual sprite arrays
    sprite_names = []
    for idx, tile in enumerate(atlas):
        name = f"chessPieceAtlas_{idx}"
        sprite_names.append(name)
        output.append(format_sprite_array(name, tile, per_line=12))
        output.append("")

    # Generate the main atlas array that references all sprites
    output.append("const uint16_t* const chessPieceAtlas[CHESS_PIECE_ATLAS_COUNT] PROGMEM = {")
    for name in sprite_names:
        output.append(f"  {name},")
    output.append("};")
    output.append("")
    output.append("#endif")

    OUT.write_text("\n".join(output) + "\n", encoding="utf-8")


def main() -> None:
    atlas = convert_image_to_atlas()
    write_header(atlas)
    print(f"Generated {OUT}")


if __name__ == "__main__":
    main()
