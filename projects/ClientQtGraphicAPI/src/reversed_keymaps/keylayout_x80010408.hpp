#pragma once

#include "keylayout_r.hpp"

namespace x80010408
{

const static int LCID = 0x10408;

const static char * const locale_name = "el-GR.220";

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0023, 0x0027,
    0x002a, 0x002b, 0x002b, 0x002c, 0x002c, 0x002d, 0x002d, 0x002e,
    0x002f, 0x0030, 0x0030, 0x0031, 0x0031, 0x0032, 0x0032, 0x0033,
    0x0033, 0x0034, 0x0034, 0x0035, 0x0035, 0x0036, 0x0036, 0x0037,
    0x0037, 0x0038, 0x0038, 0x0039, 0x0039, 0x003c, 0x005d, 0x007d,
    0x00a8, 0x00bd, 0x037e, 0x0384, 0x03b1, 0x03b2, 0x03b3, 0x03b4,
    0x03b5, 0x03b6, 0x03b7, 0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc,
    0x03bd, 0x03be, 0x03bf, 0x03c0, 0x03c1, 0x03c2, 0x03c3, 0x03c4,
    0x03c5, 0x03c6, 0x03c7, 0x03c8, 0x03c9, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_noMod[] {
        14,     15,    100,     28,      1,     57,     43,     12,
        55,     26,     78,     51,     83,     74,     53,     52,
        98,     11,     82,     79,      2,      3,     80,      4,
        81,     75,      5,     76,      6,     77,      7,      8,
        71,      9,     72,     73,     10,     86,     13,     27,
        40,     41,     16,     39,     30,     48,     34,     32,
        18,     44,     35,     22,     23,     37,     38,     50,
        49,     36,     24,     25,     19,     17,     31,     20,
        21,     33,     45,     46,     47, 
};
constexpr Keylayout_r::KeyLayoutMap_t noMod{uchar_noMod, scancode_noMod, 69};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shift[] {
    0x0008, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022, 0x0024,
    0x0025, 0x0026, 0x0028, 0x0029, 0x002a, 0x002a, 0x002b, 0x002c,
    0x002d, 0x002f, 0x002f, 0x003a, 0x003a, 0x003b, 0x003d, 0x003e,
    0x003f, 0x0040, 0x005b, 0x005f, 0x007b, 0x007e, 0x007f, 0x00a3,
    0x00a8, 0x00b1, 0x0385, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395,
    0x0396, 0x0397, 0x0398, 0x0399, 0x039a, 0x039b, 0x039c, 0x039d,
    0x039e, 0x039f, 0x03a0, 0x03a1, 0x03a3, 0x03a4, 0x03a5, 0x03a6,
    0x03a7, 0x03a8, 0x03a9, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shift[] {
        14,    100,     28,      1,     57,      2,      3,      5,
         6,      7,      9,     10,     55,     26,     78,     83,
        74,    104,      8,     16,     52,     51,     11,     86,
        12,     43,     13,     53,     27,     17,     99,      4,
        39,     41,     40,     30,     48,     34,     32,     18,
        44,     35,     22,     23,     37,     38,     50,     49,
        36,     24,     25,     19,     31,     20,     21,     33,
        45,     46,     47, 
};
constexpr Keylayout_r::KeyLayoutMap_t shift{uchar_shift, scancode_shift, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a9, 0x00ab,
    0x00ac, 0x00ae, 0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b6, 0x00bb,
    0x00bd, 0x0385, 0x0385, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_altGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,      5,      9,     21,     10,      6,     46,     26,
        43,     19,     11,     12,      3,      4,      7,     27,
        13,     39,     40,     18, 
};
constexpr Keylayout_r::KeyLayoutMap_t altGr{uchar_altGr, scancode_altGr, 28};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shiftAltGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shiftAltGr[] {
        14,     15,     28,    100,      1,     55,     78,     74,
        98, 
};
constexpr Keylayout_r::KeyLayoutMap_t shiftAltGr{uchar_shiftAltGr, scancode_shiftAltGr, 9};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0023, 0x0027,
    0x002a, 0x002b, 0x002b, 0x002c, 0x002c, 0x002d, 0x002d, 0x002e,
    0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036,
    0x0037, 0x0038, 0x0039, 0x003c, 0x005d, 0x007d, 0x00a8, 0x00bd,
    0x037e, 0x0384, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396,
    0x0397, 0x0398, 0x0399, 0x039a, 0x039b, 0x039c, 0x039d, 0x039e,
    0x039f, 0x03a0, 0x03a1, 0x03a3, 0x03a4, 0x03a5, 0x03a6, 0x03a7,
    0x03a8, 0x03a9, 0x03c2, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_noMod[] {
        14,     15,    100,     28,      1,     57,     43,     12,
        55,     26,     78,     51,     83,     53,     74,     52,
        98,     11,      2,      3,      4,      5,      6,      7,
         8,      9,     10,     86,     13,     27,     40,     41,
        16,     39,     30,     48,     34,     32,     18,     44,
        35,     22,     23,     37,     38,     50,     49,     36,
        24,     25,     19,     31,     20,     21,     33,     45,
        46,     47,     17, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_noMod{uchar_capslock_noMod, scancode_capslock_noMod, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shift[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022,
    0x0024, 0x0025, 0x0026, 0x0028, 0x0029, 0x002a, 0x002a, 0x002b,
    0x002c, 0x002d, 0x002f, 0x002f, 0x003a, 0x003a, 0x003b, 0x003d,
    0x003e, 0x003f, 0x0040, 0x005b, 0x005f, 0x007b, 0x007e, 0x00a3,
    0x00a8, 0x00b1, 0x0385, 0x03b1, 0x03b2, 0x03b3, 0x03b4, 0x03b5,
    0x03b6, 0x03b7, 0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd,
    0x03be, 0x03bf, 0x03c0, 0x03c1, 0x03c3, 0x03c4, 0x03c5, 0x03c6,
    0x03c7, 0x03c8, 0x03c9, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shift[] {
        14,     15,    100,     28,      1,     57,      2,      3,
         5,      6,      7,      9,     10,     55,     26,     78,
        83,     74,      8,     98,     16,     52,     51,     11,
        86,     12,     43,     13,     53,     27,     17,      4,
        39,     41,     40,     30,     48,     34,     32,     18,
        44,     35,     22,     23,     37,     38,     50,     49,
        36,     24,     25,     19,     31,     20,     21,     33,
        45,     46,     47, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shift{uchar_capslock_shift, scancode_capslock_shift, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a9, 0x00ab,
    0x00ac, 0x00ae, 0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b6, 0x00bb,
    0x00bd, 0x0385, 0x0385, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_altGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,      5,      9,     21,     10,      6,     46,     26,
        43,     19,     11,     12,      3,      4,      7,     27,
        13,     39,     40,     18, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_altGr{uchar_capslock_altGr, scancode_capslock_altGr, 28};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shiftAltGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shiftAltGr[] {
        14,     15,     28,    100,      1,     55,     78,     74,
        98, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shiftAltGr{uchar_capslock_shiftAltGr, scancode_capslock_shiftAltGr, 9};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_ctrl[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x001b, 0x001d, 0x001d,
    0x0020, 0x002a, 0x002b, 0x002d, 0x002f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_ctrl[] {
        14,     15,     28,    100,      1,     26,     13,     27,
        57,     55,     78,     74,     98, 
};
constexpr Keylayout_r::KeyLayoutMap_t ctrl{uchar_ctrl, scancode_ctrl, 13};

const Keylayout_r::KeyLayoutMap_t deadkeys {
};


const static uint8_t nbDeadkeys = 3;

}

static const Keylayout_r keylayout_x80010408(
    x80010408::LCID,
    x80010408::locale_name,
    x80010408::noMod,
    x80010408::shift,
    x80010408::altGr,
    x80010408::shiftAltGr,
    x80010408::ctrl,
    x80010408::capslock_noMod,
    x80010408::capslock_shift,
    x80010408::capslock_altGr,
    x80010408::capslock_shiftAltGr,
    x80010408::deadkeys,
    x80010408::nbDeadkeys
);

