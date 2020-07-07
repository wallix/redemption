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
#include "red_emscripten/val.hpp"
#include "utils/stream.hpp"
#include "core/channel_names.hpp"

#include "core/callback.hpp"

#include <tuple>
#include <vector>
#include <variant>

#ifdef IN_IDE_PARSER
# include <ostream>
#endif

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

struct BasicChannelData
{
    CHANNELS::ChannelNameId channel_name;
    std::vector<uint8_t> data;
    std::size_t total_len;
    uint32_t channel_flags;

    BasicChannelData(
        CHANNELS::ChannelNameId channel_name,
        bytes_view av,
        std::size_t total_len,
        uint32_t channel_flags)
    : channel_name(channel_name)
    , data(av.begin(), av.end())
    , total_len(total_len)
    , channel_flags(channel_flags)
    {}


    template<class Ch, class Tr>
    friend std::basic_ostream<Ch, Tr>&
    operator<<(std::basic_ostream<Ch, Tr>& out, BasicChannelData const& chann)
    {
        out << "DataChan{" << chann.channel_name << ", {";
        ut::put_view(0, out, ut::hex(chann.data));
        out << "}, " << chann.total_len << ", " << chann.channel_flags << "}";
        return out;
    }

    bool operator == (BasicChannelData const& other) const
    {
        return channel_name == other.channel_name
            && data == other.data
            && total_len == other.total_len
            && channel_flags == other.channel_flags
            ;
    }
};

template<class T>
struct WVector
{
    std::vector<T> v;

    WVector(bytes_view av)
    : v(av.begin(), av.end())
    {}

    WVector(array_view<T> av)
    : v(av.begin(), av.end())
    {}

    WVector(emscripten::val const& val)
    : WVector(array_view{
        redjs::from_memory_offset<T*>(val["byteOffset"].as<uintptr_t>()),
        val["length"].as<unsigned>(),
    })
    {}

    WVector() = default;

    template<class U>
    friend std::ostream& operator<<(std::ostream& out, WVector<U> const& x)
    {
        ut::put_view(0, out, ut::hex(x.v));
        return out;
    }

    bool operator == (WVector const& x) const { return v == x.v; }
};


template<class T> struct js_tuple_element { using type = T; };
template<> struct js_tuple_element<bytes_view> { using type = WVector<uint8_t>; };
template<class T> struct js_tuple_element<array_view<T>> { using type = WVector<T>; };
template<> struct js_tuple_element<writable_bytes_view> { using type = WVector<uint8_t>; };
template<class T> struct js_tuple_element<writable_array_view<T>> { using type = WVector<T>; };

template<class>
struct js_to_tuple;

template<class... Ts>
struct js_to_tuple<void(Ts...)>
{
    using type = std::tuple<typename js_tuple_element<Ts>::type...>;
};


template<class T> struct js_function_param { using type = T; };
template<> struct js_function_param<bytes_view> { using type = emscripten::val; };
template<class T> struct js_function_param<array_view<T>> { using type = emscripten::val; };
template<> struct js_function_param<writable_bytes_view> { using type = emscripten::val; };
template<class T> struct js_function_param<writable_array_view<T>> { using type = emscripten::val; };

template<class PtrFunc>
struct js_function_ptr;

template<class R, class... Ts>
struct js_function_ptr<R(*)(Ts...)>
{
    using type = R(*)(typename js_function_param<Ts>::type...);
};

template<class PtrFunc>
using js_function_ptr_t = typename js_function_ptr<PtrFunc>::type;


//@{
#define MAKE_BINDING_CPP_STRUCT(r, data, elem) MAKE_BINDING_CPP_STRUCT_I elem

#define MAKE_BINDING_CPP_STRUCT_I(type, ...) \
    BOOST_PP_CAT(MAKE_BINDING_CPP_STRUCT_I_, type)(__VA_ARGS__)

