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

#include "proto/desc.hpp"
#include "utils/only_type.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/bytes_view.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"
#include "mpl/kvasir/mpl/mpl.hpp"

#include <iomanip>
#include <iostream>
#include <string_view>

#define FWD(x) static_cast<decltype(x)&&>(x)


template<class T>
std::string_view type_name()
{
    std::string_view sv(__PRETTY_FUNCTION__);
#ifdef __clang__
    sv.remove_suffix(1);
    sv.remove_prefix(34);
#elif defined(__GNUG__)
    sv.remove_suffix(50);
    sv.remove_prefix(39);
#endif
    return sv;
}

template<class T>
std::string_view type_name(T const&)
{
    return type_name<T>();
}

template<class... Xs>
void print(Xs const&... xs)
{
    ((std::cout << xs), ...);
}

template<class... Xs>
void println(Xs const&... xs)
{
    print(xs..., "\n");
}

template<class Xs>
void print_type(char const* prefix = "")
{
    println(prefix, type_name<Xs>());
}

template<class... Xs>
void print_def(proto::tuple<Xs...> const&)
{
    ((println("  ", type_name<Xs>())), ...);
}


namespace proto
{
    template<template<class...> class Concept>
    struct concept_check
    {
        template<class... Xs>
        struct t
        {
            static_assert(Concept<Xs...>::value);
            static const bool value = true;
        };
    };

#define PROTO_ASSERT_TYPES(concept, ...) static_assert(( \
    ::proto::concept_check<concept>::t<__VA_ARGS__>::value && ...));

    template<class ...Data>
    auto raw_data(Data const& ...data)
    {
        PROTO_ASSERT_TYPES(is_data, Data);
        return tuple<Data...>{data...};
    }

    template<class T> struct type_only {};
    template<class T> struct value_only {};
    struct start_range { using proto_basic_type = start_range; };
    struct stop_range { using proto_basic_type = stop_range; };
}

template<class...> struct names {};

using proto::name_t;

template<class T>
struct wrap
{
    T value;
};

template<class... Name>
void print_names(names<Name...>)
{
    (print(type_name<Name>(), ", "), ...);
}

template<class Name, class X>
X& get_value(wrap<proto::value<X, Name> const&> const& v)
{
    return v.get().value;
}

template<class Tuple, class... Name>
void print_variable(Tuple const& t, names<Name...>)
{
    (print(get_value<Name>(t), ", "), ...);
}


template<class BasicType>
struct basic_type_to_cpp_type_impl;

template<class Integral, class Endianess>
struct basic_type_to_cpp_type_impl<proto::datas::types::Integer<Integral, Endianess>>
{
    using type = Integral;
};

template<class Size, class Data, class ZeroPolicy>
struct basic_type_to_cpp_type_impl<proto::datas::types::String<Size, Data, ZeroPolicy>>
{
    using type = bytes_view;
};

template<class BasicType>
using basic_type_to_cpp_type = typename basic_type_to_cpp_type_impl<BasicType>::type;

template<class Data, class... Names, class Tuple>
auto writable_variable(proto::param<Data, Names...> const&, Tuple&& t)
{
    return basic_type_to_cpp_type<typename Data::proto_basic_type>(get_value<Names>(t)...);
}

template<class Int, class Endianess>
void write_buf(proto::datas::types::Integer<Int, Endianess> const&, OutStream& out, Int x)
{
    if constexpr (sizeof(Int) == 1) {
        out.out_uint8(x);
    }
    else {
        out.out_uint16_be(x);
    }
}

template<class Buf, class... Param, class... Xs>
void inplace_emit(Buf& buf, proto::tuple<Param...> const& def, Xs const&... xs)
{
    PROTO_ASSERT_TYPES(proto::is_param, Param);
    PROTO_ASSERT_TYPES(proto::is_value, Xs);

    proto::tuple<wrap<Xs const&>...> t{{xs}...};

    OutStream out(buf);

    ([&]{
        using data_type = typename Param::data_type;
        using basic_type = typename data_type::proto_basic_type;

        println(
            "  ", type_name<name_t<Param>>(),
            "| ", type_name<basic_type>(),
            " -> ", get_value<name_t<Param>>(t)
        );

        write_buf(basic_type{}, out, writable_variable(static_cast<Param const&>(def), t));
    }(), ...);
}


class native {};
class view_buffer {};
template<class T> struct ref { T& x; };
template<class T> ref(T&) -> ref<T>;

PROTO_IS_TYPE1(ref);


inline std::ostream& operator<<(std::ostream& out, native const&)
{
    return out << "native";
}

template<class T>
inline std::ostream& operator<<(std::ostream& out, ref<T> const& ref)
{
    return out << "ref(" << ref.x << ")";
}

template<class BasicType, class Binding, class UnCVBind>
struct real_type_impl;

template<class BasicType, class Binding>
struct real_type_impl<BasicType, Binding, native>
{
    using type = basic_type_to_cpp_type<BasicType>;
};

template<class BasicType, class Binding>
struct real_type_impl<BasicType, Binding, view_buffer>
{
    using type = bytes_view;
};

template<class BasicType, class Binding>
using real_type = typename real_type_impl<BasicType, Binding,
    std::remove_const_t<std::remove_reference_t<Binding>>>::type;

template<class Int, class Endianess>
auto read_buf(proto::datas::types::Integer<Int, Endianess>, InStream& in, native)
{
    if constexpr (sizeof(Int) == 1) {
        return in.in_uint8();
    }
    else {
        return in.in_uint16_be();
    }
}

template<class Integer, class charset>
auto read_buf(proto::datas::types::StringSize<Integer, charset>, InStream& in, native)
{
    return read_buf(Integer{}, in, native());
}

template<class StringSize, class StringData, class ZeroPolicy>
auto read_buf(proto::datas::types::String<StringSize, StringData, ZeroPolicy>, InStream& in, native)
{
    auto n = read_buf(StringSize{}, in, native());
    return in.in_skip_bytes(n);
}

template<class Int, class Endianess>
auto read_buf(proto::datas::types::Integer<Int, Endianess>, InStream& in, view_buffer)
{
    return in.in_skip_bytes(sizeof(Int));
}

template<class Data>
auto readable_variable_impl(native)
{
    return native();
}

template<class Data>
auto readable_variable_impl(view_buffer)
{
    return view_buffer();
}

template<class Data, class Name, class Tuple>
auto readable_variable(proto::param<Data, Name> const&, Tuple&& t)
{
    return readable_variable_impl<Data>(get_value<Name>(t));
}

template<class Buf, class... Params, class... Xs>
auto inplace_recv(Buf& buf, proto::tuple<Params...> const& def, Xs const&... xs)
{
    PROTO_ASSERT_TYPES(proto::is_param, Params);
    PROTO_ASSERT_TYPES(proto::is_value, Xs);
    proto::tuple<std::reference_wrapper<Xs const>...> t{{xs}...};

    InStream in(buf);

    auto f = [&](auto const& p){
        using Param = std::remove_const_t<std::remove_reference_t<decltype(p)>>;
        using data_type = typename Param::data_type;
        using basic_type = typename data_type::proto_basic_type;

        println(
            "  ", type_name<name_t<Param>>(),
            "| ", type_name<decltype(get_value<name_t<Param>>(t))>(),
            " : ", type_name<basic_type>(),
            " => ", type_name<real_type<basic_type, decltype(get_value<name_t<Param>>(t))>>()
        );

        return typename decltype(Param::name::mem()(proto::wrap_type<decltype(
            read_buf(basic_type{}, in, readable_variable(static_cast<Param const&>(def), t))
        )>()))::type{
            read_buf(basic_type{}, in, readable_variable(static_cast<Param const&>(def), t))
        };
    };
    return proto::tuple{f(static_cast<Params const&>(def))...};

    // (println("  ",
    //     type_name<typename Data::name>(), " = ",
    //     type_name<typename decltype(Data::name::mem()(proto::wrap_type<real_type<typename Data::proto_basic_type, decltype(get_value<typename Data::name>(t))>>()))::type>(),
    //     "] :",
    //     type_name<typename decltype(Data::name::mem()(proto::wrap_type<real_type<typename Data::proto_basic_type, decltype(get_value<typename Data::name>(t))>>()))::type::type>()
    //     // " <- ",
    //     // xs.value
    // ), ...);
}

// type => has_proto_basic_type
//      proto_basic_type_list (optional) -> group type pack<u8, u8>
//
// name => has_proto_label_name
//
// a := label<name>
// d := data<type>
//
// d(x) := var<T>
// nx := a(x) => named<name, value<void, x>> ????
// dn := d[a] => data<type, name>
//
// d(x) => value<type, x>
// d(nx) => value<type, named<name, x>>
// dn(x) => named<name, value<type, x>>
//

