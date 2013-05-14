/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010-2012
   Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_ACL_MODCONTEXT_HPP_
#define _REDEMPTION_ACL_MODCONTEXT_HPP_

#include <string>

#include "config.hpp"
#include "log.hpp"
#include "dico.hpp"

using namespace std;

typedef enum{
    AUTHID_UNKNOWN = 0,
    AUTHID_TARGET_USER,     // target_login
    AUTHID_TARGET_PASSWORD, // target_password
    AUTHID_HOST,            // ip_client
    AUTHID_PASSWORD,        // password
    AUTHID_AUTH_USER,       // login
    AUTHID_TARGET_DEVICE,   // target_device
    AUTHID_TARGET_PORT,     // target_port
    AUTHID_TARGET_PROTOCOL, // proto_dest
    AUTHID_AUTHCHANNEL_TARGET, // WabLauncher target request
    AUTHID_AUTHCHANNEL_RESULT, // WabLauncher session result
    AUTHID_AUTHCHANNEL_ANSWER, // WabLauncher target answer
    AUTHID_END_TIME,        // end time as text
    AUTHID_SELECTOR_GROUP_FILTER, // group filter text
    AUTHID_SELECTOR_DEVICE_FILTER, // device filter text
    AUTHID_SELECTOR_LINES_PER_PAGE, // number of lines per page
    AUTHID_SELECTOR_NUMBER_OF_PAGES, // number of pages
    AUTHID_SELECTOR_CURRENT_PAGE, // current page
    AUTHID_REJECTED,        // rejected
    AUTHID_SESSION_ID,      // session_id
    AUTHID_END_DATE_CNX,    // timeclose
    AUTHID_MESSAGE, // warning_message
    AUTHID_OPT_BITRATE,     // Bit rate for video encoding
    AUTHID_OPT_FRAMERATE,   // Frame rate for video encoding
    AUTHID_OPT_QSCALE,      // QScale parameter for vdeo encoding
    AUTHID_OPT_WIDTH,       // client width
    AUTHID_OPT_HEIGHT,      // client height
    AUTHID_OPT_BPP,         // bits per planes (number of colors)
    AUTHID_DISPLAY_MESSAGE, // display a dialog box with a message
    AUTHID_ACCEPT_MESSAGE,  // display a dialog to valid a message
    AUTHID_AUTH_ERROR_MESSAGE,
    AUTHID_PROXY_TYPE,
    AUTHID_AUTHENTICATED,
    AUTHID_SELECTOR,
    AUTHID_KEEPALIVE,

    AUTHID_MODE_CONSOLE,
    AUTHID_TIMEZONE,
    // Encryption
    AUTHID_TRACE_SEAL,          // after closing trace file trace is sealed using a signature hash
    MAX_AUTHID

/*
    // Translation text
    AUTHID_TRANS_BUTTON_OK,
    AUTHID_TRANS_BUTTON_CANCEL,
    AUTHID_TRANS_BUTTON_HELP,
    AUTHID_TRANS_BUTTON_CLOSE,
    AUTHID_TRANS_BUTTON_REFUSED,
    AUTHID_TRANS_LOGIN,
    AUTHID_TRANS_USERNAME,
    AUTHID_TRANS_PASSWORD,
    AUTHID_TRANS_TARGET,
    AUTHID_TRANS_DIAGNOSTIC,
    AUTHID_TRANS_CONNECTION_CLOSED,
    AUTHID_TRANS_HELP_MESSAGE,
    // Options
    AUTHID_OPT_CLIPBOARD,         // clipboard
    AUTHID_OPT_DEVICEREDIRECTION, // device_redirection
    AUTHID_OPT_FILE_ENCRYPTION,   // file encryption
    // Video capture
    AUTHID_OPT_CODEC_ID,    // CODEC_ID for video encoding
    AUTHID_OPT_MOVIE,       // is_rec
    AUTHID_OPT_MOVIE_PATH,  // rec_path
    AUTHID_VIDEO_QUALITY,
    // Alternate shell
    AUTHID_ALTERNATE_SHELL,
    AUTHID_SHELL_WORKING_DIRECTORY,
*/
} authid_t;


