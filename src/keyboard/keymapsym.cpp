/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012-2013
   Author(s): Christophe Grosjean, Dominique Lafages

   header file. KeymapSym object for keymap translation from RDP to X (VNC)
*/

#include "keyboard/keymapsym.hpp"
#include "utils/log.hpp"
#include <cstring>
#include <cstdio>
#include <stdlib.h>

// using namespace std;

KeymapSym::KeymapSym(int keylayout, int key_flags, bool is_unix, bool is_apple, int verbose)
: ibuf_sym(0)
, nbuf_sym(0)
, dead_key(DEADKEY_NONE)
, is_unix(is_unix)
, is_apple(is_apple)
, verbose(verbose)
{
    memset(this->keys_down, 0, 256 * sizeof(int));
    memset(&this->keylayout_WORK_noshift_sym,       0, 128 * sizeof(int));
    memset(&this->keylayout_WORK_shift_sym,         0, 128 * sizeof(int));
    memset(&this->keylayout_WORK_altgr_sym,         0, 128 * sizeof(int));
    memset(&this->keylayout_WORK_capslock_sym,      0, 128 * sizeof(int));
    memset(&this->keylayout_WORK_shiftcapslock_sym, 0, 128 * sizeof(int));

    this->last_sym = 0;
    
    if (is_apple) {
        this->init_layout_sym(0x0409);
    } else {
        this->init_layout_sym(keylayout);
    }
    // Initial state of keys (at least lock keys) is copied from Keymap2
    this->key_flags = key_flags;
}

// [MS-RDPBCGR] - 2.2.8.1.2.2.5 Fast-Path Synchronize Event
//  (TS_FP_SYNC_EVENT)
// ========================================================

// The TS_FP_SYNC_EVENT structure is the fast-path variant of the TS_SYNC_EVENT
//  (section 2.2.8.1.1.3.1.1.5) structure.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |  eventHeader  |
// +---------------+

// eventHeader (1 byte): An 8-bit, unsigned integer. The format of this field is
//  the same as the eventHeader byte field, specified in section 2.2.8.1.2.2.
//  The eventCode bitfield (3 bits in size) MUST be set to
//  FASTPATH_INPUT_EVENT_SYNC (3). The eventFlags bitfield (5 bits in size)
//  contains flags indicating the "on" status of the keyboard toggle keys.

// +--------------------------------------+------------------------------------+
// | 5-Bit Codes                          | Meaning                            |
// +--------------------------------------+------------------------------------+
// | 0x01 FASTPATH_INPUT_SYNC_SCROLL_LOCK | Indicates that the Scroll Lock     |
// |                                      | indicator light SHOULD be on.      |
// +--------------------------------------+------------------------------------+
// | 0x02 FASTPATH_INPUT_SYNC_NUM_LOCK    | Indicates that the Num Lock        |
// |                                      | indicator light SHOULD be on.      |
// +--------------------------------------+------------------------------------+
// | 0x04 FASTPATH_INPUT_SYNC_CAPS_LOCK   | Indicates that the Caps Lock       |
// |                                      | indicator light SHOULD be on.      |
// +--------------------------------------+------------------------------------+
// | 0x08 FASTPATH_INPUT_SYNC_KANA_LOCK   | Indicates that the Kana Lock       |
// |                                      | indicator light SHOULD be on.      |
// +--------------------------------------+------------------------------------+


// TODO: synchronize is not called, we currently have a direct change of key_flags from vnc
void KeymapSym::synchronize(uint16_t param1)
{
    this->key_flags = param1 & 0x07;
    // non sticky keys are forced to be UP
    this->keys_down[LEFT_SHIFT] = 0;
    this->keys_down[RIGHT_SHIFT] = 0;
    this->keys_down[LEFT_CTRL] = 0;
    this->keys_down[RIGHT_CTRL] = 0;
    this->keys_down[LEFT_ALT] = 0;
    this->keys_down[RIGHT_ALT] = 0;
}