#include <array>

inline std::ostream& operator<<(std::ostream& out, bytes_view const& av)
{
    out << "[" << av.size() << "]";
    auto t = "0123456789abcdef";
    for (auto c : av) {
        out << t[c>>4] << t[c&0xf];
    }
    return out;
}

template<class Buf>
void dump(Buf& buf)
{
    auto t = "0123456789abcdef";
    print("dump(size=", buf.size(), "): 0x");
    for (auto c : buf) {
        print(t[c>>4], t[c&0xf]);
    }
    println();
}


namespace mp = kvasir::mpl;
namespace mpe = kvasir::mpl::eager;

// template<class F, class... args>
// struct bind_front
// {
//     template<class... xs>
//     using f = typename F::template f<args..., xs...>;
// };
//
// template<class F, class... args>
// struct bind_back
// {
//     template<class... xs>
//     using f = typename F::template f<xs..., args...>;
// };

template<auto i>
using tnumber = mp::integral_constant<decltype(i), i>;

template<class C = mp::identity>
struct va_plus
{
    template<class... xs>
    using f = typename C::template f<tnumber<(xs::value + ...)>>;
};

struct no_name
{
    static constexpr zstring_view _proto_name() noexcept
    {
        return "no name"_zv;
    }
};

namespace test
{
    using namespace proto;

    template<class C = mp::identity> using mp_tpl0 = mp::unpack<mp::front<C>>;
    template<class C = mp::identity> using mp_tpl1 = mp::unpack<mp::at1<C>>;

    template<class Params, class Values>
    struct Definition;

    namespace detail
    {
        class discard {};

        template<class X, class Symbols>
        struct value_to_definition_param;

        template<class T, class Symbols>
        struct value_to_definition_param<data<T>, Symbols>
        {
            using type = param<T, Symbols>;
        };

        template<class T, class Symbols>
        struct value_to_definition_param<type_only<T>, Symbols>
        {
            using type = param<T, Symbols>;
        };

        template<template<class...> class Tpl, class Symbols>
        struct value_to_definition_param<value_data<Tpl>, Symbols>
        {
            using type = discard;
        };

        template<class Symbols>
        struct value_to_definition_param<start_range, Symbols>
        {
            using type = discard;
        };

        template<class Symbols>
        struct value_to_definition_param<stop_range, Symbols>
        {
            using type = discard;
        };


        template<class X, class Symbols>
        struct value_to_definition_value;

        template<class T, class Symbols>
        struct value_to_definition_value<type_only<T>, Symbols>
        {
            using type = discard;
        };

        template<class T, class Symbols>
        struct value_to_definition_value<data<T>, Symbols>
        {
            using type = lazy_value<as_param, Symbols>;
        };

        template<template<class...> class Tpl, class Symbols>
        struct value_to_definition_value<value_data<Tpl>, Symbols>
        {
            using type = lazy_value<Tpl<as_param>, Symbols>;
        };

        template<class Symbols>
        struct value_to_definition_value<start_range, Symbols>
        {
            using type = lazy_value<start_range, Symbols>;
        };

        template<class Symbols>
        struct value_to_definition_value<stop_range, Symbols>
        {
            using type = lazy_value<stop_range, Symbols>;
        };


        template<class V>
        using value_to_definition_value_t = typename detail::value_to_definition_value<
            std::decay_t<value_type_t<V>>, name_t<V>>::type;

        template<class V>
        using value_to_definition_param_t = typename detail::value_to_definition_param<
            std::decay_t<value_type_t<V>>, name_t<V>>::type;


        template<class T> using type_t = typename T::type;
        template<class T> using names_t = typename T::names;

        template<class C, class... xs>
        using mpc_remove_discard = mp::call<
            mp::remove_if<
                mp::same_as<detail::discard>,
                C
            >,
            xs...
        >;

        // memoise for clang
        template<class T, size_t n>
        struct filled_sequence_impl
        {
            using type = mp::call<
                mp::make_int_sequence<mp::transform<mp::always<T>>>,
                mp::uint_<n>>;
        };

        template<class T, size_t n>
        using filled_sequence = typename filled_sequence_impl<T, n>::type;

        template<class x, class... xs>
        constexpr bool _contains_v = (... && !std::is_same<x, xs>::value);

        template<class... x>
        struct set_difference
        {
            template<class... y>
            using f = mp::call<mp::join<>,
                typename mp::conditional<_contains_v<x, y...>>
                ::template f<mp::list<x>, mp::list<>
            >...>;
        };

        template<class... p>
        struct param_list
        {
            template<class C = mp::listify>
            using symbols = mp::call<C, name_t<p>...>;

            struct param_must_be_unique : name_t<p>...
            {
                using list = mp::list<p...>;
            };
        };

        template<class... v>
        struct lazy_value_list
        {
            template<class C = mp::listify>
            // TODO symbols_t + join
            using symbols = mp::call<C, name_t<v>...>;

            using list = mp::list<v...>;
        };

        struct no_error
        {
            template<class...>
            using f = no_error;

            static const bool checked = true;
        };

        template<class E>
        constexpr bool check_error(E)
        {
            static_assert(E::checked); return true;
        }

        template<class x, class y, class Error>
        void check_unused_params()
        {
            using l = typename x::template symbols<
                mp::cfe<detail::set_difference,
                    mp::cfe<y::template symbols>>>;

            using errtype = typename mp::conditional<std::is_same<l, mp::list<>>::value>
                ::template f<no_error, mp::unpack<Error>>;

            static_assert(check_error(mp::call<errtype, l>{}));
        }

        template<class x, class Pred, class Error>
        void check_pred()
        {
            using errtype = typename mp::conditional<mp::call<Pred, x>::value>
                ::template f<no_error, Error>;

            static_assert(check_error(mp::call<errtype, x>{}));
        }
    }

    using detail::type_t;

#define PROTO_ERROR_C(name) struct name { template<class... ctx> struct f{}; }

    namespace errors
    {
        PROTO_ERROR_C(some_parameters_are_unused);
        PROTO_ERROR_C(some_values_are_unused);
    }

    // TODO LazyValues
    template<class Params, class Values>
    struct Definition
    {
        static Params params() { return Params{}; }
        static Values values() { return Values{}; }
    };

    template<class... Xs>
    Definition(Xs const&...) -> Definition<Xs...>;

    template<class T>
    auto type(data<T> const&)
    {
        return type_only<T>{};
    }

    inline auto start()
    {
        return start_range{};
    }

    inline auto stop()
    {
        return stop_range{};
    }

    template<class... Xs>
    auto definition(Xs const&... /*xs*/)
    {
        using params = detail::mpc_remove_discard<
            mp::cfe<detail::param_list>,
            detail::value_to_definition_param_t<Xs>...>;

        using lazy_values = detail::mpc_remove_discard<
            mp::cfe<detail::lazy_value_list>,
            detail::value_to_definition_value_t<Xs>...>;

        detail::check_unused_params<params, lazy_values, errors::some_parameters_are_unused>();
        detail::check_unused_params<lazy_values, params, errors::some_values_are_unused>();

        return Definition<
            typename params::param_must_be_unique::list,
            typename lazy_values::list
        >{};
    }


    template<class Value, class Name>
    struct variable
    {
        using value_type = Value;
        using name = Name;
        Value value;
    };

    template<class Name, class X>
    auto& get_var(variable<X, Name>& v)
    {
        return v;
    }

    template<class Int, class Endianess>
    void write_buf2(OutStream& out, proto::datas::types::Integer<Int, Endianess>, Int x)
    {
        if constexpr (sizeof(Int) == 1) {
            out.out_uint8(x);
        }
        else {
            out.out_uint16_be(x);
        }
    }

    template<class Integer, class charset>
    void write_buf2(OutStream& out, proto::datas::types::StringSize<Integer, charset>, bytes_view view)
    {
        write_buf2(out, Integer{}, value_type_t<Integer>(view.size()));
    }

    template<class charset>
    void write_buf2(OutStream& out, proto::datas::types::StringData<charset>, bytes_view view)
    {
        out.out_copy_bytes(view);
    }

    template<class StringSize, class StringData, class ZeroPolicy>
    void write_buf2(
        OutStream& out,
        proto::datas::types::String<StringSize, StringData, ZeroPolicy>,
        bytes_view view)
    {
        write_buf2(out, StringSize{}, view);
        write_buf2(out, StringData{}, view);
    }

    template<class... Ts, class F>
    constexpr auto apply(mp::list<Ts...>, F&& f)
    {
        return f(Ts{}...);
    }

    class none {};
    PROTO_IS_SAME_TYPE(none);

