/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "core/callback.hpp"
#include "utils/out_param.hpp"
#include "utils/sugar/split.hpp"
#include "utils/sugar/chars_to_int.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/utf.hpp"
#include "utils/ascii.hpp"
#include "keyboard/kbdtypes.hpp"
#include "keyboard/keymap.hpp"
#include "headlessclient/headless_command.hpp"
#include "headlessclient/headless_command_values.hpp"

#include <vector>
#include <memory>


namespace
{

bool parameter_name_compare(std::string_view a, std::string_view b)
{
    if (a.size() < b.size()) {
        return true;
    }
    if (a.size() > b.size()) {
        return false;
    }
    return a < b;
}

template<class Pair>
struct ParameterList
{
    // tp_uppercase + sort
    explicit ParameterList(array_view<Pair> seq)
    : len(seq.size())
    {
        // create a unique buffer for all chars
        std::size_t char_count = 0;
        for (auto const& elem : seq) {
            char_count += elem.name.size();
        }
        chars.reset(new char[char_count]);
        values.reset(new Pair[len]);


        // uppercase + copy
        char* charp = chars.get();
        auto* it = values.get();
        for (auto const& elem : seq) {
            *it = {{charp, elem.name.size()}, elem.value};
            ++it;
            for (char c : elem.name) {
                *charp++ = ascii_to_upper(c);
            }
        }

        std::sort(values.get(), values.get() + len, [](Pair const& a, Pair const& b) {
            return parameter_name_compare(a.name, b.name);
        });
    }

    Pair const* find(TaggedUpperStringView name) const
    {
        auto end = values.get() + len;
        auto it = std::lower_bound(values.get(), end, name.sv(), [](Pair const& elem, std::string_view name) {
            return parameter_name_compare(elem.name, name);
        });
        if (it != end && !(name.sv() < it->name)) {
            return it;
        }
        return nullptr;
    }

private:
    std::unique_ptr<Pair[]> values;
    std::unique_ptr<char[]> chars;
    std::size_t len = 0;
};

using headlessclient::ScancodePair;
using headlessclient::KeyLockFlagPair;
using headlessclient::MouseFlagPair;

namespace paramlists
{
    ParameterList<ScancodePair> scancodes_en {headlessclient::names_scancodes_en};
    ParameterList<ScancodePair> scancodes_fr {headlessclient::names_scancodes_fr};
    ParameterList<KeyLockFlagPair> mods {headlessclient::names_lock_flags};
    ParameterList<MouseFlagPair> mouses {headlessclient::names_mouse_flags};
}

template<class T>
bool parse_hexdecimal(OutParam<T> out_param, chars_view str)
{
    auto r = hexadecimal_chars_to_int(str, out_param.out_value);
    return r.ec == std::errc() && r.ptr == str.end();
}

template<class T>
bool parse_decimal(OutParam<T> out_param, chars_view str)
{
    auto r = decimal_chars_to_int(str, out_param.out_value);
    return r.ec == std::errc() && r.ptr == str.end();
}

template<class T>
bool parse_integer(OutParam<T> out_param, chars_view str)
{
    if (str.size() > 2 && str[0] == '0' && (str[1] == 'X' || str[1] == 'x')) {
        return parse_hexdecimal(out_param, str.drop_front(2));
    }

    return parse_decimal(out_param, str);
}

template<class T, class Pair>
bool parse_kw_or_hexadecimal(OutParam<T> out_param, ParameterList<Pair> const& pairs, TaggedUpperStringView upper_str)
{
    auto str = upper_str.chars();
    if (str.size() > 2 && str[0] == '0' && str[1] == 'X') {
        return parse_hexdecimal(out_param, str.drop_front(2));
    }

    // before decimal_chars_to_int because pairs maybe contains a digit value
    if (auto* elem = pairs.find(upper_str)) {
        out_param.out_value = elem->value;
        return true;
    }

    return false;
}

enum class KeyFlags : uint8_t
{
    Release = 0b01,
    Acquire = 0b10,
    AcquireAndRelease = 0b11,
};

bool has_acquire_flag(KeyFlags flags)
{
    return underlying_cast(flags) & underlying_cast(KeyFlags::Acquire);
}

bool has_release_flag(KeyFlags flags)
{
    return underlying_cast(flags) & underlying_cast(KeyFlags::Release);
}

bool parse_key_flag(OutParam<KeyFlags> flag, chars_view str)
{
    if (str.empty()) {
        flag.out_value = KeyFlags::AcquireAndRelease;
        return true;
    }

    auto upper = ascii_to_limited_upper<8>(str);
    auto sv = upper.sv();

    if (sv == "DOWN_UP" || sv == "DOWNUP" || sv == "2" || sv == "DU") {
        flag.out_value = KeyFlags::AcquireAndRelease;
        return true;
    }

    if (sv == "DOWN" || sv == "1" || sv == "D") {
        flag.out_value = KeyFlags::Acquire;
        return true;
    }

    if (sv == "UP" || sv == "0" || sv == "U") {
        flag.out_value = KeyFlags::Release;
        return true;
    }

    return false;
}

bool parse_boolean(OutParam<bool> b, std::string_view str)
{
    if (str.size() == 3 && ascii_to_upper(str[0]) == 'O' && ascii_to_upper(str[1]) == 'F' && ascii_to_upper(str[2]) == 'F') {
        b.out_value = false;
    }
    else if (str.size() == 2 && ascii_to_upper(str[0]) == 'O' && ascii_to_upper(str[1]) == 'N') {
        b.out_value = true;
    }
    else if (str.size() == 1 && str[0] == '1') {
        b.out_value = true;
    }
    else if (str.size() == 1 && str[0] == '0') {
        b.out_value = false;
    }
    else {
        return false;
    }

    return true;
}

struct ValueFlagParam
{
    chars_view value_av;
    chars_view flag_av;

    ValueFlagParam(chars_view str)
    {
        auto first = str.begin();
        auto last = str.end();

        char const* end_value = last;
        char const* start_flag = last;

        for (; first != last; ++first) {
            if (*first == ',') {
                end_value = first;
                start_flag = first+1;
                break;
            }
        }

        value_av = {str.begin(), end_value};
        flag_av = {start_flag, last};
    }

