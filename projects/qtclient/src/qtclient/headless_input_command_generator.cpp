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
    using AV = bounded_chars_view<0, 4>;
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
        minute_suffix = (has_second || has_milli) ? AV("min"_sized_av) : AV("min "_sized_av);
    }

    if (has_second) {
        int_to_decimal_chars(seconds_chars, sec.count());
        second_suffix = has_milli ? AV("s "_sized_av) : AV("s"_sized_av);
    }

    if (has_milli) {
        int_to_decimal_chars(milliseconds_chars, milli.count());
        milli_suffix = "ms"_sized_av;
    }

    return static_str_concat<128>(
        cmd,
        minutes_chars, minute_suffix,
        seconds_chars, second_suffix,
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
    notifier(Status::NewLine, enable ? "kbd fr"_av : "kbd en"_av, 0);
    cmd_type = CmdType::Default;
    cmd.clear();
}

void HeadlessInputCommandGenerator::set_key_delay(DelayConfig key_delay)
{
    notifier(Status::NewLine, make_cmd_delay("keydelay "_sized_av, key_delay.delay), 0);
    max_keydelay = std::chrono::duration_cast<MonotonicTimePoint::duration>(key_delay.delay + key_delay.threshold);
    cmd_type = CmdType::Default;
    cmd.clear();
}

void HeadlessInputCommandGenerator::scancode(MonotonicTimePoint now, KbdFlags flags, Scancode scancode)
{
    if (cmd_type != CmdType::Scancode || now > previous_time + max_keydelay) {
        notifier(Status::NewLine, make_cmd_delay("sleep "_sized_av, now - previous_time), 0);
        cmd_type = CmdType::Sleep;
    }

    auto oldtype = std::exchange(cmd_type, CmdType::Scancode);
    auto status = Status::UpdateLastLine;
    if (oldtype != CmdType::Scancode) {
        cmd = "key ";
        mods = 0;
        status = Status::NewLine;
    }

    previous_time = now;

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

    int_to_chars_result rep_buffer;
    int_to_chars_result sc_buffer;

    auto sc_and_flag_to_hex = [&]{
        auto n = checked_cast<uint16_t>(uint_sc | underlying_cast(flags & ~KbdFlags::Release));
        int_to_hexadecimal_upper_chars(sc_buffer, n);
        return chars_view(sc_buffer);
    };

    struct Format
    {
        chars_view open;
        chars_view sc;
        chars_view flag;
        chars_view rep;
        chars_view close;
    };
    Format format {};

    auto set_format = [&](chars_view open, chars_view flag, chars_view rep, chars_view close){
        if (has_named_sc()) {
            format = Format{
                .open = open,
                .sc = names[uint_sc],
                .flag = flag,
                .rep = rep,
                .close = close,
            };
        }
        else {
            format = Format{
                .open = "{0x"_av,
                .sc = sc_and_flag_to_hex(),
                .flag = flag,
                .rep = rep,
                .close = "}"_av,
            };
        }
    };

    if (same_key && released) {
        // {sc down} + {sc up} => sc
        if (previous_values.scancode.repetition == 1) {
            cmd.resize(previous_values.scancode.previous_len);

            bool escaped = true;
            if (has_named_sc() && names[uint_sc].size() == 1) {
                char c = names[uint_sc][0];
                // escape special key mod
                escaped = (c == '!' || c == '+' || c == '^' || c == '#' || c == '~' || c == '{');
            }
            auto open = escaped ? "{"_av : ""_av;
            auto close = escaped ? "}"_av : ""_av;
            set_format(open, ""_av, ""_av, close);
        }
        // {sc down repetition} + {sc up} => {sc repetition}
        else {
            mods = 0;
            cmd.resize(previous_values.scancode.previous_len);

            int_to_decimal_chars(rep_buffer, previous_values.scancode.repetition);
            set_format("{"_av, " "_av, rep_buffer, "}"_av);
        }

        previous_values.scancode.repetition = 0;
    }
    // {sc down} + {sc down} => {sc down repetition}
    else if (same_key && !released) {
        mods = 0;
        ++previous_values.scancode.repetition;

        cmd.resize(previous_values.scancode.previous_len);

        int_to_decimal_chars(rep_buffer, previous_values.scancode.repetition);
        set_format("{"_av, " down "_av, rep_buffer, "}"_av);
    }
    // {sc flag}
    else {
        auto modmask = to_mod_mask(flags, scancode);

        // previous mod equal released current mod
        // {mod down} sc {mod up} => mod+sc
        if (modmask && released
         && (mods & 0b11111'00000) == (modmask << 5)
         && previous_values.scancode.repetition == 0
        ) {
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
            previous_values.scancode.repetition = 1;

            auto av_flag = released ? " up"_av : " down"_av;
            set_format("{"_av, av_flag, ""_av, "}"_av);
        }
    }

    str_append(cmd, format.open, format.sc, format.flag, format.rep, format.close);

    previous_values.scancode.flags = flags;
    previous_values.scancode.scancode = scancode;
    notifier(status, cmd, status == Status::UpdateLastLine ? previous_values.scancode.previous_len : 0);
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
