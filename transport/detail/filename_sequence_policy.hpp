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

#ifndef REDEMPTION_TRANSPORT_DETAIL_FILENAME_SEQUENCE_POLICY_HPP
#define REDEMPTION_TRANSPORT_DETAIL_FILENAME_SEQUENCE_POLICY_HPP

#include "log.hpp"
#include "sequence_generator.hpp"
#include "buffer/input_output_buf.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>

namespace detail
{
    struct FilenameSequencePolicyParams
    {
        FilenameGenerator::Format format;
        const char * const prefix;
        const char * const filename;
        const char * const extension;
        const int groupid;

        FilenameSequencePolicyParams(FilenameGenerator::Format format,
                                     const char * const prefix,
                                     const char * const filename,
                                     const char * const extension,
                                     const int groupid) /*noexcept*/
        : format(format)
        , prefix(prefix)
        , filename(filename)
        , extension(extension)
        , groupid(groupid)
        {}
    };

    class FilenameSequencePolicy
    : public transbuf::open_close_base
    {
        FilenameGenerator filegen;
        char current_filename[1024];
        unsigned num_file;

    public:
        FilenameSequencePolicy(FilenameSequencePolicyParams const & params)
        : filegen(params.format, params.prefix, params.filename, params.extension, params.groupid)
        , num_file(-1u)
        {
            this->current_filename[0] = 0;
        }

        template<class Buf>
        int init(Buf & file) /*noexcept*/
        {
            if (!file.is_open()) {
                std::snprintf(this->current_filename, sizeof(this->current_filename), "%sred-XXXXXX.tmp", this->filegen.path);
                const int fd = ::mkostemps(this->current_filename, 4, O_WRONLY | O_CREAT);
                if (fd < 0) {
                    return fd;
                }
                if (chmod( this->current_filename, (this->filegen.groupid ? (S_IRUSR | S_IRGRP) : S_IRUSR)) == -1) {
                    LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                       , this->current_filename, strerror(errno), errno
                       , (this->filegen.groupid ? "u+r, g+r" : "u+r"));
                }
                file.open(fd);
                ++this->num_file;
                this->filegen.set_last_filename(this->num_file, this->current_filename);
            }
            return 0;
        }

        template<class Buf>
        int close(Buf & file) /*noexcept*/
        {
            if (file.is_open()) {
                if (file.close() < 0) {
                    LOG(LOG_ERR, "closing file failed erro=%u : %s\n", errno, strerror(errno));
                }
                this->filegen.set_last_filename(0, 0);

                // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
                const char * filename = this->filegen.get(this->num_file);
                const int res = ::rename(this->current_filename, filename);
                if (res < 0) {
                    LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
                    , this->current_filename, filename, errno, strerror(errno));
                }

                this->current_filename[0] = 0;
            }
            return 0;
        }

        const FilenameGenerator & seqgen() const /*noexcept*/
        { return this->filegen; }

        unsigned seqnum() const /*noexcept*/
        { return this->num_file; }

        void request_full_cleaning() const
        {
            for (unsigned i = this->num_file+1; i > 0 && !::unlink(this->filegen.get(--i)); ) {
            }
        }
    };
}

#endif
