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
#include "utils/stream.hpp"
#include "utils/sugar/bytes_view.hpp"

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

template<class T, class... Xs>
void print_def(proto::tuple<T, Xs...> const&)
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
        return tuple<RawData, Data...>{data...};
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

template<class T>
using name_t = typename T::name;

template<class... Name>
void print_names(names<Name...>)
{
    (print(type_name<Name>(), ", "), ...);
}

template<class Name, class X>
X& get_variable(std::reference_wrapper<proto::value<X, Name> const> const& v)
{
    return v.get().value;
}

template<class Tuple, class... Name>
void print_variable(Tuple const& t, names<Name...>)
{
    (print(get_variable<Name>(t), ", "), ...);
}


template<class BasicType>
struct basic_type_to_cpp_type_impl;

template<class Integral, class Endianess>
struct basic_type_to_cpp_type_impl<proto::datas::types::Integer<Integral, Endianess>>
{
    using type = Integral;
};

template<class BasicType>
using basic_type_to_cpp_type = typename basic_type_to_cpp_type_impl<BasicType>::type;

template<class Data, class... Names, class Tuple>
auto write_variable(proto::param<Data, Names...> const&, Tuple&& t)
{
    return basic_type_to_cpp_type<typename Data::proto_basic_type>(get_variable<Names>(t)...);
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

template<class Buf, class T, class... Param, class... Xs>
void inplace_emit(Buf& buf, proto::tuple<T, Param...> const& def, Xs const&... xs)
{
    PROTO_ASSERT_TYPES(proto::is_param, Param);
    PROTO_ASSERT_TYPES(proto::is_value, Xs);

    proto::tuple<void, std::reference_wrapper<Xs const>...> t{{xs}...};

    OutStream out(buf);

    ([&]{
        using data_type = typename Param::data_type;
        using basic_type = typename data_type::proto_basic_type;

        println(
            "  ", type_name<name_t<Param>>(),
            "| ", type_name<basic_type>(),
            " -> ", get_variable<name_t<Param>>(t)
        );

        write_buf(basic_type{}, out, write_variable(static_cast<Param const&>(def), t));
    }(), ...);
}


class native {};
class view_buffer {};

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
auto read_buf(proto::datas::types::Integer<Int, Endianess> const&, InStream& in, native)
{
    if constexpr (sizeof(Int) == 1) {
        return in.in_uint8();
    }
    else {
        return in.in_uint16_be();
    }
}

template<class Int, class Endianess>
auto read_buf(proto::datas::types::Integer<Int, Endianess> const&, InStream& in, view_buffer)
{
    return in.in_skip_bytes(sizeof(Int));
}

template<class Data>
auto read_variable_impl(native)
{
    return native();
}

template<class Data>
auto read_variable_impl(view_buffer)
{
    return view_buffer();
}

template<class Data, class Name, class Tuple>
auto read_variable(proto::param<Data, Name> const&, Tuple&& t)
{
    return read_variable_impl<Data>(get_variable<Name>(t));
}

template<class Buf, class T, class... Params, class... Xs>
auto inplace_recv(Buf& buf, proto::tuple<T, Params...> const& def, Xs const&... xs)
{
    PROTO_ASSERT_TYPES(proto::is_param, Params);
    PROTO_ASSERT_TYPES(proto::is_value, Xs);
    proto::tuple<void, std::reference_wrapper<Xs const>...> t{{xs}...};

    InStream in(buf);

    auto f = [&](auto const& p){
        using Param = std::remove_const_t<std::remove_reference_t<decltype(p)>>;
        using data_type = typename Param::data_type;
        using basic_type = typename data_type::proto_basic_type;

        println(
            "  ", type_name<name_t<Param>>(),
            "| ", type_name<decltype(get_variable<name_t<Param>>(t))>(),
            " : ", type_name<basic_type>(),
            " => ", type_name<real_type<basic_type, decltype(get_variable<name_t<Param>>(t))>>()
        );

        return typename decltype(Param::name::mem()(proto::wrap_type<decltype(
            read_buf(basic_type{}, in, read_variable(static_cast<Param const&>(def), t))
        )>()))::type{
            read_buf(basic_type{}, in, read_variable(static_cast<Param const&>(def), t))
        };
    };
    return proto::tuple{f(static_cast<Params const&>(def))...};

    // (println("  ",
    //     type_name<typename Data::name>(), " = ",
    //     type_name<typename decltype(Data::name::mem()(proto::wrap_type<real_type<typename Data::proto_basic_type, decltype(get_variable<typename Data::name>(t))>>()))::type>(),
    //     "] :",
    //     type_name<typename decltype(Data::name::mem()(proto::wrap_type<real_type<typename Data::proto_basic_type, decltype(get_variable<typename Data::name>(t))>>()))::type::type>()
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
    out << "[0x";
    auto t = "0123456789abcdef";
    for (auto c : av) {
        out << t[c>>4] << t[c&0xf];
    }
    return out << "]";
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
};

int main()
{
    std::array<uint8_t, 3> buf {};

    struct S {
        PROTO_LOCAL_NAME(a);
        PROTO_LOCAL_NAME(b);
    } s;

    using namespace proto::datas;

    // auto def = proto::definition(s.a.*u8, s.b.*u16_le);
    auto def = proto::definition(u8[s.a], u16_le[s.b]);

    print("def:\n");
    print_def(def);

    uint8_t a = 0x78;
    uint16_t b = 0x1f23;

    dump(buf);
    print("emit:\n");
    inplace_emit(buf, def, s.b = b, s.a = a);
    dump(buf);

    print("recv:\n");
    auto r = inplace_recv(buf, def, s.b = view_buffer(), s.a = native());
    println("a: ", type_name(r.a), " = ", r.a);
    println("b: ", type_name(r.b), " = ", r.b);
}
