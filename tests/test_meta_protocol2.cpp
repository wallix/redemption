#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestVerifier
#include <boost/test/auto_unit_test.hpp>

#include <iostream>

#include <limits>
#include <utility>
#include <cstdint>


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
#include "utils/sugar/bytes_t.hpp"


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

namespace proto
{
    template<std::size_t N>
    using size_ = std::integral_constant<std::size_t, N>;

    template<class T>
    struct internal_type
    { using type = typename T::type; };

    template<class T>
    using internal_type_t = typename internal_type<T>::type;


    struct dyn_size {};
    template<std::size_t i> struct limited_size {};

    namespace tags
    {
        class static_buffer {};
        class dynamic_buffer {};
        class view_buffer {};
        class limited_buffer {};
    }

    template<class T> struct sizeof_impl { using type = typename T::sizeof_; };

    template<class T> using sizeof_ = typename sizeof_impl<T>::type;

    template<class...> using void_t = void;

    namespace detail
    {
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

    template<class T>
    using t_ = typename T::type;

    template<class... Ts, class F>
    void for_each(brigand::list<Ts...>, F && f) {
        (void)std::initializer_list<int>{
            (void(f(Ts{})), 1)...
        };
    }


    // clang narrowing checker... (cf: safe_int<T> = T)
    template<class T>
    struct safe_int
    {
        T val;

        template<class U> constexpr safe_int(U x) noexcept : val{x} {}

        operator T () const { return val; }
    };

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

            static_assert(std::is_integral<T>::value, "");

            safe_int<type> val;

            sizeof_ static_serialize(uint8_t * p) const
            {
                std::cout << " [static_buffer] [sizeof_: " << sizeof(T) << "] {" << static_cast<void*>(p) << "}";
                using rng = brigand::range<std::size_t, 0, sizeof(T)>;
                using is_little_endian = t_<std::is_same<Endianess, le_tag>>;
                for_each(rng{}, [&p, this](auto i) {
                    // TODO std::make_unsigned
                    std::cout << " { *p++ }";
                    *p++ = this->val >> ((is_little_endian{} ? i : sizeof(T)-1-i) * 8);
                });
                return sizeof_{};
            }
        };

        template<class E, class ProtoType>
        struct enum_ : ProtoType
        {
            static_assert(std::is_enum<E>::value, "");

            using type = E;

            constexpr enum_(E e) noexcept : ProtoType{typename ProtoType::type(e)} {}
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

        template<class E> using enum_s8 = enum_<E, s8>;
        template<class E> using enum_u8 = enum_<E, u8>;

        template<class E> using enum_s16_be = enum_<E, s16_be>;
        template<class E> using enum_s16_le = enum_<E, s16_le>;
        template<class E> using enum_u16_be = enum_<E, u16_be>;
        template<class E> using enum_u16_le = enum_<E, u16_le>;

        template<class E> using enum_s32_be = enum_<E, s32_be>;
        template<class E> using enum_s32_le = enum_<E, s32_le>;
        template<class E> using enum_u32_be = enum_<E, u32_be>;
        template<class E> using enum_u32_le = enum_<E, u32_le>;

        template<class E> using enum_s64_be = enum_<E, s64_be>;
        template<class E> using enum_s64_le = enum_<E, s64_le>;
        template<class E> using enum_u64_be = enum_<E, u64_be>;
        template<class E> using enum_u64_le = enum_<E, u64_le>;
        /** @} */

        /**
        * @{
        */
        struct u16_encoding
        {
            using type = uint16_t;
            using sizeof_ = limited_size<sizeof(type)>;

            safe_int<type> val;

            std::size_t limited_serialize(uint8_t * p) const
            {
                assert(!(val & 0x8000));
                return (val <= 127)
                    ? u8{uint8_t(val)}.static_serialize(p)
                    : u16_be{val}.static_serialize(p);
            }
        };
        template<class E> using enum_u16_encoding = enum_<E, u16_encoding>;

        struct u32_encoding
        {
            using type = uint32_t;
            using sizeof_ = limited_size<sizeof(type)>;

            safe_int<type> val;

            std::size_t limited_serialize(uint8_t * p) const
            {
                assert(!(val & 0xC0000000));
                auto serial = [&p](uint8_t v) { return u8{v}.static_serialize(p++); };
                return (val <= 0x3FFF)
                ?
                    (val <= 0x3F)
                    ?
                        serial(        val       )
                    :
                        serial(0x40 | (val >> 8 )) +
                        serial(        val       )
                :
                    (val <= 0x3FFFFF)
                    ?
                        serial(0x80 | (val >> 16)) +
                        serial(        val >> 8  ) +
                        serial(        val       )
                    :
                        serial(0xC0 | (val >> 24)) +
                        serial(       (val >> 16)) +
                        serial(        val >> 8  ) +
                        serial(        val       )
                ;
            }
        };
        template<class E> using enum_u32_encoding = enum_<E, u32_encoding>;
        /** @} */

