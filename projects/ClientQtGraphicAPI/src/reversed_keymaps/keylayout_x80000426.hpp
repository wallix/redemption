#pragma once

#include "keylayout_r.hpp"

namespace x80000426
{

const static int LCID = 0x426;

const static char * const locale_name = "lv-LV";

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x002a, 0x002b,
    0x002c, 0x002c, 0x002d, 0x002d, 0x002e, 0x002f, 0x0030, 0x0030,
    0x0031, 0x0031, 0x0032, 0x0032, 0x0033, 0x0033, 0x0034, 0x0034,
    0x0035, 0x0035, 0x0036, 0x0036, 0x0037, 0x0037, 0x0038, 0x0038,
    0x0039, 0x0039, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066,
    0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e,
    0x006f, 0x0070, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x007a,
    0x00ad, 0x00b4, 0x0101, 0x010d, 0x0113, 0x0123, 0x012b, 0x0137,
    0x013c, 0x0146, 0x0161, 0x016b, 0x017e, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_noMod[] {
        14,     15,     28,    100,      1,     57,     55,     78,
        83,     51,     74,     12,     52,     98,     82,     11,
        79,      2,      3,     80,      4,     81,      5,     75,
         6,     76,     77,      7,      8,     71,      9,     72,
        10,     73,     36,     45,     39,     35,     38,     13,
        17,     27,     33,     18,     47,     34,     20,     22,
        49,     48,     19,     32,     37,     31,     21,     23,
        41,     40,     50,     25,     24,     86,     46,     43,
        53,     44,     30,     16,     26, 
};
constexpr Keylayout_r::KeyLayoutMap_t noMod{uchar_noMod, scancode_noMod, 69};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shift[] {
    0x0008, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0024, 0x0025,
    0x0026, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002f,
    0x002f, 0x003a, 0x003b, 0x003f, 0x0041, 0x0042, 0x0043, 0x0044,
    0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c,
    0x004d, 0x004e, 0x004f, 0x0050, 0x0052, 0x0053, 0x0054, 0x0055,
    0x0056, 0x005a, 0x005f, 0x007f, 0x00ab, 0x00b0, 0x00bb, 0x00d7,
    0x0100, 0x010c, 0x0112, 0x0122, 0x012a, 0x0136, 0x013b, 0x0145,
    0x0160, 0x016a, 0x017d, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shift[] {
        14,    100,     28,      1,     57,      2,      5,      6,
         8,     10,     11,     55,     78,     83,     74,    104,
         7,     52,     51,     41,     36,     45,     39,     35,
        38,     13,     17,     27,     33,     18,     47,     34,
        20,     22,     49,     48,     19,     32,     37,     31,
        21,     23,     12,     99,      3,     40,      4,      9,
        50,     25,     24,     86,     46,     43,     53,     44,
        30,     16,     26, 
};
constexpr Keylayout_r::KeyLayoutMap_t shift{uchar_shift, scancode_shift, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0022, 0x002a, 0x002b,
    0x002d, 0x002f, 0x003a, 0x003c, 0x003d, 0x003e, 0x005b, 0x005c,
    0x005d, 0x0071, 0x0077, 0x0078, 0x0079, 0x00ab, 0x00b4, 0x00f5,
    0x0123, 0x0137, 0x0157, 0x2013, 0x2019, 0x20ac, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_altGr[] {
        14,     15,    100,     28,      1,      6,     55,     78,
        74,     98,      9,     51,     13,     52,     26,     86,
        27,     16,     20,     45,     21,      2,     40,     49,
        17,     47,     19,     12,      7,     38,      5, 
};
constexpr Keylayout_r::KeyLayoutMap_t altGr{uchar_altGr, scancode_altGr, 31};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shiftAltGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0023, 0x0024, 0x002a,
    0x002b, 0x002d, 0x002f, 0x003b, 0x0040, 0x0051, 0x0057, 0x0058,
    0x0059, 0x005e, 0x007b, 0x007c, 0x007d, 0x007e, 0x00a8, 0x00b1,
    0x00d5, 0x0122, 0x0136, 0x0156, 0x2014, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shiftAltGr[] {
        14,     15,    100,     28,      1,      4,      5,     55,
        78,     74,     98,     13,      3,     16,     20,     45,
        21,      7,     26,     86,     27,      6,     40,      8,
        49,     17,     47,     19,     12, 
};
constexpr Keylayout_r::KeyLayoutMap_t shiftAltGr{uchar_shiftAltGr, scancode_shiftAltGr, 29};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x002a, 0x002b,
    0x002c, 0x002c, 0x002d, 0x002d, 0x002e, 0x002f, 0x0030, 0x0031,
    0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039,
    0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048,
    0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 0x0050,
    0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x005a, 0x00ad, 0x00b4,
    0x0100, 0x010c, 0x0112, 0x0122, 0x012a, 0x0136, 0x013b, 0x0145,
    0x0160, 0x016a, 0x017d, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_noMod[] {
        14,     15,    100,     28,      1,     57,     55,     78,
        83,     51,     74,     12,     52,     98,     11,      2,
         3,      4,      5,      6,      7,      8,      9,     10,
        36,     45,     39,     35,     38,     13,     17,     27,
        33,     18,     47,     34,     20,     22,     49,     48,
        19,     32,     37,     31,     21,     23,     41,     40,
        50,     25,     24,     86,     46,     43,     53,     44,
        30,     16,     26, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_noMod{uchar_capslock_noMod, scancode_capslock_noMod, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shift[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0024,
    0x0025, 0x0026, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d,
    0x002f, 0x002f, 0x003a, 0x003b, 0x003f, 0x005f, 0x0061, 0x0062,
    0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a,
    0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 0x0070, 0x0072, 0x0073,
    0x0074, 0x0075, 0x0076, 0x007a, 0x00ab, 0x00b0, 0x00bb, 0x00d7,
    0x0101, 0x010d, 0x0113, 0x0123, 0x012b, 0x0137, 0x013c, 0x0146,
    0x0161, 0x016b, 0x017e, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shift[] {
        14,     15,    100,     28,      1,     57,      2,      5,
         6,      8,     10,     11,     55,     78,     83,     74,
         7,     98,     52,     51,     41,     12,     36,     45,
        39,     35,     38,     13,     17,     27,     33,     18,
        47,     34,     20,     22,     49,     48,     19,     32,
        37,     31,     21,     23,      3,     40,      4,      9,
        50,     25,     24,     86,     46,     43,     53,     44,
        30,     16,     26, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shift{uchar_capslock_shift, scancode_capslock_shift, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0022, 0x002a, 0x002b,
    0x002d, 0x002f, 0x003a, 0x003c, 0x003d, 0x003e, 0x005b, 0x005c,
    0x005d, 0x0071, 0x0077, 0x0078, 0x0079, 0x00ab, 0x00b4, 0x00f5,
    0x0123, 0x0137, 0x0157, 0x2013, 0x2019, 0x20ac, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_altGr[] {
        14,     15,    100,     28,      1,      6,     55,     78,
        74,     98,      9,     51,     13,     52,     26,     86,
        27,     16,     20,     45,     21,      2,     40,     49,
        17,     47,     19,     12,      7,     38,      5, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_altGr{uchar_capslock_altGr, scancode_capslock_altGr, 31};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shiftAltGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0023, 0x0024, 0x002a,
    0x002b, 0x002d, 0x002f, 0x003b, 0x0040, 0x0051, 0x0057, 0x0058,
    0x0059, 0x005e, 0x007b, 0x007c, 0x007d, 0x007e, 0x00a8, 0x00b1,
    0x00d5, 0x0122, 0x0136, 0x0156, 0x2014, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shiftAltGr[] {
        14,     15,    100,     28,      1,      4,      5,     55,
        78,     74,     98,     13,      3,     16,     20,     45,
        21,      7,     26,     86,     27,      6,     40,      8,
        49,     17,     47,     19,     12, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shiftAltGr{uchar_capslock_shiftAltGr, scancode_capslock_shiftAltGr, 29};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_ctrl[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x001b, 0x001c, 0x001c,
    0x001d, 0x0020, 0x002a, 0x002b, 0x002d, 0x002f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_ctrl[] {
        14,     15,     28,    100,      1,     44,     43,     86,
        46,     57,     55,     78,     74,     98, 
};
constexpr Keylayout_r::KeyLayoutMap_t ctrl{uchar_ctrl, scancode_ctrl, 14};

const Keylayout_r::KeyLayoutMap_t deadkeys {
};


const static uint8_t nbDeadkeys = 4;

}

static const Keylayout_r keylayout_x80000426(
    x80000426::LCID,
    x80000426::locale_name,
    x80000426::noMod,
    x80000426::shift,
    x80000426::altGr,
    x80000426::shiftAltGr,
    x80000426::ctrl,
    x80000426::capslock_noMod,
    x80000426::capslock_shift,
    x80000426::capslock_altGr,
    x80000426::capslock_shiftAltGr,
    x80000426::deadkeys,
    x80000426::nbDeadkeys
);

