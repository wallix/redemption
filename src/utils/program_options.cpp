#include "program_options.hpp"

#include <limits>
#include <ostream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>

#include <cerrno>
#include <cassert>


namespace program_options {

bool option_value::parse_int(char const * s, int * x) {
    char * p;
    unsigned long const n = std::strtol(s, &p, 10);
    if (n > long(std::numeric_limits<int>::max())) {
        return false;
    }
    *x = n;
    return p != s && !*p;
}

bool option_value::parse_unsigned_int(char const * s, unsigned int * x) {
    char * p;
    unsigned long const n = std::strtoul(s, &p, 10);
    if (n > long(std::numeric_limits<unsigned>::max())) {
        return false;
    }
    *x = n;
    return p != s && !*p;
}

bool option_value::parse_long(char const * s, long * x) {
    char * p;
    *x = std::strtol(s, &p, 10);
    return p != s && !*p && errno != ERANGE;
}

bool option_value::parse_long_long(char const * s, long long * x) {
    char * p;
    *x = std::strtoll(s, &p, 10);
    return p != s && !*p && errno != ERANGE;
}

bool option_value::parse_unsigned_long(char const * s, unsigned long * x) {
    char * p;
    *x = std::strtoul(s, &p, 10);
    return p != s && !*p && errno != ERANGE;
}

bool option_value::parse_unsigned_long_long(char const * s, unsigned long long * x) {
    char * p;
    *x = std::strtoull(s, &p, 10);
    return p != s && !*p && errno != ERANGE;
}

bool option_value::parse_float(char const * s, float * x) {
    char * p;
    *x = std::strtof(s, &p);
    return p != s && !*p && errno != ERANGE;
}

bool option_value::parse_double(char const * s, double * x) {
    char * p;
    *x = std::strtod(s, &p);
    return p != s && !*p && errno != ERANGE;
}

bool option_value::parse_long_double(char const * s, long double * x) {
    char * p;
    *x = std::strtold(s, &p);
    return p != s && !*p && errno != ERANGE;
}

bool option_value::parse_string(char const * s, std::string * x) {
    *x = s;
    return true;
}

namespace {
    template<bool Print, class Fn>
    void output_desc(std::ostream & os, options_description const & desc, Fn f) {
        for (option_description const & opt : desc) {
            size_t len = opt.long_name().size();
            if (opt.has_short_name()) {
                if (Print) os << "  -" << opt.short_name();
                len += 1;
                if (opt.has_long_name()) {
                    if (Print) os << " [ --" << opt.long_name() << " ]";
                    len += 7;
                }
            }
            else if (opt.has_long_name()) {
                if (Print) os << "  --" << opt.long_name();
                len += 2;
            }

            if (opt.has_value()) {
                if (Print) os << " arg";
                len += 4;
            }

            f(len);

            if (Print) os << opt.description() << '\n';
        }
    }
}

std::ostream & operator<<(std::ostream & os, options_description const & desc) {
    size_t max_len = 0;
    output_desc<false>(os, desc, [&](size_t len) { max_len = std::max(max_len, len); });
    output_desc<true >(os, desc, [&](size_t len) { os << std::setw(max_len - len + 2) << ""; });
    return os;
}


size_t variables_map::count(const std::string& name) const
{
    for (option_description const & desc : ref_descriptions) {
        if (desc.long_name() == name) {
            return 1;
        }
    }
    return 0;
}

size_t variables_map::count(const char* name) const
{
    for (option_description const & desc : ref_descriptions) {
        if (desc.long_name() == name) {
            return 1;
        }
    }
    return 0;
}

size_t variables_map::count(char name) const
{
    for (option_description const & desc : ref_descriptions) {
        if (desc.short_name() == name) {
            return 1;
        }
    }
    return 0;
}


unknow_option::unknow_option()
: std::runtime_error("unrecognised empty option")
{}

unknow_option::unknow_option(char opt)
: std::runtime_error(std::move(std::string("unrecognised option '-") + opt + '\''))
{}

unknow_option::unknow_option(std::string opt)
: std::runtime_error(std::move(std::string("unrecognised option '--") + opt + '\''))
{}


invalid_option_value::invalid_option_value(option_description const & option, char const * arg)
: std::runtime_error([&]{
    std::string s = "the argument ('";
    s += arg;
    s += "') for option '-";
    if (option.has_long_name()) {
        s += '-';
        s += option.long_name();
    }
    else {
        s += option.short_name();
    }
    s += "' is invalid";
    return s;
}())
{}


invalid_command_line_syntax::invalid_command_line_syntax(option_description const & option)
: std::runtime_error([&]{
    std::string s = "the required argument for option '";
    if (option.has_long_name()) {
        s += '-';
        s += option.long_name();
    }
    else {
        s += option.short_name();
    }
    s += "' is missing";
    return s;
}())
{}

invalid_command_line_syntax::invalid_command_line_syntax()
: runtime_error("invalid syntax")
{}


namespace {

option_description const & get_option_description(options_description const & desc, char opt) {
    if (!opt) {
        throw unknow_option();
    }
    auto pred = [&](option_description const & option) {
        return option.short_name() == opt;
    };
    auto it = std::find_if(desc.begin(), desc.end(), pred);
    if (it == desc.end()) {
        throw unknow_option(opt);
    }
    assert(std::find_if(it+1, desc.end(), pred) == desc.end());
    return *it;
}

option_description const & get_option_description(options_description const & desc, char const *opt) {
    if (!*opt) {
        throw unknow_option();
    }
    auto pred = [&](option_description const & option) {
        return option.long_name() == opt;
    };
    auto it = std::find_if(desc.begin(), desc.end(), pred);
    if (it == desc.end()) {
        throw unknow_option(opt);
    }
    assert(std::find_if(it+1, desc.end(), pred) == desc.end());
    return *it;
}

}

variables_map parse_command_line(int ac, char** av, const options_description & desc)
{
    variables_map variables;

    option_description const * option = nullptr;

    for (int i = 1; i < ac; ++i) {
        char const * arg = av[i];
        if (option) {
            if (!option->value().parse(arg)) {
                throw invalid_option_value(*option, arg);
            }
            variables.ref_descriptions.push_back(*option);
            option = nullptr;
        }
        else if (arg[0] == '-') {
            if (arg[1] == '-') {
                option = &get_option_description(desc, arg + 2);
                if (!option->has_value()) {
                    variables.ref_descriptions.push_back(*option);
                    option = nullptr;
                }
            }
            else {
                ++arg;
                do {
                    option = &get_option_description(desc, arg[0]);
                    if (option->has_value()) {
                        if (arg[1]) {
                            if (!option->value().parse(arg+1)) {
                                throw invalid_option_value(*option, arg);
                            }
                            variables.ref_descriptions.push_back(*option);
                            option = nullptr;
                        }
                        break;
                    }

                    variables.ref_descriptions.push_back(*option);
                    ++arg;
                    option = nullptr;
                } while (*arg);
            }
        }
        else {
            throw invalid_command_line_syntax();
        }
    }

    if (option) {
        throw invalid_command_line_syntax(*option);
    }

    return variables;
}

}
