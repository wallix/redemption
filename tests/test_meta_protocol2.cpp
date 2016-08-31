#include <limits>
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


// (standalone version): https://github.com/edouarda/brigand
#define BRIGAND_NO_BOOST_SUPPORT
#include <brigand/brigand.hpp>

// https://github.com/jonathanpoelen/brigand/blob/ext_call/brigand/functions/lambda/call.hpp
namespace brigand
{
namespace detail
{
    template<template<class...> class F>
    struct call_
    {};

    template <template<class...> class F, class... Args>
    struct apply<call_<F>, Args...>
    {
        using type = F<Args...>;
    };

    template<template<class> class F>
    bind<F, _1> get_call_impl(call_<F>, int);

    template<template<class, class> class F>
    bind<F, _1, _2> get_call_impl(call_<F>, int);

    template<template<class, class, class> class F>
    bind<F, _1, _2, _3> get_call_impl(call_<F>, int);

    template<class F>
    F get_call_impl(F, char);
}

    template<template<class...> class F>
    using call = decltype(detail::get_call_impl(detail::call_<F>{}, 1));

    template<class L, class P>
    using copy_if = remove_if<L, bind<not_, P>>;
}


#include "utils/sugar/array_view.hpp"

namespace proto
{
    namespace detail
    {
        template<class T, class DescType, bool = (std::is_integral<T>::value && std::is_integral<DescType>::value)>
        struct adapt_default_type
        {
            static_assert(std::numeric_limits<T>::min() >= std::numeric_limits<DescType>::min(), "narrowing conversion");
            static_assert(std::numeric_limits<T>::max() <= std::numeric_limits<DescType>::max(), "narrowing conversion");
            using type = DescType;
        };

        template<class T, class DescType>
        struct adapt_default_type<T, DescType, false>
        {
            using type = DescType;
        };

        template<class T, T value, class DescType>
        struct adapt_default_type<std::integral_constant<T, value>, DescType, false>
        {
            static_assert(DescType{value} == value, "narrowing conversion");
            using type = DescType;
        };
    }

    namespace ext
    {
        template<class Desc, class T>
        constexpr typename ::proto::detail::adapt_default_type<T, typename Desc::type>::type
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


    struct dyn_size {};
    template<std::size_t i> struct limited_size {};

    namespace tags {
        class static_buffer {};
        class dynamic_buffer {};
        class view_buffer {};
        class limited_buffer {};
    }

    template<class T> struct sizeof_impl { using type = typename T::sizeof_; };

    template<class T> using sizeof_ = typename sizeof_impl<T>::type;

    template<class...> using void_t = void;

    namespace detail {
        template<class T> struct sizeof_to_buffer_cat { using type = tags::dynamic_buffer; };
        template<std::size_t n> struct sizeof_to_buffer_cat<size_<n>> { using type = tags::static_buffer; };
        template<std::size_t n> struct sizeof_to_buffer_cat<limited_size<n>> { using type = tags::limited_buffer; };

        template<class T, class = void>
        struct buffer_category_impl : sizeof_to_buffer_cat<sizeof_<T>> {};

        template<class T>
        struct buffer_category_impl<T, void_t<typename T::buffer_category>>
        { using type = typename T::buffer_category; };
    }

    template<class T> struct buffer_category_impl : detail::buffer_category_impl<T> {};
    template<class T> using buffer_category = typename buffer_category_impl<T>::type;

    template<class T> using is_static_buffer
      = typename std::is_same<tags::static_buffer, buffer_category<T>>::type;
    template<class T> using is_limited_buffer
      = typename std::is_same<tags::limited_buffer, buffer_category<T>>::type;
    template<class T> using is_view_buffer
      = typename std::is_same<tags::view_buffer, buffer_category<T>>::type;
    template<class T> using is_dynamic_buffer
      = typename std::is_same<tags::dynamic_buffer, buffer_category<T>>::type;

    namespace detail
    {
        template<class T, class U> struct common_size;
        template<class T, class U> struct common_buffer;
    }

    template<class T, class U> using common_size = typename detail::common_size<T, U>::type;
    template<class T, class U> using common_buffer = typename detail::common_buffer<T, U>::type;

    namespace types {
        class le_tag {};
        class be_tag {};

        /**
        * fixed width integer types
        * @{
        */
        template<class T, class Endianess>
        struct integer
        {
            using type = T;
            using sizeof_ = size_<sizeof(T)>;
        };

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

        /**
        * @{
        */
        template<class T>
        struct integer_encoding
        {
            using type = T;
            using sizeof_ = limited_size<sizeof(T)>;
        };

        using u16_encoding = integer_encoding<uint16_t>;
        using u32_encoding = integer_encoding<uint32_t>;
        /** @} */

        struct bytes
        {
            using type = array_view_const_u8;
            using sizeof_ = dyn_size;
            using buffer_category = tags::view_buffer;
        };

        struct mutable_bytes
        {
            using type = array_view_u8;
            using sizeof_ = dyn_size;
            using buffer_category = tags::view_buffer;
        };

