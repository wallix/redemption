#include "write_python_spec.hpp"
#include "utils/apps/app_write_python_spec.hpp"

int write_python_spec(char const * application_name, char const * output) {
    char const * av[] = {application_name, output, nullptr};
    int ac = static_cast<int>(sizeof(av) / sizeof(av[0]) - 1);
    struct Writer : python_spec_writer::PythonSpecWriterBase<Writer> {
        Writer() {
            config_spec::config_spec_definition(*this);
        };
    };
    return app_write_python_spec<Writer>(ac, av);
}
