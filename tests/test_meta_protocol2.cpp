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
#include <brigand/brigand.hpp>

#include "utils/sugar/array_view.hpp"

namespace proto
{
    namespace ext
    {
        template<class Desc, class T>
        typename Desc::type
        adapt(Desc, T x)
        { return x; }
    }

    template<std::size_t N>
    using size_ = std::integral_constant<std::size_t, N>;

    template<class T>
    struct internal_type
    { using type = typename T::type; };

    template<class T>
    using internal_type_t = typename internal_type<T>::type;

    namespace types {
        class le_tag {};
        class be_tag {};

        /**
        * fixed width integer types
        * @{
        */
        template<class T, class Endianess> struct integer { using type = T; };

        using s8 = integer<int8_t, void>;
        using u8 = integer<uint8_t, void>;

        using s16_be = integer<int16_t, be_tag>;
        using s16_le = integer<int16_t, le_tag>;
        using u16_be = integer<uint16_t, be_tag>;
        using u16_le = integer<uint16_t, le_tag>;

        using s32_be = integer<int32_t, be_tag>;
        using s32_le = integer<int32_t, le_tag>;
        using u32_be = integer<uint32_t, be_tag>;
        using u32_le = integer<uint32_t, le_tag>;

        using s64_be = integer<int64_t, be_tag>;
        using s64_le = integer<int64_t, le_tag>;
        using u64_be = integer<uint64_t, be_tag>;
        using u64_le = integer<uint64_t, le_tag>;
        /** @} */

        struct u8_or_u16_le { using type = uint16_t; };
        struct bytes { using type = array_view_const_u8; };
        struct str8_to_str16 { using type = char const *; };
        template<class T> struct pkt_sz { using type = T; };
        template<class T> struct pkt_sz_with_self { using type = T; };
    }

    namespace ext
    {
        inline
        array_view_const_u8
        adapt(types::bytes, array_view_const_char av) noexcept
        { return {reinterpret_cast<uint8_t const *>(av.data()), av.size()}; }
    }


    // TODO type_traits { sizeof_, buffer+tag }

    struct dyn_size {};
    template<std::size_t i> struct limited_size {};
    namespace detail {
        template<class T> struct sizeof_ : sizeof_<typename T::desc_type> {};
        template<class T, class Endianess> struct sizeof_<types::integer<T, Endianess>> : size_<sizeof(T)> {};
        template<> struct sizeof_<types::u8_or_u16_le> { using type = /*dyn_size*/limited_size<sizeof(uint16_t)>; };
        template<> struct sizeof_<types::bytes> { using type = dyn_size; };
        template<> struct sizeof_<types::str8_to_str16> { using type = dyn_size; };
        template<class T> struct sizeof_<types::pkt_sz<T>> : sizeof_<T> {};
        template<class T> struct sizeof_<types::pkt_sz_with_self<T>> : sizeof_<T> {};
    }
    template<class T> using sizeof_ = typename detail::sizeof_<T>::type;

    namespace tags {
        class static_buffer {};
        class dynamic_buffer {};
        class view_buffer {};
        class limited_buffer {};
    }

    template<class T> struct buffer_category : buffer_category<typename T::desc_type> {};
    template<> struct buffer_category<types::u8> { using type = tags::static_buffer; };
    template<> struct buffer_category<types::u16_le> { using type = tags::static_buffer; };
    template<> struct buffer_category<types::u8_or_u16_le> { using type = tags::limited_buffer; };
    template<> struct buffer_category<types::bytes> { using type = tags::view_buffer; };
    template<> struct buffer_category<types::str8_to_str16> { using type = tags::dynamic_buffer; };
    template<class T> struct buffer_category<types::pkt_sz<T>> : buffer_category<T> {};
    template<class T> struct buffer_category<types::pkt_sz_with_self<T>> : buffer_category<T> {};
    template<class T> using buffer_category_t = typename buffer_category<T>::type;

