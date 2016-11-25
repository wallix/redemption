/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Jonathan Poelen, Raphael Zhou, Meng Tan

    Configuration file,
    parsing config file rdpproxy.ini
*/

#pragma once

#include "configs/io.hpp"

#include "core/authid.hpp"

#include "utils/sugar/underlying_cast.hpp"
#include "utils/sugar/exchange.hpp"
#include "core/defines.hpp"
#include "core/font.hpp"
#include "utils/log.hpp"

#include <set>
#include <string>
#include <chrono>
#include <stdexcept>

#include <cstdint>
#include <cassert>
#include <cstdio>

namespace configs {
    template<class... Ts>
    struct Pack : Ts... {};

    template<class Base, template<class> class Tpl, class Pack>
    struct PointerPackArray;

    template<class Base, template<class> class Tpl, class... Ts>
    struct PointerPackArray<Base, Tpl, Pack<Ts...>>
    : Tpl<Ts>...
    {
        Base * values[sizeof...(Ts)];

        PointerPackArray()
        : values{&static_cast<Tpl<Ts>&>(*this)...}
        {}

        Base & operator[](std::size_t i) {
            assert(i < sizeof...(Ts));
            return *this->values[i];
        }

        Base const & operator[](std::size_t i) const {
            assert(i < sizeof...(Ts));
            return *this->values[i];
        }
    };

    template<class Config>
    struct CBuf : zstr_buffer_from<typename Config::type> {
    };

    template<class>
    struct BufferPack;

    template<class... Ts>
    struct BufferPack<Pack<Ts...>>
    : CBuf<Ts>...
    {};
}

// members
//@{
#include "core/font.hpp"
#include "utils/theme.hpp"
#include "utils/redirection_info.hpp"
//@}

#include "configs/autogen/enums.hpp"
#include "configs/autogen/enums_func_ini.hpp"
#include "configs/autogen/variables_configuration.hpp"

#include "utils/fileutils.hpp" // file_exist

class Inifile;

namespace configs
{
    template<class T, class U>
    parse_error parse_and_log(const char * context, const char * key, T & x, U u, array_view_const_char av)
    {
        auto const err = ::configs::parse(x, u, av);
        if (err) {
            LOG(
                LOG_ERR,
                "parsing error with parameter '%s' in section [%s] for \"%*s\": %s",
                key, context, int(av.size()), av.data(), err.c_str()
            );
        }
        return err;
    }

    template<class CfgType>
    void post_set_value(VariablesConfiguration &, CfgType const &)
    {}

    inline void post_set_value(VariablesConfiguration & vars, ::cfg::internal_mod::theme const & cfg_value)
    {
        Theme & theme = static_cast<cfg::theme&>(vars).value;

        auto & str = cfg_value.value;
        if (static_cast<cfg::debug::config>(vars).value) {
            LOG(LOG_INFO, "LOAD_THEME: %s", str.c_str());
        }

        // load theme

        {
            char theme_path[1024] = {};
            snprintf(theme_path, 1024, CFG_PATH "/themes/%s/" THEME_INI, str.c_str());
            theme_path[sizeof(theme_path) - 1] = 0;

            ConfigurationLoader theme_load;
            theme_load.cparse(theme, theme_path);
        }

        if (theme.global.logo) {
            char logo_path[1024] = {};
            snprintf(logo_path, 1024, CFG_PATH "/themes/%s/" LOGO_PNG, str.c_str());
            logo_path[sizeof(logo_path) - 1] = 0;
            if (!file_exist(logo_path)) {
                snprintf(logo_path, 1024, CFG_PATH "/themes/%s/" LOGO_BMP, str.c_str());
                logo_path[sizeof(logo_path) - 1] = 0;
                if (!file_exist(logo_path)) {
                    theme.global.logo = false;
                    return;
                }
            }
            theme.set_logo_path(logo_path);
        }
    }
}


class Inifile
{
public:
    using authid_t = ::authid_t;
    using parse_error = configs::parse_error;

    // TODO temporary
    using debug_section_type = cfg::debug;


    explicit Inifile()
    {
        this->initialize();
    }

    template<class T>
    typename T::type const & get() const noexcept {
        //static_assert(T::is_readable(), "T isn't readable");
        return static_cast<T const &>(this->variables).value;
    }

