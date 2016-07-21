#include "configs/specs/config_spec.hpp"
#include "configs/specs/config_type.hpp"
#include "configs/generators/python_spec.hpp"

int main(int ac, char ** av)
{
    struct Writer : cfg_generators::python_spec_writer::PythonSpecWriterBase<Writer>
    {
        using base_type::base_type;

        void do_init()
        {
            cfg_specs::config_type_definition(this->enums);
            cfg_specs::config_spec_definition(*this);
        }
    };
    return cfg_generators::app_write_python_spec<Writer>(ac, av);
}