    TaggedUpperStringView assume_upper_value() const
    {
        assert(!ascii_contains_lower(value_av));
        return TaggedUpperStringView{value_av.as<std::string_view>()};
    }
};

struct ScancodeMod
{
    std::string_view name;
    uint16_t mod;
    uint16_t flag;
};

constexpr uint16_t mod_shift_flag  = 0b00000001;
constexpr uint16_t mod_ctrl_flag   = 0b00000010;
constexpr uint16_t mod_meta_flag   = 0b00000100;
constexpr uint16_t mod_alt_flag    = 0b00001000;
constexpr uint16_t mod_altgr_flag  = 0b00010000;
constexpr uint16_t mod_rshift_flag = 0b00100000;
constexpr uint16_t mod_rctrl_flag  = 0b01000000;
constexpr uint16_t mod_rmeta_flag  = 0b10000000;

constexpr std::array scancode_mods = {
    ScancodeMod{"LSHIFT", 0x2A,  mod_shift_flag},
    ScancodeMod{"SHIFT",  0x2A,  mod_shift_flag},
    ScancodeMod{"S",      0x2A,  mod_shift_flag},
    ScancodeMod{"LCTRL",  0x1D,  mod_ctrl_flag},
    ScancodeMod{"CTRL",   0x1D,  mod_ctrl_flag},
    ScancodeMod{"C",      0x1D,  mod_ctrl_flag},
    ScancodeMod{"LMETA",  0x15B, mod_meta_flag},
    ScancodeMod{"LMETA",  0x15B, mod_meta_flag},
    ScancodeMod{"META",   0x15B, mod_meta_flag},
    ScancodeMod{"WIN",    0x15B, mod_meta_flag},
    ScancodeMod{"M",      0x15B, mod_meta_flag},
    ScancodeMod{"W",      0x15B, mod_meta_flag},
    ScancodeMod{"LALT",   0x38,  mod_alt_flag},
    ScancodeMod{"ALT",    0x38,  mod_alt_flag},
    ScancodeMod{"A",      0x38,  mod_alt_flag},
    ScancodeMod{"ALTGR",  0x138, mod_altgr_flag},
    ScancodeMod{"RALT",   0x138, mod_altgr_flag},
    ScancodeMod{"RA",     0x138, mod_altgr_flag},
    ScancodeMod{"G",      0x138, mod_altgr_flag},
    ScancodeMod{"RSHIFT", 0x36,  mod_rshift_flag},
    ScancodeMod{"RS",     0x36,  mod_rshift_flag},
    ScancodeMod{"RCTRL",  0x11D, mod_rctrl_flag},
    ScancodeMod{"RC",     0x11D, mod_rctrl_flag},
    ScancodeMod{"RMETA",  0x15C, mod_rmeta_flag},
    ScancodeMod{"RWIN",   0x15C, mod_rmeta_flag},
    ScancodeMod{"RM",     0x15C, mod_rmeta_flag},
    ScancodeMod{"RW",     0x15C, mod_rmeta_flag},
};

char const* memchr(chars_view str, char c)
{
    return static_cast<char const*>(::memchr(str.data(), c, str.size()));
}

struct ScancodeParser
{
    ParameterList<ScancodePair> const* scancode_list;
    uint16_t scs_and_extended_flags[9];
    uint16_t* scs_end = nullptr;
    KeyFlags flags = KeyFlags::AcquireAndRelease;
    bool pause_sc = false;

    ScancodeParser(ParameterList<ScancodePair> const* scancode_list) noexcept
    : scancode_list(scancode_list)
    {}

    // format: mods* scancode (',' flag?)?
    bool parse(chars_view original_str)
    {
        scs_end = scs_and_extended_flags;

        auto upper = ascii_to_limited_upper<128>(original_str);
        chars_view str = upper.chars();

        // mod parser "Shift+", "Ctrl+", ...
        unsigned modflags = 0;
        while (REDEMPTION_UNLIKELY(str.size() > 2)) {
            char const* p = memchr(str, '+');
            if (!p) {
                break;
            }

            auto modname = array_view{str.data(), p};

            auto apply_mod = [&](auto pred){
                for (auto&& smod : scancode_mods) {
                    if (pred(smod)) {
                        if (!(modflags & smod.flag)) {
                            modflags |= smod.flag;
                            *scs_end++ = smod.mod;
                        }
                        str = {modname.end() + 1, str.end()};
                        return true;
                    }
                }
                return false;
            };

            if (modname.size() > 2 && modname[0] == '0' && modname[1] == 'X') {
                uint16_t mod;
                if (!parse_hexdecimal(OutParam{mod}, modname.drop_front(2))) {
                    return false;
                }

                if (!apply_mod([mod](ScancodeMod smod) { return smod.mod == mod; })) {
                    return false;
                }
            }
            else if (!apply_mod([modname](ScancodeMod smod) { return smod.name == modname.as<std::string_view>(); })) {
                return false;
            }
        }

        // support of ",,flag"
        if (REDEMPTION_UNLIKELY(str[0] == ',')) {
            auto first = str.begin();
            auto last = str.end();

            *scs_end++ = 0x33;
            if (first + 1 != last) {
                if (first[1] != ',') {
                    return false;
                }

                if (!parse_key_flag(OutParam{flags}, {first+2, last})) {
                    return false;
                }
            }
            return true;
        }

        ValueFlagParam value_flag(str);
        auto upper_value = value_flag.assume_upper_value();

        if (value_flag.value_av.size() == 1) {
            std::size_t ipos = checked_int(value_flag.value_av.data() - upper.data());
            // add Shift with upper alpha
            if (ascii_is_upper(original_str[ipos]) && !(modflags & (mod_shift_flag | mod_rshift_flag))) {
                *scs_end++ = 0x2A;
            }
            if (!parse_kw_or_hexadecimal(OutParam{*scs_end++}, *scancode_list, upper_value)) {
                return false;
            }
        }
        else {
            pause_sc = (upper_value == "PAUSE"_ascii_upper);
            if (!pause_sc && !parse_kw_or_hexadecimal(OutParam{*scs_end++}, *scancode_list, upper_value)) {
                return false;
            }
        }

        return parse_key_flag(OutParam{flags}, value_flag.flag_av);
    }
};

struct ModPair
{
    uint16_t mod;
    uint16_t flag;
};

struct KeyParser
{
    ParameterList<ScancodePair> const* scancode_list;
    char const* endp;
    uint16_t scs_and_extended_flags[5];
    uint16_t* scs_end;
    uint16_t repetition = 1;
    KeyFlags flags = KeyFlags::AcquireAndRelease;
    bool pause_sc = false;

    KeyParser(ParameterList<ScancodePair> const* scancode_list) noexcept
    : scancode_list(scancode_list)
    {}

