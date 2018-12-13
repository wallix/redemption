/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#pragma once

#include "configs/attributes/spec.hpp"
#include "configs/generators/utils/spec_writer.hpp"
#include "configs/generators/utils/write_template.hpp"
#include "configs/enumeration.hpp"



#include "configs/generators/python_spec.hpp"
#include "utils/sugar/algostring.hpp"

#include <array>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <cerrno>
#include <cstring>


namespace cfg_generators {

namespace connection_policy_writer {

using namespace cfg_attributes;

template<class Inherit>
struct ConnectionPolicyWriterBase : python_spec_writer::PythonSpecWriterBase<Inherit, connpolicy::name>
{
    using base_type = ConnectionPolicyWriterBase;

    using filename_map_t = std::array<std::pair<std::string, char const*>, 2>;

    ConnectionPolicyWriterBase(filename_map_t filename_map, char const* sesman_map_filename)
    : python_spec_writer::PythonSpecWriterBase<Inherit, connpolicy::name>(sesman_map_filename)
    , filename_map(std::move(filename_map))
    {}

    // used by python_spec_writer
    static io_prefix_lines comment(char const * s)
    {
        return io_prefix_lines{s, "# ", "", 0};
    }

    template<class Pack>
    void do_member(std::string const & section_name, Pack const& infos)
    {
        if constexpr (is_convertible_v<Pack, connection_policy_t>) {
            auto type = get_type<spec::type_>(infos);
            std::string const& member_name = get_name<connpolicy::name>(infos);

            this->write_description(type, infos);
            this->inherit().write_type_info(type);
            this->write_enumeration_value_description(type, infos);

            auto& connpolicy = get_elem<connection_policy_t>(infos);

            using attr1_t = spec::internal::attr;
            using attr2_t = connpolicy::internal::attr;
            auto attr1 = value_or<spec_attr_t>(infos, spec_attr_t{attr1_t::no_ini_no_gui}).value;
            auto attr2 = connpolicy.spec;

            if (bool(attr1 & attr1_t::advanced_in_gui)
             || bool(attr2 & attr2_t::advanced_in_connpolicy))
                this->out() << "#_advanced\n";
            if (bool(attr1 & attr1_t::hex_in_gui)
             || bool(attr2 & attr2_t::hex_in_connpolicy))
                this->out() << "#_hex\n";

            this->out() << member_name << " = ";
            this->inherit().write_type(type, get_default<connpolicy::default_>(type, infos));
            this->out() << "\n\n";

            auto&& sections = this->file_map[connpolicy.file];
            auto const& section = value_or<connpolicy::section>(
                infos, connpolicy::section{section_name.c_str()});

            if (this->section_names.emplace(section.name).second) {
                this->ordered_section.emplace_back(section.name);
            }

            Section& sec = sections[section.name];

            auto& sesman_name = get_name<sesman::name>(infos);

            sec.contains += this->out_member_.str();
            update_sesman_contains(sec.sesman_contains, sesman_name, member_name);

            if constexpr (is_convertible_v<Pack, sesman::deprecated_names>) {
                for (auto&& old_name : get_elem<sesman::deprecated_names>(infos).names) {
                    update_sesman_contains(sec.sesman_contains, old_name, member_name, " # Deprecated, for compatibility only.");
                }
            }

            this->out_member_.str("");
        }
    }

    static void update_sesman_contains(
        std::string& s,
        std::string const sesman_name,
        std::string const connpolicy_name,
        char const* extra = "")
    {
        str_append(s, "    u'", sesman_name, "': '", connpolicy_name, "',", extra, '\n');
    }

    void do_init()
    {
    }