        struct str8_to_str16
        {
            using type = array_view_const_u8;
            using sizeof_ = dyn_size;
        };

        template<class T>
        struct pkt_sz
        {
            using type = T;
            using sizeof_ = proto::sizeof_<T>;
        };

        template<class T>
        struct pkt_sz_with_self
        {
            using type = T;
            using sizeof_ = proto::sizeof_<T>;
        };

        template<class Cond, class True, class False>
        struct if_
        {
            //using type =
            using sizeof_ = common_size<proto::sizeof_<True>, proto::sizeof_<False>>;
            using buffer_category = common_buffer<proto::buffer_category<True>, proto::buffer_category<False>>;
        };

        template<class Cond, class T>
        struct enable_if
        {
            using type = typename T::type;
            using sizeof_ = common_size<proto::sizeof_<T>, size_<0>>;
            using buffer_category = std::conditional_t<
                is_static_buffer<T>::value,
                tags::limited_buffer,
                proto::buffer_category<T>
            >;
        };

        template<class T>
        struct val
        {};
    }

    template<class... Ts>
    struct inherits : Ts...
    {
        template<class... Us>
        constexpr inherits(Us && ... v)
        : Ts{std::forward<Us>(v)}...
        {}
    };

    template<class T, class... Ts>
    struct only
    {
        template<class U, class = std::enable_if_t<std::is_base_of<brigand::type_<U>, inherits<brigand::type_<T>, brigand::type_<Ts>...>>::value>>
        constexpr only(U x)
        : value(x)
        {}

        T value;
        constexpr operator T () const { return value; }
    };

    namespace ext
    {
        inline
        array_view_const_u8
        adapt(types::bytes, array_view_const_char av) noexcept
        { return {reinterpret_cast<uint8_t const *>(av.data()), av.size()}; }

        template<class T>
        inline
        array_view_const_u8
        adapt(types::bytes, T && av) noexcept
        { return array_view_const_u8{av}; }

        inline
        array_view_u8
        adapt(types::mutable_bytes, array_view_char av) noexcept
        { return {reinterpret_cast<uint8_t *>(av.data()), av.size()}; }

        template<class T>
        inline
        array_view_u8
        adapt(types::mutable_bytes, T && av) noexcept
        { return array_view_u8{av}; }

        inline
        array_view_const_u8
        adapt(types::str8_to_str16, array_view_const_char av) noexcept
        { return {reinterpret_cast<uint8_t const *>(av.data()), av.size()}; }

        constexpr inline
        uint16_t
        adapt(types::u16_encoding, only<uint16_t, uint8_t> i) noexcept
        { return i; }

        template<class Cond, class T, class U>
        constexpr inline auto
        adapt(types::enable_if<Cond, T>, U && x) noexcept
        { return adapt(T{}, std::forward<U>(x)); }

        template<class T, class U>
        constexpr inline
        T
        adapt(types::val<T>, U && x) noexcept
        { return std::forward<U>(x); }
    }


    namespace detail {
        template<class T, class U> struct common_size;

        template<std::size_t n>
        struct common_size<size_<n>, size_<n>> { using type = size_<n>; };

        template<std::size_t n1, std::size_t n2>
        struct common_size<size_<n1>, size_<n2>> { using type = limited_size<std::max(n1, n2)>; };

        template<std::size_t n1, std::size_t n2>
        struct common_size<limited_size<n1>, limited_size<n2>> { using type = limited_size<std::max(n1, n2)>; };
        template<std::size_t n1, std::size_t n2>
        struct common_size<size_<n1>, limited_size<n2>> { using type = limited_size<std::max(n1, n2)>; };
        template<std::size_t n1, std::size_t n2>
        struct common_size<limited_size<n1>, size_<n2>> { using type = limited_size<std::max(n1, n2)>; };

        template<class T> struct common_size<T, dyn_size> { using type = dyn_size; };
        template<class U> struct common_size<dyn_size, U> { using type = dyn_size; };
        template<> struct common_size<dyn_size, dyn_size> { using type = dyn_size; };


        template<class T, class U> struct common_buffer { using type = tags::dynamic_buffer; };
        template<class T> struct common_buffer<T, T> { using type = T; };
        template<> struct common_buffer<tags::static_buffer, tags::limited_buffer>
        { using type = tags::limited_buffer ; };
    }


    namespace detail {
        template<class T>
        struct is_pkt_sz : std::false_type
        {};

        template<class T>
        struct is_pkt_sz<types::pkt_sz<T>> : std::true_type
        {};

        template<class T>
        struct is_pkt_sz_with_self : std::false_type
        {};

        template<class T>
        struct is_pkt_sz_with_self<types::pkt_sz_with_self<T>> : std::true_type
        {};
    }
    template<class T> using is_pkt_sz = typename detail::is_pkt_sz<T>::type;
    template<class T> using is_pkt_sz_with_self = typename detail::is_pkt_sz_with_self<T>::type;
    template<class T> using is_pkt_sz_category = brigand::bool_<is_pkt_sz<T>{} or is_pkt_sz_with_self<T>{}>;


