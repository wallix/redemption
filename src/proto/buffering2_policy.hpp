/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once


#include "proto/proto.hpp"
#include "proto/iovec.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#if defined(IN_IDE_PARSER)
# define PROTO_TRACE std::cout <<
# define PROTO_ENABLE_IF_TRACE
#elif defined(ENABLE_PROTO_TRACE)
# include "proto/printer_policy.hpp"
# include <iostream>
# define PROTO_TRACE(...) std::cout << __VA_ARGS__
# define PROTO_ENABLE_IF_TRACE(...) __VA_ARGS__
#else
# define PROTO_TRACE(...) void(0)
# define PROTO_ENABLE_IF_TRACE(...) void(0)
#endif

#ifdef __clang__
# include <tuple>
#endif

namespace proto_buffering2 {

#ifdef IN_IDE_PARSER
#define PROTO_DECLTYPE_AUTO_RETURN(expr) -> decltype(expr) { return (expr); }
#else
#define PROTO_DECLTYPE_AUTO_RETURN(...) -> decltype(__VA_ARGS__) { return (__VA_ARGS__); }
#endif

#ifndef __clang__
namespace detail {
    template<class T> struct arg_impl;
    template<class... Ts>
    struct arg_impl<brigand::list<Ts...>>
    {
        template<class T> static T * impl(Ts..., T * x, ...) { return x; }
    };
}
#endif

template<std::size_t i, class... T>
auto const & arg(T const & ... args)
{
#ifdef __clang__
    // inlining with clang
    return std::get<i>(std::forward_as_tuple(args...));
#else
    // not inlining with clang
    return *detail::arg_impl<brigand::filled_list<void const *, i>>::impl(&args...);
#endif
}

template<std::size_t i, class L>
auto const & larg(L const & l)
{ return l.apply([](auto const & ... v) PROTO_DECLTYPE_AUTO_RETURN(arg<i>(v...))); }


template<class T>
using is_buffer_delimiter = brigand::bool_<
    proto::is_dynamic_buffer<T>::value or
    proto::is_view_buffer<T>::value
>;

template<std::size_t n>
using mk_seq = brigand::range<std::size_t, 0, n>;

template<class n>
using mk_seq2 = brigand::range<std::size_t, 0, n::value>;

template<class I, class Size>
using mk_filled_list = brigand::filled_list<I, Size::value>;

template<class IPacket, class IVar, class DescType>
struct var_info {
    using ipacket = IPacket;
    using ivar = IVar;
    using desc_type = DescType;
};

template<class var_info>
using var_info_is_buffer_delimiter = is_buffer_delimiter<typename var_info::desc_type>;

template<std::size_t n> struct static_size : brigand::size_t<n> {};
template<std::size_t n> struct dynamic_size : brigand::size_t<n> {};
template<std::size_t n> struct limited_size : brigand::size_t<n> {};

namespace lazy {
    template<class p, class i>
    struct add_size_impl;

    template<template<std::size_t> class Size, std::size_t n, class add>
    struct add_size_impl<Size<n>, add>
    { using type = Size<(n+add::value)>; };

    template<template<std::size_t> class Size, std::size_t n>
    struct add_size_impl<Size<n>, proto::dyn_size>
    { using type = dynamic_size<n>; };

    template<template<std::size_t> class Size, std::size_t n1, std::size_t n2>
    struct add_size_impl<Size<n1>, proto::limited_size<n2>>
    { using type = Size<n1+n2>; };

    template<std::size_t n1, std::size_t n2>
    struct add_size_impl<brigand::size_t<n1>, proto::limited_size<n2>>
    { using type = limited_size<n1+n2>; };

    template<std::size_t n1, std::size_t n2>
    struct add_size_impl<static_size<n1>, proto::limited_size<n2>>
    { using type = limited_size<n1+n2>; };

    template<std::size_t n1, std::size_t n2>
    struct add_size_impl<brigand::size_t<n1>, brigand::size_t<n2>>
    { using type = brigand::size_t<n1+n2>; };
}
template<class i1, class i2>
using add_size = typename lazy::add_size_impl<i1, i2>::type;

template<class L>
using sizeof_packet = brigand::fold<
    brigand::transform<L, brigand::call<proto::sizeof_>>,
    static_size<0>,
    brigand::call<add_size>
>;

namespace detail {
    template<class T>
    struct limited_size_to_dyn_size
    { using type = T; };

