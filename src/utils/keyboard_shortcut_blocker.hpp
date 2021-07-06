/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2021
    Author(s): Christophe Grosjean, Jonathan Poelen,
               Meng Tan, Raphael Zhou
*/

#include "core/RDP/slowpath.hpp"
#include "keyboard/scancode/unicode_to_scancode.hpp"
#include "utils/log.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/splitter.hpp"
#include "utils/utf.hpp"

class KeyboardShortcutBlocker
{
    class Shortcut
    {
        const bool alt;
        const bool ctrl;
        const bool shift;
        const bool meta;

        const std::string final_key_str;

        uint16_t final_key_code = 0;

        bool pressed = false;

        std::string shortcut_string;

        const bool verbose;

    public:
        Shortcut(uint32_t keyboardLayout, bool alt, bool ctrl, bool shift, bool meta,
                std::string && final_key, bool verbose)
            : alt(alt), ctrl(ctrl), shift(shift), meta(meta)
            , final_key_str(std::move(final_key))
            , shortcut_string([this]() {
                    std::string shortcut_str;

                    if (this->ctrl)
                    {
                        if (!shortcut_str.empty()) {
                            shortcut_str += "+";
                        }
                        shortcut_str += "Ctrl";
                    }
                    if (this->alt)
                    {
                        if (!shortcut_str.empty()) {
                            shortcut_str += "+";
                        }
                        shortcut_str += "Alt";
                    }
                    if (this->shift)
                    {
                        if (!shortcut_str.empty()) {
                            shortcut_str += "+";
                        }
                        shortcut_str += "Shift";
                    }
                    if (this->meta)
                    {
                        if (!shortcut_str.empty()) {
                            shortcut_str += "+";
                        }
                        shortcut_str += "Meta";
                    }

                    if (!shortcut_str.empty()) {
                        shortcut_str += "+";
                    }
                    shortcut_str += this->final_key_str;

                    return shortcut_str;
                }())
            , verbose(verbose)
        {
            LOG_IF(this->verbose, LOG_INFO,
                "KeyboardShortcutBlocker::Shortcut::Shortcut(): Shortcut=\"%s\"",
                this->shortcut_string.c_str());

            this->setup_final_key(keyboardLayout);
        }

        void setup_final_key(uint32_t keyboardLayout)
        {
            if (!::strcasecmp(this->final_key_str.c_str(), "enter"))
            {
                this->final_key_code = 43;
            }
            else
            {
                uint16_t final_key_u16 = 0;

                if (UTF8toUTF16({this->final_key_str.c_str(), this->final_key_str.length()}, reinterpret_cast<uint8_t*>(&final_key_u16), sizeof(final_key_u16)) == 2)
                {
                    UTF16Lower(reinterpret_cast<uint8_t*>(&final_key_u16), sizeof(final_key_u16));

                    this->final_key_code = 0;

                    using namespace scancode;

                    ScancodeSeq scancode_seq =
                        unicode_to_scancode(keyboardLayout, final_key_u16);
                    array_view<Scancode16bits> scancodes =
                        scancode_seq.scancodes();

                    if (scancodes.empty())
                    {
                        LOG(LOG_ERR, "KeyboardShortcutBlocker::Shortcut::Shortcut(): No scancode sequence for unicode=%d", final_key_u16);
                    }
                    else if (scancodes.size() != 1)
                    {
                        LOG(LOG_ERR, "KeyboardShortcutBlocker::Shortcut::Shortcut(): Too much scancode sequence for unicode=%d", final_key_u16);
                    }
                    else
                    {
                        this->final_key_code = scancodes[0];
                    }
                }
                else
                {
                    LOG(LOG_ERR, "KeyboardShortcutBlocker::Shortcut::Shortcut(): Failed to convert UTF-8 string to UTF-16 string!");
                }
            }

            LOG_IF(this->verbose, LOG_INFO,
                "KeyboardShortcutBlocker::Shortcut::Shortcut(): FinalKeyCode=0x%X", this->final_key_code);
        }

        bool scancode_must_be_blocked(bool alt, bool ctrl, bool shift, bool meta, uint16_t keyboardFlags,
            uint16_t keyCode)
        {
            LOG_IF(this->verbose, LOG_INFO,
                "KeyboardShortcutBlocker::Shortcut::scancode_must_be_blocked(): "
                    "Ctrl=%s Alt=%s Shift=%s Meta=%s keyboardFlags=0x%04X",
                ctrl ? "Yes" : "No",
                alt ? "Yes" : "No",
                shift ? "Yes" : "No",
                meta ? "Yes" : "No",
                keyboardFlags);

            if (alt == this->alt &&
                ctrl == this->ctrl &&
                shift == this->shift &&
                meta == this->meta &&
                keyCode == this->final_key_code)
            {
                if (SlowPath::KBDFLAGS_RELEASE & keyboardFlags)
                {
                    if (this->pressed)
                    {
                        this->pressed = false;

                        LOG_IF(this->verbose, LOG_INFO,
                            "KeyboardShortcutBlocker::Shortcut::scancode_must_be_blocked(): Key-release event. Shortcut=\"%s\"",
                            this->shortcut_string.c_str());

                        return true;
                    }
                }
                else
                {
                    this->pressed = true;

                    LOG_IF(this->verbose, LOG_INFO,
                        "KeyboardShortcutBlocker::Shortcut::scancode_must_be_blocked(): Key-down event. Shortcut=\"%s\"",
                        this->shortcut_string.c_str());

                    return true;
                }
            }

            return false;
        }
    };

