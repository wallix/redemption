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
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#pragma once

#ifdef IN_IDE_PARSER

# define RED_DATA_TEST_CASE(test_name, dataset, varname)                        \
    inline auto const& test_name##_type_element_() {                            \
        using std::begin; return *begin(dataset);                               \
    }                                                                           \
    class test_name {                                                           \
        public: void operator()(decltype(test_name##_type_element_()) varname); \
    };                                                                          \
    void test_name::operator()(decltype(test_name##_type_element_()) varname)

namespace redemption_unit_test__
{
    struct AnyType
    {
        template<class T>
        operator T ();
    };
}

#define RED_BIND_DATA_TEST_CASE(test_name, dataset, ...) \
    class test_name {                                    \
        redemption_unit_test__::AnyType __VA_ARGS__;     \
        void void_() { (void)dataset; }                  \
        public: void operator()();                       \
    };                                                   \
    void test_name::operator()()

#else

#include "impl/data_test_case_impl.hpp"
#include "utils/sugar/array.hpp"

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/comparison/greater.hpp>
#include <boost/preprocessor/list/adt.hpp>
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/variadic/to_list.hpp>

#include <tuple>

namespace redemption_unit_test__
{
    template<class T>
    struct wrap_data_test_elem
    {
        std::remove_reference_t<T> t;
        char const* prefix_message;
    };

    template<class It>
    struct wrap_data_test_iter
    {
        It it;
        char const* prefix_message;

        auto operator*() const
        {
            return wrap_data_test_elem<decltype(*it)>{*it, prefix_message};
        }

        wrap_data_test_iter& operator++()
        {
            ++it;
            return *this;
        }
    };

    template<class T>
    struct wrap_data_test
    {
        T a;
        char const* prefix_message;

        using internal_iterator = decltype(utils::begin(std::declval<T const&>()));
        using internal_value = wrap_data_test_elem<decltype(*std::declval<internal_iterator>())>;
        using const_iterator = wrap_data_test_iter<internal_iterator>;

        using value_type = internal_value;

        const_iterator begin() const { return {utils::begin(a), prefix_message}; }
        const_iterator end() const { return {utils::end(a), prefix_message}; }

        std::size_t size() const { return a.size(); }
    };

    template <class T>
    wrap_data_test(T, char const*) -> wrap_data_test<T>;

    template<class T>
    std::ostream& operator<<(std::ostream& out, wrap_data_test_elem<T> const& e)
    {
        out << e.prefix_message;
        std::apply([&](auto const& x, auto const&... xs){
            auto print = [&](auto const& y){ out << ", " << y; };
            out << x;
            (print(xs), ...);
        }, e.t);
        out << ")";
        return out;
    }
}


#define RED_BIND_DATA_TEST_CASE_VARIABLE(r, data, elem) \
    BOOST_PP_COMMA_IF(BOOST_PP_GREATER(r, 2))           \
    BOOST_PP_CAT(T, r) const& elem

#define RED_BIND_DATA_TEST_CASE_TYPE_TEMPLATE(r, data, elem) \
    BOOST_PP_COMMA_IF(BOOST_PP_GREATER(r, 2))                \
    class BOOST_PP_CAT(T, r)


#define RED_BIND_DATA_TEST_CASE_I(test_name, dataset, names_list, strvars) \
    RED_BIND_DATA_TEST_CASE_II(test_name, dataset, names_list, "[" strvars "] = (")

#define RED_BIND_DATA_TEST_CASE_II(                           \
    test_name, dataset, names_list, prefix_message            \
)                                                             \
    RED_BIND_DATA_TEST_CASE_PROTOTYPED_I(                     \
        (template<BOOST_PP_LIST_FOR_EACH(                     \
            RED_BIND_DATA_TEST_CASE_TYPE_TEMPLATE,            \
            BOOST_PP_NIL, names_list                          \
        )>                                                    \
        void RedBinding__##test_name (BOOST_PP_LIST_FOR_EACH( \
            RED_BIND_DATA_TEST_CASE_VARIABLE,                 \
            BOOST_PP_NIL, names_list                          \
        ))),                                                  \
        test_name, dataset, prefix_message)

#define RED_BIND_DATA_TEST_CASE_TO_PROTOTYPE(...) __VA_ARGS__

#define RED_BIND_DATA_TEST_CASE_PROTOTYPED_I(prototype, test_name, dataset, prefix_message) \
    RED_BIND_DATA_TEST_CASE_PROTOTYPED_II(prototype, test_name, dataset, prefix_message)

#define RED_BIND_DATA_TEST_CASE_PROTOTYPED_II(      \
    prototype, test_name, dataset, prefix_message   \
)                                                   \
    RED_BIND_DATA_TEST_CASE_TO_PROTOTYPE prototype; \
    RED_DATA_TEST_CASE(                             \
        test_name,                                  \
        (redemption_unit_test__::wrap_data_test{    \
            dataset, prefix_message}),              \
        datas                                       \
    ) {                                             \
        std::apply([&](auto&&... xs){               \
            RedBinding__##test_name (xs...);        \
        }, datas.t);                                \
    }                                               \
    RED_BIND_DATA_TEST_CASE_TO_PROTOTYPE prototype

#define RED_BIND_DATA_TEST_CASE(test_name, dataset, ...) \
    RED_BIND_DATA_TEST_CASE_I(                           \
        test_name, dataset,                              \
        BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__),          \
        #__VA_ARGS__)

#endif
