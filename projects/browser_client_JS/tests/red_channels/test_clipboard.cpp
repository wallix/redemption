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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "red_emscripten/em_asm.hpp"
#include "red_emscripten/bind.hpp"
#include "red_channels/clipboard.hpp"

#include "core/callback.hpp"
#include "core/RDP/clipboard.hpp"

#include <tuple>
#include <vector>
#include <variant>
#include <string_view>

#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/list/adt.hpp>
#include <boost/preprocessor/variadic/to_list.hpp>
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/tuple/enum.hpp>


namespace
{

template<class>
struct js_to_tuple;

template<class... Ts>
struct js_to_tuple<void(Ts...)>
{
    using type = std::tuple<Ts...>;
};

void print_bytes(std::ostream& out, cbytes_view v);

template<class T>
struct WVector
{
    std::vector<T> v;

    WVector(cbytes_view av)
    : v(av.begin(), av.end())
    {}

    WVector() = default;

    template<class U>
    friend std::ostream& operator<<(std::ostream& out, WVector<U> const& x)
    {
        print_bytes(out, x.v);
        return out;
    }

    bool operator == (WVector const& x) const { return v == x.v; }
};

#define JS_c(classname)                                                      \
    (classname, struct classname {                                           \
        friend std::ostream& operator<<(std::ostream& out, classname const&) \
        { return out << #classname "{}"; }                                   \
        bool operator == (classname const) const { return true; }            \
    }, +[]() { clip_datas.push_back(classname{}); })

#define JS_x(classname, ...) (                                                 \
    classname,                                                                 \
    struct classname : decltype(js_to_tuple<void(__VA_ARGS__)>::type{})        \
    {                                                                          \
        using decltype(js_to_tuple<void(__VA_ARGS__)>::type{})::tuple;         \
        friend std::ostream& operator<<(std::ostream& out, classname const& x) \
        { out << #classname << "{"; print_tuple(out, x); return out << "}"; }  \
    },                                                                         \
    static_cast<void(*)(__VA_ARGS__)>(                                         \
        [](auto... args) { clip_datas.push_back(classname{args...}); }         \
    )                                                                          \
)

#define JS_d(classname, data_type, ...) (                                      \
    classname,                                                                 \
    struct classname                                                           \
    : decltype(js_to_tuple<void(WVector<data_type>, __VA_ARGS__)>::type{})     \
    {                                                                          \
        using decltype(js_to_tuple<void(                                       \
            WVector<data_type>, __VA_ARGS__)>::type{})::tuple;                 \
        friend std::ostream& operator<<(std::ostream& out, classname const& x) \
        { out << #classname << "{"; print_tuple(out, x); return out << "}"; }  \
    },                                                                         \
    static_cast<void(*)(uintptr_t, uint32_t, __VA_ARGS__)>(                    \
        [](uintptr_t iptr_, uint32_t n_, auto... args) {                       \
            auto p_ = reinterpret_cast<data_type*>(iptr_);                     \
            clip_datas.push_back(classname{                                    \
                WVector<data_type>{{p_, p_+n_}}, args...});                    \
    })                                                                         \
)

#define MAKE_TYPE_NAME(classname, ...) classname

#define MAKE_BIND_TYPE_E(r, data, elem)                           \
    template<class... Ts> struct binding_type<__COUNTER__, Ts...> \
    : binding_type<__COUNTER__-3, Ts..., MAKE_TYPE_NAME elem>     \
    {};

#define MAKE_JS_TO_CPP_E(r, data, elem) MAKE_JS_TO_CPP_E_S elem
#define MAKE_JS_TO_CPP_E_S(classname, struct_def, func) struct_def;

#define MAKE_JS_CALL_E(r, data, elem)   \
    MAKE_TYPE_NAME elem: (...args) => { \
        BOOST_PP_CAT(Module.test_clip_, MAKE_TYPE_NAME elem)(...args) },

#define MAKE_CPP_BINDING_E(r, data, elem) \
    redjs::function("test_clip_" MAKE_CPP_BINDING_F elem);

#define MAKE_CPP_BINDING_F(classname, struct_def, func) #classname, func

#define MAKE_BINDING_CALLBACKS(S, extratypes)           \
    BOOST_PP_SEQ_FOR_EACH(MAKE_JS_TO_CPP_E, _, S)       \
                                                        \
    template<int, class...> class binding_type;         \
    template<class... Ts>                               \
    struct binding_type<__COUNTER__-1, Ts...>           \
    { using type = std::variant<                        \
        BOOST_PP_TUPLE_ENUM(extratypes), Ts...>; };     \
                                                        \
    BOOST_PP_SEQ_FOR_EACH(MAKE_BIND_TYPE_E, _, S)       \
                                                        \
    std::vector<binding_type<__COUNTER__-2>::type>      \
        clip_datas;                                     \
                                                        \
    EMSCRIPTEN_BINDINGS(test_clipboard_channel)         \
    {                                                   \
        BOOST_PP_SEQ_FOR_EACH(MAKE_CPP_BINDING_E, _, S) \
    }                                                   \
                                                        \
    void init_clip()                                    \
    {                                                   \
        clip_datas.clear();                             \
        RED_EM_ASM({Module.test_init_clip({             \
            BOOST_PP_SEQ_FOR_EACH(MAKE_JS_CALL_E, _, S) \
        });});                                          \
    }

inline void print_bytes(std::ostream& out, cbytes_view v)
{
    out << "\"";
    char const* s = "0123456789abcdef";
    uint8_t previous = 1;
    for (auto const& x : v)
    {
        if (x >= 0x20 && x < 127)
        {
            auto ishex = [](uint8_t c){
                return ('0' <= c && c <= '9')
                    || ('a' <= c && c <= 'f')
                    || ('A' <= c && c <= 'F');
            };

            if ((!previous && ishex(x)) || ishex(previous))
            {
                out << "\"\"";
            }
            out << x;
        }
        else
        {
            previous = x;
            if (x)
            {
                out << "\\x" << s[x>>4] << s[x&0xf];
            }
            else
            {
                out << "\\0";
            }
        }
    }
    out << "\"_av";
}

template<class... Ts>
void print_tuple(std::ostream& out, std::tuple<Ts...> const& t)
{
    std::apply([&](auto const& x, auto const&... xs){
        out << x;
        ((out << ", " << xs), ...);
    }, t);
}

using DataChan_tuple = std::tuple<CHANNELS::ChannelNameId, std::vector<uint8_t>, std::size_t, uint32_t>;
struct DataChan : DataChan_tuple
{
    using DataChan_tuple::tuple;

    DataChan(cbytes_view av, size_t total_len, uint32_t channel_flags)
    : DataChan(
        channel_names::cliprdr,
        {av.begin(), av.end()},
        total_len != ~0u ? total_len : av.size(),
        channel_flags)
    {}

    friend std::ostream& operator<<(std::ostream& out, DataChan const& x)
    {
        DataChan_tuple const& t = x;
        out << "DataChan{" << std::get<0>(t) << ", {";
        InStream in_stream(std::get<1>(t));
        RDPECLIP::CliprdrHeader header;
        header.recv(in_stream);
        out << "0x" << std::hex << header.msgType() << ", 0x" << header.msgFlags()
            << std::dec << ", " << header.dataLen() << ", ";
        print_bytes(out, in_stream.remaining_bytes());
        out << "}, " << std::get<2>(t) << ", 0x" << std::hex << std::get<3>(t) << std::dec << "}";
        return out;
    }
};

MAKE_BINDING_CALLBACKS(
    (JS_c(receiveFormatStart))
    (JS_c(receiveFormatStop))
    (JS_d(receiveFormat, uint8_t, uint32_t formatId, bool isUTF8))
    (JS_d(receiveData, uint8_t, uint32_t formatId, uint32_t channelFlags))
    (JS_x(receiveNbFileName, uint32_t nb))
    (JS_d(receiveFileName, uint8_t, uint32_t attr, uint32_t flags, uint32_t sizeLow, uint32_t sizeHigh, uint32_t lastWriteTimeLow, uint32_t lastWriteTimeHigh))
    (JS_d(receiveFileContents, uint8_t, uint32_t streamId, uint32_t channelFlags))
    (JS_x(receiveFileSize, uint32_t sizeHigh, uint32_t sizeLow, uint32_t streamId))
    (JS_x(receiveFormatId, uint32_t format_id)),
    (DataChan)
)

struct Mod : Callback
{
    void rdp_input_invalidate(Rect) override {}
    void rdp_input_mouse(int, int, int, Keymap2*) override {}
    void rdp_input_scancode(long, long, long, long, Keymap2*) override {}
    void rdp_input_synchronize(uint32_t, uint16_t, int16_t, int16_t) override {}
    void refresh(Rect) override {}

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream & chunk, std::size_t length, uint32_t flags) override
    {
        clip_datas.push_back(DataChan{
            front_channel_name,
            {chunk.get_data(), chunk.get_data_end()},
            length,
            flags
        });
    }
};

Mod mod;
std::unique_ptr<redjs::ClipboardChannel> clip;

EMSCRIPTEN_BINDINGS(test_clipboard)
{
    redjs::function("test_init_clip", [](emscripten::val&& v){
        clip = std::make_unique<redjs::ClipboardChannel>(mod, std::move(v), RDPVerbose{});
        clip_datas.clear();
    });
}

constexpr int first_last_show_proto_channel_flags
    = CHANNELS::CHANNEL_FLAG_LAST
    | CHANNELS::CHANNEL_FLAG_FIRST
    | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
;

constexpr int first_last_channel_flags
    = CHANNELS::CHANNEL_FLAG_LAST
    | CHANNELS::CHANNEL_FLAG_FIRST
;

void clip_raw_receive(cbytes_view data, int channel_flags = first_last_show_proto_channel_flags)
{
    clip->receive(data, channel_flags);
}

enum class Padding : unsigned;

struct Serializer
{
    StaticOutStream<65536> out_stream;

