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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan

   configuration file,
   parsing config file rdpproxy.ini
*/

#ifndef _REDEMPTION_CORE_CONFIG_HPP_
#define _REDEMPTION_CORE_CONFIG_HPP_

#include <dirent.h>
#include <stdio.h>

#include "log.hpp"

#include <istream>
#include <string>
#include <stdint.h>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#include <string.hpp>

using namespace std;

/*
static inline bool bool_from_string(string str)
{
    return (boost::iequals(string("1"),str))
        || (boost::iequals(string("yes"),str))
        || (boost::iequals(string("on"),str))
        || (boost::iequals(string("true"),str));
}
*/

static inline bool bool_from_cstr(const char * str)
{
    return (0 == strcasecmp("1",str))
        || (0 == strcasecmp("yes",str))
        || (0 == strcasecmp("on",str))
        || (0 == strcasecmp("true",str));
}

static inline unsigned level_from_cstr(const char * str)
{ // low = 0, medium = 1, high = 2
    unsigned res = 0;
         if (0 == strcasecmp("medium", str)) { res = 1; }
    else if (0 == strcasecmp("high",   str)) { res = 2; }
    return res;
}

static inline unsigned logtype_from_cstr(const char * str)
{ // null = 0, print = 1, syslog = 2, file = 3, encryptedfile = 4
    unsigned res = 0;
         if (0 == strcasecmp("print",         str)) { res = 1; }
    else if (0 == strcasecmp("syslog",        str)) { res = 2; }
    else if (0 == strcasecmp("file",          str)) { res = 3; }
    else if (0 == strcasecmp("encryptedfile", str)) { res = 4; }
    return res;
}

static inline unsigned ulong_from_cstr(const char * str)
{ // 10 = 10, 0x10 = 16
    if ((*str == '0') && (*(str + 1) == 'x')){
        return strtol(str + 2, 0, 16);
    }

    return atol(str);
}

static inline signed _long_from_cstr(const char * str)
{
    return atol(str);
}

static inline bool check_name(const char * str)
{
    return ((strlen(str) > 0) && (strlen(str) < 250));
}

static inline bool check_ask(const char * str)
{
    return (0 == strcmp(str, "ask"));
}

static inline void ask_string(const char * str, char buffer[], bool & flag)
{
    flag = check_ask(str);
    if (!flag){
        strncpy(buffer, str, strlen(str));
        buffer[strlen(str)] = 0;
    }
    else {
        buffer[0] = 0;
    }
}

struct IniAccounts {
    char accountname[255];
    char username[255]; // should use string
    char password[255]; // should use string
};

typedef enum
{
    AUTHID_UNKNOWN = 0,

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
    AUTHID_OPT_CLIPBOARD,           // clipboard
    AUTHID_OPT_DEVICEREDIRECTION,   // device_redirection
    AUTHID_OPT_FILE_ENCRYPTION,     // file encryption

    // Video capture
    AUTHID_OPT_CODEC_ID,    // CODEC_ID for video encoding
    AUTHID_OPT_MOVIE,       // is_rec
    AUTHID_OPT_MOVIE_PATH,  // rec_path
    AUTHID_VIDEO_QUALITY,

    // Alternate shell
    AUTHID_ALTERNATE_SHELL,
    AUTHID_SHELL_WORKING_DIRECTORY,

    // Context
    AUTHID_OPT_BITRATE,     // Bit rate for video encoding
    AUTHID_OPT_FRAMERATE,   // Frame rate for video encoding
    AUTHID_OPT_QSCALE,      // QScale parameter for vdeo encoding

    AUTHID_OPT_BPP,         // bits per planes (number of colors)
    AUTHID_OPT_HEIGHT,      // client height
    AUTHID_OPT_WIDTH,       // client width

    AUTHID_AUTH_ERROR_MESSAGE,

    AUTHID_SELECTOR,
    AUTHID_SELECTOR_CURRENT_PAGE,       // current page
    AUTHID_SELECTOR_DEVICE_FILTER,      // device filter text
    AUTHID_SELECTOR_GROUP_FILTER,       // group filter text
    AUTHID_SELECTOR_LINES_PER_PAGE,     // number of lines per page
    AUTHID_SELECTOR_NUMBER_OF_PAGES,    // number of pages

    AUTHID_TARGET_DEVICE,   // target_device
    AUTHID_TARGET_PASSWORD, // target_password
    AUTHID_TARGET_PORT,     // target_port
    AUTHID_TARGET_PROTOCOL, // proto_dest
    AUTHID_TARGET_USER,     // target_login

    AUTHID_AUTH_USER,       // login
    AUTHID_HOST,            // ip_client
    AUTHID_TARGET,          // ip_target
    AUTHID_PASSWORD,        // password

    AUTHID_AUTHCHANNEL_ANSWER,  // WabLauncher target answer
    AUTHID_AUTHCHANNEL_RESULT,  // WabLauncher session result
    AUTHID_AUTHCHANNEL_TARGET,  // WabLauncher target request

    AUTHID_MESSAGE, // warning_message

    AUTHID_ACCEPT_MESSAGE,  // display a dialog to valid a message
    AUTHID_DISPLAY_MESSAGE, // display a dialog box with a message

    AUTHID_AUTHENTICATED,
    AUTHID_REJECTED,        // rejected

    AUTHID_KEEPALIVE,
    AUTHID_PROXY_TYPE,

    AUTHID_TRACE_SEAL,      // after closing trace file trace is sealed using a signature hash

    AUTHID_SESSION_ID,      // session_id

    AUTHID_END_DATE_CNX,    // timeclose
    AUTHID_END_TIME,        // end time as text

    AUTHID_MODE_CONSOLE,
    AUTHID_TIMEZONE,

    AUTHID_REAL_TARGET_DEVICE,  // target device in ip transparent mode

    AUTHID_AUTHENTICATION_CHALLENGE,

    MAX_AUTHID
} authid_t;

// Translation text
#define STRAUTHID_TRANS_BUTTON_OK          "trans_ok"
#define STRAUTHID_TRANS_BUTTON_CANCEL      "trans_cancel"
#define STRAUTHID_TRANS_BUTTON_HELP        "trans_help"
#define STRAUTHID_TRANS_BUTTON_CLOSE       "trans_close"
#define STRAUTHID_TRANS_BUTTON_REFUSED     "trans_refused"
#define STRAUTHID_TRANS_LOGIN              "trans_login"
#define STRAUTHID_TRANS_USERNAME           "trans_username"
#define STRAUTHID_TRANS_PASSWORD           "trans_password"
#define STRAUTHID_TRANS_TARGET             "trans_target"
#define STRAUTHID_TRANS_DIAGNOSTIC         "trans_diagnostic"
#define STRAUTHID_TRANS_CONNECTION_CLOSED  "trans_connection_closed"
#define STRAUTHID_TRANS_HELP_MESSAGE       "trans_help_message"
// Options
#define STRAUTHID_OPT_CLIPBOARD            "clipboard"
#define STRAUTHID_OPT_DEVICEREDIRECTION    "device_redirection"
#define STRAUTHID_OPT_FILE_ENCRYPTION      "file_encryption"
// Video capture
#define STRAUTHID_OPT_CODEC_ID             "codec_id"
#define STRAUTHID_OPT_MOVIE                "is_rec"
#define STRAUTHID_OPT_MOVIE_PATH           "rec_path"
#define STRAUTHID_VIDEO_QUALITY            "video_quality"
// Alternate shell
#define STRAUTHID_ALTERNATE_SHELL          "alternate_shell"
#define STRAUTHID_SHELL_WORKING_DIRECTORY  "shell_working_directory"
// Context
#define STRAUTHID_OPT_BITRATE              "bitrate"
#define STRAUTHID_OPT_FRAMERATE            "framerate"
#define STRAUTHID_OPT_QSCALE               "qscale"
#define STRAUTHID_OPT_BPP                  "bpp"
#define STRAUTHID_OPT_HEIGHT               "height"
#define STRAUTHID_OPT_WIDTH                "width"

#define STRAUTHID_AUTH_ERROR_MESSAGE       "error_message"

#define STRAUTHID_SELECTOR                 "selector"
#define STRAUTHID_SELECTOR_CURRENT_PAGE    "selector_current_page"
#define STRAUTHID_SELECTOR_DEVICE_FILTER   "selector_device_filter"
#define STRAUTHID_SELECTOR_GROUP_FILTER    "selector_group_filter"
#define STRAUTHID_SELECTOR_LINES_PER_PAGE  "selector_lines_per_page"
#define STRAUTHID_SELECTOR_NUMBER_OF_PAGES "selector_number_of_pages"

#define STRAUTHID_TARGET_DEVICE            "target_device"
#define STRAUTHID_TARGET_PASSWORD          "target_password"
#define STRAUTHID_TARGET_PORT              "target_port"
#define STRAUTHID_TARGET_PROTOCOL          "proto_dest"
#define STRAUTHID_TARGET_USER              "target_login"

#define STRAUTHID_AUTH_USER                "login"
#define STRAUTHID_HOST                     "ip_client"
#define STRAUTHID_TARGET                   "ip_target"
#define STRAUTHID_PASSWORD                 "password"

TODO("This is not a translation but auth_channel answer, change key name in sesman")
#define STRAUTHID_AUTHCHANNEL_ANSWER       "trans_auth_channel"
#define STRAUTHID_AUTHCHANNEL_RESULT       "auth_channel_result"
#define STRAUTHID_AUTHCHANNEL_TARGET       "auth_channel_target"

#define STRAUTHID_MESSAGE                  "message"

#define STRAUTHID_ACCEPT_MESSAGE           "accept_message"
#define STRAUTHID_DISPLAY_MESSAGE          "display_message"

#define STRAUTHID_AUTHENTICATED            "authenticated"
#define STRAUTHID_REJECTED                 "rejected"

#define STRAUTHID_KEEPALIVE                "keepalive"
#define STRAUTHID_PROXY_TYPE               "proxy_type"

#define STRAUTHID_TRACE_SEAL               "trace_seal"

#define STRAUTHID_SESSION_ID               "session_id"

#define STRAUTHID_END_DATE_CNX             "timeclose"
#define STRAUTHID_END_TIME                 "end_time"

#define STRAUTHID_MODE_CONSOLE             "mode_console"
#define STRAUTHID_TIMEZONE                 "timezone"

#define STRAUTHID_REAL_TARGET_DEVICE       "real_target_device"

#define STRAUTHID_AUTHENTICATION_CHALLENGE "authentication_challenge"

