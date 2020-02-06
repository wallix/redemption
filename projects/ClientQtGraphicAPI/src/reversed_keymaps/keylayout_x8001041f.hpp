#pragma once

#include "keylayout_r.hpp"

namespace x8001041f
{

const static int LCID = 0x1041f;

const static char * const locale_name = "tr-TR.f";

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x002a, 0x002b,
    0x002b, 0x002c, 0x002c, 0x002d, 0x002d, 0x002e, 0x002e, 0x002f,
    0x002f, 0x0030, 0x0030, 0x0031, 0x0031, 0x0032, 0x0032, 0x0033,
    0x0033, 0x0034, 0x0034, 0x0035, 0x0035, 0x0036, 0x0036, 0x0037,
    0x0037, 0x0038, 0x0038, 0x0039, 0x0039, 0x003c, 0x0061, 0x0062,
    0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a,
    0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 0x0070, 0x0071, 0x0072,
    0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a,
    0x00e7, 0x00f6, 0x00fc, 0x011f, 0x0131, 0x015f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_noMod[] {
        14,     15,     28,    100,      1,     57,     55,     78,
        41,     83,     53,     74,     13,    126,     52,     12,
        98,     82,     11,     79,      2,      3,     80,     81,
         4,      5,     75,      6,     76,      7,     77,     71,
         8,     72,      9,     10,     73,     86,     33,     51,
        47,     21,     32,     16,     17,     24,     31,     44,
        36,     38,     37,     23,     20,     25,     26,     22,
        50,     35,     30,     46,     27,     43,     39,     49,
        48,     45,     34,     18,     19,     40, 
};
constexpr Keylayout_r::KeyLayoutMap_t noMod{uchar_noMod, scancode_noMod, 70};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shift[] {
    0x0008, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022, 0x0024,
    0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002a, 0x002b,
    0x002c, 0x002d, 0x002e, 0x002f, 0x003a, 0x003b, 0x003d, 0x003e,
    0x003f, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005a, 0x005e, 0x005f, 0x007f, 0x00c7, 0x00d6,
    0x00dc, 0x011e, 0x0130, 0x015e, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shift[] {
        14,     28,    100,      1,     57,      2,      3,      5,
         6,      7,      8,      9,     10,     55,     41,     78,
        83,     74,    126,    104,     52,     53,     11,     86,
        12,     33,     51,     47,     21,     32,     16,     17,
        24,     19,     44,     36,     38,     37,     23,     20,
        25,     26,     22,     50,     35,     30,     46,     27,
        43,     39,     49,      4,     13,     99,     48,     45,
        34,     18,     31,     40, 
};
constexpr Keylayout_r::KeyLayoutMap_t shift{uchar_shift, scancode_shift, 60};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0023, 0x002a, 0x002b,
    0x002d, 0x002f, 0x0040, 0x005b, 0x005c, 0x005d, 0x0060, 0x007b,
    0x007c, 0x007c, 0x007d, 0x007e, 0x00a2, 0x00a3, 0x00a5, 0x00a8,
    0x00ab, 0x00ac, 0x00ad, 0x00b2, 0x00b4, 0x00b5, 0x00b6, 0x00b9,
    0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00d7, 0x00df, 0x00e6, 0x00f7,
    0x00f8, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_altGr[] {
        14,     15,    100,     28,      1,      4,     55,     78,
        74,     98,     16,      9,     12,     10,     43,      8,
        13,     86,     11,     27,     46,     25,     21,     26,
        44,     41,     53,      3,     39,     50,     19,      2,
        45,      5,      6,      7,     51,     31,     30,     52,
        24,     32, 
};
constexpr Keylayout_r::KeyLayoutMap_t altGr{uchar_altGr, scancode_altGr, 42};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shiftAltGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x003c, 0x003e, 0x00a1, 0x00a4, 0x00a6, 0x00a7, 0x00a9,
    0x00aa, 0x00ae, 0x00b3, 0x00ba, 0x00bf, 0x00c6, 0x00d8, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shiftAltGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,     44,     45,      2,      5,     86,     31,     46,
        33,     19,      4,     50,     12,     30,     24, 
};
constexpr Keylayout_r::KeyLayoutMap_t shiftAltGr{uchar_shiftAltGr, scancode_shiftAltGr, 23};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x002a, 0x002b,
    0x002b, 0x002c, 0x002c, 0x002d, 0x002d, 0x002e, 0x002e, 0x002f,
    0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036,
    0x0037, 0x0038, 0x0039, 0x003c, 0x0041, 0x0042, 0x0043, 0x0044,
    0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c,
    0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054,
    0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x00c7, 0x00d6,
    0x00dc, 0x011e, 0x0130, 0x015e, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_noMod[] {
        14,     15,     28,    100,      1,     57,     55,     78,
        41,     83,     53,     74,     13,    126,     52,     12,
        98,     11,      2,      3,      4,      5,      6,      7,
         8,      9,     10,     86,     33,     51,     47,     21,
        32,     16,     17,     24,     19,     44,     36,     38,
        37,     23,     20,     25,     26,     22,     50,     35,
        30,     46,     27,     43,     39,     49,     48,     45,
        34,     18,     31,     40, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_noMod{uchar_capslock_noMod, scancode_capslock_noMod, 60};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shift[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022,
    0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002a,
    0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 0x003a, 0x003b, 0x003d,
    0x003e, 0x003f, 0x005e, 0x005f, 0x0061, 0x0062, 0x0063, 0x0064,
    0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c,
    0x006d, 0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
    0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x00e7, 0x00f6,
    0x00fc, 0x011f, 0x0131, 0x015f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shift[] {
        14,     15,    100,     28,      1,     57,      2,      3,
         5,      6,      7,      8,      9,     10,     55,     41,
        78,     83,     74,    126,     98,     52,     53,     11,
        86,     12,      4,     13,     33,     51,     47,     21,
        32,     16,     17,     24,     31,     44,     36,     38,
        37,     23,     20,     25,     26,     22,     50,     35,
        30,     46,     27,     43,     39,     49,     48,     45,
        34,     18,     19,     40, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shift{uchar_capslock_shift, scancode_capslock_shift, 60};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0023, 0x002a, 0x002b,
    0x002d, 0x002f, 0x0040, 0x005b, 0x005c, 0x005d, 0x0060, 0x007b,
    0x007c, 0x007c, 0x007d, 0x007e, 0x00a2, 0x00a3, 0x00a5, 0x00a7,
    0x00a8, 0x00ab, 0x00ac, 0x00ad, 0x00b2, 0x00b4, 0x00b5, 0x00b6,
    0x00b9, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00d7, 0x00d8, 0x00e6,
    0x00f7, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_altGr[] {
        14,     15,    100,     28,      1,      4,     55,     78,
        74,     98,     16,      9,     12,     10,     43,      8,
        13,     86,     11,     27,     46,     25,     21,     31,
        26,     44,     41,     53,      3,     39,     50,     19,
         2,     45,      5,      6,      7,     51,     24,     30,
        52,     32, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_altGr{uchar_capslock_altGr, scancode_capslock_altGr, 42};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shiftAltGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x003c, 0x003e, 0x00a1, 0x00a4, 0x00a6, 0x00a9, 0x00aa,
    0x00ae, 0x00b3, 0x00ba, 0x00bf, 0x00c6, 0x00df, 0x00f8, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shiftAltGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,     44,     45,      2,      5,     86,     46,     33,
        19,      4,     50,     12,     30,     31,     24, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shiftAltGr{uchar_capslock_shiftAltGr, scancode_capslock_shiftAltGr, 23};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_ctrl[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x001b, 0x001c, 0x001c,
    0x001d, 0x0020, 0x002a, 0x002b, 0x002d, 0x002f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_ctrl[] {
        14,     15,     28,    100,      1,      9,     12,     45,
        34,     57,     55,     78,     74,     98, 
};
constexpr Keylayout_r::KeyLayoutMap_t ctrl{uchar_ctrl, scancode_ctrl, 14};

const Keylayout_r::KeyLayoutMap_t deadkeys {
};


const static uint8_t nbDeadkeys = 5;

}

static const Keylayout_r keylayout_x8001041f(
    x8001041f::LCID,
    x8001041f::locale_name,
    x8001041f::noMod,
    x8001041f::shift,
    x8001041f::altGr,
    x8001041f::shiftAltGr,
    x8001041f::ctrl,
    x8001041f::capslock_noMod,
    x8001041f::capslock_shift,
    x8001041f::capslock_altGr,
    x8001041f::capslock_shiftAltGr,
    x8001041f::deadkeys,
    x8001041f::nbDeadkeys
);