    // format: ('!' | '+' | '^' | '#' | '~')? ( '{' sc (' ' ('up' | 'down' | decimal))? '}' | . )
    bool parse(chars_view str)
    {
        scs_end = scs_and_extended_flags;

        auto first = str.begin();
        auto last = str.end();

        // extract mod keys
        uint16_t modflags = 0;
        for (; first != last; ++first) {
            auto mflag = consume_mod(*first);
            if (!mflag.mod) {
                break;
            }
            if (!(modflags & mflag.flag)) {
                modflags |= mflag.flag;
                *scs_end++ = mflag.mod;
            }
        }

        auto maybe_add_shift = [&](char c){
            if (ascii_is_upper(c) && !(modflags & (mod_shift_flag | mod_rshift_flag))) {
                *scs_end++ = 0x2A;
            }
        };

        if (first == last) {
            if (modflags) {
                endp = first;
                return true;
            }
            return false;
        }

        // long format { ... }
        if (REDEMPTION_UNLIKELY(*first == '{')) {
            char const* p = memchr({first + 1, str.end()}, '}');
            if (!p) {
                return false;
            }

            chars_view keyname;

            char const* optp = memchr({first + 1, p}, ' ');
            if (optp) {
                keyname = {first + 1, optp};

                chars_view option = {optp + 1, p};
                if (!parse_decimal(OutParam{repetition}, option)) {
                    if (!parse_key_flag(OutParam{flags}, option)) {
                        return false;
                    }
                }
            }
            else {
                keyname = {first + 1, p};
            }

            auto upper_value = ascii_to_limited_upper<16>(keyname);
            pause_sc = (upper_value == "PAUSE"_ascii_upper);
            if (!pause_sc) {
                if (keyname.size() == 1) {
                    maybe_add_shift(keyname[0]);
                }
                if (!parse_kw_or_hexadecimal(OutParam{*scs_end++}, *scancode_list, upper_value)) {
                    return false;
                }
            }

            endp = p + 1;
            return true;
        }

        endp = first + 1;

        // simple scancode
        auto upper_value = ascii_to_upper(*first);
        if (auto* elem = scancode_list->find(TaggedUpperStringView{{&upper_value, 1}})) {
            maybe_add_shift(*first);
            *scs_end++ = elem->value;
            return true;
        }

        if (*first == ' ') {
            *scs_end++ = 0x39;
            return true;
        }

        return false;
    }

private:
    static ModPair consume_mod(char c)
    {
        switch (c) {
        case '!': // alt
            return {0x38, mod_alt_flag};

        case '+': // shift
            return ModPair{0x2A, mod_shift_flag};

        case '^': // ctrl
            return ModPair{0x1D, mod_ctrl_flag};

        case '#': // meta
            return ModPair{0x15B, mod_meta_flag};

        case '~': // altgr
            return ModPair{0x138, mod_altgr_flag};

        default:
            return ModPair{};
        }
    }
};

std::array<uint16_t, 2> unicode_to_utf16_with_surrogate(uint32_t uc)
{
    return {
        static_cast<uint16_t>(0b110110'00'00000000 | ((uc - 0x10000) >> 10)),
        static_cast<uint16_t>(0b110111'00'00000000 | (uc & 0b11'11111111)),
    };
}

struct UnicodeParser
{
    std::array<uint16_t, 2> utf16;
    KeyFlags flags = KeyFlags::AcquireAndRelease;

    // format: u32 (',' flag?)?
    bool parse(chars_view str)
    {
        ValueFlagParam value_flag(str);

        uint32_t unicode;
        if (!parse_integer(OutParam{unicode}, value_flag.value_av)) {
            return false;
        }

        if (!parse_key_flag(OutParam{flags}, value_flag.flag_av)) {
            return false;
        }

        if (unicode <= 0xffff) {
            utf16[0] = static_cast<uint16_t>(unicode);
            utf16[1] = 0;
        }
        else {
            utf16 = unicode_to_utf16_with_surrogate(unicode);
        }

        return true;
    }
};

struct MouseParser
{
    uint16_t mouse_flags = 0;
    KeyFlags flags = KeyFlags::AcquireAndRelease;

    // format: button (',' flag?)?
    bool parse(chars_view str)
    {
        auto upper = ascii_to_limited_upper<16>(str);
        ValueFlagParam value_flag(upper.chars());

        if (!parse_kw_or_hexadecimal(OutParam{mouse_flags}, paramlists::mouses, value_flag.assume_upper_value())) {
            return false;
        }

        if (!parse_key_flag(OutParam{flags}, value_flag.flag_av)) {
            return false;
        }

        return true;
    }
};

struct MousePositionParser
{
    int32_t screen_value;
    int32_t mouse_value;

    // format: ('+' | '-')?\d
    bool parse(OutParam<uint16_t> mouse_axis, chars_view str)
    {
        int32_t negate = 1;
        int32_t relative = 0;
        if (str.size() > 1 && (str[0] == '+' || str[0] == '-')) {
            relative = 1;
            negate = (str[0] == '-') ? -1 : 1;
            str = str.drop_front(1);
        }

        uint16_t uvalue;
        if (!parse_integer(OutParam{uvalue}, str)) {
            return false;
        }

        int32_t value = uvalue * negate;
        value = std::min(screen_value, mouse_value * relative + value);

        mouse_axis.out_value = checked_int(std::max(int32_t(), value));

        return true;
    }
};

struct ScrollParser
{
    uint8_t step;
    uint16_t flag = MOUSE_FLAG_WHEEL_NEGATIVE;

    bool parse(chars_view str)
    {
        if (str.front() == '-') {
            str = str.drop_front(1);
            flag = 0xFF;
        }

        return parse_decimal(OutParam{step}, str);
    }
};

struct KbdLockParser
{
    uint8_t locks;

    bool parse(chars_view str)
    {
        if (!parse_kw_or_hexadecimal(OutParam{locks}, paramlists::mods, ascii_to_limited_upper<16>(str))) {
            return parse_decimal(OutParam{locks}, str);
        }
        return true;
    }
};

struct DelayParser
{
    unsigned default_unit_factor;
    std::chrono::milliseconds delay {};

    // (number '.' number | number ('/' number)?) unit?
    bool parse(chars_view str)
    {
        return parse(OutParam{delay}, str);
    }

    bool parse(OutParam<std::chrono::milliseconds> delay, chars_view str)
    {
        unsigned next_factor = (default_unit_factor == 1000u) ? 1 : 0;
        unsigned next_no_unit_factor = default_unit_factor;

        if (str.empty()) {
            return true;
        }

        for (;;) {
            unsigned d1 = 0;
            unsigned d2 = 0;
            bool real = false;
            bool fraction = false;

            auto r = decimal_chars_to_int(str, d1);
            if (r.ec != std::errc()) {
                if (r.ec != std::errc::invalid_argument || str[0] != '.') {
                    return false;
                }
            }

            if (r.ptr != str.end() && (r.ptr[0] == '/' || r.ptr[0] == '.')) {
                auto* ptr = r.ptr + 1;
                auto* end = str.end();

                if (r.ptr[0] == '.') {
                    real = true;
                    // maximum 3 digits
                    if (end - ptr > 3) {
                        end = ptr + 3;
                    }
                }
                else {
                    fraction = true;
                }

                r = decimal_chars_to_int(chars_view(ptr, end), d2);

                if (r.ec != std::errc()) {
                    if (fraction) {
                        return false;
                    }
                    r.ptr = ptr;
                }

                if (real) {
                    // 0.1 => d = 1 -> d = 100
                    if (r.ptr - ptr == 1) {
                        d2 *= 100;
                    }
                    // 0.12 => d = 12 -> d = 120
                    else if (r.ptr - ptr == 2) {
                        d2 *= 10;
                    }
                    // ptr - r.ptr >= 3
                    else {
                        // skip digit greater than 3 significant numbers
                        while (r.ptr != str.end() && is_decimal_char(*r.ptr)) {
                            ++r.ptr;
                        }
                    }
                }
            }

            unsigned factor = next_factor;
            unsigned unit_factor = next_no_unit_factor;
            next_factor = 0;
            next_no_unit_factor = 0;

            if (r.ptr != str.end()) {
                if (r.ptr[0] == 's') {
                    factor = 1;
                    next_factor = 0;
                    unit_factor = 1000;
                    next_no_unit_factor = 1;
                    r.ptr += 1;
                }
                else if (r.ptr[0] == 'm') {
                    // m / min
                    if (str.end() - r.ptr <= 1 || r.ptr[1] != 's') {
                        factor = 600;
                        next_factor = 1000;
                        unit_factor = 60 * 1000;
                        next_no_unit_factor = 1000;
                        bool is_long = (str.end() - r.ptr > 2 && r.ptr[1] == 'i' && r.ptr[2] == 'n');
                        r.ptr += is_long ? 3 : 1;
                    }
                    else {
                        factor = 0;
                        unit_factor = 1;
                        r.ptr += 2;
                    }
                }
                else if (r.ptr[0] == '.') {
                    return false;
                }
            }

            d1 *= unit_factor;

            if (fraction) {
                d1 /= d2;
            }
            else if (real) {
                d1 += d2 * factor;
            }

            delay.out_value += std::chrono::milliseconds(d1);

            if (r.ptr == str.end()) {
                return true;
            }

            str = chars_view(r.ptr, str.end());
        }
    }
};

struct KbdNameParser
{
    bool parse(OutParam<bool> is_en, chars_view str)
    {
        if (str.size() == 2) {
            if ((str[0] == 'e' || str[0] == 'E') && (str[1] == 'n' || str[1] == 'N')) {
                is_en.out_value = true;
                return true;
            }
            if ((str[0] == 'f' || str[0] == 'F') && (str[1] == 'r' || str[1] == 'R')) {
                is_en.out_value = false;
                return true;
            }
        }
        return false;
    }
};

using kbdtypes::KbdFlags;
using kbdtypes::Scancode;

inline Keymap null_keymap(KeyLayout::null_layout());

void send_scancode(RdpInput& mod, uint16_t sc_and_flags)
{
    auto const sc = Scancode(sc_and_flags & 0xFF);
    auto const flags = KbdFlags(sc_and_flags & 0xFF00);
    mod.rdp_input_scancode(flags, sc, 0, null_keymap);
}

void send_pause(RdpInput& mod, KeyFlags flags)
{
    bool const has_acquire = has_acquire_flag(flags);
    bool const has_release = has_release_flag(flags);

    auto const sc1 = Scancode(0x1D);
    auto const sc2 = Scancode(0x45);
    auto const flag1 = KbdFlags::Extended1;
    auto const flag2 = KbdFlags::NoFlags;

    if (has_acquire) {
        mod.rdp_input_scancode(flag1, sc1, 0, null_keymap);
        mod.rdp_input_scancode(flag2, sc2, 0, null_keymap);
    }

    if (has_release) {
        mod.rdp_input_scancode(flag1 | KbdFlags::Release, sc1, 0, null_keymap);
        mod.rdp_input_scancode(flag2 | KbdFlags::Release, sc2, 0, null_keymap);
    }
}

void send_acquire(RdpInput& mod, array_view<uint16_t> scs_and_flags)
{
    for (uint16_t sc_and_flags : scs_and_flags) {
        send_scancode(mod, sc_and_flags);
    }
}

void send_release(RdpInput& mod, array_view<uint16_t> scs_and_flags)
{
    if (!scs_and_flags.empty()) {
        auto it = scs_and_flags.begin();
        auto end = scs_and_flags.end();
        do {
            --end;
            send_scancode(mod, *end | 0x8000);
        } while (end != it);
    }
}

void send_scancode(RdpInput& mod, ScancodeParser const& scs_and_flags)
{
    bool const has_acquire = has_acquire_flag(scs_and_flags.flags);
    bool const has_release = has_release_flag(scs_and_flags.flags);

    array_view av{scs_and_flags.scs_and_extended_flags, scs_and_flags.scs_end};

    if (has_acquire) {
        send_acquire(mod, av);
    }

    if (scs_and_flags.pause_sc) {
        send_pause(mod, scs_and_flags.flags);
    }

    if (has_release) {
        send_release(mod, av);
    }
}

void send_scancode(RdpInput& mod, KeyParser const& scs_and_flags)
{
    bool const has_acquire = has_acquire_flag(scs_and_flags.flags);
    bool const has_release = has_release_flag(scs_and_flags.flags);

    array_view av{scs_and_flags.scs_and_extended_flags, scs_and_flags.scs_end};

    if (has_acquire) {
        send_acquire(mod, av);

        // send repetition
        if (!av.empty() && has_release) {
            uint16_t sc_and_flags = *(av.end() - 1);
            for (uint16_t rep = 1; rep < scs_and_flags.repetition; ++rep) {
                send_scancode(mod, sc_and_flags);
            }
        }
    }

    if (scs_and_flags.pause_sc) {
        send_pause(mod, scs_and_flags.flags);
    }

    if (has_release) {
        send_release(mod, av);
    }
}

void send_unicode(RdpInput& mod, UnicodeParser unicode)
{
    using kbdtypes::KbdFlags;

    auto send = [&](KbdFlags flag){
        mod.rdp_input_unicode(flag, unicode.utf16[0]);
        if (unicode.utf16[1]) {
            mod.rdp_input_unicode(flag, unicode.utf16[1]);
        }
    };

    if (has_acquire_flag(unicode.flags)) {
        send(KbdFlags::NoFlags);
    }

    if (has_release_flag(unicode.flags)) {
        send(KbdFlags::Release);
    }
}

void send_text(RdpInput& mod, chars_view text)
{
    uint8_t const* first = byte_ptr_cast(text.begin());
    uint8_t const* last = byte_ptr_cast(text.end());

    using kbdtypes::KbdFlags;

    auto send_unicode1 = [&](uint16_t utf16){
        mod.rdp_input_unicode(KbdFlags::NoFlags, utf16);
        mod.rdp_input_unicode(KbdFlags::Release, utf16);
    };

    auto send_unicode2 = [&](uint32_t uc){
        auto utf16_pair = unicode_to_utf16_with_surrogate(uc);
        mod.rdp_input_unicode(KbdFlags::NoFlags, utf16_pair[0]);
        mod.rdp_input_unicode(KbdFlags::NoFlags, utf16_pair[1]);
        mod.rdp_input_unicode(KbdFlags::Release, utf16_pair[0]);
        mod.rdp_input_unicode(KbdFlags::Release, utf16_pair[1]);
    };

    while (first < last) {
        switch (*first >> 4) {
        [[likely]]
        case 0:
        case 1: case 2: case 3:
        case 4: case 5: case 6: case 7:
            send_unicode1(*first);
            first += 1;
            break;

        /* handle U+0080..U+07FF inline : 2 bytes sequences */
        case 0xC: case 0xD:
            if (REDEMPTION_UNLIKELY(last - first < 2)) {
                return;
            }
            send_unicode1(checked_int(utf8_2_bytes_to_ucs(first[0], first[1])));
            first += 2;
            break;

        /* handle U+8FFF..U+FFFF inline : 3 bytes sequences */
        case 0xE:
            if (REDEMPTION_UNLIKELY(last - first < 3)) {
                return;
            }
            send_unicode2(utf8_3_bytes_to_ucs(first[0], first[1], first[2]));
            first += 3;
            break;

        case 0xF:
            if (REDEMPTION_UNLIKELY(last - first < 4)) {
                return;
            }
            send_unicode2(utf8_4_bytes_to_ucs(first[0], first[1], first[2], first[3]));
            first += 4;
            break;

        // these should never happen on valid UTF8
        case 8: case 9: case 0x0A: case 0x0B:
            first += 1;
            break;
        }
    }
}

void append(std::vector<char>& out, chars_view s)
{
    out.insert(out.end(), s.begin(), s.end());
}

template<class Seq>
chars_view populate_cache(std::vector<char>& out, Seq const& seq)
{
    if (!out.empty()) {
        return chars_view(out);
    }

    std::size_t max_len = 0;
    for (auto const& pair : seq) {
        max_len = std::max(pair.name.size(), max_len);
    }
    for (auto const& pair : seq) {
        append(out, pair.name);
        out.insert(out.end(), max_len - pair.name.size() + 1, ' ');
        append(out, "0x"_av);
        append(out, int_to_hexadecimal_upper_chars(pair.value));
        out.push_back('\n');
    }
    return chars_view(out);
}

constexpr auto physical_layout = R"(
Scancodes physical layout reference (hexadecimal number):

+----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +-----+----+-------+
| 01 |  | 3B | 3C | 3D | 3E |  | 3F | 40 | 41 | 42 |  | 43 | 44 | 57 | 58 |  | 137 | 46 | 1D+45 |
+----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +-----+----+-------+
+----+----+----+----+----+----+----+----+----+----+----+----+----+--------+  +----+----+----+  +--------------------+
| 29 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 0A | 0B | 0C | 0D |   0E   |  | 152| 147| 149|  | 45 | 135| 37 | 4A  |
+-------------------------------------------------------------------------+  +----+----+----+  +----+----+----+-----+
|  0F  | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 1A | 1B |      |  | 153| 14F| 151|  | 47 | 48 | 49 |     |
+------------------------------------------------------------------+  1C  |  +----+----+----+  +----+----+----| 4E  |
|  3A   | 1E | 1F | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 2B |     |                    | 4B | 4C | 4D |     |
+-------------------------------------------------------------------------+       +----+       +----+----+----+-----+
|  2A | 56 | 2C | 2D | 2E | 2F | 30 | 31 | 32 | 33 | 34 | 35 |     36     |       | 148|       | 4F | 50 | 51 |     |
+-------------------------------------------------------------------------+  +----+----+----+  +---------+----| 11C |
|  1D  |  15B | 38 |           39           |  138  |  15C |  15D |  11D  |  | 14B| 150| 14D|  |    52   | 53 |     |
+------+------+----+------------------------+-------+------+------+-------+  +----+----+----+  +---------+----+-----+
)"_av;

constexpr auto named_physical_layout_en = "\nScancode names for \x1b[1mEN\x1b[m layout:" R"(

+-----+  +----+----+----+----+  +----+----+----+----+  +----+-----+-----+-----+  +--------+--------+-------+
| esc |  | F1 | F2 | F3 | F4 |  | F5 | F6 | F7 | F8 |  | F9 | F10 | F11 | F12 |  | screen | scroll | pause |
+-----+  +----+----+----+----+  +----+----+----+----+  +----+-----+-----+-----+  +--------+--------+-------+
+---+---+---+---+---+---+---+---+---+---+---+---+---+-----------+  +--------+------+--------+  +----------------------------+
| ` | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 0 | - | = | backspace |  | insert | home |  pgup  |  | numlock | n/ | n* | n-     |
+---------------------------------------------------------------+  +--------+------+--------+  +---------+----+----+--------+
|  tab  | q | w | e | r | t | y | u | i | o | p | [ | ] | enter |  | delete | end  | pgdown |  |      n7 | n8 | n9 |        |
+-------------------------------------------------------+--+    |  +--------+------+--------+  +---------+----+----| n+     |
| capsLock | a | s | d | f | g | h | j | k | l | ; | ' | \ |    |                              |      n4 | n5 | n6 |        |
+---------------------------------------------------------------+           +------+           +---------+----+----+--------+
| shift | | | z | x | c | v | b | n | m | , | . | / |  rshift   |           |  up  |           |      n1 | n2 | n3 |        |
+---------------------------------------------------------------+    +------+------+-------+   +--------------+----| nenter |
| ctrl | win | alt |     space    | altgr | rwin | menu | rctrl |    | left | down | right |   |      n0      | n. |        |
+------+------+----+--------------+-------+------+------+-------+    +------+------+-------+   +--------------+----+--------+
)"_av;

constexpr auto named_physical_layout_fr = "\nScancode names for \x1b[1mFR\x1b[m layout:" R"(

+-----+  +----+----+----+----+  +----+----+----+----+  +----+-----+-----+-----+  +--------+--------+-------+
| esc |  | F1 | F2 | F3 | F4 |  | F5 | F6 | F7 | F8 |  | F9 | F10 | F11 | F12 |  | screen | scroll | pause |
+-----+  +----+----+----+----+  +----+----+----+----+  +----+-----+-----+-----+  +--------+--------+-------+
+---+---+---+---+---+---+---+---+---+---+---+---+---+-----------+  +--------+------+--------+  +----------------------------+
| ² | & | é | " | ' | ( | - | è | _ | ç | à | ) | = | backspace |  | insert | home |  pgup  |  | numlock | n/ | n* | n-     |
+---------------------------------------------------------------+  +--------+------+--------+  +---------+----+----+--------+
|  tab  | a | z | e | r | t | y | u | i | o | p | ^ | $ | enter |  | delete | end  | pgdown |  |      n7 | n8 | n9 |        |
+-------------------------------------------------------+--+    |  +--------+------+--------+  +---------+----+----| n+     |
| capsLock | q | s | d | f | g | h | j | k | l | m | ù | * |    |                              |      n4 | n5 | n6 |        |
+---------------------------------------------------------------+           +------+           +---------+----+----+--------+
| shift | > | w | x | c | v | b | n | , | ; | : | ! |  rshift   |           |  up  |           |      n1 | n2 | n3 |        |
+---------------------------------------------------------------+    +------+------+-------+   +--------------+----| nenter |
| ctrl | win | alt |     space    | altgr | rwin | menu | rctrl |    | left | down | right |   |      n0      | n. |        |
+------+------+----+--------------+-------+------+------+-------+    +------+------+-------+   +--------------+----+--------+
)"_av;


constexpr auto help_commands = R"(Commands:

N(scancode) [P(scancode_and_flag)...]
alias: sc

    Send a scancode sequence.

    Format:
        scancode_and_flag: [P(mod)V(+)]P(sc)[V(,)P(flag)]

        - P(mod) is a modifier key (Ctrl, Shift, etc) in hexadecimal format (V(0x...)) or insensitive named key.
        - P(sc) is a RDP scancode in hexadecimal format (V(0x...)) or insensitive named key.
        - P(flag) is V(0)/V(up), V(1)/V(down) or V(2)/V(downUp) (default=V(2)).

    Note: when P(sc) is an alpha uppercase it is equivalent to Shift+key:
        V(Shift+a) == V(S+a) == V(A)

    Use C(help mod) for listed mod name.
    Use C(help sc) for listed key name.
    Use C(help flag) for listed flag name.


N(key) [P(key)...]
alias: k

    Send a scancode sequence.

    Format:
        key: [P(mod)...]P(sc) or [P(mod)...]V({)P(scname)[ P(option)]V(})

        - mod are:
            V(!) for Alt
            V(+) for Shift
            V(^) for Ctrl
            V(#) for Meta / Window
            V(~) for AltGr
        - P(sc) single caracter mapped to scancode
        - P(scname) is named scancode
        - P(option) is V(up), V(down), V(downUp) or a repetition number

    Note: when P(sc) is an alpha uppercase it is equivalent to Shift+key:
        V(+a) == V(A)

    Example:
        C(key {q 3}) send q 3 times ; equivalent to C(sc q,down q,down q,down q,up)
        C(key Hello) is equivalent to C(sc S+h e l l o)


N(unicode) [P(unicode_and_flag)...]
alias: uc

    Send a unicode sequence.

    Format:
        unicode_and_flag: P(uc)[V(,)P(flag)]

        - P(uc) is positive number in hexadecimal (V(0x...)) or decimal format.
        - P(flag) is V(0)/V(up), V(1)/V(down) or V(2)/V(downUp) (default=V(2)).


N(text) [P(text)...]
alias: t

    Send a unicode text.

    C(t x, y or z) is equivalent to C(uc x , space y space o r space z)
    C(t  x) is equivalent to C(uc space x)


N(textln) [P(text)...]
alias: tln

    Send a unicode text with newline (see N(textln)).


N(mouse) [P(event_and_flag)...]
alias: m

    Send a mouse event.

    Format:
        event_and_flag: P(mevent)[V(,)P(flag)]

        - P(mevent) is a hexadecimal number (V(0x...)) or insensitive named event.
        - P(flag) is V(0)/V(up), V(1)/V(down) or V(2)/V(downUp) (default=V(2)).

    Use C(help mevent) for listed mevent name.
    Use C(help flag) for listed flag name.


N(move) <P(x)> <P(y)>
alias: mv

    Move the mouse.

    Format:
        [P(relative)]P(integer)

        - P(relative) is V(+) or V(-)


N(scroll) P(step)
N(hscroll) P(step)

    Vertical scroll and horizontal scroll.

    Format:
        - P(step) is negative or positive decimal number.

    C(scroll 3) is equivalent to C(m scroll scroll scroll)


N(lock) [P(lock)...]

    Synchronize keymap modificator (CapsLock, NumLock, etc)

    P(lock) is V(scroll), V(num), V(caps), V(kana) or a hexadecimal number (V(0x...)).

    C(lock none), C(lock 0) or C(lock) reset all modificators.


N(kbd) {P(en)|P(fr)}

    Select a scancode name mapping for scancode command.

    Use C(help kbd) for listed mapping.


N(keydelay) [P(delay)...]
N(mousedelay) [P(delay)...]

    Set a delay between two keyboard or mouse event.
    Keyboard events are N(scancode), N(unicode), N(key), N(text), N(textln) and N(lock).
    Mouse events are N(mouse), N(move), N(scroll) and N(hscroll).

    See C(help delay) for format (default unit is milliseconds).


N(sleep) [P(delay)...]

    Add a delay before executing the next command.

    See C(help delay) for format (default unit is seconds).


N(username) P(username)
alias: user

    Set username.


N(password) P(password)
alias: pwd and pass

    Set password.


N(rdp) [P(address) [P(port)]]
alias: connect and co

    Connect or reconnect to RDP target. Default port is V(3389).


N(vnc) P(address) [P(port)]
alias: connect and co

    Connect or reconnect to VNC target. Default port is V(5900).


N(disconnect)
alias: disco


N(reconnect)
alias: reco


N(wrm) [P(boolean)] [P(path)]

    Enable or disable wrm capture.
    When boolean is not specified, only the wrm path is modified.


N(record-transport) [P(boolean)] [P(path)]

    Enable or disable transport recording.


N(png) [P(path)]
alias: p

    Set png filename and save screen to png file.


N(ipng) P(delay) [P(suffix-name)]
alias: pp

    On each P(delay), save png with a incremental number in P(directory) (specified with C(ipng-dir)).
    Stoped when P(delay) is 0.

    See C(help delay) for format (default unit is seconds).


N(ipng-directory) P(directory)
alias: ipng-dir and ppd

    Reset counter of C(ipng) and set directory for C(ipng) and C(png) commands.


N(enable-png) [P(bool)]

    Enable or disable C(png) and C(ipng).


N(basename) [P(filename)]
alias: sid

    Basename for N(wrm), N(record-transport) and N(png) when the name is never specified.


N(directory) [P(directory)]
alias: sid

    Directory output for N(wrm), N(record-transport) and N(png).


N(configfile) [P(filename)]
alias: conff and f

    Read a config file (.ini).


N(configstr) P(str)
alias: confstr and conf

    Read a config as a string.


N(help) [P(param-name)]
alias: h and ?

    List named value.


N(repeat) P(delay) [P(number_or_repetitions)] P(cmd)

    Repeat a command with a delay.
    Stop timer when P(delay) is 0.
    Infinite loop when N(number_or_repeatitions) is negative.

    See C(help delay) for format (default unit is seconds).


N(quit)
alias: q

    Exit.


N(#)

    A comment, is ignored.
)"
    ""_av;

constexpr auto help_delay = R"(Delay format:
delay: P(number)[P(unit)]
number: P([0-9]) | [P([0-9])].P([0-9]) | P([0-9])/P([0-9])

- unit are:
    V(s) for seconds
    V(ms) for milliseconds
    V(m) or V(min) for minutes

Exemple:
    C(sleep 1/4h) == C(sleep 0.25h) == C(sleep 15min) == C(sleep 900)
    C(sleep 1min3s) == C(sleep 1min 3s) == C(sleep 63s)
)"_av;

constexpr std::size_t count_help_replacement(chars_view str)
{
    std::size_t n = 0;

    auto first = str.begin();
    auto last = str.end();

    for (; first != last; ++first) {
        if (*first == '(') {
            char c = *(first-1);
            if (c == 'P' || c == 'V' || c == 'C' || c == 'N') {
                ++n;
            }
        }
    }

    return n;
}

template<std::size_t nrep, std::size_t len>
constexpr auto make_colorized_string(chars_view string)
{
    std::array<char, len + nrep * 8 + 1> colorized_help {};
    char* p = colorized_help.data();

    auto first = string.begin();
    auto last = string.end();

    auto push_color = [&](char const* styles) {
        // skip (
        ++first;
        --p;
        *p++ = '\x1b';
        *p++ = '[';
        while (*styles) {
            *p++ = *styles++;
        }
        *p++ = 'm';
        while (*first != ')') {
            *p++ = *first++;
        }
        *p++ = '\x1b';
        *p++ = '[';
        *p++ = 'm';
    };

    for (; first != last; ++first) {
        if (*first == '(') {
            char c = *(first-1);
            if (c == 'P') {
                push_color("32");
            }
            else if (c == 'V') {
                push_color("33");
            }
            else if (c == 'C') {
                push_color("34");
            }
            else if (c == 'N') {
                push_color("31");
            }
            else {
                *p++ = *first;
            }
        }
        else {
            *p++ = *first;
        }
    }

    return colorized_help;
}

constexpr auto colorized_help = make_colorized_string<
    count_help_replacement(help_commands), help_commands.size()
>(help_commands);

constexpr auto colorized_help_delay = make_colorized_string<
    count_help_replacement(help_delay), help_delay.size()
>(help_delay);

std::vector<char> help_sc_param_en;
std::vector<char> help_sc_param_fr;
std::vector<char> help_kbd_param_en;
std::vector<char> help_kbd_param_fr;
std::vector<char> help_mevent_param;
std::vector<char> help_lock_param;

chars_view cmd_help_sc(std::vector<char>& out, chars_view named_physical_layout, array_view<ScancodePair> names_scancodes)
{
    if (out.empty()) {
        populate_cache(out, names_scancodes);
        append(out, physical_layout);
        append(out, named_physical_layout);
    }
    return out;
}

chars_view cmd_help_kbd(std::vector<char>& out, chars_view named_physical_layout)
{
    if (out.empty()) {
        append(out, physical_layout);
        append(out, named_physical_layout);
    }
    return out;
}

chars_view cmd_help(std::string_view name, bool is_kbdmap_en)
{
    if (name == "scancode" || name == "sc") {
        if (is_kbdmap_en) {
            return cmd_help_sc(
                help_sc_param_en,
                named_physical_layout_en,
                array_view{headlessclient::names_scancodes_en}
            );
        }
        else {
            return cmd_help_sc(
                help_sc_param_fr,
                named_physical_layout_fr,
                array_view{headlessclient::names_scancodes_fr}
            );
        }
    }

    if (name == "mevent" || name == "event") {
        return populate_cache(help_mevent_param, headlessclient::names_mouse_flags);
    }

    if (name == "flag") {
        return
            "        2\n"
            "down_up 2\n"
            "du      2\n"
            "down    1\n"
            "d       1\n"
            "up      0\n"
            "u       0"
            ""_av;
    }

    if (name == "mod") {
        return
            "Shift          | S        0x2A\n"
            "Ctrl           | C        0x1D\n"
            "Meta   | Win   | M  | W   0x15B\n"
            "Alt            | A        0x38\n"
            "RAlt   | AltGr | G        0x138\n"
            "RShift         | RS       0x36\n"
            "RCtrl          | RC       0x11D\n"
            "RMeta  | RWin  | RM | RW  0x15C"
            ""_av;
    }

    if (name == "kbd") {
        if (is_kbdmap_en) {
            return cmd_help_kbd(help_kbd_param_en, named_physical_layout_en);
        }
        else {
            return cmd_help_kbd(help_kbd_param_fr, named_physical_layout_fr);
        }
    }

    if (name == "lock") {
        return populate_cache(help_lock_param, headlessclient::names_lock_flags);
    }

    if (name == "bool" || name == "boolean") {
        return
            "off 0\n"
            "on  1"
            ""_av;
    }

    if (name == "delay") {
        return chars_view{colorized_help_delay};
    }

    return {};
}

HeadlessCommand::Result set_param_error(
    HeadlessCommand& cmd, HeadlessCommand::ErrorType error_type,
    unsigned index, chars_view param, chars_view expected_arg)
{
    cmd.error_type = error_type;
    cmd.index_param_error = index;
    cmd.output_message = param;
    cmd.expected_arg = expected_arg;
    return HeadlessCommand::Result::Fail;
}

} // anonymous namespace


chars_view HeadlessCommand::help_all() const
{
    return colorized_help;
}

chars_view HeadlessCommand::help(chars_view cmd) const
{
    if (cmd.empty()) {
        return colorized_help;
    }

    return cmd_help(cmd.as<std::string_view>(), is_kbdmap_en);
}

namespace
{

template<class T>
struct DecimalParser
{
    bool parse(OutParam<T> n, chars_view str)
    {
        return parse_decimal(n, str);
    }
};

struct BoolParser
{
    bool parse(OutParam<bool> b, chars_view str)
    {
        return parse_boolean(b, str.as<std::string_view>());
    }
};

struct CharsParser
{
    bool parse(OutParam<chars_view> s, chars_view str)
    {
        s.out_value = str;
        return true;
    }
};


REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wunused-template")
template<class T, class U>
U get_output_type(bool(T::*)(OutParam<U>, chars_view));
REDEMPTION_DIAGNOSTIC_POP()

template<class Parser>
using get_output_type_t = decltype(get_output_type(&Parser::parse));

template<class T>
using arg_value_t = std::conditional_t<std::is_same_v<T, std::string>, chars_view, T>;

template<class Parser>
struct CmdArgParser
{
    static constexpr bool is_optional = false;
    static constexpr bool remaining_parser = false;

    using value_type = get_output_type_t<Parser>;

    Parser parser;
    OutParam<value_type> out_param;
    chars_view expected_arg;
    arg_value_t<value_type> value {};

    void move_to_out_param()
    {
        if constexpr (std::is_same_v<value_type, std::string>) {
            out_param.out_value = value.template as<std::string_view>();
        }
        else {
            out_param.out_value = std::move(value);
        }
    }
};

template<class Parser, class... Ts>
CmdArgParser(Parser, Ts&&...) -> CmdArgParser<Parser>;

template<class Parser>
struct CmdArgOptionalParser : CmdArgParser<Parser>
{
    static constexpr bool is_optional = true;
};

template<class Parser, class... Ts>
CmdArgOptionalParser(Parser, Ts&&...) -> CmdArgOptionalParser<Parser>;

struct CmdArgRemaining
{
    static constexpr bool is_optional = false;
    static constexpr bool remaining_parser = true;

    OutParam<chars_view> out_param;
    chars_view expected_arg;
    chars_view value = ""_av;

    void move_to_out_param()
    {
        out_param.out_value = value;
    }
};

struct CmdArgOptionalRemaining : CmdArgRemaining
{
    static constexpr bool is_optional = true;
};

template<class T>
CmdArgOptionalParser<DecimalParser<T>> optional_decimal_parser(OutParam<T> n, chars_view expected_arg, T default_value = {})
{
    return {DecimalParser<T>(), n, expected_arg, default_value};
}

template<class It, class Parser, class... Parsers>
bool cmd_parse_impl(
    OutParam<HeadlessCommand::Result> result,
    HeadlessCommand& cmd,
    unsigned int index_param,
    It& first, It& last, char const* cmd_end,
    Parser& parser, Parsers&... parsers)
{
    if (first == last) {
        if constexpr (Parser::is_optional) {
            return true;
        }
        else {
            result.out_value = set_param_error(
                cmd, HeadlessCommand::ErrorType::MissingArgument,
                index_param, "missing parameter"_av, parser.expected_arg
            );
            return false;
        }
    }

    if constexpr (Parser::remaining_parser) {
        static_assert(Parser::remaining_parser && !sizeof...(parsers), "remaining parser followed with parser");
        parser.value = {first->data(), cmd_end};
        return true;
    }
    else {
        if (!parser.parser.parse(OutParam{parser.value}, *first)) {
            if constexpr (!Parser::is_optional || !sizeof...(parsers)) {
                result.out_value = set_param_error(
                    cmd, HeadlessCommand::ErrorType::InvalidFormat,
                    index_param, *first, parser.expected_arg
                );
                return false;
            }
            else {
                return cmd_parse_impl(result, cmd, index_param, first, last, cmd_end, parsers...);
            }
        }

        ++first;
        ++index_param;
        if constexpr (sizeof...(parsers)) {
            return cmd_parse_impl(result, cmd, index_param, first, last, cmd_end, parsers...);
        }
        else if (first == last) {
            return true;
        }
        else {
            result.out_value = set_param_error(
                cmd, HeadlessCommand::ErrorType::TooManyArgument,
                index_param, *first, ""_av
            );
            return false;
        }
    }
}

template<class It, class... Parsers>
HeadlessCommand::Result cmd_parse(
    HeadlessCommand::Result result,
    HeadlessCommand& cmd,
    unsigned int index_param,
    It& first, It& last, char const* cmd_end,
    Parsers&&... parsers)
{
    ++first;
    ++index_param;
    if (cmd_parse_impl(OutParam{result}, cmd, index_param, first, last, cmd_end, parsers...)) {
        (parsers.move_to_out_param(), ...);
    }
    return result;
}

}

HeadlessCommand::Result HeadlessCommand::execute_command(chars_view cmd, RdpInput& mod)
{
    using namespace std::string_view_literals;

    using kbdtypes::KbdFlags;
    using kbdtypes::Scancode;

    // skip left whitespaces
    while (!cmd.empty() && (cmd.front() == ' ' || cmd.front() == '\t')) {
        cmd = cmd.drop_front(1);
    }

    // empty line or commentary
    if (cmd.empty() || cmd.front() == '#') {
        return Result::Ok;
    }

    auto splitter = split_with(cmd, ' ');
    auto first = splitter.begin();
    auto last = splitter.end();

    unsigned index_param = 0;

    auto parse_sequence = [&](const auto parser, chars_view expected_arg, auto fn){
        while (++first != last) {
            // ignore multi spaces
            if (first->empty()) {
                continue;
            }

            ++index_param;
            auto cp_parser = parser;
            if (!cp_parser.parse(*first)) {
                return set_param_error(*this, ErrorType::InvalidFormat, index_param, *first, expected_arg);
            }

            fn(cp_parser);
        }
        return Result::Ok;
    };

    auto scroll_sequence = [&](uint16_t flag){
        int nseq = 0;
        auto res = parse_sequence(ScrollParser(), "step"_av, [&](ScrollParser scroll) {
            ++nseq;
            uint16_t flags = flag | scroll.flag;
            for (uint8_t i = 0; i < scroll.step; ++i) {
                mod.rdp_input_mouse(flags, mouse_x, mouse_y);
            }
        });
        if (res == Result::Ok && nseq == 0) {
            mod.rdp_input_mouse(flag | ScrollParser().flag, mouse_x, mouse_y);
        }
        return res;
    };

    auto too_many_argument_or = [&](Result result){
        if (++first != last) {
            return set_param_error(*this, ErrorType::TooManyArgument, index_param + 1, *first, ""_av);
        }

        return result;
    };

    auto cmd_name = first->as<std::string_view>();

    if (cmd_name == "sc" || cmd_name == "scancode") {
        auto* names_scancodes = is_kbdmap_en ? &paramlists::scancodes_en : &paramlists::scancodes_fr;
        return parse_sequence(ScancodeParser{names_scancodes}, "scancode"_av, [&](ScancodeParser const& sc_and_flags) {
            send_scancode(mod, sc_and_flags);
        });
    }

    else if (cmd_name == "k" || cmd_name == "key") {
        if (cmd.size() != cmd_name.size()) {
            auto* names_scancodes = is_kbdmap_en ? &paramlists::scancodes_en : &paramlists::scancodes_fr;
            std::size_t ipos = + cmd_name.size() + 1u;
            char const* start = cmd.begin() + ipos;
            char const* p = start;
            while (p != cmd.end()) {
                chars_view remaining {p, cmd.end()};
                KeyParser parser{names_scancodes};
                if (!parser.parse(remaining)) {
                    return set_param_error(*this, ErrorType::InvalidFormat, checked_int(p - start), remaining, "key"_av);
                }
                send_scancode(mod, parser);
                p = parser.endp;
            }
        }
        return Result::Ok;
    }

    else if (cmd_name == "m" || cmd_name == "mouse") {
        return parse_sequence(MouseParser(), "mouse flags"_av, [&](MouseParser mouse) {
            if (has_acquire_flag(mouse.flags)) {
                mod.rdp_input_mouse(mouse.mouse_flags | MOUSE_FLAG_DOWN, mouse_x, mouse_y);
            }

            if (has_release_flag(mouse.flags)) {
                mod.rdp_input_mouse(mouse.mouse_flags, mouse_x, mouse_y);
            }
        });
    }

    else if (cmd_name == "mv" || cmd_name == "move") {
        auto res = cmd_parse(Result::Ok, *this, index_param, first, last, cmd.end(),
            CmdArgParser{MousePositionParser{screen_width, mouse_x}, OutParam{mouse_x}, "x position"_av},
            CmdArgParser{MousePositionParser{screen_height, mouse_y}, OutParam{mouse_y}, "y position"_av}
        );
        if (res == Result::Ok) {
            mod.rdp_input_mouse(MOUSE_FLAG_MOVE, mouse_x, mouse_y);
        }
        return res;
    }

    else if (cmd_name == "t" || cmd_name == "text") {
        if (cmd.size() != cmd_name.size()) {
            send_text(mod, {cmd.begin() + cmd_name.size() + 1, cmd.end()});
        }
        return Result::Ok;
    }

    else if (cmd_name == "tln" || cmd_name == "textln") {
        if (cmd.size() != cmd_name.size()) {
            send_text(mod, {cmd.begin() + cmd_name.size() + 1, cmd.end()});
        }
        mod.rdp_input_unicode(KbdFlags::NoFlags, '\n');
        mod.rdp_input_unicode(KbdFlags::Release, '\n');
        return Result::Ok;
    }

    else if (cmd_name == "uc" || cmd_name == "unicode") {
        return parse_sequence(UnicodeParser(), "unicode char or hexadecimal code"_av, [&](UnicodeParser unicode) {
            send_unicode(mod, unicode);
        });
    }

    else if (cmd_name == "sleep" || cmd_name == "keydelay" || cmd_name == "mousedelay") {
        delay = {};
        unsigned unit = cmd_name == "sleep" ? 1000u : 1u;
        auto res = parse_sequence(DelayParser{unit}, "delay"_av, [&](DelayParser parser) {
            delay += parser.delay;
        });
        if (res == Result::Ok) {
            // sleep command
            if (cmd_name[0] == 's') {
                return delay.count() <= 0 ? res : Result::Sleep;
            }
            // keydelay or mousedelay command
            return (cmd_name[0] == 'k') ? Result::KeyDelay : Result::MouseDelay;
        }
        return res;
    }

    else if (cmd_name == "scroll") {
        return scroll_sequence(MOUSE_FLAG_WHEEL);
    }

    else if (cmd_name == "hscroll") {
        return scroll_sequence(MOUSE_FLAG_HWHEEL);
    }

    else if (cmd_name == "lock") {
        uint8_t locks = 0;
        auto result = parse_sequence(KbdLockParser(), "lock flags"_av, [&](KbdLockParser sync) {
            locks |= sync.locks;
        });
        if (result == Result::Ok) {
            mod.rdp_input_synchronize(kbdtypes::KeyLocks(locks));
        }
        return result;
    }

    else if (cmd_name == "h" || cmd_name == "?" || cmd_name == "help") {
        if (++first == last) {
            output_message = colorized_help;
            return Result::OutputResult;
        }

        output_message = cmd_help(first->as<std::string_view>(), is_kbdmap_en);
        return output_message.data() ? Result::OutputResult : Result::Fail;
    }

    else if (cmd_name == "kbd") {
        return cmd_parse(Result::Ok, *this, index_param, first, last, cmd.end(),
            CmdArgParser{KbdNameParser{}, OutParam{is_kbdmap_en}, "keyboard name"_av}
        );
    }

    else if (cmd_name == "co" || cmd_name == "connect" || cmd_name == "rdp" || cmd_name == "vnc") {
        auto default_port = (cmd_name == "rdp") ? 3389u : (cmd_name == "vnc") ? 5900u : port;
        return cmd_parse(Result::Connect, *this, index_param, first, last, cmd.end(),
            CmdArgOptionalParser{CharsParser{}, OutParam{output_message}, "address"_av},
            optional_decimal_parser(OutParam{port}, "port"_av, default_port)
        );
    }

    else if (cmd_name == "disco" || cmd_name == "disconnect") {
        return too_many_argument_or(Result::Disconnect);
    }

    else if (cmd_name == "reco" || cmd_name == "reconnect") {
        return too_many_argument_or(Result::Reconnect);
    }

    else if (cmd_name == "wrm") {
        return cmd_parse(Result::WrmPath, *this, index_param, first, last, cmd.end(),
            CmdArgOptionalParser{BoolParser{}, OutParam{output_bool}, "boolean"_av, true},
            CmdArgOptionalRemaining{OutParam{output_message}, "filepath"_av}
        );
    }

    else if (cmd_name == "record-transport") {
        return cmd_parse(Result::RecordTarnsportPath, *this, index_param, first, last, cmd.end(),
            CmdArgOptionalParser{BoolParser{}, OutParam{output_bool}, "boolean"_av, true},
            CmdArgOptionalRemaining{OutParam{output_message}, "filepath"_av}
        );
    }

    else if (cmd_name == "p" || cmd_name == "png") {
        return cmd_parse(Result::Screen, *this, index_param, first, last, cmd.end(),
            CmdArgOptionalRemaining{OutParam{output_message}, "filepath"_av}
        );
    }

    else if (cmd_name == "pp" || cmd_name == "ipng") {
        return cmd_parse(Result::ScreenRepetition, *this, index_param, first, last, cmd.end(),
            CmdArgParser{DelayParser{}, OutParam{delay}, "delay"_av},
            CmdArgOptionalRemaining{OutParam{output_message}, "filepath"_av}
        );
    }

    else if (cmd_name == "ppd" || cmd_name == "ipng-dir" || cmd_name == "ipng-directory") {
        return cmd_parse(Result::ScreenDirectory, *this, index_param, first, last, cmd.end(),
            CmdArgParser{CharsParser{}, OutParam{output_message}, "dirname"_av}
        );
    }

    else if (cmd_name == "enable-png") {
        return cmd_parse(Result::EnableScreen, *this, index_param, first, last, cmd.end(),
            CmdArgOptionalParser{BoolParser{}, OutParam{output_bool}, "boolean"_av, true}
        );
    }

    else if (cmd_name == "user" || cmd_name == "username") {
        return cmd_parse(Result::Username, *this, index_param, first, last, cmd.end(),
            CmdArgOptionalRemaining{OutParam{output_message}, "username"_av}
        );
    }

    else if (cmd_name == "pass" || cmd_name == "password") {
        return cmd_parse(Result::Password, *this, index_param, first, last, cmd.end(),
            CmdArgOptionalRemaining{OutParam{output_message}, "password"_av}
        );
    }

    else if (cmd_name == "sid" || cmd_name == "basename") {
        return cmd_parse(Result::Basename, *this, index_param, first, last, cmd.end(),
            CmdArgOptionalRemaining{OutParam{output_message}, "basename"_av}
        );
    }

    else if (cmd_name == "dir" || cmd_name == "directory") {
        return cmd_parse(Result::Directory, *this, index_param, first, last, cmd.end(),
            CmdArgOptionalRemaining{OutParam{output_message}, "directory"_av}
        );
    }

    else if (cmd_name == "f" || cmd_name == "conff" || cmd_name == "configfile") {
        return cmd_parse(Result::ConfigFile, *this, index_param, first, last, cmd.end(),
            CmdArgRemaining{OutParam{output_message}, "config filename"_av}
        );
    }

    else if (cmd_name == "conf" || cmd_name == "confstr" || cmd_name == "configstr") {
        output_message = cmd.drop_front(std::min(cmd_name.size() + 1, cmd.size()));
        return Result::ConfigStr;
    }

    else if (cmd_name == "repeat") {
        return cmd_parse(Result::RepetitionCommand, *this, index_param, first, last, cmd.end(),
            CmdArgParser{DelayParser{1000u}, OutParam{delay}, "delay"_av, std::chrono::milliseconds()},
            optional_decimal_parser(OutParam{repeat_delay}, "repetition"_av, -1),
            CmdArgOptionalRemaining{OutParam{output_message}, "command"_av}
        );
    }

    else if (cmd_name == "q" || cmd_name == "quit") {
        return Result::Quit;
    }

    else {
        return set_param_error(*this, ErrorType::UnknownCommand, 0, *first, "command"_av);
    }
}