        template<class Obj, class T>
        using enable_if_not_default_ctor_argument_t
            = std::enable_if_t<(!std::is_same<Obj, std::remove_reference_t<T>>::value)>;

        struct bytes
        {
            using type = array_view_const_u8;
            using sizeof_ = dyn_size;
            using buffer_category = tags::view_buffer;

            type av;

            template<class T, class = enable_if_not_default_ctor_argument_t<bytes, T>>
            constexpr bytes(T && av) noexcept
            : av(av)
            {}

            constexpr bytes(array_view_const_u8 av) noexcept
            : av(av)
            {}

            bytes(array_view_const_char av) noexcept
            : av(reinterpret_cast<uint8_t const*>(av.data()), av.size())
            {}

            array_view_const_u8 get_view_buffer() const
            {
                std::cout << " [view_buffer] [size: " << av.size() << "]";
                return av;
            }
        };

        struct mutable_bytes
        {
            using type = array_view_u8;
            using sizeof_ = dyn_size;
            using buffer_category = tags::view_buffer;

            type av;

            template<class T, class = enable_if_not_default_ctor_argument_t<mutable_bytes, T>>
            constexpr mutable_bytes(T && av) noexcept
            : av(av)
            {}

            constexpr mutable_bytes(array_view_u8 av) noexcept
            : av(av)
            {}

            mutable_bytes(array_view_char av) noexcept
            : av(reinterpret_cast<uint8_t *>(av.data()), av.size())
            {}

            array_view_const_u8 get_view_buffer() const
            {
                std::cout << " [view_buffer] [size: " << av.size() << "]";
                return av;
            }
        };

        struct str8_to_str16
        {
            using type = const_bytes_array;
            using sizeof_ = dyn_size;

            type str;

            constexpr str8_to_str16(const_bytes_array av) noexcept
            : str(av)
            {}

            template<class F>
            void dynamic_serialize(F && f) const
            {
                std::cout << " [dynamic_buffer]";
                f(this->str);
            }
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

        // TODO unimplemented
        template<class Cond, class True, class False>
        struct if_
        {
            //using type =
            using sizeof_ = common_size<proto::sizeof_<True>, proto::sizeof_<False>>;
            using buffer_category = common_buffer<proto::buffer_category<True>, proto::buffer_category<False>>;
        };

        // TODO unimplemented
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

            type val;

            array_view_const_u8 get_view_buffer() const
            {
                return Cond{}(this->val) ? T{val}.get_view_buffer() : array_view_const_u8{};
            }

            std::size_t serialize_limited_buffer(uint8_t * p) const
            {
                return Cond{}(this->val) ? T{val}.limited_serialize(p) : 0u;
            }

            template<class F>
            void dynamic_serialize(F && f) const
            {
                if (Cond{}(this->val)) {
                    T{val}.context_dynamic_buffer(f);
                }
                else {
                    f({});
                }
            }
        };

        template<class T>
        struct value
        {
            T val;
        };
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
        Var var;
        T x;
    };

    template<class Derived, class Desc, class T>
    constexpr auto make_val(T && x)
    { return Desc{std::forward<T>(x)}; }


    template<class Desc, class Derived>
    struct var
    {
        using desc_type = Desc;
        using var_type = var;

        template<class U>
        constexpr auto operator = (U && x) const
        // Clang bug
        // {
        //     return val<Derived, decltype(trace_adapt<Derived>(Desc{}, std::forward<U>(x)))>{
        //         trace_adapt<Derived>(Desc{}, std::forward<U>(x))
        //     };
        // }
        { return impl(std::forward<U>(x)); }

        template<class U>
        constexpr auto impl(U && x) const
        -> val<Derived, decltype(make_val<Desc, Desc>(std::forward<U>(x)))>
        { return {Derived{}, make_val<Desc, Desc>(std::forward<U>(x))}; }
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

        template<class T> struct is_proto_value_impl : std::false_type {};
        template<class Var, class T> struct is_proto_value_impl<val<Var, T>> : std::true_type {};
    }
    template<class T> using is_proto_variable = typename detail::is_proto_variable_impl<T>::type;
    template<class T> using is_proto_value = typename detail::is_proto_value_impl<T>::type;

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
    get_packet_value(Refs refs, Vals &, int)
    -> decltype(ref_to_val<Var>(refs))
    { return refs; }


    template<class T, class = void>
    using enable_type = T;

    template<std::size_t n> using mk_iseq = std::make_integer_sequence<std::size_t, n>;

    namespace detail
    {
        template<class T>
        struct var_or_val_to_var_impl
        { using type = T; };

        template<class Var, class T>
        struct var_or_val_to_var_impl<val<Var, T>>
        { using type = Var; };
    }
    template<class T>
    using var_or_val_to_var = typename detail::var_or_val_to_var_impl<T>::type;

