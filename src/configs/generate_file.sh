#!/bin/sh

bin="$(cd ../.. && NO_MAKE_CONFIG=1 bjam -a make_cpp_config)"
err=$?
[ $err = 0 ] && ../../"$(echo "$bin" | grep -o '[^ ]*make_cpp_config')" \
  config_authid.hpp \
  config_variables_configuration.hpp \
  config_initialize.tcc \
  config_set_value.tcc

err2=$?

echo "$bin"

[ $err2 = 0 ] && exit $err || exit $err2
