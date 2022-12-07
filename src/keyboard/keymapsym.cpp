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
#include "keyboard/keylayout.hpp"
#include "keyboard/keymap.hpp"
#include "utils/log.hpp"


// see /usr/include/X11/keysymdef.h
namespace
{

inline uint32_t unicode_to_ksym(uint32_t uc) noexcept
{
    // is latin1 and visible
    if (0x20 <= uc && uc <= 0xFF) {
        return uc;
    }
    // legacy (unicode) keysym
    // if (0x0100 <= uc && uc <= 0x20ff) {
    //     return uc;
    // }
    // unicode: 0x01000100 to 0x0110ffff
    return uc | 0x01000000;
}

constexpr uint32_t vnc_key_LCtrl = 0xffe3;
constexpr uint32_t vnc_key_RCtrl = 0xffe4;
constexpr uint32_t vnc_key_LShift = 0xffe1;
constexpr uint32_t vnc_key_RShift = 0xffe2;
constexpr uint32_t vnc_key_LAlt = 0xffe9;
constexpr uint32_t vnc_key_RAlt = 0xffea;
constexpr uint32_t vnc_key_LWin = 0xffeb;
constexpr uint32_t vnc_key_RWin = 0xffec;

inline uint32_t keycode_to_sym(kbdtypes::KeyCode keycode, kbdtypes::KeyModFlags mods) noexcept
{
    using kbdtypes::KeyCode;
    using kbdtypes::KeyMod;

    switch (keycode)
    {
        case KeyCode::Esc: return 0xff1b;
        case KeyCode::F1: return 0xff91;
        case KeyCode::F2: return 0xff92;
        case KeyCode::F3: return 0xff93;
        case KeyCode::F4: return 0xff94;
        case KeyCode::F5: return 0xffc2;
        case KeyCode::F6: return 0xffc3;
        case KeyCode::F7: return 0xffc4;
        case KeyCode::F8: return 0xffc5;
        case KeyCode::F9: return 0xffc6;
        case KeyCode::F10: return 0xffc7;
        case KeyCode::F11: return 0xffc8;
        case KeyCode::F12: return 0xffc9;
        case KeyCode::F13: return 0xffca;
        case KeyCode::F14: return 0xffcb;
        case KeyCode::F15: return 0xffcc;
        case KeyCode::F16: return 0xffcd;
        case KeyCode::F17: return 0xffce;
        case KeyCode::F18: return 0xffcf;
        case KeyCode::F19: return 0xffd0;
        case KeyCode::F20: return 0xffd1;
        case KeyCode::F21: return 0xffd2;
        case KeyCode::F22: return 0xffd3;
        case KeyCode::F23: return 0xffd4;
        case KeyCode::F24: return 0xffd5;

        case KeyCode::PrintScreen: return 0xfd1d;
        case KeyCode::PauseFirstPart: return 0xff13;

        case KeyCode::LCtrl: return vnc_key_LCtrl;
        case KeyCode::RCtrl: return vnc_key_RCtrl;
        case KeyCode::LShift: return vnc_key_LShift;
        case KeyCode::RShift: return vnc_key_RShift;
        case KeyCode::LAlt: return vnc_key_LAlt;
        case KeyCode::RAlt: return vnc_key_RAlt;
        case KeyCode::LWin: return vnc_key_LWin;
        case KeyCode::RWin: return vnc_key_RWin;
        case KeyCode::ContextMenu: return 0xff67;

        case KeyCode::CapsLock: return 0xffe5;
        case KeyCode::ScrollLock: return 0xff14;
        case KeyCode::NumLock: return 0xff7f;

        case KeyCode::UpArrow: return 0xff52;
        case KeyCode::LeftArrow: return 0xff51;
        case KeyCode::RightArrow: return 0xff53;
        case KeyCode::DownArrow: return 0xff54;
        case KeyCode::Home: return 0xff50;
        case KeyCode::End: return 0xff57;
        case KeyCode::PgUp: return 0xff55;
        case KeyCode::PgDown: return 0xff56;
        case KeyCode::Insert: return 0xff63;
        case KeyCode::Delete: return 0xffff;
        case KeyCode::Enter: return 0xff0d;
        case KeyCode::Tab: return 0xff09;
        case KeyCode::Backspace: return 0xff08;

        case KeyCode::Paste: return 0x1008ff6d;
        case KeyCode::Copy: return 0x1008ff57;
        case KeyCode::Cut: return 0x1008ff58;

        case KeyCode::AudioVolumeDown: return 0x1008ff11;
        case KeyCode::AudioVolumeMute: return 0x1008ff12;
        case KeyCode::AudioVolumeUp: return 0x1008ff13;
        case KeyCode::MediaPlayPause: return 0x1008ff14;
        case KeyCode::MediaStop: return 0x1008ff15;
        case KeyCode::MediaTrackNext: return 0x1008FF17;
        case KeyCode::MediaTrackPrevious: return 0x1008FF16;

        case KeyCode::Undo: return 0xff65;
        // case KeyCode::Redo: return 0xff66;

        case KeyCode::Numpad7: return mods.test(KeyMod::NumLock) ? 0xffb7 : 0xff95;
        case KeyCode::Numpad8: return mods.test(KeyMod::NumLock) ? 0xffb8 : 0xff97;
        case KeyCode::Numpad9: return mods.test(KeyMod::NumLock) ? 0xffb9 : 0xff9a;
        case KeyCode::Numpad4: return mods.test(KeyMod::NumLock) ? 0xffb4 : 0xff96;
        case KeyCode::Numpad5: return mods.test(KeyMod::NumLock) ? 0xffb5 : 0xff9d;
        case KeyCode::Numpad6: return mods.test(KeyMod::NumLock) ? 0xffb6 : 0xff98;
        case KeyCode::Numpad1: return mods.test(KeyMod::NumLock) ? 0xffb1 : 0xff9c;
        case KeyCode::Numpad2: return mods.test(KeyMod::NumLock) ? 0xffb2 : 0xff99;
        case KeyCode::Numpad3: return mods.test(KeyMod::NumLock) ? 0xffb3 : 0xff9b;
        case KeyCode::Numpad0: return mods.test(KeyMod::NumLock) ? 0xffb0 : 0xff9e;
        case KeyCode::NumpadDecimal: return mods.test(KeyMod::NumLock) ? 0xffae : 0xff9f;
        case KeyCode::NumpadDivide: return 0xffaf;
        case KeyCode::NumpadMultiply: return 0xffaa;
        case KeyCode::NumpadSubtract: return 0xffad;
        case KeyCode::NumpadAdd: return 0xffab;
        case KeyCode::NumpadEnter: return 0xff8d;

        // locale dependent
        case KeyCode::Key_A:
        case KeyCode::Key_B:
        case KeyCode::Key_C:
        case KeyCode::Key_D:
        case KeyCode::Key_E:
        case KeyCode::Key_F:
        case KeyCode::Key_G:
        case KeyCode::Key_H:
        case KeyCode::Key_I:
        case KeyCode::Key_J:
        case KeyCode::Key_K:
        case KeyCode::Key_L:
        case KeyCode::Key_M:
        case KeyCode::Key_N:
        case KeyCode::Key_O:
        case KeyCode::Key_P:
        case KeyCode::Key_Q:
        case KeyCode::Key_R:
        case KeyCode::Key_S:
        case KeyCode::Key_T:
        case KeyCode::Key_U:
        case KeyCode::Key_V:
        case KeyCode::Key_W:
        case KeyCode::Key_X:
        case KeyCode::Key_Y:
        case KeyCode::Key_Z:
        case KeyCode::IntlBackslash:
        case KeyCode::BracketLeft:
        case KeyCode::BracketRight:
        case KeyCode::Semicolon:
        case KeyCode::Quote:
        case KeyCode::Backslash:
        case KeyCode::Comma:
        case KeyCode::Period:
        case KeyCode::Slash:
        case KeyCode::Backquote:
        case KeyCode::Digit1:
        case KeyCode::Digit2:
        case KeyCode::Digit3:
        case KeyCode::Digit4:
        case KeyCode::Digit5:
        case KeyCode::Digit6:
        case KeyCode::Digit7:
        case KeyCode::Digit8:
        case KeyCode::Digit9:
        case KeyCode::Digit0:
        case KeyCode::Minus:
        case KeyCode::Equal:
        case KeyCode::Space:
            break;
    }
    return 0;
}

const uint16_t macos_noshift[] = {
    0x0000, 0xff1b,    '1',    '2',    '3',    '4',    '5',    '6', // 0x00 - 0x07
       '7',    '8',    '9',    '0',    '-',    '=', 0xff08, 0xff09, // 0x08 - 0x0f
       'q',    'w',    'e',    'r',    't',    'y',    'u',    'i', // 0x10 - 0x17
       'o',    'p',    '[',    ']', 0xff0d, 0xffe3,    'a',    's', // 0x18 - 0x1f
       'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';', // 0x20 - 0x27
      '\'', 0x00a4, 0xffe1,   '\\',    'z',    'x',    'c',    'v', // 0x28 - 0x2f
       'b',    'n',    'm',    ',',    '.',    '/', 0xffe2, 0xffaa, // 0x30 - 0x37
    0xffe7,    ' ', 0xffe5, 0xffbe, 0xffbf, 0xffc0, 0xffc1, 0xffc2, // 0x38 - 0x3f
    0xffc3, 0xffc4, 0xffc5, 0xffc6, 0xffc7, 0xff7f, 0xff14, 0xff95, // 0x40 - 0x47
    0xff97, 0xff9a, 0xffad, 0xff96, 0xff9d, 0xff98, 0xffab, 0xff9c, // 0x48 - 0x4f
    0xff99, 0xff9b, 0xff9e, 0xff9f, 0x0000, 0xff7e,    '<', 0xffc8, // 0x50 - 0x57
    0xffc9, 0xff50, 0xff52, 0x0000, 0x0000, 0x0000, 0xff53, 0xff57, // 0x58 - 0x5f
    0xff54, 0xff56, 0xff63, 0xffff, 0xff8d, 0xffe4, 0xff13, 0xff61, // 0x60 - 0x67
    0xffaf, 0xffe8, 0x0000, 0xffeb, 0xffec, 0xff67, 0x0000, 0x0000, // 0x68 - 0x6f
    0x0000, 0x0000, 0x0000, 0x0000, 0xfe03, 0x0000, 0xffbd, 0x0000, // 0x70 - 0x77
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x78 - 0x7f
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x80 - 0x87
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x88 - 0x8f
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x90 - 0x97
    0x0000, 0x0000, 0x0000, 0x0000, 0xff8d, 0xffe4, 0x0000, 0x0000, // 0x98 - 0x9f
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xa0 - 0xa7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xa8 - 0xaf
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffaf, 0x0000, 0xff61, // 0xb0 - 0xb7
    0xffe8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xb8 - 0xbf
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xff50, // 0xc0 - 0xc7
    0xff52, 0xff55, 0x0000, 0xff51, 0x0000, 0xff53, 0x0000, 0xff57, // 0xc8 - 0xcf
    0xff54, 0xff56, 0xff63, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, // 0xd0 - 0xd7
    0x0000, 0x0000, 0x0000, 0xffeb, 0xffec, 0xff67, 0x0000, 0x0000, // 0xd8 - 0xdf
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xe0 - 0xe7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xe8 - 0xef
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xf0 - 0xf7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xf8 - 0xff
};
static_assert(sizeof(macos_noshift) == 256 * 2);

const uint16_t macos_shift[] = {
    0x0000, 0xff1b,    '!',    '@',    '#',    '$',    '%',    '^', // 0x00 - 0x07
       '&',    '8',    '(',    ')',    '_',    '+', 0xff08, 0xfe20, // 0x08 - 0x0f
       'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I', // 0x10 - 0x17
       'O',    'P',    '{',    '}', 0xff0d, 0xffe3,    'A',    'S', // 0x18 - 0x1f
       'D',    'F',    'G',    'H',    'J',    'K',    'L',    ':', // 0x20 - 0x27
       '"',    '~', 0xffe1,    '|',    'Z',    'X',    'C',    'V', // 0x28 - 0x2f
       'B',    'N',    'M',    '<',    '>',    '?', 0xffe2, 0xffaa, // 0x30 - 0x37
    0xffe7,    ' ', 0xffe5, 0xffbe, 0xffbf, 0xffc0, 0xffc1, 0xffc2, // 0x38 - 0x3f
    0xffc3, 0xffc4, 0xffc5, 0xffc6, 0xffc7, 0xfef9, 0xff14, 0xffb7, // 0x40 - 0x47
    0xffb8, 0xffb9, 0xffad, 0xffb4, 0xffb5, 0xffb6, 0xffab, 0xffb1, // 0x48 - 0x4f
    0xffb2, 0xffb3, 0xffb0, 0xffae, 0x0000, 0xff7e,    '>', 0xffc8, // 0x50 - 0x57
    0xffc9, 0xff50, 0xff52, 0x0000, 0x0000, 0x0000, 0xff53, 0xff57, // 0x58 - 0x5f
    0xff54, 0xff56, 0xff63, 0xffff, 0xff8d, 0xffe4, 0xff13, 0xff61, // 0x60 - 0x67
    0xffaf, 0xffe8, 0x0000, 0xffeb, 0xffec, 0xff67, 0x0000, 0x0000, // 0x68 - 0x6f
    0x0000, 0x0000, 0x0000, 0x0000, 0xfe03, 0xffe9, 0xffbd, 0xffeb, // 0x70 - 0x77
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x78 - 0x7f
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x80 - 0x87
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x88 - 0x8f
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x90 - 0x97
    0x0000, 0x0000, 0x0000, 0x0000, 0xff8d, 0xffe4, 0x0000, 0x0000, // 0x98 - 0x9f
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xa0 - 0xa7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xa8 - 0xaf
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffaf, 0x0000, 0xff61, // 0xb0 - 0xb7
    0xffe8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xb8 - 0xbf
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xff50, // 0xc0 - 0xc7
    0xff52, 0xff55, 0x0000, 0xff51, 0x0000, 0xff53, 0x0000, 0xff57, // 0xc8 - 0xcf
    0xff54, 0xff56, 0xff63, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, // 0xd0 - 0xd7
    0x0000, 0x0000, 0x0000, 0xffeb, 0xffec, 0xff67, 0x0000, 0x0000, // 0xd8 - 0xdf
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xe0 - 0xe7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xe8 - 0xef
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xf0 - 0xf7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xf8 - 0xff
};
static_assert(sizeof(macos_shift) == 256 * 2);

const uint16_t macos_altgr[] = {
    0x0000, 0xff1b,    '1',    '2',    '3',    '4',    '5',    '6', // 0x00 - 0x07
       '7',    '8',    '9',    '0',    '-',    '=', 0xff08, 0xff09, // 0x08 - 0x0f
       'q',    'w',    'e',    'r',    't',    'y',    'u',    'i', // 0x10 - 0x17
       'o',    'p',    '[',    ']', 0xff0d, 0xffe3,    'a',    's', // 0x18 - 0x1f
       'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';', // 0x20 - 0x27
      '\'',    '`', 0xffe1,   '\\',    'z',    'x',    'c',    'v', // 0x28 - 0x2f
       'b',    'n',    'm',    ',',    '.',    '/', 0xffe2, 0xffaa, // 0x30 - 0x37
    0xffe7,    ' ', 0xffe5, 0xffbe, 0xffbf, 0xffc0, 0xffc1, 0xffc2, // 0x38 - 0x3f
    0xffc3, 0xffc4, 0xffc5, 0xffc6, 0xffc7, 0xff7f, 0xff14, 0xff95, // 0x40 - 0x47
    0xff97, 0xff9a, 0xffad, 0xff96, 0xff9d, 0xff98, 0xffab, 0xff9c, // 0x48 - 0x4f
    0xff99, 0xff9b, 0xff9e, 0xff9f, 0x0000, 0xff7e,    '<', 0xffc8, // 0x50 - 0x57
    0xffc9, 0xff50, 0xff52, 0x0000, 0x0000, 0x0000, 0xff53, 0xff57, // 0x58 - 0x5f
    0xff54, 0xff56, 0xff63, 0xffff, 0xff8d, 0xffe4, 0xff13, 0x0000, // 0x60 - 0x67
    0xffaf, 0xffe8, 0x0000, 0xffeb, 0xffec, 0xff67, 0x0000, 0x0000, // 0x68 - 0x6f
    0x0000, 0x0000, 0x0000, 0x0000, 0xfe03, 0x0000, 0xffbd, 0x0000, // 0x70 - 0x77
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x78 - 0x7f
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x80 - 0x87
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x88 - 0x8f
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x90 - 0x97
    0x0000, 0x0000, 0x0000, 0x0000, 0xff8d, 0xffe4, 0x0000, 0x0000, // 0x98 - 0x9f
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xa0 - 0xa7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xa8 - 0xaf
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffaf, 0x0000, 0x0000, // 0xb0 - 0xb7
    0xffe8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xb8 - 0xbf
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xff50, // 0xc0 - 0xc7
    0xff52, 0xff55, 0x0000, 0xff51, 0x0000, 0xff53, 0x0000, 0xff57, // 0xc8 - 0xcf
    0xff54, 0xff56, 0xff63, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, // 0xd0 - 0xd7
    0x0000, 0x0000, 0x0000, 0xffeb, 0xffec, 0xff67, 0x0000, 0x0000, // 0xd8 - 0xdf
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xe0 - 0xe7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xe8 - 0xef
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xf0 - 0xf7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xf8 - 0xff
};
static_assert(sizeof(macos_altgr) == 256 * 2);

const uint16_t macos_capslock[] = {
    0x0000, 0xff1b,    '1',    '2',    '3',    '4',    '5',    '6', // 0x00 - 0x07
       '7',    '8',    '9',    '0',    '-',    '=', 0xff08, 0xff09, // 0x08 - 0x0f
       'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I', // 0x10 - 0x17
       'O',    'P',    '[',    ']', 0xff0d, 0xffe3,    'A',    'S', // 0x18 - 0x1f
       'D',    'F',    'G',    'H',    'J',    'K',    'L',    ';', // 0x20 - 0x27
      '\'',    '`', 0xffe1,   '\\',    'Z',    'X',    'C',    'V', // 0x28 - 0x2f
       'B',    'N',    'M',    ',',    '.',    '/', 0xffe2, 0xffaa, // 0x30 - 0x37
    0xffe7,    ' ', 0xffe5, 0xffbe, 0xffbf, 0xffc0, 0xffc1, 0xffc2, // 0x38 - 0x3f
    0xffc3, 0xffc4, 0xffc5, 0xffc6, 0xffc7, 0xff7f, 0xff14, 0xff95, // 0x40 - 0x47
    0xff97, 0xff9a, 0xffad, 0xff96, 0xff9d, 0xff98, 0xffab, 0xff9c, // 0x48 - 0x4f
    0xff99, 0xff9b, 0xff9e, 0xff9f, 0x0000, 0xff7e,    '<', 0xffc8, // 0x50 - 0x57
    0xffc9, 0xff50, 0xff52, 0x0000, 0x0000, 0x0000, 0xff53, 0xff57, // 0x58 - 0x5f
    0xff54, 0xff56, 0xff63, 0xffff, 0xff8d, 0xffe4, 0xff13, 0xff61, // 0x60 - 0x67
    0xffaf, 0xffe8, 0x0000, 0xffeb, 0xffec, 0xff67, 0x0000, 0x0000, // 0x68 - 0x6f
    0x0000, 0x0000, 0x0000, 0x0000, 0xfe03, 0x0000, 0xffbd, 0x0000, // 0x70 - 0x77
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x78 - 0x7f
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x80 - 0x87
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x88 - 0x8f
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x90 - 0x97
    0x0000, 0x0000, 0x0000, 0x0000, 0xff8d, 0xffe4, 0x0000, 0x0000, // 0x98 - 0x9f
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xa0 - 0xa7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xa8 - 0xaf
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffaf, 0x0000, 0xff61, // 0xb0 - 0xb7
    0xffe8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xb8 - 0xbf
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xff50, // 0xc0 - 0xc7
    0xff52, 0xff55, 0x0000, 0xff51, 0x0000, 0xff53, 0x0000, 0xff57, // 0xc8 - 0xcf
    0xff54, 0xff56, 0xff63, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, // 0xd0 - 0xd7
    0x0000, 0x0000, 0x0000, 0xffeb, 0xffec, 0xff67, 0x0000, 0x0000, // 0xd8 - 0xdf
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xe0 - 0xe7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xe8 - 0xef
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xf0 - 0xf7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xf8 - 0xff
};
static_assert(sizeof(macos_capslock) == 256 * 2);

const uint16_t macos_shift_capslock[] = {
    0x0000, 0xff1b,    '!',    '@',    '#',    '$',    '%',    '^', // 0x00 - 0x07
       '&',    '*',    '(',    ')',    '_',    '+', 0xff08, 0xfe20, // 0x08 - 0x0f
       'q',    'w',    'e',    'r',    't',    'y',    'u',    'i', // 0x10 - 0x17
       'o',    'p',    '{',    '}', 0xff0d, 0xffe3,    'a',    's', // 0x18 - 0x1f
       'd',    'f',    'g',    'h',    'j',    'k',    'l',    ':', // 0x20 - 0x27
       '"',    '~', 0xffe1,    '|',    'z',    'x',    'c',    'v', // 0x28 - 0x2f
       'b',    'n',    'm',    '<',    '>',    '?', 0xffe2, 0xffaa, // 0x30 - 0x37
    0xffe7,    ' ', 0xffe5, 0xffbe, 0xffbf, 0xffc0, 0xffc1, 0xffc2, // 0x38 - 0x3f
    0xffc3, 0xffc4, 0xffc5, 0xffc6, 0xffc7, 0xfef9, 0xff14, 0xffb7, // 0x40 - 0x47
    0xffb8, 0xffb9, 0xffad, 0xffb4, 0xffb5, 0xffb6, 0xffab, 0xffb1, // 0x48 - 0x4f
    0xffb2, 0xffb3, 0xffb0, 0xffae, 0x0000, 0xff7e,    '<', 0xffc8, // 0x50 - 0x57
    0xffc9, 0xff50, 0xff52, 0x0000, 0x0000, 0x0000, 0xff53, 0xff57, // 0x58 - 0x5f
    0xff54, 0xff56, 0xff63, 0xffff, 0xff8d, 0xffe4, 0xff13, 0xff61, // 0x60 - 0x67
    0xffaf, 0xffe8, 0x0000, 0xffeb, 0xffec, 0xff67, 0x0000, 0x0000, // 0x68 - 0x6f
    0x0000, 0x0000, 0x0000, 0x0000, 0xfe03, 0xffe9, 0xffbd, 0xffeb, // 0x70 - 0x77
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x78 - 0x7f
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x80 - 0x87
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x88 - 0x8f
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x90 - 0x97
    0x0000, 0x0000, 0x0000, 0x0000, 0xff8d, 0xffe4, 0x0000, 0x0000, // 0x98 - 0x9f
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xa0 - 0xa7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xa8 - 0xaf
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffaf, 0x0000, 0xff61, // 0xb0 - 0xb7
    0xffe8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xb8 - 0xbf
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xff50, // 0xc0 - 0xc7
    0xff52, 0xff55, 0x0000, 0xff51, 0x0000, 0xff53, 0x0000, 0xff57, // 0xc8 - 0xcf
    0xff54, 0xff56, 0xff63, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, // 0xd0 - 0xd7
    0x0000, 0x0000, 0x0000, 0xffeb, 0xffec, 0xff67, 0x0000, 0x0000, // 0xd8 - 0xdf
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xe0 - 0xe7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xe8 - 0xef
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xf0 - 0xf7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0xf8 - 0xff
};
static_assert(sizeof(macos_shift_capslock) == 256 * 2);

sized_array_view<KeyLayout::unicode_t, 256> macos_layout_for(kbdtypes::KeyModFlags mods, bool verbose) noexcept
{
    using kbdtypes::KeyMod;

    char const* mod_name = "PLain layout";
    sized_array_view<KeyLayout::unicode_t, 256> layout = macos_noshift;

    // if left ctrl and left alt are pressed, vnc server will convert key combination itself.
    if (((mods.test(KeyMod::LCtrl) || mods.test(KeyMod::RCtrl)) && mods.test(KeyMod::LAlt))
     || mods.test(KeyMod::RAlt)
    ) {
        mod_name = "Altgr Layout";
        layout = macos_altgr;
    }
    else if (mods.test(KeyMod::LShift) || mods.test(KeyMod::RShift)) {
        if (mods.test(KeyMod::CapsLock)) {
            mod_name = "Shift Capslock Layout";
            layout = macos_shift_capslock;
        }
        else {
            mod_name = "Shift Layout";
            layout = macos_shift;
        }
    }
    else if (mods.test(KeyMod::CapsLock)) {
        mod_name = "CapsLock Layout";
        layout = macos_capslock;
    }

    LOG_IF(verbose, LOG_INFO, "KeymapSym::_update_keymap(): %s", mod_name);
    return layout;
}

void push_alt_gr_state(
    KeymapSym::Keys& keys, kbdtypes::KeyModFlags mods,
    KeymapSym::VncKeyState down_flag, bool is_win) noexcept
{
    using kbdtypes::KeyMod;
    using kbdtypes::KeyCode;

    if (REDEMPTION_UNLIKELY(mods.test(KeyMod::RAlt))) {
        keys.push({keycode_to_sym(KeyCode::RAlt, mods), down_flag});
    }

    // on window ctrl+alt = altgr
    if (REDEMPTION_UNLIKELY(is_win && mods.test(KeyMod::LAlt))) {
        if (mods.test(KeyMod::LCtrl) || mods.test(KeyMod::RCtrl)) {
            keys.push({keycode_to_sym(KeyCode::LAlt, mods), down_flag});
            if (mods.test(KeyMod::RCtrl)) {
                keys.push({keycode_to_sym(KeyCode::RCtrl, mods), down_flag});
            }
            if (mods.test(KeyMod::LCtrl)) {
                keys.push({keycode_to_sym(KeyCode::LCtrl, mods), down_flag});
            }
        }
    }
}

bool is_ctrl_alt(kbdtypes::KeyModFlags mods) noexcept
{
    using kbdtypes::KeyMod;
    auto ctrl_alt = mods & (KeyMod::LCtrl | KeyMod::RCtrl | KeyMod::LAlt);
    ctrl_alt.clear(KeyMod::LAlt);
    return ctrl_alt.as_uint();
}

KeymapSym::VncKeyState to_vnc_key_state(kbdtypes::KbdFlags flags) noexcept
{
    return bool(flags & kbdtypes::KbdFlags::Release)
        ? KeymapSym::VncKeyState::Up
        : KeymapSym::VncKeyState::Down;
}

void push_key(KeymapSym::Keys& keys, uint32_t uc, KeymapSym::VncKeyState down_flag, kbdtypes::KeyModFlags mods, bool is_win) noexcept
{
    const auto ksym = unicode_to_ksym(uc);
    push_alt_gr_state(keys, mods, KeymapSym::VncKeyState::Up, is_win);
    keys.push({ksym, down_flag});
    push_alt_gr_state(keys, mods, KeymapSym::VncKeyState::Down, is_win);
}

using kbdtypes::KeyMod;
using kbdtypes::KeyCode;

} // anonymous namespace