    namespace errors
    {
        PROTO_ERROR_C(missing_parameters);
        PROTO_ERROR_C(unknown_parameters);
    }

    namespace detail
    {
        template<class Traits, class Param, class X>
        auto make_variable2(proto::value<X, name_t<Param>> const& v)
        {
            using builder = typename Traits::template value_variable_builder<
                data_type_t<Param>,
                std::remove_reference_t<X>>;
            // TODO v.value&&
            return variable<
                decltype(builder::make(v.value)),
                name_t<Param>
            >{builder::make(v.value)};
        }

        template<class Traits, class Params, class... Xs>
        auto build_params2(Xs const&... xs)
        {
            PROTO_ASSERT_TYPES(proto::is_value, Xs);

            using params = mp::call<mp::unpack<mp::cfe<detail::param_list>>, Params>;
            using value_params = detail::param_list<Xs...>;

            detail::check_unused_params<params, value_params, errors::missing_parameters>();
            detail::check_unused_params<value_params, params, errors::unknown_parameters>();

            // TODO value_type_t must be rvalue or lvalue
            proto::tuple<Xs...> t{Xs{static_cast<value_type_t<Xs>>(xs.value)}...};

            return apply(Params{}, [&](auto... p){
                return tuple{make_variable2<Traits, decltype(p)>(t)...};
            });
        }
    }

    namespace traits
    {
        template<class T> using is_static_size_t = typename T::is_static_size;
        template<class T> using static_min_size_t = typename T::static_min_size;
        template<class T> using context_value_list_t = typename T::context_value_list;

        template<class> class lazy {};
        template<class Data, class T> struct val { T x; };

        template<class T>
        std::ostream& operator<<(std::ostream& out, lazy<T> const&)
        {
            return out << "lazy<" << type_name<T>() << ">";
        }

        template<class Data, class T>
        std::ostream& operator<<(std::ostream& out, val<Data, T> const& v)
        {
            return out << v.x;
        }


        template<class> class data_to_value_data_impl;

        template<class T>
        using data_to_value_data
            = typename data_to_value_data_impl<proto_basic_type_t<T>>::type;

        template<class StringSize, class StringData, class ZeroPolicy>
        struct data_to_value_data_impl<datas::types::String<StringSize, StringData, ZeroPolicy>>
        {
            using type = StringData;
        };

        template<class> class data_to_value_size_impl;

        template<class T>
        using data_to_value_size
            = typename data_to_value_size_impl<proto_basic_type_t<T>>::type;

        template<class StringSize, class StringData, class ZeroPolicy>
        struct data_to_value_size_impl<datas::types::String<StringSize, StringData, ZeroPolicy>>
        {
            using type = StringSize;
        };


        template<class TParams, class TCtxValues>
        struct CtxW
        {
            OutStream out;
            TParams params;
            TCtxValues ctx_values;
        };

        struct stream_writable
        {
            template<class Data, class BasicType>
            struct value_variable_builder_impl;

            template<class Data, class T>
            using value_variable_builder
                = value_variable_builder_impl<Data, proto_basic_type_t<Data>>;

            template<class BasicType, class... NamedValues>
            struct next_value;

            template<class T>
            struct final_value;

            template<class T>
            struct write_data_impl;

            template<class T>
            using write_data = write_data_impl<proto_basic_type_t<T>>;

            template<class TParams, class>
            struct LazyValueToNextValue;

            template<class TParams, class Data, class... Name>
            struct LazyValueToNextValue<TParams, lazy_value<Data, Name...>>
            {
                using type = next_value<
                    proto_basic_type_t<Data>,
                    std::remove_reference_t<decltype(get_var<Name>(std::declval<TParams>()))>...
                >;
            };

            template<class Params_, class Values_, class... Xs>
            static auto make_context(writable_bytes_view buf, Xs const&... xs)
            {
                using Params = mpe::flatten<Params_>;
                using Values = mpe::flatten<Values_>;

                using tparams = decltype(detail::build_params2<stream_writable, Params>(xs...));
                using states_list = mp::call<
                    mp::unpack<
                        mp::transform<
                            mp::push_front<tparams&, mp::cfl<LazyValueToNextValue>>,
                            mp::fork<
                                mp::transform<mp::cfe<context_value_list_t>,
                                    mp::join<mp::cfe<tuple>>>,
                                mp::listify
                            >
                        >
                    >,
                    Values
                >;

                using ctx_values = mp::eager::at<states_list, 0>;

                return CtxW<tparams, ctx_values>{
                    OutStream{buf},
                    detail::build_params2<stream_writable, Params>(xs...),
                    ctx_values{} /** TODO uninit<ctx_values> ? **/
                };
            }

            template<class Ctx, class Data, class... Name>
            static auto next_state(Ctx& ctx, lazy_value<Data, Name...>)
            {
                using Next = next_value<
                    proto_basic_type_t<Data>,
                    std::remove_reference_t<decltype(get_var<Name>(ctx.params))>...
                >;
                return Next::make(ctx, get_var<Name>(ctx.params).value...);
            }

            template<class Ctx>
            static auto final(Ctx& ctx)
            {
                return ctx.out.get_bytes();
            }
        };


        template<class Data, class Int, class Endianess>
        struct stream_writable::value_variable_builder_impl<
            Data, datas::types::Integer<Int, Endianess>>
        {
            static auto make(safe_int<Int> x)
            {
                return val<Data, Int>{x.underlying()};
            }
        };

        template<class Data, class StringSize, class StringData>
        struct stream_writable::value_variable_builder_impl<
            Data, datas::types::String<StringSize, StringData, datas::types::no_zero>>
        {
            static auto make(bytes_view str)
            {
                return val<Data, bytes_view>{str};
            }
        };

        template<class Data, class Size>
        struct stream_writable::value_variable_builder_impl<
            Data, datas::types::PktSize<Size>>
        {
            static auto make(native)
            {
                return Data{};
            }
        };


        template<class Data, class T, class Name>
        struct stream_writable::next_value<as_param, variable<val<Data, T>, Name>>
        {
            using context_value_list = mp::list<>;

            template<class Ctx>
            static bool make(Ctx& ctx, val<Data, T> const& v)
            {
                write_data<Data>::write(ctx.out, v.x);
                return true;
            }
        };

        template<class Name, class Data, class Bytes>
        struct stream_writable::next_value<
            datas::values::types::SizeBytes<as_param>,
            variable<val<Data, Bytes>, Name>>
        {
            using context_value_list = mp::list<>;

            template<class Ctx>
            static bool make(Ctx& ctx, val<Data, Bytes> const& v)
            {
                write_data<data_to_value_size<Data>>::write(ctx.out, v.x.size());
                return true;
            }
        };

        template<class Name, class Data, class Bytes>
        struct stream_writable::next_value<
            datas::values::types::Data<as_param>,
            variable<val<Data, Bytes>, Name>>
        {
            using context_value_list = mp::list<>;

            template<class Ctx>
            static bool make(Ctx& ctx, val<Data, Bytes> const& v)
            {
                write_data<data_to_value_data<Data>>::write(ctx.out, v.x);
                return true;
            }
        };

        template<class DataSize, class Name>
        using pkt_size_variable = variable<val<DataSize, std::array<uint8_t*, 2>>, Name>;

        template<class DataSize, class Name>
        struct stream_writable::next_value<as_param, variable<datas::types::PktSize<DataSize>, Name>>
        {
            using var_size = pkt_size_variable<DataSize, Name>;
            using context_value_list = mp::list<var_size>;

            template<class Ctx>
            static bool make(Ctx& ctx, datas::types::PktSize<DataSize>)
            {
                static_cast<var_size&>(ctx.ctx_values).value.x[0] = ctx.out.get_current();
                ctx.out.out_skip_bytes(sizeof(value_type_t<DataSize>));
                return true;
            }
        };

        template<class DataSize, class Name>
        struct stream_writable::next_value<start_range, variable<datas::types::PktSize<DataSize>, Name>>
        {
            using context_value_list = mp::list<>;

            template<class Ctx>
            static bool make(Ctx& ctx, datas::types::PktSize<DataSize>)
            {
                static_cast<pkt_size_variable<DataSize, Name>&>(ctx.ctx_values).value.x[1]
                    = ctx.out.get_current();
                return true;
            }
        };

        template<class DataSize, class Name>
        struct stream_writable::next_value<stop_range, variable<datas::types::PktSize<DataSize>, Name>>
        {
            // TODO automatically write size if packet is static
            // TODO compute static distance with a static packet
            using context_value_list = mp::list<>;