    template<std::size_t n>
    struct limited_size_to_dyn_size<proto::limited_size<n>>
    { using type = proto::dyn_size; };
}
template<class T>
using limited_size_to_dyn_size = typename detail::limited_size_to_dyn_size<T>::type;

template<class L>
using sizeof_packet_with_limited_size_to_dyn_size = brigand::fold<
    brigand::transform<
        brigand::transform<L, brigand::call<proto::sizeof_>>,
        brigand::call<limited_size_to_dyn_size>
    >,
    static_size<0>,
    brigand::call<add_size>
>;

namespace lazy {
    template<class L, class Add>
    struct mk_list_accu;

    template<template<class...> class L, class... Ts, class add>
    struct mk_list_accu<L<Ts...>, add>
    { using type = L<add_size<Ts, add>..., add>; };
}
template<class L, class x>
using mk_list_accu = typename lazy::mk_list_accu<L, x>::type;

template<class L>
using make_accumulate_sizeof_list = brigand::fold<L, brigand::list<>, brigand::call<mk_list_accu>>;


namespace detail {
    template<class T, std::size_t n> struct pkt_sz_with_size { using desc_type = T; };
}

}

namespace proto {
    template<class T, std::size_t n>
    struct sizeof_impl<proto_buffering2::detail::pkt_sz_with_size<T, n>>
    : sizeof_impl<T>
    {};
}

namespace proto_buffering2 {

namespace detail {
    template<template<class> class IsPktSz, class Pkt, class Sz>
    struct convert_pkt_sz
    { using type = Pkt; };

    template<template<class> class IsPktSz, class... Ts, std::size_t n>
    struct convert_pkt_sz<IsPktSz, brigand::list<Ts...>, proto::size_<n>>
    { using type = brigand::list<std::conditional_t<IsPktSz<Ts>{}, pkt_sz_with_size<Ts, n>, Ts>...>; };
}
template<class Pkt, class Sz, class SzNext>
using convert_pkt_sz = typename detail::convert_pkt_sz<
    proto::is_pkt_sz_with_self,
    typename detail::convert_pkt_sz<proto::is_pkt_sz, Pkt, SzNext>::type,
    Sz
>::type;


using proto::desc_type_t;

template<std::size_t i>
using i_ = std::integral_constant<std::size_t, i>;

template<class L>
using mk_sizeof_var_info_list = brigand::transform<
    brigand::transform<L, brigand::call<proto::desc_type_t>>,
    brigand::call<proto::sizeof_>
>;

namespace detail {
    template<class>
    struct sizeof_to_buffer;

    template<std::size_t n>
    struct sizeof_to_buffer<dynamic_size<n>>
    { using type = proto::dyn_size; };

    template<std::size_t n>
    struct uninitialized_buf
    {
        uninitialized_buf() {}
        uninitialized_buf(uninitialized_buf const &) = delete;
        alignas(4) uint8_t buf [n];
    };

    template<std::size_t n>
    struct sizeof_to_buffer<static_size<n>>
    { using type = uninitialized_buf<n>; };

    template<std::size_t n>
    struct sizeof_to_buffer<limited_size<n>>
    { using type = uninitialized_buf<n>; };
}
template<class T>
using sizeof_to_buffer = typename detail::sizeof_to_buffer<T>::type;

template<class L>
using sizeof_var_infos = brigand::fold<
    mk_sizeof_var_info_list<L>,
    static_size<0>,
    brigand::call<add_size>
>;

template<class L>
using buffer_from_var_infos = sizeof_to_buffer<sizeof_var_infos<L>>;

template<class VarInfos>
using var_infos_is_not_dynamic = proto::is_dynamic_buffer<desc_type_t<brigand::front<VarInfos>>>;

template<class T>
using var_info_is_pkt_sz = proto::is_pkt_sz_category<desc_type_t<T>>;

template<class VarInfos>
using var_infos_has_pkt_sz = brigand::any<VarInfos, brigand::call<var_info_is_pkt_sz>>;

namespace detail {
    template<class T>
    struct is_static_size : std::false_type
    {};

    template<std::size_t n>
    struct is_static_size<static_size<n>> : std::true_type
    {};

