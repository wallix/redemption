#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Dominique Lafages, Jonathan Poelen
# Copyright WALLIX 2018

###############################################################################################
# script extracting a font from a truetype opensource definition and converting it to the RBF1
# format used by ReDemPtion.
#
# HINTs:
# - Each RBF1 glyph is sketched in a bitmap whose dimensions are mutiples of 8. As PIL glyphes
#   width are not multiple of 8 they have to be padded. By convention, they are padded to left
#   and bottom.
# - The glyphs are not antialiased.
# - The police is variable sized
# - Thus, each pixel in a sketch is represented by only one bit
#
# FORMATs :
# - the RBF1 file always begins by the label "RBF1"
# - Police global informations are :
#     * name (32 bytes) (ex : Deja Vu Sans)
#     * size (2 bytes)
#     * style (2 bytes) (always '1')
#     * max height (4 bytes)
#     * number of glyph (4 bytes)
#     * total data len (4 bytes)
# - Individual glyph informations are :
#     * value (4 bytes)
#     * offsetx (2 bytes)
#     * offsety (2 bytes)
#     * abcA (left space, 2 bytes)
#     * abcB (glyph width, 2 bytes)
#     * abcC (right space, 2 bytes)
#     * cx (2 bytes)
#     * cy (2 bytes)
#     * data (the bitmap representing the sketch of the glyph, one bit by pixel, 0 for
#       background, 1 for foreground) (aligned of 4 bytes)
#
# TECHs :
# - struct.pack formats are :
#     * '<' little endian
#     * 'h' [short] for a two bytes emision
#     * 'B' [unsigned char] for a one byte emision
#     * 'L' [unsigned int] for a four bytes emision
# - the data generation loop print each glyph sketch to sdtout, with each bit represented as
#   follow :
#     * '.' for a PIL background bit
#     * '#' for a PIL foreground bit
#     * 'o' for an horizontal end of line paddind bit
#     * '+' for a vertical paddind line of bits
###############################################################################################

from PIL.ImageFont import ImageFont, truetype as load_truetype
from unicodedata import category
from typing import Any, Iterable, NamedTuple
from enum import IntEnum
from collections.abc import Collection

import os
import sys
import struct
import PIL


BBox = tuple[int, int, int, int]  # x1, y1, x2, y2
Pixels = bytes


global_fontsize = 14
font_descriptions: Iterable[tuple[
    int,  # fontsize or 0 for global fontsize
    str,  # path of font
    Iterable[str]  # glyph for invalid char rendering
]] = (
    (global_fontsize, "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", ('\u02ef', '\u20e3',)),
    # (global_fontsize, "/usr/share/fonts/truetype/lato/Lato-Light.ttf", ('\u0104', '\u0302',)),
    (global_fontsize, "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc", ('\u0104', '\u0302')),
    # (global_fontsize, "/usr/share/fonts/truetype/noto/NotoSansThai-Regular.ttf", ('\u0104', '\u0302',)),
)
output = f"{os.path.splitext(os.path.basename(font_descriptions[0][1]))[0]}_{global_fontsize}.rbf"

CHARSET_START = 32
CHARSET_END = 0x3134b
# CHARSET_START = 0x20e3
# CHARSET_END = CHARSET_START + 2

if len(sys.argv) > 1:
    import argparse
    parser = argparse.ArgumentParser(description='rfb2 font generator')
    parser.add_argument('-o', '--output', metavar='FILENAME', default=output)
    parser.add_argument('-r', '--range', nargs=2, type=int, default=(CHARSET_START, CHARSET_END))

    args = parser.parse_args()

    CHARSET_START = int(args.range[0])
    CHARSET_END = int(args.range[1])
    output = args.output


def nbbytes(x: int) -> int:
    return (x + 7) // 8

def align4(x: int) -> int:
    return (x+3) & ~3

def count_bit_padding(cx: int) -> int:
    return (8 - cx % 8) % 8


class FontInfo(NamedTuple):
    font: ImageFont
    unknown_chars: Iterable[tuple[Pixels, BBox, BBox]]
    ascent: int


def mask_to_tuple(mask: PIL.Image.core, bbox: None | BBox = None) -> Pixels:
    if bbox is None:
        bbox = mask.getbbox()
    yseq = range(bbox[1], bbox[3])
    return bytes(bytearray(mask.getpixel((ix, iy))
                           for iy in yseq
                           for ix in range(bbox[0], bbox[2])))


font_infos: list[FontInfo] = [
    FontInfo(font := load_truetype(fontpath, fontsize or global_fontsize),
             (*((mask_to_tuple(mask := font.getmask(uni, mode='1')), mask.getbbox(), font.getbbox(uni))
                for uni in unknown_unicode_for_glyphs),),
             font.getmetrics()[0],
             )
    for fontsize, fontpath, unknown_unicode_for_glyphs in font_descriptions
]

max_ascent = max(font_info.ascent for font_info in font_infos)
max_height = max_ascent + max(font_info.font.getmetrics()[1] for font_info in font_infos)

class GlyphType(IntEnum):
    Normal = 0
    Replacement = 1
    Unknown = 2

GlyphInfo = tuple[GlyphType, Pixels, BBox, int, int]  # int, int = offsetx, offsety

