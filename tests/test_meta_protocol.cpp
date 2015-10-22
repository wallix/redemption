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

#include <type_traits>
#include <tuple>

// #include "RapidTuple/include/rapidtuple/tuple.hpp"


static void escape(void const * p) {
   asm volatile("" : : "g"(p) : "memory");
}

static void clobber() {
   asm volatile("" : : : "memory");
}

#if __cplusplus <= 201103L
namespace meta_ {
    template<class T, T... Ints>
    struct integer_sequence {};

    template<std::size_t... Ints>
    using index_sequence = integer_sequence<std::size_t, Ints...>;

    namespace detail_ {
        template<std::size_t... Indexes>
        struct integer_sequence_ext
        {
            using next = integer_sequence_ext<Indexes..., sizeof...(Indexes)>;
            using seq = integer_sequence<std::size_t, Indexes...>;
        };

        template<std::size_t Num>
        struct make_integer_sequence_ext
        { using type = typename make_integer_sequence_ext<Num-1>::type::next; };

        template<>
        struct make_integer_sequence_ext<0>
        { using type = integer_sequence_ext<>; };

        /// Builds an parameter_index<0, 1, 2, ..., Num-1>.
        template<std::size_t Num>
        struct build_parameter_index
        { using type = typename make_integer_sequence_ext<Num>::type::seq; };

        template<class T, class Seq>
        struct type_sequence_converter;

        template<class T, class U, U... Ints>
        struct type_sequence_converter<T, integer_sequence<U, Ints...>>
        { using type = integer_sequence<T, T(Ints)...>; };
    }

    template<std::size_t N>
    using make_index_sequence = typename detail_::make_integer_sequence_ext<N>::type::seq;

    template<class T, T N>
    using make_integer_sequence = typename detail_::type_sequence_converter<
        T, make_index_sequence<std::size_t(N)>
    >::type;

    template<class... T>
    using index_sequence_for = make_index_sequence<sizeof...(T)>;
}
namespace std {
    using ::meta_::integer_sequence;
    using ::meta_::index_sequence;
    using ::meta_::make_index_sequence;
    using ::meta_::make_integer_sequence;
    using ::meta_::index_sequence_for;
}
#endif

namespace meta {
    template<class T>
    struct static_const {
        static constexpr T value {};
    };

    template<class T>
    constexpr T static_const<T>::value;
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

template<class T>
std::make_index_sequence<std::tuple_size<T>::value>
tuple_to_index_sequence(T const &) {
    return {};
}

template<class Fn, class... T>
struct lazy_fn
{
    std::tuple<T...> t;
    Fn fn;

    template<class Layout>
    void operator()(Layout && layout) const {
        this->apply(layout, tuple_to_index_sequence(this->t));
    }

private:
    template<class Layout, size_t... Ints>
    void apply(Layout && layout, std::integer_sequence<size_t, Ints...>) const {
        using std::get;
        this->fn(layout, get<Ints>(this->t)...);
    }
};

template<class Fn, class... T>
lazy_fn<Fn, T...> lazy(Fn fn, T && ... args) {
    return lazy_fn<Fn, T...>{std::tuple<T...>{args...}, fn};
}

template<class T, class U = void>
struct enable_type {
    using type = U;
};

namespace detail_ {
    template<class T, class = void>
    struct is_layout_impl : std::false_type
    {};

    template<class T>
    struct is_layout_impl<T, typename enable_type<typename T::is_layout>::type> : T::is_layout
    {};
}

template<class T>
using is_layout = detail_::is_layout_impl<typename std::remove_reference<T>::type>;

template<class FnClass>
struct protocol_wrapper
{
    FnClass operator()() const {
        return {};
    }

    template<class T, class... Ts>
    typename std::enable_if<is_layout<T>::value>::type
    operator()(T && arg, Ts && ... args) const {
        FnClass()(arg, args...);
    }

    template<class T, class... Ts>
    typename std::enable_if<!is_layout<T>::value, lazy_fn<FnClass, T, Ts...>>::type
    operator()(T && arg, Ts && ... args) const {
        return {std::tuple<T, Ts...>{arg, args...}, FnClass{}};
    }
};

template<class FnClass>
using protocol_fn = meta::static_const<protocol_wrapper<FnClass>>;

struct pkt_data {};
struct pkt_sz {};
struct pkt_sz_with_header {};

struct dt_tpdu_send_fn
{
    template<class Layout>
    void operator()(Layout && layout) const {
        layout(
          u8<0x03>() // version 3
        , u8<0x00>()
        , u16_be(pkt_sz_with_header())
        , u8<7-5>() // LI
        , u8<X224::DT_TPDU>()
        , u8<X224::EOT_EOT>());
    }
};

namespace {
    constexpr auto && dt_tpdu_send = protocol_fn<dt_tpdu_send_fn>::value;
}


struct signature_send
{
    uint8_t * data;
    size_t len;
    CryptContext & crypt;