static const std::string authstr[MAX_AUTHID - 1] = {
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
    STRAUTHID_OPT_CLIPBOARD,            // clipboard
    STRAUTHID_OPT_DEVICEREDIRECTION,    // device_redirection
    STRAUTHID_OPT_FILE_ENCRYPTION,      // file encryption

    // Video capture
    STRAUTHID_OPT_CODEC_ID,     // CODEC_ID for video encoding
    STRAUTHID_OPT_MOVIE,        // is_rec
    STRAUTHID_OPT_MOVIE_PATH,   // rec_path
    STRAUTHID_VIDEO_QUALITY,

    // Alternate shell
    STRAUTHID_ALTERNATE_SHELL,
    STRAUTHID_SHELL_WORKING_DIRECTORY,

    // Context
    STRAUTHID_OPT_BITRATE,      // Bit rate for video encoding
    STRAUTHID_OPT_FRAMERATE,    // Frame rate for video encoding
    STRAUTHID_OPT_QSCALE,       // QScale parameter for vdeo encoding

    STRAUTHID_OPT_BPP,          // bits per planes (number of colors)
    STRAUTHID_OPT_HEIGHT,       // client height
    STRAUTHID_OPT_WIDTH,        // client width

    STRAUTHID_AUTH_ERROR_MESSAGE,

    STRAUTHID_SELECTOR,
    STRAUTHID_SELECTOR_CURRENT_PAGE,    // current page
    STRAUTHID_SELECTOR_DEVICE_FILTER,   // device filter text
    STRAUTHID_SELECTOR_GROUP_FILTER,    // group filter text
    STRAUTHID_SELECTOR_LINES_PER_PAGE,  // number of lines per page
    STRAUTHID_SELECTOR_NUMBER_OF_PAGES, // number of pages

    STRAUTHID_TARGET_DEVICE,    // target_device
    STRAUTHID_TARGET_PASSWORD,  // target_password
    STRAUTHID_TARGET_PORT,      // target_port
    STRAUTHID_TARGET_PROTOCOL,  // proto_dest
    STRAUTHID_TARGET_USER,      // target_login

    STRAUTHID_AUTH_USER,        // login
    STRAUTHID_HOST,             // ip_client
    STRAUTHID_TARGET,           // ip_target
    STRAUTHID_PASSWORD,         // password

    STRAUTHID_AUTHCHANNEL_ANSWER,   // WabLauncher target answer
    STRAUTHID_AUTHCHANNEL_RESULT,   // WabLauncher session result
    STRAUTHID_AUTHCHANNEL_TARGET,   // WabLauncher target request

    STRAUTHID_MESSAGE,  // warning_message

    STRAUTHID_ACCEPT_MESSAGE,   // display a dialog to valid a message
    STRAUTHID_DISPLAY_MESSAGE,  // display a dialog box with a message

    STRAUTHID_AUTHENTICATED,
    STRAUTHID_REJECTED,         // rejected

    STRAUTHID_KEEPALIVE,
    STRAUTHID_PROXY_TYPE,

    STRAUTHID_TRACE_SEAL,   // after closing trace file trace is sealed using a signature hash

    STRAUTHID_SESSION_ID,   // session_id

    STRAUTHID_END_DATE_CNX, // timeclose
    STRAUTHID_END_TIME,     // end time as text

    STRAUTHID_MODE_CONSOLE,
    STRAUTHID_TIMEZONE,

    STRAUTHID_REAL_TARGET_DEVICE,

    STRAUTHID_AUTHENTICATION_CHALLENGE,
};
static inline authid_t authid_from_string(const char * strauthid) {

    std::string str = std::string(strauthid);
    authid_t res = AUTHID_UNKNOWN;
    for (int i = 0; i < MAX_AUTHID - 1 ; i++) {
        if (0 == authstr[i].compare(str)) {
            res = (authid_t)(i + 1);
            break;
        }
    }
    return res;
}

static inline const char * string_from_authid(authid_t authid) {
    if ((authid == AUTHID_UNKNOWN) || (authid >= MAX_AUTHID))
        return "";
    return authstr[(unsigned)authid - 1].c_str();
}



struct Inifile {
    //private:
    /******************************************************
     * BaseField is an abstract class which carries:
     * - some flags indicating some meta state of the field.
     * - a pointer to a Inifile it is attached to.
     * - an authid_t if it is attached to a Inifile.
     ******************************************************
     */
    class BaseField {
    protected:
        bool        asked;         // the value is asked in the context
        bool        modified;      // the value has been modified since last use
        bool        read;          // the value has been read since last set (modified)
        Inifile *   ini;           // Inifile to which the field is attached
        authid_t    authid;        // Auth Id of the field in the Inifile
        BaseField()
            : asked(false)
            , modified(true)
            , read(false)
            , ini(NULL)
            , authid(AUTHID_UNKNOWN)
        {
        }
        virtual ~BaseField(){
            if (this->ini){
                this->ini->remove_field(authid);
            }
        }
        /**********************
         * notify the Inifile that the field has been changed
         */
        void notify() {
            if (this->ini)
                this->ini->notify(this);
        }
        void use_notify() {
            if (this->ini)
                this->ini->use_notify(this);
        }
        inline void unask(){
            this->asked = false;
        }

    public:
        /*******************************
         * link this field to an Inifile
         *******************************
         */
        void attach_ini(Inifile * p_ini, authid_t authid = AUTHID_UNKNOWN) {
            this->ini = p_ini;
            if (authid != AUTHID_UNKNOWN
                && this->ini) {
                this->authid = authid;
                this->ini->attach_field(this,authid);
            }
        }

        /**************************
         * Use this field to mark it as modified
         ***************************
         */
        void use() {
            if (this->modified){
                this->use_notify();
            }
            this->modified = false;

        }

        /**************************
         * Check if the field has been modified (since last use)
         ***************************
         */
        bool has_changed() {
            // if (!this->asked)
            //     this->use();
            return this->modified;
        }
        /**************************
         * Set the field as asked.
         **************************
         */
        void ask() {
            this->asked = true;
            this->modified = true;
            this->notify();
        }
        /**************************
         * Check if the field is asked
         ***************************
         */

        bool is_asked() {
            return this->asked;
        }
        /**************************
         * Check if the field has been read (since last effective set)
         ***************************
         */
        bool has_been_read() {
            return this->read;
        }


        authid_t get_authid() {
            return this->authid;
        }


        virtual void set_from_cstr(const char * cstr) = 0;

        virtual const char* get_value() = 0;

        const char* get_serialized(char * buff, size_t size) {
            TODO("The buffer managing is not clear here, "
                 "can segfault if buff is not big enough");
            const char * key = string_from_authid(this->authid);
            char * p = buff;
            strncpy(p, key, size);
            buff[size - 1] = 0;
            while (*p)
                p++;
            *(p++) = '\n';
            if (this->is_asked()) {
                LOG(LOG_INFO, "get_serialized(): sending (from authid) %s=ASK\n", key);
                strcpy(p, "ASK\n");
            }
            else {
                const char * tmp = this->get_value();
                if ((strncasecmp("password", (char*)key, 8) == 0)
                    ||(strncasecmp("target_password", (char*)key, 15) == 0)){
                    LOG(LOG_INFO, "get_serialized(): sending (from authid) %s=<hidden>\n", key);
                }
                else {
                    LOG(LOG_INFO, "get_serialized(): sending (from authid) %s=%s\n", key, tmp);
                }
                *(p++) = '!';
                strncpy(p, tmp, strlen(tmp));
                while (*p)
                    p++;
                *(p++) = '\n';
                *p = '\0';
            }
            return buff;
        }

    };
    /*************************************
     * Field which contains a String type
     *************************************
     */
    class StringField : public BaseField {
    protected:
        redemption::string data;
    public:
        StringField() : BaseField()
        {
        }

        void set(redemption::string & string) {
            this->set_from_cstr(string.c_str());
        }
        void set_empty() {
            if (!this->data.is_empty()){
                this->modified = true;
                this->read = false;
                this->notify();
                this->data.empty();
            }

        }
        void set_from_cstr(const char * cstr) {
            this->asked = false;
            if (strcmp(this->data.c_str(),cstr)) {
                this->modified = true;
                this->read = false;
                this->notify();
                this->data.copy_c_str(cstr);
            }

        }
        const redemption::string & get() {
            this->read = true;
            return this->data;
        }

        const char * get_cstr() {
            return this->get().c_str();
        }

        const char * get_value() {
            if (this->is_asked()) {
                return "ASK";
            }
            return this->get().c_str();
        }

    };
    /*************************************
     * Field which contains an Unsigned Integer type
     * implemented as a 32 bits data
     *************************************
     */
    class UnsignedField : public BaseField {
    protected:
        uint32_t data;
        char buff[20];
    public:
        UnsignedField(): BaseField()
                       , data(0) {
        }

        void set(uint32_t that) {
            this->asked = false;
            if (this->data != that) {
                this->modified = true;
                this->read = false;
                this->notify();
                this->data = that;
            }
        }

        void set_from_cstr(const char * cstr) {
            this->set(ulong_from_cstr(cstr));
        }

        void set_from_level_cstr(const char * cstr) {
            this->set(level_from_cstr(cstr));
        }

        void set_from_logtype_cstr(const char * cstr) {
            this->set(logtype_from_cstr(cstr));
        }

        const uint32_t get() {
            this->read = true;
            return this->data;
        }

        const char * get_value() {
            if (this->is_asked()) {
                return "ASK";
            }
            snprintf(buff, sizeof(buff), "%u", this->data);
            return buff;
        }
    };
    /*************************************
     * Field which contains a Signed Integer type
     * implementation according to compiler
     *************************************
     */

    class SignedField : public BaseField {
    protected:
        signed data;
        char buff[20];
    public:
        SignedField(): BaseField()
                     , data(0) {
        }

        void set(signed that) {
            this->asked = false;
            if (this->data != that) {
                this->modified = true;
                this->read = false;
                this->notify();
                this->data = that;
            }

        }

        void set_from_cstr(const char * cstr) {
            this->set(_long_from_cstr(cstr));
        }

        const signed get() {
            this->read = true;
            return this->data;
        }
        const char * get_value() {
            if (this->is_asked()) {
                return "ASK";
            }
            snprintf(buff, sizeof(buff), "%d", this->data);
            return buff;
        }
    };

    /*************************************
     * Field which contains a Boolean type
     *************************************
     */
    class BoolField : public BaseField {
    protected:
        bool data;
    public:
        BoolField(): BaseField()
                   , data(false) {
        }

        void set(bool that) {
            this->asked = false;
            if (this->data != that) {
                this->modified = true;
                this->read = false;
                this->notify();
                this->data = that;
            }

        }
        void set_from_cstr(const char * cstr) {
            this->set(bool_from_cstr(cstr));
        }

        const bool get() {
            this->read = true;
            return this->data;
        }
        const char * get_value() {
            if (this->is_asked()) {
                return "ASK";
            }
            return this->data?"True":"False";
        }
    };

private:
    // flag indicating if a Field attached to this inifile has been changed
    bool something_changed;

    // list of Field which has been changed
    std::set< BaseField * > changed_set;

    // Map associating authid with a Field.
    std::map< authid_t, BaseField *> field_list;



public:
    std::set< authid_t > to_send_set;
    const std::map< authid_t, BaseField *>& get_field_list() {
        return this->field_list;
    }
    void remove_field(authid_t authid) {
        this->field_list.erase(authid);
    }

