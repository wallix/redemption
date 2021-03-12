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
  Copyright (C) Wallix 2020
  Author(s): Proxy Team
*/

#include "core/pid_file.hpp"
#include "core/app_path.hpp"
#include "utils/log.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/int_to_chars.hpp"

#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


namespace
{
    void init_pid_filename(std::string& filename, chars_view id)
    {
        str_assign(filename, app_path(AppPath::LockDir), "/session_", id, ".pid");
    }
}

PidFile::PidFile(int pid)
{
    auto session_id = int_to_decimal_chars(pid);
    init_pid_filename(this->filename, session_id);

    int fd = ::open(this->filename.c_str(), O_WRONLY | O_CREAT, S_IRWXU);
    if (fd == -1) {
        int errnum = errno;
        this->filename.clear();
        LOG(LOG_ERR, "Writing process id to SESSION ID FILE failed. Maybe no rights ?:%d:%s",
            errnum, strerror(errnum));
    }
    else {
        if (write(fd, session_id.data(), session_id.size()) == -1) {
            int errnum = errno;
            LOG(LOG_ERR, "Couldn't write pid to %s/session_<pid>.pid: %s",
                app_path(AppPath::LockDir), strerror(errnum));
            unlink(this->filename.c_str());
            this->filename.clear();
        }

        close(fd);
    }
}

PidFile::~PidFile()
{
    unlink(this->filename.c_str());
}

bool PidFile::is_open() const
{
    return !this->filename.empty();
}

void PidFile::rename(chars_view id)
{
    if (!id.empty()) {
        assert(this->is_open());
        auto old_filename = this->filename;
        init_pid_filename(this->filename, id);
        std::rename(old_filename.c_str(), this->filename.c_str());
    }
}
