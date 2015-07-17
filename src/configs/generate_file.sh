#!/bin/sh

bin="$(cd ../.. && NO_MAKE_CONFIG=1 bjam -a make_config)"
err=$?
[ $? = 0 ] && ../../"$(echo "$bin" | grep -o '[^ ]*make_config')" \
  config_authid.hpp \
  config_variables_configuration.hpp \
  config_initialize.tcc \
  config_set_value.tcc

echo "$bin"

exit $err
