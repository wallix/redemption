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

    template<class T, T N>
    using make_integer_sequence = typename detail_::type_sequence_converter<
        T, typename detail_::make_integer_sequence_ext<std::size_t(N)>::type::seq
    >::type;

    template<std::size_t N>
    using make_index_sequence = make_integer_sequence<std::size_t, N>;

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

    static constexpr protocol_wrapper value{};
};
template<class FnClass>
constexpr protocol_wrapper<FnClass> protocol_wrapper<FnClass>::value;


struct pkt_data {};
struct pkt_sz {};
struct pkt_sz_with_header {};

struct dt_tpdu_send_fn
{
    template<class Layout>
    void operator()(Layout && layout, size_t payload_len) const {
        layout(
          u8<0x03>()
        , u8<0x00>()
        , u16_be(payload_len + 7u)
        , u8<7-5>()
        , u8<X224::DT_TPDU>()
        , u8<X224::EOT_EOT>());
    }

    template<class Layout>
    void operator()(Layout && layout) const {
        layout(
          u8<0x03>()
        , u8<0x00>()
        , u16_be(pkt_sz_with_header())
        , u8<7-5>()
        , u8<X224::DT_TPDU>()
        , u8<X224::EOT_EOT>());
    }
};

namespace {
    constexpr auto && dt_tpdu_send = protocol_wrapper<dt_tpdu_send_fn>::value;
}


struct signature_send
{
    uint8_t * data;
    size_t len;
    CryptContext & crypt;
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
    constexpr auto && sec_send = protocol_wrapper<sec_send_fn>::value;
}

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


template<class Fn, class Tuple, size_t... Ints>
auto tuple_fold(Fn fn, Tuple && t, std::index_sequence<Ints...>)
-> decltype(fold(fn, get<Ints>(t)...)) {
    return fold(fn, get<Ints>(t)...);
}

template<class Fn, class Tuple>
auto tuple_fold(Fn fn, Tuple && t)
-> decltype(tuple_fold(fn, t, tuple_to_index_sequence(t))) {
    return tuple_fold(fn, t, tuple_to_index_sequence(t));
}



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

    void write(types::dyn_u32_le x) const {
        this->out_stream.out_uint32_le(x.x);
    }

    void write(signature_send const & x) const {
        size_t const sig_sz = 8;
        auto & signature = reinterpret_cast<uint8_t(&)[sig_sz]>(*this->out_stream.get_current());
        x.crypt.sign(x.data, x.len, signature);
        this->out_stream.out_skip_bytes(sig_sz);
        x.crypt.decrypt(x.data, x.len);
    }
};



template<class, class U = void> using use = U;

template<class...> struct pack {};

using std::get;

template<class Tuple, class TupleFn, size_t Level, size_t LevelEnd>
struct suitable_layout
{
    Tuple & tuple;
    TupleFn const & fn;

    using is_layout = std::true_type;

    template<class... Ts>
    void operator()(Ts && ... args) const {
        std::tuple<Ts&...> t(args...);
        this->filter_branch(
            std::is_same<
                pack<is_condition<Ts>...>,
                pack<use<Ts, std::false_type>...>
            >(),
            t
        );
    }

    void operator()() const {
        std::tuple<> t;
        this->filter_branch(std::true_type{}, t);
    }

    /// \brief if no branch condition
    template<class TupleArgs>
    void filter_branch(std::true_type, TupleArgs & t) const {
        auto tcat = std::tie(this->tuple, t);
        get<Level>(fn)(suitable_layout<decltype(tcat), TupleFn, Level+1, LevelEnd>{tcat, fn});
    }

    /// \brief if branch condition
    template<class TupleArgs>
    void filter_branch(std::false_type, TupleArgs & t) const {
        this->eval_branch(
            size_<0>(),
            size_<std::tuple_size<TupleArgs>::value>(),
            t
        );
    }

