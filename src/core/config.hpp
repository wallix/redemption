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

#include "log.hpp"
#include "font.hpp"
#include "defines.hpp"
#include "exchange.hpp"

#include "authid.hpp"

#include "dynamic_buffer.hpp"

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
    char const * get_value() const noexcept {
        if (this->is_asked<T>()) {
            return "";
        }
        return this->c_str<T>();
    }

    template<class T>
    typename T::type & get_ref() noexcept {
        static_assert(!bool(T::properties() & properties_t::write), "reference on write variable isn't safe");
        return static_cast<T&>(this->variables).value;
    }

    template<class T>
    char const * c_str() const {
        return ::configs::c_str(
            const_cast<configs::CStrBuf<typename T::type> &>(
                static_cast<configs::CStrBuf<typename T::type> const &>(
                    static_cast<configs::CBuf<T> const &>(this->buffers)
                )
            ), this->get<T>()
        );
    }

    // TODO authid_t authid
    char const * get_cstr_from_key(const char * strauthid) const {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            return this->fields[authid-1].c_str(this->variables, this->buffers);
        }
        else {
            LOG(LOG_WARNING, "Inifile::set_from_acl(strid): unknown strauthid=\"%s\"", strauthid);
        }
        return "";
    }

public:
    template<class T, class U>
    void set(U && new_value) {
        //static_assert(bool(T::properties() & properties_t::write), "T isn't writable");
        static_cast<T&>(this->variables).value = std::forward<U>(new_value);
        this->insert_index<T>(std::integral_constant<bool, bool(T::properties() & properties_t::write)>());
        this->unask<T>(std::integral_constant<bool, bool(T::properties() & properties_t::read)>());
    }

