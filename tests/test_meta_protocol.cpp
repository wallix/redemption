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
    void eval_if(size_<I>, size_<N>, types::if_<Cond, Yes, No> & if_, TupleArgs & t, Ts & ... args) const {
        if (if_.cond()) {
            this->unpack(if_.yes, size_<I>(), size_<N>(), t, args...);
        }
        else {
            this->unpack(if_.no, size_<I>(), size_<N>(), t, args...);
        }
    }

    template<size_t I, size_t N, class Cond, class Yes, class TupleArgs, class... Ts>
    void eval_if(size_<I>, size_<N>, types::if_<Cond, Yes, types::none> & if_, TupleArgs & t, Ts & ... args) const {
        if (if_.cond()) {
            this->unpack(if_.yes, size_<I>(), size_<N>(), t, args...);
        }
        else {
            this->eval_branch(size_<I+1>(), size_<N>(), t, args...);
        }
    }

    template<size_t I, size_t N, class T, class TupleArgs, class... Ts>
    void eval_if(size_<I>, size_<N>, T & a, TupleArgs & t, Ts & ... args) const {
        this->eval_branch(size_<I+1>(), size_<N>(), t, args..., a);
    }

    template<size_t N, size_t... I>
    static std::integer_sequence<size_t, (I + N)...> plus_index(std::integer_sequence<size_t, I...>) {
        return {};
    }

    // recursive if
    template<class Cond, class Yes, class No, size_t I, size_t N, class TupleArgs, class... Ts>
    void unpack(types::if_<Cond, Yes, No> & if_, size_<I>, size_<N>, TupleArgs & t, Ts & ... args) const {
        this->eval_if(size_<I>(), size_<N>(), if_, t, args...);
    }

    // extends TupleArgs: tuple(get<0..I>(), get<0..tuple_size(arg)>(arg), get<I+1..tuple_size(t)>(t))
    template<class... T, size_t I, size_t N, class TupleArgs, class... Ts>
    void unpack(std::tuple<T...> & arg, size_<I>, size_<N>, TupleArgs & t, Ts & ... args) const {
        this->unpack_cat(
            std::make_index_sequence<I>(),
            std::make_index_sequence<sizeof...(T)>(),
            plus_index<I+1u>(std::make_index_sequence<N-I-1u>()),
            arg, size_<I>(), size_<N + sizeof...(T) - 1>(), t, args...
        );
    }

    template<class T, size_t I, size_t N, class TupleArgs, class... Ts>
    void unpack(T & arg, size_<I>, size_<N>, TupleArgs & t, Ts & ... args) const {
        this->eval_branch(size_<I+1>(), size_<N>(), t, args..., arg);
    }

    template<
        size_t... IBefore,
        size_t... IPack,
        size_t... IAfter,
        class Pack, class I, class N, class TupleArgs, class... Ts
    >
    void unpack_cat(
        std::integer_sequence<size_t, IBefore...>,
        std::integer_sequence<size_t, IPack...>,
        std::integer_sequence<size_t, IAfter...>,
        I, N, Pack & pack, I, N, TupleArgs & t, Ts & ... args
    ) const {
        auto newt = std::tie(get<IBefore>(t)..., get<IPack>(pack)..., get<IAfter>(t)...);
        this->eval_branch(I(), N(), newt, args...);
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
        proto::send(
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

        proto::send(
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
//         uint8_t data[10];
//         CryptContext crypt;
//         proto::send(
//             [&](uint8_t * data, size_t sz) {
// //                 escape(data);
//                 memcpy(p, data, sz);
// //                 clobber();
//             },
//             proto::dt_tpdu_send,
//             proto::sec_send(data, 10, ~SEC::SEC_ENCRYPT, crypt, 0)
//             //[&](auto && layout) { return proto::sec_send(layout, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0); }
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
