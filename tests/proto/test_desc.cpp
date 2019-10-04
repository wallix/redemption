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

#include <iostream>
#include <string_view>


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
    class RawData;

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
}

template<class...> struct names {};

template<class T>
struct extract_name_impl;

template<class T, class... Names>
struct extract_name_impl<proto::value<T, Names...>>
{
    using type = names<Names...>;
};

template<class T, class... Names>
struct extract_name_impl<proto::param<T, Names...>>
{
    using type = names<Names...>;
};

template<class T>
using extract_name = typename extract_name_impl<T>::type;

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

template<class... Name, class F>
void each_name(names<Name...>, F&& f)
{
    return (f(Name()), ...);
}

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
    print("dump: 0x");
    for (auto c : buf) {
        print(t[c>>4], t[c&0xf]);
    }
    println();
}


namespace mp = kvasir::mpl;
namespace mpe = kvasir::mpl::eager;

namespace test
{
    using namespace proto;

    template<class C = mp::identity> using mp_tpl0 = mp::unpack<mp::front<C>>;
    template<class C = mp::identity> using mp_tpl1 = mp::unpack<mp::at1<C>>;

    namespace detail
    {
        class discard {};
        template<class T> using is_discard = typename std::is_same<T, discard>::type;
        template<class T> inline constexpr bool is_discard_v = std::is_same<T, discard>::value;

        template<class X>
        auto extract_param(X const& x)
        {
            if constexpr (is_param_v<X>)
            {
                return x;
            }
            else if constexpr (is_param_and_lazy_value_v<X>)
            {
                return param<data_type_t<X>, name_t<X>>{};
            }
            else
            {
                return discard{};
            }
        }

        template<class X>
        auto extract_lazy_value(X const& x)
        {
            if constexpr (is_lazy_value_v<X>)
            {
                return x;
            }
            else if constexpr (is_param_and_lazy_value_v<X>)
            {
                return lazy_value<as_param, name_t<X>>{};
            }
            else
            {
                return discard{};
            }
        }

        template<class T> using type_t = typename T::type;
        template<class T> using index_t = typename T::index;
        template<class T> using names_t = typename T::names;

        template<class C = mp::identity> using mp_indexed_type_t = mp::unpack<mp::front<C>>;
        template<class C = mp::identity> using mp_indexed_index_t = mp::unpack<mp::at1<C>>;

        template<class C, class... xs>
        using mpc_remove_discard = mp::call<mp::remove_if<mp::same_as<detail::discard>, C>, xs...>;

        template<class X, class I>
        struct indexed : X
        {
            using type = X;
            using index = I;
        };

        template<class T, int n>
        using filled_sequence = mp::call<
            mp::make_int_sequence<mp::transform<mp::always<T>>>,
            mp::int_<n>>;

        template<class... x>
        struct set_difference
        {
            template<class... y>
            using f = mp::call<mp::join<>,
                mp::call<mp::conditional<std::is_same_v<
                    mp::call<mp::transform<mp::same_as<x>>, y...>,
                    filled_sequence<mp::false_, sizeof...(y)>
                >>, mp::list<x>, mp::list<>
            >...>;
        };

        template<class... p>
        struct param_list
        {
            template<class C = mp::listify>
            using names = mp::call<C, mp::call<mp_tpl1<>, p>...>;

            struct param_must_be_unique : mp::call<mp_tpl1<>, p>...
            {
                using list = mp::list<p...>;
            };
        };

        template<class... ints>
        struct indexed_tuple
        {
            template<class... xs>
            using f = tuple<indexed<xs, ints>...>;
        };

        using mp_lazy_value_names = mp::unpack<mp::pop_front<>>;

        template<class... v>
        struct lazy_value_list
        {
            template<class C = mp::listify>
            using names = mp::call<mp::transform<mp_lazy_value_names, mp::join<C>>, v...>;

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
            using l = typename x::template names<
                mp::cfe<detail::set_difference,
                    mp::cfe<y::template names>>>;

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