    Serializer(uint16_t msgType, uint16_t msgFlags, cbytes_view data, Padding padding_data)
    {
        RDPECLIP::CliprdrHeader header(msgType, msgFlags, data.size());
        header.emit(out_stream);
        out_stream.out_copy_bytes(data);
        auto padding = unsigned(padding_data);
        while (padding--) {
            out_stream.out_uint8(0);
        }
    }

    operator cbytes_view () const
    {
        return out_stream.get_bytes();
    }
};

void clip_receive(
    uint16_t msgType, uint16_t msgFlags,
    cbytes_view data = {},
    Padding padding_data = Padding{},
    uint32_t channel_flags = first_last_show_proto_channel_flags)
{
    clip_raw_receive(Serializer(msgType, msgFlags, data, padding_data), channel_flags);
}

DataChan data_chan(
    uint16_t msgType, uint16_t msgFlags,
    cbytes_view data = {},
    Padding padding_data = Padding{},
    std::size_t len = ~0u, uint32_t channel_flags = first_last_show_proto_channel_flags)
{
    return DataChan{Serializer(msgType, msgFlags, data, padding_data), len, channel_flags};
}

template<class... Fs>
struct Overload : Fs...
{
    using Fs::operator()...;
};

template<class... Fs>
Overload(Fs...) -> Overload<Fs...>;

template<class T>
using remove_cvref_t = std::remove_const_t<std::remove_reference_t<T>>;

template<class T>
std::string_view get_type()
{
    return [](auto&& a) {
        return std::string_view(a+79, sizeof(a)-81);
    }(__PRETTY_FUNCTION__);
}

// template<class F>
// struct run_f
// {
//     int i;
//     F f;
//
//     template<class FT>
//     void operator=(FT f)
//     {
//         std::visit(Overload{f, this->f}, clip_datas[i]);
//     }
// };
//
// template<class F>
// run_f(int, F) -> run_f<F>;

// template<class> class extract_p1;
// template<class T> struct extract_p1<void(T)> { using type = T; };
// template<class T> using extract_p1_t = remove_cvref_t<typename extract_p1<T>::type>;

// #define RED_CHECK_TYPE(i, Type) ::run_f{i,                    \
//     [&](auto const& x){ RED_CHECK_MESSAGE(false, "check "     \
//         << ::get_type<::extract_p1_t<void(Type)>>() << " == " \
//         << ::get_type<::remove_cvref_t<decltype(x)>>()        \
//         << " has failed"                                      \
//     ); } } = [&](Type)

struct datas_checker
{
    void(*err)(datas_checker&);
    unsigned i = 0;
    template<class F>
    void operator = (F f)
    {
        f(*this);
        if (i < clip_datas.size())
        {
            err(*this);
        }
        clip_datas.clear();
    }
};

#define RED_CHECK_V(i, ...) std::visit(Overload{                  \
    [&](decltype(__VA_ARGS__) const& x) {                         \
        RED_CHECK((__VA_ARGS__) == x); },                         \
    [&](auto const& x){ RED_CHECK_MESSAGE(false, "check "         \
        << ::get_type<::remove_cvref_t<decltype(__VA_ARGS__)>>()  \
        << " == " << ::get_type<::remove_cvref_t<decltype(x)>>()  \
        << " has failed [" << (__VA_ARGS__) << " != " << x << "]" \
    ); }                                                          \
}, clip_datas[i])

