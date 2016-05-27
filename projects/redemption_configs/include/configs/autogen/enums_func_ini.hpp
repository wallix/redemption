#pragma once

#include "configs/io.hpp"
#include "configs/autogen/enums.hpp"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>


namespace configs {

template<> struct szbuffer_traits<CaptureFlags> : szbuffer_traits<unsigned long> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<CaptureFlags> & buf,
    cfg_s_type<CaptureFlags>,
    CaptureFlags x
) {
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return non_owner_string(buf.get(), sz);
}

parse_error parse(CaptureFlags & x, spec_type<CaptureFlags>, array_view_const_char value)
{
    return parse_enum_u(x, value, static_cast<unsigned long>((1 << (5 - 1)) - 1));
}

template<> struct szbuffer_traits<Level> : szbuffer_traits<void> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<Level> & buf,
    cfg_s_type<Level>,
    Level x
) {
    (void)buf;    constexpr non_owner_string arr[]{
        non_owner_string("low"),
        non_owner_string("medium"),
        non_owner_string("high"),
    };
    assert(static_cast<unsigned long>(x) < 3);
    return arr[static_cast<unsigned long>(x)];
}

parse_error parse(Level & x, spec_type<Level>, array_view_const_char value)
{
    return parse_enum_str(x, value, {
        {cstr_array_view("low"), Level::low},
        {cstr_array_view("medium"), Level::medium},
        {cstr_array_view("high"), Level::high},
    });
}

template<> struct szbuffer_traits<Language> : szbuffer_traits<void> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<Language> & buf,
    cfg_s_type<Language>,
    Language x
) {
    (void)buf;    constexpr non_owner_string arr[]{
        non_owner_string("en"),
        non_owner_string("fr"),
    };
    assert(static_cast<unsigned long>(x) < 2);
    return arr[static_cast<unsigned long>(x)];
}

parse_error parse(Language & x, spec_type<Language>, array_view_const_char value)
{
    return parse_enum_str(x, value, {
        {cstr_array_view("en"), Language::en},
        {cstr_array_view("fr"), Language::fr},
    });
}

template<> struct szbuffer_traits<ClipboardEncodingType> : szbuffer_traits<void> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<ClipboardEncodingType> & buf,
    cfg_s_type<ClipboardEncodingType>,
    ClipboardEncodingType x
) {
    (void)buf;    constexpr non_owner_string arr[]{
        non_owner_string("utf8"),
        non_owner_string("latin1"),
    };
    assert(static_cast<unsigned long>(x) < 2);
    return arr[static_cast<unsigned long>(x)];
}

parse_error parse(ClipboardEncodingType & x, spec_type<ClipboardEncodingType>, array_view_const_char value)
{
    return parse_enum_str(x, value, {
        {cstr_array_view("utf-8"), ClipboardEncodingType::utf8},
        {cstr_array_view("latin1"), ClipboardEncodingType::latin1},
    });
}

template<> struct szbuffer_traits<KeyboardLogFlags> : szbuffer_traits<unsigned long> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<KeyboardLogFlags> & buf,
    cfg_s_type<KeyboardLogFlags>,
    KeyboardLogFlags x
) {
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return non_owner_string(buf.get(), sz);
}

parse_error parse(KeyboardLogFlags & x, spec_type<KeyboardLogFlags>, array_view_const_char value)
{
    return parse_enum_u(x, value, static_cast<unsigned long>((1 << (3 - 1)) - 1));
}

template<> struct szbuffer_traits<ClipboardLogFlags> : szbuffer_traits<unsigned long> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<ClipboardLogFlags> & buf,
    cfg_s_type<ClipboardLogFlags>,
    ClipboardLogFlags x
) {
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return non_owner_string(buf.get(), sz);
}

parse_error parse(ClipboardLogFlags & x, spec_type<ClipboardLogFlags>, array_view_const_char value)
{
    return parse_enum_u(x, value, static_cast<unsigned long>((1 << (3 - 1)) - 1));
}

template<> struct szbuffer_traits<FileSystemLogFlags> : szbuffer_traits<unsigned long> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<FileSystemLogFlags> & buf,
    cfg_s_type<FileSystemLogFlags>,
    FileSystemLogFlags x
) {
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return non_owner_string(buf.get(), sz);
}

parse_error parse(FileSystemLogFlags & x, spec_type<FileSystemLogFlags>, array_view_const_char value)
{
    return parse_enum_u(x, value, static_cast<unsigned long>((1 << (3 - 1)) - 1));
}

template<> struct szbuffer_traits<ServerNotification> : szbuffer_traits<unsigned long> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<ServerNotification> & buf,
    cfg_s_type<ServerNotification>,
    ServerNotification x
) {
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return non_owner_string(buf.get(), sz);
}

parse_error parse(ServerNotification & x, spec_type<ServerNotification>, array_view_const_char value)
{
    return parse_enum_u(x, value, static_cast<unsigned long>((1 << (4 - 1)) - 1));
}

