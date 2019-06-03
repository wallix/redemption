#include <ostream>
#include "core/app_path.hpp"
// force dependency in bjam (comments is ok)
// #include "configs/config.hpp"

int main()
{
    std::ostream{nullptr}
      <<
        #include "configs/autogen/str_python_spec.hpp"
      <<
        #include "configs/autogen/str_ini.hpp"
    ;
}