    void notify(BaseField * field) {
        this->something_changed = true;
        this->changed_set.insert(field);
    }
    void use_notify(BaseField * field) {
        this->changed_set.erase(field);
        if (this->changed_set.empty()) {
            this->something_changed = false;
        }
    }
    bool check() {
        return this->something_changed;
    }
    std::set< BaseField * > get_changed_set() {
        return this->changed_set;
    }

    void reset() {
        this->something_changed = false;
        changed_set.clear();
    }
    void attach_field(BaseField* field, authid_t authid){
        field_list[authid] = field;
    }

    struct Inifile_globals {
        BoolField capture_chunk;

        StringField auth_user;                    // AUTHID_AUTH_USER //
        StringField host;                         // client_ip AUTHID_HOST //
        StringField target;                       // target ip AUTHID_TARGET //

        StringField target_device;                // AUTHID_TARGET_DEVICE //
        StringField target_user;                  // AUTHID_TARGET_USER //

        // BEGIN globals
        bool bitmap_cache;       // default true
        bool bitmap_compression; // default true
        int port;                // default 3389
        bool nomouse;
        bool notimestamp;
        int encryptionLevel;     // 0=low, 1=medium, 2=high
        char authip[255];
        int authport;
        bool autovalidate;       // dialog autovalidation for test

        // keepalive and no traffic auto deconnexion
        int max_tick;
        int keepalive_grace_delay;

        bool internal_domain;

        char dynamic_conf_path[1024]; // directory where to look for dynamic configuration files
        char auth_channel[512];
        bool enable_file_encryption; // AUTHID_OPT_FILE_ENCRYPTION --
        bool enable_tls;
        char listen_address[256];
        bool enable_ip_transparent;
        char certificate_password[256];

        char png_path[1024];
        char wrm_path[1024];

        char alternate_shell[1024];
        char shell_working_directory[1024];

        char codec_id[512];          // AUTHID_OPT_CODEC_ID --
        bool movie;                  // AUTHID_OPT_MOVIE --
        char movie_path[512];        // AUTHID_OPT_MOVIE_PATH --
        char video_quality[512];     // AUTHID_VIDEO_QUALITY --
        bool enable_bitmap_update;
        // END globals

        uint64_t flv_break_interval;  // time between 2 flv movies captures (in seconds)
        unsigned flv_frame_interval;
    } globals;

    // section "client"
    struct {
        bool ignore_logon_password; // if true, ignore password provided by RDP client, user need do login manually. default

        uint32_t performance_flags_default;
        uint32_t performance_flags_force_present;
        uint32_t performance_flags_force_not_present;

        bool tls_fallback_legacy;

        bool clipboard;             // AUTHID_OPT_CLIPBOARD --
        bool device_redirection;    // AUTHID_OPT_DEVICEREDIRECTION --
    } client;

    // Section "video"
    struct {
        unsigned capture_flags;  // 1 PNG capture, 2 WRM
        // video opt from capture_flags
        bool capture_png;
        bool capture_wrm;
        bool capture_flv;
        bool capture_ocr;

        unsigned ocr_interval;
        unsigned png_interval;   // time between 2 png captures (in 1/10 seconds)
        unsigned capture_groupid;
        unsigned frame_interval; // time between 2 frame captures (in 1/100 seconds)
        unsigned break_interval; // time between 2 wrm movies (in seconds)
        unsigned png_limit;    // number of png captures to keep
        char replay_path[1024];

        int l_bitrate;         // bitrate for low quality
        int l_framerate;       // framerate for low quality
        int l_height;          // height for low quality
        int l_width;           // width for low quality
        int l_qscale;          // qscale (parameter given to ffmpeg) for low quality

        // Same for medium quality
        int m_bitrate;
        int m_framerate;
        int m_height;
        int m_width;
        int m_qscale;

        // Same for high quality
        int h_bitrate;
        int h_framerate;
        int h_height;
        int h_width;
        int h_qscale;
    } video;

    // Section "debug"
    struct {
        uint32_t x224;
        uint32_t mcs;
        uint32_t sec;
        uint32_t rdp;
        uint32_t primary_orders;
        uint32_t secondary_orders;
        uint32_t bitmap;
        uint32_t capture;
        uint32_t auth;
        uint32_t session;
        uint32_t front;
        uint32_t mod_rdp;
        uint32_t mod_vnc;
        uint32_t mod_int;
        uint32_t mod_xup;
        uint32_t widget;
        uint32_t input;

        int log_type;
        char log_file_path[1024]; // log file location

    } debug;

    // section "translation"
    struct {
        StringField button_ok;              // AUTHID_TRANS_BUTTON_OK
        StringField button_cancel;          // AUTHID_TRANS_BUTTON_CANCEL
        StringField button_help;            // AUTHID_TRANS_BUTTON_HELP
        StringField button_close;           // AUTHID_TRANS_BUTTON_CLOSE
        StringField button_refused;         // AUTHID_TRANS_BUTTON_REFUSED
        StringField login;                  // AUTHID_TRANS_LOGIN
        StringField username;               // AUTHID_TRANS_USERNAME
        StringField password;               // AUTHID_TRANS_PASSWORD
        StringField target;                 // AUTHID_TRANS_TARGET
        StringField diagnostic;             // AUTHID_TRANS_DIAGNOSTIC
        StringField connection_closed;      // AUTHID_TRANS_CONNECTION_CLOSED
        StringField help_message;           // AUTHID_TRANS_HELP_MESSAGE
    } translation;

    // section "context"
    struct {
        unsigned           selector_focus;           //  --
        char               movie[1024];              //  --

        UnsignedField      opt_bitrate;              // AUTHID_OPT_BITRATE --
        UnsignedField      opt_framerate;            // AUTHID_OPT_FRAMERATE --
        UnsignedField      opt_qscale;               // AUTHID_OPT_QSCALE --

        UnsignedField      opt_bpp;                  // AUTHID_OPT_BPP //
        UnsignedField      opt_height;               // AUTHID_OPT_HEIGHT //
        UnsignedField      opt_width;                // AUTHID_OPT_WIDTH //

        // auth_error_message is left as redemption::string type
        // because SocketTransport and ReplayMod take it as argument on
        // constructor and modify it as a redemption::string
        redemption::string auth_error_message;       // AUTHID_AUTH_ERROR_MESSAGE --


        BoolField          selector;                 // AUTHID_SELECTOR //
        UnsignedField      selector_current_page;    // AUTHID_SELECTOR_CURRENT_PAGE //
        StringField        selector_device_filter;   // AUTHID_DEVICE_FILTER  //
        StringField        selector_group_filter;    // AUTHID_SELECTOR_GROUP_FILTER //
        UnsignedField      selector_lines_per_page;  // AUTHID_SELECTOR_LINES_PER_PAGE //
        UnsignedField      selector_number_of_pages; // AUTHID_SELECTOR_NUMBER_OF_PAGES --

        StringField        target_password;          // AUTHID_TARGET_PASSWORD //
        UnsignedField      target_port;              // AUTHID_TARGET_PORT --
        StringField        target_protocol;          // AUTHID_TARGET_PROTOCOL //

        StringField        password;                 // AUTHID_PASSWORD --


        StringField        authchannel_answer;       // AUTHID_AUTHCHANNEL_ANSWER --
        StringField        authchannel_result;       // AUTHID_AUTHCHANNEL_RESULT //
        StringField        authchannel_target;       // AUTHID_AUTHCHANNEL_TARGET //

        StringField        message;                  // AUTHID_MESSAGE --

        StringField        accept_message;           // AUTHID_ACCEPT_MESSAGE --
        StringField        display_message;          // AUTHID_DISPLAY_MESSAGE --

        StringField        rejected;                 // AUTHID_REJECTED --

        BoolField          authenticated;            // AUTHID_AUTHENTICATED --

        BoolField          keepalive;                // AUTHID_KEEPALIVE //
        StringField        proxy_type;               // AUTHID_PROXY_TYPE //

        StringField        trace_seal;               // AUTHID_TRACE_SEAL //

        StringField        session_id;               // AUTHID_SESSION_ID --

        UnsignedField      end_date_cnx;             // AUTHID_END_DATE_CNX --
        StringField        end_time;                 // AUTHID_END_TIME --

        StringField        mode_console;             // AUTHID_MODE_CONSOLE --
        SignedField        timezone;                 // AUTHID_TIMEZONE --

        StringField        real_target_device;       // AUHTID_REAL_TARGET_DEVICE  //

        StringField        authentication_challenge; // AUTHID_AUTHENTICATION_CHALLENGE --
    } context;

    struct IniAccounts account;
public:
    Inifile() {
        std::stringstream oss("");
        this->init();
        this->cparse(oss);
    }

    Inifile(const char * filename) {
        this->init();
        this->cparse(filename);
    }

    Inifile(istream & Inifile_stream) {
        this->init();
        this->cparse(Inifile_stream);
    }

