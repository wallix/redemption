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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
              Martin Potier, Jonatan Poelen, Raphael Zhou, Meng Tan
*/

#pragma once

#include <vector>
#include <functional>
#include <cerrno>
#include <fcntl.h>
#include <snappy-c.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h> // localtime_r
#include <memory>
#include <ctime>
#include <cassert>
#include <chrono>
#include <utility>
#include <sys/time.h>
#include <cstdio>
#include <string>
#include <chrono>
#include <algorithm>
#include <cerrno>
#include <limits>
#include <ctime>
#include <stdio.h> //snprintf
#include <stdlib.h> //mkostemps
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstddef>

#include "core/error.hpp"

#include "utils/log.hpp"

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/local_fd.hpp"
#include "utils/sugar/range.hpp"
#include "utils/sugar/bytes_t.hpp"
#include "utils/sugar/noncopyable.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/sugar/make_unique.hpp"
#include "utils/sugar/iter.hpp"
#include "utils/sugar/exchange.hpp"

#include "utils/match_finder.hpp"
#include "utils/difftimeval.hpp"
#include "utils/drawable.hpp"
#include "utils/apps/recording_progress.hpp"
#include "utils/dump_png24_from_rdp_drawable_adapter.hpp"
#include "utils/genrandom.hpp"
#include "utils/urandom_read.hpp"
#include "utils/fileutils.hpp"
#include "utils/bitmap_shrink.hpp"
#include "utils/pattutils.hpp"
#include "utils/colors.hpp"
#include "utils/compression_transport_builder.hpp"
#include "utils/stream.hpp"
#include "utils/verbose_flags.hpp"
#include "utils/png.hpp"
#include "utils/fdbuf.hpp"

#include "cxx/attributes.hpp"

#include "transport/transport.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/sequence_generator.hpp"

#include "core/RDP/RDPDrawable.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "core/RDP/caches/bmpcache.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/caches/pointercache.hpp"
#include "core/RDP/share.hpp"
#include "core/RDP/RDPSerializer.hpp"

#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"

#include "core/wait_obj.hpp"

#include "configs/config.hpp"

#include "gdi/capture_probe_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/graphic_cmd_color.hpp"
#include "gdi/kbd_input_api.hpp"
#include "gdi/dump_png24.hpp"

#include "acl/auth_api.hpp"

#include "capture/utils/match_finder.hpp"
#include "capture/title_extractors/agent_title_extractor.hpp"
#include "capture/title_extractors/ocr_title_filter.hpp"
#include "capture/title_extractors/ocr_titles_extractor.hpp"
#include "capture/title_extractors/ppocr_titles_extractor.hpp"
#include "capture/title_extractors/ocr_title_extractor_builder.hpp"

#include "capture/wrm_params.hpp"
#include "capture/png_params.hpp"
#include "capture/flv_params.hpp"
#include "capture/ocr_params.hpp"
#include "capture/meta_params.hpp"
#include "capture/sequenced_video_params.hpp"
#include "capture/full_video_params.hpp"
#include "capture/pattern_checker_params.hpp"
#include "capture/kbdlog_params.hpp"

#include "capture/wrm_label.hpp"
#include "capture/cryptofile.hpp"
#include "capture/video_recorder.hpp"

#include "capture/RDPChunkedDevice.hpp"

#include "openssl_crypto.hpp"

struct FilenameGenerator
{
    enum Format {
        PATH_FILE_PID_COUNT_EXTENSION,
        PATH_FILE_COUNT_EXTENSION,
        PATH_FILE_PID_EXTENSION,
        PATH_FILE_EXTENSION
    };

private:
    char         path[1024];
    char         filename[1012];
    char         extension[12];
    Format       format;
    unsigned     pid;
    mutable char filename_gen[1024];

    const char * last_filename;
    unsigned     last_num;

public:
    FilenameGenerator(
        Format format,
        const char * const prefix,
        const char * const filename,
        const char * const extension)
    : format(format)
    , pid(getpid())
    , last_filename(nullptr)
    , last_num(-1u)
    {
        if (strlen(prefix) > sizeof(this->path) - 1
         || strlen(filename) > sizeof(this->filename) - 1
         || strlen(extension) > sizeof(this->extension) - 1) {
            throw Error(ERR_TRANSPORT);
        }

        strcpy(this->path, prefix);
        strcpy(this->filename, filename);
        strcpy(this->extension, extension);

        this->filename_gen[0] = 0;
    }

    const char * get(unsigned count) const
    {
        if (count == this->last_num && this->last_filename) {
            return this->last_filename;
        }

        using std::snprintf;
        switch (this->format) {
            default:
            case PATH_FILE_PID_COUNT_EXTENSION:
                snprintf( this->filename_gen, sizeof(this->filename_gen), "%s%s-%06u-%06u%s", this->path
                        , this->filename, this->pid, count, this->extension);
                break;
            case PATH_FILE_COUNT_EXTENSION:
                snprintf( this->filename_gen, sizeof(this->filename_gen), "%s%s-%06u%s", this->path
                        , this->filename, count, this->extension);
                break;
            case PATH_FILE_PID_EXTENSION:
                snprintf( this->filename_gen, sizeof(this->filename_gen), "%s%s-%06u%s", this->path
                        , this->filename, this->pid, this->extension);
                break;
            case PATH_FILE_EXTENSION:
                snprintf( this->filename_gen, sizeof(this->filename_gen), "%s%s%s", this->path
                        , this->filename, this->extension);
                break;
        }
        return this->filename_gen;
    }

    void set_last_filename(unsigned num, const char * name)
    {
        this->last_num = num;
        this->last_filename = name;
    }

private:
    FilenameGenerator(FilenameGenerator const &) = delete;
    FilenameGenerator& operator=(FilenameGenerator const &) = delete;
};

typedef FilenameGenerator::Format FilenameFormat;

namespace transbuf {
    class ofile_buf_out
    {
        int fd;
    public:
        ofile_buf_out() : fd(-1) {}
        ~ofile_buf_out()
        {
            this->close();
        }

        int open(const char * filename, mode_t mode)
        {
            this->close();
            this->fd = ::open(filename, O_WRONLY | O_CREAT, mode);
            return this->fd;
        }

        int close()
        {
            if (this->is_open()) {
                const int ret = ::close(this->fd);
                this->fd = -1;
                return ret;
            }
            return 0;
        }

        ssize_t write(const void * data, size_t len)
        {
            size_t remaining_len = len;
            size_t total_sent = 0;
            while (remaining_len) {
                ssize_t ret = ::write(this->fd,
                    static_cast<const char*>(data) + total_sent, remaining_len);
                if (ret <= 0){
                    if (errno == EINTR){
                        continue;
                    }
                    return -1;
                }
                remaining_len -= ret;
                total_sent += ret;
            }
            return total_sent;
        }

        bool is_open() const noexcept
        { return -1 != this->fd; }

        off64_t seek(off64_t offset, int whence) const
        { return ::lseek64(this->fd, offset, whence); }

        int flush() const
        { return 0; }
    };

}


namespace transbuf {

using std::size_t;


class ochecksum_buf_null_buf
{
    static constexpr size_t nosize = ~size_t{};
    static constexpr size_t quick_size = 4096;

    SslHMAC_Sha256_Delayed hmac;
    SslHMAC_Sha256_Delayed quick_hmac;
    unsigned char const (&hmac_key)[MD_HASH_LENGTH];
    size_t file_size = nosize;

public:
    explicit ochecksum_buf_null_buf(unsigned char const (&hmac_key)[MD_HASH_LENGTH])
    : hmac_key(hmac_key)
    {}

    ochecksum_buf_null_buf(ochecksum_buf_null_buf const &) = delete;
    ochecksum_buf_null_buf & operator=(ochecksum_buf_null_buf const &) = delete;

    template<class... Ts>
    int open(Ts && ... args)
    {
        this->hmac.init(this->hmac_key, sizeof(this->hmac_key));
        this->quick_hmac.init(this->hmac_key, sizeof(this->hmac_key));
        int ret = 0;
        this->file_size = 0;
        return ret;
    }

    ssize_t write(const void * data, size_t len)
    {
        REDASSERT(this->file_size != nosize);
        this->hmac.update(static_cast<const uint8_t *>(data), len);
        if (this->file_size < quick_size) {
            auto const remaining = std::min(quick_size - this->file_size, len);
            this->quick_hmac.update(static_cast<const uint8_t *>(data), remaining);
            this->file_size += remaining;
        }
        return len;
    }

    int close(unsigned char (&hash)[MD_HASH_LENGTH * 2])
    {
        REDASSERT(this->file_size != nosize);
        this->quick_hmac.final(reinterpret_cast<unsigned char(&)[MD_HASH_LENGTH]>(hash[0]));
        this->hmac.final(reinterpret_cast<unsigned char(&)[MD_HASH_LENGTH]>(hash[MD_HASH_LENGTH]));
        this->file_size = nosize;
        return 0;
    }

    int close() {
        return 0;
    }
};

}


namespace detail
{

    struct no_param {};

    template<class Buf>
    struct empty_ctor
    : Buf
    {
        explicit empty_ctor(no_param = no_param()) noexcept
        {}
    };

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

    template<class Writer>
    int write_filename(Writer & writer, const char * filename)
    {
        auto pfile = filename;
        auto epfile = filename;
        for (; *epfile; ++epfile) {
            if (*epfile == '\\') {
                ssize_t len = epfile - pfile + 1;
                auto res = writer.write(pfile, len);
                if (res < len) {
                    return res < 0 ? res : 1;
                }
                pfile = epfile;
            }
            if (*epfile == ' ') {
                ssize_t len = epfile - pfile;
                auto res = writer.write(pfile, len);
                if (res < len) {
                    return res < 0 ? res : 1;
                }
                res = writer.write("\\", 1u);
                if (res < 1) {
                    return res < 0 ? res : 1;
                }
                pfile = epfile;
            }
        }

        if (pfile != epfile) {
            ssize_t len = epfile - pfile;
            auto res = writer.write(pfile, len);
            if (res < len) {
                return res < 0 ? res : 1;
            }
        }

        return 0;
    }

    using hash_type = unsigned char[MD_HASH_LENGTH*2];

    constexpr std::size_t hash_string_len = (1 + MD_HASH_LENGTH * 2) * 2;

    inline char * swrite_hash(char * p, hash_type const & hash)
    {
        auto write = [&p](unsigned char const * hash) {
            *p++ = ' ';                // 1 octet
            for (unsigned c : iter(hash, MD_HASH_LENGTH)) {
                sprintf(p, "%02x", c); // 64 octets (hash)
                p += 2;
            }
        };
        write(hash);
        write(hash + MD_HASH_LENGTH);
        return p;
    }

