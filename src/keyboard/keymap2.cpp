/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Dominique Lafages, Raphael Zhou,
              Meng Tan

   header file. Keymap2 object, used to manage key stroke events
*/

#include "keyboard/keymap2.hpp"
#include "keyboard/keylayouts.hpp"
#include "utils/log.hpp"

#include <cinttypes>


namespace
{
    static const Keylayout * keylayouts[] = {
        &keylayout_x00000405, &keylayout_x00000406, &keylayout_x00000407, &keylayout_x00000408,
        &keylayout_x00000409, &keylayout_x0000040a, &keylayout_x0000040b, &keylayout_x0000040c,
        &keylayout_x0000040f, &keylayout_x00000410, &keylayout_x00000413, &keylayout_x00000414,
        &keylayout_x00000415, &keylayout_x00000416, &keylayout_x00000418, &keylayout_x00000419,
        &keylayout_x0000041a, &keylayout_x0000041b, &keylayout_x0000041d, &keylayout_x0000041f,
        &keylayout_x00000422, &keylayout_x00000424, &keylayout_x00000425, &keylayout_x00000426,
        &keylayout_x00000427, &keylayout_x0000042f, &keylayout_x00000438, &keylayout_x0000043a,
        &keylayout_x0000043b, &keylayout_x0000043f, &keylayout_x00000440, &keylayout_x00000444,
        &keylayout_x00000450, &keylayout_x00000452, &keylayout_x0000046e, &keylayout_x00000481,
        &keylayout_x00000807, &keylayout_x00000809, &keylayout_x0000080a, &keylayout_x0000080c,
        &keylayout_x00000813, &keylayout_x00000816, &keylayout_x0000081a, &keylayout_x0000083b,
        &keylayout_x00000843, &keylayout_x0000085d, &keylayout_x00000c0c, &keylayout_x00000c1a,
        &keylayout_x00001009, &keylayout_x0000100c, &keylayout_x0000201a, &keylayout_x00010402,
        &keylayout_x00010405, &keylayout_x00001809, &keylayout_x00010407, &keylayout_x00010408,
        &keylayout_x0001040a, &keylayout_x0001040e, &keylayout_x00010409, &keylayout_x00010410,
        &keylayout_x00010415, &keylayout_x00010416, &keylayout_x00010419, &keylayout_x0001041b,
        &keylayout_x0001041f, &keylayout_x00010426, &keylayout_x00010427, &keylayout_x0001043a,
        &keylayout_x0001043b, &keylayout_x0001080c, &keylayout_x0001083b, &keylayout_x00011009,
        &keylayout_x00011809, &keylayout_x00020405, &keylayout_x00020408, &keylayout_x00020409,
        &keylayout_x0002083b, &keylayout_x00030402, &keylayout_x00030408, &keylayout_x00030409,
        &keylayout_x00040408, &keylayout_x00040409, &keylayout_x00050408, &keylayout_x00060408
    };
}

inline Keymap2::KeyFlags& operator^=(Keymap2::KeyFlags& f, Keymap2::KeyFlags const& x) noexcept
{
    f = Keymap2::KeyFlags(int(f) ^ int(x));
    return f;
}

Keymap2::Keymap2(uint32_t verbose)
    : keys_down{}
    , key_flags(NoFlag)
    , ibuf(0)
    , nbuf(0)
    , buffer{}
    , ibuf_kevent(0)
    , nbuf_kevent(0)
    , buffer_kevent{}
    , deadkey(DEADKEY_NONE)
    , keylayout_WORK(&default_layout())
    , verbose(verbose)
{
}

void Keymap2::synchronize(uint16_t param1)
{
    this->key_flags = KeyFlags(param1 & (SCROLLLOCK | NUMLOCK | CAPSLOCK));
    // non sticky keys are forced to be UP
    // TODO Non sticky keys are forced in up state. Is it what we should do ? We have up and down events for these key anyway hence up is not likelier than down (really it should be 'unknown', but we do not have this state")
    this->keys_down[LEFT_SHIFT] = 0;
    this->keys_down[RIGHT_SHIFT] = 0;
    this->keys_down[LEFT_CTRL] = 0;
    this->keys_down[RIGHT_CTRL] = 0;
    this->keys_down[LEFT_ALT] = 0;
    this->keys_down[RIGHT_ALT] = 0;
}


