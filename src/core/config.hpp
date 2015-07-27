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

#include "config_parse.hpp"

namespace configs {
    template<class... Ts>
    struct Pack : Ts... {};

    template<class Base, class Pack>
    struct PointerPackArray;

    template<class Base, class... Ts>
    struct PointerPackArray<Base, Pack<Ts...>>
    {
        Base * values[sizeof...(Ts)];

        template<class TPack>
        PointerPackArray(TPack & pack)
        : values{&static_cast<Ts&>(pack)...}
        {}

        Base & operator[](std::size_t i) {
            return this->values[i];
        }

        Base const & operator[](std::size_t i) const {
            return this->values[i];
        }
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

struct Inifile : ConfigurationHolder
{
    explicit Inifile(const char * default_font_name = SHARE_PATH "/" DEFAULT_FONT_NAME)
    : variables(default_font_name)
    , fields(this->variables)
    {
        this->initialize();
    }

    template<class T>
    typename T::type const & get() const noexcept {
        static_assert(T::is_readable, "T isn't readable");
        return static_cast<T const &>(this->variables).value;
    }

    template<class T, class U>
    void set(U && new_value) {
        static_assert(T::is_writable, "T isn't writable");
        static_cast<T&>(this->variables).value = std::forward<U>(new_value);
        this->insert_index<T>(std::integral_constant<bool, T::is_sendable>());
    }

private:
    template<class T> void insert_index(std::false_type) {}
    template<class T> void insert_index(std::true_type) { this->to_send_index.insert(T::index); }

    struct FieldBase
    {
        bool asked = false;
        virtual void parse(configs::VariablesConfiguration & variables, char const * value) = 0;
        virtual ~FieldBase() {}
    };

    template<class T>
    struct Field final : FieldBase
    {
        void parse(configs::VariablesConfiguration & variables, char const * value) override {
          ::configs::parse(static_cast<T&>(variables).value, value);
        }
        ~Field() final {}
    };

public:
    template<class T>
    void ask() {
        static_assert(T::is_sendable, "T isn't askable");
        constexpr auto index = T::index;
        this->to_send_index.insert(index);
        const_cast<Field<T>&>(this->fields).asked = true;
    }

    template<class T>
    void is_asked() const {
        static_assert(T::is_sendable, "T isn't askable");
        return const_cast<Field<T>const&>(this->fields).asked;
    }

    void set_value(const char * context, const char * key, const char * value) override;

    using authid_t = ::authid_t;

    static const uint32_t ENABLE_DEBUG_CONFIG = 1;

    ///\brief  sets a value to corresponding field but does not mark it as changed
    void set_from_acl(const char * strauthid, const char * value) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            this->fields[authid].parse(this->variables, value);
            this->fields[authid].asked = false;
            this->new_from_acl = true;
        }
        else {
            LOG(LOG_WARNING, "Inifile::set_from_acl(strid): unknown strauthid=\"%s\"", strauthid);
        }
    }

    ///\brief  sets a value to corresponding field but does not mark it as changed
    void ask_from_acl(const char * strauthid) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            this->fields[authid].asked = true;
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

private:
    std::set<unsigned> to_send_index;
    configs::VariablesConfiguration variables;
    configs::PointerPackArray<FieldBase, configs::VariablesAclPack> fields;
    bool new_from_acl = false;

    void initialize();

    void parse_username(const char * username)
    {
        this->ask<cfg::context::selector>();
        LOG(LOG_INFO, "asking for selector");

        this->set<cfg::globals::auth_user>(username);
        this->ask<cfg::globals::target_user>();
        this->ask<cfg::globals::target_device>();
        this->ask<cfg::context::target_protocol>();
    }

//     static int serialized(char * buff, std::size_t size, BaseField & bfield, uint32_t password_printing_mode)
//     {
//         const char * key = string_from_authid(static_cast<authid_t>(bfield.get_authid()));
//         int n;
//         if (bfield.is_asked()) {
//             n = snprintf(buff, size, "%s\nASK\n",key);
//             LOG(LOG_INFO, "sending %s=ASK", key);
//         }
//         else {
//             const char * val         = bfield.get_value();
//             const char * display_val = val;
//             n = snprintf(buff, size, "%s\n!%s\n", key, val);
//             if ((strncasecmp("password", key, 8) == 0)
//                 ||(strncasecmp("target_password", key, 15) == 0)){
//                 display_val = get_printable_password(val, password_printing_mode);
//             }
//             LOG(LOG_INFO, "sending %s=%s", key, display_val);
//         }
//         if (n < 0 || static_cast<std::size_t>(n) >= size ) {
//             LOG(LOG_ERR, "Sending Data to ACL Error: Buffer overflow,"
//                 " should have write %u bytes but buffer size is %u bytes", n, size);
//             throw Error(ERR_ACL_MESSAGE_TOO_BIG);
//         }
//
//         return n;
//     }

    void check_record_config();
};

#include "config_initialize.tcc"
#include "config_set_value.tcc"
#include "config_check_record_config.tcc"

#endif
