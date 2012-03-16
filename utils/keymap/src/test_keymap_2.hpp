
#if !defined(__KEYMAP__)
#define __KEYMAP__

#include <stdio.h>
//#include <fstream>
#include "constants.hpp"
//#include "log.hpp"
#include <string.h>

struct Keymap2 {

    // keyboard info
    int keys_up_down[256];  // key states 0 up 1 down

    int key_flags;          // scrool_lock = 1, num_lock = 2, caps_lock = 4,
                            // shift = 8, ctrl = 16, Alt = 32,
                            // Windows = 64, AltGr = 128


    // KeyLayout reference tables
    static const int keyLayout_040C_noshift[130];
    static const int keyLayout_040C_shift[130];

    static const int keyLayout_DEFAULT_noshift[130];
    static const int keyLayout_DEFAULT_shift[130];

    // KeyLayout working tables
    int keyLayout_WORK_noshift[128];
    int keyLayout_WORK_shift[128];


    // constructor
// ################################################################
    Keymap2() {
// ################################################################
        memset(this->keys_up_down, 0, 256 * sizeof(int));

//        memset(this->keyLayout_040C_noshift,   0, 130 * sizeof(int));
//        memset(this->keyLayout_040C_shift,   0, 130 * sizeof(int));
//        memset(this->keyLayout_DEFAULT_noshift,   0, 130 * sizeof(int));
//        memset(this->keyLayout_DEFAULT_shift,   0, 130 * sizeof(int));
        memset(this->keyLayout_WORK_noshift,   0, 130 * sizeof(int));
        memset(this->keyLayout_WORK_shift,   0, 130 * sizeof(int));

        this->key_flags = 0;
    }

// ################################################################
    void keymap_init();
// ################################################################

// ################################################################
    int get_key_info_from_scan_code( int device_flags, int scan_code ) const;
// ################################################################

};


#endif
