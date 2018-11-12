#include "configs/specs/config_spec.hpp"
#include "configs/specs/config_type.hpp"
#include "configs/generators/connection_policy.hpp"

int main(int ac, char ** av)
{
    struct Writer : cfg_generators::connection_policy_writer::ConnectionPolicyWriterBase<Writer>
    {
        using base_type::base_type;

        void do_init()
        {
            base_type::do_init();
            cfg_specs::config_type_definition(this->enums);
            cfg_specs::config_spec_definition(*this);
        }
    };
    return cfg_generators::app_write_connection_policy<Writer>(ac, av);
}