KeymapSym::KeymapSym(KeyLayout layout, kbdtypes::KeyLocks locks, IsApple is_apple, IsUnix is_unix, bool verbose) noexcept
: layout_(layout)
, keymap_(macos_noshift)
, mods_(locks)
, is_win_(!bool(is_unix))
, is_apple_(bool(is_apple))
, verbose_(verbose)
{
    _update_keymap();
}

KeymapSym::Keys KeymapSym::scancode_to_keysyms(KbdFlags flags, Scancode scancode) noexcept
{
    Keys keys;

    const auto keycode = kbdtypes::to_keycode(flags, scancode);
    const auto down_flag = to_vnc_key_state(flags);

    auto set_mod = [&](KeyMod mod){
        keys.push({keycode_to_sym(keycode, mods_), down_flag});
        mods_.update(flags, mod);
        _update_keymap();
    };

    auto set_locks_mod = [&](KeyMod mod){
        keys.push({keycode_to_sym(keycode, mods_), down_flag});
        if (!(underlying_cast(flags) & underlying_cast(KbdFlags::Release))) {
            mods_.flip(mod);
            _update_keymap();
        }
    };

    switch (underlying_cast(keycode))
    {
        // Lock keys
        case underlying_cast(KeyCode::CapsLock):   set_locks_mod(KeyMod::CapsLock); break;
        case underlying_cast(KeyCode::NumLock):    set_locks_mod(KeyMod::NumLock); break;
        case underlying_cast(KeyCode::ScrollLock): set_locks_mod(KeyMod::ScrollLock); break;

        // Modifier keys
        case underlying_cast(KeyCode::LCtrl):  set_mod(KeyMod::LCtrl); break;
        case underlying_cast(KeyCode::RCtrl):  set_mod(KeyMod::RCtrl); break;
        case underlying_cast(KeyCode::LShift): set_mod(KeyMod::LShift); break;
        case underlying_cast(KeyCode::RShift): set_mod(KeyMod::RShift); break;
        case underlying_cast(KeyCode::LAlt):   set_mod(KeyMod::LAlt); break;
        case underlying_cast(KeyCode::RAlt):   set_mod(KeyMod::RAlt); break;

        // ctrl+alt+fin as ctrl+alt+del
        case underlying_cast(KeyCode::End):  {
            const auto ksym = is_ctrl_alt(mods_)
                ? keycode_to_sym(KeyCode::Delete, mods_)
                : keycode_to_sym(KeyCode::End, mods_);
            keys.push({ksym, down_flag});
            break;
        }

        default:
            if (auto ksym = keycode_to_sym(keycode, mods_)) {
                keys.push({ksym, down_flag});
            }
            else if (REDEMPTION_UNLIKELY(!keycode_is_compressable_to_byte(keycode))) {
                // unknown value
            }
            // MacOs (old version) considers keysyms to be keycodes
            else if (REDEMPTION_UNLIKELY(is_apple_)) {
                auto keymap_index = keycode_to_byte_index(keycode);
                keys.push({keymap_[keymap_index], down_flag});
            }
            else {
                const std::size_t keymap_index = keycode_to_byte_index(keycode);
                auto unicode = keymap_[keymap_index];

                if (REDEMPTION_UNLIKELY(!unicode)) {
                    break;
                }

                auto push1 = [&](uint32_t uc){
                    push_key(keys, uc, down_flag, mods_, is_win_);
                };

                if (underlying_cast(flags) & underlying_cast(KbdFlags::Release)) {
                    if (!dkeys_) {
                        if (!(unicode & KeyLayout::DK)) {
                            push1(unicode);
                        }
                        else {
                            push1(layout_.dkeymap_by_mod[imods_][keymap_index].accent());
                        }
                    }
                }
                else if (REDEMPTION_UNLIKELY(dkeys_)) {
                    if (auto unicode2 = dkeys_.find_composition(unicode)) {
                        push1(unicode2);
                    }
                    // Windows(c) behavior for backspace following a Deadkey
                    else if (keycode != KeyCode::Backspace) {
                        const auto ksym1 = unicode_to_ksym(dkeys_.accent());
                        const auto ksym2 = unicode_to_ksym(unicode & ~KeyLayout::DK);
                        push_alt_gr_state(keys, mods_, VncKeyState::Up, is_win_);
                        keys.push({ksym1, VncKeyState::Down});
                        keys.push({ksym1, VncKeyState::Up});
                        keys.push({ksym2, VncKeyState::Down});
                        push_alt_gr_state(keys, mods_, VncKeyState::Down, is_win_);
                    }
                    dkeys_ = {};
                }
                else if (REDEMPTION_UNLIKELY(unicode & KeyLayout::DK)) {
                    dkeys_ = layout_.dkeymap_by_mod[imods_][keymap_index];
                }
                else {
                    push1(unicode);
                }
            }
    }

    return keys;
}

