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
 *   Copyright (C) Wallix 2010-2015
 *   Author(s): Jonathan Poelen
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestVerifier
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT

// #include "stream.hpp"
#include "RDP/x224.hpp"
#include "RDP/sec.hpp"
// #include "RDP/mcs.hpp"

#include "../src/meta_protocol/types.hpp"
#include "../src/meta_protocol/protocol_fn.hpp"
#include "../src/meta_protocol/stream_writer.hpp"

#include <type_traits>
#include <tuple>

// #include "RapidTuple/include/rapidtuple/tuple.hpp"


static void escape(void const * p) {
   asm volatile("" : : "g"(p) : "memory");
}

static void clobber() {
   asm volatile("" : : : "memory");
}

namespace proto {

using namespace meta_protocol;

using std::get;

struct plus
{
    template<class T, class U>
    auto operator()(T&& x, U&& y) const
    -> decltype(std::forward<T>(x) + std::forward<U>(y))
    { return std::forward<T>(x) + std::forward<U>(y); }
};

struct pkt_sz {};
struct pkt_sz_with_header {};

template<class T>
struct pkt_data
{
    T x;
};

}

namespace meta_protocol { namespace types {
    template<class T> struct is_protocol_type<proto::pkt_data<T>> : is_protocol_type<T> {};
} }

namespace proto {

template<class T>
struct check_protocol_type
{
    using value_type = typename std::decay<T>::type;
    static_assert(types::is_protocol_type<value_type>::value, "isn't a protocol type");
};

constexpr int test(...) { return 1; }

template<class T>
pkt_data<T> make_pkt_data(T && x)
{
    static_assert(test(check_protocol_type<T>()), "");
    static_assert(!is_condition<T>::value, "make_pkt_data(if_(...)) -> if_(make_pkt_data(...))");
    return pkt_data<T>{std::forward<T>(x)};
}

template<class T>
auto sizeof_(pkt_data<T> const & pkt)
-> decltype(sizeof_(pkt.x)) {
    return sizeof_(pkt.x);
}

#define RDP_PROTOCOL_STRUCT(name)                               \
    struct name##_fn;                                           \
    namespace {                                                 \
        constexpr auto && name = protocol_fn<name##_fn>::value; \
    }                                                           \
                                                                \
    struct name##_fn

// struct dt_tpdu_send_fn
RDP_PROTOCOL_STRUCT(dt_tpdu_send)
{
    template<class Layout>
    void operator()(Layout && layout) const {
        layout(
          out_u8<0x03>() // version 3
        , out_u8<0x00>()
        , out_u16_be(pkt_sz_with_header())
        , out_u8<7-5>() // LI
        , out_u8<X224::DT_TPDU>()
        , out_u8<X224::EOT_EOT>());
    }
};

// namespace {
//     constexpr auto && dt_tpdu_send = protocol_fn<dt_tpdu_send_fn>::value;
// }



struct out_signature_send
{
    CryptContext & crypt;
};

inline size_<8> sizeof_(out_signature_send const &) {
    return {};
}

inline void write(out_signature_send const & sig, OutStream & out_stream, uint8_t * data, std::size_t len) {
    constexpr auto sig_sz = decltype(sizeof_(sig))();
    auto * array = out_stream.get_current();
    auto & signature = reinterpret_cast<uint8_t(&)[sig_sz]>(*array);
    sig.crypt.sign(data, len, signature);
    sig.crypt.decrypt(data, len);
    out_stream.out_skip_bytes(sig_sz);
}

RDP_PROTOCOL_STRUCT(sec_send)
{
    template<class Layout>
    void operator()(Layout && layout, uint32_t flags, CryptContext & crypt, uint32_t encryptionLevel) const {
        flags |= encryptionLevel ? SEC::SEC_ENCRYPT : 0;
        layout(
            if_(flags, out_u32_le(flags)),
            if_(flags & SEC::SEC_ENCRYPT, pkt_data<out_signature_send>{{crypt}})
        );
    }
};


namespace detail_ {
    template<class Fn, class Accu, class...>
    struct fold_type
    { using type = Accu; };

