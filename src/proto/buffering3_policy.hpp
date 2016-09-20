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

#include "proto/buffering2_policy.hpp"

namespace proto_buffering3 {

using namespace proto_buffering2;

namespace detail
{
    using namespace proto_buffering2::detail;

    template<class>
    struct to_is_pkt_first_list;

    template<class T, class...>
    using enable_type = T;

    template<class T, class... Ts>
    struct to_is_pkt_first_list<brigand::list<T, Ts...>>
    { using type = brigand::list<brigand::bool_<1>, enable_type<brigand::bool_<0>, Ts>...>; };
}

template<class L>
using to_is_pkt_first_list = typename detail::to_is_pkt_first_list<L>::type;

template<class Policy>
struct Buffering3
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

        // var_info_list with only pkt_sz
        using pkt_sz_list = brigand::copy_if<
            var_info_list,
            brigand::bind<
                proto::is_pkt_sz_category,
                brigand::call<proto::desc_type_t>
            >
        >;

        using pkt_ptr_is_first_list = brigand::wrap<
            brigand::transform<ipacket_list_by_var, brigand::call<to_is_pkt_first_list>>,
            brigand::append
        >;

        std::array<uint8_t *, brigand::size<pkt_sz_list>::value> pkt_sz_ptrs;
        std::array<uint8_t *, brigand::size<packet_list>::value> pkt_ptrs;
        Policy const & policy;
        array_view_u8 av;
        uint8_t * buf;

        Impl(Policy const & policy, array_view_u8 av) noexcept : policy(policy), av(av), buf(av.data()) {}

        void impl(Pkts const & ... packets)
        {
            PROTO_TRACE("pkt_sz_ptrs.size: " << this->pkt_sz_ptrs.size() << "\n");

            this->serialize(var_info_list{}, pkt_ptr_is_first_list{}, packets...);
        }

        template<class... VarInfos, class... IsFirstPkt>
        void serialize(brigand::list<VarInfos...>, brigand::list<IsFirstPkt...>, Pkts const & ... pkts)
        {
            (void)std::initializer_list<int>{(void((
                this->serialize_type_(
                    IsFirstPkt{},
                    VarInfos{},
                    larg<VarInfos::ivar::value>(arg<VarInfos::ipacket::value>(pkts...))
                )
            )), 1)...};

            PROTO_TRACE("sizes: ");
            PROTO_ENABLE_IF_TRACE(for (auto p : this->pkt_sz_ptrs) PROTO_TRACE(this->buf - p << " "));
            PROTO_TRACE("\n\n");

            PROTO_TRACE("--- serialize_pkt_sz ---\n");
            (void)std::initializer_list<int>{(void((
                this->serialize_pkt_sz(VarInfos{})
            )), 1)...};
            PROTO_TRACE("---------------------\n\n");

            this->policy.send(array_view_u8{this->av.data(), std::size_t(buf-this->av.data())});
        }

        template<class IsFirstPkt, class VarInfo, class Val>
        void serialize_type_(IsFirstPkt is_first_pkt, VarInfo var_info, Val const & val)
        {
            PROTO_TRACE(name(val) << " = ");
            PROTO_ENABLE_IF_TRACE(this->print(val));
            if (is_first_pkt) {
                this->pkt_ptrs[VarInfo::ipacket::value] = this->buf;
            }
            this->serialize_type(
                proto::buffer_category<typename VarInfo::desc_type>{},
                var_info,
                val
            );
        }

        template<class VarInfo, class Val>
        void serialize_type(proto::tags::limited_buffer, VarInfo, Val const & val)
        {
            std::size_t len = policy.limited_serialize(this->buf, val.x);
            PROTO_TRACE(" [len: " << len << "]\n");
            this->buf += len;
            static_assert(!var_info_is_pkt_sz<VarInfo>{}, "");
        }

        template<class VarInfo, class Val>
        void serialize_type(proto::tags::view_buffer, VarInfo, Val const & val)
        {
            auto av = policy.get_view_buffer(val.x);
            memcpy(this->buf, av.data(), av.size());
            PROTO_TRACE(" [view: 0x" << static_cast<void const *>(this->buf) << " | len: " << av.size() << "]\n");
            this->buf += av.size();
            static_assert(!var_info_is_pkt_sz<VarInfo>{}, "");
        }

# define PROTO_NIL
#ifndef NDEBUG
# define PROTO_ENABLE_IF_DEBUG(...) __VA_ARGS__
#else
# define PROTO_ENABLE_IF_DEBUG(...)
#endif
        template<class VarInfo, class Val>
        void serialize_type(proto::tags::dynamic_buffer, VarInfo, Val const & val)
        {
            PROTO_ENABLE_IF_DEBUG(bool dynamic_is_used = false;)
            // PERFORMANCE or limited_serialize (policy rule)
            this->policy.dynamic_serialize(
                [this PROTO_ENABLE_IF_DEBUG(PROTO_NIL, &dynamic_is_used)]
                (array_view_const_u8 av) {
                    PROTO_ENABLE_IF_DEBUG(dynamic_is_used = true;)
                    memcpy(this->buf, av.data(), av.size());
                    this->buf += av.size();
                    PROTO_TRACE(" [size: " << av.size() << "]");
                    PROTO_TRACE("\n");
                },
                val.x
            );
            assert(dynamic_is_used);
            static_assert(!var_info_is_pkt_sz<VarInfo>{}, "");
        }
#undef PROTO_ENABLE_IF_DEBUG
#undef PROTO_NIL