    void init(){
        this->something_changed = false;

        //this->globals.capture_chunk = false;
        this->globals.capture_chunk.set(false);


        this->globals.auth_user.set_from_cstr("");
        this->globals.host.set_from_cstr("");
        this->globals.target_device.set_from_cstr("");
        this->globals.target_user.set_from_cstr("");

        this->globals.auth_user.attach_ini(this,AUTHID_AUTH_USER);
        this->globals.host.attach_ini(this,AUTHID_HOST);
        this->globals.target.attach_ini(this,AUTHID_TARGET);
        this->globals.target_device.attach_ini(this,AUTHID_TARGET_DEVICE);
        this->globals.target_user.attach_ini(this,AUTHID_TARGET_USER);

        // this->globals.auth_user[0]     = 0;
        // this->globals.host[0]          = 0;
        // this->globals.target_device[0] = 0;
        // this->globals.target_user[0]   = 0;

        // Init globals
        this->globals.bitmap_cache = true;
        this->globals.bitmap_compression = true;
        this->globals.port = 3389;
        this->globals.nomouse = false;
        this->globals.notimestamp = false;
        this->globals.encryptionLevel = level_from_cstr("low");
        strcpy(this->globals.authip, "127.0.0.1");
        this->globals.authport = 3350;
        this->globals.autovalidate = false;

        this->globals.max_tick    = 30;
        this->globals.keepalive_grace_delay = 30;

        this->globals.internal_domain = false;
        strcpy(this->globals.dynamic_conf_path, "/tmp/rdpproxy/");
        memcpy(this->globals.auth_channel, "\0\0\0\0\0\0\0\0", 8);
        this->globals.enable_file_encryption = false;
        this->globals.enable_tls             = false;
        strcpy(this->globals.listen_address, "0.0.0.0");
        this->globals.enable_ip_transparent  = false;
        strcpy(this->globals.certificate_password, "inquisition");

        strcpy(this->globals.png_path, PNG_PATH);
        strcpy(this->globals.wrm_path, WRM_PATH);

        this->globals.alternate_shell[0]         = 0;
        this->globals.shell_working_directory[0] = 0;

        strcpy(this->globals.codec_id, "flv");
        this->globals.movie            = false;
        this->globals.movie_path[0]    = 0;
        TODO("this could be some kind of enumeration")
        strcpy(this->globals.video_quality, "medium");
        this->globals.enable_bitmap_update = false;
        // End Init globals


        this->globals.flv_break_interval = 600000000l;
        this->globals.flv_frame_interval = 1000000L;

        strcpy(this->account.accountname, "");
        strcpy(this->account.username,    "");
        strcpy(this->account.password,    "");

        // Begin Section "client".
        this->client.ignore_logon_password               = false;
//      this->client.performance_flags_default           = PERF_DISABLE_WALLPAPER | PERF_DISABLE_FULLWINDOWDRAG | PERF_DISABLE_MENUANIMATIONS;
        this->client.performance_flags_default           = 0;
        this->client.performance_flags_force_present     = 0;
        this->client.performance_flags_force_not_present = 0;
        this->client.tls_fallback_legacy                 = false;
        this->client.clipboard                           = true;
        this->client.device_redirection                  = true;
        // End Section "client"

        // Begin section video
        this->video.capture_flags = 1; // 1 png, 2 wrm, 4 flv, 8 ocr
        this->video.capture_wrm   = true;
        this->video.capture_png   = true;
        this->video.capture_flv   = false;
        this->video.capture_ocr   = false;

        this->video.ocr_interval    = 100; // 1 every second
        this->video.png_interval    = 3000;
        this->video.capture_groupid = 33;
        this->video.frame_interval  = 40;
        this->video.break_interval  = 600;
        this->video.png_limit       = 3;
        strcpy(this->video.replay_path, "/tmp/");

        this->video.l_bitrate   = 20000;
        this->video.l_framerate = 1;
        this->video.l_height    = 480;
        this->video.l_width     = 640;
        this->video.l_qscale    = 25;
        this->video.m_bitrate   = 40000;
        this->video.m_framerate = 1;
        this->video.m_height    = 768;
        this->video.m_width     = 1024;
        this->video.m_qscale    = 15;
        this->video.h_bitrate   = 200000;
        this->video.h_framerate = 5;
        this->video.h_height    = 1024;
        this->video.h_width     = 1280;
        this->video.h_qscale    = 15;
        // End section "video"


        // Begin Section "debug".
        this->debug.x224              = 0;
        this->debug.mcs               = 0;
        this->debug.sec               = 0;
        this->debug.rdp               = 0;
        this->debug.primary_orders    = 0;
        this->debug.secondary_orders  = 0;
        this->debug.bitmap            = 0;
        this->debug.capture           = 0;
        this->debug.auth              = 0;
        this->debug.session           = 0;
        this->debug.front             = 0;
        this->debug.mod_rdp           = 0;
        this->debug.mod_vnc           = 0;
        this->debug.mod_int           = 0;
        this->debug.mod_xup           = 0;
        this->debug.widget            = 0;
        this->debug.input             = 0;

        this->debug.log_type          = 2; // syslog by default
        this->debug.log_file_path[0]  = 0;
        // End Section "debug"

        // Begin Section "translation"
        this->translation.button_ok.set_from_cstr("OK");
        this->translation.button_cancel.set_from_cstr("Cancel");
        this->translation.button_help.set_from_cstr("Help");
        this->translation.button_close.set_from_cstr("Close");
        this->translation.button_refused.set_from_cstr("Refused");
        this->translation.login.set_from_cstr("login");
        this->translation.username.set_from_cstr("username");
        this->translation.password.set_from_cstr("password");
        this->translation.target.set_from_cstr("target");
        this->translation.diagnostic.set_from_cstr("diagnostic");
        this->translation.connection_closed.set_from_cstr("Connection closed");
        this->translation.help_message.set_from_cstr("Help message");

        this->translation.button_ok.attach_ini(this,AUTHID_TRANS_BUTTON_OK);
        this->translation.button_cancel.attach_ini(this,AUTHID_TRANS_BUTTON_CANCEL);
        this->translation.button_help.attach_ini(this,AUTHID_TRANS_BUTTON_HELP);
        this->translation.button_close.attach_ini(this,AUTHID_TRANS_BUTTON_CLOSE);
        this->translation.button_refused.attach_ini(this,AUTHID_TRANS_BUTTON_REFUSED);
        this->translation.login.attach_ini(this,AUTHID_TRANS_LOGIN);
        this->translation.username.attach_ini(this,AUTHID_TRANS_USERNAME);
        this->translation.password.attach_ini(this,AUTHID_TRANS_PASSWORD);
        this->translation.target.attach_ini(this,AUTHID_TRANS_TARGET);
        this->translation.diagnostic.attach_ini(this,AUTHID_TRANS_DIAGNOSTIC);
        this->translation.connection_closed.attach_ini(this,AUTHID_TRANS_CONNECTION_CLOSED);
        this->translation.help_message.attach_ini(this,AUTHID_TRANS_HELP_MESSAGE);
        // End Section "translation"

        // Begin section "context"

        this->context.selector_focus              = 0;
        this->context.movie[0]                    = 0;


        this->context.opt_bitrate.set(40000);
        this->context.opt_framerate.set(5);
        this->context.opt_qscale.set(15);

        this->context.opt_bitrate.attach_ini(this,AUTHID_OPT_BITRATE);
        this->context.opt_framerate.attach_ini(this,AUTHID_OPT_FRAMERATE);
        this->context.opt_qscale.attach_ini(this,AUTHID_OPT_QSCALE);
        // this->context.opt_bitrate                 = 40000;
        // this->context.opt_framerate               = 5;
        // this->context.opt_qscale                  = 15;



        this->context.opt_bpp.set(24);
        this->context.opt_height.set(600);
        this->context.opt_width.set(800);
        // this->context.opt_bpp                     = 24;
        // this->context.opt_height                  = 600;
        // this->context.opt_width                   = 800;

        this->context.auth_error_message.empty();
        // this->context.auth_error_message.attach_ini(this,AUTHID_AUTH_ERROR_MESSAGE);
        // this->context.auth_error_message.empty();


        this->context.selector.set(false);
        this->context.selector_current_page.set(1);
        this->context.selector_device_filter.set_empty();
        this->context.selector_group_filter.set_empty();
        this->context.selector_lines_per_page.set(20);
        this->context.selector_number_of_pages.set(1);

        this->context.selector_number_of_pages.attach_ini(this, AUTHID_SELECTOR_NUMBER_OF_PAGES);

        this->globals.target_device.ask();
        this->globals.target_user.ask();
        this->globals.auth_user.ask();


        this->context.target_password.set_empty();
        this->context.target_password.ask();

        this->context.target_port.set(3389);
        this->context.target_port.ask();


        this->context.target_protocol.set_from_cstr("RDP");
        this->context.target_protocol.ask();


        // this->globals.state_host.asked                    = false;
        // this->globals.state_host.modified                    = true;

        // this->globals.state_target.asked                  = false;
        // this->globals.state_target.modified                  = true;


        // this->globals.state_auth_user.asked               = true;
        // this->globals.state_auth_user.modified               = true;

        this->context.password.set_empty();

        this->context.password.ask();
        // this->context.state_password.asked                = true;
        // this->context.state_password.modified                = true;


        this->context.authchannel_answer.set_empty();
        this->context.authchannel_result.set_empty();
        this->context.authchannel_target.set_empty();


        // this->context.state_accept_message.asked          = false;
        // this->context.state_accept_message.modified          = true;

        // this->context.state_display_message.asked         = false;
        // this->context.state_display_message.modified         = true;

        this->context.message.set_empty();
        this->context.message.attach_ini(this, AUTHID_MESSAGE);

        this->context.accept_message.set_empty();
        this->context.display_message.set_empty();

        this->context.rejected.set_from_cstr("Connection refused by authentifier.");
        this->context.rejected.attach_ini(this, AUTHID_REJECTED);

        this->context.authenticated.set(false);
        this->context.authenticated.attach_ini(this, AUTHID_AUTHENTICATED);

        this->context.keepalive.set(false);
        this->context.keepalive.ask();
        // this->context.state_keepalive.asked               = true;
        // this->context.state_keepalive.modified               = true;

        // this->context.state_proxy_type.asked              = false;
        // this->context.state_proxy_type.modified              = true;



        this->context.proxy_type.set_from_cstr("RDP");


        // this->context.state_trace_seal.asked              = false;
        // this->context.state_trace_seal.modified              = true;

        this->context.trace_seal.set_empty();

        this->context.session_id.set_empty();
        this->context.session_id.attach_ini(this, AUTHID_SESSION_ID);

        this->context.end_date_cnx.set(0);
        this->context.end_time.set_empty();
        this->context.end_date_cnx.attach_ini(this, AUTHID_END_DATE_CNX);
        this->context.end_time.attach_ini(this, AUTHID_END_TIME);



        this->context.mode_console.set_from_cstr("allow");
        this->context.timezone.set(-3600);
        this->context.mode_console.attach_ini(this, AUTHID_MODE_CONSOLE);
        this->context.timezone.attach_ini(this, AUTHID_TIMEZONE);



        this->context.real_target_device.set_empty();

        // this->context.state_real_target_device.asked              = false;
        // this->context.state_real_target_device.modified              = true;


        this->context.authentication_challenge.set_empty();
        this->context.authentication_challenge.attach_ini(this, AUTHID_AUTHENTICATION_CHALLENGE);
        // Attaching ini struct to values
        this->context.opt_bpp.attach_ini(this,AUTHID_OPT_BPP);
        this->context.opt_height.attach_ini(this,AUTHID_OPT_HEIGHT);
        this->context.opt_width.attach_ini(this,AUTHID_OPT_WIDTH);

        this->context.selector.attach_ini(this,AUTHID_SELECTOR);
        this->context.selector_current_page.attach_ini(this,AUTHID_SELECTOR_CURRENT_PAGE);
        this->context.selector_device_filter.attach_ini(this,AUTHID_SELECTOR_DEVICE_FILTER);
        this->context.selector_group_filter.attach_ini(this,AUTHID_SELECTOR_GROUP_FILTER);
        this->context.selector_lines_per_page.attach_ini(this,AUTHID_SELECTOR_LINES_PER_PAGE);

        this->context.target_password.attach_ini(this,AUTHID_TARGET_PASSWORD);
        this->context.target_protocol.attach_ini(this,AUTHID_TARGET_PROTOCOL);
        this->context.target_port.attach_ini(this,AUTHID_TARGET_PORT);

        this->context.password.attach_ini(this,AUTHID_PASSWORD);

        this->context.accept_message.attach_ini(this,AUTHID_ACCEPT_MESSAGE);
        this->context.display_message.attach_ini(this,AUTHID_DISPLAY_MESSAGE);

        this->context.proxy_type.attach_ini(this,AUTHID_PROXY_TYPE);
        this->context.real_target_device.attach_ini(this,AUTHID_REAL_TARGET_DEVICE);

        this->context.authchannel_answer.attach_ini(this,AUTHID_AUTHCHANNEL_ANSWER);
        this->context.authchannel_target.attach_ini(this,AUTHID_AUTHCHANNEL_TARGET);
        this->context.authchannel_result.attach_ini(this,AUTHID_AUTHCHANNEL_RESULT);
        this->context.keepalive.attach_ini(this,AUTHID_KEEPALIVE);
        this->context.trace_seal.attach_ini(this,AUTHID_TRACE_SEAL);


        //init to_send_set of authid
        this->to_send_set.insert(AUTHID_PROXY_TYPE);
        this->to_send_set.insert(AUTHID_DISPLAY_MESSAGE);
        this->to_send_set.insert(AUTHID_ACCEPT_MESSAGE);
        this->to_send_set.insert(AUTHID_HOST);
        this->to_send_set.insert(AUTHID_TARGET);
        this->to_send_set.insert(AUTHID_AUTH_USER);
        this->to_send_set.insert(AUTHID_PASSWORD);
        this->to_send_set.insert(AUTHID_TARGET_USER);
        this->to_send_set.insert(AUTHID_TARGET_DEVICE);
        this->to_send_set.insert(AUTHID_TARGET_PROTOCOL);
        this->to_send_set.insert(AUTHID_SELECTOR);
        this->to_send_set.insert(AUTHID_SELECTOR_GROUP_FILTER);
        this->to_send_set.insert(AUTHID_SELECTOR_DEVICE_FILTER);
        this->to_send_set.insert(AUTHID_SELECTOR_LINES_PER_PAGE);
        this->to_send_set.insert(AUTHID_SELECTOR_CURRENT_PAGE);
        this->to_send_set.insert(AUTHID_TARGET_PASSWORD);
        this->to_send_set.insert(AUTHID_OPT_WIDTH);
        this->to_send_set.insert(AUTHID_OPT_HEIGHT);
        this->to_send_set.insert(AUTHID_OPT_BPP);
        this->to_send_set.insert(AUTHID_REAL_TARGET_DEVICE);
    };