    template<class Fn, class Accu, class T, class... Ts>
    struct fold_type<Fn, Accu, T, Ts...>
    : fold_type<Fn, decltype(std::declval<Fn>()(std::declval<Accu>(), std::declval<T&&>())), Ts...>
    {};
}

template<class Fn, class Accu>
Accu fold(Fn const &, Accu accu) {
    return accu;
}

template<class Fn, class Accu, class T, class... Ts>
typename detail_::fold_type<Fn, Accu, T&&, Ts&&...>::type
fold(Fn fn, Accu accu, T && arg, Ts && ... args) {
    return fold(fn, fn(accu, arg), args...);
}


template<class, class U = void> using use = U;

template<class...> struct pack {};

template<class... Ts>
size_<sizeof...(Ts)> tuple_size(std::tuple<Ts...> const &) {
    return {};
}


template<std::size_t BufLen, class Szs, class AccuSzs, class TuplePackets>
struct write_evaluator
{
    Szs & szs;
    AccuSzs & accu_szs;
    TuplePackets & packets;

    //uint8_t data_[(N + sizeof(void*) - 1u) & -sizeof(void*)];
    //uint8_t data_[sizeof(std::aligned_storage_t<N>)];
    uint8_t data[sizeof(typename std::aligned_storage<BufLen, sizeof(void*)>::type)];
    size_t size;

    write_evaluator(Szs & szs, AccuSzs & accu_szs, TuplePackets & packets)
    : szs(szs)
    , accu_szs(accu_szs)
    , packets(packets)
    {
        OutStream out_stream(this->data, BufLen);
        this->write_packets(size_<0>(), out_stream);
        this->size = out_stream.get_offset();
    }

private:
    template<size_t I>
    void write_packets(size_<I>, OutStream & out_stream) {
        auto p = out_stream.get_current();
        this->write_packet(
            size_<I>(),
            p,
            out_stream,
            size_<0>(),
            tuple_size(get<I>(this->packets)),
            get<I>(this->packets)
        );
    }

    using size_N = size_<std::tuple_size<typename std::decay<TuplePackets>::type>::value>;
    static_assert(size_N::value == std::tuple_size<AccuSzs>::value, "");
    void write_packets(size_N, OutStream &) {
    }


    template<size_t IPacket, size_t I, size_t N, class Packet>
    void write_packet(
        size_<IPacket>, uint8_t * p, OutStream & out_stream,
        size_<I>, size_<N>, Packet & packet
    ) {
        this->eval_expr(size_<IPacket>(), p, out_stream, size_<I>(), size_<N>(), packet, get<I>(packet));
    }

    template<size_t IPacket, size_t N, class Packet>
    void write_packet(
        size_<IPacket>, uint8_t * p, OutStream & out_stream,
        size_<N>, size_<N>, Packet & packet
    ) {
        this->write_packets(size_<IPacket+1>(), out_stream);
    }


    size_<0> get_data_len(size_N) {
        return {};
    }

    template<size_t I>
    size_t get_data_len(size_<I>) {
        return get<I>(this->accu_szs);
    }


    // out_expr<T, pkt_sz, Tag> to types::out_dyn<T, Tag>
    template<size_t IPacket, size_t I, size_t N, class Packet, class T, class Tag>
    void eval_expr(
        size_<IPacket>, uint8_t * p, OutStream & out_stream,
        size_<I>, size_<N>, Packet & packet,
        types::out_expr<T, pkt_sz, Tag> &
    ) {
        stream_writer{out_stream}(types::out_dyn<T, Tag>{T(
            get_data_len(size_<IPacket+1>())
        )});
        this->write_packet(size_<IPacket>(), p, out_stream, size_<I+1>(), size_<N>(), packet);
    }

