

#if !defined(__KEYMAP__)
#define __KEYMAP__

#include <stdio.h>
#include <fstream>
#include <stdint.h>
#include "constants.hpp"
#include "log.hpp"
#include <string.h>


typedef enum{
    ID_UNKNOWN,
    ID_NOSHIFT,
    ID_SHIFT,
    ID_ALTGR,
    ID_CAPSLOCK,
    ID_SHIFTCAPSLOCK
} id_keymap;

struct key_info {
    int sym;
    int chr;
    key_info() {
        this->sym = 0;
        this->chr = 0;
    }
};

struct Keymap {
    // keyboard info
    int keys[256]; // key states 0 up 1 down
    int key_flags; // scrool_lock = 1, num_lock = 2, caps_lock = 4, shift = 8, ctrl = 0x10, ALT = 0x20, Windows = 0x40, AlrGr = 0x80

    struct key_info keys_noshift[128];
    struct key_info keys_shift[128];
    struct key_info keys_altgr[128];
    struct key_info keys_capslock[128];
    struct key_info keys_shiftcapslock[128];

    Keymap(){
        memset(this->keys,   0, 256 * sizeof(int)); // key states 0 up 1 down
        // scrool_lock = 1, num_lock = 2, caps_lock = 4
        this->key_flags = 0;
    }

    bool shift_pressed() const {
        return (this->keys[42] != 0) || (this->keys[54] != 0);
    }

