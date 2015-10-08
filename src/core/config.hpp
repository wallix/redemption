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

#include <stdio.h>
#include <stdint.h>

#include <stdexcept>
#include <string>
#include <set>

#include "log.hpp"
#include "font.hpp"
#include "defines.hpp"
#include "exchange.hpp"

#include "authid.hpp"

#include "get_printable_password.hpp"

#include "config_parse.hpp"
#include "config_c_str_buf.hpp"

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

#include "config_variables_configuration.hpp"

#include "config_parse.hpp"

class Inifile : public ConfigurationHolder
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
        //static_assert(bool(T::properties() & properties_t::read), "T isn't readable");
        return static_cast<T const &>(this->variables).value;
    }

    template<class T>
    typename T::type & get_ref() noexcept {
        static_assert(!bool(T::properties() & properties_t::write), "reference on write variable isn't safe");
        return static_cast<T&>(this->variables).value;
    }

    template<class T, class U>
    void set(U && new_value) {
        static_assert(!bool(T::properties() & properties_t::write), "T is writable, used set_acl<T>().");
        static_cast<T&>(this->variables).value = std::forward<U>(new_value);
        this->unask<T>(std::integral_constant<bool, bool(T::properties() & properties_t::read)>());
    }

    template<class T, class U>
    void set_acl(U && new_value) {
        static_assert(bool(T::properties() & properties_t::write), "T isn't writable, used set<T>().");
        static_cast<T&>(this->variables).value = std::forward<U>(new_value);
        this->insert_index<T>(std::integral_constant<bool, bool(T::properties() & properties_t::write)>());
        this->unask<T>(std::integral_constant<bool, bool(T::properties() & properties_t::read)>());
    }

    template<class T>
    void ask() {
        static_assert(bool(T::properties() & properties_t::read), "T isn't askable");
        this->to_send_index.insert(T::index());
        static_cast<Field<T>&>(this->fields).asked_ = true;
    }

    template<class T>
    bool is_asked() const {
        static_assert(bool(T::properties() & properties_t::read), "T isn't askable");
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
        virtual int copy_val(configs::VariablesConfiguration const & variables, char * buff, std::size_t n) const = 0;
        virtual char const * c_str(configs::VariablesConfiguration const & variables, Buffers const & buffers) const = 0;
        virtual ~FieldBase() {}

    private:
        friend class Inifile;
        bool asked_ = false;
    };

    template<class T>
    struct Field : FieldBase
    {
        void parse(configs::VariablesConfiguration & variables, char const * value) final {
            ::configs::parse(static_cast<T&>(variables).value, value);
        }

        int copy_val(configs::VariablesConfiguration const & variables, char * buff, std::size_t n) const final {
            return ::configs::copy_val(static_cast<T const &>(variables).value, buff, n);
        }

        char const * c_str(configs::VariablesConfiguration const & variables, Buffers const & buffers) const final {
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
    void set_value(const char * context, const char * key, const char * value) override;

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
            this->field->parse(this->ini.variables, value);
            this->field->asked_ = false;
            this->ini.new_from_acl = true;
        }

        int copy(char * buff, std::size_t n) const {
            return this->field->copy_val(this->ini.variables, buff, n);
        }

        char const * c_str() const {
            return this->field->c_str(this->ini.variables, this->ini.buffers);
        }

        explicit operator bool () const {
            return this->field;
        }

        FieldReference(FieldReference &&) = default;

    private:
        FieldBase * field;
        Inifile & ini;

        FieldReference(FieldReference const &) = delete;
        FieldReference & operator=(FieldReference const &) = delete;

        FieldReference(FieldBase * field_ptr, Inifile & ini)
        : field(field_ptr)
        , ini(ini)
        {}

        friend class Inifile;
    };

    FieldReference get_acl_field(authid_t authid) {
        return {authid >= authid_t::MAX_AUTHID ? nullptr : &this->fields[static_cast<unsigned>(authid)], *this};
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

    template<class Fn>
    void for_each_changed_field(Fn fn)
    {
        for (unsigned i : this->to_send_index) {
            fn(FieldReference{&this->fields[i], *this}, static_cast<authid_t>(i));
        }
    }

    void check_record_config();

private:
    std::set<unsigned> to_send_index;
    configs::VariablesConfiguration variables;
    configs::PointerPackArray<FieldBase, Field, configs::VariablesAclPack> fields;
    Buffers buffers;
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

#include "config_set_value.tcc"
#include "config_check_record_config.tcc"

#endif