    template<class T>
    struct is_not_static_size : std::true_type
    {};

    template<std::size_t n>
    struct is_not_static_size<static_size<n>> : std::false_type
    {};

    template<class T>
    struct is_dynamic_size : std::false_type
    {};

    template<std::size_t n>
    struct is_dynamic_size<dynamic_size<n>> : std::true_type
    {};
}
template<class T>
using is_dynamic_size = typename detail::is_dynamic_size<T>::type;
template<class T>
using is_static_size = typename detail::is_static_size<T>::type;
template<class T>
using is_not_static_size = typename detail::is_not_static_size<T>::type;

template<class T, std::size_t n>
struct static_array_view;
template<class T, std::size_t n>
struct static_array_view<T const, n>
{
    static_array_view(T const (&a)[n]) noexcept : av{a} {}
    static_array_view(std::array<T, n> & a) noexcept
      : static_array_view(reinterpret_cast<T(&)[n]>(a.front())) {}
    static_array_view(std::array<T, n> const & a) noexcept
      : static_array_view(reinterpret_cast<T const (&)[n]>(a.front())) {}

    operator array_view<T const> () const noexcept { return {av}; }

    T const * data() const noexcept { return av; }
    std::size_t size() const noexcept { return n; }

    T const * begin() const noexcept { return av; }
    T const * end() const noexcept { return av + n; }

private:
    T const (&av)[n];
};


namespace detail
{
    template<class Ints, class... Ts>
    struct tuple_buf;

    template<std::size_t, class T>
    struct tuple_element
    { T elem; };

    template<std::size_t... Ints, class... Ts>
    struct tuple_buf<std::integer_sequence<std::size_t, Ints...>, Ts...>
    : tuple_element<Ints, Ts>...
    {};
}
template<class... Ts>
using tuple_buf = detail::tuple_buf<std::index_sequence_for<Ts...>, Ts...>;

template<std::size_t... Ints, class... Ts, class F>
void each_element_with_index(
    detail::tuple_buf<std::integer_sequence<std::size_t, Ints...>, Ts...> & t,
    F && f
) {
    (void)std::initializer_list<int>{
        (f(
            static_cast<detail::tuple_element<Ints, Ts>&>(t).elem,
            std::integral_constant<std::size_t, Ints>{}
        ), 1)...
    };
}


namespace detail
{
    template<std::size_t n>
    struct Buffers
    {
        std::array<iovec, n> data {};

        template<class TupleBuf>
        Buffers(TupleBuf & t)
        {
            each_element_with_index(t, [this](auto & elem, auto i) {
                this->init_buf(i, elem);
            });
        }

        iovec & operator[](std::size_t i) noexcept
        {
            return this->data[i];
        }

        static_array_view<iovec const, n> view() const
        {
            return {this->data};
        }

        template<class TupleBuf>
        void reset_ptr(TupleBuf & t)
        {
            each_element_with_index(t, [this](auto & elem, auto i) {
                this->reset_buf_ptr(i, elem);
            });
        }

    private:
        template<class I, std::size_t arr_len>
        void init_buf(I i, uninitialized_buf<arr_len> & uninit_buf) {
            PROTO_TRACE(i << " - " << static_cast<void*>(uninit_buf.buf) << std::endl);
            this->data[i].iov_base = uninit_buf.buf;
            //this->data[i].iov_len = arr_len;
        }

        template<class I>
        void init_buf(I, proto::dyn_size) {
        }

        template<class I, std::size_t arr_len>
        void reset_buf_ptr(I i, uninitialized_buf<arr_len> & uninit_buf) {
            this->data[i].iov_len = static_cast<uint8_t*>(this->data[i].iov_base) - uninit_buf.buf;
            this->data[i].iov_base = uninit_buf.buf;
        }

        template<class I>
        void reset_buf_ptr(I, proto::dyn_size) {
        }
    };

    template<class>
    struct Sizes;

    template<class PktSz>
    struct Sizes<brigand::list<PktSz>>
    {
        std::size_t data[2] { PktSz::value };

        void propagate_size()
        {}
    };

    template<class... PktSz>
    struct Sizes<brigand::list<PktSz...>>
    {
        std::size_t data[sizeof...(PktSz)+1] { PktSz::value... };

