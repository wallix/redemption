#!env python3
# -*- coding: utf-8 -*-
# Dominique Lafages, Jonathan Poelen
# Copyright WALLIX 2018

###############################################################################
# script extracting a font from a truetype opensource definition and
# converting it to the FV2 format used by ReDemPtion.
#
# HINTs:
# - Each FV1 glyph is sketched in a bitmap whose dimensions are mutiples of 8.
# As PIL glyphes width are not multiple of 8 they have to be padded. By
# convention, they are padded to left and bottom.
# - The glyphs are not antialiased.
# - The police is variable sized
# - Thus, each pixel in a sketch is represented by only one bit
#
# FORMATs :
# - the FV1 file always begins by the label "FNT1"
# - Police global informations are :
#     * len name
#     * name (ex : Deja Vu Sans)
#     * size (ex : 10)
#     * style (always '1')
# - Individual glyph informations are :
#     * offsetx (based on PIL ABC.abcA - this is the space before the character)
#     * offsety
#     * width (based on PIL ABC.abcB - this is the real width of character)
#     * right space (based on PIL ABC.abcC - this is the space after the character)
#     * cx (a round multiple of 8)
#     * cy
#     * data (the bitmap representing the sketch of the glyph, one bit by
#       pixel, 0 for background, 1 for foreground)
#
# TECHs :
# - struct.pack formats are :
#     * 'h' [short] for a two bytes emision
#     * 'B' [unsigned char] for a one byte emision
# - the data generation loop print each glyph sketch to sdtout, with each bit
# represented as follow :
#     * '.' for a PIL background bit
#     * '#' for a PIL foreground bit
#     * 'o' for an horizontal end of line paddind bit
#     * '+' for a vertical paddind line of bits
###############################################################################

import PIL.ImageFont as ImageFont
import struct

import sys
import codecs
import sys

# python2: sys.stdout = codecs.getwriter("utf-8")(sys.stdout)
sys.stdout = codecs.getwriter("utf-8")(sys.stdout.buffer)


FONT_SIZE = int(sys.argv[1]) if len(sys.argv) > 1 else 14
CHARSET_SIZE = 0x4e00


police = ImageFont.truetype("/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf", FONT_SIZE)

f = open(f"./dejavu_{FONT_SIZE}.fv2", u'wb')

# Magic number
f.write(u"FNT2".encode('utf-8'))

# Name of font
data = police.getname()[0].encode('utf-8')
f.write(struct.pack('h', len(data)))
f.write(data)

# font size
f.write(struct.pack('h', FONT_SIZE))

# Style of font (always 01)
f.write(struct.pack('h', 1))

for i in range(32, CHARSET_SIZE):
    char = chr(i)
    abc = police.font.getabc(char)
    abc = (int(abc[0]), int(abc[1]), int(abc[2]))
    mask = police.getmask(char, mode="1")
    w, h = mask.size # police.getsize(char)
    bbox = mask.getbbox()
    if bbox is None:
        bbox = (0,0,-1,0)
    x1 = bbox[0]
    y1 = bbox[1]
    x2 = bbox[2]
    y2 = bbox[3]
    cx = x2 - x1
    cy = y2 - y1

    print(f"CHR = {char}   SIZE = {w},{h}   ABC = {abc}   BBOX = {bbox}")

    left = abc[0] + x1
    right = abc[2]

    if cx > abc[1]:
        sys.stderr.write(f'bad size: {cx} > {abc[1]}')
        exit(1)

    f.write(struct.pack('h', abc[0]))
    f.write(struct.pack('h', y1))
    f.write(struct.pack('h', abc[1]))
    f.write(struct.pack('h', abc[2]))
    f.write(struct.pack('h', int((cx + 7) / 8)))
    f.write(struct.pack('h', cy))

    byteBitmap = 0
    cptPix   = 0
    totBytes = 0

    padding = 8 - cx % 8
    if padding == 8:
        padding = 0
    empty_line = '+'*(w+padding) + '\n'
    padding_line = 'o'*padding
    left_empty_line = '+'*x1
    right_empty_line = '+'*(w-x1-cx)
    print(empty_line*y1)
    for iy in range(y1, y1+cy):
        line = left_empty_line
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
                f.write(struct.pack('B', byte))
                counter = 0
                byte = 0

        if counter != 0:
            line += padding_line
            f.write(struct.pack('B', byte << padding))

        print(line+right_empty_line)

    print(empty_line*(h-y2)+'\n')

    while totBytes % 4 != 0:
        print("COMPLETING")
        f.write(chr(0))
        totBytes = totBytes + 1