// #define RED_CHECK_COUNT(n) RED_CHECK(::clip_datas.size() == n);

#define CTX_CHECK_DATAS()                                           \
    ::datas_checker{[](datas_checker& _data_checker){               \
        RED_CHECK_MESSAGE(false, "there is " << ::clip_datas.size() \
            << " elements, " << _data_checker.i << " checked");     \
    }, 0} = [&]([[maybe_unused]] datas_checker& _data_checker)

#define RECEIVE_DATAS(...) ::clip_receive(__VA_ARGS__); CTX_CHECK_DATAS()
#define CALL_CB(...) clip->__VA_ARGS__; CTX_CHECK_DATAS()

#define CHECK_NEXT_DATA(...) do {                   \
    if (_data_checker.i == clip_datas.size()) {     \
        RED_CHECK_MESSAGE(false, "there is only "   \
            << ::clip_datas.size() << " elements"); \
        return;                                     \
    }                                               \
    RED_CHECK_V(_data_checker.i++, __VA_ARGS__);    \
} while (0);

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wgnu-string-literal-operator-template")
template<class C, C... cs>
std::array<uint8_t, sizeof...(cs) * 2> const operator "" _utf16()
{
    std::array<uint8_t, sizeof...(cs) * 2> a;
    char s[] {cs...};
    auto p = a.data();
    for (char c : s)
    {
        p[0] = c;
        p[1] = 0;
        p += 2;
    }
    return a;
}
REDEMPTION_DIAGNOSTIC_POP

}

