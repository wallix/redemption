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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   header file. Check object, used to check file's existance
*/

#ifndef _REDEMPTION_CORE_CHECK_FILES_HPP_
#define _REDEMPTION_CORE_CHECK_FILES_HPP_

#include <iostream>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

TODO("merge that with find and unlink")

/*
struct Check_files {
    struct {
        bool ad24b;
        bool cursor0;
        bool cursor1;
        bool font;
        bool logo;
        bool wallix_logo;
    } share;

    struct {
        bool keys;
        bool config_file;
        bool rdpproxy_crt;
        bool rdpproxy_key;
        bool dh1024_pem;
    } cfg;

    Check_files() {
        this->share.ad24b   = false;
        this->share.cursor0 = false;
        this->share.cursor1 = false;
        this->share.font    = false;
        this->share.logo    = false;

        this->cfg.keys         = false;
        this->cfg.config_file  = false;
        this->cfg.rdpproxy_crt = false;
        this->cfg.rdpproxy_key = false;
        this->cfg.dh1024_pem   = false;
    }

    bool check_share() {
        using namespace std;

        string login_1(LOGIN_LOGO24);
        string cursor_arrow(CURSOR0);
        string cursor_pointer(CURSOR1);
        string font(DEFAULT_FONT_NAME);
        string logo(REDEMPTION_LOGO24);

        DIR * dirp = opendir(SHARE_PATH);
        if (!dirp){
            cerr << "Error opening " SHARE_PATH " directory : "
                 << strerror(errno)
                 << endl;
            return 0;
        }
        struct dirent * d = readdir(dirp);
        while (d) {
            string line(d->d_name);

                 if (login_1.compare(line) == 0) {
                this->share.ad24b = true;
            }
            else if(cursor_arrow.compare(line) == 0) {
                this->share.cursor0 = true;
            }
            else if(cursor_pointer.compare(line) == 0) {
                this->share.cursor1 = true;
            }
            else if(font.compare(line) == 0) {
                this->share.font = true;
            }
            else if(logo.compare(line) == 0) {
                this->share.logo = true;
            }

            d = readdir(dirp);
        }
        closedir(dirp);

        return this->share.ad24b
            && this->share.cursor0
            && this->share.cursor1
            && this->share.font
            && this->share.logo;
    }

    bool check_etc() {
        using namespace std;

        string keys(RSAKEYS_INI);
        string config_file(RDPPROXY_INI);

        DIR * dirp = opendir(CFG_PATH);
        if (!dirp){
            cerr << "Error opening " CFG_PATH " directory : "
                 << strerror(errno)
                 << endl;
            return 0;
        }
        struct dirent * d = readdir(dirp);
        while (d) {
            string line(d->d_name);

                 if (keys.compare(line) == 0) {
                this->cfg.keys = true;
            }
            else if(config_file.compare(line) == 0) {
                this->cfg.config_file = true;
            }
            else if (!line.compare(RDPPROXY_CRT)) {
                this->cfg.rdpproxy_crt = true;
            }
            else if (!line.compare(RDPPROXY_KEY)) {
                this->cfg.rdpproxy_key = true;
            }
            else if (!line.compare(DH1024_PEM)) {
                this->cfg.dh1024_pem = true;
            }

            d = readdir(dirp);
        }
        closedir(dirp);

        return this->cfg.keys
            && this->cfg.config_file
            && this->cfg.rdpproxy_crt
            && this->cfg.rdpproxy_key
            && this->cfg.dh1024_pem;
    }
};
*/

struct CheckFileList
{
    const char * filename;
    const char * directory;
    bool         need_be_readable;
    bool         need_be_writeable;
    bool         is_readable;
    bool         is_writeable;
};

static CheckFileList user_check_file_list[] =
{
    { RDPPROXY_INI, CFG_PATH, true,  false, false, false },

    { NULL,         NULL,     false, false, false, false }
};

static CheckFileList euser_check_file_list[] =
{
    { LOGIN_LOGO24,      SHARE_PATH,      true,  false, false, false },
    { CURSOR0,           SHARE_PATH,      true,  false, false, false },
    { CURSOR1,           SHARE_PATH,      true,  false, false, false },
    { DEFAULT_FONT_NAME, SHARE_PATH,      true,  false, false, false },
    { REDEMPTION_LOGO24, SHARE_PATH,      true,  false, false, false },

    { "",                CERTIF_PATH,     false, true,  false, false },

    { "",                RECORD_PATH,     false, true,  false, false },

    { "",                RECORD_TMP_PATH, false, true,  false, false },

    { RSAKEYS_INI,       CFG_PATH,        true,  false, false, false },
    { RDPPROXY_INI,      CFG_PATH,        true,  false, false, false },
    { RDPPROXY_CRT,      CFG_PATH,        true,  false, false, false },
    { RDPPROXY_KEY,      CFG_PATH,        true,  false, false, false },
    { DH1024_PEM,        CFG_PATH,        true,  false, false, false },

    { NULL,              NULL,            false, false, false, false }
};

