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

#include <string>
#include <string.h>

#include <map>
#include <set>

enum {
    MCTX_STATUS_EXIT,
    MCTX_STATUS_WAITING,
    MCTX_STATUS_LOGIN,
    MCTX_STATUS_CLOSE,
    MCTX_STATUS_DIALOG,
    MCTX_STATUS_VNC,
    MCTX_STATUS_RDP,
    MCTX_STATUS_XUP,
    MCTX_STATUS_BOUNCER,
    MCTX_STATUS_TRANSITORY,
    MCTX_STATUS_AUTH,
    MCTX_STATUS_CLI,
};

enum {
    MOD_STATE_INIT,
    MOD_STATE_RECEIVED_CREDENTIALS,
    MOD_STATE_DISPLAY_MESSAGE,
    MOD_STATE_VALID_MESSAGE,
    MOD_STATE_LOGIN_BOX,
    MOD_STATE_PASSWORD_BOX,
    MOD_STATE_CONNECTED_RDP,
    MOD_STATE_CONNECTED_VNC,
    MOD_STATE_CLOSE,
    MOD_STATE_MESSAGE_CONNEXION_CLOSE_AT_LIMIT
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

struct ModContext : public Dico {
    int mod_state;
    int wab_auth;

    public:
    ModContext(ProtocolKeyword * KeywordsDefinitions, unsigned nbkeywords) :
        Dico(KeywordsDefinitions, nbkeywords)
    {
    }

    ~ModContext(){
//        LOG(LOG_INFO, "mod context deleted");
    }

    void parse_username(const char * username)
    {
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
            char * target_user = this->get(STRAUTHID_TARGET_USER);
            char * target_device = this->get(STRAUTHID_TARGET_DEVICE);
            char * auth_user = this->get(STRAUTHID_AUTH_USER);

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
    }
};

#endif