    void do_finish()
    {
        auto& out_sesman = this->out_file_;

        out_sesman <<
          "#!/usr/bin/python -O\n"
          "# -*- coding: utf-8 -*-\n\n"
          "cp_spec = {\n"
        ;

        for (auto const& [cat, filename] : this->filename_map) {
            auto file_it = file_map.find(cat);
            if (file_it == file_map.end()) {
                this->errorstring += "Config: unknown ";
                this->errorstring += cat;
                this->errorstring += " for connpolicy";
                return ;
            }

            std::ofstream out_spec(filename);
            out_spec << R"g([general]

# Secondary login Transformation rule
# ${LOGIN} will be replaced by login
# ${DOMAIN} (optional) will be replaced by domain if it exists.
# Empty value means no transformation rule.
transformation_rule = string(default='')

# Account Mapping password retriever
# Transformation to apply to find the correct account.
# ${USER} will be replaced by the user's login.
# ${DOMAIN} will be replaced by the user's domain (in case of LDAP mapping).
# ${USER_DOMAIN} will be replaced by the user's login + "@" + user's domain (or just user's login if there's no domain).
# ${GROUP} will be replaced by the authorization's user group.
# ${DEVICE} will be replaced by the device's name.
# A regular expression is allowed to transform a variable, with the syntax: ${USER:/regex/replacement}, groups can be captured with parentheses and used with \1, \2, ...
# For example to replace leading "A" by "B" in the username: ${USER:/^A/B}
# Empty value means no transformation rule.
vault_transformation_rule = string(default='')


)g";
            auto& section_map = file_it->second;
            for (auto& section_name : this->ordered_section) {
                auto section_it = section_map.find(section_name);
                if (section_it != section_map.end()) {
                    out_spec << "[" << section_name << "]\n\n" << section_it->second.contains;
                    out_sesman << "  '" << section_name << "': {\n" << section_it->second.sesman_contains;
                    out_sesman << "  },\n";
                }
            }

            if (!out_spec) {
                this->errorstring = filename;
                return;
            }
            out_spec.close();
        }

        out_sesman << "}\n";
    }

private:
    struct Section
    {
        std::string contains;
        std::string sesman_contains;
    };
    using data_by_section_t = std::unordered_map<std::string, Section>;
    std::unordered_map<std::string, data_by_section_t> file_map;
    std::vector<std::string> ordered_section;
    std::unordered_set<std::string> section_names;
    filename_map_t filename_map;

public:
    std::string errorstring;
};



template<class Inherit, class AttributeName = spec::name>
struct JsonSpecWriterBase : ConfigSpecWriterBase<Inherit, AttributeName>
{
    using base_type = JsonSpecWriterBase;

    std::ofstream out_file_;
    std::ostringstream out_member_;

    std::ostream & out() { return this->out_member_; }

    JsonSpecWriterBase(char const * filename)
    : out_file_(filename)
    {}

    void do_init()
    {
        this->out_file_ <<
            "#include \"config_variant.hpp\"\n\n"
            "\"## Python spec file for RDP proxy.\\n\\n\\n\"\n"
        ;
    }

    void do_stop_section(std::string const & section_name)
    {
        auto str = this->out_member_.str();
        if (!str.empty()) {
            if (!section_name.empty()) {
                this->out_file_ << "\"[" << section_name << "]\\n\\n\"\n\n";
            }
            this->out_file_ << str;
            this->out_member_.str("");
        }
    }

    template<class Pack>
    void do_member(std::string const & /*section_name*/, Pack const & infos)
    {
        if constexpr (is_convertible_v<Pack, spec_attr_t>) {
            do_member_impl(infos, get_type<spec::type_>(infos), get_name<spec::name>(infos));
        }
    }

    template<class Type, class Pack>
    void do_member_impl(Pack const & infos, Type type, std::string const & member_name)
    {
        this->write_description(type, infos);
        this->inherit().write_type_info(type);
        this->write_enumeration_value_description(type, infos);

        using attr_t = spec::internal::attr;
        auto attr = get_elem<spec_attr_t>(infos).value;

        if (bool(attr & attr_t::iptables_in_gui)) this->out() << "\"#_iptables\\n\"\n";
        if (bool(attr & attr_t::advanced_in_gui)) this->out() << "\"#_advanced\\n\"\n";
        if (bool(attr & attr_t::hidden_in_gui))   this->out() << "\"#_hidden\\n\"\n";
        if (bool(attr & attr_t::hex_in_gui))      this->out() << "\"#_hex\\n\"\n";
        if (bool(attr & attr_t::password_in_gui)) this->out() << "\"#_password\\n\"\n";

        this->out() << "\"" << member_name << " = ";
        this->inherit().write_type(type, get_default(type, infos));
        this->out() << "\\n\\n\"\n\n";
    }


