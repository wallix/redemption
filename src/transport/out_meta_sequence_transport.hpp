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


#pragma once

#include <cerrno>
#include <fcntl.h>
#include <snappy-c.h>
#include <stdint.h>
#include <unistd.h>
#include <memory>
#include <limits>
#include <cerrno>
#include <ctime>
#include <stdio.h> //snprintf
#include <stdlib.h> //mkostemps
#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include <fcntl.h>
#include <snappy-c.h>
#include <cstddef>
#include <cerrno>


#include "utils/sugar/iter.hpp"
#include "utils/fdbuf.hpp"
#include "utils/log.hpp"
#include "utils/urandom_read.hpp"
#include "utils/genrandom.hpp"
#include "utils/fileutils.hpp"
#include "utils/sugar/exchange.hpp"
#include "core/error.hpp"
#include "acl/auth_api.hpp"

#include "openssl_crypto.hpp"

#include "capture/cryptofile.hpp"
#include "transport/transport.hpp"
#include "transport/sequence_generator.hpp"

#include "utils/log.hpp"



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

namespace transbuf
{
    struct null_buf
    {
        int open() noexcept { return 0; }

        int close() noexcept { return 0; }

        ssize_t write(const void *, size_t len) noexcept { return len; }

        ssize_t read(void *, size_t len) noexcept { return len; }

        int flush() const noexcept { return 0; }
    };
}


namespace detail
{
    struct NoCurrentPath {
        template<class Buf>
        static const char * current_path(Buf &)
        { return nullptr; }
    };

    struct GetCurrentPath {
        template<class Buf>
        static const char * current_path(Buf & buf)
        { return buf.current_path(); }
    };
}

