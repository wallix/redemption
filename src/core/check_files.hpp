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


#pragma once

#include <cstring>
#include <cstdio>
#include <sys/types.h>
#include "utils/log.hpp"
#include "core/app_path.hpp"

// TODO merge that with find and unlink

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
    { RDPPROXY_INI, app_path(AppPath::Cfg), true,  false, false, false },

    // End of list.
    { nullptr,         nullptr,     false, false, false, false }
};

static CheckFileList euser_check_file_list[] =
{
    // Directories.
    { "",                 app_path(AppPath::Certif),            false, true,  false, false },
    { "",                 app_path(AppPath::Record),            false, true,  false, false },
    { "",                 app_path(AppPath::RecordTmp),         false, true,  false, false },
    { "",                 app_path(AppPath::Hash),              false, true,  false, false },
    { "",                 app_path(AppPath::Video),             false, true,  false, false },
    { "",                 app_path(AppPath::Ocr),               false, true,  false, false },
    { "",                 app_path(AppPath::Png),               false, true,  false, false },
    { "",                 app_path(AppPath::License),           false, true,  false, false },
    { "",                 app_path(AppPath::Certif),            false, true,  false, false },
    { "",                 app_path(AppPath::Persistent),        false, true,  false, false },
    { "",                 app_path(AppPath::DriveRedirection),  false, true,  false, false },
    { "",                 app_path(AppPath::PidParent),         false, true,  false, false },

    //  Files
    { DEFAULT_FONT_NAME,  app_path(AppPath::Share),             true,  false, false, false },
    { REDEMPTION_LOGO24,  app_path(AppPath::Share),             true,  false, false, false },
    { LOGIN_WAB_BLUE,     app_path(AppPath::Share),             true,  false, false, false },
    { LOCKFILE,           app_path(AppPath::PidParent),         true,  false, false, false },
    { RDPPROXY_INI,       app_path(AppPath::Cfg),               true,  false, false, false },
//     { LOGO_PNG,           CFG_THEMES_PATH, true,  false, false, false },
//     { LOGO_BMP,           CFG_THEMES_PATH, true,  false, false, false },
//     { THEME_INI,          CFG_THEMES_PATH, true,  false, false, false },
    { RDPPROXY_KEY,       app_path(AppPath::Cfg),               true,  false, false, false },
    { RDPPROXY_CRT,       app_path(AppPath::Cfg),               true,  false, false, false },
    { DH_PEM,             app_path(AppPath::Cfg),               true,  false, false, false },


    // End of list.
    { nullptr,              nullptr,            false, false, false, false }
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
                assert(false);

                continue;
            }

            if (::strlen(tmp_check_list->directory) + 1 +
                ::strlen(tmp_check_list->filename) >= sizeof(full_path))
            {
                assert(false);

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
            const char * accessibility = nullptr;

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
                    "User (euid=%u egid=%u) has no %s access to file \"%s/%s\"",
                    euid, egid, accessibility, tmp_check_list->directory,
                    tmp_check_list->filename);
            }
            else
            {
                LOG(LOG_ERR,
                    "User (euid=%u egid=%u) has no %s access to directory \"%s\"",
                    euid, egid, accessibility, tmp_check_list->directory);
            }
        }
    }

    static void ShowAll(CheckFileList * check_file_list, uid_t euid, gid_t egid)
    {
        for (CheckFileList * tmp_check_list = check_file_list;
             tmp_check_list->filename; tmp_check_list++)
        {
            const char * accessibility = nullptr;

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
                        "File \"%s\" at \"%s\" is present and accessible by user (euid=%u egid=%u) ... yes",
                        tmp_check_list->filename, tmp_check_list->directory, euid,
                        egid);
                }
                else
                {
                    LOG(LOG_INFO,
                        "Directory \"%s\" is present and accessible by user (euid=%u egid=%u) ... yes",
                        tmp_check_list->directory, euid, egid);
                }

                continue;
            }

            if (*tmp_check_list->filename)
            {
                LOG(LOG_INFO,
                    "User (euid=%u egid=%u) has no %s access to file \"%s/%s\"",
                    euid, egid, accessibility, tmp_check_list->directory,
                    tmp_check_list->filename);
            }
            else
            {
                LOG(LOG_INFO,
                    "User (euid=%u egid=%u) has no %s access to directory \"%s\"",
                    euid, egid, accessibility, tmp_check_list->directory);
            }
        }
    }
};

