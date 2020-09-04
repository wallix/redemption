/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2013
   Author(s): Clément Moroldo
*/

#pragma once

#include <cstdint>

struct Qt_ScanCode_KeyMap
{
    static uint16_t x11_native_scancode_to_rdp_scancode(uint32_t native_scancode)
    {
        static const uint16_t scancodes[] {
/* 00 - 03 */      0          ,      0          ,      0          ,      0          ,
/* 04 - 07 */      0          ,      0          ,      0          ,      0          ,
/* 08 - 0b */      0          , 0x0001   /*Esc*/, 0x0002     /*!*/, 0x0003     /*@*/,
/* 0c - 0f */ 0x0004     /*#*/, 0x0005     /*$*/, 0x0006     /*%*/, 0x0007     /*^*/,
/* 10 - 13 */ 0x0008     /*&*/, 0x0009     /***/, 0x000a     /*(*/, 0x000b     /*)*/,
/* 14 - 17 */ 0x000c     /*_*/, 0x000d     /*=*/, 0x000E /*BackS*/, 0x000F   /*Tab*/,
/* 18 - 1b */ 0x0010     /*q*/, 0x0011     /*w*/, 0x0012     /*e*/, 0x0013     /*r*/,
/* 1c - 1f */ 0x0014     /*t*/, 0x0015     /*y*/, 0x0016     /*u*/, 0x0017     /*i*/,
/* 20 - 23 */ 0x0018     /*o*/, 0x0019     /*p*/, 0x001A     /*[*/, 0x001B     /*]*/,
/* 24 - 27 */ 0x001C /*Enter*/, 0x001D /*CtrlL*/, 0x001E     /*a*/, 0x001F     /*s*/,
/* 28 - 2b */ 0x0020     /*d*/, 0x0021     /*f*/, 0x0022     /*g*/, 0x0023     /*h*/,
/* 2c - 2f */ 0x0024     /*j*/, 0x0025     /*k*/, 0x0026     /*l*/, 0x0027     /*;*/,
/* 30 - 33 */ 0x0028     /*'*/, 0x0029     /*`*/, 0x002A /*Shift*/, 0x002B     /*\*/,
/* 34 - 37 */ 0x002C     /*z*/, 0x002D     /*x*/, 0x002E     /*c*/, 0x002F     /*v*/,
/* 38 - 3b */ 0x0030     /*b*/, 0x0031     /*n*/, 0x0032     /*m*/, 0x0033     /*,*/,
/* 3c - 3f */ 0x0034     /*.*/, 0x0035     /*/*/,      0          , 0x0037     /***/,
/* 40 - 43 */ 0x0038   /*Alt*/, 0x0039     /* */, 0x003A /*CapsL*/, 0x003B    /*F1*/,
/* 44 - 47 */ 0x003C    /*F2*/, 0x003D    /*F3*/, 0x003E    /*F4*/, 0x003F    /*F5*/,
/* 48 - 4b */ 0x0040    /*F6*/, 0x0041    /*F7*/, 0x0042    /*F8*/, 0x0043    /*F9*/,
/* 4c - 4f */ 0x0044   /*F10*/,      0          , 0x0046 /*VerDe*/, 0x0047     /*7*/,
/* 50 - 53 */ 0x0048     /*8*/, 0x0049     /*9*/, 0x004A     /*-*/, 0x004B     /*4*/,
/* 54 - 57 */ 0x004C     /*5*/, 0x004D     /*6*/, 0x004E     /*+*/, 0x004F     /*1*/,
/* 58 - 5b */ 0x0050     /*2*/, 0x0051     /*3*/, 0x0052     /*0*/, 0x0053     /*.*/,
/* 5c - 5f */      0          ,      0          , 0x0056     /*\*/, 0x0057   /*F11*/,
/* 60 - 63 */ 0x0058   /*F12*/,      0          ,      0          ,      0          ,
/* 64 - 67 */      0          ,      0          ,      0          ,      0          ,
/* 68 - 6b */ 0x011C /*EntPa*/, 0x011D /*ctrlR*/, 0x0135     /*/*/, 0x0037 /*Scree*/,
/* 6c - 6f */ 0x0138 /*AltGr*/,      0          , 0x0147  /*Home*/, 0x0148    /*Up*/,
/* 70 - 73 */ 0x0149  /*PgUp*/, 0x014B  /*Left*/, 0x014D /*Right*/, 0x014F   /*End*/,
/* 74 - 77 */ 0x0150  /*Down*/, 0x0151  /*PgDo*/, 0x0152 /*Inser*/, 0x0153   /*Del*/,
/* 78 - 7b */      0          , 0x013B   /*F13*/, 0x013C   /*F14*/, 0x013D   /*F15*/,
/* 7c - 7f */ 0x013E   /*F16*/,      0          ,      0          ,      0          ,
/* 80 - 83 */      0          ,      0          ,      0          ,      0          ,
/* 84 - 87 */      0          , 0x015B /*MetaL*/, 0x015C /*MetaR*/, 0x015D  /*Menu*/,
/* 88 - 8b */      0          ,      0          ,      0          ,      0          ,
/* 8c - 8f */      0          ,      0          ,      0          ,      0          ,
/* 90 - 93 */      0          ,      0          ,      0          ,      0          ,
/* 94 - 97 */      0          ,      0          ,      0          ,      0          ,
/* 98 - 9b */      0          ,      0          ,      0          ,      0          ,
/* 9c - 9f */      0          ,      0          ,      0          ,      0          ,
/* a0 - a3 */      0          ,      0          ,      0          ,      0          ,
/* a4 - a7 */      0          ,      0          ,      0          ,      0          ,
/* a8 - ab */      0          ,      0          ,      0          , 0x0142   /*F20*/,
/* ac - af */ 0x013F   /*F17*/, 0x0141   /*F19*/, 0x0140   /*F18*/,      0          ,
/* b0 - b3 */      0          ,      0          ,      0          ,      0          ,
/* b4 - b7 */      0          ,      0          ,      0          ,      0          ,
/* b8 - bb */      0          ,      0          ,      0          ,      0          ,
/* bc - bf */      0          ,      0          ,      0          ,      0          ,
/* c0 - c3 */      0          ,      0          ,      0          ,      0          ,
/* c4 - c7 */      0          ,      0          ,      0          ,      0          ,
/* c8 - cc */      0          ,      0          ,      0          ,      0          ,
/* cc - cf */      0          ,      0          ,      0          ,      0          ,
/* d0 - d3 */      0          ,      0          ,      0          ,      0          ,
/* d4 - d7 */      0          ,      0          ,      0          ,      0          ,
/* d8 - dd */      0          ,      0          ,      0          ,      0          ,
/* dc - df */      0          ,      0          ,      0          ,      0          ,
/* e0 - e3 */      0          ,      0          ,      0          ,      0          ,
/* e4 - e7 */      0          ,      0          ,      0          ,      0          ,
/* e8 - ee */      0          ,      0          ,      0          ,      0          ,
/* ec - ef */      0          ,      0          ,      0          ,      0          ,
/* f0 - f3 */      0          ,      0          ,      0          ,      0          ,
/* f4 - f7 */      0          ,      0          ,      0          ,      0          ,
/* f8 - ff */ 0x0143   /*F21*/,      0          ,      0          ,      0          ,
/* fc - ff */      0          ,      0          ,      0          ,      0          ,
        };

        return native_scancode < 0xff ? scancodes[native_scancode] : uint16_t();
        // 0x7f /* pause */ = 0x1d, 0x45
    }

    static uint16_t win_native_scancode_to_rdp_scancode(uint32_t native_scancode)
    {
        return native_scancode < 0xff ? native_scancode : uint16_t();
    }
};