    // out_expr<T, pkt_sz_with_header, Tag> to types::out_dyn<T, Tag>
    template<size_t IPacket, size_t I, size_t N, class Packet, class T, class Tag>
    void eval_expr(
        size_<IPacket>, uint8_t * p, OutStream & out_stream,
        size_<I>, size_<N>, Packet & packet,
        types::out_expr<T, pkt_sz_with_header, Tag> &
    ) {
        stream_writer{out_stream}(types::out_dyn<T, Tag>{T(get<IPacket>(this->accu_szs))});
        this->write_packet(size_<IPacket>(), p, out_stream, size_<I+1>(), size_<N>(), packet);
    }

    // out_expr<T, Expr, Tag> to types::out_dyn<T, Tag>
    template<size_t IPacket, size_t I, size_t N, class Packet, class T, class Expr, class Tag>
    void eval_expr(
        size_<IPacket>, uint8_t * p, OutStream & out_stream,
        size_<I>, size_<N>, Packet & packet,
        types::out_expr<T, Expr, Tag> & x
    ) {
        stream_writer{out_stream}(types::out_dyn<T, Tag>{T(x.expr())});
        this->write_packet(size_<IPacket>(), p, out_stream, size_<I+1>(), size_<N>(), packet);
    }

    // pkt_data<T>
    template<size_t IPacket, size_t I, size_t N, class Packet, class T>
    void eval_expr(
        size_<IPacket>, uint8_t * p, OutStream & out_stream,
        size_<I>, size_<N>, Packet & packet,
        pkt_data<T> & pkt
    ) {
        auto sz = sizeof_(pkt.x);
        auto tmp = out_stream.get_current();
        out_stream.out_skip_bytes(sz);
        this->write_packet(size_<IPacket>(), p, out_stream, size_<I+1>(), size_<N>(), packet);
        OutStream substream(tmp, sz);
        p += get<IPacket>(this->szs);
        assert(size_t(out_stream.get_data_end() - p) == get_data_len(size_<IPacket+1>()));
        this->write_packet_with_data(substream, p, pkt.x, get_data_len(size_<IPacket+1>()));
    }

    template<class T, class Expr, class Tag, class Size>
    void write_packet_with_data(
        OutStream & out_stream, uint8_t * p,
        types::out_expr<T, Expr, Tag> & x, Size sz
    ) {
        stream_writer{out_stream}(types::out_dyn<T, Tag>{T(x.expr(p, sz))});
    }

    template<class T, class Size>
    void write_packet_with_data(OutStream & out_stream, uint8_t * p, T & x, Size sz) {
        stream_writer{out_stream}(x, p, sz);
    }

    template<size_t IPacket, size_t I, size_t N, class Packet, class T>
    void eval_expr(
        size_<IPacket>, uint8_t * p, OutStream & out_stream,
        size_<I>, size_<N>, Packet & packet,
        T & x
    ) {
        stream_writer{out_stream}(x);
        this->write_packet(size_<IPacket>(), p, out_stream, size_<I+1>(), size_<N>(), packet);
    }
};

template<class Fn, size_t Sz, class Szs, class AccuSzs, class TuplePackets>
void eval_terminal(Fn && fn, size_<Sz>, Szs & szs, AccuSzs & accu_szs, TuplePackets && packets) {
    write_evaluator<Sz, Szs, AccuSzs, TuplePackets> arr{szs, accu_szs, packets};
    assert(Sz == arr.size);
    fn(arr.data, arr.size);
}

template<class Fn, class Szs, class AccuSzs, class TuplePackets>
void eval_terminal(Fn && fn, size_t sz, Szs & szs, AccuSzs & accu_szs, TuplePackets && packets) {
    constexpr size_t max_rdp_buf_size = 65536;
    assert(sz <= max_rdp_buf_size);
    write_evaluator<max_rdp_buf_size, Szs, AccuSzs, TuplePackets> arr{szs, accu_szs, packets};
    fn(arr.data, arr.size);
}

namespace detail_ {
    template<size_t Int, class TupleSz, class T, class... Ts>
    struct accumulate_size_type
    : accumulate_size_type<
        Int-1,
        TupleSz,
        decltype(std::declval<T>() + std::declval<typename std::tuple_element<Int-1, TupleSz>::type>()),
        T,
        Ts...
    > {};

