#include "configs/generators/python_spec.hpp"

int main(int ac, char ** av) {
    struct Writer : python_spec_writer::PythonSpecWriterBase<Writer> {
        Writer() {
            config_spec::config_spec_definition(*this);
        };
    };
    return app_write_python_spec<Writer>(ac, av);
}