void Keymap2::DecodedKeys::set_uchar(uint32_t uchar)
{
    assert(this->count < 2);
    this->uchars[this->count++] = uchar;
}

Keymap2::DecodedKeys Keymap2::event(const uint16_t keyboardFlags, const uint16_t keyCode, bool & tsk_switch_shortcuts)
{
    DecodedKeys decoded_key;

    // The scancode and its extended nature are merged in a new variable (whose most significant bit indicates the extended nature)
    uint8_t extendedKeyCode = keyCode|((keyboardFlags >> 1)&0x80);
    // The state of that key is updated in the Keyboard status array (1=Make ; 0=Break)
    this->keys_down[extendedKeyCode] = !(keyboardFlags & KBDFLAGS_RELEASE);

    {
        bool const ctrl = this->keys_down[LEFT_CTRL] || this->keys_down[RIGHT_CTRL];

        tsk_switch_shortcuts =
            ctrl &&                                                             // Ctrl
            (this->keys_down[LEFT_ALT] || this->keys_down[RIGHT_ALT]) &&        // Alt
            (this->keys_down[0xD3] || this->keys_down[0x53]);                   // Del (or Numpad del)
        tsk_switch_shortcuts |=
            ctrl &&                                                             // Ctrl
            (this->keys_down[LEFT_SHIFT] || this->keys_down[RIGHT_SHIFT]) &&    // Shift
            this->keys_down[0x01];                                              // Escape

        this->is_application_switching_shortcut_pressed =
            (ctrl ^ bool(this->keys_down[LEFT_ALT])) &&                         // Ctrl xor Alt
            !this->keys_down[RIGHT_ALT] &&
            this->keys_down[0x0F];                                              // Tab
    }

    if (is_ctrl_pressed() && is_alt_pressed()
    && ((extendedKeyCode == 207)||(extendedKeyCode == 83))){
        //    Delete                           65535     0xffff
        extendedKeyCode = 211; // SUPPR
    }

    switch (extendedKeyCode){
    //----------------
    // Lock keys
    //----------------
        case 0x3A: // capslock
            if (this->keys_down[extendedKeyCode]){
                this->key_flags ^= CAPSLOCK;
            }
            break;
        case 0x45: // numlock
            if (this->keys_down[extendedKeyCode]){
                this->key_flags ^= NUMLOCK;
            }
            break;
        case 0x46: // scrolllock
            if (this->keys_down[extendedKeyCode]){
                this->key_flags ^= SCROLLLOCK;
            }
            break;
    //----------------
    // Modifier keys
    //----------------
        case LEFT_SHIFT:  // left shift
            this->key_flags ^= FLG_SHIFT;
            break;
        case RIGHT_SHIFT: // right shift
            this->key_flags ^= FLG_SHIFT;
            break;
        case LEFT_CTRL:   // left ctrl
            this->key_flags ^= FLG_CTRL;
            break;
        case RIGHT_CTRL:  // right ctrl
            this->key_flags ^= FLG_CTRL;
            break;
        case LEFT_ALT:    // left alt
            this->key_flags ^= FLG_ALT;
            break;
        case RIGHT_ALT:   // right alt
            this->key_flags ^= FLG_ALTGR;
            break;
    //----------------
    // All other keys
    //----------------
        default: // all other codes
            // This table translates the RDP scancodes to X11 scancodes :
            //  - the fist block (0-127) forces some values to 0
            //  - the second block (128-255) give the code for the extended keys that have a meaningful one
            uint8_t map[256] =  {

                 0x0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 0x00 - 0x07
                0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, // 0x08 - 0x0f
                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, // 0x10 - 0x17
                0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, // 0x18 - 0x1f
                0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, // 0x20 - 0x27
                0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, // 0x28 - 0x2f
                0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, // 0x30 - 0x37
                0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, // 0x38 - 0x3f
                0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, // 0x40 - 0x47
                0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, // 0x48 - 0x4f
                0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, // 0x50 - 0x57
                0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, // 0x58 - 0x5f
                0x60, 0x61, 0x62,  0x0,  0x0,  0x0, 0x66, 0x67, // 0x60 - 0x67
                0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, // 0x68 - 0x6f
                0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, // 0x70 - 0x77
                0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, // 0x78 - 0x7f

                 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // 0x80 - 0x87
                 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // 0x88 - 0x8f
                 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // 0x90 - 0x97
                 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // 0x98 - 0x9f
                 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // 0xa0 - 0xa7
                 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // 0xa8 - 0xaf
                 0x0,  0x0,  0x0,  0x0,  0x0, 0x62,  0x0,  0x0, // 0xb0 - 0xb7
                 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // 0xb8 - 0xbf
                 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // 0xc0 - 0xc7
                 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // 0xc8 - 0xcf
                 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // 0xd0 - 0xd7
                 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // 0xd8 - 0xdf
                 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // 0xe0 - 0xe7
                 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // 0xe8 - 0xef
                 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // 0xf0 - 0xf7
                 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, // 0xf8 - 0xff
            } ;

            // if event is a Make
            if (this->keys_down[extendedKeyCode]){
                if (this->verbose){
                    LOG(LOG_INFO, "Event is Make for key: Ox%#02x", static_cast<unsigned>(extendedKeyCode));
                }

                //this->last_char_key = extendedKeyCode;

                //-------------------------------------------------------------------------
                // KEYPAD : Keypad keys whose meanings depends on Numlock are handled apart
                //-------------------------------------------------------------------------
                if ( ( (extendedKeyCode >= 0x47) && (extendedKeyCode <= 0x49) )
                  || ( (extendedKeyCode >= 0x4b) && (extendedKeyCode <= 0x4d) )
                  || ( (extendedKeyCode >= 0x4f) && (extendedKeyCode <= 0x53) )
                   ){
                    if (this->verbose){
                        LOG(LOG_INFO, "Key from keypad: 0x%02x", static_cast<unsigned>(extendedKeyCode));
                    }

                    // if numlock is activated AND shift is up, keys are printable characters
                    if (  (this->key_flags & NUMLOCK)
                       && ( ! this->is_shift_pressed())
                       ) {
                        const Keylayout::KeyLayout_t * layout
                          = (this->is_ctrl_pressed())
                           ? &this->keylayout_WORK->ctrl
                           : (this->is_ctrl_pressed() && this->is_alt_pressed())
                            ? &this->keylayout_WORK->altGr
                            : &this->keylayout_WORK->noMod;
                        // Translate the incoming RDP scancode to a X11 scancode
                        uint8_t sym = map[extendedKeyCode];
                        // Translate the X11 scancode to an unicode code point
                        uint32_t uchar = (*layout)[sym];
                        decoded_key.set_uchar(uchar);
                        this->push(uchar);
                    }
                    // if numlock is not activated OR shift is down, keys are NOT printable characters
                    else {
                        //LOG(LOG_INFO, "extendedKeyCode=0x%02X", extendedKeyCode);
                        switch (extendedKeyCode){
                           /* kEYPAD LEFT ARROW */
                            case 0x4b:
                                decoded_key.set_uchar(0x2190);
                                this->push_kevent(KEVENT_LEFT_ARROW);
                                break;
                            /* kEYPAD UP ARROW */
                            case 0x48:
                                decoded_key.set_uchar(0x2191);
                                this->push_kevent(KEVENT_UP_ARROW);
                                break;
                            /* kEYPAD RIGHT ARROW */
                            case 0x4d:
                                decoded_key.set_uchar(0x2192);
                                this->push_kevent(KEVENT_RIGHT_ARROW);
                                break;
                            /* kEYPAD DOWN ARROW */
                            case 0x50:
                                decoded_key.set_uchar(0x2193);
                                this->push_kevent(KEVENT_DOWN_ARROW);
                                break;
                            /* kEYPAD HOME */
                            case 0x47:
                                decoded_key.set_uchar(0x2196);
                                this->push_kevent(KEVENT_HOME);
                                break;
                            /* kEYPAD PGUP */
                            case 0x49:
                                this->push_kevent(KEVENT_PGUP);
                                break;
                            /* kEYPAD PGDOWN */
                            case 0x51:
                                this->push_kevent(KEVENT_PGDOWN);
                                break;
                            /* kEYPAD END */
                            case 0x4F:
                                decoded_key.set_uchar(0x2198);
                                this->push_kevent(KEVENT_END);
                                break;
                            /* kEYPAD INSERT */
                            case 0x52:
                                this->push_kevent(KEVENT_INSERT);
                                break;
                            /* kEYPAD DELETE */
                            case 0x53:
                                decoded_key.set_uchar(0x007F);
                                this->push_kevent(KEVENT_DELETE);
                                break;
                            default:
                                break;
                        }
                    }
                }
                //--------------------
                // NOT KEYPAD Specific
                //--------------------
                else {

                    if (this->verbose){
                        LOG(LOG_INFO, "Key not from keypad: 0x%02x", static_cast<unsigned>(extendedKeyCode));
                    }

                    // Set the layout block to be used, depending on active modifier keys and capslock status
                    const Keylayout::KeyLayout_t * layout;
                    if (this->is_caps_locked()) {
                        if (this->is_ctrl_alt_pressed() && this->is_shift_pressed()){
                            layout = &this->keylayout_WORK->capslock_shiftAltGr;
                        }
                        else if (this->is_ctrl_alt_pressed()){
                            layout = &this->keylayout_WORK->capslock_altGr;
                        }
                        else if (this->is_ctrl_pressed()){
                            layout = &this->keylayout_WORK->ctrl;
                        }
                        else if (this->is_shift_pressed()){
                            layout = &this->keylayout_WORK->capslock_shift;
                        }
                        else{
                            layout = &this->keylayout_WORK->capslock_noMod;
                        }
                    }
                    else {
                        if (this->is_ctrl_alt_pressed() && this->is_shift_pressed()){
                            layout = &this->keylayout_WORK->shiftAltGr;
                        }
                        else if (this->is_ctrl_alt_pressed()){
                            layout = &this->keylayout_WORK->altGr;
                        }
                        else if (this->is_ctrl_pressed()){
                            layout = &this->keylayout_WORK->ctrl;
                        }
                        else if (this->is_shift_pressed()){

                            layout = &this->keylayout_WORK->shift;
                        }
                        else{
                            layout = &this->keylayout_WORK->noMod;
                        }
                    }
                    // Translate the RDP scancode to an X11 scancode
                    uint8_t sym = map[extendedKeyCode];
                    // Translate the X11 scancode to an unicode code point
                    uint32_t uchar = (*layout)[sym];

                    if (this->verbose){
                        LOG(LOG_INFO, "uchar=0x%02" PRIx32, uchar);
                    }
                    //----------------------------------------------
                    // uchar is in Printable unicode character range
                    //----------------------------------------------
                    // That is :
                    //  * > 0x20 is for ruling out NUL, but also TAB, ESC and BACKSPACE that has unicode values but
                    //           are not actually printable characters and that we don't want to track
                    //  * And not delete (0x7f) nor a dead key (0x5e, 0xa8, 0x60)
                    if (this->verbose){
                        LOG(LOG_INFO, "nb chars in buffer: %u nb events in buffer %u\n", this->nbuf, this->nbuf_kevent);
                    }
                    if (  (uchar >= 0x20)                                               // Not an ASCII Control
                       && (uchar != 0x7F)                                               // Not the Backspace ASCII code
                       && not (this->keylayout_WORK->isDeadkey(uchar, extendedKeyCode)) // Not a deadkey
                       )
                    {
                        if (this->verbose){
                            LOG(LOG_INFO, "Printable key : uchar=Ox%x02\n", uchar);
                        }
                        // If previous key was a dead key, push a translated unicode char
                        if (this->deadkey == DEADKEY_FOUND){
                            if (this->verbose){
                                LOG(LOG_INFO, "Dead key : uchar=0x%02x", uchar);
                            }
                            bool deadkeyTranslated = false;
                            // Search for uchar to translate in the current DEADKEY entry
                            for (uint8_t i = 0; i < this->deadkey_pending_def.nbSecondKeys; i++) {
                                 if (this->deadkey_pending_def.secondKeys[i].secondKey == uchar) {

                                    // push the translation into keyboard buffer
                                    decoded_key.set_uchar(this->deadkey_pending_def.secondKeys[i].modifiedKey);
                                    this->push(this->deadkey_pending_def.secondKeys[i].modifiedKey);
                                    deadkeyTranslated = true;
                                    break;
                                }
                            }
                            // If that second key is not associated with that deadkey,
                            // push both deadkey uchar and unmodified second key uchar in keyboard buffer
                            if (not deadkeyTranslated) {
                                decoded_key.set_uchar(this->deadkey_pending_def.uchar);
                                this->push(this->deadkey_pending_def.uchar);
                                decoded_key.set_uchar(uchar);
                                this->push(uchar);
                            }
                            this->deadkey = DEADKEY_NONE;
                        }
                        // If previous key wasn't a dead key, simply push
                        else {
                            if (this->verbose){
                                LOG(LOG_INFO, "not dead key - so pushing char %02x", uchar);
                            }
                            decoded_key.set_uchar(uchar);
                            this->push(uchar);
                        }
                    }
                    //--------------------------------------------------
                    // uchar is NOT in Printable unicode character range
                    //--------------------------------------------------
                    else {
                        if (this->verbose) {
                            LOG(LOG_INFO, "pushing event extendedKeyCode = >0x%02x<", static_cast<unsigned>(extendedKeyCode));
                        }

                        // Test if the extendedKeyCode is a deadkey in the current keyboard layout
                        bool check_extendedkey = true;
                        for (int i=0; i < keylayout_WORK->nbDeadkeys; i++) {
                            if (   (keylayout_WORK->deadkeys[i].uchar == uchar)
                               and (keylayout_WORK->deadkeys[i].extendedKeyCode == extendedKeyCode)
                               )
                            {
                                if (this->deadkey == DEADKEY_FOUND) {
                                    check_extendedkey = false;
                                    decoded_key.set_uchar(this->deadkey_pending_def.uchar);
                                    this->push(this->deadkey_pending_def.uchar);
                                    decoded_key.set_uchar(uchar);
                                    this->push(uchar);
                                    this->deadkey = DEADKEY_NONE;
                                }
                                else {
                                    this->deadkey = DEADKEY_FOUND;
                                    // set the deadkey definition pointer with the current deadkey definition
                                    this->deadkey_pending_def = keylayout_WORK->deadkeys[i];
                                }
                                break;
                            }
                        }

                        if (check_extendedkey) {
                            //LOG(LOG_INFO, "extendedKeyCode=0x%02X", extendedKeyCode);
                            switch (extendedKeyCode){
                            // ESCAPE
                            case 0x01:
                                decoded_key.set_uchar(0x001B);
                                this->push_kevent(KEVENT_ESC);
                                break;
                            // LEFT ARROW
                            case 0xCB:
                                decoded_key.set_uchar(0x2190);
                                this->push_kevent(KEVENT_LEFT_ARROW);
                                break;
                            // UP ARROW
                            case 0xC8:
                                decoded_key.set_uchar(0x2191);
                                this->push_kevent(KEVENT_UP_ARROW);
                                break;
                            // RIGHT ARROW
                            case 0xCD:
                                decoded_key.set_uchar(0x2192);
                                this->push_kevent(KEVENT_RIGHT_ARROW);
                                break;
                            // DOWN ARROW
                            case 0xD0:
                                decoded_key.set_uchar(0x2193);
                                this->push_kevent(KEVENT_DOWN_ARROW);
                                break;
                            // HOME
                            case 0xC7:
                                decoded_key.set_uchar(0x2196);
                                this->push_kevent(KEVENT_HOME);
                                break;
                            // PGUP
                            case 0xC9:
                                this->push_kevent(KEVENT_PGUP);
                                break;
                            // PGDOWN
                            case 0xD1:
                                this->push_kevent(KEVENT_PGDOWN);
                                break;
                            // END
                            case 0xCF:
                                decoded_key.set_uchar(0x2198);
                                this->push_kevent(KEVENT_END);
                                break;
                            // TAB
                            case 0x0F:
                                if (this->is_shift_pressed()){
                                    this->push_kevent(KEVENT_BACKTAB);
                                }
                                else {
                                    decoded_key.set_uchar(0x0009);
                                    this->push_kevent(KEVENT_TAB);
                                }
                                break;
                            // backspace
                            case 0x0E:
                                // Windows(c) behavior for backspace following a Deadkey
                                if (this->deadkey == DEADKEY_NONE) {
                                    decoded_key.set_uchar(0x0008);
                                    this->push_kevent(KEVENT_BACKSPACE);
                                }
                                else {
                                    this->deadkey = DEADKEY_NONE;
                                }
                                break;
                            case 0xD2: // insert
                            case 0x52: // numpad insert
                                this->push_kevent(KEVENT_INSERT);
                                break;
                            case 0xD3: // delete
                            case 0x53: // numpad delete
                                decoded_key.set_uchar(0x007F);
                                this->push_kevent(KEVENT_DELETE);
                                break;
                            case 0x1C: // enter
                                decoded_key.set_uchar(0x000D);
                                this->push_kevent(KEVENT_ENTER);
                                break;
                            case 0x9C: // numpad enter
                                decoded_key.set_uchar(0x000D);
                                this->push_kevent(KEVENT_ENTER);
                                break;
                            case 45: // ctrl+x
                                this->push_kevent(KEVENT_CUT);
                                break;
                            case 46: // ctrl+c
                                this->push_kevent(KEVENT_COPY);
                                break;
                            case 47: // ctrl+v
                                this->push_kevent(KEVENT_PASTE);
                                break;
                            default:
                                break;
                            }
                        } // IF no deadkey pending
                    } // IF printable ELSE not printable
                } // IF Keypad ELSE not Keypad
            } // IF event is a Make
        break;
    } // END SWITCH : extendedKeyCode

    return decoded_key;
} // END METHOD : event


