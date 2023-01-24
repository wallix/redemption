/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/mod/accumulate_input_mod.hpp"
#include "utils/sugar/int_to_chars.hpp"

#include <cstring>

namespace
{

char* cpy(char* p, chars_view av)
{
    std::memcpy(p, av.data(), av.size());
    return p + av.size();
}

struct AppendBuffer
{
    char buffer[128];

    void append(std::string& output, char* end)
    {
        if (!output.empty()) {
            output += ", ";
        }
        output.append(buffer, end);
    }
};

} // anonymous namespace

void AccumulateInputMod::rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y)
{
    AppendBuffer buffer;
    char* p = buffer.buffer;
    p = cpy(p, "{flags=0x"_av);
    p = int_to_fixed_hexadecimal_upper_chars(p, device_flags);
    p = cpy(p, ", x="_av);
    p = cpy(p, int_to_decimal_chars(x));
    p = cpy(p, ", y="_av);
    p = cpy(p, int_to_decimal_chars(y));
    p = cpy(p, "}"_av);
    buffer.append(session_log.messages, p);
}

void AccumulateInputMod::rdp_input_scancode(
    RdpInput::KbdFlags flags, RdpInput::Scancode scancode, uint32_t time, const Keymap& keymap)
{
    (void)time;
    (void)keymap;

    AppendBuffer buffer;
    char* p = buffer.buffer;
    p = cpy(p, "{KbdFlags=0x"_av);
    p = int_to_fixed_hexadecimal_upper_chars(p, underlying_cast(flags));
    p = cpy(p, ", Scancode=0x"_av);
    p = int_to_fixed_hexadecimal_upper_chars(p, underlying_cast(scancode));
    p = cpy(p, "}"_av);
    buffer.append(session_log.messages, p);
}

void AccumulateInputMod::rdp_input_unicode(RdpInput::KbdFlags flag, uint16_t unicode)
{
    AppendBuffer buffer;
    char* p = buffer.buffer;
    p = cpy(p, "{KbdFlags=0x"_av);
    p = int_to_fixed_hexadecimal_upper_chars(p, underlying_cast(flag));
    p = cpy(p, ", Unicode=0x"_av);
    p = int_to_fixed_hexadecimal_upper_chars(p, unicode);
    p = cpy(p, "}"_av);
    buffer.append(session_log.messages, p);
}

void AccumulateInputMod::rdp_input_synchronize(RdpInput::KeyLocks locks)
{
    AppendBuffer buffer;
    char* p = buffer.buffer;
    p = cpy(p, "{KeyLocks=0x"_av);
    p = int_to_fixed_hexadecimal_upper_chars(p, underlying_cast(locks));
    p = cpy(p, "}"_av);
    buffer.append(session_log.messages, p);
}

void AccumulateInputMod::rdp_input_invalidate(Rect rect)
{
    AppendBuffer buffer;
    char* p = buffer.buffer;
    p = cpy(p, "{Invalidate={"_av);
    p = cpy(p, int_to_decimal_chars(rect.x));
    p = cpy(p, ", "_av);
    p = cpy(p, int_to_decimal_chars(rect.y));
    p = cpy(p, ", "_av);
    p = cpy(p, int_to_decimal_chars(rect.cx));
    p = cpy(p, ", "_av);
    p = cpy(p, int_to_decimal_chars(rect.cy));
    p = cpy(p, "}"_av);
    buffer.append(session_log.messages, p);
}