    struct exprio {
        const char * value;
        friend std::ostream & operator << (std::ostream & os, exprio const & eio) {
            return os << "\" << (" << eio.value << ") << \"";
        }
    };
    static exprio quoted2(cpp::expr e) { return {e.value}; }
    template<class T> static io_quoted2 quoted2(T const & s) { return s; }
    template<class T> static char const * quoted2(types::list<T> const &) { return ""; }


    template<class T>
    static char const * get_string(T const & s) { return s.c_str(); }


    static char const * stringize_integral(bool x) = delete;
    static char const * stringize_integral(types::integer_base) { return "0"; }
    static char const * stringize_integral(types::u16) { return "0"; }
    static char const * stringize_integral(types::u32) { return "0"; }
    static char const * stringize_integral(types::u64) { return "0"; }
    template<class T> static T const & stringize_integral(T const & x) { return x; }

    template<class Int, long min, long max, class T>
    static T stringize_integral(types::range<Int, min, max>)
    {
        static_assert(!min, "unspecified value but 'min' isn't 0");
        return {};
    }

    static exprio stringize_integral(cpp::expr e)
    {
        return {e.value};
    }


    static std::string stringize_bool(bool x)
    {
        return bool(x) ? "true" : "false";
    }

    static exprio stringize_bool(cpp::expr e)
    {
        return {e.value};
    }


    static io_prefix_lines comment(char const * s)
    {
        return io_prefix_lines{s, "", "\n", 0};
    }

    template<class T, class Pack>
    void write_description(type_<T>, Pack const & pack)
    {
        this->out() << "    \"description\": \"";
        auto& d = get_desc(pack);
        if (!d.empty()) {
            this->out() << this->inherit().comment(d.c_str());
        }
        else if constexpr (std::is_enum_v<T>) {
            apply_enumeration_for<T>(this->enums, [this](auto const & e) {
                if (e.desc) {
                    this->out() << this->inherit().comment(e.desc);
                }
            });
        }
        this->out() << "\"\n";
    }


    template<class T>
    void write_type_info(type_<T>)
    {}

    void write_type_info(type_<std::chrono::hours>)
    { this->out() << this->inherit().comment("(is in hour)"); }

    void write_type_info(type_<std::chrono::minutes>)
    { this->out() << this->inherit().comment("(is in minute)"); }

    void write_type_info(type_<std::chrono::seconds>)
    { this->out() << this->inherit().comment("(is in second)"); }

    void write_type_info(type_<std::chrono::milliseconds>)
    { this->out() << this->inherit().comment("(is in millisecond)"); }

    template<class T, class Ratio>
    void write_type_info(type_<std::chrono::duration<T, Ratio>>)
    {
        auto prefixes = this->inherit().comment("");
        this->out() << prefixes.prefix << "(is in " << Ratio::num << "/" << Ratio::den << " second)" << prefixes.suffix << "\n";
    }

    template<class T, class Ratio, long min, long max>
    void write_type_info(type_<types::range<std::chrono::duration<T, Ratio>, min, max>>)
    { write_type_info(type_<std::chrono::duration<T, Ratio>>{}); }


    template<class T, class V>
    void write_value_(T const & name, V const & v, char const * prefix)
    {
        auto prefixes = this->inherit().comment("");
        this->out() << prefixes.prefix << "  " << name;
        if (v.desc) {
            this->out() << ": ";
            if (prefix) {
                this->out() << prefix << " ";
            }
            this->out() << v.desc;
        }
        else if (std::is_integral<T>::value) {
            this->out() << ": " << io_replace(v.name, '_', ' ');
        }
        this->out() << prefixes.suffix << "\n";
    }

    void write_desc_value(type_enumeration const & e, char const * prefix)
    {
        if (e.is_string_parser) {
            if (std::none_of(begin(e.values), end(e.values), [](type_enumeration::Value const & v) {
                return v.desc;
            })) {
                return ;
            }
        }

        unsigned d = 0;
        bool const is_autoinc = e.flag == type_enumeration::autoincrement;
        for (type_enumeration::Value const & v : e.values) {
            if (e.is_string_parser) {
                this->write_value_((v.alias ? v.alias : v.name), v, prefix);
            }
            else {
                this->write_value_((is_autoinc ? d : (1 << d >> 1)), v, prefix);
            }
            ++d;
        }

        if (type_enumeration::flags == e.flag) {
            this->out() << this->inherit().comment("(note: values can be added (everyone: 1+2+4=7, mute: 0))");
        }
    }