RED_AUTO_TEST_CASE(TestClipboardChannel)
{
    using namespace RDPECLIP;
    namespace cbchan = redjs::channels::clipboard;
    init_clip();

    const bool is_utf = true;
    // const bool not_utf = false;

    RECEIVE_DATAS(CB_CLIP_CAPS, CB_RESPONSE__NONE_,
        "\x01\x00\x00\x00\x01\x00\x0c\x00\x02\x00\x00\x00\x1e\x00\x00\x00"_av, Padding(4))
    {
    };

    RECEIVE_DATAS(CB_MONITOR_READY, CB_RESPONSE__NONE_)
    {
        CHECK_NEXT_DATA(data_chan(CB_CLIP_CAPS, CB_RESPONSE__NONE_,
            "\x01\0\0\0\x01\0\x0C\0\x02\0\0\0.\0\0\0"_av));
        CHECK_NEXT_DATA(data_chan(CB_FORMAT_LIST, CB_ASCII_NAMES,
            "\x0d\0\0\0\0\0"_av));
    };

    RECEIVE_DATAS(CB_FORMAT_LIST_RESPONSE, CB_RESPONSE_OK)
    {
    };

    // copy

    RECEIVE_DATAS(CB_FORMAT_LIST, CB_RESPONSE__NONE_,
        "\x0d\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x01\x00"
        "\x00\x00\x00\x00\x07\x00\x00\x00\x00\x00"_av, Padding(4))
    {
        CHECK_NEXT_DATA(receiveFormatStart{});
        CHECK_NEXT_DATA(receiveFormat{"unicodetext"_av, CF_UNICODETEXT, is_utf});
        CHECK_NEXT_DATA(receiveFormat{"locale"_av, CF_LOCALE, is_utf});
        CHECK_NEXT_DATA(receiveFormat{"text"_av, CF_TEXT, is_utf});
        CHECK_NEXT_DATA(receiveFormat{"oemtext"_av, CF_OEMTEXT, is_utf});
        CHECK_NEXT_DATA(data_chan(CB_FORMAT_LIST_RESPONSE, CB_RESPONSE_OK));
        CHECK_NEXT_DATA(receiveFormatStop{});
    };

    CALL_CB(send_request_format(CF_UNICODETEXT, cbchan::CustomFormat::None))
    {
        CHECK_NEXT_DATA(data_chan(CB_FORMAT_DATA_REQUEST, CB_RESPONSE__NONE_, "\x0d\0\0\0"_av));
    };

    RECEIVE_DATAS(CB_FORMAT_DATA_RESPONSE, CB_RESPONSE_OK,
        "\x70\x00\x6c\x00\x6f\x00\x70\x00\x00\x00"_av, Padding(4))
    {
        CHECK_NEXT_DATA(receiveData("plop"_utf16, CF_UNICODETEXT, first_last_channel_flags));
    };

    // paste

    CALL_CB(send_format(CF_UNICODETEXT, cbchan::Charset::Utf16, ""_av, true))
    {
        CHECK_NEXT_DATA(data_chan(CB_FORMAT_LIST, CB_RESPONSE__NONE_, "\x0d\0\0\0\0\0"_av));
    };

    RECEIVE_DATAS(CB_FORMAT_LIST_RESPONSE, CB_RESPONSE_OK, ""_av, Padding(4))
    {
    };

    RECEIVE_DATAS(CB_FORMAT_DATA_REQUEST, CB_RESPONSE__NONE_, "\x0d\x00\x00\x00"_av, Padding(4))
    {
        CHECK_NEXT_DATA(receiveFormatId{CF_UNICODETEXT});
    };

    const auto paste1 = "xyz\0"_utf16;
    CALL_CB(send_header(CB_FORMAT_DATA_RESPONSE, CB_RESPONSE_OK, paste1.size(), 0))
    {
        CHECK_NEXT_DATA(DataChan{"\x05\0\1\0\x08\0\0\0"_av, paste1.size() + 8,
            CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL});
    };

    CALL_CB(send_data(paste1, 0, CHANNELS::CHANNEL_FLAG_LAST))
    {
        CHECK_NEXT_DATA(DataChan{paste1, 0,
            CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL});
    };
}
