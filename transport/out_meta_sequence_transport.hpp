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

#ifndef REDEMPTION_PUBLIC_TRANSPORT_META_SEQUENCE_TRANSPORT_HPP
#define REDEMPTION_PUBLIC_TRANSPORT_META_SEQUENCE_TRANSPORT_HPP

#include "detail/filename_sequence_policy.hpp"
#include "buffer_transport.hpp"
#include "buffer/input_output_buf.hpp"
#include "buffer/file_buf.hpp"
#include "sequence_generator.hpp"

#include <limits>
#include <cstdio>
#include <cerrno>
#include <ctime>

namespace detail
{
    template<class Buf>
    struct out_meta_nexter
    : protected Buf
    {
        out_meta_nexter(time_t sec) /*noexcept*/
        : start_sec(sec)
        , stop_sec(sec)
        {}

        template<class Transport, class TransportBuf>
        int next(Transport & /*trans*/, TransportBuf & buf) /*noexcept*/
        {
            if (buf.is_open()) {
                buf.close();
                const char * filename = this->current_path(buf);
                size_t len = strlen(filename);
                ssize_t res = this->write(filename, len);
                if (res >= 0 && size_t(res) == len) {
                    char mes[(std::numeric_limits<unsigned>::digits10 + 1) * 2 + 5];
                    len = snprintf(mes, sizeof(mes), " %u %u\n",
                                (unsigned)this->start_sec,
                                (unsigned)this->stop_sec+1);
                    res = this->write(mes, len);
                    this->start_sec = this->stop_sec;
                }
                if (res < 0) {
                    int err = errno;
                    LOG(LOG_ERR, "Write to transport failed (M): code=%d", err);
                    return res;
                }
                return 0;
            }
            return 1;
        }

        template<class TransportBuf>
        bool next_end(TransportBuf & buf) /*noexcept*/
        {
            return this->next(buf, buf);
        }

        template<class TransportBuf>
        /*constexpr*/ const char * current_path(TransportBuf & buf) const /*noexcept*/
        {
            return buf.impl().seqgen().get(buf.impl().seqnum());
        }

        void update_sec(time_t sec) /*noexcept*/
        { this->stop_sec = sec; }

    private:
        time_t start_sec;
        time_t stop_sec;
    };

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


struct OutMetaSequenceTransport
: OutBufferTransport<
    transbuf::output_buf<io::posix::fdbuf, detail::FilenameSequencePolicy>,
    detail::out_meta_nexter<transbuf::ofile_base>
>
{
    OutMetaSequenceTransport(
        const char * path,
        const char * basename,
        timeval now,
        uint16_t width,
        uint16_t height,
        const int groupid,
        auth_api * authentifier = NULL,
        unsigned verbose = 0,
        FilenameFormat format = FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION)
    : OutMetaSequenceTransport::TransportType(
        detail::FilenameSequencePolicyParams(format, path, basename, ".wrm", groupid),
        now.tv_sec
    )
    {
        (void)verbose;

        if (this->OutMetaSequenceTransport::NexterType::open(
            detail::MetaFilename(path, basename, format).filename, S_IRUSR) < 0)
        {
            throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
        }

        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        detail::write_meta_headers(static_cast<transbuf::ofile_base&>(*this), path, width, height, this->authentifier);
    }

    virtual void timestamp(timeval now)
    {
        this->update_sec(now.tv_sec);
    }

    const FilenameGenerator * seqgen() const /*noexcept*/
    {
        return &this->impl().seqgen();
    }

    virtual void request_full_cleaning()
    {
        this->impl().request_full_cleaning();
    }
};

#endif
