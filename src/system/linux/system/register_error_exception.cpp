#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
// #define BOOST_TEST_ALTERNATIVE_INIT_API

#include <boost/test/unit_test_monitor.hpp>

#include <string>
#include <stdexcept>

#include "core/error.hpp"
#include "cxx/compiler_version.hpp"
#include "cxx/diagnostic.hpp"
#include "utils/sugar/algostring.hpp"

namespace
{
    const std::string prefix_msg_error = "Exception of type 'Error': "; /*NOLINT*/
} // namespace

namespace redemption_unit_test__
{
    struct register_exception
    {
        register_exception()
        {
            REDEMPTION_DIAGNOSTIC_PUSH
            REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wzero-as-null-pointer-constant")
            #if REDEMPTION_COMP_CLANG_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
                REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wzero-as-null-pointer-constant")
            #endif
            boost::unit_test::unit_test_monitor.register_exception_translator<Error>(+[](Error const & e){
                if (e.errnum) {
                    throw std::runtime_error{str_concat(
                        prefix_msg_error, e.errmsg(), ", errno=", std::to_string(e.errnum)
                    )};
                }
                throw std::runtime_error{prefix_msg_error + e.errmsg()};
            });
            REDEMPTION_DIAGNOSTIC_POP
        }
    };
    static register_exception Init; /*NOLINT*/
} // namespace redemption_unit_test__