void Keymap2::push(uint32_t uchar)
{
    this->push_char(uchar);
    this->push_kevent(KEVENT_KEY);

} // END METHOD : push

void Keymap2::push_char(uint32_t uchar)
{
    if (this->nbuf < SIZE_KEYBUF){
        this->buffer[this->ibuf] = uchar;
        this->ibuf++;
        if (this->ibuf >= SIZE_KEYBUF){
            this->ibuf = 0;
        }
        this->nbuf++;
    }

} // END METHOD : push_char

uint32_t Keymap2::get_char()
{
    if (this->nbuf > 0){
        // remove top KEY KEVENT if present and any event may have occured before it
        if (this->nbuf_kevent > 0 && this->top_kevent() == KEVENT_KEY){
            this->nbuf_kevent--;
        }
        uint32_t res = this->buffer[(SIZE_KEYBUF + this->ibuf - this->nbuf) % SIZE_KEYBUF];

        if (this->nbuf > 0){
            this->nbuf--;
        }
        return res;
    }
    return 0;
} // END METHOD : get_char


// head of keyboard buffer (or keyboard buffer of size 1)
uint32_t Keymap2::top_char() const
{
    return this->buffer[(SIZE_KEYBUF + this->ibuf - this->nbuf) % SIZE_KEYBUF];

} // END METHOD : top_char