    template<bool write_time, class Writer>
    int write_meta_file_impl(
        Writer & writer, const char * filename,
        struct stat const & stat,
        time_t start_sec, time_t stop_sec,
        hash_type const * hash = nullptr
    ) {
        if (int err = write_filename(writer, filename)) {
            return err;
        }

        using ull = unsigned long long;
        using ll = long long;
        char mes[
            (std::numeric_limits<ll>::digits10 + 1 + 1) * 8 +
            (std::numeric_limits<ull>::digits10 + 1 + 1) * 2 +
            hash_string_len + 1 +
            2
        ];
        ssize_t len = std::sprintf(
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
            len += std::sprintf(
                mes + len,
                " %lld %lld",
                ll(start_sec),
                ll(stop_sec)
            );
        }

        char * p = mes + len;
        if (hash) {
            p = swrite_hash(p, *hash);
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
        struct stat stat;
        int err = ::stat(filename, &stat);
        return err ? err : write_meta_file_impl<true>(writer, filename, stat, start_sec, stop_sec, hash);
    }


    struct out_sequence_filename_buf_param
    {
        FilenameGenerator::Format format;
        const char * const prefix;
        const char * const filename;
        const char * const extension;
        const int groupid;

        out_sequence_filename_buf_param(
            FilenameGenerator::Format format,
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid)
        : format(format)
        , prefix(prefix)
        , filename(filename)
        , extension(extension)
        , groupid(groupid)
        {}
    };

    class out_sequence_filename_buf_impl
    {
        char current_filename_[1024];
        FilenameGenerator filegen_;
        detail::empty_ctor<io::posix::fdbuf> buf_;
        unsigned num_file_;
        int groupid_;

    public:
        explicit out_sequence_filename_buf_impl(out_sequence_filename_buf_param const & params)
        : filegen_(params.format, params.prefix, params.filename, params.extension)
        , buf_()
        , num_file_(0)
        , groupid_(params.groupid)
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

        off64_t seek(int64_t offset, int whence)
        { return this->buf_.seek(offset, whence); }

        const FilenameGenerator & seqgen() const noexcept
        { return this->filegen_; }

        detail::empty_ctor<io::posix::fdbuf> & buf() noexcept
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
            if (chmod(this->current_filename_, this->groupid_ ? (S_IRUSR | S_IRGRP) : S_IRUSR) == -1) {
                LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                   , this->current_filename_
                   , this->groupid_ ? "u+r, g+r" : "u+r"
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


    template<class MetaParams = no_param>
    struct out_meta_sequence_filename_buf_param
    {
        out_sequence_filename_buf_param sq_params;
        time_t sec;
        MetaParams meta_buf_params;
        const char * hash_prefix;

        out_meta_sequence_filename_buf_param(
            time_t start_sec,
            FilenameGenerator::Format format,
            const char * const hash_prefix,
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid,
            MetaParams const & meta_buf_params = MetaParams())
        : sq_params(format, prefix, filename, extension, groupid)
        , sec(start_sec)
        , meta_buf_params(meta_buf_params)
        , hash_prefix(hash_prefix)
        {}
    };


    template<class BufMeta>
    class out_meta_sequence_filename_buf_impl
    : public out_sequence_filename_buf_impl
    {
        typedef out_sequence_filename_buf_impl sequence_base_type;

        BufMeta meta_buf_;
        MetaFilename mf_;
        MetaFilename hf_;
        time_t start_sec_;
        time_t stop_sec_;

    public:
        template<class MetaParams>
        explicit out_meta_sequence_filename_buf_impl(
            out_meta_sequence_filename_buf_param<MetaParams> const & params
        )
        : out_sequence_filename_buf_impl(params.sq_params)
        , meta_buf_(params.meta_buf_params)
        , mf_(params.sq_params.prefix, params.sq_params.filename, params.sq_params.format)
        , hf_(params.hash_prefix, params.sq_params.filename, params.sq_params.format)
        , start_sec_(params.sec)
        , stop_sec_(params.sec)
        {
            if (this->meta_buf_.open(this->mf_.filename, S_IRUSR | S_IRGRP | S_IWUSR) < 0) {
                LOG(LOG_ERR, "Failed to open meta file %s", this->mf_.filename);
                throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
            }
            if (chmod(this->mf_.filename, S_IRUSR | S_IRGRP) == -1) {
                LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                   , this->mf_.filename
                   , "u+r, g+r"
                   , strerror(errno), errno);
            }
        }

        int close()
        {
            const int res1 = this->next();
            const int res2 = (this->meta_buf().is_open() ? this->meta_buf_.close() : 0);
            int err = res1 ? res1 : res2;
            if (!err) {
                char const * hash_filename = this->hash_filename();
                char const * meta_filename = this->meta_filename();
                transbuf::ofile_buf_out crypto_hash;

                char path[1024] = {};
                char basename[1024] = {};
                char extension[256] = {};
                char filename[2048] = {};

                canonical_path(
                    meta_filename,
                    path, sizeof(path),
                    basename, sizeof(basename),
                    extension, sizeof(extension)
                );

                snprintf(filename, sizeof(filename), "%s%s", basename, extension);

                if (crypto_hash.open(hash_filename, S_IRUSR|S_IRGRP) >= 0) {
                    char header[] = "v2\n\n\n";
                    crypto_hash.write(header, sizeof(header)-1);

                    struct stat stat;
                    int err = ::stat(meta_filename, &stat);
                    if (!err) {
                        err = write_meta_file_impl<false>(crypto_hash, filename, stat, 0, 0, nullptr);
                    }
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
        int next_meta_file(hash_type const * hash = nullptr)
        {
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

        char const * hash_filename() const noexcept
        {
            return this->hf_.filename;
        }

        char const * meta_filename() const noexcept
        {
            return this->mf_.filename;
        }

    public:
        void request_full_cleaning()
        {
            this->out_sequence_filename_buf_impl::request_full_cleaning();
            ::unlink(this->mf_.filename);
        }

        int flush()
        {
            const int res1 = this->out_sequence_filename_buf_impl::flush();
            const int res2 = this->meta_buf_.flush();
            return res1 == 0 ? res2 : res1;
        }

        BufMeta & meta_buf() noexcept
        { return this->meta_buf_; }

        void update_sec(time_t sec)
        { this->stop_sec_ = sec; }
    };
}

struct OutFilenameSequenceTransport : public Transport
{
    using Buf = detail::out_sequence_filename_buf_impl;

    OutFilenameSequenceTransport(
        FilenameGenerator::Format format,
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid,
        auth_api * authentifier)
    : buf(detail::out_sequence_filename_buf_param(format, prefix, filename, extension, groupid))
    {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }
    }

    const FilenameGenerator * seqgen() const noexcept
    { return &(this->buffer().seqgen()); }

    bool next() override {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        const ssize_t res = this->buffer().next();
        if (res) {
            this->status = false;
            if (res < 0){
                LOG(LOG_ERR, "Write to transport failed (M): code=%d", errno);
                throw Error(ERR_TRANSPORT_WRITE_FAILED, -res);
            }
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
        ++this->seqno;
        return true;
    }

    bool disconnect() override {
        return !this->buf.close();
    }

    void request_full_cleaning() override {
        this->buffer().request_full_cleaning();
    }

    ~OutFilenameSequenceTransport() {
        this->buf.close();
    }

private:

    void do_send(const uint8_t * data, size_t len) override {
        const ssize_t res = this->buf.write(data, len);
        if (res < 0) {
            this->status = false;
            if (errno == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|unknown");
                this->authentifier->report("FILESYSTEM_FULL", message);
                errno = ENOSPC;
                throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, ENOSPC);
            }
            else {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        this->last_quantum_sent += res;
    }

    Buf & buffer() noexcept
    { return this->buf; }

    const Buf & buffer() const noexcept
    { return this->buf; }

    Buf buf;
};

struct OutFilenameSequenceSeekableTransport : public Transport
{
    using Buf = detail::out_sequence_filename_buf_impl;

    OutFilenameSequenceSeekableTransport(
        FilenameGenerator::Format format,
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid,
        auth_api * authentifier = nullptr)
    : buf(detail::out_sequence_filename_buf_param(format, prefix, filename, extension, groupid))
    {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }
    }

    const FilenameGenerator * seqgen() const noexcept
    { return &(this->buffer().seqgen()); }

    void seek(int64_t offset, int whence) override {
        if (static_cast<off64_t>(-1) == this->buffer().seek(offset, whence)){
            throw Error(ERR_TRANSPORT_SEEK_FAILED, errno);
        }
    }

    bool next() override {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        const ssize_t res = this->buffer().next();
        if (res) {
            this->status = false;
            if (res < 0){
                LOG(LOG_ERR, "Write to transport failed (M): code=%d", errno);
                throw Error(ERR_TRANSPORT_WRITE_FAILED, -res);
            }
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
        ++this->seqno;
        return true;
    }

    bool disconnect() override {
        return !this->buf.close();
    }

    void request_full_cleaning() override {
        this->buffer().request_full_cleaning();
    }

    ~OutFilenameSequenceSeekableTransport() {
        this->buf.close();
    }

private:
    void do_send(const uint8_t * data, size_t len) override {
        const ssize_t res = this->buf.write(data, len);
        if (res < 0) {
            this->status = false;
            if (errno == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|unknown");
                this->authentifier->report("FILESYSTEM_FULL", message);
                errno = ENOSPC;
                throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, ENOSPC);
            }
            else {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        this->last_quantum_sent += res;
    }

    Buf & buffer() noexcept
    { return this->buf; }

    const Buf & buffer() const noexcept
    { return this->buf; }

private:
    Buf buf;

};

namespace transfil {

    class encrypt_filter
    {
        char           buf[CRYPTO_BUFFER_SIZE]; //
        EVP_CIPHER_CTX ectx;                    // [en|de]cryption context
        EVP_MD_CTX     hctx;                    // hash context
        EVP_MD_CTX     hctx4k;                  // hash context
        uint32_t       pos;                     // current position in buf
        uint32_t       raw_size;                // the unciphered/uncompressed file size
        uint32_t       file_size;               // the current file size

    public:
        encrypt_filter() = default;
        //: pos(0)
        //, raw_size(0)
        //, file_size(0)
        //{}

        template<class Sink>
        int open(Sink & snk, const unsigned char * trace_key, CryptoContext & cctx, const unsigned char * iv)
        {
            ::memset(this->buf, 0, sizeof(this->buf));
            ::memset(&this->ectx, 0, sizeof(this->ectx));
            ::memset(&this->hctx, 0, sizeof(this->hctx));
            ::memset(&this->hctx4k, 0, sizeof(this->hctx4k));
            this->pos = 0;
            this->raw_size = 0;
            this->file_size = 0;

            const EVP_CIPHER * cipher  = ::EVP_aes_256_cbc();
            const unsigned int salt[]  = { 12345, 54321 };    // suspicious, to check...
            const int          nrounds = 5;
            unsigned char      key[32];
            const int i = ::EVP_BytesToKey(cipher, ::EVP_sha1(), reinterpret_cast<const unsigned char *>(salt),
                                           trace_key, CRYPTO_KEY_LENGTH, nrounds, key, nullptr);
            if (i != 32) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: EVP_BytesToKey size is wrong\n", ::getpid());
                return -1;
            }

            ::EVP_CIPHER_CTX_init(&this->ectx);
            if (::EVP_EncryptInit_ex(&this->ectx, cipher, nullptr, key, iv) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize encrypt context\n", ::getpid());
                return -1;
            }

            // MD stuff
            const EVP_MD * md = EVP_get_digestbyname(MD_HASH_NAME);
            if (!md) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not find message digest algorithm!\n", ::getpid());
                return -1;
            }

            ::EVP_MD_CTX_init(&this->hctx);
            ::EVP_MD_CTX_init(&this->hctx4k);
            if (::EVP_DigestInit_ex(&this->hctx, md, nullptr) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize MD hash context!\n", ::getpid());
                return -1;
            }
            if (::EVP_DigestInit_ex(&this->hctx4k, md, nullptr) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize 4k MD hash context!\n", ::getpid());
                return -1;
            }

            // HMAC: key^ipad
            const int     blocksize = ::EVP_MD_block_size(md);
            unsigned char * key_buf = new(std::nothrow) unsigned char[blocksize];
            {
                if (key_buf == nullptr) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: malloc!\n", ::getpid());
                    return -1;
                }
                const std::unique_ptr<unsigned char[]> auto_free(key_buf);
                ::memset(key_buf, 0, blocksize);
                if (CRYPTO_KEY_LENGTH > blocksize) { // keys longer than blocksize are shortened
                    unsigned char keyhash[MD_HASH_LENGTH];
                    if ( ! ::MD_HASH_FUNC(cctx.get_hmac_key(), CRYPTO_KEY_LENGTH, keyhash)) {
                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not hash crypto key!\n", ::getpid());
                        return -1;
                    }
                    ::memcpy(key_buf, keyhash, MIN(MD_HASH_LENGTH, blocksize));
                }
                else {
                    ::memcpy(key_buf, cctx.get_hmac_key(), CRYPTO_KEY_LENGTH);
                }
                for (int idx = 0; idx <  blocksize; idx++) {
                    key_buf[idx] = key_buf[idx] ^ 0x36;
                }
                if (::EVP_DigestUpdate(&this->hctx, key_buf, blocksize) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash!\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&this->hctx4k, key_buf, blocksize) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update 4k hash!\n", ::getpid());
                    return -1;
                }
            }

            // update context with previously written data
            char tmp_buf[40];
            tmp_buf[0] = WABCRYPTOFILE_MAGIC & 0xFF;
            tmp_buf[1] = (WABCRYPTOFILE_MAGIC >> 8) & 0xFF;
            tmp_buf[2] = (WABCRYPTOFILE_MAGIC >> 16) & 0xFF;
            tmp_buf[3] = (WABCRYPTOFILE_MAGIC >> 24) & 0xFF;
            tmp_buf[4] = WABCRYPTOFILE_VERSION & 0xFF;
            tmp_buf[5] = (WABCRYPTOFILE_VERSION >> 8) & 0xFF;
            tmp_buf[6] = (WABCRYPTOFILE_VERSION >> 16) & 0xFF;
            tmp_buf[7] = (WABCRYPTOFILE_VERSION >> 24) & 0xFF;
            ::memcpy(tmp_buf + 8, iv, 32);

            // TODO: if I suceeded writing a broken file, wouldn't it be better to remove it ?
            if (const ssize_t write_ret = this->raw_write(snk, tmp_buf, 40)){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: write error! error=%s\n", ::getpid(), ::strerror(errno));
                return write_ret;
            }
            // update file_size
            this->file_size += 40;

            return this->xmd_update(tmp_buf, 40);
        }

        template<class Sink>
        ssize_t write(Sink & snk, const void * data, size_t len)
        {
            unsigned int remaining_size = len;
            while (remaining_size > 0) {
                // Check how much we can append into buffer
                unsigned int available_size = MIN(CRYPTO_BUFFER_SIZE - this->pos, remaining_size);
                // Append and update pos pointer
                ::memcpy(this->buf + this->pos, static_cast<const char*>(data) + (len - remaining_size), available_size);
                this->pos += available_size;
                // If buffer is full, flush it to disk
                if (this->pos == CRYPTO_BUFFER_SIZE) {
                    if (this->flush(snk)) {
                        return -1;
                    }
                }
                remaining_size -= available_size;
            }
            // Update raw size counter
            this->raw_size += len;
            return len;
        }

        /* Flush procedure (compression, encryption, effective file writing)
         * Return 0 on success, negatif on error
         */
        template<class Sink>
        int flush(Sink & snk)
        {
            // No data to flush
            if (!this->pos) {
                return 0;
            }

            // Compress
            // TODO: check this
            char compressed_buf[65536];
            //char compressed_buf[compressed_buf_sz];
            size_t compressed_buf_sz = ::snappy_max_compressed_length(this->pos);
            snappy_status status = snappy_compress(this->buf, this->pos, compressed_buf, &compressed_buf_sz);

            switch (status)
            {
                case SNAPPY_OK:
                    break;
                case SNAPPY_INVALID_INPUT:
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy compression failed with status code INVALID_INPUT!\n", getpid());
                    return -1;
                case SNAPPY_BUFFER_TOO_SMALL:
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy compression failed with status code BUFFER_TOO_SMALL!\n", getpid());
                    return -1;
            }

            // Encrypt
            unsigned char ciphered_buf[4 + 65536];
            //char ciphered_buf[ciphered_buf_sz];
            uint32_t ciphered_buf_sz = compressed_buf_sz + AES_BLOCK_SIZE;
            {
                const unsigned char * src_buf = reinterpret_cast<unsigned char*>(compressed_buf);
                if (this->xaes_encrypt(src_buf, compressed_buf_sz, ciphered_buf + 4, &ciphered_buf_sz)) {
                    return -1;
                }
            }

            ciphered_buf[0] = ciphered_buf_sz & 0xFF;
            ciphered_buf[1] = (ciphered_buf_sz >> 8) & 0xFF;
            ciphered_buf[2] = (ciphered_buf_sz >> 16) & 0xFF;
            ciphered_buf[3] = (ciphered_buf_sz >> 24) & 0xFF;

            ciphered_buf_sz += 4;

            if (const ssize_t err = this->raw_write(snk, ciphered_buf, ciphered_buf_sz)) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Write error : %s\n", ::getpid(), ::strerror(errno));
                return err;
            }
            if (-1 == this->xmd_update(&ciphered_buf, ciphered_buf_sz)) {
                return -1;
            }
            this->file_size += ciphered_buf_sz;

            // Reset buffer
            this->pos = 0;
            return 0;
        }

        template<class Sink>
        int close(Sink & snk, unsigned char hash[MD_HASH_LENGTH << 1], const unsigned char * hmac_key)
        {
            int result = this->flush(snk);

            const uint32_t eof_magic = WABCRYPTOFILE_EOF_MAGIC;
            unsigned char tmp_buf[8] = {
                eof_magic & 0xFF,
                (eof_magic >> 8) & 0xFF,
                (eof_magic >> 16) & 0xFF,
                (eof_magic >> 24) & 0xFF,
                uint8_t(this->raw_size & 0xFF),
                uint8_t((this->raw_size >> 8) & 0xFF),
                uint8_t((this->raw_size >> 16) & 0xFF),
                uint8_t((this->raw_size >> 24) & 0xFF),
            };

            int write_ret1 = this->raw_write(snk, tmp_buf, 8);
            if (write_ret1){
                // TOOD: actual error code could help
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Write error : %s\n", ::getpid(), ::strerror(errno));
            }
            this->file_size += 8;

            this->xmd_update(tmp_buf, 8);

            if (hash) {
                unsigned char tmp_hash[MD_HASH_LENGTH << 1];
                if (::EVP_DigestFinal_ex(&this->hctx4k, tmp_hash, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute 4k MD digests\n", ::getpid());
                    result = -1;
                    tmp_hash[0] = '\0';
                }
                if (::EVP_DigestFinal_ex(&this->hctx, tmp_hash + MD_HASH_LENGTH, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute MD digests\n", ::getpid());
                    result = -1;
                    tmp_hash[MD_HASH_LENGTH] = '\0';
                }
                // HMAC: MD(key^opad + MD(key^ipad))
                const EVP_MD *md = ::EVP_get_digestbyname(MD_HASH_NAME);
                if (!md) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not find MD message digest\n", ::getpid());
                    return -1;
                }
                const int     blocksize = ::EVP_MD_block_size(md);
                unsigned char * key_buf = new(std::nothrow) unsigned char[blocksize];
                if (key_buf == nullptr) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: malloc\n", ::getpid());
                    return -1;
                }
                const std::unique_ptr<unsigned char[]> auto_free(key_buf);
                ::memset(key_buf, '\0', blocksize);
                if (CRYPTO_KEY_LENGTH > blocksize) { // keys longer than blocksize are shortened
                    unsigned char keyhash[MD_HASH_LENGTH];
                    if ( ! ::MD_HASH_FUNC(hmac_key, CRYPTO_KEY_LENGTH, keyhash)) {
                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not hash crypto key\n", ::getpid());
                        return -1;
                    }
                    ::memcpy(key_buf, keyhash, MIN(MD_HASH_LENGTH, blocksize));
                }
                else {
                    ::memcpy(key_buf, hmac_key, CRYPTO_KEY_LENGTH);
                }
                for (int idx = 0; idx <  blocksize; idx++) {
                    key_buf[idx] = key_buf[idx] ^ 0x5c;
                }

                EVP_MD_CTX mdctx;
                ::EVP_MD_CTX_init(&mdctx);
                if (::EVP_DigestInit_ex(&mdctx, md, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize MD hash context\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&mdctx, key_buf, blocksize) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&mdctx, tmp_hash, MD_HASH_LENGTH) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestFinal_ex(&mdctx, hash, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute MD digests\n", ::getpid());
                    result = -1;
                    hash[0] = '\0';
                }
                ::EVP_MD_CTX_cleanup(&mdctx);
                ::EVP_MD_CTX_init(&mdctx);
                if (::EVP_DigestInit_ex(&mdctx, md, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize MD hash context\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&mdctx, key_buf, blocksize) != 1){
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestUpdate(&mdctx, tmp_hash + MD_HASH_LENGTH, MD_HASH_LENGTH) != 1){
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash\n", ::getpid());
                    return -1;
                }
                if (::EVP_DigestFinal_ex(&mdctx, hash + MD_HASH_LENGTH, nullptr) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute MD digests\n", ::getpid());
                    result = -1;
                    hash[MD_HASH_LENGTH] = '\0';
                }
                ::EVP_MD_CTX_cleanup(&mdctx);
            }

            return result;
        }

    private:
        ///\return 0 if success, otherwise a negatif number
        template<class Sink>
        ssize_t raw_write(Sink & snk, void * data, size_t len)
        {
            ssize_t err = snk.write(data, len);
            return err < ssize_t(len) ? (err < 0 ? err : -1) : 0;
        }

        /* Encrypt src_buf into dst_buf. Update dst_sz with encrypted output size
         * Return 0 on success, negative value on error
         */
        int xaes_encrypt(const unsigned char *src_buf, uint32_t src_sz, unsigned char *dst_buf, uint32_t *dst_sz)
        {
            int safe_size = *dst_sz;
            int remaining_size = 0;

            /* allows reusing of ectx for multiple encryption cycles */
            if (EVP_EncryptInit_ex(&this->ectx, nullptr, nullptr, nullptr, nullptr) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not prepare encryption context!\n", getpid());
                return -1;
            }
            if (EVP_EncryptUpdate(&this->ectx, dst_buf, &safe_size, src_buf, src_sz) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could encrypt data!\n", getpid());
                return -1;
            }
            if (EVP_EncryptFinal_ex(&this->ectx, dst_buf + safe_size, &remaining_size) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not finish encryption!\n", getpid());
                return -1;
            }
            *dst_sz = safe_size + remaining_size;
            return 0;
        }

        /* Update hash context with new data.
         * Returns 0 on success, -1 on error
         */
        int xmd_update(const void * src_buf, uint32_t src_sz)
        {
            if (::EVP_DigestUpdate(&this->hctx, src_buf, src_sz) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash!\n", ::getpid());
                return -1;
            }
            if (this->file_size < 4096) {
                size_t remaining_size = 4096 - this->file_size;
                size_t hashable_size = MIN(remaining_size, src_sz);
                if (::EVP_DigestUpdate(&this->hctx4k, src_buf, hashable_size) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update 4k hash!\n", ::getpid());
                    return -1;
                }
            }
            return 0;
        }
    };
}

namespace transbuf {
    struct ocrypto_filename_params
    {
        CryptoContext & crypto_ctx;
        Random & rnd;
    };
}

namespace detail
{
    struct ocrypto_filter
    : transfil::encrypt_filter
    {
        CryptoContext & cctx;
        Random & rnd;

        explicit ocrypto_filter(transbuf::ocrypto_filename_params params)
        : cctx(params.crypto_ctx)
        , rnd(params.rnd)
        {}

        template<class Buf>
        int open(Buf & buf, char const * filename) {
            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher

            size_t base_len = 0;
            const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(filename, base_len));

            this->cctx.get_derived_key(trace_key, base, base_len);
            unsigned char iv[32];
            this->rnd.random(iv, 32);
            return transfil::encrypt_filter::open(buf, trace_key, this->cctx, iv);
        }
    };
}

namespace detail
{

    template<class FilterParams = no_param>
    struct out_hash_meta_sequence_filename_buf_param
    {
        out_meta_sequence_filename_buf_param<FilterParams> meta_sq_params;
        FilterParams filter_params;
        CryptoContext & cctx;

        out_hash_meta_sequence_filename_buf_param(
            CryptoContext & cctx,
            time_t start_sec,
            FilenameGenerator::Format format,
            const char * const hash_prefix,
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid,
            FilterParams const & filter_params = FilterParams())
        : meta_sq_params(start_sec, format, hash_prefix, prefix, filename, extension, groupid, filter_params)
        , filter_params(filter_params)
        , cctx(cctx)
        {}
    };





    template<class BufFilter, class BufMeta, class BufHash, class Params>
    class out_hash_meta_sequence_filename_buf_impl
    : public out_meta_sequence_filename_buf_impl<BufMeta>
    {
        CryptoContext & cctx;
        Params hash_ctx;
        BufFilter wrm_filter;

        using sequence_base_type = out_meta_sequence_filename_buf_impl<BufMeta>;

    public:
        explicit out_hash_meta_sequence_filename_buf_impl(
            out_hash_meta_sequence_filename_buf_param<Params> const & params
        )
        : sequence_base_type(params.meta_sq_params)
        , cctx(params.cctx)
        , hash_ctx(params.filter_params)
        , wrm_filter(params.filter_params)
        {}

        ssize_t write(const void * data, size_t len)
        {
            if (!this->buf().is_open()) {
                const char * filename = this->get_filename_generate();
                const int res = this->open_filename(filename);
                if (res < 0) {
                    return res;
                }
                if (int err = this->wrm_filter.open(this->buf(), filename)) {
                    return err;
                }
            }
            return this->wrm_filter.write(this->buf(), data, len);
        }

        int close()
        {
            if (this->buf().is_open()) {
                if (this->next()) {
                    return 1;
                }
            }

            BufHash hash_buf(this->hash_ctx);

            if (!this->meta_buf().is_open()) {
                return 1;
            }

            hash_type hash;

            if (this->meta_buf().close(hash)) {
                return 1;
            }

            char const * hash_filename = this->hash_filename();
            char const * meta_filename = this->meta_filename();

            char path[1024] = {};
            char basename[1024] = {};
            char extension[256] = {};
            char filename[2048] = {};

            canonical_path(
                meta_filename,
                path, sizeof(path),
                basename, sizeof(basename),
                extension, sizeof(extension)
            );

            snprintf(filename, sizeof(filename), "%s%s", basename, extension);

            if (hash_buf.open(hash_filename, S_IRUSR|S_IRGRP) >= 0) {
                char header[] = "v2\n\n\n";
                hash_buf.write(header, sizeof(header)-1);

                struct stat stat;
                int err = ::stat(meta_filename, &stat);
                if (!err) {
                    err = write_meta_file_impl<false>(hash_buf, filename, stat, 0, 0, &hash);
                }
                if (!err) {
                    err = hash_buf.close(/*hash*/);
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

        int next()
        {
            if (this->buf().is_open()) {
                hash_type hash;
                {
                    const int res1 = this->wrm_filter.close(this->buf(), hash, this->cctx.get_hmac_key());
                    const int res2 = this->buf().close();
                    if (res1) {
                        return res1;
                    }
                    if (res2) {
                        return res2;
                    }
                }

                return this->next_meta_file(&hash);
            }
            return 1;
        }
    };

    class ochecksum_filter
    {
        transbuf::ochecksum_buf_null_buf sum_buf;

    public:
        explicit ochecksum_filter(CryptoContext & cctx)
        : sum_buf(cctx.get_hmac_key())
        {}

        template<class Buf>
        int open(Buf &, char const * /*filename*/) {
            return this->sum_buf.open();
        }

        template<class Buf>
        int write(Buf & buf, const void * data, size_t len) {
            this->sum_buf.write(data, len);
            return buf.write(data, len);
        }

        template<class Buf>
        int close(Buf &, hash_type & hash, unsigned char const (&)[MD_HASH_LENGTH]) {
            return this->sum_buf.close(hash);
        }
    };

    struct cctx_ofile_buf
    : transbuf::ofile_buf_out
    {
        explicit cctx_ofile_buf(CryptoContext &)
        {}
    };


    class cctx_ochecksum_file
    {
        int fd;

        static constexpr size_t nosize = ~size_t{};
        static constexpr size_t quick_size = 4096;

        SslHMAC_Sha256_Delayed hmac;
        SslHMAC_Sha256_Delayed quick_hmac;
        unsigned char const (&hmac_key)[MD_HASH_LENGTH];
        size_t file_size = nosize;

    public:
        explicit cctx_ochecksum_file(CryptoContext & cctx)
        : fd(-1)
        , hmac_key(cctx.get_hmac_key())
        {}

        ~cctx_ochecksum_file()
        {
            this->close();
        }

        cctx_ochecksum_file(cctx_ochecksum_file const &) = delete;
        cctx_ochecksum_file & operator=(cctx_ochecksum_file const &) = delete;

        int open(const char * filename, mode_t mode)
        {
            this->hmac.init(this->hmac_key, sizeof(this->hmac_key));
            this->quick_hmac.init(this->hmac_key, sizeof(this->hmac_key));
            this->file_size = 0;
            this->fd = ::open(filename, O_WRONLY | O_CREAT, mode);
            return this->fd;
        }

        ssize_t write(const void * data, size_t len)
        {
            REDASSERT(this->file_size != nosize);

            // TODO: hmac returns error as exceptions while write errors are returned as -1
            // this is inconsistent and probably need a fix.
            // also, if we choose to raise exception every error should have it's own one
            this->hmac.update(static_cast<const uint8_t *>(data), len);
            if (this->file_size < quick_size) {
                auto const remaining = std::min(quick_size - this->file_size, len);
                this->quick_hmac.update(static_cast<const uint8_t *>(data), remaining);
                this->file_size += remaining;
            }

            size_t remaining_len = len;
            size_t total_sent = 0;
            while (remaining_len) {
                ssize_t ret = ::write(this->fd,
                    static_cast<const char*>(data) + total_sent, remaining_len);
                if (ret <= 0){
                    if (errno == EINTR){
                        continue;
                    }
                    return -1;
                }
                remaining_len -= ret;
                total_sent += ret;
            }
            return total_sent;
        }

        int close(unsigned char (&hash)[MD_HASH_LENGTH * 2])
        {
            REDASSERT(this->file_size != nosize);
            this->quick_hmac.final(reinterpret_cast<unsigned char(&)[MD_HASH_LENGTH]>(hash[0]));
            this->hmac.final(reinterpret_cast<unsigned char(&)[MD_HASH_LENGTH]>(hash[MD_HASH_LENGTH]));
            this->file_size = nosize;
            return this->close();
        }

        int close()
        {
            if (this->is_open()) {
                const int ret = ::close(this->fd);
                this->fd = -1;
                return ret;
            }
            return 0;
        }

        bool is_open() const noexcept
        { return -1 != this->fd; }

        off64_t seek(off64_t offset, int whence) const
        { return ::lseek64(this->fd, offset, whence); }

        int flush() const
        { return 0; }
    };
}


namespace transbuf {
    class ocrypto_filename_buf
    {
        transfil::encrypt_filter encrypt;
        CryptoContext & cctx;
        Random & rnd;
        ofile_buf_out file;

    public:
        explicit ocrypto_filename_buf(ocrypto_filename_params params)
        : cctx(params.crypto_ctx)
        , rnd(params.rnd)
        {}

        ~ocrypto_filename_buf()
        {
            if (this->is_open()) {
                this->close();
            }
        }

        int open(const char * filename, mode_t mode = 0600)
        {
            int err = this->file.open(filename, mode);
            if (err < 0) {
                return err;
            }

            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            size_t base_len = 0;
            const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(filename, base_len));
            this->cctx.get_derived_key(trace_key, base, base_len);
            unsigned char iv[32];
            this->rnd.random(iv, 32);
            return this->encrypt.open(this->file, trace_key, this->cctx, iv);
        }

        ssize_t write(const void * data, size_t len)
        { return this->encrypt.write(this->file, data, len); }

        int close(unsigned char hash[MD_HASH_LENGTH << 1])
        {
            const int res1 = this->encrypt.close(this->file, hash, this->cctx.get_hmac_key());
            const int res2 = this->file.close();
            return res1 < 0 ? res1 : (res2 < 0 ? res2 : 0);
        }

        int close()
        {
            unsigned char hash[MD_HASH_LENGTH << 1];
            return this->close(hash);
        }

        bool is_open() const noexcept
        { return this->file.is_open(); }

        off64_t seek(off64_t offset, int whence) const
        { return this->file.seek(offset, whence); }

        int flush() const
        { return this->file.flush(); }
    };
}

struct OutMetaSequenceTransport : public Transport
{
    OutMetaSequenceTransport(
        const char * path,
        const char * hash_path,
        const char * basename,
        timeval now,
        uint16_t width,
        uint16_t height,
        const int groupid,
        auth_api * authentifier = nullptr,
        FilenameFormat format = FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
    : buf(detail::out_meta_sequence_filename_buf_param<>(
        now.tv_sec, format, hash_path, path, basename, ".wrm", groupid
    ))
    {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        detail::write_meta_headers(this->buffer().meta_buf(), path, width, height, this->authentifier, false);
    }

    void timestamp(timeval now) override {
        this->buffer().update_sec(now.tv_sec);
    }

    const FilenameGenerator * seqgen() const noexcept
    {
        return &(this->buffer().seqgen());
    }
    using Buf = detail::out_meta_sequence_filename_buf_impl<detail::empty_ctor<transbuf::ofile_buf_out>>;

    bool next() override {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        const ssize_t res = this->buffer().next();
        if (res) {
            this->status = false;
            if (res < 0){
                LOG(LOG_ERR, "Write to transport failed (M): code=%d", errno);
                throw Error(ERR_TRANSPORT_WRITE_FAILED, -res);
            }
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
        ++this->seqno;
        return true;
    }

    bool disconnect() override {
        return !this->buf.close();
    }

    void request_full_cleaning() override {
        this->buffer().request_full_cleaning();
    }

    ~OutMetaSequenceTransport() {
        this->buf.close();
    }

private:
    void do_send(const uint8_t * data, size_t len) override {
        const ssize_t res = this->buf.write(data, len);
        if (res < 0) {
            this->status = false;
            if (errno == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|%s", buf.current_path());
                this->authentifier->report("FILESYSTEM_FULL", message);
                errno = ENOSPC;
                throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, ENOSPC);
            }
            else {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        this->last_quantum_sent += res;
    }

    Buf & buffer() noexcept
    { return this->buf; }

    const Buf & buffer() const noexcept
    { return this->buf; }

    Buf buf;

};

namespace detail {


    class out_meta_sequence_filename_buf_impl_cctx
    : public out_sequence_filename_buf_impl
    {
        cctx_ochecksum_file meta_buf_;
        MetaFilename mf_;
        MetaFilename hf_;
        time_t start_sec_;
        time_t stop_sec_;

    public:
        template<class MetaParams>
        explicit out_meta_sequence_filename_buf_impl_cctx(
            out_meta_sequence_filename_buf_param<MetaParams> const & params
        )
        : out_sequence_filename_buf_impl(params.sq_params)
        , meta_buf_(params.meta_buf_params)
        , mf_(params.sq_params.prefix, params.sq_params.filename, params.sq_params.format)
        , hf_(params.hash_prefix, params.sq_params.filename, params.sq_params.format)
        , start_sec_(params.sec)
        , stop_sec_(params.sec)
        {
            if (this->meta_buf_.open(this->mf_.filename, S_IRUSR | S_IRGRP | S_IWUSR) < 0) {
                LOG(LOG_ERR, "Failed to open meta file %s", this->mf_.filename);
                throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
            }
            if (chmod(this->mf_.filename, S_IRUSR | S_IRGRP) == -1) {
                LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                   , this->mf_.filename
                   , "u+r, g+r"
                   , strerror(errno), errno);
            }
        }

        int close()
        {
            const int res1 = this->next();
            const int res2 = (this->meta_buf().is_open() ? this->meta_buf_.close() : 0);
            int err = res1 ? res1 : res2;
            if (!err) {
                char const * hash_filename = this->hash_filename();
                char const * meta_filename = this->meta_filename();
                transbuf::ofile_buf_out crypto_hash;

                char path[1024] = {};
                char basename[1024] = {};
                char extension[256] = {};
                char filename[2048] = {};

                canonical_path(
                    meta_filename,
                    path, sizeof(path),
                    basename, sizeof(basename),
                    extension, sizeof(extension)
                );

                snprintf(filename, sizeof(filename), "%s%s", basename, extension);

                if (crypto_hash.open(hash_filename, S_IRUSR|S_IRGRP) >= 0) {
                    char header[] = "v2\n\n\n";
                    crypto_hash.write(header, sizeof(header)-1);

                    struct stat stat;
                    int err = ::stat(meta_filename, &stat);
                    if (!err) {
                        err = write_meta_file_impl<false>(crypto_hash, filename, stat, 0, 0, nullptr);
                    }
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
        int next_meta_file(hash_type const * hash = nullptr)
        {
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

        char const * hash_filename() const noexcept
        {
            return this->hf_.filename;
        }

        char const * meta_filename() const noexcept
        {
            return this->mf_.filename;
        }

    public:
        void request_full_cleaning()
        {
            this->out_sequence_filename_buf_impl::request_full_cleaning();
            ::unlink(this->mf_.filename);
        }

        cctx_ochecksum_file & meta_buf() noexcept
        { return this->meta_buf_; }

        void update_sec(time_t sec)
        { this->stop_sec_ = sec; }
    };

class out_hash_meta_sequence_filename_buf_impl_cctx
: public out_meta_sequence_filename_buf_impl_cctx
{
    using BufFilter = detail::ochecksum_filter;
//    using BufMeta = detail::cctx_ochecksum_file;
    using BufHash = detail::cctx_ofile_buf;
    using Params = CryptoContext&;

    CryptoContext & cctx;
    Params hash_ctx;
    BufFilter wrm_filter;

public:
    explicit out_hash_meta_sequence_filename_buf_impl_cctx(
        out_hash_meta_sequence_filename_buf_param<Params> const & params
    )
    : out_meta_sequence_filename_buf_impl_cctx(params.meta_sq_params)
    , cctx(params.cctx)
    , hash_ctx(params.filter_params)
    , wrm_filter(params.filter_params)
    {}

    ssize_t write(const void * data, size_t len)
    {
        if (!this->buf().is_open()) {
            const char * filename = this->get_filename_generate();
            const int res = this->open_filename(filename);
            if (res < 0) {
                return res;
            }
            if (int err = this->wrm_filter.open(this->buf(), filename)) {
                return err;
            }
        }
        return this->wrm_filter.write(this->buf(), data, len);
    }

    int close()
    {
        if (this->buf().is_open()) {
            if (this->next()) {
                return 1;
            }
        }

        BufHash hash_buf(this->hash_ctx);

        if (!this->meta_buf().is_open()) {
            return 1;
        }

        hash_type hash;

        if (this->meta_buf().close(hash)) {
            return 1;
        }

        char const * hash_filename = this->hash_filename();
        char const * meta_filename = this->meta_filename();

        char path[1024] = {};
        char basename[1024] = {};
        char extension[256] = {};
        char filename[2048] = {};

        canonical_path(
            meta_filename,
            path, sizeof(path),
            basename, sizeof(basename),
            extension, sizeof(extension)
        );

        snprintf(filename, sizeof(filename), "%s%s", basename, extension);

        if (hash_buf.open(hash_filename, S_IRUSR|S_IRGRP) >= 0) {
            char header[] = "v2\n\n\n";
            hash_buf.write(header, sizeof(header)-1);

            struct stat stat;
            int err = ::stat(meta_filename, &stat);
            if (!err) {
                err = write_meta_file_impl<false>(hash_buf, filename, stat, 0, 0, &hash);
            }
            if (!err) {
                err = hash_buf.close(/*hash*/);
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

    int next()
    {
        if (this->buf().is_open()) {
            hash_type hash;
            {
                const int res1 = this->wrm_filter.close(this->buf(), hash, this->cctx.get_hmac_key());
                const int res2 = this->buf().close();
                if (res1) {
                    return res1;
                }
                if (res2) {
                    return res2;
                }
            }

            return this->next_meta_file(&hash);
        }
        return 1;
    }
};

}

struct OutMetaSequenceTransportWithSum : public Transport {

    OutMetaSequenceTransportWithSum(
        CryptoContext & crypto_ctx,
        const char * path,
        const char * hash_path,
        const char * basename,
        timeval now,
        uint16_t width,
        uint16_t height,
        const int groupid,
        auth_api * authentifier = nullptr,
        FilenameFormat format = FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
    : buf(
        detail::out_hash_meta_sequence_filename_buf_param<CryptoContext&>(
            crypto_ctx,
            now.tv_sec, format, hash_path, path, basename, ".wrm", groupid,
            crypto_ctx
        )
    ) {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        detail::write_meta_headers(this->buffer().meta_buf(), path, width, height, this->authentifier, true);
    }

    void timestamp(timeval now) override {
        this->buffer().update_sec(now.tv_sec);
    }

    const FilenameGenerator * seqgen() const noexcept
    {
        return &(this->buffer().seqgen());
    }

    bool next() override {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        const ssize_t res = this->buffer().next();
        if (res) {
            this->status = false;
            if (res < 0){
                LOG(LOG_ERR, "Write to transport failed (M): code=%d", errno);
                throw Error(ERR_TRANSPORT_WRITE_FAILED, -res);
            }
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
        ++this->seqno;
        return true;
    }

    bool disconnect() override {
        return !this->buf.close();
    }

    void request_full_cleaning() override {
        this->buffer().request_full_cleaning();
    }

    ~OutMetaSequenceTransportWithSum() {
        this->buf.close();
    }

private:
    void do_send(const uint8_t * data, size_t len) override {
        const ssize_t res = this->buf.write(data, len);
        if (res < 0) {
            this->status = false;
            if (errno == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|%s", buf.current_path());
                this->authentifier->report("FILESYSTEM_FULL", message);
                errno = ENOSPC;
                throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, ENOSPC);
            }
            else {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        this->last_quantum_sent += res;
    }

protected:
    detail::out_hash_meta_sequence_filename_buf_impl_cctx & buffer() noexcept
    { return this->buf; }

    const detail::out_hash_meta_sequence_filename_buf_impl_cctx & buffer() const noexcept
    { return this->buf; }

private:
    detail::out_hash_meta_sequence_filename_buf_impl_cctx buf;

};


struct CryptoOutMetaSequenceTransport
: public Transport {

    using Buf =
        detail::out_hash_meta_sequence_filename_buf_impl<
            detail::ocrypto_filter,
            transbuf::ocrypto_filename_buf,
            transbuf::ocrypto_filename_buf,
            transbuf::ocrypto_filename_params
        >;

    CryptoOutMetaSequenceTransport(
        CryptoContext & crypto_ctx,
        Random & rnd,
        const char * path,
        const char * hash_path,
        const char * basename,
        timeval now,
        uint16_t width,
        uint16_t height,
        const int groupid,
        auth_api * authentifier = nullptr,
        FilenameFormat format = FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
    : buf(
        detail::out_hash_meta_sequence_filename_buf_param<transbuf::ocrypto_filename_params>(
            crypto_ctx,
            now.tv_sec, format, hash_path, path, basename, ".wrm", groupid,
            {crypto_ctx, rnd}
        )) {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        detail::write_meta_headers(this->buffer().meta_buf(), path, width, height, this->authentifier, true);
    }

    void timestamp(timeval now) override {
        this->buffer().update_sec(now.tv_sec);
    }

    const FilenameGenerator * seqgen() const noexcept
    {
        return &(this->buffer().seqgen());
    }
    bool next() override {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        const ssize_t res = this->buffer().next();
        if (res) {
            this->status = false;
            if (res < 0){
                LOG(LOG_ERR, "Write to transport failed (M): code=%d", errno);
                throw Error(ERR_TRANSPORT_WRITE_FAILED, -res);
            }
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
        ++this->seqno;
        return true;
    }

    bool disconnect() override {
        return !this->buf.close();
    }

    void request_full_cleaning() override {
        this->buffer().request_full_cleaning();
    }

    ~CryptoOutMetaSequenceTransport() {
        this->buf.close();
    }

private:
    void do_send(const uint8_t * data, size_t len) override {
        const ssize_t res = this->buf.write(data, len);
        if (res < 0) {
            this->status = false;
            if (errno == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|%s", buf.current_path());
                this->authentifier->report("FILESYSTEM_FULL", message);
                errno = ENOSPC;
                throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, ENOSPC);
            }
            else {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        this->last_quantum_sent += res;
    }

    Buf & buffer() noexcept
    { return this->buf; }

    const Buf & buffer() const noexcept
    { return this->buf; }

    typedef CryptoOutMetaSequenceTransport TransportType;

    Buf buf;

};


class SaveStateChunk {
public:
    SaveStateChunk() {}

    void recv(InStream & stream, StateChunk & sc, uint8_t info_version) {
        this->send_recv(stream, sc, info_version);
    }

    void send(OutStream & stream, StateChunk & sc) {
        this->send_recv(stream, sc, ~0);
    }

private:
    static void io_uint8(InStream & stream, uint8_t & value) { value = stream.in_uint8(); }
    static void io_uint8(OutStream & stream, uint8_t value) { stream.out_uint8(value); }

    static void io_sint8(InStream & stream, int8_t & value) { value = stream.in_sint8(); }
    static void io_sint8(OutStream & stream, int8_t value) { stream.out_sint8(value); }

    // TODO BUG this is an error
    static void io_uint8_unsafe(InStream & stream, uint16_t & value) { value = stream.in_uint8(); }
    static void io_uint8_unsafe(OutStream & stream, uint16_t value) { stream.out_uint8(value); }

    static void io_uint16_le(InStream & stream, uint16_t & value) { value = stream.in_uint16_le(); }
    static void io_uint16_le(OutStream & stream, uint16_t value) { stream.out_uint16_le(value); }

    static void io_sint16_le(InStream & stream, int16_t & value) { value = stream.in_sint16_le(); }
    static void io_sint16_le(OutStream & stream, int16_t value) { stream.out_sint16_le(value); }

    static void io_uint32_le(InStream & stream, uint32_t & value) { value = stream.in_uint32_le(); }
    static void io_uint32_le(OutStream & stream, uint32_t value) { stream.out_uint32_le(value); }

    static void io_color(InStream & stream, uint32_t & color) {
        uint8_t const red   = stream.in_uint8();
        uint8_t const green = stream.in_uint8();
        uint8_t const blue  = stream.in_uint8();
        color = red | green << 8 | blue << 16;
    }
    static void io_color(OutStream & stream, uint32_t color) {
        stream.out_uint8(color);
        stream.out_uint8(color >> 8);
        stream.out_uint8(color >> 16);
    }

    static void io_copy_bytes(InStream & stream, uint8_t * buf, unsigned n) { stream.in_copy_bytes(buf, n); }
    static void io_copy_bytes(OutStream & stream, uint8_t * buf, unsigned n) { stream.out_copy_bytes(buf, n); }

    template<class Stream>
    static void io_delta_encoded_rects(Stream & stream, array_view<RDP::DeltaEncodedRectangle> delta_rectangles) {
        // TODO: check room to write or enough data to read, another io unified function necessary io_avail()
        for (RDP::DeltaEncodedRectangle & delta_rectangle : delta_rectangles) {
            io_sint16_le(stream, delta_rectangle.leftDelta);
            io_sint16_le(stream, delta_rectangle.topDelta);
            io_sint16_le(stream, delta_rectangle.width);
            io_sint16_le(stream, delta_rectangle.height);
        }
    }

    template<class Stream>
    static void io_brush(Stream & stream, RDPBrush & brush) {
        io_sint8(stream, brush.org_x);
        io_sint8(stream, brush.org_y);
        io_uint8(stream, brush.style);
        io_uint8(stream, brush.hatch);
        io_copy_bytes(stream, brush.extra, 7);
    }

    template<class Stream>
    static void io_rect(Stream & stream, Rect & rect) {
        io_sint16_le(stream, rect.x);
        io_sint16_le(stream, rect.y);
        io_uint16_le(stream, rect.cx);
        io_uint16_le(stream, rect.cy);
    }

    template<class Stream>
    void send_recv(Stream & stream, StateChunk & sc, uint8_t info_version) {
        const bool mem3blt_support         = (info_version > 1);
        const bool polyline_support        = (info_version > 2);
        const bool multidstblt_support     = (info_version > 3);
        const bool multiopaquerect_support = (info_version > 3);
        const bool multipatblt_support     = (info_version > 3);
        const bool multiscrblt_support     = (info_version > 3);

        // RDPOrderCommon common;
        io_uint8(stream, sc.common.order);
        io_rect(stream, sc.common.clip);

        // RDPDestBlt destblt;
        io_rect(stream, sc.destblt.rect);
        io_uint8(stream, sc.destblt.rop);

        // RDPPatBlt patblt;
        io_rect(stream, sc.patblt.rect);
        io_uint8(stream, sc.patblt.rop);
        io_uint32_le(stream, sc.patblt.back_color);
        io_uint32_le(stream, sc.patblt.fore_color);
        io_brush(stream, sc.patblt.brush);

        // RDPScrBlt scrblt;
        io_rect(stream, sc.scrblt.rect);
        io_uint8(stream, sc.scrblt.rop);
        io_uint16_le(stream, sc.scrblt.srcx);
        io_uint16_le(stream, sc.scrblt.srcy);

        // RDPOpaqueRect opaquerect;
        io_rect(stream, sc.opaquerect.rect);
        io_color(stream, sc.opaquerect.color);

        // RDPMemBlt memblt;
        io_uint16_le(stream, sc.memblt.cache_id);
        io_rect(stream, sc.memblt.rect);
        io_uint8(stream, sc.memblt.rop);
        // TODO bad length
        io_uint8_unsafe(stream, sc.memblt.srcx);
        io_uint8_unsafe(stream, sc.memblt.srcy);
        io_uint16_le(stream, sc.memblt.cache_idx);

        // RDPMem3Blt memblt;
        if (mem3blt_support) {
            io_uint16_le(stream, sc.mem3blt.cache_id);
            io_rect(stream, sc.mem3blt.rect);
            io_uint8(stream, sc.mem3blt.rop);
            io_uint8_unsafe(stream, sc.mem3blt.srcx);
            io_uint8_unsafe(stream, sc.mem3blt.srcy);
            io_uint32_le(stream, sc.mem3blt.back_color);
            io_uint32_le(stream, sc.mem3blt.fore_color);
            io_brush(stream, sc.mem3blt.brush);
            io_uint16_le(stream, sc.mem3blt.cache_idx);
        }

        // RDPLineTo lineto;
        io_uint8(stream, sc.lineto.back_mode);
        io_sint16_le(stream, sc.lineto.startx);
        io_sint16_le(stream, sc.lineto.starty);
        io_sint16_le(stream, sc.lineto.endx);
        io_sint16_le(stream, sc.lineto.endy);
        io_uint32_le(stream, sc.lineto.back_color);
        io_uint8(stream, sc.lineto.rop2);
        io_uint8(stream, sc.lineto.pen.style);
        io_uint8(stream, sc.lineto.pen.width);
        io_uint32_le(stream, sc.lineto.pen.color);

        // RDPGlyphIndex glyphindex;
        io_uint8(stream, sc.glyphindex.cache_id);
        io_sint16_le(stream, sc.glyphindex.fl_accel);
        io_sint16_le(stream, sc.glyphindex.ui_charinc);
        io_sint16_le(stream, sc.glyphindex.f_op_redundant);
        io_uint32_le(stream, sc.glyphindex.back_color);
        io_uint32_le(stream, sc.glyphindex.fore_color);
        io_rect(stream, sc.glyphindex.bk);
        io_rect(stream, sc.glyphindex.op);
        io_brush(stream, sc.glyphindex.brush);
        io_sint16_le(stream, sc.glyphindex.glyph_x);
        io_sint16_le(stream, sc.glyphindex.glyph_y);
        io_uint8(stream, sc.glyphindex.data_len);
        io_copy_bytes(stream, sc.glyphindex.data, 256);

        // RDPPolyine polyline;
        if (polyline_support) {
            io_sint16_le(stream, sc.polyline.xStart);
            io_sint16_le(stream, sc.polyline.yStart);
            io_uint8(stream, sc.polyline.bRop2);
            io_uint16_le(stream, sc.polyline.BrushCacheEntry);
            io_uint32_le(stream, sc.polyline.PenColor);
            io_uint8(stream, sc.polyline.NumDeltaEntries);
            // TODO: check room to write or enough data to read, another io unified function necessary io_avail()
            for (uint8_t i = 0; i < sc.polyline.NumDeltaEntries; i++) {
                io_sint16_le(stream, sc.polyline.deltaEncodedPoints[i].xDelta);
                io_sint16_le(stream, sc.polyline.deltaEncodedPoints[i].yDelta);
            }
        }

        // RDPMultiDstBlt multidstblt;
        if (multidstblt_support) {
            io_sint16_le(stream, sc.multidstblt.nLeftRect);
            io_sint16_le(stream, sc.multidstblt.nTopRect);
            io_sint16_le(stream, sc.multidstblt.nWidth);
            io_sint16_le(stream, sc.multidstblt.nHeight);
            io_uint8(stream, sc.multidstblt.bRop);
            io_uint8(stream, sc.multidstblt.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multidstblt.deltaEncodedRectangles,
                sc.multidstblt.nDeltaEntries
            });
        }

        // RDPMultiOpaqueRect multiopaquerect;
        if (multiopaquerect_support) {
            io_sint16_le(stream, sc.multiopaquerect.nLeftRect);
            io_sint16_le(stream, sc.multiopaquerect.nTopRect);
            io_sint16_le(stream, sc.multiopaquerect.nWidth);
            io_sint16_le(stream, sc.multiopaquerect.nHeight);
            io_color(stream, sc.multiopaquerect._Color);
            io_uint8(stream, sc.multiopaquerect.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multiopaquerect.deltaEncodedRectangles,
                sc.multiopaquerect.nDeltaEntries
            });
        }

        // RDPMultiPatBlt multipatblt;
        if (multipatblt_support) {
            io_rect(stream, sc.multipatblt.rect);
            io_uint8(stream, sc.multipatblt.bRop);
            io_uint32_le(stream, sc.multipatblt.BackColor);
            io_uint32_le(stream, sc.multipatblt.ForeColor);
            io_brush(stream, sc.multipatblt.brush);
            io_uint8(stream, sc.multipatblt.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multipatblt.deltaEncodedRectangles,
                sc.multipatblt.nDeltaEntries
            });
        }

        // RDPMultiScrBlt multiscrblt;
        if (multiscrblt_support) {
            io_rect(stream, sc.multiscrblt.rect);
            io_uint8(stream, sc.multiscrblt.bRop);
            io_sint16_le(stream, sc.multiscrblt.nXSrc);
            io_sint16_le(stream, sc.multiscrblt.nYSrc);
            io_uint8(stream, sc.multiscrblt.nDeltaEntries);
            io_delta_encoded_rects(stream, {
                sc.multiscrblt.deltaEncodedRectangles,
                sc.multiscrblt.nDeltaEntries
            });
        }
    }
};

class InChunkedImageTransport : public Transport
{
    uint16_t chunk_type;
    uint32_t chunk_size;
    uint16_t chunk_count;
    Transport * trans;
    char buf[65536];
    InStream in_stream;

public:
    InChunkedImageTransport(uint16_t chunk_type, uint32_t chunk_size, Transport * trans)
        : chunk_type(chunk_type)
        , chunk_size(chunk_size)
        , chunk_count(1)
        , trans(trans)
        , in_stream(this->buf, this->chunk_size - 8)
    {
        auto * p = this->buf;
        this->trans->recv(&p, this->in_stream.get_capacity());
    }

private:
    void do_recv(uint8_t ** pbuffer, size_t len) override {
        size_t total_len = 0;
        while (total_len < len){
            size_t remaining = in_stream.in_remain();
            if (remaining >= (len - total_len)){
                in_stream.in_copy_bytes(*pbuffer + total_len, len - total_len);
                *pbuffer += len;
                return;
            }
            in_stream.in_copy_bytes(*pbuffer + total_len, remaining);
            total_len += remaining;
            switch (this->chunk_type){
            case PARTIAL_IMAGE_CHUNK:
            {
                const size_t header_sz = 8;
                char header_buf[header_sz];
                InStream header(header_buf);
                auto * p = header_buf;
                this->trans->recv(&p, header_sz);
                this->chunk_type = header.in_uint16_le();
                this->chunk_size = header.in_uint32_le();
                this->chunk_count = header.in_uint16_le();
                this->in_stream = InStream(this->buf, this->chunk_size - 8);
                p = this->buf;
                this->trans->recv(&p, this->chunk_size - 8);
            }
            break;
            case LAST_IMAGE_CHUNK:
                LOG(LOG_ERR, "Failed to read embedded image from WRM (transport closed)");
                throw Error(ERR_TRANSPORT_NO_MORE_DATA);
            default:
                LOG(LOG_ERR, "Failed to read embedded image from WRM");
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
        }
    }
};

struct FileToGraphic
{
    enum {
        HEADER_SIZE = 8
    };

private:
    uint8_t stream_buf[65536];
    InStream stream;

    CompressionInTransportBuilder compression_builder;

    Transport * trans_source;
    Transport * trans;

public:
    Rect screen_rect;

    // Internal state of orders
    StateChunk ssc;

    BmpCache     * bmp_cache;
    PointerCache   ptr_cache;
    GlyphCache     gly_cache;

    // variables used to read batch of orders "chunks"
    uint32_t chunk_size;
    uint16_t chunk_type;
    uint16_t chunk_count;
private:
    uint16_t remaining_order_count;

public:
    // total number of RDP orders read from the start of the movie
    // (non orders chunks are counted as 1 order)
    uint32_t total_orders_count;

    timeval record_now;

private:
    timeval start_record_now;
    timeval start_synctime_now;

public:
    template<class T, std::size_t N>
    struct fixed_ptr_array
    {
        fixed_ptr_array() : last(arr) {}

        void push_back(T * p) {
            if (p) {
                assert(this->size() < N);
                *this->last = p;
                ++this->last;
            }
        }

        T * * begin() { return this->arr; }
        T * * end() { return this->last; }

        std::size_t size() const noexcept {
            return static_cast<std::size_t>(this->last - this->arr);
        }

    private:
        T * arr[N];
        T * * last = arr;
    };

    fixed_ptr_array<gdi::GraphicApi, 10> graphic_consumers;
    fixed_ptr_array<gdi::CaptureApi, 10> capture_consumers;
    fixed_ptr_array<gdi::KbdInputApi, 10> kbd_input_consumers;
    fixed_ptr_array<gdi::CaptureProbeApi, 10> capture_probe_consumers;
    fixed_ptr_array<gdi::ExternalCaptureApi, 10> external_event_consumers;

    bool meta_ok;
    bool timestamp_ok;
    uint16_t mouse_x;
    uint16_t mouse_y;
    bool real_time;

    const BGRPalette & palette = BGRPalette::classic_332(); // We don't really care movies are always 24 bits for now

    const timeval begin_capture;
    const timeval end_capture;
    uint32_t max_order_count;

    uint16_t info_version;
    uint16_t info_width;
    uint16_t info_height;
    uint16_t info_bpp;
    uint16_t info_number_of_cache;
    bool     info_use_waiting_list;
    uint16_t info_cache_0_entries;
    uint16_t info_cache_0_size;
    bool     info_cache_0_persistent;
    uint16_t info_cache_1_entries;
    uint16_t info_cache_1_size;
    bool     info_cache_1_persistent;
    uint16_t info_cache_2_entries;
    uint16_t info_cache_2_size;
    bool     info_cache_2_persistent;
    uint16_t info_cache_3_entries;
    uint16_t info_cache_3_size;
    bool     info_cache_3_persistent;
    uint16_t info_cache_4_entries;
    uint16_t info_cache_4_size;
    bool     info_cache_4_persistent;
    WrmCompressionAlgorithm info_compression_algorithm;

    bool ignore_frame_in_timeval;

    struct Statistics {
        uint32_t DstBlt;
        uint32_t MultiDstBlt;
        uint32_t PatBlt;
        uint32_t MultiPatBlt;
        uint32_t OpaqueRect;
        uint32_t MultiOpaqueRect;
        uint32_t ScrBlt;
        uint32_t MultiScrBlt;
        uint32_t MemBlt;
        uint32_t Mem3Blt;
        uint32_t LineTo;
        uint32_t GlyphIndex;
        uint32_t Polyline;
        uint32_t EllipseSC;

        uint32_t CacheBitmap;
        uint32_t CacheColorTable;
        uint32_t CacheGlyph;

        uint32_t FrameMarker;

        uint32_t BitmapUpdate;

        uint32_t CachePointer;
        uint32_t PointerIndex;

        uint32_t graphics_update_chunk;
        uint32_t bitmap_update_chunk;
        uint32_t timestamp_chunk;
    } statistics;

    bool break_privplay_qt;
    uint64_t movie_elapsed_qt;

    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        play        = 8,
        timestamp   = 16,
        rdp_orders  = 32,
        probe       = 64,
    };

    FileToGraphic(Transport & trans, const timeval begin_capture, const timeval end_capture, bool real_time, Verbose verbose)
        : stream(stream_buf)
        , compression_builder(trans, WrmCompressionAlgorithm::no_compression)
        , trans_source(&trans)
        , trans(&trans)
        , bmp_cache(nullptr)
        // variables used to read batch of orders "chunks"
        , chunk_size(0)
        , chunk_type(0)
        , chunk_count(0)
        , remaining_order_count(0)
        , total_orders_count(0)
        , meta_ok(false)
        , timestamp_ok(false)
        , mouse_x(0)
        , mouse_y(0)
        , real_time(real_time)
        , begin_capture(begin_capture)
        , end_capture(end_capture)
        , max_order_count(0)
        , info_version(0)
        , info_width(0)
        , info_height(0)
        , info_bpp(0)
        , info_number_of_cache(0)
        , info_use_waiting_list(true)
        , info_cache_0_entries(0)
        , info_cache_0_size(0)
        , info_cache_0_persistent(false)
        , info_cache_1_entries(0)
        , info_cache_1_size(0)
        , info_cache_1_persistent(false)
        , info_cache_2_entries(0)
        , info_cache_2_size(0)
        , info_cache_2_persistent(false)
        , info_cache_3_entries(0)
        , info_cache_3_size(0)
        , info_cache_3_persistent(false)
        , info_cache_4_entries(0)
        , info_cache_4_size(0)
        , info_cache_4_persistent(false)
        , info_compression_algorithm(WrmCompressionAlgorithm::no_compression)
        , ignore_frame_in_timeval(false)
        , statistics()
        , break_privplay_qt(false)
        , movie_elapsed_qt(0)
        , verbose(verbose)
    {
        while (this->next_order()){
            this->interpret_order();
            if (this->meta_ok && this->timestamp_ok){
                break;
            }
        }
    }

    ~FileToGraphic()
    {
        delete this->bmp_cache;
    }

    void add_consumer(
        gdi::GraphicApi * graphic_ptr,
        gdi::CaptureApi * capture_ptr,
        gdi::KbdInputApi * kbd_input_ptr,
        gdi::CaptureProbeApi * capture_probe_ptr,
        gdi::ExternalCaptureApi * external_event_ptr
    ) {
        this->graphic_consumers.push_back(graphic_ptr);
        this->capture_consumers.push_back(capture_ptr);
        this->kbd_input_consumers.push_back(kbd_input_ptr);
        this->capture_probe_consumers.push_back(capture_probe_ptr);
        this->external_event_consumers.push_back(external_event_ptr);
    }

    /* order count set this->stream.p to the beginning of the next order.
     * Most of the times it means not changing it, except when it must read next chunk
     * when remaining order count is 0.
     * It update chunk headers (merely remaining orders count) and
     * reads the next chunk if necessary.
     */
    bool next_order()
    {
        if (this->chunk_type != LAST_IMAGE_CHUNK
         && this->chunk_type != PARTIAL_IMAGE_CHUNK) {
            if (this->stream.get_current() == this->stream.get_data_end()
             && this->remaining_order_count) {
                LOG(LOG_ERR, "Incomplete order batch at chunk %" PRIu16 " "
                             "order [%u/%" PRIu16 "] "
                             "remaining [%zu/%" PRIu32 "]",
                             this->chunk_type,
                             (this->chunk_count-this->remaining_order_count), this->chunk_count,
                             this->stream.in_remain(), this->chunk_size);
                return false;
            }
        }
        if (!this->remaining_order_count){
            for (gdi::GraphicApi * gd : this->graphic_consumers){
                gd->sync();
            }

            try {
                uint8_t buf[HEADER_SIZE];
                {
                    auto end = buf;
                    this->trans->recv(&end, HEADER_SIZE);
                }
                InStream header(buf);
                this->chunk_type = header.in_uint16_le();
                this->chunk_size = header.in_uint32_le();
                this->remaining_order_count = this->chunk_count = header.in_uint16_le();

                if (this->chunk_type != LAST_IMAGE_CHUNK && this->chunk_type != PARTIAL_IMAGE_CHUNK) {
                    switch (this->chunk_type) {
                        case RDP_UPDATE_ORDERS:
                            this->statistics.graphics_update_chunk++; break;
                        case RDP_UPDATE_BITMAP:
                            this->statistics.bitmap_update_chunk++;   break;
                        case TIMESTAMP:
                            this->statistics.timestamp_chunk++;       break;
                    }
                    if (this->chunk_size > 65536){
                        LOG(LOG_INFO,"chunk_size (%d) > 65536", this->chunk_size);
                        return false;
                    }
                    this->stream = InStream(this->stream_buf);
                    if (this->chunk_size - HEADER_SIZE > 0) {
                        this->stream = InStream(this->stream_buf, this->chunk_size - HEADER_SIZE);
                        auto end = this->stream_buf;
                        this->trans->recv(&end, this->chunk_size - HEADER_SIZE);
                    }
                }
            }
            catch (Error & e){
                if (e.id == ERR_TRANSPORT_OPEN_FAILED) {
                    throw;
                }

                if (this->verbose) {
                    LOG(LOG_INFO,"receive error %u : end of transport", e.id);
                }
                // receive error, end of transport
                return false;
            }
        }
        if (this->remaining_order_count > 0){this->remaining_order_count--;}
        return true;
    }

    void interpret_order()
    {
        this->total_orders_count++;
        switch (this->chunk_type){
        case RDP_UPDATE_ORDERS:
        {
            if (!this->meta_ok){
                LOG(LOG_ERR, "Drawing orders chunk must be preceded by a META chunk to get drawing device size");
                throw Error(ERR_WRM);
            }
            if (!this->timestamp_ok){
                LOG(LOG_ERR, "Drawing orders chunk must be preceded by a TIMESTAMP chunk to get drawing timing\n");
                throw Error(ERR_WRM);
            }
            uint8_t control = this->stream.in_uint8();
            uint8_t class_ = (control & (RDP::STANDARD | RDP::SECONDARY));
            if (class_ == RDP::SECONDARY) {
                RDP::AltsecDrawingOrderHeader header(control);
                switch (header.orderType) {
                    case RDP::AltsecDrawingOrderHeader::FrameMarker:
                    {
                        this->statistics.FrameMarker++;
                        RDP::FrameMarker order;

                        order.receive(stream, header);
                        if (this->verbose & Verbose::rdp_orders){
                            order.log(LOG_INFO);
                        }
                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                            gd->draw(order);
                        }
                    }
                    break;
                    case RDP::AltsecDrawingOrderHeader::Window:
                        this->process_windowing(stream, header);
                    break;
                    default:
                        LOG(LOG_ERR, "unsupported Alternate Secondary Drawing Order (%d)", header.orderType);
                        /* error, unknown order */
                    break;
                }
            }
            else if (class_ == (RDP::STANDARD | RDP::SECONDARY)) {
                RDPSecondaryOrderHeader header(this->stream);
                uint8_t const *next_order = this->stream.get_current() + header.order_data_length();
                switch (header.type) {
                case RDP::TS_CACHE_BITMAP_COMPRESSED:
                case RDP::TS_CACHE_BITMAP_UNCOMPRESSED:
                {
                    this->statistics.CacheBitmap++;
                    RDPBmpCache cmd;
                    cmd.receive(this->stream, header, this->palette, this->info_bpp);
                    if (this->verbose & Verbose::rdp_orders){
                        cmd.log(LOG_INFO);
                    }
                    this->bmp_cache->put(cmd.id, cmd.idx, cmd.bmp, cmd.key1, cmd.key2);
                }
                break;
                case RDP::TS_CACHE_COLOR_TABLE:
                    this->statistics.CacheColorTable++;
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_COLOR_TABLE (%d)", header.type);
                    break;
                case RDP::TS_CACHE_GLYPH:
                {
                    this->statistics.CacheGlyph++;
                    RDPGlyphCache cmd;
                    cmd.receive(this->stream, header);
                    if (this->verbose & Verbose::rdp_orders){
                        cmd.log(LOG_INFO);
                    }
                    this->gly_cache.set_glyph(
                        FontChar(std::move(cmd.aj), cmd.x, cmd.y, cmd.cx, cmd.cy, -1),
                        cmd.cacheId, cmd.cacheIndex
                    );
                }
                break;
                case RDP::TS_CACHE_BITMAP_COMPRESSED_REV2:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV2 (%d)", header.type);
                  break;
                case RDP::TS_CACHE_BITMAP_UNCOMPRESSED_REV2:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_UNCOMPRESSED_REV2 (%d)", header.type);
                  break;
                case RDP::TS_CACHE_BITMAP_COMPRESSED_REV3:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER TS_CACHE_BITMAP_COMPRESSED_REV3 (%d)", header.type);
                  break;
                default:
                    LOG(LOG_ERR, "unsupported SECONDARY ORDER (%d)", header.type);
                    /* error, unknown order */
                    break;
                }
                this->stream.in_skip_bytes(next_order - this->stream.get_current());
            }
            else if (class_ == RDP::STANDARD) {
                RDPPrimaryOrderHeader header = this->ssc.common.receive(this->stream, control);
                const Rect clip = (control & RDP::BOUNDS) ? this->ssc.common.clip : this->screen_rect;
                switch (this->ssc.common.order) {
                case RDP::GLYPHINDEX:
                    this->statistics.GlyphIndex++;
                    this->ssc.glyphindex.receive(this->stream, header);
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.glyphindex, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette), this->gly_cache);
                    }
                    break;
                case RDP::DESTBLT:
                    this->statistics.DstBlt++;
                    this->ssc.destblt.receive(this->stream, header);
                    if (this->verbose & Verbose::rdp_orders){
                        this->ssc.destblt.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.destblt, clip);
                    }
                    break;
                case RDP::MULTIDSTBLT:
                    this->statistics.MultiDstBlt++;
                    this->ssc.multidstblt.receive(this->stream, header);
                    if (this->verbose & Verbose::rdp_orders){
                        this->ssc.multidstblt.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.multidstblt, clip);
                    }
                    break;
                case RDP::MULTIOPAQUERECT:
                    this->statistics.MultiOpaqueRect++;
                    this->ssc.multiopaquerect.receive(this->stream, header);
                    if (this->verbose & Verbose::rdp_orders){
                        this->ssc.multiopaquerect.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.multiopaquerect, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                    }
                    break;
                case RDP::MULTIPATBLT:
                    this->statistics.MultiPatBlt++;
                    this->ssc.multipatblt.receive(this->stream, header);
                    if (this->verbose & Verbose::rdp_orders){
                        this->ssc.multipatblt.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.multipatblt, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                    }
                    break;
                case RDP::MULTISCRBLT:
                    this->statistics.MultiScrBlt++;
                    this->ssc.multiscrblt.receive(this->stream, header);
                    if (this->verbose & Verbose::rdp_orders){
                        this->ssc.multiscrblt.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.multiscrblt, clip);
                    }
                    break;
                case RDP::PATBLT:
                    this->statistics.PatBlt++;
                    this->ssc.patblt.receive(this->stream, header);
                    if (this->verbose & Verbose::rdp_orders){
                        this->ssc.patblt.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.patblt, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                    }
                    break;
                case RDP::SCREENBLT:
                    this->statistics.ScrBlt++;
                    this->ssc.scrblt.receive(this->stream, header);
                    if (this->verbose & Verbose::rdp_orders){
                        this->ssc.scrblt.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.scrblt, clip);
                    }
                    break;
                case RDP::LINE:
                    this->statistics.LineTo++;
                    this->ssc.lineto.receive(this->stream, header);
                    if (this->verbose & Verbose::rdp_orders){
                        this->ssc.lineto.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.lineto, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                    }
                    break;
                case RDP::RECT:
                    this->statistics.OpaqueRect++;
                    this->ssc.opaquerect.receive(this->stream, header);
                    if (this->verbose & Verbose::rdp_orders){
                        this->ssc.opaquerect.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.opaquerect, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                    }
                    break;
                case RDP::MEMBLT:
                    {
                        this->statistics.MemBlt++;
                        this->ssc.memblt.receive(this->stream, header);
                        if (this->verbose & Verbose::rdp_orders){
                            this->ssc.memblt.log(LOG_INFO, clip);
                        }
                        const Bitmap & bmp = this->bmp_cache->get(this->ssc.memblt.cache_id, this->ssc.memblt.cache_idx);
                        if (!bmp.is_valid()){
                            LOG(LOG_ERR, "Memblt bitmap not found in cache at (%u, %u)", this->ssc.memblt.cache_id, this->ssc.memblt.cache_idx);
                            throw Error(ERR_WRM);
                        }
                        else {
                            for (gdi::GraphicApi * gd : this->graphic_consumers){
                                gd->draw(this->ssc.memblt, clip, bmp);
                            }
                        }
                    }
                    break;
                case RDP::MEM3BLT:
                    {
                        this->statistics.Mem3Blt++;
                        this->ssc.mem3blt.receive(this->stream, header);
                        if (this->verbose & Verbose::rdp_orders){
                            this->ssc.mem3blt.log(LOG_INFO, clip);
                        }
                        const Bitmap & bmp = this->bmp_cache->get(this->ssc.mem3blt.cache_id, this->ssc.mem3blt.cache_idx);
                        if (!bmp.is_valid()){
                            LOG(LOG_ERR, "Mem3blt bitmap not found in cache at (%u, %u)", this->ssc.mem3blt.cache_id, this->ssc.mem3blt.cache_idx);
                            throw Error(ERR_WRM);
                        }
                        else {
                            for (gdi::GraphicApi * gd : this->graphic_consumers){
                                gd->draw(this->ssc.mem3blt, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette), bmp);
                            }
                        }
                    }
                    break;
                case RDP::POLYLINE:
                    this->statistics.Polyline++;
                    this->ssc.polyline.receive(this->stream, header);
                    if (this->verbose & Verbose::rdp_orders){
                        this->ssc.polyline.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.polyline, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                    }
                    break;
                case RDP::ELLIPSESC:
                    this->statistics.EllipseSC++;
                    this->ssc.ellipseSC.receive(this->stream, header);
                    if (this->verbose & Verbose::rdp_orders){
                        this->ssc.ellipseSC.log(LOG_INFO, clip);
                    }
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(this->ssc.ellipseSC, clip, gdi::ColorCtx::from_bpp(this->info_bpp, this->palette));
                    }
                    break;
                default:
                    /* error unknown order */
                    LOG(LOG_ERR, "unsupported PRIMARY ORDER (%d)", this->ssc.common.order);
                    throw Error(ERR_WRM);
                }
            }
            else {
                /* error, this should always be set */
                LOG(LOG_ERR, "Unsupported drawing order detected : protocol error");
                throw Error(ERR_WRM);
            }
            }
            break;
            case TIMESTAMP:
            {
                this->stream.in_timeval_from_uint64le_usec(this->record_now);

                for (gdi::ExternalCaptureApi * obj : this->external_event_consumers){
                    obj->external_time(this->record_now);
                }

                // If some data remains, it is input data : mouse_x, mouse_y and decoded keyboard keys (utf8)
                if (this->stream.in_remain() > 0){
                    if (this->stream.in_remain() < 4){
                        LOG(LOG_WARNING, "Input data truncated");
                        hexdump_d(stream.get_data(), stream.in_remain());
                    }

                    this->mouse_x = this->stream.in_uint16_le();
                    this->mouse_y = this->stream.in_uint16_le();

                    if (  (this->info_version > 1)
                       && this->stream.in_uint8()) {
                        this->ignore_frame_in_timeval = true;
                    }

                    if (this->verbose & Verbose::timestamp) {
                        LOG( LOG_INFO, "TIMESTAMP %lu.%lu mouse (x=%" PRIu16 ", y=%" PRIu16 ")\n"
                           , static_cast<unsigned long>(this->record_now.tv_sec)
                           , static_cast<unsigned long>(this->record_now.tv_usec)
                           , this->mouse_x
                           , this->mouse_y);
                    }


                    auto const input_data = this->stream.get_current();
                    auto const input_len = this->stream.in_remain();
                    this->stream.in_skip_bytes(input_len);
                    for (gdi::KbdInputApi * kbd : this->kbd_input_consumers){
                        InStream input(input_data, input_len);
                        while (input.in_remain()) {
                            kbd->kbd_input(this->record_now, input.in_uint32_le());
                        }
                    }

                    if (this->verbose & Verbose::timestamp) {
                        for (auto data = input_data, end = data + input_len/4; data != end; data += 4) {
                            uint8_t         key8[6];
                            const size_t    len = UTF32toUTF8(data, 4, key8, sizeof(key8)-1);
                            key8[len] = 0;

                            LOG( LOG_INFO, "TIMESTAMP %lu.%lu keyboard '%s'"
                                , static_cast<unsigned long>(this->record_now.tv_sec)
                                , static_cast<unsigned long>(this->record_now.tv_usec)
                                , key8);
                        }
                    }
                }

                if (!this->timestamp_ok) {
                   if (this->real_time) {
                        this->start_record_now   = this->record_now;
                        this->start_synctime_now = tvtime();
                    }
                    this->timestamp_ok = true;
                }
                else {
                   if (this->real_time) {
                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                            gd->sync();
                        }

                        this->movie_elapsed_qt = difftimeval(this->record_now, this->start_record_now);

                        /*struct timeval now     = tvtime();
                        uint64_t       elapsed = difftimeval(now, this->start_synctime_now);

                        uint64_t movie_elapsed = difftimeval(this->record_now, this->start_record_now);
                        this->movie_elapsed_qt = movie_elapsed;

                        if (elapsed < movie_elapsed) {
                            struct timespec wtime     = {
                                  static_cast<time_t>( (movie_elapsed - elapsed) / 1000000LL)
                                , static_cast<time_t>(((movie_elapsed - elapsed) % 1000000LL) * 1000)
                                };
                            struct timespec wtime_rem = { 0, 0 };*/

                            /*while ((nanosleep(&wtime, &wtime_rem) == -1) && (errno == EINTR)) {
                                wtime = wtime_rem;
                            }
                        } */
                    }
                }
            }
            break;
            case META_FILE:
            // TODO Cache meta_data (sizes, number of entries) should be put in META chunk
            {
                this->info_version                   = this->stream.in_uint16_le();
                this->info_width                     = this->stream.in_uint16_le();
                this->info_height                    = this->stream.in_uint16_le();
                this->info_bpp                       = this->stream.in_uint16_le();
                this->info_cache_0_entries           = this->stream.in_uint16_le();
                this->info_cache_0_size              = this->stream.in_uint16_le();
                this->info_cache_1_entries           = this->stream.in_uint16_le();
                this->info_cache_1_size              = this->stream.in_uint16_le();
                this->info_cache_2_entries           = this->stream.in_uint16_le();
                this->info_cache_2_size              = this->stream.in_uint16_le();

                if (this->info_version <= 3) {
                    this->info_number_of_cache       = 3;
                    this->info_use_waiting_list      = false;

                    this->info_cache_0_persistent    = false;
                    this->info_cache_1_persistent    = false;
                    this->info_cache_2_persistent    = false;
                }
                else {
                    this->info_number_of_cache       = this->stream.in_uint8();
                    this->info_use_waiting_list      = (this->stream.in_uint8() ? true : false);

                    this->info_cache_0_persistent    = (this->stream.in_uint8() ? true : false);
                    this->info_cache_1_persistent    = (this->stream.in_uint8() ? true : false);
                    this->info_cache_2_persistent    = (this->stream.in_uint8() ? true : false);

                    this->info_cache_3_entries       = this->stream.in_uint16_le();
                    this->info_cache_3_size          = this->stream.in_uint16_le();
                    this->info_cache_3_persistent    = (this->stream.in_uint8() ? true : false);

                    this->info_cache_4_entries       = this->stream.in_uint16_le();
                    this->info_cache_4_size          = this->stream.in_uint16_le();
                    this->info_cache_4_persistent    = (this->stream.in_uint8() ? true : false);

                    this->info_compression_algorithm = static_cast<WrmCompressionAlgorithm>(this->stream.in_uint8());
                    REDASSERT(is_valid_enum_value(this->info_compression_algorithm));
                    if (!is_valid_enum_value(this->info_compression_algorithm)) {
                        this->info_compression_algorithm = WrmCompressionAlgorithm::no_compression;
                    }

                    this->trans = &this->compression_builder.reset(
                        *this->trans_source, this->info_compression_algorithm
                    );
                }

                this->stream.in_skip_bytes(this->stream.in_remain());

                if (!this->meta_ok) {
                    this->bmp_cache = new BmpCache(BmpCache::Recorder, this->info_bpp, this->info_number_of_cache,
                        this->info_use_waiting_list,
                        BmpCache::CacheOption(
                            this->info_cache_0_entries, this->info_cache_0_size, this->info_cache_0_persistent),
                        BmpCache::CacheOption(
                            this->info_cache_1_entries, this->info_cache_1_size, this->info_cache_1_persistent),
                        BmpCache::CacheOption(
                            this->info_cache_2_entries, this->info_cache_2_size, this->info_cache_2_persistent),
                        BmpCache::CacheOption(
                            this->info_cache_3_entries, this->info_cache_3_size, this->info_cache_3_persistent),
                        BmpCache::CacheOption(
                            this->info_cache_4_entries, this->info_cache_4_size, this->info_cache_4_persistent));
                    this->screen_rect = Rect(0, 0, this->info_width, this->info_height);
                    this->meta_ok = true;
                }
                else {
                    if (this->screen_rect.cx != this->info_width ||
                        this->screen_rect.cy != this->info_height) {
                        LOG( LOG_ERR,"Inconsistant redundant meta chunk: (%u x %u) -> (%u x %u)"
                           , this->screen_rect.cx, this->screen_rect.cy, this->info_width, this->info_height);
                        throw Error(ERR_WRM);
                    }
                }

                for (gdi::ExternalCaptureApi * obj : this->external_event_consumers){
                    obj->external_breakpoint();
                }
            }
            break;
            case SAVE_STATE:
            {
                SaveStateChunk ssc;
                ssc.recv(this->stream, this->ssc, this->info_version);
            }
            break;
            case LAST_IMAGE_CHUNK:
            case PARTIAL_IMAGE_CHUNK:
            {
                if (this->graphic_consumers.size()) {

                    InChunkedImageTransport chunk_trans(this->chunk_type, this->chunk_size, this->trans);

                    png_struct * ppng = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
                    png_set_read_fn(ppng, &chunk_trans, &png_read_data_fn);
                    png_info * pinfo = png_create_info_struct(ppng);
                    png_read_info(ppng, pinfo);

                    size_t height = png_get_image_height(ppng, pinfo);
                    const size_t width = screen_rect.cx;
                    // TODO check png row_size is identical to drawable rowsize

                    uint32_t tmp[8192];
                    assert(sizeof(tmp) / sizeof(tmp[0]) >= width);
                    for (size_t k = 0; k < height; ++k) {
                        png_read_row(ppng, reinterpret_cast<uint8_t*>(tmp), nullptr);

                        uint32_t bgrtmp[8192];
                        const uint32_t * s = reinterpret_cast<const uint32_t*>(tmp);
                        uint32_t * t = bgrtmp;
                        for (size_t n = 0; n < (width / 4); n++){
                            unsigned bRGB = *s++;
                            unsigned GBrg = *s++;
                            unsigned rgbR = *s++;
                            *t++ = ((GBrg << 16) & 0xFF000000)
                                 | ((bRGB << 16) & 0x00FF0000)
                                 | (bRGB         & 0x0000FF00)
                                 | ((bRGB >> 16) & 0x000000FF);
                            *t++ = (GBrg         & 0xFF000000)
                                 | ((rgbR << 16) & 0x00FF0000)
                                 | ((bRGB >> 16) & 0x0000FF00)
                                 | ( GBrg        & 0x000000FF);
                            *t++ = ((rgbR << 16) & 0xFF000000)
                                 | (rgbR         & 0x00FF0000)
                                 | ((rgbR >> 16) & 0x0000FF00)
                                 | ((GBrg >> 16) & 0x000000FF);
                        }

                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                            gd->set_row(k, reinterpret_cast<uint8_t*>(bgrtmp));
                        }
                    }
                    png_read_end(ppng, pinfo);
                    png_destroy_read_struct(&ppng, &pinfo, nullptr);
                }
                else {
                    // If no drawable is available ignore images chunks
                    this->stream.rewind();
                    std::size_t sz = this->chunk_size - HEADER_SIZE;
                    auto end = this->stream_buf;
                    this->trans->recv(&end, sz);
                    this->stream = InStream(this->stream_buf, sz, sz);
                }
                this->remaining_order_count = 0;
            }
            break;
            case RDP_UPDATE_BITMAP:
            {
                if (!this->meta_ok) {
                    LOG(LOG_ERR, "Drawing orders chunk must be preceded by a META chunk to get drawing device size");
                    throw Error(ERR_WRM);
                }
                if (!this->timestamp_ok) {
                    LOG(LOG_ERR, "Drawing orders chunk must be preceded by a TIMESTAMP chunk to get drawing timing");
                    throw Error(ERR_WRM);
                }

                this->statistics.BitmapUpdate++;
                RDPBitmapData bitmap_data;
                bitmap_data.receive(this->stream);

                const uint8_t * data = this->stream.in_uint8p(bitmap_data.bitmap_size());

                Bitmap bitmap( this->info_bpp
                             , bitmap_data.bits_per_pixel
                             , /*0*/&this->palette
                             , bitmap_data.width
                             , bitmap_data.height
                             , data
                             , bitmap_data.bitmap_size()
                             , (bitmap_data.flags & BITMAP_COMPRESSION)
                             );

                if (this->verbose & Verbose::rdp_orders){
                    bitmap_data.log(LOG_INFO);
                }

                for (gdi::GraphicApi * gd : this->graphic_consumers){
                    gd->draw(bitmap_data, bitmap);
                }

            }
            break;
            case POINTER:
            {
                uint8_t          cache_idx;

                this->mouse_x = this->stream.in_uint16_le();
                this->mouse_y = this->stream.in_uint16_le();
                cache_idx     = this->stream.in_uint8();

                if (  chunk_size - 8 /*header(8)*/
                    > 5 /*mouse_x(2) + mouse_y(2) + cache_idx(1)*/) {
                    this->statistics.CachePointer++;
                    struct Pointer cursor(Pointer::POINTER_NULL);
                    cursor.width = 32;
                    cursor.height = 32;
                    cursor.bpp = 24;
                    cursor.x = this->stream.in_uint8();
                    cursor.y = this->stream.in_uint8();
                    stream.in_copy_bytes(cursor.data, 32 * 32 * 3);
                    stream.in_copy_bytes(cursor.mask, 128);

                    this->ptr_cache.add_pointer_static(cursor, cache_idx);

                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->set_pointer(cursor);
                    }
                }
                else {
                    this->statistics.PointerIndex++;
                    Pointer & pi = this->ptr_cache.Pointers[cache_idx];
                    Pointer cursor(Pointer::POINTER_NULL);
                    cursor.width = 32;
                    cursor.height = 32;
                    cursor.bpp = 24;
                    cursor.x = pi.x;
                    cursor.y = pi.y;
                    memcpy(cursor.data, pi.data, sizeof(pi.data));
                    memcpy(cursor.mask, pi.mask, sizeof(pi.mask));

                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->set_pointer(cursor);
                    }
                }
            }
            break;
            case RESET_CHUNK:
                this->info_compression_algorithm = WrmCompressionAlgorithm::no_compression;

                this->trans = this->trans_source;
            break;
            case SESSION_UPDATE:
                this->stream.in_timeval_from_uint64le_usec(this->record_now);

                for (gdi::ExternalCaptureApi * obj : this->external_event_consumers){
                    obj->external_time(this->record_now);
                }

                {
                    uint16_t message_length = this->stream.in_uint16_le();

                    const char * message =  ::char_ptr_cast(this->stream.get_current()); // Null-terminator is included.

                    this->stream.in_skip_bytes(message_length);

                    for (gdi::CaptureProbeApi * cap_probe : this->capture_probe_consumers){
                        cap_probe->session_update(this->record_now, {message, message_length});
                    }
                }

                if (!this->timestamp_ok) {
                   if (this->real_time) {
                        this->start_record_now   = this->record_now;
                        this->start_synctime_now = tvtime();
                    }
                    this->timestamp_ok = true;
                }
                else {
                   if (this->real_time) {
                        for (gdi::GraphicApi * gd : this->graphic_consumers){
                            gd->sync();
                        }

                        this->movie_elapsed_qt = difftimeval(this->record_now, this->start_record_now);

                        /*struct timeval now     = tvtime();
                        uint64_t       elapsed = difftimeval(now, this->start_synctime_now);

                        uint64_t movie_elapsed = difftimeval(this->record_now, this->start_record_now);


                        if (elapsed < movie_elapsed) {
                            struct timespec wtime     = {
                                  static_cast<time_t>( (movie_elapsed - elapsed) / 1000000LL)
                                , static_cast<time_t>(((movie_elapsed - elapsed) % 1000000LL) * 1000)
                                };
                            struct timespec wtime_rem = { 0, 0 };*/

                            /*while ((nanosleep(&wtime, &wtime_rem) == -1) && (errno == EINTR)) {
                                wtime = wtime_rem;
                            }
                        }*/
                    }
                }
            break;
            default:
                LOG(LOG_ERR, "unknown chunk type %d", this->chunk_type);
                throw Error(ERR_WRM);
        }
    }


    void process_windowing( InStream & stream, const RDP::AltsecDrawingOrderHeader & header) {
        if (this->verbose & Verbose::probe) {
            LOG(LOG_INFO, "rdp_orders::process_windowing");
        }

        const uint32_t FieldsPresentFlags = [&]{
            InStream stream2(stream.get_current(), stream.in_remain());
            stream2.in_skip_bytes(2);    // OrderSize(2)
            return stream2.in_uint32_le();
        }();

        switch (FieldsPresentFlags & (  RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                                      | RDP::RAIL::WINDOW_ORDER_TYPE_NOTIFY
                                      | RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP)) {
            case RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW:
                this->process_window_information(stream, header, FieldsPresentFlags);
                break;

            case RDP::RAIL::WINDOW_ORDER_TYPE_NOTIFY:
                this->process_notification_icon_information(stream, header, FieldsPresentFlags);
                break;

            case RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP:
                this->process_desktop_information(stream, header, FieldsPresentFlags);
                break;

            default:
                LOG(LOG_INFO,
                    "rdp_orders::process_windowing: "
                        "unsupported Windowing Alternate Secondary Drawing Orders! "
                        "FieldsPresentFlags=0x%08X",
                    FieldsPresentFlags);
                break;
        }
    }

    void process_window_information( InStream & stream, const RDP::AltsecDrawingOrderHeader &
                                   , uint32_t FieldsPresentFlags) {
        if (this->verbose & Verbose::probe) {
            LOG(LOG_INFO, "rdp_orders::process_window_information");
        }

        switch (FieldsPresentFlags & (  RDP::RAIL::WINDOW_ORDER_STATE_NEW
                                      | RDP::RAIL::WINDOW_ORDER_ICON
                                      | RDP::RAIL::WINDOW_ORDER_CACHEDICON
                                      | RDP::RAIL::WINDOW_ORDER_STATE_DELETED))
        {
            case RDP::RAIL::WINDOW_ORDER_ICON: {
                    RDP::RAIL::WindowIcon order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(order);
                    }
                }
                break;

            case RDP::RAIL::WINDOW_ORDER_CACHEDICON: {
                    RDP::RAIL::CachedIcon order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(order);
                    }
                }
                break;

            case RDP::RAIL::WINDOW_ORDER_STATE_DELETED: {
                    RDP::RAIL::DeletedWindow order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(order);
                    }
                }
                break;

            case 0:
            case RDP::RAIL::WINDOW_ORDER_STATE_NEW: {
                    RDP::RAIL::NewOrExistingWindow order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(order);
                    }
                }
                break;
        }
    }

    void process_notification_icon_information( InStream & stream, const RDP::AltsecDrawingOrderHeader &
                                              , uint32_t FieldsPresentFlags) {
        if (this->verbose & Verbose::probe) {
            LOG(LOG_INFO, "rdp_orders::process_notification_icon_information");
        }

        switch (FieldsPresentFlags & (  RDP::RAIL::WINDOW_ORDER_STATE_NEW
                                      | RDP::RAIL::WINDOW_ORDER_STATE_DELETED))
        {
            case RDP::RAIL::WINDOW_ORDER_STATE_DELETED: {
                    RDP::RAIL::DeletedNotificationIcons order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(order);
                    }
                }
                break;

            case 0:
            case RDP::RAIL::WINDOW_ORDER_STATE_NEW: {
                    RDP::RAIL::NewOrExistingNotificationIcons order;
                    order.receive(stream);
                    order.log(LOG_INFO);
                    for (gdi::GraphicApi * gd : this->graphic_consumers){
                        gd->draw(order);
                    }
                }
                break;
        }
    }

    void process_desktop_information( InStream & stream, const RDP::AltsecDrawingOrderHeader &
                                    , uint32_t FieldsPresentFlags) {
        if (this->verbose & Verbose::probe) {
            LOG(LOG_INFO, "rdp_orders::process_desktop_information");
        }

        if (FieldsPresentFlags & RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_NONE) {
            RDP::RAIL::NonMonitoredDesktop order;
            order.receive(stream);
            order.log(LOG_INFO);
            for (gdi::GraphicApi * gd : this->graphic_consumers){
                gd->draw(order);
            }
        }
        else {
            RDP::RAIL::ActivelyMonitoredDesktop order;
            order.receive(stream);
            order.log(LOG_INFO);
            for (gdi::GraphicApi * gd : this->graphic_consumers){
                gd->draw(order);
            }
        }
    }

    void play(bool const & requested_to_stop) {
        this->privplay([](time_t){}, requested_to_stop);
    }

    bool play_qt() {
        return this->privplay_qt([](time_t){});
    }

    template<class CbUpdateProgress>
    void play(CbUpdateProgress update_progess, bool const & requested_to_stop) {
        time_t last_sent_record_now = 0;
        this->privplay([&](time_t record_now) {
            if (last_sent_record_now != record_now) {
                update_progess(record_now);
                last_sent_record_now = record_now;
            }
        }, requested_to_stop);
    }