    namespace cexp
    {
        constexpr std::size_t strlen(char const * s)
        {
            std::size_t n = 0;
            while (*s) {
                ++n;
                s++;
            }
            return n;
        }

        constexpr std::size_t strcpy(char * dst, char const * src)
        {
            char * p = dst;
            while (*src) {
                *p++ = *src++;
            }
            return p - dst;
        }

        template<class T, class... Ts>
        constexpr T fold(T a, Ts... as)
        {
            (void)std::initializer_list<int>{
                (void(a += as), 1)...
            };
            return a;
        }
    }

    class unamed {};

    template<class T, class Varname, class... Var>
    struct vars
    : var<T, vars<T, Varname, Var...>>
    {
        using ::proto::var<T, vars>::var;
        using ::proto::var<T, vars>::operator = ;

        constexpr vars(char const * varname) noexcept : varname(varname) {}

        constexpr char const *
        name() const noexcept
        { return this->varname; }

        char const * varname;
    };

    template<class T, class... Var>
    struct vars<T, unamed, Var...>
    : var<T, vars<T, unamed, Var...>>
    {
        using ::proto::var<T, vars>::var;
        using ::proto::var<T, vars>::operator = ;

        constexpr vars(unamed) noexcept {}

        static constexpr auto
        name() noexcept
        {
            struct Name
            {
                char s[cexp::fold(cexp::strlen(Var::name())...) + 4 + sizeof...(Var)];

                constexpr Name()
                {
                    char * p = s;
                    *p++ = '{';
                    (void)std::initializer_list<int>{
                        (void(p += cexp::strcpy(&(p[0] = ' ') + 1, Var::name()) + 1), 1)...
                    };
                    *p++ = ' ';
                    *p++ = '}';
                    *p = 0;
                }

                constexpr operator char const * () const noexcept { return s; }
            };
            return Name();
        }
    };

    template<class T, class Varname, class... Descs>
    class creator
    {
        using type_list = brigand::list<Descs...>;
        using type_list_only_val = brigand::filter<type_list, brigand::call<is_proto_value>>;
        using desc_value_tuple = brigand::wrap<type_list_only_val, inherits>;

        Varname varname;
        desc_value_tuple values;

        template<class... Us, class Refs>
        constexpr creator(Varname varname, brigand::list<Us...>, Refs refs)
        : varname{varname}
        , values{static_cast<Us>(refs)...}
        {}

        template<class Refs>
        constexpr creator(Varname varname, brigand::list<>, Refs)
        : varname{varname}
        , values{}
        {}

    public:
        constexpr creator(Varname varname, Descs... descs)
        : creator(varname, type_list_only_val{}, inherits<Descs...>{descs...})
        {}

        template<class... Val>
        constexpr val<vars<T, Varname, Descs...>, T> operator()(inherit_refs<Val...> refs) const
        {
#ifdef __clang__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmissing-braces"
#endif
            return {
                vars<T, Varname, Descs...>{varname},
                T{(get_packet_value<var_or_val_to_var<Descs>>(refs, this->values, 1).x)...}
            };
#ifdef __clang__
# pragma GCC diagnostic pop
#endif
        }

        template<class... Val>
        constexpr val<vars<T, Varname, Descs...>, T> operator()(Val... values) const
        {
            return (*this)(inherit_refs<Val...>{values...});
        }
    };

    template<class... Ts>
    struct subpacket_value
    {
        using sizeof_ = brigand::fold<
            brigand::list<proto::sizeof_<desc_type_t<Ts>>...>,
            size_<0>,
            brigand::call<common_size>
        >;

        constexpr subpacket_value(desc_type_t<Ts>... v) : values{v...} {}

        inherits<desc_type_t<Ts>...> values;

        std::size_t limited_serialize(uint8_t * p) const
        {
            std::size_t sz = 0;
            (void)std::initializer_list<int>{
                (sz += static_cast<Ts>(this->values).serialize(p + sz))...
            };
            return sz;
        }

        std::size_t static_serialize(uint8_t * p) const
        {
            std::size_t sz = 0;
            (void)std::initializer_list<int>{
                (sz += serialize(p + sz, static_cast<Ts>(this->values)))...
            };
            return sz;
        }

    private:
        template<class T>
        static std::enable_if_t<is_static_buffer<T>::value, std::size_t>
        serialize(uint8_t * p, T const & x)
        { return x.static_serialize(p); }

        template<class T>
        static std::enable_if_t<!is_static_buffer<T>::value, std::size_t>
        serialize(uint8_t * p, T const & x)
        { return x.limited_serialize(p); }
    };

    namespace detail
    {
        template<class T>
        struct flatten_description_to_list_impl
        { using type = brigand::list<var_or_val_to_var<T>>; };

        template<class T, class Varname, class... Descs>
        struct flatten_description_to_list_impl<creator<T, Varname, Descs...>>
        { using type = brigand::list<var_or_val_to_var<Descs>...>; };

