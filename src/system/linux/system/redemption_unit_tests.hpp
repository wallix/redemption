#include <boost/test/auto_unit_test.hpp>

// fixed link error (API changed)
#ifdef __clang__
namespace boost { namespace unit_test { namespace ut_detail {
    std::string normalize_test_case_name(const_string name) {
        return ( name[0] == '&' ? std::string(name.begin()+1, name.size()-1) : std::string(name.begin(), name.size() ));
    }
}}}
#endif
