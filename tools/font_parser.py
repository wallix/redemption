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

import os
import sys
import struct

BBox = tuple[int, int, int, int]
Mask = Any


global_fontsize = 14
font_descriptions: Iterable[tuple[int, str, Iterable[str]]] = (
    (0, "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", ('\u20e3', '\u4e2d',)),
    # (0, "/usr/share/fonts/truetype/lato/Lato-Light.ttf", ('0x20e3', '0x4e2d',)),
    (0, "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc", ('\u0104', '\u0302', '\U0003134a')),
    # (0, "/usr/share/fonts/truetype/noto/NotoSansThai-Regular.ttf", ('\u20e3', '\u4e2d',)),
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
    parser.add_argument('fonts_and_sizes', nargs='*', help='[fontsize [font [size]]...]')

    args = parser.parse_args()

    CHARSET_START = int(args.range[0])
    CHARSET_END = int(args.range[1])
    output = args.output

    if args.fonts_and_sizes:
        i = 0
        try:
            global_fontsize = int(args.fonts_and_sizes[0])
            i += 1
        except ValueError:
            pass

        new_font_descriptions = []
        end = len(args.fonts_and_sizes)
        while i < end:
            fontpath = args.fonts_and_sizes[i]
            i += 1
            fontsize = 0
            if i < end:
                try:
                    fontsize = int(args.fonts_and_sizes[i])
                    i += 1
                except ValueError:
                    pass
            new_font_descriptions.append((fontsize, fontpath))

        if new_font_descriptions:
            font_descriptions = new_font_descriptions


def nbbytes(x: int) -> int:
    return (x + 7) // 8

def align4(x: int) -> int:
    return (x+3) & ~3

def count_bit_padding(cx: int) -> int:
    return (8 - cx % 8) % 8


class FontInfo(NamedTuple):
    font: ImageFont
    unknown_chars: Iterable[tuple[Mask, BBox]]
    ascent: int


class GlyphInfo(NamedTuple):
    font_info: FontInfo
    mask: Mask
    bbox: BBox


font_infos: list[FontInfo] = [
    FontInfo(font := load_truetype(fontpath, fontsize or global_fontsize),
             (*((mask := font.getmask(uni, mode='1'), mask.getbbox())
                for uni in unknown_unicode_for_glyphs),),
             font.getmetrics()[0],
             )
    for fontsize, fontpath, unknown_unicode_for_glyphs in font_descriptions
]

unknown_chars_by_font = (*(
    (*(GlyphInfo(font_info, mask, bbox)
       for mask, bbox in font_info.unknown_chars),)
    for font_info in font_infos
),)
replacement_char = GlyphInfo(font_infos[0], mask := font_infos[0].font.getmask('\uFFFD', mode='1'), mask.getbbox())

def find_unknown_char(mask: Mask, bbox: BBox, font_info: FontInfo) -> None | GlyphInfo:
    for i, (unknown_char, bbox2) in enumerate(font_info.unknown_chars):
        if bbox == bbox2 and all(mask.getpixel((ix, iy)) == unknown_char.getpixel((ix, iy))
                                 for iy in range(bbox[1], bbox[3])
                                 for ix in range(bbox[0], bbox[2])):
            return unknown_chars_by_font[0][i]
    return None

def get_font_mask(char: str) -> GlyphInfo:
    unknown_char = None
    for font_info in font_infos:
        mask = font_info.font.getmask(char, mode='1')
        bbox = mask.getbbox()
        # is None for spaces
        if not bbox:
            bbox = font_info.font.getbbox(char)
            bbox = (bbox[0], 0, bbox[2], bbox[3] - bbox[1])
        elif unknown_char := find_unknown_char(mask, bbox, font_info):
            continue
        return GlyphInfo(font_info, mask, bbox)
    return unknown_char

def valid_chr(char: str) -> bool:
    cat = category(char)
    general_cat = cat[0]
    return general_cat != 'C' and (general_cat != 'Z' or cat == 'Zs')


ichar_gen = range(CHARSET_START, CHARSET_END)

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

    max_ascent = max(font_info.ascent for font_info in font_infos)
    max_height = max_ascent + max(font_info.font.getmetrics()[1] for font_info in font_infos)

    total_data_len = 0
    glyph_infos: list[GlyphInfo] = []
    for i in ichar_gen:
        char = chr(i)
        is_printable = valid_chr(char)
        # if not is_unknown_char:
        #     glyph_infos.append(replacement_char)
        #     continue

        font_info, mask, bbox = get_font_mask(char) if is_printable else replacement_char

        x1, y1, x2, y2 = bbox
        cx = x2 - x1
        cy = y2 - y1
        total_data_len += align4(nbbytes(cx) * cy)

        glyph_infos.append((font_info, mask, bbox))

    f.write(struct.pack('<H', global_fontsize))
    f.write(struct.pack('<h', 1))
    f.write(struct.pack('<H', max_height))
    f.write(struct.pack('<I', len(ichar_gen)))
    f.write(struct.pack('<I', total_data_len))

    for i, glyph in zip(ichar_gen, glyph_infos):
        char = chr(i)
        font_info, mask, bbox = glyph
        font = font_info.font
        bbox_char = font.getbbox(char)

        x1, y1, x2, y2 = bbox_char
        offsetx = x1
        offsety = y1 + max_ascent - font_info.ascent

        # for display only
        w = x2 - x1
        h = y2 - y1

        x1, y1, x2, y2 = bbox
        offsetx += x1
        offsety += y1
        cx = x2 - x1
        cy = y2 - y1
        # incby = max(0, offsetx) + cx
        incby = x2

        is_printable = valid_chr(char)
        if not is_printable:
            char = 'NonPrintable'

        print(f"{i:#x}  CHR: {char}  SIZE: {w},{h}  CX/CY: {cx},{cy}  INCBY: {incby}  OFFSET: {offsetx},{offsety}  IMGBBOX: {bbox_char}  BBOX: {bbox}")

        f.write(struct.pack('<I', i))
        f.write(struct.pack('<h', offsetx))
        f.write(struct.pack('<h', offsety))
        f.write(struct.pack('<h', 0))
        f.write(struct.pack('<h', 0))
        f.write(struct.pack('<h', incby))
        f.write(struct.pack('<H', cx))
        f.write(struct.pack('<H', cy))

        # TODO new format (save 1/3: 6.8 -> 4.3)
        # f.write(struct.pack('<b', offsetx))
        # f.write(struct.pack('<b', offsety))
        # f.write(struct.pack('<B', incby))
        # f.write(struct.pack('<B', cx))
        # f.write(struct.pack('<B', cy))

        # if not is_printable:
        #     continue

        padding = count_bit_padding(cx)
        empty_line = '+' * (w + padding) + '\n'
        padding_line = 'o' * padding
        left_empty_line = '+' * x1
        right_empty_line = '+' * (w - x1 - cx)
        line = empty_line * y1
        for iy in range(y1, y1 + cy):
            line += left_empty_line
            byte = 0
            counter = 0

            for ix in range(x1, x1+cx):
                pix = mask.getpixel((ix, iy))
                byte <<= 1
                if pix == 255:
                    line += '#'
                    byte |= 1
                else:
                    line += '.'

                counter += 1
                if counter == 8:
                    f.write(struct.pack('<B', byte))
                    counter = 0
                    byte = 0

            if counter != 0:
                line += padding_line
                f.write(struct.pack('<B', byte << padding))

            line += right_empty_line + '\n'

        f.write(b'\0' * (align4(nbbytes(cx) * cy) - nbbytes(cx) * cy))

        print(line + empty_line * (h - y2) + '\n')

print(f'Output file: {output}')
