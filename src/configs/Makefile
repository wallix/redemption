config_variables_configuration.hpp: \
../utils/config_spec.hpp \
../main/write_config.cpp \
../utils/apps/app_write_config.hpp
	../../`cd ../.. && MAKE_CONFIG=1 bjam -a make_config | grep -o '[^ ]*make_config'` \
		config_authid.hpp \
		config_variables_configuration.hpp \
		config_initialize.tcc \
		config_set_value.tcc
