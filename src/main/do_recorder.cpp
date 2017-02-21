/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redver video verifier program
*/

#include "main/do_recorder.hpp"

#include <type_traits>
#include <string>
#include <vector>
#include <utility>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <snappy-c.h>
#include <stdint.h>
#include <unistd.h>
#include "openssl_crypto.hpp"

#include "utils/log.hpp"
#include "transport/transport.hpp"
#include "system/ssl_calls.hpp"

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/exchange.hpp"
#include "utils/sugar/iter.hpp"
#include "utils/chex_to_int.hpp"
#include "utils/fdbuf.hpp"
#include "utils/fileutils.hpp"
#include "utils/urandom_read.hpp"
#include "utils/word_identification.hpp"
#include "configs/config.hpp"
#include "program_options/program_options.hpp"

#include "main/version.hpp"

#include "transport/in_meta_sequence_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "transport/transport.hpp"

#include "acl/auth_api.hpp"
#include "utils/genrandom.hpp"
#include "capture/capture.hpp"
#include "capture/cryptofile.hpp"
#include "utils/apps/recording_progress.hpp"

#include "capture/png_params.hpp"
#include "capture/wrm_params.hpp"
#include "capture/flv_params.hpp"
#include "capture/ocr_params.hpp"
#include "capture/wrm_capture.hpp"

enum {
    USE_ORIGINAL_COMPRESSION_ALGORITHM = 0xFFFFFFFF
};

enum {
    USE_ORIGINAL_COLOR_DEPTH           = 0xFFFFFFFF
};


//template<class MetaParams = wrmcapture_no_param>
//struct wrmcapture_out_meta_sequence_filename_buf_param
//{
//    wrmcapture_out_sequence_filename_buf_param sq_params;
//    time_t sec;
//    MetaParams meta_buf_params;
//    const char * hash_prefix;

//    wrmcapture_out_meta_sequence_filename_buf_param(
//        time_t start_sec,
//        wrmcapture_FilenameGenerator::Format format,
//        const char * const hash_prefix,
//        const char * const prefix,
//        const char * const filename,
//        const char * const extension,
//        const int groupid,
//        MetaParams const & meta_buf_params = MetaParams())
//    : sq_params(format, prefix, filename, extension, groupid)
//    , sec(start_sec)
//    , meta_buf_params(meta_buf_params)
//    , hash_prefix(hash_prefix)
//    {}
//};


template<class FilterParams = wrmcapture_no_param>
struct dorecompress_out_hash_meta_sequence_filename_buf_param
{
    wrmcapture_out_meta_sequence_filename_buf_param<FilterParams> meta_sq_params;
    FilterParams filter_params;
    CryptoContext & cctx;

