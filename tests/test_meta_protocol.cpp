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
   Copyright (C) Wallix 2010-2013
   Author(s): Raphael Zhou

   Unit test of Verifier module
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestVerifier
#include <boost/test/auto_unit_test.hpp>

// #include "stream.hpp"
#include "RDP/x224.hpp"
// #include "RDP/sec.hpp"
// #include "RDP/mcs.hpp"

#include "../src/meta_protocol/types.hpp"

#include <type_traits>
// #include <tuple>

#include "RapidTuple/include/rapidtuple/tuple.hpp"


static void escape(void const * p) {
   asm volatile("" : : "g"(p) : "memory");
}

static void clobber() {
   asm volatile("" : : : "memory");
}

namespace proto {

using namespace meta_protocol;

struct pkt_data {};
struct pkt_sz {};
struct pkt_sz_with_header {};

struct dt_tpdu_send_fn
{
    template<class Layout>
    auto operator()(Layout && layout, size_t payload_len) const {
        return layout(
          u8<0x03>()
        , u8<0x00>()
        , u16_be(payload_len + 7u)
        , u8<7-5>()
        , u8<X224::DT_TPDU>()
        , u8<X224::EOT_EOT>());
    }

    template<class Layout>
    auto operator()(Layout && layout) const {
        return layout(
          u8<0x03>()
        , u8<0x00>()
        , u16_be(pkt_sz_with_header())
        , u8<7-5>()
        , u8<X224::DT_TPDU>()
        , u8<X224::EOT_EOT>());
    }
} dt_tpdu_send;

// template<class... Ts> using packet_type = std::tuple<Ts...>;
//
// struct packet_layout {
//     template<class... Ts>
//     packet_type<Ts...> operator()(Ts && ... args) const {
//         return packet_type<Ts...>(args...);
//     }
// };

template<class T> struct type_ { using type = T; };

template<size_t N>
struct size_ : std::integral_constant<size_t, N>
{};

template<size_t n1, size_t n2>
size_<n1+n2> operator+(size_<n1> const &, size_<n2> const &) {
    return {};
}


template<class T>
size_<sizeof(T)> sizeof_(type_<T>) {
    return {};
}

template<class Fn, class Accu>
Accu fold(Fn const &, Accu accu) {
    return accu;
}

template<class Fn, class Accu, class T, class... Ts>
auto fold(Fn fn, Accu accu, T && arg, Ts && ... args) {
    return fold(fn, fn(accu, arg), args...);
}

struct count_byte_layout
{
    template<class... Ts>
    void operator()(Ts && ... args) const {
//         return this->count_bytes(size_<0>{}, args...);
    }

//     template<class Sz>
//     size_t count_bytes(Sz) const {
//         return {};
//     }
//
//     template<class T, class... Ts>
//     size_t count_bytes(size_t x, T && arg, Ts && ... args) const {
//         return count_bytes(x + sizeof_(type_<T>{}), args...);
//     }
};

struct stream_writer_layout
{
    OutStream & out_stream;

    template<class... Ts>
    void operator()(Ts && ... args) const {
        (void)std::initializer_list<int>{(void(
            this->write(args)
        ), 1)...};
    }

    template<uint8_t x>
    void write(types::u8<x>) const {
        this->out_stream.out_uint8(x);
    }

    void write(types::dyn_u16_be x) const {
        this->out_stream.out_uint16_be(x.x);
    }
};


struct stream_maker_layout
{
    template<std::size_t N>
    struct Array {
        size_t size() const { return this->size_; }
        uint8_t const * data() const { return this->data_; }

    private:
        friend stream_maker_layout;

        template<class... Ts>
        Array(Ts & ... args) {
            OutStream out_stream(this->data_, N);
            //escape(out_stream.get_data());
            stream_writer_layout{out_stream}(args...);
            //clobber();
            this->size_ = out_stream.get_offset();
        }

        //uint8_t data_[(N + sizeof(void*) - 1u) & -sizeof(void*)];
        //uint8_t data_[sizeof(std::aligned_storage_t<N>)];
        uint8_t data_[sizeof(std::aligned_storage_t<N, sizeof(void*)>)];
        size_t size_;
    };