private:
    template<class CbUpdateProgress>
    void privplay(CbUpdateProgress update_progess, bool const & requested_to_stop) {
        while (!requested_to_stop && this->next_order()) {
            if (this->verbose & Verbose::play) {
                LOG( LOG_INFO, "replay TIMESTAMP (first timestamp) = %u order=%u\n"
                   , unsigned(this->record_now.tv_sec), unsigned(this->total_orders_count));
            }
            this->interpret_order();
            if (  (this->begin_capture.tv_sec == 0) || this->begin_capture <= this->record_now ) {
                for (gdi::CaptureApi * cap : this->capture_consumers){
                    cap->periodic_snapshot(
                        this->record_now, this->mouse_x, this->mouse_y
                      , this->ignore_frame_in_timeval
                    );
                }

                this->ignore_frame_in_timeval = false;

                update_progess(this->record_now.tv_sec);
            }
            if (this->max_order_count && this->max_order_count <= this->total_orders_count) {
                break;
            }
            if (this->end_capture.tv_sec && this->end_capture < this->record_now) {
                break;
            }
        }
    }

    template<class CbUpdateProgress>
    bool privplay_qt(CbUpdateProgress update_progess) {

        struct timeval now     = tvtime();
        uint64_t       elapsed = difftimeval(now, this->start_synctime_now);

        bool res(false);

        if (elapsed >= this->movie_elapsed_qt) {
            if (this->next_order()) {
                if (this->verbose & Verbose::play) {
                    LOG( LOG_INFO, "replay TIMESTAMP (first timestamp) = %u order=%u\n"
                    , unsigned(this->record_now.tv_sec), unsigned(this->total_orders_count));
                }

                if (this->remaining_order_count > 0) {
                    res = true;
                }

                this->interpret_order();

                if (  (this->begin_capture.tv_sec == 0) || this->begin_capture <= this->record_now ) {
                    for (gdi::CaptureApi * cap : this->capture_consumers){
                        cap->periodic_snapshot(
                            this->record_now, this->mouse_x, this->mouse_y
                        , this->ignore_frame_in_timeval
                        );
                    }

                    this->ignore_frame_in_timeval = false;

                    update_progess(this->record_now.tv_sec);
                }
                if (this->max_order_count && this->max_order_count <= this->total_orders_count) {
                    break_privplay_qt = true;
                }
                if (this->end_capture.tv_sec && this->end_capture < this->record_now) {
                    break_privplay_qt = true;
                }
            } else {
                break_privplay_qt = true;
            }
        }

        return res;
    }
};