#define STRAUTHID_TARGET_USER              "target_login"
#define STRAUTHID_TARGET_PASSWORD          "target_password"
#define STRAUTHID_HOST                     "ip_client"
#define STRAUTHID_PASSWORD                 "password"
#define STRAUTHID_AUTH_USER                "login"
#define STRAUTHID_TARGET_DEVICE            "target_device"
#define STRAUTHID_TARGET_PORT              "target_port"
#define STRAUTHID_TARGET_PROTOCOL          "proto_dest"
#define STRAUTHID_AUTHCHANNEL_TARGET       "auth_channel_target"
#define STRAUTHID_AUTHCHANNEL_RESULT       "auth_channel_result"
TODO("This is not a translation but auth_channel answer, change key name in sesman")
#define STRAUTHID_AUTHCHANNEL_ANSWER       "trans_auth_channel"
#define STRAUTHID_END_TIME                 "end_time"
#define STRAUTHID_SELECTOR_GROUP_FILTER    "selector_group_filter"
#define STRAUTHID_SELECTOR_DEVICE_FILTER   "selector_device_filter"
#define STRAUTHID_SELECTOR_LINES_PER_PAGE  "selector_lines_per_page"
#define STRAUTHID_SELECTOR_NUMBER_OF_PAGES "selector_number_of_pages"
#define STRAUTHID_SELECTOR_CURRENT_PAGE    "selector_current_page"
#define STRAUTHID_REJECTED                 "rejected"
#define STRAUTHID_SESSION_ID               "session_id"
#define STRAUTHID_END_DATE_CNX             "timeclose"
#define STRAUTHID_MESSAGE                  "message"
#define STRAUTHID_OPT_BITRATE              "bitrate"
#define STRAUTHID_OPT_FRAMERATE            "framerate"
#define STRAUTHID_OPT_QSCALE               "qscale"
#define STRAUTHID_OPT_WIDTH                "width"
#define STRAUTHID_OPT_HEIGHT               "height"
#define STRAUTHID_OPT_BPP                  "bpp"
#define STRAUTHID_DISPLAY_MESSAGE          "display_message"
#define STRAUTHID_ACCEPT_MESSAGE           "accept_message"
#define STRAUTHID_AUTH_ERROR_MESSAGE       "error_message"
#define STRAUTHID_PROXY_TYPE               "proxy_type"
#define STRAUTHID_AUTHENTICATED            "authenticated"
#define STRAUTHID_SELECTOR                 "selector"
#define STRAUTHID_KEEPALIVE                "keepalive"
#define STRAUTHID_MODE_CONSOLE             "mode_console"
#define STRAUTHID_TIMEZONE                 "timezone"
#define STRAUTHID_TRACE_SEAL               "trace_seal"
// Translation text
#define _STRAUTHID_TRANS_BUTTON_OK         "trans_ok"
#define _STRAUTHID_TRANS_BUTTON_CANCEL     "trans_cancel"
#define _STRAUTHID_TRANS_BUTTON_HELP       "trans_help"
#define _STRAUTHID_TRANS_BUTTON_CLOSE      "trans_close"
#define _STRAUTHID_TRANS_BUTTON_REFUSED    "trans_refused"
#define _STRAUTHID_TRANS_LOGIN             "trans_login"
#define _STRAUTHID_TRANS_USERNAME          "trans_username"
#define _STRAUTHID_TRANS_PASSWORD          "trans_password"
#define _STRAUTHID_TRANS_TARGET            "trans_target"
#define _STRAUTHID_TRANS_DIAGNOSTIC        "trans_diagnostic"
#define _STRAUTHID_TRANS_CONNECTION_CLOSED "trans_connection_closed"
#define _STRAUTHID_TRANS_HELP_MESSAGE      "trans_help_message"
// Options
#define _STRAUTHID_OPT_CLIPBOARD           "clipboard"
#define _STRAUTHID_OPT_DEVICEREDIRECTION   "device_redirection"
#define _STRAUTHID_OPT_FILE_ENCRYPTION     "file_encryption"
// Video capture
#define _STRAUTHID_OPT_CODEC_ID             "codec_id"
#define _STRAUTHID_OPT_MOVIE                "is_rec"
#define _STRAUTHID_OPT_MOVIE_PATH           "rec_path"
#define _STRAUTHID_VIDEO_QUALITY            "video_quality"
// Alternate shell
#define _STRAUTHID_ALTERNATE_SHELL          "alternate_shell"
#define _STRAUTHID_SHELL_WORKING_DIRECTORY  "shell_working_directory"


