/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Dominique Lafages
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   header file. Keylayout object, used by keymap managers
*/

#ifndef _REDEMPTION_KEYLAYOUT_X00010408_HPP_
#define _REDEMPTION_KEYLAYOUT_X00010408_HPP_

#include"keylayout.hpp"

namespace x00010408 {    // Greek (Greece) // Greek (220)

const static int LCID = 0x00010408;

const Keylayout::KeyLayout_t noMod = {
    /* x00 - x07 */    0x0000, 0x001B,    '1',    '2',    '3',    '4',    '5',    '6',
    /* x08 - x0F */       '7',    '8',    '9',    '0', 0x0027, 0x005D, 0x0008, 0x0009,
    /* x10 - x17 */    0x037E, 0x03C2, 0x03B5, 0x03C1, 0x03C4, 0x03C5, 0x03B8, 0x03B9,
    /* x18 - x1F */    0x03BF, 0x03C0, 0x002B, 0x007D, 0x000D, 0x0000, 0x03B1, 0x03C3,
    /* x20 - x27 */    0x03B4, 0x03C6, 0x03B3, 0x03B7, 0x03BE, 0x03BA, 0x03BB, 0x0384,
    /* x28 - x2F */    0x00A8, 0x00BD, 0x0000, 0x0023, 0x03B6, 0x03C7, 0x03C8, 0x03C9,
    /* x30 - x37 */    0x03B2, 0x03BD, 0x03BC, 0x002C, 0x002E, 0x002D, 0x0000,    '*',
    /* x38 - x3F */    0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x40 - x47 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,    '7',
    /* x48 - x4F */       '8',    '9',    '-',    '4',    '5',    '6',    '+',    '1',
    /* x50 - x57 */       '2',    '3',    '0', 0x002C, 0x0000, 0x0000, 0x003C, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000,    '/', 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x78 - x7F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t shift = {
    /* x00 - x07 */    0x0000, 0x001B, 0x0021, 0x0022, 0x00A3, 0x0024, 0x0025, 0x0026,
    /* x08 - x0F */    0x002F, 0x0028, 0x0029, 0x003D, 0x003F, 0x005B, 0x0008, 0x0000,
    /* x10 - x17 */    0x003A, 0x007E, 0x0395, 0x03A1, 0x03A4, 0x03A5, 0x0398, 0x0399,
    /* x18 - x1F */    0x039F, 0x03A0, 0x002A, 0x007B, 0x000D, 0x0000, 0x0391, 0x03A3,
    /* x20 - x27 */    0x0394, 0x03A6, 0x0393, 0x0397, 0x039E, 0x039A, 0x039B, 0x00A8,
    /* x28 - x2F */    0x0385, 0x00B1, 0x0000, 0x0040, 0x0396, 0x03A7, 0x03A8, 0x03A9,
    /* x30 - x37 */    0x0392, 0x039D, 0x039C, 0x003B, 0x003A, 0x005F, 0x0000,    '*',
    /* x38 - x3F */    0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x40 - x47 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000,    '-', 0x0000, 0x0000, 0x0000,    '+', 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x0000, 0x002C, 0x0000, 0x0000, 0x003E, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000, 0x0000, 0x007F, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */       '/', 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x78 - x7F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t altGr = {
    /* x00 - x07 */    0x0000, 0x001B, 0x0000, 0x00B2, 0x00B3, 0x00A3, 0x00A7, 0x00B6,
    /* x08 - x0F */    0x0000, 0x00A4, 0x00A6, 0x00B0, 0x00B1, 0x00BD, 0x0008, 0x0009,
    /* x10 - x17 */    0x0000, 0x0000, 0x20AC, 0x00AE, 0x0000, 0x00A5, 0x0000, 0x0000,
    /* x18 - x1F */    0x0000, 0x0000, 0x00AB, 0x00BB, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x20 - x27 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0385,
    /* x28 - x2F */    0x0385, 0x0000, 0x0000, 0x00AC, 0x0000, 0x0000, 0x00A9, 0x0000,
    /* x30 - x37 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,    '*',
    /* x38 - x3F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x40 - x47 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000,    '-', 0x0000, 0x0000, 0x0000,    '+', 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000,    '/', 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x78 - x7F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t shiftAltGr = {
    /* x00 - x07 */    0x0000, 0x001B, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x08 - x0F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0008, 0x0009,
    /* x10 - x17 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x18 - x1F */    0x0000, 0x0000, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x20 - x27 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x28 - x2F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x30 - x37 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,    '*',
    /* x38 - x3F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x40 - x47 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000,    '-', 0x0000, 0x0000, 0x0000,    '+', 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000,    '/', 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x78 - x7F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t ctrl = {
    /* x00 - x07 */    0x0000, 0x001B, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x08 - x0F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x001D, 0x0008, 0x0009,
    /* x10 - x17 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x18 - x1F */    0x0000, 0x0000, 0x001B, 0x001D, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x20 - x27 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x28 - x2F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x30 - x37 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,    '*',
    /* x38 - x3F */    0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x40 - x47 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000,    '-', 0x0000, 0x0000, 0x0000,    '+', 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000,    '/', 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x78 - x7F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t capslock_noMod = {
    /* x00 - x07 */    0x0000, 0x001B,    '1',    '2',    '3',    '4',    '5',    '6',
    /* x08 - x0F */       '7',    '8',    '9',    '0', 0x0027, 0x005D, 0x0008, 0x0009,
    /* x10 - x17 */    0x037E, 0x03C2, 0x0395, 0x03A1, 0x03A4, 0x03A5, 0x0398, 0x0399,
    /* x18 - x1F */    0x039F, 0x03A0, 0x002B, 0x007D, 0x000D, 0x0000, 0x0391, 0x03A3,
    /* x20 - x27 */    0x0394, 0x03A6, 0x0393, 0x0397, 0x039E, 0x039A, 0x039B, 0x0384,
    /* x28 - x2F */    0x00A8, 0x00BD, 0x0000, 0x0023, 0x0396, 0x03A7, 0x03A8, 0x03A9,
    /* x30 - x37 */    0x0392, 0x039D, 0x039C, 0x002C, 0x002E, 0x002D, 0x0000,    '*',
    /* x38 - x3F */    0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x40 - x47 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000,    '-', 0x0000, 0x0000, 0x0000,    '+', 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x0000, 0x002C, 0x0000, 0x0000, 0x003C, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000,    '/', 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x78 - x7F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t capslock_shift = {
    /* x00 - x07 */    0x0000, 0x001B, 0x0021, 0x0022, 0x00A3, 0x0024, 0x0025, 0x0026,
    /* x08 - x0F */    0x002F, 0x0028, 0x0029, 0x003D, 0x003F, 0x005B, 0x0008, 0x0009,
    /* x10 - x17 */    0x003A, 0x007E, 0x03B5, 0x03C1, 0x03C4, 0x03C5, 0x03B8, 0x03B9,
    /* x18 - x1F */    0x03BF, 0x03C0, 0x002A, 0x007B, 0x000D, 0x0000, 0x03B1, 0x03C3,
    /* x20 - x27 */    0x03B4, 0x03C6, 0x03B3, 0x03B7, 0x03BE, 0x03BA, 0x03BB, 0x00A8,
    /* x28 - x2F */    0x0385, 0x00B1, 0x0000, 0x0040, 0x03B6, 0x03C7, 0x03C8, 0x03C9,
    /* x30 - x37 */    0x03B2, 0x03BD, 0x03BC, 0x003B, 0x003A, 0x005F, 0x0000,    '*',
    /* x38 - x3F */    0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x40 - x47 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000,    '-', 0x0000, 0x0000, 0x0000,    '+', 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x0000, 0x002C, 0x0000, 0x0000, 0x003E, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000,    '/', 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x78 - x7F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t capslock_altGr = {
    /* x00 - x07 */    0x0000, 0x001B, 0x0000, 0x00B2, 0x00B3, 0x00A3, 0x00A7, 0x00B6,
    /* x08 - x0F */    0x0000, 0x00A4, 0x00A6, 0x00B0, 0x00B1, 0x00BD, 0x0008, 0x0009,
    /* x10 - x17 */    0x0000, 0x0000, 0x20AC, 0x00AE, 0x0000, 0x00A5, 0x0000, 0x0000,
    /* x18 - x1F */    0x0000, 0x0000, 0x00AB, 0x00BB, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x20 - x27 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0385,
    /* x28 - x2F */    0x0385, 0x0000, 0x0000, 0x00AC, 0x0000, 0x0000, 0x00A9, 0x0000,
    /* x30 - x37 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,    '*',
    /* x38 - x3F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x40 - x47 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000,    '-', 0x0000, 0x0000, 0x0000,    '+', 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000,    '/', 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x78 - x7F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::KeyLayout_t capslock_shiftAltGr = {
    /* x00 - x07 */    0x0000, 0x001B, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x08 - x0F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0008, 0x0009,
    /* x10 - x17 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x18 - x1F */    0x0000, 0x0000, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x20 - x27 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x28 - x2F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x30 - x37 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,    '*',
    /* x38 - x3F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x40 - x47 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x48 - x4F */    0x0000, 0x0000,    '-', 0x0000, 0x0000, 0x0000,    '+', 0x0000,
    /* x50 - x57 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x58 - x5F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x60 - x67 */    0x0000, 0x0000,    '/', 0x0000, 0x000D, 0x0000, 0x0000, 0x0000,
    /* x68 - x6F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x70 - x77 */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    /* x78 - x7F */    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

const Keylayout::dkey_t deadkeys[] = {
    { 0x0384, 0x27, 15, { {0x0395, 0x0388}  // 'Ε' = 'Έ'
                        , {0x0020, 0x0384}  // ' ' = '΄'
                        , {0x03c9, 0x03ce}  // 'ω' = 'ώ'
                        , {0x03a9, 0x038f}  // 'Ω' = 'Ώ'
                        , {0x03a5, 0x038e}  // 'Υ' = 'Ύ'
                        , {0x0397, 0x0389}  // 'Η' = 'Ή'
                        , {0x0399, 0x038a}  // 'Ι' = 'Ί'
                        , {0x03c5, 0x03cd}  // 'υ' = 'ύ'
                        , {0x03b5, 0x03ad}  // 'ε' = 'έ'
                        , {0x03b7, 0x03ae}  // 'η' = 'ή'
                        , {0x03bf, 0x03cc}  // 'ο' = 'ό'
                        , {0x03b1, 0x03ac}  // 'α' = 'ά'
                        , {0x039f, 0x038c}  // 'Ο' = 'Ό'
                        , {0x03b9, 0x03af}  // 'ι' = 'ί'
                        , {0x0391, 0x0386}  // 'Α' = 'Ά'
                        }
    },
    { 0x00a8, 0x27,  5, { {0x03a5, 0x03ab}  // 'Υ' = 'Ϋ'
                        , {0x0020, 0x00a8}  // ' ' = '¨'
                        , {0x03b9, 0x03ca}  // 'ι' = 'ϊ'
                        , {0x0399, 0x03aa}  // 'Ι' = 'Ϊ'
                        , {0x03c5, 0x03cb}  // 'υ' = 'ϋ'
                        }
    },
    { 0x0385, 0x28,  3, { {0x0020, 0x0385}  // ' ' = '΅'
                        , {0x03b9, 0x0390}  // 'ι' = 'ΐ'
                        , {0x03c5, 0x03b0}  // 'υ' = 'ΰ'
                        }
    },
};

const static uint8_t nbDeadkeys = 3;

} // END NAMESPACE - x00010408

static const Keylayout keylayout_x00010408( x00010408::LCID
                                          , x00010408::noMod
                                          , x00010408::shift
                                          , x00010408::altGr
                                          , x00010408::shiftAltGr
                                          , x00010408::ctrl
                                          , x00010408::capslock_noMod
                                          , x00010408::capslock_shift
                                          , x00010408::capslock_altGr
                                          , x00010408::capslock_shiftAltGr
                                          , x00010408::deadkeys
                                          , x00010408::nbDeadkeys
);

#endif
