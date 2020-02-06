#pragma once

#include "keylayout_r.hpp"

namespace x80010419
{

const static int LCID = 0x10419;

const static char * const locale_name = "ru-RU.typewriter";

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022,
    0x0025, 0x0029, 0x0029, 0x002a, 0x002b, 0x002c, 0x002c, 0x002d,
    0x002d, 0x002e, 0x002e, 0x002f, 0x002f, 0x0030, 0x0031, 0x0032,
    0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a,
    0x003b, 0x003f, 0x005f, 0x007c, 0x0430, 0x0431, 0x0432, 0x0433,
    0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043a, 0x043b,
    0x043c, 0x043d, 0x043e, 0x043f, 0x0440, 0x0441, 0x0442, 0x0443,
    0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044a, 0x044b,
    0x044c, 0x044d, 0x044e, 0x044f, 0x0451, 0x2116, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_noMod[] {
        14,     15,    100,     28,      1,     57,     12,      5,
        11,     43,     86,     55,     78,     83,      7,      3,
        74,    126,      8,     98,      4,     82,     79,     80,
        81,     75,     76,     77,     71,     72,     73,      6,
        13,     10,      9,     41,     33,     51,     32,     22,
        38,     20,     39,     25,     48,     16,     19,     37,
        47,     21,     36,     34,     35,     46,     49,     18,
        30,     26,     17,     45,     23,     24,     27,     31,
        50,     40,     52,     44,     53,      2, 
};
constexpr Keylayout_r::KeyLayoutMap_t noMod{uchar_noMod, scancode_noMod, 70};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shift[] {
    0x0008, 0x000d, 0x000d, 0x001b, 0x0020, 0x0028, 0x0028, 0x002a,
    0x002b, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 0x0030, 0x0031,
    0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039,
    0x003d, 0x005c, 0x007f, 0x0401, 0x0410, 0x0411, 0x0412, 0x0413,
    0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041a, 0x041b,
    0x041c, 0x041d, 0x041e, 0x041f, 0x0420, 0x0421, 0x0422, 0x0423,
    0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042a, 0x042b,
    0x042c, 0x042d, 0x042e, 0x042f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shift[] {
        14,    100,     28,      1,     57,     86,     43,     55,
        78,     41,     83,     74,    126,    104,     11,      2,
         3,      4,      5,      6,      7,      8,      9,     10,
        12,     13,     99,     53,     33,     51,     32,     22,
        38,     20,     39,     25,     48,     16,     19,     37,
        47,     21,     36,     34,     35,     46,     49,     18,
        30,     26,     17,     45,     23,     24,     27,     31,
        50,     40,     52,     44, 
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
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022,
    0x0025, 0x0029, 0x0029, 0x002a, 0x002b, 0x002c, 0x002c, 0x002d,
    0x002d, 0x002e, 0x002e, 0x002f, 0x002f, 0x003a, 0x003b, 0x003f,
    0x005f, 0x007c, 0x0401, 0x0410, 0x0411, 0x0412, 0x0413, 0x0414,
    0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c,
    0x041d, 0x041e, 0x041f, 0x0420, 0x0421, 0x0422, 0x0423, 0x0424,
    0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042a, 0x042b, 0x042c,
    0x042d, 0x042e, 0x042f, 0x2116, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_noMod[] {
        14,     15,     28,    100,      1,     57,     12,      5,
        11,     43,     86,     55,     78,      7,     83,      3,
        74,      8,    126,     98,      4,      6,     13,     10,
         9,     41,     53,     33,     51,     32,     22,     38,
        20,     39,     25,     48,     16,     19,     37,     47,
        21,     36,     34,     35,     46,     49,     18,     30,
        26,     17,     45,     23,     24,     27,     31,     50,
        40,     52,     44,      2, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_noMod{uchar_capslock_noMod, scancode_capslock_noMod, 60};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shift[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0028, 0x0028,
    0x002a, 0x002b, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 0x0030,
    0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038,
    0x0039, 0x003d, 0x005c, 0x0430, 0x0431, 0x0432, 0x0433, 0x0434,
    0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043a, 0x043b, 0x043c,
    0x043d, 0x043e, 0x043f, 0x0440, 0x0441, 0x0442, 0x0443, 0x0444,
    0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044a, 0x044b, 0x044c,
    0x044d, 0x044e, 0x044f, 0x0451, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shift[] {
        14,     15,     28,    100,      1,     57,     86,     43,
        55,     78,     41,     83,     74,    126,     98,     11,
         2,      3,      4,      5,      6,      7,      8,      9,
        10,     12,     13,     33,     51,     32,     22,     38,
        20,     39,     25,     48,     16,     19,     37,     47,
        21,     36,     34,     35,     46,     49,     18,     30,
        26,     17,     45,     23,     24,     27,     31,     50,
        40,     52,     44,     53, 
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
        14,     15,     28,    100,      1,     27,     43,     86,
        39,     57,     55,     78,     74,     98, 
};
constexpr Keylayout_r::KeyLayoutMap_t ctrl{uchar_ctrl, scancode_ctrl, 14};

const Keylayout_r::KeyLayoutMap_t deadkeys {
};


const static uint8_t nbDeadkeys = 0;

}

static const Keylayout_r keylayout_x80010419(
    x80010419::LCID,
    x80010419::locale_name,
    x80010419::noMod,
    x80010419::shift,
    x80010419::altGr,
    x80010419::shiftAltGr,
    x80010419::ctrl,
    x80010419::capslock_noMod,
    x80010419::capslock_shift,
    x80010419::capslock_altGr,
    x80010419::capslock_shiftAltGr,
    x80010419::deadkeys,
    x80010419::nbDeadkeys
);

