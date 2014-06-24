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
#include "no_param.hpp"
#include "error.hpp"
#include "log.hpp"
#include "auth_api.hpp"

#include <limits>
#include <cerrno>
#include <ctime>
#include <stdint.h>
#include <stdio.h> //snprintf
#include <stdlib.h> //mkostemps
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
        char header1[(std::numeric_limits<unsigned>::digits10 + 1) * 2 + 2 + 3];
        const int len = sprintf(header1, "%u %u\n\n\n", width, height);
        const ssize_t res = writer.write(header1, len);
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


    template<class BufWrmParams = no_param>
    struct out_sequence_filename_buf_param
    {
        FilenameGenerator::Format format;
        const char * const prefix;
        const char * const filename;
        const char * const extension;
        const int groupid;
        BufWrmParams wrm;

        out_sequence_filename_buf_param(
            FilenameGenerator::Format format,
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid,
            const BufWrmParams & wrm_params = BufWrmParams())
        : format(format)
        , prefix(prefix)
        , filename(filename)
        , extension(extension)
        , groupid(groupid)
        , wrm(wrm_params)
        {}
    };

    template<class BufWrm>
    class out_sequence_filename_buf
    {
        char current_filename_[1024];
        FilenameGenerator filegen_;
        BufWrm wrm_;
        unsigned num_file_;

    public:
        template<class BufWrmParams>
        out_sequence_filename_buf(out_sequence_filename_buf_param<BufWrmParams> const & params)
        : filegen_(params.format, params.prefix, params.filename, params.extension, params.groupid)
        , wrm_(params.wrm)
        , num_file_(0)
        {}

        ~out_sequence_filename_buf()
        {
            this->close();
        }

        int close() /*noexcept*/
        { return this->next(); }

        ssize_t write(const void * data, size_t len) /*noexcept*/
        {
            if (!this->wrm_.is_open()) {
                const int res = this->open_wrm(this->filegen_.get(this->num_file_));
                if (res < 0) {
                    return res;
                }
            }
            return this->wrm_.write(data, len);
        }

        int next() /*noexcept*/
        {
            if (this->wrm_.is_open()) {
                this->wrm_.close();
                // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
                return this->rename_wrm() ? 0 : -1;
            }
            return 1;
        }

        void request_full_cleaning()
        {
            unsigned i = this->num_file_ + 1;
            while (i > 0 && !::unlink(this->filegen_.get(--i))) {
            }
            if (this->wrm_.is_open()) {
                this->wrm_.close();
            }
        }

        //off_t seek(int64_t offset, int whence) /*noexcept*/
        //{ return this->wrm.seek(offset, whence); }

        const FilenameGenerator & seqgen() const /*noexcept*/
        { return this->filegen_; }

        BufWrm & wrm()
        { return this->wrm_; }

        const char * current_path() const
        {
            if (!this->current_filename_[0] && !this->num_file_) {
                return 0;
            }
            return this->filegen_.get(this->num_file_ - 1);
        }

    protected:
        ssize_t open_wrm(const char * filename) /*noexcept*/
        {
            snprintf(this->current_filename_, sizeof(this->current_filename_),
                        "%sred-XXXXXX.tmp", filename);
            const int fd = ::mkostemps(this->current_filename_, 4, O_WRONLY | O_CREAT);
            if (fd < 0) {
                return fd;
            }
            if (chmod(this->current_filename_, this->filegen_.groupid ? (S_IRUSR | S_IRGRP) : S_IRUSR) == -1) {
                LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                   , this->current_filename_, strerror(errno), errno
                   , this->filegen_.groupid ? "u+r, g+r" : "u+r");
            }
            this->filegen_.set_last_filename(this->num_file_, this->current_filename_);
            return this->wrm_.open(fd);
        }

        const char * rename_wrm()
        {
            const char * filename = this->get_filename_generate();
            const int res = ::rename(this->current_filename_, filename);
            if (res < 0) {
                LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
                   , this->current_filename_, filename, errno, strerror(errno));
                return 0;
            }

            this->current_filename_[0] = 0;
            ++this->num_file_;
            this->filegen_.set_last_filename(-1u, "");

            return filename;
        }

        const char * get_filename_generate()
        {
            this->filegen_.set_last_filename(-1u, "");
            const char * filename = this->filegen_.get(this->num_file_);
            this->filegen_.set_last_filename(this->num_file_, this->current_filename_);
            return filename;
        }
    };

    template<class T = no_param>
    struct out_meta_sequence_filename_buf_param
    {
        out_sequence_filename_buf_param<> sq_params;
        time_t sec;
        T mwrm_param;

        out_meta_sequence_filename_buf_param(
            time_t start_sec,
            FilenameGenerator::Format format,
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid,
            T const & mwrm_param = T())
        : sq_params(format, prefix, filename, extension, groupid)
        , sec(start_sec)
        , mwrm_param(mwrm_param)
        {}
    };


    template<class BufWrm, class BufMwrm>
    class out_meta_sequence_filename_buf
    : public out_sequence_filename_buf<BufWrm>
    {
        typedef out_sequence_filename_buf<BufWrm> sequence_base_type;

        BufMwrm mwrm_;

    protected:
        detail::MetaFilename mf_;
        time_t start_sec_;
        time_t stop_sec_;

    public:
        template<class T>
        out_meta_sequence_filename_buf(out_meta_sequence_filename_buf_param<T> const & params)
        : sequence_base_type(params.sq_params)
        , mwrm_(params.mwrm_param)
        , mf_(params.sq_params.prefix, params.sq_params.filename, params.sq_params.format)
        , start_sec_(params.sec)
        , stop_sec_(params.sec)
        {
            if (this->mwrm_.open(this->mf_.filename, S_IRUSR) < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
            }
        }

        ~out_meta_sequence_filename_buf()
        {
            this->close();
        }

        int close() /*noexcept*/
        {
            const int res1 = this->next();
            const int res2 = this->mwrm_.close();
            return res1 ? res1 : res2;
        }

        int next() /*noexcept*/
        {
            if (this->wrm().is_open()) {
                this->wrm().close();
                // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
                const char * filename = this->rename_wrm();
                if (!filename) {
                    return 1;
                }
                ssize_t len = strlen(filename);
                ssize_t res = this->mwrm_.write(filename, len);
                if (res == len) {
                    char mes[(std::numeric_limits<unsigned>::digits10 + 1) * 2 + 4];
                    len = sprintf(mes, " %u %u\n", (unsigned)this->start_sec_, (unsigned)this->stop_sec_+1);
                    res = this->mwrm_.write(mes, len);
                }
                if (res < len) {
                    return res < 0 ? res : 1;
                }
                this->start_sec_ = this->stop_sec_;
                return 0;
            }
            return 1;
        }

        void request_full_cleaning()
        {
            this->sequence_base_type::request_full_cleaning();
            ::unlink(this->mf_.filename);
        }

        BufMwrm & mwrm()
        { return this->mwrm_; }

        void update_sec(time_t sec) /*noexcept*/
        { this->stop_sec_ = sec; }
    };
}

#endif
