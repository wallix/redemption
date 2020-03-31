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

#pragma once

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "red_emscripten/em_asm.hpp"
#include "red_emscripten/bind.hpp"
#include "utils/stream.hpp"
#include "core/channel_names.hpp"

#include "core/callback.hpp"

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

inline namespace test_channel
{

using DataChan_tuple = std::tuple<
    CHANNELS::ChannelNameId, std::vector<uint8_t>, std::size_t, uint32_t>;

template<class>
struct js_to_tuple;

template<class... Ts>
struct js_to_tuple<void(Ts...)>
{
    using type = std::tuple<Ts...>;
};

void print_bytes(std::ostream& out, bytes_view v);

template<class T>
struct WVector
{
    std::vector<T> v;

    WVector(bytes_view av)
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
    }, +[]() { g_channel_data_received.push_back(classname{}); })

#define JS_X_MAKE_CLASS(classname, ...)                                        \
    struct classname : decltype(js_to_tuple<void(__VA_ARGS__)>::type{})        \
    {                                                                          \
        using decltype(js_to_tuple<void(__VA_ARGS__)>::type{})::tuple;         \
        friend std::ostream& operator<<(std::ostream& out, classname const& x) \
        { out << #classname << "{"; print_tuple(out, x); return out << "}"; }  \
    }

#define JS_x(classname, ...) (                                 \
    classname,                                                 \
    JS_X_MAKE_CLASS(classname, __VA_ARGS__),                   \
    static_cast<void(*)(__VA_ARGS__)>([](auto... args) {       \
        g_channel_data_received.push_back(classname{args...}); \
    })                                                         \
)

#define JS_x_f(classname, body_func, ...) (                         \
    classname,                                                      \
    JS_X_MAKE_CLASS(classname, __VA_ARGS__),                        \
    ([]{                                                            \
        static auto user_func = [](__VA_ARGS__) { body_func; };     \
        return static_cast<decltype(+user_func)>([](auto... args) { \
            g_channel_data_received.push_back(classname{args...});  \
            return user_func(args...);                              \
        });                                                         \
    }())                                                            \
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
            g_channel_data_received.push_back(classname{                       \
                WVector<data_type>{{p_, p_+n_}}, args...});                    \
    })                                                                         \
)

#define MAKE_TYPE_NAME(classname, ...) classname

#define MAKE_BIND_TYPE_E(r, data, elem)                           \
    template<class... Ts> struct binding_type<__COUNTER__, Ts...> \
    : binding_type<__COUNTER__-3, Ts..., MAKE_TYPE_NAME elem>     \
    {};

#define MAKE_JS_TO_CPP_E(r, data, elem) MAKE_JS_TO_CPP_E_S elem
#define MAKE_JS_TO_CPP_E_S(classname, struct_def, func) \
    inline namespace test_channel { struct_def; }

#define MAKE_JS_CALL_E(r, data, elem) \
    MAKE_TYPE_NAME elem: (...args) => \
        BOOST_PP_CAT(Module.test_js_chan__, MAKE_TYPE_NAME elem)(...args),

#define MAKE_CPP_BINDING_E(r, data, elem) \
    redjs::function("test_js_chan__" MAKE_CPP_BINDING_F elem);

#define MAKE_CPP_BINDING_F(classname, struct_def, func) #classname, func

#define MAKE_BINDING_CALLBACKS(ModChannelDataType, S)       \
    BOOST_PP_SEQ_FOR_EACH(MAKE_JS_TO_CPP_E, _, S)           \
                                                            \
    template<int, class...> class binding_type;             \
    template<class... Ts>                                   \
    struct binding_type<__COUNTER__-1, Ts...>               \
    { using type = std::variant<                            \
        ModChannelDataType, Ts...>; };                      \
                                                            \
    BOOST_PP_SEQ_FOR_EACH(MAKE_BIND_TYPE_E, _, S)           \
                                                            \
    std::vector<binding_type<__COUNTER__-2>::type>          \
        g_channel_data_received;                            \
                                                            \
    void TestBindingSendModChannel                          \
        ::send_to_mod_channel(                              \
        CHANNELS::ChannelNameId front_channel_name,         \
        InStream & chunk, std::size_t length,               \
        uint32_t flags)                                     \
    {                                                       \
        g_channel_data_received.push_back(                  \
            ModChannelDataType{front_channel_name, {        \
                chunk.get_data(),                           \
                chunk.get_data_end()                        \
            }, length, flags});                             \
    }                                                       \
                                                            \
    template<class F>                                       \
    void ::datas_checker::operator = (F f)                  \
    {                                                       \
        f(*this);                                           \
        if (i < g_channel_data_received.size())             \
        {                                                   \
            err(*this);                                     \
        }                                                   \
        g_channel_data_received.clear();                    \
    }                                                       \
                                                            \
    TestBindingSendModChannel g_send_mod_channel;           \
                                                            \
    EMSCRIPTEN_BINDINGS(test_channel_)                      \
    {                                                       \
        BOOST_PP_SEQ_FOR_EACH(MAKE_CPP_BINDING_E, _, S)     \
        auto init = [](emscripten::val&& v){                \
            _global_create_channel(                         \
                static_cast<Callback&>(g_send_mod_channel), \
                std::move(v));                              \
            g_channel_data_received.clear();                \
        };                                                  \
        redjs::function("test_init_js_channel_", init);     \
    }                                                       \
                                                            \
    void init_js_channel()                                  \
    {                                                       \
        g_channel_data_received.clear();                    \
        RED_EM_ASM({Module.test_init_js_channel_({          \
            BOOST_PP_SEQ_FOR_EACH(MAKE_JS_CALL_E, _, S)     \
        });});                                              \
    }