    template<class... Ts>
    auto operator()(Ts && ... args) const {
        auto sz = fold(std::plus<>{}, sizeof_(type_<Ts>{})...);
        using sz_t = decltype(sz);
        return Array<sz_t::value>(args...);
    }
};

template<class, class U = void> using use = U;

template<class...> struct pack {};

struct if_base {};

template<class T>
using is_condition = std::integral_constant<bool, std::is_base_of<if_base, T>::value>;

using std::get;

template<class Tuple, class TupleFn, size_t Level, size_t LevelEnd>
struct suitable_layout
{
    Tuple & tuple;
    TupleFn const & fn;

    template<class... Ts>
    auto operator()(Ts && ... args) const {
        std::tuple<Ts&...> t(args...);
        return this->filter_branch(
            std::is_same<
                pack<is_condition<Ts>...>,
                pack<use<Ts, std::true_type>...>
            >(),
            t
        );
    }

    auto operator()() const {
        std::tuple<> t;
        return this->filter_branch(std::false_type{}, t);
    }

    /// \brief if no branch condition
    template<class TupleArgs>
    auto filter_branch(std::false_type, TupleArgs & t) const {
        auto tcat = std::tie(this->tuple, t);
        return get<Level>(fn)(suitable_layout<decltype(tcat), TupleFn, Level+1, LevelEnd>{tcat, fn});
    }

    /// \brief if branch condition
    template<class TupleArgs>
    auto filter_branch(std::true_type, TupleArgs & t) const {
        return this->eval_branchs(
            size_<0>(),
            size_<std::tuple_size<TupleArgs>::value>(),
            t
        );
    }

    template<size_t I, size_t N, class TupleArgs, class... Ts>
    auto eval_branchs(size_<I>, size_<N>, TupleArgs & t, Ts & ... args) const {
        return this->eval_if(
            is_condition<std::decay_t<decltype(get<I>(t))>>{},
            size_<I>(),
            size_<N>(),
            t,
            get<I>(t),
            args...
        );
    }

    template<size_t N, class TupleArgs, class... Ts>
    auto eval_branchs(size_<N>, size_<N>, TupleArgs &, Ts & ... args) const {
        std::tuple<Ts&...> t(args...);
        return this->filter_branch(std::false_type{}, t);
    }

    template<size_t I, size_t N, class T, class TupleArgs, class... Ts>
    auto eval_if(std::true_type, size_<I>, size_<N>, T & a, TupleArgs & t, Ts & ... args) const {
        if (a.test()) {
            static_assert(
                !std::is_base_of<if_base, decltype(a.true_())>::value,
                "if_(if_(...)) not implemented"
            );
            this->eval_branchs(size_<I+1>(), size_<N>(), t, args..., a.true_());
        }
        else {
            static_assert(
                !std::is_base_of<if_base, decltype(a.false_())>::value,
                "if_(if_(...)) not implemented"
            );
            this->eval_branchs(size_<I+1>(), size_<N>(), t, args..., a.false_());
        }
    }

    template<size_t I, size_t N, class T, class TupleArgs, class... Ts>
    auto eval_if(std::false_type, size_<I>, size_<N>, T & a, TupleArgs & t, Ts & ... args) const {
        this->eval_branchs(size_<I+1>(), size_<N>(), t, args..., a);
    }
};

template<class T>
static std::make_index_sequence<std::tuple_size<T>::value>
tuple_to_index_sequence(T const &) {
    return {};
}


template<class Fn, class Tuple, size_t... Ints>
auto tuple_fold(Fn fn, Tuple && t, std::index_sequence<Ints...>) {
    return fold(fn, get<Ints>(t)...);
}

template<class Fn, class Tuple>
auto tuple_fold(Fn fn, Tuple && t) {
    return tuple_fold(fn, t, tuple_to_index_sequence(t));
}


template<std::size_t N>
struct Array {
    size_t size() const { return this->size_; }
    uint8_t const * data() const { return this->data_; }

