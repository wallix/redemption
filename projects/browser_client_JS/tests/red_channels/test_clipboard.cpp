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

template<class>
struct js_to_tuple;

template<class... Ts>
struct js_to_tuple<void(Ts...)>
{
    using type = std::tuple<Ts...>;
};

#define JS_c(classname)                                                        \
    (classname, struct classname {                                             \
        friend std::ostream& operator<<(std::ostream& out, classname const& x) \
        { return out << #classname "{}"; }                                     \
        bool operator == (classname const&) const { return true; }             \
    }, +[]() { clip_datas.push_back(classname{}); })

#define JS_x(classname, ...) (                                                 \
    classname,                                                                 \
    struct classname : decltype(js_to_tuple<void(__VA_ARGS__)>::type{})        \
    {                                                                          \
        using decltype(js_to_tuple<void(__VA_ARGS__)>::type{})::tuple;         \
        friend std::ostream& operator<<(std::ostream& out, classname const& x) \
        { out << #classname; print_tuple(out, x); return out << "}"; }         \
    },                                                                         \
    static_cast<void(*)(__VA_ARGS__)>(                                         \
        [](auto... args) { clip_datas.push_back(classname{args...}); }         \
    )                                                                          \
)

#define JS_d(classname, data_type, ...) (                                      \
    classname,                                                                 \
    struct classname                                                           \
    : decltype(js_to_tuple<void(std::vector<data_type>, __VA_ARGS__)>::type{}) \
    {                                                                          \
        using decltype(js_to_tuple<void(                                       \
            std::vector<data_type>, __VA_ARGS__)>::type{})::tuple;             \
        friend std::ostream& operator<<(std::ostream& out, classname const& x) \
        { out << #classname; print_tuple(out, x); return out << "}"; }         \
    },                                                                         \
    static_cast<void(*)(uintptr_t, uint32_t, __VA_ARGS__)>(                    \
        [](uintptr_t iptr_, uint32_t n_, auto... args) {                       \
            auto p_ = reinterpret_cast<data_type*>(iptr_);                     \
            clip_datas.push_back(classname{                                    \
                std::vector<data_type>{p_, p_+n_}, args...});                  \
    })                                                                         \
)

#define MAKE_TYPE_NAME(classname, ...) classname

#define MAKE_BIND_TYPE_E(r, data, elem)                           \
    template<class... Ts> struct binding_type<__COUNTER__, Ts...> \
    : binding_type<__COUNTER__-3, Ts..., MAKE_TYPE_NAME elem>     \
    {};

#define MAKE_JS_TO_CPP_E(r, data, elem) MAKE_JS_TO_CPP_E_S elem
#define MAKE_JS_TO_CPP_E_S(classname, struct_def, func) struct_def;

#define MAKE_JS_CALL_E(r, data, elem) \
    MAKE_TYPE_NAME elem: (...args) => \
        BOOST_PP_CAT(Module.test_clip_, MAKE_TYPE_NAME elem),

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

namespace
{

namespace detail
{
    template<class T>
    std::ostream& print_value(std::ostream& out, T const& x)
    {
        return out << x;
    }

    template<class T>
    void print_value(std::ostream& out, std::vector<T> const& v)
    {
        out << "{";
        for (auto const& x : v)
        {
            out << +x << ", ";
        }
        out << "}";
    }
}

template<class... Ts>
void print_tuple(std::ostream& out, std::tuple<Ts...> const& t)
{
    std::apply([&](auto const& x, auto const&... xs){
        detail::print_value(out, x);
        (detail::print_value(out << ", ", xs), ...);
    }, t);
}

using DataChan_tuple = std::tuple<CHANNELS::ChannelNameId, std::vector<uint8_t>, std::size_t, uint32_t>;
struct DataChan : DataChan_tuple
{
    using DataChan_tuple::tuple;
    friend std::ostream& operator<<(std::ostream& out, DataChan const& x)
    { out << "DataChan{"; print_tuple(out, x); return out << "}"; }
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
    (JS_x(receiveFormatId, uint32_t id)),
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
    });
}

constexpr int first_last_channel_flags
    = CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST
;

void raw_send(cbytes_view data, int channel_flags = first_last_channel_flags)
{
    clip->receive(data, channel_flags);
}

enum class Padding : unsigned;