class PatternSearcher
{
    struct TextSearcher
    {
        re::Regex::PartOfText searcher;
        re::Regex::range_matches matches;

        void reset(re::Regex & rgx) {
            this->searcher = rgx.part_of_text_search(false);
        }

        bool next(uint8_t const * uchar) {
            return re::Regex::match_success == this->searcher.next(char_ptr_cast(uchar));
        }

        re::Regex::range_matches const & match_result(re::Regex & rgx) {
            this->matches.clear();
            return rgx.match_result(this->matches, false);
        }
    };

    class Utf8KbdData
    {
        static constexpr const size_t buf_len = 128;

        uint8_t kbd_data[buf_len] = { 0 };
        uint8_t * p = kbd_data;
        uint8_t * beg = p;

        uint8_t * data_begin() {
            using std::begin;
            return begin(this->kbd_data);
        }
        uint8_t * data_end() {
            using std::end;
            return end(this->kbd_data);
        }

    public:
        uint8_t const * get_data() const {
            return this->beg;
        }

        void reset() {
            this->p = this->kbd_data;
            this->beg = this->p;
        }

        void push_utf8_char(uint8_t const * c, size_t char_len) {
            assert(c && char_len <= 4);

            if (static_cast<size_t>(this->data_end() - this->beg) < char_len + 1u) {
                std::size_t pchar_len = 0;
                do {
                    size_t const len = get_utf8_char_size(this->beg);
                    size_t const tailroom = this->data_end() - this->beg;
                    if (tailroom < len) {
                        this->beg = this->data_begin() + (len - tailroom);
                    }
                    else {
                        this->beg += len;
                    }
                    pchar_len += len;
                } while (pchar_len < char_len + 1);
            }

            auto ec = c + char_len;
            for (; c != ec; ++c) {
                *this->p = *c;
                ++this->p;
                if (this->p == this->data_end()) {
                    this->p = this->data_begin();
                }
            }
            *this->p = 0;
        }

        void linearize() {
            if (!this->is_linearized()) {
                std::rotate(this->data_begin(), this->beg, this->data_end());
                auto const diff = this->beg - this->p;
                this->p = this->data_end() - diff;
                this->beg = this->data_begin();
            }
        }

        bool is_linearized() const {
            return this->beg <= this->p;
        }
    };

    utils::MatchFinder::NamedRegexArray regexes_filter;
    std::unique_ptr<TextSearcher[]> regexes_searcher;
    Utf8KbdData utf8_kbd_data;

public:
    PatternSearcher(utils::MatchFinder::ConfigureRegexes conf_regex, char const * filters, int verbose = 0) {
        utils::MatchFinder::configure_regexes(conf_regex, filters, this->regexes_filter, verbose, true);
        auto const count_regex = this->regexes_filter.size();
        if (count_regex) {
            this->regexes_searcher = std::make_unique<TextSearcher[]>(count_regex);
            auto searcher_it = this->regexes_searcher.get();
            for (auto & named_regex : this->regexes_filter) {
                searcher_it->reset(named_regex.regex);
                ++searcher_it;
            }
        }
    }

    void rewind_search() {
        TextSearcher * test_searcher_it = this->regexes_searcher.get();
        for (utils::MatchFinder::NamedRegex & named_regex : this->regexes_filter) {
            test_searcher_it->reset(named_regex.regex);
            ++test_searcher_it;
        }
    }

