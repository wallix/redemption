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
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Jonatan Poelen
*/

#pragma once


#include "utils/log.hpp"
#include "utils/difftimeval.hpp"

#include "gdi/capture_api.hpp"
#include "capture/wrm_params.hpp"

#include <cerrno>
#include <cstddef>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gdi/kbd_input_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/capture_probe_api.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/caches/bmpcache.hpp"
#include "core/RDP/caches/pointercache.hpp"

#include "gdi/dump_png24.hpp"
#include "core/RDP/RDPDrawable.hpp"

struct wrmcapture_no_param {};

class wrmcapture_ofile_buf_out
{
    int fd;
public:
    wrmcapture_ofile_buf_out() : fd(-1) {}
    ~wrmcapture_ofile_buf_out()
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


struct wrmcapture_FilenameGenerator
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
    wrmcapture_FilenameGenerator(
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
    wrmcapture_FilenameGenerator(wrmcapture_FilenameGenerator const &) = delete;
    wrmcapture_FilenameGenerator& operator=(wrmcapture_FilenameGenerator const &) = delete;
};

typedef wrmcapture_FilenameGenerator::Format FilenameFormat;

struct wrmcapture_out_sequence_filename_buf_param
{
    wrmcapture_FilenameGenerator::Format format;
    const char * const prefix;
    const char * const filename;
    const char * const extension;
    const int groupid;