    template<class TupleSz, class T, class... Ts>
    struct accumulate_size_type<0, TupleSz, T, Ts...>
    { using type = std::tuple<T, Ts...>; };
}

template<class TupleSz, class T, class... Ts>
std::tuple<T, Ts...> accumulate_size(size_<0>, TupleSz &, T sz, Ts ... szs) {
    return std::tuple<T, Ts...>(sz, szs...);
}

template<size_t Int, class TupleSz, class T, class... Ts>
typename detail_::accumulate_size_type<Int, TupleSz, T, Ts...>::type
accumulate_size(size_<Int>, TupleSz & szs, T sz, Ts ... sz_others) {
    return accumulate_size(size_<Int-1>{}, szs, sz + get<Int-1>(szs), sz, sz_others...);
}

struct terminal_evaluator
{
    template<class Fn, size_t... Ints, class TuplePacket>
    void operator()(Fn && fn, std::integer_sequence<size_t, Ints...>, TuplePacket && packets) const {
        // tuple(sizeof_(packets)...)
        auto szs = std::make_tuple(this->packet_size(
            tuple_to_index_sequence(std::get<Ints>(packets)), std::get<Ints>(packets)
        )...);
        constexpr size_t sz_count = sizeof...(Ints);
        // tuple(sz3+sz2+sz1, sz2+sz1, sz1)
        auto accu_szs = accumulate_size(size_<sz_count-1>(), szs, get<sz_count-1>(szs));
        static_assert(sizeof...(Ints) == std::tuple_size<decltype(accu_szs)>::value, "");
        eval_terminal(fn, get<0>(accu_szs), szs, accu_szs, packets);
    }

    template<size_t... Ints, class Packet>
    auto packet_size(std::integer_sequence<size_t, Ints...>, Packet & packet) const
    -> decltype(fold(plus{}, sizeof_(get<Ints>(packet))...)) {
        return fold(plus{}, sizeof_(get<Ints>(packet))...);
    }

    template<class Packet>
    size_<0> packet_size(std::index_sequence<>, Packet & packet) const {
        return {};
    }
};

template<class Fn, size_t N, class PacketFns, class EvaluatedPackets>
void eval_packets(Fn && fn, size_<N>, size_<N>, PacketFns && , EvaluatedPackets && evaluated_packets) {
    terminal_evaluator()(fn, tuple_to_index_sequence(evaluated_packets), evaluated_packets);
}

template<class Fn, size_t N, class PacketFns>
void eval_packets(Fn && fn, size_<N>, size_<N>, PacketFns && , std::tuple<>) {
    fn(nullptr, 0u);
}

template<class Fn, size_t I, size_t N, class PacketFns, class EvaluatedPackets>
void eval_packets(Fn && fn, size_<I>, size_<N>, PacketFns && packet_fns, EvaluatedPackets && evaluated_packets);


template<size_t... Ints, class... Elements, class... Ts>
std::tuple<Elements..., std::tuple<Ts&...>>
extends_tuple_reference(std::integer_sequence<size_t, Ints...>, std::tuple<Elements...> & t, Ts & ... elems) {
    return std::tuple<Elements..., std::tuple<Ts&...>>(get<Ints>(t)..., std::tuple<Ts&...>(elems...));
}

template<class Fn, size_t IPacket, size_t NPacket, class PacketFns, class EvaluatedPackets>
struct branch_evaluator
{
    Fn & fn;
    PacketFns & packet_fns;
    EvaluatedPackets & evaluated_packets;