        template<class T>
        using flatten_description_to_list_t = typename flatten_description_to_list_impl<T>::type;
    }
    template<class L>
    using flatten_packet_description = brigand::wrap<
        brigand::transform<L, brigand::call<detail::flatten_description_to_list_t>>,
        brigand::append
    >;


    template<class T>
    struct packet_param_to_val_class
    {
        template<class Refs, class Vals>
        static auto impl(Refs refs, Vals & vals)
        { return get_packet_value<var_or_val_to_var<T>>(refs, vals, 1); }
    };

    template<class T, class Varname, class... Descs>
    struct packet_param_to_val_class<creator<T, Varname, Descs...>>
    {
        template<class Refs>
        static auto impl(Refs refs, creator<T, Varname, Descs...> c)
        { return c(refs); }
    };

    namespace detail
    {
        template<class T> struct is_creator : std::false_type {};
        template<class T, class Varname, class... Descs>
        struct is_creator<creator<T, Varname, Descs...>> : std::true_type {};
    }
    template<class T>
    using is_creator = typename detail::is_creator<T>::type;

    template<class T>
    using is_proto_value_or_proto_creator = brigand::bool_<is_proto_value<T>::value or is_creator<T>::value>;


    template<class... Ts>
    class packet_description
    {
        using type_list = brigand::list<Ts...>;

        using type_list_only_val_or_creator = brigand::filter<type_list, brigand::call<is_proto_value_or_proto_creator>>;

        using desc_value_tuple = brigand::wrap<type_list_only_val_or_creator, inherits>;

        desc_value_tuple values;

        template<class... Us, class Refs>
        constexpr packet_description(brigand::list<Us...>, Refs refs)
        : values{static_cast<Us>(refs)...}
        {}

        template<class Refs>
        constexpr packet_description(brigand::list<>, Refs)
        : values{}
        {}

        using var_list = brigand::wrap<brigand::remove_if<
            flatten_packet_description<type_list>,
            brigand::bind<is_pkt_sz_category, brigand::call<proto::desc_type_t>>
        >, inherits>;

        template<class Val>
        using check_param = enable_type<Val, void_t<check_and_return_t<std::is_base_of, var_type_t<check_and_return_t<is_proto_value, Val>>, var_list>>>;

