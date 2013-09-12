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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_MOD_INTERNAL_RWL_LOGIN_MOD_HPP
#define REDEMPTION_MOD_INTERNAL_RWL_LOGIN_MOD_HPP

#include "front_api.hpp"
#include "config.hpp"
#include "widget2/window_login.hpp"
#include "widget2/image.hpp"
#include "internal_mod.hpp"
#include "widget2/notify_api.hpp"

#include <fstream>
#include <boost/lexical_cast.hpp>

///rwl: redemption widget language

class RwlDefinition {
public:
    struct utils{
        static const char * ignorespace(const char * s){
            while (*s && std::isspace(*s)) {
                ++s;
            }
            return s;
        }

        static const char * ignoretoken(const char * s){
            while (*s && (
                          ('a' <= *s && *s <= 'z')
                          || ('A' <= *s && *s <= 'Z')
                          || ('0' <= *s && *s <= '9')
                          || *s == '_' || *s == '-')
                   ) {
                ++s;
            }
            return s;
        }

        static int chex2i(char c, bool& err) {
            if ('0' <= c && c <= '9') {
                return c - '0';
            }
            if ('A' <= c && c <= 'Z') {
                return c - 'A' + 10;
            }
            if ('a' <= c && c <= 'z') {
                return c - 'a' + 10;
            }
            err = 1;
            return 0;
        }
    };

    static int parse_color(const char * first, const char * last, bool& err)
    {
        int color = 0;

        if (*first == '#') {
            if (last - first == 4) {
                int r = utils::chex2i(first[1], err);
                int g = utils::chex2i(first[2], err);
                int b = utils::chex2i(first[3], err);
                color = (r << 20) + (r << 16) + (g << 12) + (g << 8) + (b << 4) + b;
            }
            else if (last - first == 7) {
                color = (utils::chex2i(first[1], err) << 20)
                    + (utils::chex2i(first[2], err) << 16)
                    + (utils::chex2i(first[3], err) << 12)
                    + (utils::chex2i(first[4], err) << 8)
                    + (utils::chex2i(first[5], err) << 4)
                    + utils::chex2i(first[6], err);
            }
            else {
                err = 1;
            }
        }
        else if (last - first >= 10
                 && first[0] == 'r'
                 && first[1] == 'g'
                 && first[2] == 'b'
                 && last[-1] == ')')
            {
                first = utils::ignorespace(first + 3);
                if (*first != '(') {
                    err = 1;
                    return color;
                }

                struct next_component {
                    static int next(const char *& p, bool& err){
                        p = utils::ignorespace(p);
                        const char * s = p;
                        while ('0' <= *p && *p <= '9') {
                            std::cout << "*p: " << (*p) << std::endl;
                            ++p;
                        }

                        int component = 0;
                        switch (p - s) {
                        case 3: component = (s[0]-'0') * 100 + (s[1]-'0') * 10 + s[2]-'0';
                            break;
                        case 2: component = (s[0]-'0') * 10 + (s[1]-'0');
                            break;
                        case 1: component = (s[0]-'0');
                            break;
                        default:
                            err = 1;
                        }
                        p = utils::ignorespace(p);
                        return component;
                    }
                };

                int component = next_component::next(++first, err);
                std::cout << "component: " << (component) << std::endl;
                if (*first != ',' && component > 255) {
                    err = 1;
                    return 0;
                }
                color = component << 16;
                component = next_component::next(++first, err);
                std::cout << "component: " << (component) << std::endl;
                if (*first != ',' && component > 255) {
                    err = 1;
                    return 0;
                }
                color += component << 8;
                component = next_component::next(++first, err);
                std::cout << "component: " << (component) << std::endl;
                if (first + 1 != last && component > 255) {
                    err = 1;
                    return 0;
                }
                color += component;
            }
        else {
            std::cout << "else *first: " << (*first) << std::endl;
            err = 1;
        }

        return color;
    }

    struct Property {
        std::string name;

        union Value {
            const char * s;
            int i;
            unsigned u;
            long l;
            unsigned long ul;
        } value;
        int type;

        Property()
            : name()
            , type(0)
        {}

