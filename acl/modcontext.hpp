/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni

*/

#if !defined(__MODCONTEXT_HPP__)
#define __MODCONTEXT_HPP__

#include "config.hpp"
#include "log.hpp"
#include "dico.hpp"

enum {
    MCTX_STATUS_EXIT,
    MCTX_STATUS_WAITING,
    MCTX_STATUS_VNC,
    MCTX_STATUS_RDP,
    MCTX_STATUS_XUP,
    MCTX_STATUS_INTERNAL,
    MCTX_STATUS_TRANSITORY,
    MCTX_STATUS_AUTH,
    MCTX_STATUS_CLI,
};

// ModContext structure is used for modules to communicate with each other

// status tell to session what should be done when a module terminates
// either exit session or continue using another module.

// context should contains data necessary to run the module like username,
// password, ip address and port, etc. It is the responsibility of the previous
// module to set both status and all relevant information for the user module
// (ie: not of the session).

// At startup session will setup the initial context from configuration files.
// (it could also become the role of the initial null module that would allow
// to simplify further session code)

static ProtocolKeyword KeywordsDefinitions[] = {
    {STRAUTHID_OPT_MOVIE_PATH, TYPE_TEXT, "!"},
    {STRAUTHID_AUTH_ERROR_MESSAGE, TYPE_TEXT, "!"},
    {STRAUTHID_TARGET_USER, TYPE_TEXT, "!"},
    {STRAUTHID_TARGET_PASSWORD, TYPE_TEXT, "!"},
    {STRAUTHID_HOST, TYPE_TEXT, "!"},
    {STRAUTHID_PASSWORD, TYPE_TEXT, "!"},
    {STRAUTHID_AUTH_USER, TYPE_TEXT, "!"},
    {STRAUTHID_TARGET_DEVICE, TYPE_TEXT, "!"},
    {STRAUTHID_TARGET_PORT, TYPE_INTEGER, "!3389"},
    {STRAUTHID_TARGET_PROTOCOL, TYPE_TEXT, "!RDP"},
    {STRAUTHID_END_TIME, TYPE_TEXT, "!-"},
    {STRAUTHID_SELECTOR_GROUP_FILTER, TYPE_TEXT, "!"},
    {STRAUTHID_SELECTOR_DEVICE_FILTER, TYPE_TEXT, "!"},
    {STRAUTHID_SELECTOR_LINES_PER_PAGE, TYPE_TEXT, "!20"},
    {STRAUTHID_SELECTOR_NUMBER_OF_PAGES, TYPE_TEXT, "!"},
    {STRAUTHID_SELECTOR_CURRENT_PAGE, TYPE_TEXT, "!1"},
    {STRAUTHID_OPT_MOVIE, TYPE_BOOLEAN, "!False"},
    {STRAUTHID_OPT_CLIPBOARD, TYPE_BOOLEAN, "!True"},
    {STRAUTHID_OPT_DEVICEREDIRECTION, TYPE_BOOLEAN, "!True"},
    {STRAUTHID_MESSAGE, TYPE_TEXT, "!"},
    {STRAUTHID_DISPLAY_MESSAGE, TYPE_TEXT, "!"},
    {STRAUTHID_ACCEPT_MESSAGE, TYPE_TEXT, "!"},
    {STRAUTHID_OPT_WIDTH, TYPE_INTEGER, "!800"},
    {STRAUTHID_OPT_HEIGHT, TYPE_INTEGER, "!600"},
    {STRAUTHID_OPT_BPP, TYPE_INTEGER, "!24"},
    {STRAUTHID_PROXY_TYPE, TYPE_TEXT, "!RDP"},
    {STRAUTHID_AUTHENTICATED, TYPE_BOOLEAN, "!False"},
    {STRAUTHID_SELECTOR, TYPE_BOOLEAN, "!False"},
    {STRAUTHID_KEEPALIVE, TYPE_BOOLEAN, "ASK"},
    {STRAUTHID_END_DATE_CNX, TYPE_INTEGER, "!0"},
    {STRAUTHID_OPT_BITRATE, TYPE_INTEGER, "!40000"},
    {STRAUTHID_OPT_FRAMERATE, TYPE_INTEGER, "!5"},
    {STRAUTHID_OPT_QSCALE, TYPE_INTEGER, "!15"},
    {STRAUTHID_OPT_CODEC_ID, TYPE_TEXT, "!flv"},
    {STRAUTHID_REJECTED, TYPE_TEXT, "!Connection refused by authentifier."},
    // password or AuthenticationInteractive
    {"authentication_challenge", TYPE_TEXT, "!password"},
    // Translation
    {STRAUTHID_TRANS_BUTTON_OK, TYPE_TEXT, "!OK"},
    {STRAUTHID_TRANS_BUTTON_CANCEL, TYPE_TEXT, "!Cancel"},
    {STRAUTHID_TRANS_BUTTON_HELP, TYPE_TEXT, "!Help"},
    {STRAUTHID_TRANS_BUTTON_CLOSE, TYPE_TEXT, "!Close"},
    {STRAUTHID_TRANS_BUTTON_REFUSED, TYPE_TEXT, "!Refused"},
    {STRAUTHID_TRANS_LOGIN, TYPE_TEXT, "!login"},
    {STRAUTHID_TRANS_USERNAME, TYPE_TEXT, "!username"},
    {STRAUTHID_TRANS_PASSWORD, TYPE_TEXT, "!password"},
    {STRAUTHID_TRANS_TARGET, TYPE_TEXT, "!target"},
    {STRAUTHID_TRANS_DIAGNOSTIC, TYPE_TEXT, "!diagnostic"},
    {STRAUTHID_TRANS_CONNECTION_CLOSED, TYPE_TEXT, "!Connection closed"},
    {STRAUTHID_TRANS_HELP_MESSAGE, TYPE_TEXT, "!Help message"},
    {STRAUTHID_MODE_CONSOLE, TYPE_TEXT, "!allow"},
    {STRAUTHID_VIDEO_QUALITY, TYPE_TEXT, "!medium"},
    {STRAUTHID_TIMEZONE, TYPE_INTEGER, "!-3600"},
};


