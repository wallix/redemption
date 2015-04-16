/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   stream object, used for input / output communication between
   entities
*/

#ifndef REDEMPTION_UTILS_PROGRAM_OPTION_HPP
#define REDEMPTION_UTILS_PROGRAM_OPTION_HPP

#include <vector>
#include <string>
#include <utility>
#include <stdexcept>
#include <functional>
#include <type_traits>

namespace program_options {
    struct option_value
    {
#define ACCEPT_TYPE(type, name)                              \
        private:                                             \
        static bool parse_ ## name(char const * s, type *);  \
        static bool parse_ ## name(char const * s, void * x) \
        { return parse_ ## name(s, static_cast<type*>(x)); } \
        public:                                              \
        explicit option_value(type * x) : data_(x), parser_(&parse_ ## name) {}

        ACCEPT_TYPE(int, int)
        ACCEPT_TYPE(long, long)
        ACCEPT_TYPE(long long, long_long)
        ACCEPT_TYPE(unsigned int, unsigned_int)
        ACCEPT_TYPE(unsigned long, unsigned_long)
        ACCEPT_TYPE(unsigned long long, unsigned_long_long)
        ACCEPT_TYPE(float, float)
        ACCEPT_TYPE(double, double)
        ACCEPT_TYPE(long double, long_double)
        ACCEPT_TYPE(std::string, string)
#undef ACCEPT_TYPE

        option_value() = default;

        explicit operator bool () const noexcept { return this->data_; }

        bool parse(char const * s) const { return this->parser_(s, this->data_); }

    private:
        void * data_ = nullptr;
        bool (*parser_)(char const *, void *) = nullptr;
    };


    struct option_description
    {
        template<class T, class = typename std::enable_if<(!std::is_const<T>::value)>::type>
        option_description(char short_name, std::string long_name, T * value, std::string desc)
        : short_name_(short_name)
        , long_name_(std::move(long_name))
        , value_(value)
        , desc_(std::move(desc))
        {}

        template<class T, class = typename std::enable_if<(!std::is_const<T>::value)>::type>
        option_description(char short_name, T * value, std::string desc)
        : short_name_(short_name)
        , value_(value)
        , desc_(std::move(desc))
        {}

        template<class T, class = typename std::enable_if<(!std::is_const<T>::value)>::type>
        option_description(std::string long_name, T * value, std::string desc)
        : long_name_(std::move(long_name))
        , value_(value)
        , desc_(std::move(desc))
        {}

        option_description(char short_name, std::string long_name, std::string desc)
        : short_name_(short_name)
        , long_name_(std::move(long_name))
        , desc_(std::move(desc))
        {}

        option_description(char short_name, std::string desc)
        : short_name_(short_name)
        , desc_(std::move(desc))
        {}

        option_description(std::string long_name, std::string desc)
        : long_name_(std::move(long_name))
        , desc_(std::move(desc))
        {}

        bool has_value() const noexcept { return bool(this->value_); }
        bool has_short_name() const noexcept { return this->short_name_; }
        bool has_long_name() const noexcept { return !this->long_name_.empty(); }
        char short_name() const noexcept { return this->short_name_; }
        std::string const & long_name() const noexcept { return this->long_name_; }
        std::string const & description() const noexcept { return this->desc_; }
        option_value const & value() const { return this->value_; }

    private:
        char short_name_ = 0;
        std::string long_name_;
        option_value value_;
        std::string desc_;
    };

    struct options_description
    {
        using options_list = std::vector<option_description>;
        using const_iterator = options_list::const_iterator;
        using iterator = const_iterator;

        options_description(options_list options)
        : options_(std::move(options))
        {}

        void add(option_description option)
        { options_.push_back(std::move(option)); }

        iterator begin() const { return this->options_.begin(); }
        iterator end() const { return this->options_.end(); }

    private:
        options_list options_;
    };

    std::ostream & operator<<(std::ostream & os, options_description const & desc);

    struct variables_map
    {
        size_t count(char name) const;
        size_t count(char const * name) const;
        size_t count(std::string const & name) const;

    private:
        std::vector<std::reference_wrapper<option_description const>> ref_descriptions;
        friend variables_map parse_command_line(int ac, char ** av, options_description const & desc);
    };


    struct unknow_option : std::runtime_error
    {
        unknow_option();
        unknow_option(char opt);
        unknow_option(std::string opt);
    };

    struct invalid_option_value : std::runtime_error
    {
        invalid_option_value(option_description const & option, char const * arg);
    };


    struct invalid_command_line_syntax : std::runtime_error
    {
        invalid_command_line_syntax(option_description const & option);
        invalid_command_line_syntax();
    };

    variables_map parse_command_line(int ac, char ** av, options_description const & desc);
}

#endif