            template<class Ctx>
            static bool make(Ctx& ctx, datas::types::PktSize<DataSize>)
            {
                auto& ptrs = static_cast<pkt_size_variable<DataSize, Name>&>(ctx.ctx_values).value.x;
                assert(ptrs[1] <= ctx.out.get_current());
                OutStream out({ptrs[0], sizeof(value_type_t<DataSize>)});
                write_data<DataSize>::write(out, ctx.out.get_current() - ptrs[1]);
                return true;
            }
        };

        template<class Int, class Endianess>
        struct stream_writable::write_data_impl<datas::types::Integer<Int, Endianess>>
        {
            static void write(OutStream& out, Int x)
            {
                auto* p = out.out_skip_bytes(sizeof(Int)).data();
                using rng = mpe::make_int_sequence<mp::int_<sizeof(Int)>>;
                using is_little_endian = std::is_same<Endianess, datas::types::le_tag>;
                apply(rng{}, [&](auto... i) {
                    ((
                        *p++ = x >> ((is_little_endian::value ? i.value : sizeof(Int)-1u-i.value) * 8u)
                    ), ...);
                });
            }
        };

        template<>
        struct stream_writable::write_data_impl<datas::types::StringData<proto::datas::types::ascii_charset>>
        {
            static void write(OutStream& out, bytes_view x)
            {
                out.out_copy_bytes(x);
            }
        };

        template<class CheckedSizes>
        struct CheckedRanges
        {
            using size_type = std::size_t;

            CheckedRanges(size_type size)
            {
                this->unchecked_sizes[0] = size;
            }

            void push(size_type size)
            {
                ++this->unchecked_pos;
                this->unchecked_sizes[this->unchecked_pos] = size;
            }

            void pop()
            {
                --this->unchecked_pos;
            }

            size_type& unchecked_size()
            {
                return this->unchecked_sizes[this->unchecked_pos];
            }

            template<class Name>
            auto& get(Name)
            {
                return get_var<Name>(this->states).value;
            }

        private:
            // TODO optimal size:
            // start1 stop1 start2 stop2 -> size = 1
            std::array<size_type, mp::eager::size<CheckedSizes>::value> unchecked_sizes;
            size_type unchecked_pos = 0;
            mp::call<mp::unpack<mp::cfe<tuple>>, CheckedSizes> states;
        };

        // TODO C++20: class SizeCtx, SizeCtx const& size_ctx -> auto size_ctx
        template<class TParams, class TCtxValues, class ErrorFn, class SizeCtx, SizeCtx const& size_ctx>
        struct Ctx
        {
            InStream in;
            std::array<byte_ptr, size_ctx.rng_stack_size> end_stream_stack;
            std::array<byte_ptr, size_ctx.ptr_stack_size> start_ptr_stack;
            TParams params;
            TCtxValues ctx_values;
            ErrorFn error;

            void save_stream(int i, std::size_t new_size)
            {
                auto remain = in.remaining_bytes();
                end_stream_stack[i] = remain.end();
                in = InStream(remain.first(new_size));
            }

            void restore_stream(int i)
            {
                in = InStream({in.get_current(), end_stream_stack[i]});
            }

            void save_start_pos(int i)
            {
                start_ptr_stack[i] = in.get_current();
            }

            std::ptrdiff_t get_start_distance(int i)
            {
                return in.get_current() - start_ptr_stack[i];
            }

            static constexpr decltype(size_ctx.size_infos) const&
            size_infos = size_ctx.size_infos;
        };


        struct stream_readable2
        {
            template<class Data, class BasicType>
            struct value_variable_builder_impl;

            template<class Data, class T>
            using value_variable_builder
                = value_variable_builder_impl<Data, proto_basic_type_t<Data>>;

            // TODO next_value<BasicType, variable<V, Names...>...>::type
            // -> next_value_impl<BasicType, Names...>
            template<class BasicType, class... Variable>
            struct next_value;

            struct reader_impl;

            template<class T>
            using reader = reader_impl;

            template<class T>
            struct read_data_impl;

            template<class T>
            using read_data = read_data_impl<proto_basic_type_t<T>>;

            template<class TParams, class>
            struct LazyValueToNextValue;

            template<class TParams, class Data, class... Name>
            struct LazyValueToNextValue<TParams, lazy_value<Data, Name...>>
            {
                using type = next_value<
                    proto_basic_type_t<Data>,
                    std::remove_reference_t<decltype(get_var<Name>(std::declval<TParams>()))>...>;
            };

            template<class>
            struct next_value_is_start_range
            : std::false_type
            {};

            template<class... Ts>
            struct next_value_is_start_range<next_value<start_range, Ts...>>
            : std::true_type
            {};

            template<class>
            struct next_value_is_range
            : std::false_type
            {};

            template<class... Ts>
            struct next_value_is_range<next_value<start_range, Ts...>>
            : std::true_type
            {};

            template<class... Ts>
            struct next_value_is_range<next_value<stop_range, Ts...>>
            : std::true_type
            {};

            template<class>
            struct next_value_is_pkt_size
            : std::false_type
            {};

            template<class DataSize, class... Ts>
            struct next_value_is_pkt_size<
                next_value<as_param,
                    variable<lazy<datas::types::PktSize<DataSize>>, Ts...>>>
            : std::true_type
            {};

            template<class>
            struct next_value_to_names;

            template<class Data, class T, class... Names>
            struct next_value_to_names<next_value<Data, variable<T, Names...>>>
            {
                using type = mp::list<Names...>;
            };

            template<class x>
            using next_value_to_names_t = typename next_value_to_names<x>::type;

            using lazy_value_to_name_list_if_start_range = mp::if_<
                mp::cfe<next_value_is_start_range>,
                mp::cfl<next_value_to_names, mp::listify>,
                mp::always<mp::list<>>
            >;

            template<int SizeOrRange, bool IsSize, bool IsStaticSize, int RangeBinding>
            struct size_info_t
            {
                static constexpr bool is_size = IsSize;
                // if false, size_or_range is static_min_size
                static constexpr bool is_static_size = IsStaticSize;
                static constexpr int size_or_range = SizeOrRange;
                static constexpr int range_binding = RangeBinding;
            };

            template<class... Names>
            struct indexed_start_range
            {
                template<class SearchNames>
                static constexpr int idx_names = sizeof...(Names) - int(
                    mp::find_if<mp::same_as<SearchNames>, mp::size<>>
                    ::template f<Names...>
                    ::value
                );

                struct range_to_size_info
                {
                    template<class NextValue>
                    using f = size_info_t<
                        idx_names<next_value_to_names_t<NextValue>>,
                        false,
                        true,
                        -1
                    >;
                };

                struct pkt_size_to_size_info
                {
                    template<class NextValue>
                    using f = size_info_t<
                        static_min_size_t<NextValue>::value,
                        true,
                        is_static_size_t<NextValue>::value,
                        idx_names<next_value_to_names_t<NextValue>>
                    >;
                };

                struct regular_to_size_info
                {
                    template<class NextValue>
                    using f = size_info_t<
                        static_min_size_t<NextValue>::value,
                        true,
                        is_static_size_t<NextValue>::value,
                        -1
                    >;
                };

                // to_size_or_negatif_index_range
                using type = mp::if_<
                    mp::cfe<next_value_is_range>,
                    // negative index
                    range_to_size_info,
                    mp::if_<
                        mp::cfe<next_value_is_pkt_size>,
                        pkt_size_to_size_info,
                        regular_to_size_info
                    >
                >;
            };

            struct RangeInfo
            {
                int idx_value = -1;
                int idx_start = -1;
                int idx_stop = -1;

                friend std::ostream& operator<<(std::ostream& out, RangeInfo const& info)
                {
                    return out << "RangeInfo:"
                        << "  idx_value: " << std::setw(2) << info.idx_value
                        << "  idx_start: " << std::setw(2) << info.idx_start
                        << "  idx_stop: " << std::setw(2) << info.idx_stop
                    ;
                }
            };

            struct SizeInfo
            {
                int size;
                int accu_size;
                bool is_static_size;
                bool is_rng_rstatic_size;
                int idx_range;
                bool is_inclusive_range = true;
                int rng_raccu = -1;
                int rng_raccu_next = -1;
                int start_ptr_pos = -1;
                int end_stream_pos = -1;

                friend std::ostream& operator<<(std::ostream& out, SizeInfo const& info)
                {
                    return out << "SizeInfo:"
                        << "  size: " << std::setw(2) << info.size
                        << "  accu_size: " << std::setw(2) << info.accu_size
                        << "  is_static_size: " << std::setw(2) << info.is_static_size
                        << "  is_rng_rstatic_size: " << std::setw(2) << info.is_rng_rstatic_size
                        << "  idx_range: " << std::setw(2) << info.idx_range
                        << "  is_inclusive_range: " << std::setw(2) << info.is_inclusive_range
                        << "  rng_raccu: " << std::setw(2) << info.rng_raccu
                        << "  rng_raccu_next: " << std::setw(2) << info.rng_raccu_next
                        << "  start_ptr_pos: " << std::setw(2) << info.start_ptr_pos
                        << "  end_stream_pos: " << std::setw(2) << info.end_stream_pos
                    ;
                }
            };