#define MAKE_BINDING_CPP_STRUCT_I_c(classname)                               \
    struct classname {                                                       \
        friend std::ostream& operator<<(std::ostream& out, classname const&) \
        { return out << #classname "{}"; }                                   \
        bool operator == (classname const) const { return true; }            \
    };

#define MAKE_BINDING_CPP_STRUCT_I_x(classname, ...)                            \
    struct classname : decltype(js_to_tuple<void(__VA_ARGS__)>::type{})        \
    {                                                                          \
        using decltype(js_to_tuple<void(__VA_ARGS__)>::type{})::tuple;         \
        friend std::ostream& operator<<(std::ostream& out, classname const& x) \
        { out << #classname << "{"; print_tuple(out, x); return out << "}"; }  \
    };

#define MAKE_BINDING_CPP_STRUCT_I_x_f(classname, body_func, ...) \
    MAKE_BINDING_CPP_STRUCT_I_x(classname, __VA_ARGS__)
//@}

//@{
#define MAKE_BINDING_LIST_NAME(r, struct_namespace, elem) \
    MAKE_BINDING_LIST_NAME_II(struct_namespace, MAKE_BINDING_LIST_NAME_I elem)
#define MAKE_BINDING_LIST_NAME_I(type, ...) __VA_ARGS__, _
#define MAKE_BINDING_LIST_NAME_II(...) MAKE_BINDING_LIST_NAME_III(__VA_ARGS__)
#define MAKE_BINDING_LIST_NAME_III(struct_namespace, classname, ...) \
    , struct_namespace::classname
//@}

//@{
#define MAKE_BINDING_FUNCTION_PTR(r, channel_type, elem) .function_ptr( \
    MAKE_BINDING_FUNCTION_PTR_II(channel_type, MAKE_BINDING_FUNCTION_PTR_I elem))

#define MAKE_BINDING_FUNCTION_PTR_I(...) __VA_ARGS__

#define MAKE_BINDING_FUNCTION_PTR_II(...) MAKE_BINDING_FUNCTION_PTR_III(__VA_ARGS__)

#define MAKE_BINDING_FUNCTION_PTR_III(channel_type, type, ...) \
    BOOST_PP_CAT(MAKE_BINDING_FUNCTION_PTR_I_, type)(channel_type, __VA_ARGS__)

#define MAKE_BINDING_FUNCTION_PTR_I_c(channel_type, classname) \
    #classname, +[](channel_type& channel_ctx_) {              \
        channel_ctx_.datas.push_back(structs::classname{});    \
    }

#define MAKE_BINDING_FUNCTION_PTR_I_x(channel_type, classname, ...)                  \
    #classname, static_cast<js_function_ptr_t<void(*)(channel_type&, __VA_ARGS__)>>( \
        [](channel_type& channel_ctx_, auto... args) {                               \
            channel_ctx_.datas.push_back(structs::classname{args...});               \
        }                                                                            \
    )

#define MAKE_BINDING_FUNCTION_PTR_I_x_f(channel_type, classname, body_func, ...) \
    #classname, []{                                                              \
        struct FRet_ {                                                           \
            static auto impl(channel_type&, __VA_ARGS__) { body_func; }          \
        };                                                                       \
        return static_cast<js_function_ptr_t<decltype(&FRet_::impl)>>(           \
            [](channel_type& channel_ctx_, auto... args) {                       \
                channel_ctx_.datas.push_back(structs::classname{args...});       \
                return FRet_::impl(channel_ctx_, args...);                       \
            }                                                                    \
        );                                                                       \
    }()
//@}

//@{
#define MAKE_BINDING_JS_FUNCTION(r, data, elem) \
    MAKE_BINDING_JS_FUNCTION_II(MAKE_BINDING_JS_FUNCTION_I elem)

#define MAKE_BINDING_JS_FUNCTION_I(type, ...) __VA_ARGS__, 0
#define MAKE_BINDING_JS_FUNCTION_II(...) MAKE_BINDING_JS_FUNCTION_III(__VA_ARGS__)
#define MAKE_BINDING_JS_FUNCTION_III(classname, ...) \
    classname: function(...args) { return this.self_.classname(...args); },