uint32_t Keymap2::nb_char_available() const
{
    return this->nbuf;

} // END METHOD : nb_char_available


void Keymap2::push_kevent(uint32_t uevent)
{
    if (this->nbuf_kevent < SIZE_KEYBUF_KEVENT){
        this->buffer_kevent[this->ibuf_kevent] = uevent;
        this->ibuf_kevent++;
        if (this->ibuf_kevent >= SIZE_KEYBUF_KEVENT){
            this->ibuf_kevent = 0;
        }
        this->nbuf_kevent++;
    }
} // END METHOD : push_kevent


uint32_t Keymap2::get_kevent()
{
    uint32_t res = this->buffer_kevent[(SIZE_KEYBUF_KEVENT + this->ibuf_kevent - this->nbuf_kevent) % SIZE_KEYBUF_KEVENT];

    if (this->nbuf_kevent > 0){
        if (res == KEVENT_KEY && this->nbuf > 0){
            this->nbuf--;
        }
        this->nbuf_kevent--;
    }
    return res;

} // END METHOD : get_kevent


// head of keyboard buffer (or keyboard buffer of size 1)
uint32_t Keymap2::top_kevent() const
{
    return this->buffer_kevent[this->ibuf_kevent?this->ibuf_kevent-1:SIZE_KEYBUF_KEVENT-1];

} // END METHOD : top_kevent