    template<std::size_t n> using mk_iseq = std::make_integer_sequence<std::size_t, n>;


    namespace detail {
        template<class T>
        struct is_pkt_sz : std::false_type
        {};

        template<class T>
        struct is_pkt_sz<proto::types::pkt_sz<T>> : std::true_type
        {};

        template<class T>
        struct is_pkt_sz_with_self : std::false_type
        {};

        template<class T>
        struct is_pkt_sz_with_self<proto::types::pkt_sz_with_self<T>> : std::true_type
        {};
    }
    template<class T> using is_pkt_sz = typename detail::is_pkt_sz<T>::type;
    template<class T> using is_pkt_sz_with_self = typename detail::is_pkt_sz_with_self<T>::type;
    template<class T> using is_pkt_sz_cat = brigand::bool_<is_pkt_sz<T>{} or is_pkt_sz_with_self<T>{}>;


    template<class Var, class T>
    struct val
    {
        using var_type = Var;
        T x;
    };

    template<class Desc, class Derived>
    struct var
    {
        static_assert(std::is_empty<Desc>{}, "");

        using desc_type = Desc;
        using var_type = var;

        template<class U>
        auto operator = (U y) const
        //-> val<Derived, decltype(ext::adapt(desc_type{}, y))>
        -> val<Derived, decltype(ext::adapt(Desc{}, y))>
        { return {ext::adapt(Desc{}, y)}; }
    };

    template<class T>
    using desc_type = typename T::desc_type;

    template<class...> using void_t = void;

    template<class T, class = void> struct check;
    template<class T> struct check<T, std::enable_if_t<T::value>> { constexpr operator bool () { return 0; } };

#define PROTO_CHECKS(Tpl, Ts)            \
    (void)::std::initializer_list<bool>{ \
      ::proto::check<Tpl<Ts>>{}...       \
    }

    namespace detail {
        template<template<class> class Tpl, class T, class = std::true_type>
        struct check_and_return_impl {};

        template<template<class> class Tpl, class T>
        struct check_and_return_impl<Tpl, T, typename Tpl<T>::type>
        { using type = T; };
    }

    template<template<class> class Tpl, class T>
    struct check_and_return : detail::check_and_return_impl<Tpl, T> {};

    template<template<class> class Tpl, class T>
    using check_and_return_t = typename check_and_return<Tpl, T>::type;


    namespace detail {
        template<class T, class = void> struct is_proto_variable_impl : std::false_type {};
        template<class T> struct is_proto_variable_impl<T, void_t<typename T::var_type>> : std::true_type {};
    }
    template<class T> struct is_proto_variable : detail::is_proto_variable_impl<T> {};

    template<class T> struct is_proto_value : std::false_type {};
    template<class Var, class T> struct is_proto_value<val<Var, T>> : std::true_type {};

    template<class T> using var_type_t = typename T::var_type;

    template<class Ints, class... Ts>
    struct packet;

    template<class Var, class Refs>
    std::enable_if_t<is_pkt_sz_cat<desc_type<Var>>{}, Var>
    val_or_pkt_size(Refs)
    { return {}; }

    template<class Var, class T>
    std::enable_if_t<!is_pkt_sz_cat<desc_type<Var>>{}, val<Var, T>&>
    val_or_pkt_size(std::reference_wrapper<val<Var, T>> ref)
    { return ref; }

    template<class T, class = void>
    using enable_type = T;

    template<class... Ts>
    struct packet_description
    {
        using type_list = brigand::list<Ts...>;

        using strong_type_list = brigand::remove<type_list, brigand::bind<brigand::not_<brigand::call<is_pkt_sz_cat>>>>;

        template<class T> using contains = brigand::contains<brigand::wrap<strong_type_list, brigand::set>, T>;

        template<class Val>
        using check_param = enable_type<Val, void_t<check_and_return_t<contains, var_type_t<check_and_return_t<is_proto_value, Val>>>>>;