#define GLOBAL_SECTION_UNKNOWN      NULL
#define GLOBAL_SECTION_CLIENT       "client"
#define GLOBAL_SECTION_GLOBALS      "globals"
#define GLOBAL_SECTION_TRANSLATION  "translation"


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

TODO("This should be initialized in constructor")
static ProtocolKeyword KeywordsDefinitions[] = {
    {STRAUTHID_AUTH_ERROR_MESSAGE,       TYPE_TEXT,    "!"                                    },
    {STRAUTHID_TARGET_USER,              TYPE_TEXT,    "!"                                    },
    {STRAUTHID_TARGET_PASSWORD,          TYPE_TEXT,    "!"                                    },
    {STRAUTHID_HOST,                     TYPE_TEXT,    "!"                                    },
    {STRAUTHID_PASSWORD,                 TYPE_TEXT,    "!"                                    },
    {STRAUTHID_AUTH_USER,                TYPE_TEXT,    "!"                                    },
    {STRAUTHID_TARGET_DEVICE,            TYPE_TEXT,    "!"                                    },
    {STRAUTHID_TARGET_PORT,              TYPE_INTEGER, "!3389"                                },
    {STRAUTHID_TARGET_PROTOCOL,          TYPE_TEXT,    "!RDP"                                 },
    {STRAUTHID_AUTHCHANNEL_TARGET,       TYPE_TEXT,    "!"                                    },
    {STRAUTHID_AUTHCHANNEL_RESULT,       TYPE_TEXT,    "!"                                    },
    {STRAUTHID_AUTHCHANNEL_ANSWER,       TYPE_TEXT,    "!"                                    },

    {STRAUTHID_END_TIME,                 TYPE_TEXT,    "!-"                                   },
    {STRAUTHID_SELECTOR_GROUP_FILTER,    TYPE_TEXT,    "!"                                    },
    {STRAUTHID_SELECTOR_DEVICE_FILTER,   TYPE_TEXT,    "!"                                    },
    {STRAUTHID_SELECTOR_LINES_PER_PAGE,  TYPE_TEXT,    "!20"                                  },
    {STRAUTHID_SELECTOR_NUMBER_OF_PAGES, TYPE_TEXT,    "!"                                    },
    {STRAUTHID_SELECTOR_CURRENT_PAGE,    TYPE_TEXT,    "!1"                                   },
    {STRAUTHID_MESSAGE,                  TYPE_TEXT,    "!"                                    },
    {STRAUTHID_DISPLAY_MESSAGE,          TYPE_TEXT,    "!"                                    },
    {STRAUTHID_ACCEPT_MESSAGE,           TYPE_TEXT,    "!"                                    },
    {STRAUTHID_OPT_WIDTH,                TYPE_INTEGER, "!800"                                 },
    {STRAUTHID_OPT_HEIGHT,               TYPE_INTEGER, "!600"                                 },
    {STRAUTHID_OPT_BPP,                  TYPE_INTEGER, "!24"                                  },
    {STRAUTHID_PROXY_TYPE,               TYPE_TEXT,    "!RDP"                                 },
    {STRAUTHID_AUTHENTICATED,            TYPE_BOOLEAN, "!False"                               },
    {STRAUTHID_SELECTOR,                 TYPE_BOOLEAN, "!False"                               },
    {STRAUTHID_KEEPALIVE,                TYPE_BOOLEAN, "ASK"                                  },
    {STRAUTHID_END_DATE_CNX,             TYPE_INTEGER, "!0"                                   },
    {STRAUTHID_OPT_BITRATE,              TYPE_INTEGER, "!40000"                               },
    {STRAUTHID_OPT_FRAMERATE,            TYPE_INTEGER, "!5"                                   },
    {STRAUTHID_OPT_QSCALE,               TYPE_INTEGER, "!15"                                  },
    {STRAUTHID_REJECTED,                 TYPE_TEXT,    "!Connection refused by authentifier." },
    {STRAUTHID_SESSION_ID,               TYPE_TEXT,    "!"                                    },
    // password or AuthenticationInteractive
    {"authentication_challenge",         TYPE_TEXT,    "!password"                            },

    {STRAUTHID_MODE_CONSOLE,             TYPE_TEXT,    "!allow"                               },
    {STRAUTHID_TIMEZONE,                 TYPE_INTEGER, "!-3600"                               },
    // Encryption
    {STRAUTHID_TRACE_SEAL,               TYPE_TEXT,    "!"                                    },

/*
    // Translation text
    {STRAUTHID_TRANS_BUTTON_OK,          TYPE_TEXT,    "!OK"                                  },
    {STRAUTHID_TRANS_BUTTON_CANCEL,      TYPE_TEXT,    "!Cancel"                              },
    {STRAUTHID_TRANS_BUTTON_HELP,        TYPE_TEXT,    "!Help"                                },
    {STRAUTHID_TRANS_BUTTON_CLOSE,       TYPE_TEXT,    "!Close"                               },
    {STRAUTHID_TRANS_BUTTON_REFUSED,     TYPE_TEXT,    "!Refused"                             },
    {STRAUTHID_TRANS_LOGIN,              TYPE_TEXT,    "!login"                               },
    {STRAUTHID_TRANS_USERNAME,           TYPE_TEXT,    "!username"                            },
    {STRAUTHID_TRANS_PASSWORD,           TYPE_TEXT,    "!password"                            },
    {STRAUTHID_TRANS_TARGET,             TYPE_TEXT,    "!target"                              },
    {STRAUTHID_TRANS_DIAGNOSTIC,         TYPE_TEXT,    "!diagnostic"                          },
    {STRAUTHID_TRANS_CONNECTION_CLOSED,  TYPE_TEXT,    "!Connection closed"                   },
    {STRAUTHID_TRANS_HELP_MESSAGE,       TYPE_TEXT,    "!Help message"                        },
    // Options
    {STRAUTHID_OPT_CLIPBOARD,            TYPE_BOOLEAN, "!True"                                },
    {STRAUTHID_OPT_DEVICEREDIRECTION,    TYPE_BOOLEAN, "!True"                                },
    {STRAUTHID_OPT_FILE_ENCRYPTION,      TYPE_BOOLEAN, "!False"                               },
    // Video capture
    {STRAUTHID_OPT_CODEC_ID,             TYPE_TEXT,    "!flv"                                 },
    {STRAUTHID_OPT_MOVIE,                TYPE_BOOLEAN, "!False"                               },
    {STRAUTHID_OPT_MOVIE_PATH,           TYPE_TEXT,    "!"                                    },
    {STRAUTHID_VIDEO_QUALITY,            TYPE_TEXT,    "!medium"                              },
    // Alternate shell
    {STRAUTHID_ALTERNATE_SHELL,          TYPE_TEXT,    "!"                                    },
    {STRAUTHID_SHELL_WORKING_DIRECTORY,  TYPE_TEXT,    "!"                                    },
*/
};