    public:
        constexpr packet_description(Ts... vals)
        : packet_description(type_list_only_val_or_creator{}, inherits<Ts...>{vals...})
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
                decltype(packet_param_to_val_class<Ts>::impl(refs, this->values))...
            >{packet_param_to_val_class<Ts>::impl(refs, this->values)...};
        }
    };


    template<class T>
    using is_empty_or_proto_val = brigand::bool_<is_proto_value<T>::value or std::is_empty<T>::value>;

    template<class T, class... Desc>
    constexpr auto
    creater(Desc... d)
    {
        return creator<T, unamed, check_and_return_t<is_empty_or_proto_val, check_and_return_t<is_proto_variable, Desc>>...>{unamed{}, d...};
    }

    template<class T, class... Desc>
    constexpr auto
    creater(char const * name, Desc... d)
    {
        return creator<T, char const *, check_and_return_t<is_empty_or_proto_val, check_and_return_t<is_proto_variable, Desc>>...>{name, d...};
    }

    template<class... Desc>
    constexpr auto
    compose(Desc... d)
    {
        return creator<
            subpacket_value<typename Desc::var_type...>,
            unamed,
            check_and_return_t<is_empty_or_proto_val, check_and_return_t<is_proto_variable, Desc>>...
        >{unamed{}, d...};
    }

    template<class... Desc>
    constexpr auto
    compose(char const * name, Desc... d)
    {
        return creator<
            subpacket_value<typename Desc::var_type...>,
            char const *,
            check_and_return_t<is_empty_or_proto_val, check_and_return_t<is_proto_variable, Desc>>...
        >{name, d...};
    }

    template<class T>
    using is_empty_or_proto_data = brigand::bool_<
        is_creator<T>::value or
        (is_proto_variable<T>::value and is_empty_or_proto_val<T>::value)
    >;

    template<class... Desc>
    constexpr auto
    desc(Desc... d)
    {
        return packet_description<check_and_return_t<is_empty_or_proto_data, Desc>...>{d...};
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


    namespace utils
    {
        template<class... Ts>
        struct parameters
        {
            parameters(Ts & ... x) : refs{x...}
            {}

            template<class T>
            decltype(auto) operator[](T) const noexcept
            { return proto::ref_to_val<T>(refs).x; }

        private:
            proto::inherit_refs<Ts...> refs;
        };
    }


    constexpr struct params_
    {
        template<class T>
        struct param {};

        constexpr params_() noexcept {}

        template<class Desc, class Derived>
        auto operator[](var<Desc, Derived>) const noexcept {
            return param<Derived>{};
        }
    } params;

    namespace lazy
    {
#define PROTO_LAZY_BINARY_OP(name, op)                  \
        template<class T, class U>                      \
        struct name                                     \
        {                                               \
            template<class Params>                      \
            decltype(auto) operator()(Params p) const { \
                return x(p) op y(p);                    \
            }                                           \
                                                        \
            T x;                                        \
            U y;                                        \
        }

        PROTO_LAZY_BINARY_OP(bit_and, &);

#undef PROTO_LAZY_BINARY_OP

        template<class T>
        struct val
        {
            template<class Params>
            decltype(auto) operator()(Params) const {
                return x;
            }

            T x;
        };

        template<class Var>
        struct param
        {
            template<class Params>
            decltype(auto) operator()(Params p) const {
                return p[Var{}].val;
            }
        };
    }

    template<class T, class U>
    lazy::bit_and<lazy::param<T>, lazy::val<U>>
    constexpr operator & (params_::param<T>, U && x)
    { return {{}, x}; }

    namespace filters
    {
        namespace detail
        {
            template<class Val>
            struct only_if_true
            {
                using sizeof_ = dyn_size;
                using buffer_category = tags::view_buffer;

                std::size_t limited_serialize(uint8_t * p) const;

                array_view_const_u8 get_view_buffer() const;

                template<class F>
                void dynamic_serialize(F && f) const;

                bool is_ok;
                Val val;
            };

            template<class Cond, class Var>
            struct if_act
            {
                template<class Params>
                constexpr auto operator()(Params params) const
                {
                    return only_if_true<decltype(params[var])>{bool(cond(params)), params[var]};
                }

                Cond cond;
                Var var;
            };

            template<class Cond>
            struct if_
            {
                template<class Var>
                auto operator[](Var var) const
                {
                    return if_act<Cond, check_and_return_t<is_empty_or_proto_data, Var>>{cond, var};
                }

                Cond cond;
            };

            template<class Var>
            struct param_to_bool
            {
                template<class Params>
                constexpr bool operator()(Params params) const
                {
                    return bool(lazy::param<Var>{}(params));
                }
            };
        }

        template<class Cond>
        auto if_(Cond cond)
        {
            return detail::if_<Cond>{cond};
        }

        template<class Var>
        auto if_true(Var v)
        {
            return if_(detail::param_to_bool<Var>{})[v];
        }
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

struct Printer
{
    template<class var, class T>
    void operator()(proto::val<var, T> x) const {
        std::cout << x.var.name() << " = ";
        print(x.x, 1);
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
    static auto print(T const & x, int)
    -> decltype(void(std::cout << x.val))
    {
        using type = decltype(x.val);
        using casted_type = std::conditional_t<
            std::is_same<type, unsigned char>::value ||
            std::is_same<type, proto::safe_int<unsigned char>>::value,
            int,
            type const &
        >;
        std::cout << static_cast<casted_type>(x.val);
    }

    template<class T>
    static auto print(T const & x, int)
    -> decltype(void(std::cout << x.av.data()))
    { std::cout << x.av.data(); }

    template<class T>
    static auto print(T const & x, int)
    -> decltype(void(std::cout << x.str.data()))
    { std::cout << x.str.data(); }

    template<class T>
    static void print(T const & x, char)
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
using var_type = proto::var_or_val_to_var<T>;

using proto::t_;

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
    struct iovec {
        void  *iov_base;
        size_t iov_len;
    };

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

        brigand::wrap<buffer_list, tuple_buf> buffer_tuple;
        detail::Buffers<brigand::size<buffer_list>::value> buffers{buffer_tuple};
        uint8_t * pktptrs[brigand::size<pkt_sz_list>::value];
        detail::Sizes<default_buffer_size> sizes;
        // TODO reference
        Policy policy;

        void impl(Pkts & ... packets)
        {
            std::cout << "pktptrs.size: " << (sizeof(this->pktptrs)/sizeof(this->pktptrs[0])) << "\n";

            std::cout << "--- serialize_not_dynamic_bufs ---\n";
            this->serialize_not_dynamic_bufs(
                var_info_list_by_buffer{},
                mk_seq2<brigand::size<var_info_list_by_buffer>>{},
                packets...
            );
            std::cout << "------------------------------\n\n";

            std::cout << "--- serialize_dynamic_bufs ---\n";
            this->serialize_dynamic_bufs(
                i_<0>{},
                var_info_list_by_buffer{},
                packets...
            );
        }

        template<class... VarInfos, class... Ints>
        void serialize_not_dynamic_bufs(brigand::list<VarInfos...>, brigand::list<Ints...>, Pkts & ... pkts) {
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
        { std::cout << "-------\n(dyn) { 0x" << buffer.iov_base << " }\n"; }

        template<class... VarInfos>
        void serialize_not_dynamic_buf(std::false_type, brigand::list<VarInfos...>, iovec & buffer, Pkts & ... pkts) {
            std::cout << "-------\n";
            (void)std::initializer_list<int>{(void(
                this->serialize_type(
                    proto::buffer_category<typename VarInfos::desc_type>{}, VarInfos{}, buffer,
                    larg<VarInfos::ivar::value>(arg<VarInfos::ipacket::value>(pkts...)))
            ), 1)...};
        }

        template<class VarInfo, class Val>
        void serialize_type(proto::tags::static_buffer, VarInfo, iovec & buffer, Val & val) {
            std::cout << name(val) << " = ";
            this->print(val);
            this->serialize_pkt_sz_with_size_or_var(VarInfo{}, buffer, val);
            buffer.iov_base = static_cast<uint8_t*>(buffer.iov_base) + proto::sizeof_<desc_type_t<VarInfo>>{};
            std::cout << "\n";
        }

        template<class IPacket, class IVar, class T, class Var>
        void serialize_pkt_sz_with_size_or_var(
            var_info<IPacket, IVar, proto::types::pkt_sz<T>> vinfo, iovec & buffer, Var &
        ) {
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            this->pktptrs[pkt_idx] = static_cast<uint8_t*>(buffer.iov_base);
            std::cout << "  {" << static_cast<void*>(buffer.iov_base) << "} { pktptrs[" << pkt_idx << "] }";
        }

        template<class IPacket, class IVar, class T, class Var>
        void serialize_pkt_sz_with_size_or_var(
            var_info<IPacket, IVar, proto::types::pkt_sz_with_self<T>> vinfo, iovec & buffer, Var &
        ) {
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            this->pktptrs[pkt_idx] = static_cast<uint8_t*>(buffer.iov_base);
            std::cout << "  {" << static_cast<void*>(buffer.iov_base) << "} { pktptrs[" << pkt_idx << "] }";
        }

        template<class IPacket, class IVar, class T, std::size_t n, class Var>
        void serialize_pkt_sz_with_size_or_var(
            var_info<IPacket, IVar, detail::pkt_sz_with_size<T, n>>, iovec & buffer, Var &
        ) {
            using proto_integer = typename T::type;
            policy.serialize_static_buffer(
                static_cast<uint8_t*>(buffer.iov_base),
                proto_integer{checked_cast<typename proto_integer::type>(n)}
            );
            std::cout << " = " << n;
        }

        template<class VarInfo, class Var>
        void serialize_pkt_sz_with_size_or_var(VarInfo, iovec & buffer, Var const & var)
        {
            policy.serialize_static_buffer(static_cast<uint8_t*>(buffer.iov_base), var.x);
        }

        template<class VarInfo, class Val>
        void serialize_type(proto::tags::limited_buffer, VarInfo, iovec & buffer, Val & val) {
            std::cout << name(val) << " = ";
            this->print(val);
            std::size_t len = policy.serialize_limited_buffer(
                static_cast<uint8_t*>(buffer.iov_base),
                val.x
            );
            std::cout << " [len: " << len << "]\n";
            buffer.iov_base = static_cast<uint8_t*>(buffer.iov_base) + len;
            this->sizes.data[VarInfo::ipacket::value] += len;
            static_assert(!var_info_is_pkt_sz<VarInfo>{}, "");
        }

        template<class VarInfo, class Val>
        void serialize_type(proto::tags::view_buffer, VarInfo, iovec & buffer, Val & val) {
            std::cout << name(val) << " = ";
            this->print(val);
            auto av = policy.get_view_buffer(val.x);
            buffer.iov_base = const_cast<uint8_t *>(av.data());
            buffer.iov_len = av.size();
            this->sizes.data[VarInfo::ipacket::value] += av.size();
            static_assert(!var_info_is_pkt_sz<VarInfo>{}, "");
            std::cout << " [view: 0x" << buffer.iov_base << " | len: " << buffer.iov_len << "]\n";
        }

        template<class I, class VarInfosByBuffer>
        void serialize_dynamic_bufs(I, VarInfosByBuffer, Pkts & ... pkts) {
            using new_list = brigand::find<VarInfosByBuffer, brigand::call<var_infos_is_not_dynamic>>;
            using old_size = brigand::size<VarInfosByBuffer>;
            using new_size = brigand::size<new_list>;
            using new_index = brigand::size_t<I::value + (old_size::value - new_size::value)>;

            serialize_dynamic_buf(new_index{}, new_list{}, pkts...);
        }

        template<class I>
        void serialize_dynamic_buf(I, brigand::list<>, Pkts & ...) {
            std::cout << "--------------------------\n\n";

            this->buffers.reset_ptr(this->buffer_tuple);

            this->sizes.propagate_size();

            std::cout << "sizes: ";
            for (auto i : this->sizes.data) {
                std::cout << i << " ";
            }
            std::cout << "\n\n";

            std::cout << "--- serialize_pkt_szs ---\n";
            this->serialize_pkt_szs(
                i_<0>{},
                var_info_list_by_buffer{}
            );
            std::cout << "---------------------\n\n";

            this->policy.send(this->buffers.view());
        }


# define PROTO_NIL
#ifndef NDEBUG
# define PROTO_ENABLE_IF_DEBUG(...) __VA_ARGS__
#else
# define PROTO_ENABLE_IF_DEBUG(...)
#endif
        template<class I, class VarInfos, class... VarInfosBuffers>
        void serialize_dynamic_buf(I, brigand::list<VarInfos, VarInfosBuffers...>, Pkts & ... pkts) {
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
                    std::cout << " [size: " << av.size() << "]";
                    std::cout << "\n";
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
            std::cout << name(val) << " = ";
            print(val);
            policy.context_dynamic_buffer(f, val.x);
        }


        template<class I, class VarInfosByBuffer>
        void serialize_pkt_szs(I, VarInfosByBuffer) {
            using new_list = brigand::find<VarInfosByBuffer, brigand::call<var_infos_has_pkt_sz>>;
            using old_size = brigand::size<VarInfosByBuffer>;
            using new_size = brigand::size<new_list>;
            using new_index = brigand::size_t<I::value + (old_size::value - new_size::value)>;

            this->serialize_pkt_sz(new_index{}, new_list{});
        }

        template<class I>
        static void serialize_pkt_sz(I, brigand::list<>) {
        }

        template<
            class I,
            class VarInfos, class... VarInfosBuffers>
        void serialize_pkt_sz(I, brigand::list<VarInfos, VarInfosBuffers...>) {
            std::cout << "-------\n";
            serialize_pkt_sz_from_buffer(VarInfos{});
            serialize_pkt_szs(brigand::size_t<I::value + 1>{}, brigand::list<VarInfosBuffers...>{});
        }

        template<class... VarInfo>
        void serialize_pkt_sz_from_buffer(brigand::list<VarInfo...>) {
            (void)std::initializer_list<int>{(void(
                serialize_pkt_sz_var(VarInfo{})
            ), 1)...};
        }

        template<class VarInfo>
        static void serialize_pkt_sz_var(VarInfo) {
        }

        template<class IPacket, class IVar, class T>
        void serialize_pkt_sz_var(var_info<IPacket, IVar, proto::types::pkt_sz<T>> vinfo) {
            using is_proto_integer = std::is_same<T, brigand::wrap<T, proto::types::integer>>;
            static_assert(is_proto_integer{}, "only proto::types::integer is supported with pkt_sz");
            // TODO
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            std::cout << "pktptrs[" << pkt_idx << "] {"
              << static_cast<void*>(this->pktptrs[pkt_idx]) << "} = "
              << this->sizes.data[IPacket::value+1] << "\n";
            policy.serialize_static_buffer(
                this->pktptrs[pkt_idx],
                T{checked_cast<typename T::type>(this->sizes.data[IPacket::value+1])}
            );
            std::cout << "\n";
        }

        template<class IPacket, class IVar, class T>
        void serialize_pkt_sz_var(var_info<IPacket, IVar, proto::types::pkt_sz_with_self<T>> vinfo) {
            using is_proto_integer = std::is_same<T, brigand::wrap<T, proto::types::integer>>;
            static_assert(is_proto_integer{}, "only proto::types::integer is supported with pkt_sz_with_self");
            // TODO
            constexpr auto pkt_idx = brigand::index_of<pkt_sz_list, decltype(vinfo)>::value;
            std::cout << "pktptrs[" << pkt_idx << "] {"
              << static_cast<void*>(this->pktptrs[pkt_idx]) << "} = "
              << this->sizes.data[IPacket::value] << "\n";
            policy.serialize_static_buffer(
                this->pktptrs[pkt_idx],
                T{checked_cast<typename T::type>(this->sizes.data[IPacket::value])}
            );
            std::cout << "\n";
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
        { Printer{}.print(x.x, 1); }

        template<class T, class Derived>
        static void print(proto::var<proto::types::pkt_sz<T>, Derived>)
        { std::cout << "[pkt_sz]"; }

        template<class T, class Derived>
        static void print(proto::var<proto::types::pkt_sz_with_self<T>, Derived>)
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
#include "utils/log.hpp" //hexdump_c

struct stream_protocol_policy
{
    template<class T>
    auto serialize_static_buffer(uint8_t * p, T val)
    {
        return val.static_serialize(p);
    }

    template<class T>
    auto get_view_buffer(T val)
    {
        return val.get_view_buffer();
    }

    template<class T>
    std::size_t serialize_limited_buffer(uint8_t * p, T val)
    {
        return val.limited_serialize(p);
    }

    template<class F, class T>
    void context_dynamic_buffer(F && f, T val)
    {
        val.dynamic_serialize(f);
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
void test();

BOOST_AUTO_TEST_CASE(proto_test)
{
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

    test();
}


void test()
{
    std::cout << "\n\n======== old ========\n\n";
    test_old();
    std::cout << "\n\n======== new ========\n\n";
    test_new();
}

#include "core/RDP/sec.hpp"
#include "core/RDP/x224.hpp"

// https://github.com/jonathanpoelen/falcon.parse_number
#include <falcon/literals/integer_constant.hpp>
using namespace falcon::literals::integer_constant_literals;

namespace x224
{
    PROTO_VAR(proto::types::u8, version);
    PROTO_VAR(proto::types::u8, unknown);
    PROTO_VAR(proto::types::pkt_sz_with_self<proto::types::u16_be>, pkt_len);
    PROTO_VAR(proto::types::u8, LI);
    PROTO_VAR(proto::types::enum_u8<decltype(X224::DT_TPDU)>, type);
    PROTO_VAR(proto::types::enum_u8<decltype(X224::EOT_EOT)>, cat);

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
    struct proto_signature
    {
        proto::types::mutable_bytes av;
        proto::types::value<CryptContext&> crypt;

        using sizeof_ = proto::size_<8>;

        auto static_serialize(uint8_t * p) const
        {
            auto & signature = reinterpret_cast<uint8_t(&)[proto_signature::sizeof_{}]>(*p);
            this->crypt.val.sign(this->av.av.data(), this->av.av.size(), signature);
            this->crypt.val.decrypt(const_cast<uint8_t*>(this->av.av.data()), this->av.av.size());
            return sizeof_{};
        }
    };

    PROTO_VAR(proto::types::u32_le, flags);
    PROTO_VAR(proto::types::mutable_bytes, data);
    PROTO_VAR(proto::types::value<CryptContext&>, crypt);

    struct sec_send_pkt
    {
        proto::types::u32_le flags_;
        proto_signature sig;

        using buffer_size = proto::size_<proto_signature::sizeof_{} + proto::get_size(flags)>;
        using sizeof_ = proto::limited_size<buffer_size::value>;

        std::size_t limited_serialize(uint8_t * buf) const
        {
            std::cout << " [limited_buffer]";
            uint8_t * p = buf;
            if (this->flags_.val) {
                p += this->flags_.static_serialize(p);
            }
            if (this->flags_.val & SEC::SEC_ENCRYPT){
                p += this->sig.static_serialize(p);
            }
            return p - buf;
        }
    };

    inline std::ostream & operator <<(std::ostream & os, sec_send_pkt const &)
    {
        return os << "sec_send_pkt";
    }

    //constexpr auto sec_send = proto::desc(proto::creater<sec_send_pkt>(flags, data, crypt));
    constexpr auto sec_send = proto::desc(proto::creater<sec_send_pkt>("blahblah", flags, data, crypt));

    // TODO
    // constexpr auto sec_send2 = proto::desc(
    //     proto::filters::if_true(flags),
    //     proto::filters::if_(proto::params[flags] & SEC::SEC_ENCRYPT)
    //         [proto::creater<proto_signature>(data, crypt)]
    // );
}


void test_old() {
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
    hexdump_c(out_stream.get_data(), out_stream.get_offset());
}

#include "utils/sugar/bytes_t.hpp"
inline bool check_range(const_bytes_array p, const_bytes_array mem, char * message)
{
    if (p.size() != mem.size() || memcmp(p.data(), mem.data(), p.size())) {
        if (auto len = p.size()) {
            auto sig = p.data();
            message += std::sprintf(message, "Expected signature: \"\\x%.2x", unsigned(*sig));
            while (--len) {
                message += std::sprintf(message, "\\x%.2x", unsigned(*++sig));
            }
            message[0] = '"';
            message[1] = 0;
        }
        message[0] = 0;
        return false;
    }
    return true;
}

#define CHECK_RANGE(p, mem)                      \
    {                                            \
        char message[1024*64];                   \
        if (!check_range(p, mem, message)) {     \
            BOOST_CHECK_MESSAGE(false, message); \
        }                                        \
    }


void test_new()
{
    auto packet1 = x224::dt_tpdu_send();

    uint8_t data[10];
    CryptContext crypt;
    auto packet2 = sec::sec_send(
        sec::flags = uint32_t(~SEC::SEC_ENCRYPT),
        sec::crypt = crypt,
        sec::data = data
    );

    struct Policy : stream_protocol_policy {
        void send(iovec_view iovs) {
            BOOST_CHECK_EQUAL(iovs.size(), 1);
            CHECK_RANGE(
                make_array_view(reinterpret_cast<uint8_t const *>(iovs[0].iov_base), iovs[0].iov_len),
                cstr_array_view("\x03\x00\x00\x0b\x02\xf0\x80\xf7\xff\xff\xff")
            );
            stream_protocol_policy::send(iovs);
        }
    };

    proto::apply(Buffering2<Policy>{}, packet1, packet2);


    auto v = x224::LI = 1_c;
    proto::utils::parameters<decltype(v)> p{v};
    proto::filters::if_true(x224::LI)(p);

    proto::filters::if_(proto::params[x224::LI] & 1_c)
    [x224::LI]
    (p)
    ;
}