        template<class... Val>
        constexpr auto
        operator()(Val... values) const
        {
            return ordering_parameter<check_param<Val>...>(values...);
        }

        template<class... Val>
        constexpr auto
        ordering_parameter(Val... values) const
        {
            struct ValRefs : std::reference_wrapper<Val>... {
                ValRefs(Val & ... val) : std::reference_wrapper<Val>{val}... {}
            } refs {values...};
            return packet<
                mk_iseq<sizeof...(Ts)>,
                std::remove_reference_t<decltype(val_or_pkt_size<Ts>(refs))>...
            >{val_or_pkt_size<Ts>(refs)...};
        }
    };

    template<class... Desc>
    constexpr auto
    desc(Desc...)
    {
        return packet_description<
            check_and_return_t<std::is_empty, check_and_return_t<is_proto_variable, Desc>>...
        >{};
    }

    template<std::size_t i, class Var>
    struct value_desc {
        Var x;
    };

    template<std::size_t... Ints, class... Ts>
    struct packet<std::integer_sequence<std::size_t, Ints...>, Ts...>
    : value_desc<Ints, Ts>...
    {
        using type_list = brigand::list<Ts...>;

        constexpr packet(Ts... args)
        : value_desc<Ints, Ts>{args}...
        {}

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

    template<class T> struct is_proto_packet : std::false_type {};
    template<class Ints, class... Ts>
    struct is_proto_packet<packet<Ints, Ts...>> : std::true_type {};

    template<class F, class... Pkts>
    void apply(F f, Pkts ... pkts)
    {
        PROTO_CHECKS(is_proto_packet, Pkts);
        f(pkts...);
    }
}

namespace XXX {
    PROTO_VAR(proto::types::u8, a);
    PROTO_VAR(proto::types::u8, b);
    PROTO_VAR(proto::types::bytes, c);
    PROTO_VAR(proto::types::u16_le, d);
    PROTO_VAR(proto::types::str8_to_str16, e);
    PROTO_VAR(proto::types::u8_or_u16_le, f);
    PROTO_VAR(proto::types::pkt_sz<proto::types::u8>, sz);
    PROTO_VAR(proto::types::pkt_sz_with_self<proto::types::u8>, sz2);

