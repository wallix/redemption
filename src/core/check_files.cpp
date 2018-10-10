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
Copyright (C) Wallix 2010-2018
Author(s): Christophe Grosjean, Jonathan Poelen
*/

#include "core/app_path.hpp"
#include "core/check_files.hpp"
#include "utils/log.hpp"
#include "utils/sugar/array_view.hpp"

#include <algorithm>


struct CheckFileData
{
    enum class Accessibility : bool { Read, Write, /* RearAndWrite*/ };
    enum class FileType : bool { File, Directory, };

    FileType      type;
    Accessibility accessibility;
    const char *  filename;
};

constexpr auto Readable = CheckFileData::Accessibility::Read;
constexpr auto Writable = CheckFileData::Accessibility::Write;

constexpr auto IsFile = CheckFileData::FileType::File;
constexpr auto IsDirectory = CheckFileData::FileType::Directory;

namespace
{
    CheckFileData user_check_file_list[] =
    {
        // Directories.
        { IsDirectory, Writable, app_path(AppPath::Certif)           },
        { IsDirectory, Writable, app_path(AppPath::Record)           },
        { IsDirectory, Writable, app_path(AppPath::RecordTmp)        },
        { IsDirectory, Writable, app_path(AppPath::Hash)             },
        { IsDirectory, Writable, app_path(AppPath::Video)            },
        { IsDirectory, Writable, app_path(AppPath::Ocr)              },
        { IsDirectory, Writable, app_path(AppPath::Png)              },
        { IsDirectory, Writable, app_path(AppPath::Wrm)              },
        { IsDirectory, Writable, app_path(AppPath::License)          },
        { IsDirectory, Writable, app_path(AppPath::Persistent)       },
        { IsDirectory, Writable, app_path(AppPath::PersistentRdp)    },
        { IsDirectory, Writable, app_path(AppPath::DriveRedirection) },
        { IsDirectory, Writable, app_path(AppPath::LockDir)          },

        //  Files
        { IsFile, Readable, app_path(AppPath::DefaultFontFile)  },
        { IsFile, Readable, app_path(AppPath::RedemptionLogo24) },
        { IsFile, Readable, app_path(AppPath::LoginWabBlue)     },
        { IsFile, Readable, app_path(AppPath::CfgIni)           },
        { IsFile, Readable, app_path(AppPath::CfgCrt)           },
        { IsFile, Readable, app_path(AppPath::CfgKey)           },
        { IsFile, Readable, app_path(AppPath::CfgDhPem)         },
    };

    bool check_files(
        array_view<CheckFileData const> check_file_list,
        unsigned euid, unsigned egid)
    {
        bool result = true;

        CheckFileData const* uses_files[std::size(user_check_file_list)];
        CheckFileData const** uses_files_last = uses_files;

        for (CheckFileData const& check_file : check_file_list)
        {
            // ignore duplicated files
            if (std::any_of(uses_files, uses_files_last, [&](auto* pcheck_file){
                using P = void const*;
                return P(pcheck_file->filename) == P(check_file.filename)
                    && pcheck_file->accessibility == check_file.accessibility
                    && pcheck_file->type == check_file.type
                ;
            })) {
                continue;
            }

            *uses_files_last = &check_file;
            ++uses_files_last;

            const char * accessibility = nullptr;
            int access_type = 0;

            switch (check_file.accessibility)
            {
            case Readable:
                accessibility = "read";
                access_type = R_OK;
                break;
            case Writable:
                accessibility = "write";
                access_type = W_OK;
                break;
            }

            if (!::eaccess(check_file.filename, access_type))
            {
                LOG(LOG_INFO,
                    "%s \"%s\" is present and accessible by user (euid=%u egid=%u) ... yes",
                    (check_file.type == IsFile ? "File" : "Directory"),
                    check_file.filename, euid, egid);
            }
            else
            {
                result = false;
                LOG(LOG_ERR,
                    "User (euid=%u egid=%u) has no %s access to %s \"%s\"",
                    euid, egid, accessibility,
                    (check_file.type == IsFile ? "file" : "directory"),
                    check_file.filename);
            }
        }

        return result;
    }
}  // namespace

bool check_files(unsigned euid, unsigned egid)
{
    return check_files(user_check_file_list, euid, egid);
}