        void propagate_size()
        {
            std::size_t i = sizeof...(PktSz);
            while (i-- > 0) {
                this->data[i] += this->data[i+1];
            }
        }
    };
}

template<class T>
using var_to_desc_type = desc_type_t<proto::var_type_t<T>>;

template<class Policy>
struct Buffering2
{
    template<class... Pkts>
    struct Impl
    {
        // [ [ val | pkt_sz_var ... ] ... ]
        using packet_list_ = brigand::list<brigand::transform<typename Pkts::type_list, brigand::call<var_to_desc_type>>...>;

        // [ { static | dynamic | limited }_size<n> ... ]
        using sizeof_by_packet = brigand::transform<packet_list_, brigand::call<sizeof_packet>>;

        // [ { static | dynamic | limited }_size<n> ... ] == [ Xsize<0..N>, Xsize<1..N> ... ]
        using accu_sizeof_by_packet = make_accumulate_sizeof_list<sizeof_by_packet>;

        // [ [ val | pkt_sz_var | pkt_sz_with_size ... ] ... ]
        using packet_list = brigand::transform<
            packet_list_,
            accu_sizeof_by_packet,
            brigand::push_back<brigand::pop_front<accu_sizeof_by_packet>, proto::size_<0>>,
            brigand::call<convert_pkt_sz>
        >;

        // [ size<packet> ... ]
        using packet_count_list = brigand::transform<packet_list, brigand::call<brigand::size>>;

        // [ filled_list<ipacket, size<packet>> ... ]
        using ipacket_list_by_var = brigand::transform<mk_seq<sizeof...(Pkts)>, packet_count_list, brigand::call<mk_filled_list>>;

        // flatten<ipacket_list_by_var>
        using ipacket_list = brigand::wrap<ipacket_list_by_var, brigand::append>;

        // flatten<packet_list>
        using var_list = brigand::wrap<packet_list, brigand::append>;

        // flatten<range<0, size<packet> ... >
        using ivar_list = brigand::wrap<brigand::transform<packet_count_list, brigand::call<mk_seq2>>, brigand::append>;

        // [ var_info<ipacket, ivar, var> ... ]
        using var_info_list = brigand::transform<ipacket_list, ivar_list, var_list, brigand::call<var_info>>;


        // [ [ var_info ] ... ]
        using var_info_list_by_buffer = brigand::split_if<var_info_list, brigand::call<var_info_is_buffer_delimiter>>;


        // [ uninitialized_buf | dyn_size ... ]
        using buffer_list = brigand::transform<var_info_list_by_buffer, brigand::call<buffer_from_var_infos>>;

        // [ { static | dynamic | limited }_size<n> ... ]
        using default_buffer_size = brigand::transform<packet_list_, brigand::call<sizeof_packet_with_limited_size_to_dyn_size>>;

        // var_info_list with only pkt_sz
        using pkt_sz_list = brigand::copy_if<
            var_info_list,
            brigand::bind<
                proto::is_pkt_sz_category,
                brigand::call<proto::desc_type_t>
            >
        >;

        brigand::wrap<buffer_list, tuple_buf> buffer_tuple;
        detail::Buffers<brigand::size<buffer_list>::value> buffers{buffer_tuple};
        std::array<uint8_t *, brigand::size<pkt_sz_list>::value> pkt_sz_ptrs;
        detail::Sizes<default_buffer_size> sizes;
        Policy const & policy;

        Impl(Policy const & policy) noexcept : policy(policy) {}

        void impl(Pkts const & ... packets)
        {
            PROTO_TRACE("pkt_sz_ptrs.size: " << this->pkt_sz_ptrs.size() << "\n");

            PROTO_TRACE("--- serialize_not_dynamic_bufs ---\n");
            this->serialize_not_dynamic_bufs(
                var_info_list_by_buffer{},
                mk_seq2<brigand::size<var_info_list_by_buffer>>{},
                packets...
            );
            PROTO_TRACE("------------------------------\n\n");

            PROTO_TRACE("--- serialize_dynamic_bufs ---\n");
            this->serialize_dynamic_bufs(
                i_<0>{},
                var_info_list_by_buffer{},
                packets...
            );
        }

