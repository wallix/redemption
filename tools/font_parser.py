# Dominique Lafages
# Copyright WALLIX 2012

########################################################################################################################
# script extracting a font from a truetype opensource definition and converting it to the FV1 format used by
# ReDemPtion.
#
# HINTs:
# - Each FV1 glyph is sketched in a bitmap whose dimensions are mutiples of 8. As PIL glyphes width are not
#   multiple of 8 they have to be padded. By convention, they are padded to left and bottom.
# - The glyphs are not antialiased.
# - The police is variable sized
# - Thus, each pixel in a sketch is represented by only one bit
#
# FORMATs :
# - the FV1 file always begins by the label "FNT1"
# - Police global informations are :
#      * name (ex : Deja Vu Sans)
#      * size (ex : 10)
#      * style (always '1')
# - Individual glyph informations are :
#      * width (a round multiple of 8 ; theorically it should be based on PIL ABC.abcB; but after some experimentation,
#        it has been based on PIL size.x, for esthetical reasons as lack of space between characters)
#      * height (a round multiple of 8 ; based on PIL size.y)
#      * baseline (negative value of height)
#      * offset (based on PIL ABC.abcA - this is the space before the character)
#      * data (the bitmap representing the sketch of the glyph, one bit by pixel, 0 for background, 1 for foreground)
#
# TECHs :
# - struct.pack formats are :
#     * 'h' [short] for a two bytes emision
#     * 'B' [unsigned char] for a one byte emision
# - the data generation loop print each glyph sketch to sdtout, with each bit represented as follow :
#     * '.' for a PIL background bit
#     * '#' for a PIL foreground bit
#     * 'o' for an horizontal end of line paddind bit
#     * '+' for a vertical paddind line of bits
########################################################################################################################

import ImageFont
import struct

import sys
import codecs
sys.stdout = codecs.getwriter("utf-8")(sys.stdout)

FONT_SIZE = 18
CHARSET_SIZE = 0x4e00

########################################################################################################################
# FUNCTIONS
########################################################################################################################

def doBitmap( f, cptPix, pix, byteBmp, totB):
    """ Manage bitmap bytes bit-by-bit construction and flushing to .fv1 file

        KEYWORDS :
            - [IN] f = file handle
            - [IN / OUT] pt = bit rank in the byte bitmap (between 0 and 7)
            - [IN] color of the pixel (0 or 1)
            - [IN / OUT] byteBitmap = a bitmap byte variable
            - [IN / OUT] totB = total number of bytes flushed
    """

    if pix == 1:
        byteBmp = byteBmp | (128 >> (cptPix))
    cptPix = cptPix + 1
    # if bit pointer is 8, flush the byte to file and reset the byte managment variables
    if cptPix == 8:
        cptPix = 0 # bits counter
        aByte = struct.pack('B', byteBmp)
        f.write(aByte)
        totB = totB  + 1  # bytes counter
        byteBmp = 0       # byte bitmap

    return cptPix, byteBmp, totB

# END FUNCTION - doBitmap


########################################################################################################################
# MAIN
########################################################################################################################

#police = ImageFont.truetype("/home/dlafages/work/wab/tags/wab2-2.1.8/redemption/tests/fixtures/DejaVuSans.ttf", FONT_SIZE)
police = ImageFont.truetype("/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf", FONT_SIZE)

filename = u"./dejavu_{fontsize}.fv1".format(fontsize = FONT_SIZE)
f = open(filename, u'w')

# Magic number
line = u"FNT1"
f.write(line)

# Name of font
line = u"{fontname}".format(fontname = police.getname()[0])
f.write(line.encode(u'utf-8'))

# Pad name of font up to 32 bytes
print len(police.getname()[0])
for i in range(len(police.getname()[0]), 32):
    f.write(chr(0))

# font size
line = struct.pack('h', FONT_SIZE)
f.write(line)

# Style of font (always 01)
line = struct.pack('h', 1)
f.write(line)

# Pad with 8 zeros
for i in range(0, 8):
    f.write(chr(0))

cptChar = 0
for i in range(32, CHARSET_SIZE):
    cptChar = cptChar + 1