    template<size_t I, size_t N, class TupleArgs, class... Ts>
    void eval_branch(size_<I>, size_<N>, TupleArgs & t, Ts & ... args) const {
        this->eval_if(
            is_condition<typename std::decay<decltype(get<I>(t))>::type>{},
            size_<I>(),
            size_<N>(),
            get<I>(t),
            t,
            args...
        );
    }

    template<size_t N, class TupleArgs, class... Ts>
    void eval_branch(size_<N>, size_<N>, TupleArgs &, Ts & ... args) const {
        std::tuple<Ts&...> t(args...);
        this->filter_branch(std::true_type{}, t);
    }

    template<size_t I, size_t N, class Cond, class Yes, class No, class TupleArgs, class... Ts>
    void eval_if(std::true_type, size_<I>, size_<N>, types::if_<Cond, Yes, No> & if_, TupleArgs & t, Ts & ... args) const {
        static_assert(!is_condition<decltype(if_.yes)>::value, "if_(if_(...)) not implemented");
        static_assert(!is_condition<decltype(if_.no)>::value, "if_(if_(...)) not implemented");
        if (if_.cond()) {
            this->eval_branch(size_<I+1>(), size_<N>(), t, args..., if_.yes);
        }
        else {
            this->eval_branch(size_<I+1>(), size_<N>(), t, args..., if_.no);
        }
    }

    template<size_t I, size_t N, class Cond, class Yes, class TupleArgs, class... Ts>
    void eval_if(std::true_type, size_<I>, size_<N>, types::if_<Cond, Yes, types::none> & if_, TupleArgs & t, Ts & ... args) const {
        static_assert(!is_condition<decltype(if_.yes)>::value, "if_(if_(...)) not implemented");
        if (if_.cond()) {
            this->eval_branch(size_<I+1>(), size_<N>(), t, args..., if_.yes);
        }
        else {
            this->eval_branch(size_<I+1>(), size_<N>(), t, args...);
        }
    }

    template<size_t I, size_t N, class T, class TupleArgs, class... Ts>
    void eval_if(std::false_type, size_<I>, size_<N>, T & a, TupleArgs & t, Ts & ... args) const {
        this->eval_branch(size_<I+1>(), size_<N>(), t, args..., a);
    }
};