struct ModContext : public Dico {
    unsigned selector_focus;
    enum {
        INTERNAL_NONE,
        INTERNAL_LOGIN,
        INTERNAL_DIALOG_DISPLAY_MESSAGE,
        INTERNAL_DIALOG_VALID_MESSAGE,
        INTERNAL_CLOSE,
        INTERNAL_SELECTOR,
        INTERNAL_BOUNCER2,
        INTERNAL_TEST,
        INTERNAL_CARD,
    } nextmod;

    char movie[1024];

    public:
    ModContext() 
        : Dico(KeywordsDefinitions, sizeof(KeywordsDefinitions)/sizeof(ProtocolKeyword))
        , nextmod(INTERNAL_NONE)
    {
        this->selector_focus = 0;
    }

    ~ModContext(){
//        LOG(LOG_INFO, "mod context deleted");
    }

    void parse_username(const char * username)
    {
//        LOG(LOG_INFO, "parse_username(%s)", username);
        char target_user[256];
        char target_device[256];
        char auth_user[256];
        target_user[0] = 0;
        target_device[0] = 0;
        auth_user[0] = 0;

        this->ask(STRAUTHID_SELECTOR);
        LOG(LOG_INFO, "asking for selector");

        if (username[0]){
            unsigned iusername = 0;
            unsigned ihost = 0;
            unsigned iauthuser = 0;
            // well if that is not obvious the code below this
            // is a finite state automata that split login@host:authuser
            // between it's components parts.
            // ':' is forbidden in login, host or authuser.
            // '@' is forbidden in host or authuser.
            // login can contain an @ character (necessary because it is used
            // for domain names), the rule is that host follow the last @,
            // the login is what is before, even if it contains an @.
            enum {
                COPY_USERNAME,
                COPY_HOST,
                COPY_AUTHUSER
            } state = COPY_USERNAME;
            unsigned c;

            for (unsigned i = 0; i < 255 && (c = username[i]); i++){
                switch (state) {
                case COPY_USERNAME:
                    switch (c){
                    case ':': state = COPY_AUTHUSER;
                    break;
                    case '@': state = COPY_HOST;
                    break;
                    default: target_user[iusername++] = c;
                    break;
                    }
                break;
                case COPY_HOST:
                    switch (c){
                    case ':': state = COPY_AUTHUSER;
                    break;
                    case '@':
                        target_user[iusername++] = '@';
                        memcpy(target_user+iusername, target_device, ihost);
                        iusername += ihost;
                        ihost = 0;
                    break;
                    default: target_device[ihost++] = c;
                    break;
                    }
                break;
                case COPY_AUTHUSER:
                     auth_user[iauthuser++] = c;
                break;
                }
            }
            target_user[iusername] = 0;
            target_device[ihost] = 0;
            auth_user[iauthuser] = 0;
            if ((iusername > 0) && (ihost == 0) && (iauthuser == 0)){
                memcpy(auth_user, target_user, iusername);
                auth_user[iusername] = 0;
                target_user[0] = 0;
            }
        }
        if (!*target_user)
        {
            this->ask(STRAUTHID_TARGET_USER);
        }
        else {
            this->cpy(STRAUTHID_TARGET_USER, target_user);
        }
        if (!*target_device) {
            this->ask(STRAUTHID_TARGET_DEVICE);
        }
        else {
            this->cpy(STRAUTHID_TARGET_DEVICE, target_device);
        }
        if (!*auth_user) {
            this->ask(STRAUTHID_AUTH_USER);
        }
        else {
            this->cpy(STRAUTHID_AUTH_USER, auth_user);
        }
    }
};

#endif