#    if (i == 95) and (FONT_SIZE == 11):
#        write_custom_underscore(f)
#        continue

    x, y = police.getsize(unichr(i))
    msk = police.getmask(unichr(i), mode="1")

    print "CHR = ", unichr(i), "  SIZE = ", police.getsize(unichr(i)), "  ABC  = ", police.font.getabc(unichr(i)), "  BBOX = ", msk.getbbox()

    baseW = x
    baseH = y

    abc = police.font.getabc(unichr(i))

    # width of glyph (based on ABC.abcB, rounded to the next multiple of 8)
    xx = x
    if abc[1] != 0:
      if abc[0] == 0:
        xx = xx + 1
      if abc[2] == 0:
        xx = xx + 1
    fullW = (((xx - 1 ) / 8 ) + 1 ) * 8
    if fullW == 0:
        fullW = 8
    line = struct.pack('h', fullW )
    f.write(line)

    # height of glyph (rounded to the next multiple of 8)
    fullH = (((y - 1) / 8 ) + 1) * 8
    if fullH == 0:
        fullH = 8
    line = struct.pack('h', fullH )
    f.write(line)

    # baseline of glyph (negative of height)
    line = struct.pack('h', -fullH )
    f.write(line)

    # offset of glyph (ABC.abcA)
    # an offset < 1 is forced to 1 (tests show that a null or negative offset causes a bad centering of the glyph)
    offset = police.font.getabc(unichr(i))[0]
    if offset < 1:
        offset = 1
    line = struct.pack('h', offset )
    f.write(line)

    # incby of glyph
    line = struct.pack('h', xx)
    f.write(line)

    # Pad with 6 zeros
    for ii in range(0, 6):
        f.write(chr(0))

    byteBitmap = 0
    cptPix   = 0
    totBytes = 0

    # Quick-fix to generate a makeshift "_" (underscore ) glyph
    # (that glyph is blank in the police generated by PIL from dejaVueSans.ttf 11)
    if (i == 95) and (FONT_SIZE == 11):
        for iy in range(0, fullH):

            if iy < baseH:

                for ix in range (0, fullW):

                    if ix < baseW:
                        pix = 0
                        if (iy == 12):
                            pix = 255
                        if pix == 255:
                            print "#",
                            cptPix, byteBitmap, totBytes = doBitmap(f, cptPix, 1, byteBitmap, totBytes)
                        else:
                            print ".",
                            cptPix, byteBitmap, totBytes = doBitmap(f, cptPix, 0, byteBitmap, totBytes)
                    else:
                        print "o",
                        cptPix, byteBitmap, totBytes = doBitmap(f, cptPix, 0, byteBitmap, totBytes)
            else:
                for ix in range (0, fullW):
                    print "+",
                    cptPix, byteBitmap, totBytes = doBitmap(f, cptPix, 0, byteBitmap, totBytes)
            print
    else:
        for iy in range(0, fullH):

            if iy < baseH:

                if x != xx and abc[0] == 0:
                  print ".",
                  cptPix, byteBitmap, totBytes = doBitmap(f, cptPix, 0, byteBitmap, totBytes)

                for ix in range (0, fullW - (xx-x)):

                    if ix < baseW:
                        pix = msk.getpixel((ix, iy))
                        if pix == 255:
                            print "#",
                            cptPix, byteBitmap, totBytes = doBitmap(f, cptPix, 1, byteBitmap, totBytes)
                        else:
                            print ".",
                            cptPix, byteBitmap, totBytes = doBitmap(f, cptPix, 0, byteBitmap, totBytes)
                    else:
                        if x != xx and abc[2] == 0 and ix == baseW:
                          print ".",
                          cptPix, byteBitmap, totBytes = doBitmap(f, cptPix, 0, byteBitmap, totBytes)
                        print "o",
                        cptPix, byteBitmap, totBytes = doBitmap(f, cptPix, 0, byteBitmap, totBytes)

                X = fullW - (xx-x)
                if x != xx and abc[0] == 0:
                  X = X + 1

                if x != xx and abc[2] == 0 and X < xx:
                  print ".",
                  cptPix, byteBitmap, totBytes = doBitmap(f, cptPix, 0, byteBitmap, totBytes)

            else:
                for ix in range (0, fullW):
                    print "+",
                    cptPix, byteBitmap, totBytes = doBitmap(f, cptPix, 0, byteBitmap, totBytes)
            print

    while totBytes % 4 != 0:
        print "COMPLETING"
        f.write(chr(0))
        totBytes = totBytes + 1

# END OF SCRIPT