    template<class Report>
    bool test_uchar(uint8_t const * const utf8_char, size_t const char_len, Report report)
    {
        if (char_len == 0) {
            return false;
        }

        bool has_notify = false;

        utf8_kbd_data.push_utf8_char(utf8_char, char_len);
        TextSearcher * test_searcher_it = this->regexes_searcher.get();

        for (utils::MatchFinder::NamedRegex & named_regex : this->regexes_filter) {
            if (test_searcher_it->next(utf8_char)) {
                utf8_kbd_data.linearize();
                char const * char_kbd_data = ::char_ptr_cast(utf8_kbd_data.get_data());
                test_searcher_it->reset(named_regex.regex);

                if (named_regex.regex.search_with_matches(char_kbd_data)) {
                    auto & match_result = test_searcher_it->match_result(named_regex.regex);
                    auto str = (!match_result.empty() && match_result[0].first)
                        ? match_result[0].first
                        : char_kbd_data;
                    report(named_regex.name.c_str(), str);
                    has_notify = true;
                }
            }

            ++test_searcher_it;
        }
        if (has_notify) {
            utf8_kbd_data.reset();
        }

        return has_notify;
    }

    template<class Report>
    bool test_uchar(uint32_t uchar, Report report)
    {
        uint8_t utf8_char[5];
        size_t const char_len = UTF32toUTF8(uchar, utf8_char, 4u);
        return this->test_uchar(utf8_char, char_len, report);
    }

    bool is_empty() const {
        return this->regexes_filter.empty();
    }
};


template<class Utf8CharFn, class NoPrintableFn>
void filtering_kbd_input(uint32_t uchar, Utf8CharFn utf32_char_fn, NoPrintableFn no_printable_fn)
{
    constexpr struct {
        uint32_t uchar;
        array_view_const_char str;
        // for std::sort and std::lower_bound
        operator uint32_t () const { return this->uchar; }
    } noprintable_table[] = {
        {0x00000008, cstr_array_view("/<backspace>")},
        {0x00000009, cstr_array_view("/<tab>")},
        {0x0000000D, cstr_array_view("/<enter>")},
        {0x0000001B, cstr_array_view("/<escape>")},
        {0x0000007F, cstr_array_view("/<delete>")},
        {0x00002190, cstr_array_view("/<left>")},
        {0x00002191, cstr_array_view("/<up>")},
        {0x00002192, cstr_array_view("/<right>")},
        {0x00002193, cstr_array_view("/<down>")},
        {0x00002196, cstr_array_view("/<home>")},
        {0x00002198, cstr_array_view("/<end>")},
    };
    using std::begin;
    using std::end;
    // TODO used static_assert
    assert(std::is_sorted(begin(noprintable_table), end(noprintable_table)));

    auto p = std::lower_bound(begin(noprintable_table), end(noprintable_table), uchar);
    if (p != end(noprintable_table) && *p == uchar) {
        no_printable_fn(p->str);
    }
    else {
        utf32_char_fn(uchar);
    }
}


class PatternKbd : public gdi::KbdInputApi
{
    auth_api * authentifier;
    PatternSearcher pattern_kill;
    PatternSearcher pattern_notify;

public:
    PatternKbd(
        auth_api * authentifier,
        char const * str_pattern_kill, char const * str_pattern_notify,
        int verbose = 0)
    : authentifier(authentifier)
    , pattern_kill(utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                   str_pattern_kill && authentifier ? str_pattern_kill : nullptr, verbose)
    , pattern_notify(utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                     str_pattern_notify && authentifier ? str_pattern_notify : nullptr, verbose)
    {}

    bool contains_pattern() const {
        return !this->pattern_kill.is_empty() || !this->pattern_notify.is_empty();
    }

    bool kbd_input(const timeval& /*now*/, uint32_t uchar) override {
        bool can_be_sent_to_server = true;

        filtering_kbd_input(
            uchar,
            [this, &can_be_sent_to_server](uint32_t uchar) {
                uint8_t buf_char[5];
                size_t const char_len = UTF32toUTF8(uchar, buf_char, sizeof(buf_char));

                if (char_len > 0) {
                    buf_char[char_len] = '\0';
                    if (!this->pattern_kill.is_empty()) {
                        can_be_sent_to_server &= !this->test_pattern(
                            buf_char, char_len, this->pattern_kill, true
                        );
                    }
                    if (!this->pattern_notify.is_empty()) {
                        this->test_pattern(
                            buf_char, char_len, this->pattern_notify, false
                        );
                    }
                }
            },
            [this](array_view_const_char const &) {
                this->pattern_kill.rewind_search();
                this->pattern_notify.rewind_search();
            }
        );

        return can_be_sent_to_server;
    }

    void enable_kbd_input_mask(bool /*enable*/) override {
    }

private:
    bool test_pattern(
        uint8_t const * uchar, size_t char_len,
        PatternSearcher & searcher, bool is_pattern_kill
    ) {
        return searcher.test_uchar(
            uchar, char_len,
            [&, this](std::string const & pattern, char const * str) {
                assert(this->authentifier);
                utils::MatchFinder::report(
                    *this->authentifier,
                    is_pattern_kill,
                    utils::MatchFinder::ConfigureRegexes::KBD_INPUT,
                    pattern.c_str(),
                    str
                );
            }
        );
    }
};




class SyslogKbd : public gdi::KbdInputApi, public gdi::CaptureApi
{
    uint8_t kbd_buffer[1024];
    OutStream kbd_stream;
    bool keyboard_input_mask_enabled = false;
    timeval last_snapshot;

private:
    void write_shadow_keys() {
        if (!this->kbd_stream.has_room(1)) {
            this->flush();
        }
        this->kbd_stream.out_uint8('*');
    }

    void write_keys(uint32_t uchar) {
        filtering_kbd_input(
            uchar,
            [this](uint32_t uchar) {
                uint8_t buf_char[5];
                if (uchar == '/') {
                    this->copy_bytes({"//", 2});
                }
                else if (size_t const char_len = UTF32toUTF8(uchar, buf_char, sizeof(buf_char))) {
                    this->copy_bytes({buf_char, char_len});
                }
            },
            [this](array_view_const_char no_printable_str) {
                this->copy_bytes(no_printable_str);
            }
        );
    }

    void copy_bytes(const_bytes_array bytes) {
        if (this->kbd_stream.tailroom() < bytes.size()) {
            this->flush();
        }
        this->kbd_stream.out_copy_bytes(bytes.data(), std::min(this->kbd_stream.tailroom(), bytes.size()));
    }

public:
    explicit SyslogKbd(timeval const & now)
    : kbd_stream(this->kbd_buffer)
    , last_snapshot(now)
    {}

    ~SyslogKbd() {
        this->flush();
    }

    void enable_kbd_input_mask(bool enable) override {
        if (this->keyboard_input_mask_enabled != enable) {
            this->flush();
            this->keyboard_input_mask_enabled = enable;
        }
    }

    bool kbd_input(const timeval& /*now*/, uint32_t keys) override {
        if (this->keyboard_input_mask_enabled) {
            this->write_shadow_keys();
        }
        else {
            this->write_keys(keys);
        }
        return true;
    }

    void flush() {
        if (this->kbd_stream.get_offset()) {
            LOG(LOG_INFO, R"x(type="KBD input" data="%*s")x",
                int(this->kbd_stream.get_offset()),
                reinterpret_cast<char const *>(this->kbd_stream.get_data()));
            this->kbd_stream.rewind();
        }
    }

private:
    std::chrono::microseconds do_snapshot(
        const timeval& now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval
    ) override {
        (void)cursor_x;
        (void)cursor_y;
        (void)ignore_frame_in_timeval;
        std::chrono::microseconds const time_to_wait = std::chrono::seconds{2};
        std::chrono::microseconds const diff {difftimeval(now, this->last_snapshot)};

        if (diff < time_to_wait && this->kbd_stream.get_offset() < 8 * sizeof(uint32_t)) {
            return time_to_wait;
        }

        this->flush();
        this->last_snapshot = now;

        return time_to_wait;
    }
};


namespace {
    constexpr array_view_const_char session_log_prefix() { return cstr_array_view("data='"); }
    constexpr array_view_const_char session_log_suffix() { return cstr_array_view("'"); }
}


class SessionLogKbd : public gdi::KbdInputApi, public gdi::CaptureProbeApi
{
    OutStream kbd_stream;
    bool keyboard_input_mask_enabled = false;
    static const std::size_t buffer_size = 64;
    uint8_t buffer[buffer_size + session_log_prefix().size() + session_log_suffix().size() + 1];
    bool is_probe_enabled_session = false;
    auth_api & authentifier;

    void copy_bytes(const_bytes_array bytes) {
        if (this->kbd_stream.tailroom() < bytes.size()) {
            this->flush();
        }
        this->kbd_stream.out_copy_bytes(bytes.data(), std::min(this->kbd_stream.tailroom(), bytes.size()));
    }

    void write_shadow_keys() {
        if (!this->kbd_stream.has_room(1)) {
            this->flush();
        }
        this->kbd_stream.out_uint8('*');
    }

    void write_keys(uint32_t uchar) {
        filtering_kbd_input(
            uchar,
            [this](uint32_t uchar) {
                uint8_t buf_char[5];
                if (uchar == '/') {
                    this->copy_bytes({"//", 2});
                }
                else if (size_t const char_len = UTF32toUTF8(uchar, buf_char, sizeof(buf_char))) {
                    this->copy_bytes({buf_char, char_len});
                }
            },
            [this](array_view_const_char no_printable_str) {
                this->copy_bytes(no_printable_str);
            }
        );
    }

public:
    explicit SessionLogKbd(auth_api & authentifier)
    : kbd_stream{this->buffer + session_log_prefix().size(), buffer_size}
    , authentifier(authentifier)
    {
        memcpy(this->buffer, session_log_prefix().data(), session_log_prefix().size());
    }

    ~SessionLogKbd() {
        this->flush();
    }

    bool kbd_input(const timeval& /*now*/, uint32_t uchar) override {
        if (this->keyboard_input_mask_enabled) {
            if (this->is_probe_enabled_session) {
                this->write_shadow_keys();
            }
        }
        else {
            this->write_keys(uchar);
        }
        return true;
    }

    void enable_kbd_input_mask(bool enable) override {
        if (this->keyboard_input_mask_enabled != enable) {
            this->flush();
            this->keyboard_input_mask_enabled = enable;
        }
    }

    void flush() {
        if (this->kbd_stream.get_offset()) {
            memcpy(this->kbd_stream.get_current(), session_log_suffix().data(), session_log_suffix().size() + 1);
            this->authentifier.log4(false, "KBD_INPUT", reinterpret_cast<char const *>(this->buffer));
            this->kbd_stream.rewind();
        }
    }

    void session_update(const timeval& /*now*/, array_view_const_char message) override {
        this->is_probe_enabled_session = (::strcmp(message.data(), "Probe.Status=Unknown") != 0);
        this->flush();
    }

    void possible_active_window_change() override {
        this->flush();
    }
};


using std::begin;
using std::end;



class FileToChunk
{
    unsigned char stream_buf[65536];
    InStream stream;

    CompressionInTransportBuilder compression_builder;

    Transport * trans_source;
    Transport * trans;

    // variables used to read batch of orders "chunks"
    uint32_t chunk_size;
    uint16_t chunk_type;
    uint16_t chunk_count;

    uint16_t nbconsumers;

    RDPChunkedDevice * consumers[10];

public:
    timeval record_now;

    bool meta_ok;

    uint16_t info_version;
    uint16_t info_width;
    uint16_t info_height;
    uint16_t info_bpp;
    uint16_t info_number_of_cache;
    bool     info_use_waiting_list;
    uint16_t info_cache_0_entries;
    uint16_t info_cache_0_size;
    bool     info_cache_0_persistent;
    uint16_t info_cache_1_entries;
    uint16_t info_cache_1_size;
    bool     info_cache_1_persistent;
    uint16_t info_cache_2_entries;
    uint16_t info_cache_2_size;
    bool     info_cache_2_persistent;
    uint16_t info_cache_3_entries;
    uint16_t info_cache_3_size;
    bool     info_cache_3_persistent;
    uint16_t info_cache_4_entries;
    uint16_t info_cache_4_size;
    bool     info_cache_4_persistent;
    WrmCompressionAlgorithm info_compression_algorithm;

    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        end_of_transport = 1,
    };

    FileToChunk(Transport * trans, Verbose verbose)
        : stream(this->stream_buf)
        , compression_builder(*trans, WrmCompressionAlgorithm::no_compression)
        , trans_source(trans)
        , trans(trans)
        // variables used to read batch of orders "chunks"
        , chunk_size(0)
        , chunk_type(0)
        , chunk_count(0)
        , nbconsumers(0)
        , consumers()
        , meta_ok(false)
        , info_version(0)
        , info_width(0)
        , info_height(0)
        , info_bpp(0)
        , info_number_of_cache(0)
        , info_use_waiting_list(true)
        , info_cache_0_entries(0)
        , info_cache_0_size(0)
        , info_cache_0_persistent(false)
        , info_cache_1_entries(0)
        , info_cache_1_size(0)
        , info_cache_1_persistent(false)
        , info_cache_2_entries(0)
        , info_cache_2_size(0)
        , info_cache_2_persistent(false)
        , info_cache_3_entries(0)
        , info_cache_3_size(0)
        , info_cache_3_persistent(false)
        , info_cache_4_entries(0)
        , info_cache_4_size(0)
        , info_cache_4_persistent(false)
        , info_compression_algorithm(WrmCompressionAlgorithm::no_compression)
        , verbose(verbose)
    {
        while (this->next_chunk()) {
            this->interpret_chunk();
            if (this->meta_ok) {
                break;
            }
        }
    }

    void add_consumer(RDPChunkedDevice * chunk_device) {
        REDASSERT(nbconsumers < (sizeof(consumers) / sizeof(consumers[0]) - 1));
        this->consumers[this->nbconsumers++] = chunk_device;
    }

    bool next_chunk() {
        try {
            {
                auto const buf_sz = FileToGraphic::HEADER_SIZE;
                unsigned char buf[buf_sz];
                auto * p = buf;
                this->trans->recv(&p, buf_sz);
                InStream header(buf);
                this->chunk_type  = header.in_uint16_le();
                this->chunk_size  = header.in_uint32_le();
                this->chunk_count = header.in_uint16_le();
            }

            if (this->chunk_size > 65536) {
                LOG(LOG_INFO,"chunk_size (%d) > 65536", this->chunk_size);
                return false;
            }
            this->stream = InStream(this->stream_buf, 0);   // empty stream
            if (this->chunk_size - FileToGraphic::HEADER_SIZE > 0) {
                auto * p = this->stream_buf;
                this->trans->recv(&p, this->chunk_size - FileToGraphic::HEADER_SIZE);
                this->stream = InStream(this->stream_buf, p - this->stream_buf);
            }
        }
        catch (Error const & e) {
            if (e.id == ERR_TRANSPORT_OPEN_FAILED) {
                throw;
            }

            if (this->verbose) {
                LOG(LOG_INFO, "receive error %u : end of transport", e.id);
            }
            // receive error, end of transport
            return false;
        }

        return true;
    }

    void interpret_chunk() {
        switch (this->chunk_type) {
        case META_FILE:
            this->info_version                   = this->stream.in_uint16_le();
            this->info_width                     = this->stream.in_uint16_le();
            this->info_height                    = this->stream.in_uint16_le();
            this->info_bpp                       = this->stream.in_uint16_le();
            this->info_cache_0_entries           = this->stream.in_uint16_le();
            this->info_cache_0_size              = this->stream.in_uint16_le();
            this->info_cache_1_entries           = this->stream.in_uint16_le();
            this->info_cache_1_size              = this->stream.in_uint16_le();
            this->info_cache_2_entries           = this->stream.in_uint16_le();
            this->info_cache_2_size              = this->stream.in_uint16_le();

            if (this->info_version <= 3) {
                this->info_number_of_cache       = 3;
                this->info_use_waiting_list      = false;

                this->info_cache_0_persistent    = false;
                this->info_cache_1_persistent    = false;
                this->info_cache_2_persistent    = false;
            }
            else {
                this->info_number_of_cache       = this->stream.in_uint8();
                this->info_use_waiting_list      = (this->stream.in_uint8() ? true : false);

                this->info_cache_0_persistent    = (this->stream.in_uint8() ? true : false);
                this->info_cache_1_persistent    = (this->stream.in_uint8() ? true : false);
                this->info_cache_2_persistent    = (this->stream.in_uint8() ? true : false);

                this->info_cache_3_entries       = this->stream.in_uint16_le();
                this->info_cache_3_size          = this->stream.in_uint16_le();
                this->info_cache_3_persistent    = (this->stream.in_uint8() ? true : false);

                this->info_cache_4_entries       = this->stream.in_uint16_le();
                this->info_cache_4_size          = this->stream.in_uint16_le();
                this->info_cache_4_persistent    = (this->stream.in_uint8() ? true : false);

                this->info_compression_algorithm = static_cast<WrmCompressionAlgorithm>(this->stream.in_uint8());
                REDASSERT(is_valid_enum_value(this->info_compression_algorithm));
                if (!is_valid_enum_value(this->info_compression_algorithm)) {
                    this->info_compression_algorithm = WrmCompressionAlgorithm::no_compression;
                }

                // re-init
                this->trans = &this->compression_builder.reset(
                    *this->trans_source, this->info_compression_algorithm
                );
            }

            this->stream.rewind();

            if (!this->meta_ok) {
                this->meta_ok = true;
            }
            break;
        case RESET_CHUNK:
            this->info_compression_algorithm = WrmCompressionAlgorithm::no_compression;

            this->trans = this->trans_source;
            break;
        }

        for (size_t i = 0; i < this->nbconsumers ; i++) {
            if (this->consumers[i]) {
                this->consumers[i]->chunk(this->chunk_type, this->chunk_count, this->stream.clone());
            }
        }
    }   // void interpret_chunk()

    void play(bool const & requested_to_stop) {
        while (!requested_to_stop && this->next_chunk()) {
            this->interpret_chunk();
        }
    }
};


class PatternsChecker : noncopyable
{
    utils::MatchFinder::NamedRegexArray regexes_filter_kill;
    utils::MatchFinder::NamedRegexArray regexes_filter_notify;
    auth_api & authentifier;

public:
    PatternsChecker(
        auth_api & authentifier,
        const char * const filters_kill,
        const char * const filters_notify,
        int verbose = 0
    )
    : authentifier(authentifier)
    {
        utils::MatchFinder::configure_regexes(utils::MatchFinder::ConfigureRegexes::OCR,
            filters_kill, this->regexes_filter_kill, verbose);

        utils::MatchFinder::configure_regexes(utils::MatchFinder::ConfigureRegexes::OCR,
            filters_notify, this->regexes_filter_notify, verbose);
    }

    bool contains_pattern() const {
        return !this->regexes_filter_kill.empty() || !this->regexes_filter_notify.empty();
    }

    void operator()(array_view_const_char str) {
        assert(str.data() && str.size());
        this->check_filter(this->regexes_filter_kill, str.data());
        this->check_filter(this->regexes_filter_notify, str.data());
    }

private:
    void check_filter(utils::MatchFinder::NamedRegexArray & regexes_filter, char const * str) {
        if (regexes_filter.begin()) {
            utils::MatchFinder::NamedRegexArray::iterator first = regexes_filter.begin();
            utils::MatchFinder::NamedRegexArray::iterator last = regexes_filter.end();
            for (; first != last; ++first) {
                if (first->regex.search(str)) {
                    utils::MatchFinder::report(this->authentifier,
                        &regexes_filter == &this->regexes_filter_kill, // pattern_kill = FINDPATTERN_KILL
                        utils::MatchFinder::ConfigureRegexes::OCR,
                        first->name.c_str(), str);
                }
            }
        }
    }
};


namespace gdi {
    class GraphicApi;
    class CaptureApi;
    class CaptureProbeApi;
    class KbdInputApi;
    class ExternalCaptureApi;
}



struct MouseTrace
{
    timeval last_now;
    int     last_x;
    int     last_y;
};


class PngCapture : public gdi::CaptureApi
{
public:
    OutFilenameSequenceTransport trans;
    RDPDrawable & drawable;
    timeval start_capture;
    std::chrono::microseconds frame_interval;

    unsigned zoom_factor;
    unsigned scaled_width;
    unsigned scaled_height;

    std::unique_ptr<uint8_t[]> scaled_buffer;


    PngCapture(const timeval & now, RDPDrawable & drawable, const PngParams & png_params)
    : trans(FilenameGenerator::PATH_FILE_COUNT_EXTENSION, png_params.record_tmp_path, png_params.basename, ".png", png_params.groupid, png_params.authentifier)
    , drawable(drawable)
    , start_capture(now)
    , frame_interval(png_params.png_interval)
    , zoom_factor(png_params.zoom)
    , scaled_width{(((this->drawable.width() * this->zoom_factor) / 100)+3) & 0xFFC}
    , scaled_height{((this->drawable.height() * this->zoom_factor) / 100)}
    {
        if (this->zoom_factor != 100) {
            this->scaled_buffer.reset(new uint8_t[this->scaled_width * this->scaled_height * 3]);
        }
    }

    void dump(void)
    {
        if (this->zoom_factor == 100) {
            ::transport_dump_png24(
                this->trans, this->drawable.data(),
                this->drawable.width(), this->drawable.height(),
                this->drawable.rowsize(), true);
        }
        else {
            scale_data(
                this->scaled_buffer.get(), this->drawable.data(),
                this->scaled_width, this->drawable.width(),
                this->scaled_height, this->drawable.height(),
                this->drawable.rowsize());
            ::transport_dump_png24(
                this->trans, this->scaled_buffer.get(),
                this->scaled_width, this->scaled_height,
                this->scaled_width * 3, false);
        }
    }

     virtual void clear_old() {}

     void clear_png_interval(uint32_t num_start, uint32_t num_end){
        for(uint32_t num = num_start ; num < num_end ; num++) {
            // unlink may fail, for instance if file does not exist, just don't care
            ::unlink(this->trans.seqgen()->get(num));
        }
     }

    std::chrono::microseconds do_snapshot(
        timeval const & now, int x, int y, bool ignore_frame_in_timeval
    ) override {
        (void)x;
        (void)y;
        (void)ignore_frame_in_timeval;
        uint64_t const duration = difftimeval(now, this->start_capture);
        uint64_t const interval = this->frame_interval.count();
        if (duration >= interval) {
             // Snapshot at end of Frame or force snapshot if diff_time_val >= 1.5 x frame_interval.
            if (this->drawable.logical_frame_ended() || (duration >= interval * 3 / 2)) {
                this->drawable.trace_mouse();
                tm ptm;
                localtime_r(&now.tv_sec, &ptm);
                this->drawable.trace_timestamp(ptm);

                this->dump();
                this->clear_old();
                this->trans.next();

                this->drawable.clear_timestamp();
                this->start_capture = now;
                this->drawable.clear_mouse();

                return std::chrono::microseconds(interval ? interval - duration % interval : 0u);
            }
            else {
                // Wait 0.3 x frame_interval.
                return this->frame_interval / 3;
            }
        }
        return std::chrono::microseconds(interval - duration);
    }
};

class PngCaptureRT : public PngCapture
{
public:
    uint32_t num_start;
    unsigned png_limit;

    bool enable_rt_display = false;

    PngCaptureRT(
        const timeval & now, RDPDrawable & drawable, const PngParams & png_params)
    : PngCapture(now, drawable, png_params)
    , num_start(this->trans.get_seqno())
    , png_limit(png_params.png_limit)
    {
    }

    ~PngCaptureRT(){
        this->clear_png_interval(this->num_start, this->trans.get_seqno() + 1);
    }

    void update_config(bool enable_rt_display) {
        if (enable_rt_display != this->enable_rt_display){
            this->enable_rt_display = enable_rt_display;
            // clear files if we go from RT to non-RT
            if (!this->enable_rt_display) {
                this->clear_png_interval(this->num_start, this->trans.get_seqno() + 1);
            }
        }
    }

     virtual void clear_old() {
        if (this->trans.get_seqno() < this->png_limit) {
            return;
        }
        uint32_t num_start = this->trans.get_seqno() - this->png_limit;
        this->clear_png_interval(num_start, num_start + 1);
    }
};

class VideoCapture
{
    Transport & trans;

    FlvParams flv_params;

    RDPDrawable & drawable;
    std::unique_ptr<video_recorder> recorder;

    timeval start_video_capture;
    std::chrono::microseconds inter_frame_interval;
    bool no_timestamp;

public:
    VideoCapture(
        const timeval & now,
        Transport & trans,
        RDPDrawable & drawable,
        bool no_timestamp,
        FlvParams flv_params)
    : trans(trans)
    , flv_params(std::move(flv_params))
    , drawable(drawable)
    , start_video_capture(now)
    , inter_frame_interval(1000000L / this->flv_params.frame_rate)
    , no_timestamp(no_timestamp)
    {
        if (this->flv_params.verbosity) {
            LOG(LOG_INFO, "Video recording %d x %d, rate: %d, qscale: %d, brate: %d, codec: %s",
                this->flv_params.target_width, this->flv_params.target_height,
                this->flv_params.frame_rate, this->flv_params.qscale, this->flv_params.bitrate,
                this->flv_params.codec.c_str());
        }

        this->next_video();
    }

    void next_video() {
        if (this->recorder) {
            this->recorder.reset();
            this->trans.next();
        }

        io_video_recorder_with_transport io{this->trans};
        this->recorder.reset(new video_recorder(
            io.write_fn(), io.seek_fn(), io.params(),
            drawable.width(), drawable.height(),
            drawable.pix_len(),
            drawable.data(),
            this->flv_params.bitrate,
            this->flv_params.frame_rate,
            this->flv_params.qscale,
            this->flv_params.codec.c_str(),
            this->flv_params.target_width,
            this->flv_params.target_height,
            this->flv_params.verbosity
        ));
    }

    void preparing_video_frame() {
        this->drawable.trace_mouse();
        if (!this->no_timestamp) {
            time_t rawtime = this->start_video_capture.tv_sec;
            tm tm_result;
            localtime_r(&rawtime, &tm_result);
            this->drawable.trace_timestamp(tm_result);
        }
        this->recorder->preparing_video_frame(true);
        if (!this->no_timestamp) { this->drawable.clear_timestamp(); }
        this->drawable.clear_mouse();
    }

    void encoding_video_frame() {
        this->recorder->encoding_video_frame();
    }

    std::chrono::microseconds frame_marker_event(
        const timeval& /*now*/, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) {
        this->preparing_video_frame();
        return std::chrono::microseconds{};
    }

    std::chrono::microseconds do_snapshot(
        const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool ignore_frame_in_timeval
    ) {
        uint64_t tick = difftimeval(now, this->start_video_capture);
        uint64_t const inter_frame_interval = this->inter_frame_interval.count();
        if (tick >= inter_frame_interval) {
            auto encoding_video_frame = [this](time_t rawtime){
                this->drawable.trace_mouse();
                if (!this->no_timestamp) {
                    tm tm_result;
                    localtime_r(&rawtime, &tm_result);
                    this->drawable.trace_timestamp(tm_result);
                    this->recorder->encoding_video_frame();
                    this->drawable.clear_timestamp();
                }
                else {
                    this->recorder->encoding_video_frame();
                }
                this->drawable.clear_mouse();
            };

            if (ignore_frame_in_timeval) {
                auto const nframe = tick / inter_frame_interval;
                encoding_video_frame(this->start_video_capture.tv_sec);
                auto const usec = inter_frame_interval * nframe;
                auto sec = usec / 1000000LL;
                this->start_video_capture.tv_usec += usec - sec * inter_frame_interval;
                if (this->start_video_capture.tv_usec >= 1000000LL){
                    this->start_video_capture.tv_usec -= 1000000LL;
                    ++sec;
                }
                this->start_video_capture.tv_sec += sec;
                tick -= inter_frame_interval * nframe;
            }
            else {
                do {
                    encoding_video_frame(this->start_video_capture.tv_sec);
                    this->start_video_capture.tv_usec += inter_frame_interval;
                    if (this->start_video_capture.tv_usec >= 1000000LL){
                        this->start_video_capture.tv_sec += 1;
                        this->start_video_capture.tv_usec -= 1000000LL;
                    }
                    tick -= inter_frame_interval;
                } while (tick >= inter_frame_interval);
            }
        }

        return std::chrono::microseconds(inter_frame_interval - tick);
    }

    std::chrono::microseconds periodic_snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) {
        // assert(now >= previous);
        auto next_duration = this->do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        assert(next_duration.count() >= 0);
        return next_duration;
    }

};

class FullVideoCaptureImpl : public gdi::CaptureApi
{
    OutFilenameSequenceSeekableTransport trans;
public:

    std::chrono::microseconds frame_marker_event(const timeval& now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval)
    override {
        return this->vc.frame_marker_event(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }

    std::chrono::microseconds do_snapshot(
        const timeval& now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval)
    override {
        return this->vc.do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }

    std::chrono::microseconds periodic_snapshot(const timeval& now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval)
    override {
        return this->vc.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }

    struct VideoCapture
    {
        Transport & trans;

        FlvParams flv_params;

        RDPDrawable & drawable;
        std::unique_ptr<video_recorder> recorder;

        timeval start_video_capture;
        std::chrono::microseconds inter_frame_interval;
        bool no_timestamp;

    public:
        VideoCapture(
            const timeval & now,
            Transport & trans,
            RDPDrawable & drawable,
            bool no_timestamp,
            FlvParams flv_params)
        : trans(trans)
        , flv_params(std::move(flv_params))
        , drawable(drawable)
        , start_video_capture(now)
        , inter_frame_interval(1000000L / this->flv_params.frame_rate)
        , no_timestamp(no_timestamp)
        {
            if (this->flv_params.verbosity) {
                LOG(LOG_INFO, "Video recording %d x %d, rate: %d, qscale: %d, brate: %d, codec: %s",
                    this->flv_params.target_width, this->flv_params.target_height,
                    this->flv_params.frame_rate, this->flv_params.qscale, this->flv_params.bitrate,
                    this->flv_params.codec.c_str());
            }

            this->next_video();
        }

        void next_video() {
            if (this->recorder) {
                this->recorder.reset();
                this->trans.next();
            }

            io_video_recorder_with_transport io{this->trans};
            this->recorder.reset(new video_recorder(
                io.write_fn(), io.seek_fn(), io.params(),
                drawable.width(), drawable.height(),
                drawable.pix_len(),
                drawable.data(),
                this->flv_params.bitrate,
                this->flv_params.frame_rate,
                this->flv_params.qscale,
                this->flv_params.codec.c_str(),
                this->flv_params.target_width,
                this->flv_params.target_height,
                this->flv_params.verbosity
            ));
        }

        void preparing_video_frame() {
            this->drawable.trace_mouse();
            if (!this->no_timestamp) {
                time_t rawtime = this->start_video_capture.tv_sec;
                tm tm_result;
                localtime_r(&rawtime, &tm_result);
                this->drawable.trace_timestamp(tm_result);
            }
            this->recorder->preparing_video_frame(true);
            if (!this->no_timestamp) { this->drawable.clear_timestamp(); }
            this->drawable.clear_mouse();
        }

        void encoding_video_frame() {
            this->recorder->encoding_video_frame();
        }

        std::chrono::microseconds frame_marker_event(
            const timeval& /*now*/, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
        ) {
            this->preparing_video_frame();
            return std::chrono::microseconds{};
        }

        std::chrono::microseconds do_snapshot(
            const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool ignore_frame_in_timeval
        ) {
            uint64_t tick = difftimeval(now, this->start_video_capture);
            uint64_t const inter_frame_interval = this->inter_frame_interval.count();
            if (tick >= inter_frame_interval) {
                auto encoding_video_frame = [this](time_t rawtime){
                    this->drawable.trace_mouse();
                    if (!this->no_timestamp) {
                        tm tm_result;
                        localtime_r(&rawtime, &tm_result);
                        this->drawable.trace_timestamp(tm_result);
                        this->recorder->encoding_video_frame();
                        this->drawable.clear_timestamp();
                    }
                    else {
                        this->recorder->encoding_video_frame();
                    }
                    this->drawable.clear_mouse();
                };

                if (ignore_frame_in_timeval) {
                    auto const nframe = tick / inter_frame_interval;
                    encoding_video_frame(this->start_video_capture.tv_sec);
                    auto const usec = inter_frame_interval * nframe;
                    auto sec = usec / 1000000LL;
                    this->start_video_capture.tv_usec += usec - sec * inter_frame_interval;
                    if (this->start_video_capture.tv_usec >= 1000000LL){
                        this->start_video_capture.tv_usec -= 1000000LL;
                        ++sec;
                    }
                    this->start_video_capture.tv_sec += sec;
                    tick -= inter_frame_interval * nframe;
                }
                else {
                    do {
                        encoding_video_frame(this->start_video_capture.tv_sec);
                        this->start_video_capture.tv_usec += inter_frame_interval;
                        if (this->start_video_capture.tv_usec >= 1000000LL){
                            this->start_video_capture.tv_sec += 1;
                            this->start_video_capture.tv_usec -= 1000000LL;
                        }
                        tick -= inter_frame_interval;
                    } while (tick >= inter_frame_interval);
                }
            }

            return std::chrono::microseconds(inter_frame_interval - tick);
        }

        std::chrono::microseconds periodic_snapshot(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        ) {
            // assert(now >= previous);
            auto next_duration = this->do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
            assert(next_duration.count() >= 0);
            return next_duration;
        }

    } vc;

    FullVideoCaptureImpl(
        const timeval & now,
        const char * const record_path,
        const char * const basename,
        const int groupid,
        bool no_timestamp,
        RDPDrawable & drawable,
        FlvParams flv_params)
    : trans(
        FilenameGenerator::PATH_FILE_EXTENSION,
        record_path, basename, ("." + flv_params.codec).c_str(), groupid)
    , vc(now, this->trans, drawable, no_timestamp, std::move(flv_params))
    {
        ::unlink((std::string(record_path) + basename + "." + flv_params.codec).c_str());
    }

    virtual ~FullVideoCaptureImpl() {}

    void encoding_video_frame() {
        this->vc.encoding_video_frame();
    }

    void request_full_cleaning() {
        this->trans.request_full_cleaning();
    }
};

struct NotifyNextVideo : private noncopyable
{
    enum class reason { sequenced, external };
    virtual void notify_next_video(const timeval& now, reason) = 0;
    virtual ~NotifyNextVideo() = default;
};

class SequencedVideoCaptureImpl : public gdi::CaptureApi
{
    bool ic_has_first_img = false;

public:
    std::chrono::microseconds do_snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override
    {
        this->vc.do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        if (!this->ic_has_first_img) {
            return this->first_image.do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        }
        return this->video_sequencer.do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }

    std::chrono::microseconds frame_marker_event(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override
    {
        this->vc.frame_marker_event(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        if (!this->ic_has_first_img) {
            return this->first_image.frame_marker_event(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        }
        return this->video_sequencer.frame_marker_event(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }

    std::chrono::microseconds periodic_snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override
    {
        this->vc.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        if (!this->ic_has_first_img) {
            return this->first_image.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        }
        return this->video_sequencer.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }

private:

    class VideoTransport final : public OutFilenameSequenceSeekableTransport
    {
        using transport_base = OutFilenameSequenceSeekableTransport;

    public:
        VideoTransport(
            const char * const record_path,
            const char * const basename,
            const char * const suffix,
            const int groupid
        )
        : transport_base(FilenameGenerator::PATH_FILE_COUNT_EXTENSION, record_path, basename, suffix, groupid)
        {
            this->remove_current_path();
        }

        bool next() override {
            if (transport_base::next()) {
                this->remove_current_path();
                return true;
            }
            return false;
        }

    private:
        void remove_current_path() {
            const char * const path = this->seqgen()->get(this->get_seqno());
            ::unlink(path);
        }
    };



public:
    // first next_video is ignored
    struct FirstImage
    {
        SequencedVideoCaptureImpl & first_image_impl;

        const timeval first_image_start_capture;

        FirstImage(timeval const & now, SequencedVideoCaptureImpl & impl)
        : first_image_impl(impl)
        , first_image_start_capture(now)
        {}

        std::chrono::microseconds periodic_snapshot(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        ) {
            // assert(now >= previous);
            auto next_duration = this->do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
            assert(next_duration.count() >= 0);
            return next_duration;
        }

        std::chrono::microseconds frame_marker_event(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        )
        {
            return this->periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        }

        std::chrono::microseconds do_snapshot(
            const timeval& now, int x, int y, bool ignore_frame_in_timeval
        ) {
            std::chrono::microseconds ret;

            auto const duration = std::chrono::microseconds(difftimeval(now, this->first_image_start_capture));
            auto const interval = std::chrono::microseconds(std::chrono::seconds(3))/2;
            if (duration >= interval) {
                auto video_interval = first_image_impl.video_sequencer.get_interval();
                if (this->first_image_impl.ic_drawable.logical_frame_ended() || duration > std::chrono::seconds(2) || duration >= video_interval) {
                    tm ptm;
                    localtime_r(&now.tv_sec, &ptm);
                    this->first_image_impl.ic_drawable.trace_timestamp(ptm);
                    this->first_image_impl.ic_flush();
                    this->first_image_impl.ic_drawable.clear_timestamp();
                    this->first_image_impl.ic_has_first_img = true;
                    this->first_image_impl.ic_trans.next();
                    ret = video_interval;
                }
                else {
                    ret = interval / 3;
                }
            }
            else {
                ret = interval - duration;
            }

            return std::min(ret, this->first_image_impl.video_sequencer.periodic_snapshot(now, x, y, ignore_frame_in_timeval));
        }

    } first_image;

public:
    VideoTransport vc_trans;
    VideoCapture vc;
    OutFilenameSequenceTransport ic_trans;

    unsigned ic_zoom_factor;
    unsigned ic_scaled_width;
    unsigned ic_scaled_height;

    /* const */ RDPDrawable & ic_drawable;

    private:
        std::unique_ptr<uint8_t[]> ic_scaled_buffer;

    public:
    void zoom(unsigned percent) {
        percent = std::min(percent, 100u);
        const unsigned zoom_width = (this->ic_drawable.width() * percent) / 100;
        const unsigned zoom_height = (this->ic_drawable.height() * percent) / 100;
        this->ic_zoom_factor = percent;
        this->ic_scaled_width = (zoom_width + 3) & 0xFFC;
        this->ic_scaled_height = zoom_height;
        if (this->ic_zoom_factor != 100) {
            this->ic_scaled_buffer.reset(new uint8_t[this->ic_scaled_width * this->ic_scaled_height * 3]);
        }
    }

    void ic_flush() {
        if (this->ic_zoom_factor == 100) {
            this->dump24();
        }
        else {
            this->scale_dump24();
        }
    }

    void dump24() {
        ::transport_dump_png24(
            this->ic_trans, this->ic_drawable.data(),
            this->ic_drawable.width(), this->ic_drawable.height(),
            this->ic_drawable.rowsize(), true);
    }

    void scale_dump24() {
        scale_data(
            this->ic_scaled_buffer.get(), this->ic_drawable.data(),
            this->ic_scaled_width, this->ic_drawable.width(),
            this->ic_scaled_height, this->ic_drawable.height(),
            this->ic_drawable.rowsize());
        ::transport_dump_png24(
            this->ic_trans, this->ic_scaled_buffer.get(),
            this->ic_scaled_width, this->ic_scaled_height,
            this->ic_scaled_width * 3, false);
    }

    class VideoSequencer : public gdi::CaptureApi
    {
        timeval start_break;
        std::chrono::microseconds break_interval;

    protected:
        SequencedVideoCaptureImpl & impl;

    public:
        VideoSequencer(const timeval & now, std::chrono::microseconds break_interval, SequencedVideoCaptureImpl & impl)
        : start_break(now)
        , break_interval(break_interval)
        , impl(impl)
        {}

        std::chrono::microseconds get_interval() const {
            return this->break_interval;
        }

        void reset_now(const timeval& now) {
            this->start_break = now;
        }

        std::chrono::microseconds do_snapshot(
            const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
        ) override {
            assert(this->break_interval.count());
            auto const interval = difftimeval(now, this->start_break);
            if (interval >= uint64_t(this->break_interval.count())) {
                this->impl.next_video_impl(now, NotifyNextVideo::reason::sequenced);
                this->start_break = now;
            }
            return this->break_interval;
        }

        std::chrono::microseconds periodic_snapshot(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        ) {
            // assert(now >= previous);
            auto next_duration = this->do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
            assert(next_duration.count() >= 0);
            return next_duration;
        }

        std::chrono::microseconds frame_marker_event(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        )
        {
            return this->periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        }

    } video_sequencer;

    NotifyNextVideo & next_video_notifier;

    void next_video_impl(const timeval& now, NotifyNextVideo::reason reason) {
        this->video_sequencer.reset_now(now);
        if (!this->ic_has_first_img) {
            tm ptm;
            localtime_r(&now.tv_sec, &ptm);
            this->ic_drawable.trace_timestamp(ptm);
            this->ic_flush();
            this->ic_drawable.clear_timestamp();
            this->ic_has_first_img = true;
            this->ic_trans.next();
        }
        this->vc.next_video();
        tm ptm;
        localtime_r(&now.tv_sec, &ptm);
        this->ic_drawable.trace_timestamp(ptm);
        this->ic_flush();
        this->ic_drawable.clear_timestamp();
        this->ic_trans.next();
        this->next_video_notifier.notify_next_video(now, reason);
    }

public:
    SequencedVideoCaptureImpl(
        const timeval & now,
        const char * const record_path,
        const char * const basename,
        const int groupid,
        bool no_timestamp,
        unsigned image_zoom,
        /* const */RDPDrawable & drawable,
        FlvParams flv_params,
        std::chrono::microseconds video_interval,
        NotifyNextVideo & next_video_notifier)
    : first_image(now, *this)

    , vc_trans(record_path, basename, ("." + flv_params.codec).c_str(), groupid)
    , vc(now, this->vc_trans, drawable, no_timestamp, std::move(flv_params))
    , ic_trans(FilenameGenerator::PATH_FILE_COUNT_EXTENSION, record_path, basename, ".png", groupid, nullptr)
    , ic_zoom_factor(std::min(image_zoom, 100u))
    , ic_scaled_width(drawable.width())
    , ic_scaled_height(drawable.height())
    , ic_drawable(drawable)
    , video_sequencer(
        now, video_interval > std::chrono::microseconds(0) ? video_interval : std::chrono::microseconds::max(), *this)
    , next_video_notifier(next_video_notifier)
    {
        const unsigned zoom_width = (this->ic_drawable.width() * this->ic_zoom_factor) / 100;
        const unsigned zoom_height = (this->ic_drawable.height() * this->ic_zoom_factor) / 100;
        this->ic_scaled_width = (zoom_width + 3) & 0xFFC;
        this->ic_scaled_height = zoom_height;
        if (this->ic_zoom_factor != 100) {
            this->ic_scaled_buffer.reset(new uint8_t[this->ic_scaled_width * this->ic_scaled_height * 3]);
        }
    }

    void next_video(const timeval& now) {
        this->next_video_impl(now, NotifyNextVideo::reason::external);
    }

    void encoding_video_frame() {
        this->vc.encoding_video_frame();
    }

    void request_full_cleaning() {
        this->vc_trans.request_full_cleaning();
        this->ic_trans.request_full_cleaning();
    }
};

namespace {
    template<std::size_t N>
    inline bool cstr_equal(char const (&s1)[N], array_view_const_char s2) {
        return N - 1 == s2.size() && std::equal(s1, s1 + N - 1, begin(s2));
    }

    template<std::size_t N>
    void str_append(std::string & s, char const (&s2)[N]) {
        s.append(s2, N-1);
    }

    inline void str_append(std::string & s, array_view_const_char const & s2) {
        s.append(s2.data(), s2.size());
    }

    template<class... S>
    void str_append(std::string & s, S const & ... strings) {
        (void)std::initializer_list<int>{
            (str_append(s, strings), 0)...
        };
    }
}

inline void agent_data_extractor(std::string & line, array_view_const_char data)
{
    using Av = array_view_const_char;

    auto find = [](Av & s, char c) {
        auto p = std::find(begin(s), end(s), c);
        return p == end(s) ? nullptr : p;
    };

    auto separator = find(data, '=');

    if (separator) {
        auto right = [](Av s, char const * pos) { return Av(begin(s), pos - begin(s)); };
        auto left = [](Av s, char const * pos) { return Av(pos + 1, begin(s) - (pos + 1)); };

        auto order = left(data, separator);
        auto parameters = right(data, separator);

        auto line_with_1_var = [&](Av var1) {
            str_append(
                line,
                "type=\"", order, "\" ",
                Av(var1.data(), var1.size()-1), "=\"", parameters, "\""
            );
        };
        auto line_with_2_var = [&](Av var1, Av var2) {
            if (auto subitem_separator = find(parameters, '\x01')) {
                str_append(
                    line,
                    "type=\"", order, "\" ",
                    Av(var1.data(), var1.size()-1), "=\"", left(parameters, subitem_separator), "\" ",
                    Av(var2.data(), var2.size()-1), "=\"", right(parameters, subitem_separator), "\""
                );
            }
        };
        auto line_with_3_var = [&](Av var1, Av var2, Av var3) {
            if (auto subitem_separator = find(parameters, '\x01')) {
                auto text = left(parameters, subitem_separator);
                auto remaining = right(parameters, subitem_separator);
                if (auto subitem_separator2 = find(remaining, '\x01')) {
                    str_append(
                        line,
                        "type=\"", order, "\" ",
                        Av(var1.data(), var1.size()-1), "=\"", text, "\" ",
                        Av(var2.data(), var2.size()-1), "=\"", left(remaining, subitem_separator2), "\" ",
                        Av(var3.data(), var3.size()-1), "=\"", right(remaining, subitem_separator2), "\""
                    );
                }
            }
        };

        // TODO used string_id: switch (sid(order)) { case "string"_sid: ... }
        if (cstr_equal("PASSWORD_TEXT_BOX_GET_FOCUS", order)
         || cstr_equal("UAC_PROMPT_BECOME_VISIBLE", order)) {
            line_with_1_var("status");
        }
        else if (cstr_equal("INPUT_LANGUAGE", order)) {
            line_with_2_var("identifier", "display_name");
        }
        else if (cstr_equal("NEW_PROCESS", order)
              || cstr_equal("COMPLETED_PROCESS", order)) {
            line_with_1_var("command_line");
        }
        else if (cstr_equal("OUTBOUND_CONNECTION_BLOCKED", order)) {
            line_with_2_var("rule", "application_name");
        }
        else if (cstr_equal("FOREGROUND_WINDOW_CHANGED", order)) {
            line_with_3_var("windows", "class", "command_line");
        }
        else if (cstr_equal("BUTTON_CLICKED", order)) {
            line_with_2_var("windows", "button");
        }
        else if (cstr_equal("EDIT_CHANGED", order)) {
            line_with_2_var("windows", "edit");
        }
        else {
            LOG(LOG_WARNING,
                "MetaDataExtractor(): Unexpected order. Data=\"%*s\"",
                int(data.size()), data.data());
            return;
        }
    }

    if (line.empty()) {
        LOG(LOG_WARNING,
            "MetaDataExtractor(): Invalid data format. Data=\"%*s\"",
            int(data.size()), data.data());
        return;
    }
}

namespace {
    constexpr array_view_const_char session_meta_kbd_prefix() { return cstr_array_view("[Kbd]"); }
    constexpr array_view_const_char session_meta_kbd_suffix() { return cstr_array_view("\n"); }
}

/*
* Format:
*
* $date ' - [Kbd]' $kbd
* $date ' ' [+-] ' ' $title? '[Kbd]' $kbd
* $date ' - ' $line
*/

class SessionMeta final : public gdi::KbdInputApi, public gdi::CaptureApi, public gdi::CaptureProbeApi
{
    OutStream kbd_stream;
    bool keyboard_input_mask_enabled = false;
    uint8_t kbd_buffer[1024];
    timeval last_snapshot;
    time_t last_flush;
    Transport & trans;
    std::string title;
    bool require_kbd = false;
    char current_seperator = '-';
    bool is_probe_enabled_session = false;

    void write_shadow_keys() {
        if (!this->kbd_stream.has_room(1)) {
            this->flush();
        }
        this->kbd_stream.out_uint8('*');
    }

    void write_keys(uint32_t uchar) {
        filtering_kbd_input(
            uchar,
            [this](uint32_t uchar) {
                uint8_t buf_char[5];
                if (uchar == '/') {
                    this->copy_bytes({"//", 2});
                }
                else if (size_t const char_len = UTF32toUTF8(uchar, buf_char, sizeof(buf_char))) {
                    this->copy_bytes({buf_char, char_len});
                }
            },
            [this](array_view_const_char no_printable_str) {
                this->copy_bytes(no_printable_str);
            }
        );
    }

    void copy_bytes(const_bytes_array bytes) {
        if (this->kbd_stream.tailroom() < bytes.size()) {
            this->flush();
        }
        this->kbd_stream.out_copy_bytes(bytes.data(), std::min(this->kbd_stream.tailroom(), bytes.size()));
    }

public:
    SessionMeta(const timeval & now, Transport & trans)
    : kbd_stream{
        this->kbd_buffer + session_meta_kbd_prefix().size(),
        sizeof(this->kbd_buffer) - session_meta_kbd_prefix().size() - session_meta_kbd_suffix().size()}
    , last_snapshot(now)
    , last_flush(now.tv_sec)
    , trans(trans)
    {
        OutStream(this->kbd_buffer).out_copy_bytes(session_meta_kbd_prefix().data(), session_meta_kbd_prefix().size());

        // force file creation even if no text recognized
        this->trans.send("", 0);
    }

    ~SessionMeta() {
        this->send_kbd();
    }

    void enable_kbd_input_mask(bool enable) override {
        if (this->keyboard_input_mask_enabled != enable) {
            this->flush();
            this->keyboard_input_mask_enabled = enable;
        }
    }

    bool kbd_input(const timeval& /*now*/, uint32_t uchar) override {
        if (this->keyboard_input_mask_enabled) {
            if (this->is_probe_enabled_session) {
                this->write_shadow_keys();
            }
        }
        else {
            this->write_keys(uchar);
        }
        return true;
    }

    void title_changed(time_t rawtime, array_view_const_char title) {
        this->send_kbd();
        this->send_date(rawtime, '+');
        this->trans.send(title.data(), title.size());
        this->last_flush = rawtime;

        this->title.assign(title.data(), title.size());
        this->require_kbd = true;
    }

    void send_line(time_t rawtime, array_view_const_char line) {
        this->send_kbd();
        this->send_date(rawtime, '+');
        this->trans.send(line.data(), line.size());
        this->trans.send("\n", 1);
        this->last_flush = rawtime;
    }

    void session_update(const timeval& /*now*/, array_view_const_char message) override {
        this->is_probe_enabled_session = (::strcmp(message.data(), "Probe.Status=Unknown") != 0);
    }

    void possible_active_window_change() override {
    }

private:
    std::chrono::microseconds do_snapshot(
        const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) override {
        std::chrono::microseconds const time_to_wait = std::chrono::seconds{2};
        std::chrono::microseconds const diff {difftimeval(now, this->last_snapshot)};

        if (diff < time_to_wait && this->kbd_stream.get_offset() < 8 * sizeof(uint32_t)) {
            return time_to_wait;
        }

        this->send_kbd();

        this->last_snapshot = now;
        this->last_flush = this->last_snapshot.tv_sec;

        return time_to_wait;
    }

    void flush() {
        this->send_kbd();
    }

    void send_date(time_t rawtime, char sep) {
        tm ptm;
        localtime_r(&rawtime, &ptm);

        char string_date[256];

        auto const data_sz = std::sprintf(
            string_date, "%4d-%02d-%02d %02d:%02d:%02d %c ",
            ptm.tm_year+1900, ptm.tm_mon+1, ptm.tm_mday,
            ptm.tm_hour, ptm.tm_min, ptm.tm_sec, sep
        );

        this->trans.send(string_date, data_sz);
    }

    void send_kbd() {
        if (this->kbd_stream.get_offset()) {
            if (!this->require_kbd) {
                this->send_date(this->last_flush, this->current_seperator);
                this->trans.send(this->title.data(), this->title.size());
            }
            auto end = this->kbd_stream.get_current();
            memcpy(end, session_meta_kbd_suffix().data(), session_meta_kbd_suffix().size());
            end += session_meta_kbd_suffix().size();
            this->trans.send(this->kbd_buffer, std::size_t(end - this->kbd_buffer));
            this->kbd_stream.rewind();
            this->require_kbd = false;
        }
        else if (this->require_kbd) {
            this->trans.send("\n", 1);
            this->require_kbd = false;
        }
        this->current_seperator = '-';
    }
};

class SessionLogAgent : public gdi::CaptureProbeApi
{
    std::string line;
    SessionMeta & session_meta;

public:
    SessionLogAgent(SessionMeta & session_meta)
    : session_meta(session_meta)
    {}

    void session_update(const timeval& now, array_view_const_char message) override {
        line.clear();
        agent_data_extractor(this->line, message);
        if (!this->line.empty()) {
            this->session_meta.send_line(now.tv_sec, this->line);
        }
    }

    void possible_active_window_change() override {
    }
};

class MetaCaptureImpl
{
public:
    local_fd file;
    OutFileTransport meta_trans;
    SessionMeta meta;
    SessionLogAgent session_log_agent;
    bool enable_agent;

    MetaCaptureImpl(
        const timeval & now,
        std::string record_path,
        const char * const basename,
        bool enable_agent)
    : file([](const char * filename){
        int fd = ::open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0440);
        if (fd < 0) {
            LOG(LOG_ERR, "failed opening=%s\n", filename);
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
        return fd;
    }(record_path.append(basename).append(".meta").c_str()))
    , meta_trans(this->file.fd())
    , meta(now, this->meta_trans)
    , session_log_agent(this->meta)
    , enable_agent(enable_agent)
    {
    }

    SessionMeta & get_session_meta() {
        return this->meta;
    }

    void request_full_cleaning() {
        this->meta_trans.request_full_cleaning();
    }
};


struct NotifyTitleChanged : private noncopyable
{
    virtual void notify_title_changed(const timeval & now, array_view_const_char title) = 0;
    virtual ~NotifyTitleChanged() = default;
};

class TitleCaptureImpl : public gdi::CaptureApi, public gdi::CaptureProbeApi
{
public:
    OcrTitleExtractorBuilder ocr_title_extractor_builder;
    AgentTitleExtractor agent_title_extractor;

    std::reference_wrapper<TitleExtractorApi> title_extractor;

    timeval  last_ocr;
    std::chrono::microseconds usec_ocr_interval;

    NotifyTitleChanged & notify_title_changed;

    TitleCaptureImpl(
        const timeval & now,
        auth_api * authentifier,
        RDPDrawable & drawable,
        OcrParams ocr_params,
        NotifyTitleChanged & notify_title_changed)
    : ocr_title_extractor_builder(
        drawable.impl(), authentifier != nullptr,
        ocr_params.ocr_version,
        ocr_params.ocr_locale,
        ocr_params.ocr_on_title_bar_only,
        ocr_params.max_unrecog_char_rate)
    , title_extractor(this->ocr_title_extractor_builder.get_title_extractor())
    , last_ocr(now)
    , usec_ocr_interval(ocr_params.usec_ocr_interval)
    , notify_title_changed(notify_title_changed)
    {
    }


    std::chrono::microseconds do_snapshot(
        const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
    ) override {
        std::chrono::microseconds const diff {difftimeval(now, this->last_ocr)};

        if (diff >= this->usec_ocr_interval) {
            this->last_ocr = now;

            auto title = this->title_extractor.get().extract_title();

            if (title.data()/* && title.size()*/) {
                notify_title_changed.notify_title_changed(now, title);
            }

            return this->usec_ocr_interval;
        }
        else {
            return this->usec_ocr_interval - diff;
        }
    }

    void session_update(timeval const & /*now*/, array_view_const_char message) override {
        bool const enable_probe = (::strcmp(message.data(), "Probe.Status=Unknown") != 0);
        if (enable_probe) {
            this->title_extractor = this->agent_title_extractor;
        }
        else {
            this->title_extractor = this->ocr_title_extractor_builder.get_title_extractor();
        }

        this->agent_title_extractor.session_update(message);
    }

    void possible_active_window_change() override {}
};


inline void send_wrm_chunk(Transport & t, uint16_t chunktype, uint16_t data_size, uint16_t count)
{
    StaticOutStream<8> header;
    header.out_uint16_le(chunktype);
    header.out_uint32_le(8 + data_size);
    header.out_uint16_le(count);
    t.send(header.get_data(), header.get_offset());
}


inline void send_meta_chunk(
    Transport & t
  , uint8_t wrm_format_version

  , uint16_t info_width
  , uint16_t info_height
  , uint16_t info_bpp

  , uint16_t info_cache_0_entries
  , uint16_t info_cache_0_size
  , uint16_t info_cache_1_entries
  , uint16_t info_cache_1_size
  , uint16_t info_cache_2_entries
  , uint16_t info_cache_2_size

  , uint16_t info_number_of_cache
  , bool     info_use_waiting_list

  , bool     info_cache_0_persistent
  , bool     info_cache_1_persistent
  , bool     info_cache_2_persistent

  , uint16_t info_cache_3_entries
  , uint16_t info_cache_3_size
  , bool     info_cache_3_persistent
  , uint16_t info_cache_4_entries
  , uint16_t info_cache_4_size
  , bool     info_cache_4_persistent

  , uint8_t  index_algorithm
) {
    StaticOutStream<36> payload;
    payload.out_uint16_le(wrm_format_version);
    payload.out_uint16_le(info_width);
    payload.out_uint16_le(info_height);
    payload.out_uint16_le(info_bpp);

    payload.out_uint16_le(info_cache_0_entries);
    payload.out_uint16_le(info_cache_0_size);
    payload.out_uint16_le(info_cache_1_entries);
    payload.out_uint16_le(info_cache_1_size);
    payload.out_uint16_le(info_cache_2_entries);
    payload.out_uint16_le(info_cache_2_size);

    if (wrm_format_version > 3) {
        payload.out_uint8(info_number_of_cache);
        payload.out_uint8(info_use_waiting_list);

        payload.out_uint8(info_cache_0_persistent);
        payload.out_uint8(info_cache_1_persistent);
        payload.out_uint8(info_cache_2_persistent);

        payload.out_uint16_le(info_cache_3_entries);
        payload.out_uint16_le(info_cache_3_size);
        payload.out_uint8(info_cache_3_persistent);
        payload.out_uint16_le(info_cache_4_entries);
        payload.out_uint16_le(info_cache_4_size);
        payload.out_uint8(info_cache_4_persistent);

        payload.out_uint8(index_algorithm);
    }

    send_wrm_chunk(t, META_FILE, payload.get_offset(), 1);
    t.send(payload.get_data(), payload.get_offset());
}


struct ChunkToFile : public RDPChunkedDevice {
private:
    CompressionOutTransportBuilder compression_bullder;
    Transport & trans_target;
    Transport & trans;

    const uint8_t wrm_format_version;

    uint16_t info_version = 0;

public:
    ChunkToFile(Transport * trans

               , uint16_t info_width
               , uint16_t info_height
               , uint16_t info_bpp
               , uint16_t info_cache_0_entries
               , uint16_t info_cache_0_size
               , uint16_t info_cache_1_entries
               , uint16_t info_cache_1_size
               , uint16_t info_cache_2_entries
               , uint16_t info_cache_2_size

               , uint16_t info_number_of_cache
               , bool     info_use_waiting_list

               , bool     info_cache_0_persistent
               , bool     info_cache_1_persistent
               , bool     info_cache_2_persistent

               , uint16_t info_cache_3_entries
               , uint16_t info_cache_3_size
               , bool     info_cache_3_persistent
               , uint16_t info_cache_4_entries
               , uint16_t info_cache_4_size
               , bool     info_cache_4_persistent

               , WrmCompressionAlgorithm wrm_compression_algorithm)
    : RDPChunkedDevice()
    , compression_bullder(*trans, wrm_compression_algorithm)
    , trans_target(*trans)
    , trans(this->compression_bullder.get())
    , wrm_format_version(bool(this->compression_bullder.get_algorithm()) ? 4 : 3)
    {
        if (wrm_compression_algorithm != this->compression_bullder.get_algorithm()) {
            LOG( LOG_WARNING, "compression algorithm %u not fount. Compression disable."
               , static_cast<unsigned>(wrm_compression_algorithm));
        }

        send_meta_chunk(
            this->trans_target
          , this->wrm_format_version

          , info_width
          , info_height
          , info_bpp
          , info_cache_0_entries
          , info_cache_0_size
          , info_cache_1_entries
          , info_cache_1_size
          , info_cache_2_entries
          , info_cache_2_size

          , info_number_of_cache
          , info_use_waiting_list

          , info_cache_0_persistent
          , info_cache_1_persistent
          , info_cache_2_persistent

          , info_cache_3_entries
          , info_cache_3_size
          , info_cache_3_persistent
          , info_cache_4_entries
          , info_cache_4_size
          , info_cache_4_persistent

          , static_cast<unsigned>(this->compression_bullder.get_algorithm())
        );
    }

public:
    void chunk(uint16_t chunk_type, uint16_t chunk_count, InStream stream) override {
        switch (chunk_type) {
        case META_FILE:
            {
                this->info_version                  = stream.in_uint16_le();
                uint16_t info_width                 = stream.in_uint16_le();
                uint16_t info_height                = stream.in_uint16_le();
                uint16_t info_bpp                   = stream.in_uint16_le();
                uint16_t info_cache_0_entries       = stream.in_uint16_le();
                uint16_t info_cache_0_size          = stream.in_uint16_le();
                uint16_t info_cache_1_entries       = stream.in_uint16_le();
                uint16_t info_cache_1_size          = stream.in_uint16_le();
                uint16_t info_cache_2_entries       = stream.in_uint16_le();
                uint16_t info_cache_2_size          = stream.in_uint16_le();

                uint16_t info_number_of_cache       = 3;
                bool     info_use_waiting_list      = false;

                bool     info_cache_0_persistent    = false;
                bool     info_cache_1_persistent    = false;
                bool     info_cache_2_persistent    = false;

                uint16_t info_cache_3_entries       = 0;
                uint16_t info_cache_3_size          = 0;
                bool     info_cache_3_persistent    = false;
                uint16_t info_cache_4_entries       = 0;
                uint16_t info_cache_4_size          = 0;
                bool     info_cache_4_persistent    = false;

                if (this->info_version > 3) {
                    info_number_of_cache            = stream.in_uint8();
                    info_use_waiting_list           = (stream.in_uint8() ? true : false);

                    info_cache_0_persistent         = (stream.in_uint8() ? true : false);
                    info_cache_1_persistent         = (stream.in_uint8() ? true : false);
                    info_cache_2_persistent         = (stream.in_uint8() ? true : false);

                    info_cache_3_entries            = stream.in_uint16_le();
                    info_cache_3_size               = stream.in_uint16_le();
                    info_cache_3_persistent         = (stream.in_uint8() ? true : false);

                    info_cache_4_entries            = stream.in_uint16_le();
                    info_cache_4_size               = stream.in_uint16_le();
                    info_cache_4_persistent         = (stream.in_uint8() ? true : false);

                    //uint8_t info_compression_algorithm = stream.in_uint8();
                    //REDASSERT(info_compression_algorithm < 3);
                }


                send_meta_chunk(
                    this->trans_target
                  , this->wrm_format_version

                  , info_width
                  , info_height
                  , info_bpp
                  , info_cache_0_entries
                  , info_cache_0_size
                  , info_cache_1_entries
                  , info_cache_1_size
                  , info_cache_2_entries
                  , info_cache_2_size

                  , info_number_of_cache
                  , info_use_waiting_list

                  , info_cache_0_persistent
                  , info_cache_1_persistent
                  , info_cache_2_persistent

                  , info_cache_3_entries
                  , info_cache_3_size
                  , info_cache_3_persistent
                  , info_cache_4_entries
                  , info_cache_4_size
                  , info_cache_4_persistent

                  , static_cast<unsigned>(this->compression_bullder.get_algorithm())
                );
            }
            break;

        case SAVE_STATE:
            {
                StateChunk sc;
                SaveStateChunk ssc;

                ssc.recv(stream, sc, this->info_version);

                StaticOutStream<65536> payload;

                ssc.send(payload, sc);

                send_wrm_chunk(this->trans, SAVE_STATE, payload.get_offset(), chunk_count);
                this->trans.send(payload.get_data(), payload.get_offset());
            }
            break;

        case RESET_CHUNK:
            {
                send_wrm_chunk(this->trans, RESET_CHUNK, 0, 1);
                this->trans.next();
            }
            break;

        case TIMESTAMP:
            {
                timeval record_now;
                stream.in_timeval_from_uint64le_usec(record_now);
                this->trans_target.timestamp(record_now);
            }
            REDEMPTION_CXX_FALLTHROUGH;
        default:
            {
                send_wrm_chunk(this->trans, chunk_type, stream.get_capacity(), chunk_count);
                this->trans.send(stream.get_data(), stream.get_capacity());
            }
            break;
        }
    }
};



template <size_t SZ>
class OutChunkedBufferingTransport : public Transport
{
    Transport & trans;
    size_t max;
    uint8_t buf[SZ];
    OutStream stream;

    static_assert(SZ >= 8, "");

public:
    explicit OutChunkedBufferingTransport(Transport & trans)
        : trans(trans)
        , max(SZ-8)
        , stream(buf)
    {
    }

    void flush() override {
        if (this->stream.get_offset() > 0) {
            send_wrm_chunk(this->trans, LAST_IMAGE_CHUNK, this->stream.get_offset(), 1);
            this->trans.send(this->stream.get_data(), this->stream.get_offset());
            this->stream = OutStream(buf);
        }
    }

private:
    void do_send(const uint8_t * const buffer, size_t len) override {
        size_t to_buffer_len = len;
        while (this->stream.get_offset() + to_buffer_len > this->max) {
            send_wrm_chunk(this->trans, PARTIAL_IMAGE_CHUNK, this->max, 1);
            this->trans.send(this->stream.get_data(), this->stream.get_offset());
            size_t to_send = this->max - this->stream.get_offset();
            this->trans.send(buffer + len - to_buffer_len, to_send);
            to_buffer_len -= to_send;
            this->stream = OutStream(buf);
        }
        this->stream.out_copy_bytes(buffer + len - to_buffer_len, to_buffer_len);
    }
};

/**
 * To keep things easy all chunks have 8 bytes headers
 * starting with chunk_type, chunk_size and order_count
 *  (whatever it means, depending on chunks)
 */
class GraphicToFile
: public RDPSerializer
, public gdi::KbdInputApi
, public gdi::CaptureProbeApi
{
    enum {
        GTF_SIZE_KEYBUF_REC = 1024
    };

    CompressionOutTransportBuilder compression_bullder;
    Transport & trans_target;
    Transport & trans;
    StaticOutStream<65536> buffer_stream_orders;
    StaticOutStream<65536> buffer_stream_bitmaps;

    const std::chrono::microseconds delta_time = std::chrono::seconds(1);
    timeval timer;
    timeval last_sent_timer;
    const uint16_t width;
    const uint16_t height;
    uint16_t mouse_x;
    uint16_t mouse_y;
    const bool send_input;
    gdi::DumpPng24Api & dump_png24_api;


    uint8_t keyboard_buffer_32_buf[GTF_SIZE_KEYBUF_REC * sizeof(uint32_t)];
    // Extractor
    OutStream keyboard_buffer_32;

    const uint8_t wrm_format_version;

public:
    enum class SendInput { NO, YES };

    GraphicToFile(const timeval & now
                , Transport & trans
                , const uint16_t width
                , const uint16_t height
                , const uint8_t  capture_bpp
                , BmpCache & bmp_cache
                , GlyphCache & gly_cache
                , PointerCache & ptr_cache
                , gdi::DumpPng24Api & dump_png24
                , WrmCompressionAlgorithm wrm_compression_algorithm
                , SendInput send_input = SendInput::NO
                , Verbose verbose = Verbose::none)
    : RDPSerializer( this->buffer_stream_orders, this->buffer_stream_bitmaps, capture_bpp
                   , bmp_cache, gly_cache, ptr_cache, 0, true, true, 32 * 1024, verbose)
    , compression_bullder(trans, wrm_compression_algorithm)
    , trans_target(trans)
    , trans(this->compression_bullder.get())
    , timer(now)
    , last_sent_timer{0, 0}
    , width(width)
    , height(height)
    , mouse_x(0)
    , mouse_y(0)
    , send_input(send_input == SendInput::YES)
    , dump_png24_api(dump_png24)
    , keyboard_buffer_32(keyboard_buffer_32_buf)
    , wrm_format_version(bool(this->compression_bullder.get_algorithm()) ? 4 : 3)
    {
        if (wrm_compression_algorithm != this->compression_bullder.get_algorithm()) {
            LOG( LOG_WARNING, "compression algorithm %u not fount. Compression disable."
               , static_cast<unsigned>(wrm_compression_algorithm));
        }

        this->order_count = 0;

        this->send_meta_chunk();
        this->send_image_chunk();
    }

    void dump_png24(Transport & trans, bool bgr) const {
        this->dump_png24_api.dump_png24(trans, bgr);
    }

    /// \brief Update timestamp but send nothing, the timestamp will be sent later with the next effective event
    void timestamp(const timeval& now)
    {
        if (this->timer < now) {
            this->flush_orders();
            this->flush_bitmaps();
            this->timer = now;
            this->trans.timestamp(now);
        }
    }

    void mouse(uint16_t mouse_x, uint16_t mouse_y)
    {
        this->mouse_x = mouse_x;
        this->mouse_y = mouse_y;
    }

    bool kbd_input(const timeval & now, uint32_t uchar) override {
        (void)now;
        if (keyboard_buffer_32.has_room(sizeof(uint32_t))) {
            keyboard_buffer_32.out_uint32_le(uchar);
        }
        return true;
    }

    void enable_kbd_input_mask(bool) override {
    }

    void send_meta_chunk()
    {
        const BmpCache::cache_ & c0 = this->bmp_cache.get_cache(0);
        const BmpCache::cache_ & c1 = this->bmp_cache.get_cache(1);
        const BmpCache::cache_ & c2 = this->bmp_cache.get_cache(2);
        const BmpCache::cache_ & c3 = this->bmp_cache.get_cache(3);
        const BmpCache::cache_ & c4 = this->bmp_cache.get_cache(4);

        ::send_meta_chunk(
            this->trans_target
          , this->wrm_format_version

          , this->width
          , this->height
          , this->capture_bpp

          , c0.entries()
          , c0.bmp_size()
          , c1.entries()
          , c1.bmp_size()
          , c2.entries()
          , c2.bmp_size()

          , this->bmp_cache.number_of_cache
          , this->bmp_cache.use_waiting_list

          , c0.persistent()
          , c1.persistent()
          , c2.persistent()

          , c3.entries()
          , c3.bmp_size()
          , c3.persistent()
          , c4.entries()
          , c4.bmp_size()
          , c4.persistent()

          , static_cast<unsigned>(this->compression_bullder.get_algorithm())
        );
    }

    // this one is used to store some embedded image inside WRM
    void send_image_chunk(void)
    {
        OutChunkedBufferingTransport<65536> png_trans(this->trans);
        this->dump_png24_api.dump_png24(png_trans, false);
    }

    void send_reset_chunk()
    {
        send_wrm_chunk(this->trans, RESET_CHUNK, 0, 1);
    }

    void send_timestamp_chunk(bool ignore_time_interval = false)
    {
        StaticOutStream<12 + GTF_SIZE_KEYBUF_REC * sizeof(uint32_t) + 1> payload;
        payload.out_timeval_to_uint64le_usec(this->timer);
        if (this->send_input) {
            payload.out_uint16_le(this->mouse_x);
            payload.out_uint16_le(this->mouse_y);

            payload.out_uint8(ignore_time_interval ? 1 : 0);

            payload.out_copy_bytes(keyboard_buffer_32.get_data(), keyboard_buffer_32.get_offset());
            keyboard_buffer_32 = OutStream(keyboard_buffer_32_buf);
        }

        send_wrm_chunk(this->trans, TIMESTAMP, payload.get_offset(), 1);
        this->trans.send(payload.get_data(), payload.get_offset());

        this->last_sent_timer = this->timer;
    }

    void send_save_state_chunk()
    {
        StaticOutStream<4096> payload;
        SaveStateChunk ssc;
        ssc.send(payload, this->ssc);

        //------------------------------ missing variable length ---------------

        send_wrm_chunk(this->trans, SAVE_STATE, payload.get_offset(), 1);
        this->trans.send(payload.get_data(), payload.get_offset());
    }

    void save_bmp_caches()
    {
        for (uint8_t cache_id = 0
        ; cache_id < this->bmp_cache.number_of_cache
        ; ++cache_id) {
            const size_t entries = this->bmp_cache.get_cache(cache_id).entries();
            for (size_t i = 0; i < entries; i++) {
                this->bmp_cache.set_cached(cache_id, i, false);
            }
        }
    }

    void save_glyph_caches()
    {
        for (uint8_t cacheId = 0; cacheId < NUMBER_OF_GLYPH_CACHES; ++cacheId) {
            for (uint8_t cacheIndex = 0; cacheIndex < NUMBER_OF_GLYPH_CACHE_ENTRIES; ++cacheIndex) {
                this->glyph_cache.set_cached(cacheId, cacheIndex, false);
            }
        }
    }

    void save_ptr_cache() {
        for (int index = 0; index < MAX_POINTER_COUNT; ++index) {
            this->pointer_cache.set_cached(index, false);
        }
    }

    void send_caches_chunk()
    {
        this->save_bmp_caches();
        this->save_glyph_caches();
        this->save_ptr_cache();
        if (this->order_count > 0) {
            this->send_orders_chunk();
        }
    }

    void breakpoint()
    {
        this->flush_orders();
        this->flush_bitmaps();
        this->send_timestamp_chunk();
        if (bool(this->compression_bullder.get_algorithm())) {
            this->send_reset_chunk();
        }
        this->trans.next();
        this->send_meta_chunk();
        this->send_timestamp_chunk();
        this->send_save_state_chunk();

        OutChunkedBufferingTransport<65536> png_trans(this->trans);

        this->dump_png24_api.dump_png24(png_trans, true);

        this->send_caches_chunk();
    }

private:
    std::chrono::microseconds elapsed_time() const
    {
        using us = std::chrono::microseconds;
        return us(ustime(this->timer)) - us(ustime(this->last_sent_timer));
    }

protected:
    void flush_orders() override {
        if (this->order_count > 0) {
            if (this->elapsed_time() >= delta_time) {
                this->send_timestamp_chunk();
            }
            this->send_orders_chunk();
        }
    }

public:
    void send_orders_chunk()
    {
        send_wrm_chunk(this->trans, RDP_UPDATE_ORDERS, this->stream_orders.get_offset(), this->order_count);
        this->trans.send(this->stream_orders.get_data(), this->stream_orders.get_offset());
        this->order_count = 0;
        this->stream_orders.rewind();
    }

protected:
    void flush_bitmaps() override {
        if (this->bitmap_count > 0) {
            if (this->elapsed_time() >= delta_time) {
                this->send_timestamp_chunk();
            }
            this->send_bitmaps_chunk();
        }
    }

public:
    void sync() override {
        this->flush_bitmaps();
        this->flush_orders();
    }

    void send_bitmaps_chunk()
    {
        send_wrm_chunk(this->trans, RDP_UPDATE_BITMAP, this->stream_bitmaps.get_offset(), this->bitmap_count);
        this->trans.send(this->stream_bitmaps.get_data(), this->stream_bitmaps.get_offset());
        this->bitmap_count = 0;
        this->stream_bitmaps.rewind();
    }

protected:
    void send_pointer(int cache_idx, const Pointer & cursor) override {
        size_t size =   2           // mouse x
                      + 2           // mouse y
                      + 1           // cache index
                      + 1           // hotspot x
                      + 1           // hotspot y
                      + 32 * 32 * 3 // data
                      + 128         // mask
                      ;
        send_wrm_chunk(this->trans, POINTER, size, 0);

        StaticOutStream<16> payload;
        payload.out_uint16_le(this->mouse_x);
        payload.out_uint16_le(this->mouse_y);
        payload.out_uint8(cache_idx);
        payload.out_uint8(cursor.x);
        payload.out_uint8(cursor.y);
        this->trans.send(payload.get_data(), payload.get_offset());

        this->trans.send(cursor.data, cursor.data_size());
        this->trans.send(cursor.mask, cursor.mask_size());
    }

    void set_pointer(int cache_idx) override {
        size_t size =   2                   // mouse x
                      + 2                   // mouse y
                      + 1                   // cache index
                      ;
        send_wrm_chunk(this->trans, POINTER, size, 0);

        StaticOutStream<16> payload;
        payload.out_uint16_le(this->mouse_x);
        payload.out_uint16_le(this->mouse_y);
        payload.out_uint8(cache_idx);
        this->trans.send(payload.get_data(), payload.get_offset());
    }

public:
    void session_update(timeval const & now, array_view_const_char message) override {
        uint16_t message_length = message.size() + 1;       // Null-terminator is included.

        StaticOutStream<16> payload;
        payload.out_timeval_to_uint64le_usec(now);
        payload.out_uint16_le(message_length);

        send_wrm_chunk(this->trans, SESSION_UPDATE, payload.get_offset() + message_length, 1);
        this->trans.send(payload.get_data(), payload.get_offset());
        this->trans.send(message.data(), message.size());
        this->trans.send("\0", 1);

        this->last_sent_timer = this->timer;
    }

    void possible_active_window_change() override {}

    using RDPSerializer::set_pointer;
};  // struct GraphicToFile



class WrmCaptureImpl :
    public gdi::KbdInputApi,
    public gdi::CaptureApi,
    public gdi::GraphicApi,
    public gdi::CaptureProbeApi,
    public gdi::ExternalCaptureApi
{
public:
    BmpCache     bmp_cache;
    GlyphCache   gly_cache;
    PointerCache ptr_cache;

    DumpPng24FromRDPDrawableAdapter dump_png24_api;

    struct TransportVariant
    {
        union Variant
        {
            OutMetaSequenceTransportWithSum out_with_sum;
            CryptoOutMetaSequenceTransport out_crypto;
            OutMetaSequenceTransport out;

            struct {} dummy;
            Variant() : dummy() {}
            ~Variant() {}
        } variant;
        ::Transport * trans;

        template<class... Ts>
        TransportVariant(
            TraceType trace_type,
            CryptoContext & cctx,
            Random & rnd,
            const char * path,
            const char * hash_path,
            const char * basename,
            timeval const & now,
            uint16_t width,
            uint16_t height,
            const int groupid,
            auth_api * authentifier
        ) {
            // TODO there should only be one outmeta, not two. Capture code should not really care if file is encrypted or not. Here is not the right level to manage anything related to encryption.
            // TODO Also we may wonder why we are encrypting wrm and not png (This is related to the path split between png and wrm). We should stop and consider what we should actually do
            switch (trace_type) {
                case TraceType::cryptofile:
                    this->trans = new (&this->variant.out_crypto)
                    CryptoOutMetaSequenceTransport(cctx, rnd, path, hash_path, basename, now, width, height, groupid, authentifier);
                    break;
                case TraceType::localfile_hashed:
                    this->trans = new (&this->variant.out_with_sum)
                    OutMetaSequenceTransportWithSum(cctx, path, hash_path, basename, now, width, height, groupid, authentifier);
                    break;
                default:
                case TraceType::localfile:
                    this->trans = new (&this->variant.out)
                    OutMetaSequenceTransport(path, hash_path, basename, now, width, height, groupid, authentifier);
                    break;
            }
        }

        TransportVariant & operator = (TransportVariant const &) = delete;

        ~TransportVariant() {
            this->trans->~Transport();
        }
    } trans_variant;


private:
    struct Serializer final : GraphicToFile {
        Serializer(const timeval & now
                , Transport & trans
                , const uint16_t width
                , const uint16_t height
                , const uint8_t  capture_bpp
                , BmpCache & bmp_cache
                , GlyphCache & gly_cache
                , PointerCache & ptr_cache
                , gdi::DumpPng24Api & dump_png24
                , WrmCompressionAlgorithm wrm_compression_algorithm
                , SendInput send_input = SendInput::NO
                , GraphicToFile::Verbose verbose = GraphicToFile::Verbose::none)
            : GraphicToFile(now, trans, width, height,
                            capture_bpp,
                            bmp_cache, gly_cache, ptr_cache,
                            dump_png24, wrm_compression_algorithm,
                            send_input, verbose)
        {}

        using GraphicToFile::draw;
        using GraphicToFile::capture_bpp;

        void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
            auto compress_and_draw_bitmap_update = [&bitmap_data, this](const Bitmap & bmp) {
                StaticOutStream<65535> bmp_stream;
                bmp.compress(this->capture_bpp, bmp_stream);

                RDPBitmapData target_bitmap_data = bitmap_data;

                target_bitmap_data.bits_per_pixel = bmp.bpp();
                target_bitmap_data.flags          = BITMAP_COMPRESSION | NO_BITMAP_COMPRESSION_HDR;
                target_bitmap_data.bitmap_length  = bmp_stream.get_offset();

                GraphicToFile::draw(target_bitmap_data, bmp);
            };

            if (bmp.bpp() > this->capture_bpp) {
                // reducing the color depth of image.
                Bitmap capture_bmp(this->capture_bpp, bmp);
                compress_and_draw_bitmap_update(capture_bmp);
            }
            else if (!bmp.has_data_compressed()) {
                compress_and_draw_bitmap_update(bmp);
            }
            else {
                GraphicToFile::draw(bitmap_data, bmp);
            }
        }

        void enable_kbd_input_mask(bool) override {}

        bool kbd_input(const timeval & now, uint32_t uchar) override {
            return this->GraphicToFile::kbd_input(now, uchar);
        }
    } graphic_to_file;

public:

    // EXTERNAL CAPTURE API
    void external_breakpoint() override {
        this->nc.external_breakpoint();
    }

    void external_time(timeval const & now) override {
        this->nc.external_time(now);
    }

    // CAPTURE PROBE API
    void session_update(timeval const & now, array_view_const_char message) override {
        this->graphic_to_file.session_update(now, message);
    }
    void possible_active_window_change() override { this->graphic_to_file.possible_active_window_change(); }

    // GRAPHIC API
    void draw(RDP::FrameMarker    const & cmd) override { this->graphic_to_file.draw(cmd);}
    void draw(RDPDestBlt          const & cmd, Rect clip) override {this->graphic_to_file.draw(cmd, clip);}
    void draw(RDPMultiDstBlt      const & cmd, Rect clip) override {this->graphic_to_file.draw(cmd, clip);}
    void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPScrBlt           const & cmd, Rect clip) override {this->graphic_to_file.draw(cmd, clip);}
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override {this->graphic_to_file.draw(cmd, clip);}
    void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx);
    }
    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override {
        this->graphic_to_file.draw(cmd, bmp);
    }
    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override {
        this->graphic_to_file.draw(cmd, clip, bmp);
    }
    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx, bmp);
    }
    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override {
        this->graphic_to_file.draw(cmd, clip, color_ctx, gly_cache);
    }
    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::WindowIcon                     & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::CachedIcon                     & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::DeletedWindow                  & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(RDPColCache   const & cmd) override {
        this->graphic_to_file.draw(cmd);
    }
    void draw(RDPBrushCache const & cmd) override {
        this->graphic_to_file.draw(cmd);
    }

    class NativeCaptureLocal : public gdi::CaptureApi, public gdi::ExternalCaptureApi
    {
        timeval start_native_capture;
        uint64_t inter_frame_interval_native_capture;

        timeval start_break_capture;
        uint64_t inter_frame_interval_start_break_capture;

        GraphicToFile & recorder;
        uint64_t time_to_wait;

    public:
        NativeCaptureLocal(
            GraphicToFile & recorder,
            const timeval & now,
            std::chrono::duration<unsigned int, std::ratio<1, 100>> frame_interval,
            std::chrono::seconds break_interval
        )
        : start_native_capture(now)
        , inter_frame_interval_native_capture(
            std::chrono::duration_cast<std::chrono::microseconds>(frame_interval).count())
        , start_break_capture(now)
        , inter_frame_interval_start_break_capture(
            std::chrono::duration_cast<std::chrono::microseconds>(break_interval).count())
        , recorder(recorder)
        , time_to_wait(0)
        {}

        ~NativeCaptureLocal() override {
            this->recorder.sync();
        }

        // toggles externally genareted breakpoint.
        void external_breakpoint() override {
            this->recorder.breakpoint();
        }

        void external_time(const timeval & now) override {
            this->recorder.sync();
            this->recorder.timestamp(now);
        }

    private:
        std::chrono::microseconds do_snapshot(
            const timeval & now, int x, int y, bool ignore_frame_in_timeval
        ) override {
            (void)ignore_frame_in_timeval;
            if (difftimeval(now, this->start_native_capture)
                    >= this->inter_frame_interval_native_capture) {
                this->recorder.timestamp(now);
                this->time_to_wait = this->inter_frame_interval_native_capture;
                this->recorder.mouse(static_cast<uint16_t>(x), static_cast<uint16_t>(y));
                this->start_native_capture = now;
                if ((difftimeval(now, this->start_break_capture) >=
                     this->inter_frame_interval_start_break_capture)) {
                    this->recorder.breakpoint();
                    this->start_break_capture = now;
                }
            }
            else {
                this->time_to_wait = this->inter_frame_interval_native_capture - difftimeval(now, this->start_native_capture);
            }
            return std::chrono::microseconds{this->time_to_wait};
        }

    } nc;

    bool kbd_input_mask_enabled;