    template<size_t I, size_t N, class Packet, class... Ts>
    void eval_branchs(size_<I>, size_<N>, Packet && packet, Ts & ... elems) const {
        this->eval_condition(size_<I>(), size_<N>(), get<I>(packet), packet, elems...);
    }

    template<size_t N, class Packet, class... Ts>
    void eval_branchs(size_<N>, size_<N>, Packet && packet, Ts & ... elems) const {
        eval_packets(
            fn, size_<IPacket+1>(), size_<NPacket>(), packet_fns,
            extends_tuple_reference(tuple_to_index_sequence(evaluated_packets), evaluated_packets, elems...)
        );
    }

    template<size_t N, class Packet>
    void eval_branchs(size_<N>, size_<N>, Packet && packet) const {
        eval_packets(
            fn, size_<IPacket+1>(), size_<NPacket>(), packet_fns, evaluated_packets
        );
    }


    // if_ else
    template<size_t I, size_t N, class Cond, class Yes, class No, class Packet, class... Ts>
    void eval_condition(size_<I>, size_<N>, types::if_<Cond, Yes, No> & if_, Packet & packet, Ts & ... args) const {
        if (if_.cond()) {
            this->unpack(if_.yes, size_<I>(), size_<N>(), packet, args...);
        }
        else {
            this->unpack(if_.no, size_<I>(), size_<N>(), packet, args...);
        }
    }

    // if_
    template<size_t I, size_t N, class Cond, class Yes, class Packet, class... Ts>
    void eval_condition(size_<I>, size_<N>, types::if_<Cond, Yes, types::none> & if_, Packet & packet, Ts & ... args) const {
        if (if_.cond()) {
            this->unpack(if_.yes, size_<I>(), size_<N>(), packet, args...);
        }
        else {
            this->eval_branchs(size_<I+1>(), size_<N>(), packet, args...);
        }
    }

    template<size_t I, size_t N, class T, class Packet, class... Ts>
    void eval_condition(size_<I>, size_<N>, T & a, Packet & packet, Ts & ... args) const {
        this->eval_branchs(size_<I+1>(), size_<N>(), packet, args..., a);
    }


    // recursive if
    template<class Cond, class Yes, class No, size_t I, size_t N, class Packet, class... Ts>
    void unpack(types::if_<Cond, Yes, No> & if_, size_<I>, size_<N>, Packet & packet, Ts & ... args) const {
        this->eval_condition(size_<I>(), size_<N>(), if_, packet, args...);
    }

    // extends Packet: tuple(get<0..I>(), get<0..tuple_size(arg)>(arg), get<I+1..tuple_size(packet)>(packet))
    template<class... T, size_t I, size_t N, class Packet, class... Ts>
    void unpack(std::tuple<T...> & arg, size_<I>, size_<N>, Packet & packet, Ts & ... args) const {
        this->unpack_cat(
            std::make_index_sequence<I>(),
            std::make_index_sequence<sizeof...(T)>(),
            plus_index<I+1u>(std::make_index_sequence<N-I-1u>()),
            arg, size_<I>(), size_<N + sizeof...(T) - 1>(), packet, args...
        );
    }

    template<class T, size_t I, size_t N, class Packet, class... Ts>
    void unpack(T & arg, size_<I>, size_<N>, Packet & packet, Ts & ... args) const {
        this->eval_branchs(size_<I+1>(), size_<N>(), packet, args..., arg);
    }


    template<size_t N, size_t... I>
    static std::integer_sequence<size_t, (I + N)...>
    plus_index(std::integer_sequence<size_t, I...>) {
        return {};
    }