    template<class... Packets>
    Array(Packets & ... packets) {
        OutStream out_stream(this->data_, N);
        (void)std::initializer_list<int>{(void(
            write(out_stream, tuple_to_index_sequence(packets), packets)
        ), 1)...};
        this->size_ = out_stream.get_offset();
    }

private:
    template<size_t... Ints, class... Ts>
    static void write(OutStream & out_stream, std::index_sequence<Ints...>, std::tuple<Ts...> & packet) {
        stream_writer_layout{out_stream}(get<Ints>(packet)...);
    }

    //uint8_t data_[(N + sizeof(void*) - 1u) & -sizeof(void*)];
    //uint8_t data_[sizeof(std::aligned_storage_t<N>)];
    uint8_t data_[sizeof(std::aligned_storage_t<N, sizeof(void*)>)];
    size_t size_;
};

// template<class Tuple, class TupleFn, size_t I>
// struct suitable_layout<Tuple, TupleFn, I, I>
// {
//     Tuple & tuple;
//     TupleFn const & fn;
//
//     template<class T>
//     static std::make_index_sequence<std::tuple_size<std::decay_t<T>>::value>
//     tuple_to_index_sequence(T const &) {
//         return {};
//     }
//
//     auto operator()() const {
//         return derec(this->tuple);
//     }
//
//     template<class TupleRec, class... Ts>
//     auto derec(TupleRec && trec, Ts && ... args) const {
//         return propagate(
//             std::get<0>(trec),
//             std::get<1>(trec),
//             tuple_to_index_sequence(std::get<1>(tuple)),
//             args...
//         );
//     }
//
//     template<class TupleRec, class T, size_t... Ints, class... Ts>
//     auto propagate(TupleRec && trec, T && t, std::integer_sequence<size_t, Ints...>, Ts && ... args) const {
//         return derec(trec, std::get<Ints>(t)..., args...);
//     }
//
//     template<class... Ts>
//     auto derec(std::tuple<>, Ts && ... args) const {
//         auto sz = fold(std::plus<>{}, sizeof_(type_<std::decay_t<Ts>>{})...);
//         using sz_t = decltype(sz);
//         return Array<sz_t::value>{args...};
//     }
// };

template<class List, class Reversed>
struct reverse_index_impl;

template<size_t OldInt, size_t... OldInts, size_t... NewInts>
struct reverse_index_impl<
    std::integer_sequence<size_t, OldInt, OldInts...>,
    std::integer_sequence<size_t, NewInts...>
> : reverse_index_impl<
    std::integer_sequence<size_t, OldInts...>,
    std::integer_sequence<size_t, OldInt, NewInts...>
>
{};

template<size_t... Ints>
struct reverse_index_impl<
    std::integer_sequence<size_t>,
    std::integer_sequence<size_t, Ints...>
>
{ using type = std::integer_sequence<size_t, Ints...>; };

template<class Tuple, class TupleFn, size_t I>
struct suitable_layout<Tuple, TupleFn, I, I>
{
    Tuple & tuple;
    TupleFn const & fn;

    template<size_t... Ints>
    static typename reverse_index_impl<
        std::integer_sequence<size_t, Ints...>,
        std::integer_sequence<size_t>
    >::type
    reverse_index(std::integer_sequence<size_t, Ints...>) {
        return {};
    }


    auto operator()() const {
        return derec(this->tuple);
    }

    template<class TupleRec, class... Packets>
    auto derec(TupleRec && trec, Packets && ... packets) const {
        return this->derec(
            std::get<0>(trec),
            std::get<1>(trec),
            packets...
        );
    }

    template<class... Packets>
    auto derec(std::tuple<>, Packets && ... packets) const {
        auto szs = std::make_tuple(this->packet_size(packets)...);
        auto accu_szs = accumulate_size(reverse_index(tuple_to_index_sequence(szs)), szs);
        auto sz = tuple_fold(std::plus<>{}, accu_szs);
        using sz_t = decltype(sz);
        return Array<sz_t::value>{packets...};
    }