public:
    WrmCaptureImpl(const timeval & now, const WrmParams wrm_params, auth_api * authentifier, RDPDrawable & drawable)
    : bmp_cache(
        BmpCache::Recorder, wrm_params.capture_bpp, 3, false,
        BmpCache::CacheOption(600, 768, false),
        BmpCache::CacheOption(300, 3072, false),
        BmpCache::CacheOption(262, 12288, false))
    , ptr_cache(/*pointerCacheSize=*/0x19)
    , dump_png24_api{drawable}
    , trans_variant(
        wrm_params.trace_type, wrm_params.cctx, wrm_params.rnd, wrm_params.record_path, wrm_params.hash_path, wrm_params.basename, now,
        drawable.width(), drawable.height(), wrm_params.groupid, authentifier)
    , graphic_to_file(
        now, *this->trans_variant.trans, drawable.width(), drawable.height(), wrm_params.capture_bpp,
        this->bmp_cache, this->gly_cache, this->ptr_cache, this->dump_png24_api,
        wrm_params.wrm_compression_algorithm, GraphicToFile::SendInput::YES,
        GraphicToFile::Verbose(wrm_params.wrm_verbose)
    )
    , nc(this->graphic_to_file, now, wrm_params.frame_interval, wrm_params.break_interval)
    , kbd_input_mask_enabled{false}
    {}

    // shadow text
    bool kbd_input(const timeval& now, uint32_t uchar) override {
        return this->graphic_to_file.kbd_input(now, this->kbd_input_mask_enabled?'*':uchar);
    }

    void enable_kbd_input_mask(bool enable) override {
        this->kbd_input_mask_enabled = enable;
    }


    void enable_keyboard_log()
    {
        this->kbd_input_mask_enabled = false;
    }

    void send_timestamp_chunk(timeval const & now, bool ignore_time_interval) {
        this->graphic_to_file.timestamp(now);
        this->graphic_to_file.send_timestamp_chunk(ignore_time_interval);
    }

    void request_full_cleaning() {
        this->trans_variant.trans->request_full_cleaning();
    }

    std::chrono::microseconds do_snapshot(
        const timeval & now, int x, int y, bool ignore_frame_in_timeval
    ) override {
        return this->nc.periodic_snapshot(now, x, y, ignore_frame_in_timeval);
    }

};

class Capture final
: public gdi::GraphicApi
, public gdi::CaptureApi
, public gdi::KbdInputApi
, public gdi::CaptureProbeApi
, public gdi::ExternalCaptureApi
{
    const bool is_replay_mod;

    using string_view = array_view_const_char;

public:
    void draw(RDP::FrameMarker    const & cmd) override { this->draw_impl( cmd); }
    void draw(RDPDestBlt          const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPMultiDstBlt      const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPScrBlt           const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
    void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
    void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override { this->draw_impl(cmd, bmp); }
    void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override { this->draw_impl(cmd, clip, bmp);}
    void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override { this->draw_impl(cmd, clip, color_ctx, bmp); }
    void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override { this->draw_impl(cmd, clip, color_ctx, gly_cache); }

    void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::WindowIcon                     & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::CachedIcon                     & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::DeletedWindow                  & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override { this->draw_impl(cmd); }
    void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override { this->draw_impl(cmd); }

    void draw(RDPColCache   const & cmd) override { this->draw_impl(cmd); }
    void draw(RDPBrushCache const & cmd) override { this->draw_impl(cmd); }
private:
    // Title changed
    //@{
    struct TitleChangedFunctions final : NotifyTitleChanged
    {
        Capture & capture;

        TitleChangedFunctions(Capture & capture) : capture(capture) {}

        void notify_title_changed(timeval const & now, string_view title) override
        {
            if (this->capture.patterns_checker) {
                this->capture.patterns_checker->operator()(title);
            }
            if (this->capture.meta_capture_obj) {
                this->capture.meta_capture_obj->get_session_meta().title_changed(now.tv_sec, title);
            }
            if (this->capture.sequenced_video_capture_obj) {
                this->capture.sequenced_video_capture_obj->next_video(now);
            }
            if (this->capture.update_progress_data) {
                this->capture.update_progress_data->next_video(now.tv_sec);
            }
        }

    } notifier_title_changed{*this};
    //@}

    // Next video
    //@{
    struct NotifyMetaIfNextVideo final : NotifyNextVideo
    {
        SessionMeta * session_meta = nullptr;

        void notify_next_video(const timeval& now, NotifyNextVideo::reason reason) override
        {
            assert(this->session_meta);
            if (reason == NotifyNextVideo::reason::sequenced) {
                this->session_meta->send_line(now.tv_sec, cstr_array_view("(break)"));
            }
        }
    } notifier_next_video;
    struct NullNotifyNextVideo final : NotifyNextVideo
    {
        void notify_next_video(const timeval&, NotifyNextVideo::reason) override {}
    } null_notifier_next_video;
    //@}

public:

    RDPDrawable * gd_drawable;

private:

    struct Graphic final : public gdi::GraphicApi
    {
    public:
        void draw(RDP::FrameMarker    const & cmd) override { this->draw_impl(cmd); }
        void draw(RDPDestBlt          const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDPMultiDstBlt      const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDPPatBlt           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDP::RDPMultiPatBlt const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPOpaqueRect       const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPMultiOpaqueRect  const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPScrBlt           const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDP::RDPMultiScrBlt const & cmd, Rect clip) override { this->draw_impl(cmd, clip); }
        void draw(RDPLineTo           const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPPolygonSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPPolygonCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPPolyline         const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPEllipseSC        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPEllipseCB        const & cmd, Rect clip, gdi::ColorCtx color_ctx) override { this->draw_impl(cmd, clip, color_ctx); }
        void draw(RDPBitmapData       const & cmd, Bitmap const & bmp) override { this->draw_impl(cmd, bmp); }
        void draw(RDPMemBlt           const & cmd, Rect clip, Bitmap const & bmp) override { this->draw_impl(cmd, clip, bmp);}
        void draw(RDPMem3Blt          const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override { this->draw_impl(cmd, clip, color_ctx, bmp); }
        void draw(RDPGlyphIndex       const & cmd, Rect clip, gdi::ColorCtx color_ctx, GlyphCache const & gly_cache) override { this->draw_impl(cmd, clip, color_ctx, gly_cache); }

        void draw(const RDP::RAIL::NewOrExistingWindow            & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::WindowIcon                     & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::CachedIcon                     & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::DeletedWindow                  & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::NewOrExistingNotificationIcons & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::DeletedNotificationIcons       & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::ActivelyMonitoredDesktop       & cmd) override { this->draw_impl(cmd); }
        void draw(const RDP::RAIL::NonMonitoredDesktop            & cmd) override { this->draw_impl(cmd); }

        void draw(RDPColCache   const & cmd) override { this->draw_impl(cmd); }
        void draw(RDPBrushCache const & cmd) override { this->draw_impl(cmd); }

        void set_pointer(Pointer    const & pointer) override {
            for (gdi::GraphicApi & gd : this->gds){
                gd.set_pointer(pointer);
            }
        }

        void set_palette(BGRPalette const & palette) override {
            for (gdi::GraphicApi & gd : this->gds){
                gd.set_palette(palette);
            }
        }

        void sync() override {
            for (gdi::GraphicApi & gd : this->gds){
                gd.sync();
            }
        }

        void set_row(std::size_t rownum, const uint8_t * data) override {
            for (gdi::GraphicApi & gd : this->gds){
                gd.set_row(rownum, data);
            }
        }

        void begin_update() override {
            for (gdi::GraphicApi & gd : this->gds){
                gd.begin_update();
            }
        }

        void end_update() override {
            for (gdi::GraphicApi & gd : this->gds){
                gd.end_update();
            }
        }

    private:
        template<class... Ts>
        void draw_impl(Ts const & ... args) {
            for (gdi::GraphicApi & gd : this->gds){
                gd.draw(args...);
            }
        }

        void draw_impl(RDP::FrameMarker const & cmd) {
            for (gdi::GraphicApi & gd : this->gds) {
                gd.draw(cmd);
            }

            if (cmd.action == RDP::FrameMarker::FrameEnd) {
                for (gdi::CaptureApi & cap : this->caps) {
                    cap.frame_marker_event(this->mouse.last_now, this->mouse.last_x, this->mouse.last_y, false);
                }
            }
        }

    public:
        MouseTrace const & mouse;
        const std::vector<std::reference_wrapper<gdi::GraphicApi>> & gds;
        const std::vector<std::reference_wrapper<gdi::CaptureApi>> & caps;

        Graphic(MouseTrace const & mouse,
                const std::vector<std::reference_wrapper<gdi::GraphicApi>> & gds,
                const std::vector<std::reference_wrapper<gdi::CaptureApi>> & caps)
        : mouse(mouse)
        , gds(gds)
        , caps(caps)
        {}
    };

    std::unique_ptr<Graphic> graphic_api;

    std::unique_ptr<WrmCaptureImpl> wrm_capture_obj;
    std::unique_ptr<PngCapture> png_capture_obj;
    std::unique_ptr<PngCaptureRT> png_capture_real_time_obj;

    std::unique_ptr<SyslogKbd> syslog_kbd_capture_obj;
    std::unique_ptr<SessionLogKbd> session_log_kbd_capture_obj;
    std::unique_ptr<PatternKbd> pattern_kbd_capture_obj;

    std::unique_ptr<SequencedVideoCaptureImpl> sequenced_video_capture_obj;
    std::unique_ptr<FullVideoCaptureImpl> full_video_capture_obj;
    std::unique_ptr<MetaCaptureImpl> meta_capture_obj;
    std::unique_ptr<TitleCaptureImpl> title_capture_obj;
    std::unique_ptr<PatternsChecker> patterns_checker;

    UpdateProgressData * update_progress_data;

    MouseTrace mouse_info;
    wait_obj capture_event;

    std::vector<std::reference_wrapper<gdi::GraphicApi>> gds;
    // Objects willing to be warned of FrameMarker Events
    std::vector<std::reference_wrapper<gdi::CaptureApi>> caps;
    std::vector<std::reference_wrapper<gdi::KbdInputApi>> kbds;
    std::vector<std::reference_wrapper<gdi::CaptureProbeApi>> probes;
    std::vector<std::reference_wrapper<gdi::ExternalCaptureApi>> objs;

    bool capture_drawable = false;


public:
    Capture(
        bool capture_wrm, const WrmParams wrm_params,
        bool capture_png, const PngParams png_params,
        bool capture_pattern_checker, const PatternCheckerParams pattern_checker_params,
        bool capture_ocr, const OcrParams ocr_params,
        bool capture_flv, const SequencedVideoParams sequenced_video_params,
        bool capture_flv_full, const FullVideoParams full_video_params,
        bool capture_meta, const MetaParams meta_params,
        bool capture_kbd, const KbdLogParams kbd_log_params,
        const char * basename,
        const timeval & now,
        int width,
        int height,
        int order_bpp,
        int capture_bpp,
        const char * record_tmp_path,
        const char * record_path,
        const int groupid,
        const FlvParams flv_params,
        bool no_timestamp,
        auth_api * authentifier,
        UpdateProgressData * update_progress_data,
        const char * pattern_kill,
        const char * pattern_notify,
        int debug_capture,
        bool flv_capture_chunk,
        bool meta_enable_session_log,
        const std::chrono::duration<long int> flv_break_interval,
        bool syslog_keyboard_log,
        bool rt_display,
        bool disable_keyboard_log,
        bool session_log_enabled,
        bool keyboard_fully_masked,
        bool meta_keyboard_log)
    : is_replay_mod(!authentifier)
    , gd_drawable(nullptr)
    , update_progress_data(update_progress_data)
    , mouse_info{now, width / 2, height / 2}
    , capture_event{}
    , capture_drawable(capture_wrm || capture_flv || capture_ocr || capture_png || capture_flv_full)
    {
        REDASSERT(authentifier ? order_bpp == capture_bpp : true);

        if (capture_png || (authentifier && (capture_flv || capture_ocr))) {
            if (recursive_create_directory(record_tmp_path, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP, -1) != 0) {
                LOG(LOG_INFO, "Failed to create directory: \"%s\"", record_tmp_path);
            }
        }

        if (capture_wrm || capture_flv || capture_ocr || capture_png || capture_flv_full) {
            this->gd_drawable = new RDPDrawable(width, height);
            this->gds.push_back(*this->gd_drawable);

            this->graphic_api.reset(new Graphic(this->mouse_info, this->gds, this->caps));

            if (capture_png) {
                if (png_params.real_time_image_capture) {
                    this->png_capture_real_time_obj.reset(new PngCaptureRT(
                        now, *this->gd_drawable, png_params));
                }
                else {
                    this->png_capture_obj.reset(new PngCapture(
                        now, *this->gd_drawable, png_params));
                }
            }

            if (capture_wrm) {
                this->wrm_capture_obj.reset(new WrmCaptureImpl(now, wrm_params, authentifier, *this->gd_drawable));
            }

            if (capture_meta) {
                this->meta_capture_obj.reset(new MetaCaptureImpl(
                    now, record_tmp_path, basename,
                    meta_enable_session_log
                ));
            }

            if (capture_flv) {

                std::reference_wrapper<NotifyNextVideo> notifier = this->null_notifier_next_video;
                if (flv_capture_chunk && this->meta_capture_obj) {
                    this->notifier_next_video.session_meta = &this->meta_capture_obj->get_session_meta();
                    notifier = this->notifier_next_video;
                }
                this->sequenced_video_capture_obj.reset(new SequencedVideoCaptureImpl(
                    now, record_path, basename, groupid, no_timestamp, png_params.zoom, *this->gd_drawable,
                    flv_params,
                    flv_break_interval, notifier
                ));
            }

            if (capture_flv_full) {
                this->full_video_capture_obj.reset(new FullVideoCaptureImpl(
                    now, record_path, basename, groupid, no_timestamp, *this->gd_drawable,
                    flv_params));
            }

            if (capture_pattern_checker) {
                this->patterns_checker.reset(new PatternsChecker(
                    *authentifier,
                    pattern_kill,
                    pattern_notify,
                    debug_capture)
                );
                if (!this->patterns_checker->contains_pattern()) {
                    LOG(LOG_WARNING, "Disable pattern_checker");
                    this->patterns_checker.reset();
                }
            }

            if (capture_ocr) {
                if (this->patterns_checker || this->meta_capture_obj || this->sequenced_video_capture_obj) {
                    this->title_capture_obj.reset(new TitleCaptureImpl(
                        now, authentifier, *this->gd_drawable,
                        ocr_params,
                        this->notifier_title_changed
                    ));
                }
                else {
                    LOG(LOG_INFO, "Disable title_extractor");
                }
            }

            if (capture_wrm) {
                this->gds.push_back(static_cast<gdi::GraphicApi&>(*this->wrm_capture_obj));   // gdi::GraphicApi
                this->caps.push_back(static_cast<gdi::CaptureApi&>(*this->wrm_capture_obj));
                this->objs.push_back(*this->wrm_capture_obj);     // gdi::ExternalCaptureApi
                this->probes.push_back(static_cast<gdi::CaptureProbeApi&>(*this->wrm_capture_obj)); // gdi::CaptureProbeApi

                if (!disable_keyboard_log) {
                    this->wrm_capture_obj->enable_keyboard_log();
                }
            }

            if (this->png_capture_real_time_obj) {
                this->png_capture_real_time_obj->enable_rt_display = rt_display;
                this->caps.push_back(static_cast<gdi::CaptureApi&>(*this->png_capture_real_time_obj));
            }

            if (this->png_capture_obj) {
                this->caps.push_back(static_cast<gdi::CaptureApi&>(*this->png_capture_obj));
            }

            if (this->sequenced_video_capture_obj) {
//                this->caps.push_back(this->sequenced_video_capture_obj->vc);
                this->caps.push_back(*this->sequenced_video_capture_obj);
           }

            if (this->full_video_capture_obj) {
                this->caps.push_back(*this->full_video_capture_obj);
            }
        }

        if (capture_kbd) {
            this->syslog_kbd_capture_obj.reset(new SyslogKbd(now));
            this->session_log_kbd_capture_obj.reset(new SessionLogKbd(*authentifier));
            this->pattern_kbd_capture_obj.reset(new PatternKbd(authentifier, pattern_kill, pattern_notify, debug_capture));
        }

        if (this->syslog_kbd_capture_obj.get() && (!syslog_keyboard_log)) {
            this->kbds.push_back(*this->syslog_kbd_capture_obj.get());
            this->caps.push_back(*this->syslog_kbd_capture_obj.get());
        }

        if (this->session_log_kbd_capture_obj.get() && session_log_enabled && keyboard_fully_masked) {
            this->kbds.push_back(*this->session_log_kbd_capture_obj.get());
            this->probes.push_back(*this->session_log_kbd_capture_obj.get());
        }

        if (this->pattern_kbd_capture_obj.get() && this->pattern_kbd_capture_obj->contains_pattern()) {
            this->kbds.push_back(*this->pattern_kbd_capture_obj.get());
        }

        if (this->meta_capture_obj) {
            this->caps.push_back(this->meta_capture_obj->meta);
            if (!meta_keyboard_log) {
                this->kbds.push_back(this->meta_capture_obj->meta);
                this->probes.push_back(this->meta_capture_obj->meta);
            }

            if (this->meta_capture_obj->enable_agent) {
                this->probes.push_back(this->meta_capture_obj->session_log_agent);
            }
        }
        if (this->title_capture_obj) {
            this->caps.push_back(static_cast<gdi::CaptureApi&>(*this->title_capture_obj));
            this->probes.push_back(static_cast<gdi::CaptureProbeApi&>(*this->title_capture_obj));
        }
    }

    ~Capture() {
        if (this->is_replay_mod) {
            this->png_capture_obj.reset();
            if (this->png_capture_real_time_obj) { this->png_capture_real_time_obj.reset(); }
            this->wrm_capture_obj.reset();
            if (this->sequenced_video_capture_obj) {
                try {
                    this->sequenced_video_capture_obj->encoding_video_frame();
                }
                catch (Error const &) {
                    this->sequenced_video_capture_obj->request_full_cleaning();
                    if (this->meta_capture_obj) {
                        this->meta_capture_obj->request_full_cleaning();
                    }
                }
                this->sequenced_video_capture_obj.reset();
            }
            if (this->full_video_capture_obj) {
                try {
                    this->full_video_capture_obj->encoding_video_frame();
                }
                catch (Error const &) {
                    this->full_video_capture_obj->request_full_cleaning();
                }
                this->full_video_capture_obj.reset();
            }
        }
        else {
            this->title_capture_obj.reset();
            this->session_log_kbd_capture_obj.reset();
            this->syslog_kbd_capture_obj.reset();
            this->pattern_kbd_capture_obj.reset();
            this->sequenced_video_capture_obj.reset();
            this->png_capture_obj.reset();
            if (this->png_capture_real_time_obj) { this->png_capture_real_time_obj.reset(); }

            if (this->wrm_capture_obj) {
                timeval now = tvtime();
                this->wrm_capture_obj->send_timestamp_chunk(now, false);
                this->wrm_capture_obj.reset();
            }
        }
    }

    wait_obj & get_capture_event() {
        return this->capture_event;
    }

    public:
    // TODO: this could be done directly in external png_capture_real_time_obj object
    void update_config(bool enable_rt_display) {
        if (this->png_capture_real_time_obj) {
            this->png_capture_real_time_obj->update_config(enable_rt_display);
        }
    }

    void set_row(size_t rownum, const uint8_t * data) override {
        if (this->capture_drawable) {
            this->gd_drawable->set_row(rownum, data);
        }
    }

    void sync() override
    {
        if (this->capture_drawable) {
            this->graphic_api->sync();
        }
    }

    bool kbd_input(timeval const & now, uint32_t uchar) override {
        bool ret = true;
        for (gdi::KbdInputApi & kbd : this->kbds) {
            ret &= kbd.kbd_input(now, uchar);
        }
        return ret;
    }

    void enable_kbd_input_mask(bool enable) override {
        for (gdi::KbdInputApi & kbd : this->kbds) {
            kbd.enable_kbd_input_mask(enable);
        }
    }

    gdi::GraphicApi * get_graphic_api() const {
        return this->graphic_api.get();
    }

    void add_graphic(gdi::GraphicApi & gd) {
        if (this->capture_drawable) {
            this->gds.push_back(gd);
        }
    }

protected:
    std::chrono::microseconds do_snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override {
        this->capture_event.reset();

        if (this->gd_drawable) {
            this->gd_drawable->set_mouse_cursor_pos(cursor_x, cursor_y);
        }
        this->mouse_info = {now, cursor_x, cursor_y};

        std::chrono::microseconds time = std::chrono::microseconds::max();
        if (!this->caps.empty()) {
            for (gdi::CaptureApi & cap : this->caps) {
                time = std::min(time, cap.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval));
            }
            this->capture_event.update(time.count());
        }
        return time;
    }

    template<class... Ts>
    void draw_impl(const Ts & ... args) {
        if (this->capture_drawable) {
            this->graphic_api->draw(args...);
        }
    }

public:
    void set_pointer(const Pointer & cursor) override {
        if (this->capture_drawable) {
            this->graphic_api->set_pointer(cursor);
        }
    }

    void set_palette(const BGRPalette & palette) override {
        if (this->capture_drawable) {
            this->graphic_api->set_palette(palette);
        }
    }

    void set_pointer_display() {
        if (this->capture_drawable) {
            this->gd_drawable->show_mouse_cursor(false);
        }
    }

    void external_breakpoint() override {
        for (gdi::ExternalCaptureApi & obj : this->objs) {
            obj.external_breakpoint();
        }
    }

    void external_time(timeval const & now) override {
        for (gdi::ExternalCaptureApi & obj : this->objs) {
            obj.external_time(now);
        }
    }

    void session_update(const timeval & now, array_view_const_char message) override {
        for (gdi::CaptureProbeApi & cap_prob : this->probes) {
            cap_prob.session_update(now, message);
        }
    }

    void possible_active_window_change() override {
        for (gdi::CaptureProbeApi & cap_prob : this->probes) {
            cap_prob.possible_active_window_change();
        }
    }
};

enum {
    USE_ORIGINAL_COMPRESSION_ALGORITHM = 0xFFFFFFFF
};

enum {
    USE_ORIGINAL_COLOR_DEPTH           = 0xFFFFFFFF
};

inline
static int do_recompress(
    CryptoContext & cctx, Random & rnd, Transport & in_wrm_trans, const timeval begin_record,
    bool & program_requested_to_shutdown,
    int wrm_compression_algorithm_, std::string const & output_filename, Inifile & ini, uint32_t verbose
) {
    FileToChunk player(&in_wrm_trans, to_verbose_flags(verbose));

/*
    char outfile_path     [1024] = PNG_PATH "/"   ; // default value, actual one should come from output_filename
    char outfile_basename [1024] = "redrec_output"; // default value, actual one should come from output_filename
    char outfile_extension[1024] = ""             ; // extension is ignored for targets anyway

    canonical_path( output_filename.c_str()
                  , outfile_path
                  , sizeof(outfile_path)
                  , outfile_basename
                  , sizeof(outfile_basename)
                  , outfile_extension
                  , sizeof(outfile_extension)
                  );
*/
    std::string outfile_path;
    std::string outfile_basename;
    std::string outfile_extension;
    ParsePath(output_filename.c_str(), outfile_path, outfile_basename, outfile_extension);

    if (verbose) {
        std::cout << "Output file path: " << outfile_path << outfile_basename << outfile_extension << '\n' << std::endl;
    }

    if (recursive_create_directory(outfile_path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP, ini.get<cfg::video::capture_groupid>()) != 0) {
        std::cerr << "Failed to create directory: \"" << outfile_path << "\"" << std::endl;
    }

//    if (ini.get<cfg::video::wrm_compression_algorithm>() == USE_ORIGINAL_COMPRESSION_ALGORITHM) {
//        ini.set<cfg::video::wrm_compression_algorithm>(player.info_compression_algorithm);
//    }
    ini.set<cfg::video::wrm_compression_algorithm>(
        (wrm_compression_algorithm_ == static_cast<int>(USE_ORIGINAL_COMPRESSION_ALGORITHM))
        ? player.info_compression_algorithm
        : static_cast<WrmCompressionAlgorithm>(wrm_compression_algorithm_)
    );

    int return_code = 0;
    try {
        if (ini.get<cfg::globals::trace_type>() == TraceType::cryptofile) {
            CryptoOutMetaSequenceTransport trans(
                cctx,
                rnd,
                outfile_path.c_str(),
                ini.get<cfg::video::hash_path>().c_str(),
                outfile_basename.c_str(),
                begin_record,
                player.info_width,
                player.info_height,
                ini.get<cfg::video::capture_groupid>()
            );
            {
                ChunkToFile recorder( &trans
                            , player.info_width
                            , player.info_height
                            , player.info_bpp
                            , player.info_cache_0_entries
                            , player.info_cache_0_size
                            , player.info_cache_1_entries
                            , player.info_cache_1_size
                            , player.info_cache_2_entries
                            , player.info_cache_2_size

                            , player.info_number_of_cache
                            , player.info_use_waiting_list

                            , player.info_cache_0_persistent
                            , player.info_cache_1_persistent
                            , player.info_cache_2_persistent

                            , player.info_cache_3_entries
                            , player.info_cache_3_size
                            , player.info_cache_3_persistent
                            , player.info_cache_4_entries
                            , player.info_cache_4_size
                            , player.info_cache_4_persistent
                            , ini.get<cfg::video::wrm_compression_algorithm>());

                player.add_consumer(&recorder);

                player.play(program_requested_to_shutdown);
            }

            if (program_requested_to_shutdown) {
                trans.request_full_cleaning();
            }
        }
        else {
            OutMetaSequenceTransport trans(
                    outfile_path.c_str(),
                    ini.get<cfg::video::hash_path>().c_str(),
                    outfile_basename.c_str(),
                    begin_record,
                    player.info_width,
                    player.info_height,
                    ini.get<cfg::video::capture_groupid>()
                );
            {
                ChunkToFile recorder( &trans
                            , player.info_width
                            , player.info_height
                            , player.info_bpp
                            , player.info_cache_0_entries
                            , player.info_cache_0_size
                            , player.info_cache_1_entries
                            , player.info_cache_1_size
                            , player.info_cache_2_entries
                            , player.info_cache_2_size

                            , player.info_number_of_cache
                            , player.info_use_waiting_list

                            , player.info_cache_0_persistent
                            , player.info_cache_1_persistent
                            , player.info_cache_2_persistent

                            , player.info_cache_3_entries
                            , player.info_cache_3_size
                            , player.info_cache_3_persistent
                            , player.info_cache_4_entries
                            , player.info_cache_4_size
                            , player.info_cache_4_persistent
                            , ini.get<cfg::video::wrm_compression_algorithm>());

                player.add_consumer(&recorder);

                player.play(program_requested_to_shutdown);
            }

            if (program_requested_to_shutdown) {
                trans.request_full_cleaning();
            }
        }
    }
    catch (...) {
        return_code = -1;
    }

    return return_code;
}   // do_recompress