//@}

// c = no parameter
// x = with parameter
// x_f = with parameter and return
// MAKE_BINDING_CALLBACKS(ChannelType, ModChannelDataType,
//     ((c, funcname)),
//     ((x, funcname, type1 [name1], type2 [name2], ...)),
//     ((x_f, funcname, body_with_return, type1 [name1], type2 [name2], ...)),
//     ...
// )
#define MAKE_BINDING_CALLBACKS(ChannelType, ModChannelDataType, S)           \
    namespace test_channel_data                                              \
    {                                                                        \
        namespace ChannelType ## _structs                                    \
        {                                                                    \
            BOOST_PP_SEQ_FOR_EACH(MAKE_BINDING_CPP_STRUCT, _, S)             \
        }                                                                    \
                                                                             \
        struct ChannelType ## _js                                            \
        {                                                                    \
            using vector_t = std::vector<test_channel::Variant<              \
                ModChannelDataType                                           \
                BOOST_PP_SEQ_FOR_EACH(                                       \
                    MAKE_BINDING_LIST_NAME,                                  \
                    ChannelType ## _structs,                                 \
                    S)                                                       \
            >>;                                                              \
            using ChannelPtr = std::unique_ptr<ChannelType>;                 \
            vector_t datas;                                                  \
            test_channel::TestBindingCallback<                               \
                ModChannelDataType, vector_t> cb;                            \
            ChannelPtr channel_ptr;                                          \
                                                                             \
            ChannelType ## _js(                                              \
                emscripten::val&& val,                                       \
                intptr_t ifunc, intptr_t ictx)                               \
            : cb(datas)                                                      \
            , channel_ptr((*reinterpret_cast<ChannelPtr(*)(                  \
                Callback&, emscripten::val&, intptr_t                        \
            )>(ifunc))(cb, val, ictx))                                       \
            {}                                                               \
                                                                             \
            struct TestContext                                               \
            {                                                                \
                using ferr_type = void(unsigned i, std::size_t n);           \
                ferr_type* ferr;                                             \
                vector_t& datas;                                             \
                unsigned i = 0;                                              \
                                                                             \
                template<class F>                                            \
                void operator = (F f)                                        \
                {                                                            \
                    f(*this);                                                \
                    if (i < datas.size()) {                                  \
                        ferr(i, datas.size());                               \
                    }                                                        \
                    datas.clear();                                           \
                }                                                            \
            };                                                               \
                                                                             \
            TestContext test_context(TestContext::ferr_type* ferr)           \
            {                                                                \
                return TestContext{ferr, datas};                             \
            }                                                                \
        };                                                                   \
    }                                                                        \
                                                                             \
    EMSCRIPTEN_BINDINGS(test_channel_)                                       \
    {                                                                        \
        namespace structs = test_channel_data::ChannelType ## _structs;      \
        using Class = test_channel_data::ChannelType ## _js;                 \
        redjs::class_<Class>(#ChannelType "_test")                           \
            .constructor<emscripten::val&&, intptr_t, intptr_t>()            \
            .function_ptr("as_ptr", [](Class& self){                         \
                return reinterpret_cast<uintptr_t>(&self);                   \
            })                                                               \
            BOOST_PP_SEQ_FOR_EACH(                                           \
                MAKE_BINDING_FUNCTION_PTR,                                   \
                test_channel_data::ChannelType ## _js,                       \
                S                                                            \
            )                                                                \
        ;                                                                    \
    }                                                                        \
                                                                             \
    template<class... Ts>                                                    \
    std::unique_ptr<test_channel_data::ChannelType ## _js>                   \
    ChannelType ## _ctx(Ts&&... xs)                                          \
    {                                                                        \
        auto make_channel = [&](Callback& cb, emscripten::val& val) {        \
            return std::unique_ptr<ChannelType>(new ChannelType{             \
                cb, std::move(val), static_cast<Ts&&>(xs)...});              \
        };                                                                   \
                                                                             \
        auto func = [](Callback& cb, emscripten::val& val, intptr_t maker) { \
            auto& make = *reinterpret_cast<decltype(make_channel)*>(maker);  \
            return make(cb, val);                                            \
        };                                                                   \
                                                                             \
        auto iptr = RED_EM_ASM_INT(                                          \
            {                                                                \
                const callbacks = ({                                         \
                    BOOST_PP_SEQ_FOR_EACH(MAKE_BINDING_JS_FUNCTION, _, S)    \
                });                                                          \
                const mod = new Module.ChannelType ## _test(                 \
                    callbacks, $0, $1);                                      \
                callbacks.self_ = mod;                                       \
                return mod.as_ptr();                                         \
            },                                                               \
            reinterpret_cast<intptr_t>(+func),                               \
            reinterpret_cast<intptr_t>(&make_channel)                        \
        );                                                                   \
                                                                             \
        return std::unique_ptr<test_channel_data::ChannelType ## _js>(       \
            reinterpret_cast<test_channel_data::ChannelType ## _js*>(iptr)   \
        );                                                                   \
    }

