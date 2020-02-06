#pragma once

#include "keylayout_r.hpp"

namespace x80000c1a
{

const static int LCID = 0xc1a;

const static char * const locale_name = "sr-Cy";

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_noMod[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0027, 0x002a,
    0x002b, 0x002b, 0x002c, 0x002d, 0x002d, 0x002e, 0x002e, 0x002f,
    0x0030, 0x0030, 0x0031, 0x0031, 0x0032, 0x0032, 0x0033, 0x0033,
    0x0034, 0x0034, 0x0035, 0x0035, 0x0036, 0x0036, 0x0037, 0x0037,
    0x0038, 0x0038, 0x0039, 0x0039, 0x003c, 0x0060, 0x0430, 0x0431,
    0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x043a,
    0x043b, 0x043c, 0x043d, 0x043e, 0x043f, 0x0440, 0x0441, 0x0442,
    0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0452, 0x0455,
    0x0458, 0x0459, 0x045a, 0x045b, 0x045f, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_noMod[] {
        14,     15,     28,    100,      1,     57,     12,     55,
        78,     13,     51,     74,     53,    126,     52,     98,
        82,     11,     79,      2,      3,     80,      4,     81,
         5,     75,      6,     76,      7,     77,      8,     71,
         9,     72,     73,     10,     86,     41,     30,     48,
        47,     34,     32,     18,     43,     21,     23,     37,
        38,     50,     49,     24,     25,     19,     31,     20,
        22,     33,     35,     46,     39,     26,     27,     44,
        36,     16,     17,     40,     45, 
};
constexpr Keylayout_r::KeyLayoutMap_t noMod{uchar_noMod, scancode_noMod, 69};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_shift[] {
    0x0008, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022, 0x0023,
    0x0024, 0x0025, 0x0026, 0x0028, 0x0029, 0x002a, 0x002a, 0x002b,
    0x002d, 0x002e, 0x002f, 0x002f, 0x003a, 0x003b, 0x003d, 0x003e,
    0x003f, 0x005f, 0x007e, 0x007f, 0x0402, 0x0405, 0x0408, 0x0409,
    0x040a, 0x040b, 0x040f, 0x0410, 0x0411, 0x0412, 0x0413, 0x0414,
    0x0415, 0x0416, 0x0417, 0x0418, 0x041a, 0x041b, 0x041c, 0x041d,
    0x041e, 0x041f, 0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425,
    0x0426, 0x0427, 0x0428, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_shift[] {
        14,     28,    100,      1,     57,      2,      3,      4,
         5,      6,      7,      9,     10,     13,     55,     78,
        74,    126,      8,    104,     52,     51,     11,     86,
        12,     53,     41,     99,     27,     44,     36,     16,
        17,     40,     45,     30,     48,     47,     34,     32,
        18,     43,     21,     23,     37,     38,     50,     49,
        24,     25,     19,     31,     20,     22,     33,     35,
        46,     39,     26, 
};
constexpr Keylayout_r::KeyLayoutMap_t shift{uchar_shift, scancode_shift, 59};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x003c, 0x003e, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_altGr[] {
        14,     15,     28,    100,      1,     55,     78,     74,
        98,     51,     52,     18, 
};
constexpr Keylayout_r::KeyLayoutMap_t altGr{uchar_altGr, scancode_altGr, 12};

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
    0x002b, 0x002b, 0x002c, 0x002d, 0x002d, 0x002e, 0x002e, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003c, 0x0060, 0x0402, 0x0405, 0x0408, 0x0409,
    0x040a, 0x040b, 0x040f, 0x0410, 0x0411, 0x0412, 0x0413, 0x0414,
    0x0415, 0x0416, 0x0417, 0x0418, 0x041a, 0x041b, 0x041c, 0x041d,
    0x041e, 0x041f, 0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425,
    0x0426, 0x0427, 0x0428, 0xffffffff, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_noMod[] {
        14,     15,     28,    100,      1,     57,     12,     55,
        78,     13,     51,     74,     53,    126,     52,     98,
        11,      2,      3,      4,      5,      6,      7,      8,
         9,     10,     86,     41,     27,     44,     36,     16,
        17,     40,     45,     30,     48,     47,     34,     32,
        18,     43,     21,     23,     37,     38,     50,     49,
        24,     25,     19,     31,     20,     22,     33,     35,
        46,     39,     26,     83, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_noMod{uchar_capslock_noMod, scancode_capslock_noMod, 60};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_shift[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x0020, 0x0021, 0x0022,
    0x0023, 0x0024, 0x0025, 0x0026, 0x0028, 0x0029, 0x002a, 0x002a,
    0x002b, 0x002d, 0x002e, 0x002f, 0x002f, 0x003a, 0x003b, 0x003d,
    0x003e, 0x003f, 0x005f, 0x007e, 0x0430, 0x0431, 0x0432, 0x0433,
    0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x043a, 0x043b, 0x043c,
    0x043d, 0x043e, 0x043f, 0x0440, 0x0441, 0x0442, 0x0443, 0x0444,
    0x0445, 0x0446, 0x0447, 0x0448, 0x0452, 0x0455, 0x0458, 0x0459,
    0x045a, 0x045b, 0x045f, 0xffffffff, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_shift[] {
        14,     15,     28,    100,      1,     57,      2,      3,
         4,      5,      6,      7,      9,     10,     13,     55,
        78,     74,    126,      8,     98,     52,     51,     11,
        86,     12,     53,     41,     30,     48,     47,     34,
        32,     18,     43,     21,     23,     37,     38,     50,
        49,     24,     25,     19,     31,     20,     22,     33,
        35,     46,     39,     26,     27,     44,     36,     16,
        17,     40,     45,     83, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_shift{uchar_capslock_shift, scancode_capslock_shift, 60};

constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_capslock_altGr[] {
    0x0008, 0x0009, 0x000d, 0x000d, 0x001b, 0x002a, 0x002b, 0x002d,
    0x002f, 0x003c, 0x003e, 0x20ac, 
};
constexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_capslock_altGr[] {
        14,     15,     28,    100,      1,     55,     78,     74,
        98,     51,     52,     18, 
};
constexpr Keylayout_r::KeyLayoutMap_t capslock_altGr{uchar_capslock_altGr, scancode_capslock_altGr, 12};

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


const static uint8_t nbDeadkeys = 1;

}

static const Keylayout_r keylayout_x80000c1a(
    x80000c1a::LCID,
    x80000c1a::locale_name,
    x80000c1a::noMod,
    x80000c1a::shift,
    x80000c1a::altGr,
    x80000c1a::shiftAltGr,
    x80000c1a::ctrl,
    x80000c1a::capslock_noMod,
    x80000c1a::capslock_shift,
    x80000c1a::capslock_altGr,
    x80000c1a::capslock_shiftAltGr,
    x80000c1a::deadkeys,
    x80000c1a::nbDeadkeys
);