    std::vector<Shortcut> shortcuts;

public:
    KeyboardShortcutBlocker(uint32_t keyboardLayout, std::string const & configuration_string, bool verbose) :
        verbose(verbose)
    {
        LOG_IF(this->verbose, LOG_INFO,
            "KeyboardShortcutBlocker::KeyboardShortcutBlocker(): "
                "KeyboardLayout=0x%X ConfigurationString=\"%s\"",
            keyboardLayout, configuration_string.c_str());

        std::string shortcut;

        for (auto r : make_splitter(configuration_string, ',')) {
            auto trimmed = trim(r);
            if (trimmed.empty()) {
                continue;
            }

            shortcut.assign(trimmed.begin(), trimmed.end());
            this->add_shortcur(keyboardLayout, shortcut);
        }
    }

private:
    void add_shortcur(uint32_t keyboardLayout, std::string const & shortcut)
    {
        LOG_IF(this->verbose, LOG_INFO,
            "KeyboardShortcutBlocker::add_shortcur(): KeyboardLayout=0x%X Shortcut=\"%s\"",
            keyboardLayout, shortcut.c_str());

        bool alt = false;
        bool ctrl = false;
        bool shift = false;
        bool meta = false;

        std::string final_key;

        for (auto r : make_splitter(shortcut, '+')) {
            auto trimmed = trim(r);
            if (trimmed.empty()) {
                continue;
            }

            std::string item(trimmed.begin(), trimmed.end());

            if (!::strcasecmp(item.c_str(), "alt"))
            {
                alt = true;
            }
            else if (!::strcasecmp(item.c_str(), "ctrl"))
            {
                ctrl = true;
            }
            else if (!::strcasecmp(item.c_str(), "shift"))
            {
                shift = true;
            }
            else if (!::strcasecmp(item.c_str(), "meta") || !::strcasecmp(item.c_str(), "windows"))
            {
                meta = true;
            }
            else
            {
                if (!final_key.empty())
                {
                    LOG(LOG_WARNING,
                        "KeyboardShortcutBlocker::add_shortcur(): Invalid shortcut \"%s\"",
                        shortcut.c_str());
                    return;
                }

                final_key = std::move(item);
            }
        }

        this->shortcuts.emplace_back(keyboardLayout, alt, ctrl, shift, meta, std::move(final_key), this->verbose);
    }

public:
    bool scancode_must_be_blocked(uint16_t keyboardFlags, uint16_t keyCode)
    {
        bool pressed = !(keyboardFlags & SlowPath::KBDFLAGS_RELEASE);

        if (0x1D == keyCode)
        {
            if (keyboardFlags & SlowPath::KBDFLAGS_EXTENDED)
            {
                this->right_ctrl_pressed = pressed;
            }
            else
            {
                this->left_ctrl_pressed = pressed;
            }
        }
        else if (0x2A == keyCode)
        {
            this->left_shift_pressed = pressed;
        }
        else if (0x36 == keyCode)
        {
            this->right_shift_pressed = pressed;
        }
        else if (0x38 == keyCode)
        {
            if (keyboardFlags & SlowPath::KBDFLAGS_EXTENDED)
            {
                this->right_alt_pressed = pressed;
            }
            else
            {
                this->left_alt_pressed = pressed;
            }
        }
        else if (0x5B == keyCode)
        {
            this->left_meta_pressed = pressed;
        }
        else if (0x5C == keyCode)
        {
            this->right_meta_pressed = pressed;
        }

        bool mast_be_blocked = false;

        for (auto & shortcut : this->shortcuts)
        {
            if (shortcut.scancode_must_be_blocked(
                    this->left_alt_pressed || this->right_alt_pressed,
                    this->left_ctrl_pressed || this->right_ctrl_pressed,
                    this->left_shift_pressed || this->right_shift_pressed,
                    this->left_meta_pressed || this->right_meta_pressed,
                    keyboardFlags,
                    keyCode
                ))
                mast_be_blocked = true;
        }

        return mast_be_blocked;
    }

private:
    bool left_alt_pressed = false;
    bool left_ctrl_pressed = false;
    bool left_meta_pressed = false;
    bool left_shift_pressed = false;

    bool right_alt_pressed = false;
    bool right_ctrl_pressed = false;
    bool right_meta_pressed = false;
    bool right_shift_pressed = false;

    const bool verbose;
};

