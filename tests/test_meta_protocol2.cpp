#include <iostream>
#include <utility>
#include <cstdint>

#define PROTO_VAR(t, v)                        \
    constexpr struct v                         \
    : ::proto::var<t, v>                       \
    {                                          \
        using ::proto::var<t, v>::var;         \
        using ::proto::var<t, v>::operator = ; \
                                               \
        static constexpr char const *          \
        name() noexcept { return #v; }         \
    } v {}                                     \


#define BRIGAND_NO_BOOST_SUPPORT
#include <brigand/brigand.hpp> // https://github.com/edouarda/brigand + fix on brigand::remove

namespace proto
{
    namespace types {
        struct u8 { using type = uint8_t; };
        struct u16_le { using type = uint16_t; };
        struct bytes { using type = char const *; };
        template<class T> struct pkt_sz { using type = T; };
        template<class T> struct pkt_sz_with_self { using type = T; };
    }

    struct dyn_size {};
    namespace lazy {
        template<class T> struct sizeof_ : sizeof_<typename T::desc_type> {};
        template<> struct sizeof_<types::u8> : std::integral_constant<std::size_t, 1> {};
        template<> struct sizeof_<types::u16_le> : std::integral_constant<std::size_t, 2> {};
        template<> struct sizeof_<types::bytes> { using type = dyn_size; };
        template<class T> struct sizeof_<types::pkt_sz<T>> : sizeof_<T> {};
        template<class T> struct sizeof_<types::pkt_sz_with_self<T>> : sizeof_<T> {};
    }
    template<class T> using sizeof_ = typename lazy::sizeof_<T>::type;


    using std::size_t;

    template<size_t n> using mk_iseq = std::make_integer_sequence<size_t, n>;

    template<class Var>
    struct val
    {
        using var_type = Var;
        typename Var::desc_type::type x;
    };

    template<class T, class Derived>
    struct var
    {
        using desc_type = T;
        using var_type = var;

        template<class U>
        constexpr val<Derived> operator = (U y) const
        { return val<Derived>{y}; }
    };

    template<char const * Name, class>
    struct tag
    {
        constexpr static char const * const name = Name;
    };


    template<class Ints, class... Ts>
    struct description;

    template<class... Ts>
    constexpr description<mk_iseq<sizeof...(Ts)>, Ts...>
    def(Ts... args)
    { return {args...}; }

    template<size_t i, class Var>
    struct value_desc {
        Var x;
    };

    template<size_t... Ints, class... Ts>
    struct description<std::integer_sequence<size_t, Ints...>, Ts...>
    : value_desc<Ints, Ts>...
    {
        using type_list = brigand::list<Ts...>;

        constexpr description(Ts... args)
        : value_desc<Ints, Ts>{args}...
        {}

        template<class... Us>
        constexpr auto
        operator()(Us... args) const
        { return def(args...); }

        template<class F>
        void apply_for_each(F f)
        {
            (void)std::initializer_list<int>{
                (void(f(static_cast<value_desc<Ints, Ts>&>(*this).x)), 1)...
            };
        }

        template<class F>
        decltype(auto) apply(F f)
        {
            return f(static_cast<value_desc<Ints, Ts>&>(*this).x...);
        }
    };
}

namespace XXX {
    PROTO_VAR(proto::types::u8, a);
    PROTO_VAR(proto::types::u8, b);
    PROTO_VAR(proto::types::bytes, c);
    PROTO_VAR(proto::types::u16_le, d);
    PROTO_VAR(proto::types::pkt_sz<proto::types::u8>, sz);
    PROTO_VAR(proto::types::pkt_sz_with_self<proto::types::u8>, sz2);

    constexpr auto desc = proto::def(a, b, c, d, sz, sz2);
}






namespace lazy {
    // TODO rename to is_dyn_buf
    template<class T> struct is_buffer : std::false_type {};

    template<> struct is_buffer<proto::types::bytes> : std::true_type {};
}
template<class T> using is_buffer = typename lazy::is_buffer<T>::type;


struct Printer
{
    template<class var>
    void operator()(proto::val<var> x) const {
        std::cout << var::name() << " = "; print(x.x, 1);
        std::cout << "  isbuf: " << is_buffer<typename var::desc_type>{} << '\n';
    }

    template<class T, class tag>
    void operator()(proto::var<proto::types::pkt_sz<T>, tag>) const {
        std::cout << "[pkt_sz]\n";
    }

    template<class T, class tag>
    void operator()(proto::var<proto::types::pkt_sz_with_self<T>, tag>) const {
        std::cout << "[pkt_sz_with_self]\n";
    }

    template<class T>
    std::enable_if_t<(std::is_integral<T>{} && sizeof(T) < sizeof(int))>
    print(T const & x, int) const
    { std::cout << int(x); }

    template<class T>
    void
    print(T const & x, char) const
    { std::cout << x; }
};


namespace brigand {
    namespace detail {
        template<class Seq, class Pred, class I = index_if<Seq, Pred>>
        struct split_if_impl
        : append_impl<
            list<front<split_at<Seq, I>>>,
            list<list<front<front<pop_front<split_at<Seq, I>>>>>>,
            typename split_if_impl<pop_front<front<pop_front<split_at<Seq, I>>>>, Pred>::type
        >
        {};

        template<class Seq, class Pred>
        struct split_if_impl<Seq, Pred, no_such_type_>
        { using type = list<Seq>; };

        template<template<class...> class L, class Pred>
        struct split_if_impl<L<>, Pred, no_such_type_>
        { using type = list<>; };
    }

    namespace lazy {
        template<class L, class Pred>
        using split_if = detail::split_if_impl<L, Pred>;
    }

    template<class L, class Pred>
    using split_if = typename detail::split_if_impl<L, Pred>::type;
}

namespace detail {
    template<class T> struct arg_impl;
    template<class... Ts>
    struct arg_impl<brigand::list<Ts...>>
    {
        template<class T> static T * impl(Ts..., T * x, ...) { return x; }
    };
}


template<class idx_var>
using buffer_delimiter = is_buffer<typename idx_var::second_type::desc_type>;

template<std::size_t i, class... T>
auto & arg(T & ... args)
{ return *detail::arg_impl<brigand::filled_list<void const *, i>>::impl(&args...); }

#ifdef IN_IDE_PARSER
#define PROTO_DECLTYPE_AUTO_RETURN(expr) -> decltype(expr) { return (expr); }
#else
#define PROTO_DECLTYPE_AUTO_RETURN(...) -> decltype(__VA_ARGS__) { return (__VA_ARGS__); }
#endif

template<std::size_t i, class L>
auto & larg(L && l)
{ return l.apply([](auto & ... v) PROTO_DECLTYPE_AUTO_RETURN(arg<i>(v...))); }

template<class T>
using t_ = typename T::type;

namespace detail {
    template<class T>
    struct var_type_impl
    { using type = T; };

    template<class T>
    struct var_type_impl<proto::val<T>>
    { using type = T; };
}

template<class T>
using var_type = typename detail::var_type_impl<T>::type;


struct Buferring
{
    template<class... Val>
    void operator()(Val ... val) const {
        using iseq = brigand::range<size_t, 0, sizeof...(val)>;
        using list = brigand::list<var_type<Val>...>;
        using list_pair = brigand::transform<iseq, list, brigand::call<brigand::pair>>;
        using list_by_buffer = brigand::split_if<list_pair, brigand::call<buffer_delimiter>>;

        brigand::for_each<list_by_buffer>([&val...](auto g) {
            std::cout << '[';
            brigand::for_each<t_<decltype(g)>>([&val...](auto v) {
                using pair_type = t_<decltype(v)>;
                std::cout << pair_type::second_type::name() << " = ";
                print(arg<pair_type::first_type::value>(val...));
                std::cout << ", ";
            });
            std::cout << "]\n";
        });
    }

    template<class T>
    static void print(proto::val<T> const & x)
    { Printer{}.print(x.x, 1); }

    template<class T, class tag>
    static void print(proto::var<proto::types::pkt_sz<T>, tag>)
    { std::cout << "[pkt_sz]"; }

    template<class T, class tag>
    static void print(proto::var<proto::types::pkt_sz_with_self<T>, tag>)
    { std::cout << "[pkt_sz_with_self]"; }
};


template<std::size_t n>
using mk_seq = brigand::range<std::size_t, 0, n>;

template<class n>
using mk_seq2 = brigand::range<std::size_t, 0, n::value>;

template<class Start, class Size>
using mk_range = brigand::range<std::size_t, Start::value, Start::value + Size::value>;

template<class I, class Size>
using mk_filled_list = brigand::filled_list<I, Size::value>;

template<class IPacket, class IVar, class DescType>
struct var_info {
    using ipacket = IPacket;
    using ivar = IVar;
    using desc_type = DescType;
};

template<class var_info>
using buffer_var_info_delimiter = is_buffer<typename var_info::desc_type>;

template<std::size_t n> struct static_size : brigand::size_t<n> {};
template<std::size_t n> struct dynamic_size : brigand::size_t<n> {};

namespace lazy {
    template<class p, class i>
    struct add_size;

    template<template<std::size_t> class Size, std::size_t n, class add>
    struct add_size<Size<n>, add>
    { using type = Size<(n+add::value)>; };

    template<template<std::size_t> class Size, std::size_t n>
    struct add_size<Size<n>, proto::dyn_size>
    { using type = dynamic_size<n>; };
}
template<class i1, class i2>
using add_size = typename lazy::add_size<i1, i2>::type;

template<class L>
using sizeof_packet = brigand::fold<
    brigand::transform<L, brigand::call<proto::sizeof_>>,
    static_size<0>,
    brigand::call<add_size>
>;

namespace lazy {
    template<class L, class Add>
    struct mk_list_accu;

    template<template<class...> class L, class... Ts, class add>
    struct mk_list_accu<L<Ts...>, add>
    { using type = L<::add_size<Ts, add>..., add>; };
}
template<class L, class x>
using mk_list_accu = typename lazy::mk_list_accu<L, x>::type;

template<class L>
using make_accumulate_sizeof_list = brigand::fold<L, brigand::list<>, brigand::call<mk_list_accu>>;

template<class T>
using desc_type = typename T::desc_type;

template<class T>
using var_to_desc_type = desc_type<var_type<T>>;

template<std::size_t i>
using i_ = std::integral_constant<std::size_t, i>;

template<class L>
using sizeof_var_infos = brigand::fold<
    brigand::transform<
        brigand::transform<L, brigand::call<desc_type>>,
        brigand::call<proto::sizeof_>
    >,
    static_size<0>,
    brigand::call<add_size>
>;

namespace detail {
    template<class>
    struct sizeof_to_buffer;

    template<std::size_t n>
    struct sizeof_to_buffer<dynamic_size<n>>
    { using type = proto::dyn_size; };

    template<std::size_t n>
    struct sizeof_to_buffer<static_size<n>>
    { using type = uint8_t [n]; };
}
template<class T>
using sizeof_to_buffer = typename detail::sizeof_to_buffer<T>::type;

template<class VarInfos>
using is_dynamic_buffer = typename std::is_same<proto::sizeof_<brigand::front<VarInfos>>, proto::dyn_size>::type;

namespace detail {
    template<class T>
    struct is_pkt_sz : std::false_type
    {};

    template<class T>
    struct is_pkt_sz<proto::types::pkt_sz<T>> : std::true_type
    {};

    template<class T>
    struct is_pkt_sz<proto::types::pkt_sz_with_self<T>> : std::true_type
    {};
}
template<class T>
using is_pkt_size = typename detail::is_pkt_sz<T>::type;

template<class VarInfos>
using has_pkt_sz = typename brigand::any<VarInfos, brigand::call<is_pkt_size>>;

namespace detail {
    template<class T>
    struct is_static_size : std::false_type
    {};

    template<std::size_t n>
    struct is_static_size<static_size<n>> : std::true_type
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


struct Buferring2
{
    template<std::size_t n>
    struct Buffers
    {
        struct iovec {
            void  *iov_base;
            size_t iov_len;
        };

        iovec buffers[n];

        template<class Tuple>
        Buffers(Tuple & t)
        {
            // TODO slow
            brigand::for_each<mk_seq<n>>([this, &t](auto I) {
                using i = t_<decltype(I)>;
                this->init_buf(i{}, std::get<i::value>(t));
            });
        }

        template<class I, std::size_t arr_len>
        void init_buf(I i, uint8_t(&a)[arr_len]) {
            this->buffers[i].iov_base = a;
            this->buffers[i].iov_len = arr_len;
        }

        template<class I>
        void init_buf(I, proto::dyn_size) {
        }
    };

    template<class>
    struct Sizes;

    template<class... PktSz>
    struct Sizes<brigand::list<PktSz...>>
    {
        // TODO sizeof -1
        std::size_t sizes[sizeof...(PktSz)] { PktSz::value... };

        template<class Buffers>
        Sizes(Buffers & bufs)
        {
            foldr(mk_seq<sizeof...(PktSz)>{}, brigand::list<PktSz...>{}, bufs);
        }

        template<class Buffers>
        i_<0> foldr(brigand::list<>, brigand::list<>, Buffers &)
        { return {}; }

        template<class I, class... Ints, class Sz, class... Szs, class Buffers>
        std::size_t foldr(brigand::list<I, Ints...>, brigand::list<Sz, Szs...>, Buffers & buffers)
        {
            return plus(
                I{},
                size(I{}, Sz{}, buffers),
                foldr(brigand::list<Ints...>{}, brigand::list<Szs...>{}, buffers)
            );
        }

        template<class I, std::size_t n, class Buffers>
        std::size_t size(I, dynamic_size<n>, Buffers & bufs)
        { return bufs[I{}].iov_len; }

        template<class I, std::size_t n, class Buffers>
        i_<0> size(I, static_size<n>, Buffers &)
        { return {}; }

        template<class I, std::size_t i1, std::size_t i2>
        i_<i1+i2> plus(I, std::integral_constant<std::size_t, i1>, std::integral_constant<std::size_t, i2>)
        { return {}; }

        template<class I>
        std::size_t plus(I, std::size_t i1, std::size_t i2)
        {
            sizes[I{}] += i1+i2;
            return i1+i2;
        }
    };

    template<class... Packets>
    void operator()(Packets ... packets) const {
        using brigand::transform;
        using brigand::append;
        using brigand::fold;
        using brigand::wrap;
        using brigand::call;
        using brigand::size;

        // TODO rapidtuple
        using std::tuple;

        using packet_list = brigand::list<brigand::transform<typename Packets::type_list, brigand::call<var_to_desc_type>>...>;
        using sizeof_by_packet = transform<packet_list, call<sizeof_packet>>;
        using accu_sizeof_by_packet = make_accumulate_sizeof_list<sizeof_by_packet>;
        // TODO here recall with convert pkt_sz static
        using packet_count_list = transform<packet_list, call<size>>;
        using ipacket_list_by_var = transform<mk_seq<sizeof...(Packets)>, packet_count_list, call<mk_filled_list>>;
        using ipacket_list = wrap<ipacket_list_by_var, append>;
        using var_list = wrap<packet_list, append>;
        using ivar_list = wrap<transform<packet_count_list, call<mk_seq2>>, append>;
        using var_info_list = transform<ipacket_list, ivar_list, var_list, call<var_info>>;
        using var_info_list_by_buffer = brigand::split_if<var_info_list, call<buffer_var_info_delimiter>>;

        using sizeof_by_buffer = transform<var_info_list_by_buffer, call<sizeof_var_infos>>;
        using buffer_list = transform<sizeof_by_buffer, call<sizeof_to_buffer>>;
        wrap<buffer_list, tuple> buffer_tuple;
        Buffers<size<buffer_list>::value> buffers(buffer_tuple);

        std::cout << "--- write_not_dynamic_bufs ---\n";
        this->write_not_dynamic_bufs(
            var_info_list_by_buffer{},
            accu_sizeof_by_packet{},
            mk_seq2<size<var_info_list_by_buffer>>{},
            buffers.buffers,
            packets...
        );
        std::cout << "------------------------------\n\n";

        std::cout << "--- write_dynamic_bufs ---\n";
        this->write_dynamic_bufs(
            i_<0>{},
            var_info_list_by_buffer{},
            buffers.buffers,
            packets...
        );
        std::cout << "--------------------------\n\n";

        using accu_sizeof_by_buffer = make_accumulate_sizeof_list<sizeof_by_buffer>;
        Sizes<accu_sizeof_by_buffer> sizes(buffers.buffers);

        std::cout << "--- write_pkt_szs ---\n";
        this->write_pkt_szs(
            i_<0>{},
            var_info_list_by_buffer{},
            buffers.buffers,
            sizes.sizes
        );
        std::cout << "---------------------\n\n";


        //using rlast_dyn_pkt_ = brigand::index_if<brigand::reverse<sizeof_by_packet>, call<is_static_pck_sz>>;
        //using rlast_dyn_pkt = std::conditional_t<
        //    std::is_same<last_dynamic_packet, brigand::no_such_type_>::value,
        //    i_<0>,
        //    last_dynamic_packet
        //>;
        //using last_dyn_pkt = i_<brigand::size<packet_list>::value - last_dynamic_packet::value - 1>;




        // TODO make buffers

        impl(
            i_<0>{}, size<var_info_list_by_buffer>{},
            var_info_list_by_buffer{},
            accu_sizeof_by_packet{},
            packets...
        );
    }

    template<class... VarInfos, class PktSizes, class... Ints, class Buffers, class... Pkts>
    static void write_not_dynamic_bufs(
        brigand::list<VarInfos...>, PktSizes, brigand::list<Ints...>,
        Buffers & buffers, Pkts & ... pkts
    ) {
        (void)std::initializer_list<int>{(void(
            write_not_dynamic_buf(
                is_dynamic_buffer<VarInfos>{},
                VarInfos{}, PktSizes{},
                buffers[Ints::value], pkts...
            )
        ), 1)...};
    }

    template<class... Ts>
    static void write_not_dynamic_buf(std::true_type, Ts && ...)
    { std::cout << "-------\n(dyn)\n"; }

    template<class... VarInfos, class PktSizes, class Buffer, class... Pkts>
    static void write_not_dynamic_buf(
        std::false_type, brigand::list<VarInfos...>,
        PktSizes pkt_sizes, Buffer & buffer, Pkts & ... pkts
    ) {
        std::cout << "-------\n";
        (void)std::initializer_list<int>{(void(
            write_type(
                buffer, pkt_sizes,
                larg<VarInfos::ivar::value>(arg<VarInfos::ipacket::value>(pkts...))
            )
        ), 1)...};
    }

    template<class Buffer, class PktSizes, class Var>
    static void write_type(Buffer &, PktSizes, Var & var) {
        std::cout << var_type<Var>::name() << " = ";
        print(var);
        std::cout << "\n";
    }


    template<class I, class VarInfosByBuffer, class Buffers, class... Pkts>
    static void write_dynamic_bufs(
        I, VarInfosByBuffer, Buffers & buffers, Pkts & ... pkts
    ) {
        using new_list = brigand::find<VarInfosByBuffer, brigand::call<is_dynamic_buffer>>;
        using old_size = brigand::size<VarInfosByBuffer>;
        using new_size = brigand::size<new_list>;
        using new_index = brigand::size_t<I::value + (old_size::value - new_size::value)>;

        write_dynamic_buf(new_index{}, new_list{}, buffers, pkts...);
    }

    template<class I, class Buffers, class... Pkts>
    static void write_dynamic_buf(
        I, brigand::list<>, Buffers &, Pkts & ...
    ) {
    }

    template<class I, class VarInfos, class... VarInfosBuffers, class Buffers, class... Pkts>
    static void write_dynamic_buf(
        I, brigand::list<VarInfos, VarInfosBuffers...>, Buffers & buffers, Pkts & ... pkts
    ) {
        using var_info = brigand::front<VarInfos>;
        write_dyn_type(
            buffers[I::value],
            larg<var_info::ivar::value>(arg<var_info::ipacket::value>(pkts...)),
            [&buffers, &pkts...](){
                write_dynamic_bufs(I{}, brigand::list<VarInfosBuffers...>{}, buffers, pkts...);
            }
        );
    }

    template<class Buffer, class Var, class Continue>
    static void write_dyn_type(Buffer &, Var & var, Continue f) {
        std::cout << var_type<Var>::name() << " = ";
        print(var);
        std::cout << "\n";
        f();
    }


    template<class I, class VarInfosByBuffer, class Buffers, class Sizes>
    static void write_pkt_szs(I, VarInfosByBuffer, Buffers & buffers, Sizes & sizes) {
        using new_list = brigand::find<VarInfosByBuffer, brigand::call<has_pkt_sz>>;
        using old_size = brigand::size<VarInfosByBuffer>;
        using new_size = brigand::size<new_list>;
        using new_index = brigand::size_t<I::value + (old_size::value - new_size::value)>;

        write_pkt_sz(new_index{}, new_list{}, buffers, sizes);
    }

    template<class I, class Buffers, class Sizes>
    constexpr static void write_pkt_sz(
        I, brigand::list<>, Buffers &, Sizes &
    ) {
    }

    template<
        class I,
        class VarInfos, class... VarInfosBuffers,
        class Buffers, class Sizes>
    static void write_pkt_sz(
        I, brigand::list<VarInfos, VarInfosBuffers...>, Buffers & buffers, Sizes & sizes
    ) {
        write_pkt_sz_from_buffer(I{}, VarInfos{}, sizes);
        auto n = write_pkt_szs(I{}, brigand::list<VarInfosBuffers...>{}, buffers, sizes);
    }

    template<class I, class VarInfo, class Buffers, class Sizes>
    static void write_pkt_sz_from_buffer(I, VarInfo, Buffers & buffers, Sizes & sizes) {
        std::cout << "x";
    }


    template<
        std::size_t ibuf, std::size_t end,
        class var_info_list_by_buffer, class accu_sizeof_by_packet,
        class... Packets>
    static void impl(
        i_<ibuf>, i_<end>,
        var_info_list_by_buffer, accu_sizeof_by_packet t1,
        Packets & ... packets
    ) {
        using var_info_list = brigand::front<var_info_list_by_buffer>;

        std::cout << '[';
        brigand::for_each<var_info_list>([&packets...](auto v) {
            using info_type = t_<decltype(v)>;
            auto && value = larg<info_type::ivar::value>(arg<info_type::ipacket::value>(packets...));
            std::cout << var_type<std::remove_reference_t<decltype(value)>>::name() << " = ";
            print(value);
            print_pkt_sz(info_type{}, accu_sizeof_by_packet{});
            std::cout << ", ";
        });
        std::cout << "]\n";

        if (is_buffer<desc_type<brigand::front<var_info_list>>>{}) {
            //TODO buffers_size += this->current_buffer_size();
        }

        impl(
            i_<ibuf+1>{}, i_<end>{},
            brigand::pop_front<var_info_list_by_buffer>{}, t1, packets...
        );
    }

    template<
        std::size_t end,
        class var_info_list_by_buffer, class accu_sizeof_by_packet,
        class... Packets>
    static void impl(
        i_<end>, i_<end>,
        var_info_list_by_buffer, accu_sizeof_by_packet, Packets & ...
    ) {
    }

    template<class T>
    static void print(proto::val<T> const & x)
    { Printer{}.print(x.x, 1); }

    template<class T, class tag>
    static void print(proto::var<proto::types::pkt_sz<T>, tag>)
    { std::cout << "[pkt_sz]"; }

    template<class T, class tag>
    static void print(proto::var<proto::types::pkt_sz_with_self<T>, tag>)
    { std::cout << "[pkt_sz_with_self]"; }

    template<class T, class accu_sizeof_by_packet>
    static void print_pkt_sz(T, accu_sizeof_by_packet)
    {}

    template<class ipkt, class ivar, class T, class accu_sizeof_by_packet>
    static void print_pkt_sz(var_info<ipkt, ivar, proto::types::pkt_sz<T>>, accu_sizeof_by_packet)
    {
        // TODO maybe an error
        using i = brigand::apply<
            std::conditional_t<
                (ipkt::value+1 < brigand::size<accu_sizeof_by_packet>::value),
                brigand::call<brigand::at>,
                brigand::protect<static_size<0>>
            >,
            accu_sizeof_by_packet,
            i_<ipkt::value+1>
        >;
        print_size(i{});
    }

    template<class ipkt, class ivar, class T, class accu_sizeof_by_packet>
    static void print_pkt_sz(var_info<ipkt, ivar, proto::types::pkt_sz_with_self<T>>, accu_sizeof_by_packet)
    {
        print_size(brigand::at<accu_sizeof_by_packet, ipkt>{});
    }

    template<std::size_t n>
    static void print_size(dynamic_size<n>)
    { std::cout << "dyn("<<n<<")"; }

    template<std::size_t n>
    static void print_size(static_size<n>)
    { std::cout << "fixed("<<n<<")"; }
};

int main() {

    struct {
        uint8_t a = 1;
        uint8_t b = 2;
        uint16_t d = 3;
        char const c[3] = "ab";
    } pkt;

    auto packet = XXX::desc(
        XXX::a = pkt.a,
        XXX::b = pkt.b,
        XXX::c = pkt.c,
        XXX::d = pkt.d,
        XXX::sz,
        XXX::sz2
    );

packet.apply_for_each(Printer{});
std::cout << "\n";
packet.apply(Buferring{});
std::cout << "\n";
XXX::desc(packet, packet).apply(Buferring2{});

/*
 * type, type_adapter, type_list.
 * value, integral_constant, value_list?
 * visiteur par type, par groupe
 *
 * pkt1: [ 1.u8 2.u8 3.bytes 4.u16 ]
 * buffers1: [ [u8 u8] [bytes] [u16] ] (split on buffer_view)
 *
 * pkt2: [ 1.pkt_sz 2.u32 3.bytes ]
 * buffers2: [ [pkt_sz u32] [bytes] ] (split on buffer_view)
 *
 * buffers: [ [1.1.u8 1.2.u8] [1.3.bytes] [1.3.u16 2.1.pkt_sz 2.2.u32] [2.3.bytes] ]
 *
 * static pkt_sz resolved (auto_val)
 * write to buffers
 * dynamic pkt_sz resolved
 */



}
