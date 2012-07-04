#include "init.hpp"
#include "get_type.hpp"

int parse_command_line(WrmRecoderOption& opt, int argc, char** argv)
{
    opt.parse_command_line(argc, argv);

    if (opt.options.count("version")) {
        std::cout << argv[0] << ' ' << opt.version() << '\n';
    }

    if (opt.options.count("help")) {
        std::cout << opt.desc << std::endl;
        return 30;
    }

    RecorderOptionError::enum_t error = opt.notify_options();
    if (error){
        std::cerr
        << RecorderOptionError::get_cstr(error) << '\n'
        << opt.desc << std::endl
        ;
        return 30 + error;
    }
    return 0;
}

int set_iotype(WrmRecoderOption& opt,
               InputType::enum_t& itype, OutputType::enum_t& otype)
{
    otype = get_output_type(opt);
    if (otype == OutputType::NOT_FOUND){
        std::cerr
        << "Incorrect output-type, "
        << opt.desc.find("output-type", false).description() << '\n';
        return 100;
    }

    itype = get_input_type(opt);
    if (itype == InputType::NOT_FOUND){
        std::cerr
        << "Incorrect input-type, "
        << opt.desc.find("input-type", false).description() << '\n';
        return 110;
    }
    return 0;
}

int init_opt_and_iotype(WrmRecoderOption& opt, int argc, char** argv,
                        InputType::enum_t& itype, OutputType::enum_t& otype)
{
    int error = parse_command_line(opt, argc, argv);
    if (error)
        return error;
    error = set_iotype(opt, itype, otype);
    if (error)
        return error;

    RecorderOptionError::enum_t eerror = opt.normalize_options();
    if (eerror){
        std::cerr
        << RecorderOptionError::get_cstr(eerror) << '\n'
        << opt.desc << std::endl
        ;
        error = 150 + eerror;
    }
    return error;
}