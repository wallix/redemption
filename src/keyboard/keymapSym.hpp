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

#pragma once

#include <cstdint>

struct KeymapSym
{
    enum {
           KBDFLAGS_EXTENDED = 0x0100
         , KBDFLAGS_DOWN     = 0x4000
         , KBDFLAGS_RELEASE  = 0x8000
    };

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

    enum :  uint16_t {
            VNC_LEFT_ALT  = 0xffe9,
            VNC_RIGHT_ALT = 0xffea,
            VNC_LEFT_CTRL = 0xffe3, 
    };

    enum {
           LEFT_SHIFT  = 0x36
         , RIGHT_SHIFT = 0x2A
         , LEFT_CTRL   = 0x1D
         , RIGHT_CTRL  = 0x9D
         , LEFT_ALT    = 0x38
         , RIGHT_ALT   = 0xB8
    };

    // keyboard info
    int keys_down[256];  // key states 0 up 1 down (0..127 plain keys, 128..255 extended keys)

    int key_flags;          // scroll_lock = 1, num_lock = 2, caps_lock = 4,
                            // shift = 8, ctrl = 16, Alt = 32,
                            // Windows = 64, AltGr = 128

    enum {
        SIZE_KEYBUF_SYM = 20
    };

    enum {
        KEVENT_KEY,
        KEVENT_TAB,
        KEVENT_BACKTAB,
        KEVENT_ENTER,
        KEVENT_ESC,
        KEVENT_DELETE,
        KEVENT_BACKSPACE,
        KEVENT_LEFT_ARROW,
        KEVENT_RIGHT_ARROW,
        KEVENT_UP_ARROW,
        KEVENT_DOWN_ARROW,
        KEVENT_HOME,
        KEVENT_END,
        KEVENT_PGUP,
        KEVENT_PGDOWN
    };

    uint32_t ibuf_sym; // first free position
    uint32_t nbuf_sym; // number of char in char buffer
    uint32_t buffer_sym[SIZE_KEYBUF_SYM]; // actual char buffer

    uint8_t dead_key;

    enum {
        DEADKEY_NONE,
        DEADKEY_CIRC,
        DEADKEY_UML,
        DEADKEY_GRAVE,
        DEADKEY_TILDE
    };

    enum {
         VNC_KBDFLAGS_RELEASE  = 0x00
       , VNC_KBDFLAGS_DOWN     = 0x01
    };

    uint32_t verbose;

    int last_sym;

    using KeyLayout_t = int[128];

    // keylayout working tables (X11 mode : begins in 8e position.)
    KeyLayout_t keylayout_WORK_noshift_sym;
    KeyLayout_t keylayout_WORK_shift_sym;
    KeyLayout_t keylayout_WORK_altgr_sym;
    KeyLayout_t keylayout_WORK_capslock_sym;
    KeyLayout_t keylayout_WORK_shiftcapslock_sym;

    explicit KeymapSym(int verbose = 0);

    void synchronize(uint16_t param1);

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

    void event(const uint16_t keyboardFlags, const uint16_t keyCode);

    // Push only sym
    void push_sym(uint32_t sym);

    uint32_t get_sym();

    uint32_t nb_sym_available() const;

    bool is_caps_locked() const;

    bool is_scroll_locked() const;

    bool is_num_locked() const;

    void toggle_num_lock(bool on);

    bool is_left_shift_pressed() const;

    bool is_right_shift_pressed() const;

    bool is_shift_pressed() const;

    bool is_left_ctrl_pressed() const;

    bool is_right_ctrl_pressed() const;

    bool is_ctrl_pressed() const;

    bool is_left_alt_pressed() const;

    // altgr
    bool is_right_alt_pressed() const;

    bool is_alt_pressed() const;


    void init_layout_sym(int keyb);

}; // STRUCT - KeymapSym
