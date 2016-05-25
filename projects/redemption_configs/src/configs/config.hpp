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

#ifndef _REDEMPTION_CORE_CONFIG_HPP_
#define _REDEMPTION_CORE_CONFIG_HPP_

#include "configs/io.hpp"

#include "core/authid.hpp"

#include "utils/underlying_cast.hpp"
#include "utils/exchange.hpp"
#include "core/defines.hpp"
#include "core/font.hpp"
#include "utils/log.hpp"

#include <set>
#include <string>
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
    struct CBuf : CStrBuf<typename Config::type> {
    };

    template<class>
    struct BufferPack;

    template<class... Ts>
    struct BufferPack<Pack<Ts...>>
    : CBuf<Ts>...
    {};

    enum class VariableProperties {
        none,
        read  = 1 << 0,
        write = 1 << 1,
    };

    constexpr VariableProperties operator | (VariableProperties x, VariableProperties y) {
        return static_cast<VariableProperties>(underlying_cast(x) | underlying_cast(y));
    }

    constexpr VariableProperties operator & (VariableProperties x, VariableProperties y) {
        return static_cast<VariableProperties>(underlying_cast(x) & underlying_cast(y));
    }
}

#include "configs/variant/includes.hpp"

#include "configs/autogen/enums.hpp"
#include "configs/autogen/variables_configuration.hpp"


class Inifile
{
    using properties_t = configs::VariableProperties;

public:
    using authid_t = ::authid_t;


    explicit Inifile(const char * default_font_name = SHARE_PATH "/" DEFAULT_FONT_NAME)
    : variables(default_font_name)
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

    template<class T, class U>
    void set(U && new_value) {
        static_assert(!T::is_writable(), "T is writable, used set_acl<T>().");
        static_cast<T&>(this->variables).value = std::forward<U>(new_value);
        this->unask<T>(std::integral_constant<bool, T::is_readable()>());
    }

    template<class T, class U, class... O>
    void set(U && param1, O && ... other_params) {
        static_assert(!T::is_writable(), "T is writable, used set_acl<T>().");
        static_cast<T&>(this->variables).value = {std::forward<U>(param1), std::forward<O>(other_params)...};
        this->unask<T>(std::integral_constant<bool, T::is_readable()>());
    }

    template<class T, class U>
    void set_acl(U && new_value) {
        static_assert(T::is_writable(), "T isn't writable, used set<T>().");
        static_cast<T&>(this->variables).value = std::forward<U>(new_value);
        this->insert_index<T>(std::integral_constant<bool, T::is_writable()>());
        this->unask<T>(std::integral_constant<bool, T::is_readable()>());
    }

    template<class T, class U, class... O>
    void set_acl(U && param1, O && ... other_params) {
        static_assert(T::is_writable(), "T isn't writable, used set<T>().");
        static_cast<T&>(this->variables).value = {std::forward<U>(param1), std::forward<O>(other_params)...};
        this->insert_index<T>(std::integral_constant<bool, T::is_writable()>());
        this->unask<T>(std::integral_constant<bool, T::is_readable()>());
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
    template<class T> void insert_index(std::false_type) {}
    template<class T> void insert_index(std::true_type) { this->to_send_index.insert(T::index()); }

    template<class T> void unask(std::false_type) {}
    template<class T> void unask(std::true_type) { this->fields[T::index()].asked_ = false; }

    using Buffers = configs::BufferPack<configs::VariablesAclPack>;

    struct FieldBase
    {
        bool is_asked() const { return this->asked_; }
        virtual void parse(configs::VariablesConfiguration & variables, char const * value) = 0;
        virtual char const * c_str(configs::VariablesConfiguration const & variables, Buffers const & buffers) const = 0;
        virtual ~FieldBase() {}

    private:
        friend class Inifile;
        bool asked_ = false;
    };

    template<class T>
    struct Field : FieldBase
    {
        void parse(configs::VariablesConfiguration & variables, char const * value) override final {
            ::configs::parse(
                static_cast<T&>(variables).value,
                configs::spec_type<typename T::spec_type>{},
                {value, strlen(value)}
            );
        }

        char const * c_str(configs::VariablesConfiguration const & variables, Buffers const & buffers) const override final {
            return ::configs::c_str(
                const_cast<configs::CStrBuf<typename T::type> &>(
                    static_cast<configs::CStrBuf<typename T::type> const &>(
                        static_cast<configs::CBuf<T> const &>(buffers)
                    )
                ), static_cast<T const &>(variables).value
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

        void set(char const * value) {
            this->field->parse(this->ini->variables, value);
            this->field->asked_ = false;
            this->ini->new_from_acl = true;
        }

        char const * c_str() const {
            return this->field->c_str(this->ini->variables, this->ini->buffers);
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

    void notify_from_acl() {
        this->new_from_acl = true;
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

            friend class FieldsChanged;

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

    void check_record_config();

private:
    std::set<unsigned> to_send_index;
    configs::VariablesConfiguration variables;
    configs::PointerPackArray<FieldBase, Field, configs::VariablesAclPack> fields;
    Buffers buffers;
    ConfigurationHolder conf_holder = variables;
    bool new_from_acl = false;

    void initialize() {
        this->to_send_index.insert(cfg::context::opt_bpp::index());
        this->to_send_index.insert(cfg::context::opt_width::index());
        this->to_send_index.insert(cfg::context::opt_height::index());
        this->to_send_index.insert(cfg::context::selector::index());
        this->to_send_index.insert(cfg::context::selector_current_page::index());
        this->to_send_index.insert(cfg::context::selector_device_filter::index());
        this->to_send_index.insert(cfg::context::selector_group_filter::index());
        this->to_send_index.insert(cfg::context::selector_proto_filter::index());
        this->to_send_index.insert(cfg::context::selector_lines_per_page::index());
        this->ask<cfg::context::target_password>();
        this->ask<cfg::context::target_host>();
        this->ask<cfg::context::target_protocol>();
        this->ask<cfg::context::password>();
        this->to_send_index.insert(cfg::context::reporting::index());
        this->to_send_index.insert(cfg::context::auth_channel_result::index());
        this->to_send_index.insert(cfg::context::auth_channel_target::index());
        this->to_send_index.insert(cfg::context::accept_message::index());
        this->to_send_index.insert(cfg::context::display_message::index());
        this->to_send_index.insert(cfg::context::real_target_device::index());
        this->ask<cfg::globals::auth_user>();
        this->to_send_index.insert(cfg::globals::host::index());
        this->to_send_index.insert(cfg::globals::target::index());
        this->ask<cfg::globals::target_device>();
        this->ask<cfg::globals::target_user>();

        static_cast<Field<cfg::context::target_port>&>(this->fields).asked_ = true;
    }
};

#include "configs/autogen/set_value.tcc"
#include "configs/variant/check_record_config.tcc"

#endif