KeymapSym::Keys KeymapSym::reset_mods(KeyLocks locks) noexcept
{
    KeymapSym::Keys keys;

    auto push = [&](uint32_t ksym, KeyMod mod){
        keys.keys[keys.len] = Key{ksym, VncKeyState::Up};
        keys.len += mods_.test_as_uint(mod);
    };

    push(vnc_key_RAlt, KeyMod::RAlt);
    push(vnc_key_LAlt, KeyMod::LAlt);
    push(vnc_key_LCtrl, KeyMod::LCtrl);
    push(vnc_key_RCtrl, KeyMod::RCtrl);
    push(vnc_key_LShift, KeyMod::LShift);
    push(vnc_key_RShift, KeyMod::RShift);
    // Window key has an event on release, discard
    // push(vnc_key_LWin, KeyMod::LMeta);
    // push(vnc_key_RWin, KeyMod::RMeta);

    mods_.reset();
    mods_.sync_locks(locks);
    _update_keymap();

    return keys;
}

void KeymapSym::_update_keymap() noexcept
{
    if (REDEMPTION_UNLIKELY(is_apple_)) {
        keymap_ = macos_layout_for(mods_, verbose_);
        return ;
    }

    auto rctrl_is_ctrl = unsigned(layout_.right_ctrl_is_ctrl);

    unsigned numlock = mods_.test_as_uint(KeyMod::NumLock);
    unsigned capslock = mods_.test_as_uint(KeyMod::CapsLock);
    unsigned ctrl = mods_.test_as_uint(KeyMod::LCtrl)
                  | (mods_.test_as_uint(KeyMod::RCtrl) & rctrl_is_ctrl);
    unsigned oem8 = mods_.test_as_uint(KeyMod::RCtrl) & ~rctrl_is_ctrl;
    unsigned alt = mods_.test_as_uint(KeyMod::LAlt);
    unsigned shift = mods_.test_as_uint(KeyMod::LShift)
                   | mods_.test_as_uint(KeyMod::RShift);
    // enable Ctrl and Alt when AltGr
    unsigned altgr = mods_.test_as_uint(KeyMod::RAlt) | (ctrl & alt);

    imods_ = checked_int(0u
           | (shift << KeyLayout::Mods::Shift)
           | (altgr << KeyLayout::Mods::Control)
           | (altgr << KeyLayout::Mods::Menu)
           | (oem8 << KeyLayout::Mods::OEM_8)
           | (numlock << KeyLayout::Mods::NumLock)
           | (capslock << KeyLayout::Mods::CapsLock)
           );
    keymap_ = layout_.keymap_by_mod[imods_];
}