const KeymapSym::KeyLayout_t * KeymapSym::select_layout()
{
    // pick the LAYOUT to use (depending on current keyboard state)
    //----------------------------------------
    // if left ctrl and left alt are pressed, vnc server will convert key combination itself.
    if ((this->is_ctrl_pressed() && this->is_left_alt_pressed())
    || (this->is_right_alt_pressed())) {
        if (this->verbose & 2) {
            LOG(LOG_INFO, "Altgr Layout");
        }
        return &this->keylayout_WORK_altgr_sym;
    }
    if (this->is_shift_pressed() && this->is_caps_locked()){
        if (this->verbose & 2) {
            LOG(LOG_INFO, "Shift Capslock Layout");
        }
        return &this->keylayout_WORK_shiftcapslock_sym;
    }
    if (this->is_shift_pressed()){
        if (this->verbose & 2) {
            LOG(LOG_INFO, "Shift Layout");
        }
        return &this->keylayout_WORK_shift_sym;
    }
    if (this->is_caps_locked()) {
        if (this->verbose & 2) {
            LOG(LOG_INFO, "Capslock Layout");
        }
        return &this->keylayout_WORK_capslock_sym;
    }
    if (this->verbose & 2) {
        LOG(LOG_INFO, "Plain Layout");
    }
    return &this->keylayout_WORK_noshift_sym;
}

// The TS_KEYBOARD_EVENT structure is a standard T.128 Keyboard Event (see [T128] section
// 8.18.2). RDP keyboard input is restricted to keyboard scancodes, unlike the code-point or virtual
// codes supported in T.128 (a scancode is an 8-bit value specifying a key location on the keyboard).
// The server accepts a scancode value and translates it into the correct character depending on the
// language locale and keyboard layout used in the session.

// keyboardFlags (2 bytes): A 16-bit, unsigned integer. The flags describing the keyboard event.

// +--------------------------+------------------------------------------------+
// | 0x0100 KBDFLAGS_EXTENDED | The keystroke message contains an extended     |
// |                          | scancode. For enhanced 101-key and 102-key     |
// |                          | keyboards, extended keys include "he right     |
// |                          | ALT and right CTRL keys on the main section    |
// |                          | of the keyboard; the INS, DEL, HOME, END,      |
// |                          | PAGE UP, PAGE DOWN and ARROW keys in the       |
// |                          | clusters to the left of the numeric keypad;    |
// |                          | and the Divide ("/") and ENTER keys in the     |
// |                          | numeric keypad.                                |
// +--------------------------+------------------------------------------------+
// | 0x4000 KBDFLAGS_DOWN     | Indicates that the key was down prior to this  |
// |                          | event.                                         |
// +--------------------------+------------------------------------------------+
// | 0x8000 KBDFLAGS_RELEASE  | The absence of this flag indicates a key-down  |
// |                          | event, while its presence indicates a          |
// |                          | key-release event.                             |
// +--------------------------+------------------------------------------------+

// keyCode (2 bytes): A 16-bit, unsigned integer. The scancode of the key which
// triggered the event.


void KeymapSym::event(int device_flags, long keycode)
{
     if (this->verbose & 2)
     {
        LOG(LOG_INFO, "KeymapSym::event(keyboardFlags=%04x (%s%s), keycode=%04x flags=%04x (%s %s %s %s %s %s %s))",
            static_cast<unsigned>(device_flags), (device_flags & KBDFLAGS_RELEASE)?"UP":"DOWN",(device_flags & KBDFLAGS_EXTENDED)?" EXT":"",
            static_cast<unsigned>(keycode), static_cast<unsigned>(this->key_flags),
            (this->key_flags & SCROLLLOCK)?"SCR ":"",
            (this->key_flags & NUMLOCK)?"NUM ":"",
            (this->key_flags & CAPSLOCK)?"CAPS ":"",
            (this->key_flags & FLG_SHIFT)?"SHIFT ":"",
            (this->key_flags & FLG_ALT)?"ALT ":"",
            (this->key_flags & FLG_WINDOWS)?"WIN ":"",
            (this->key_flags & FLG_ALTGR)?"ALTGR ":"");
     }

    if (this->is_apple) {
        this->apple_keyboard_translation(device_flags, keycode);
    } else {
        this->key_event(device_flags, keycode);
    }
}

void KeymapSym::remove_modifiers()
{
    // KS_Alt_L = 0xffe9,
    if (this->is_left_alt_pressed()){
        this->push_sym(KeySym(0xffe9, 0));
    }
    // KS_Alt_R = 0xffea,
    if (this->is_right_alt_pressed()){
        this->push_sym(KeySym(0xffea, 0));
    }
    // KS_Control_R = 0xffe4,
    if (this->is_right_ctrl_pressed()){
        this->push_sym(KeySym(0xffe4, 0));
    }
    // KS_Control_L = 0xffe3,
    if (this->is_left_ctrl_pressed()){
        this->push_sym(KeySym(0xffe3, 0));
    }
    // KS_Shift_L = 0xffe1,
    if (this->is_left_shift_pressed()){
        this->push_sym(KeySym(0xffe1, 0));
    }
    // KS_Shift_R = 0xffe2,
    if (this->is_right_shift_pressed()){
        this->push_sym(KeySym(0xffe2, 0));
    }
}

