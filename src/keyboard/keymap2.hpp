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

#pragma once

#include "keyboard/keylayout.hpp"
#include "utils/sugar/array_view.hpp"

#include <cstdint>


struct Keymap2
{
    enum {
           KBDFLAGS_EXTENDED = 0x0100
         , KBDFLAGS_DOWN     = 0x4000
         , KBDFLAGS_RELEASE  = 0x8000
         };

    enum KeyFlags {
           NoFlag      = 0
         , SCROLLLOCK  = 0x01
         , NUMLOCK     = 0x02
         , CAPSLOCK    = 0x04
         , FLG_SHIFT   = 0x08
         , FLG_CTRL    = 0x10
         , FLG_ALT     = 0x20
         , FLG_WINDOWS = 0x40
         , FLG_ALTGR   = 0x80
    };

    enum {
           LEFT_SHIFT  = 0x2A
         , RIGHT_SHIFT = 0x36
         , LEFT_CTRL   = 0x1D
         , RIGHT_CTRL  = 0x9D
         , LEFT_ALT    = 0x38
         , RIGHT_ALT   = 0xB8
         , F11         = 0x57
         , F12         = 0x58
    };

private:
    /* TODO we should be able to unify unicode support and events. Idea would be to attribute codes 0xFFFFxxxx on 32 bits for events.
     * These are outside unicode range. It would enable to use only one keycode stack instead of two and it's more similar
     * to the way X11 manage inputs (hence easier to unify with keymapSym) */

    // keyboard info
    int keys_down[256];  // key states 0 up 1 down (0..127 plain keys, 128..255 extended keys)

public:
    KeyFlags key_flags;

    enum {
        SIZE_KEYBUF = 20
    };

    enum {
        SIZE_KEYBUF_KEVENT = 20
    };

    enum {
          KEVENT_KEY = 0x01
        , KEVENT_TAB = 0x02
        , KEVENT_BACKTAB = 0x3
        , KEVENT_ENTER = 0x04
        , KEVENT_ESC = 0x05
        , KEVENT_DELETE = 0x06
        , KEVENT_BACKSPACE = 0x07
        , KEVENT_LEFT_ARROW = 0x08
        , KEVENT_RIGHT_ARROW = 0x09
        , KEVENT_UP_ARROW = 0x0A
        , KEVENT_DOWN_ARROW = 0x0B
        , KEVENT_HOME = 0x0C
        , KEVENT_END = 0x0D
        , KEVENT_PGUP = 0x0E
        , KEVENT_PGDOWN = 0x0F
        , KEVENT_CUT = 0x10
        , KEVENT_COPY = 0x11
        , KEVENT_PASTE = 0x12
        , KEVENT_INSERT = 0x13
    };

private:
    uint32_t ibuf; // first free position in char buffer
    uint32_t nbuf; // number of char in char buffer
    uint32_t buffer[SIZE_KEYBUF]; // actual char buffer

    uint32_t ibuf_kevent; // first free position
    uint32_t nbuf_kevent; // number of char in char buffer
    uint32_t buffer_kevent[SIZE_KEYBUF_KEVENT]; // actual char buffer

    //uint32_t last_char_key;

    //int last_chr_unicode;

    uint8_t deadkey;

    enum {
          DEADKEY_NONE  = 0
        , DEADKEY_FOUND = 1
    };

    const Keylayout * keylayout_WORK;  // layout suitable for the client keyboard code

    Keylayout::dkey_t deadkey_pending_def;  // object containing the current deadkey definition if any

    uint32_t verbose;

public:
    bool is_application_switching_shortcut_pressed = false;

public:
    explicit Keymap2(uint32_t verbose = 0);

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
    struct DecodedKeys
    {
        uint32_t uchars[2]{};
        unsigned count = 0;

        void set_uchar(uint32_t uchar);
    };

    DecodedKeys event(const uint16_t keyboardFlags, const uint16_t keyCode, bool & tsk_switch_shortcuts);

    void push(uint32_t uchar);
    void push_char(uint32_t uchar);
    void push_kevent(uint32_t uevent);

    uint32_t get_char();
    uint32_t get_kevent();

    // head of keyboard buffer (or keyboard buffer of size 1)
    uint32_t top_char() const;

    uint32_t nb_char_available() const;

    // head of keyboard buffer (or keyboard buffer of size 1)
    uint32_t top_kevent() const;

    uint32_t nb_kevent_available() const;

    bool is_alt_pressed() const;
    bool is_caps_locked() const;
    bool is_ctrl_alt_pressed() const;
    bool is_ctrl_pressed() const;
    bool is_left_alt_pressed() const;
    bool is_left_ctrl_pressed() const;
    bool is_left_shift_pressed() const;
    bool is_num_locked() const;
    bool is_right_alt_pressed() const; // altGr
    bool is_right_ctrl_pressed() const;
    bool is_right_shift_pressed() const;
    bool is_scroll_locked() const;
    bool is_shift_pressed() const;

    void toggle_caps_lock(bool on);
    void toggle_num_lock(bool on);
    void toggle_scroll_lock(bool on);

    void init_layout(int LCID);

    static array_view<Keylayout const *> keylayouts() noexcept;
    static Keylayout const & default_layout() noexcept;

    unsigned layout_id() const;
}; // END STRUCT - Keymap2