    template<class... Xs>
    auto definition2(Xs const&... xs)
    {
        using params = detail::mpc_remove_discard<
            mp::cfe<detail::param_list>,
            decltype(detail::extract_param(xs))...>;

        using lazy_values = detail::mpc_remove_discard<
            mp::cfe<detail::lazy_value_list>,
            decltype(detail::extract_lazy_value(xs))...>;

        // TODO add automatically parameter ?
        detail::check_unused_params<params, lazy_values, errors::some_parameters_are_unused>();
        detail::check_unused_params<lazy_values, params, errors::some_values_are_unused>();

        return Definition<
            typename params::param_must_be_unique::list,
            typename lazy_values::list
        >{};
    }

    template<class F>
    auto unindex(F&& f)
    {
        return [f](auto... x){
            return f(static_cast<mp::call<detail::mp_indexed_type_t<>, decltype(x)>&>(x)...);
        };
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

    template<class Data, class X>
    struct typed_value
    {
        using data_type = Data;
        using type = X;

        X x;
    };


    PROTO_IS_TYPE2(typed_value);


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
    auto apply(mp::list<Ts...>, F&& f)
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
        struct stream_writable
        {
            template<class BasicType, class T>
            struct value_variable_builder;

            template<class ValueBasicType, class BasicType, class T>
            struct value_builder;

            template<class BasicType>
            struct writer;
        };


        template<class BasicType, class T>
        struct stream_writable::value_builder<as_param, BasicType, T>
        {
            static auto make(T x)
            {
                return typed_value<BasicType, T>{x};
            }
        };

        template<class StringSize, class StringData, class T>
        struct stream_writable::value_builder<
            datas::values::types::SizeBytes<as_param>,
            datas::types::String<StringSize, StringData, datas::types::no_zero>,
            T>
        {
            static auto make(bytes_view v)
            {
                using int_type = value_type_t<StringSize>;
                return typed_value<StringSize, int_type>{checked_cast<int_type>(v.size())};
            }
        };

        template<class StringSize, class StringData, class T>
        struct stream_writable::value_builder<
            datas::values::types::Data<as_param>,
            datas::types::String<StringSize, StringData, datas::types::no_zero>,
            T>
        {
            static auto make(bytes_view v)
            {
                return typed_value<datas::types::BinaryData, bytes_view>{v};
            }
        };


        template<class Int, class Endianess, class T>
        struct stream_writable::value_variable_builder<datas::types::Integer<Int, Endianess>, T>
        {
            static auto make(safe_int<Int> x)
            {
                return x.underlying();
            }
        };

        template<class StringSize, class StringData, class T>
        struct stream_writable::value_variable_builder<datas::types::String<StringSize, StringData, datas::types::no_zero>, T>
        {
            static auto make(bytes_view str)
            {
                return str;
            }
        };


        template<class Int, class Endianess>
        struct stream_writable::writer<datas::types::Integer<Int, Endianess>>
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
        struct stream_writable::writer<datas::types::BinaryData>
        {
            static void write(OutStream& out, bytes_view bytes)
            {
                out.out_copy_bytes(bytes);
            }
        };


        struct stream_readable
        {
            template<class BasicType, class T>
            struct value_variable_builder;

            template<class ValueBasicType, class BasicType, class T>
            struct value_builder;

            template<class BasicType>
            struct reader;

            struct bytes_size_ref { writable_bytes_view& bytes; };
            struct bytes_data_ref { writable_bytes_view& bytes; };
        };

        inline std::ostream& operator<<(std::ostream& out, stream_readable::bytes_size_ref const& ref)
        {
            return out << "bytes_size_ref(" << ref.bytes.size() << ")";
        }

        inline std::ostream& operator<<(std::ostream& out, stream_readable::bytes_data_ref const& ref)
        {
            return out << "bytes_data_ref(" << ref.bytes << ")";
        }


        template<class Int, class Endianess, class T>
        struct stream_readable::value_variable_builder<datas::types::Integer<Int, Endianess>, T>
        {
            static Int make(native)
            {
                return Int();
            }

            static ref<Int> make(ref<Int> x)
            {
                return x;
            }
        };

        template<class StringSize, class StringData, class T>
        struct stream_readable::value_variable_builder<datas::types::String<StringSize, StringData, datas::types::no_zero>, T>
        {
            static auto make(writable_bytes_view str)
            {
                return str;
            }
        };


        template<class BasicType, class T>
        struct stream_readable::value_builder<as_param, BasicType, T>
        {
            static auto make(T& x)
            {
                return typed_value<BasicType, T&>{x};
            }
        };

        template<class StringSize, class StringData, class T>
        struct stream_readable::value_builder<
            datas::values::types::SizeBytes<as_param>,
            datas::types::String<StringSize, StringData, datas::types::no_zero>,
            T>
        {
            static auto make(writable_bytes_view& v)
            {
                return typed_value<StringSize, stream_readable::bytes_size_ref>{v};
            }
        };

        template<class StringSize, class StringData, class T>
        struct stream_readable::value_builder<
            datas::values::types::Data<as_param>,
            datas::types::String<StringSize, StringData, datas::types::no_zero>,
            T>
        {
            static auto make(writable_bytes_view& v)
            {
                // TODO lazy_param
                return typed_value<datas::types::BinaryData, stream_readable::bytes_data_ref>{v};
            }
        };


        template<class Int, class Endianess>
        struct stream_readable::reader<datas::types::Integer<Int, Endianess>>
        {
            static void read(InStream& in, bytes_size_ref bytes_ref)
            {
                Int x;
                read(in, x);
                bytes_ref.bytes = {bytes_ref.bytes.data(), safe_cast<std::size_t>(x)};
            }

            static void read(InStream& in, ref<Int> ref)
            {
                read(in, ref.x);
            }

            static void read(InStream& in, Int& x)
            {
                auto* p = in.in_skip_bytes(sizeof(Int)).data();
                using rng = mpe::make_int_sequence<mp::int_<sizeof(Int)>>;
                using is_little_endian = std::is_same<Endianess, datas::types::le_tag>;
                apply(rng{}, [&](auto... i) {
                    ((
                        x |= *p++ << ((is_little_endian::value ? i.value : sizeof(Int)-1u-i.value) * 8u)
                    ), ...);
                });
            }
        };

        template<>
        struct stream_readable::reader<datas::types::BinaryData>
        {
            static void read(InStream& in, bytes_data_ref bytes_ref)
            {
                in.in_copy_bytes(bytes_ref.bytes);
            }
        };

        template<class Tuple>
        struct Ctx
        {
            InStream in;
            Tuple params;
        };

        template<class Tuple>
        Ctx(InStream, Tuple&&) -> Ctx<Tuple>;


        template<class Params, class R>
        struct State
        {
            InStream& in;
            Params params;
            R result;
        };

        template<class Params, class R>
        State(InStream&, Params&&, R&&) -> State<Params, R>;

        struct stream_readable2
        {
            template<class Data, class BasicType>
            struct value_variable_builder_impl;

            template<class Data, class T>
            using value_variable_builder
                = value_variable_builder_impl<Data, proto_basic_type_t<Data>>;

            template<class BasicType, class... NamedValues>
            struct next_value;

            struct reader_impl;

            template<class T>
            using reader = reader_impl;

            template<class T>
            struct read_data_impl;

            template<class T>
            using read_data = read_data_impl<proto_basic_type_t<T>>;

            template<class> class lazy {};
            template<class Data, class T> struct val { T x; };

            template<class Params, class Values, class... Xs>
            static auto make_context(bytes_view buf, Xs const&... xs)
            {
                return Ctx{InStream{buf}, detail::build_params2<stream_readable2, Params>(xs...)};
            }

            template<class... Ts>
            static auto make_state(Ctx<tuple<Ts...>>& ctx)
            {
                return State{ctx.in, ctx.params, tuple<>{}};
            }

            template<class State, class Data, class... Name>
            static auto next_state(State& state, lazy_value<Data, Name...>)
            {
                using builder = next_value<
                    proto_basic_type_t<Data>,
                    std::remove_reference_t<decltype(get_var<Name>(state.params))>...>;
                return builder::make(state, get_var<Name>(state.params).value...);
            }

            template<class State>
            static auto final(State&& state)
            {
                // TODO sort by alignement ?
                return state.result;
            }
        };

        template<class T>
        std::ostream& operator<<(std::ostream& out, stream_readable2::lazy<T> const&)
        {
            return out << "lazy<" << type_name<T>() << ">";
        }

        template<class Data, class T>
        std::ostream& operator<<(std::ostream& out, stream_readable2::val<Data, T> const& v)
        {
            return out << v.x;
        }


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
            static auto make(writable_bytes_view str)
            {
                return val<Data, writable_bytes_view>{str};
            }
        };


