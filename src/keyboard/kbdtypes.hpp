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
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include <cstdint>
#include "utils/sugar/cast.hpp"

//====================================
// SCANCODES PHYSICAL LAYOUT REFERENCE
//====================================
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +-----+----+-------+
// | 01 |  | 3B | 3C | 3D | 3E |  | 3F | 40 | 41 | 42 |  | 43 | 44 | 57 | 58 |  | 37x | 46 | 1D+45 |
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +-----+----+-------+
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

namespace kbdtypes
{
    enum class Scancode : uint8_t
    {
        Esc = 0x01,
        Tab = 0x0F,
        Enter = 0x1C,
        LCtrl = 0x1D,
        LShift = 0x2A,
        RShift = 0x36,
        LAlt = 0x38,
        LWin = 0x5B,
        Delete = 0x53,
        F12 = 0x58,

        A = 0x1E,
        B = 0x30,
        C = 0x2E,
        D = 0x20,
        E = 0x12,
        F = 0x21,
        G = 0x22,
        H = 0x23,
        I = 0x17,
        J = 0x24,
        K = 0x25,
        L = 0x26,
        M = 0x32,
        N = 0x31,
        O = 0x18,
        P = 0x19,
        Q = 0x10,
        R = 0x13,
        S = 0x1F,
        T = 0x14,
        U = 0x16,
        V = 0x2F,
        W = 0x11,
        X = 0x2D,
        Y = 0x15,
        Z = 0x2C,
    };


    // (SlotPath)
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
    // +---------------------------+-----------------------------------------------+
    // | 0x0200 KBDFLAGS_EXTENDED1 | Used to send keyboard events triggered by the |
    // |                           | PAUSE key.                                    |
    // |                           |                                               |
    // |                           | A PAUSE key press and release MUST be sent as |
    // |                           | the following sequence of keyboard events:    |
    // |                           |                                               |
    // |                           | * CTRL (0x1D) DOWN                            |
    // |                           |                                               |
    // |                           | * NUMLOCK (0x45) DOWN                         |
    // |                           |                                               |
    // |                           | * CTRL (0x1D) UP                              |
    // |                           |                                               |
    // |                           | * NUMLOCK (0x45) UP                           |
    // |                           |                                               |
    // |                           | The CTRL DOWN and CTRL UP events MUST both    |
    // |                           | include the KBDFLAGS_EXTENDED1 flag.          |
    // +--------------------------+------------------------------------------------+
    // | 0x4000 KBDFLAGS_DOWN     | Indicates that the key was down prior to this  |
    // |                          | event.                                         |
    // +--------------------------+------------------------------------------------+
    // | 0x8000 KBDFLAGS_RELEASE  | The absence of this flag indicates a key-down  |
    // |                          | event, while its presence indicates a          |
    // |                          | key-release event.                             |
    // +--------------------------+------------------------------------------------+
    //
    // Note: only Release with unicode
    enum class KbdFlags : uint16_t
    {
        NoFlags   = 0,
        Extended  = 0x0100,
        Extended1 = 0x0200, // for Pause / Attn
        // Down      = 0x4000, // unused
        Release   = 0x8000,
    };

    constexpr KbdFlags operator | (KbdFlags a, KbdFlags b) noexcept
    {
        return KbdFlags(underlying_cast(a) | underlying_cast(b));
    }

    constexpr KbdFlags operator & (KbdFlags a, KbdFlags b) noexcept
    {
        return KbdFlags(underlying_cast(a) & underlying_cast(b));
    }

    constexpr KbdFlags& operator |= (KbdFlags& a, KbdFlags b) noexcept
    {
        a = KbdFlags(underlying_cast(a) | underlying_cast(b));
        return a;
    }

    constexpr KbdFlags& operator &= (KbdFlags& a, KbdFlags b) noexcept
    {
        a = KbdFlags(underlying_cast(a) & underlying_cast(b));
        return a;
    }


    // (same with SlotPath)
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
    enum class KeyLocks : uint8_t
    {
        NoLocks     = 0,
        ScrollLock  = 0x01,
        NumLock     = 0x02,
        CapsLock    = 0x04,
        KanaLock    = 0x08,
    };

    constexpr KeyLocks operator | (KeyLocks a, KeyLocks b) noexcept
    {
        return KeyLocks(underlying_cast(a) | underlying_cast(b));
    }

    constexpr KeyLocks operator & (KeyLocks a, KeyLocks b) noexcept
    {
        return KeyLocks(underlying_cast(a) & underlying_cast(b));
    }

    constexpr KeyLocks& operator |= (KeyLocks& a, KeyLocks b) noexcept
    {
        a = KeyLocks(underlying_cast(a) | underlying_cast(b));
        return a;
    }

    constexpr KeyLocks& operator &= (KeyLocks& a, KeyLocks b) noexcept
    {
        a = KeyLocks(underlying_cast(a) & underlying_cast(b));
        return a;
    }


    // Scancode + Extended
    enum class KeyCode : uint16_t
    {
        Esc = 0x01,
        F1 = 0x3B,
        F2 = 0x3C,
        F3 = 0x3D,
        F4 = 0x3E,
        F5 = 0x3F,
        F6 = 0x40,
        F7 = 0x41,
        F8 = 0x42,
        F9 = 0x43,
        F10 = 0x44,
        F11 = 0x57,
        F12 = 0x58,
        F13 = 0x64,
        F14 = 0x65,
        F15 = 0x66,
        F16 = 0x67,
        F17 = 0x68,
        F18 = 0x69,
        F19 = 0x6A,
        F20 = 0x6B,
        F21 = 0x6C,
        F22 = 0x6D,
        F23 = 0x6E,
        F24 = 0x76,