struct CheckFile {
    static bool check(CheckFileList * check_file_list) {
        char full_path[1024];
        bool result;

        result = true;

        for (CheckFileList * tmp_check_list = check_file_list;
             tmp_check_list->filename; tmp_check_list++)
        {
            if (!tmp_check_list->need_be_readable && !tmp_check_list->need_be_writeable)
            {
                REDASSERT(false);

                continue;
            }

            if (::strlen(tmp_check_list->directory) + 1 +
                ::strlen(tmp_check_list->filename) >= sizeof(full_path))
            {
                REDASSERT(false);

                continue;
            }

            ::snprintf(full_path, sizeof(full_path), "%s/%s",
                tmp_check_list->directory, tmp_check_list->filename);

            tmp_check_list->is_readable = false;
            if (tmp_check_list->need_be_readable)
            {
                if (!::eaccess(full_path, R_OK))
                {
                    tmp_check_list->is_readable = true;
                }
                else
                {
                    result = false;
                }
            }

            tmp_check_list->is_writeable = false;
            if (tmp_check_list->need_be_writeable)
            {
                if (!::eaccess(full_path, W_OK))
                {
                    tmp_check_list->is_writeable = true;
                }
                else
                {
                    result = false;
                }
            }
        }

        return result;
    }

    static void ShowErrors(CheckFileList * check_file_list, uid_t euid, gid_t egid)
    {
        for (CheckFileList * tmp_check_list = check_file_list;
             tmp_check_list->filename; tmp_check_list++)
        {
            const char * accessibility = NULL;

            if (tmp_check_list->need_be_readable && !tmp_check_list->is_readable &&
                tmp_check_list->need_be_writeable && !tmp_check_list->is_writeable)
            {
                accessibility = "read/write";
            }
            else if (tmp_check_list->need_be_readable && !tmp_check_list->is_readable)
            {
                accessibility = "read";
            }
            else if (tmp_check_list->need_be_writeable && !tmp_check_list->is_writeable)
            {
                accessibility = "write";
            }
            else
            {
                continue;
            }

            if (*tmp_check_list->filename)
            {
                LOG(LOG_ERR,
                    "User (euid=%u guid=%u) has no %s access to file \"%s/%s\"",
                    euid, egid, accessibility, tmp_check_list->directory,
                    tmp_check_list->filename);
            }
            else
            {
                LOG(LOG_ERR,
                    "User (euid=%u guid=%u) has no %s access to directory \"%s\"",
                    euid, egid, accessibility, tmp_check_list->directory);
            }
        }
    }

    static void ShowAll(CheckFileList * check_file_list, uid_t euid, gid_t egid)
    {
        for (CheckFileList * tmp_check_list = check_file_list;
             tmp_check_list->filename; tmp_check_list++)
        {
            const char * accessibility = NULL;

            if (!tmp_check_list->need_be_readable && !tmp_check_list->need_be_writeable)
            {
                continue;
            }

            if (tmp_check_list->need_be_readable && !tmp_check_list->is_readable &&
                tmp_check_list->need_be_writeable && !tmp_check_list->is_writeable)
            {
                accessibility = "read/write";
            }
            else if (tmp_check_list->need_be_readable && !tmp_check_list->is_readable)
            {
                accessibility = "read";
            }
            else if (tmp_check_list->need_be_writeable && !tmp_check_list->is_writeable)
            {
                accessibility = "write";
            }
            else
            {
                if (*tmp_check_list->filename)
                {
                    LOG(LOG_INFO,
                        "File \"%s\" at \"%s\" is present and accessible by user (euid=%u guid=%u) ... yes",
                        tmp_check_list->filename, tmp_check_list->directory, euid,
                        egid);
                }
                else
                {
                    LOG(LOG_INFO,
                        "Directory \"%s\" is present and accessible by user (euid=%u guid=%u) ... yes",
                        tmp_check_list->directory, euid, egid);
                }

                continue;
            }

            if (*tmp_check_list->filename)
            {
                LOG(LOG_INFO,
                    "User (euid=%u guid=%u) has no %s access to file \"%s/%s\"",
                    euid, egid, accessibility, tmp_check_list->directory,
                    tmp_check_list->filename);
            }
            else
            {
                LOG(LOG_INFO,
                    "User (euid=%u guid=%u) has no %s access to directory \"%s\"",
                    euid, egid, accessibility, tmp_check_list->directory);
            }
        }
    }
};

#endif
