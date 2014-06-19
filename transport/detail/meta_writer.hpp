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
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_TRANSPORT_DETAIL_META_WRITER_HPP
#define REDEMPTION_TRANSPORT_DETAIL_META_WRITER_HPP

#include "sequence_generator.hpp"
#include "error.hpp"
#include "log.hpp"
#include "auth_api.hpp"

#include <limits>
#include <cerrno>
#include <stdint.h>

namespace detail
{
    struct MetaFilename
    {
        char filename[2048];

        MetaFilename(const char * path, const char * basename,
                     FilenameFormat format = FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION)
        {
            int res = format == (
               FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION
            || format == FilenameGenerator::PATH_FILE_PID_EXTENSION)
            ? snprintf(this->filename, sizeof(this->filename)-1, "%s%s-%06u.mwrm", path, basename, getpid())
            : snprintf(this->filename, sizeof(this->filename)-1, "%s%s.mwrm", path, basename);
            if (res > int(sizeof(this->filename) - 6) || res < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
        }
    };

    template<class Writer>
    void write_meta_headers(Writer & writer, const char * path,
                            uint16_t width, uint16_t height, auth_api * authentifier)
    {
        char header1[(std::numeric_limits<unsigned>::digits10 + 1) * 2 + 2];
        const int len = sprintf(header1, "%u %u", width, height);
        ssize_t res = writer.write(header1, len);
        if (res > 0) {
            res = writer.write("\n\n\n", 3);
        }

        if (res < 0) {
            int err = errno;
            if (err == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|%s", path);
                authentifier->report("FILESYSTEM_FULL", message);
            }

            LOG(LOG_ERR, "Write to transport failed (M): code=%d", err);
            throw Error(ERR_TRANSPORT_WRITE_FAILED, err);
        }
    }
}

#endif
