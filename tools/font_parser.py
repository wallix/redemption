#!/usr/bin/env python3
# Dominique Lafages, Jonathan Poelen
# Copyright WALLIX 2023

###############################################################################################
# script extracting a font definition and converting it to the RBF2 format used by ReDemPtion.
#
# HINTs:
# - Each RBF2 glyph is sketched in a bitmap whose dimensions are mutiples of 8. As PIL glyphes
#   width are not multiple of 8 they have to be padded. By convention, they are padded to left
#   and bottom.
# - The glyphs are not antialiased.
# - The police is variable sized
# - Thus, each pixel in a sketch is represented by only one bit
#
# FORMATs :
# - the RBF2 file always begins by the label "RBF2"
# - Police global informations are :
#     * version (u32)
#     * name (u8[32]) (ex : Deja Vu Sans)
#     * fontsize (u16)
#     * fontstyle (u16) (always '1')
#     * max ascent (u16)
#     * max descent (u16)
#     * number of glyph (u32)
#     * unicode max (u32)
#     * total data len: sum of aligned_of_4(glyph_data_len) (u32)
#     * replacement glyph (assume uni < CONTIGUOUS_LIMIT)
#     * glyph in range [CHARSET_START..CHARSET_END]
# - Individual glyph informations are :
#     ? when uni < CONTIGUOUS_LIMIT
#       * has_glyph (u8 = 1 or 0)
#     ? when has_glyph = 1 or when uni < CONTIGUOUS_LIMIT
#       ? when uni >= CONTIGUOUS_LIMIT
#         * unicode value (u32)
#       * offsetx (u8)
#       * offsety (u8)
#       * incby (u8)
#       * cx (s8)
#       * cy (s8)
#       * data (the bitmap representing the sketch of the glyph, one bit by pixel,
#               0 for background, 1 for foreground)
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
#     * '>' for a right space
###############################################################################################

from PIL.ImageFont import ImageFont, truetype
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
fallback_fontpath = ''
name = ''
output = ''

CHARSET_START = 32
# CHARSET_END = 0x3134b
CHARSET_END = 0x2fa1e
# CHARSET_START = 0x20e3
# CHARSET_END = CHARSET_START + 2

CONTIGUOUS_LIMIT = 0xD7FC


if len(sys.argv) > 1:
    import argparse
    parser = argparse.ArgumentParser(description='rfb2 font generator')
    parser.add_argument('-o', '--output', metavar='FILENAME', default='')
    parser.add_argument('-r', '--range', nargs=2, type=int, default=(CHARSET_START, CHARSET_END))
    parser.add_argument('-p', '--fontpath', type=str, default=fallback_fontpath)
    parser.add_argument('-s', '--fontsize', type=int, default=global_fontsize)
    parser.add_argument('-n', '--name', type=str, default=name)

    args = parser.parse_args()

    CHARSET_START = int(args.range[0])
    CHARSET_END = int(args.range[1])
    fallback_fontpath = args.fontpath
    global_fontsize = args.fontsize
    output = args.output
    name = args.name


def get_fontpath(filename, dirnames: Iterable[str]) -> str:
    for d in dirnames:
        path = f'{d}/{filename}'
        if os.path.exists(path):
            return path
    return f'{fallback_fontpath}/{filename}'


font_descriptions: Iterable[tuple[
    int,  # fontsize or 0 for global fontsize
    str,  # path of font
    Iterable[str]  # glyph for invalid char rendering
]] = (
    (global_fontsize, '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf', ('\u02ef', '\u20e3',)),

    # https://www.latofonts.com/lato-free-fonts/
    # (global_fontsize, get_fontpath('Lato-Light.ttf', (
    #     '/usr/share/fonts/truetype/lato/'  # ubuntu
    #     '/usr/share/fonts/TTF/',  # arch
    # )), ('\u0370',)),

    # https://github.com/notofonts/noto-cjk/raw/main/Sans/OTC/NotoSansCJK-Regular.ttc
    (global_fontsize - 2, get_fontpath('NotoSansCJK-Regular.ttc', (
        '/usr/share/fonts/opentype/noto/',  # ubuntu
        '/usr/share/fonts/noto-cjk/',  # arch
    )), ('\u0104', '\u0302')),
)

if not name:
    name = ','.join(os.path.splitext(os.path.basename(font_desc[1]))[0] for font_desc in font_descriptions)

if not output:
    output = f"{name}_{global_fontsize}.rbf2"


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


def mask_to_tuple(mask: PIL.Image.core) -> Pixels:
    bbox = mask.getbbox()
    yseq = range(bbox[1], bbox[3])
    xseq = range(bbox[0], bbox[2])
    return bytes(bytearray(mask.getpixel((ix, iy))
                           for iy in yseq
                           for ix in xseq))

def load_truetype(fontpath: str, fontsize: int, unknown_unicode_for_glyphs: tuple[str]) -> FontInfo:
    print(f'load {fontpath}')
    font = truetype(fontpath, fontsize or global_fontsize)
    return FontInfo(font,
                    (*((mask_to_tuple(mask := font.getmask(uni, mode='1')),
                        mask.getbbox(),
                        font.getbbox(uni, mode='1'))
                       for uni in unknown_unicode_for_glyphs),),
                    font.getmetrics()[0],
                    )

font_infos: list[FontInfo] = [
    load_truetype(fontpath, fontsize or global_fontsize, unknown_unicode_for_glyphs)
    for fontsize, fontpath, unknown_unicode_for_glyphs in font_descriptions
]

