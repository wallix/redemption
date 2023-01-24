/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "headlessclient/headless_configuration.hpp"
#include "configs/config.hpp"
#include "core/client_info.hpp"
#include "utils/ascii.hpp"
#include "utils/sugar/chars_to_int.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/sugar/static_array_to_hexadecimal_chars.hpp"


void headless_init_client_info(ClientInfo& client_info)
{
    client_info.number_of_cache = 3;
    client_info.screen_info = {800, 600, BitsPerPixel::BitsPP16};
    client_info.keylayout = KeyLayout::KbdId(0x040C);
    client_info.build = 420;
    client_info.rdp5_performanceflags = 0x80 /*Enable font smoothing*/
                                      | 0x20 /*Disable mouse cursor shadows*/
                                      | 0x28 /*Disable theme*/
                                      ;
    client_info.large_pointer_caps.largePointerSupportFlags = LARGE_POINTER_FLAG_96x96;
    client_info.multi_fragment_update_caps.MaxRequestSize = 38'055;
    for (auto& x : client_info.order_caps.orderSupport) {
        x = 1;
    }
}

void headless_init_ini(Inifile& ini)
{
    ini.set<cfg::mod_rdp::persistent_disk_bitmap_cache>(false);
    ini.set<cfg::mod_rdp::ignore_auth_channel>(false);
}