    constexpr auto desc = proto::desc(a, b, c, d, e, f, sz, sz2);
}



namespace detail {
    template<class T> using is_static_buffer = std::is_same<T, proto::tags::static_buffer>;
    template<class T> using is_dynamic_buffer = std::is_same<T, proto::tags::dynamic_buffer>;
    template<class T> using is_view_buffer = std::is_same<T, proto::tags::view_buffer>;
    template<class T> using is_limited_buffer = std::is_same<T, proto::tags::limited_buffer>;
}
template<class T> using is_static_buffer = typename detail::is_static_buffer<proto::buffer_category_t<T>>::type;
template<class T> using is_dynamic_buffer = typename detail::is_dynamic_buffer<proto::buffer_category_t<T>>::type;
template<class T> using is_view_buffer = typename detail::is_view_buffer<proto::buffer_category_t<T>>::type;
template<class T> using is_limited_buffer = typename detail::is_limited_buffer<proto::buffer_category_t<T>>::type;


struct Printer
{
    template<class var, class T>
    void operator()(proto::val<var, T> x) const {
        std::cout << var::name() << " = "; print(x.x, 1);
        std::cout
            << "  static: " << is_static_buffer<typename var::desc_type>{}
            << "  dyn: " << is_dynamic_buffer<typename var::desc_type>{}
            << "  view: " << is_view_buffer<typename var::desc_type>{}
            << "  limited: " << is_limited_buffer<typename var::desc_type>{}
            << "\n";
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
    print(array_view<T> av, char) const
    { std::cout << av.data(); }

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


template<class T>
using is_buffer_delimiter = brigand::bool_<
    is_dynamic_buffer<T>::value or
    is_view_buffer<T>::value
>;

template<class idx_var>
using pair_is_buffer_delimiter = is_buffer_delimiter<typename idx_var::second_type::desc_type>;

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

    template<class Var, class T>
    struct var_type_impl<proto::val<Var, T>>
    { using type = Var; };
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
        using list_by_buffer = brigand::split_if<list_pair, brigand::call<pair_is_buffer_delimiter>>;

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

    template<class Var, class T>
    static void print(proto::val<Var, T> const & x)
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
using var_info_is_buffer_delimiter = is_buffer_delimiter<typename var_info::desc_type>;

template<std::size_t n> struct static_size : brigand::size_t<n> {};
template<std::size_t n> struct dynamic_size : brigand::size_t<n> {};
template<std::size_t n> struct limited_size : brigand::size_t<n> {};

namespace lazy {
    template<class p, class i>
    struct add_size;

    template<template<std::size_t> class Size, std::size_t n, class add>
    struct add_size<Size<n>, add>
    { using type = Size<(n+add::value)>; };

    template<template<std::size_t> class Size, std::size_t n>
    struct add_size<Size<n>, proto::dyn_size>
    { using type = dynamic_size<n>; };

    template<template<std::size_t> class Size, std::size_t n1, std::size_t n2>
    struct add_size<Size<n1>, proto::limited_size<n2>>
    { using type = Size<n1+n2>; };

    template<std::size_t n1, std::size_t n2>
    struct add_size<brigand::size_t<n1>, proto::limited_size<n2>>
    { using type = limited_size<n1+n2>; };

    template<std::size_t n1, std::size_t n2>
    struct add_size<static_size<n1>, proto::limited_size<n2>>
    { using type = limited_size<n1+n2>; };

    template<std::size_t n1, std::size_t n2>
    struct add_size<brigand::size_t<n1>, brigand::size_t<n2>>
    { using type = brigand::size_t<n1+n2>; };
}
template<class i1, class i2>
using add_size = typename lazy::add_size<i1, i2>::type;

template<class L>
using sizeof_packet = brigand::fold<
    brigand::transform<L, brigand::call<proto::sizeof_>>,
    static_size<0>,
    brigand::call<add_size>
>;

namespace detail {
    template<class T>
    struct limited_size_like_dyn_size
    { using type = T; };

    template<std::size_t n>
    struct limited_size_like_dyn_size<proto::limited_size<n>>
    { using type = proto::dyn_size; };
}
template<class T>
using limited_size_like_dyn_size = typename detail::limited_size_like_dyn_size<T>::type;

template<class L>
using sizeof_packet_with_limited_size_like_dyn_size = brigand::fold<
    brigand::transform<
        brigand::transform<L, brigand::call<proto::sizeof_>>,
        brigand::call<limited_size_like_dyn_size>
    >,
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


namespace detail {
    template<class T, std::size_t n> struct pkt_sz_with_size { using desc_type = T; };

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


using proto::desc_type;

template<class T>
using var_to_desc_type = desc_type<var_type<T>>;

template<std::size_t i>
using i_ = std::integral_constant<std::size_t, i>;

template<class L>
using mk_sizeof_var_info_list = brigand::transform<
    brigand::transform<L, brigand::call<desc_type>>,
    brigand::call<proto::sizeof_>
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

    template<std::size_t n>
    struct sizeof_to_buffer<limited_size<n>>
    { using type = uint8_t [n]; };
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
using var_infos_is_not_dynamic = is_dynamic_buffer<brigand::front<VarInfos>>;

template<class T>
using var_info_is_pkt_sz = proto::is_pkt_sz_cat<var_to_desc_type<T>>;

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

namespace detail
{
    template<std::size_t n>
    struct Buffers
    {
        struct iovec {
            void  *iov_base;
            size_t iov_len;
        };

        iovec data[n]{};

        template<class Tuple>
        Buffers(Tuple & t)
        {
            // TODO slow
            brigand::for_each<mk_seq<n>>([this, &t](auto I) {
                using i = t_<decltype(I)>;
                this->init_buf(i{}, std::get<i::value>(t));
            });
        }

    private:
        template<class I, std::size_t arr_len>
        void init_buf(I i, uint8_t(&a)[arr_len]) {
            this->data[i].iov_base = a;
            this->data[i].iov_len = arr_len;
        }

        template<class I>
        void init_buf(I, proto::dyn_size) {
        }
    };

    template<class>
    struct Sizes;

    template<class PktSz>
    struct Sizes<brigand::list<PktSz>>
    {
        // TODO sizeof -1
        std::size_t data[2] { PktSz::value };

        void propagate_size()
        {}
    };

    template<class... PktSz>
    struct Sizes<brigand::list<PktSz...>>
    {
        // TODO sizeof -1
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

template<class Policy>
struct Buferring2
{
    template<class... Pkts>
    struct Impl
    {
        using packet_list_ = brigand::list<brigand::transform<typename Pkts::type_list, brigand::call<var_to_desc_type>>...>;
        using sizeof_by_packet = brigand::transform<packet_list_, brigand::call<sizeof_packet>>;
        using accu_sizeof_by_packet = make_accumulate_sizeof_list<sizeof_by_packet>;
        using packet_list = brigand::transform<
            packet_list_,
            accu_sizeof_by_packet,
            brigand::push_back<brigand::pop_front<accu_sizeof_by_packet>, proto::size_<0>>,
            brigand::call<convert_pkt_sz>
        >;
        using packet_count_list = brigand::transform<packet_list, brigand::call<brigand::size>>;
        using ipacket_list_by_var = brigand::transform<mk_seq<sizeof...(Pkts)>, packet_count_list, brigand::call<mk_filled_list>>;
        using ipacket_list = brigand::wrap<ipacket_list_by_var, brigand::append>;
        using var_list = brigand::wrap<packet_list, brigand::append>;
        using ivar_list = brigand::wrap<brigand::transform<packet_count_list, brigand::call<mk_seq2>>, brigand::append>;
        using var_info_list = brigand::transform<ipacket_list, ivar_list, var_list, brigand::call<var_info>>;
        using var_info_list_by_buffer = brigand::split_if<var_info_list, brigand::call<var_info_is_buffer_delimiter>>;

        using buffer_list = brigand::transform<var_info_list_by_buffer, brigand::call<buffer_from_var_infos>>;
        using default_buffer_size = brigand::transform<packet_list_, brigand::call<sizeof_packet_with_limited_size_like_dyn_size>>;

        // TODO rapidtuple
        brigand::wrap<buffer_list, std::tuple> buffer_tuple;
        detail::Buffers<brigand::size<buffer_list>::value> buffers{buffer_tuple};
        uint8_t * pktptrs[brigand::size<packet_list>::value];
        detail::Sizes<default_buffer_size> sizes;
        // TODO reference
        Policy policy;

        void impl(Pkts & ... packets)
        {
            // TODO check if pkt_sz is a static_buffer

            std::cout << "--- write_not_dynamic_bufs ---\n";
            this->write_not_dynamic_bufs(
                var_info_list_by_buffer{},
                mk_seq2<brigand::size<var_info_list_by_buffer>>{},
                packets...
            );
            std::cout << "------------------------------\n\n";

            std::cout << "--- write_dynamic_bufs ---\n";
            this->write_dynamic_bufs(
                i_<0>{},
                var_info_list_by_buffer{},
                packets...
            );
        }

        template<class... VarInfos, class... Ints>
        void write_not_dynamic_bufs(brigand::list<VarInfos...>, brigand::list<Ints...>, Pkts & ... pkts) {
            (void)std::initializer_list<int>{(void(
                write_not_dynamic_buf(
                    var_infos_is_not_dynamic<VarInfos>{},
                    VarInfos{},
                    this->buffers.data[Ints::value],
                    pkts...
                )
            ), 1)...};
        }

        template<class... Ts>
        static void write_not_dynamic_buf(std::true_type, Ts && ...)
        { std::cout << "-------\n(dyn)\n"; }

        template<class... VarInfos, class Buffer>
        void write_not_dynamic_buf(std::false_type, brigand::list<VarInfos...>, Buffer & buffer, Pkts & ... pkts) {
            std::cout << "-------\n";
            (void)std::initializer_list<int>{(void(
                this->write_type(
                    proto::buffer_category_t<VarInfos>{}, VarInfos{}, buffer,
                    larg<VarInfos::ivar::value>(arg<VarInfos::ipacket::value>(pkts...)))
            ), 1)...};
        }

        template<class VarInfo, class Buffer, class Var>
        void write_type(proto::tags::static_buffer, VarInfo, Buffer & buffer, Var & var) {
            std::cout << var_type<Var>::name() << " = ";
            this->print(var);
            this->write_pkt_sz_with_size_or_var(desc_type<VarInfo>{}, buffer, var);
            std::cout << "\n";
            if (var_info_is_pkt_sz<VarInfo>{}) {
                this->pktptrs[VarInfo::ipacket::value] = static_cast<uint8_t*>(buffer.iov_base);
            }
        }

        template<class T, class Buffer, class Var>
        void write_pkt_sz_with_size_or_var(proto::types::pkt_sz<T>, Buffer &, Var &)
        {}

        template<class T, class Buffer, class Var>
        void write_pkt_sz_with_size_or_var(proto::types::pkt_sz_with_self<T>, Buffer &, Var &)
        {}

        template<class T, std::size_t n, class Buffer, class Var>
        void write_pkt_sz_with_size_or_var(detail::pkt_sz_with_size<T, n>, Buffer & buffer, Var &)
        {
            using proto_integer = typename T::type;
            policy.write_static_buffer(
                static_cast<uint8_t*>(buffer.iov_base),
                typename proto_integer::type{n}, proto_integer{}
            );
            buffer.iov_base = static_cast<uint8_t*>(buffer.iov_base) + proto::sizeof_<T>{};
            std::cout << " = " << n;
        }

        template<class T, class Buffer, class Var>
        void write_pkt_sz_with_size_or_var(T, Buffer & buffer, Var const & var)
        {
            policy.write_static_buffer(static_cast<uint8_t*>(buffer.iov_base), var.x, T{});
            buffer.iov_base = static_cast<uint8_t*>(buffer.iov_base) + proto::sizeof_<T>{};
        }

        template<class VarInfo, class Buffer, class Var>
        void write_type(proto::tags::limited_buffer, VarInfo, Buffer & buffer, Var & var) {
            std::cout << var_type<Var>::name() << " = ";
            this->print(var);
            std::size_t len = policy.write_limited_buffer(
                static_cast<uint8_t*>(buffer.iov_base),
                var.x,
                desc_type<VarInfo>{}
            );
            buffer.iov_base = static_cast<uint8_t*>(buffer.iov_base) + len;

            // TODO
            len = 3;
            this->sizes.data[VarInfo::ipacket::value] += len;
            std::cout << "  [size: " << len << "]\n";
            static_assert(!var_info_is_pkt_sz<VarInfo>{}, "");
        }

        template<class VarInfo, class Buffer, class Var>
        void write_type(proto::tags::view_buffer, VarInfo, Buffer & buffer, Var & var) {
            std::cout << var_type<Var>::name() << " = ";
            this->print(var);
            auto av = policy.get_view_buffer(var.x, desc_type<VarInfo>{});
            buffer.iov_base = const_cast<uint8_t *>(av.data());
            buffer.iov_len = av.size();
            this->sizes.data[VarInfo::ipacket::value] += av.size();
            static_assert(!var_info_is_pkt_sz<VarInfo>{}, "");
        }

        template<class I, class VarInfosByBuffer>
        void write_dynamic_bufs(I, VarInfosByBuffer, Pkts & ... pkts) {
            using new_list = brigand::find<VarInfosByBuffer, brigand::call<var_infos_is_not_dynamic>>;
            using old_size = brigand::size<VarInfosByBuffer>;
            using new_size = brigand::size<new_list>;
            using new_index = brigand::size_t<I::value + (old_size::value - new_size::value)>;

            write_dynamic_buf(new_index{}, new_list{}, pkts...);
        }

        template<class I>
        void write_dynamic_buf(I, brigand::list<>, Pkts & ...) {
            std::cout << "--------------------------\n\n";

            // TODO reset limited_buffer ptr

            this->sizes.propagate_size();

            std::cout << "sizes: ";
            for (auto i : this->sizes.data) {
                std::cout << i << " ";
            }
            std::cout << "\n\n";

            std::cout << "--- write_pkt_szs ---\n";
            this->write_pkt_szs(
                i_<0>{},
                var_info_list_by_buffer{}
            );
            std::cout << "---------------------\n\n";
        }

        template<class I, class VarInfos, class... VarInfosBuffers>
        void write_dynamic_buf(I, brigand::list<VarInfos, VarInfosBuffers...>, Pkts & ... pkts) {
            using var_info = brigand::front<VarInfos>;
            write_dyn_type(
                larg<var_info::ivar::value>(arg<var_info::ipacket::value>(pkts...)),
                [this, &pkts...](array_view_const_u8 av) {
                    auto & buffer = this->buffers.data[I::value];
                    buffer.iov_base = const_cast<uint8_t *>(av.data());
                    buffer.iov_len = av.size();
                    this->sizes.data[var_info::ipacket::value] += av.size();
                    // TODO
                    this->sizes.data[var_info::ipacket::value] += 3;
                    std::cout << "  [size: " << av.size() << "]";
                    // TODO assert called
                    std::cout << "\n";
                    this->write_dynamic_bufs(
                        brigand::size_t<I::value + 1>{},
                        brigand::list<VarInfosBuffers...>{},
                        pkts...
                    );
                }
            );
        }

        template<class Var, class Continue>
        void write_dyn_type(Var & var, Continue f) {
            std::cout << var_type<Var>::name() << " = ";
            print(var);
            policy.context_dynamic_buffer(f, var.x, var_to_desc_type<Var>{});
        }


        template<class I, class VarInfosByBuffer>
        void write_pkt_szs(I, VarInfosByBuffer) {
            using new_list = brigand::find<VarInfosByBuffer, brigand::call<var_infos_has_pkt_sz>>;
            using old_size = brigand::size<VarInfosByBuffer>;
            using new_size = brigand::size<new_list>;
            using new_index = brigand::size_t<I::value + (old_size::value - new_size::value)>;

            this->write_pkt_sz(new_index{}, new_list{});
        }

        template<class I>
        static void write_pkt_sz(I, brigand::list<>) {
        }

        template<
            class I,
            class VarInfos, class... VarInfosBuffers>
        void write_pkt_sz(I, brigand::list<VarInfos, VarInfosBuffers...>) {
            std::cout << "-------\n";
            write_pkt_sz_from_buffer(VarInfos{});
            write_pkt_szs(brigand::size_t<I::value + 1>{}, brigand::list<VarInfosBuffers...>{});
        }

        template<class... VarInfo>
        void write_pkt_sz_from_buffer(brigand::list<VarInfo...>) {
            (void)std::initializer_list<int>{(void(
                write_pkt_sz_var(VarInfo{})
            ), 1)...};
        }

        template<class VarInfo>
        static void write_pkt_sz_var(VarInfo) {
        }

        template<class IPacket, class IVar, class T>
        void write_pkt_sz_var(var_info<IPacket, IVar, proto::types::pkt_sz<T>>) {
            using is_proto_integer = std::is_same<T, brigand::wrap<T, proto::types::integer>>;
            static_assert(is_proto_integer{}, "only proto::types::integer is supported with pkt_sz");
            // TODO
            std::cout << "pktptrs[" << IPacket::value << "] = " << this->sizes.data[IPacket::value+1] << "\n";
        }

        template<class IPacket, class IVar, class T>
        void write_pkt_sz_var(var_info<IPacket, IVar, proto::types::pkt_sz_with_self<T>>) {
            using is_proto_integer = std::is_same<T, brigand::wrap<T, proto::types::integer>>;
            static_assert(is_proto_integer{}, "only proto::types::integer is supported with pkt_sz_with_self");
            // TODO
            std::cout << "pktptrs[" << IPacket::value << "] = " << this->sizes.data[IPacket::value] << "\n";
        }

        template<class Var, class T>
        static void print(proto::val<Var, T> const & x)
        { Printer{}.print(x.x, 1); }

        template<class T, class tag>
        static void print(proto::var<proto::types::pkt_sz<T>, tag>)
        { std::cout << "[pkt_sz]"; }

        template<class T, class tag>
        static void print(proto::var<proto::types::pkt_sz_with_self<T>, tag>)
        { std::cout << "[pkt_sz_with_self]"; }

        static void print_buffer_type(proto::tags::static_buffer) { std::cout << "[static_buffer]"; }
        static void print_buffer_type(proto::tags::dynamic_buffer) { std::cout << "[dyn_buffer]"; }
        static void print_buffer_type(proto::tags::view_buffer) { std::cout << "[view_buffer]"; }
        static void print_buffer_type(proto::tags::limited_buffer) { std::cout << "[limited_buffer]"; }
    };


    template<class... Packets>
    void operator()(Packets ... packets) const {
        Impl<Packets...> impl;
        impl.impl(packets...);
    }

    template<class... Packets>
    void impl(std::false_type, Packets ... packets) const = delete;
};


struct stream_protocol_policy
{
    template<class T, class Endianess>
    void write_static_buffer(uint8_t * p, T val, proto::types::integer<T, Endianess>)
    {
        std::cout << " [static_buffer] [sizeof_: " << sizeof(T) << "]";
        using rng = std::conditional_t<
            std::is_same<Endianess, proto::types::le_tag>{},
            brigand::range<std::size_t, 0, sizeof(T)>,
            brigand::reverse_range<std::size_t, sizeof(T), 0>
        >;
        brigand::for_each<rng>([&p, val](auto I) {
            constexpr const std::size_t i = t_<decltype(I)>::value;
            *p++ = (val & (0xff << i)) > i;
        });
    }

    array_view_const_u8 get_view_buffer(array_view_const_u8 av, proto::types::bytes)
    {
        std::cout << " [view_buffer] [size: " << av.size() << "]";
        return av;
    }

    template<class T, class Desc>
    std::size_t write_limited_buffer(uint8_t *, T, Desc)
    {
        std::cout << " [limited_buffer]";
        return {};
    }

    template<class F, class T, class Desc>
    void context_dynamic_buffer(F f, T, Desc)
    {
        std::cout << " [dynamic_buffer]";
        f(array_view_const_u8{});
    }
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
        XXX::c = /*cstr_*/make_array_view(pkt.c),
        XXX::d = pkt.d,
        XXX::e = pkt.c,
        XXX::f = pkt.d/*,
//         XXX::sz,
        XXX::sz2
      , 1*/
    );

packet.apply_for_each(Printer{});
std::cout << "\n";
packet.apply(Buferring{});
std::cout << "\n";
proto::apply(Buferring2<stream_protocol_policy>{}, packet, packet);

}