            template<std::size_t N>
            struct SizeCtx
            {
                std::array<SizeInfo, N> size_infos;
                int rng_stack_size;
                int ptr_stack_size;
            };

            template<std::size_t N>
            struct size_ctx_stack
            {
                int stack[N];
                int i = 0;

                constexpr void push(int x)
                {
                    stack[i++] = x;
                }

                constexpr int pop()
                {
                    return stack[--i];
                }

                constexpr int size() const
                {
                    return i;
                }
            };

            template<class... SizeOrRange>
            struct size_ctx_creator
            {
                constexpr static auto make_size_infos()
                {
                    // println(type_name<compute_sizes>());

                    const std::size_t range_count
                        = (sizeof...(SizeOrRange) - (SizeOrRange::is_size + ...)) / 2;

                    RangeInfo ranges[range_count+1] {
                        {-1, 0, sizeof...(SizeOrRange)-1},
                    };

                    // init ranges
                    {
                        int i = 0;
                        ((
                            (not SizeOrRange::is_size ? void(
                                ranges[SizeOrRange::size_or_range+1].idx_start == -1
                                    ? void(ranges[SizeOrRange::size_or_range+1].idx_start = i)
                                    : void(ranges[SizeOrRange::size_or_range+1].idx_stop = i)
                            ) : void()),
                            void(
                                SizeOrRange::range_binding != -1 ? void(
                                    ranges[SizeOrRange::range_binding+1].idx_value = i
                                ) : void()
                            ),
                            ++i
                        ), ...);
                    }

                    // /**/for (auto const& p : ranges) println(p);


                    SizeCtx<sizeof...(SizeOrRange)> result{
                        {(SizeOrRange::is_size
                            ? SizeInfo{
                                SizeOrRange::size_or_range, 0,
                                SizeOrRange::is_static_size,
                                SizeOrRange::is_static_size,
                                SizeOrRange::range_binding+1,}
                            : SizeInfo{0, 0, 1, 1,
                                SizeOrRange::size_or_range+1,}
                        )...},
                        0, 0
                    };
                    auto& sizes = result.size_infos;

                    // init is_inclusive_range
                    for (auto const& rng : ranges)
                    {
                        if (rng.idx_value < rng.idx_start and rng.idx_value != -1)
                        {
                            sizes[rng.idx_value].is_inclusive_range = false;
                            sizes[rng.idx_start].is_inclusive_range = false;
                            sizes[rng.idx_stop].is_inclusive_range = false;
                        }
                    }

                    // init accu_size
                    {
                        int accu = 0;
                        for (auto& sz : sizes)
                        {
                            accu += sz.size;
                            sz.accu_size = accu;
                        }
                    }

                    // init is_rng_rstatic_size
                    {
                        size_ctx_stack<range_count> is_rng_static_size_stack{};
                        int is_rng_static_size = 1;

                        int i = std::size(sizes)-1;
                        for (; i >= 0; --i)
                        {
                            auto& sz = sizes[i];

                            if (sz.idx_range)
                            {
                                auto& rng = ranges[sz.idx_range];
                                if (&sizes[rng.idx_start] == &sz)
                                {
                                    is_rng_static_size &= is_rng_static_size_stack.pop();
                                }
                                else if (&sizes[rng.idx_stop] == &sz)
                                {
                                    is_rng_static_size_stack.push(is_rng_static_size);
                                    is_rng_static_size = 1;
                                }
                            }
                            is_rng_static_size &= sz.is_static_size;
                            sz.is_rng_rstatic_size = is_rng_static_size;
                        }
                    }

                    // init idx_range, rng_raccu, rng_stack_size, ptr_stack_size
                    // start_ptr_pos, end_stream_pos
                    {
                        size_ctx_stack<range_count> idx_rng_stack{};
                        int idx_rng = 0;

                        size_ctx_stack<range_count> count_ptr_stack{};
                        int count_ptr = 0;

                        for (auto& sz : sizes)
                        {
                            if (sz.idx_range)
                            {
                                auto& rng = ranges[sz.idx_range];
                                if (&sizes[rng.idx_start] == &sz)
                                {
                                    if (sz.is_inclusive_range)
                                    {
                                        sz.start_ptr_pos = count_ptr;
                                    }
                                    sz.end_stream_pos = idx_rng_stack.size();
                                    idx_rng_stack.push(idx_rng);
                                    idx_rng = sz.idx_range;
                                    result.rng_stack_size = std::max(result.rng_stack_size, idx_rng_stack.size());
                                }
                                else if (&sizes[rng.idx_stop] == &sz)
                                {
                                    idx_rng = idx_rng_stack.pop();
                                    sz.end_stream_pos = idx_rng_stack.size();
                                    count_ptr = count_ptr_stack.pop();
                                }
                                else
                                {
                                    count_ptr_stack.push(count_ptr);
                                    if (sz.is_inclusive_range)
                                    {
                                        sz.start_ptr_pos = count_ptr;
                                        sz.end_stream_pos = idx_rng_stack.size()-1;
                                        ++count_ptr;
                                        result.ptr_stack_size = std::max(result.rng_stack_size, count_ptr);
                                    }
                                }
                            }
                            else
                            {
                                sz.idx_range = idx_rng;
                            }

                            auto& rng = ranges[sz.idx_range];
                            sz.rng_raccu = sizes[rng.idx_stop].accu_size - sz.accu_size + sz.size;
                        }
                    }

                    // init rng_raccu_next
                    for (auto const& rng : ranges)
                    {
                        auto& sz = sizes[rng.idx_stop];
                        if (&sz != &sizes.back())
                        {
                            sz.rng_raccu_next = (&sz+1)->rng_raccu;
                        }
                    }

                    // /**/ for (auto const& sz : sizes) println(sz);

                    return result;
                }

                constexpr static SizeCtx<sizeof...(SizeOrRange)> size_ctx = make_size_infos();
            };

            using make_compute_sizes = mp::fork_front<
                mp::transform<
                    lazy_value_to_name_list_if_start_range,
                    mp::join<
                        mp::cfl<
                            indexed_start_range,
                            mp::fork<
                                mp::identity,
                                mp::always<mp::cfe<size_ctx_creator>>,
                                mp::cfe<mp::transform>
                            >
                        >
                    >
                >,
                mp::cfe<mp::call>
            >;

            template<class Params, class Values, class ErrorFn, class... Xs>
            static auto make_context(bytes_view buf, ErrorFn& error, Xs const&... xs)
            {
                using tparams = decltype(detail::build_params2<stream_readable2, Params>(xs...));
                using states_list = mp::call<
                    mp::unpack<
                        mp::transform<
                            mp::push_front<tparams&, mp::cfl<LazyValueToNextValue>>,
                            mp::fork<
                                mp::transform<mp::cfe<context_value_list_t>,
                                    mp::join<mp::cfe<tuple>>>,
                                make_compute_sizes,
                                mp::listify
                            >
                        >
                    >,
                    Values
                >;

                using SizeCtx = mp::eager::at<states_list, 1>;

                constexpr auto& size_ctx = SizeCtx::size_ctx;
                constexpr auto& size_infos = size_ctx.size_infos;

                println("rng_stack_size: ", size_ctx.rng_stack_size);
                println("ptr_stack_size: ", size_ctx.ptr_stack_size);
                println(type_name(size_infos));
                for (auto& sz_infos : size_infos) {
                    println(sz_infos);
                }

                using ctx_values = mp::eager::at<states_list, 0>;

                constexpr auto min_size = size_infos.back().accu_size;

                print("min_size: ", min_size, "\n\n");

                if (buf.size() < min_size) {
                    error(no_name{}, min_size, buf.size());
                }

                return Ctx<tparams, ctx_values, ErrorFn&, decltype(SizeCtx::size_ctx), SizeCtx::size_ctx>{
                    InStream{buf}, {}, {},
                    detail::build_params2<stream_readable2, Params>(xs...),
                    ctx_values{} /** TODO uninit<ctx_values> ? **/, error
                };
            }

            // TODO rename to apply_state
            template<class Ctx, class I, class Data, class... Name>
            static auto next_state(Ctx& ctx, mp::list<I, lazy_value<Data, Name...>>)
            {
                using Next = next_value<
                    proto_basic_type_t<Data>,
                    std::remove_reference_t<decltype(get_var<Name>(ctx.params))>...>;
                println(type_name<Next>());
                return Next::make(ctx, I{}, get_var<Name>(ctx.params).value...);
            }