        template<class... VarInfos, class... Ints>
        void serialize_not_dynamic_bufs(brigand::list<VarInfos...>, brigand::list<Ints...>, Pkts const & ... pkts) {
            (void)std::initializer_list<int>{(void(
                serialize_not_dynamic_buf(
                    var_infos_is_not_dynamic<VarInfos>{},
                    VarInfos{},
                    this->buffers[Ints::value],
                    pkts...
                )
            ), 1)...};
        }

        template<class VarInfos, class... Ts>
        static void serialize_not_dynamic_buf(std::true_type, VarInfos, iovec & buffer, Ts && ...)
        {
            PROTO_TRACE("-------\n(dyn) { 0x" << buffer.iov_base << " }\n");
            (void)buffer;
        }

        template<class... VarInfos>
        void serialize_not_dynamic_buf(
            std::false_type, brigand::list<VarInfos...>, iovec & buffer, Pkts const & ... pkts
        ) {
            PROTO_TRACE("-------\n");
            (void)std::initializer_list<int>{(void(
                this->serialize_type_(
                    VarInfos{}, buffer,
                    larg<VarInfos::ivar::value>(arg<VarInfos::ipacket::value>(pkts...))
                )
            ), 1)...};
        }

        template<class VarInfo, class Val>
        void serialize_type_(VarInfo var_info, iovec & buffer, Val const & val)
        {
            PROTO_TRACE(name(val) << " = ");
            PROTO_ENABLE_IF_TRACE(this->print(val));
            using buf_cat = proto::buffer_category<typename VarInfo::desc_type>;
            this->serialize_type(buf_cat{}, var_info, buffer, val);
        }

        template<class VarInfo, class Val>
        void serialize_type(proto::tags::static_buffer, VarInfo, iovec & buffer, Val & val) {
            this->serialize_pkt_sz_with_size_or_var(VarInfo{}, buffer, val);
            buffer.iov_base = static_cast<uint8_t*>(buffer.iov_base) + proto::sizeof_<desc_type_t<VarInfo>>{};
            PROTO_TRACE("\n");
        }

        template<class IPacket, class IVar, class T, class Var>
        void serialize_pkt_sz_with_size_or_var(
            var_info<IPacket, IVar, proto::types::pkt_sz<T>> vinfo, iovec & buffer, Var const &
        ) {
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            this->pkt_sz_ptrs[pkt_idx] = static_cast<uint8_t*>(buffer.iov_base);
            PROTO_TRACE("  {" << static_cast<void*>(buffer.iov_base) << "} { pkt_sz_ptrs[" << pkt_idx << "] }");
        }

        template<class IPacket, class IVar, class T, class Var>
        void serialize_pkt_sz_with_size_or_var(
            var_info<IPacket, IVar, proto::types::pkt_sz_with_self<T>> vinfo, iovec & buffer, Var const &
        ) {
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            this->pkt_sz_ptrs[pkt_idx] = static_cast<uint8_t*>(buffer.iov_base);
            PROTO_TRACE("  {" << static_cast<void*>(buffer.iov_base) << "} { pkt_sz_ptrs[" << pkt_idx << "] }");
        }

        template<class IPacket, class IVar, class T, std::size_t n, class Var>
        void serialize_pkt_sz_with_size_or_var(
            var_info<IPacket, IVar, detail::pkt_sz_with_size<T, n>>, iovec & buffer, Var const &
        ) {
            using proto_integer = typename T::type;
            policy.static_serialize(
                static_cast<uint8_t*>(buffer.iov_base),
                proto_integer{checked_cast<typename proto_integer::type>(n)}
            );
            PROTO_TRACE(" = " << n);
        }

        template<class VarInfo, class Var>
        void serialize_pkt_sz_with_size_or_var(VarInfo, iovec & buffer, Var const & var)
        {
            policy.static_serialize(static_cast<uint8_t*>(buffer.iov_base), var.x);
        }

        template<class VarInfo, class Val>
        void serialize_type(proto::tags::limited_buffer, VarInfo, iovec & buffer, Val const & val) {
            std::size_t len = policy.limited_serialize(
                static_cast<uint8_t*>(buffer.iov_base),
                val.x
            );
            PROTO_TRACE(" [len: " << len << "]\n");
            buffer.iov_base = static_cast<uint8_t*>(buffer.iov_base) + len;
            this->sizes.data[VarInfo::ipacket::value] += len;
            static_assert(!var_info_is_pkt_sz<VarInfo>{}, "");
        }