void send(
    uint16_t msgType, uint16_t msgFlags, cbytes_view data, Padding padding_data = Padding{},
    int channel_flags = first_last_channel_flags)
{
    StaticOutStream<65536> out_stream;
    RDPECLIP::CliprdrHeader header(msgType, msgFlags, data.size());
    header.emit(out_stream);
    out_stream.out_copy_bytes(data);
    auto padding = unsigned(padding_data);
    while (padding--) {
        out_stream.out_uint8(0);
    }
    clip->receive(out_stream.get_bytes(), channel_flags);
}

template<class... Fs>
struct Overload : Fs...
{
    using Fs::operator()...;
};

template<class... Fs>
Overload(Fs...) -> Overload<Fs...>;

template<class F>
struct run_f
{
    int i;
    F f;

    template<class FT>
    void operator=(FT f)
    {
        std::visit(Overload{f, this->f}, clip_datas[i]);
    }
};

template<class F>
run_f(int, F) -> run_f<F>;

template<class T>
using remove_cvref_t = std::remove_const_t<std::remove_reference_t<T>>;

template<class> class extract_p1;
template<class T> struct extract_p1<void(T)> { using type = T; };
template<class T> using extract_p1_t = remove_cvref_t<typename extract_p1<T>::type>;

template<class T>
std::string_view get_type()
{
    return [](auto&& a) {
        return std::string_view(a+79, sizeof(a)-81);
    }(__PRETTY_FUNCTION__);
}

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

#define RED_CHECK_TYPE(i, Type) ::run_f{i,                    \
    [&](auto const& x){ RED_CHECK_MESSAGE(false, "check "     \
        << ::get_type<::extract_p1_t<void(Type)>>() << " == " \
        << ::get_type<::remove_cvref_t<decltype(x)>>()        \
        << " has failed"                                      \
    ); } } = [&](Type)

#define RED_CHECK_V(i, ...) std::visit(Overload{                  \
    [&](decltype(__VA_ARGS__) const& x) {                         \
        RED_CHECK(x == (__VA_ARGS__)); },                         \
    [&](auto const& x){ RED_CHECK_MESSAGE(false, "check "         \
        << ::get_type<::remove_cvref_t<decltype(__VA_ARGS__)>>()  \
        << " == " << ::get_type<::remove_cvref_t<decltype(x)>>()  \
        << " has failed [" << (__VA_ARGS__) << " != " << x << "]" \
    ); }                                                          \
}, clip_datas[i])

#define RED_CHECK_COUNT(n) RED_REQUIRE(::clip_datas.size() == n);

#define CTX_DATAS() ::datas_checker{[](datas_checker& _data_checker){ \
    RED_CHECK_MESSAGE(false, "there is " << ::clip_datas.size()       \
        << " elements, " << _data_checker.i << " checked");           \
}, 0} = [&](datas_checker& _data_checker)

#define CHECK_NEXT_DATA(...) do {                   \
    if (_data_checker.i == clip_datas.size()) {     \
        RED_CHECK_MESSAGE(false, "there is only "   \
            << ::clip_datas.size() << " elements"); \
        return;                                     \
    }                                               \
    RED_CHECK_V(_data_checker.i++, __VA_ARGS__);    \
} while (0);

}

RED_AUTO_TEST_CASE(TestClipboardChannel)
{
    init_clip();

    send(RDPECLIP::CB_CLIP_CAPS, RDPECLIP::CB_RESPONSE__NONE_,
        "\x01\x00\x00\x00\x01\x00\x0c\x00\x02\x00\x00\x00\x1e\x00\x00\x00"_av, Padding(4));

    RED_CHECK_COUNT(0);

    send(RDPECLIP::CB_MONITOR_READY, RDPECLIP::CB_RESPONSE__NONE_, {});

    RED_CHECK_COUNT(2);
    RED_CHECK_V(0, DataChan{channel_names::cliprdr, {}, 0, 0});
    RED_CHECK_TYPE(0, DataChan& x){

    };

    CTX_DATAS() {
    };

    CTX_DATAS() {
        CHECK_NEXT_DATA(DataChan{channel_names::cliprdr, {}, 0, 0});
        CHECK_NEXT_DATA(DataChan{channel_names::cliprdr, {}, 0, 0});
    };

    CTX_DATAS() {
        CHECK_NEXT_DATA(DataChan{channel_names::cliprdr, {}, 0, 0});
        CHECK_NEXT_DATA(DataChan{channel_names::cliprdr, {}, 0, 0});
        CHECK_NEXT_DATA(DataChan{channel_names::cliprdr, {}, 0, 0});
    };
}