    template<class Var, class T>
    struct val
    {
        using var_type = Var;
        T x;
    };

    template<class Derived, class Desc, class T>
    constexpr auto trace_adapt(Desc d, T && x)
    -> decltype(ext::adapt(d, std::forward<T>(x)))
    { return ext::adapt(d, std::forward<T>(x)); }


    template<class Desc, class Derived>
    struct var
    {
        static_assert(std::is_empty<Desc>{}, "");

        using desc_type = Desc;
        using var_type = var;

        template<class U>
        constexpr auto operator = (U && x) const
//         {
//             return val<Derived, decltype(trace_adapt<Derived>(Desc{}, std::forward<U>(x)))>{
//                 trace_adapt<Derived>(Desc{}, std::forward<U>(x))
//             };
//         }
        { return impl(std::forward<U>(x)); }

        template<class U>
        constexpr auto impl(U && x) const
        -> val<Derived, decltype(trace_adapt<Derived>(Desc{}, std::forward<U>(x)))>
        { return {ext::adapt(Desc{}, std::forward<U>(x))}; }
    };


    template<class Desc, class Derived>
    constexpr sizeof_<Desc> get_size(var<Desc, Derived>)
    { return {}; }

    template<class Desc, class Derived>
    constexpr Desc get_desc(var<Desc, Derived>)
    { return {}; }


    template<class T>
    using desc_type_t = typename T::desc_type;

    template<class T, class = void> struct check;
    template<class T> struct check<T, std::enable_if_t<T::value>> { constexpr operator bool () { return 0; } };

#define PROTO_CHECKS(Tpl, Ts)            \
    (void)::std::initializer_list<bool>{ \
      ::proto::check<Tpl<Ts>>{}...       \
    }

    namespace detail {
        template<template<class...> class Tpl, class L, class = std::true_type>
        struct check_and_return_impl {};

        template<template<class...> class Tpl, class T, class... Ts>
        struct check_and_return_impl<Tpl, brigand::list<T, Ts...>, typename Tpl<T, Ts...>::type>
        { using type = T; };
    }

    template<template<class...> class Tpl, class... Ts>
    struct check_and_return : detail::check_and_return_impl<Tpl, brigand::list<Ts...>> {};

    template<template<class...> class Tpl, class T, class... Ts>
    using check_and_return_t = typename check_and_return<Tpl, T, Ts...>::type;


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

    template<class T>
    struct Ref
    {
        constexpr Ref(T & x) noexcept : x(x) {}
        constexpr operator T & () const noexcept { return x; }
        constexpr T & get() const noexcept { return x; }

    private:
        T & x;
    };

    // TODO using inherits
    template<class... Val>
    struct inherit_refs : Ref<Val>... {
        constexpr inherit_refs(Val & ... val) : Ref<Val>{val}... {}
    };


    template<class Var, class T>
    val<Var, T>
    vals_to_val(val<Var, T> const &);

    template<class Var, class T>
    val<Var, T>
    ref_to_val(Ref<val<Var, T>> x)
    { return x; }


    template<class Var, class Vals>
    auto
    get_packet_value2(Vals & vals, int)
    -> decltype(vals_to_val<Var>(vals))
    { return vals; }

    template<class Var, class Vals>
    std::enable_if_t<is_pkt_sz_category<desc_type_t<Var>>::value, Var>
    get_packet_value2(Vals &, char)
    { return {}; }


    template<class Var, class Refs, class Vals>
    auto
    get_packet_value(Refs, Vals & vals, char)
    { return get_packet_value2<Var>(vals, 1); }

    template<class Var, class Refs, class Vals>
    auto
    get_packet_value(Refs ref, Vals &, int)
    -> decltype(ref_to_val<Var>(ref))
    { return ref; }


    template<class T, class = void>
    using enable_type = T;

    template<std::size_t n> using mk_iseq = std::make_integer_sequence<std::size_t, n>;

    namespace detail
    {
        template<class T>
        struct var_or_val_to_var
        { using type = T; };

        template<class Var, class T>
        struct var_or_val_to_var<val<Var, T>>
        { using type = Var; };
    }
    template<class T>
    using var_or_val_to_var = typename detail::var_or_val_to_var<T>::type;

    template<class... Ts>
    class packet_description
    {
        using type_list = brigand::list<Ts...>;

        using type_list_only_val = brigand::filter<type_list, brigand::call<is_proto_value>>;

        using values_type = brigand::wrap<type_list_only_val, inherits>;

        values_type values;

        template<class... Us, class Refs>
        constexpr packet_description(brigand::list<Us...>, Refs refs)
        : values{static_cast<Us>(refs)...}
        {}

        template<class Refs>
        constexpr packet_description(brigand::list<>, Refs)
        : values{}
        {}

        using var_list = brigand::wrap<brigand::remove_if<
            brigand::transform<type_list, brigand::call<var_or_val_to_var>>,
            //brigand::remove_if<type_list, brigand::call<is_proto_value>>, // disabled override default value
            brigand::bind<is_pkt_sz_category, brigand::call<proto::desc_type_t>>
        >, inherits>;