uint32_t Keymap2::nb_kevent_available() const
{
    return this->nbuf_kevent;

} // END METHOD : nb_kevent_available


bool Keymap2::is_caps_locked() const
{
    return this->key_flags & CAPSLOCK;

} // END METHOD : is_caps_locked

void Keymap2::toggle_caps_lock(bool on)
{
    if (((this->key_flags & CAPSLOCK) == CAPSLOCK) != on) {
        this->key_flags ^= CAPSLOCK;
    }
}


bool Keymap2::is_scroll_locked() const
{
    return this->key_flags & SCROLLLOCK;

} // END METHOD : is_scroll_locked

void Keymap2::toggle_scroll_lock(bool on)
{
    if (((this->key_flags & SCROLLLOCK) == SCROLLLOCK) != on) {
        this->key_flags ^= SCROLLLOCK;
    }
}


bool Keymap2::is_num_locked() const
{
    return this->key_flags & NUMLOCK;

} // END METHOD : is_num_locked

void Keymap2::toggle_num_lock(bool on)
{
    if (((this->key_flags & NUMLOCK) == NUMLOCK) != on) {
        this->key_flags ^= NUMLOCK;
    }
}


bool Keymap2::is_left_shift_pressed() const
{
    return this->keys_down[LEFT_SHIFT];

} // END METHOD : is_left_shift_pressed