    template<class T>
    typename T::type & get_ref() noexcept {
        static_assert(!T::is_writable(), "reference on write variable isn't safe");
        return static_cast<T&>(this->variables).value;
    }

    template<class T, class... Args>
    void set(Args && ... args) {
        static_assert(!T::is_writable(), "T is writable, used set_acl<T>() instead.");
        this->set_value<T>(std::forward<Args>(args)...);
    }

    template<class T, class... Args>
    void set_acl(Args && ... args) {
        static_assert(T::is_writable(), "T isn't writable, used set<T>() instead.");
        this->set_value<T>(std::forward<Args>(args)...);
    }

    template<class T>
    void ask() {
        static_assert(T::is_readable(), "T isn't askable");
        this->to_send_index.insert(T::index());
        static_cast<Field<T>&>(this->fields).asked_ = true;
    }

    template<class T>
    bool is_asked() const {
        static_assert(T::is_readable(), "T isn't askable");
        return static_cast<Field<T>const&>(this->fields).asked_;
    }

private:
    template<class T, class... Args>
    void set_value(Args && ... args) {
        configs::set_value(
            static_cast<T&>(this->variables).value,
            configs::spec_type<typename T::mapped_type>{},
            std::forward<Args>(args)...
        );
        configs::post_set_value(this->variables, static_cast<T&>(this->variables));
        this->insert_index<T>(std::integral_constant<bool, T::is_writable()>());
        this->unask<T>(std::integral_constant<bool, T::is_readable()>());
    }

    template<class T> void insert_index(std::false_type) {}
    template<class T> void insert_index(std::true_type) { this->to_send_index.insert(T::index()); }

    template<class T> void unask(std::false_type) {}
    template<class T> void unask(std::true_type) { this->fields[T::index()].asked_ = false; }

    struct Buffers : configs::BufferPack<configs::VariablesAclPack> {};

    struct FieldBase
    {
        bool is_asked() const { return this->asked_; }
        virtual bool parse(configs::VariablesConfiguration & variables, array_view_const_char value) = 0;
        virtual array_view_const_char
        to_string_view(configs::VariablesConfiguration const & variables, Buffers const & buffers) const = 0;
        virtual ~FieldBase() = default;

    private:
        friend class Inifile;
        bool asked_ = false;
    };

    template<class T>
    struct Field : FieldBase
    {
        bool parse(configs::VariablesConfiguration & variables, array_view_const_char value) override final
        {
            return ! ::configs::parse_and_log(
                T::section(), T::name(),
                static_cast<T&>(variables).value,
                configs::spec_type<typename T::sesman_and_spec_type>{},
                value
            );
        }

        /// \return array_view_const_char::data() guarantee with null terminal
        array_view_const_char
        to_string_view(configs::VariablesConfiguration const & variables, Buffers const & buffers) const override final
        {
            return ::configs::assign_zbuf_from_cfg(
                const_cast<configs::zstr_buffer_from<typename T::type> &>(
                    static_cast<configs::zstr_buffer_from<typename T::type> const &>(
                        static_cast<configs::CBuf<T> const &>(buffers)
                    )
                ),
                configs::cfg_s_type<typename T::sesman_and_spec_type>{},
                static_cast<T const &>(variables).value
            );
        }
    };

public:
    struct ConfigurationHolder : ::ConfigurationHolder
    {
        void set_value(const char * context, const char * key, const char * value) override;

    private:
        friend class Inifile;

        ConfigurationHolder(configs::VariablesConfiguration & variables)
        : variables(variables)
        {}

        configs::VariablesConfiguration & variables;
    };

    ConfigurationHolder & configuration_holder() {
        return this->conf_holder;
    }

    static const uint32_t ENABLE_DEBUG_CONFIG = 1;

    struct FieldReference
    {
        bool is_asked() const {
            return this->field->asked_;
        }

        void ask() {
            this->field->asked_ = true;
        }

        bool set(char const *) = delete; // use `set(cstr_array_view("blah blah"))` instead

        bool set(array_view_const_char value) {
            auto const err = this->field->parse(this->ini->variables, value);
            if (err) {
                this->field->asked_ = false;
                this->ini->new_from_acl = true;
            }
            return err;
        }