    template<
        size_t... IBefore,
        size_t... IPack,
        size_t... IAfter,
        class Pack, class I, class N, class Packet, class... Ts
    >
    void unpack_cat(
        std::integer_sequence<size_t, IBefore...>,
        std::integer_sequence<size_t, IPack...>,
        std::integer_sequence<size_t, IAfter...>,
        Pack & pack, I, N, Packet & packet, Ts & ... args
    ) const {
        auto newt = std::tie(get<IBefore>(packet)..., get<IPack>(pack)..., get<IAfter>(packet)...);
        this->eval_branchs(I(), N(), newt, args...);
    }
};

template<class Fn, size_t I, size_t N, class PacketFns, class EvaluatedPackets>
struct packet_evaluator
{
    branch_evaluator<Fn, I, N, PacketFns, EvaluatedPackets> branch_evaluator_;

    template<class... Elements>
    void operator()(Elements && ... elems) {
        static_assert(test(check_protocol_type<decltype(elems)>()...), "");
        this->branch_evaluator_.eval_branchs(size_<0>(), size_<sizeof...(elems)>(), std::tie(elems...));
    }
};

template<class Fn, size_t I, size_t N, class PacketFns, class EvaluatedPackets>
void eval_packets(Fn && fn, size_<I>, size_<N>, PacketFns && packet_fns, EvaluatedPackets && evaluated_packets) {
    get<I>(packet_fns)(
        packet_evaluator<Fn, I, N, PacketFns, EvaluatedPackets>
        {{fn, packet_fns, evaluated_packets}}
    );
}

template<class Fn, class... PacketFns>
void eval(Fn && fn, PacketFns && ... packet_fns) {
    eval_packets(fn, size_<0>(), size_<sizeof...(packet_fns)>(), std::tie(packet_fns...), std::tuple<>());
}

template<class Fn> void eval(Fn && fn) = delete;

}

#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>

struct recursive_if_test {
    template<class Layout>
    void operator()(Layout && layout) {
        bool yes = 1;
        using namespace proto;
        layout(if_(yes, if_(yes, if_(yes, out_u8(1)))));
    }
};

struct data_modified
{
    template<class Layout>
    void operator()(Layout && layout) const {
        layout(proto::make_pkt_data(meta_protocol::out_u8([](uint8_t * data, std::size_t sz) {
            BOOST_REQUIRE_EQUAL(sz, 1);
            ++data[0];
            return 0xff;
        })));
    }
};

struct simple_byte
{
    template<class Layout>
    void operator()(Layout && layout) const {
        layout(meta_protocol::out_u8<1>());
    }
};

