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
   Author(s): Clément Moroldo
*/

#pragma once

#include "reversed_keymaps/keylayout_r.hpp"

#include <QtCore/QEvent>

#include "utils/log.hpp"

class Qt_ScanCode_KeyMap
{
    int8_t _keyboardMods = 0;
    Keylayout_r::KeyLayoutMap_t currentLayout;
    Keylayout_r::KeyLayoutMap_t const* currentDeadLayout = nullptr;
    Keylayout_r::DeadKeyLayoutMap_t deadLayout;
    Keylayout_r::KeyLayoutMap_t layouts[9];

    enum : int8_t
    {
        NO_MOD       = 0x00,
        SHIFT_MOD    = 0x01,
        ALTGR_MOD    = 0x02,
        CAPSLOCK_MOD = 0x04,
        CTRL_MOD     = 0x08,
        ALT_MOD      = 0x10,
    };

public:
    Qt_ScanCode_KeyMap(Keylayout_r const& keylayout)
    {
        this->setKeyboardLayout(keylayout);
    }

    void setKeyboardLayout(Keylayout_r const& keylayout)
    {
        LOG(LOG_DEBUG, "layout = %s", keylayout.locale_name);
        this->layouts[NO_MOD] = keylayout.noMod;
        this->layouts[SHIFT_MOD] = keylayout.shift;
        this->layouts[ALTGR_MOD] = keylayout.altGr;
        this->layouts[CTRL_MOD] = keylayout.ctrl;

        this->layouts[SHIFT_MOD | ALTGR_MOD] = keylayout.shiftAltGr;

        this->layouts[CAPSLOCK_MOD] = keylayout.capslock_noMod;
        this->layouts[CAPSLOCK_MOD | SHIFT_MOD] = keylayout.capslock_shift;
        this->layouts[CAPSLOCK_MOD | ALTGR_MOD] = keylayout.capslock_altGr;
        this->layouts[CAPSLOCK_MOD | SHIFT_MOD | ALTGR_MOD] = keylayout.capslock_shiftAltGr;

        this->deadLayout = keylayout.dead;
        this->currentLayout = keylayout.noMod;
        this->currentDeadLayout = nullptr;
    }

    struct Scancode
    {
        uint16_t scancode;
        uint16_t flag;
    };

