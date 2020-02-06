#pragma once

#include "keylayout_r.hpp"

namespace x80050408
{

const static int LCID = 0x50408;

const static char * const locale_name = "el-GR.latin";

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0027, 0x002a,
    0x002b, 0x002c, 0x002c, 0x002d, 0x002d, 0x002e, 0x002f, 0x002f,
    0x0030, 0x0030, 0x0031, 0x0031, 0x0032, 0x0032, 0x0033, 0x0033,
    0x0034, 0x0034, 0x0035, 0x0035, 0x0036, 0x0036, 0x0037, 0x0037,
    0x0038, 0x0038, 0x0039, 0x0039, 0x003b, 0x003d, 0x005b, 0x005c,
    0x005c, 0x005d, 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065,
    0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d,
    0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075,
    0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_noMod[] {
        14,     15,     28,    100,      1,     57,     40,     55,
        78,     83,     51,     74,     12,     52,     98,     53,
        82,     11,     79,      2,      3,     80,     81,      4,
        75,      5,     76,      6,      7,     77,      8,     71,
         9,     72,     73,     10,     39,     13,     26,     43,
        86,     27,     41,     30,     48,     46,     32,     18,
        33,     34,     35,     23,     36,     37,     38,     50,
        49,     24,     25,     16,     19,     31,     20,     22,
        47,     17,     45,     21,     44, 
};
constexpr Keylayout_r::KeyLayoutMap_t noMod{uchar_noMod, scancode_noMod, 69};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shift[] {
    0x0008, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022, 0x0023,
    0x0024, 0x0025, 0x0026, 0x0028, 0x0029, 0x002a, 0x002a, 0x002b,
    0x002b, 0x002d, 0x002e, 0x002f, 0x003a, 0x003c, 0x003e, 0x003f,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005a, 0x005e, 0x005f, 0x007b, 0x007c, 0x007c,
    0x007d, 0x007e, 0x007f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shift[] {
        14,     28,    100,      1,     57,      2,     40,      4,
         5,      6,      8,     10,     11,      9,     55,     13,
        78,     74,     83,    104,     39,     51,     52,     53,
         3,     30,     48,     46,     32,     18,     33,     34,
        35,     23,     36,     37,     38,     50,     49,     24,
        25,     16,     19,     31,     20,     22,     47,     17,
        45,     21,     44,      7,     12,     26,     86,     43,
        27,     41,     99, 
};
constexpr Keylayout_r::KeyLayoutMap_t shift{uchar_shift, scancode_shift, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x00a1, 0x00a4, 0x00a5, 0x00a9, 0x00ab, 0x00ac, 0x00ae,
    0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00bb, 0x00bc, 0x00bd,
    0x00be, 0x00bf, 0x00d7, 0x00df, 0x00e1, 0x00e4, 0x00e5, 0x00e6,
    0x00e7, 0x00e9, 0x00ed, 0x00f0, 0x00f1, 0x00f3, 0x00f6, 0x00f8,
    0x00fa, 0x00fc, 0x00fe, 0x2018, 0x2019, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_altGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,      2,      5,     12,     46,     26,     43,     19,
         3,      4,     40,     50,     39,     27,      7,      8,
         9,     53,     13,     31,     30,     16,     17,     44,
        51,     18,     23,     32,     49,     24,     25,     38,
        22,     21,     20,     10,     11,      6, 
};
constexpr Keylayout_r::KeyLayoutMap_t altGr{uchar_altGr, scancode_altGr, 46};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shiftAltGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x00a2, 0x00a3, 0x00a6, 0x00a7, 0x00a8, 0x00b0, 0x00b9,
    0x00c1, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c9, 0x00cd, 0x00d0,
    0x00d1, 0x00d3, 0x00d6, 0x00d8, 0x00da, 0x00dc, 0x00de, 0x00f7,

};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shiftAltGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,     46,      5,     43,     31,     40,     39,      2,
        30,     16,     17,     44,     51,     18,     23,     32,
        49,     24,     25,     38,     22,     21,     20,     13,

};
constexpr Keylayout_r::KeyLayoutMap_t shiftAltGr{uchar_shiftAltGr, scancode_shiftAltGr, 32};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0027, 0x002a,
    0x002b, 0x002c, 0x002c, 0x002d, 0x002d, 0x002e, 0x002f, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003b, 0x003d, 0x0041, 0x0042, 0x0043, 0x0044,
    0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c,
    0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054,
    0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c,
    0x005c, 0x005d, 0x0060, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_noMod[] {
        14,     15,     28,    100,      1,     57,     40,     55,
        78,     83,     51,     74,     12,     52,     53,     98,
        11,      2,      3,      4,      5,      6,      7,      8,
         9,     10,     39,     13,     30,     48,     46,     32,
        18,     33,     34,     35,     23,     36,     37,     38,
        50,     49,     24,     25,     16,     19,     31,     20,
        22,     47,     17,     45,     21,     44,     26,     43,
        86,     27,     41, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_noMod{uchar_capslock_noMod, scancode_capslock_noMod, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shift[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022,
    0x0023, 0x0024, 0x0025, 0x0026, 0x0028, 0x0029, 0x002a, 0x002a,
    0x002b, 0x002b, 0x002d, 0x002e, 0x002f, 0x003a, 0x003c, 0x003e,
    0x003f, 0x0040, 0x005e, 0x005f, 0x0061, 0x0062, 0x0063, 0x0064,
    0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c,
    0x006d, 0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
    0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c,
    0x007c, 0x007d, 0x007e, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shift[] {
        14,     15,    100,     28,      1,     57,      2,     40,
         4,      5,      6,      8,     10,     11,      9,     55,
        78,     13,     74,     83,     98,     39,     51,     52,
        53,      3,      7,     12,     30,     48,     46,     32,
        18,     33,     34,     35,     23,     36,     37,     38,
        50,     49,     24,     25,     16,     19,     31,     20,
        22,     47,     17,     45,     21,     44,     26,     43,
        86,     27,     41, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shift{uchar_capslock_shift, scancode_capslock_shift, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x00a1, 0x00a4, 0x00a5, 0x00a9, 0x00ab, 0x00ac, 0x00ae,
    0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00bb, 0x00bc, 0x00bd,
    0x00be, 0x00bf, 0x00d7, 0x00df, 0x00e1, 0x00e4, 0x00e5, 0x00e6,
    0x00e7, 0x00e9, 0x00ed, 0x00f0, 0x00f1, 0x00f3, 0x00f6, 0x00f8,
    0x00fa, 0x00fc, 0x00fe, 0x2018, 0x2019, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_altGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,      2,      5,     12,     46,     26,     43,     19,
         3,      4,     40,     50,     39,     27,      7,      8,
         9,     53,     13,     31,     30,     16,     17,     44,
        51,     18,     23,     32,     49,     24,     25,     38,
        22,     21,     20,     10,     11,      6, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_altGr{uchar_capslock_altGr, scancode_capslock_altGr, 46};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shiftAltGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x00a2, 0x00a3, 0x00a6, 0x00a7, 0x00a8, 0x00b0, 0x00b9,
    0x00c1, 0x00c4, 0x00c5, 0x00c6, 0x00c7, 0x00c9, 0x00cd, 0x00d0,
    0x00d1, 0x00d3, 0x00d6, 0x00d8, 0x00da, 0x00dc, 0x00de, 0x00f7,

};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shiftAltGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,     46,      5,     43,     31,     40,     39,      2,
        30,     16,     17,     44,     51,     18,     23,     32,
        49,     24,     25,     38,     22,     21,     20,     13,

};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shiftAltGr{uchar_capslock_shiftAltGr, scancode_capslock_shiftAltGr, 32};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_ctrl[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x001b, 0x001c, 0x001c,
    0x001d, 0x0020, 0x002a, 0x002b, 0x002d, 0x002f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_ctrl[] {
        14,     15,     28,    100,      1,     26,     43,     86,
        27,     57,     55,     78,     74,     98, 
};
constexpr Keylayout_r::KeyLayoutMap_t ctrl{uchar_ctrl, scancode_ctrl, 14};

const Keylayout_r::KeyLayoutMap_t deadkeys {
};


const static uint8_t nbDeadkeys = 5;

}

static const Keylayout_r keylayout_x80050408(
    x80050408::LCID,
    x80050408::locale_name,
    x80050408::noMod,
    x80050408::shift,
    x80050408::altGr,
    x80050408::shiftAltGr,
    x80050408::ctrl,
    x80050408::capslock_noMod,
    x80050408::capslock_shift,
    x80050408::capslock_altGr,
    x80050408::capslock_shiftAltGr,
    x80050408::deadkeys,
    x80050408::nbDeadkeys
);