template<class Buf, class PathTraits = detail::NoCurrentPath>
struct RequestCleaningAndNextTransport
: Transport
{
    RequestCleaningAndNextTransport() = default;

    template<class Params>
    explicit RequestCleaningAndNextTransport(const Params & buf_params)
    : buf(buf_params)
    {}

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

    ~RequestCleaningAndNextTransport() {
        this->buf.close();
    }

private:
    void do_send(const uint8_t * data, size_t len) override {
        const ssize_t res = this->buf.write(data, len);
        if (res < 0) {
            this->status = false;
            if (errno == ENOSPC) {
                char message[1024];
                const char * filename = PathTraits::current_path(this->buf);
                snprintf(message, sizeof(message), "100|%s", filename ? filename : "unknow");
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
    Buf & buffer() noexcept
    { return this->buf; }

    const Buf & buffer() const noexcept
    { return this->buf; }

    typedef RequestCleaningAndNextTransport TransportType;

private:
    Buf buf;
};


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

class ochecksum_buf_ofile_buf_out
: public transbuf::ofile_buf_out
{
    struct HMac
    {
        HMAC_CTX hmac;
        bool initialized = false;

        HMac() = default;

        ~HMac() {
            if (this->initialized) {
                HMAC_CTX_cleanup(&this->hmac);
            }
        }

        void init(const uint8_t * const crypto_key, size_t key_len) {
            HMAC_CTX_init(&this->hmac);
            if (!HMAC_Init_ex(&this->hmac, crypto_key, key_len, EVP_sha256(), nullptr)) {
                throw Error(ERR_SSL_CALL_HMAC_INIT_FAILED);
            }
            this->initialized = true;
        }

        void update(const void * const data, size_t data_size) {
            assert(this->initialized);
            if (!HMAC_Update(&this->hmac, reinterpret_cast<uint8_t const *>(data), data_size)) {
                throw Error(ERR_SSL_CALL_HMAC_UPDATE_FAILED);
            }
        }

        void final(uint8_t (&out_data)[SHA256_DIGEST_LENGTH]) {
            assert(this->initialized);
            unsigned int len = 0;
            if (!HMAC_Final(&this->hmac, out_data, &len)) {
                throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
            }
            this->initialized = false;
        }
    };

    static constexpr size_t nosize = ~size_t{};
    static constexpr size_t quick_size = 4096;

    HMac hmac;
    HMac quick_hmac;
    unsigned char const (&hmac_key)[MD_HASH_LENGTH];
    size_t file_size = nosize;

public:
    explicit ochecksum_buf_ofile_buf_out(unsigned char const (&hmac_key)[MD_HASH_LENGTH])
    : hmac_key(hmac_key)
    {}

    ochecksum_buf_ofile_buf_out(ochecksum_buf_ofile_buf_out const &) = delete;
    ochecksum_buf_ofile_buf_out & operator=(ochecksum_buf_ofile_buf_out const &) = delete;

    template<class... Ts>
    int open(Ts && ... args)
    {
        this->hmac.init(this->hmac_key, sizeof(this->hmac_key));
        this->quick_hmac.init(this->hmac_key, sizeof(this->hmac_key));
        int ret = this->transbuf::ofile_buf_out::open(args...);
        this->file_size = 0;
        return ret;
    }

    ssize_t write(const void * data, size_t len)
    {
        REDASSERT(this->file_size != nosize);
        this->hmac.update(data, len);
        if (this->file_size < quick_size) {
            auto const remaining = std::min(quick_size - this->file_size, len);
            this->quick_hmac.update(data, remaining);
            this->file_size += remaining;
        }
        return this->transbuf::ofile_buf_out::write(data, len);
    }

    int close(unsigned char (&hash)[MD_HASH_LENGTH * 2])
    {
        REDASSERT(this->file_size != nosize);
        this->quick_hmac.final(reinterpret_cast<unsigned char(&)[MD_HASH_LENGTH]>(hash[0]));
        this->hmac.final(reinterpret_cast<unsigned char(&)[MD_HASH_LENGTH]>(hash[MD_HASH_LENGTH]));
        this->file_size = nosize;
        return this->transbuf::ofile_buf_out::close();
    }

    int close() {
        return this->transbuf::ofile_buf_out::close();
    }
};

class ochecksum_buf_null_buf
: public transbuf::null_buf
{
    struct HMac
    {
        HMAC_CTX hmac;
        bool initialized = false;

        HMac() = default;

        ~HMac() {
            if (this->initialized) {
                HMAC_CTX_cleanup(&this->hmac);
            }
        }

        void init(const uint8_t * const crypto_key, size_t key_len) {
            HMAC_CTX_init(&this->hmac);
            if (!HMAC_Init_ex(&this->hmac, crypto_key, key_len, EVP_sha256(), nullptr)) {
                throw Error(ERR_SSL_CALL_HMAC_INIT_FAILED);
            }
            this->initialized = true;
        }

        void update(const void * const data, size_t data_size) {
            assert(this->initialized);
            if (!HMAC_Update(&this->hmac, reinterpret_cast<uint8_t const *>(data), data_size)) {
                throw Error(ERR_SSL_CALL_HMAC_UPDATE_FAILED);
            }
        }

        void final(uint8_t (&out_data)[SHA256_DIGEST_LENGTH]) {
            assert(this->initialized);
            unsigned int len = 0;
            if (!HMAC_Final(&this->hmac, out_data, &len)) {
                throw Error(ERR_SSL_CALL_HMAC_FINAL_FAILED);
            }
            this->initialized = false;
        }
    };

    static constexpr size_t nosize = ~size_t{};
    static constexpr size_t quick_size = 4096;

    HMac hmac;
    HMac quick_hmac;
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
        int ret = this->transbuf::null_buf::open(args...);
        this->file_size = 0;
        return ret;
    }

    ssize_t write(const void * data, size_t len)
    {
        REDASSERT(this->file_size != nosize);
        this->hmac.update(data, len);
        if (this->file_size < quick_size) {
            auto const remaining = std::min(quick_size - this->file_size, len);
            this->quick_hmac.update(data, remaining);
            this->file_size += remaining;
        }
        return this->transbuf::null_buf::write(data, len);
    }

    int close(unsigned char (&hash)[MD_HASH_LENGTH * 2])
    {
        REDASSERT(this->file_size != nosize);
        this->quick_hmac.final(reinterpret_cast<unsigned char(&)[MD_HASH_LENGTH]>(hash[0]));
        this->hmac.final(reinterpret_cast<unsigned char(&)[MD_HASH_LENGTH]>(hash[MD_HASH_LENGTH]));
        this->file_size = nosize;
        return this->transbuf::null_buf::close();
    }

    int close() {
        return this->transbuf::null_buf::close();
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

    template<class Buf>
    class out_sequence_filename_buf_impl
    {
        char current_filename_[1024];
        FilenameGenerator filegen_;
        Buf buf_;
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

    template<class HashBuf>
    int write_meta_hash(
        char const * hash_filename, char const * meta_filename,
        HashBuf & crypto_hash, hash_type const * hash
    ) {
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
                err = write_meta_file_impl<false>(crypto_hash, filename, stat, 0, 0, hash);
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

    public:
        template<class MetaParams>
        explicit out_meta_sequence_filename_buf_impl(
            out_meta_sequence_filename_buf_param<MetaParams> const & params
        )
        : sequence_base_type(params.sq_params)
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
                transbuf::ofile_buf_out ofile;
                return write_meta_hash(
                    this->hash_filename(), this->meta_filename(),
                    ofile, nullptr
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
}


struct OutFilenameSequenceTransport
: RequestCleaningAndNextTransport<
    detail::out_sequence_filename_buf_impl<
        detail::empty_ctor<io::posix::fdbuf>
>>
{
    OutFilenameSequenceTransport(
        FilenameGenerator::Format format,
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid,
        auth_api * authentifier = nullptr)
    : OutFilenameSequenceTransport::TransportType(
        detail::out_sequence_filename_buf_param(format, prefix, filename, extension, groupid))
    {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }
    }

    const FilenameGenerator * seqgen() const noexcept
    { return &(this->buffer().seqgen()); }
};

struct OutFilenameSequenceSeekableTransport
: RequestCleaningAndNextTransport<
    detail::out_sequence_filename_buf_impl<
        detail::empty_ctor<io::posix::fdbuf>
>>
{
    OutFilenameSequenceSeekableTransport(
        FilenameGenerator::Format format,
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid,
        auth_api * authentifier = nullptr)
    : OutFilenameSequenceSeekableTransport::TransportType(
        detail::out_sequence_filename_buf_param(format, prefix, filename, extension, groupid))
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

    template<class Buf, class BufFilter, class BufMeta, class BufHash, class Params>
    class out_hash_meta_sequence_filename_buf_impl
    : public out_meta_sequence_filename_buf_impl<Buf, BufMeta>
    {
        CryptoContext & cctx;
        Params hash_ctx;
        BufFilter wrm_filter;

        using sequence_base_type = out_meta_sequence_filename_buf_impl<Buf, BufMeta>;

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

            return write_meta_hash(this->hash_filename(), this->meta_filename(), hash_buf, &hash);
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

    struct cctx_ochecksum_file
    : transbuf::ochecksum_buf_ofile_buf_out
    {
        explicit cctx_ochecksum_file(CryptoContext & cctx)
        : transbuf::ochecksum_buf_ofile_buf_out(cctx.get_hmac_key())
        {}
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


struct OutMetaSequenceTransport
: RequestCleaningAndNextTransport<
    detail::out_meta_sequence_filename_buf_impl<
        detail::empty_ctor<io::posix::fdbuf>,
        detail::empty_ctor<transbuf::ofile_buf_out>
    >,
    detail::GetCurrentPath
>
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
    : OutMetaSequenceTransport::TransportType(detail::out_meta_sequence_filename_buf_param<>(
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
};


struct OutMetaSequenceTransportWithSum
: RequestCleaningAndNextTransport<
    detail::out_hash_meta_sequence_filename_buf_impl<
        detail::empty_ctor<io::posix::fdbuf>,
        detail::ochecksum_filter,
        detail::cctx_ochecksum_file,
        detail::cctx_ofile_buf,
        CryptoContext&
    >,
    detail::GetCurrentPath
>
{
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
    : OutMetaSequenceTransportWithSum::TransportType(
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
};


struct CryptoOutMetaSequenceTransport
: RequestCleaningAndNextTransport<
    detail::out_hash_meta_sequence_filename_buf_impl<
        detail::empty_ctor<io::posix::fdbuf>,
        detail::ocrypto_filter,
        transbuf::ocrypto_filename_buf,
        transbuf::ocrypto_filename_buf,
        transbuf::ocrypto_filename_params
    >,
    detail::GetCurrentPath
>
{
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
    : CryptoOutMetaSequenceTransport::TransportType(
        detail::out_hash_meta_sequence_filename_buf_param<transbuf::ocrypto_filename_params>(
            crypto_ctx,
            now.tv_sec, format, hash_path, path, basename, ".wrm", groupid,
            {crypto_ctx, rnd}
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
};