private:
    template<class T> void insert_index(std::false_type) {}
    template<class T> void insert_index(std::true_type) { this->to_send_index.insert(T::index()); }

    template<class T> void unask(std::false_type) {}
    template<class T> void unask(std::true_type) { this->fields[T::index()].asked_ = false; }

    struct FieldBase
    {
        bool is_asked() const { return this->asked_; }
        virtual void parse(configs::VariablesConfiguration & variables, char const * value) const = 0;
        virtual int copy_val(configs::VariablesConfiguration const & variables, char * buff, std::size_t n) const = 0;
        virtual char const * c_str(configs::VariablesConfiguration const & variables, configs::Buffers const & buffers) const = 0;
        virtual ~FieldBase() {}

    private:
        friend class Inifile;
        bool asked_ = false;
    };

    template<class T>
    struct Field : FieldBase
    {
        void parse(configs::VariablesConfiguration & variables, char const * value) const final {
            ::configs::parse(static_cast<T&>(variables).value, value);
        }
        int copy_val(configs::VariablesConfiguration const & variables, char * buff, std::size_t n) const final {
            return ::configs::copy_val(static_cast<T const &>(variables).value, buff, n);
        }
        char const * c_str(configs::VariablesConfiguration const & variables, configs::Buffers const & buffers) const final {
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

    void set_value(const char * context, const char * key, const char * value) override;

    using authid_t = ::authid_t;

    static const uint32_t ENABLE_DEBUG_CONFIG = 1;

    char const * get_by_id(authid_t authid) const {
        if (authid != AUTHID_UNKNOWN) {
            return this->fields[authid-1].c_str(this->variables, this->buffers);
        }
        else {
            auto const & strauthid = string_from_authid(authid);
            LOG(LOG_WARNING, "Inifile::set_from_acl(strid): unknown strauthid=\"%s\"", strauthid);
        }
        return "";
    }

    char const * get_value(authid_t authid) const {
        if (authid != AUTHID_UNKNOWN) {
            if (!this->fields[authid-1].asked_) {
                return this->fields[authid-1].c_str(this->variables, this->buffers);
            }
        }
        else {
            auto const & strauthid = string_from_authid(authid);
            LOG(LOG_WARNING, "Inifile::set_from_acl(strid): unknown strauthid=\"%s\"", strauthid);
        }
        return "";
    }

    void set_by_id(authid_t authid, char const * value) {
        if (authid != AUTHID_UNKNOWN) {
            this->fields[authid-1].parse(this->variables, value);
            this->fields[authid-1].asked_ = false;
        }
        else {
            auto const & strauthid = string_from_authid(authid);
            LOG(LOG_WARNING, "Inifile::set_from_acl(strid): unknown strauthid=\"%s\"", strauthid);
        }
    }

    void ask_by_id(authid_t authid) {
        if (authid != AUTHID_UNKNOWN) {
            this->fields[authid-1].asked_ = true;
        }
        else {
            auto const & strauthid = string_from_authid(authid);
            LOG(LOG_WARNING, "Inifile::set_from_acl(strid): unknown strauthid=\"%s\"", strauthid);
        }
    }

    ///\brief  sets a value to corresponding field but does not mark it as changed
    // TODO authid_t authid
    void set_from_acl(const char * strauthid, const char * value) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            this->fields[authid-1].parse(this->variables, value);
            this->fields[authid-1].asked_ = false;
            this->new_from_acl = true;
        }
        else {
            LOG(LOG_WARNING, "Inifile::set_from_acl(strid): unknown strauthid=\"%s\"", strauthid);
        }
    }

    ///\brief  sets a value to corresponding field but does not mark it as changed
    // TODO authid_t authid
    void ask_from_acl(const char * strauthid) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            this->fields[authid-1].asked_ = true;
        }
        else {
            LOG(LOG_WARNING, "Inifile::ask_from_acl(strid): unknown strauthid=\"%s\"", strauthid);
        }
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

    void parse_username(const char * username)
    {
        this->ask<cfg::context::selector>();
        LOG(LOG_INFO, "asking for selector");

        this->set<cfg::globals::auth_user>(username);
        this->ask<cfg::globals::target_user>();
        this->ask<cfg::globals::target_device>();
        this->ask<cfg::context::target_protocol>();
    }

    template<class Fn>
    void serialized(Fn writer, uint32_t password_printing_mode)
    {
        char buff[65536];
        auto const size = sizeof(buff);

        for (unsigned i : this->to_send_index) {
            const char * key = string_from_authid(static_cast<authid_t>(i+1));
            int n;
            FieldBase & bfield = this->fields[i];
            if (bfield.is_asked()) {
                n = snprintf(buff, size, "%s\nASK\n",key);
                LOG(LOG_INFO, "sending %s=ASK", key);
            }
            else {
                n = snprintf(buff, size, "%s\n!", key);
                auto val = buff + n;
                int tmpn = bfield.copy_val(this->variables, buff + n, size);
                if (tmpn >= 0) {
                    const char * display_val = val;
                    if ((strncasecmp("password", key, 8) == 0)
                      ||(strncasecmp("target_password", key, 15) == 0)){
                        display_val = get_printable_password(val, password_printing_mode);
                    }
                    LOG(LOG_INFO, "sending %s=%s", key, display_val);
                    n += tmpn;
                    if (std::size_t(n+1) < size) {
                        buff[n] = '\n';
                        buff[n+1] = 0;
                        ++n;
                    }
                    else {
                        n = -1;
                    }
                }
                else {
                    LOG(LOG_INFO, "Copy of %s fail", key);
                    n = -1;
                }
            }

            writer(buff, n, size);
        }
    }

    void check_record_config();

private:
    std::set<unsigned> to_send_index;
    configs::VariablesConfiguration variables;
    configs::PointerPackArray<FieldBase, Field, configs::VariablesAclPack> fields;
    configs::Buffers buffers;
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

// #include "config_initialize.tcc"
#include "config_set_value.tcc"
#include "config_check_record_config.tcc"

#endif