        template<class Name, class T>
        auto make_mem(T&& x)
        {
            return typename decltype(Name::mem()(wrap_type<T>()))::type{static_cast<T&&>(x)};
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

        template<class... Ts, class X>
        auto tuple_add(tuple<Ts...>& t, X&& x)
        {
            return tuple<Ts..., X>{static_cast<Ts&>(t)..., {static_cast<X&&>(x)}};
        }

        template<class Name, class Data>
        struct stream_readable2::next_value<as_param,
            variable<stream_readable2::lazy<Data>, Name>>
        {
            template<class St>
            static auto make(St& state, stream_readable2::lazy<Data>)
            {
                return State{
                    state.in,
                    state.params,
                    tuple_add(state.result, make_mem<Name>(read_data<Data>::read(state.in)))
                };
            }
        };

        template<class Name, class Data, class T>
        struct stream_readable2::next_value<as_param,
            variable<stream_readable2::val<Data, T&>, Name>>
        {
            template<class St>
            static auto make(St& state, stream_readable2::val<Data, T&> v)
            {
                v.x = read_data<Data>::read(state.in);
                return State{
                    state.in,
                    state.params,
                    tuple_add(state.result, make_mem<Name>(v.x))
                };
            }
        };

        template<class Name, class Data>
        struct stream_readable2::next_value<
            datas::values::types::SizeBytes<as_param>,
            variable<stream_readable2::val<Data, writable_bytes_view>, Name>>
        {
            template<class Tuple, class R>
            static auto make(State<Tuple, R>& state, stream_readable2::val<Data, writable_bytes_view>&)
            {
                auto n = read_data<data_to_value_size<Data>>::read(state.in);
                return State{
                    state.in,
                    tuple_add(
                        state.params,
                        variable<decltype(n), datas::values::types::SizeBytes<Name>>{n}
                    ),
                    state.result
                };
            }
        };

        template<class Name, class Data>
        struct stream_readable2::next_value<
            datas::values::types::Data<as_param>,
            variable<stream_readable2::val<Data, writable_bytes_view>, Name>>
        {
            template<class St>
            static auto make(St& state, stream_readable2::val<Data, writable_bytes_view>& v)
            {
                auto n = get_var<datas::values::types::SizeBytes<Name>>(state.params).value;
                v.x = {v.x.data(), std::size_t(n)};
                state.in.in_copy_bytes(v.x);
                return State{
                    state.in,
                    state.params,
                    tuple_add(state.result, make_mem<Name>(writable_bytes_view(v.x)))
                };
            }
        };

        template<class Int, class Endianess>
        struct stream_readable2::read_data_impl<datas::types::Integer<Int, Endianess>>
        {
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
    class native_variable;

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

    template<class Traits, class Param, class X>
    auto make_variable(proto::value<X, name_t<Param>> const& v)
    {
        using Data = data_type_t<Param>;
        using BasicType = proto_basic_type_t<Data>;
        using Name = name_t<Param>;
        using Value = std::remove_reference_t<X>;
        using builder = typename Traits::template value_variable_builder<BasicType, Value>;

        // TODO v.value&&
        using value_type = decltype(builder::make(v.value));
        if constexpr (is_typed_value_v<value_type>)
        {
            return variable<value_type, Name>{builder::make(v.value)};
        }
        else
        {
            return variable<typed_value<Data, value_type>, Name>{
                typed_value<Data, value_type>{builder::make(v.value)}
            };
        }
    }

    template<class Traits, class Params, class... Xs>
    auto build_params(Xs const&... xs)
    {
        PROTO_ASSERT_TYPES(proto::is_value, Xs);

        using params = mp::call<mp::unpack<mp::cfe<detail::param_list>>, Params>;
        using value_params = detail::param_list<Xs...>;

        detail::check_unused_params<params, value_params, errors::missing_parameters>();
        detail::check_unused_params<value_params, params, errors::unknown_parameters>();

        // TODO value_type_t must be rvalue or lvalue
        proto::tuple<Xs...> t{Xs{static_cast<value_type_t<Xs>>(xs.value)}...};

        return apply(Params{}, [&](auto... p){
            return tuple{make_variable<Traits, decltype(p)>(t)...};
        });
    }

    template<class Traits, class Name, class Data, class Tuple>
    auto make_value(lazy_value<Data, Name>, Tuple&& t)
    {
        auto&& v = get_var<Name>(t).value;
        using Value = std::decay_t<decltype(v)>;
        using builder = typename Traits::template value_builder<
            proto_basic_type_t<Data>,
            proto_basic_type_t<data_type_t<Value>>,
            typename Value::type>;

        // TODO v.value&&
        using value_type = decltype(builder::make(v.x));
        if constexpr (is_typed_value_v<value_type>)
        {
            return builder::make(v.x);
        }
        else
        {
            return typed_value<Data, value_type>{builder::make(v.x)};
        }
    }

    template<class Params, class Values, class... Xs>
    writable_bytes_view inplace_emit(writable_bytes_view buf, Definition<Params, Values> const&, Xs const&... xs)
    {
        using Traits = traits::stream_writable;

        OutStream out(buf);

        auto tx = build_params<Traits, Params>(xs...);

        tx.apply([&](auto... vars){
            (println("  ", type_name(vars), " = ", vars.value.x), ...);
        });

        auto write = [&](auto v){
            auto x = make_value<Traits>(v, tx);
            println("  ", type_name(x), " = ", x.x);
            Traits::template writer<proto_basic_type_t<typename decltype(x)::data_type>>
                ::write(out, x.x);
        };

        println();
        apply(Values{}, [&](auto... v){
            (write(v), ...);
        });

        return out.get_bytes();
    }

    template<class Params, class Values, class... Xs>
    auto inplace_recv(bytes_view buf, Definition<Params, Values> const&, Xs const&... xs)
    {
        using Traits = traits::stream_readable;

        InStream in(buf);

        auto tx = build_params<Traits, Params>(xs...);

        tx.apply([&](auto... vars){
            (println("  ", type_name(vars), " = ", vars.value.x), ...);
        });

        auto read = [&](auto v){
            auto x = make_value<Traits>(v, tx);
            Traits::template reader<proto_basic_type_t<typename decltype(x)::data_type>>
                ::read(in, x.x);
            println("  ", type_name(x), " = ", x.x);
        };

        println();
        apply(Values{}, [&](auto... v){
            (read(v), ...);
        });

        auto unref = [](auto x) -> decltype(auto) {
            if constexpr (is_ref_v<decltype(x)>)
            {
                // lvalue
                return (x.x);
            }
            else
            {
                return x;
            }
        };

        auto f = [&](auto var){
            using Var = decltype(var);
            return typename decltype(
                name_t<Var>::mem()(wrap_type<decltype(unref(var.value.x))>())
            )::type{unref(var.value.x)};
        };

        return tx.apply([f](auto... vars){
            return proto::tuple{f(vars)...};
        });
    }

    namespace detail
    {
        template<class Traits, class State>
        auto recursive_inplace_recv2(State&& state)
        {
            return Traits::final(static_cast<State&&>(state));
        }

        template<class Traits, class State, class V, class... Vs>
        auto recursive_inplace_recv2(State&& state, V&& v, Vs&&... vs)
        {
            println("  ", type_name<decltype(Traits::next_state(state, v))>());
            return recursive_inplace_recv2<Traits>(Traits::next_state(state, v), vs...);
        }
    }

    template<class Params, class Values, class... Xs>
    auto inplace_recv2(bytes_view buf, Definition<Params, Values> const&, Xs const&... xs)
    {
        using Traits = traits::stream_readable2;

        auto ctx = Traits::make_context<Params, Values>(buf, xs...);

        ctx.params.apply([&](auto... vars){
            (println("  ", type_name(vars), " = ", vars.value), ...);
        });

        return apply(Values{}, [&](auto... v){
            return detail::recursive_inplace_recv2<Traits>(Traits::make_state(ctx), v...);
        });
    }
}

int main()
{
    struct S {
        PROTO_LOCAL_NAME(a);
        PROTO_LOCAL_NAME(b);
        PROTO_LOCAL_NAME(c);
    } s;

    using namespace proto::datas;

    auto def = test::definition2(
        u8[s.a],

        ascii_string(u16_be)[s.c].type(),

        values::size_bytes[s.c],
        u16_le[s.b],
        values::data[s.c]
    );

    // auto def = test::definition2(
    //     s.a = as(u8),
    //
    //     s.c = type(ascii_string(u16_be)),
    //
    //     s.c = values::size_bytes(),
    //     s.b = as(u16_le),
    //     s.c = values::data(),
    // );

    // auto def = test::definition2(
    //     s.a.as(u8),
    //
    //     s.c.type(ascii_string(u16_be)),
    //
    //     s.c.value(values::size_bytes()),
    //     s.b.as(u16_le),
    //     s.c.value(values::data()),
    // );

    // auto def = test::definition2(
    //     bind(s.a, as(u8)),
    //
    //     bind(s.c, type(ascii_string(u16_be))),
    //
    //     bind(s.c, value(values::size_bytes())),
    //     bind(s.b, as(u16_le)),
    //     bind(s.c, value(values::data())),
    // );

    // auto def = test::definition2(
    //     bind(s.a, u8),
    //
    //     type(s.c, ascii_string(u16_be)),
    //
    //     value(s.c, values::size_bytes()),
    //     as(s.b, u16_le),
    //     value(s.c, values::data()),
    // );

    auto print_list = [](char const* s, auto t){
        println("  ", s, ":");
        test::apply(t, [](auto... xs){
            (println("    ",
                // mp::call<test::detail::mp_indexed_index_t<>, decltype(xs)>::value, "\t",
                type_name(xs)
                // type_name<mp::call<test::detail::mp_indexed_type_t<>, decltype(xs)>>()
                ), ...);
        });
    };

    uint8_t a = 0x78;
    uint16_t b = 0x1f23;

    // println('\n', type_name(test::definition2(u8[s.a])));
    println("definition:\n  ", type_name(def));
    print_list("params", def.params());
    print_list("values", def.values());

    print("\n\ninplace_emit:\n\n");
    std::array<uint8_t, 30> buf {};
    auto out = test::inplace_emit(buf, def, s.b = b, s.a = a, s.c = "plop"_av);
    dump(out);

    {
        print("\n\ninplace_recv:\n\n");
        char strbuf[10];
        a = '0';
        auto datas = test::inplace_recv(out, def, s.b = native(), s.a = ref{a}, s.c = make_array_view(strbuf));
        println("\n", type_name(datas));
        println("datas.a = ", std::hex, datas.a, " ", type_name<decltype(datas.a)>());
        println("datas.b = ", std::hex, datas.b, " ", type_name<decltype(datas.b)>());
        println("datas.c = ", datas.c, " ", type_name<decltype(datas.c)>());
        println("a = ", a);
    }

    {
        print("\n\ninplace_recv2:\n\n");
        char strbuf[10];
        a = '0';
        auto datas = test::inplace_recv2(out, def, s.b = native(), s.a = ref{a}, s.c = make_array_view(strbuf));
        println("\n", type_name(datas));
        println("datas.a = ", std::hex, datas.a, " ", type_name<decltype(datas.a)>());
        println("datas.b = ", std::hex, datas.b, " ", type_name<decltype(datas.b)>());
        println("datas.c = ", datas.c, " ", type_name<decltype(datas.c)>());
        println("a = ", a);
    }
}
