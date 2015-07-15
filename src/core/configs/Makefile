variables_configuration.hpp: ../../utils/config_spec.hpp ../../main/write_config.cpp
	../../../`cd ../../.. && MAKE_CONFIG=1 bjam -a make_config | grep -o '[^ ]*make_config'` \
		authid.hpp \
		variables_configuration.hpp \
		config_initialize.tcc \
		config_set_value.tcc