static inline authid_t authid_from_string(const char * kw)
{
    static const string authstr[MAX_AUTHID-1] = {
    STRAUTHID_TARGET_USER,
    STRAUTHID_TARGET_PASSWORD,
    STRAUTHID_HOST,
    STRAUTHID_PASSWORD,
    STRAUTHID_AUTH_USER,
    STRAUTHID_TARGET_DEVICE,
    STRAUTHID_TARGET_PORT,
    STRAUTHID_TARGET_PROTOCOL,
    STRAUTHID_AUTHCHANNEL_TARGET,
    STRAUTHID_AUTHCHANNEL_RESULT,
    STRAUTHID_AUTHCHANNEL_ANSWER, // WabLauncher target answer
    STRAUTHID_END_TIME,
    STRAUTHID_SELECTOR_GROUP_FILTER,
    STRAUTHID_SELECTOR_DEVICE_FILTER,
    STRAUTHID_SELECTOR_LINES_PER_PAGE,
    STRAUTHID_SELECTOR_NUMBER_OF_PAGES,
    STRAUTHID_SELECTOR_CURRENT_PAGE,
    STRAUTHID_REJECTED,
    STRAUTHID_END_DATE_CNX,
    STRAUTHID_MESSAGE,
    STRAUTHID_OPT_BITRATE,
    STRAUTHID_OPT_FRAMERATE,
    STRAUTHID_OPT_QSCALE,
    STRAUTHID_OPT_WIDTH,
    STRAUTHID_OPT_HEIGHT,
    STRAUTHID_OPT_BPP,
    STRAUTHID_DISPLAY_MESSAGE,
    STRAUTHID_ACCEPT_MESSAGE,
    STRAUTHID_AUTH_ERROR_MESSAGE,
    STRAUTHID_PROXY_TYPE,
    STRAUTHID_AUTHENTICATED,
    STRAUTHID_SELECTOR,
    STRAUTHID_KEEPALIVE,

    STRAUTHID_MODE_CONSOLE,
    STRAUTHID_TIMEZONE,
    STRAUTHID_TRACE_SEAL,

/*
    // Translation text
    STRAUTHID_TRANS_BUTTON_OK,
    STRAUTHID_TRANS_BUTTON_CANCEL,
    STRAUTHID_TRANS_BUTTON_HELP,
    STRAUTHID_TRANS_BUTTON_CLOSE,
    STRAUTHID_TRANS_BUTTON_REFUSED,
    STRAUTHID_TRANS_LOGIN,
    STRAUTHID_TRANS_USERNAME,
    STRAUTHID_TRANS_PASSWORD,
    STRAUTHID_TRANS_TARGET,
    STRAUTHID_TRANS_DIAGNOSTIC,
    STRAUTHID_TRANS_CONNECTION_CLOSED,
    STRAUTHID_TRANS_HELP_MESSAGE,
    // Options
    STRAUTHID_OPT_FILE_ENCRYPTION,
    STRAUTHID_OPT_CLIPBOARD,
    STRAUTHID_OPT_DEVICEREDIRECTION,
    // Video capture
    STRAUTHID_OPT_CODEC_ID,
    STRAUTHID_OPT_MOVIE,
    STRAUTHID_OPT_MOVIE_PATH,
    STRAUTHID_VIDEO_QUALITY,
    STRAUTHID_ALTERNATE_SHELL,
    STRAUTHID_SHELL_WORKING_DIRECTORY,
*/
    };

    string str = string(kw);
    authid_t res = AUTHID_UNKNOWN;
    for (int i = 0; i < MAX_AUTHID-1 ; i++){
        if (0 == authstr[i].compare(str)){
            res = (authid_t)(i+1);
            break;
        }
    }
    return res;
}