    template<size_t Int, size_t... Ints, class TupleSz>
    auto accumulate_size(std::index_sequence<Int, Ints...>, TupleSz & szs) const {
        return accumulate_size(std::index_sequence<Ints...>{}, szs, get<Int>(szs));
    }

    template<size_t Int, size_t... Ints, class TupleSz, class T, class... Ts>
    auto accumulate_size(std::index_sequence<Int, Ints...>, TupleSz & szs, T sz, Ts ... sz_others) const {
        return accumulate_size(std::index_sequence<Ints...>{}, szs, sz + get<Int>(szs), sz, sz_others...);
    }

    template<class TupleSz, class... Ts>
    auto accumulate_size(std::index_sequence<>, TupleSz &, Ts ... sz) const {
        return std::make_tuple(sz...);
    }

    template<class... Ts>
    auto packet_size(std::tuple<Ts...> & packet) const {
        return fold(std::plus<>{}, sizeof_(type_<std::decay_t<Ts>>{})...);
    }
};

struct terminal_
{
    template<class Layout>
    auto operator()(Layout && layout) const {
        return layout();
    }
};

template<class Fn, class W, class... Ws>
auto send(Fn fn, W && writer, Ws && ... writers) {
    terminal_ terminal;
    auto ws = rapidtuple::tie(fn, writers..., terminal);
    std::tuple<> t;
    return writer(suitable_layout<decltype(t), decltype(ws), 1, sizeof...(writers)+2u>{t, ws});
}

}

#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>

BOOST_AUTO_TEST_CASE(TestMetaProtocol)
{
    //proto::dt_tpdu_send(proto::count_byte_layout{}, 8);

    {
        StaticOutStream<1024> out_stream;
        X224::DT_TPDU_Send(out_stream, 8);

        auto array = proto::dt_tpdu_send(proto::stream_maker_layout{}, 8);

        BOOST_REQUIRE_EQUAL(array.size(), out_stream.get_offset());
        BOOST_REQUIRE(!memcmp(array.data(), out_stream.get_data(), out_stream.get_offset()));
    }

    {
        StaticOutStream<1024> out_stream;
        X224::DT_TPDU_Send(out_stream, 16);
        X224::DT_TPDU_Send(out_stream, 8);

        auto array = proto::send(
            [&](auto) {},
            [](auto && layout) { return proto::dt_tpdu_send(layout, 16); },
            [](auto && layout) { return proto::dt_tpdu_send(layout, 8); }
        );

        BOOST_REQUIRE_EQUAL(array.size(), out_stream.get_offset());
        BOOST_REQUIRE(!memcmp(array.data(), out_stream.get_data(), out_stream.get_offset()));
    }

//     auto test1 = [](uint8_t * p) {
//         auto array = proto::dt_tpdu_send(proto::stream_maker_layout{}, 8);
//         auto sz = array.size();
//         //std::cout << sz << std::endl;
//         memcpy(p, array.data(), sz);
//     };
//     auto test2 = [](uint8_t * p) {
//         StaticOutStream<1024> out_stream;
//         //escape(out_stream.get_data());
//         X224::DT_TPDU_Send(out_stream, 8);
//         //clobber();
//         memcpy(p, out_stream.get_data(), out_stream.get_offset());
//     };
//
//     auto bench = [](auto test) {
//         std::vector<long long> v;
//
//         for (auto i = 0; i < 60; ++i) {
//             uint8_t data[999999];
//             auto p = data;
//             test(p);
//             auto sz = 8;
//
//             using resolution_clock = std::chrono::high_resolution_clock;
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
//     //v1 = decltype(v1)(&v1[v1.size()/2-30], &v1[v1.size()/2+29]);
//     //v2 = decltype(v2)(&v2[v2.size()/2-30], &v2[v2.size()/2+29]);
//
//     std::cerr << "test1\ttest2\n";
//     auto it1 = v1.begin();
//     for (auto t : v2) {
//         std::cerr << *it1 << "\t" << t << "\n";
//         ++it1;
//     }
}

