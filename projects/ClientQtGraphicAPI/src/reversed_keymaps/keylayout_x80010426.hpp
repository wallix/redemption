#pragma once

#include "keylayout_r.hpp"

namespace x80010426
{

const static int LCID = 0x10426;

const static char * const locale_name = "lv-LV.qwerty";

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0027, 0x002a,
    0x002b, 0x002c, 0x002d, 0x002d, 0x002e, 0x002e, 0x002f, 0x002f,
    0x0030, 0x0030, 0x0031, 0x0031, 0x0032, 0x0032, 0x0033, 0x0033,
    0x0034, 0x0034, 0x0035, 0x0035, 0x0036, 0x0036, 0x0037, 0x0037,
    0x0038, 0x0038, 0x0039, 0x0039, 0x003b, 0x003d, 0x005b, 0x005c,
    0x005d, 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066,
    0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e,
    0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076,
    0x0077, 0x0078, 0x0079, 0x007a, 0x00b0, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_noMod[] {
        14,     15,     28,    100,      1,     57,     40,     55,
        78,     51,     74,     12,     83,     52,     98,     53,
        82,     11,     79,      2,      3,     80,      4,     81,
         5,     75,     76,      6,      7,     77,      8,     71,
         9,     72,     10,     73,     39,     13,     26,     86,
        27,     41,     30,     48,     46,     32,     18,     33,
        34,     35,     23,     36,     37,     38,     50,     49,
        24,     25,     16,     19,     31,     20,     22,     47,
        17,     45,     21,     44,     43, 
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
    0x002f, 0x00a0, 0x00ab, 0x00ad, 0x00b4, 0x00bb, 0x00f5, 0x0101,
    0x010d, 0x0113, 0x0123, 0x012b, 0x0137, 0x013c, 0x0146, 0x0157,
    0x0161, 0x016b, 0x017e, 0x2013, 0x2019, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_altGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,      2,      3,     41,     40,      4,     24,     30,
        46,     18,     34,     23,     37,     38,     49,     19,
        31,     22,     44,     12,      7,      5, 
};
constexpr Keylayout_r::KeyLayoutMap_t altGr{uchar_altGr, scancode_altGr, 30};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shiftAltGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x00a7, 0x00a8, 0x00b0, 0x00b1, 0x00d5, 0x00d7, 0x0100,
    0x010c, 0x0112, 0x0122, 0x012a, 0x0136, 0x013b, 0x0145, 0x0156,
    0x0160, 0x016a, 0x017d, 0x2014, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shiftAltGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,      5,     40,      6,      8,     24,      9,     30,
        46,     18,     34,     23,     37,     38,     49,     19,
        31,     22,     44,     12, 
};
constexpr Keylayout_r::KeyLayoutMap_t shiftAltGr{uchar_shiftAltGr, scancode_shiftAltGr, 28};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0027, 0x002a,
    0x002b, 0x002c, 0x002d, 0x002d, 0x002e, 0x002e, 0x002f, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003b, 0x003d, 0x0041, 0x0042, 0x0043, 0x0044,
    0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c,
    0x004d, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054,
    0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c,
    0x005d, 0x0060, 0x00b0, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_noMod[] {
        14,     15,     28,    100,      1,     57,     40,     55,
        78,     51,     74,     12,     52,     83,     53,     98,
        11,      2,      3,      4,      5,      6,      7,      8,
         9,     10,     39,     13,     30,     48,     46,     32,
        18,     33,     34,     35,     23,     36,     37,     38,
        50,     49,     24,     25,     16,     19,     31,     20,
        22,     47,     17,     45,     21,     44,     26,     86,
        27,     41,     43, 
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
    0x002f, 0x00a0, 0x00ab, 0x00ad, 0x00b4, 0x00bb, 0x00f5, 0x0101,
    0x010d, 0x0113, 0x0123, 0x012b, 0x0137, 0x013c, 0x0146, 0x0157,
    0x0161, 0x016b, 0x017e, 0x2013, 0x2019, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_altGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,      2,      3,     41,     40,      4,     24,     30,
        46,     18,     34,     23,     37,     38,     49,     19,
        31,     22,     44,     12,      7,      5, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_altGr{uchar_capslock_altGr, scancode_capslock_altGr, 30};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shiftAltGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x00a7, 0x00a8, 0x00b0, 0x00b1, 0x00d5, 0x00d7, 0x0100,
    0x010c, 0x0112, 0x0122, 0x012a, 0x0136, 0x013b, 0x0145, 0x0156,
    0x0160, 0x016a, 0x017d, 0x2014, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shiftAltGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,      5,     40,      6,      8,     24,      9,     30,
        46,     18,     34,     23,     37,     38,     49,     19,
        31,     22,     44,     12, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shiftAltGr{uchar_capslock_shiftAltGr, scancode_capslock_shiftAltGr, 28};

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


const static uint8_t nbDeadkeys = 4;

}

static const Keylayout_r keylayout_x80010426(
    x80010426::LCID,
    x80010426::locale_name,
    x80010426::noMod,
    x80010426::shift,
    x80010426::altGr,
    x80010426::shiftAltGr,
    x80010426::ctrl,
    x80010426::capslock_noMod,
    x80010426::capslock_shift,
    x80010426::capslock_altGr,
    x80010426::capslock_shiftAltGr,
    x80010426::deadkeys,
    x80010426::nbDeadkeys
);