void KeymapSym::putback_modifiers()
{
    // KS_Alt_L = 0xffe9,
    if (this->is_left_alt_pressed()){
        this->push_sym(KeySym(0xffe9, 1));
    }
    // KS_Alt_R = 0xffea,
    if (this->is_right_alt_pressed()){
        this->push_sym(KeySym(0xffea, 1));
    }
    // KS_Control_R = 0xffe4,
    if (this->is_right_ctrl_pressed()){
        this->push_sym(KeySym(0xffe4, 1));
    }
    // KS_Control_L = 0xffe3,
    if (this->is_left_ctrl_pressed()){
        this->push_sym(KeySym(0xffe3, 1));
    }
    // KS_Shift_L = 0xffe1,
    if (this->is_left_shift_pressed()){
        this->push_sym(KeySym(0xffe1, 1));
    }
    // KS_Shift_R = 0xffe2,
    if (this->is_right_shift_pressed()){
        this->push_sym(KeySym(0xffe2, 1));
    }
}


void KeymapSym::key_event(int device_flags, long keycode) {

    KeySym ks = this->get_key(device_flags, keycode);
    int key = ks.sym;
    uint8_t downflag = ks.down;
    
    if (this->is_unix 
    && this->is_altgr_pressed() 
    && (key == 0x65))
    {
        this->remove_modifiers();
        switch (key){
        default:
        case 0x65:
            this->push_sym(KeySym(0x20AC, downflag));
            break;
        }
        this->putback_modifiers();
    }
    else if (!this->is_unix 
    && this->is_altgr_pressed() 
    && (key == 0x65))
    {
        if (downflag == 1){
            this->remove_modifiers();
            switch (key){
            default:
                break;
            case 0x65:
                this->push_sym(KeySym(0xffe3, 1));
                this->push_sym(KeySym(0xffe9, 1));
                this->push_sym(KeySym(0x65, downflag));
                this->push_sym(KeySym(0xffe3, 0));
                this->push_sym(KeySym(0xffe9, 0));
                break;
            }
            this->putback_modifiers();
        }
    }
    else
    if (this->is_altgr_pressed() 
    // this is plain ascii: trust our decoder
    && (key >= 0x20 && key <= 0x7e)){
        this->remove_modifiers();
        this->push_sym(KeySym(key, downflag));
        this->putback_modifiers();
    }
    else {
        this->push_sym(KeySym(key, downflag));
    }
}

void KeymapSym::apple_keyboard_translation(int device_flags, long keycode) {

    uint8_t downflag = !(device_flags & KBDFLAGS_RELEASE);
    
    switch (this->keylayout) {

        case 0x040c:                                    // French
            switch (keycode) {

                case 0x0b:
                    if (this->is_alt_pressed()) {
                        this->push_sym(KeySym(0xffe9, 0));
                        this->push_sym(KeySym(0xa4, downflag)); /* @ */
                        this->push_sym(KeySym(0xffe9, 1));
                    } else {
                        this->key_event(device_flags, keycode);
                    }
                    break;

                case 0x04:
                    if (this->is_alt_pressed()) {
                        this->push_sym(KeySym(0xffe9, 0));
                        this->push_sym(KeySym(0xffe2, 1));
                        this->push_sym(KeySym(0xa4, downflag)); /* # */
                        this->push_sym(KeySym(0xffe2, 0));
                        this->push_sym(KeySym(0xffe9, 1));
                    } else {
                        this->key_event(device_flags, keycode);
                    }
                    break;

                case 0x35:
                    if (this->is_shift_pressed()) {
                        this->push_sym(KeySym(0xffe2, 0));
                        this->push_sym(KeySym(0x36, downflag)); /* § */
                        this->push_sym(KeySym(0xffe2, 1));
                    } else {
                        if (device_flags & KeymapSym::KBDFLAGS_EXTENDED) {
                            this->push_sym(KeySym(0xffe2, 1));
                            this->push_sym(KeySym(0x3e, downflag)); /* / */
                            this->push_sym(KeySym(0xffe2, 0));
                        } else {
                            this->push_sym(KeySym(0x38, downflag)); /* ! */
                        }
                    }
                    break;

                case 0x07: /* - */
                    if (!this->is_shift_pressed()) {
                        this->push_sym(KeySym(0xffe2, 1));
                        this->push_sym(KeySym(0x3d, downflag));
                        this->push_sym(KeySym(0xffe2, 0));
                    } else {
                        this->key_event(device_flags, keycode);
                    }
                    break;

                case 0x2b: /* * */
                    this->push_sym(KeySym(0xffe2, 1));
                    this->push_sym(KeySym(0x2a, downflag));
                    this->push_sym(KeySym(0xffe2, 0));
                    break;

                case 0x1b: /* £ */
                    if (this->is_shift_pressed()) {
                        this->push_sym(KeySym(0x5c, downflag));
                    } else {
                        this->key_event(device_flags, keycode);
                    }
                    break;

                case 0x09: /* _ */
                    if (!this->is_shift_pressed()) {
                        this->push_sym(KeySym(0xffe2, 1));
                        this->push_sym(KeySym(0xad, downflag));
                        this->push_sym(KeySym(0xffe2, 0));
                    } else {
                        this->push_sym(KeySym(0x38, downflag)); /* 8 */
                    }
                    break;

                case 0x56:
                    if (this->is_shift_pressed()) {
                        this->push_sym(KeySym(0x7e, downflag)); /* > */
                    } else {
                        this->push_sym(KeySym(0xffe2, 1));
                        this->push_sym(KeySym(0x60, downflag)); /* < */
                        this->push_sym(KeySym(0xffe2, 0));
                    }
                    break;

                case 0x0d: /* = */
                    this->push_sym(KeySym(0x2f, downflag));
                    break;

                default:
                    this->key_event(device_flags, keycode);
                    break;
            }
            break;

        // Note: specialize and treat special case if need arise.
        // (like french keyboard above)
        // -----------------------------------------------------------------
        
//            case 0x100c: // French Swizerland
//            case 0x0813: // Dutch Belgium
//            case 0x080c: // French Belgium
//            case 0x0809: // English UK
//            case 0x0807: // German Swizerland
//            case 0x046e: // Luxemburgish
//            case 0x041d: // Swedish
//            case 0x0419: // Russian
//            case 0x0410: // Italian
//            case 0x0409: // United States
//            case 0x0407: // GERMAN
        default:
           this->key_event(device_flags, keycode);
           break;
    }
}


