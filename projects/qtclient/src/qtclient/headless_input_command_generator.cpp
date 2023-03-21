/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "headlessclient/headless_command_values.hpp"
#include "qtclient/headless_input_command_generator.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/sugar/bounded_array_view.hpp"
#include "utils/strutils.hpp"
#include "utils/static_string.hpp"
#include "utils/ascii.hpp"

#include <utility>


namespace
{

auto make_cmd_delay_impl(
    bounded_chars_view<0, 16> cmd,
    std::chrono::minutes min,
    std::chrono::seconds sec,
    std::chrono::milliseconds milli)
{
    using AV = bounded_chars_view<0, 3>;
    AV empty_suffix = ""_sized_av;
    AV minute_suffix = empty_suffix;
    AV second_suffix = empty_suffix;
    AV milli_suffix = empty_suffix;

    int_to_chars_result minutes_chars;
    int_to_chars_result seconds_chars;
    int_to_chars_result milliseconds_chars;

    bool has_minute = min.count();
    bool has_second = sec.count();
    bool has_milli = !has_minute && milli.count();

    if (has_minute) {
        int_to_decimal_chars(minutes_chars, min.count());
        minute_suffix = "min"_sized_av;
    }

    if (has_second) {
        int_to_decimal_chars(seconds_chars, sec.count());
        if (!has_minute) {
            second_suffix = "s"_sized_av;
        }
    }

    if (has_milli) {
        int_to_decimal_chars(milliseconds_chars, milli.count());
        if (!has_second) {
            milli_suffix = "ms"_sized_av;
        }
    }

    return static_str_concat<128>(
        cmd,
        minutes_chars, minute_suffix,
        seconds_chars, seconds_chars,
        milliseconds_chars, milli_suffix
    );
}

template<class Duration>
auto make_cmd_delay(bounded_chars_view<0, 16> cmd, Duration delay)
{
    auto min = std::chrono::duration_cast<std::chrono::minutes>(delay);
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(delay - min);
    auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(delay - min - sec);
    return make_cmd_delay_impl(cmd, min, sec, milli);
}

constexpr size_t compute_reversed_scancode_table_size(bool extended, array_view<headlessclient::ScancodePair> names_scancodes)
{
    std::size_t max = 0;
    uint16_t mask = extended ? 0x100 : 0;
    for (auto name_sc : names_scancodes) {
        if ((name_sc.value & 0x100) == mask) {
            max = std::max(max, static_cast<std::size_t>(name_sc.value & 0xff));
        }
    }
    return max + 1;
}

template<auto const& names_scancodes>
struct named_reversed_scancode_table
{
    std::string_view names[compute_reversed_scancode_table_size(false, names_scancodes)] {};
    std::string_view extended_names[compute_reversed_scancode_table_size(true, names_scancodes)] {};

    constexpr named_reversed_scancode_table()
    {
        for (auto name_sc : names_scancodes) {
            if (name_sc.value & 0x100) {
                extended_names[name_sc.value & 0xff] = name_sc.name;
            }
            else {
                names[name_sc.value] = name_sc.name;
            }
        }
    }
};

constexpr named_reversed_scancode_table<headlessclient::names_scancodes_en> reversed_scancodes_table_en;
constexpr named_reversed_scancode_table<headlessclient::names_scancodes_fr> reversed_scancodes_table_fr;

constexpr array_view<std::string_view> reversed_scancodes_names[] {
    reversed_scancodes_table_fr.names,
    reversed_scancodes_table_en.names,
};
constexpr array_view<std::string_view> reversed_extended_scancodes_names_en[] {
    reversed_scancodes_table_fr.extended_names,
    reversed_scancodes_table_en.extended_names,
};

namespace modmasks
{
    constexpr uint32_t LShift = 0b10000;
    constexpr uint32_t LCtrl  = 0b01000;
    constexpr uint32_t LWin   = 0b00100;
    constexpr uint32_t LAlt   = 0b00010;
    constexpr uint32_t AltGr  = 0b00001;
}

uint32_t to_mod_mask(kbdtypes::KbdFlags flags, kbdtypes::Scancode scancode)
{
    using Scancode = kbdtypes::Scancode;
    using KbdFlags = kbdtypes::KbdFlags;

    // without extended flag
    if ((flags | KbdFlags::Release) == KbdFlags::Release) {
        if (scancode == Scancode::LShift) return modmasks::LShift;
        if (scancode == Scancode::LCtrl) return modmasks::LCtrl;
        if (scancode == Scancode::LAlt) return modmasks::LAlt;
    }
    // with extended flag
    else if ((flags | KbdFlags::Release | KbdFlags::Extended) == (KbdFlags::Release | KbdFlags::Extended)) {
        if (scancode == Scancode::LWin) return modmasks::LWin;
        if (scancode == Scancode::LAlt) return modmasks::AltGr;
    }

    return 0;
}

constexpr std::array<std::size_t, 32> key_len_table_for_mod_mask = []{
    std::array<std::size_t, 32> a {};
    a[modmasks::LShift] = sizeof("{LShift down}") - 1;
    a[modmasks::LCtrl] = sizeof("{LCtrl down}") - 1;
    a[modmasks::LWin] = sizeof("{LMeta down}") - 1;
    a[modmasks::LAlt] = sizeof("{LAlt down}") - 1;
    a[modmasks::AltGr] = sizeof("{AltGr down}") - 1;
    return a;
}();

constexpr std::array<char, 32> key_table_for_mod_mask = []{
    std::array<char, 32> a {};
    a[modmasks::LShift] = '+';
    a[modmasks::LCtrl] = '^';
    a[modmasks::LWin] = '#';
    a[modmasks::LAlt] = '!';
    a[modmasks::AltGr] = '~';
    return a;
}();

} // anonymous namespace