struct ModContext : public Dico {
    unsigned selector_focus;
    char movie[1024];

    public:
    ModContext()
        : Dico(KeywordsDefinitions, sizeof(KeywordsDefinitions)/sizeof(ProtocolKeyword))
    {
        this->selector_focus = 0;
    }

    ~ModContext(){
//        LOG(LOG_INFO, "mod context deleted");
    }

    void parse_username(const char * username)
    {
//        LOG(LOG_INFO, "parse_username(%s)", username);
        TODO("These should be results of the parsing function, not storing it away immediately in context. Mixing context management and parsing is not right")
        char target_user[256];
        char target_device[256];
        char target_protocol[256];
        char auth_user[256];
        target_user[0] = 0;
        target_device[0] = 0;
        target_protocol[0] = 0;
        auth_user[0] = 0;

        this->ask(STRAUTHID_SELECTOR);
        LOG(LOG_INFO, "asking for selector");

        if (username[0]){
            unsigned itarget_user = 0;
            unsigned itarget_device = 0;
            unsigned iprotocol = 0;
            unsigned iauthuser = 0;
            // well if that is not obvious the code below this
            // is a finite state automata that split login@host:protocol:authuser
            // between it's components parts.
            // ':' is forbidden in login, host or authuser.
            // '@' is forbidden in host or authuser.
            // login can contain an @ character (necessary because it is used
            // for domain names), the rule is that host follow the last @,
            // the login is what is before, even if it contains an @.
            // the protocol is what follows the first :
            // the user is what follows the second :, or what follows the unique : (if only one is found)

            enum { COPY_TARGET_USER
                 , COPY_HOST
                 , COPY_AUTHUSER
            } state = COPY_TARGET_USER;

            unsigned c;

            for (unsigned i = 0; i < 255 && (c = username[i]); i++){
                switch (state) {
                case COPY_TARGET_USER:
                    switch (c){
                    case ':': state = COPY_AUTHUSER;
                    break;
                    case '@': state = COPY_HOST;
                    break;
                    default: target_user[itarget_user++] = c;
                    break;
                    }
                break;
                case COPY_HOST:
                    switch (c){
                        case ':': state = COPY_AUTHUSER;
                           break;
                        case '@':
                            target_user[itarget_user++] = '@';
                            memcpy(target_user+itarget_user, target_device, itarget_device);
                            itarget_user += itarget_device;
                            itarget_device = 0;
                            break;
                        default: target_device[itarget_device++] = c;
                         break;
                    }
                break;
                case COPY_AUTHUSER:
                    switch (c){
                        case ':': // second ':' means we had 'protocol:user' pair
                            memcpy(target_protocol, auth_user, iauthuser);
                            iprotocol = iauthuser;
                            iauthuser = 0;
                            break;
                        default: auth_user[iauthuser++] = c;
                            break;
                    }
                break;
                }
            }
            target_user[itarget_user] = 0;
            target_device[itarget_device] = 0;
            target_protocol[iprotocol] = 0;
            auth_user[iauthuser] = 0;
            if (iauthuser == 0){
                if ((itarget_user > 0) && (itarget_device == 0)){
                    memcpy(auth_user, target_user, itarget_user);
                    target_user[0] = 0;
                    auth_user[itarget_user] = 0;
                }
                if ((itarget_user > 0) && (itarget_device > 0)){
                    memcpy(auth_user, target_user, itarget_user);
                    target_user[0] = 0;
                    auth_user[itarget_user] = '@';
                    memcpy(auth_user + 1 + itarget_user, target_device, itarget_device);
                    target_device[0] = 0;
                    auth_user[itarget_user + 1 + itarget_device] = 0;
                }
            }
            // 'win:user' means user@win:user
            else if ((itarget_user != 0) && (itarget_device == 0)){
                memcpy(target_device, target_user, itarget_user);
                target_device[itarget_user] = 0;
                memcpy(target_user, auth_user, iauthuser);
                target_user[iauthuser] = 0;
            }
        }

        if (*target_user == 0)
        {
            this->ask(STRAUTHID_TARGET_USER);
        }
        else {
            this->cpy(STRAUTHID_TARGET_USER, target_user);
        }
        if (*target_device == 0) {
            this->ask(STRAUTHID_TARGET_DEVICE);
        }
        else {
            this->cpy(STRAUTHID_TARGET_DEVICE, target_device);
        }
        if (*target_protocol == 0) {
            this->ask(STRAUTHID_TARGET_PROTOCOL);
        }
        else {
            this->cpy(STRAUTHID_TARGET_PROTOCOL, target_protocol);
        }
        if (*auth_user == 0) {
            this->ask(STRAUTHID_AUTH_USER);
        }
        else {
            this->cpy(STRAUTHID_AUTH_USER, auth_user);
        }
    }
};

#endif
