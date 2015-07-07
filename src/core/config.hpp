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

#include "authid.hpp"

#include "configs/parse.hpp"
#include "configs/variables_configuration.hpp"


struct Inifile : FieldObserver, configs::VariablesConfiguration
{
    Inifile(const char * default_font_name = SHARE_PATH "/" DEFAULT_FONT_NAME)
    : VariablesConfiguration(default_font_name)
    {
        this->initialize();
    }

    virtual void set_value(const char * context, const char * key, const char * value) override;

    using authid_t = ::authid_t;

    static const uint32_t ENABLE_DEBUG_CONFIG = 1;

private:
    void initialize();

    BaseField * get_field(authid_t authid) const {
        auto pos = this->field_list.find(authid);
        if (pos != this->field_list.end()) {
            return pos->second;
        }
        return nullptr;
    }

public:
    /******************
     * Set_from_acl sets a value to corresponding field but does not mark it as changed
     */
    void set_from_acl(const char * strauthid, const char * value) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            if (BaseField * field = this->get_field(authid)) {
                field->set_from_acl(value);
            }
            else {
                LOG(LOG_WARNING, "Inifile::set_from_acl(id): unknown authid=%d", authid);
            }
        }
        else {
            LOG(LOG_WARNING, "Inifile::set_from_acl(strid): unknown strauthid=\"%s\"", strauthid);
        }
    }

    /******************
     * ask_from_acl sets a value to corresponding field but does not mark it as changed
     */
    void ask_from_acl(const char * strauthid) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            if (BaseField * field = this->get_field(authid)) {
                field->ask_from_acl();
            }
            else {
                LOG(LOG_WARNING, "Inifile::ask_from_acl(id): unknown authid=%d", authid);
            }
        }
        else {
            LOG(LOG_WARNING, "Inifile::ask_from_acl(strid): unknown strauthid=\"%s\"", strauthid);
        }
    }

    void context_set_value(authid_t authid, const char * value) {
        if (BaseField * field = this->get_field(authid)) {
            field->set_from_cstr(value);
        }
        else {
            LOG(LOG_WARNING, "Inifile::context_set_value(id): unknown authid=%d", authid);
        }
    }

    const char * context_get_value_by_string(const char * strauthid) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            return context_get_value(authid);
        }

        LOG(LOG_WARNING, "Inifile::context_get_value(strid): unknown strauthid=\"%s\"", strauthid);

        return "";
    }

    const char * context_get_value(authid_t authid) {
        const char * pszReturn = "";

        //LOG(LOG_INFO, "Getting value for authid=%d", authid);
        if (BaseField * field = this->get_field(authid)) {
            if (!field->is_asked()) {
                pszReturn = field->get_value();
            }
        }
        else {
            LOG(LOG_WARNING, "Inifile::context_get_value(id): unknown authid=\"%d\"", authid);
        }

        return pszReturn;
    }

    void context_ask(authid_t authid) {
        if (BaseField * field = this->get_field(authid)) {
            field->ask();
        }
        else {
            LOG(LOG_WARNING, "Inifile::context_ask(id): unknown authid=%d", authid);
        }
    }

    bool context_is_asked(authid_t authid) {
        if (BaseField * field = this->get_field(authid)) {
            return field->is_asked();
        }
        else {
            LOG(LOG_WARNING, "Inifile::context_is_asked(id): unknown authid=%d", authid);
            return false;
        }
    }

    bool context_get_bool(authid_t authid) {
        TODO("ask related behavior is a problem. How do we make the difference between a False value and asked ?");
        switch (authid)
            {
            case AUTHID_SELECTOR:
                if (!this->context.selector.is_asked()) {
                    return this->context.selector.get();
                }
                break;
            case AUTHID_KEEPALIVE:
                if (!this->context.keepalive.is_asked()) {
                    return this->context.keepalive.get();
                }
                break;
            case AUTHID_AUTHENTICATED:
                return this->context.authenticated.get();
            case AUTHID_AUTHENTICATION_CHALLENGE:
                if (!this->context.authentication_challenge.is_asked()) {
                    return this->context.authentication_challenge.get();
                }
                break;
            default:
                LOG(LOG_WARNING, "Inifile::context_get_bool(id): unknown authid=\"%d\"", authid);
                break;
            }

        return false;
    }

    void parse_username(const char * username)
    {
        this->context_ask(AUTHID_SELECTOR);
        LOG(LOG_INFO, "asking for selector");

        this->context_set_value(AUTHID_AUTH_USER, username);
        this->context_ask(AUTHID_TARGET_USER);
        this->context_ask(AUTHID_TARGET_DEVICE);
        this->context_ask(AUTHID_TARGET_PROTOCOL);
    }

    static int serialized(char * buff, std::size_t size, BaseField & bfield, uint32_t password_printing_mode)
    {
        const char * key = string_from_authid(static_cast<authid_t>(bfield.get_authid()));
        int n;
        if (bfield.is_asked()) {
            n = snprintf(buff, size, "%s\nASK\n",key);
            LOG(LOG_INFO, "sending %s=ASK", key);
        }
        else {
            const char * val         = bfield.get_value();
            const char * display_val = val;
            n = snprintf(buff, size, "%s\n!%s\n", key, val);
            if ((strncasecmp("password", key, 8) == 0)
                ||(strncasecmp("target_password", key, 15) == 0)){
                display_val = get_printable_password(val, password_printing_mode);
            }
            LOG(LOG_INFO, "sending %s=%s", key, display_val);
        }
        if (n < 0 || static_cast<std::size_t>(n) >= size ) {
            LOG(LOG_ERR, "Sending Data to ACL Error: Buffer overflow,"
                " should have write %u bytes but buffer size is %u bytes", n, size);
            throw Error(ERR_ACL_MESSAGE_TOO_BIG);
        }

        return n;
    }
};

inline
#include "configs/config_initialize.tcc"
inline
#include "configs/config_set_value.tcc"

#endif
