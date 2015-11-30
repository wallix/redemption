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

#include "../buffer/file_buf.hpp"
#include "sequence_generator.hpp"
#include "no_param.hpp"
#include "error.hpp"
#include "log.hpp"
#include "auth_api.hpp"
#include "fileutils.hpp"
#include "iter.hpp"

#include "cryptofile.h" // MD_HASH_LENGTH

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
            int res =
            (   format == FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION
             || format == FilenameGenerator::PATH_FILE_PID_EXTENSION)
            ? snprintf(this->filename, sizeof(this->filename)-1, "%s%s-%06u.mwrm", path, basename, unsigned(getpid()))
            : snprintf(this->filename, sizeof(this->filename)-1, "%s%s.mwrm", path, basename);
            if (res > int(sizeof(this->filename) - 6) || res < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED);
            }
        }

        MetaFilename(MetaFilename const &) = delete;
        MetaFilename & operator = (MetaFilename const &) = delete;
    };

    template<class Writer>
    void write_meta_headers(Writer & writer, const char * path,
                            uint16_t width, uint16_t height,
                            auth_api * authentifier,
                            bool has_checksum
                           )
    {
        char header1[3 + ((std::numeric_limits<unsigned>::digits10 + 1) * 2 + 2) + (10 + 1) + 2 + 1];
        const int len = sprintf(
            header1,
            "v2\n"
            "%u %u\n"
            "%s\n"
            "\n\n",
            unsigned(width),
            unsigned(height),
            has_checksum  ? "checksum" : "nochecksum"
        );
        const ssize_t res = writer.write(header1, len);
        if (res < 0) {
            int err = errno;
            LOG(LOG_ERR, "Write to transport failed (M): code=%d", err);

            if (err == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|%s", path);
                authentifier->report("FILESYSTEM_FULL", message);

                throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, err);
            }
            else {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, err);
            }
        }
    }

    using hash_type = unsigned char[MD_HASH_LENGTH*2];

    template<bool write_time, class Writer>
    int write_meta_file_impl(
        Writer & writer, const char * filename,
        time_t start_sec, time_t stop_sec,
        hash_type const * hash = nullptr
    ) {
        auto pfile = filename;
        auto epfile = filename;
        for (; *epfile; ++epfile) {
            if (*epfile == '\\') {
                bool const is_backslash = (*(epfile+1) == '\\');
                ssize_t len = epfile - pfile + is_backslash;
                auto res = writer.write(pfile, len);
                if (res < len) {
                    return res < 0 ? res : 1;
                }
                pfile = epfile + 1 + is_backslash;
            }
        }

        if (pfile != epfile) {
            ssize_t len = epfile - pfile;
            auto res = writer.write(pfile, len);
            if (res < len) {
                return res < 0 ? res : 1;
            }
        }

        using ull = unsigned long long;
        using ll = long long;
        char mes[
            (std::numeric_limits<ll>::digits10 + 1 + 1 + 1) * 8 +
            (std::numeric_limits<ull>::digits10 + 1 + 1) * 2 +
            (1 + sizeof(hash_type)) * 2 + 1 +
            2
        ];
        struct stat stat;
        if (0 != ::stat(filename, &stat)) {
            return 1;
        }
        ssize_t len = sprintf(
            mes,
            " %lld %llu %lld %lld %llu %lld %lld %lld",
            ll(stat.st_size),
            ull(stat.st_mode),
            ll(stat.st_uid),
            ll(stat.st_gid),
            ull(stat.st_dev),
            ll(stat.st_ino),
            ll(stat.st_mtim.tv_sec),
            ll(stat.st_ctim.tv_sec)
        );
        if (write_time) {
            len += sprintf(
                mes + len,
                " %lld %lld",
                ll(start_sec),
                ll(stop_sec)
            );
        }

        char * p = mes + len;
        if (hash) {
            auto write = [&p](unsigned char const * hash){
                *p++ = ' ';                // 1 octet
                for (unsigned c : iter(hash, MD_HASH_LENGTH)) {
                    sprintf(p, "%02x", c); // 64 octets (hash)
                    p += 2;
                }
            };
            write(*hash);
            write(*hash + MD_HASH_LENGTH);
        }
        *p++ = '\n';

        ssize_t res = writer.write(mes, p-mes);

        if (res < p-mes) {
            return res < 0 ? res : 1;
        }

        return 0;
    }

    template<class Writer>
    int write_meta_file(
        Writer & writer, const char * filename,
        time_t start_sec, time_t stop_sec,
        hash_type const * hash = nullptr
    ) {
        return write_meta_file_impl<true>(writer, filename, start_sec, stop_sec, hash);
    }


    template<class BufParams = no_param>
    struct out_sequence_filename_buf_param
    {
        FilenameGenerator::Format format;
        const char * const prefix;
        const char * const filename;
        const char * const extension;
        const int groupid;
        BufParams buf_params;

        out_sequence_filename_buf_param(
            FilenameGenerator::Format format,
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid,
            const BufParams & buf_params = BufParams())
        : format(format)
        , prefix(prefix)
        , filename(filename)
        , extension(extension)
        , groupid(groupid)
        , buf_params(buf_params)
        {}
    };

    template<class Buf>
    class out_sequence_filename_buf_impl
    {
        char current_filename_[1024];
        FilenameGenerator filegen_;
        Buf buf_;
        unsigned num_file_;

    public:
        template<class BufParams>
        explicit out_sequence_filename_buf_impl(out_sequence_filename_buf_param<BufParams> const & params)
        : filegen_(params.format, params.prefix, params.filename, params.extension, params.groupid)
        , buf_(params.buf_params)
        , num_file_(0)
        {
            this->current_filename_[0] = 0;
        }

        int close()
        { return this->next(); }

        ssize_t write(const void * data, size_t len)
        {
            if (!this->buf_.is_open()) {
                const int res = this->open_filename(this->filegen_.get(this->num_file_));
                if (res < 0) {
                    return res;
                }
            }
            return this->buf_.write(data, len);
        }

        /// \return 0 if success
        int next()
        {
            if (this->buf_.is_open()) {
                this->buf_.close();
                // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
                return this->rename_filename() ? 0 : 1;
            }
            return 1;
        }

        void request_full_cleaning()
        {
            unsigned i = this->num_file_ + 1;
            while (i > 0 && !::unlink(this->filegen_.get(--i))) {
            }
            if (this->buf_.is_open()) {
                this->buf_.close();
            }
        }

        int flush()
        {
            if (this->buf_.is_open()) {
                return this->buf_.flush();
            }
            return 0;
        }

        off64_t seek(int64_t offset, int whence)
        { return this->buf_.seek(offset, whence); }

        const FilenameGenerator & seqgen() const noexcept
        { return this->filegen_; }

        Buf & buf() noexcept
        { return this->buf_; }

        const char * current_path() const
        {
            if (!this->current_filename_[0] && !this->num_file_) {
                return nullptr;
            }
            return this->filegen_.get(this->num_file_ - 1);
        }

    protected:
        ssize_t open_filename(const char * filename)
        {
            snprintf(this->current_filename_, sizeof(this->current_filename_),
                        "%sred-XXXXXX.tmp", filename);
            const int fd = ::mkostemps(this->current_filename_, 4, O_WRONLY | O_CREAT);
            if (fd < 0) {
                return fd;
            }
            if (chmod(this->current_filename_, this->filegen_.groupid ? (S_IRUSR | S_IRGRP) : S_IRUSR) == -1) {
                LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                   , this->current_filename_
                   , this->filegen_.groupid ? "u+r, g+r" : "u+r"
                   , strerror(errno), errno);
            }
            this->filegen_.set_last_filename(this->num_file_, this->current_filename_);
            return this->buf_.open(fd);
        }

        const char * rename_filename()
        {
            const char * filename = this->get_filename_generate();
            const int res = ::rename(this->current_filename_, filename);
            if (res < 0) {
                LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
                   , this->current_filename_, filename, errno, strerror(errno));
                return nullptr;
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

    template<class Buf>
    struct CloseWrapper final : Buf {
        using Buf::Buf;
        ~CloseWrapper() {
            this->close();
        }
    };

    template<class HashBuf>
    int write_meta_hash(
        char const * hash_filename, char const * meta_filename,
        HashBuf & crypto_hash, hash_type const * hash, uint32_t verbose
    ) {
        char path[1024] = {};
        char basename[1024] = {};
        char extension[256] = {};
        char filename[2048] = {};

        canonical_path( meta_filename
                      , path, sizeof(path)
                      , basename, sizeof(basename)
                      , extension, sizeof(extension)
                      , verbose);
        snprintf(filename, sizeof(filename), "%s%s", basename, extension);

        if (crypto_hash.open(hash_filename, S_IRUSR|S_IRGRP) >= 0) {
            char header[] = "v2\n\n\n";
            crypto_hash.write(header, sizeof(header)-1);
            int err = write_meta_file_impl<false>(crypto_hash, filename, 0, 0, hash);
            if (!err) {
                err = crypto_hash.close(/*hash*/);
            }
            if (err) {
                LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                return 1;
            }
        }
        else {
            int e = errno;
            LOG(LOG_ERR, "Open to transport failed: code=%d", e);
            errno = e;
            return 1;
        }

        return 0;
    }

    template<class Buf>
    using out_sequence_filename_buf = CloseWrapper<out_sequence_filename_buf_impl<Buf>>;

    template<class MetaParams = no_param, class BufParams = no_param>
    struct out_meta_sequence_filename_buf_param
    {
        out_sequence_filename_buf_param<BufParams> sq_params;
        time_t sec;
        MetaParams meta_buf_params;
        const char * hash_prefix;
        uint32_t verbose;

        out_meta_sequence_filename_buf_param(
            time_t start_sec,
            FilenameGenerator::Format format,
            const char * const hash_prefix,
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid,
            uint32_t verbose = 0,
            MetaParams const & meta_buf_params = MetaParams(),
            BufParams const & buf_params = BufParams())
        : sq_params(format, prefix, filename, extension, groupid, buf_params)
        , sec(start_sec)
        , meta_buf_params(meta_buf_params)
        , hash_prefix(hash_prefix)
        , verbose(verbose)
        {}
    };


    template<class Buf, class BufMeta>
    class out_meta_sequence_filename_buf_impl
    : public out_sequence_filename_buf_impl<Buf>
    {
        typedef out_sequence_filename_buf_impl<Buf> sequence_base_type;

        BufMeta meta_buf_;
        MetaFilename mf_;
        MetaFilename hf_;
        time_t start_sec_;
        time_t stop_sec_;

    protected:
        uint32_t verbose;

    public:
        template<class MetaParams, class BufParams>
        explicit out_meta_sequence_filename_buf_impl(
            out_meta_sequence_filename_buf_param<MetaParams, BufParams> const & params
        )
        : sequence_base_type(params.sq_params)
        , meta_buf_(params.meta_buf_params)
        , mf_(params.sq_params.prefix, params.sq_params.filename, params.sq_params.format)
        , hf_(params.hash_prefix, params.sq_params.filename, params.sq_params.format)
        , start_sec_(params.sec)
        , stop_sec_(params.sec)
        {
            if (this->meta_buf_.open(this->mf_.filename, S_IRUSR | S_IRGRP) < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
            }
        }

        int close()
        {
            const int res1 = this->next();
            const int res2 = (this->meta_buf().is_open() ? this->meta_buf_.close() : 0);
            int err = res1 ? res1 : res2;
            if (!err) {
                transbuf::ofile_buf ofile;
                return write_meta_hash(
                    this->hash_filename(), this->meta_filename(),
                    ofile, nullptr, this->verbose
                );
            }
            return err;
        }

        /// \return 0 if success
        int next()
        {
            if (this->buf().is_open()) {
                this->buf().close();
                return this->next_meta_file();
            }
            return 1;
        }

    protected:
        int next_meta_file(hash_type const * hash = nullptr) {
            // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
            const char * filename = this->rename_filename();
            if (!filename) {
                return 1;
            }

            if (int err = write_meta_file(
                this->meta_buf_, filename, this->start_sec_, this->stop_sec_+1, hash
            )) {
                return err;
            }

            this->start_sec_ = this->stop_sec_;

            return 0;
        }

        char const * hash_filename() const noexcept {
            return this->hf_.filename;
        }

        char const * meta_filename() const noexcept {
            return this->mf_.filename;
        }

    public:
        void request_full_cleaning()
        {
            this->sequence_base_type::request_full_cleaning();
            ::unlink(this->mf_.filename);
        }

        int flush()
        {
            const int res1 = this->sequence_base_type::flush();
            const int res2 = this->meta_buf_.flush();
            return res1 == 0 ? res2 : res1;
        }

        BufMeta & meta_buf() noexcept
        { return this->meta_buf_; }

        void update_sec(time_t sec)
        { this->stop_sec_ = sec; }
    };

    template<class Buf, class BufMeta>
    using out_meta_sequence_filename_buf = CloseWrapper<out_meta_sequence_filename_buf_impl<Buf, BufMeta>>;
}

#endif