        template<class VarInfo, class Val>
        void serialize_type(proto::tags::static_buffer, VarInfo, Val & val)
        {
            this->serialize_pkt_sz_with_size_or_var(VarInfo{}, val);
            this->buf += proto::sizeof_<desc_type_t<VarInfo>>{};
            PROTO_TRACE("\n");
        }

        template<class IPacket, class IVar, class T, class Var>
        void serialize_pkt_sz_with_size_or_var(
            var_info<IPacket, IVar, proto::types::pkt_sz<T>> vinfo, Var const &
        ) {
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            this->pkt_sz_ptrs[pkt_idx] = this->buf;
            PROTO_TRACE("  {" << static_cast<void*>(this->buf) << "} { pkt_sz_ptrs[" << pkt_idx << "] }");
        }

        template<class IPacket, class IVar, class T, class Var>
        void serialize_pkt_sz_with_size_or_var(
            var_info<IPacket, IVar, proto::types::pkt_sz_with_self<T>> vinfo, Var const &
        ) {
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            this->pkt_sz_ptrs[pkt_idx] = this->buf;
            PROTO_TRACE("  {" << static_cast<void*>(this->buf) << "} { pkt_sz_ptrs[" << pkt_idx << "] }");
        }

        template<class IPacket, class IVar, class T, std::size_t n, class Var>
        void serialize_pkt_sz_with_size_or_var(
            var_info<IPacket, IVar, detail::pkt_sz_with_size<T, n>>, Var const &
        ) {
            using proto_integer = typename T::type;
            policy.static_serialize(this->buf, proto_integer{checked_cast<typename proto_integer::type>(n)});
            PROTO_TRACE(" = " << n);
        }

        template<class VarInfo, class Var>
        void serialize_pkt_sz_with_size_or_var(VarInfo, Var const & var)
        {
            policy.static_serialize(this->buf, var.x);
        }


        template<class IPacket, class IVar, class T>
        void serialize_pkt_sz(var_info<IPacket, IVar, proto::types::pkt_sz<T>> vinfo) {
            using is_proto_integer = std::is_same<T, brigand::wrap<T, proto::types::integer>>;
            static_assert(is_proto_integer{}, "only proto::types::integer is supported with pkt_sz");
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            auto const sz = this->buf - this->pkt_ptrs[IPacket::value+1];
            PROTO_TRACE("pkt_sz_ptrs[" << pkt_idx << "] {"
                << static_cast<void*>(this->pkt_sz_ptrs[pkt_idx]) << "} = "
                << sz << "\n"
            );
            policy.static_serialize(this->pkt_sz_ptrs[pkt_idx], T{checked_cast<typename T::type>(sz)});
            PROTO_TRACE("\n");
        }

        template<class IPacket, class IVar, class T>
        void serialize_pkt_sz(var_info<IPacket, IVar, proto::types::pkt_sz_with_self<T>> vinfo) {
            using is_proto_integer = std::is_same<T, brigand::wrap<T, proto::types::integer>>;
            static_assert(is_proto_integer{}, "only proto::types::integer is supported with pkt_sz_with_self");
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            auto const sz = this->buf - this->pkt_ptrs[IPacket::value];
            PROTO_TRACE("pkt_sz_ptrs[" << pkt_idx << "] {"
                << static_cast<void*>(this->pkt_sz_ptrs[pkt_idx]) << "} = "
                << sz << "\n"
            );
            policy.static_serialize(this->pkt_sz_ptrs[pkt_idx], T{checked_cast<typename T::type>(sz)});
            PROTO_TRACE("\n");
        }

        template<class IPacket, class IVar, class T>
        static void serialize_pkt_sz(var_info<IPacket, IVar, T>) {
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
    array_view_u8 av;

    template<class... Packets>
    void operator()(Packets const & ... packets) const
    {
        Impl<Packets...> impl{this->policy, this->av};
        impl.impl(packets...);
    }
};

}

using proto_buffering3::Buffering3;
