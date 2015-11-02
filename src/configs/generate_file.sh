#!/bin/sh

bin="$(cd ../.. && NO_MAKE_CONFIG=1 bjam -a make_cpp_config)"
err=$?
[ $err = 0 ] && ../../"$(echo "$bin" | grep -o '[^ ]*make_cpp_config')" \
  autogen/configs/autogen/authid.hpp \
  autogen/configs/autogen/variables_configuration.hpp \
  autogen/configs/autogen/set_value.tcc

err2=$?

echo "$bin"

[ $err2 = 0 ] && exit $err || exit $err2