        template<class Val>
        using check_param = enable_type<Val, void_t<check_and_return_t<std::is_base_of, var_type_t<check_and_return_t<is_proto_value, Val>>, var_list>>>;

    public:
        constexpr packet_description(Ts... vals)
        : packet_description(type_list_only_val{}, inherits<Ts...>{vals...})
        {}

        template<class... Val>
        constexpr auto
        operator()(Val... values) const
        {
            return ordering_parameter<check_param<Val>...>({values...});
        }

    private:
        template<class... Val>
        constexpr auto
        ordering_parameter(inherit_refs<Val...> refs) const
        {
            return packet<
                mk_iseq<sizeof...(Ts)>,
                decltype(get_packet_value<var_or_val_to_var<Ts>>(refs, this->values, 1))...
            >{get_packet_value<var_or_val_to_var<Ts>>(refs, this->values, 1)...};
        }
    };

    template<class T>
    struct is_empty_or_proto_val
    : std::is_empty<T>
    {};

    template<class Var, class T>
    struct is_empty_or_proto_val<val<Var, T>>
    : std::true_type
    {};

    template<class... Desc>
    constexpr auto
    desc(Desc... d)
    {
        return packet_description<
            check_and_return_t<is_empty_or_proto_val, check_and_return_t<is_proto_variable, Desc>>...
        >{d...};
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

    namespace utils {
        template<class... Ts>
        struct selector
        {
            selector(Ts & ... x) : refs{x...}
            {}

            template<class T>
            decltype(auto) get(T) const noexcept
            { return proto::ref_to_val<T>(refs).x; }

        private:
            proto::inherit_refs<Ts...> refs;
        };
    }
}

namespace XXX {
    PROTO_VAR(proto::types::u8, a);
    PROTO_VAR(proto::types::u8, b);
    PROTO_VAR(proto::types::bytes, c);
    PROTO_VAR(proto::types::u16_le, d);
    PROTO_VAR(proto::types::str8_to_str16, e);
    PROTO_VAR(proto::types::u16_encoding, f);
    PROTO_VAR(proto::types::pkt_sz<proto::types::u8>, sz);
    PROTO_VAR(proto::types::pkt_sz_with_self<proto::types::u8>, sz2);

    constexpr auto desc = proto::desc(a, b, c, d, e, f, sz, sz2);
}


#include "utils/sugar/numerics/safe_conversions.hpp"
#include <iostream>

struct Printer
{
    template<class var, class T>
    void operator()(proto::val<var, T> x) const {
        std::cout << var::name() << " = "; print(x.x, 1);
        std::cout
            << "  static: " << proto::is_static_buffer<typename var::desc_type>{}
            << "  dyn: " << proto::is_dynamic_buffer<typename var::desc_type>{}
            << "  view: " << proto::is_view_buffer<typename var::desc_type>{}
            << "  limited: " << proto::is_limited_buffer<typename var::desc_type>{}
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
    proto::is_dynamic_buffer<T>::value or
    proto::is_view_buffer<T>::value
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

template<class T>
using var_type = proto::var_or_val_to_var<T>;


struct Buffering
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
    { using type = L<::add_size<Ts, add>..., add>; };
}
template<class L, class x>
using mk_list_accu = typename lazy::mk_list_accu<L, x>::type;

template<class L>
using make_accumulate_sizeof_list = brigand::fold<L, brigand::list<>, brigand::call<mk_list_accu>>;


namespace detail {
    template<class T, std::size_t n> struct pkt_sz_with_size { using desc_type = T; };
}

namespace proto {
    template<class T, std::size_t n>
    struct sizeof_impl<::detail::pkt_sz_with_size<T, n>>
    : sizeof_impl<T>
    {};
}

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

template<class T>
using var_to_desc_type = desc_type_t<var_type<T>>;

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
        uint8_t buf [n];
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
using var_infos_is_not_dynamic = proto::is_dynamic_buffer<typename brigand::front<VarInfos>::desc_type>;

template<class T>
using var_info_is_pkt_sz = proto::is_pkt_sz_category<var_to_desc_type<T>>;

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
    struct iovec {
        void  *iov_base;
        size_t iov_len;
    };

    template<std::size_t n>
    struct Buffers
    {
        std::array<iovec, n> data {};

        template<class Tuple>
        Buffers(Tuple & t)
        {
            // TODO slow
            brigand::for_each<mk_seq<n>>([this, &t](auto I) {
                using i = t_<decltype(I)>;
                this->init_buf(i{}, std::get<i::value>(t));
            });
        }

        iovec & operator[](std::size_t i) noexcept
        {
            return this->data[i];
        }

        // TODO static_array_view<T, n>
        array_view<iovec const> view() const
        {
            return make_array_view(this->data.data(), this->data.size());
        }

        template<class Tuple>
        void reset_ptr(Tuple & t)
        {
            // TODO slow
            brigand::for_each<mk_seq<n>>([this, &t](auto I) {
                using i = t_<decltype(I)>;
                this->reset_buf_ptr(i{}, std::get<i::value>(t));
            });
        }