        Property(const Property& other)
            : name(other.name)
            , type(other.type)
        {
            if ('i' == other.type) {
                this->value.i = other.value.i;
            }
            if ('s' == other.type) {
                this->value.s = strdup(other.value.s);
            }
        }

        bool set_value(const char * first, const char * last) {
            if (this->name == "bgcolor" || this->name == "color") {
                bool err = false;
                this->value.i = parse_color(first, last, err);
                this->type = 'i';
                return !err;
            }
            else {
                char * s = static_cast<char*>(std::malloc(last - first + 1));
                std::memcpy(s, first, last - first + 1);
                s[last - first] = 0;
                this->value.s = s;
                this->type = 's';
                return true;
            }
        }

        ~Property()
        {
            if (this->type == 's') {
                free(const_cast<char*>(this->value.s));
            }
        }
    };
    struct Target {
        std::string name;
        std::vector<Property> properties;
        bool is_name() const{
            return this->name[0] == '#';
        }
        bool operator < (const Target& other) const {
            return this->name < other.name;
        }

        template<typename U>
        const U get(const char * s, const U& default_value) const
        {
            typedef std::vector<Property>::const_iterator iterator;
            iterator first = this->properties.begin();
            iterator last = this->properties.end();
            for (; first != last; ++first) {
                if (0 == first->name.compare(s)) {
                    return this->to(first->value, default_value);
                }
            }
            return default_value;
        }
    private:
        static int to(const Property::Value& v, int) {
            return v.i;
        }
        static const char * to(const Property::Value& v, const char *) {
            return v.s;
        }
    };
    std::vector<Target> targets;
    size_t boundary;
    const Target empty_target;

    RwlDefinition(const char * filename = 0)
        : targets()
        , boundary(0)
        , empty_target()
    {
        if (filename) {
            std::ifstream ifs(filename);
            std::string s;
            std::getline(ifs, s, '\0');
            if (!ifs.eof()) {
                throw std::runtime_error("read file error");
            }
            std::cout << (s) << std::endl;
            size_t pos = this->parse_add(s.c_str());
            if (pos != -1u) {
                std::cout << "error character" << (pos) << std::endl;
            }
        }
    }

    const Target& get(const char * s) const
    {
        typedef std::vector<Target>::const_iterator iterator;
        iterator first = this->targets.begin();
        iterator last = this->targets.end();
        for (; first != last; ++first) {
            if (0 == first->name.compare(s)) {
                return *first;
            }
        }
        return this->empty_target;
    }

    size_t parse_add(const char * s)
    {
        struct utils{
            static const char * ignorespace(const char * s){
                while (*s && std::isspace(*s)) {
                    ++s;
                }
                return s;
            }
            static const char * ignoretoken(const char * s){
                while (*s && (
                              ('a' <= *s && *s <= 'z')
                              || ('A' <= *s && *s <= 'Z')
                              || ('0' <= *s && *s <= '9')
                              || *s == '_' || *s == '-')
                       ) {
                    ++s;
                }
                return s;
            }
        };
        size_t before_size = this->targets.size();
        const char * begin_s = s;
        s = utils::ignorespace(s);
        const char * first = s;
        while (*s) {
            if (*s == '#') {
                ++s;
            }
            s = utils::ignoretoken(s);
            if (!*s) {
                return s - begin_s - 1;
            }
            Target target;
            target.name.assign(first, s);
            s = utils::ignorespace(s);
            if (*s != '{') {
                return s - begin_s;
            }
            while (1) {
                first = utils::ignorespace(s+1);
                s = utils::ignoretoken(first);
                if (!*s) {
                    return s - begin_s - 1;
                }
                if (*s == '}') {
                    break ;
                }
                Property property;
                property.name.assign(first, s);
                first = utils::ignorespace(s);
                if (*first != ':') {
                    return first - begin_s;
                }
                first = utils::ignorespace(first+1);
                s = first;
                while (*s && *s != '\n' && *s != ';' && *s != '}') {
                    ++s;
                }
                if (!*s) {
                    return s - begin_s - 1;
                }
                const char * tmps = s;
                if (*s == '\n' || *s == ';' || *s == '}') {
                    while (std::isspace(s[-1])) {
                        --s;
                    }
                }
                property.set_value(first, s);
                s = tmps;
                target.properties.push_back(property);
                if (*s == '}') {
                    break ;
                }
            }
            s = utils::ignorespace(s+1);
            first = s;
            if (!target.properties.empty()){
                this->targets.push_back(target);
            }
        }

        std::sort(this->targets.begin() + before_size, this->targets.end());
        if (before_size) {
            std::inplace_merge(this->targets.begin(),
                               this->targets.begin() + before_size,
                               this->targets.end());
        }
        size_t last = this->targets.size();
        while (this->boundary < last && this->targets[this->boundary].is_name()) {
            ++this->boundary;
        }
        return size_t(-1);
    }
};