bool Keymap2::is_right_shift_pressed() const
{
    return this->keys_down[RIGHT_SHIFT];

} // END METHOD : is_right_shift_pressed


bool Keymap2::is_shift_pressed() const
{
    return this->is_left_shift_pressed() || this->is_right_shift_pressed();

} // END METHOD : is_shift_pressed


bool Keymap2::is_ctrl_alt_pressed() const
{
    // TODO This is a hack to know if there is a Altgr map defined or not. It should be set through map generator... not reading map content like here
    bool has_AltGr = (this->keylayout_WORK->altGr[0x12] | this->keylayout_WORK->altGr[2]) != 0;
    return ((this->is_ctrl_pressed() && this->is_alt_pressed())
        || (this->is_right_alt_pressed() && has_AltGr));
}

bool Keymap2::is_left_ctrl_pressed() const
{
    return this->keys_down[LEFT_CTRL];

} // END METHOD : is_left_ctrl_pressed


bool Keymap2::is_right_ctrl_pressed() const
{
    return this->keys_down[RIGHT_CTRL];

} // END METHOD : is_right_ctrl_pressed


bool Keymap2::is_ctrl_pressed() const
{
    return is_right_ctrl_pressed() || is_left_ctrl_pressed();

} // END METHOD : is_ctrl_pressed


bool Keymap2::is_left_alt_pressed() const
{
    return this->keys_down[LEFT_ALT];

} // END METHOD : is_left_alt_pressed


bool Keymap2::is_right_alt_pressed() const // altGr
{
    return this->keys_down[RIGHT_ALT];

} // END METHOD : is_right_alt_pressed


bool Keymap2::is_alt_pressed() const
{
    return is_right_alt_pressed() || is_left_alt_pressed();

} // END METHOD : is_alt_pressed


void Keymap2::init_layout(int LCID)
{
    bool found = false;
    for (auto * k : keylayouts()) {
        if (k->LCID == LCID){
            this->keylayout_WORK = k;
            found = true;
            break;
        }
    }
    if (!found){
        LOG(LOG_INFO, "Unknown keyboard layout #0x%02x. Reverting to default (English - United States)", static_cast<unsigned>(LCID));
    }
} // END METHOD - init_layout

unsigned Keymap2::layout_id() const
{
    return this->keylayout_WORK->LCID;
}

array_view<Keylayout const *> Keymap2::keylayouts() noexcept
{
    return {::keylayouts};
}

Keylayout const & Keymap2::default_layout() noexcept
{
    return keylayout_x00000409;
}