        array_view_const_char to_string_view() const {
            return this->field->to_string_view(this->ini->variables, this->ini->buffers);
        }

        char const * c_str() const {
            return this->to_string_view().data();
        }

        explicit operator bool () const {
            return this->field;
        }

        authid_t authid() const {
            return this->id;
        }

        FieldReference(FieldReference &&) = default;

        FieldReference() = default;

    private:
        FieldBase * field = nullptr;
        Inifile * ini = nullptr;
        authid_t id = authid_t::AUTHID_UNKNOWN;

        FieldReference(FieldReference const &) = delete;
        FieldReference & operator=(FieldReference const &) = delete;

        FieldReference(Inifile & ini, authid_t id)
        : field(&ini.fields[static_cast<unsigned>(id)])
        , ini(&ini)
        , id(id)
        {}

        friend class Inifile;
        friend class iterator;
    };

    FieldReference get_acl_field(authid_t authid) {
        if (authid >= authid_t::MAX_AUTHID) {
            return {};
        }
        return {*this, authid};
    }

    bool check_from_acl() {
        return exchange(this->new_from_acl, false);
    }

    std::size_t changed_field_size() const {
        return this->to_send_index.size();
    }

    void clear_send_index() {
        this->to_send_index.clear();
    }

    struct FieldsChanged
    {
        struct iterator
        {
            iterator & operator++() {
                ++this->it;
                return *this;
            }

            bool operator != (iterator const & other) {
                return this->it != other.it;
            }

            FieldReference operator*() const {
                return {*this->ini, static_cast<authid_t>(*this->it)};
            }

        private:
            std::set<unsigned>::const_iterator it;
            Inifile * ini;

            friend struct FieldsChanged;

            iterator(std::set<unsigned>::const_iterator it, Inifile & ini)
            : it(it)
            , ini(&ini)
            {}
        };

        iterator begin() const { return {this->ini->to_send_index.cbegin(), *this->ini}; }
        iterator end() const { return {this->ini->to_send_index.cend(), *this->ini}; }

    private:
        Inifile * ini;

        friend class Inifile;

        FieldsChanged(Inifile & ini)
        :ini(&ini)
        {}
    };

    FieldsChanged get_fields_changed() const {
        return {*const_cast<Inifile*>(this)};
    }

private:
    std::set<unsigned> to_send_index;
    configs::VariablesConfiguration variables;
    configs::PointerPackArray<FieldBase, Field, configs::VariablesAclPack> fields;
    Buffers buffers;
    ConfigurationHolder conf_holder {variables};
    bool new_from_acl = false;

    template<class T>
    void push_to_send_index()
    {
        static_assert(T::is_writable(), "is not writable");
        this->to_send_index.insert(T::index());
    }

    void initialize()
    {
        this->push_to_send_index<cfg::context::opt_bpp>();
        this->push_to_send_index<cfg::context::opt_width>();
        this->push_to_send_index<cfg::context::opt_height>();
        this->push_to_send_index<cfg::context::selector_current_page>();
        this->push_to_send_index<cfg::context::selector_device_filter>();
        this->push_to_send_index<cfg::context::selector_group_filter>();
        this->push_to_send_index<cfg::context::selector_proto_filter>();
        this->push_to_send_index<cfg::context::selector_lines_per_page>();
        this->ask<cfg::context::target_password>();
        this->ask<cfg::context::target_host>();
        this->ask<cfg::context::target_protocol>();
        this->ask<cfg::context::password>();
        this->push_to_send_index<cfg::context::reporting>();
        this->push_to_send_index<cfg::context::auth_channel_target>();
        this->push_to_send_index<cfg::context::accept_message>();
        this->push_to_send_index<cfg::context::display_message>();
        this->push_to_send_index<cfg::context::real_target_device>();
        this->ask<cfg::globals::auth_user>();
        this->push_to_send_index<cfg::globals::host>();
        this->push_to_send_index<cfg::globals::target>();
        this->ask<cfg::globals::target_device>();
        this->ask<cfg::globals::target_user>();
        this->push_to_send_index<cfg::session_log::session_log_redirection>();

        static_cast<Field<cfg::context::target_port>&>(this->fields).asked_ = true;
    }
};

#include "configs/autogen/set_value.tcc"