unknown_char = font_infos[0].unknown_chars[0]
unknown_glyph = (GlyphType.Unknown,
                 unknown_char[0],
                 unknown_char[1],
                 unknown_char[2][0],
                 unknown_char[2][1] + max_ascent - font_infos[0].ascent
                 )

replacement_unicode_char = '\uFFFD'
replacement_char: GlyphInfo = (GlyphType.Replacement,
                               mask_to_tuple(mask := font_infos[0].font.getmask(replacement_unicode_char, mode='1')),
                               mask.getbbox(),
                               (bbox_font := font_infos[0].font.getbbox(replacement_unicode_char))[0],
                               bbox_font[1] + max_ascent - font_infos[0].ascent,
                               )

def find_unknown_char(pixels: Pixels, bbox: BBox, unknown_chars: Iterable[tuple[Pixels, BBox, BBox]]) -> GlyphInfo:
    for i, unknown_char in enumerate(unknown_chars):
        if bbox == unknown_char[1] and pixels == unknown_char[0]:
            return unknown_glyph
    return None

def get_glyph_info(char: str) -> GlyphInfo:
    unknown_char = replacement_char
    for font_info in font_infos:
        mask = font_info.font.getmask(char, mode='1')
        bbox_font = font_info.font.getbbox(char)
        x1, y1, x2, y2 = bbox_font
        bbox = mask.getbbox()
        # is None for spaces
        if not bbox:
            bbox = (x1, 0, x2, y2 - y1)
            pixels = mask_to_tuple(mask, bbox)
        else:
            pixels = mask_to_tuple(mask)
            unknown_char = find_unknown_char(pixels, bbox, font_info.unknown_chars)
            if unknown_char:
                continue

        offsetx = x1
        offsety = y1 + max_ascent - font_info.ascent
        return GlyphType.Normal, pixels, bbox, offsetx, offsety
    return unknown_char

def valid_chr(char: str) -> bool:
    cat = category(char)
    general_cat = cat[0]
    return general_cat != 'C' and (general_cat != 'Z' or cat == 'Zs')


ichar_gen = range(CHARSET_START, CHARSET_END)

total_data_len = 0
data_glyphs = []
cache = {}

for i in ichar_gen:
    char = chr(i)
    is_printable = valid_chr(char)

    glyph_type, pixels, bbox, offsetx, offsety = get_glyph_info(char) if is_printable else replacement_char

    x1, y1, x2, y2 = bbox
    offsetx += x1
    offsety += y1
    cx = x2 - x1
    cy = y2 - y1
    # incby = max(0, offsetx) + cx
    incby = x2

    total_data_len += align4(nbbytes(cx) * cy)

    if not is_printable:
        char = 'NonPrintable'

    print(f"{i:#x}  CHR: {char}  TYPE: {glyph_type}  CX/CY: {cx},{cy}  INCBY: {incby}  OFFSET: {offsetx},{offsety}  BBOX: {bbox}")

    cache_id = glyph_type if glyph_type else (cx, cy, incby, offsetx, offsety, pixels)
    cache_elem = cache.get(cache_id)

    if not cache_elem:
        data = b''
        padding = count_bit_padding(cx)
        empty_line = '+' * incby + '\n'
        # padding_line = 'o' * padding
        left_empty_line = '+' * offsetx
        right_empty_line = '>' * (incby - cy) + '\n'
        line = empty_line * y1
        for iy in range(y1, y1 + cy):
            line += left_empty_line
            byte = 0
            counter = 0

            for ix in range(x1, x1 + cx):
                pix = pixels[(iy - y1) * cx + (ix - x1)]
                byte <<= 1
                if pix == 255:
                    line += '#'
                    byte |= 1
                else:
                    line += '.'

                counter += 1
                if counter == 8:
                    data += struct.pack('<B', byte)
                    counter = 0
                    byte = 0

            if counter != 0:
                # line += padding_line
                data += struct.pack('<B', byte << padding)

            line += right_empty_line

        data += b'\0' * (align4(nbbytes(cx) * cy) - nbbytes(cx) * cy)
        datainfo = struct.pack('<IhhhhhHH', i, offsetx, offsety, 0, 0, incby, cx, cy)
        # TODO new format (save 1/3: 6.8M -> 4.3M)
        # datainfo = struct.pack('<bbBBB', offsetx, offsety, incby, cx, cy)

        cache_elem = (datainfo + data, line + '\n')
        cache[cache_id] = cache_elem

    print(cache_elem[1])
    data_glyphs.append(cache_elem[0])


print(f'Output file: {output}')

with open(output, 'wb') as f:
    # Magic number
    f.write("RBF1".encode())

    # Name of font
    name = font_infos[0].font.getname()[0].encode()
    if len(name) > 32:
        name = name[0:32]
    f.write(name)
    f.write(b'\0' * (max(0, 32 - len(name))))

    f.write(struct.pack('<H', global_fontsize))
    f.write(struct.pack('<H', 1))
    f.write(struct.pack('<H', max_height))
    f.write(struct.pack('<I', len(ichar_gen)))
    f.write(struct.pack('<I', total_data_len))

    for data in data_glyphs:
        f.write(data)