enum class HeadlessInputCommandGenerator::CmdType : uint8_t
{
    Default,
    Sleep,
    Scancode,
    Unicode,
    Key,
    Text,
    Mouse,
    Move,
    Scroll,
    Lock,
    Kbd,
    Connect,
    Disconnect,
};

void HeadlessInputCommandGenerator::start(MonotonicTimePoint now)
{
    previous_time = now;
    cmd_type = CmdType::Default;
    cmd.clear();
}

void HeadlessInputCommandGenerator::set_kbd_fr(bool enable)
{
    is_en_kbd = !enable;
    notifier(Status::NewLine, enable ? "kbd fr"_av : "kbd en"_av);
    cmd_type = CmdType::Default;
    cmd.clear();
}

void HeadlessInputCommandGenerator::set_key_delay(DelayConfig key_delay)
{
    notifier(Status::NewLine, make_cmd_delay("keydelay "_sized_av, key_delay.delay));
    max_keydelay = std::chrono::duration_cast<MonotonicTimePoint::duration>(key_delay.delay + key_delay.threshold);
    cmd_type = CmdType::Default;
    cmd.clear();
}

void HeadlessInputCommandGenerator::scancode(MonotonicTimePoint now, KbdFlags flags, Scancode scancode)
{
    auto oldtype = std::exchange(cmd_type, CmdType::Scancode);

    if (oldtype != CmdType::Scancode || now > previous_time + max_keydelay) {
        notifier(Status::NewLine, make_cmd_delay("sleep "_sized_av, now - previous_time));
        oldtype = CmdType::Sleep;
    }

    auto status = Status::UpdateLastLine;
    if (oldtype != CmdType::Scancode) {
        cmd = "key ";
        mods = 0;
        status = Status::NewLine;
    }

    previous_time = now;

    // TODO Print Scancode
    // TODO lshift+down sc lshift+up => SC

    auto const mask = KbdFlags::Extended | KbdFlags::Release;
    auto uint_sc = underlying_cast(scancode);
    auto is_extended = bool(flags & KbdFlags::Extended);
    auto released = bool(flags & KbdFlags::Release);
    auto names = is_extended
        ? reversed_extended_scancodes_names_en[is_en_kbd]
        : reversed_scancodes_names[is_en_kbd];

    auto has_named_sc = [&]{
        return (flags | mask) == mask
            && uint_sc < names.size()
            && !names[uint_sc].empty()
        ;
    };

    auto same_key
       = oldtype == CmdType::Scancode
      && previous_values.scancode.scancode == scancode
      && previous_values.scancode.flags == (flags & ~KbdFlags::Release);

    if (same_key && released) {
        // {sc down} + {sc up} => sc
        if (previous_values.scancode.repetition == 1) {
            cmd.resize(previous_values.scancode.previous_len);

            if (has_named_sc()) {
                auto name = names[uint_sc];
                if (name.size() == 1) {
                    char c = name[0];
                    // escape special key mod
                    if (REDEMPTION_UNLIKELY(c == '!' || c == '+' || c == '^' || c == '#' || c == '~' || c == '{')) {
                        char buf[3] {'{', c, '}'};
                        cmd += std::string_view(buf, 3);
                    }
                    else {
                        cmd += name[0];
                    }
                }
                else {
                    str_append(cmd, '{', name, '}');
                }
            }
            else {
                str_append(cmd,
                    is_extended ? "{0x1"_av : "{0x"_av,
                    int_to_fixed_hexadecimal_upper_chars(uint_sc), '}'
                );
            }
        }
        // {sc down repetition} + {sc up} => {sc repetition}
        else {
            mods = 0;
            cmd.resize(previous_values.scancode.previous_len);

            auto rep = int_to_decimal_chars(previous_values.scancode.repetition);
            if (has_named_sc()) {
                str_append(cmd, '{', names[uint_sc], ' ', rep, '}');
            }
            else {
                str_append(cmd,
                    is_extended ? "{0x1"_av : "{0x"_av,
                    int_to_fixed_hexadecimal_upper_chars(uint_sc), ' ',
                    rep, '}'
                );
            }
        }

        previous_values.scancode.repetition = 0;
    }
    // {sc down} + {sc down} => {sc down repetition}
    else if (same_key && !released) {
        mods = 0;
        ++previous_values.scancode.repetition;

        cmd.resize(previous_values.scancode.previous_len);

        auto rep = int_to_decimal_chars(previous_values.scancode.repetition);
        if (has_named_sc()) {
            str_append(cmd, '{', names[uint_sc], " down "_av, rep, '}');
        }
        else {
            str_append(cmd,
                is_extended ? "{0x1"_av : "{0x"_av,
                int_to_fixed_hexadecimal_upper_chars(uint_sc), " down "_av,
                rep, '}'
            );
        }
    }
    // {sc flag}
    else {
        auto modmask = to_mod_mask(flags, scancode);

        // previous mod equal released current mod
        // {mod down} sc {mod up} => mod+sc
        if (modmask && released && (mods & 0b11111'00000) == (modmask << 5) && previous_values.scancode.repetition == 0) {
            auto previous_len = previous_values.scancode.previous_len;
            auto mod_len = key_len_table_for_mod_mask[modmask];
            auto p = cmd.begin() + checked_int(previous_len);
            auto end = p;
            // upper case for shift: {LShift down} x => X
            if (modmask == modmasks::LShift && cmd.size() - previous_len == 1 && ascii_is_lower(cmd[previous_len])) {
                cmd[previous_len - mod_len] = ascii_to_upper(cmd[previous_len]);
                ++end;
            }
            else {
                cmd[previous_len - mod_len] = key_table_for_mod_mask[modmask];
            }
            previous_values.scancode.previous_len = previous_len - mod_len;
            cmd.erase(p - checked_int(mod_len) + 1, end);
            mods = (mods >> 10) << 5;
        }
        else {
            previous_values.scancode.previous_len = cmd.size();

            // previous key have no mod, reset all mod
            if (released || !(mods & 0b11111)) {
                mods = 0;
            }
            // mod already defined, reset all mods
            else if (mods & ((modmask << 20) | (modmask << 15) | (modmask << 10) | modmask)) {
                mods = 0;
            }
            else {
                mods <<= 5;
            }

            mods |= modmask;

            auto av_flag = released ? " up}"_av : " down}"_av;
            if (has_named_sc()) {
                str_append(cmd, '{', names[uint_sc], av_flag);
            }
            else {
                str_append(cmd,
                    is_extended ? "{0x1"_av : "{0x"_av,
                    int_to_fixed_hexadecimal_upper_chars(uint_sc),
                    av_flag
                );
            }

            previous_values.scancode.repetition = 1;
        }
    }

    previous_values.scancode.flags = flags;
    previous_values.scancode.scancode = scancode;
    notifier(status, cmd);
}

void HeadlessInputCommandGenerator::unicode(MonotonicTimePoint now, KbdFlags flag, uint16_t unicode)
{
    (void)now;
    (void)flag;
    (void)unicode;
    // lines.emplace_back(str_concat(
    //     "uni "_av,
    //     int_to_fixed_hexadecimal_upper_chars(unicode), ',',
    //     int_to_fixed_hexadecimal_upper_chars(underlying_cast(flag))
    // ));
    // notifier(Status::NewLine, lines.back());
}

void HeadlessInputCommandGenerator::mouse(MonotonicTimePoint now, uint16_t device_flags, uint16_t x, uint16_t y)
{
    (void)now;
    (void)device_flags;
    (void)x;
    (void)y;
    // uint16_t button_mask = (MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_BUTTON2 | MOUSE_FLAG_BUTTON3 | MOUSE_FLAG_BUTTON4 | MOUSE_FLAG_BUTTON5);
    //
    // if (device_flags & MOUSE_FLAG_MOVE) {
    //     mouse_x = x;
    //     mouse_y = y;
    // }
    // else if (device_flags & MOUSE_FLAG_HWHEEL) {
    //     auto cmd = (device_flags & MOUSE_FLAG_WHEEL_NEGATIVE) ? "hscroll -1"_av : "hscroll"_av;
    //     lines.emplace_back(cmd.as<std::string>());
    //     notifier(Status::NewLine, lines.back());
    // }
    // else if (device_flags & MOUSE_FLAG_WHEEL) {
    //     auto cmd = (device_flags & MOUSE_FLAG_WHEEL_NEGATIVE) ? "scroll -1"_av : "scroll"_av;
    //     lines.emplace_back(cmd.as<std::string>());
    //     notifier(Status::NewLine, lines.back());
    // }
    // else if (device_flags & button_mask) {
    //     lines.emplace_back(str_concat(
    //         "m "_av,
    //         int_to_fixed_hexadecimal_upper_chars(checked_cast<uint16_t>(device_flags & button_mask)), ',',
    //         int_to_fixed_hexadecimal_upper_chars(device_flags)
    //     ));
    //     notifier(Status::NewLine, lines.back());
    // }
}

void HeadlessInputCommandGenerator::synchronize(MonotonicTimePoint now, KeyLocks locks)
{
    (void)now;
    (void)locks;
    // lines.emplace_back(str_concat(
    //     "lock "_av,
    //     int_to_fixed_hexadecimal_upper_chars(underlying_cast(locks))
    // ));
    // notifier(Status::NewLine, lines.back());
}
