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

enum KS_Symbols {

    XK_EuroSign = 0x20AC,

    KS_BackSpace                        =     0xff08,
    KS_Tab                              =     0xff09,
    KS_Linefeed                         =     0xff0a,
    KS_Clear                            =     0xff0b,
    KS_Return                           =     0xff0d,
    KS_Pause                            =     0xff13,
    KS_Scroll_Lock                      =     0xff14,
    KS_Sys_Req                          =     0xff15,
    KS_Escape                           =     0xff1b,
    KS_Multi_key                        =     0xff20,
    KS_Kanji                            =     0xff21,
    KS_Home                             =     0xff50,
    KS_Left                             =     0xff51,
    KS_Up                               =     0xff52,
    KS_Right                            =     0xff53,
    KS_Down                             =     0xff54,
    KS_Prior                            =     0xff55,
    KS_Next                             =     0xff56,
    KS_End                              =     0xff57,
    KS_Begin                            =     0xff58,
    KS_Win_L                            =     0xff5b,
    KS_Win_R                            =     0xff5c,

    KS_App                              =     0xff5d,
    KS_Select                           =     0xff60,
    KS_Print                            =     0xff61,
    KS_Execute                          =     0xff62,
    KS_Insert                           =     0xff63,
    KS_Undo                             =     0xff65,
    KS_Redo                             =     0xff66,
    KS_Menu                             =     0xff67,
    KS_Find                             =     0xff68,
    KS_Cancel                           =     0xff69,
    KS_Help                             =     0xff6a,
    KS_Break                            =     0xff6b,
    KS_Hebrew_switch                    =     0xff7e,
    KS_Num_Lock                         =     0xff7f,
    KS_KP_Space                         =     0xff80,
    KS_KP_Tab                           =     0xff89,
    KS_KP_Enter                         =     0xff8d,
    KS_KP_F1                            =     0xff91,
    KS_KP_F2                            =     0xff92,
    KS_KP_F3                            =     0xff93,
    KS_KP_F4                            =     0xff94,
    KS_KP_Multiply                      =     0xffaa,
    KS_KP_Add                           =     0xffab,
    KS_KP_Separator                     =     0xffac,
    KS_KP_Subtract                      =     0xffad,
    KS_KP_Decimal                       =     0xffae,
    KS_KP_Divide                        =     0xffaf,
    KS_KP_0                             =     0xffb0,
    KS_KP_1                             =     0xffb1,
    KS_KP_2                             =     0xffb2,
    KS_KP_3                             =     0xffb3,
    KS_KP_4                             =     0xffb4,
    KS_KP_5                             =     0xffb5,
    KS_KP_6                             =     0xffb6,
    KS_KP_7                             =     0xffb7,
    KS_KP_8                             =     0xffb8,
    KS_KP_9                             =     0xffb9,
    KS_KP_Equal                         =     0xffbd,
    KS_F1                               =     0xffbe,
    KS_F2                               =     0xffbf,
    KS_F3                               =     0xffc0,
    KS_F4                               =     0xffc1,
    KS_F5                               =     0xffc2,
    KS_F6                               =     0xffc3,
    KS_F7                               =     0xffc4,
    KS_F8                               =     0xffc5,
    KS_F9                               =     0xffc6,
    KS_F10                              =     0xffc7,
    KS_L1                               =     0xffc8,
    KS_L2                               =     0xffc9,
    KS_L3                               =     0xffca,
    KS_L4                               =     0xffcb,
    KS_L5                               =     0xffcc,
    KS_L6                               =     0xffcd,
    KS_L7                               =     0xffce,
    KS_L8                               =     0xffcf,
    KS_L9                               =     0xffd0,
    KS_L10                              =     0xffd1,
    KS_R1                               =     0xffd2,
    KS_R2                               =     0xffd3,
    KS_R3                               =     0xffd4,
    KS_R4                               =     0xffd5,
    KS_R5                               =     0xffd6,
    KS_R6                               =     0xffd7,
    KS_R7                               =     0xffd8,
    KS_R8                               =     0xffd9,
    KS_R9                               =     0xffda,
    KS_R10                              =     0xffdb,
    KS_R11                              =     0xffdc,
    KS_R12                              =     0xffdd,
    KS_F33                              =     0xffde,
    KS_R14                              =     0xffdf,
    KS_R15                              =     0xffe0,
    KS_Shift_L                          =     0xffe1,
    KS_Shift_R                          =     0xffe2,
    KS_Control_L                        =     0xffe3,
    KS_Control_R                        =     0xffe4,
    KS_Caps_Lock                        =     0xffe5,
    KS_Shift_Lock                       =     0xffe6,
    KS_Meta_L                           =     0xffe7,
    KS_Meta_R                           =     0xffe8,
    KS_Alt_L = 0xffe9,
    KS_Alt_R = 0xffea,
    KS_Super_L                          =     0xffeb,
    KS_Super_R                          =     0xffec,
    KS_Hyper_L                          =     0xffed,
    KS_Hyper_R                          =     0xffee,
    KS_Delete = 0xffff

};


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

    bool remove_server_alt_state_for_char;
    bool apple_server;

    int keylayout;

    bool remove_state_mode;
    bool add_state_mode;

    explicit KeymapSym(int verbose, bool remove_server_alt_state_for_char, bool apple_server);

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

    bool is_remove_state_mode();

    bool is_add_state_mode();

}; // STRUCT - KeymapSym
