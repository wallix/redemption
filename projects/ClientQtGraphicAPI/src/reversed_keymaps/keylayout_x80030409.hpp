#pragma once

#include "keylayout_r.hpp"

namespace x80030409
{

const static int LCID = 0x30409;

const static char * const locale_name = "en-US.dvorak_left";

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0027, 0x002a,
    0x002b, 0x002c, 0x002d, 0x002d, 0x002e, 0x002e, 0x002e, 0x002f,
    0x002f, 0x0030, 0x0030, 0x0031, 0x0031, 0x0032, 0x0032, 0x0033,
    0x0033, 0x0034, 0x0034, 0x0035, 0x0035, 0x0036, 0x0036, 0x0037,
    0x0037, 0x0038, 0x0038, 0x0039, 0x0039, 0x003b, 0x003d, 0x005b,
    0x005c, 0x005c, 0x005d, 0x0060, 0x0061, 0x0062, 0x0063, 0x0064,
    0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c,
    0x006d, 0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
    0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_noMod[] {
        14,     15,     28,    100,      1,     57,     44,     55,
        78,     51,     74,     30,     24,    126,     83,     98,
         4,     52,     82,     79,     13,     80,     12,     81,
        11,     75,     10,     26,     76,     25,     77,     71,
        40,     72,     39,     73,     53,     16,     27,      2,
        43,     86,      3,     41,     37,     18,     32,     33,
        36,      6,     46,     35,     50,      9,     31,      8,
         7,     49,     23,      5,     17,     21,     22,     34,
        20,     47,     48,     45,     19,     38, 
};
constexpr Keylayout_r::KeyLayoutMap_t noMod{uchar_noMod, scancode_noMod, 70};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shift[] {
    0x0008, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022, 0x0023,
    0x0024, 0x0025, 0x0026, 0x0028, 0x0029, 0x002a, 0x002a, 0x002b,
    0x002b, 0x002d, 0x002e, 0x002e, 0x002f, 0x003a, 0x003c, 0x003e,
    0x003f, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046,
    0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e,
    0x004f, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056,
    0x0057, 0x0058, 0x0059, 0x005a, 0x005e, 0x005f, 0x007b, 0x007c,
    0x007c, 0x007d, 0x007e, 0x007f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shift[] {
        14,     28,    100,      1,     57,     13,     44,     11,
        10,     26,     40,     53,     52,     39,     55,     78,
        27,     74,    126,     83,    104,     16,     51,     24,
         4,     12,     37,     18,     32,     33,     36,      6,
        46,     35,     50,      9,     31,      8,      7,     49,
        23,      5,     17,     21,     22,     34,     20,     47,
        48,     45,     19,     38,     25,     30,      2,     43,
        86,      3,     41,     99, 
};
constexpr Keylayout_r::KeyLayoutMap_t shift{uchar_shift, scancode_shift, 60};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_altGr[] {
        14,     15,     28,    100,      1,     55,     78,     74,
        98, 
};
constexpr Keylayout_r::KeyLayoutMap_t altGr{uchar_altGr, scancode_altGr, 9};

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
    0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036,
    0x0037, 0x0038, 0x0039, 0x003b, 0x003d, 0x0041, 0x0042, 0x0043,
    0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b,
    0x004c, 0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x0053,
    0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b,
    0x005c, 0x005c, 0x005d, 0x0060, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_noMod[] {
        14,     15,    100,     28,      1,     57,     44,     55,
        78,     51,     30,     74,    126,     24,     83,      4,
        98,     52,     13,     12,     11,     10,     26,     25,
        40,     39,     53,     16,     27,     37,     18,     32,
        33,     36,      6,     46,     35,     50,      9,     31,
         8,      7,     49,     23,      5,     17,     21,     22,
        34,     20,     47,     48,     45,     19,     38,      2,
        43,     86,      3,     41, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_noMod{uchar_capslock_noMod, scancode_capslock_noMod, 60};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shift[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022,
    0x0023, 0x0024, 0x0025, 0x0026, 0x0028, 0x0029, 0x002a, 0x002a,
    0x002b, 0x002b, 0x002d, 0x002e, 0x002e, 0x002f, 0x003a, 0x003c,
    0x003e, 0x003f, 0x0040, 0x005e, 0x005f, 0x0061, 0x0062, 0x0063,
    0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b,
    0x006c, 0x006d, 0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073,
    0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b,
    0x007c, 0x007c, 0x007d, 0x007e, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shift[] {
        14,     15,    100,     28,      1,     57,     13,     44,
        11,     10,     26,     40,     53,     52,     55,     39,
        78,     27,     74,    126,     83,     98,     16,     51,
        24,      4,     12,     25,     30,     37,     18,     32,
        33,     36,      6,     46,     35,     50,      9,     31,
         8,      7,     49,     23,      5,     17,     21,     22,
        34,     20,     47,     48,     45,     19,     38,      2,
        43,     86,      3,     41, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shift{uchar_capslock_shift, scancode_capslock_shift, 60};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_altGr[] {
        14,     15,     28,    100,      1,     55,     78,     74,
        98, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_altGr{uchar_capslock_altGr, scancode_capslock_altGr, 9};

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
        14,     15,     28,    100,      1,      2,     43,     86,
         3,     57,     55,     78,     74,     98, 
};
constexpr Keylayout_r::KeyLayoutMap_t ctrl{uchar_ctrl, scancode_ctrl, 14};

const Keylayout_r::KeyLayoutMap_t deadkeys {
};


const static uint8_t nbDeadkeys = 0;

}

static const Keylayout_r keylayout_x80030409(
    x80030409::LCID,
    x80030409::locale_name,
    x80030409::noMod,
    x80030409::shift,
    x80030409::altGr,
    x80030409::shiftAltGr,
    x80030409::ctrl,
    x80030409::capslock_noMod,
    x80030409::capslock_shift,
    x80030409::capslock_altGr,
    x80030409::capslock_shiftAltGr,
    x80030409::deadkeys,
    x80030409::nbDeadkeys
);