    private:
        template<class I, std::size_t arr_len>
        void init_buf(I i, uninitialized_buf<arr_len> & uninit_buf) {
            std::cout << i << " - " << static_cast<void*>(uninit_buf.buf) << std::endl;
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

using iovec_view = array_view<detail::iovec const>;

template<class Policy>
struct Buffering2
{
    using iovec = detail::iovec;

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
        using default_buffer_size = brigand::transform<packet_list_, brigand::call<sizeof_packet_with_limited_size_to_dyn_size>>;

        using pkt_sz_list = brigand::copy_if<
            var_info_list,
            brigand::bind<
                proto::is_pkt_sz_category,
                brigand::call<proto::desc_type_t>
            >
        >;

        // TODO rapidtuple and contiguous addr
        brigand::wrap<buffer_list, std::tuple> buffer_tuple;
        detail::Buffers<brigand::size<buffer_list>::value> buffers{buffer_tuple};
        uint8_t * pktptrs[brigand::size<pkt_sz_list>::value];
        detail::Sizes<default_buffer_size> sizes;
        // TODO reference
        Policy policy;

        void impl(Pkts & ... packets)
        {
            // TODO check if pkt_sz is a static_buffer
            std::cout << "pktptrs.size: " << (sizeof(this->pktptrs)/sizeof(this->pktptrs[0])) << "\n";

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
                    this->buffers[Ints::value],
                    pkts...
                )
            ), 1)...};
        }

        template<class VarInfos, class... Ts>
        static void write_not_dynamic_buf(std::true_type, VarInfos, iovec & buffer, Ts && ...)
        { std::cout << "-------\n(dyn) { 0x" << buffer.iov_base << " }\n"; }

        template<class... VarInfos>
        void write_not_dynamic_buf(std::false_type, brigand::list<VarInfos...>, iovec & buffer, Pkts & ... pkts) {
            std::cout << "-------\n";
            (void)std::initializer_list<int>{(void(
                this->write_type(
                    proto::buffer_category<typename VarInfos::desc_type>{}, VarInfos{}, buffer,
                    larg<VarInfos::ivar::value>(arg<VarInfos::ipacket::value>(pkts...)))
            ), 1)...};
        }

        template<class VarInfo, class Var>
        void write_type(proto::tags::static_buffer, VarInfo, iovec & buffer, Var & var) {
            std::cout << var_type<Var>::name() << " = ";
            this->print(var);
            this->write_pkt_sz_with_size_or_var(VarInfo{}, buffer, var);
            buffer.iov_base = static_cast<uint8_t*>(buffer.iov_base) + proto::sizeof_<desc_type_t<VarInfo>>{};
            std::cout << "\n";
        }

        template<class IPacket, class IVar, class T, class Var>
        void write_pkt_sz_with_size_or_var(
            var_info<IPacket, IVar, proto::types::pkt_sz<T>> vinfo, iovec & buffer, Var &
        ) {
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            this->pktptrs[pkt_idx] = static_cast<uint8_t*>(buffer.iov_base);
            std::cout << "  {" << static_cast<void*>(buffer.iov_base) << "} { pktptrs[" << pkt_idx << "] }";
        }

        template<class IPacket, class IVar, class T, class Var>
        void write_pkt_sz_with_size_or_var(
            var_info<IPacket, IVar, proto::types::pkt_sz_with_self<T>> vinfo, iovec & buffer, Var &
        ) {
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            this->pktptrs[pkt_idx] = static_cast<uint8_t*>(buffer.iov_base);
            std::cout << "  {" << static_cast<void*>(buffer.iov_base) << "} { pktptrs[" << pkt_idx << "] }";
        }

        template<class IPacket, class IVar, class T, std::size_t n, class Var>
        void write_pkt_sz_with_size_or_var(
            var_info<IPacket, IVar, detail::pkt_sz_with_size<T, n>>, iovec & buffer, Var &
        ) {
            using proto_integer = typename T::type;
            policy.write_static_buffer(
                static_cast<uint8_t*>(buffer.iov_base),
                checked_cast<typename proto_integer::type>(n),
                proto_integer{}
            );
            std::cout << " = " << n;
        }

        template<class VarInfo, class Var>
        void write_pkt_sz_with_size_or_var(VarInfo, iovec & buffer, Var const & var)
        {
            policy.write_static_buffer(static_cast<uint8_t*>(buffer.iov_base), var.x, desc_type_t<VarInfo>{});
        }

        template<class VarInfo, class Var>
        void write_type(proto::tags::limited_buffer, VarInfo, iovec & buffer, Var & var) {
            std::cout << var_type<Var>::name() << " = ";
            this->print(var);
            std::size_t len = policy.write_limited_buffer(
                static_cast<uint8_t*>(buffer.iov_base),
                var.x,
                desc_type_t<VarInfo>{}
            );
            std::cout << " [len: " << len << "]\n";
            buffer.iov_base = static_cast<uint8_t*>(buffer.iov_base) + len;
            this->sizes.data[VarInfo::ipacket::value] += len;
            static_assert(!var_info_is_pkt_sz<VarInfo>{}, "");
        }