    void write_desc_value(type_enumeration_set const & e, char const * prefix)
    {
        for (type_enumeration_set::Value const & v : e.values) {
            this->write_value_(v.val, v, prefix);
        }
    }

    template<class T, class Pack>
    void write_enumeration_value_description(type_<T>, Pack const & pack)
    {
        if constexpr (std::is_enum_v<T>) {
            apply_enumeration_for<T>(this->enums, [this, &pack](auto const & e) {
                this->write_desc_value(e, value_or<prefix_value>(pack, prefix_value{}).value);
                if (e.info) {
                    this->out() << this->inherit().comment(e.info);
                }
            });
        }
        else {
            static_assert(!is_convertible_v<Pack, prefix_value>, "prefix_value only with enums type");
        }
    }

    template<class T>
    void write_type(type_<bool>, T x)
    {
        this->out() <<
            "    \"type\": \"bool\",\n"
            "    \"default\": " << stringize_bool(x) << ",\n"
        ;
    }

    template<class T>
    void write_type(type_<std::string>, T const & s)
    {
        this->out() <<
            "    \"type\": \"string\",\n"
            "    \"default\": " << quoted2(s) << ",\n"
        ;
    }

    template<class Int, class T>
    std::enable_if_t<
        std::is_base_of<types::integer_base, Int>::value
        or
        std::is_integral<Int>::value
    >
    write_type(type_<Int>, T i)
    {
        this->out() << "    \"type\": \"integer\",\n";
        if (std::is_unsigned<Int>::value || std::is_base_of<types::unsigned_base, Int>::value) {
            this->out() << "    \"min\": 0,\n";
        }
        this->out() << "    \"default\": " << stringize_integral(i) << ",\n";
    }

    template<class Int, long min, long max, class T>
    void write_type(type_<types::range<Int, min, max>>, T i)
    {
        this->out() <<
            "    \"type\": \"integer\",\n"
            "    \"min\": " << min << ",\n"
            "    \"max\": " << max << ",\n"
            "    \"default\": " << stringize_integral(i) << ",\n"
        ;
    }


    template<class T, class Ratio, class U>
    void write_type(type_<std::chrono::duration<T, Ratio>>, U i)
    {
        this->out() <<
            "    \"type\": \"duration\",\n"
            "    \"min\": 0,\n"
            "    \"default\": " << stringize_integral(i) << ",\n"
        ;
    }

    template<unsigned N, class T>
    void write_type(type_<types::fixed_binary<N>>, T const & x)
    {
        this->out() <<
            "    \"type\": \"binary\",\n"
            "    \"min\": " << N*2 << ",\n"
            "    \"max\": " << N*2 << ",\n"
            "    \"default\": " << io_hexkey{get_string(x), N} << ",\n"
        ;
    }

    template<unsigned N, class T>
    void write_type(type_<types::fixed_string<N>>, T const & x)
    {
        this->out() <<
            "    \"type\": \"string\",\n"
            "    \"maxlen\": " << N << ",\n"
            "    \"default\": " << quoted2(x) << ",\n"
        ;
    }

    template<class T>
    void write_type(type_<types::dirpath>, T const & x)
    {
        this->out() <<
            "    \"type\": \"dirpath\",\n"
            "    \"default\": " << quoted2(x) << ",\n"
        ;
    }

    template<class T>
    void write_type(type_<types::ip_string>, T const & x)
    {
        this->write_type(type_<std::string>{}, x);
    }

    template<class T, class L>
    void write_type(type_<types::list<T>>, L const & x)
    {
        this->write_type(type_<T>{}, x);
        this->out() << "    \"multivalue\"=true,\n";
    }

    template<class T, class E>
    std::enable_if_t<std::is_enum_v<E>>
    write_type(type_<T>, E const & x)
    {
        static_assert(std::is_same<T, E>::value, "");
        using ll = long long;
        apply_enumeration_for<T>(this->enums, [&x, this](auto const & e) {
            this->write_enum_value(e, ll{static_cast<std::underlying_type_t<E>>(x)});
        });
    }