namespace
{

constexpr zstring_view section_name_of_client_info = "client_info"_zv;

char const* check_number_result(zstring_view str, std::errc ec, char const* end_ptr)
{
    if (ec == std::errc()) {
        if (end_ptr == str.end()) {
            return nullptr;
        }
    }
    else if (ec == std::errc::result_out_of_range) {
        return "out of range";
    }
    return "wrong number format";
}

template<class T>
char const* parse_value(zstring_view str, T& value)
{
    if constexpr (std::is_unsigned_v<T>) {
        if (str.size() > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
            auto r = hexadecimal_chars_to_int(str.c_str() + 2, value);
            return check_number_result(str, r.ec, r.ptr);
        }
    }

    auto r = decimal_chars_to_int(str.c_str(), value);
    return check_number_result(str, r.ec, r.ptr);
}

char const* parse_value(zstring_view str, bool& value)
{
    constexpr std::size_t max_size = 5;

    if (str.size() <= max_size) {
        if (str == "1") {
            value = true;
            return nullptr;
        }
        if (str == "0") {
            value = false;
            return nullptr;
        }

        auto upper = ascii_to_limited_upper<max_size>(str);
        if (upper == "ON"_ascii_upper || upper == "YES"_ascii_upper || upper == "TRUE"_ascii_upper) {
            value = false;
            return nullptr;
        }
        if (upper == "OFF"_ascii_upper || upper == "NO"_ascii_upper || upper == "FALSE"_ascii_upper) {
            value = false;
            return nullptr;
        }
    }

    return "bad value, expected: 1, on, yes, true, 0, off, no, false";
}

char const* parse_value(zstring_view str, BitsPerPixel& value)
{
    auto x = underlying_cast(value);
    auto err = parse_value(str, x);
    if (!err) {
        if (x == 8 || x == 15 || x == 16 || x == 24 || x == 32) {
            value = safe_int(x);
            return err;
        }
        return "bad bytes per pixel, expected 8, 15, 16, 24 or 32";
    }
    return err;
}

template<std::size_t N>
char const* parse_value(zstring_view str, char(&value)[N])
{
    if (str.size() < N) {
        memcpy(value, str.data(), str.size());
        value[str.size()] = '\0';
        return nullptr;
    }
    return "too many character";
}

template<std::size_t N>
char const* parse_value(zstring_view str, uint8_t(&value)[N])
{
    if (str.size() != N * 2) {
        return "the number of hexadecimal characters does not match";
    }

    for (char c : str) {
        if (!is_hexadecimal_char(c)) {
            return "invalid hexadecimal string format";
        }
    }

    uint8_t* it = value;
    for (std::size_t i = 0; i < str.size(); i += 2) {
        *it = (unchecked_hexadecimal_char_to_int(str[i]) << 4)
            | unchecked_hexadecimal_char_to_int(str[i+1]);
        ++it;
    }
    return nullptr;
}

template<class Fn>
auto apply_on_client_info(ClientInfo& client_info, Fn fn)
{
    return fn("width"_zv, client_info.screen_info.width)
        && fn("height"_zv, client_info.screen_info.height)
        && fn("bpp"_zv, client_info.screen_info.bpp)

        && fn("pointer_cache_entries"_zv, client_info.pointer_cache_entries)
        && fn("supported_new_pointer_update"_zv, client_info.supported_new_pointer_update)
        && fn("hostname"_zv, client_info.hostname)
        && fn("build"_zv, client_info.build)
        && fn("keylayout"_zv, client_info.keylayout)
        && fn("username"_zv, client_info.username)
        && fn("password"_zv, client_info.password)
        && fn("domain"_zv, client_info.domain)
        && fn("has_sound_code"_zv, client_info.has_sound_code)
        && fn("has_sound_capture_code"_zv, client_info.has_sound_capture_code)
        && fn("rdp5_performanceflags"_zv, client_info.rdp5_performanceflags)
        && fn("console_session"_zv, client_info.console_session)
        && fn("restricted_admin_mode"_zv, client_info.restricted_admin_mode)

        && fn("remote_program"_zv, client_info.remote_program)
        && fn("remote_program_enhanced"_zv, client_info.remote_program_enhanced)
        && fn("alternate_shell"_zv, client_info.alternate_shell)
        && fn("working_dir"_zv, client_info.working_dir)

        && fn("desktop_physical_width"_zv, client_info.desktop_physical_width)
        && fn("desktop_physical_height"_zv, client_info.desktop_physical_height)
        && fn("desktop_orientation"_zv, client_info.desktop_orientation)
        && fn("desktop_scale_factor"_zv, client_info.desktop_scale_factor)
        && fn("device_scale_factor"_zv, client_info.device_scale_factor)

        && fn("monitor_count"_zv, client_info.cs_monitor.monitorCount)
        && fn("monitor[0].left"_zv, client_info.cs_monitor.monitorDefArray[0].left)
        && fn("monitor[0].top"_zv, client_info.cs_monitor.monitorDefArray[0].top)
        && fn("monitor[0].right"_zv, client_info.cs_monitor.monitorDefArray[0].right)
        && fn("monitor[0].bottom"_zv, client_info.cs_monitor.monitorDefArray[0].bottom)
        && fn("monitor[0].flags"_zv, client_info.cs_monitor.monitorDefArray[0].flags)
        && fn("monitor[1].left"_zv, client_info.cs_monitor.monitorDefArray[1].left)
        && fn("monitor[1].top"_zv, client_info.cs_monitor.monitorDefArray[1].top)
        && fn("monitor[1].right"_zv, client_info.cs_monitor.monitorDefArray[1].right)
        && fn("monitor[1].bottom"_zv, client_info.cs_monitor.monitorDefArray[1].bottom)
        && fn("monitor[1].flags"_zv, client_info.cs_monitor.monitorDefArray[1].flags)
        && fn("monitor[2].left"_zv, client_info.cs_monitor.monitorDefArray[2].left)
        && fn("monitor[2].top"_zv, client_info.cs_monitor.monitorDefArray[2].top)
        && fn("monitor[2].right"_zv, client_info.cs_monitor.monitorDefArray[2].right)
        && fn("monitor[2].bottom"_zv, client_info.cs_monitor.monitorDefArray[2].bottom)
        && fn("monitor[2].flags"_zv, client_info.cs_monitor.monitorDefArray[2].flags)
        && fn("monitor[3].left"_zv, client_info.cs_monitor.monitorDefArray[3].left)
        && fn("monitor[3].top"_zv, client_info.cs_monitor.monitorDefArray[3].top)
        && fn("monitor[3].right"_zv, client_info.cs_monitor.monitorDefArray[3].right)
        && fn("monitor[3].bottom"_zv, client_info.cs_monitor.monitorDefArray[3].bottom)
        && fn("monitor[3].flags"_zv, client_info.cs_monitor.monitorDefArray[3].flags)
        && fn("monitor[4].left"_zv, client_info.cs_monitor.monitorDefArray[4].left)
        && fn("monitor[4].top"_zv, client_info.cs_monitor.monitorDefArray[4].top)
        && fn("monitor[4].right"_zv, client_info.cs_monitor.monitorDefArray[4].right)
        && fn("monitor[4].bottom"_zv, client_info.cs_monitor.monitorDefArray[4].bottom)
        && fn("monitor[4].flags"_zv, client_info.cs_monitor.monitorDefArray[4].flags)
        && fn("monitor[5].left"_zv, client_info.cs_monitor.monitorDefArray[5].left)
        && fn("monitor[5].top"_zv, client_info.cs_monitor.monitorDefArray[5].top)
        && fn("monitor[5].right"_zv, client_info.cs_monitor.monitorDefArray[5].right)
        && fn("monitor[5].bottom"_zv, client_info.cs_monitor.monitorDefArray[5].bottom)
        && fn("monitor[5].flags"_zv, client_info.cs_monitor.monitorDefArray[5].flags)
        && fn("monitor[6].left"_zv, client_info.cs_monitor.monitorDefArray[6].left)
        && fn("monitor[6].top"_zv, client_info.cs_monitor.monitorDefArray[6].top)
        && fn("monitor[6].right"_zv, client_info.cs_monitor.monitorDefArray[6].right)
        && fn("monitor[6].bottom"_zv, client_info.cs_monitor.monitorDefArray[6].bottom)
        && fn("monitor[6].flags"_zv, client_info.cs_monitor.monitorDefArray[6].flags)
        && fn("monitor[7].left"_zv, client_info.cs_monitor.monitorDefArray[7].left)
        && fn("monitor[7].top"_zv, client_info.cs_monitor.monitorDefArray[7].top)
        && fn("monitor[7].right"_zv, client_info.cs_monitor.monitorDefArray[7].right)
        && fn("monitor[7].bottom"_zv, client_info.cs_monitor.monitorDefArray[7].bottom)
        && fn("monitor[7].flags"_zv, client_info.cs_monitor.monitorDefArray[7].flags)
        && fn("monitor[8].left"_zv, client_info.cs_monitor.monitorDefArray[8].left)
        && fn("monitor[8].top"_zv, client_info.cs_monitor.monitorDefArray[8].top)
        && fn("monitor[8].right"_zv, client_info.cs_monitor.monitorDefArray[8].right)
        && fn("monitor[8].bottom"_zv, client_info.cs_monitor.monitorDefArray[8].bottom)
        && fn("monitor[8].flags"_zv, client_info.cs_monitor.monitorDefArray[8].flags)
        && fn("monitor[9].left"_zv, client_info.cs_monitor.monitorDefArray[9].left)
        && fn("monitor[9].top"_zv, client_info.cs_monitor.monitorDefArray[9].top)
        && fn("monitor[9].right"_zv, client_info.cs_monitor.monitorDefArray[9].right)
        && fn("monitor[9].bottom"_zv, client_info.cs_monitor.monitorDefArray[9].bottom)
        && fn("monitor[9].flags"_zv, client_info.cs_monitor.monitorDefArray[9].flags)
        && fn("monitor[10].left"_zv, client_info.cs_monitor.monitorDefArray[10].left)
        && fn("monitor[10].top"_zv, client_info.cs_monitor.monitorDefArray[10].top)
        && fn("monitor[10].right"_zv, client_info.cs_monitor.monitorDefArray[10].right)
        && fn("monitor[10].bottom"_zv, client_info.cs_monitor.monitorDefArray[10].bottom)
        && fn("monitor[10].flags"_zv, client_info.cs_monitor.monitorDefArray[10].flags)
        && fn("monitor[11].left"_zv, client_info.cs_monitor.monitorDefArray[11].left)
        && fn("monitor[11].top"_zv, client_info.cs_monitor.monitorDefArray[11].top)
        && fn("monitor[11].right"_zv, client_info.cs_monitor.monitorDefArray[11].right)
        && fn("monitor[11].bottom"_zv, client_info.cs_monitor.monitorDefArray[11].bottom)
        && fn("monitor[11].flags"_zv, client_info.cs_monitor.monitorDefArray[11].flags)
        && fn("monitor[12].left"_zv, client_info.cs_monitor.monitorDefArray[12].left)
        && fn("monitor[12].top"_zv, client_info.cs_monitor.monitorDefArray[12].top)
        && fn("monitor[12].right"_zv, client_info.cs_monitor.monitorDefArray[12].right)
        && fn("monitor[12].bottom"_zv, client_info.cs_monitor.monitorDefArray[12].bottom)
        && fn("monitor[12].flags"_zv, client_info.cs_monitor.monitorDefArray[12].flags)
        && fn("monitor[13].left"_zv, client_info.cs_monitor.monitorDefArray[13].left)
        && fn("monitor[13].top"_zv, client_info.cs_monitor.monitorDefArray[13].top)
        && fn("monitor[13].right"_zv, client_info.cs_monitor.monitorDefArray[13].right)
        && fn("monitor[13].bottom"_zv, client_info.cs_monitor.monitorDefArray[13].bottom)
        && fn("monitor[13].flags"_zv, client_info.cs_monitor.monitorDefArray[13].flags)
        && fn("monitor[14].left"_zv, client_info.cs_monitor.monitorDefArray[14].left)
        && fn("monitor[14].top"_zv, client_info.cs_monitor.monitorDefArray[14].top)
        && fn("monitor[14].right"_zv, client_info.cs_monitor.monitorDefArray[14].right)
        && fn("monitor[14].bottom"_zv, client_info.cs_monitor.monitorDefArray[14].bottom)
        && fn("monitor[14].flags"_zv, client_info.cs_monitor.monitorDefArray[14].flags)
        && fn("monitor[15].left"_zv, client_info.cs_monitor.monitorDefArray[15].left)
        && fn("monitor[15].top"_zv, client_info.cs_monitor.monitorDefArray[15].top)
        && fn("monitor[15].right"_zv, client_info.cs_monitor.monitorDefArray[15].right)
        && fn("monitor[15].bottom"_zv, client_info.cs_monitor.monitorDefArray[15].bottom)
        && fn("monitor[15].flags"_zv, client_info.cs_monitor.monitorDefArray[15].flags)

        && fn("monitor_ex_attribute_size"_zv, client_info.cs_monitor_ex.monitorAttributeSize)
        && fn("monitor_ex_count"_zv, client_info.cs_monitor_ex.monitorCount)
        && fn("monitor_ex[0].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[0].physicalWidth)
        && fn("monitor_ex[0].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[0].physicalHeight)
        && fn("monitor_ex[0].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[0].orientation)
        && fn("monitor_ex[0].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[0].desktopScaleFactor)
        && fn("monitor_ex[0].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[0].deviceScaleFactor)
        && fn("monitor_ex[1].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[1].physicalWidth)
        && fn("monitor_ex[1].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[1].physicalHeight)
        && fn("monitor_ex[1].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[1].orientation)
        && fn("monitor_ex[1].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[1].desktopScaleFactor)
        && fn("monitor_ex[1].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[1].deviceScaleFactor)
        && fn("monitor_ex[2].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[2].physicalWidth)
        && fn("monitor_ex[2].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[2].physicalHeight)
        && fn("monitor_ex[2].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[2].orientation)
        && fn("monitor_ex[2].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[2].desktopScaleFactor)
        && fn("monitor_ex[2].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[2].deviceScaleFactor)
        && fn("monitor_ex[3].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[3].physicalWidth)
        && fn("monitor_ex[3].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[3].physicalHeight)
        && fn("monitor_ex[3].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[3].orientation)
        && fn("monitor_ex[3].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[3].desktopScaleFactor)
        && fn("monitor_ex[3].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[3].deviceScaleFactor)
        && fn("monitor_ex[4].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[4].physicalWidth)
        && fn("monitor_ex[4].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[4].physicalHeight)
        && fn("monitor_ex[4].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[4].orientation)
        && fn("monitor_ex[4].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[4].desktopScaleFactor)
        && fn("monitor_ex[4].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[4].deviceScaleFactor)
        && fn("monitor_ex[5].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[5].physicalWidth)
        && fn("monitor_ex[5].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[5].physicalHeight)
        && fn("monitor_ex[5].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[5].orientation)
        && fn("monitor_ex[5].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[5].desktopScaleFactor)
        && fn("monitor_ex[5].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[5].deviceScaleFactor)
        && fn("monitor_ex[6].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[6].physicalWidth)
        && fn("monitor_ex[6].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[6].physicalHeight)
        && fn("monitor_ex[6].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[6].orientation)
        && fn("monitor_ex[6].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[6].desktopScaleFactor)
        && fn("monitor_ex[6].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[6].deviceScaleFactor)
        && fn("monitor_ex[7].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[7].physicalWidth)
        && fn("monitor_ex[7].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[7].physicalHeight)
        && fn("monitor_ex[7].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[7].orientation)
        && fn("monitor_ex[7].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[7].desktopScaleFactor)
        && fn("monitor_ex[7].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[7].deviceScaleFactor)
        && fn("monitor_ex[8].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[8].physicalWidth)
        && fn("monitor_ex[8].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[8].physicalHeight)
        && fn("monitor_ex[8].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[8].orientation)
        && fn("monitor_ex[8].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[8].desktopScaleFactor)
        && fn("monitor_ex[8].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[8].deviceScaleFactor)
        && fn("monitor_ex[9].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[9].physicalWidth)
        && fn("monitor_ex[9].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[9].physicalHeight)
        && fn("monitor_ex[9].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[9].orientation)
        && fn("monitor_ex[9].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[9].desktopScaleFactor)
        && fn("monitor_ex[9].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[9].deviceScaleFactor)
        && fn("monitor_ex[10].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[10].physicalWidth)
        && fn("monitor_ex[10].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[10].physicalHeight)
        && fn("monitor_ex[10].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[10].orientation)
        && fn("monitor_ex[10].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[10].desktopScaleFactor)
        && fn("monitor_ex[10].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[10].deviceScaleFactor)
        && fn("monitor_ex[11].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[11].physicalWidth)
        && fn("monitor_ex[11].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[11].physicalHeight)
        && fn("monitor_ex[11].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[11].orientation)
        && fn("monitor_ex[11].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[11].desktopScaleFactor)
        && fn("monitor_ex[11].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[11].deviceScaleFactor)
        && fn("monitor_ex[12].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[12].physicalWidth)
        && fn("monitor_ex[12].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[12].physicalHeight)
        && fn("monitor_ex[12].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[12].orientation)
        && fn("monitor_ex[12].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[12].desktopScaleFactor)
        && fn("monitor_ex[12].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[12].deviceScaleFactor)
        && fn("monitor_ex[13].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[13].physicalWidth)
        && fn("monitor_ex[13].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[13].physicalHeight)
        && fn("monitor_ex[13].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[13].orientation)
        && fn("monitor_ex[13].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[13].desktopScaleFactor)
        && fn("monitor_ex[13].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[13].deviceScaleFactor)
        && fn("monitor_ex[14].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[14].physicalWidth)
        && fn("monitor_ex[14].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[14].physicalHeight)
        && fn("monitor_ex[14].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[14].orientation)
        && fn("monitor_ex[14].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[14].desktopScaleFactor)
        && fn("monitor_ex[14].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[14].deviceScaleFactor)
        && fn("monitor_ex[15].width"_zv, client_info.cs_monitor_ex.monitorAttributesArray[15].physicalWidth)
        && fn("monitor_ex[15].height"_zv, client_info.cs_monitor_ex.monitorAttributesArray[15].physicalHeight)
        && fn("monitor_ex[15].orientation"_zv, client_info.cs_monitor_ex.monitorAttributesArray[15].orientation)
        && fn("monitor_ex[15].desktop_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[15].desktopScaleFactor)
        && fn("monitor_ex[15].device_scale_factor"_zv, client_info.cs_monitor_ex.monitorAttributesArray[15].deviceScaleFactor)

        && fn("glyph_cache_frag_cache"_zv, client_info.glyph_cache_caps.FragCache)
        && fn("glyph_cache[0].entries"_zv, client_info.glyph_cache_caps.GlyphCache[0].CacheEntries)
        && fn("glyph_cache[0].max_size"_zv, client_info.glyph_cache_caps.GlyphCache[0].CacheMaximumCellSize)
        && fn("glyph_cache[1].entries"_zv, client_info.glyph_cache_caps.GlyphCache[1].CacheEntries)
        && fn("glyph_cache[1].max_size"_zv, client_info.glyph_cache_caps.GlyphCache[1].CacheMaximumCellSize)
        && fn("glyph_cache[2].entries"_zv, client_info.glyph_cache_caps.GlyphCache[2].CacheEntries)
        && fn("glyph_cache[2].max_size"_zv, client_info.glyph_cache_caps.GlyphCache[2].CacheMaximumCellSize)
        && fn("glyph_cache[3].entries"_zv, client_info.glyph_cache_caps.GlyphCache[3].CacheEntries)
        && fn("glyph_cache[3].max_size"_zv, client_info.glyph_cache_caps.GlyphCache[3].CacheMaximumCellSize)
        && fn("glyph_cache[4].entries"_zv, client_info.glyph_cache_caps.GlyphCache[4].CacheEntries)
        && fn("glyph_cache[4].max_size"_zv, client_info.glyph_cache_caps.GlyphCache[4].CacheMaximumCellSize)
        && fn("glyph_cache[5].entries"_zv, client_info.glyph_cache_caps.GlyphCache[5].CacheEntries)
        && fn("glyph_cache[5].max_size"_zv, client_info.glyph_cache_caps.GlyphCache[5].CacheMaximumCellSize)
        && fn("glyph_cache[6].entries"_zv, client_info.glyph_cache_caps.GlyphCache[6].CacheEntries)
        && fn("glyph_cache[6].max_size"_zv, client_info.glyph_cache_caps.GlyphCache[6].CacheMaximumCellSize)
        && fn("glyph_cache[7].entries"_zv, client_info.glyph_cache_caps.GlyphCache[7].CacheEntries)
        && fn("glyph_cache[7].max_size"_zv, client_info.glyph_cache_caps.GlyphCache[7].CacheMaximumCellSize)
        && fn("glyph_cache[8].entries"_zv, client_info.glyph_cache_caps.GlyphCache[8].CacheEntries)
        && fn("glyph_cache[8].max_size"_zv, client_info.glyph_cache_caps.GlyphCache[8].CacheMaximumCellSize)
        && fn("glyph_cache[9].entries"_zv, client_info.glyph_cache_caps.GlyphCache[9].CacheEntries)
        && fn("glyph_cache[9].max_size"_zv, client_info.glyph_cache_caps.GlyphCache[9].CacheMaximumCellSize)

        && fn("large_pointer_flags"_zv, client_info.large_pointer_caps.largePointerSupportFlags)
        && fn("multi_fragment_max_request_size"_zv, client_info.multi_fragment_update_caps.MaxRequestSize)
        && fn("bitmap_cache_entries"_zv, client_info.number_of_cache)
        && fn("rail_support_level"_zv, client_info.rail_caps.RailSupportLevel)

        && fn("window_list_support_level"_zv, client_info.window_list_caps.WndSupportLevel)
        && fn("window_list_num_icon_caches"_zv, client_info.window_list_caps.NumIconCaches)
        && fn("window_list_num_icon_cache_entries"_zv, client_info.window_list_caps.NumIconCacheEntries)

        && fn("bitmap_codec_have_remove_fx"_zv, client_info.bitmap_codec_caps.haveRemoteFxCodec)
        && fn("bitmap_codec_count"_zv, client_info.bitmap_codec_caps.bitmapCodecCount)
        && fn("bitmap_codec[0].guid"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[0].codecGUID)
        && fn("bitmap_codec[0].id"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[0].codecID)
        && fn("bitmap_codec[0].properties_length"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[0].codecPropertiesLength)
        && fn("bitmap_codec[0].type"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[0].codecType)
        && fn("bitmap_codec[1].guid"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[1].codecGUID)
        && fn("bitmap_codec[1].id"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[1].codecID)
        && fn("bitmap_codec[1].properties_length"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[1].codecPropertiesLength)
        && fn("bitmap_codec[1].type"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[1].codecType)
        && fn("bitmap_codec[2].guid"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[2].codecGUID)
        && fn("bitmap_codec[2].id"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[2].codecID)
        && fn("bitmap_codec[2].properties_length"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[2].codecPropertiesLength)
        && fn("bitmap_codec[2].type"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[2].codecType)
        && fn("bitmap_codec[3].guid"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[3].codecGUID)
        && fn("bitmap_codec[3].id"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[3].codecID)
        && fn("bitmap_codec[3].properties_length"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[3].codecPropertiesLength)
        && fn("bitmap_codec[3].type"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[3].codecType)
        && fn("bitmap_codec[4].guid"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[4].codecGUID)
        && fn("bitmap_codec[4].id"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[4].codecID)
        && fn("bitmap_codec[4].properties_length"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[4].codecPropertiesLength)
        && fn("bitmap_codec[4].type"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[4].codecType)
        && fn("bitmap_codec[5].guid"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[5].codecGUID)
        && fn("bitmap_codec[5].id"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[5].codecID)
        && fn("bitmap_codec[5].properties_length"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[5].codecPropertiesLength)
        && fn("bitmap_codec[5].type"_zv, client_info.bitmap_codec_caps.bitmapCodecArray[5].codecType)
    ;
}

struct HeadlessConfigurationHolder : ConfigurationHolder
{
    explicit HeadlessConfigurationHolder(Inifile& ini, ClientInfo& client_info) noexcept
    : client_info(client_info)
    , loader(ini)
    , ini(ini)
    {}

    ConfigurationHolder& as_ref()
    {
        return *this;
    }

    void set_section(zstring_view section) override
    {
        if (section != section_name_of_client_info) {
            is_headless_section = false;
            loader.set_section(section);
        }
        else {
            is_headless_section = true;
        }
    }

    void set_value(zstring_view key, zstring_view value) override
    {
        if (!is_headless_section) {
            loader.set_value(key, value);
            return ;
        }

        char const* err_msg = nullptr;
        if (apply_on_client_info(client_info, [&](zstring_view name, auto& mem){
            if (key == name) {
                err_msg = parse_value(value, mem);
                return false;
            }
            return true;
        })) {
            LOG_IF(ini.get<cfg::debug::config>(), LOG_WARNING,
                "Unknown parameter %s in section [%s]", key, section_name_of_client_info.c_str());
        }
        else if (err_msg) {
            LOG_IF(ini.get<cfg::debug::config>(), LOG_WARNING,
                "parsing error with parameter '%s' in section [%s] for \"%s\": %s",
                key.c_str(), section_name_of_client_info.c_str(), value.c_str(), err_msg);
        }
    }

private:
    bool is_headless_section = 0;
    ClientInfo& client_info;
    Inifile::ConfigurationHolder loader;
    Inifile& ini;
};

} // anonymous namespace

void load_headless_config_from_file(Inifile& ini, ClientInfo& client_info, char const* filename)
{
    configuration_load(HeadlessConfigurationHolder{ini, client_info}.as_ref(), filename);
}

void load_headless_config_from_string(Inifile& ini, ClientInfo& client_info, char* str)
{
    configuration_load_from_string(HeadlessConfigurationHolder{ini, client_info}.as_ref(), str);
}

namespace
{
    template<class T>
    struct headless_config_type_to_string
    {};

#define MK_DEF(type)                                       \
    template<> struct headless_config_type_to_string<type> \
    { static constexpr char const* value = #type; }

    MK_DEF(bool);
    MK_DEF(char);
    // MK_DEF(int8_t);
    // MK_DEF(int16_t);
    MK_DEF(int32_t);
    // MK_DEF(int64_t);
    MK_DEF(uint8_t);
    MK_DEF(uint16_t);
    MK_DEF(uint32_t);
    // MK_DEF(uint64_t);

#undef MK_DEF

    template<class T>
    struct headless_config_type_to_string_enum
    : headless_config_type_to_string<std::underlying_type_t<T>>
    {};

#define MK_DEF(type)                                            \
    template<> struct headless_config_type_to_string_enum<type> \
    { static constexpr char const* value = #type; }

    MK_DEF(BitsPerPixel);

#undef MK_DEF

} // anonymous namespace

std::string headless_client_info_config_as_string(ClientInfo const& client_info)
{
    std::string s;
    s.reserve(1024);
    s += '[';
    s += section_name_of_client_info.to_sv();
    s += ']';
    s += '\n';

    auto& info = const_cast<ClientInfo&>(client_info); // NOLINT
    apply_on_client_info(info, [&](zstring_view name, auto& mem){
        using Mem = std::remove_reference_t<decltype(mem)>;

        // write type
        s += "\n# ";
        if constexpr (std::is_array_v<Mem>) {
            using value_type = std::remove_pointer_t<std::decay_t<decltype(mem)>>;
            constexpr std::size_t extent = std::extent_v<Mem>;
            s += headless_config_type_to_string<value_type>::value;
            s += '[';
            s += int_to_decimal_chars(extent).sv();
            s += ']';
        }
        else if constexpr (std::is_enum_v<Mem>) {
            s += headless_config_type_to_string_enum<Mem>::value;
        }
        else {
            s += headless_config_type_to_string<Mem>::value;
        }
        s += '\n';

        s += name.to_sv();
        s += '=';

        // write value
        if constexpr (std::is_array_v<Mem>) {
            using value_type = std::remove_pointer_t<std::decay_t<decltype(mem)>>;
            if constexpr (std::is_same_v<char, value_type>) {
                s += mem;
            }
            else {
                static_assert(std::is_same_v<uint8_t, value_type>);
                s += chars_view(static_array_to_hexadecimal_upper_chars(mem)).as<std::string_view>();
            }
        }
        else if constexpr (std::is_enum_v<Mem>) {
            s += int_to_decimal_chars(underlying_cast(mem)).sv();
        }
        else {
            s += int_to_decimal_chars(mem).sv();
        }
        s += '\n';

        return true;
    });

    return s;
}