KeymapSym::Keys KeymapSym::utf16_to_keysyms(KbdFlags flag, uint16_t utf16) noexcept
{
    auto is_surrogate = [](uint16_t uc) noexcept { return (uc - 0xd800u) < 2048u; };
    auto is_high_surrogate = [](uint16_t uc) noexcept { return (uc & 0xfffffc00) == 0xd800; };
    auto is_low_surrogate = [](uint16_t uc) noexcept { return (uc & 0xfffffc00) == 0xdc00; };
    auto surrogate_to_utf32 = [](uint32_t high, uint32_t low) noexcept { return (high << 10) + low - 0x35fdc00; };

    const auto unicode = [&]() -> uint32_t {
        if (REDEMPTION_UNLIKELY(previous_unicode16_)) {
            if (is_low_surrogate(utf16)) {
                auto uc = surrogate_to_utf32(previous_unicode16_, utf16);
                previous_unicode16_ = 0;
                return uc;
            }
            // else {
            //     // error
            // }
            previous_unicode16_ = 0;
        }

        if (!is_surrogate(utf16)) {
            return utf16;
        }

        if (is_high_surrogate(utf16)) {
            previous_unicode16_ = utf16;
        }

        return 0;
    }();

    Keys keys;

    if (unicode) {
        push_key(keys, unicode, to_vnc_key_state(flag), mods_, is_win_);
    }

    return keys;
}