max_ascent = max(font_info.ascent for font_info in font_infos)
max_descent = max(font_info.font.getmetrics()[1] for font_info in font_infos)

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

replacement_uni = 0xFFFD
replacement_unicode_char = chr(replacement_uni)
replacement_char: GlyphInfo = (
    GlyphType.Replacement,
    mask_to_tuple(mask := font_infos[0].font.getmask(replacement_unicode_char, mode='1')),
    mask.getbbox(),
    (bbox_font := font_infos[0].font.getbbox(replacement_unicode_char, mode='1'))[0],
    bbox_font[1] + max_ascent - font_infos[0].ascent,
)

def get_glyph_info(char: str) -> GlyphInfo:
    for font_info in font_infos:
        mask = font_info.font.getmask(char, mode='1')
        bbox_font = font_info.font.getbbox(char, mode='1')
        x1, y1, x2, y2 = bbox_font
        bbox = mask.getbbox()
        # is None for spaces
        if not bbox:
            # rdesktop require non empty data for glyph
            # create a transparent image with height=1
            y = y2 - y1 if y2 - y1 else y2
            bbox = (x1, y - 1, x2, y)
            pixels = b'0' * (x2 * y2)
        else:
            pixels = mask_to_tuple(mask)
            if any(bbox == unknown_char[1] and pixels == unknown_char[0]
                   for unknown_char in font_info.unknown_chars):
                continue

        offsetx = x1
        offsety = y1 + max_ascent - font_info.ascent
        return GlyphType.Normal, pixels, bbox, offsetx, offsety
    return unknown_glyph

def valid_chr(char: str) -> bool:
    cat = category(char)
    general_cat = cat[0]
    return general_cat != 'C' and (general_cat != 'Z' or cat == 'Zs')

def serialize_glyph(x1: int, y1: int, cx: int, cy: int, incby: int, offsetx: int, pixels: bytes) -> tuple[bytes, str]:
    data = b''
    padding = count_bit_padding(cx)
    empty_line = '+' * incby + '\n'
    # padding_line = 'o' * padding
    left_empty_line = '+' * offsetx
    right_empty_line = '>' * (incby - cx) + '\n'
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
            data += struct.pack('<B', byte << padding)

        line += right_empty_line

    return data, line

glyph_graph_adjust_y = set((
    0x25b8,  # ▸
    0x25b9,  # ▹
    0x25ba,  # ►
    0x25bb,  # ▻
    0x25c2,  # ◂
    0x25c3,  # ◃
    0x25c4,  # ◄
    0x25c5,  # ◅
))

class Glyphs:
    def __init__(self):
        self.total_data_len = 0
        self.data_glyphs = []
        self.max_heigth = 0

    def add(self, uni: int, char: str, glyph_info: GlyphInfo, force_insert: bool = False) -> None:
        glyph_type, pixels, bbox, offsetx, offsety = glyph_info

        x1, y1, x2, y2 = bbox
        incby = x2 - offsetx
        cx = x2 - x1
        cy = y2 - y1
        offsetx = max(1, offsetx + x1)
        offsety = max(0, offsety + y1)

        self.max_heigth = max(offsety + cy, self.max_heigth)

        # because space is usually too big
        if uni == 114 and global_fontsize == 14:  # 'r'
            incby -= 1
         # align with '◀'/'◁'/'▶'/'▷'
        elif uni in glyph_graph_adjust_y and global_fontsize == 14:
            offsety += 1

        print(f'{uni:#x}  CHR: {char}  TYPE: {glyph_type}  CX/CY: {cx},{cy}  INCBY: {incby}  OFFSET: {offsetx},{offsety}  BBOX: {bbox}')

        if glyph_type == GlyphType.Normal or force_insert:
            data, line = serialize_glyph(x1, y1, cx, cy, incby, offsetx, pixels)

            if uni < CONTIGUOUS_LIMIT or force_insert:
                datainfo = struct.pack('<bbbBBB', 1, offsetx, offsety, incby, cx, cy)
            else:
                datainfo = struct.pack('<IbbBBB', uni, offsetx, offsety, incby, cx, cy)

            self.total_data_len += align4(nbbytes(cx) * cy)

            print(line, end='\n\n')
            self.data_glyphs.append(datainfo + data)

        elif uni < CONTIGUOUS_LIMIT:
            # replacement
            self.data_glyphs.append(b'\0')


ichar_gen = range(CHARSET_START, CHARSET_END)

glyphs = Glyphs()

glyphs.add(replacement_uni, 'ReplacementChar', replacement_char, True)

for uni in ichar_gen:
    char = chr(uni)
    # TODO space as special glyph ?
    is_printable = valid_chr(char)

    if is_printable:
        glyphs.add(uni, char, get_glyph_info(char))
    else:
        if uni < CONTIGUOUS_LIMIT:
            # replacement
            glyphs.data_glyphs.append(b'\0')

        print(f'{uni:#x}  CHR: NonPrintable')


print(f'Output file: {output}')

with open(output, 'wb') as f:
    # Magic number
    f.write("RBF2".encode())

    f.write(struct.pack('<I', 1))  # version
    f.write((name.encode() + b'\0' * 32)[0:32])
    f.write(struct.pack('<H', global_fontsize))
    f.write(struct.pack('<H', 1))  # style
    f.write(struct.pack('<H', max_ascent))
    f.write(struct.pack('<H', max_descent))
    f.write(struct.pack('<I', len(glyphs.data_glyphs)))
    f.write(struct.pack('<I', CHARSET_END))
    f.write(struct.pack('<I', glyphs.total_data_len))

    for data in glyphs.data_glyphs:
        f.write(data)