    void write(uint8_t * p) const {
        size_t const sig_sz = 8;
        auto & signature = reinterpret_cast<uint8_t(&)[sig_sz]>(*p);
        this->crypt.sign(this->data, this->len, signature);
        this->crypt.decrypt(this->data, this->len);
    }
};

using meta_protocol::sizeof_;
size_<8> sizeof_(signature_send const &) {
    return {};
}

struct sec_send_fn
{
    template<class Layout>
    void operator()(Layout && layout,
        uint8_t * data, size_t data_sz, uint32_t flags, CryptContext & crypt, uint32_t encryptionLevel
    ) const {
        flags |= encryptionLevel ? SEC::SEC_ENCRYPT : 0;
        layout(
            if_(flags, u32_le(flags)),
            if_(flags & SEC::SEC_ENCRYPT, signature_send{data, data_sz, crypt})
        );
    }
};

namespace {
    constexpr auto && sec_send = protocol_fn<sec_send_fn>::value;
}


template<class Fn, class Accu>
Accu fold(Fn const &, Accu accu) {
    return accu;
}

template<class Fn, class Accu, class T, class... Ts>
auto fold(Fn fn, Accu accu, T && arg, Ts && ... args) {
    return fold(fn, fn(accu, arg), args...);
}


struct stream_writer
{
    OutStream & out_stream;

    template<class... Ts>
    void operator()(Ts && ... args) const {
        (void)std::initializer_list<int>{(void(
            this->write(args)
        ), 1)...};
    }

    template<class T, T x, class Tag>
    void write(types::integral<T, x, Tag>) const {
        this->write(types::dyn<T, Tag>{x});
    }

    void write(types::dyn_u8 x) const {
        this->out_stream.out_uint8(x.x);
    }

    void write(types::dyn_u16_be x) const {
        this->out_stream.out_uint16_be(x.x);
    }

    void write(types::dyn_u32_le x) const {
        this->out_stream.out_uint32_le(x.x);
    }

    void write(signature_send const & x) const {
        x.write(this->out_stream.get_current());
        this->out_stream.out_skip_bytes(sizeof_(x));
    }
};



template<class, class U = void> using use = U;

template<class...> struct pack {};

using std::get;

constexpr int test(...) { return 1; }

template<class T>
struct check_protocol_type
{
    using value_type = typename std::decay<T>::type;
    static_assert(types::is_protocol_type<value_type>::value, "isn't a protocol type");
};


template<std::size_t N>
struct Array
{
    size_t size() const { return this->size_; }
    uint8_t * data() { return this->data_; }

    template<size_t... Ints, class TupleSz, class... Packets>
    Array(std::integer_sequence<size_t, Ints...>, TupleSz & t, Packets & ... packets) {
        static_assert(sizeof...(Ints) == sizeof...(packets), "");
        OutStream out_stream(this->data_, N);
        (void)std::initializer_list<int>{(void(
            write(proto::size_<Ints>(), t, out_stream, tuple_to_index_sequence(packets), packets)
        ), 1)...};
        this->size_ = out_stream.get_offset();
    }

private:
    template<size_t Int, class TupleSz, size_t... Ints, class Packet>
    static void write(
        proto::size_<Int>,
        TupleSz & t,
        OutStream & out_stream,
        std::index_sequence<Ints...>,
        Packet & packet
    ) {
        stream_writer{out_stream}(eval_expr(proto::size_<Int>(), t, get<Ints>(packet))...);
    }

    template<size_t Int, class TupleSz, class T, class Tag>
    static types::dyn<T, Tag>
    eval_expr(proto::size_<Int>, TupleSz & t, types::expr<T, pkt_sz, Tag> &) {
        return {T(get<Int+1>(t))};
    }

    template<size_t Int, class TupleSz, class T, class Tag>
    static types::dyn<T, Tag>
    eval_expr(proto::size_<Int>, TupleSz & t, types::expr<T, pkt_sz_with_header, Tag> &) {
        return {T(get<Int>(t))};
    }

    template<size_t Int, class TupleSz, size_t... Ints, class T>
    static T & eval_expr(proto::size_<Int>, TupleSz &, T & x) {
        return x;
    }

