#pragma once

#include "keylayout_r.hpp"

namespace x80000427
{

const static int LCID = 0x427;

const static char * const locale_name = "lt-LT.ibm";

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002b, 0x002c, 0x002c, 0x002d,
    0x002e, 0x002f, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034,
    0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003f,
    0x005c, 0x005f, 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065,
    0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d,
    0x006e, 0x006f, 0x0070, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076,
    0x0079, 0x007a, 0x007c, 0x0105, 0x010d, 0x0117, 0x0119, 0x012f,
    0x0161, 0x016b, 0x0173, 0x017e, 0x201c, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_noMod[] {
        14,     15,    100,     28,      1,     57,      2,      3,
        10,     11,     55,     13,     78,      7,     83,     74,
         8,     98,      4,     82,     79,     80,     81,     75,
        76,     77,     71,     72,     73,      6,      5,      9,
        86,     12,     41,     30,     48,     46,     32,     18,
        33,     34,     35,     23,     36,     37,     38,     50,
        49,     24,     25,     19,     31,     20,     22,     47,
        21,     44,     43,     16,     51,     40,     53,     26,
        52,     45,     39,     17,     27, 
};
constexpr Keylayout_r::KeyLayoutMap_t noMod{uchar_noMod, scancode_noMod, 69};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shift[] {
    0x0008, 0x000d, 0x000d, 0x001b, 0x0020, 0x002a, 0x002b, 0x002c,
    0x002d, 0x002d, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034,
    0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003d, 0x0041, 0x0042,
    0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a,
    0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 0x0050, 0x0052, 0x0053,
    0x0054, 0x0055, 0x0056, 0x0059, 0x005a, 0x005c, 0x007c, 0x007e,
    0x007f, 0x0104, 0x010c, 0x0116, 0x0118, 0x012e, 0x0160, 0x016a,
    0x0172, 0x017d, 0x201d, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shift[] {
        14,    100,     28,      1,     57,     55,     78,     83,
        74,     12,    104,     11,      2,      3,      4,      5,
         6,      7,      8,      9,     10,     13,     30,     48,
        46,     32,     18,     33,     34,     35,     23,     36,
        37,     38,     50,     49,     24,     25,     19,     31,
        20,     22,     47,     21,     44,     43,     86,     41,
        99,     16,     51,     40,     53,     26,     52,     45,
        39,     17,     27, 
};
constexpr Keylayout_r::KeyLayoutMap_t shift{uchar_shift, scancode_shift, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x005b, 0x005d, 0x007b, 0x007d, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_altGr[] {
        14,     15,     28,    100,      1,     55,     78,     74,
        98,      9,     10,      8,     11,     18, 
};
constexpr Keylayout_r::KeyLayoutMap_t altGr{uchar_altGr, scancode_altGr, 14};

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
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002b, 0x002c, 0x002c, 0x002d,
    0x002e, 0x002f, 0x002f, 0x003a, 0x003b, 0x003f, 0x0041, 0x0042,
    0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a,
    0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 0x0050, 0x0052, 0x0053,
    0x0054, 0x0055, 0x0056, 0x0059, 0x005a, 0x005c, 0x005f, 0x0060,
    0x007c, 0x0104, 0x010c, 0x0116, 0x0118, 0x012e, 0x0160, 0x016a,
    0x0172, 0x017d, 0x201c, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_noMod[] {
        14,     15,    100,     28,      1,     57,      2,      3,
        10,     11,     55,     13,     78,      7,     83,     74,
         8,     98,      4,      6,      5,      9,     30,     48,
        46,     32,     18,     33,     34,     35,     23,     36,
        37,     38,     50,     49,     24,     25,     19,     31,
        20,     22,     47,     21,     44,     86,     12,     41,
        43,     16,     51,     40,     53,     26,     52,     45,
        39,     17,     27, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_noMod{uchar_capslock_noMod, scancode_capslock_noMod, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shift[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x002a, 0x002b,
    0x002c, 0x002d, 0x002d, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033,
    0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003d, 0x005c,
    0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068,
    0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 0x0070,
    0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0079, 0x007a, 0x007c,
    0x007e, 0x0105, 0x010d, 0x0117, 0x0119, 0x012f, 0x0161, 0x016b,
    0x0173, 0x017e, 0x201d, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shift[] {
        14,     15,    100,     28,      1,     57,     55,     78,
        83,     74,     12,     98,     11,      2,      3,      4,
         5,      6,      7,      8,      9,     10,     13,     43,
        30,     48,     46,     32,     18,     33,     34,     35,
        23,     36,     37,     38,     50,     49,     24,     25,
        19,     31,     20,     22,     47,     21,     44,     86,
        41,     16,     51,     40,     53,     26,     52,     45,
        39,     17,     27, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shift{uchar_capslock_shift, scancode_capslock_shift, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x005b, 0x005d, 0x007b, 0x007d, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_altGr[] {
        14,     15,     28,    100,      1,     55,     78,     74,
        98,      9,     10,      8,     11,     18, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_altGr{uchar_capslock_altGr, scancode_capslock_altGr, 14};

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


const static uint8_t nbDeadkeys = 0;

}

static const Keylayout_r keylayout_x80000427(
    x80000427::LCID,
    x80000427::locale_name,
    x80000427::noMod,
    x80000427::shift,
    x80000427::altGr,
    x80000427::shiftAltGr,
    x80000427::ctrl,
    x80000427::capslock_noMod,
    x80000427::capslock_shift,
    x80000427::capslock_altGr,
    x80000427::capslock_shiftAltGr,
    x80000427::deadkeys,
    x80000427::nbDeadkeys
);