inline void print_bytes(std::ostream& out, bytes_view v)
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
                previous = 1;
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

struct TestBindingSendModChannel : Callback
{
    void rdp_input_invalidate(Rect) override {}
    void rdp_input_mouse(int, int, int, Keymap2*) override {}
    void rdp_input_scancode(long, long, long, long, Keymap2*) override {}
    void rdp_input_synchronize(uint32_t, uint16_t, int16_t, int16_t) override {}
    void refresh(Rect) override {}
    void rdp_gdi_down() override {}
    void rdp_gdi_up_and_running(ScreenInfo & ) override {}
    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream & chunk, std::size_t length, uint32_t flags) override;
};

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
//         std::visit(Overload{f, this->f}, g_channel_data_received[i]);
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
    void operator = (F f);
};

#define RED_CHECK_V(i, ...) std::visit(Overload{                  \
    [&](decltype(__VA_ARGS__) const& expected) {                  \
        RED_CHECK((__VA_ARGS__) == expected); },                  \
    [&](auto const& x){ RED_CHECK_MESSAGE(false, "check "         \
        << ::get_type<::remove_cvref_t<decltype(__VA_ARGS__)>>()  \
        << " == " << ::get_type<::remove_cvref_t<decltype(x)>>()  \
        << " has failed [" << (__VA_ARGS__) << " != " << x << "]" \
    ); }                                                          \
}, ::g_channel_data_received[i])

// #define RED_CHECK_COUNT(n) RED_CHECK(::g_channel_data_received.size() == n);

#define CTX_CHECK_DATAS()                                       \
    ::datas_checker{[](datas_checker& _data_checker){           \
        RED_CHECK_MESSAGE(false, "there is "                    \
            << ::g_channel_data_received.size()                 \
            << " elements, " << _data_checker.i << " checked"); \
    }, 0} = [&]([[maybe_unused]] datas_checker& _data_checker)

#define CHECK_NEXT_DATA(...) do {                                \
    if (_data_checker.i == ::g_channel_data_received.size()) {   \
        RED_CHECK_MESSAGE(false, "there is only "                \
            << ::g_channel_data_received.size() << " elements"); \
        return;                                                  \
    }                                                            \
    RED_CHECK_V(_data_checker.i++, __VA_ARGS__);                 \
} while (0);

struct DataChanPrintable : DataChan_tuple
{
    using DataChan_tuple::DataChan_tuple;

    friend std::ostream& operator<<(std::ostream& out, DataChanPrintable const& x)
    {
        auto& t = static_cast<DataChan_tuple const&>(x);
        out << "DataChan{" << std::get<0>(t) << ", {";
        print_bytes(out, std::get<1>(t));
        out << "}, " << std::get<2>(t) << ", " << std::get<3>(t) << "}";
        return out;
    }
};

void* _global_channel_instance = nullptr;
void (*_global_create_channel)(Callback&, emscripten::val&&) = nullptr;

}
}

#define RED_AUTO_TEST_CHANNEL(test_name, create_channel, chann_name)            \
    namespace {                                                                 \
        auto CHANNEL_TU_ ## test_name ## create_channel() {                     \
            return create_channel;                                              \
        };                                                                      \
    }                                                                           \
    struct CHANNEL_TU_ ## test_name {                                           \
        using channel_type = decltype(                                          \
            CHANNEL_TU_ ## test_name ## create_channel()(                       \
                std::declval<Callback&>(), std::declval<emscripten::val&&>())); \
        static void test(channel_type& chann_name);                             \
    };                                                                          \
    RED_AUTO_TEST_CASE(test_name) {                                             \
        using channel_type = CHANNEL_TU_ ## test_name::channel_type;            \
                                                                                \
        ::test_channel::_global_create_channel = [](                            \
            Callback& cb, emscripten::val&& v                                   \
        ) {                                                                     \
            auto* chann = new channel_type{                                     \
                CHANNEL_TU_ ## test_name ## create_channel()(                   \
                    cb, static_cast<emscripten::val&&>(v))};                    \
            ::test_channel::_global_channel_instance = chann;                   \
        };                                                                      \
                                                                                \
        init_js_channel();                                                      \
        std::unique_ptr<channel_type> pchann{static_cast<channel_type*>(        \
            ::test_channel::_global_channel_instance)};                         \
        CHANNEL_TU_ ## test_name ::test(*pchann);                               \
        pchann.reset();                                                         \
        ::test_channel::_global_channel_instance = nullptr;                     \
        CTX_CHECK_DATAS() { CHECK_NEXT_DATA(::test_channel::free{}); };         \
    }                                                                           \
    void CHANNEL_TU_ ## test_name ::test(channel_type& chann_name)