    wrmcapture_out_sequence_filename_buf_param(
        wrmcapture_FilenameGenerator::Format format,
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


template<class MetaParams = no_param>
struct wrmcapture_out_meta_sequence_filename_buf_param
{
    wrmcapture_out_sequence_filename_buf_param sq_params;
    time_t sec;
    MetaParams meta_buf_params;
    const char * hash_prefix;

    wrmcapture_out_meta_sequence_filename_buf_param(
        time_t start_sec,
        wrmcapture_FilenameGenerator::Format format,
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


template<class FilterParams = wrmcapture_no_param>
struct wrmcapture_out_hash_meta_sequence_filename_buf_param
{
    wrmcapture_out_meta_sequence_filename_buf_param<FilterParams> meta_sq_params;
    FilterParams filter_params;
    CryptoContext & cctx;

    wrmcapture_out_hash_meta_sequence_filename_buf_param(
        CryptoContext & cctx,
        time_t start_sec,
        wrmcapture_FilenameGenerator::Format format,
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

struct wrmcapture_MetaFilename
{
    char filename[2048];

    wrmcapture_MetaFilename(const char * path, const char * basename,
                 FilenameFormat format = wrmcapture_FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION)
    {
        int res =
        (   format == wrmcapture_FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION
         || format == wrmcapture_FilenameGenerator::PATH_FILE_PID_EXTENSION)
        ? snprintf(this->filename, sizeof(this->filename)-1, "%s%s-%06u.mwrm", path, basename, unsigned(getpid()))
        : snprintf(this->filename, sizeof(this->filename)-1, "%s%s.mwrm", path, basename);
        if (res > int(sizeof(this->filename) - 6) || res < 0) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
    }

    wrmcapture_MetaFilename(wrmcapture_MetaFilename const &) = delete;
    wrmcapture_MetaFilename & operator = (wrmcapture_MetaFilename const &) = delete;
};

class wrmcapture_out_sequence_filename_buf_impl
{
    char current_filename_[1024];
    wrmcapture_FilenameGenerator filegen_;
    empty_ctor<io::posix::fdbuf> buf_;
    unsigned num_file_;
    int groupid_;

public:
    explicit wrmcapture_out_sequence_filename_buf_impl(wrmcapture_out_sequence_filename_buf_param const & params)
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

    const wrmcapture_FilenameGenerator & seqgen() const noexcept
    { return this->filegen_; }

    empty_ctor<io::posix::fdbuf> & buf() noexcept
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


template<class BufMeta>
class wrmcapture_out_meta_sequence_filename_buf_impl
: public wrmcapture_out_sequence_filename_buf_impl
{
    typedef wrmcapture_out_sequence_filename_buf_impl sequence_base_type;

    BufMeta meta_buf_;
    wrmcapture_MetaFilename mf_;
    wrmcapture_MetaFilename hf_;
    time_t start_sec_;
    time_t stop_sec_;

public:
    template<class MetaParams>
    explicit wrmcapture_out_meta_sequence_filename_buf_impl(
        wrmcapture_out_meta_sequence_filename_buf_param<MetaParams> const & params
    )
    : wrmcapture_out_sequence_filename_buf_impl(params.sq_params)
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
            wrmcapture_ofile_buf_out crypto_hash;

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
                    err = wrmcapture_write_meta_file_impl<false>(crypto_hash, filename, stat, 0, 0, nullptr);
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
    int next_meta_file(wrmcapture_hash_type const * hash = nullptr)
    {
        // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
        const char * filename = this->rename_filename();
        if (!filename) {
            return 1;
        }

        if (int err = wrmcapture_write_meta_file(
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
        this->wrmcapture_out_sequence_filename_buf_impl::request_full_cleaning();
        ::unlink(this->mf_.filename);
    }

    int flush()
    {
        const int res1 = this->wrmcapture_out_sequence_filename_buf_impl::flush();
        const int res2 = this->meta_buf_.flush();
        return res1 == 0 ? res2 : res1;
    }

    BufMeta & meta_buf() noexcept
    { return this->meta_buf_; }

    void update_sec(time_t sec)
    { this->stop_sec_ = sec; }
};

struct wrmcapture_cctx_ofile_buf
: wrmcapture_ofile_buf_out
{
    explicit wrmcapture_cctx_ofile_buf(CryptoContext &)
    {}
};

class wrmcapture_ochecksum_buf_null_buf
{
    static constexpr size_t nosize = ~size_t{};
    static constexpr size_t quick_size = 4096;

    SslHMAC_Sha256_Delayed hmac;
    SslHMAC_Sha256_Delayed quick_hmac;
    unsigned char const (&hmac_key)[MD_HASH_LENGTH];
    size_t file_size = nosize;

public:
    explicit wrmcapture_ochecksum_buf_null_buf(unsigned char const (&hmac_key)[MD_HASH_LENGTH])
    : hmac_key(hmac_key)
    {}

    wrmcapture_ochecksum_buf_null_buf(wrmcapture_ochecksum_buf_null_buf const &) = delete;
    wrmcapture_ochecksum_buf_null_buf & operator=(wrmcapture_ochecksum_buf_null_buf const &) = delete;

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


class wrmcapture_ochecksum_filter
{
    wrmcapture_ochecksum_buf_null_buf sum_buf;

public:
    explicit wrmcapture_ochecksum_filter(CryptoContext & cctx)
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
    int close(Buf &, wrmcapture_hash_type & hash, unsigned char const (&)[MD_HASH_LENGTH]) {
        return this->sum_buf.close(hash);
    }
};

//class wrmcapture_cctx_ochecksum_file
//{
//    int fd;

//    static constexpr size_t nosize = ~size_t{};
//    static constexpr size_t quick_size = 4096;

//    SslHMAC_Sha256_Delayed hmac;
//    SslHMAC_Sha256_Delayed quick_hmac;
//    unsigned char const (&hmac_key)[MD_HASH_LENGTH];
//    size_t file_size = nosize;

//public:
//    explicit wrmcapture_cctx_ochecksum_file(CryptoContext & cctx)
//    : fd(-1)
//    , hmac_key(cctx.get_hmac_key())
//    {}

//    ~wrmcapture_cctx_ochecksum_file()
//    {
//        this->close();
//    }

//    wrmcapture_cctx_ochecksum_file(wrmcapture_cctx_ochecksum_file const &) = delete;
//    wrmcapture_cctx_ochecksum_file & operator=(wrmcapture_cctx_ochecksum_file const &) = delete;

//    int open(const char * filename, mode_t mode)
//    {
//        this->hmac.init(this->hmac_key, sizeof(this->hmac_key));
//        this->quick_hmac.init(this->hmac_key, sizeof(this->hmac_key));
//        this->file_size = 0;
//        this->fd = ::open(filename, O_WRONLY | O_CREAT, mode);
//        return this->fd;
//    }

//    ssize_t write(const void * data, size_t len)
//    {
//        REDASSERT(this->file_size != nosize);

//        // TODO: hmac returns error as exceptions while write errors are returned as -1
//        // this is inconsistent and probably need a fix.
//        // also, if we choose to raise exception every error should have it's own one
//        this->hmac.update(static_cast<const uint8_t *>(data), len);
//        if (this->file_size < quick_size) {
//            auto const remaining = std::min(quick_size - this->file_size, len);
//            this->quick_hmac.update(static_cast<const uint8_t *>(data), remaining);
//            this->file_size += remaining;
//        }

//        size_t remaining_len = len;
//        size_t total_sent = 0;
//        while (remaining_len) {
//            ssize_t ret = ::write(this->fd,
//                static_cast<const char*>(data) + total_sent, remaining_len);
//            if (ret <= 0){
//                if (errno == EINTR){
//                    continue;
//                }
//                return -1;
//            }
//            remaining_len -= ret;
//            total_sent += ret;
//        }
//        return total_sent;
//    }

//    int close(unsigned char (&hash)[MD_HASH_LENGTH * 2])
//    {
//        REDASSERT(this->file_size != nosize);
//        this->quick_hmac.final(reinterpret_cast<unsigned char(&)[MD_HASH_LENGTH]>(hash[0]));
//        this->hmac.final(reinterpret_cast<unsigned char(&)[MD_HASH_LENGTH]>(hash[MD_HASH_LENGTH]));
//        this->file_size = nosize;
//        return this->close();
//    }

//    int close()
//    {
//        if (this->is_open()) {
//            const int ret = ::close(this->fd);
//            this->fd = -1;
//            return ret;
//        }
//        return 0;
//    }

//    bool is_open() const noexcept
//    { return -1 != this->fd; }

//    off64_t seek(off64_t offset, int whence) const
//    { return ::lseek64(this->fd, offset, whence); }

//    int flush() const
//    { return 0; }
//};


class wrmcapture_out_hash_meta_sequence_filename_buf_impl_cctx
: public wrmcapture_out_meta_sequence_filename_buf_impl_cctx
{
    using BufFilter = wrmcapture_ochecksum_filter;
//    using BufMeta = wrmcapture_cctx_ochecksum_file;
    using BufHash = wrmcapture_cctx_ofile_buf;
    using Params = CryptoContext&;

    CryptoContext & cctx;
    Params hash_ctx;
    BufFilter wrm_filter;

public:
    explicit wrmcapture_out_hash_meta_sequence_filename_buf_impl_cctx(
        wrmcapture_out_hash_meta_sequence_filename_buf_param<Params> const & params
    )
    : wrmcapture_out_meta_sequence_filename_buf_impl_cctx(params.meta_sq_params)
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

        wrmcapture_hash_type hash;

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
                err = wrmcapture_write_meta_file_impl<false>(hash_buf, filename, stat, 0, 0, &hash);
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
            wrmcapture_hash_type hash;
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

template<class Writer>
void wrmcapture_write_meta_headers(Writer & writer, const char * path,
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
int wrmcapture_write_filename(Writer & writer, const char * filename)
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

using wrmcapture_hash_type = unsigned char[MD_HASH_LENGTH*2];

constexpr std::size_t wrmcapture_hash_string_len = (1 + MD_HASH_LENGTH * 2) * 2;

inline char * wrmcapture_swrite_hash(char * p, wrmcapture_hash_type const & hash)
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
int wrmcapture_write_meta_file_impl(
    Writer & writer, const char * filename,
    struct stat const & stat,
    time_t start_sec, time_t stop_sec,
    wrmcapture_hash_type const * hash = nullptr
) {
    if (int err = wrmcapture_write_filename(writer, filename)) {
        return err;
    }

    using ull = unsigned long long;
    using ll = long long;
    char mes[
        (std::numeric_limits<ll>::digits10 + 1 + 1) * 8 +
        (std::numeric_limits<ull>::digits10 + 1 + 1) * 2 +
        wrmcapture_hash_string_len + 1 +
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
        p = wrmcapture_swrite_hash(p, *hash);
    }
    *p++ = '\n';

    ssize_t res = writer.write(mes, p-mes);

    if (res < p-mes) {
        return res < 0 ? res : 1;
    }

    return 0;
}

template<class Writer>
int wrmcapture_write_meta_file(
    Writer & writer, const char * filename,
    time_t start_sec, time_t stop_sec,
    wrmcapture_hash_type const * hash = nullptr
) {
    struct stat stat;
    int err = ::stat(filename, &stat);
    return err ? err : wrmcapture_write_meta_file_impl<true>(writer, filename, stat, start_sec, stop_sec, hash);
}

struct wrmcapture_OutMetaSequenceTransport : public Transport
{
    wrmcapture_OutMetaSequenceTransport(
        const char * path,
        const char * hash_path,
        const char * basename,
        timeval now,
        uint16_t width,
        uint16_t height,
        const int groupid,
        auth_api * authentifier = nullptr,
        FilenameFormat format = wrmcapture_FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
    : buf(wrmcapture_out_meta_sequence_filename_buf_param<>(
        now.tv_sec, format, hash_path, path, basename, ".wrm", groupid
    ))
    {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        wrmcapture_write_meta_headers(this->buffer().meta_buf(), path, width, height, this->authentifier, false);
    }

    void timestamp(timeval now) override {
        this->buffer().update_sec(now.tv_sec);
    }

    const wrmcapture_FilenameGenerator * seqgen() const noexcept
    {
        return &(this->buffer().seqgen());
    }
    using Buf = wrmcapture_out_meta_sequence_filename_buf_impl<wrmcapture_empty_ctor<wrmcapture_ofile_buf_out>>;

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

    ~wrmcapture_OutMetaSequenceTransport() {
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


template<class BufFilter, class BufMeta, class BufHash, class Params>
class wrmcapture_out_hash_meta_sequence_filename_buf_impl
: public wrmcapture_out_meta_sequence_filename_buf_impl<BufMeta>
{
    CryptoContext & cctx;
    Params hash_ctx;
    BufFilter wrm_filter;

    using sequence_base_type = wrmcapture_out_meta_sequence_filename_buf_impl<BufMeta>;

public:
    explicit wrmcapture_out_hash_meta_sequence_filename_buf_impl(
        wrmcapture_out_hash_meta_sequence_filename_buf_param<Params> const & params
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

        wrmcapture_hash_type hash;

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
                err = wrmcapture_write_meta_file_impl<false>(hash_buf, filename, stat, 0, 0, &hash);
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
            wrmcapture_hash_type hash;
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


struct wrmcapture_OutMetaSequenceTransportWithSum : public Transport {

    wrmcapture_OutMetaSequenceTransportWithSum(
        CryptoContext & crypto_ctx,
        const char * path,
        const char * hash_path,
        const char * basename,
        timeval now,
        uint16_t width,
        uint16_t height,
        const int groupid,
        auth_api * authentifier = nullptr,
        FilenameFormat format = wrmcapture_FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
    : buf(
        wrmcapture_out_hash_meta_sequence_filename_buf_param<CryptoContext&>(
            crypto_ctx,
            now.tv_sec, format, hash_path, path, basename, ".wrm", groupid,
            crypto_ctx
        )
    ) {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        wrmcapture_write_meta_headers(this->buffer().meta_buf(), path, width, height, this->authentifier, true);
    }

    void timestamp(timeval now) override {
        this->buffer().update_sec(now.tv_sec);
    }

    const wrmcapture_FilenameGenerator * seqgen() const noexcept
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

    ~wrmcapture_OutMetaSequenceTransportWithSum() {
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
    wrmcapture_out_hash_meta_sequence_filename_buf_impl_cctx & buffer() noexcept
    { return this->buf; }

    const wrmcapture_out_hash_meta_sequence_filename_buf_impl_cctx & buffer() const noexcept
    { return this->buf; }

private:
    wrmcapture_out_hash_meta_sequence_filename_buf_impl_cctx buf;

};


class wrmcapture_ocrypto_filename_buf
{
    transfil::encrypt_filter encrypt;
    CryptoContext & cctx;
    Random & rnd;
    wrmcapture_ofile_buf_out file;

public:
    explicit wrmcapture_ocrypto_filename_buf(wrmcapture_ocrypto_filename_params params)
    : cctx(params.crypto_ctx)
    , rnd(params.rnd)
    {}

    ~wrmcapture_ocrypto_filename_buf()
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

struct wrmcapture_ocrypto_filename_params
{
    CryptoContext & crypto_ctx;
    Random & rnd;
};

struct wrmcapture_ocrypto_filter
: transfil::encrypt_filter
{
    CryptoContext & cctx;
    Random & rnd;

    explicit wrmcapture_ocrypto_filter(dorecompress_ocrypto_filename_params params)
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

struct wrmcapture_CryptoOutMetaSequenceTransport
: public Transport {

    using Buf =
        wrmcapture_out_hash_meta_sequence_filename_buf_impl<
            wrmcapture_ocrypto_filter,
            wrmcapture_ocrypto_filename_buf,
            wrmcapture_ocrypto_filename_buf,
            wrmcapture_ocrypto_filename_params
        >;

    wrmcapture_CryptoOutMetaSequenceTransport(
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
        FilenameFormat format = wrmcapture_FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
    : buf(
        wrmcapture_out_hash_meta_sequence_filename_buf_param<wrmcapture_ocrypto_filename_params>(
            crypto_ctx,
            now.tv_sec, format, hash_path, path, basename, ".wrm", groupid,
            {crypto_ctx, rnd}
        )) {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        wrmcapture_write_meta_headers(this->buffer().meta_buf(), path, width, height, this->authentifier, true);
    }

    void timestamp(timeval now) override {
        this->buffer().update_sec(now.tv_sec);
    }

    const wrmcapture_FilenameGenerator * seqgen() const noexcept
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

    ~wrmcapture_CryptoOutMetaSequenceTransport() {
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

    typedef wrmcapture_CryptoOutMetaSequenceTransport TransportType;

    Buf buf;

};


// TODO temporary
struct DumpPng24FromRDPDrawableAdapter : gdi::DumpPng24Api  {
    RDPDrawable & drawable;

    explicit DumpPng24FromRDPDrawableAdapter(RDPDrawable & drawable) : drawable(drawable) {}

    void dump_png24(Transport& trans, bool bgr) const override {
      ::dump_png24(this->drawable.impl(), trans, bgr);
    }
};


class WrmCaptureImpl :
    public gdi::KbdInputApi,
    public gdi::CaptureApi,
    public gdi::GraphicApi,
    public gdi::CaptureProbeApi,
    public gdi::ExternalCaptureApi // from gdi/capture_api.hpp
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
            wrmcapture_OutMetaSequenceTransportWithSum out_with_sum;
            wrmcapture_CryptoOutMetaSequenceTransport out_crypto;
            wrmcapture_OutMetaSequenceTransport out;

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
                    wrmcapture_CryptoOutMetaSequenceTransport(cctx, rnd, path, hash_path, basename, now, width, height, groupid, authentifier);
                    break;
                case TraceType::localfile_hashed:
                    this->trans = new (&this->variant.out_with_sum)
                    wrmcapture_OutMetaSequenceTransportWithSum(cctx, path, hash_path, basename, now, width, height, groupid, authentifier);
                    break;
                default:
                case TraceType::localfile:
                    this->trans = new (&this->variant.out)
                    wrmcapture_OutMetaSequenceTransport(path, hash_path, basename, now, width, height, groupid, authentifier);
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

