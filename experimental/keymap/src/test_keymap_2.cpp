#include <stdio.h>
#include "test_keymap_2.hpp"
#include <stdlib.h>
//#include <iostream>
//#include <fstream>
//#include <sstream>
//#include <map>
#include <string>
#include <string.h>
#include <stdint.h>


    const int Keymap2::keyLayout_040C_noshift[130]={
        0, 27, 38, 233, 34, 39, 40, 45, 232, 95, 231,
        224,41,61, 8, 9,97, 122, 101, 114, 116, 121,
        117, 105, 111, 112, 94, 36, 13, 0, 113, 115, 100,
        102, 103, 104, 106, 107, 108, 109, 249, 178, 0, 42,
        119, 120, 99, 118, 98, 110, 44, 59, 58, 33, 0,
        42, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 45, 0, 0,
        0, 43, 0, 0, 0, 0, 0, 0, 0, 60, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        127, 13, 0, 0, 0, 47, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 61, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    const int Keymap2::keyLayout_040C_shift[130]={
        0, 27, 49, 50, 51, 52, 53, 54, 55, 56, 57,
        48, 176, 43, 8, 0, 65, 90, 69, 82, 84, 89,
        85, 73, 79, 80, 168, 163, 13, 0, 81, 83, 68,
        70, 71, 72, 74, 75, 76, 77, 37, 126, 0, 181,
        87, 88, 67, 86, 66, 78, 63, 46, 47, 167, 0,
        42, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 55, 56, 57, 45, 52, 53,
        54, 43, 49, 50, 51, 48, 46, 0, 0, 62, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        127, 13, 0, 0, 0, 47, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 61, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    const int Keymap2::keyLayout_DEFAULT_noshift[130]={
        0, 27, 49, 50, 51, 52, 53, 54, 55, 56, 57,
        48, 223, 180, 8, 9, 113, 119, 101, 114, 116, 122,
        117, 105, 111, 112, 252, 43, 13, 0, 97, 115, 100,
        102, 103, 104, 106, 107, 108, 246, 228, 94, 0, 35,
        121, 120, 99, 118, 98, 110, 109, 44, 46, 45, 0,
        42, 0, 32, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 45, 0, 0,
        0, 43, 0, 0, 0, 0, 0, 0, 0, 60, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        127, 13, 0, 0, 0, 47, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 61, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    };

// ################################################################
void Keymap2::keymap_init( )
// ################################################################
{




//    try{
        // Intialize the WORK tables
        for( int i=8; i< 128; i++) {

            // NOSHIFT
            keyLayout_WORK_noshift[i-8] = keyLayout_DEFAULT_noshift[i];
            if (keyLayout_040C_noshift[i] != 0)
                keyLayout_WORK_noshift[i-8] = keyLayout_040C_noshift[i];

            // SHIFT
//            keyLayout_WORK_shift[i-8] = keyLayout_DEFAULT_shift[i];
//            if (keyLayout_040C_shift[i] != 0)
//                keyLayout_WORK_shift[i-8] = keyLayout_040C_shift[i];
        }
//    } catch (exception& e){
//        clog << "Exception raised " << e.what();
//        LOG(LOG_ERR, "failed to read keymap, keyboard mapping is likely incomplete\n");
//    };


    // DEBUG
//    for( int i=0; i< 120; i++) {
//        printf("%3d = <%3d>\n", i, keyLayout_WORK_shift[i]);
//    }

} // Keymap2::keymap_init


// ################################################################
int Keymap2::get_key_info_from_scan_code( int device_flags, int scan_code ) const
// ################################################################
{
    printf("DEBUT GET_KEY_INFO_FROM_SCAN_CODE\n");

    int resu = 0;


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

// KBD_FLAG_EXT     0x0100
// KBD_FLAG_DOWN    0x4000
// KBD_FLAG_UP      0x8000

    // test if device is ext (???)
    int ext = device_flags & KBD_FLAG_EXT; // 0x0100

    // Shift pressed filter (Right or Left)
    int shift = this->keys_up_down[42] || this->keys_up_down[54];

    // AltGr pressed filter (AltGr or Ctrl + AltGr)
    int altgr = (this->keys_up_down[56] & KBD_FLAG_EXT) || (this->keys_up_down[29] && this->keys_up_down[56]);

    // suppress leftmost bit from scancode (so Break state is treated as Make state)
    scan_code = scan_code & 0x7f;

    // retreive the index of key in "map" (depending on 'ext' flag  set or not set)
    int index = ext ? map[scan_code-8].code2 : map[scan_code-8].code1;

    // keymap file is created with numlock off so we have to do this

    // ### KEYPAD KEY ###        // ### KEYPAD KEY ###
    if ((index >= 79) && (index <= 91)) {

        // ### KEYPAD + NUMLOCK ###
        if ((key_flags & 2)) {
            resu = keyLayout_WORK_shift[index];
        }
        // ### KEYPAD alone ###
        else {
            resu = keyLayout_WORK_noshift[index];
        }
    }
    // ### SHIFT + CAPSLOCK ###
    else if (shift && (this->key_flags & 4)) {

//        resu = &(this->keyLayout_WORK_shiftcapslock[index]);
    }
    // ### SHIFT ###
    else if (shift) {

        resu = this->keyLayout_WORK_shift[index];
    }
    // ### CAPSLOCK ###
    else if (this->key_flags & 4) {

//        resu = this->keyLayout_WORK_capslock[index];
    }
    // ### ALTGr ###
    else if (altgr) {

//        resu = this->keyLayout_WORK_altgr[index];
    }
    // ### DEFAULT : NO MODIFIER KEY ###
    else {

        resu = this->keyLayout_WORK_noshift[index];
    }

    printf("FIN GET_KEY_INFO_FROM_SCAN_CODE\n");
    return resu;

} // Keymap2::get_key_info_from_scan_code


// ################################################################
int main()
// ################################################################
{
    printf("DEBUT TKM\n");

    Keymap2 keymap;

    keymap.keymap_init();

    keymap.keys_up_down[42] = 1;
    keymap.keys_up_down[54] = 0;
    keymap.key_flags += 2;

    int chr = keymap.get_key_info_from_scan_code( 0, 16 );

    printf("CHR = %d\n", chr);

//    keymap.keymap_init("./src//km-0409.ini");
//    keymap.keymap_tempo();

    printf("FIN TKM\n");

    return 0;
}