KeymapSym::KeySym KeymapSym::get_key(const uint16_t keyboardFlags, const uint16_t keyCode)
{
    enum {
           SCROLLLOCK  = 0x01
         , NUMLOCK     = 0x02
         , CAPSLOCK    = 0x04
         , FLG_SHIFT   = 0x08
         , FLG_CTRL    = 0x10
         , FLG_ALT     = 0x20
         , FLG_WINDOWS = 0x40
         , FLG_ALTGR   = 0x80
    };

    // The scancode and its extended nature are merged in a new variable (whose most significant bit indicates the extended nature)

//    uint16_t keyboardFlags_pos = keyboardFlags;
//    if (keyboardFlags_pos & KBDFLAGS_EXTENDED) {
//        keyboardFlags_pos -=  KBDFLAGS_EXTENDED;
//    }
//    uint8_t extendedKeyCode = keyCode|((keyboardFlags_pos >> 1)&0x80);

    // Commented code above is disabling all extended codes, putting them back
    uint8_t extendedKeyCode = keyCode|((keyboardFlags >> 1)&0x80);


    // TODO: see how it interacts with autorepeat
    // The state of that key is updated in the Keyboard status array (1=Make ; 0=Break)
    
    if (keyboardFlags & KBDFLAGS_RELEASE){ // up or down and released
       // Down and key released
       this->keys_down[extendedKeyCode] = 0; // up
    }
    else {
        this->keys_down[extendedKeyCode] = 1; // down
    }
    uint8_t downflag = this->keys_down[extendedKeyCode];

    // if ctrl+alt+fin or ctrl+alt+suppr -> insert delete
    if (is_ctrl_pressed() && is_alt_pressed()
    && ((extendedKeyCode == 0xCF)||(extendedKeyCode == 0x53))){
    //    Delete                           65535     0xffff
        extendedKeyCode = 0xD3;
    }

    switch (extendedKeyCode){
    //----------------
    // Lock keys
    //----------------
        // These keys are managed internally by proxy and never
        // transmitted to target system
        case 0x3A: // capslock
            if (this->keys_down[extendedKeyCode]){
                this->key_flags ^= CAPSLOCK;
            }
            return KeySym(0, downflag);
        case 0x45: // numlock
            if (this->keys_down[extendedKeyCode]){
                this->key_flags ^= NUMLOCK;
            }
            return KeySym(0, downflag);
        case 0x46: // scrolllock
            if (this->keys_down[extendedKeyCode]){
                this->key_flags ^= SCROLLLOCK;
            }
            return KeySym(0, downflag);
 
            //--------------------------------------------------------
            // KEYPAD : Keypad keys whose meaning depends on Numlock 
            //          are handled apart by the code below
            //          47 48 49 4B 4C 4D 4F 50 51 52 53
            //--------------------------------------------------------
            /* KP_4 or KEYPAD LEFT ARROW */
        case 0x4b:
                return KeySym((this->key_flags & NUMLOCK)?KS_KP_4:KS_Left, downflag);
        /* KP_8 or kEYPAD UP ARROW */
        case 0x48:
            return KeySym((this->key_flags & NUMLOCK)?KS_KP_8:KS_Up, downflag);
        /* KP_6 or KEYPAD RIGHT ARROW */
        case 0x4d:
            return KeySym((this->key_flags & NUMLOCK)?KS_KP_6:KS_Right, downflag);
        /* KP_2 or KEYPAD DOWN ARROW */
        case 0x50:
            return KeySym((this->key_flags & NUMLOCK)?KS_KP_2:KS_Down, downflag);
        /* Kp_9 or KEYPAD PGUP */
        case 0x49:
            return KeySym((this->key_flags & NUMLOCK)?KS_KP_9:KS_Prior, downflag);
        /* KP_3 or kEYPAD PGDOWN */
        case 0x51:
            return KeySym((this->key_flags & NUMLOCK)?KS_KP_3:KS_Next, downflag);
        /* KP_1 or KEYPAD END */
        case 0x4F:
            return KeySym((this->key_flags & NUMLOCK)?KS_KP_1:KS_End, downflag);
        /* kEYPAD EMPTY 5 */
        case 0x4c:
            return KeySym((this->key_flags & NUMLOCK)?KS_KP_5:0, downflag);
        /* kEYPAD HOME */
        case 0x47:
            return KeySym((this->key_flags & NUMLOCK)?KS_KP_7:KS_Home, downflag);
        /* KP_0 or kEYPAD INSER */
        case 0x52:
            return KeySym((this->key_flags & NUMLOCK)?'0':0xFF63, downflag);

    //----------------
    // All other keys
    //----------------
        default:
        {
            // This table translates the RDP scancodes to X11 scancodes :
            //  - the fist block (0-127) simply applies the +8 Windows to X11 translation and forces some 0 values
            //  - the second block (128-255) give codes for the extended keys that have a meaningful one
            // as in this code extended bit is cleared: it won't work

            uint8_t map[256] =  {
                0x00, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, // 0x00 - 0x07
                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, // 0x08 - 0x0f
                0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, // 0x10 - 0x17
                0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, // 0x18 - 0x1f
                0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, // 0x20 - 0x27
                0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, // 0x28 - 0x2f
                0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, // 0x30 - 0x37
                0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, // 0x38 - 0x3f
                0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, // 0x40 - 0x47
                0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, // 0x48 - 0x4f
                0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, // 0x50 - 0x57
                0x60, 0x61, 0x62, 0x00, 0x00, 0x00, 0x66, 0x67, // 0x58 - 0x5f
                0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, // 0x60 - 0x67
                0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, // 0x68 - 0x6f
                0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, // 0x70 - 0x77
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x78 - 0x7f

                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80 - 0x87
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x88 - 0x8f
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x90 - 0x97
                0x00, 0x00, 0x00, 0x00, 0x6c, 0x6d, 0x00, 0x00, // 0x98 - 0x9f
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xa0 - 0xa7
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xa8 - 0xaf
                0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x6f, // 0xb0 - 0xb7
                0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xb8 - 0xbf
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, // 0xc0 - 0xc7
                0x62, 0x63, 0x00, 0x64, 0x00, 0x66, 0x00, 0x67, // 0xc8 - 0xcf
                0x68, 0x69, 0x6a, 0x6b, 0x00, 0x00, 0x00, 0x00, // 0xd0 - 0xd7
                0x00, 0x00, 0x00, 0x73, 0x74, 0x75, 0x00, 0x00, // 0xd8 - 0xdf
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xe0 - 0xe7
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xe8 - 0xef
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xf0 - 0xf7
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // 0xf8 - 0xff
            } ;

            // Maps RDP Scancode to X11 code syms matrix
            // -----------------------------------------
            // The actual code to use will be looked-up in keyboard mapping tables

// KEYBOARD MAP WITH RDP SCANCODES
// -------------------------------
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +----+----+-------+
// | 01 |  | 3B | 3C | 3D | 3E |  | 3F | 40 | 41 | 42 |  | 43 | 44 | 57 | 58 |  | 37 | 46 | 1D+45 |
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +----+----+-------+
//                                     ***  keycodes suffixed by 'x' are extended ***
// +----+----+----+----+----+----+----+----+----+----+----+----+----+--------+  +----+----+----+  +--------------------+
// | 29 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 0A | 0B | 0C | 0D |   0E   |  | 52x| 47x| 49x|  | 45 | 35x| 37 | 4A  |
// +-------------------------------------------------------------------------+  +----+----+----+  +----+----+----+-----+
// |  0F  | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 1A | 1B |      |  | 53x| 4Fx| 51x|  | 47 | 48 | 49 |     |
// +------------------------------------------------------------------+  1C  |  +----+----+----+  +----+----+----| 4E  |
// |  3A   | 1E | 1F | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 2B |     |                    | 4B | 4C | 4D |     |
// +-------------------------------------------------------------------------+       +----+       +----+----+----+-----+
// |  2A | 56 | 2C | 2D | 2E | 2F | 30 | 31 | 32 | 33 | 34 | 35 |     36     |       | 48x|       | 4F | 50 | 51 |     |
// +-------------------------------------------------------------------------+  +----+----+----+  +---------+----| 1Cx |
// |  1D  |  5Bx | 38 |           39           |  38x  |  5Cx |  5Dx |  1Dx  |  | 4Bx| 50x| 4Dx|  |    52   | 53 |     |
// +------+------+----+------------------------+-------+------+------+-------+  +----+----+----+  +---------+----+-----+

// KEYBOARD MAP WITH X KSYM LAYOUT SCANCODES
// -----------------------------------------
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +----+----+-------+
// | 09 |  | 43 | 44 | 45 | 46 |  | 47 | 48 | 49 | 4A |  | 4B | 4C | 5F | 60 |  | 37 | 4E | 6DX   |
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +----+----+-------+
//                                     ***  keycodes suffixed by 'x' are extended ***
// +----+----+----+----+----+----+----+----+----+----+----+----+----+--------+  +----+----+----+  +--------------------+
// | 31 | 0A | 0B | 0C | 0D | 0E | 0F | 10 | 11 | 12 | 13 | 14 | 15 |   16   |  | 6AX| 61X| 63X|  | 4D | 70X| 3F | 52  |
// +-------------------------------------------------------------------------+  +----+----+----+  +----+----+----+-----+
// |  17  | 18 | 19 | 1A | 1B | 1C | 1D | 1E | 1F | 20 | 21 | 22 | 23 |      |  | 6BX| 67X| 69X|  | 4F | 50 | 51 |     |
// +------------------------------------------------------------------+  24  |  +----+----+----+  +----+----+----| 56  |
// |  42   | 26 | 27 | 28 | 29 | 2A | 2B | 2C | 2D | 2E | 2F | 30 | 33 |     |                    | 53 | 54 | 55 |     |
// +-------------------------------------------------------------------------+       +----+       +----+----+----+-----+
// |  32 | 56 | 34 | 35 | 36 | 37 | 38 | 39 | 3A | 3B | 3C | 3D |     3E     |       | 62X|       | 57 | 58 | 59 |     |
// +-------------------------------------------------------------------------+  +----+----+----+  +---------+----| 6CX |
// |  25  |  73X | 40 |           41           |  71X  |  74X |  75X |  6DX  |  | 64X| 68X| 66X|  |    5A   | 5B |     |
// +------+------+----+------------------------+-------+------+------+-------+  +----+----+----+  +---------+----+-----+

            const KeyLayout_t * layout = this->select_layout();

            /* KP_ POINT or kEYPAD DELETE */
            if (extendedKeyCode == 0x53){
                if (this->key_flags & NUMLOCK){
                    // This one has to be localized
                    layout = &this->keylayout_WORK_shift_sym;
                    uint8_t sym = map[extendedKeyCode];
                    uint32_t ksym = (*layout)[sym];
                    return KeySym(ksym, downflag);
                }
                else {
                    return KeySym(0xFFFF, downflag);
                }
                break;
            }

            if (this->verbose) {
                LOG(LOG_INFO, "Use KEYLAYOUT WORK no shift");
            }

            // Translate the scancode to a KeySym
            //----------------------------------------
            uint8_t sym = map[extendedKeyCode];
            uint32_t ksym = (*layout)[sym];
//            if (this->verbose){
                LOG(LOG_INFO, "extendedKeyCode=0x%X sym=0x%X ksym=0x%X", extendedKeyCode, sym, ksym);
//            }
            if ((ksym == 0xFE52 ) // DEADKEYS 
            || (ksym == 0xFE57) 
            || (ksym == 0x60) 
            || (ksym == 0x7E)) {

                LOG(LOG_INFO, "deadkey=0x%X", ksym);

                //-------------------------------------------------
                // ksym is NOT in Printable unicode character range
                //-------------------------------------------------
                // That is, A dead key (0xFE52 (^), 0xFE57 ("), 0x60 (`), 0x7E (~) )
                // The flag is set accordingly
                switch (extendedKeyCode){
                    case 0x1A:
                        this->is_shift_pressed() ? this->dead_key = DEADKEY_UML : this->dead_key = DEADKEY_CIRC;
                        break;
                    case 0x08:
                        this->dead_key = DEADKEY_GRAVE;
                        break;
                    case 0x03:
                        this->dead_key = DEADKEY_TILDE;
                        break;
                    default:
                        break;
                } // Switch extendedKeyCode
                return KeySym(0, 0);
            }

            //-------------------------------------------------
            // ksym is in Printable character range.
            //-------------------------------------------------
            if (this->dead_key != DEADKEY_NONE){
                uint32_t current_dead_key = this->dead_key;
                // if releasing next key after key: disable dead_key mode for following keys
                if (this->keys_down[extendedKeyCode])
                {
                    this->dead_key = DEADKEY_NONE;
                }
                switch (current_dead_key){
                    case DEADKEY_CIRC:
                        switch (ksym){
                            case 'a':
                                return KeySym(0xE2, downflag); // unicode for â (acirc)
                            case 'A':
                                return KeySym(0xC2, downflag); // unicode for Â (Acirc)
                            case 'e':
                                return KeySym(0xEA, downflag); // unicode for ê (ecirc)
                            case 'E':
                                return KeySym(0xCA, downflag); // unicode for Ê (Ecirc)
                            case 'i':
                                return KeySym(0xEE, downflag); // unicode for î (icirc)
                            case 'I':
                                return KeySym(0xCE, downflag); // unicode for Î (Icirc)
                            case 'o':
                                return KeySym(0xF4, downflag); // unicode for ô (ocirc)
                            case 'O':
                                return KeySym(0xD4, downflag); // unicode for Ô (Ocirc)
                            case 'u':
                                return KeySym(0xFB, downflag); // unicode for û (ucirc)
                            case 'U':
                                return KeySym(0xDB, downflag); // unicode for Û (Ucirc)
                            case ' ':
                                return KeySym(0x5E, downflag); // unicode for ^ (caret)
                            default:
                                return KeySym(ksym, downflag); // unmodified unicode
                        }
                        break;

                    case DEADKEY_UML:
                        switch (ksym){
                            case 'a':
                                return KeySym(0xE4, downflag); // unicode for ä (auml)
                            case 'A':
                                return KeySym(0xC4, downflag); // unicode for Ä (Auml)
                            case 'e':
                                return KeySym(0xEB, downflag); // unicode for ë (euml)
                            case 'E':
                                return KeySym(0xCB, downflag); // unicode for Ë (Euml)
                            case 'i':
                                return KeySym(0xEF, downflag); // unicode for ï (iuml)
                            case 'I':
                                return KeySym(0xCF, downflag); // unicode for Ï (Iuml)
                            case 'o':
                                return KeySym(0xF6, downflag); // unicode for ö (ouml)
                            case 'O':
                                return KeySym(0xD6, downflag); // unicode for Ö (Ouml)
                            case 'u':
                                return KeySym(0xFC, downflag); // unicode for ü (uuml)
                            case 'U':
                                return KeySym(0xDC, downflag); // unicode for Ü (Uuml)
                            case ' ':
                                return KeySym(0xA8, downflag); // unicode for " (umlaut)
                            default:
                                return KeySym(ksym, downflag); // unmodified unicode
                        }
                        break;
                    case DEADKEY_GRAVE:
                        switch (ksym){
                            case 'a':
                                return KeySym(0xE0, downflag); // unicode for à (agrave)
                            case 'A':
                                return KeySym(0xC0, downflag); // unicode for À (Agrave)
                            case 'e':
                                return KeySym(0xE8, downflag); // unicode for è (egrave)
                            case 'E':
                                return KeySym(0xC8, downflag); // unicode for È (Egrave)
                            case 'i':
                                return KeySym(0xEC, downflag); // unicode for ì (igrave)
                            case 'I':
                                return KeySym(0xCC, downflag); // unicode for Ì (Igrave)
                            case 'o':
                                return KeySym(0xF2, downflag); // unicode for ò (ograve)
                            case 'O':
                                return KeySym(0xD2, downflag); // unicode for Ò (Ograve)
                            case 'u':
                                return KeySym(0xF9, downflag); // unicode for ù (ugrave)
                            case 'U':
                                return KeySym(0xD9, downflag); // unicode for Ù (Ugrave)
                            case ' ':
                                return KeySym(0x60, downflag); // unicode for ` (backquote)
                            default:
                                return KeySym(ksym, downflag); // unmodified unicode
                        }
                        break;
                    case DEADKEY_TILDE:
                        switch (ksym){
                            case 'n':
                                return KeySym(0xF1, downflag); // unicode for ~n (ntilde)
                            case 'N':
                                return KeySym(0xD1, downflag); // unicode for ~N (Ntilde)
                            case ' ':
                                return KeySym(0x7E, downflag); // unicode for ~ (tilde)
                            default:
                                return KeySym(ksym, downflag); // unmodified unicode
                        }
                        break;
                    default:
                        return KeySym(ksym, downflag); // unmodified unicode
                } // Switch DEAD_KEY
            } // Is a dead Key
            else {
                // If previous key wasn't a dead key, simply return it
                return KeySym(ksym, downflag);
            }
        } // END if KEYPAD specific / else
        break;
    } // END SWITCH : ExtendedKeyCode
    // Should never happen
    return KeySym(0, 0);
} // END FUNCT : get_key