        template<class VarInfo, class Var>
        void write_type(proto::tags::view_buffer, VarInfo, iovec & buffer, Var & var) {
            std::cout << var_type<Var>::name() << " = ";
            this->print(var);
            auto av = policy.get_view_buffer(var.x, desc_type_t<VarInfo>{});
            buffer.iov_base = const_cast<uint8_t *>(av.data());
            buffer.iov_len = av.size();
            this->sizes.data[VarInfo::ipacket::value] += av.size();
            static_assert(!var_info_is_pkt_sz<VarInfo>{}, "");
            std::cout << " [view: 0x" << buffer.iov_base << " | len: " << buffer.iov_len << "]\n";
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

            this->buffers.reset_ptr(this->buffer_tuple);

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

            this->policy.send(this->buffers.view());
        }

        template<class I, class VarInfos, class... VarInfosBuffers>
        void write_dynamic_buf(I, brigand::list<VarInfos, VarInfosBuffers...>, Pkts & ... pkts) {
            using var_info = brigand::front<VarInfos>;
            this->write_dyn_type(
                larg<var_info::ivar::value>(arg<var_info::ipacket::value>(pkts...)),
                [this, &pkts...](array_view_const_u8 av) {
                    auto & buffer = this->buffers[I::value];
                    buffer.iov_base = const_cast<uint8_t *>(av.data());
                    buffer.iov_len = av.size();
                    this->sizes.data[var_info::ipacket::value] += av.size();
                    std::cout << " [size: " << av.size() << "]";
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
        void write_pkt_sz_var(var_info<IPacket, IVar, proto::types::pkt_sz<T>> vinfo) {
            using is_proto_integer = std::is_same<T, brigand::wrap<T, proto::types::integer>>;
            static_assert(is_proto_integer{}, "only proto::types::integer is supported with pkt_sz");
            // TODO
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            std::cout << "pktptrs[" << pkt_idx << "] {"
              << static_cast<void*>(this->pktptrs[pkt_idx]) << "} = "
              << this->sizes.data[IPacket::value+1] << "\n";
            policy.write_static_buffer(
                this->pktptrs[pkt_idx],
                checked_cast<typename T::type>(this->sizes.data[IPacket::value+1]),
                T{}
            );
            std::cout << "\n";
        }

        template<class IPacket, class IVar, class T>
        void write_pkt_sz_var(var_info<IPacket, IVar, proto::types::pkt_sz_with_self<T>> vinfo) {
            using is_proto_integer = std::is_same<T, brigand::wrap<T, proto::types::integer>>;
            static_assert(is_proto_integer{}, "only proto::types::integer is supported with pkt_sz_with_self");
            // TODO
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            std::cout << "pktptrs[" << pkt_idx << "] {"
              << static_cast<void*>(this->pktptrs[pkt_idx]) << "} = "
              << this->sizes.data[IPacket::value] << "\n";
            policy.write_static_buffer(
                this->pktptrs[pkt_idx],
                checked_cast<typename T::type>(this->sizes.data[IPacket::value]),
                T{}
            );
            std::cout << "\n";
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

#define LOGPRINT
#include "utils/log.hpp"

struct stream_protocol_policy;

namespace iov_ext
{
    template<class T, class Endianess>
    void write_static_buffer(uint8_t * p, T val, proto::types::integer<T, Endianess>)
    {
        std::cout << " [static_buffer] [sizeof_: " << sizeof(T) << "] {" << static_cast<void*>(p) << "}";
        using rng = brigand::range<std::size_t, 0, sizeof(T)>;
        using is_little_endian = t_<std::is_same<Endianess, proto::types::le_tag>>;
        brigand::for_each<rng>([&p, val](auto I) {
            constexpr const std::size_t i = t_<decltype(I)>::value;
            // TODO std::make_unsigned
            std::cout << " { *p++ }";
            *p++ = val >> ((is_little_endian{} ? i : sizeof(T)-1-i) * 8);
        });
    }

    inline array_view_const_u8 get_view_buffer(array_view_const_u8 av, proto::types::bytes)
    {
        std::cout << " [view_buffer] [size: " << av.size() << "]";
        return av;
    }

    template<class T, class Desc>
    inline std::size_t
    write_static_buffer_as_limited_buffer(uint8_t * p, T val, Desc desc)
    {
        write_static_buffer(p, val, desc);
        return proto::sizeof_<Desc>{};
    }

    inline std::size_t write_limited_buffer(uint8_t * p, uint16_t val, proto::types::u16_encoding)
    {
        assert(!(val & 0x8000));
        std::cout << " [limited_buffer]";
        if (val <= 127) {
            return write_static_buffer_as_limited_buffer(p, uint8_t(val), proto::types::u8{});
        }
        else {
            return write_static_buffer_as_limited_buffer(p, val, proto::types::u16_be{});
        }
    }

    inline std::size_t write_limited_buffer(uint8_t * p, uint32_t val, proto::types::u32_encoding)
    {
        assert(!(val & 0xC0000000));
        std::cout << " [limited_buffer]";
        if (val <= 0x3FFF  ) {
            if (val <= 0x3F      ) {
                write_static_buffer(p, uint8_t(val), proto::types::u8{});
                return 1u;
            }
            else {
                write_static_buffer(  p, uint8_t(0x40 | ((val >> 8 ) & 0x3F)), proto::types::u8{});
                write_static_buffer(++p, uint8_t(        (val        & 0xFF)), proto::types::u8{});
                return 2u;
            }
        }
        else {
            if (val <= 0x3FFFFF) {
                write_static_buffer(  p, uint8_t(0x80 | ((val >> 16) & 0xFF)), proto::types::u8{});
                write_static_buffer(++p, uint8_t(       ((val >> 8 ) & 0xFF)), proto::types::u8{});
                write_static_buffer(++p, uint8_t(        (val        & 0xFF)), proto::types::u8{});
                return 3u;
            }
            else {
                write_static_buffer(  p, uint8_t(0xC0 | ((val >> 24) & 0x3F)), proto::types::u8{});
                write_static_buffer(++p, uint8_t(0x80 | ((val >> 16) & 0x3F)), proto::types::u8{});
                write_static_buffer(++p, uint8_t(       ((val >> 8 ) & 0xFF)), proto::types::u8{});
                write_static_buffer(++p, uint8_t(        (val        & 0xFF)), proto::types::u8{});
                return 4u;
            }
        }
    }

    template<class F>
    void context_dynamic_buffer(F && f, array_view_const_u8 av, proto::types::str8_to_str16)
    {
        std::cout << " [dynamic_buffer]";
        f(av);
    }


    template<class Cond, class T>
    array_view_const_u8 get_view_buffer(typename T::type val, proto::types::enable_if<Cond, T>)
    {
        return Cond{}(val) ? get_view_buffer(val, T{}) : array_view_const_u8{};
    }

    template<class Cond, class T>
    std::enable_if_t<proto::is_static_buffer<T>::value, std::size_t>
    write_limited_buffer(uint8_t * p, typename T::type val, proto::types::enable_if<Cond, T>)
    {
        return Cond{}(val) ? write_static_buffer_as_limited_buffer(p, val, T{}) : 0u;
    }

    template<class Cond, class T>
    std::enable_if_t<!proto::is_static_buffer<T>::value, std::size_t>
    write_limited_buffer(uint8_t * p, typename T::type val, proto::types::enable_if<Cond, T>)
    {
        return Cond{}(val) ? write_limited_buffer(p, val, T{}) : 0u;
    }

    template<class F, class Cond, class T>
    void context_dynamic_buffer(F && f, typename T::type val, proto::types::enable_if<Cond, T>)
    {
        if (Cond{}(val)) {
            context_dynamic_buffer(f, val, T{});
        }
        else {
            f({});
        }
    }


    template<class T, class Desc>
    void write_static_buffer_impl(uint8_t * p, T val, Desc desc)
    {
        write_static_buffer(p, val, desc);
    }

    template<class T, class Desc>
    auto get_view_buffer_impl(T val, Desc desc)
    {
        return get_view_buffer(val, desc);
    }

    template<class T, class Desc>
    std::size_t write_limited_buffer_impl(uint8_t * p, T val, Desc desc)
    {
        return write_limited_buffer(p, val, desc);
    }

    template<class F, class T, class Desc>
    void context_dynamic_buffer_impl(F && f, T val, Desc desc)
    {
        context_dynamic_buffer(f, val, desc);
    }
}

struct stream_protocol_policy
{
    template<class T, class Desc>
    void write_static_buffer(uint8_t * p, T val, Desc desc)
    {
        iov_ext::write_static_buffer_impl(p, val, desc);
    }

    template<class T, class Desc>
    auto get_view_buffer(T val, Desc desc)
    {
        return iov_ext::get_view_buffer_impl(val, desc);
    }

    template<class T, class Cond, class True, class False>
    std::size_t write_limited_buffer(uint8_t * p, T val, proto::types::if_<Cond, True, False>);
//     {
//         std::cout << " [limited_buffer]";
//         if (Cond{}(val)) {
//             return this->write(p, /*typename True::type*/(val), True{});
//         }
//         else {
//             return this->write(p, /*typename False::type*/(val), False{});
//         }
//     }

    template<class T, class Desc>
    std::size_t write_limited_buffer(uint8_t * p, T val, Desc desc)
    {
        return iov_ext::write_limited_buffer_impl(p, val, desc);
    }

    template<class F, class T, class Desc>
    void context_dynamic_buffer(F && f, T val, Desc desc)
    {
        iov_ext::context_dynamic_buffer_impl(f, val, desc);
    }

    void send(iovec_view iovs) {
        for (auto iov : iovs) {
            std::cout << " [" << iov.iov_base << "] [len: " << iov.iov_len << "]\n";
            hexdump_c(static_cast<uint8_t const*>(iov.iov_base), iov.iov_len);
        }
    }
};


void test_old();
void test_new();

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
        XXX::e = /*cstr_*/make_array_view(pkt.c),
        XXX::f = pkt.d/*,
//         XXX::sz,
        XXX::sz2
      , 1*/
    );

    packet.apply_for_each(Printer{});
    std::cout << "\n";
    packet.apply(Buffering{});
    std::cout << "\n";
    proto::apply(Buffering2<stream_protocol_policy>{}, packet, packet);

//     std::cout << "\n\n======== old ========\n\n";
//     test_old();
//     std::cout << "\n\n======== new ========\n\n";
//     test_new();
}

#include "core/RDP/sec.hpp"
#include "core/RDP/x224.hpp"

void test_old() {
    uint8_t data[10];
    CryptContext crypt;

    uint8_t buf[256];
    OutStream out_stream(buf + 126, 126);
    StaticOutStream<128> hstream;
    SEC::Sec_Send(out_stream, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0);
    X224::DT_TPDU_Send(hstream, out_stream.get_offset());
//     BOOST_REQUIRE_EQUAL(4, out_stream.get_offset());
//     BOOST_REQUIRE_EQUAL(7, hstream.get_offset());
    auto p = out_stream.get_data() - hstream.get_offset();
//     BOOST_REQUIRE_EQUAL(11, out_stream.get_current() - p);
    memcpy(p, hstream.get_data(), hstream.get_offset());
    out_stream = OutStream(p, out_stream.get_current() - p);
    out_stream.out_skip_bytes(out_stream.get_capacity());
    hexdump_c(out_stream.get_data(), out_stream.get_offset());
}

// https://github.com/jonathanpoelen/falcon.parse_number
#include <falcon/literals/integer_constant.hpp>
using namespace falcon::literals::integer_constant_literals;

namespace x224
{
    PROTO_VAR(proto::types::u8, version);
    PROTO_VAR(proto::types::u8, unknown);
    PROTO_VAR(proto::types::pkt_sz_with_self<proto::types::u16_be>, pkt_len);
    PROTO_VAR(proto::types::u8, LI);
    PROTO_VAR(proto::types::u8, type);
    PROTO_VAR(proto::types::u8, cat);

    constexpr auto dt_tpdu_send = proto::desc(
        version = 3_c,
        unknown = 0_c,
        pkt_len,
        LI = 2_c,
        type = X224::DT_TPDU,
        cat = X224::EOT_EOT
    );
}

namespace sec
{
    struct signature_fn
    {
        struct type {
            array_view_u8 av;
            CryptContext & crypt;
        };

        using sizeof_ = proto::size_<8>;
    };

    inline void write_static_buffer(uint8_t * p, signature_fn::type ctx, signature_fn)
    {
        auto & signature = reinterpret_cast<uint8_t(&)[signature_fn::sizeof_{}]>(*p);
        ctx.crypt.sign(ctx.av.data(), ctx.av.size(), signature);
        ctx.crypt.decrypt(ctx.av.data(), ctx.av.size());
    }

    PROTO_VAR(proto::types::mutable_bytes, data);
    PROTO_VAR(proto::types::u32_le, flags);
    PROTO_VAR(proto::types::u32_le, encryptionLevel);
    PROTO_VAR(proto::types::val<CryptContext&>, crypt);

    struct sec_send_pkt
    {
        struct type
        {
            uint32_t flags;
            signature_fn::type sig;
        };

        using buffer_size = proto::size_<signature_fn::sizeof_{} + proto::get_size(flags)>;
        using sizeof_ = proto::limited_size<buffer_size::value>;
    };

    inline std::ostream & operator <<(std::ostream & os, sec_send_pkt::type const &)
    {
        return os << "sec_send_pkt::type";
    }

    inline std::size_t write_limited_buffer(uint8_t * buf, sec_send_pkt::type ctx, sec_send_pkt)
    {
        std::cout << " [limited_buffer]";
        uint8_t * p = buf;
        if (ctx.flags) {
            p += iov_ext::write_static_buffer_as_limited_buffer(p, ctx.flags, proto::get_desc(flags));
        }
        if (ctx.flags & SEC::SEC_ENCRYPT){
            p += iov_ext::write_static_buffer_as_limited_buffer(p, ctx.sig, signature_fn{});
        }
        return p - buf;
    }

    PROTO_VAR(sec_send_pkt, pkt);

    template<class... T>
    auto sec_send(T... args)
    {
        proto::utils::selector<T...> selector{args...};
        return proto::desc(pkt)(pkt = sec_send_pkt::type{
            selector.get(flags) | unsigned(selector.get(encryptionLevel) ? SEC::SEC_ENCRYPT : 0),
            {
                selector.get(data),
                selector.get(crypt),
            }
        });
    }
}


void test_new()
{
    auto packet1 = x224::dt_tpdu_send();

    uint8_t data[10];
    CryptContext crypt;
    auto packet2 = sec::sec_send(sec::flags = ~SEC::SEC_ENCRYPT, sec::crypt = crypt, sec::data = data, sec::encryptionLevel = 0_c);

    proto::apply(Buffering2<stream_protocol_policy>{}, packet1, packet2);
}