        template<class VarInfo, class Val>
        void serialize_type(proto::tags::view_buffer, VarInfo, iovec & buffer, Val const & val) {
            auto av = policy.get_view_buffer(val.x);
            PROTO_TRACE(" [view: 0x" << buffer.iov_base << " | len: " << buffer.iov_len << "]\n");
            buffer.iov_base = const_cast<uint8_t *>(av.data());
            buffer.iov_len = av.size();
            this->sizes.data[VarInfo::ipacket::value] += av.size();
            static_assert(!var_info_is_pkt_sz<VarInfo>{}, "");
        }

        template<class I, class VarInfosByBuffer>
        void serialize_dynamic_bufs(I, VarInfosByBuffer, Pkts const & ... pkts) {
            using new_list = brigand::find<VarInfosByBuffer, brigand::call<var_infos_is_not_dynamic>>;
            using old_size = brigand::size<VarInfosByBuffer>;
            using new_size = brigand::size<new_list>;
            using new_index = brigand::size_t<I::value + (old_size::value - new_size::value)>;

            serialize_dynamic_buf(new_index{}, new_list{}, pkts...);
        }

        template<class I>
        void serialize_dynamic_buf(I, brigand::list<>, Pkts const & ...) {
            PROTO_TRACE("--------------------------\n\n");

            this->buffers.reset_ptr(this->buffer_tuple);

            this->sizes.propagate_size();

            PROTO_TRACE("sizes: ");
            PROTO_ENABLE_IF_TRACE(for (auto i : this->sizes.data) PROTO_TRACE(i << " "));
            PROTO_TRACE("\n\n");

            PROTO_TRACE("--- serialize_pkt_szs ---\n");
            this->serialize_pkt_szs(var_info_list{});
            PROTO_TRACE("---------------------\n\n");

            this->policy.send(this->buffers.view());
        }


# define PROTO_NIL
#ifndef NDEBUG
# define PROTO_ENABLE_IF_DEBUG(...) __VA_ARGS__
#else
# define PROTO_ENABLE_IF_DEBUG(...)
#endif
        template<class I, class VarInfos, class... VarInfosBuffers>
        void serialize_dynamic_buf(I, brigand::list<VarInfos, VarInfosBuffers...>, Pkts const & ... pkts) {
            using var_info = brigand::front<VarInfos>;
            PROTO_ENABLE_IF_DEBUG(int dynamic_buf_ctxfunc_is_used = 0;)
            this->serialize_dyn_type(
                larg<var_info::ivar::value>(arg<var_info::ipacket::value>(pkts...)),
                [this, PROTO_ENABLE_IF_DEBUG(&dynamic_buf_ctxfunc_is_used, PROTO_NIL) &pkts...]
                (array_view_const_u8 av) {
                    PROTO_ENABLE_IF_DEBUG(++dynamic_buf_ctxfunc_is_used;)
                    auto & buffer = this->buffers[I::value];
                    buffer.iov_base = const_cast<uint8_t *>(av.data());
                    buffer.iov_len = av.size();
                    this->sizes.data[var_info::ipacket::value] += av.size();
                    PROTO_TRACE(" [size: " << av.size() << "]");
                    PROTO_TRACE("\n");
                    this->serialize_dynamic_bufs(
                        brigand::size_t<I::value + 1>{},
                        brigand::list<VarInfosBuffers...>{},
                        pkts...
                    );
                }
            );
            assert(dynamic_buf_ctxfunc_is_used == 1);
        }
#undef PROTO_ENABLE_IF_DEBUG
#undef PROTO_NIL

        template<class Val, class Continue>
        void serialize_dyn_type(Val & val, Continue f) {
            PROTO_TRACE(name(val) << " = ");
            PROTO_ENABLE_IF_TRACE(this->print(val));
            this->policy.dynamic_serialize(f, val.x);
        }


        template<class... VarInfos>
        void serialize_pkt_szs(brigand::list<VarInfos...>) {
            (void)std::initializer_list<int>{(void((
                this->serialize_pkt_sz(VarInfos{})
            )), 1)...};
        }