// Push only sym
void KeymapSym::push_sym(KeySym sym)
{
    if (this->verbose & 2){
        LOG(LOG_INFO, "KeymapSym::push_sym(sym=%08x) nbuf_sym=%u", sym.sym, this->nbuf_sym);
    }
    if (sym.sym == 0) return;
    if (this->nbuf_sym < SIZE_KEYBUF_SYM){
        this->buffer_sym[this->ibuf_sym] = sym;
        this->ibuf_sym++;
        if (this->ibuf_sym >= SIZE_KEYBUF_SYM){
            this->ibuf_sym = 0;
        }
        this->nbuf_sym++;
    }
}

uint32_t KeymapSym::get_sym(uint8_t & downflag)
{
    if (this->nbuf_sym > 0){
        auto & s = this->buffer_sym[(SIZE_KEYBUF_SYM + this->ibuf_sym - this->nbuf_sym) % SIZE_KEYBUF_SYM];
        uint32_t res = s.sym;
        downflag = s.down;

        if (this->nbuf_sym > 0){
            this->nbuf_sym--;
            if (this->nbuf_sym == 0){
                this->ibuf_sym = 0;
            }
        }
        return res;
    }
    return 0;
}

uint32_t KeymapSym::nb_sym_available() const
{
    return this->nbuf_sym;
}