BOOST_AUTO_TEST_CASE(TestMetaProtocol)
{
    {
        bool is_run = false;
        proto::eval(
            [&](uint8_t * data, size_t size) {
                BOOST_REQUIRE_EQUAL(size, 3);
                //hexdump(out_stream.get_data(), out_stream.get_offset());
                //hexdump(array.data(), array.size());
                BOOST_REQUIRE_EQUAL(  1, data[0]);
                BOOST_REQUIRE_EQUAL(255, data[1]);
                BOOST_REQUIRE_EQUAL(  2, data[2]);
                is_run = true;
            },
            simple_byte(),
            data_modified(),
            simple_byte()
        );
        BOOST_REQUIRE(is_run);
    }

    {
        bool is_run = false;
        StaticOutStream<1024> out_stream;
        X224::DT_TPDU_Send(out_stream, 7);
        X224::DT_TPDU_Send(out_stream, 0);

        proto::eval(
            [&](uint8_t * data, size_t size) {
                BOOST_REQUIRE_EQUAL(size, out_stream.get_offset());
                //hexdump(out_stream.get_data(), out_stream.get_offset());
                //hexdump(array.data(), array.size());
                BOOST_REQUIRE(!memcmp(data, out_stream.get_data(), out_stream.get_offset()));
                is_run = true;
            },
            proto::dt_tpdu_send,
            proto::dt_tpdu_send
        );
        BOOST_REQUIRE(is_run);
    }

    {
        bool is_run = false;
        proto::eval(
            [&](uint8_t * data, size_t size) {
                BOOST_REQUIRE_EQUAL(size, 1);
                BOOST_REQUIRE_EQUAL(*data, 1);
                is_run = true;
            },
            recursive_if_test()
        );
        BOOST_REQUIRE(is_run);
    }

    {
        uint8_t data[10];
        CryptContext crypt;

        uint8_t buf[256];
        OutStream out_stream(buf + 126, 126);
        StaticOutStream<128> hstream;
        SEC::Sec_Send(out_stream, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0);
        X224::DT_TPDU_Send(hstream, out_stream.get_offset());
        BOOST_REQUIRE_EQUAL(4, out_stream.get_offset());
        BOOST_REQUIRE_EQUAL(7, hstream.get_offset());
        auto p = out_stream.get_data() - hstream.get_offset();
        BOOST_REQUIRE_EQUAL(11, out_stream.get_current() - p);
        memcpy(p, hstream.get_data(), hstream.get_offset());
        out_stream = OutStream(p, out_stream.get_current() - p);
        out_stream.out_skip_bytes(out_stream.get_capacity());

        bool is_run = false;
        proto::eval(
            [&](uint8_t * data, size_t size) {
                BOOST_REQUIRE_EQUAL(size, out_stream.get_offset());
                BOOST_REQUIRE(!memcmp(data, out_stream.get_data(), out_stream.get_offset()));
                is_run = true;
            },
            proto::dt_tpdu_send(),
            proto::sec_send.bind(~SEC::SEC_ENCRYPT, crypt, 0)
            //[&](auto && layout) { return proto::sec_send.bind((layout, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0); }
        );
        BOOST_REQUIRE(is_run);
    }

    {
        using namespace meta_protocol;

        using T1 = decltype(if_(0, out_u8<0>()));
        using T2 = decltype(out_u8<0>());

        static_assert(is_condition<T1>::value, "");
        static_assert(!is_condition<T2>::value, "");

        static_assert(std::is_same<
            proto::pack<is_condition<T2>, is_condition<T2>>,
            proto::pack<proto::use<T2, std::false_type>, proto::use<T2, std::false_type>>
        >::value, "");

        static_assert(!std::is_same<
            proto::pack<is_condition<T1>, is_condition<T2>>,
            proto::pack<proto::use<T1, std::false_type>, proto::use<T2, std::false_type>>
        >::value, "");
    }

    {
        StaticOutStream<1024> out_stream;
        uint8_t data[10];
        CryptContext crypt;
        SEC::Sec_Send(out_stream, data, 10, 0, crypt, 0);

        bool is_run = false;
        proto::eval(
            [&](uint8_t * data, size_t size) {
                BOOST_REQUIRE_EQUAL(size, out_stream.get_offset());
                BOOST_REQUIRE(!memcmp(data, out_stream.get_data(), out_stream.get_offset()));
                is_run = true;
            },
            proto::sec_send.bind(0, crypt, 0)
        );
        BOOST_REQUIRE(is_run);
    }

    {
        StaticOutStream<1024> out_stream;
        uint8_t data[10];
        CryptContext crypt;
        SEC::Sec_Send(out_stream, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0);

        bool is_run = false;
        proto::eval(
            [&](uint8_t * data, size_t size) {
                BOOST_REQUIRE_EQUAL(size, out_stream.get_offset());
                BOOST_REQUIRE(!memcmp(data, out_stream.get_data(), out_stream.get_offset()));
                is_run = true;
            },
            proto::lazy(proto::sec_send_fn(), ~SEC::SEC_ENCRYPT, crypt, 0)
            //[&](auto && layout) { return proto::sec_send.bind((layout, ~SEC::SEC_ENCRYPT, crypt, 0); }
        );
        BOOST_REQUIRE(is_run);
    }

    {
        StaticOutStream<526> data_stream;
        StaticOutStream<1024> out_stream;
        uint8_t data[10];
        CryptContext crypt;
        SEC::Sec_Send(data_stream, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0);
        X224::DT_TPDU_Send(out_stream, data_stream.get_offset());
        out_stream.out_copy_bytes(data_stream.get_data(), data_stream.get_offset());

        bool is_run = false;
        proto::eval(
            [&](uint8_t * data, size_t size) {
                BOOST_REQUIRE_EQUAL(size, out_stream.get_offset());
                hexdump(out_stream.get_data(), out_stream.get_offset());
                hexdump(data, size);
                BOOST_REQUIRE(!memcmp(data, out_stream.get_data(), out_stream.get_offset()));
                is_run = true;
            },
            proto::dt_tpdu_send(),
            proto::sec_send.bind(~SEC::SEC_ENCRYPT, crypt, 0)
            //[&](auto && layout) { return proto::sec_send.bind((layout, ~SEC::SEC_ENCRYPT, crypt, 0); }
        );
        BOOST_REQUIRE(is_run);
    }

//     auto test1 = [](uint8_t * p) {
//         uint8_t data[10];
//         CryptContext crypt;
//         proto::eval(
//             [&](uint8_t * data, size_t sz) {
// //                 escape(data);
//                 memcpy(p, data, sz);
// //                 clobber();
//             },
//             proto::dt_tpdu_send,
//             proto::sec_send.bind((~SEC::SEC_ENCRYPT, crypt, 0)
//             //[&](auto && layout) { return proto::sec_send.bind((layout, ~SEC::SEC_ENCRYPT, crypt, 0); }
//         );
//     };
//     auto test2 = [](uint8_t * p) {
//         uint8_t data[10];
//         CryptContext crypt;
//         uint8_t buf[256];
//         OutStream out_stream(buf + 126, 126);
//         StaticOutStream<128> hstream;
//         SEC::Sec_Send(out_stream, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0);
//         X224::DT_TPDU_Send(hstream, out_stream.get_offset());
//         auto bufp = out_stream.get_data() - hstream.get_offset();
//         memcpy(bufp, hstream.get_data(), hstream.get_offset());
//         out_stream = OutStream(bufp, out_stream.get_current() - bufp);
//         out_stream.out_skip_bytes(out_stream.get_capacity());
// //         escape(out_stream.get_data());
// //         escape(hstream.get_data());
//         memcpy(p, out_stream.get_data(), out_stream.get_offset());
// //         clobber();
// //         clobber();
//     };
//
//     auto bench = [](auto test) {
//         std::vector<long long> v;
//
//         for (auto i = 0; i < 1000; ++i) {
//             //uint8_t data[262144];
//             uint8_t data[1048576];
//             auto p = data;
//             test(p);
//             auto sz = 11;
//
//             using resolution_clock = std::chrono::steady_clock; // std::chrono::high_resolution_clock;
//
//             auto t1 = resolution_clock::now();
//
//             while (static_cast<size_t>(p - data + sz) < sizeof(data)) {
//                 escape(p);
//                 test(p);
//                 clobber();
//                 p += sz;
//             }
//
//             auto t2 = resolution_clock::now();
//             v.push_back((t2-t1).count()/1000);
//         }
//         return v;
//     };
//
//     auto v1 = bench(test1);
//     auto v2 = bench(test2);
//
//     std::sort(v1.begin(), v1.end());
//     std::sort(v2.begin(), v2.end());
//
//     v1 = decltype(v1)(&v1[v1.size()/2-30], &v1[v1.size()/2+29]);
//     v2 = decltype(v2)(&v2[v2.size()/2-30], &v2[v2.size()/2+29]);
//
//     std::cerr << "test1\ttest2\n";
//     auto it1 = v1.begin();
//     for (auto t : v2) {
//         std::cerr << *it1 << "\t" << t << "\n";
//         ++it1;
//     }
}