template<class Ch, class Tr, class... Ts>
void print_tuple(std::basic_ostream<Ch, Tr>& out, std::tuple<Ts...> const& t)
{
    std::apply([&](auto const& x, auto const&... xs){
        out << x;
        ((out << ", " << xs), ...);
    }, t);
}

template<class... Ts>
struct Variant
{
    std::variant<Ts...> variant;

    template<class T>
    Variant(T&& x)
    : variant(static_cast<T&&>(x))
    {}

    template<class T>
    bool operator == (T const& other) const
    {
        T const* value = std::get_if<T>(&variant);
        return value && *value == other;
    }

    template<class... Us>
    friend std::ostream& operator<<(std::ostream& out, Variant<Us...> const& variant)
    {
        std::visit([&](auto const& value) { out << value; }, variant.variant);
        return out;
    }
};

template<class ChannelData, class Datas>
struct TestBindingCallback : Callback
{
    void rdp_input_invalidate(Rect /*unused*/) override {}
    void rdp_input_mouse(
        int /*unused*/, int /*unused*/, int /*unused*/, Keymap2* /*unused*/) override {}
    void rdp_input_scancode(
        long /*unused*/, long /*unused*/, long /*unused*/, long /*unused*/, Keymap2* /*unused*/)
        override {}
    void rdp_input_synchronize(
        uint32_t /*unused*/, uint16_t /*unused*/, int16_t /*unused*/, int16_t /*unused*/)
        override {}
    void refresh(Rect /*unused*/) override {}
    void rdp_gdi_down() override {}
    void rdp_gdi_up_and_running() override {}

    void send_to_mod_channel(
        CHANNELS::ChannelNameId front_channel_name, InStream & chunk,
        std::size_t length, uint32_t flags) override
    {
        this->datas.push_back(ChannelData{
            front_channel_name, chunk.remaining_bytes(), length, flags
        });
    }

    TestBindingCallback(Datas& datas)
    : datas(datas)
    {}

    Datas& datas;
};

#define CTX_CHECK_DATAS(ctx)                           \
    ctx->test_context([](unsigned i, std::size_t n){   \
        RED_CHECK_MESSAGE(false, "there is "           \
            << n << " elements, " << i << " checked"); \
    }) = [&]([[maybe_unused]] auto& _data_checker)

#define CHECK_NEXT_DATA(...) do {                                     \
    if (_data_checker.i >= _data_checker.datas.size()) {              \
        RED_REQUIRE(_data_checker.i < _data_checker.datas.size());    \
        return;                                                       \
    }                                                                 \
    {                                                                 \
        auto& _current_data = _data_checker.datas[_data_checker.i++]; \
        RED_CHECK(_current_data == (__VA_ARGS__));                    \
    }                                                                 \
} while (0)

}
}
