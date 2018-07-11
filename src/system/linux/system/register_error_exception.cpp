#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
// #define BOOST_TEST_ALTERNATIVE_INIT_API

#include <boost/test/unit_test_monitor.hpp>

#include <string>
#include <stdexcept>

#include "core/error.hpp"
#include "cxx/cxx.hpp"
#include "cxx/diagnostic.hpp"


namespace redemption_unit_test__
{
    struct register_exception
    {
        register_exception()
        {
            REDEMPTION_DIAGNOSTIC_PUSH
            REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wzero-as-null-pointer-constant")
            #if REDEMPTION_COMP_CLANG >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
                REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wzero-as-null-pointer-constant")
            #endif
            boost::unit_test::unit_test_monitor.register_exception_translator<Error>(+[](Error const & e){
                std::string s = "Exception of type 'Error': "; s += e.errmsg();
                throw std::runtime_error{std::move(s)};
            });
            REDEMPTION_DIAGNOSTIC_POP
        }
    };
    static register_exception Init;
} // namespace redemption_unit_test__