template<std::size_t N>
struct Array {
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
        stream_writer_layout{out_stream}(eval_expr(proto::size_<Int>(), t, get<Ints>(packet))...);
    }

    template<size_t Int, class TupleSz, class T, class Tag>
    static types::dyn_base<T, Tag>
    eval_expr(proto::size_<Int>, TupleSz & t, types::expr_base<T, pkt_sz, Tag> &) {
        return {T(get<Int+1>(t))};
    }

    template<size_t Int, class TupleSz, class T, class Tag>
    static types::dyn_base<T, Tag>
    eval_expr(proto::size_<Int>, TupleSz & t, types::expr_base<T, pkt_sz_with_header, Tag> &) {
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

namespace detail_ {
    template<class Seq, class TupleSz, class... Ts>
    struct accumulate_size_type
    { using type = std::tuple<Ts...>; };

    template<size_t Int, size_t... Ints, class TupleSz>
    struct accumulate_size_type<std::index_sequence<Int, Ints...>, TupleSz>
    : accumulate_size_type<
        std::index_sequence<Ints...>,
        TupleSz,
        typename std::tuple_element<Int, TupleSz>::type
    > {};

    template<size_t Int, size_t... Ints, class TupleSz, class T, class... Ts>
    struct accumulate_size_type<std::index_sequence<Int, Ints...>, TupleSz, T, Ts...>
    : accumulate_size_type<
        std::index_sequence<Ints...>,
        TupleSz,
        decltype(std::declval<T>() + std::declval<typename std::tuple_element<Int, TupleSz>::type>()),
        T,
        Ts...
    > {};
}

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

    void operator()() const {
        derec(this->tuple);
    }

    template<class TupleRec, class... Packets>
    void derec(TupleRec && trec, Packets && ... packets) const {
        this->derec(
            std::get<0>(trec),
            std::get<1>(trec),
            packets...
        );
    }

    template<class... Packets>
    void derec(std::tuple<>, Packets && ... packets) const {
        auto szs = std::make_tuple(this->packet_size(tuple_to_index_sequence(packets), packets)...);
        auto accu_szs = accumulate_size(reverse_index(tuple_to_index_sequence(szs)), szs);
        auto sz = tuple_fold(plus{}, accu_szs);
        using sz_t = decltype(sz);
        Array<sz_t::value> arr{tuple_to_index_sequence(accu_szs), accu_szs, packets...};
        get<0>(this->fn)(arr.data(), arr.size());
    }

    template<size_t Int, size_t... Ints, class TupleSz>
    typename detail_::accumulate_size_type<std::index_sequence<Int, Ints...>, TupleSz>::type
    accumulate_size(std::index_sequence<Int, Ints...>, TupleSz & szs) const {
        return accumulate_size(std::index_sequence<Ints...>{}, szs, get<Int>(szs));
    }

    template<size_t Int, size_t... Ints, class TupleSz, class T, class... Ts>
    typename detail_::accumulate_size_type<std::index_sequence<Int, Ints...>, TupleSz, T, Ts...>::type
    accumulate_size(std::index_sequence<Int, Ints...>, TupleSz & szs, T sz, Ts ... sz_others) const {
        return accumulate_size(std::index_sequence<Ints...>{}, szs, sz + get<Int>(szs), sz, sz_others...);
    }

    template<class TupleSz, class... Ts>
    std::tuple<Ts...> accumulate_size(std::index_sequence<>, TupleSz &, Ts ... sz) const {
        return std::tuple<Ts...>(sz...);
    }

    template<size_t... Ints, class Packet>
    auto packet_size(std::index_sequence<Ints...>, Packet & packet) const
    -> decltype(fold(plus{}, sizeof_(get<Ints>(packet))...)) {
        return fold(plus{}, sizeof_(get<Ints>(packet))...);
    }

    template<class Packet>
    size_<0> packet_size(std::index_sequence<>, Packet & packet) const {
        return {};
    }
};

struct terminal_
{
    template<class Layout>
    void operator()(Layout && layout) const {
        layout();
    }
};

template<class Fn, class W, class... Ws>
void send(Fn fn, W && writer, Ws && ... writers) {
    std::tuple<Fn&, Ws&..., terminal_> ws(fn, writers..., terminal_());
    std::tuple<> t;
    writer(suitable_layout<decltype(t), decltype(ws), 1, sizeof...(writers) + 2u>{t, ws});
}

}

#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>

BOOST_AUTO_TEST_CASE(TestMetaProtocol)
{
    {
        StaticOutStream<1024> out_stream;
        X224::DT_TPDU_Send(out_stream, 7);
        X224::DT_TPDU_Send(out_stream, 0);

        proto::send(
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

        proto::send(
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

        proto::send(
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

        proto::send(
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

//     auto test1 = [](uint8_t * p) {
//         auto array = proto::send(
//             [&](auto) {},
//             proto::dt_tpdu_send,
//             proto::dt_tpdu_send
//         );
//         auto sz = array.size();
//         escape(array.data());
//         memcpy(p, array.data(), sz);
//         clobber();
//     };
//     auto test2 = [](uint8_t * p) {
//         StaticOutStream<14> out_stream;
//         X224::DT_TPDU_Send(out_stream, 7);
//         X224::DT_TPDU_Send(out_stream, 0);
//         escape(out_stream.get_data());
//         memcpy(p, out_stream.get_data(), out_stream.get_offset());
//         clobber();
//     };
//
//     auto bench = [](auto test) {
//         std::vector<long long> v;
//
//         for (auto i = 0; i < 10000; ++i) {
//             //uint8_t data[262144];
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
//     auto v2 = bench(test2);
//     auto v1 = bench(test1);
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
