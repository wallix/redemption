#pragma once

#include "keylayout_r.hpp"

namespace x80000444
{

const static int LCID = 0x444;

const static char * const locale_name = "tt-RU";

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x002a, 0x002b,
    0x002c, 0x002d, 0x002d, 0x002e, 0x002e, 0x002f, 0x0030, 0x0030,
    0x0031, 0x0031, 0x0032, 0x0032, 0x0033, 0x0033, 0x0034, 0x0034,
    0x0035, 0x0035, 0x0036, 0x0036, 0x0037, 0x0037, 0x0038, 0x0038,
    0x0039, 0x0039, 0x003d, 0x005c, 0x0430, 0x0431, 0x0432, 0x0433,
    0x0434, 0x0435, 0x0437, 0x0438, 0x0439, 0x043a, 0x043b, 0x043c,
    0x043d, 0x043e, 0x043f, 0x0440, 0x0441, 0x0442, 0x0443, 0x0444,
    0x0445, 0x0447, 0x0448, 0x044b, 0x044d, 0x044e, 0x044f, 0x0491,
    0x0497, 0x04a3, 0x04af, 0x04bb, 0x04d9, 0x04e9, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_noMod[] {
        14,     15,     28,    100,      1,     57,     55,     78,
        83,     74,     12,    126,     53,     98,     82,     11,
        79,      2,      3,     80,      4,     81,      5,     75,
         6,     76,      7,     77,     71,      8,     72,      9,
        10,     73,     13,     43,     33,     51,     32,     22,
        38,     20,     25,     48,     16,     19,     37,     47,
        21,     36,     34,     35,     46,     49,     18,     30,
        26,     45,     23,     31,     40,     52,     44,     86,
        50,     39,     27,     41,     24,     17, 
};
constexpr Keylayout_r::KeyLayoutMap_t noMod{uchar_noMod, scancode_noMod, 70};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shift[] {
    0x0008, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022, 0x0025,
    0x0028, 0x0029, 0x002a, 0x002a, 0x002b, 0x002b, 0x002c, 0x002c,
    0x002d, 0x002e, 0x002f, 0x002f, 0x003a, 0x003b, 0x003f, 0x005f,
    0x007f, 0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0417,
    0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0427, 0x0428,
    0x042b, 0x042d, 0x042e, 0x042f, 0x0490, 0x0496, 0x04a2, 0x04ae,
    0x04ba, 0x04d8, 0x04e8, 0x2116, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shift[] {
        14,     28,    100,      1,     57,      2,      3,      6,
        10,     11,      9,     55,     78,     13,     83,     53,
        74,    126,     43,    104,      7,      5,      8,     12,
        99,     33,     51,     32,     22,     38,     20,     25,
        48,     16,     19,     37,     47,     21,     36,     34,
        35,     46,     49,     18,     30,     26,     45,     23,
        31,     40,     52,     44,     86,     50,     39,     27,
        41,     24,     17,      4, 
};
constexpr Keylayout_r::KeyLayoutMap_t shift{uchar_shift, scancode_shift, 60};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0023, 0x0024, 0x0027,
    0x002a, 0x002b, 0x002d, 0x002f, 0x003c, 0x003e, 0x0040, 0x005b,
    0x005d, 0x007b, 0x007d, 0x0436, 0x0446, 0x0449, 0x044a, 0x044c,
    0x0451, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_altGr[] {
        14,     15,    100,     28,      1,      4,      5,     40,
        55,     78,     74,     98,     51,     52,      3,      8,
         9,     10,     11,     39,     17,     24,     27,     50,
        41, 
};
constexpr Keylayout_r::KeyLayoutMap_t altGr{uchar_altGr, scancode_altGr, 25};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shiftAltGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x0401, 0x0416, 0x0426, 0x0429, 0x042a, 0x042c, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shiftAltGr[] {
        14,     15,     28,    100,      1,     55,     78,     74,
        98,     41,     39,     17,     24,     27,     50, 
};
constexpr Keylayout_r::KeyLayoutMap_t shiftAltGr{uchar_shiftAltGr, scancode_shiftAltGr, 15};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x002a, 0x002b,
    0x002c, 0x002d, 0x002d, 0x002e, 0x002e, 0x002f, 0x0030, 0x0031,
    0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039,
    0x003d, 0x005c, 0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415,
    0x0417, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e,
    0x041f, 0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0427,
    0x0428, 0x042b, 0x042d, 0x042e, 0x042f, 0x0490, 0x0496, 0x04a2,
    0x04ae, 0x04ba, 0x04d8, 0x04e8, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_noMod[] {
        14,     15,    100,     28,      1,     57,     55,     78,
        83,     74,     12,    126,     53,     98,     11,      2,
         3,      4,      5,      6,      7,      8,      9,     10,
        13,     43,     33,     51,     32,     22,     38,     20,
        25,     48,     16,     19,     37,     47,     21,     36,
        34,     35,     46,     49,     18,     30,     26,     45,
        23,     31,     40,     52,     44,     86,     50,     39,
        27,     41,     24,     17, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_noMod{uchar_capslock_noMod, scancode_capslock_noMod, 60};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shift[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022,
    0x0025, 0x0028, 0x0029, 0x002a, 0x002a, 0x002b, 0x002b, 0x002c,
    0x002c, 0x002d, 0x002e, 0x002f, 0x002f, 0x003a, 0x003b, 0x003f,
    0x005f, 0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0437,
    0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f,
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0447, 0x0448,
    0x044b, 0x044d, 0x044e, 0x044f, 0x0491, 0x0497, 0x04a3, 0x04af,
    0x04bb, 0x04d9, 0x04e9, 0x2116, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shift[] {
        14,     15,     28,    100,      1,     57,      2,      3,
         6,     10,     11,      9,     55,     78,     13,     83,
        53,     74,    126,     43,     98,      7,      5,      8,
        12,     33,     51,     32,     22,     38,     20,     25,
        48,     16,     19,     37,     47,     21,     36,     34,
        35,     46,     49,     18,     30,     26,     45,     23,
        31,     40,     52,     44,     86,     50,     39,     27,
        41,     24,     17,      4, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shift{uchar_capslock_shift, scancode_capslock_shift, 60};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0023, 0x0024, 0x0027,
    0x002a, 0x002b, 0x002d, 0x002f, 0x003c, 0x003e, 0x0040, 0x005b,
    0x005d, 0x007b, 0x007d, 0x0436, 0x0446, 0x0449, 0x044a, 0x044c,
    0x0451, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_altGr[] {
        14,     15,    100,     28,      1,      4,      5,     40,
        55,     78,     74,     98,     51,     52,      3,      8,
         9,     10,     11,     39,     17,     24,     27,     50,
        41, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_altGr{uchar_capslock_altGr, scancode_capslock_altGr, 25};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shiftAltGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x0401, 0x0416, 0x0426, 0x0429, 0x042a, 0x042c, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shiftAltGr[] {
        14,     15,     28,    100,      1,     55,     78,     74,
        98,     41,     39,     17,     24,     27,     50, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shiftAltGr{uchar_capslock_shiftAltGr, scancode_capslock_shiftAltGr, 15};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_ctrl[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x001b, 0x001c, 0x001c,
    0x001d, 0x0020, 0x002a, 0x002b, 0x002d, 0x002f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_ctrl[] {
        14,     15,     28,    100,      1,     27,     43,     86,
        39,     57,     55,     78,     74,     98, 
};
constexpr Keylayout_r::KeyLayoutMap_t ctrl{uchar_ctrl, scancode_ctrl, 14};

const Keylayout_r::KeyLayoutMap_t deadkeys {
};


const static uint8_t nbDeadkeys = 0;

}

static const Keylayout_r keylayout_x80000444(
    x80000444::LCID,
    x80000444::locale_name,
    x80000444::noMod,
    x80000444::shift,
    x80000444::altGr,
    x80000444::shiftAltGr,
    x80000444::ctrl,
    x80000444::capslock_noMod,
    x80000444::capslock_shift,
    x80000444::capslock_altGr,
    x80000444::capslock_shiftAltGr,
    x80000444::deadkeys,
    x80000444::nbDeadkeys
);