    dorecompress_out_hash_meta_sequence_filename_buf_param(
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


class dorecompress_out_sequence_filename_buf_impl
{
    char current_filename_[1024];
    wrmcapture_FilenameGenerator filegen_;
    wrmcapture_empty_ctor<io::posix::fdbuf> buf_;
    unsigned num_file_;
    int groupid_;

public:
    explicit dorecompress_out_sequence_filename_buf_impl(wrmcapture_out_sequence_filename_buf_param const & params)
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

    wrmcapture_empty_ctor<io::posix::fdbuf> & buf() noexcept
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

class dorecompress_ofile_buf_out
{
    int fd;
public:
    dorecompress_ofile_buf_out() : fd(-1) {}
    ~dorecompress_ofile_buf_out()
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


struct dorecompress_MetaFilename
{
    char filename[2048];

    dorecompress_MetaFilename(const char * path, const char * basename,
                 wrmcapture_FilenameFormat format = wrmcapture_FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION)
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

    dorecompress_MetaFilename(dorecompress_MetaFilename const &) = delete;
    dorecompress_MetaFilename & operator = (dorecompress_MetaFilename const &) = delete;
};


template<class Writer>
int dorecompress_write_filename(Writer & writer, const char * filename)
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

using dorecompress_hash_type = unsigned char[MD_HASH_LENGTH*2];

constexpr std::size_t dorecompress_hash_string_len = (1 + MD_HASH_LENGTH * 2) * 2;

inline char * dorecompress_swrite_hash(char * p, dorecompress_hash_type const & hash)
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
int dorecompress_write_meta_file_impl(
    Writer & writer, const char * filename,
    struct stat const & stat,
    time_t start_sec, time_t stop_sec,
    dorecompress_hash_type const * hash = nullptr
) {
    if (int err = dorecompress_write_filename(writer, filename)) {
        return err;
    }

    using ull = unsigned long long;
    using ll = long long;
    char mes[
        (std::numeric_limits<ll>::digits10 + 1 + 1) * 8 +
        (std::numeric_limits<ull>::digits10 + 1 + 1) * 2 +
        dorecompress_hash_string_len + 1 +
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
        p = dorecompress_swrite_hash(p, *hash);
    }
    *p++ = '\n';

    ssize_t res = writer.write(mes, p-mes);

    if (res < p-mes) {
        return res < 0 ? res : 1;
    }

    return 0;
}

template<class Writer>
int dorecompress_write_meta_file(
    Writer & writer, const char * filename,
    time_t start_sec, time_t stop_sec,
    dorecompress_hash_type const * hash = nullptr
) {
    struct stat stat;
    int err = ::stat(filename, &stat);
    return err ? err : dorecompress_write_meta_file_impl<true>(writer, filename, stat, start_sec, stop_sec, hash);
}


template<class BufMeta>
class dorecompress_out_meta_sequence_filename_buf_impl
: public dorecompress_out_sequence_filename_buf_impl
{
    typedef dorecompress_out_sequence_filename_buf_impl sequence_base_type;

    BufMeta meta_buf_;
    dorecompress_MetaFilename mf_;
    dorecompress_MetaFilename hf_;
    time_t start_sec_;
    time_t stop_sec_;

public:
    template<class MetaParams>
    explicit dorecompress_out_meta_sequence_filename_buf_impl(
        wrmcapture_out_meta_sequence_filename_buf_param<MetaParams> const & params
    )
    : dorecompress_out_sequence_filename_buf_impl(params.sq_params)
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
            dorecompress_ofile_buf_out crypto_hash;

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
                    err = dorecompress_write_meta_file_impl<false>(crypto_hash, filename, stat, 0, 0, nullptr);
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
    int next_meta_file(dorecompress_hash_type const * hash = nullptr)
    {
        // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
        const char * filename = this->rename_filename();
        if (!filename) {
            return 1;
        }

        if (int err = dorecompress_write_meta_file(
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
        this->dorecompress_out_sequence_filename_buf_impl::request_full_cleaning();
        ::unlink(this->mf_.filename);
    }

    int flush()
    {
        const int res1 = this->dorecompress_out_sequence_filename_buf_impl::flush();
        const int res2 = this->meta_buf_.flush();
        return res1 == 0 ? res2 : res1;
    }

    BufMeta & meta_buf() noexcept
    { return this->meta_buf_; }

    void update_sec(time_t sec)
    { this->stop_sec_ = sec; }
};

template<class BufFilter, class BufMeta, class BufHash, class Params>
class dorecompress_out_hash_meta_sequence_filename_buf_impl
: public dorecompress_out_meta_sequence_filename_buf_impl<BufMeta>
{
    CryptoContext & cctx;
    Params hash_ctx;
    BufFilter wrm_filter;

    using sequence_base_type = dorecompress_out_meta_sequence_filename_buf_impl<BufMeta>;

public:
    explicit dorecompress_out_hash_meta_sequence_filename_buf_impl(
        dorecompress_out_hash_meta_sequence_filename_buf_param<Params> const & params
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

        dorecompress_hash_type hash;

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
                err = dorecompress_write_meta_file_impl<false>(hash_buf, filename, stat, 0, 0, &hash);
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
            dorecompress_hash_type hash;
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
void dorecompress_write_meta_headers(Writer & writer, const char * path,
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


struct dorecompress_OutMetaSequenceTransport : public Transport
{
    dorecompress_OutMetaSequenceTransport(
        const char * path,
        const char * hash_path,
        const char * basename,
        timeval now,
        uint16_t width,
        uint16_t height,
        const int groupid,
        auth_api * authentifier = nullptr,
        wrmcapture_FilenameFormat format = wrmcapture_FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
    : buf(wrmcapture_out_meta_sequence_filename_buf_param<>(
        now.tv_sec, format, hash_path, path, basename, ".wrm", groupid
    ))
    {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        dorecompress_write_meta_headers(this->buffer().meta_buf(), path, width, height, this->authentifier, false);
    }

    void timestamp(timeval now) override {
        this->buffer().update_sec(now.tv_sec);
    }

    const wrmcapture_FilenameGenerator * seqgen() const noexcept
    {
        return &(this->buffer().seqgen());
    }
    using Buf = dorecompress_out_meta_sequence_filename_buf_impl<wrmcapture_empty_ctor<dorecompress_ofile_buf_out>>;

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

    ~dorecompress_OutMetaSequenceTransport() {
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

struct dorecompress_ocrypto_filename_params
{
    CryptoContext & crypto_ctx;
    Random & rnd;
};



struct dorecompress_ocrypto_filter : dorecompress_encrypt_filter
{
    CryptoContext & cctx;
    Random & rnd;

    explicit dorecompress_ocrypto_filter(dorecompress_ocrypto_filename_params params)
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
        return dorecompress_encrypt_filter::open(buf, trace_key, this->cctx, iv);
    }
};


class dorecompress_ocrypto_filename_buf
{
    dorecompress_encrypt_filter encrypt;
    CryptoContext & cctx;
    Random & rnd;
    dorecompress_ofile_buf_out file;

public:
    explicit dorecompress_ocrypto_filename_buf(dorecompress_ocrypto_filename_params params)
    : cctx(params.crypto_ctx)
    , rnd(params.rnd)
    {}

    ~dorecompress_ocrypto_filename_buf()
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

struct dorecompress_CryptoOutMetaSequenceTransport
: public Transport {

    using Buf =
        dorecompress_out_hash_meta_sequence_filename_buf_impl<
            dorecompress_ocrypto_filter,
            dorecompress_ocrypto_filename_buf,
            dorecompress_ocrypto_filename_buf,
            dorecompress_ocrypto_filename_params
        >;

    dorecompress_CryptoOutMetaSequenceTransport(
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
        wrmcapture_FilenameFormat format = wrmcapture_FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
    : buf(
        dorecompress_out_hash_meta_sequence_filename_buf_param<dorecompress_ocrypto_filename_params>(
            crypto_ctx,
            now.tv_sec, format, hash_path, path, basename, ".wrm", groupid,
            {crypto_ctx, rnd}
        )) {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        dorecompress_write_meta_headers(this->buffer().meta_buf(), path, width, height, this->authentifier, true);
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

    ~dorecompress_CryptoOutMetaSequenceTransport() {
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
            dorecompress_CryptoOutMetaSequenceTransport trans(
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
            dorecompress_OutMetaSequenceTransport trans(
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


struct HashHeader {
    unsigned version;
};


inline void load_hash(
    MetaLine2 & hash_line,
    const std::string & full_hash_path, const std::string & input_filename,
    unsigned int infile_version, bool infile_is_checksumed,
    CryptoContext & cctx, bool infile_is_encrypted, int verbose
) {
    ifile_read_encrypted in_hash_fb(cctx, infile_is_encrypted);

    if (in_hash_fb.open(full_hash_path.c_str()) < 0) {
        LOG(LOG_INFO, "Open load_hash failed");
        throw Error(ERR_TRANSPORT_OPEN_FAILED);
    }

    char buffer[8192]{};
    ssize_t len;
    {
        ssize_t remaining = sizeof(buffer);
        char * p = buffer;
        while ((len = in_hash_fb.read(p, remaining))) {
            if (len < 0){
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
            p += len;
            remaining -= len;
        }
        len = p - buffer;
    }

    char * eof = &buffer[len];
    char * cur = &buffer[0];

    if (infile_version == 1) {
        if (verbose) {
            LOG(LOG_INFO, "Hash data v1");
        }
        // Filename HASH_64_BYTES
        //         ^
        //         |
        //     separator

        int len = input_filename.length()+1;
        if (eof-cur < len){
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }

        if (0 != memcmp(cur, input_filename.c_str(), input_filename.length()))
        {
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        cur += input_filename.length();
        if (cur[0] != ' '){
            throw Error(ERR_TRANSPORT_READ_FAILED);
        }
        cur++;
        in_copy_bytes(hash_line.hash1, MD_HASH_LENGTH, cur, eof, ERR_TRANSPORT_READ_FAILED);
        in_copy_bytes(hash_line.hash2, MD_HASH_LENGTH, cur, eof, ERR_TRANSPORT_READ_FAILED);
    }
    else {
        if (verbose) {
            LOG(LOG_INFO, "Hash data v2 or higher");
        }

        // v2
        if (cur == eof || cur[0] != 'v'){
            Error(ERR_TRANSPORT_READ_FAILED, errno);
        }

        // skip 3 lines
        for (auto i = 0 ; i < 3 ; i++)
        {
            char * pos = std::find(cur, eof, '\n');
            if (pos == eof) {
                throw Error(ERR_TRANSPORT_READ_FAILED);
            }
            cur = pos + 1;
        }

        // Line format "fffff
        // st_size st_mode st_uid st_gid st_dev st_ino st_mtime
        // st_ctime hhhhh HHHHH"
        //         ^  ^  ^  ^
        //         |  |  |  |
        //         |hash1|  |
        //         |     |  |
        //       space   |hash2
        //                  |
        //                space
        //
        // filename(1 or >) + space(1)
        // + stat_info(ll|ull * 8) + space(1)
        // + hash1(64) + space(1) + hash2(64) >= 135

        // filename(1 or >) followed by space
        {
            char * pos = std::find(cur, eof, ' ');
            if (pos == eof){
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
            if (size_t(pos-cur) != input_filename.length()
            || (0 != strncmp(cur, input_filename.c_str(), pos-cur)))
            {
                std::cerr << "File name mismatch: \""
                            << input_filename
                            << "\"" << std::endl
                            << std::endl;
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
            memcpy(hash_line.filename, cur, pos - cur);
            hash_line.filename[pos-cur]=0;
            cur = pos + 1;
        }
        hash_line.size = get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
        hash_line.mode = get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
        hash_line.uid = get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
        hash_line.gid = get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
        hash_line.dev = get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
        hash_line.ino = get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
        hash_line.mtime = get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
        hash_line.ctime = get_ll(cur, eof, infile_is_checksumed ? ' ' : '\n', ERR_TRANSPORT_READ_FAILED);

        if (infile_is_checksumed){
            // HASH1 + space
            in_hex256(hash_line.hash1, MD_HASH_LENGTH, cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
            // HASH1 + CR
            in_hex256(hash_line.hash2, MD_HASH_LENGTH, cur, eof, '\n', ERR_TRANSPORT_READ_FAILED);
        }
    }
}

static inline bool meta_line_stat_equal_stat(MetaLine2 const & metadata, struct stat64 const & sb)
{
    return metadata.dev == sb.st_dev
        && metadata.ino == sb.st_ino
        && metadata.mode == sb.st_mode
        && metadata.uid == sb.st_uid
        && metadata.gid == sb.st_gid
        && metadata.mtime == sb.st_mtime
        && metadata.ctime == sb.st_ctime
        && metadata.size == sb.st_size;
}

struct out_is_mismatch
{
    bool & is_mismatch;
};

static inline int check_file(const std::string & filename, const MetaLine2 & metadata,
                      bool quick, bool has_checksum, bool ignore_stat_info,
                      uint8_t const * hmac_key, size_t hmac_key_len, bool update_stat_info, out_is_mismatch has_mismatch_stat)
{
    has_mismatch_stat.is_mismatch = false;
    struct stat64 sb;
    memset(&sb, 0, sizeof(sb));
    if (lstat64(filename.c_str(), &sb) < 0){
        std::cerr << "File \"" << filename << "\" is invalid! (can't stat file)\n" << std::endl;
        return false;
    }

    if (has_checksum){
        if (sb.st_size != metadata.size){
            std::cerr << "File \"" << filename << "\" is invalid! (size mismatch)\n" << std::endl;
            return false;
        }

        uint8_t hash[SHA256_DIGEST_LENGTH]{};
        if (file_start_hmac_sha256(filename.c_str(),
                             hmac_key, hmac_key_len,
                             quick?QUICK_CHECK_LENGTH:0, hash) < 0) {
            std::cerr << "Error reading file \"" << filename << "\"\n" << std::endl;
            return false;
        }
        if (0 != memcmp(hash, quick?metadata.hash1:metadata.hash2, SHA256_DIGEST_LENGTH)){
            std::cerr << "Error checking file \"" << filename << "\" (invalid checksum)\n" << std::endl;
            return false;
        }
    }
    else if ((!ignore_stat_info || update_stat_info) && !meta_line_stat_equal_stat(metadata, sb)) {
        if (update_stat_info) {
            has_mismatch_stat.is_mismatch = true;
        }
        if (!ignore_stat_info) {
            std::cerr << "File \"" << filename << "\" is invalid! (metafile changed)\n" << std::endl;
        }
        return false;
    }
    return true;
}

static inline int check_encrypted_or_checksumed(
    std::string const & input_filename,
    std::string const & mwrm_path,
    std::string const & hash_path,
    bool quick_check,
    bool ignore_stat_info,
    bool update_stat_info,
    uint32_t verbose,
    CryptoContext & cctx
) {

    std::string const full_mwrm_filename = mwrm_path + input_filename;

    // Let(s ifile_read autodetect encryption at opening for first file
    int encryption = 2;

//    cctx.old_encryption_scheme = true;
    ifile_read_encrypted ibuf(cctx, encryption);

    if (ibuf.open(full_mwrm_filename.c_str()) < 0){
        LOG(LOG_INFO, "ibuf.open error");
        throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
    }

    // now force encryption for sub files
    bool infile_is_encrypted = ibuf.encrypted;

    MwrmReader reader(ibuf);
    reader.read_meta_headers(ibuf.encrypted);

    // if we have version 1 header, ignore stat info
    ignore_stat_info |= (reader.header.version == 1);
    // if we have version >1 header and not checksum, update stat info
    update_stat_info &= (reader.header.version > 1) & !reader.header.has_checksum & !ibuf.encrypted;
    ignore_stat_info |= update_stat_info;

    /*****************
    * Load file hash *
    *****************/
    LOG(LOG_INFO, "Load file hash");
    MetaLine2 hash_line = {{}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}};

    std::string const full_hash_path = hash_path + input_filename;

    // if reading hash fails
    try {
        load_hash(hash_line, full_hash_path, input_filename, reader.header.version, reader.header.has_checksum, cctx, infile_is_encrypted, verbose);
    }
    catch (...) {
        std::cerr << "Cannot read hash file: \"" << full_hash_path << "\"\n" << std::endl;
        // this is an error because checksum comes from hash file
        // and extended stat info also comes from hash file
        // if we can't read hash files we are in troubles
        if (reader.header.has_checksum || !ignore_stat_info){
            return 1;
        }
    }

    bool has_mismatch_stat_hash = false;

    /******************
    * Check mwrm file *
    ******************/
    if (!check_file(
        full_mwrm_filename, hash_line, quick_check, reader.header.has_checksum,
        ignore_stat_info, cctx.get_hmac_key(), 32,
        update_stat_info, out_is_mismatch{has_mismatch_stat_hash}
    )){
        if (!has_mismatch_stat_hash) {
            return 1;
        }
    }
    else {
    }

    struct MetaLine2CtxForRewriteStat
    {
        std::string wrm_filename;
        std::string filename;
        time_t start_time;
        time_t stop_time;
    };
    std::vector<MetaLine2CtxForRewriteStat> meta_line_ctx_list;
    bool wrm_stat_is_ok = true;


    MetaLine2 meta_line_wrm;

    while (reader.read_meta_file(meta_line_wrm)) {
        size_t tmp_wrm_filename_len = 0;
        const char * tmp_wrm_filename = basename_len(meta_line_wrm.filename, tmp_wrm_filename_len);
        std::string const meta_line_wrm_filename = std::string(tmp_wrm_filename, tmp_wrm_filename_len);
        std::string const full_part_filename = mwrm_path + meta_line_wrm_filename;

//        LOG(LOG_INFO, "checking part %s", full_part_filename);


        bool has_mismatch_stat_mwrm = false;
        if (!check_file(
            full_part_filename, meta_line_wrm, quick_check, reader.header.has_checksum,
            ignore_stat_info, cctx.get_hmac_key(), 32,
            update_stat_info, out_is_mismatch{has_mismatch_stat_mwrm})
        ){
            if (has_mismatch_stat_mwrm) {
                wrm_stat_is_ok = false;
            }
            else {
                return 1;
            }
        }

        if (update_stat_info) {
            meta_line_ctx_list.push_back({
                full_part_filename,
                meta_line_wrm.filename,
                meta_line_wrm.start_time,
                meta_line_wrm.stop_time
            });
        }
    }

    ibuf.close();


    /*******************
    * Rewite stat info *
    ********************/
    struct local_auto_remove
    {
        char const * filename;
        ~local_auto_remove() {
            if (this->filename) {
                remove(this->filename);
            }
        }
    };

    if (!wrm_stat_is_ok) {
        if (verbose) {
            LOG(LOG_INFO, "%s", "Update mwrm file");
        }

        has_mismatch_stat_hash = true;

        auto full_mwrm_filename_tmp = full_mwrm_filename + ".tmp";

        // out_meta_sequence_filename_buf_impl ctor
        dorecompress_ofile_buf_out mwrm_file_cp;
        if (mwrm_file_cp.open(full_mwrm_filename_tmp.c_str(), S_IRUSR | S_IRGRP | S_IWUSR) < 0) {
            LOG(LOG_ERR, "Failed to open meta file %s", full_mwrm_filename_tmp);
            throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
        }
        local_auto_remove auto_remove{full_mwrm_filename_tmp.c_str()};
        if (chmod(full_mwrm_filename_tmp.c_str(), S_IRUSR | S_IRGRP) == -1) {
            LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                , full_mwrm_filename_tmp
                , "u+r, g+r"
                , strerror(errno), errno);
            return 1;
        }

        // copy mwrm headers
        {
            ifile_read mwrm_file;
            mwrm_file.open(full_mwrm_filename.c_str());
            LineReader line_reader(mwrm_file);

            // v2, w h, nochecksum, blank, blank
            for (int i = 0; i < 5; ++i) {
                if (!line_reader.next_line()) {
                    throw Error(ERR_TRANSPORT_READ_FAILED, 0);
                }
                auto av = line_reader.get_buf();
                if (mwrm_file_cp.write(av.data(), av.size()) != ssize_t(av.size())) {
                    throw Error(ERR_TRANSPORT_WRITE_FAILED, 0);
                }
            }
        }

        for (MetaLine2CtxForRewriteStat & ctx : meta_line_ctx_list) {
            struct stat sb;
            if (lstat(ctx.wrm_filename.c_str(), &sb) < 0
             || dorecompress_write_meta_file_impl<true>(mwrm_file_cp, ctx.filename.c_str(), sb, ctx.start_time, ctx.stop_time)
            ) {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, 0);
            }
        }

        if (rename(full_mwrm_filename_tmp.c_str(), full_mwrm_filename.c_str())) {
            std::cerr << strerror(errno) << std::endl;
            return 1;
        }

        if (verbose) {
            LOG(LOG_INFO, "%s", "Update mwrm file, done");
        }
        auto_remove.filename = nullptr;
    }

    if (has_mismatch_stat_hash) {
        if (verbose) {
            LOG(LOG_INFO, "%s", "Update hash file");
        }

        auto const full_hash_path_tmp = (full_hash_path + ".tmp");
        dorecompress_ofile_buf_out hash_file_cp;

        local_auto_remove auto_remove{full_hash_path_tmp.c_str()};

        char const * hash_filename = full_hash_path_tmp.c_str();
        char const * meta_filename = full_mwrm_filename.c_str();

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

        if (hash_file_cp.open(hash_filename, S_IRUSR|S_IRGRP) >= 0) {
            char header[] = "v2\n\n\n";
            hash_file_cp.write(header, sizeof(header)-1);

            struct stat stat;
            int err = ::stat(meta_filename, &stat);
            if (!err) {
                err = dorecompress_write_meta_file_impl<false>(hash_file_cp, filename, stat, 0, 0, nullptr);
            }
            if (!err) {
                err = hash_file_cp.close(/*hash*/);
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

        if (rename(full_hash_path_tmp.c_str(), full_hash_path.c_str())) {
            std::cerr << strerror(errno) << std::endl;
            return 1;
        }

        auto_remove.filename = nullptr;
        if (verbose) {
            LOG(LOG_INFO, "%s", "Update hash file, done");
        }
    }

    std::cout << "No error detected during the data verification.\n" << std::endl;
    return 0;
}


inline unsigned get_file_count(
    InMetaSequenceTransport & in_wrm_trans,
    uint32_t & begin_cap, uint32_t & end_cap,
    timeval & begin_record, timeval & end_record
) {
    in_wrm_trans.next();
    begin_record.tv_sec = in_wrm_trans.begin_chunk_time();
    // TODO a negative time should be a time relative to end of movie
    // less than 1 year means we are given a time relatve to beginning of movie
    if (begin_cap && (begin_cap < 31536000)) {  // less than 1 year, it is relative not absolute timestamp
        // begin_capture.tv_usec is 0
        begin_cap += in_wrm_trans.begin_chunk_time();
    }
    if (end_cap && (end_cap < 31536000)) { // less than 1 year, it is relative not absolute timestamp
        // begin_capture.tv_usec is 0
        end_cap += in_wrm_trans.begin_chunk_time();
    }
    while (begin_cap >= in_wrm_trans.end_chunk_time()) {
        in_wrm_trans.next();
    }
    unsigned const result = in_wrm_trans.get_seqno();
    try {
        do {
            end_record.tv_sec = in_wrm_trans.end_chunk_time();
            in_wrm_trans.next();
        }
        while (true);
    }
    catch (const Error & e) {
        if (e.id != static_cast<unsigned>(ERR_TRANSPORT_NO_MORE_DATA)) {
            throw;
        }
    };
    return result;
}

inline void remove_file(
    InMetaSequenceTransport & in_wrm_trans, const char * hash_path, const char * infile_path
  , const char * input_filename, const char * infile_extension, bool is_encrypted
) {
    std::vector<std::string> files;

    char infile_fullpath[2048];
    if (is_encrypted) {
        std::snprintf(infile_fullpath, sizeof(infile_fullpath), "%s%s%s", hash_path, input_filename, infile_extension);
        files.push_back(infile_fullpath);
    }
    std::snprintf(infile_fullpath, sizeof(infile_fullpath), "%s%s%s", infile_path, input_filename, infile_extension);
    files.push_back(infile_fullpath);

    try {
        do {
            in_wrm_trans.next();
            files.push_back(in_wrm_trans.path());
        }
        while (true);
    }
    catch (const Error & e) {
        if (e.id != ERR_TRANSPORT_NO_MORE_DATA) {
            throw;
        }
    };

    std::cout << std::endl;
    for (auto & s : iter(files.rbegin(), files.rend())) {
        unlink(s.c_str());
        std::cout << "Removed : " << s << std::endl;
    }
}

inline
static void raise_error(UpdateProgressData::Format pgs_format, std::string const & output_filename, int code, const char * message) {
    if (!output_filename.length()) {
        return;
    }

    char outfile_pid[32];
    std::snprintf(outfile_pid, sizeof(outfile_pid), "%06u", unsigned(getpid()));

    char outfile_path     [1024] = {};
    char outfile_basename [1024] = {};
    char outfile_extension[1024] = {};

    canonical_path( output_filename.c_str()
                  , outfile_path
                  , sizeof(outfile_path)
                  , outfile_basename
                  , sizeof(outfile_basename)
                  , outfile_extension
                  , sizeof(outfile_extension)
                  );

    char progress_filename[4096];
    std::snprintf( progress_filename, sizeof(progress_filename), "%s%s-%s.pgs"
            , outfile_path, outfile_basename, outfile_pid);

    UpdateProgressData update_progress_data(pgs_format, progress_filename, 0, 0, 0, 0);

    update_progress_data.raise_error(code, message);
}

inline
static void show_metadata(FileToGraphic const & player) {
    std::cout
    << "\nWRM file version      : " << player.info_version
    << "\nWidth                 : " << player.info_width
    << "\nHeight                : " << player.info_height
    << "\nBpp                   : " << player.info_bpp
    << "\nCache 0 entries       : " << player.info_cache_0_entries
    << "\nCache 0 size          : " << player.info_cache_0_size
    << "\nCache 1 entries       : " << player.info_cache_1_entries
    << "\nCache 1 size          : " << player.info_cache_1_size
    << "\nCache 2 entries       : " << player.info_cache_2_entries
    << "\nCache 2 size          : " << player.info_cache_2_size
    << '\n';

    if (player.info_version > 3) {
        //cout << "Cache 3 entries       : " << player.info_cache_3_entries                         << endl;
        //cout << "Cache 3 size          : " << player.info_cache_3_size                            << endl;
        //cout << "Cache 4 entries       : " << player.info_cache_4_entries                         << endl;
        //cout << "Cache 4 size          : " << player.info_cache_4_size                            << endl;
        std::cout << "Compression algorithm : " << static_cast<int>(player.info_compression_algorithm) << '\n';
    }
    std::cout.flush();
}

// TODO Signals related code should not be here, all globals if any should be in main
static bool program_requested_to_shutdown = false;

inline void shutdown(int sig)
{
    LOG(LOG_INFO, "shutting down : signal %d pid=%d\n", sig, getpid());

    program_requested_to_shutdown = true;
}

inline void init_signals()
{
    struct sigaction sa;

    sa.sa_flags = 0;

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGTERM);

    sa.sa_handler = shutdown;
    sigaction(SIGTERM, &sa, nullptr);
}


inline
static void show_statistics(FileToGraphic::Statistics const & statistics) {
    std::cout
    << "\nDstBlt                : " << statistics.DstBlt
    << "\nMultiDstBlt           : " << statistics.MultiDstBlt
    << "\nPatBlt                : " << statistics.PatBlt
    << "\nMultiPatBlt           : " << statistics.MultiPatBlt
    << "\nOpaqueRect            : " << statistics.OpaqueRect
    << "\nMultiOpaqueRect       : " << statistics.MultiOpaqueRect
    << "\nScrBlt                : " << statistics.ScrBlt
    << "\nMultiScrBlt           : " << statistics.MultiScrBlt
    << "\nMemBlt                : " << statistics.MemBlt
    << "\nMem3Blt               : " << statistics.Mem3Blt
    << "\nLineTo                : " << statistics.LineTo
    << "\nGlyphIndex            : " << statistics.GlyphIndex
    << "\nPolyline              : " << statistics.Polyline

    << "\nCacheBitmap           : " << statistics.CacheBitmap
    << "\nCacheColorTable       : " << statistics.CacheColorTable
    << "\nCacheGlyph            : " << statistics.CacheGlyph

    << "\nFrameMarker           : " << statistics.FrameMarker

    << "\nBitmapUpdate          : " << statistics.BitmapUpdate

    << "\nCachePointer          : " << statistics.CachePointer
    << "\nPointerIndex          : " << statistics.PointerIndex

    << "\ngraphics_update_chunk : " << statistics.graphics_update_chunk
    << "\nbitmap_update_chunk   : " << statistics.bitmap_update_chunk
    << "\ntimestamp_chunk       : " << statistics.timestamp_chunk
    << std::endl;
}


inline int is_encrypted_file(const char * input_filename, bool & infile_is_encrypted)
{
    infile_is_encrypted = false;
    const int fd_test = open(input_filename, O_RDONLY);
    if (fd_test != -1) {
        uint8_t data[4] = {};
        ssize_t res_test = read(fd_test, data, 4);
        const uint32_t magic = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
        if ((res_test == 4) && (magic == WABCRYPTOFILE_MAGIC)) {
            infile_is_encrypted = true;
            std::cout << "Input file is encrypted.\n";
        }
        close(fd_test);
        return 0;
    }

    return -1;
}

inline int replay(std::string & infile_path, std::string & input_basename, std::string & infile_extension,
                  std::string & hash_path,
                  CaptureFlags & capture_flags,
                  UpdateProgressData::Format pgs_format,
                  bool chunk,
                  unsigned ocr_version,
                  std::string & output_filename,
                  uint32_t begin_cap,
                  uint32_t end_cap,
                  PngParams & png_params,
                  FlvParams & flv_params,
                  int wrm_color_depth,
                  uint32_t wrm_frame_interval,
                  uint32_t wrm_break_interval,
                  bool const no_timestamp,
                  bool infile_is_encrypted,
                  uint32_t order_count,
                  bool show_file_metadata,
                  bool show_statistics,
                  uint32_t clear,
                  bool full_video,
                  bool remove_input_file,
                  int wrm_compression_algorithm,
                  uint32_t flv_break_interval,
                  TraceType encryption_type,
                  Inifile & ini, CryptoContext & cctx, Random & rnd,
                  uint32_t verbose)
{

    char infile_prefix[4096];
    std::snprintf(infile_prefix, sizeof(infile_prefix), "%s%s", infile_path.c_str(), input_basename.c_str());
    ini.set<cfg::video::hash_path>(hash_path);

    ini.set<cfg::video::frame_interval>(std::chrono::duration<unsigned int, std::centi>{wrm_frame_interval});
    ini.set<cfg::video::break_interval>(std::chrono::seconds{wrm_break_interval});
    ini.set<cfg::video::flv_break_interval>(std::chrono::seconds{flv_break_interval});
    ini.set<cfg::globals::trace_type>(encryption_type);
    ini.set<cfg::video::rt_display>(bool(capture_flags & CaptureFlags::png));

    ini.set<cfg::globals::capture_chunk>(chunk);
    ini.set<cfg::ocr::version>(ocr_version == 2 ? OcrVersion::v2 : OcrVersion::v1);

    if (chunk){
        ini.get_ref<cfg::video::disable_keyboard_log>() &= ~KeyboardLogFlags::meta;
        ini.set<cfg::ocr::interval>(std::chrono::seconds{1});
    }

    timeval  begin_record = { 0, 0 };
    timeval  end_record   = { 0, 0 };
    unsigned file_count   = 0;
    try {
        InMetaSequenceTransport in_wrm_trans_tmp(
            &cctx,
            infile_prefix,
            infile_extension.c_str(),
            infile_is_encrypted?1:0);
        file_count = get_file_count(in_wrm_trans_tmp, begin_cap, end_cap, begin_record, end_record);
    }
    catch (const Error & e) {
        if (e.id == static_cast<unsigned>(ERR_TRANSPORT_NO_MORE_DATA)) {
            std::cerr << "Asked time not found in mwrm file\n";
        }
        else {
            std::cerr << "Error: " << e.errmsg() << std::endl;
        }
        const bool msg_with_error_id = false;
        raise_error(pgs_format, output_filename, e.id, e.errmsg(msg_with_error_id));
        return -1;
    };

    InMetaSequenceTransport in_wrm_trans(
        &cctx, infile_prefix,
        infile_extension.c_str(),
        infile_is_encrypted?1:0
    );

    timeval begin_capture = {0, 0};
    timeval end_capture = {0, 0};

    int result = -1;
    try {
        bool test = (
                bool(capture_flags & CaptureFlags::flv)
            || bool(capture_flags & CaptureFlags::ocr)
            || bool(capture_flags & CaptureFlags::png)
            || full_video
            || wrm_color_depth != static_cast<int>(USE_ORIGINAL_COLOR_DEPTH)
            || show_file_metadata
            || show_statistics
            || file_count > 1
            || order_count
            || begin_cap != begin_record.tv_sec
            || end_cap != begin_cap);

        if (test){
            for (unsigned i = 1; i < file_count ; i++) {
                in_wrm_trans.next();
            }

            FileToGraphic player(in_wrm_trans, begin_capture, end_capture, false, to_verbose_flags(verbose));

            if (show_file_metadata) {
                show_metadata(player);
                std::cout << "Duration (in seconds) : " << (end_record.tv_sec - begin_record.tv_sec + 1) << std::endl;
            }

            if (show_file_metadata && !show_statistics && !output_filename.length()) {
                result = 0;
            }
            else {
                player.max_order_count = order_count;

                int return_code = 0;

                if (output_filename.length()) {
            //        char outfile_pid[32];
            //        std::snprintf(outfile_pid, sizeof(outfile_pid), "%06u", getpid());

                    char outfile_path     [1024] = {};
                    char outfile_basename [1024] = {};
                    char outfile_extension[1024] = {};

                    canonical_path( output_filename.c_str()
                                    , outfile_path
                                    , sizeof(outfile_path)
                                    , outfile_basename
                                    , sizeof(outfile_basename)
                                    , outfile_extension
                                    , sizeof(outfile_extension)
                                    );

                    if (verbose) {
                        std::cout << "Output file path: "
                                    << outfile_path << outfile_basename << outfile_extension
                                    << '\n' << std::endl;
                    }

                    if (clear == 1) {
                        clear_files_flv_meta_png(outfile_path, outfile_basename);
                    }

                    ini.set<cfg::video::wrm_compression_algorithm>(
                        (wrm_compression_algorithm == static_cast<int>(USE_ORIGINAL_COMPRESSION_ALGORITHM))
                        ? player.info_compression_algorithm
                        : static_cast<WrmCompressionAlgorithm>(wrm_compression_algorithm)
                    );

                    if (wrm_color_depth == static_cast<int>(USE_ORIGINAL_COLOR_DEPTH)) {
                        wrm_color_depth = player.info_bpp;
                    }

                    {
                        ini.set<cfg::video::hash_path>(outfile_path);
                        ini.set<cfg::video::record_tmp_path>(outfile_path);
                        ini.set<cfg::video::record_path>(outfile_path);

                        ini.set<cfg::globals::movie_path>(&output_filename[0]);

                        char progress_filename[4096];
                        std::snprintf( progress_filename, sizeof(progress_filename), "%s%s.pgs"
                                , outfile_path, outfile_basename);
                        UpdateProgressData update_progress_data(
                            pgs_format, progress_filename,
                            begin_record.tv_sec, end_record.tv_sec,
                            begin_capture.tv_sec, end_capture.tv_sec
                        );

                        if (png_params.png_width && png_params.png_height) {
                            auto get_percent = [](unsigned target_dim, unsigned source_dim) -> unsigned {
                                return ((target_dim * 100 / source_dim) + ((target_dim * 100 % source_dim) ? 1 : 0));
                            };
                            png_params.zoom = std::max<unsigned>(
                                    get_percent(png_params.png_width, player.screen_rect.cx),
                                    get_percent(png_params.png_height, player.screen_rect.cy)
                                );
                            //std::cout << "zoom: " << zoom << '%' << std::endl;
                        }

                        ini.set<cfg::globals::video_quality>(flv_params.video_quality);
                        ini.set<cfg::globals::codec_id>(flv_params.codec);
                        flv_params = flv_params_from_ini(
                            player.screen_rect.cx, player.screen_rect.cy, ini);

                        GraphicToFile::Verbose wrm_verbose = to_verbose_flags(ini.get<cfg::debug::capture>())
                            | (ini.get<cfg::debug::primary_orders>() ?GraphicToFile::Verbose::primary_orders:GraphicToFile::Verbose::none)
                            | (ini.get<cfg::debug::secondary_orders>() ?GraphicToFile::Verbose::secondary_orders:GraphicToFile::Verbose::none)
                            | (ini.get<cfg::debug::bitmap_update>() ?GraphicToFile::Verbose::bitmap_update:GraphicToFile::Verbose::none);
                            
                        WrmCompressionAlgorithm wrm_compression_algorithm = ini.get<cfg::video::wrm_compression_algorithm>();
                        std::chrono::duration<unsigned int, std::ratio<1l, 100l> > wrm_frame_interval = ini.get<cfg::video::frame_interval>();
                        std::chrono::seconds wrm_break_interval = ini.get<cfg::video::break_interval>();
                        TraceType wrm_trace_type = ini.get<cfg::globals::trace_type>();

                        const char * record_tmp_path = ini.get<cfg::video::record_tmp_path>().c_str();
                        const char * record_path = record_tmp_path;

                        bool capture_wrm = bool(capture_flags & CaptureFlags::wrm);
                        bool capture_png = bool(capture_flags & CaptureFlags::png) && (png_params.png_limit > 0);
                        bool capture_pattern_checker = false;

                        bool capture_ocr = bool(capture_flags & CaptureFlags::ocr)
                                            || capture_pattern_checker;
                        bool capture_flv = bool(capture_flags & CaptureFlags::flv);
                        bool capture_flv_full = full_video;
                        bool capture_meta = capture_ocr;
                        bool capture_kbd = false;

                        OcrParams ocr_params = {
                                ini.get<cfg::ocr::version>(),
                                ocr::locale::LocaleId(
                                    static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>())),
                                ini.get<cfg::ocr::on_title_bar_only>(),
                                ini.get<cfg::ocr::max_unrecog_char_rate>(),
                                ini.get<cfg::ocr::interval>()
                        };

                        if (ini.get<cfg::debug::capture>()) {
                            LOG(LOG_INFO, "Enable capture:  %s%s  kbd=%d %s%s%s  ocr=%d %s",
                                capture_wrm ?"wrm ":"",
                                capture_png ?"png ":"",
                                capture_kbd ? 1 : 0,
                                capture_flv ?"flv ":"",
                                capture_flv_full ?"flv_full ":"",
                                capture_pattern_checker ?"pattern ":"",
                                capture_ocr ? (ocr_params.ocr_version == OcrVersion::v2 ? 2 : 1) : 0,
                                capture_meta?"meta ":""
                            );
                        }

                        const int groupid = ini.get<cfg::video::capture_groupid>(); // www-data
                        const char * hash_path = ini.get<cfg::video::hash_path>().c_str();
                        const char * movie_path = ini.get<cfg::globals::movie_path>().c_str();

                        char path[1024];
                        char basename[1024];
                        char extension[128];
                        strcpy(path, WRM_PATH "/");     // default value, actual one should come from movie_path
                        strcpy(basename, movie_path);
                        strcpy(extension, "");          // extension is currently ignored

                        if (!canonical_path(movie_path, path, sizeof(path), basename, sizeof(basename), extension, sizeof(extension))
                        ) {
                            LOG(LOG_ERR, "Buffer Overflowed: Path too long");
                            throw Error(ERR_RECORDER_FAILED_TO_FOUND_PATH);
                        }

                        LOG(LOG_INFO, "canonical_path : %s%s%s\n", path, basename, extension);

                        // PngParams
                        png_params.authentifier = nullptr;
                        png_params.record_tmp_path = record_tmp_path;
                        png_params.basename = basename;
                        png_params.groupid = groupid;


                        MetaParams meta_params;
                        KbdLogParams kbdlog_params;
                        PatternCheckerParams patter_checker_params;
                        SequencedVideoParams sequenced_video_params;
                        FullVideoParams full_video_params;

                        WrmParams wrm_params(
                            wrm_color_depth,
                            wrm_trace_type,
                            cctx,
                            rnd,
                            record_path,
                            hash_path,
                            basename,
                            groupid,
                            wrm_frame_interval,
                            wrm_break_interval,
                            wrm_compression_algorithm,
                            int(wrm_verbose)
                        );

const char * pattern_kill = ini.get<cfg::context::pattern_kill>().c_str();
const char * pattern_notify = ini.get<cfg::context::pattern_notify>().c_str();
int debug_capture = ini.get<cfg::debug::capture>();
bool flv_capture_chunk = ini.get<cfg::globals::capture_chunk>();
bool meta_enable_session_log = false;
const std::chrono::duration<long int> flv_break_interval = ini.get<cfg::video::flv_break_interval>();
bool syslog_keyboard_log = bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::syslog);
bool rt_display = ini.get<cfg::video::rt_display>();
bool disable_keyboard_log = bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::wrm);
bool session_log_enabled = false;
bool keyboard_fully_masked = ini.get<cfg::session_log::keyboard_input_masking_level>()
     != ::KeyboardInputMaskingLevel::fully_masked;
bool meta_keyboard_log = bool(ini.get<cfg::video::disable_keyboard_log>() & KeyboardLogFlags::meta);

                        Capture capture( 
                                  capture_wrm, wrm_params
                                , capture_png, png_params
                                , capture_pattern_checker, patter_checker_params
                                , capture_ocr, ocr_params
                                , capture_flv, sequenced_video_params
                                , capture_flv_full, full_video_params
                                , capture_meta, meta_params
                                , capture_kbd, kbdlog_params
                                , basename
                                , ((player.record_now.tv_sec > begin_capture.tv_sec) ? player.record_now : begin_capture)
                                , player.screen_rect.cx
                                , player.screen_rect.cy
                                , player.info_bpp
                                , wrm_color_depth
                                , record_tmp_path
                                , record_path
                                , groupid
                                , flv_params
                                , no_timestamp
                                , nullptr
                                , &update_progress_data
                                , pattern_kill
                                , pattern_notify
                                , debug_capture
                                , flv_capture_chunk
                                , meta_enable_session_log
                                , flv_break_interval
                                , syslog_keyboard_log
                                , rt_display
                                , disable_keyboard_log
                                , session_log_enabled
                                , keyboard_fully_masked
                                , meta_keyboard_log
                                );

                        player.add_consumer(&capture, &capture, &capture, &capture, &capture);

                        if (update_progress_data.is_valid()) {
                            try {
                                player.play(std::ref(update_progress_data), program_requested_to_shutdown);

                                if (program_requested_to_shutdown) {
                                    update_progress_data.raise_error(65537, "Program requested to shutdown");
                                }
                            }
                            catch (Error const & e) {
                                const bool msg_with_error_id = false;
                                update_progress_data.raise_error(e.id, e.errmsg(msg_with_error_id));

                                return_code = -1;
                            }
                            catch (...) {
                                update_progress_data.raise_error(65536, "Unknown error");

                                return_code = -1;
                            }
                        }
                        else {
                            return_code = -1;
                        }
                    }

                    if (!return_code && program_requested_to_shutdown) {
                        clear_files_flv_meta_png(outfile_path, outfile_basename);
                    }
                }
                else {
                    try {
                        player.play(program_requested_to_shutdown);
                    }
                    catch (Error const &) {
                        return_code = -1;
                    }
                }

                if (show_statistics && return_code == 0) {
                    ::show_statistics(player.statistics);
                }

                result = return_code;
            }
        }
        else {
            result = do_recompress(
                cctx,
                rnd,
                in_wrm_trans,
                begin_record,
                program_requested_to_shutdown,
                wrm_compression_algorithm,
                output_filename,
                ini,
                verbose);
        }
    }
    catch (const Error & e) {
        const bool msg_with_error_id = false;
        raise_error(pgs_format, output_filename, e.id, e.errmsg(msg_with_error_id));
    }

    if (!result && remove_input_file) {
        InMetaSequenceTransport in_wrm_trans_tmp(
            &cctx,
            infile_prefix,
            infile_extension.c_str(),
            infile_is_encrypted?1:0);

        remove_file( in_wrm_trans_tmp, ini.get<cfg::video::hash_path>().c_str(), infile_path.c_str()
                    , input_basename.c_str(), infile_extension.c_str()
                    , infile_is_encrypted);
    }

    std::cout << std::endl;

    return result;
}

struct RecorderParams {
    std::string input_filename;
    std::string input_basename;
    std::string infile_extension;
    std::string config_filename;
    std::string mwrm_path;
    std::string hash_path;
    std::string full_path;

    bool show_file_metadata = false;
    bool show_statistics    = false;

    // input restrictions
    // ==================
    // "begin capture time (in seconds), either absolute or relative to video start
    // (negative number means relative to video end), default=from start"
    uint32_t begin_cap = 0;
    // "end capture time (in seconds), either absolute or relative to video start,
    // (nagative number means relative to video end), default=none"
    uint32_t end_cap = 0;
    // "Number of orders to execute before stopping, default=0 execute all orders"
    uint32_t order_count = 0;

    // common output options
    std::string output_filename;

    // png output options
    PngParams png_params = {0, 0, std::chrono::seconds{60}, 100, 0, false , nullptr, nullptr, nullptr, 0};
    FlvParams flv_params = {};

    // flv output options
    bool full_video; // create full video
    uint32_t    flv_break_interval = 10*60;
    std::string flv_quality;

    // wrm output options
    int wrm_compression_algorithm_ = static_cast<int>(USE_ORIGINAL_COMPRESSION_ALGORITHM);
    int wrm_color_depth = static_cast<int>(USE_ORIGINAL_COLOR_DEPTH);
    std::string wrm_encryption;
    uint32_t    wrm_frame_interval = 100;
    uint32_t    wrm_break_interval = 86400;
    TraceType encryption_type = TraceType::localfile;

    // ocr output options
    unsigned    ocr_version = -1u;

    // miscellaneous options
    CaptureFlags capture_flags = CaptureFlags::none; // output control
    bool auto_output_file   = false;
    bool remove_input_file  = false;
    uint32_t    clear       = 1; // default on
    bool infile_is_encrypted = 0;
    bool chunk = false;

    // verifier options
    bool        quick_check    = false;
    bool      ignore_stat_info = false;
    bool      update_stat_info = false;

    bool json_pgs = false;
};

int parse_command_line_options(int argc, char const ** argv, RecorderParams & recorder, Inifile & ini, uint32_t & verbose);

int parse_command_line_options(int argc, char const ** argv, RecorderParams & recorder, Inifile & ini, uint32_t & verbose)
{
    std::string png_geometry;
    std::string wrm_compression_algorithm;  // output compression algorithm.
    std::string color_depth;
    uint32_t png_interval = 0;

    program_options::options_description desc({
        {'h', "help", "produce help message"},
        {'v', "version", "show software version"},
        {'o', "output-file", &recorder.output_filename, "output base filename"},
        {'i', "input-file", &recorder.input_filename, "input base filename"},

        {'H', "hash-path",  &recorder.hash_path, "output hash dirname (if empty, use hash_path of ini)"},
        {'M', "mwrm-path",  &recorder.mwrm_path,         "mwrm file path"       },

        // verifier options
        {'Q', "quick",   "quick check only"},
        {'S', "ignore-stat-info", "ignore stat info data mismatch" },
        {'U', "update-stat-info", "update stat info data mismatch "
                                  "(only if not checksum and no encrypted)" },

        {'b', "begin", &recorder.begin_cap, "begin capture time (in seconds), default=none"},
        {'e', "end", &recorder.end_cap, "end capture time (in seconds), default=none"},
        {"count", &recorder.order_count, "Number of orders to execute before stopping, default=0 execute all orders"},

        {'n', "png_interval", &png_interval, "time interval between png captures, default=60 seconds"},

        {'r', "frameinterval", &recorder.wrm_frame_interval, "time between consecutive capture frames (in 100/th of seconds), default=100 one frame per second"},

        {'k', "breakinterval", &recorder.wrm_break_interval, "number of seconds between splitting wrm files in seconds(default, one wrm every day)"},

        {'p', "png", "enable png capture"},
        {'w', "wrm", "enable wrm capture"},
        {'t', "ocr", "enable ocr title bar detection"},
        {'f', "flv", "enable flv capture"},
        {'u', "full", "create full video"},
        {'c', "chunk", "chunk splitting on title bar change detection"},

        {"clear", &recorder.clear, "clear old capture files with same prefix (default on)"},
        {"verbose", &verbose, "more logs"},
        {"zoom", &recorder.png_params.zoom, "scaling factor for png capture (default 100%)"},
        {'g', "png-geometry", &png_geometry, "png capture geometry (Ex. 160x120)"},
        {'m', "meta", "show file metadata"},
        {'s', "statistics", "show statistics"},

        {'z', "compression", &wrm_compression_algorithm, "wrm compression algorithm (default=original, none, gzip, snappy)"},
        {'d', "color-depth", &color_depth,           "wrm color depth (default=original, 16, 24)"},
        {'y', "encryption",  &recorder.wrm_encryption,            "wrm encryption (default=original, enable, disable)"},

        {"remove-input-file", "remove input file"},

        {"config-file", &recorder.config_filename, "used another ini file"},

        {'a', "flvbreakinterval", &recorder.flv_break_interval, "number of seconds between splitting flv files (by default, one flv every 10 minutes)"},

        {'q', "flv-quality", &recorder.flv_quality, "flv quality (high, medium, low)"},

        {"ocr-version", &recorder.ocr_version, "version 1 or 2"},

        {"video-codec", &recorder.flv_params.codec, "ffmpeg video codec id (flv, mp4, etc)"},

        {"json-pgs", "use json format to .pgs file"},
    });

    auto options = program_options::parse_command_line(argc, argv, desc);

    const char * copyright_notice = "ReDemPtion " VERSION ".\n"
        "Copyright (C) Wallix 2010-2016.\n"
        "Christophe Grosjean, Jonathan Poelen, Raphael Zhou.";

    if (options.count("help") > 0) {
        std::cout << copyright_notice;
        std::cout << "\n\nUsage: redrec [options]\n\n";
        std::cout << desc << "\n\n";
        return -1;
    }

    if (options.count("version") > 0) {
        std::cout << copyright_notice << std::endl << std::endl;
        return -1;
    }

    if (options.count("config-file") > 0) {
        ConfigurationLoader cfg_loader_full(ini.configuration_holder(), recorder.config_filename.c_str());
    }
    else {
        recorder.config_filename = std::string(CFG_PATH "/" RDPPROXY_INI);
    }

    if (options.count("quick") > 0) {
        recorder.quick_check = true;
    }

    if (options.count("ignore-stat-info") > 0) {
        recorder.ignore_stat_info = true;
    }

    if (options.count("update-stat-info") > 0) {
        recorder.update_stat_info = true;
    }

    if (options.count("json-pgs") > 0) {
        recorder.json_pgs = true;
    }

    recorder.flv_params.video_quality = Level::high;
    recorder.chunk = options.count("chunk") > 0;
    recorder.capture_flags = ((options.count("wrm") > 0)              ?CaptureFlags::wrm:CaptureFlags::none)
                           | (((recorder.chunk)||(options.count("png") > 0))?CaptureFlags::png:CaptureFlags::none)
                           | (((recorder.chunk)||(options.count("flv") > 0))?CaptureFlags::flv:CaptureFlags::none)
                           | (((recorder.chunk)||(options.count("ocr") > 0))?CaptureFlags::ocr:CaptureFlags::none);

    if (options.count("flv-quality") > 0) {
            if (0 == strcmp(recorder.flv_quality.c_str(), "high")) {
            recorder.flv_params.video_quality = Level::high;
        }
        else if (0 == strcmp(recorder.flv_quality.c_str(), "low")) {
            recorder.flv_params.video_quality = Level::low;
        }
        else  if (0 == strcmp(recorder.flv_quality.c_str(), "medium")) {
            recorder.flv_params.video_quality = Level::medium;
        }
        else {
            std::cerr << "Unknown video quality" << std::endl;
            return -1;
        }
    }

    recorder.remove_input_file  = (options.count("remove-input-file") > 0);

    if (options.count("color-depth") > 0){
        recorder.wrm_color_depth = (color_depth == "16") ? 16
                                 : (color_depth == "24") ? 24
                                 : (color_depth == "original") ? static_cast<int>(USE_ORIGINAL_COLOR_DEPTH)
                                 : 0;
        if (!recorder.wrm_color_depth){
            std::cerr << "Unknown wrm color depth\n\n";
            return 1;
        }
    }

    if (options.count("png_interval") > 0){
        recorder.png_params.png_interval = std::chrono::seconds{png_interval};
    }

    if ((options.count("zoom") > 0)
    && (options.count("png-geometry") > 0)) {
        std::cerr << "Conflicting options : --zoom and --png-geometry\n\n";
        return -1;
    }

    if (options.count("png-geometry") > 0) {
        const char * png_geometry_c = png_geometry.c_str();
        const char * separator      = strchr(png_geometry_c, 'x');
        int          png_w          = atoi(png_geometry_c);
        int          png_h          = 0;
        if (separator) {
            png_h = atoi(separator + 1);
        }
        if (!png_w || !png_h) {
            std::cerr << "Invalide png geometry\n\n";
            return -1;
        }
        recorder.png_params.png_width  = png_w;
        recorder.png_params.png_height = png_h;
        std::cout << "png-geometry: " << recorder.png_params.png_width << "x" << recorder.png_params.png_height << std::endl;
    }

    //recorder.flv_params.video_codec = "flv";

    if (options.count("compression") > 0) {
         if (wrm_compression_algorithm == "none") {
            recorder.wrm_compression_algorithm_ = static_cast<int>(WrmCompressionAlgorithm::no_compression);
        }
        else if (wrm_compression_algorithm == "gzip") {
            recorder.wrm_compression_algorithm_ = static_cast<int>(WrmCompressionAlgorithm::gzip);
        }
        else if (wrm_compression_algorithm == "snappy") {
            recorder.wrm_compression_algorithm_ = static_cast<int>(WrmCompressionAlgorithm::snappy);
        }
        else if (wrm_compression_algorithm == "original") {
            recorder.wrm_compression_algorithm_ = static_cast<int>(USE_ORIGINAL_COMPRESSION_ALGORITHM);
        }
        else {
            std::cerr << "Unknown wrm compression algorithm\n\n";
            return -1;
        }
    }

    if (options.count("hash-path") > 0){
        if (recorder.hash_path.c_str()[0] == 0) {
            std::cerr << "Missing hash-path : use -h path\n\n";
            return -1;
        }
    }
    else {
        recorder.hash_path      = ini.get<cfg::video::hash_path>().c_str();
    }
    // TODO: check we do not already have a trailing slash
    if (!recorder.hash_path.empty()) {
        recorder.hash_path += '/';
    }

    if (options.count("mwrm-path") > 0){
        if (recorder.mwrm_path.c_str()[0] == 0) {
            std::cerr << "Missing mwrm-path : use -m path\n\n";
            return -1;
        }
    }
    else {
        recorder.mwrm_path = ini.get<cfg::video::record_path>().c_str();
    }

    if (recorder.input_filename.c_str()[0] == 0) {
        std::cerr << "Missing input mwrm file name : use -i filename\n\n";
        return 1;
    }

    // Input path rule is as follow:
    // -----------------------------
    //  default serach directory for mwrm is given in config file
    // if --mwrm-path is provided on command line it will be preferably used as default
    // if -i has a path component it will be used instead of mwrm-path
    // if relative all command lines path are relative to current working directory

    {
        char temp_path[1024]     = {};
        char temp_basename[1024] = {};
        char temp_extension[256] = {};

        canonical_path(recorder.input_filename.c_str(), temp_path, sizeof(temp_path), temp_basename, sizeof(temp_basename), temp_extension, sizeof(temp_extension));

        if (strlen(temp_path) > 0) {
            recorder.mwrm_path       = temp_path;
        }

        recorder.input_basename = "";
        recorder.input_filename = "";
        recorder.infile_extension = ".mwrm";
        if (strlen(temp_basename) > 0) {
            recorder.input_basename  = temp_basename;
            recorder.input_filename  = temp_basename;
            recorder.infile_extension = (strlen(temp_extension) > 0)?temp_extension:".mwrm";
            recorder.input_filename += recorder.infile_extension;
        }

        if (recorder.mwrm_path.back() != '/'){
            recorder.mwrm_path.push_back('/');
        }
        if (recorder.hash_path.back() != '/'){
            recorder.hash_path.push_back('/');
        }

    }
    recorder.full_path = recorder.mwrm_path + recorder.input_filename;

    if (verbose) {
        LOG(LOG_INFO, "Input file full_path=\"%s\"", recorder.full_path);
        LOG(LOG_INFO, "Input file base name=\"%s\"", recorder.input_filename);
        LOG(LOG_INFO, "hash_path=\"%s\"", recorder.hash_path);
        LOG(LOG_INFO, "mwrm_path=\"%s\"", recorder.mwrm_path);
    }

    if (is_encrypted_file(recorder.full_path.c_str(), recorder.infile_is_encrypted) == -1) {
        std::cerr << "Input file is missing.\n";
        return -1;
    }

    if (options.count("encryption") > 0) {
         if (0 == strcmp(recorder.wrm_encryption.c_str(), "enable")) {
            recorder.encryption_type = TraceType::cryptofile;
        }
        else if (0 == strcmp(recorder.wrm_encryption.c_str(), "disable")) {
            recorder.encryption_type = TraceType::localfile;
        }
        else if (0 == strcmp(recorder.wrm_encryption.c_str(), "original")) {
            recorder.encryption_type = recorder.infile_is_encrypted ? TraceType::cryptofile : TraceType::localfile;
        }
        else {
            std::cerr << "Unknown wrm encryption parameter\n\n";
            return -1;
        }
    }

    recorder.full_video = (options.count("full") > 0);
    recorder.show_file_metadata = (options.count("meta"             ) > 0);
    recorder.show_statistics    = (options.count("statistics"       ) > 0);

    if (recorder.output_filename.size()) {
        std::string directory = PNG_PATH "/";
        std::string filename                ;
        std::string extension = ".mwrm"     ;

        ParsePath(recorder.output_filename.c_str(), directory, filename, extension);
        MakePath(recorder.output_filename, directory.c_str(), filename.c_str(), extension.c_str());
        std::cout << "Output file is \"" << recorder.output_filename << "\".\n";
    }

    return 0;
}

extern "C" {
    __attribute__((__visibility__("default")))
    int recmemcpy(char * dest, char * source, int len)
    {
        ::memcpy(dest, source, static_cast<size_t>(len));
        return 0;
    }

    __attribute__((__visibility__("default")))
    int do_main(int argc, char const ** argv,
            get_hmac_key_prototype * hmac_fn,
            get_trace_key_prototype * trace_fn)
    {
        OpenSSL_add_all_digests();

        int arg_used = 0;


          int command = 0;
//        int command = ends_with(argv[arg_used], {"recorder.py", "verifier.py", "decoder.py"});
//        if (command){
//            command = command - 1;
//        }
//        // default command is redrec;

        if (argc > arg_used){
            command = in(argv[arg_used+1], {"redrec", "redver", "reddec"});
            if (command){
                command = command - 1;
                arg_used++;
            }
            // default command is previous one;
        }

        Inifile ini;
        ini.set<cfg::debug::config>(false);

        UdevRandom rnd;
        CryptoContext cctx;
        cctx.set_get_hmac_key_cb(hmac_fn);
        cctx.set_get_trace_key_cb(trace_fn);

        uint8_t tmp[32] = {};
        for (auto a : {0, 1}) {
            if (argc < arg_used + 1){
                break;
            }
            auto k = argv[arg_used+1];
            if (strlen(k) != 64){
                break;
            }
            int c1 = -1;
            int c2 = -1;
            for (unsigned i = 0; i < 32; ++i) {
                auto char_to_hex = [](char c){
                    auto in = [&c](char left, char right) { return left <= c && c <= right; };
                    return
                        in('0', '9') ? c-'0'
                        : in('a', 'f') ? 10 + c-'a'
                        : in('A', 'F') ? 10 + c-'A'
                        : -1;
                };
                c1 = char_to_hex(k[i*2]);
                c2 = char_to_hex(k[i*2+1]);
                if (c1 == -1 or c2 == -1){
                    break;
                }
                tmp[i] = c1 << 4 | c2;
            }
            // if any character not an hexa digit, ignore option
            if (c1 == -1 or c2 == -1){
                break;
            }
            if (a == 0){
                cctx.set_hmac_key(tmp);
            }
            else {
                cctx.set_master_key(tmp);
            }
            arg_used++;
        }

        argv += arg_used;
        argc -= arg_used;
        int res = -1;

        uint32_t    verbose     = 0;
        RecorderParams rp;
        // TODO: annoying, if we read default hash_path and mwrm_path from ini
        // we should do that after config_filename was eventually changed...

        if (parse_command_line_options(argc, argv, rp, ini, verbose) < 0){
            // parsing error
            return -1;
        }

        switch (command){
        case 0: // RECorder
        try {
            init_signals();

            if (rp.input_filename.empty()) {
                std::cerr << "Missing input filename : use -i filename\n\n";
                return -1;
            }

            if (!rp.show_file_metadata
            && !rp.show_statistics
            && !rp.auto_output_file
            && rp.output_filename.empty()) {
                std::cerr << "Missing output filename : use -o filename\n\n";
                return -1;
            }

            if (rp.chunk) {
                rp.flv_break_interval = 60*10; // 10 minutes
                rp.png_params.png_interval = std::chrono::seconds{1};
            }

            if (rp.output_filename.length()
            && !rp.full_video
            && !bool(rp.capture_flags)) {
                std::cerr << "Missing target format : need --png, --ocr, --flv, --full, --wrm or --chunk" << std::endl;
                return 1;
            }

            // TODO before continuing to work with input file, check if it's mwrm or wrm and use right object in both cases

            // TODO also check if it contains any wrm at all and at wich one we should start depending on input time
            // TODO if start and stop time are outside wrm, userreplay(s should also be warned

            res = replay(rp.mwrm_path, rp.input_basename, rp.infile_extension,
                          rp.hash_path,
                          rp.capture_flags,
                          rp.json_pgs ? UpdateProgressData::JSON_FORMAT : UpdateProgressData::OLD_FORMAT,
                          rp.chunk,
                          rp.ocr_version,
                          rp.output_filename,
                          rp.begin_cap,
                          rp.end_cap,
                          rp.png_params,
                          rp.flv_params,
                          rp.wrm_color_depth,
                          rp.wrm_frame_interval,
                          rp.wrm_break_interval,
                          false, // no_timestamp,
                          rp.infile_is_encrypted,
                          rp.order_count,
                          rp.show_file_metadata,
                          rp.show_statistics,
                          rp.clear,
                          rp.full_video,
                          rp.remove_input_file,
                          rp.wrm_compression_algorithm_,
                          rp.flv_break_interval,
                          rp.encryption_type,
                          ini, cctx, rnd,
                          verbose);

            } catch (const Error & e) {
                std::printf("decrypt failed: with id=%d\n", e.id);
            }
        break;
        case 1: // VERifier
            ini.set<cfg::debug::config>(false);
            try {

                encryption_type(rp.full_path, cctx);

                res = check_encrypted_or_checksumed(
                    rp.input_filename, rp.mwrm_path, rp.hash_path,
                    rp.quick_check, rp.ignore_stat_info, rp.update_stat_info, verbose, cctx
                );
                std::puts(res == 0 ? "verify ok\n" : "verify failed\n");
            } catch (const Error & e) {
                std::printf("verify failed: with id=%d\n", e.id);
            }
        break;
        default: // DECrypter
            try {
                // TODO file is unused
                local_fd file(rp.full_path, O_RDONLY);

                if (!file.is_open()) {
                    std::cerr << "can't open file " << rp.full_path << "\n\n";
                    std::puts("decrypt failed\n");
                    return -1;
                }

                if (0 == encryption_type(rp.full_path, cctx)){
                    std::puts("Input file is not encrypted\n");
                    return 0;
                }

                ifile_read_encrypted in_t(cctx, 1);

                ssize_t res = -1;
                local_fd fd1(rp.output_filename, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);

                if (fd1.is_open()) {
                    OutFileTransport out_t(fd1.fd());

                    try {
                        char mem[4096];

                        in_t.open(rp.full_path.c_str());
                        while (1) {
                            res = in_t.read(mem, sizeof(mem));
                            if (res == 0){
                                break;
                            }
                            if (res < 0){
                                break;
                            }
                            out_t.send(mem, res);
                        }
                    }
                    catch (Error const & e) {
                        LOG(LOG_INFO, "Exited on exception: %s", e.errmsg());
                        res = -1;
                    }
                }
                else {
                    std::cerr << strerror(errno) << std::endl << std::endl;
                }
                if (res == 0){
                    std::puts("decrypt ok\n");
                    return 0;
                }
                else {
                    std::puts("decrypt failed\n");
                    return -1;
                }
            } catch (const Error & e) {
                std::printf("decrypt failed: with id=%d\n", e.id);
            }
        break;
        }
        return res;
    }
}
