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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Dominique Lafages
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   header file. Keylayout object, used by keymap managers
*/

#pragma once

#include <cstdint>

//====================================
// SCANCODES PHYSICAL LAYOUT REFERENCE
//====================================
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +----+----+-------+
// | 01 |  | 3B | 3C | 3D | 3E |  | 3F | 40 | 41 | 42 |  | 43 | 44 | 57 | 58 |  | 37 | 46 | 1D+45 |
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +----+----+-------+
//                                     ***  keycodes suffixed by 'x' are extended ***
// +----+----+----+----+----+----+----+----+----+----+----+----+----+--------+  +----+----+----+  +--------------------+
// | 29 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 0A | 0B | 0C | 0D |   0E   |  | 52x| 47x| 49x|  | 45 | 35x| 37 | 4A  |
// +-------------------------------------------------------------------------+  +----+----+----+  +----+----+----+-----+
// |  0F  | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 1A | 1B |      |  | 53x| 4Fx| 51x|  | 47 | 48 | 49 |     |
// +------------------------------------------------------------------+  1C  |  +----+----+----+  +----+----+----| 4E  |
// |  3A   | 1E | 1F | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 2B |     |                    | 4B | 4C | 4D |     |
// +-------------------------------------------------------------------------+       +----+       +----+----+----+-----+
// |  2A | 56 | 2C | 2D | 2E | 2F | 30 | 31 | 32 | 33 | 34 | 35 |    36      |       | 48x|       | 4F | 50 | 51 |     |
// +-------------------------------------------------------------------------+  +----+----+----+  +---------+----| 1Cx |
// |  1D  |  5Bx | 38 |           39           |  38x  |  5Cx |  5Dx |  1Dx  |  | 4Bx| 50x| 4Dx|  |    52   | 53 |     |
// +------+------+----+------------------------+-------+------+------+-------+  +----+----+----+  +---------+----+-----+


//##############################################################################
struct Keylayout
//##############################################################################
{
    enum {
          MAX_DEADKEYS = 35
        , MAX_SECOND_KEYS = 35
        , MAX_LAYOUT_CHARS = 256
    };

    int LCID; // Microsoft Locale ID code used for keyboard layouts

    using KeyLayout_t = int[MAX_LAYOUT_CHARS];

    // keylayout working tables (X11 mode : begins in 8e position.)
    // Each one contains at most MAX_LAYOUT_CHARS key mappings for a given modifier keys combination
    KeyLayout_t const & noMod;
    KeyLayout_t const & shift;
    KeyLayout_t const & altGr;
    KeyLayout_t const & shiftAltGr;
    KeyLayout_t const & ctrl;
    KeyLayout_t const & capslock_noMod;
    KeyLayout_t const & capslock_shift;
    KeyLayout_t const & capslock_altGr;
    KeyLayout_t const & capslock_shiftAltGr;

    char const * locale_name;

    struct dkey_key_t {
        uint16_t secondKey;
        uint16_t modifiedKey;
    };

    struct dkey_t {                                 // Struture holding a deadkey and the rules to apply to available second keys:
         uint32_t   uchar;                       // unicode code point
         uint8_t    extendedKeyCode;             // scancode + extended bit
         uint8_t    nbSecondKeys;                // number of second keys available for that deadkey
         dkey_key_t secondKeys[MAX_SECOND_KEYS]; // the couples second key/modified key
    };

    //dkey_t deadkeys[MAX_DEADKEYS];
    dkey_t const * deadkeys;
    uint8_t nbDeadkeys;  // Effective number of deadkeys for the locale

    uint32_t verbose;


    // Constructor
    //==============================================================================
    Keylayout( int LCID
             , char const * LCID_locale_name
             , const KeyLayout_t & LCID_noMod
             , const KeyLayout_t & LCID_shift
             , const KeyLayout_t & LCID_altGr
             , const KeyLayout_t & LCID_shiftAltGr
             , const KeyLayout_t & LCID_ctrl
             , const KeyLayout_t & LCID_capslock_noMod
             , const KeyLayout_t & LCID_capslock_shift
             , const KeyLayout_t & LCID_capslock_altGr
             , const KeyLayout_t & LCID_capslock_shiftAltGr
             , const dkey_t LCID_deadkeys[MAX_DEADKEYS]
             , uint8_t nbDeadkeys
             , uint32_t verbose = 0
             )
        : LCID(LCID)
        , noMod(LCID_noMod)
        , shift(LCID_shift)
        , altGr(LCID_altGr)
        , shiftAltGr(LCID_shiftAltGr)
        , ctrl(LCID_ctrl)
        , capslock_noMod(LCID_capslock_noMod)
        , capslock_shift(LCID_capslock_shift)
        , capslock_altGr(LCID_capslock_altGr)
        , capslock_shiftAltGr(LCID_capslock_shiftAltGr)
        , locale_name(LCID_locale_name)
        , deadkeys(LCID_deadkeys)
        , nbDeadkeys(nbDeadkeys)
        , verbose(verbose)
    //==============================================================================
    {} // END Constructor

    Keylayout(Keylayout const &) = delete;
    Keylayout& operator=(Keylayout const &) = delete;


    //==============================================================================
    bool isDeadkey(uint32_t uchar, uint8_t extendedKeyCode) const
    //==============================================================================
    {
        bool resu = false;
        for (int i=0; i < this->nbDeadkeys; i++) {
            // Search if a make is a deadkey by its scancode AND by its unicode translation.
            // NB : unicode alone is not enough. (e.g. french CARET from scancode 'Ox1A' is a deadkey but
            //      from scancode '0x0A' it isn't).
            if (   (this->deadkeys[i].extendedKeyCode == extendedKeyCode)
               and (this->deadkeys[i].uchar == uchar)
               )
            {
                resu = true;
            }
        }
        return resu;

    } // END METHOD - isDeadkey


}; // END STRUCT - Keylayout