    //uint8_t data_[(N + sizeof(void*) - 1u) & -sizeof(void*)];
    //uint8_t data_[sizeof(std::aligned_storage_t<N>)];
    uint8_t data_[sizeof(typename std::aligned_storage<N, sizeof(void*)>::type)];
    size_t size_;
};


template<class Tuple, class Fn, size_t... Ints>
auto apply(std::index_sequence<Ints...>, Tuple && t, Fn fn) {
    return fn(get<Ints>(t)...);
}

template<class Tuple, class Fn>
auto apply(Tuple && t, Fn fn) {
    return apply(tuple_to_index_sequence(t), std::forward<Tuple>(t), fn);
}


template<class Fn, size_t Sz, class Szs, class AccuSzs, class... Packets>
void eval_terminal(Fn && fn, size_<Sz>, Szs & szs, AccuSzs & accu_szs, Packets && ... packets) {
    Array<Sz> arr{tuple_to_index_sequence(accu_szs), accu_szs, packets...};
    assert(Sz == arr.size());
    fn(arr.data(), arr.size());
}

template<class Fn, class Szs, class AccuSzs, class... Packets>
void eval_terminal(Fn && fn, size_t sz, Szs & szs, AccuSzs & accu_szs, Packets && ... packets) {
    constexpr size_t max_rdp_buf_size = 65536;
    assert(sz <= max_rdp_buf_size);
    Array<max_rdp_buf_size> arr{tuple_to_index_sequence(accu_szs), accu_szs, packets...};
    assert(sz == arr.size());
    fn(arr.data(), arr.size());
}

template<size_t Int, class TupleSz, class T, class... Ts>
auto accumulate_size(size_<Int>, TupleSz & szs, T sz, Ts ... sz_others) {
    return accumulate_size(size_<Int-1>{}, szs, sz + get<Int-1>(szs), sz, sz_others...);
}

template<class TupleSz, class T, class... Ts>
std::tuple<T, Ts...> accumulate_size(size_<0>, TupleSz &, T sz, Ts ... szs) {
    return std::tuple<T, Ts...>(sz, szs...);
}

template<class Fn, size_t N, class PacketFns, class EvaluatedPackets>
void eval_packets(Fn && fn, size_<N>, size_<N>, PacketFns && , EvaluatedPackets && evaluated_packets) {
    apply(evaluated_packets, [&fn](auto && ... packet) {
        auto szs = std::make_tuple(apply(packet, [](auto & ... x) {
            return fold(plus{}, size_<0>(), sizeof_(x)...);
        })...);
        constexpr size_t sz_count = sizeof...(packet);
        // tuple(sz3+sz2+sz1, sz2+sz1, sz1)
        auto accu_szs = accumulate_size(size_<sz_count-1>(), szs, get<sz_count-1>(szs));
        eval_terminal(fn, get<0>(accu_szs), szs, accu_szs, packet...);
    });
}

template<class Fn, size_t N, class PacketFns>
void eval_packets(Fn && fn, size_<N>, size_<N>, PacketFns && , std::tuple<>) {
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
void eval_packets(Fn && fn, size_<I>, size_<N>, PacketFns && packet_fns, EvaluatedPackets && evaluated_packets) {
    get<I>(packet_fns)([&](auto && ... elems) {
        static_assert(test(check_protocol_type<decltype(elems)>()...), "");
        branch_evaluator<Fn, I, N, PacketFns, EvaluatedPackets>{fn, packet_fns, evaluated_packets}
        .eval_branchs(size_<0>(), size_<sizeof...(elems)>(), std::tie(elems...));
    });
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
        layout(if_(yes, if_(yes, if_(yes, u8(1)))));
    }
};

BOOST_AUTO_TEST_CASE(TestMetaProtocol)
{
    {
        StaticOutStream<1024> out_stream;
        X224::DT_TPDU_Send(out_stream, 7);
        X224::DT_TPDU_Send(out_stream, 0);

        proto::eval(
            [&](uint8_t * data, size_t size) {
                BOOST_REQUIRE_EQUAL(size, out_stream.get_offset());
                //hexdump(out_stream.get_data(), out_stream.get_offset());
                //hexdump(array.data(), array.size());
                BOOST_REQUIRE(!memcmp(data, out_stream.get_data(), out_stream.get_offset()));
            },
            proto::dt_tpdu_send,
            proto::dt_tpdu_send
        );
    }

    {
        proto::eval(
            [&](uint8_t * data, size_t size) {
                BOOST_REQUIRE_EQUAL(size, 1);
                BOOST_REQUIRE_EQUAL(*data, 1);
            },
            recursive_if_test()
        );
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

        proto::eval(
            [&](uint8_t * data, size_t size) {
                BOOST_REQUIRE_EQUAL(size, out_stream.get_offset());
                BOOST_REQUIRE(!memcmp(data, out_stream.get_data(), out_stream.get_offset()));
            },
            proto::dt_tpdu_send,
            proto::sec_send(data, 10, ~SEC::SEC_ENCRYPT, crypt, 0)
            //[&](auto && layout) { return proto::sec_send(layout, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0); }
        );
    }

    {
        using namespace meta_protocol;

        using T1 = decltype(if_(0, u8<0>()));
        using T2 = decltype(u8<0>());

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

        proto::eval(
            [&](uint8_t * data, size_t size) {
                BOOST_REQUIRE_EQUAL(size, out_stream.get_offset());
                BOOST_REQUIRE(!memcmp(data, out_stream.get_data(), out_stream.get_offset()));
            },
            proto::sec_send(data, 10, 0, crypt, 0)
        );
    }

    {
        StaticOutStream<1024> out_stream;
        uint8_t data[10];
        CryptContext crypt;
        SEC::Sec_Send(out_stream, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0);

        proto::eval(
            [&](uint8_t * data, size_t size) {
                BOOST_REQUIRE_EQUAL(size, out_stream.get_offset());
                BOOST_REQUIRE(!memcmp(data, out_stream.get_data(), out_stream.get_offset()));
            },
            proto::lazy(proto::sec_send, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0)
            //[&](auto && layout) { return proto::sec_send(layout, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0); }
        );
    }

    {
        StaticOutStream<526> data_stream;
        StaticOutStream<1024> out_stream;
        uint8_t data[10];
        CryptContext crypt;
        SEC::Sec_Send(data_stream, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0);
        X224::DT_TPDU_Send(out_stream, data_stream.get_offset());
        out_stream.out_copy_bytes(data_stream.get_data(), data_stream.get_offset());

        proto::eval(
            [&](uint8_t * data, size_t size) {
                BOOST_REQUIRE_EQUAL(size, out_stream.get_offset());
                hexdump(out_stream.get_data(), out_stream.get_offset());
                hexdump(data, size);
                BOOST_REQUIRE(!memcmp(data, out_stream.get_data(), out_stream.get_offset()));
            },
            proto::dt_tpdu_send,
            proto::sec_send(data, 10, ~SEC::SEC_ENCRYPT, crypt, 0)
            //[&](auto && layout) { return proto::sec_send(layout, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0); }
        );
    }

    uint8_t data[10]{};
    CryptContext crypt;

    auto test1 = [&](uint8_t * p) {
        proto::eval(
            [&](uint8_t * data, size_t sz) {
//                 escape(data);
                memcpy(p, data, sz);
//                 clobber();
            },
            proto::dt_tpdu_send,
            proto::sec_send(data, 10, ~SEC::SEC_ENCRYPT, crypt, 0)
            //[&](auto && layout) { return proto::sec_send(layout, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0); }
        );
    };
    auto test2 = [&](uint8_t * p) {
        uint8_t buf[256];
        OutStream out_stream(buf + 126, 126);
        StaticOutStream<128> hstream;
        SEC::Sec_Send(out_stream, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0);
        X224::DT_TPDU_Send(hstream, out_stream.get_offset());
        auto bufp = out_stream.get_data() - hstream.get_offset();
        memcpy(bufp, hstream.get_data(), hstream.get_offset());
        out_stream = OutStream(bufp, out_stream.get_current() - bufp);
        out_stream.out_skip_bytes(out_stream.get_capacity());
//         escape(out_stream.get_data());
//         escape(hstream.get_data());
        memcpy(p, out_stream.get_data(), out_stream.get_offset());
//         clobber();
//         clobber();
    };

    auto bench = [](auto test) {
        std::vector<long long> v;

        for (auto i = 0; i < 1000; ++i) {
            //uint8_t data[262144];
            uint8_t data[1048576];
            auto p = data;
            test(p);
            auto sz = 11;

            using resolution_clock = std::chrono::steady_clock; // std::chrono::high_resolution_clock;

            auto t1 = resolution_clock::now();

            while (static_cast<size_t>(p - data + sz) < sizeof(data)) {
                escape(p);
                test(p);
                clobber();
                p += sz;
            }

            auto t2 = resolution_clock::now();
            v.push_back((t2-t1).count()/1000);
        }
        return v;
    };

    auto v1 = bench(test1);
    auto v2 = bench(test2);

    std::sort(v1.begin(), v1.end());
    std::sort(v2.begin(), v2.end());

    v1 = decltype(v1)(&v1[v1.size()/2-30], &v1[v1.size()/2+29]);
    v2 = decltype(v2)(&v2[v2.size()/2-30], &v2[v2.size()/2+29]);

    std::cerr << "test1\ttest2\n";
    auto it1 = v1.begin();
    for (auto t : v2) {
        std::cerr << *it1 << "\t" << t << "\n";
        ++it1;
    }
}