        template<class IPacket, class IVar, class T>
        void serialize_pkt_sz(var_info<IPacket, IVar, proto::types::pkt_sz<T>> vinfo) {
            using is_proto_integer = std::is_same<T, brigand::wrap<T, proto::types::integer>>;
            static_assert(is_proto_integer{}, "only proto::types::integer is supported with pkt_sz");
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            PROTO_TRACE("pkt_sz_ptrs[" << pkt_idx << "] {"
                << static_cast<void*>(this->pkt_sz_ptrs[pkt_idx]) << "} = "
                << this->sizes.data[IPacket::value+1] << "\n"
            );
            policy.static_serialize(
                this->pkt_sz_ptrs[pkt_idx],
                T{checked_cast<typename T::type>(this->sizes.data[IPacket::value+1])}
            );
            PROTO_TRACE("\n");
        }

        template<class IPacket, class IVar, class T>
        void serialize_pkt_sz(var_info<IPacket, IVar, proto::types::pkt_sz_with_self<T>> vinfo) {
            using is_proto_integer = std::is_same<T, brigand::wrap<T, proto::types::integer>>;
            static_assert(is_proto_integer{}, "only proto::types::integer is supported with pkt_sz_with_self");
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            PROTO_TRACE("pkt_sz_ptrs[" << pkt_idx << "] {"
                << static_cast<void*>(this->pkt_sz_ptrs[pkt_idx]) << "} = "
                << this->sizes.data[IPacket::value] << "\n"
            );
            policy.static_serialize(
                this->pkt_sz_ptrs[pkt_idx],
                T{checked_cast<typename T::type>(this->sizes.data[IPacket::value])}
            );
            PROTO_TRACE("\n");
        }

        template<class VarInfo>
        static void serialize_pkt_sz(VarInfo) {
        }


        template<class Var, class T>
        auto name(proto::val<Var, T> const & val)
        { return val.var.name(); }

        template<class T, class Derived>
        static char const * name(proto::var<proto::types::pkt_sz<T>, Derived>)
        { return Derived::name(); }

        template<class T, class Derived>
        static char const * name(proto::var<proto::types::pkt_sz_with_self<T>, Derived>)
        { return Derived::name(); }


        template<class Var, class T>
        static void print(proto::val<Var, T> const & x)
        {
            PROTO_ENABLE_IF_TRACE(Printer::print(x.x, 1));
            (void)x;
        }

        template<class T, class Derived>
        static void print(proto::var<proto::types::pkt_sz<T>, Derived>)
        {
            PROTO_TRACE("[pkt_sz]");
        }

        template<class T, class Derived>
        static void print(proto::var<proto::types::pkt_sz_with_self<T>, Derived>)
        {
            PROTO_TRACE("[pkt_sz_with_self]");
        }

        static void print_buffer_type(proto::tags::static_buffer)
        {
            PROTO_TRACE("[static_buffer]");
        }
        static void print_buffer_type(proto::tags::dynamic_buffer)
        {
            PROTO_TRACE("[dyn_buffer]");
        }
        static void print_buffer_type(proto::tags::view_buffer)
        {
            PROTO_TRACE("[view_buffer]");
        }
        static void print_buffer_type(proto::tags::limited_buffer)
        {
            PROTO_TRACE("[limited_buffer]");
        }
    };

    Policy policy;

    template<class... Packets>
    void operator()(Packets const & ... packets) const
    {
        Impl<Packets...> impl{this->policy};
        impl.impl(packets...);
    }
};

struct base_policy
{
    template<class T>
    static auto static_serialize(uint8_t * p, T const & val)
    {
        return val.static_serialize(p);
    }

    template<class T>
    static auto get_view_buffer(T const & val)
    {
        return val.get_view_buffer();
    }

    template<class T>
    static std::size_t limited_serialize(uint8_t * p, T const & val)
    {
        return val.limited_serialize(p);
    }

    template<class F, class T>
    static void dynamic_serialize(F && f, T const & val)
    {
        val.dynamic_serialize(f);
    }

    static void send(iovec_array iovs) = delete; // please, overrided this function
};

}

using buffering2_policy_base = proto_buffering2::base_policy;
using proto_buffering2::Buffering2;
template<std::size_t n>
using static_iovec_array = proto_buffering2::static_array_view<iovec_array, n>;