            template<class EV, class... V>
            struct grouped_values
            {
                using strong_group_members = mp::list<V...>;
                using volatile_final_members = EV;
            };

            template<class state, class v>
            struct split_after_push;

            template<class... Ts, class... Us, class... v>
            struct split_after_push<
                mp::list<mp::list<Ts...>, Us...>,
                mp::list<mp::false_, v...>>
            {
                using type = mp::list<
                    mp::list<>,
                    Us..., mp::list<Ts..., mp::list<v...>>
                >;
            };

            template<class... Ts, class... Us, class... v>
            struct split_after_push<
                mp::list<mp::list<Ts...>, Us...>,
                mp::list<mp::true_, v...>>
            {
                using type = mp::list<mp::list<Ts..., mp::list<v...>>, Us...>;
            };

            // [void, void, int, int, void, int] -> [[void, void, int], [int], [void, int]]
            template<class Ctx>
            struct group_by_return_not_void
            {
                template<class... ints>
                struct indexed_group
                {
                    template<class... values>
                    using f = typename mp::fold_left<
                        mp::cfl<split_after_push>,
                        mp::unpack<mp::cfe<grouped_values>>
                    >::template f<
                        mp::list<mp::list<>>,
                        mp::list<
                            mp::bool_<std::is_same_v<void, decltype(next_state(
                                std::declval<Ctx&>(), mp::list<ints, values>{}
                            ))>>,
                            ints,
                            values
                        >...
                    >;
                };
            };

            template<class Values, class Ctx>
            using context_members = mp::call<
                mp::unpack<
                    mp::fork_front<
                        mp::size<mp::make_int_sequence<
                            mp::cfe<group_by_return_not_void<Ctx>::template indexed_group>
                        >>,
                        mp::cfe<mp::call>
                    >
                >
              , Values>;


            template<class T, class... Name>
            struct named : T
            {};

            template<class Name, class T>
            static T& get_named(named<T, Name>& x)
            {
                return x;
            }

            template<class Name, class T>
            static T const& get_named(named<T, Name> const& x)
            {
                return x;
            }

            template<class Tuple>
            struct tuple_mem : Tuple
            {
                template<class T>
                auto& operator[](T const&)
                {
                    return get_named<T>(*this);
                }

                template<class T>
                auto const& operator[](T const&) const
                {
                    return get_named<T>(*this);
                }
            };

            template<class Ctx, class Tuple>
            static auto final(Ctx& /*ctx*/, Tuple&& t)
            {
                // TODO sort by alignement ?
                return tuple_mem<Tuple>{static_cast<Tuple&&>(t)};
            }
        };


        template<class Data, class Int, class Endianess>
        struct stream_readable2::value_variable_builder_impl<
            Data, datas::types::Integer<Int, Endianess>>
        {
            static auto make(native)
            {
                return lazy<Data>{};
            }

            static auto make(ref<Int> r)
            {
                return val<Data, Int&>{r.x};
            }
        };

        template<class Data, class StringSize, class StringData, class ZeroPolicy>
        struct stream_readable2::value_variable_builder_impl<
            Data, datas::types::String<StringSize, StringData, ZeroPolicy>>
        {
            static auto make(native)
            {
                return val<Data, lazy<bytes_view>>{};
            }

            static auto make(writable_bytes_view str)
            {
                return val<Data, writable_bytes_view>{str};
            }
        };

        template<class Data, class DataSize>
        struct stream_readable2::value_variable_builder_impl<
            Data, datas::types::PktSize<DataSize>>
        {
            static auto make(native)
            {
                return lazy<datas::types::PktSize<DataSize>>{};
            }

            static auto make(ref<value_type_t<proto_basic_type_t<DataSize>>> r)
            {
                return val<
                    datas::types::PktSize<DataSize>,
                    value_type_t<proto_basic_type_t<DataSize>>&
                >{r.x};
            }
        };


        template<class Name, class T>
        auto make_mem(T&& x)
        {
            return stream_readable2::named<
                typename decltype(Name::mem()(wrap_type<T>()))::type,
                Name
            >{{static_cast<T&&>(x)}};
        }

        template<class... Ts, class X>
        auto tuple_add(tuple<Ts...>& t, X&& x)
        {
            return tuple<Ts..., X>{static_cast<Ts&>(t)..., {static_cast<X&&>(x)}};
        }

        template<class Name, class Data>
        struct stream_readable2::next_value<as_param,
            variable<lazy<Data>, Name>>
        {
            using reader = read_data<Data>;
            using static_min_size = static_min_size_t<reader>;
            using is_static_size = is_static_size_t<reader>;
            using context_value_list = mp::list<>;

            template<class Ctx, class I>
            static auto make(Ctx& ctx, I, lazy<Data>)
            {
                return make_mem<Name>(reader::read(ctx.in));
            }
        };

        template<stream_readable2::SizeInfo const& info, class T>
        bool is_too_short(T n, InStream const& in)
        {
            if constexpr (info.is_rng_rstatic_size)
            {
                bool too_short = (n != in.in_remain());
                if constexpr (info.rng_raccu != 0)
                {
                    return too_short || (n != info.rng_raccu);
                }
                return too_short;
            }
            else
            {
                bool too_short = (n > in.in_remain());
                if constexpr (info.rng_raccu != 0)
                {
                    return too_short || (n < info.rng_raccu);
                }
                return too_short;
            }
        }

        template<class Name, class Data>
        struct stream_readable2::next_value<as_param,
            variable<lazy<datas::types::PktSize<Data>>, Name>>
        {
            using reader = read_data<Data>;
            using static_min_size = static_min_size_t<reader>;
            using is_static_size = is_static_size_t<reader>;

            using var_size = variable<value_type_t<Data>, datas::types::PktSize<Name>>;
            using context_value_list = mp::list<var_size>;

            template<class Ctx, class I>
            static auto make(Ctx& ctx, I, lazy<datas::types::PktSize<Data>>)
            {
                auto& n = static_cast<var_size&>(ctx.ctx_values).value;
                n = reader::read(ctx.in);

                auto& info = Ctx::size_infos[I::value];
                println("pktsz: ", info);
                std::flush(std::cout);

                if constexpr (info.is_inclusive_range)
                {
                    println(" inclusive");

                    auto diff = ctx.get_start_distance(info.start_ptr_pos);

                    println("diff: ", diff);


                    // TODO is_too_short ^^^ŝ
                    bool too_short = n < diff;
                    if constexpr (info.is_rng_rstatic_size)
                    {
                        too_short = too_short
                            || (n - diff != ctx.in.in_remain());
                        if constexpr (info.rng_raccu != 0)
                        {
                            too_short = too_short
                                || (n - diff != info.rng_raccu);
                        }
                    }
                    else
                    {
                        too_short = too_short
                            || (n - diff > ctx.in.in_remain());
                        if constexpr (info.rng_raccu != 0)
                        {
                            too_short = too_short
                                || (n - diff < info.rng_raccu);
                        }
                    }

                    if (REDEMPTION_UNLIKELY(too_short))
                    {
                        ctx.error(Name{}, info.rng_raccu, n);
                    }

                    ctx.save_stream(info.end_stream_pos, n - diff);
                }
                else
                {
                    println(" exclusive");
                }

                return make_mem<Name>(value_type_t<Data>(n));
            }
        };

        template<class Name, class Data, class T>
        struct stream_readable2::next_value<as_param,
            variable<val<Data, T&>, Name>>
        {
            using reader = read_data<Data>;
            using static_min_size = static_min_size_t<reader>;
            using is_static_size = is_static_size_t<reader>;
            using context_value_list = mp::list<>;

            template<class Ctx, class I>
            static auto make(Ctx& ctx, I, val<Data, T&> v)
            {
                v.x = reader::read(ctx.in);
                return make_mem<Name>(v.x);
            }
        };

        template<class Name, class Data, class Bytes>
        struct stream_readable2::next_value<
            datas::values::types::SizeBytes<as_param>,
            variable<val<Data, Bytes>, Name>>
        {
            using DataSize = data_to_value_size<Data>;
            using var_size = variable<
                value_type_t<DataSize>,
                datas::values::types::SizeBytes<Name>>;

            using reader = read_data<DataSize>;
            using static_min_size = static_min_size_t<reader>;
            using is_static_size = is_static_size_t<reader>;
            using context_value_list = mp::list<var_size>;

            template<class Ctx, class I>
            static void make(Ctx& ctx, I, val<Data, Bytes>&)
            {
                auto& n = static_cast<var_size&>(ctx.ctx_values).value;
                n = reader::read(ctx.in);
            }
        };