    void cparse(istream & ifs){
        const size_t maxlen = 256;
        char line[maxlen];
        char context[128] = {0};
        bool truncated = false;
        while (ifs.good()){
            ifs.getline(line, maxlen);
            if (ifs.fail() && ifs.gcount() == (maxlen-1)){
                if (!truncated){
                    LOG(LOG_INFO, "Line too long in configuration file");
                    hexdump(line, maxlen-1);
                }
                ifs.clear();
                truncated = true;
                continue;
            }
            if (truncated){
                truncated = false;
                continue;
            }
            this->parseline(line, context);
        };
    }

    void parseline(const char * line, char * context)
    {
        char key[128];
        char value[128];

        const char * startkey = line;
        for (; *startkey ; startkey++) {
            if (!isspace(*startkey)){
                if (*startkey == '['){
                    const char * startcontext = startkey + 1;
                    const char * endcontext = strchr(startcontext, ']');
                    if (endcontext){
                        memcpy(context, startcontext, endcontext - startcontext);
                        context[endcontext - startcontext] = 0;
                    }
                    return;
                }
                break;
            }
        }
        const char * endkey = strchr(startkey, '=');
        if (endkey && endkey != startkey){
            const char * sep = endkey;
            for (--endkey; endkey >= startkey ; endkey--) {
                if (!isspace(*endkey)){
                    TODO("RZ: Possible buffer overflow if length of key is larger than 128 bytes")
                    memcpy(key, startkey, endkey - startkey + 1);
                    key[endkey - startkey + 1] = 0;

                    const char * startvalue = sep + 1;
                    for ( ; *startvalue ; startvalue++) {
                        if (!isspace(*startvalue)){
                            break;
                        }
                    }
                    const char * endvalue;
/*
                    for (endvalue = startvalue; *endvalue ; endvalue++) {
                        TODO("RZ: Support space in value")
                        if (isspace(*endvalue) || *endvalue == '#'){
                            break;
                        }
                    }
                    TODO("RZ: Possible buffer overflow if length of value is larger than 128 bytes")
                    memcpy(value, startvalue, endvalue - startvalue + 1);
                    value[endvalue - startvalue + 1] = 0;
*/
                    char *curvalue = value;
                    for (endvalue = startvalue; *endvalue ; endvalue++) {
                        if (isspace(*endvalue) || *endvalue == '#'){
                            break;
                        }
                        else if ((*endvalue == '\\') && *(endvalue + 1)) {
                            if (endvalue > startvalue) {
                                memcpy(curvalue, startvalue, endvalue - startvalue);
                                curvalue += (endvalue - startvalue);
                            }

                            endvalue++;

                            *curvalue++ = *endvalue;

                            startvalue = endvalue + 1;
                        }
                    }
                    if (endvalue > startvalue) {
                        memcpy(curvalue, startvalue, endvalue - startvalue);
                        curvalue += (endvalue - startvalue);
                    }
                    *curvalue = 0;

                    this->setglobal_from_file(key, value, context);
                    break;
                }
            }
        }
    }