class RwlLoginMod : public InternalMod, public NotifyApi
{
    RwlDefinition definition;
    WindowLogin window_login;
    WidgetImage image;

public:
    Inifile & ini;

public:
    RwlLoginMod(Inifile& ini, FrontAPI& front, uint16_t width, uint16_t height)
        : InternalMod(front, width, height)
        , definition("/tmp/login_mod.rwl")
        , window_login(*this, 0, 0, this->screen, this, VERSION, false, 0, 0, 0, BLACK, GREY,
                       ini.translation.button_ok.get().c_str(),
                       ini.translation.button_cancel.get().c_str(),
                       ini.translation.button_help.get().c_str(),
                       ini.translation.login.get().c_str(),
                       ini.translation.password.get().c_str())
        , image(*this, 0, 0, SHARE_PATH "/" REDEMPTION_LOGO24, this->screen, NULL)
        , ini(ini)
    {
        this->screen.add_widget(&this->image);
        this->screen.add_widget(&this->window_login);

        typedef std::vector<RwlDefinition::Target>::iterator target_iterator;
        target_iterator target_first = this->definition.targets.begin();
        target_iterator target_last = this->definition.targets.end();
        for (; target_first != target_last; ++target_first){
            std::cout << target_first->name << "'\n";
            typedef std::vector<RwlDefinition::Property>::iterator property_iterator;
            property_iterator first = target_first->properties.begin();
            property_iterator last = target_first->properties.end();
            for (; first != last; ++first) {
                std::cout << "\t'" << first->name << "': '";
                if (first->type == 'i') {
                    std::cout << first->value.i << "'\n";
                }
                else {
                    std::cout << first->value.s << "'\n";
                }
            }
        }
        std::cout.flush();

        {
            const RwlDefinition::Target& rwl_window = this->definition.get("window");
            int color = rwl_window.get<int>("bgcolor", GREY);
            this->window_login.bg_color = color;
            this->window_login.login_label.bg_color = color;
            this->window_login.password_label.bg_color = color;
            color = rwl_window.get<int>("color", BLACK);
            this->window_login.login_label.fg_color = color;
            this->window_login.password_label.fg_color = color;
        }
        {
            const RwlDefinition::Target& rwl_titlebar = this->definition.get("titlebar");
            this->window_login.titlebar.bg_color = rwl_titlebar.get<int>("bgcolor", WABGREEN);
            this->window_login.titlebar.fg_color = rwl_titlebar.get<int>("color", WHITE);
        }
        {
            const RwlDefinition::Target& rwl_titlebar = this->definition.get("window_close");
            this->window_login.button_close.label.bg_color = rwl_titlebar.get<int>("bgcolor", DARK_WABGREEN);
            this->window_login.button_close.label.fg_color = rwl_titlebar.get<int>("color", WHITE);
        }
        {
            const RwlDefinition::Target& rwl_window = this->definition.get("edit");
            int color = rwl_window.get<int>("bgcolor", WHITE);
            this->window_login.login_edit.label.bg_color = color;
            this->window_login.password_edit.masked_text.bg_color = color;
            color = rwl_window.get<int>("color", BLACK);
            this->window_login.login_edit.label.fg_color = color;
            this->window_login.password_edit.masked_text.fg_color = color;
        }
        {
            const RwlDefinition::Target& rwl_window = this->definition.get("button");
            int color = rwl_window.get<int>("bgcolor", GREY);
            this->window_login.ok.label.bg_color = color;
            this->window_login.cancel.label.bg_color = color;
            this->window_login.help.label.bg_color = color;
            color = rwl_window.get<int>("color", BLACK);
            this->window_login.ok.label.fg_color = color;
            this->window_login.cancel.label.fg_color = color;
            this->window_login.help.label.fg_color = color;
        }


        this->window_login.set_xy((width - this->window_login.cx()) / 2,
                                  (height - this->window_login.cy()) / 2);

        this->image.rect.x = width - this->image.cx();
        this->image.rect.y = height - this->image.cy();

        if (this->ini.context_is_asked(AUTHID_TARGET_USER)
            ||  this->ini.context_is_asked(AUTHID_TARGET_DEVICE)){
            if (this->ini.context_is_asked(AUTHID_AUTH_USER)){
                this->ini.account.username[0] = 0;
            }
            else {
                strncpy(this->ini.account.username,
                        this->ini.globals.target_user.get_cstr(),
                        // this->ini.context_get_value(AUTHID_AUTH_USER, NULL, 0),
                        sizeof(this->ini.account.username));
                this->ini.account.username[sizeof(this->ini.account.username) - 1] = 0;
            }
        }
        else if (this->ini.context_is_asked(AUTHID_AUTH_USER)) {
            this->ini.account.username[0] = 0;
        }
        else {
            TODO("check this! Assembling parts to get user login with target is not obvious"
                 "method used below il likely to show @: if target fields are empty")
                char buffer[256];
            // snprintf( buffer, 256, "%s@%s:%s%s%s"
            //           , this->ini.context_get_value(AUTHID_TARGET_USER, NULL, 0)
            //           , this->ini.context_get_value(AUTHID_TARGET_DEVICE, NULL, 0)
            //           , (this->ini.context_get_value(AUTHID_TARGET_PROTOCOL, NULL, 0)[0] ?
            //              this->ini.context_get_value(AUTHID_TARGET_PROTOCOL, NULL, 0) : "")
            //           , (this->ini.context_get_value(AUTHID_TARGET_PROTOCOL, NULL, 0)[0] ? ":" : "")
            //           , this->ini.context_get_value(AUTHID_AUTH_USER, NULL, 0)
            //           );
            snprintf( buffer, 256, "%s@%s:%s%s%s"
                      , this->ini.globals.target_user.get_cstr()
                      , this->ini.globals.target_device.get_cstr()
                      , this->ini.context.target_protocol.get_cstr()
                      , (!this->ini.context.target_protocol.is_empty() ? ":" : "")
                      , this->ini.globals.auth_user.get_cstr()
                      );
            strcpy(this->ini.account.username, buffer);
        }

        if (this->ini.account.password[0]) {
            this->window_login.password_edit.set_text(this->ini.account.username);
        }

        Widget2 * focus_in_window;
        if (this->ini.account.username[0]) {
            focus_in_window = &this->window_login.password_edit;
            this->window_login.login_edit.set_text(this->ini.account.username);
        } else {
            focus_in_window = &this->window_login.login_edit;
        }

        this->window_login.set_widget_focus(focus_in_window);
        this->screen.set_widget_focus(&this->window_login);

        this->screen.refresh(this->screen.rect);
    }

    virtual ~RwlLoginMod()
    {
        this->screen.clear();
    }

    virtual void notify(Widget2* sender, notify_event_t event)
    {
        switch (event) {
        case NOTIFY_SUBMIT:
            this->ini.parse_username(this->window_login.login_edit.label.buffer);
            this->ini.context_set_value(AUTHID_PASSWORD, this->window_login.password_edit.get_text());
            this->event.signal = BACK_EVENT_NEXT;
            this->event.set();
            break;
        case NOTIFY_CANCEL:
            this->event.signal = BACK_EVENT_STOP;
            this->event.set();
            break;
        default:
            break;
        }
    }

    virtual void rdp_input_synchronize(uint32_t /*time*/, uint16_t /*device_flags*/,
                                       int16_t /*param1*/, int16_t /*param2*/)
    {}

    virtual void draw_event(time_t now)
    {
        this->event.reset();
    }
};

#endif