        inline writable_bytes_view read_data_bytes(
            InStream& in, writable_bytes_view& bytes, safe_int<std::size_t> n)
        {
            bytes = {bytes.data(), n};
            in.in_copy_bytes(bytes);
            return bytes;
        }

        inline bytes_view read_data_bytes(
            InStream& in, lazy<bytes_view>, safe_int<std::size_t> n)
        {
            return in.in_skip_bytes(n.underlying());
        }

        template<class Name, class Data, class Bytes>
        struct stream_readable2::next_value<
            datas::values::types::Data<as_param>,
            variable<val<Data, Bytes>, Name>>
        {
            using static_min_size = mp::int_<0>;
            using is_static_size = std::false_type;
            using context_value_list = mp::list<>;

            template<class Ctx, class I>
            static auto make(Ctx& ctx, I, val<Data, Bytes>& v)
            {
                auto n = get_var<datas::values::types::SizeBytes<Name>>(ctx.ctx_values).value;

                auto& info = Ctx::size_infos[I::value];
                println("data:  ", info);
                std::flush(std::cout);

                bool too_short;
                // TODO /!\ overflow on n
                if constexpr (info.is_rng_rstatic_size)
                {
                    too_short = (n + info.rng_raccu != ctx.in.in_remain());
                }
                else
                {
                    too_short = (n + info.rng_raccu > ctx.in.in_remain());
                }

                if (REDEMPTION_UNLIKELY(too_short))
                {
                    ctx.error(Name{}, n + info.rng_raccu, ctx.in.in_remain());
                }

                return make_mem<Name>(read_data_bytes(ctx.in, v.x, n));
            }
        };

        template<class T, class Name>
        struct stream_readable2::next_value<start_range, variable<T, Name>>
        {
            using static_min_size = mp::int_<0>;
            using is_static_size = std::false_type;
            using context_value_list = mp::list<>;

            template<class Ctx, class I, class X>
            static void make(Ctx& ctx, I, X const&)
            {
                auto& info = Ctx::size_infos[I::value];
                println("start: ", info);
                std::flush(std::cout);

                if constexpr (not info.is_inclusive_range)
                {
                    println(" exclusive");
                    auto rng_size = get_var<datas::types::PktSize<Name>>(ctx.ctx_values).value;

                    println(rng_size, " / ", info.rng_raccu, " | ", ctx.in.in_remain());

                    // TODO == if is_rng_rstatic_size
                    // TODO info.rng_raccu == 0
                    if (REDEMPTION_UNLIKELY(
                        rng_size < info.rng_raccu || not ctx.in.in_check_rem(rng_size)
                    ))
                    {
                        ctx.error(Name{}, info.rng_raccu, rng_size);
                    }

                    ctx.save_stream(info.end_stream_pos, rng_size);
                }
                else
                {
                    println(" inclusive");

                    ctx.save_start_pos(info.start_ptr_pos);
                }
            }
        };

        template<class T, class Name>
        struct stream_readable2::next_value<stop_range, variable<T, Name>>
        {
            using static_min_size = mp::int_<0>;
            using is_static_size = std::false_type;
            using context_value_list = mp::list<>;

            template<class Ctx, class I, class X>
            static void make(Ctx& ctx, I, X const&)
            {
                auto& info = Ctx::size_infos[I::value];
                println("stop: ", info);
                std::flush(std::cout);

                if (REDEMPTION_UNLIKELY(ctx.in.in_remain())) {
                    ctx.error(Name{}, ctx.in.in_remain(), 0);
                }

                ctx.restore_stream(info.end_stream_pos);

                println(" inclusive or exclusive  ", info.rng_raccu_next, "/", ctx.in.in_remain());
                if constexpr (info.rng_raccu_next > 0) {
                    if (REDEMPTION_UNLIKELY(not ctx.in.in_check_rem(info.rng_raccu_next))) {
                        ctx.error(Name{}, ctx.in.in_remain(), info.rng_raccu_next);
                    }
                }

                std::flush(std::cout);
            }
        };

        template<class Int, class Endianess>
        struct stream_readable2::read_data_impl<datas::types::Integer<Int, Endianess>>
        {
            using static_min_size = mp::int_<sizeof(Int)>;
            using is_static_size = std::true_type;

            static Int read(InStream& in)
            {
                uint8_t const* p = in.in_skip_bytes(sizeof(Int)).data();
                using rng = mpe::make_int_sequence<mp::int_<sizeof(Int)>>;
                using is_little_endian = std::is_same<Endianess, datas::types::le_tag>;
                return apply(rng{}, [&](auto... i) {
                    return ((
                        p[i.value] << ((is_little_endian::value ? i.value : sizeof(Int)-1u-i.value) * 8u)
                     ) | ...);
                });
            }
        };
    }

    template<class Tag, class BasicType, class T>
    struct variable_builder;

    class readable_variable;
    class writable_variable;

    template<class BasicType, class T>
    struct variable_builder<readable_variable, BasicType, T>
    {
        static auto make(T const& x)
        {
            return basic_type_to_cpp_type<BasicType>{x};
        }
    };

    template<class BasicType, class T>
    struct variable_builder<writable_variable, BasicType, T&>
    {
        static T& make(T& x)
        {
            static_assert(std::is_same_v<basic_type_to_cpp_type<BasicType>, T>);
            return x;
        }
    };

    template<class T>
    struct is_writable_view : std::false_type
    {};

    template<> struct is_writable_view<writable_bytes_view> : std::true_type {};
    template<class T> struct is_writable_view<array_view<T>> : std::true_type {};
    template<class T> struct is_writable_view<array_view<const T>> : std::false_type {};

    namespace errors
    {
        PROTO_ERROR_C(is_not_writable_view);
    }

    template<class BasicType, class T>
    struct variable_builder<writable_variable, BasicType, T>
    {
        using View = std::remove_reference_t<T>;

        static auto make(View v)
        {
            detail::check_pred<View, mp::cfe<is_writable_view>, errors::is_not_writable_view>();
            return v;
        }
    };

    template<class Params, class Values, class... Xs>
    writable_bytes_view inplace_emit(writable_bytes_view buf, Definition<Params, Values> const&, Xs const&... xs)
    {
        using Traits = traits::stream_writable;

        auto ctx = Traits::make_context<Params, Values>(buf, xs...);

        return apply(Values{}, [&](auto... v){
            (Traits::next_state(ctx, v), ...);
            return Traits::final(ctx);
        });
    }

    template<class ErrorFn, class Params, class Values, class... Xs>
    auto inplace_recv2(bytes_view buf, ErrorFn&& error, Definition<Params, Values> const&, Xs const&... xs)
    {
        using Traits = traits::stream_readable2;

        auto ctx = Traits::make_context<Params, Values>(buf, error, xs...);

        using ctx_mems = Traits::context_members<Values, decltype(ctx)>;
        using strong_group_members = typename ctx_mems::strong_group_members;
        using volatile_final_members = typename ctx_mems::volatile_final_members;

        println(type_name<strong_group_members>());
        println(type_name<volatile_final_members>());

        auto final = [&](auto&&... xs){
            apply(volatile_final_members{}, [&](auto... v) {
                return (Traits::next_state(ctx, v), ...);
            });
            return Traits::final(ctx, FWD(xs)...);
        };

        return apply(strong_group_members{}, [&](auto... l){
            return final(tuple{apply(l, [&](auto... v) {
                return (Traits::next_state(ctx, v), ...);
            })...});
        });
    }

    template<class ErrorFn, class... Params, class Values, class... Xs>
    auto inplace_struct(bytes_view buf, ErrorFn&& error, Definition<mp::list<Params...>, Values> const& def)
    {
        return inplace_recv2(buf, error, def, value<native, name_t<Params>>{}...);
    }

    template<class Params, class Values, class... Xs>
    auto print_size_infos(Definition<Params, Values> const&, Xs const&... xs)
    {
        using Traits = traits::stream_readable2;

        bytes_view buf{"", 99999};
        auto error_fn = [](auto...){};
        /*auto ctx =*/ Traits::make_context<Params, Values>(buf, error_fn, xs...);
    }

    template<class... Params, class Values, class... Xs>
    auto print_size_infos(Definition<mp::list<Params...>, Values> const& def)
    {
        return print_size_infos(def, value<native, name_t<Params>>{}...);
    }
}

namespace detail
{
    using mp::list;
    PROTO_IS_TYPE_VA(list);
}

using detail::is_list;
using detail::is_list_v;

namespace n1 {
    PROTO_CLASS_NAME(A, e);

