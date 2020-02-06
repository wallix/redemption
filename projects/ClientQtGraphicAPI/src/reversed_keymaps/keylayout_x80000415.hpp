#pragma once

#include "keylayout_r.hpp"

namespace x80000415
{

const static int LCID = 0x415;

const static char * const locale_name = "pl-PL.programmers";

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
    0x002b, 0x002c, 0x002d, 0x002f, 0x003a, 0x003c, 0x003e, 0x003f,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005a, 0x005e, 0x005f, 0x007b, 0x007c, 0x007c,
    0x007d, 0x007e, 0x007f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shift[] {
        14,     28,    100,      1,     57,      2,     40,      4,
         5,      6,      8,     10,     11,      9,     55,     13,
        78,     83,     74,    104,     39,     51,     52,     53,
         3,     30,     48,     46,     32,     18,     33,     34,
        35,     23,     36,     37,     38,     50,     49,     24,
        25,     16,     19,     31,     20,     22,     47,     17,
        45,     21,     44,      7,     12,     26,     86,     43,
        27,     41,     99, 
};
constexpr Keylayout_r::KeyLayoutMap_t shift{uchar_shift, scancode_shift, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x00f3, 0x0105, 0x0107, 0x0119, 0x0142, 0x0144, 0x015b,
    0x017a, 0x017c, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_altGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,     24,     30,     46,     18,     38,     49,     31,
        45,     44,     22, 
};
constexpr Keylayout_r::KeyLayoutMap_t altGr{uchar_altGr, scancode_altGr, 19};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shiftAltGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x00d3, 0x0104, 0x0106, 0x0118, 0x0141, 0x0143, 0x015a,
    0x0179, 0x017b, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shiftAltGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,     24,     30,     46,     18,     38,     49,     31,
        45,     44, 
};
constexpr Keylayout_r::KeyLayoutMap_t shiftAltGr{uchar_shiftAltGr, scancode_shiftAltGr, 18};

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
    0x002b, 0x002b, 0x002c, 0x002d, 0x002f, 0x003a, 0x003c, 0x003e,
    0x003f, 0x0040, 0x005e, 0x005f, 0x0061, 0x0062, 0x0063, 0x0064,
    0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c,
    0x006d, 0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
    0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x007c,
    0x007c, 0x007d, 0x007e, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shift[] {
        14,     15,    100,     28,      1,     57,      2,     40,
         4,      5,      6,      8,     10,     11,      9,     55,
        78,     13,     83,     74,     98,     39,     51,     52,
        53,      3,      7,     12,     30,     48,     46,     32,
        18,     33,     34,     35,     23,     36,     37,     38,
        50,     49,     24,     25,     16,     19,     31,     20,
        22,     47,     17,     45,     21,     44,     26,     43,
        86,     27,     41, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shift{uchar_capslock_shift, scancode_capslock_shift, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x00d3, 0x0104, 0x0106, 0x0118, 0x0141, 0x0143, 0x015a,
    0x0179, 0x017b, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_altGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,     24,     30,     46,     18,     38,     49,     31,
        45,     44,     22, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_altGr{uchar_capslock_altGr, scancode_capslock_altGr, 19};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shiftAltGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x00f3, 0x0105, 0x0107, 0x0119, 0x0142, 0x0144, 0x015b,
    0x017a, 0x017c, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shiftAltGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,     24,     30,     46,     18,     38,     49,     31,
        45,     44, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shiftAltGr{uchar_capslock_shiftAltGr, scancode_capslock_shiftAltGr, 18};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_ctrl[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x001b, 0x001c, 0x001c,
    0x001d, 0x001d, 0x0020, 0x002a, 0x002b, 0x002d, 0x002f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_ctrl[] {
        14,     15,     28,    100,      1,     26,     43,     86,
        27,     39,     57,     55,     78,     74,     98, 
};
constexpr Keylayout_r::KeyLayoutMap_t ctrl{uchar_ctrl, scancode_ctrl, 15};

const Keylayout_r::KeyLayoutMap_t deadkeys {
};


const static uint8_t nbDeadkeys = 1;

}

static const Keylayout_r keylayout_x80000415(
    x80000415::LCID,
    x80000415::locale_name,
    x80000415::noMod,
    x80000415::shift,
    x80000415::altGr,
    x80000415::shiftAltGr,
    x80000415::ctrl,
    x80000415::capslock_noMod,
    x80000415::capslock_shift,
    x80000415::capslock_altGr,
    x80000415::capslock_shiftAltGr,
    x80000415::deadkeys,
    x80000415::nbDeadkeys
);