bool KeymapSym::is_caps_locked() const
{
    return this->key_flags & CAPSLOCK;
}

bool KeymapSym::is_scroll_locked() const
{
    return this->key_flags & SCROLLLOCK;
}

bool KeymapSym::is_num_locked() const
{
    return this->key_flags & NUMLOCK;
}

void KeymapSym::toggle_num_lock(bool on)
{
    if (((this->key_flags & NUMLOCK) == NUMLOCK) != on) {
        this->key_flags ^= NUMLOCK;
    }
}

bool KeymapSym::is_left_shift_pressed() const
{
    return this->keys_down[LEFT_SHIFT];
}

bool KeymapSym::is_right_shift_pressed() const
{
    return this->keys_down[RIGHT_SHIFT];
}

bool KeymapSym::is_shift_pressed() const
{
    return this->is_left_shift_pressed() || this->is_right_shift_pressed();
}

bool KeymapSym::is_left_ctrl_pressed() const
{
    return this->keys_down[LEFT_CTRL];
}

bool KeymapSym::is_right_ctrl_pressed() const
{
    return this->keys_down[RIGHT_CTRL];
}

bool KeymapSym::is_ctrl_pressed() const
{
    return is_right_ctrl_pressed() || is_left_ctrl_pressed();
}

bool KeymapSym::is_left_alt_pressed() const
{
    return this->keys_down[LEFT_ALT];
}

bool KeymapSym::is_right_alt_pressed() const // altgr
{
    return this->keys_down[RIGHT_ALT];
}

bool KeymapSym::is_alt_pressed() const
{
    return is_right_alt_pressed() || is_left_alt_pressed();
}

bool KeymapSym::is_altgr_pressed() const
{
    return ((this->is_ctrl_pressed() && this->is_left_alt_pressed()) || (this->is_right_alt_pressed()));
}

#include "keyboard/keymapsymlayouts.hpp"