    template<class T>
    std::ostream& operator<<(std::ostream& out, A<T> const& x)
    {
        return out << x._proto_value();
    }
}
namespace n2 {
    PROTO_CLASS_NAME(A, e);

    template<class T>
    std::ostream& operator<<(std::ostream& out, A<T> const& x)
    {
        return out << x._proto_value();
    }
}

struct SS {
    PROTO_LOCAL_NAME(d);
    PROTO_USE_CLASS_NAME(n1::A) e;
} ss;

struct SSS {
    PROTO_USE_CLASS_NAME(n2::A) e;
} sss;


namespace detail
{
    template<class T>
    struct definition_guide
    {
        using type = T;

        template<class... xs>
        definition_guide(xs const&...);
    };

    template<class... xs>
    definition_guide(xs const&... vs) -> definition_guide<decltype(test::definition(vs...))>;
}

#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/variadic/to_list.hpp>

#define PROTO_PACKET_TYPE_value(name, type) static constexpr PROTO_LOCAL_NAME(name);
#define PROTO_PACKET_TYPE_type(name, type) static constexpr PROTO_LOCAL_NAME(name);
#define PROTO_PACKET_TYPE_data(name, type)

#define PROTO_PACKET_TYPE_DISPATCH(r, data, elem) PROTO_PACKET_TYPE_##elem

#define PROTO_PACKET_VALUE_value(name_, type_) name_ = type_,
#define PROTO_PACKET_VALUE_type(name_, type_) name_ = test::type(type_),
#define PROTO_PACKET_VALUE_data(name_, type_) name_ = type_,

#define PROTO_PACKET_VALUE_DISPATCH(r, name, elem) name().PROTO_PACKET_VALUE_##elem

#define PROTO_PACKET_I(name, cname, list)                                   \
    struct name {                                                           \
        BOOST_PP_LIST_FOR_EACH(PROTO_PACKET_TYPE_DISPATCH, _, list)         \
        static zstring_view _proto_name() { return cname ""_zv; }           \
    };                                                                      \
    constexpr inline ::proto::tuple<                                        \
        class name,                                                         \
        decltype(detail::definition_guide{                                  \
            BOOST_PP_LIST_FOR_EACH(PROTO_PACKET_VALUE_DISPATCH, name, list) \
        })::type                                                            \
    > name

#define PROTO_PACKET(name, cname, ...) \
    PROTO_PACKET_I(name, cname, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))
#define PROTO_PACKET2(name, ...) \
    PROTO_PACKET_I(name, #name, BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))


namespace X224
{
    using namespace proto::datas;

    PROTO_PACKET2(tpkt,
        value(version, u8),
        value(len, u16_be)
    );

    inline constexpr auto tdpu_recv = [](InStream& stream, auto pkt){
        return test::inplace_struct(
            {stream.get_current(), stream.get_data_end()},
            [](auto field, std::size_t n, std::size_t capacity) {
                std::flush(std::cout);
                char s[128];
                snprintf(s, sizeof(s), "Truncated %s: field '%s': stream.size=%zu expected=%zu",
                    decltype(pkt)::_proto_name().c_str(), field._proto_name().c_str(), capacity, n);
                throw std::runtime_error(s);
            },
            pkt);
    };
}


int main()
{
    struct S {
        PROTO_LOCAL_NAME(a);
        PROTO_LOCAL_NAME(b);
        PROTO_LOCAL_NAME(c);
        PROTO_LOCAL_NAME(size);
        PROTO_LOCAL_NAME(size2);
    } s;

    using namespace proto::datas;

    auto def = test::definition(
        ss.d = pkt_size(u16_be),
        ss.e = u16_le,
        ss.d = test::start(),

        s.c = test::type(ascii_string(u16_be)),

        s.size = test::start(),
        s.size = pkt_size(u16_be),

        s.a = u8,
        s.c = values::size_bytes,
        s.b = u16_le,
        s.c = values::data,

        s.size = test::stop(),

        sss.e = u16_be,
        ss.d = test::stop()
    );

    auto error_fn = [](auto field, std::size_t n, std::size_t capacity){
        std::flush(std::cout);
        char s[128];
        snprintf(s, sizeof(s), "buf is too short: field '%s': stream.in_remain=%zu expected=%zu",
            field._proto_name().c_str(), capacity, n);
        throw std::runtime_error(s);
    };

    uint8_t a = 0x78;
    uint16_t b = 0x1f23;

    auto print_list = [](char const* s, auto l){
        print(s, ":\n");
        test::apply(l, [&](auto... xs){
            (println("  ", type_name(xs)), ...);
        });
    };

    // println('\n', type_name(test::definition2(u8[s.a])));
    println("definition:\n  ", type_name(def));
    print_list("params", def.params());
    print_list("values", def.values());

    print("\n\ninplace_emit:\n\n");
    std::array<uint8_t, 30> buf {};
    auto out = test::inplace_emit(buf, def,
        s.b = b, s.a = a, s.c = "plop"_av, s.size = native(),
        ss.d = native(), ss.e = a,
        sss.e = b);
    dump(out);

    {
        print("\n\ninplace_recv2:\n");
        char strbuf[10];
        a = '0';
        auto datas = test::inplace_recv2(out, error_fn, def,
            s.b = native(), s.a = ref{a}, s.c = make_array_view(strbuf), s.size = native(),
            ss.d = native(), ss.e = native(),
            sss.e = native());
        println("\n", type_name(datas));
        println("datas.a = ", std::hex, datas.a, " ", type_name<decltype(datas.a)>());
        println("datas.b = ", datas.b, " ", type_name<decltype(datas.b)>());
        println("datas.c = ", datas.c, " ", type_name<decltype(datas.c)>());
        println("datas.d = ", std::dec, datas.d, " ", type_name<decltype(datas.d)>());
        println("datas[sss.e] = ", std::hex, datas[sss.e], " ", type_name<decltype(datas[sss.e])>());
        println("datas[ss.e] = ", std::hex, datas[ss.e], " ", type_name<decltype(datas[ss.e])>());
        println("a = ", a);
    }

    {
        print("\n\ninplace_struct:\n");
        auto datas = test::inplace_struct(out, error_fn, def);
        println("\n", type_name(datas));
        println("datas.a = ", std::hex, datas.a, " ", type_name<decltype(datas.a)>());
        println("datas.b = ", std::hex, datas.b, " ", type_name<decltype(datas.b)>());
        println("datas.c = ", datas.c, " ", type_name<decltype(datas.c)>());
        println("datas.d = ", std::dec, datas.d, " ", type_name<decltype(datas.d)>());
        println("datas[sss.e] = ", std::hex, datas[sss.e], " ", type_name<decltype(datas[sss.e])>());
        println("datas[ss.e] = ", std::hex, datas[ss.e], " ", type_name<decltype(datas[ss.e])>());

        auto p = [](auto const& x){
            println(x._proto_name(), ": ", x._proto_value());
        };
        datas.apply([&](auto const&... xs) {
            (p(xs), ...);
        });
    }

    {
        auto def = test::definition(
            ss.d = test::start(),
            ss.d = pkt_size(u16_be),

            s.size = pkt_size(u16_be),
            s.size = test::start(),

            s.c = test::type(ascii_string(u16_be)),
            s.c = values::size_bytes,
            s.c = values::data,

            s.size = test::stop(),

            ss.d = test::stop(),

            s.a = u8
        );

        std::array<uint8_t, 30> buf {};
        auto out = test::inplace_emit(buf, def, s.a = a, s.c = "plop"_av, ss.d = native(), s.size = native());
        dump(out);

        std::dec(std::cout);

        print("\n\nbounds error:\n\n");
        // "\x00\x06\x00\x04\x70\x6c\x6f\x70\x78"
        auto datas = test::inplace_struct("\x00\x0a\x00\x06\x00\x04\x70\x6c\x6f\x70\x78"_av, error_fn, def);
        println();
        println("datas.d = ", datas.d);
        println("datas.c = ", datas.c);
        println("datas.a = ", int(datas.a));
    }

    {
        print("\n\nprint size infos:\n\n");

        auto def = test::definition(
            ss.d = test::start(),
            ss.d = pkt_size(u16_be),

            s.size = pkt_size(u16_be),
            s.size = test::start(),

            s.c = test::type(ascii_string(u16_be)),
            s.c = values::size_bytes,
            s.c = values::data,

            s.size = test::stop(),

            ss.d = test::stop(),

            s.a = u8
        );

        print_size_infos(def);
    }

    {
        print("\n\nx224.tpkt:\n\n");
        InStream in("\x03\x11\x11");
        auto datas = X224::tdpu_recv(in, X224::tpkt);
        println("datas.version: ", int(datas.version));
        println("datas.len: ", std::hex, datas.len);
    }
}