// namespace proto {
//
// using namespace meta_protocol;
//
// struct stream_layout {
//     OutStream & out_stream_;
//
//     stream_layout(OutStream & out_stream)
//     : out_stream_(out_stream)
//     {}
//
//     template<uint8_t x, class... Ts>
//     void operator()(types::u8<x>, Ts const & ...) const {
//         this->out_stream_.out_uint8(x);
//     }
//
//     template<int I, class... Ts>
//     void operator()(types::dyn_u16_be<I>, Ts const & ... args) const {
//         this->out_stream_.out_uint16_be(std::get<I-1>(std::tie(args...)));
//     }
// };
//
//
// struct dt_tpdu_send
// {
//     static struct : _1 <size_t> {} payload_len;
//
//     static constexpr auto description = packet(
//       u8<0x03>()
//     , u8<0x00>()
//     , u16_be(_1 + 7u)
//     , u8<7-5>()
//     , u8<X224::DT_TPDU>()
//     , u8<X224::EOT_EOT>());
//
//     void eval(size_t payload_len) {
//         write_(description, payload_len);
//     }
//
//     template<class Layout>
//     static auto description(Layout & layout, size_t payload_len) {
//         return packet(
//           u8<0x03>()
//         , u8<0x00>()
//         , u16_be(payload_len + 7u)
//         , u8<7-5>()
//         , u8<X224::DT_TPDU>()
//         , u8<X224::EOT_EOT>());
//     }
// };
//
// auto && dt_tpdu_send_fn = static_const<write_fn<dt_tpdu_send>>::value;
//
// struct signature_send
// {
//     static struct : _1 <uint8_t *> {} data;
//     static struct : _2 <size_t> {} len;
//     static struct : _3 <CryptContext &> {} crypt;
//     static struct : _4 <uint8_t[8]>, optional {} signature;
//
//     static constexpr auto description = packet(bytes(signature));
//
//     template<class... Args>
//     static void pre(Args && ... args) {
//         crypt(args...).sign(data(args...), len(args...), signature(args...));
//     }
//
//     template<class... Args>
//     static void post(Args && ... args) {
//         crypt(args...).decrypt(data(args...), len(args...));
//     }
// };
//
// auto && signature_send_fn = static_const<write_fn<signature_send>>::value;
//
// struct sec_send
// {
//     static struct : _1 <uint8_t *> {} data;
//     static struct : _2 <size_t> {} data_sz;
//     static struct : _4 <CryptContext &> {} crypt;
//     static struct : _5 {} encryptionLevel;
//     static struct : _3 {} flags;
//
//     static constexpr auto description = packet(
//       if_(flags, u32_le(flags)),
//       if_(encryptionLevel & var<SEC_ENCRYPT>(), signature_send_fn(
//           data ->* signature_send::data,
//           data_sz ->* signature_send::len,
//           crypt ->* signature_send::crypt
//       ))
//     );
//
//     template<class... Args>
//     static void pre(Args const & ... args) {
//         if (encryptionLevel(args...)) {
//             flags(args...) |= SEC_ENCRYPT;
//         }
//     }
// };
//
// auto && sec_send_fn = static_const<write_fn<sec_send>>::value;
//
// struct out_per_length
// {
//     static struct : _1 <uint16_t> {} payload_len;
//
//     static constexpr auto description = packet(
//         if_(_1 <= var<127>(), u8(_1), u16_be(_1))
//     );
// };
//
// auto && out_per_length_fn = static_const<write_fn<out_per_length>>::value;
//
// struct senddataindication_send
// {
//     static struct : _1 <int> {} initiator;
//     static struct : _2 {} channelId;
//     static struct : _3 {} dataPriority;
//     static struct : _4 {} segmentation;
//     static struct : _5 {} payload_length;
//     static struct : _6 {} encoding;
//
//     static constexpr auto description = packet(
//       u8(MCS::MCSPDU_SendDataIndication << 2)
//     , u16_be(initiator)
//     , u16_be(channelId)
//     , u8((dataPriority << 6)|(segmentation << 4))
//     , out_per_length_fn(payload_length)
//     );
//
//     void eval(uint16_t initiator, uint16_t channelId, uint8_t dataPriority, uint8_t segmentation, size_t payload_length, int encoding) {
//         if (encoding != PER_ENCODING){
//             LOG(LOG_ERR, "SendDataIndication PER_ENCODING mandatory");
//             throw Error(ERR_MCS);
//         }
//         write_(description, ...);
//
//         description(stream, initiator, channelId, ...);
//     }
// };
//
// auto && senddataindication_send_fn = static_const<write_fn<senddataindication_send>>::value;
//
// }
//
// BOOST_AUTO_TEST_CASE(TestMetaProtocol)
// {
//     StaticOutStream<1024> out_stream1;
//     X224::DT_TPDU_Send(out_stream1, 8);
//     // trans.send(out_stream1.get_data(), out_stream1.get_offset())
//
//     StaticOutStream<16/*meta_protocol::sizeof_<proto::dt_tpdu_send>{}*/> out_stream2;
//     proto::dt_tpdu_send_fn(out_stream2, 8);
//     // write_packet(trans, proto::dt_tpdu_send_fn.values(8), proto::dt_tpdu_send_fn.values(8));
//
//     BOOST_REQUIRE_EQUAL(out_stream1.get_offset(), out_stream2.get_offset());
//     BOOST_REQUIRE(!memcmp(out_stream1.get_data(), out_stream2.get_data(), out_stream2.get_offset()));
// }
//
//
//
//
// static inline void send_data_indication_ex( Transport & trans
//                                           , int encryptionLevel, CryptContext & encrypt
//                                           , uint16_t initiator, HStream & stream)
// {
//     // stream........
//
//     BStream security_header(256);
//     SEC::Sec_Send sec(security_header, stream, 0, encrypt, encryptionLevel);
//     stream.copy_to_head(security_header.get_data(), security_header.size());
//
//     OutPerBStream mcs_header(256);
//     MCS::SendDataIndication_Send mcs( mcs_header
//                                     , initiator
//                                     , GCC::MCS_GLOBAL_CHANNEL
//                                     , 1 // dataPriority
//                                     , 3 // segmentation
//                                     , stream.size()
//                                     , MCS::PER_ENCODING);
//
//     BStream x224_header(256);
//     X224::DT_TPDU_Send(x224_header, stream.size() + mcs_header.size());
//
//     trans.send(x224_header, mcs_header, stream);
// }
//
//
// template<class... DataWriter>
// void send_data_indication_ex( Transport & trans
//                             , uint16_t channelId
//                             , int encryptionLevel, CryptContext & encrypt
//                             , uint16_t initiator, DataWriter... data_writer)
// {
//     write_packets(
//         trans,
//         data_writer,
//         [&](StreamSize<256>, OutStream & security_header, uint8_t * data, std::size_t data_sz) {
//             SEC::Sec_Send sec(security_header, data, data_sz, 0, encrypt, encryptionLevel);
//         },
//         [&](StreamSize<256>, OutStream & mcs_header, std::size_t packet_size) {
//             MCS::SendDataIndication_Send mcs( static_cast<OutPerStream &>(mcs_header)
//                                             , initiator
//                                             , channelId
//                                             , 1 // dataPriority
//                                             , 3 // segmentation
//                                             , packet_size
//                                             , MCS::PER_ENCODING);
//         },
//         [](StreamSize<256>, OutStream & x224_header, std::size_t packet_size) {
//             X224::DT_TPDU_Send(x224_header, packet_size);
//         }
//     );
// }
//
// template<class... PackeFns>
// void send_data_indication_ex( Transport & trans
//                             , uint16_t channelId
//                             , int encryptionLevel, CryptContext & encrypt
//                             , uint16_t initiator, PackeFns... packet_fns)
// {
//     send_packets(
//         trans,
//         packet_fns,
//         proto::sec_send_fn(pkt::data, pkt::sz, 0, encrypt, encryptionLevel),
//         proto::senddataindication_send_fn( initiator
//                                          , channelId
//                                          , 1 // dataPriority
//                                          , 3 // segmentation
//                                          , pkt::sz
//                                          , MCS::PER_ENCODING),
//         proto::dt_tpdu_send_fn(pkt::sz)
//     );
// }