    void setglobal_from_file(const char * key, const char * value, const char * context)
    {
        if (0 == strcmp(context, "globals")){
            if (0 == strcmp(key, "bitmap_cache")){
                this->globals.bitmap_cache = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "bitmap_compression")){
                this->globals.bitmap_compression = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "port")){
                this->globals.port = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "nomouse")){
                this->globals.nomouse = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "notimestamp")){
                this->globals.notimestamp = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "encryptionLevel")){
                this->globals.encryptionLevel = level_from_cstr(value);
            }
            else if (0 == strcmp(key, "authip")){
                strncpy(this->globals.authip, value, sizeof(this->globals.authip));
                this->globals.authip[sizeof(this->globals.authip) - 1] = 0;
            }
            else if (0 == strcmp(key, "authport")){
                this->globals.authport = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "autovalidate")){
                this->globals.autovalidate = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "max_tick")){
                this->globals.max_tick = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "keepalive_grace_delay")){
                this->globals.keepalive_grace_delay = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "internal_domain")){
                this->globals.internal_domain = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "dynamic_conf_path")){
                strncpy(this->globals.dynamic_conf_path, value, sizeof(this->globals.dynamic_conf_path));
                this->globals.dynamic_conf_path[sizeof(this->globals.dynamic_conf_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "auth_channel")){
                strncpy(this->globals.auth_channel, value, 8);
                this->globals.auth_channel[7] = 0;
            }
            else if (0 == strcmp(key, "enable_file_encryption")){
                this->globals.enable_file_encryption = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "enable_tls")){
                this->globals.enable_tls = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "listen_address")){
                strncpy(this->globals.listen_address, value, sizeof(this->globals.listen_address));
                this->globals.listen_address[sizeof(this->globals.listen_address) - 1] = 0;
            }
            else if (0 == strcmp(key, "enable_ip_transparent")){
                this->globals.enable_ip_transparent = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "certificate_password")){
                strncpy(this->globals.certificate_password, value, sizeof(this->globals.certificate_password));
                this->globals.certificate_password[sizeof(this->globals.certificate_password) - 1] = 0;
            }
            else if (0 == strcmp(key, "png_path")){
                strncpy(this->globals.png_path, value, sizeof(this->globals.png_path));
                this->globals.png_path[sizeof(this->globals.png_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "wrm_path")){
                strncpy(this->globals.wrm_path, value, sizeof(this->globals.wrm_path));
                this->globals.wrm_path[sizeof(this->globals.wrm_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "alternate_shell")) {
                strncpy(this->globals.alternate_shell, value, sizeof(this->globals.alternate_shell));
                this->globals.alternate_shell[sizeof(this->globals.alternate_shell) - 1] = 0;
            }
            else if (0 == strcmp(key, "shell_working_directory")) {
                strncpy(this->globals.shell_working_directory, value, sizeof(this->globals.shell_working_directory));
                this->globals.shell_working_directory[sizeof(this->globals.shell_working_directory) - 1] = 0;
            }
            else if (0 == strcmp(key, "codec_id")) {
                strncpy(this->globals.codec_id, value, sizeof(this->globals.codec_id));
                this->globals.codec_id[sizeof(this->globals.codec_id) - 1] = 0;
            }
            else if (0 == strcmp(key, "movie")){
                this->globals.movie = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "movie_path")) {
                strncpy(this->globals.movie_path, value, sizeof(this->globals.movie_path));
                this->globals.movie_path[sizeof(this->globals.movie_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "video_quality")) {
                strncpy(this->globals.video_quality, value, sizeof(this->globals.video_quality));
                this->globals.video_quality[sizeof(this->globals.video_quality) - 1] = 0;
            }
            else if (0 == strcmp(key, "enable_bitmap_update")){
                this->globals.enable_bitmap_update = bool_from_cstr(value);
            }
        }
        else if (0 == strcmp(context, "client")){
            if (0 == strcmp(key, "ignore_logon_password")){
                this->client.ignore_logon_password = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance_flags_default")){
                this->client.performance_flags_default = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance_flags_force_present")){
                this->client.performance_flags_force_present = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "performance_flags_force_not_present")){
                this->client.performance_flags_force_not_present = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "tls_fallback_legacy")){
                this->client.tls_fallback_legacy = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "clipboard")){
                this->client.clipboard = bool_from_cstr(value);
            }
            else if (0 == strcmp(key, "device_redirection")){
                this->client.device_redirection = bool_from_cstr(value);
            }
        }
        else if (0 == strcmp(context, "video")){
            if (0 == strcmp(key, "capture_flags")){
                this->video.capture_flags   = ulong_from_cstr(value);
                this->video.capture_png = 0 != (this->video.capture_flags & 1);
                this->video.capture_wrm = 0 != (this->video.capture_flags & 2);
                this->video.capture_flv = 0 != (this->video.capture_flags & 4);
                this->video.capture_ocr = 0 != (this->video.capture_flags & 8);
            }
            else if (0 == strcmp(key, "ocr_interval")){
                this->video.ocr_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "png_interval")){
                this->video.png_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "capture_groupid")){
                this->video.capture_groupid  = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "frame_interval")){
                this->video.frame_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "break_interval")){
                this->video.break_interval   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "png_limit")){
                this->video.png_limit   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "replay_path")){
                strncpy(this->video.replay_path, value, sizeof(this->video.replay_path));
                this->video.replay_path[sizeof(this->video.replay_path) - 1] = 0;
            }
            else if (0 == strcmp(key, "l_bitrate")){
                this->video.l_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_framerate")){
                this->video.l_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_height")){
                this->video.l_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_width")){
                this->video.l_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "l_qscale")){
                this->video.l_qscale    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_bitrate")){
                this->video.m_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_framerate")){
                this->video.m_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_height")){
                this->video.m_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_width")){
                this->video.m_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "m_qscale")){
                this->video.m_qscale    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_bitrate")){
                this->video.h_bitrate   = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_framerate")){
                this->video.h_framerate = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_height")){
                this->video.h_height    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_width")){
                this->video.h_width     = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "h_qscale")){
                this->video.h_qscale    = ulong_from_cstr(value);
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "debug")){
                 if (0 == strcmp(key, "x224")){
                this->debug.x224              = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mcs")){
                this->debug.mcs               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "sec")){
                this->debug.sec               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "rdp")){
                this->debug.rdp               = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "primary_orders")){
                this->debug.primary_orders    = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "secondary_orders")){
                this->debug.secondary_orders  = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "bitmap")){
                this->debug.bitmap            = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "capture")){
                this->debug.capture           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "auth")){
                this->debug.auth              = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "session")){
                this->debug.session           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "front")){
                this->debug.front             = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_rdp")){
                this->debug.mod_rdp           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_vnc")){
                this->debug.mod_vnc           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_int")){
                this->debug.mod_int           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "mod_xup")){
                this->debug.mod_xup           = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "widget")){
                this->debug.widget            = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "input")){
                this->debug.input             = ulong_from_cstr(value);
            }
            else if (0 == strcmp(key, "log_type")){
                this->debug.log_type = logtype_from_cstr(value);
            }
            else if (0 == strcmp(key, "log_file_path")){
                strncpy(this->debug.log_file_path, value, sizeof(this->debug.log_file_path));
                this->debug.log_file_path[sizeof(this->debug.log_file_path) - 1] = 0;
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else if (0 == strcmp(context, "translation")){
                 if (0 == strcmp(key, "button_ok")){
                this->translation.button_ok.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "button_cancel")){
                this->translation.button_cancel.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "button_help")){
                this->translation.button_help.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "button_close")){
                this->translation.button_close.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "button_refused")){
                this->translation.button_refused.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "login")){
                this->translation.login.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "username")){
                this->translation.username.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "password")){
                this->translation.password.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "target")){
                this->translation.target.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "diagnostic")){
                this->translation.diagnostic.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "connection_closed")){
                this->translation.connection_closed.set_from_cstr(value);
            }
            else if (0 == strcmp(key, "help_message")){
                this->translation.help_message.set_from_cstr(value);
            }
            else {
                LOG(LOG_ERR, "unknown parameter %s in section [%s]", key, context);
            }
        }
        else {
            LOG(LOG_ERR, "unknown section [%s]", context);
        }
    }
    TODO("Should only be used by Authentifier "
         "It currently ask if the field has been modified "
         "and set it to not modified if it is not asked ")
    bool context_has_changed(authid_t authid) {
        bool res = false;
        try {
            BaseField * field = this->field_list.at(authid);
            res = field->has_changed();
            field->use();
        }
        catch (const std::out_of_range & oor){
            LOG(LOG_WARNING, "Inifile::context_is_asked(id): unknown authid=%d", authid);
            res = false;
        }
        return res;
        /*
        bool res;
        switch (authid) {
        case AUTHID_OPT_BPP:
            res = this->context.opt_bpp.has_changed();
            this->context.opt_bpp.use();
            break;

        case AUTHID_OPT_HEIGHT:
            res = this->context.opt_height.has_changed();
            this->context.opt_height.use();
            break;

        case AUTHID_OPT_WIDTH:
            res = this->context.opt_width.has_changed();
            this->context.opt_width.use();
            break;

        case AUTHID_SELECTOR:
            res = this->context.selector.has_changed();
            this->context.selector.use();
            break;

        case AUTHID_SELECTOR_CURRENT_PAGE:
            res = this->context.selector_current_page.has_changed();
            this->context.selector_current_page.use();
            break;

        case AUTHID_SELECTOR_DEVICE_FILTER:
            res = this->context.selector_device_filter.has_changed();
            this->context.selector_device_filter.use();
            break;

        case AUTHID_SELECTOR_GROUP_FILTER:
            res = this->context.selector_group_filter.has_changed();
            this->context.selector_group_filter.use();
            break;

        case AUTHID_SELECTOR_LINES_PER_PAGE:
            res = this->context.selector_lines_per_page.has_changed();
            this->context.selector_lines_per_page.use();
            break;

        case AUTHID_TARGET_DEVICE:
            res = this->globals.target_device.has_changed();
            this->globals.target_device.use();
            break;

        case AUTHID_TARGET_PASSWORD:
            res = this->context.target_password.has_changed();
            this->context.target_password.use();
            break;

        case AUTHID_TARGET_PORT:
            res = this->context.target_port.has_changed();
            this->context.target_port.use();
            break;

        case AUTHID_TARGET_PROTOCOL:
            res = this->context.target_protocol.has_changed();
            this->context.target_protocol.use();
            break;

        case AUTHID_TARGET_USER:
            res = this->globals.target_user.has_changed();
            this->globals.target_user.use();
            break;

        case AUTHID_AUTH_USER:
            res = this->globals.auth_user.has_changed();
            this->globals.auth_user.use();
            break;

        case AUTHID_HOST:
            res = this->globals.host.has_changed();
            this->globals.host.use();
            break;

        case AUTHID_TARGET:
            res = this->globals.target.has_changed();
            this->globals.target.use();
            break;

        case AUTHID_PASSWORD:
            res = this->context.password.has_changed();
            this->context.password.use();
            break;

        case AUTHID_AUTHCHANNEL_RESULT:
            res = this->context.authchannel_result.has_changed();
            this->context.authchannel_result.use();
            break;

        case AUTHID_AUTHCHANNEL_TARGET:
            res = this->context.authchannel_target.has_changed();
            this->context.authchannel_target.use();
            break;

        case AUTHID_ACCEPT_MESSAGE:
            res = this->context.accept_message.has_changed();
            this->context.accept_message.use();
            break;

        case AUTHID_DISPLAY_MESSAGE:
            res = this->context.display_message.has_changed();
            this->context.display_message.use();
            break;

        case AUTHID_KEEPALIVE:
            res = this->context.keepalive.has_changed();
            this->context.keepalive.use();
            break;

        case AUTHID_PROXY_TYPE:
            res = this->context.proxy_type.has_changed();
            this->context.proxy_type.use();
            break;

        case AUTHID_TRACE_SEAL:
            res = this->context.trace_seal.has_changed();
            this->context.trace_seal.use();
            break;

        case AUTHID_REAL_TARGET_DEVICE:
            res = this->context.real_target_device.has_changed();
            this->context.real_target_device.use();
            break;

        default:
            LOG(LOG_WARNING, "Inifile::context_is_asked(id): unknown authid=%d", authid);
            return false;
        }
        return res;
        */
    }

    void context_set_value_by_string(const char * strauthid, const char * value) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            context_set_value(authid, value);
        }
        else {
            LOG(LOG_WARNING, "Inifile::context_set_value(strid): unknown strauthid=\"%s\"", strauthid);
        }
    }

    void context_set_value(authid_t authid, const char * value) {
        // this->field_list.at(authid)->set_from_cstr(value);
        switch (authid)
        {
	    /*
        case AUTHID_TRANS_BUTTON_OK:
            this->translation.button_ok.set_from_cstr(value);
            break;
        case AUTHID_TRANS_BUTTON_CANCEL:
            this->translation.button_cancel.set_from_cstr(value);
            break;
        case AUTHID_TRANS_BUTTON_HELP:
            this->translation.button_help.set_from_cstr(value);
            break;
        case AUTHID_TRANS_BUTTON_CLOSE:
            this->translation.button_close.set_from_cstr(value);
            break;
        case AUTHID_TRANS_BUTTON_REFUSED:
            this->translation.button_refused.set_from_cstr(value);
            break;
        case AUTHID_TRANS_LOGIN:
            this->translation.login.set_from_cstr(value);
            break;
        case AUTHID_TRANS_USERNAME:
            this->translation.username.set_from_cstr(value);
            break;
        case AUTHID_TRANS_PASSWORD:
            this->translation.password.set_from_cstr(value);
            break;
        case AUTHID_TRANS_TARGET:
            this->translation.target.set_from_cstr(value);
            break;
        case AUTHID_TRANS_DIAGNOSTIC:
            this->translation.diagnostic.set_from_cstr(value);
            break;
        case AUTHID_TRANS_CONNECTION_CLOSED:
            this->translation.connection_closed.set_from_cstr(value);
            break;
        case AUTHID_TRANS_HELP_MESSAGE:
            this->translation.help_message.set_from_cstr(value);
            break;
	    */
        // Options
        case AUTHID_OPT_CLIPBOARD:
            this->client.clipboard = bool_from_cstr(value);
            break;
        case AUTHID_OPT_DEVICEREDIRECTION:
            this->client.device_redirection = bool_from_cstr(value);
            break;
        case AUTHID_OPT_FILE_ENCRYPTION:
            this->globals.enable_file_encryption = bool_from_cstr(value);
            break;

        // Video capture
        case AUTHID_OPT_CODEC_ID:
            strncpy(this->globals.codec_id, value, sizeof(this->globals.codec_id));
            this->globals.codec_id[sizeof(this->globals.codec_id) - 1] = 0;
            break;
        case AUTHID_OPT_MOVIE:
            this->globals.movie = bool_from_cstr(value);
            break;
        case AUTHID_OPT_MOVIE_PATH:
            strncpy(this->globals.movie_path, value, sizeof(this->globals.movie_path));
            this->globals.movie_path[sizeof(this->globals.movie_path) - 1] = 0;
            break;
        case AUTHID_VIDEO_QUALITY:
            strncpy(this->globals.video_quality, value, sizeof(this->globals.video_quality));
            this->globals.video_quality[sizeof(this->globals.video_quality) - 1] = 0;
            break;

        // Alternate shell
        case AUTHID_ALTERNATE_SHELL:
            strncpy(this->globals.alternate_shell, value, sizeof(this->globals.alternate_shell));
            this->globals.alternate_shell[sizeof(this->globals.alternate_shell) - 1] = 0;
            break;
        case AUTHID_SHELL_WORKING_DIRECTORY:
            strncpy(this->globals.shell_working_directory, value, sizeof(this->globals.shell_working_directory));
            this->globals.shell_working_directory[sizeof(this->globals.shell_working_directory) - 1] = 0;
            break;

        // Context
	    /*
        case AUTHID_OPT_BITRATE:
            this->context.opt_bitrate.set_from_cstr(value);
            break;
        case AUTHID_OPT_FRAMERATE:
            this->context.opt_framerate.set_from_cstr(value);
            break;
        case AUTHID_OPT_QSCALE:
            this->context.opt_qscale.set_from_cstr(value);
            break;

        case AUTHID_OPT_WIDTH:
            this->context.opt_width.set_from_cstr(value);
            break;
        case AUTHID_OPT_HEIGHT:
            this->context.opt_height.set_from_cstr(value);
            break;
        case AUTHID_OPT_BPP:
            this->context.opt_bpp.set_from_cstr(value);
            break;
	    */
        case AUTHID_AUTH_ERROR_MESSAGE:
            this->context.auth_error_message.copy_c_str(value);
            break;
	    /*
        case AUTHID_SELECTOR:
            this->context.selector.set_from_cstr(value);
            break;
        case AUTHID_SELECTOR_CURRENT_PAGE:
            this->context.selector_current_page.set_from_cstr(value);
            break;
        case AUTHID_SELECTOR_DEVICE_FILTER:
            this->context.selector_device_filter.set_from_cstr(value);
            break;
        case AUTHID_SELECTOR_GROUP_FILTER:
            this->context.selector_group_filter.set_from_cstr(value);
            break;
        case AUTHID_SELECTOR_LINES_PER_PAGE:
            this->context.selector_lines_per_page.set_from_cstr(value);
            break;
        case AUTHID_SELECTOR_NUMBER_OF_PAGES:
            this->context.selector_number_of_pages.set_from_cstr(value);
            break;

        case AUTHID_TARGET_DEVICE:
            this->globals.target_device.set_from_cstr(value);
            break;
        case AUTHID_TARGET_PASSWORD:
            this->context.target_password.set_from_cstr(value);
            break;
        case AUTHID_TARGET_PORT:
            this->context.target_port.set_from_cstr(value);
            break;
        case AUTHID_TARGET_PROTOCOL:
            this->context.target_protocol.set_from_cstr(value);
            break;
        case AUTHID_TARGET_USER:
            this->globals.target_user.set_from_cstr(value);
            break;

        case AUTHID_AUTH_USER:
            this->globals.auth_user.set_from_cstr(value);
            break;
        case AUTHID_HOST:
            this->globals.host.set_from_cstr(value);
            break;

        case AUTHID_TARGET:
            this->globals.target.set_from_cstr(value);
            break;

        case AUTHID_PASSWORD:
            this->context.password.set_from_cstr(value);
            break;

        case AUTHID_AUTHCHANNEL_ANSWER:
            this->context.authchannel_answer.set_from_cstr(value);
            break;
        case AUTHID_AUTHCHANNEL_RESULT:
            this->context.authchannel_result.set_from_cstr(value);
            break;
        case AUTHID_AUTHCHANNEL_TARGET:
            this->context.authchannel_target.set_from_cstr(value);
            break;

        case AUTHID_MESSAGE:
            this->context.message.set_from_cstr(value);
            break;

        case AUTHID_ACCEPT_MESSAGE:
            this->context.accept_message.set_from_cstr(value);
            break;
        case AUTHID_DISPLAY_MESSAGE:
            this->context.display_message.set_from_cstr(value);
            break;

        case AUTHID_AUTHENTICATED:
            this->context.authenticated.set_from_cstr(value);
            break;
        case AUTHID_REJECTED:
            this->context.rejected.set_from_cstr(value);
            break;

        case AUTHID_KEEPALIVE:
            this->context.keepalive.set_from_cstr(value);
            break;
        case AUTHID_PROXY_TYPE:
            this->context.proxy_type.set_from_cstr(value);
            break;

        case AUTHID_TRACE_SEAL:
            this->context.trace_seal.set_from_cstr(value);
            break;

        case AUTHID_SESSION_ID:
            this->context.session_id.set_from_cstr(value);
            break;

        case AUTHID_END_DATE_CNX:
            this->context.end_date_cnx.set_from_cstr(value);
            break;
        case AUTHID_END_TIME:
            this->context.end_time.set_from_cstr(value);
            break;

        case AUTHID_MODE_CONSOLE:
            this->context.mode_console.set_from_cstr(value);
            break;
        case AUTHID_TIMEZONE:
            this->context.timezone.set_from_cstr(value);
            break;

        case AUTHID_REAL_TARGET_DEVICE:
            this->context.real_target_device.set_from_cstr(value);
            break;

        case AUTHID_AUTHENTICATION_CHALLENGE:
            this->context.authentication_challenge.set_from_cstr(value);
            break;
	    */
        default:
	    try {
		BaseField * field = this->field_list.at(authid);
		field->set_from_cstr(value);
	    }
	    catch (const std::out_of_range & oor){
		LOG(LOG_WARNING, "Inifile::context_set_value(id): unknown authid=%d", authid);
	    }
            // LOG(LOG_WARNING, "Inifile::context_set_value(id): unknown authid=%d", authid);
            break;
        }
    }

    const char * context_get_value_by_string(const char * strauthid, char * buffer, size_t size) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            return context_get_value(authid, buffer, size);
        }

        LOG(LOG_WARNING, "Inifile::context_get_value(strid): unknown strauthid=\"%s\"", strauthid);

        return "";
    }

    const char * context_get_value(authid_t authid, char * buffer, size_t size) {
        //return this->field_list.at(authid)->get_value();

        const char * pszReturn = "";

        if (size) { *buffer = 0; }

        switch (authid)
        {
	    /*
        case AUTHID_TRANS_BUTTON_OK:
            pszReturn = this->translation.button_ok.get_cstr();
            break;
        case AUTHID_TRANS_BUTTON_CANCEL:
            pszReturn = this->translation.button_cancel.get_cstr();
            break;
        case AUTHID_TRANS_BUTTON_HELP:
            pszReturn = this->translation.button_help.get_cstr();
            break;
        case AUTHID_TRANS_BUTTON_CLOSE:
            pszReturn = this->translation.button_close.get_cstr();
            break;
        case AUTHID_TRANS_BUTTON_REFUSED:
            pszReturn = this->translation.button_refused.get_cstr();
            break;
        case AUTHID_TRANS_LOGIN:
            pszReturn = this->translation.login.get_cstr();
            break;
        case AUTHID_TRANS_USERNAME:
            pszReturn = this->translation.username.get_cstr();
            break;
        case AUTHID_TRANS_PASSWORD:
            pszReturn = this->translation.password.get_cstr();
            break;
        case AUTHID_TRANS_TARGET:
            pszReturn = this->translation.target.get_cstr();
            break;
        case AUTHID_TRANS_DIAGNOSTIC:
            pszReturn = this->translation.diagnostic.get_cstr();
            break;
        case AUTHID_TRANS_CONNECTION_CLOSED:
            pszReturn = this->translation.connection_closed.get_cstr();
            break;
        case AUTHID_TRANS_HELP_MESSAGE:
            pszReturn = this->translation.help_message.get_cstr();
            break;
	    */
        case AUTHID_OPT_CLIPBOARD:
            if (size) {
                strncpy(buffer, (this->client.clipboard ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_OPT_DEVICEREDIRECTION:
            if (size) {
                strncpy(buffer, (this->client.device_redirection ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_OPT_FILE_ENCRYPTION:
            if (size) {
                strncpy(buffer, (this->globals.enable_file_encryption ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;

        case AUTHID_OPT_CODEC_ID:
            if (size) {
                strncpy(buffer, this->globals.codec_id, size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_OPT_MOVIE:
            if (size) {
                strncpy(buffer, (this->globals.movie ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_OPT_MOVIE_PATH:
            if (size) {
                strncpy(buffer, this->globals.movie_path, size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_VIDEO_QUALITY:
            if (size) {
                strncpy(buffer, this->globals.video_quality, size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;

        case AUTHID_ALTERNATE_SHELL:
            pszReturn = this->globals.alternate_shell;
            break;
        case AUTHID_SHELL_WORKING_DIRECTORY:
            pszReturn = this->globals.shell_working_directory;
            break;
	    /*
        case AUTHID_OPT_BITRATE:
            pszReturn = this->context.opt_bitrate.get_value();
            // if (size) {
            //     snprintf(buffer, size, "%u", this->context.opt_bitrate);
            //     pszReturn = buffer;
            // }
            break;
        case AUTHID_OPT_FRAMERATE:
            pszReturn = this->context.opt_framerate.get_value();
            // if (size) {
            //     snprintf(buffer, size, "%u", this->context.opt_framerate);
            //     pszReturn = buffer;
            // }
            break;
        case AUTHID_OPT_QSCALE:
            pszReturn = this->context.opt_qscale.get_value();
            // if (size) {
            //     snprintf(buffer, size, "%u", this->context.opt_qscale);
            //     pszReturn = buffer;
            // }
            break;

        case AUTHID_OPT_BPP:
            pszReturn = this->context.opt_bpp.get_value();
            // if (  size
            //       && !this->context.opt_bpp.is_asked()) {
            //     snprintf(buffer, size, "%u", this->context.opt_bpp.get());
            //     pszReturn = buffer;
            // }
            break;
        case AUTHID_OPT_HEIGHT:
            pszReturn = this->context.opt_height.get_value();
            // if (  size
            //       && !this->context.opt_height.is_asked()) {
            //     snprintf(buffer, size, "%u", this->context.opt_height.get());
            //     pszReturn = buffer;
            // }
            break;
        case AUTHID_OPT_WIDTH:
            pszReturn = this->context.opt_width.get_value();
            // if (  size
            //       && !this->context.opt_width.is_asked()) {
            //     snprintf(buffer, size, "%u", this->context.opt_width.get());
            //     pszReturn = buffer;
            // }
            break;
	    */
        case AUTHID_AUTH_ERROR_MESSAGE:
            pszReturn = this->context.auth_error_message.c_str();
            break;
	    /*
        case AUTHID_SELECTOR:
            if (  size
                  && !this->context.selector.is_asked()) {
                strncpy(buffer, (this->context.selector.get() ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_SELECTOR_CURRENT_PAGE:
            if (  size
                  && !this->context.selector_current_page.is_asked()) {
                snprintf(buffer, size, "%u", this->context.selector_current_page.get());
                pszReturn = buffer;
            }
            break;
        case AUTHID_SELECTOR_DEVICE_FILTER:
            if (!this->context.selector_device_filter.is_asked()) {
                pszReturn = this->context.selector_device_filter.get_cstr();
            }
            break;
        case AUTHID_SELECTOR_GROUP_FILTER:
            if (!this->context.selector_group_filter.is_asked()) {
                pszReturn = this->context.selector_group_filter.get_cstr();
            }
            break;
        case AUTHID_SELECTOR_LINES_PER_PAGE:
            if (  size
                  && !this->context.selector_group_filter.is_asked()) {
                snprintf(buffer, size, "%u", this->context.selector_lines_per_page.get());
                pszReturn = buffer;
            }
            break;
        case AUTHID_SELECTOR_NUMBER_OF_PAGES:
            if (size) {
                snprintf(buffer, size, "%u", this->context.selector_number_of_pages.get());
                pszReturn = buffer;
            }
            break;

        case AUTHID_TARGET_DEVICE:
            if (!this->globals.target_device.is_asked()) {
                pszReturn = this->globals.target_device.get_cstr();
            }
            break;
        case AUTHID_TARGET_PASSWORD:
            if (!this->context.target_password.is_asked()) {
                pszReturn = this->context.target_password.get_cstr();
            }
            break;
        case AUTHID_TARGET_PORT:
            if (  size
                  && !this->context.target_port.is_asked()) {
                snprintf(buffer, size, "%u", this->context.target_port.get());
                pszReturn = buffer;
            }
            break;
        case AUTHID_TARGET_PROTOCOL:
            if (!this->context.target_protocol.is_asked()) {
                pszReturn = this->context.target_protocol.get_cstr();
            }
            break;
        case AUTHID_TARGET_USER:
            if (!this->globals.target_user.is_asked()) {
                pszReturn = this->globals.target_user.get_cstr();
            }
            break;

        case AUTHID_AUTH_USER:
            if (!this->globals.auth_user.is_asked()) {
                pszReturn = this->globals.auth_user.get_cstr();
            }
            break;
        case AUTHID_HOST:
            if (!this->globals.host.is_asked()) {
                pszReturn = this->globals.host.get_cstr();
            }
            break;

        case AUTHID_TARGET:
            if (  size
                  && !this->globals.target.is_asked()) {
                pszReturn = this->globals.target.get_cstr();
            }
            break;
        case AUTHID_PASSWORD:
            if (!this->context.password.is_asked()) {
                pszReturn = this->context.password.get_cstr();
            }
            break;

        case AUTHID_AUTHCHANNEL_ANSWER:
            pszReturn = this->context.authchannel_answer.get_cstr();
            break;
        case AUTHID_AUTHCHANNEL_RESULT:
            if (!this->context.authchannel_result.is_asked()) {
                pszReturn = this->context.authchannel_result.get_cstr();
            }
            break;
        case AUTHID_AUTHCHANNEL_TARGET:
            if (!this->context.authchannel_target.is_asked()) {
                pszReturn = this->context.authchannel_target.get_cstr();
            }
            break;

        case AUTHID_MESSAGE:
            pszReturn = this->context.message.get_cstr();
            break;
        case AUTHID_ACCEPT_MESSAGE:
            if (!this->context.accept_message.is_asked()) {
                pszReturn = this->context.accept_message.get_cstr();
            }
            break;
        case AUTHID_DISPLAY_MESSAGE:
            if (!this->context.display_message.is_asked()) {
                pszReturn = this->context.display_message.get_cstr();
            }
            break;

        case AUTHID_AUTHENTICATED:
            if (size) {
                strncpy(buffer, (this->context.authenticated.get() ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;
        case AUTHID_REJECTED:
            pszReturn = this->context.rejected.get_cstr();
            break;

        case AUTHID_KEEPALIVE:
            if (  size
                  && !this->context.keepalive.is_asked()) {
                strncpy(buffer, (this->context.keepalive.get() ? "True" : "False"), size);
                buffer[size - 1] = 0;
                pszReturn = buffer;
            }
            break;

        case AUTHID_PROXY_TYPE:
            if (!this->context.proxy_type.is_asked()) {
                pszReturn = this->context.proxy_type.get_cstr();
            }
            break;
        case AUTHID_TRACE_SEAL:
            if (!this->context.trace_seal.is_asked()) {
                pszReturn = this->context.trace_seal.get_cstr();
            }
            break;

        case AUTHID_SESSION_ID:
            pszReturn = this->context.session_id.get_cstr();
            break;

        case AUTHID_END_DATE_CNX:
            if (size) {
                snprintf(buffer, size, "%u", this->context.end_date_cnx.get());
                pszReturn = buffer;
            }
            break;
        case AUTHID_END_TIME:
            pszReturn = this->context.end_time.get_cstr();
            break;

        case AUTHID_MODE_CONSOLE:
            pszReturn = this->context.mode_console.get_cstr();
            break;
        case AUTHID_TIMEZONE:
            if (size) {
                snprintf(buffer, size, "%d", this->context.timezone.get());
                pszReturn = buffer;
            }
            break;

        case AUTHID_REAL_TARGET_DEVICE:
            pszReturn = this->context.real_target_device.get_cstr();
            break;

        case AUTHID_AUTHENTICATION_CHALLENGE:
            pszReturn = this->context.authentication_challenge.get_cstr();
            break;
	    */
        default:
	    try{
		BaseField * field = this->field_list.at(authid);
		if (!field->is_asked())
		    pszReturn = field->get_value();
	    }
	    catch (const std::out_of_range & oor){
		LOG(LOG_WARNING, "Inifile::context_get_value(id): unknown authid=\"%d\"", authid);
	    }
            break;
        }

        return pszReturn;
    }

    void context_ask_by_string(const char *strauthid) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            context_ask(authid);
        }
        else {
            LOG(LOG_WARNING, "Inifile::context_ask(strid): unknown strauthid=\"%s\"", strauthid);
        }
    }

    void context_ask(authid_t authid) {
        try{
            this->field_list.at(authid)->ask();
        }
        catch (const std::out_of_range & oor){
            LOG(LOG_WARNING, "Inifile::context_ask(id): unknown authid=%d", authid);
        }
        /*
        switch (authid) {
        case AUTHID_OPT_BPP:
            this->context.opt_bpp.ask();
            break;

        case AUTHID_OPT_HEIGHT:
            this->context.opt_height.ask();
            break;

        case AUTHID_OPT_WIDTH:
            this->context.opt_width.ask();
            break;

        case AUTHID_SELECTOR:
            this->context.selector.ask();
            break;

        case AUTHID_SELECTOR_CURRENT_PAGE:
            this->context.selector_current_page.ask();
            break;

        case AUTHID_SELECTOR_DEVICE_FILTER:
            this->context.selector_device_filter.ask();
            break;

        case AUTHID_SELECTOR_GROUP_FILTER:
            this->context.selector_group_filter.ask();
            break;

        case AUTHID_SELECTOR_LINES_PER_PAGE:
            this->context.selector_lines_per_page.ask();
            break;

        case AUTHID_TARGET_DEVICE:
            this->globals.target_device.ask();
            break;

        case AUTHID_TARGET_PASSWORD:
            this->context.target_password.ask();
            break;

        case AUTHID_TARGET_PORT:
            this->context.target_port.is_asked();
            break;

        case AUTHID_TARGET_PROTOCOL:
            this->context.target_protocol.ask();
            break;

        case AUTHID_TARGET_USER:
            this->globals.target_user.ask();
            break;

        case AUTHID_AUTH_USER:
            this->globals.auth_user.ask();
            break;

        case AUTHID_HOST:
            this->globals.host.ask();
            // this->globals.state_host.asked                    = true;
            // this->globals.state_host.modified                    = true;
            break;

        case AUTHID_TARGET:
            this->globals.target.ask();
            break;

        case AUTHID_PASSWORD:
            this->context.password.ask();
            break;

        case AUTHID_AUTHCHANNEL_RESULT:
            this->context.authchannel_result.ask();
            break;

        case AUTHID_AUTHCHANNEL_TARGET:
            this->context.authchannel_target.ask();
            break;

        case AUTHID_ACCEPT_MESSAGE:
            this->context.accept_message.ask();
            break;

        case AUTHID_DISPLAY_MESSAGE:
            this->context.display_message.ask();
            break;

        case AUTHID_KEEPALIVE:
            this->context.keepalive.ask();
            break;

        case AUTHID_PROXY_TYPE:
            this->context.proxy_type.ask();
            break;

        case AUTHID_TRACE_SEAL:
            this->context.trace_seal.ask();
            break;

        default:
            LOG(LOG_WARNING, "Inifile::context_ask(id): unknown authid=%d", authid);
            break;
        }*/
    }

    bool context_is_asked_by_string(const char *strauthid) {
        authid_t authid = authid_from_string(strauthid);
        if (authid != AUTHID_UNKNOWN) {
            return context_is_asked(authid);
        }

        LOG(LOG_WARNING, "Inifile::context_is_asked(strid): unknown strauthid=\"%s\"", strauthid);

        return false;
    }

    bool context_is_asked(authid_t authid) {
        bool res = false;
        try{
        res = this->field_list.at(authid)->is_asked();
        }
        catch (const std::out_of_range & oor) {
            LOG(LOG_WARNING, "Inifile::context_is_asked(id): unknown authid=%d", authid);
        }
        return res;
        /*
        switch (authid) {
        case AUTHID_OPT_BPP:
            return this->context.opt_bpp.is_asked();

        case AUTHID_OPT_HEIGHT:
            return this->context.opt_height.is_asked();

        case AUTHID_OPT_WIDTH:
            return this->context.opt_width.is_asked();

        case AUTHID_SELECTOR:
            return this->context.selector.is_asked();

        case AUTHID_SELECTOR_CURRENT_PAGE:
            return this->context.selector_current_page.is_asked();

        case AUTHID_SELECTOR_DEVICE_FILTER:
            return this->context.selector_device_filter.is_asked();

        case AUTHID_SELECTOR_GROUP_FILTER:
            return this->context.selector_group_filter.is_asked();

        case AUTHID_SELECTOR_LINES_PER_PAGE:
            return this->context.selector_lines_per_page.is_asked();

        case AUTHID_TARGET_DEVICE:
            return this->globals.target_device.is_asked();

        case AUTHID_TARGET_PASSWORD:
            return this->context.target_password.is_asked();

        case AUTHID_TARGET_PORT:
            return this->context.target_port.is_asked();

        case AUTHID_TARGET_PROTOCOL:
            return this->context.target_protocol.is_asked();

        case AUTHID_TARGET_USER:
            return this->globals.target_user.is_asked();

        case AUTHID_AUTH_USER:
            return this->globals.auth_user.is_asked();

        case AUTHID_HOST:
            return this->globals.host.is_asked();

        case AUTHID_TARGET:
            return this->globals.target.is_asked();

        case AUTHID_PASSWORD:
            return this->context.password.is_asked();

        case AUTHID_AUTHCHANNEL_RESULT:
            return this->context.authchannel_result.is_asked();

        case AUTHID_AUTHCHANNEL_TARGET:
            return this->context.authchannel_target.is_asked();

        case AUTHID_ACCEPT_MESSAGE:
            return this->context.accept_message.is_asked();

        case AUTHID_DISPLAY_MESSAGE:
            return this->context.display_message.is_asked();

        case AUTHID_KEEPALIVE:
            return this->context.keepalive.is_asked();

        case AUTHID_PROXY_TYPE:
            return this->context.proxy_type.is_asked();

        case AUTHID_TRACE_SEAL:
            return this->context.trace_seal.is_asked();

        case AUTHID_REAL_TARGET_DEVICE:
            return false;

        default:
            LOG(LOG_WARNING, "Inifile::context_is_asked(id): unknown authid=%d", authid);
            return false;
        }*/
    }

    bool context_get_bool(authid_t authid) {
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
        default:
            LOG(LOG_WARNING, "Inifile::context_get_bool(id): unknown authid=\"%d\"", authid);
            break;
        }

        return false;
    }

    void cparse(const char * filename) {
        ifstream inifile(filename);
        this->cparse(inifile);
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

        this->context_ask(AUTHID_SELECTOR);
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
            this->context_ask(AUTHID_TARGET_USER);
        }
        else {
            this->context_set_value(AUTHID_TARGET_USER, target_user);
        }
        if (*target_device == 0) {
            this->context_ask(AUTHID_TARGET_DEVICE);
        }
        else {
            this->context_set_value(AUTHID_TARGET_DEVICE, target_device);
        }
        if (*target_protocol == 0) {
            this->context_ask(AUTHID_TARGET_PROTOCOL);
        }
        else {
            this->context_set_value(AUTHID_TARGET_PROTOCOL, target_protocol);
        }
        if (*auth_user == 0) {
            this->context_ask(AUTHID_AUTH_USER);
        }
        else {
            this->context_set_value(AUTHID_AUTH_USER, auth_user);
        }
    }


};

#endif