        PrintScreen = 0x137,
        /// /!\\ Pause is 0x1D (LCtrl) | 0x200 then 0x45 (NumLock)
        PauseFirstPart = 0x1D | 0x200,

        LCtrl = 0x1D,
        RCtrl = 0x1D | 0x100,
        LShift = 0x2A,
        RShift = 0x36,
        LAlt = 0x38,
        RAlt = 0x38 | 0x100,
        LWin = 0x5B | 0x100,
        RWin = 0x5C | 0x100,
        ContextMenu = 0x5D | 0x100,

        CapsLock = 0x3A,
        ScrollLock = 0x46,
        /// /!\\ Pause is 0x1D | 0x200, 0x45
        NumLock = 0x45,

        UpArrow = 0x48 | 0x100,
        LeftArrow = 0x4B | 0x100,
        RightArrow = 0x4D | 0x100,
        DownArrow = 0x50 | 0x100,
        Home = 0x47 | 0x100,
        End = 0x4F | 0x100,
        PgUp = 0x49 | 0x100,
        PgDown = 0x51 | 0x100,
        Insert = 0x52 | 0x100,
        Delete = 0x53 | 0x100,
        Enter = 0x1C,
        Tab = 0x0F,
        Space = 0x39,
        Backspace = 0x0E,

        Paste = 0x10A,
        Copy = 0x118,
        Cut = 0x117,

        AudioVolumeDown = 0x12E,
        AudioVolumeMute = 0x120,
        AudioVolumeUp = 0x130,
        MediaPlayPause = 0x122,
        MediaStop = 0x124,
        MediaTrackNext = 0x119,
        MediaTrackPrevious = 0x110,

        Undo = 0x108,
        // Redo = ???,

        Numpad7 = 0x47,
        Numpad8 = 0x48,
        Numpad9 = 0x49,
        Numpad4 = 0x4b,
        Numpad5 = 0x4c,
        Numpad6 = 0x4d,
        Numpad1 = 0x4f,
        Numpad2 = 0x50,
        Numpad3 = 0x51,
        Numpad0 = 0x52,
        NumpadDecimal = 0x53,
        NumpadDivide = 0x35 | 0x100,
        NumpadMultiply = 0x37,
        NumpadSubtract = 0x4A,
        NumpadAdd = 0x4E,
        NumpadEnter = 0x1C | 0x100,

        Key_A = 0x1E,
        Key_B = 0x30,
        Key_C = 0x2E,
        Key_D = 0x20,
        Key_E = 0x12,
        Key_F = 0x21,
        Key_G = 0x22,
        Key_H = 0x23,
        Key_I = 0x17,
        Key_J = 0x24,
        Key_K = 0x25,
        Key_L = 0x26,
        Key_M = 0x32,
        Key_N = 0x31,
        Key_O = 0x18,
        Key_P = 0x19,
        Key_Q = 0x10,
        Key_R = 0x13,
        Key_S = 0x1F,
        Key_T = 0x14,
        Key_U = 0x16,
        Key_V = 0x2F,
        Key_W = 0x11,
        Key_X = 0x2D,
        Key_Y = 0x15,
        Key_Z = 0x2C,

        IntlBackslash = 0x56,
        BracketLeft = 0x1A,
        BracketRight = 0x1B,
        Semicolon = 0x27,
        Quote = 0x28,
        Backslash = 0x2B,
        Comma = 0x33,
        Period = 0x34,
        Slash = 0x35,

        Backquote = 0x29,
        Digit1 = 0x02,
        Digit2 = 0x03,
        Digit3 = 0x04,
        Digit4 = 0x05,
        Digit5 = 0x06,
        Digit6 = 0x07,
        Digit7 = 0x08,
        Digit8 = 0x09,
        Digit9 = 0x0A,
        Digit0 = 0x0B,
        Minus = 0x0C,
        Equal = 0x0D,
    };

    // The scancode and its extended nature are merged in a new variable (whose most significant bit indicates the extended nature)
    constexpr KeyCode to_keycode(KbdFlags flags, Scancode scancode) noexcept
    {
        return KeyCode(underlying_cast(scancode) | underlying_cast(flags & (KbdFlags::Extended | KbdFlags::Extended1)));
    }

    constexpr Scancode pressed_scancode(KbdFlags flags, Scancode scancode) noexcept
    {
        return (underlying_cast(flags) & underlying_cast(KbdFlags::Release))
            ? Scancode()
            : scancode;
    }

    constexpr bool keycode_is_compressable_to_byte(KeyCode keycode) noexcept
    {
        // keycode with scancode <= 0x7f and with or without extended flag
        return (underlying_cast(keycode) & uint16_t(~0x17fu)) == 0;
    }

    constexpr std::size_t keycode_to_byte_index(KeyCode keycode) noexcept
    {
        return (std::size_t(keycode) & 0x7f)
             | ((std::size_t(keycode) & 0x100) >> 1);
    }
} // namespace kbdtypes