template<> struct szbuffer_traits<ServerCertCheck> : szbuffer_traits<unsigned long> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<ServerCertCheck> & buf,
    cfg_s_type<ServerCertCheck>,
    ServerCertCheck x
) {
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return non_owner_string(buf.get(), sz);
}

parse_error parse(ServerCertCheck & x, spec_type<ServerCertCheck>, array_view_const_char value)
{
    return parse_enum_u(x, value, static_cast<unsigned long>((1 << (4 - 1)) - 1));
}

template<> struct szbuffer_traits<TraceType> : szbuffer_traits<unsigned long> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<TraceType> & buf,
    cfg_s_type<TraceType>,
    TraceType x
) {
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return non_owner_string(buf.get(), sz);
}

parse_error parse(TraceType & x, spec_type<TraceType>, array_view_const_char value)
{
    return parse_enum_u(x, value, static_cast<unsigned long>((1 << (3 - 1)) - 1));
}

template<> struct szbuffer_traits<KeyboardInputMaskingLevel> : szbuffer_traits<unsigned long> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<KeyboardInputMaskingLevel> & buf,
    cfg_s_type<KeyboardInputMaskingLevel>,
    KeyboardInputMaskingLevel x
) {
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return non_owner_string(buf.get(), sz);
}

parse_error parse(KeyboardInputMaskingLevel & x, spec_type<KeyboardInputMaskingLevel>, array_view_const_char value)
{
    return parse_enum_u(x, value, static_cast<unsigned long>((1 << (4 - 1)) - 1));
}

template<> struct szbuffer_traits<SessionProbeOnLaunchFailure> : szbuffer_traits<unsigned long> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<SessionProbeOnLaunchFailure> & buf,
    cfg_s_type<SessionProbeOnLaunchFailure>,
    SessionProbeOnLaunchFailure x
) {
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return non_owner_string(buf.get(), sz);
}

parse_error parse(SessionProbeOnLaunchFailure & x, spec_type<SessionProbeOnLaunchFailure>, array_view_const_char value)
{
    return parse_enum_u(x, value, static_cast<unsigned long>((1 << (3 - 1)) - 1));
}

template<> struct szbuffer_traits<VncBogusClipboardInfiniteLoop> : szbuffer_traits<unsigned long> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<VncBogusClipboardInfiniteLoop> & buf,
    cfg_s_type<VncBogusClipboardInfiniteLoop>,
    VncBogusClipboardInfiniteLoop x
) {
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return non_owner_string(buf.get(), sz);
}

parse_error parse(VncBogusClipboardInfiniteLoop & x, spec_type<VncBogusClipboardInfiniteLoop>, array_view_const_char value)
{
    return parse_enum_u(x, value, static_cast<unsigned long>((1 << (3 - 1)) - 1));
}

template<> struct szbuffer_traits<ColorDepthSelectionStrategy> : szbuffer_traits<unsigned long> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<ColorDepthSelectionStrategy> & buf,
    cfg_s_type<ColorDepthSelectionStrategy>,
    ColorDepthSelectionStrategy x
) {
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return non_owner_string(buf.get(), sz);
}

parse_error parse(ColorDepthSelectionStrategy & x, spec_type<ColorDepthSelectionStrategy>, array_view_const_char value)
{
    return parse_enum_u(x, value, static_cast<unsigned long>((1 << (2 - 1)) - 1));
}

template<> struct szbuffer_traits<WrmCompressionAlgorithm> : szbuffer_traits<unsigned long> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<WrmCompressionAlgorithm> & buf,
    cfg_s_type<WrmCompressionAlgorithm>,
    WrmCompressionAlgorithm x
) {
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return non_owner_string(buf.get(), sz);
}

parse_error parse(WrmCompressionAlgorithm & x, spec_type<WrmCompressionAlgorithm>, array_view_const_char value)
{
    return parse_enum_u(x, value, static_cast<unsigned long>((1 << (3 - 1)) - 1));
}

template<> struct szbuffer_traits<RdpCompression> : szbuffer_traits<unsigned long> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<RdpCompression> & buf,
    cfg_s_type<RdpCompression>,
    RdpCompression x
) {
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return non_owner_string(buf.get(), sz);
}

parse_error parse(RdpCompression & x, spec_type<RdpCompression>, array_view_const_char value)
{
    return parse_enum_u(x, value, static_cast<unsigned long>((1 << (5 - 1)) - 1));
}

template<> struct szbuffer_traits<ColorDepth> : szbuffer_traits<unsigned long> {};

inline non_owner_string cfg_to_s(
    szbuffer_from<ColorDepth> & buf,
    cfg_s_type<ColorDepth>,
    ColorDepth x
) {
    int sz = snprintf(buf.get(), buf.size(), "%lu", static_cast<unsigned long>(x));
    return non_owner_string(buf.get(), sz);
}

parse_error parse(ColorDepth & x, spec_type<ColorDepth>, array_view_const_char value)
{
    return parse_enum_list(x, value, {
        ColorDepth::depth8,
        ColorDepth::depth15,
        ColorDepth::depth16,
        ColorDepth::depth24,
    });
}

}