    template<class T>
    void write_enum_value(type_enumeration const & e, T default_value)
    {
        if (e.flag == type_enumeration::flags) {
            this->out() <<
                "    \"type\": \"bitset\",\n"
                "    \"min\": 0,\n"
                "    \"max\": " << e.max() << ",\n"
                "    \"default\": " << default_value << ",\n"
            ;
        }
        else {
            this->out() <<
                "    \"type\": \"option\",\n"
                "    \"default\": \"" << e.values[default_value].name << "\",\n"
                "    \"values\": {"
            ;
            char const* prefix = "\n";
            int d = 0;
            bool const is_autoinc = (e.flag == type_enumeration::autoincrement);
            for (type_enumeration::Value const & v : e.values) {
                this->out() << prefix <<
                    "      {\n"
                ;
                if (e.is_string_parser) {
                    this->out() <<
                        "         \"value\": \"" << (v.alias ? v.alias : v.name) << "\",\n"
                        "         \"label\": \"" << io_prefix_lines{v.desc, "", "\\n", 0} << "\""
                    ;
                }
                else {
                    this->out() <<
                        "         \"value\": " << (is_autoinc ? d : (1 << d >> 1)) << ",\n"
                        "         \"label\": " << v.desc
                    ;
                }
                this->out() <<
                    ",\n"
                    "      }"
                ;
                prefix = ",\n";
                ++d;
            }
            this->out() << "    },\n";
        }
    }

    template<class T>
    void write_enum_value(type_enumeration_set const & e, T default_value)
    {
        this->out() <<
            "    \"type\": \"option\",\n"
            "    \"default\": \"" << default_value << "\",\n"
            "    \"values\": {"
        ;
        char const* prefix = "\n";
        for (type_enumeration_set::Value const & v : e.values) {
                this->out() << prefix <<
                    "      {    "
                    "         \"value\": \"" << (v.alias ? v.alias : v.name) << "\",\n"
                    "         \"label\": " << v.val << ",\n"
                    "      }"
                ;
                prefix = ",\n";
        }
        this->out() << "    },\n";
    }
};


template<class Inherit>
struct ConnectionPolicyWriterBaseJson : JsonSpecWriterBase<Inherit, connpolicy::name>
{
    using base_type = ConnectionPolicyWriterBaseJson;

    using filename_map_t = std::array<std::pair<std::string, char const*>, 2>;

    ConnectionPolicyWriterBaseJson(filename_map_t filename_map)
    : JsonSpecWriterBase<Inherit, connpolicy::name>("/dev/null")
    , filename_map(std::move(filename_map))
    {}

    static io_prefix_lines comment(char const * s)
    {
        return io_prefix_lines{s, "", "\\n", 0};
    }

    template<class Pack>
    void do_member(std::string const & section_name, Pack const& infos)
    {
        if constexpr (is_convertible_v<Pack, connection_policy_t>) {
            auto type = get_type<spec::type_>(infos);
            std::string const& member_name = get_name<connpolicy::name>(infos);

            this->out() << "  \"" << member_name << "\": {\n";
            this->inherit().write_type(type, get_default<connpolicy::default_>(type, infos));
            this->inherit().write_description(type, infos);
            this->out() << "  }\n\n";

            this->inherit().write_type_info(type);
            this->write_enumeration_value_description(type, infos);

            auto& connpolicy = get_elem<connection_policy_t>(infos);

            using attr1_t = spec::internal::attr;
            using attr2_t = connpolicy::internal::attr;
            auto attr1 = value_or<spec_attr_t>(infos, spec_attr_t{attr1_t::no_ini_no_gui}).value;
            auto attr2 = connpolicy.spec;

            if (bool(attr1 & attr1_t::advanced_in_gui)
             || bool(attr2 & attr2_t::advanced_in_connpolicy))
                this->out() << "#_advanced\n";
            if (bool(attr1 & attr1_t::hex_in_gui)
             || bool(attr2 & attr2_t::hex_in_connpolicy))
                this->out() << "#_hex\n";

            auto&& sections = this->file_map[connpolicy.file];
            auto const& section = value_or<connpolicy::section>(
                infos, connpolicy::section{section_name.c_str()});

            if (this->section_names.emplace(section.name).second) {
                this->ordered_section.emplace_back(section.name);
            }

            Section& sec = sections[section.name];

            auto& sesman_name = get_name<sesman::name>(infos);

            sec.contains += this->out_member_.str();
            update_sesman_contains(sec.sesman_contains, sesman_name, member_name);

            if constexpr (is_convertible_v<Pack, sesman::deprecated_names>) {
                for (auto&& old_name : get_elem<sesman::deprecated_names>(infos).names) {
                    update_sesman_contains(sec.sesman_contains, old_name, member_name, " # Deprecated, for compatibility only.");
                }
            }

            this->out_member_.str("");
        }
    }

