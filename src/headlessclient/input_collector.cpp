/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "headlessclient/input_collector.hpp"


bool InputCollector::has_input() const noexcept
{
    return current_input_index < inputs.size();
}

void InputCollector::push_data(InputCollector::InputData data)
{
    if (current_input_index) {
        inputs.erase(inputs.begin(), inputs.begin() + checked_int(current_input_index));
        current_input_index = 0;
    }

    inputs.push_back(data);
}

InputCollector::ConsumedInput InputCollector::send_next_input(RdpInput& mod, Keymap const& keymap)
{
    if (has_input()) {
        auto& input = inputs[current_input_index];

        switch (input.type) {
            case InputType::Scancode:
                mod.rdp_input_scancode(
                    checked_int(input.flags_or_locks),
                    checked_int(input.sc_or_uc_or_x),
                    0,
                    keymap
                );
                break;

            case InputType::Unicode:
                mod.rdp_input_unicode(
                    checked_int(input.flags_or_locks),
                    checked_int(input.sc_or_uc_or_x)
                );
                break;

            case InputType::KeyLock:
                mod.rdp_input_synchronize(checked_int(input.flags_or_locks));
                break;

            case InputType::Mouse:
                mod.rdp_input_mouse(
                    checked_int(input.flags_or_locks),
                    input.sc_or_uc_or_x,
                    input.y
                );
                break;

            case InputType::MouseEx:
                mod.rdp_input_mouse_ex(
                    checked_int(input.flags_or_locks),
                    input.sc_or_uc_or_x,
                    input.y
                );
                break;
        }

        ++current_input_index;

        if (current_input_index < inputs.size()) {
            return (input.type == InputType::Mouse)
                ? ConsumedInput::MouseEvent
                : ConsumedInput::KeyEvent
            ;
        }
    }

    return ConsumedInput::None;
}