    Scancode keyEvent(uint16_t flag, const int key)
    {
        auto update_modifier = [&](int modifier){
            if (flag == 0) {
                this->_keyboardMods += modifier;
            }
            else {
                this->_keyboardMods -= modifier;
            }

            auto mods = this->_keyboardMods;
            if ((mods & (CTRL_MOD | ALT_MOD)) == (CTRL_MOD | ALT_MOD)) {
                mods &= ~(CTRL_MOD | ALT_MOD);
                mods |= ALTGR_MOD;
            }
            else if (mods & ALTGR_MOD) {
                mods &= ~(CTRL_MOD | ALT_MOD);
            }
            else if (mods & CTRL_MOD) {
                mods = CTRL_MOD;
            }
            else {
                mods &= ~ALT_MOD; // ALT has no mod
            }

            this->currentLayout = this->layouts[mods];
        };

        const uint16_t extended = flag | 0x100;

        switch (key) {
            case Qt::Key_Alt        : update_modifier(ALT_MOD);      return {0x38, flag};
            case Qt::Key_AltGr      : update_modifier(ALTGR_MOD);    return {0x38, extended};
            case Qt::Key_Control    : update_modifier(CTRL_MOD);     return {0x1D, extended};
            case Qt::Key_Shift      : update_modifier(SHIFT_MOD);    return {0x36, flag};
            case Qt::Key_CapsLock   : update_modifier(CAPSLOCK_MOD); return {0x3A, flag};
        }

        if (this->currentDeadLayout) {
            auto scancode = key <= 0xffff ? this->currentDeadLayout->find(key) : uint8_t();
            this->currentDeadLayout = nullptr;
            if (scancode) {
                return {scancode, flag};
            }
        }

        auto dead_key = [&](uint8_t dkey) {
            auto scancode = this->layouts[0].find_ascii(dkey);
            if (scancode) {
                this->currentDeadLayout = this->deadLayout.find_layout(dkey);
            }
            return Scancode{scancode, flag};
        };

        switch (key) {
            case Qt::Key_Enter      : return {0x1C, extended};
            case Qt::Key_NumLock    : return {0x45, extended};
            case Qt::Key_Insert     : return {0x52, extended};
            case Qt::Key_Delete     : return {0x53, extended};
            case Qt::Key_End        : return {0x4F, extended};
            case Qt::Key_PageDown   : return {0x51, extended};
            case Qt::Key_PageUp     : return {0x49, extended};
            case Qt::Key_Up         : return {0x48, extended};
            case Qt::Key_Left       : return {0x4B, extended};
            case Qt::Key_Down       : return {0x50, extended};
            case Qt::Key_Right      : return {0x4D, extended};
            case Qt::Key_Meta       : return {0x5b, extended};
            case Qt::Key_Menu       : return {0x5D, extended};
            case Qt::Key_Home       : return {0x47, extended};

            case Qt::Key_Return     : return {0x1C, flag};
            case Qt::Key_Backspace  : return {0x0E, flag};
            case Qt::Key_Escape     : return {0x01, flag};
            case Qt::Key_F1         : return {0x3B, flag};
            case Qt::Key_F2         : return {0x3C, flag};
            case Qt::Key_F3         : return {0x3D, flag};
            case Qt::Key_F4         : return {0x3E, flag};
            case Qt::Key_F5         : return {0x3F, flag};
            case Qt::Key_F6         : return {0x40, flag};
            case Qt::Key_F7         : return {0x41, flag};
            case Qt::Key_F8         : return {0x42, flag};
            case Qt::Key_F9         : return {0x43, flag};
            case Qt::Key_F10        : return {0x44, flag};
            case Qt::Key_F11        : return {0x57, flag};
            case Qt::Key_F12        : return {0x58, flag};
            case Qt::Key_ScrollLock : return {0x46, flag};
            case Qt::Key_Pause      : return {0xE1, flag};
            case Qt::Key_Tab        : return {0x0F, flag};
            case Qt::Key_Backtab    : return {0x0F, flag};  // Shift + tab

            case Qt::Key_MediaStop            : return {0x24, extended};
            case Qt::Key_MediaPrevious        : return {0x10, extended};
            case Qt::Key_MediaNext            : return {0x19, extended};
            case Qt::Key_MediaTogglePlayPause : return {0x22, extended};
            case Qt::Key_LaunchMail           : return {0x47, extended};
            case Qt::Key_Search               : return {0x47, extended};
            case Qt::Key_Favorites            : return {0x66, extended};
            case Qt::Key_HomePage             : return {0x32, extended};
            case Qt::Key_VolumeDown           : return {0x2E, extended};
            case Qt::Key_VolumeUp             : return {0x30, extended};
            case Qt::Key_VolumeMute           : return {0x20, extended};
            case Qt::Key_PowerOff             : return {0x5E, extended};
            case Qt::Key_WakeUp               : return {0x63, extended};
            case Qt::Key_Sleep                : return {0x5F, extended};

            case Qt::Key_Dead_Circumflex       : return dead_key(0x5E);
            case Qt::Key_Dead_Grave            : return dead_key(0x60);
            case Qt::Key_Dead_Tilde            : return dead_key(0x7E);
            case Qt::Key_Dead_Diaeresis        : return dead_key(0xA8);
            case Qt::Key_Dead_Acute            : return dead_key(0xB4);
            case Qt::Key_Dead_Macron           : return dead_key(0xAF);
            case Qt::Key_Dead_Cedilla          : return dead_key(0xB8);
            // case Qt::Key_Dead_Breve            : return {0, flag};
            // case Qt::Key_Dead_Abovedot         : return {0, flag};
            // case Qt::Key_Dead_Abovering        : return {0, flag};
            // case Qt::Key_Dead_Doubleacute      : return {0, flag};
            // case Qt::Key_Dead_Caron            : return {0, flag};
            // case Qt::Key_Dead_Ogonek           : return {0, flag};
            // case Qt::Key_Dead_Iota             : return {0, flag};
            // case Qt::Key_Dead_Voiced_Sound     : return {0, flag};
            // case Qt::Key_Dead_Semivoiced_Sound : return {0, flag};
            // case Qt::Key_Dead_Belowdot         : return {0, flag};
            // case Qt::Key_Dead_Hook             : return {0, flag};
            // case Qt::Key_Dead_Horn             : return {0, flag};

            default:
                if (key < 0x7F) {
                    uint8_t lower = ('A' <= key && key <= 'Z') ? key + 'a' - 'A' : key;
                    if (auto scancode = this->currentLayout.find_ascii(lower)) {
                        return {scancode, flag};
                    }
                }
                return {0, flag};
        }
    }

    Scancode unicodeToScancode(uint16_t keycode) const
    {
        return {this->currentLayout.find(keycode), 0};
    }
};