    ///****************************************************************************
    /// Params :
    ///  - device_flag =
    ///  - scancode    =
    /// Return :
    ///  - a keyinfo struct with chr and symbol translated from RDP to X11
    ///     (this explanation is still to be validated by experimentation !!!!!)
    ///****************************************************************************
    const key_info*  get_key_info_from_scan_code( int device_flags, int scan_code ) const {

        // map for rdp to x11 scancodes
        // code1 is regular scancode, code2 is extended scancode

        static struct codepair {
            uint8_t code1;
            uint8_t code2;
        } map[128] =  {
            {   0,   0 }, {   9,   0 }, {  10,   0 }, {  11,   0 }, {  12,   0 }, //   0 -   4
            {  13,   0 }, {  14,   0 }, {  15,   0 }, {  16,   0 }, {  17,   0 }, //   5 -   9
            {  18,   0 }, {  19,   0 }, {  20,   0 }, {  21,   0 }, {  22,   0 }, //  10 -  14
            {  23,   0 }, {  24,   0 }, {  25,   0 }, {  26,   0 }, {  27,   0 }, //  15 -  19
            {  28,   0 }, {  29,   0 }, {  30,   0 }, {  31,   0 }, {  32,   0 }, //  20 -  24
            {  33,   0 }, {  34,   0 }, {  35,   0 }, {  36, 108 }, {  37, 109 }, //  25 -  29
            {  38,   0 }, {  39,   0 }, {  40,   0 }, {  41,   0 }, {  42,   0 }, //  30 -  34
            {  43,   0 }, {  44,   0 }, {  45,   0 }, {  46,   0 }, {  47,   0 }, //  35 -  39
            {  48,   0 }, {  49,   0 }, {  50,   0 }, {  51,   0 }, {  52,   0 }, //  40 -  44
            {  53,   0 }, {  54,   0 }, {  55,   0 }, {  56,   0 }, {  57,   0 }, //  45 -  49
            {  58,   0 }, {  59,   0 }, {  60,   0 }, {  61, 112 }, {  62,   0 }, //  50 -  54
            {  63, 111 }, {  64, 113 }, {  65,   0 }, {  66,   0 }, {  67,   0 }, //  55 -  59
            {  68,   0 }, {  69,   0 }, {  70,   0 }, {  71,   0 }, {  72,   0 }, //  60 -  64
            {  73,   0 }, {  74,   0 }, {  75,   0 }, {  76,   0 }, {  77,   0 }, //  65 -  69
            {  78,   0 }, {  79,  97 }, {  80,  98 }, {  81,  99 }, {  82,   0 }, //  70 -  74
            {  83, 100 }, {  84,   0 }, {  85, 102 }, {  86,   0 }, {  87, 103 }, //  75 -  79
            {  88, 104 }, {  89, 105 }, {  90, 106 }, {  91, 107 }, {  92,   0 }, //  80 -  84
            {  93,   0 }, {  94,   0 }, {  95,   0 }, {  96,   0 }, {  97,   0 }, //  85 -  89
            {  98,   0 }, {   0, 115 }, {   0, 116 }, {   0, 117 }, { 102,   0 }, //  90 -  94
            { 103,   0 }, { 104,   0 }, { 105,   0 }, { 106,   0 }, { 107,   0 }, //  95 -  99
            { 108,   0 }, { 109,   0 }, { 110,   0 }, { 111,   0 }, { 112,   0 }, // 100 - 104
            { 113,   0 }, { 114,   0 }, { 115,   0 }, { 116,   0 }, { 117,   0 }, // 105 - 109
            { 118,   0 }, { 119,   0 }, { 120,   0 }, { 121,   0 }, { 122,   0 }, // 110 - 114
            { 123,   0 }, { 124,   0 }, { 125,   0 }, { 126,   0 }, { 127,   0 }, // 115 - 119
            { 128,   0 }, { 129,   0 }, { 130,   0 }, { 131,   0 }, { 132,   0 }, // 120 - 124
            { 133,   0 }, { 134,   0 }, { 135,   0 }                              // 125 - 127
        };

// KBD_FLAG_RIGHT   0x0001
// KBD_FLAG_EXT     0x0100
// KBD_FLAG_QUIET   0x1000
// KBD_FLAG_DOWN    0x4000
// KBD_FLAG_UP      0x8000

        // return value set to 0
        const key_info* rv = 0;

        // test if device is ext (???)
        int ext = device_flags & KBD_FLAG_EXT; // 0x0100

        // Shift pressed filter
        int shift = this->keys[42] || this->keys[54];

        // AltGr pressed filter (<right alt> or <ctrl + alt>)
        int altgr = (this->keys[56] & KBD_FLAG_EXT) || (this->keys[29] && this->keys[56]);

        // suppress leftmost bitfrom scancode (so Break or Make are the same)
        scan_code = scan_code & 0x7f;

        // retreive the index of key in "map" (depending on 'ext' flag  set or not set)
        // the goal seemes to translate from RDP scancodes to X11 scancodes
        int index = ext ? map[scan_code].code2 : map[scan_code].code1;

        // keymap file is created with numlock off so we have to do this

        // ### KEYPAD KEY ###
        if ((index >= 79) && (index <= 91)) {

            // ### KEYPAD + NUMLOCK ###
            if ((this->key_flags & 2)) {
                rv = &(this->keys_shift[index]);
                                                //LOG(LOG_INFO, "shiftnumpad scancode=%d index=%d keyvalue=%d:%d\n", scan_code, index, rv->chr, rv->sym);
            }
            // ### KEYPAD ###
            else {
                rv = &(this->keys_noshift[index]);
                                                //LOG(LOG_INFO, "plainnumpad scancode=%d index=%d keyvalue=%d:%d\n", scan_code, index, rv->chr, rv->sym);
            }
        }
        // ### SHIFT + CAPSLOCK ###
        else if (shift && (this->key_flags & 4)) {

            rv = &(this->keys_shiftcapslock[index]);
                                                //LOG(LOG_INFO, "shiftcapslock scancode=%d index=%d keyvalue=%d;%d\n", scan_code, index, rv->chr, rv->sym);
        }
        // ### SHIFT ###
        else if (shift) {

            rv = &(this->keys_shift[index]);
                                                //LOG(LOG_INFO, "shift scancode=%d index=%d keyvalue=%d:%d\n", scan_code, index, rv->chr, rv->sym);
        }
        // ### CAPSLOCK ###
        else if (this->key_flags & 4) {

            rv = &(this->keys_capslock[index]);
                                                //LOG(LOG_INFO, "capslock scancode=%d index=%d keyvalue=%d:%d\n", scan_code, index, rv->chr, rv->sym);
        }
        // ### ALTGr ###
        else if (altgr) {

            rv = &(this->keys_altgr[index]);
                                                //LOG(LOG_INFO, "altgr scancode=%d index=%d keyvalue=%d:%d\n", scan_code, index, rv->chr, rv->sym);
        }
        // ### DEFAULT : NO MODIFIER KEY ###
        else {

            rv = &(this->keys_noshift[index]);
                                                //LOG(LOG_INFO, "noshift scancode=%d index=%d keyvalue=%d:%d\n", scan_code, index, rv->chr, rv->sym);
        }
        return rv;

    } // get_key_info_from_scan_code

    void keymap_init(const char * filename);
    void keymap_init(std::istream & Keymap_stream);
    void keymap_tempo();

}; // struct Keymap

#endif
