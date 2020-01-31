#pragma once

#include "keylayout_r.hpp"

namespace x8001040a
{

const static int LCID = 0x1040a;

const static char * const locale_name = "es-ES.variation";

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0027, 0x002a,
    0x002b, 0x002c, 0x002d, 0x002d, 0x002e, 0x002e, 0x002e, 0x002f,
    0x0030, 0x0030, 0x0031, 0x0031, 0x0032, 0x0032, 0x0033, 0x0033,
    0x0034, 0x0034, 0x0035, 0x0035, 0x0036, 0x0036, 0x0037, 0x0037,
    0x0038, 0x0038, 0x0039, 0x0039, 0x003c, 0x003d, 0x0060, 0x0061,
    0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069,
    0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 0x0070, 0x0071,
    0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079,
    0x007a, 0x00a8, 0x00b4, 0x00e7, 0x00f1, 0x00f7, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_noMod[] {
        14,     15,     28,    100,      1,     57,     41,     55,
        78,     51,     74,     12,     83,    126,     52,     98,
        82,     11,     79,      2,      3,     80,     81,      4,
         5,     75,      6,     76,      7,     77,      8,     71,
         9,     72,     10,     73,     86,     53,     27,     30,
        48,     46,     32,     18,     33,     34,     35,     23,
        36,     37,     38,     50,     49,     24,     25,     16,
        19,     31,     20,     22,     47,     17,     45,     21,
        44,     13,     43,     40,     39,     26, 
};
constexpr Keylayout_r::KeyLayoutMap_t noMod{uchar_noMod, scancode_noMod, 70};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shift[] {
    0x0008, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022, 0x0025,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002b, 0x002d, 0x002e, 0x002e,
    0x002f, 0x002f, 0x003a, 0x003b, 0x003e, 0x003f, 0x0041, 0x0042,
    0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a,
    0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052,
    0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a,
    0x0060, 0x007f, 0x00a1, 0x00a8, 0x00aa, 0x00b4, 0x00b7, 0x00bf,
    0x00c7, 0x00d1, 0x00d7, 0x20a7, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shift[] {
        14,     28,    100,      1,     57,      8,      3,     53,
         5,      6,     55,     12,     78,     74,    126,     83,
       104,      4,     52,     51,     86,     10,     30,     48,
        46,     32,     18,     33,     34,     35,     23,     36,
        37,     38,     50,     49,     24,     25,     16,     19,
        31,     20,     22,     47,     17,     45,     21,     44,
        27,     99,      7,     13,      2,     43,     41,      9,
        40,     39,     26,     11, 
};
constexpr Keylayout_r::KeyLayoutMap_t shift{uchar_shift, scancode_shift, 60};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0023, 0x0023, 0x0024,
    0x0026, 0x002a, 0x002a, 0x002b, 0x002d, 0x002f, 0x0040, 0x0040,
    0x005b, 0x005b, 0x005c, 0x005c, 0x005d, 0x005d, 0x005e, 0x005f,
    0x007b, 0x007c, 0x007c, 0x007d, 0x007e, 0x007e, 0x00a3, 0x00a7,
    0x00ac, 0x00b1, 0x00bc, 0x00bd, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_altGr[] {
        14,     15,     28,    100,      1,      9,      4,     30,
        31,     12,     55,     78,     74,     98,      3,     32,
        26,     33,     11,     41,     34,     27,     52,      8,
        40,      2,     35,     43,     39,     13,     36,     10,
         7,     37,      5,      6,     18, 
};
constexpr Keylayout_r::KeyLayoutMap_t altGr{uchar_altGr, scancode_altGr, 37};

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
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0027, 0x002a,
    0x002b, 0x002c, 0x002d, 0x002d, 0x002e, 0x002e, 0x002e, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003c, 0x003d, 0x0041, 0x0042, 0x0043, 0x0044,
    0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c,
    0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054,
    0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x0060, 0x00a8,
    0x00b4, 0x00c7, 0x00d1, 0x00d7, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_noMod[] {
        14,     15,     28,    100,      1,     57,     41,     55,
        78,     51,     74,     12,     83,     52,    126,     98,
        11,      2,      3,      4,      5,      6,      7,      8,
         9,     10,     86,     53,     30,     48,     46,     32,
        18,     33,     34,     35,     23,     36,     37,     38,
        50,     49,     24,     25,     16,     19,     31,     20,
        22,     47,     17,     45,     21,     44,     27,     13,
        43,     40,     39,     26, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_noMod{uchar_capslock_noMod, scancode_capslock_noMod, 60};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shift[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022,
    0x0025, 0x0028, 0x0029, 0x002a, 0x002b, 0x002b, 0x002d, 0x002e,
    0x002e, 0x002f, 0x002f, 0x003a, 0x003b, 0x003e, 0x003f, 0x0060,
    0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068,
    0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 0x0070,
    0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078,
    0x0079, 0x007a, 0x00a1, 0x00a8, 0x00aa, 0x00b4, 0x00b7, 0x00bf,
    0x00e7, 0x00f1, 0x00f7, 0x20a7, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shift[] {
        14,     15,     28,    100,      1,     57,      8,      3,
        53,      5,      6,     55,     12,     78,     74,    126,
        83,     98,      4,     52,     51,     86,     10,     27,
        30,     48,     46,     32,     18,     33,     34,     35,
        23,     36,     37,     38,     50,     49,     24,     25,
        16,     19,     31,     20,     22,     47,     17,     45,
        21,     44,      7,     13,      2,     43,     41,      9,
        40,     39,     26,     11, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shift{uchar_capslock_shift, scancode_capslock_shift, 60};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0023, 0x0023, 0x0024,
    0x0026, 0x002a, 0x002a, 0x002b, 0x002d, 0x002f, 0x0040, 0x0040,
    0x005b, 0x005b, 0x005c, 0x005c, 0x005d, 0x005d, 0x005e, 0x005f,
    0x007b, 0x007c, 0x007c, 0x007d, 0x007e, 0x007e, 0x00a3, 0x00a7,
    0x00ac, 0x00b1, 0x00bc, 0x00bd, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_altGr[] {
        14,     15,     28,    100,      1,      9,      4,     30,
        31,     12,     55,     78,     74,     98,      3,     32,
        26,     33,     11,     41,     34,     27,     52,      8,
        40,      2,     35,     43,     39,     13,     36,     10,
         7,     37,      5,      6,     18, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_altGr{uchar_capslock_altGr, scancode_capslock_altGr, 37};

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

static const Keylayout_r keylayout_x8001040a(
    x8001040a::LCID,
    x8001040a::locale_name,
    x8001040a::noMod,
    x8001040a::shift,
    x8001040a::altGr,
    x8001040a::shiftAltGr,
    x8001040a::ctrl,
    x8001040a::capslock_noMod,
    x8001040a::capslock_shift,
    x8001040a::capslock_altGr,
    x8001040a::capslock_shiftAltGr,
    x8001040a::deadkeys,
    x8001040a::nbDeadkeys
);