    static void update_sesman_contains(
        std::string& s,
        std::string const sesman_name,
        std::string const connpolicy_name,
        char const* extra = "")
    {
        str_append(s, "    u'", sesman_name, "': '", connpolicy_name, "',", extra, '\n');
    }

    void do_init()
    {
    }

    void do_finish()
    {
        for (auto const& [cat, filename] : this->filename_map) {
            auto file_it = file_map.find(cat);
            if (file_it == file_map.end()) {
                this->errorstring += "Config: unknown ";
                this->errorstring += cat;
                this->errorstring += " for connpolicy";
                return ;
            }

            std::ofstream out_spec(filename);
            out_spec << R"g("general": {
  "transformation_rule": {
    "type": "string",
    "default": "",
    "description": "Secondary login Transformation rule\n${LOGIN} will be replaced by login\n${DOMAIN} (optional) will be replaced by domain if it exists.\nEmpty value means no transformation rule."
  },

  "vault_transformation_rule": {
    "type": "string",
    "default": "",
    "description": "Account Mapping password retriever\nTransformation to apply to find the correct account.\n${USER} will be replaced by the user's login.\n${DOMAIN} will be replaced by the user's domain (in case of LDAP mapping).\n${USER_DOMAIN} will be replaced by the user's login + "@" + user's domain (or just user's login if there's no domain).\n${GROUP} will be replaced by the authorization's user group.\n${DEVICE} will be replaced by the device's name.\nA regular expression is allowed to transform a variable, with the syntax: ${USER:/regex/replacement}, groups can be captured with parentheses and used with \1, \2, ...\nFor example to replace leading "A" by "B" in the username: ${USER:/^A/B}\nEmpty value means no transformation rule."
  }
},
)g";
            auto& section_map = file_it->second;
            for (auto& section_name : this->ordered_section) {
                auto section_it = section_map.find(section_name);
                if (section_it != section_map.end()) {
                    out_spec << "\"" << section_name << "\": {\n" << section_it->second.contains << "}\n";
                }
            }

            if (!out_spec) {
                this->errorstring = filename;
                return;
            }
            out_spec.close();
        }
    }

private:
    struct Section
    {
        std::string contains;
        std::string sesman_contains;
    };
    using data_by_section_t = std::unordered_map<std::string, Section>;
    std::unordered_map<std::string, data_by_section_t> file_map;
    std::vector<std::string> ordered_section;
    std::unordered_set<std::string> section_names;
    filename_map_t filename_map;

public:
    std::string errorstring;
};

}


template<class SpecWriter>
int app_write_connection_policy(int ac, char const * const * av)
{
    if (ac < 4) {
        std::cerr << av[0] << " sesman-map.py rdp.spec vnc.spec\n";
        return 1;
    }

    SpecWriter writer(typename SpecWriter::filename_map_t{
        std::pair{"rdp", av[2]},
        std::pair{"vnc", av[3]}
    }, av[1]);
    writer.evaluate();

    if (!writer.out_file_) {
        std::cerr << av[0] << ": " << av[1] << ": " << strerror(errno) << "\n";
        return 1;
    }
    if (!writer.errorstring.empty()) {
        std::cerr << av[0] << ": " << writer.errorstring << "\n";
        return 1;
    }
    return 0;
}

template<class SpecWriter>
int app_write_connection_policy_json(int ac, char const * const * av)
{
    if (ac < 3) {
        std::cerr << av[0] << " rdp.spec vnc.spec\n";
        return 1;
    }

    SpecWriter writer(typename SpecWriter::filename_map_t{
        std::pair{"rdp", av[1]},
        std::pair{"vnc", av[2]}
    });
    writer.evaluate();

    if (!writer.errorstring.empty()) {
        std::cerr << av[0] << ": " << writer.errorstring << "\n";
        return 1;
    }
    return 0;
}

}
