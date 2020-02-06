#pragma once

#include "keylayout_r.hpp"

namespace x8000042f
{

const static int LCID = 0x42f;

const static char * const locale_name = "mk-MK";

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x002a, 0x002b,
    0x002c, 0x002c, 0x002d, 0x002d, 0x002e, 0x002f, 0x002f, 0x0030,
    0x0030, 0x0031, 0x0031, 0x0032, 0x0032, 0x0033, 0x0033, 0x0034,
    0x0034, 0x0035, 0x0035, 0x0036, 0x0036, 0x0037, 0x0037, 0x0038,
    0x0038, 0x0039, 0x0039, 0x003d, 0x0060, 0x0430, 0x0431, 0x0432,
    0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x043a, 0x043b,
    0x043c, 0x043d, 0x043e, 0x043f, 0x0440, 0x0441, 0x0442, 0x0443,
    0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0451, 0x0453, 0x0455,
    0x0458, 0x0459, 0x045a, 0x045c, 0x045f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_noMod[] {
        14,     15,     28,    100,      1,     57,     55,     78,
        83,     51,     74,     12,     52,     98,     53,     82,
        11,     79,      2,      3,     80,      4,     81,      5,
        75,      6,     76,     77,      7,      8,     71,      9,
        72,     73,     10,     13,     41,     30,     48,     47,
        34,     32,     18,     43,     44,     23,     37,     38,
        50,     49,     24,     25,     19,     31,     20,     22,
        33,     35,     46,     39,     26,     86,     27,     21,
        36,     16,     17,     40,     45, 
};
constexpr Keylayout_r::KeyLayoutMap_t noMod{uchar_noMod, scancode_noMod, 69};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shift[] {
    0x0008, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0025, 0x0026,
    0x0028, 0x0029, 0x002a, 0x002a, 0x002b, 0x002b, 0x002c, 0x002d,
    0x002f, 0x003a, 0x003b, 0x003f, 0x005f, 0x007e, 0x007f, 0x0401,
    0x0403, 0x0405, 0x0408, 0x0409, 0x040a, 0x040c, 0x040f, 0x0410,
    0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418,
    0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f, 0x0420, 0x0421,
    0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x2018,
    0x2019, 0x201c, 0x201e, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shift[] {
        14,     28,    100,      1,     57,      2,      6,      8,
        10,     11,      9,     55,     78,     13,     83,     74,
       104,     52,     51,     53,     12,     41,     99,     86,
        27,     21,     36,     16,     17,     40,     45,     30,
        48,     47,     34,     32,     18,     43,     44,     23,
        37,     38,     50,     49,     24,     25,     19,     31,
        20,     22,     33,     35,     46,     39,     26,      7,
         5,      4,      3, 
};
constexpr Keylayout_r::KeyLayoutMap_t shift{uchar_shift, scancode_shift, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x0040, 0x005b, 0x005d, 0x007b, 0x007d, 0x00a7, 0x0402,
    0x040b, 0x0452, 0x045b, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_altGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,     47,     33,     34,     48,     49,     50,     26,
        39,     27,     40,     18, 
};
constexpr Keylayout_r::KeyLayoutMap_t altGr{uchar_altGr, scancode_altGr, 20};

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
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x002a, 0x002b,
    0x002c, 0x002c, 0x002d, 0x002d, 0x002e, 0x002f, 0x002f, 0x0030,
    0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038,
    0x0039, 0x003d, 0x0060, 0x0401, 0x0403, 0x0405, 0x0408, 0x0409,
    0x040a, 0x040c, 0x040f, 0x0410, 0x0411, 0x0412, 0x0413, 0x0414,
    0x0415, 0x0416, 0x0417, 0x0418, 0x041a, 0x041b, 0x041c, 0x041d,
    0x041e, 0x041f, 0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425,
    0x0426, 0x0427, 0x0428, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_noMod[] {
        14,     15,     28,    100,      1,     57,     55,     78,
        83,     51,     74,     12,     52,     53,     98,     11,
         2,      3,      4,      5,      6,      7,      8,      9,
        10,     13,     41,     86,     27,     21,     36,     16,
        17,     40,     45,     30,     48,     47,     34,     32,
        18,     43,     44,     23,     37,     38,     50,     49,
        24,     25,     19,     31,     20,     22,     33,     35,
        46,     39,     26, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_noMod{uchar_capslock_noMod, scancode_capslock_noMod, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shift[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0025,
    0x0026, 0x0028, 0x0029, 0x002a, 0x002a, 0x002b, 0x002b, 0x002c,
    0x002d, 0x002f, 0x003a, 0x003b, 0x003f, 0x005f, 0x007e, 0x0430,
    0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438,
    0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f, 0x0440, 0x0441,
    0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0451,
    0x0453, 0x0455, 0x0458, 0x0459, 0x045a, 0x045c, 0x045f, 0x2018,
    0x2019, 0x201c, 0x201e, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shift[] {
        14,     15,    100,     28,      1,     57,      2,      6,
         8,     10,     11,      9,     55,     13,     78,     83,
        74,     98,     52,     51,     53,     12,     41,     30,
        48,     47,     34,     32,     18,     43,     44,     23,
        37,     38,     50,     49,     24,     25,     19,     31,
        20,     22,     33,     35,     46,     39,     26,     86,
        27,     21,     36,     16,     17,     40,     45,      7,
         5,      4,      3, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shift{uchar_capslock_shift, scancode_capslock_shift, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x0040, 0x005b, 0x005d, 0x007b, 0x007d, 0x00a7, 0x0402,
    0x040b, 0x0452, 0x045b, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_altGr[] {
        14,     15,    100,     28,      1,     55,     78,     74,
        98,     47,     33,     34,     48,     49,     50,     26,
        39,     27,     40,     18, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_altGr{uchar_capslock_altGr, scancode_capslock_altGr, 20};

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
        14,     15,     28,    100,      1,     27,     43,     86,
        39,     57,     55,     78,     74,     98, 
};
constexpr Keylayout_r::KeyLayoutMap_t ctrl{uchar_ctrl, scancode_ctrl, 14};

const Keylayout_r::KeyLayoutMap_t deadkeys {
};


const static uint8_t nbDeadkeys = 0;

}

static const Keylayout_r keylayout_x8000042f(
    x8000042f::LCID,
    x8000042f::locale_name,
    x8000042f::noMod,
    x8000042f::shift,
    x8000042f::altGr,
    x8000042f::shiftAltGr,
    x8000042f::ctrl,
    x8000042f::capslock_noMod,
    x8000042f::capslock_shift,
    x8000042f::capslock_altGr,
    x8000042f::capslock_shiftAltGr,
    x8000042f::deadkeys,
    x8000042f::nbDeadkeys
);

