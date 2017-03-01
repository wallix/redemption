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

#include <cerrno>
#include <cstddef>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils/difftimeval.hpp"

#include "gdi/capture_api.hpp"
#include "capture/wrm_params.hpp"

#include "gdi/kbd_input_api.hpp"
#include "gdi/capture_api.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/capture_probe_api.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/caches/bmpcache.hpp"
#include "core/RDP/caches/pointercache.hpp"

#include "gdi/dump_png24.hpp"
#include "core/RDP/RDPDrawable.hpp"
#include "core/RDP/RDPSerializer.hpp"
#include "utils/compression_transport_builder.hpp"
#include "core/RDP/share.hpp"

// TODO enum class
enum {
    META_FILE           = 1006,
    TIMESTAMP           = 1008,
    POINTER             = 1009,
    POINTER2            = 1010,
    LAST_IMAGE_CHUNK    = 0x1000,   // 4096
    PARTIAL_IMAGE_CHUNK = 0x1001,   // 4097
    SAVE_STATE          = 0x1002,   // 4098
    RESET_CHUNK         = 0x1003,   // 4099
    SESSION_UPDATE      = 0x1004,

    INVALID_CHUNK       = 0x8000
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

typedef wrmcapture_FilenameGenerator::Format wrmcapture_FilenameFormat;

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


class iofdbuf
{
public:
    int iofdbuf_fd;

    explicit iofdbuf() noexcept
    : iofdbuf_fd(-1)
    {}

    iofdbuf(iofdbuf const &) = delete ;
    iofdbuf&operator=(iofdbuf const &) = delete ;

    ~iofdbuf()
    {
        if (-1 != this->iofdbuf_fd) {
            ::close(this->iofdbuf_fd);
        }
    }

    int open(int fd)
    {
        if (-1 != this->iofdbuf_fd) {
            ::close(this->iofdbuf_fd);
        }
        this->iofdbuf_fd = fd;
        return fd;
    }

    int close()
    {
        if (-1 != this->iofdbuf_fd) {
            const int ret = ::close(this->iofdbuf_fd);
            this->iofdbuf_fd = -1;
            return ret;
        }
        return 0;
    }

    bool is_open() const noexcept
    {
        return -1 != this->iofdbuf_fd;
    }

    ssize_t write(const void * data, size_t len) const
    {
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            ssize_t ret = ::write(this->iofdbuf_fd, static_cast<const char*>(data) + total_sent, remaining_len);
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
};




struct wrmcapture_OutFilenameSequenceTransport : public Transport
{
    wrmcapture_OutFilenameSequenceTransport(
        wrmcapture_FilenameGenerator::Format format,
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid,
        auth_api * authentifier)
    : buf(wrmcapture_out_sequence_filename_buf_param(format, prefix, filename, extension, groupid))
    {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }
    }

    const wrmcapture_FilenameGenerator * seqgen() const noexcept
    { return &(this->buf.seqgen()); }

    bool next() override {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        const ssize_t res = this->buf.next();
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
        this->buf.request_full_cleaning();
    }

    ~wrmcapture_OutFilenameSequenceTransport() {
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

    class wrmcapture_out_sequence_filename_buf_impl
    {
        char current_filename_[1024];
        wrmcapture_FilenameGenerator filegen_;
        iofdbuf buf_;
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

        const wrmcapture_FilenameGenerator & seqgen() const noexcept
        { return this->filegen_; }

        iofdbuf & buf() noexcept
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
    } buf;
};


struct wrmcapture_out_meta_sequence_filename_buf_noparam
{
    wrmcapture_out_sequence_filename_buf_param sq_params;
    time_t sec;
    const char * hash_prefix;

    wrmcapture_out_meta_sequence_filename_buf_noparam(
        time_t start_sec,
        wrmcapture_FilenameGenerator::Format format,
        const char * const hash_prefix,
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid)
    : sq_params(format, prefix, filename, extension, groupid)
    , sec(start_sec)
    , hash_prefix(hash_prefix)
    {}
};

struct wrmcapture_ocrypto_filename_params
{
    CryptoContext & crypto_ctx;
    Random & rnd;
};


struct wrmcapture_out_meta_sequence_filename_buf_param
{
    wrmcapture_out_sequence_filename_buf_param sq_params;
    time_t sec;
    wrmcapture_ocrypto_filename_params meta_buf_params;
    const char * hash_prefix;

    wrmcapture_out_meta_sequence_filename_buf_param(
        time_t start_sec,
        wrmcapture_FilenameGenerator::Format format,
        const char * const hash_prefix,
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid,
        wrmcapture_ocrypto_filename_params const & meta_buf_params)
    : sq_params(format, prefix, filename, extension, groupid)
    , sec(start_sec)
    , meta_buf_params(meta_buf_params)
    , hash_prefix(hash_prefix)
    {}
};

struct wrmcapture_out_meta_sequence_filename_buf_param_cctx
{
    wrmcapture_out_sequence_filename_buf_param sq_params;
    time_t sec;
    CryptoContext& meta_buf_params;
    const char * hash_prefix;

    wrmcapture_out_meta_sequence_filename_buf_param_cctx(
        time_t start_sec,
        wrmcapture_FilenameGenerator::Format format,
        const char * const hash_prefix,
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid,
        CryptoContext& meta_buf_params)
    : sq_params(format, prefix, filename, extension, groupid)
    , sec(start_sec)
    , meta_buf_params(meta_buf_params)
    , hash_prefix(hash_prefix)
    {}
};


struct wrmcapture_out_hash_meta_sequence_filename_buf_param_cctx
{
    wrmcapture_out_meta_sequence_filename_buf_param_cctx meta_sq_params;
    CryptoContext& filter_params;
    CryptoContext & cctx;

    wrmcapture_out_hash_meta_sequence_filename_buf_param_cctx(
        CryptoContext & cctx,
        time_t start_sec,
        wrmcapture_FilenameGenerator::Format format,
        const char * const hash_prefix,
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid,
        CryptoContext & filter_params)
    : meta_sq_params(start_sec, format, hash_prefix, prefix, filename, extension, groupid, filter_params)
    , filter_params(filter_params)
    , cctx(cctx)
    {}
};


struct wrmcapture_out_hash_meta_sequence_filename_buf_param_ocrypto
{
    wrmcapture_out_meta_sequence_filename_buf_param meta_sq_params;
    wrmcapture_ocrypto_filename_params filter_params;
    CryptoContext & cctx;

    wrmcapture_out_hash_meta_sequence_filename_buf_param_ocrypto(
        CryptoContext & cctx,
        time_t start_sec,
        wrmcapture_FilenameGenerator::Format format,
        const char * const hash_prefix,
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid,
        wrmcapture_ocrypto_filename_params const & filter_params)
    : meta_sq_params(start_sec, format, hash_prefix, prefix, filename, extension, groupid, filter_params)
    , filter_params(filter_params)
    , cctx(cctx)
    {}
};

struct wrmcapture_MetaFilename
{
    char filename[2048];

    wrmcapture_MetaFilename(const char * path, const char * basename,
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

    wrmcapture_MetaFilename(wrmcapture_MetaFilename const &) = delete;
    wrmcapture_MetaFilename & operator = (wrmcapture_MetaFilename const &) = delete;
};

typedef unsigned char wrmcapture_hash_type[MD_HASH_LENGTH*2];

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

struct ocrypto {
    char           encrypt_buf[CRYPTO_BUFFER_SIZE]; //
    EVP_CIPHER_CTX encrypt_ectx;                    // [en|de]cryption context
    EVP_MD_CTX     encrypt_hctx;                    // hash context
    EVP_MD_CTX     encrypt_hctx4k;                  // hash context
    uint32_t       encrypt_pos;                     // current position in buf
    uint32_t       encrypt_raw_size;                // the unciphered/uncompressed file size
    uint32_t       encrypt_file_size;               // the current file size

    /* Encrypt src_buf into dst_buf. Update dst_sz with encrypted output size
     * Return 0 on success, negative value on error
     */
    int xaes_encrypt(const unsigned char *src_buf, uint32_t src_sz, unsigned char *dst_buf, uint32_t *dst_sz)
    {
        int safe_size = *dst_sz;
        int remaining_size = 0;

        /* allows reusing of ectx for multiple encryption cycles */
        if (EVP_EncryptInit_ex(&this->encrypt_ectx, nullptr, nullptr, nullptr, nullptr) != 1){
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not prepare encryption context!\n", getpid());
            return -1;
        }
        if (EVP_EncryptUpdate(&this->encrypt_ectx, dst_buf, &safe_size, src_buf, src_sz) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could encrypt data!\n", getpid());
            return -1;
        }
        if (EVP_EncryptFinal_ex(&this->encrypt_ectx, dst_buf + safe_size, &remaining_size) != 1){
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
        if (::EVP_DigestUpdate(&this->encrypt_hctx, src_buf, src_sz) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash!\n", ::getpid());
            return -1;
        }
        if (this->encrypt_file_size < 4096) {
            size_t remaining_size = 4096 - this->encrypt_file_size;
            size_t hashable_size = MIN(remaining_size, src_sz);
            if (::EVP_DigestUpdate(&this->encrypt_hctx4k, src_buf, hashable_size) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update 4k hash!\n", ::getpid());
                return -1;
            }
        }
        return 0;
    }

    int encrypt_open(uint8_t * buffer, size_t buflen, size_t & towrite, const unsigned char * trace_key, CryptoContext & cctx, const unsigned char * iv)
    {
        ::memset(this->encrypt_buf, 0, sizeof(this->encrypt_buf));
        ::memset(&this->encrypt_ectx, 0, sizeof(this->encrypt_ectx));
        ::memset(&this->encrypt_hctx, 0, sizeof(this->encrypt_hctx));
        ::memset(&this->encrypt_hctx4k, 0, sizeof(this->encrypt_hctx4k));
        this->encrypt_pos = 0;
        this->encrypt_raw_size = 0;
        this->encrypt_file_size = 0;

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

        ::EVP_CIPHER_CTX_init(&this->encrypt_ectx);
        if (::EVP_EncryptInit_ex(&this->encrypt_ectx, cipher, nullptr, key, iv) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize encrypt context\n", ::getpid());
            return -1;
        }

        // MD stuff
        const EVP_MD * md = EVP_get_digestbyname(MD_HASH_NAME);
        if (!md) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not find message digest algorithm!\n", ::getpid());
            return -1;
        }

        ::EVP_MD_CTX_init(&this->encrypt_hctx);
        ::EVP_MD_CTX_init(&this->encrypt_hctx4k);
        if (::EVP_DigestInit_ex(&this->encrypt_hctx, md, nullptr) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize MD hash context!\n", ::getpid());
            return -1;
        }
        if (::EVP_DigestInit_ex(&this->encrypt_hctx4k, md, nullptr) != 1) {
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
            if (::EVP_DigestUpdate(&this->encrypt_hctx, key_buf, blocksize) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash!\n", ::getpid());
                return -1;
            }
            if (::EVP_DigestUpdate(&this->encrypt_hctx4k, key_buf, blocksize) != 1) {
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


        if (buflen < 40){
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: buffer too small!\n", ::getpid());
            return -1;
        }
        ::memcpy(buffer, tmp_buf, 40);
        towrite += 40;
        // update file_size
        this->encrypt_file_size += 40;

        return this->xmd_update(tmp_buf, 40);
    }


};

class wrmcapture_ocrypto_filename_buf : ocrypto
{
    int file_fd;

    CryptoContext & cctx;
    Random & rnd;

public:
    explicit wrmcapture_ocrypto_filename_buf(wrmcapture_ocrypto_filename_params params)
    : file_fd(-1)
    , cctx(params.crypto_ctx)
    , rnd(params.rnd)
    {
        if (-1 != this->file_fd) {
            ::close(this->file_fd);
            this->file_fd = -1;
        }
    }

    ~wrmcapture_ocrypto_filename_buf()
    {
        if (this->is_open()) {
            this->close();
        }
    }


    

    ssize_t encrypt_write(const void * data, size_t len)
    {
        unsigned int remaining_size = len;
        while (remaining_size > 0) {
            // Check how much we can append into buffer
            unsigned int available_size = MIN(CRYPTO_BUFFER_SIZE - this->encrypt_pos, remaining_size);
            // Append and update pos pointer
            ::memcpy(this->encrypt_buf + this->encrypt_pos, static_cast<const char*>(data) + (len - remaining_size), available_size);
            this->encrypt_pos += available_size;
            // If buffer is full, flush it to disk
            if (this->encrypt_pos == CRYPTO_BUFFER_SIZE) {
                if (this->encrypt_flush()) {
                    return -1;
                }
            }
            remaining_size -= available_size;
        }
        // Update raw size counter
        this->encrypt_raw_size += len;
        return len;
    }

    /* Flush procedure (compression, encryption, effective file writing)
     * Return 0 on success, negatif on error
     */
    int encrypt_flush()
    {
        // No data to flush
        if (!this->encrypt_pos) {
            return 0;
        }

        // Compress
        // TODO: check this
        char compressed_buf[65536];
        //char compressed_buf[compressed_buf_sz];
        size_t compressed_buf_sz = ::snappy_max_compressed_length(this->encrypt_pos);
        snappy_status status = snappy_compress(this->encrypt_buf, this->encrypt_pos, compressed_buf, &compressed_buf_sz);

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

        if (const ssize_t err = this->encrypt_raw_write(ciphered_buf, ciphered_buf_sz)) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Write error : %s\n", ::getpid(), ::strerror(errno));
            return err;
        }
        if (-1 == this->xmd_update(&ciphered_buf, ciphered_buf_sz)) {
            return -1;
        }
        this->encrypt_file_size += ciphered_buf_sz;

        // Reset buffer
        this->encrypt_pos = 0;
        return 0;
    }

    int encrypt_close(unsigned char hash[MD_HASH_LENGTH << 1], const unsigned char * hmac_key)
    {
        int result = this->encrypt_flush();

        const uint32_t eof_magic = WABCRYPTOFILE_EOF_MAGIC;
        unsigned char tmp_buf[8] = {
            eof_magic & 0xFF,
            (eof_magic >> 8) & 0xFF,
            (eof_magic >> 16) & 0xFF,
            (eof_magic >> 24) & 0xFF,
            uint8_t(this->encrypt_raw_size & 0xFF),
            uint8_t((this->encrypt_raw_size >> 8) & 0xFF),
            uint8_t((this->encrypt_raw_size >> 16) & 0xFF),
            uint8_t((this->encrypt_raw_size >> 24) & 0xFF),
        };

        int write_ret1 = this->encrypt_raw_write(tmp_buf, 8);
        if (write_ret1){
            // TOOD: actual error code could help
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Write error : %s\n", ::getpid(), ::strerror(errno));
        }
        this->encrypt_file_size += 8;

        this->xmd_update(tmp_buf, 8);

        if (hash) {
            unsigned char tmp_hash[MD_HASH_LENGTH << 1];
            if (::EVP_DigestFinal_ex(&this->encrypt_hctx4k, tmp_hash, nullptr) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute 4k MD digests\n", ::getpid());
                result = -1;
                tmp_hash[0] = '\0';
            }
            if (::EVP_DigestFinal_ex(&this->encrypt_hctx, tmp_hash + MD_HASH_LENGTH, nullptr) != 1) {
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

    ///\return 0 if success, otherwise a negatif number
    ssize_t encrypt_raw_write(void * data, size_t len)
    {
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            ssize_t ret = ::write(this->file_fd,
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
        return 0;
    }

    int open(const char * filename, mode_t mode = 0600)
    {
        if (-1 != this->file_fd) {
            ::close(this->file_fd);
            this->file_fd = -1;
        }
        this->file_fd = ::open(filename, O_WRONLY | O_CREAT, mode);
        int err = this->file_fd;

        if (err < 0) {
            return err;
        }

        unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
        size_t base_len = 0;
        const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(filename, base_len));
        this->cctx.get_derived_key(trace_key, base, base_len);
        unsigned char iv[32];
        this->rnd.random(iv, 32);
        
        uint8_t buffer[40];
        size_t towrite = 0;
        
        err = this->encrypt_open(buffer, sizeof(buffer), towrite, trace_key, this->cctx, iv);
        if (!err) {
            err = this->encrypt_raw_write(buffer, towrite);
        }
        return err;
    }

    ssize_t write(const void * data, size_t len)
    { return this->encrypt_write(data, len); }

    int close(unsigned char hash[MD_HASH_LENGTH << 1])
    {
        const int res1 = this->encrypt_close(hash, this->cctx.get_hmac_key());
        int res2 = 0;
        if (-1 != this->file_fd) {
            res2 = ::close(this->file_fd);
            this->file_fd = -1;
        }
        return res1 < 0 ? res1 : (res2 < 0 ? res2 : 0);
    }

    int close()
    {
        unsigned char hash[MD_HASH_LENGTH << 1];
        return this->close(hash);
    }

    bool is_open() const noexcept
    { return -1 != this->file_fd; }

    int flush() const
    { return 0; }
};


class wrmcapture_ocrypto_filter : ocrypto
{

    iofdbuf & snk;

public:
    CryptoContext & cctx;
    Random & rnd;

    explicit wrmcapture_ocrypto_filter(iofdbuf & buf, wrmcapture_ocrypto_filename_params params)
    : snk(buf)
    , cctx(params.crypto_ctx)
    , rnd(params.rnd)
    {}

    int open(char const * filename) {
        unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher

        size_t base_len = 0;
        const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(filename, base_len));

        this->cctx.get_derived_key(trace_key, base, base_len);
        unsigned char iv[32];
        this->rnd.random(iv, 32);

        ::memset(this->encrypt_buf, 0, sizeof(this->encrypt_buf));
        ::memset(&this->encrypt_ectx, 0, sizeof(this->encrypt_ectx));
        ::memset(&this->encrypt_hctx, 0, sizeof(this->encrypt_hctx));
        ::memset(&this->encrypt_hctx4k, 0, sizeof(this->encrypt_hctx4k));
        this->encrypt_pos = 0;
        this->encrypt_raw_size = 0;
        this->encrypt_file_size = 0;

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

        ::EVP_CIPHER_CTX_init(&this->encrypt_ectx);
        if (::EVP_EncryptInit_ex(&this->encrypt_ectx, cipher, nullptr, key, iv) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize encrypt context\n", ::getpid());
            return -1;
        }

        // MD stuff
        const EVP_MD * md = EVP_get_digestbyname(MD_HASH_NAME);
        if (!md) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not find message digest algorithm!\n", ::getpid());
            return -1;
        }

        ::EVP_MD_CTX_init(&this->encrypt_hctx);
        ::EVP_MD_CTX_init(&this->encrypt_hctx4k);
        if (::EVP_DigestInit_ex(&this->encrypt_hctx, md, nullptr) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize MD hash context!\n", ::getpid());
            return -1;
        }
        if (::EVP_DigestInit_ex(&this->encrypt_hctx4k, md, nullptr) != 1) {
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
            if (::EVP_DigestUpdate(&this->encrypt_hctx, key_buf, blocksize) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash!\n", ::getpid());
                return -1;
            }
            if (::EVP_DigestUpdate(&this->encrypt_hctx4k, key_buf, blocksize) != 1) {
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
        if (const ssize_t write_ret = this->raw_write(tmp_buf, 40)){
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: write error! error=%s\n", ::getpid(), ::strerror(errno));
            return write_ret;
        }
        // update file_size
        this->encrypt_file_size += 40;

        return this->xmd_update(tmp_buf, 40);
    }

    ssize_t write(const void * data, size_t len)
    {
        unsigned int remaining_size = len;
        while (remaining_size > 0) {
            // Check how much we can append into buffer
            unsigned int available_size = MIN(CRYPTO_BUFFER_SIZE - this->encrypt_pos, remaining_size);
            // Append and update pos pointer
            ::memcpy(this->encrypt_buf + this->encrypt_pos, static_cast<const char*>(data) + (len - remaining_size), available_size);
            this->encrypt_pos += available_size;
            // If buffer is full, flush it to disk
            if (this->encrypt_pos == CRYPTO_BUFFER_SIZE) {
                if (this->flush()) {
                    return -1;
                }
            }
            remaining_size -= available_size;
        }
        // Update raw size counter
        this->encrypt_raw_size += len;
        return len;
    }

    /* Flush procedure (compression, encryption, effective file writing)
     * Return 0 on success, negatif on error
     */
    int flush()
    {
        // No data to flush
        if (!this->encrypt_pos) {
            return 0;
        }

        // Compress
        // TODO: check this
        char compressed_buf[65536];
        //char compressed_buf[compressed_buf_sz];
        size_t compressed_buf_sz = ::snappy_max_compressed_length(this->encrypt_pos);
        snappy_status status = snappy_compress(this->encrypt_buf, this->encrypt_pos, compressed_buf, &compressed_buf_sz);

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

        if (const ssize_t err = this->raw_write(ciphered_buf, ciphered_buf_sz)) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Write error : %s\n", ::getpid(), ::strerror(errno));
            return err;
        }
        if (-1 == this->xmd_update(&ciphered_buf, ciphered_buf_sz)) {
            return -1;
        }
        this->encrypt_file_size += ciphered_buf_sz;

        // Reset buffer
        this->encrypt_pos = 0;
        return 0;
    }

    int close(unsigned char hash[MD_HASH_LENGTH << 1], const unsigned char * hmac_key)
    {
        int result = this->flush();

        const uint32_t eof_magic = WABCRYPTOFILE_EOF_MAGIC;
        unsigned char tmp_buf[8] = {
            eof_magic & 0xFF,
            (eof_magic >> 8) & 0xFF,
            (eof_magic >> 16) & 0xFF,
            (eof_magic >> 24) & 0xFF,
            uint8_t(this->encrypt_raw_size & 0xFF),
            uint8_t((this->encrypt_raw_size >> 8) & 0xFF),
            uint8_t((this->encrypt_raw_size >> 16) & 0xFF),
            uint8_t((this->encrypt_raw_size >> 24) & 0xFF),
        };

        int write_ret1 = this->raw_write(tmp_buf, 8);
        if (write_ret1){
            // TOOD: actual error code could help
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Write error : %s\n", ::getpid(), ::strerror(errno));
        }
        this->encrypt_file_size += 8;

        this->xmd_update(tmp_buf, 8);

        if (hash) {
            unsigned char tmp_hash[MD_HASH_LENGTH << 1];
            if (::EVP_DigestFinal_ex(&this->encrypt_hctx4k, tmp_hash, nullptr) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute 4k MD digests\n", ::getpid());
                result = -1;
                tmp_hash[0] = '\0';
            }
            if (::EVP_DigestFinal_ex(&this->encrypt_hctx, tmp_hash + MD_HASH_LENGTH, nullptr) != 1) {
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
    ssize_t raw_write(void * data, size_t len)
    {
        ssize_t err = this->snk.write(data, len);
        return err < ssize_t(len) ? (err < 0 ? err : -1) : 0;
    }


};


class wrmcapture_cctx_ochecksum_file
{
    int fd;

    static constexpr size_t nosize = ~size_t{};
    static constexpr size_t quick_size = 4096;

    SslHMAC_Sha256_Delayed hmac;
    SslHMAC_Sha256_Delayed quick_hmac;
    unsigned char const (&hmac_key)[MD_HASH_LENGTH];
    size_t file_size = nosize;

public:
    explicit wrmcapture_cctx_ochecksum_file(CryptoContext & cctx)
    : fd(-1)
    , hmac_key(cctx.get_hmac_key())
    {}

    ~wrmcapture_cctx_ochecksum_file()
    {
        this->close();
    }

    wrmcapture_cctx_ochecksum_file(wrmcapture_cctx_ochecksum_file const &) = delete;
    wrmcapture_cctx_ochecksum_file & operator=(wrmcapture_cctx_ochecksum_file const &) = delete;

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

    int flush() const
    { return 0; }
};


static inline int wrmcapture_write_meta_file_cctx(
    wrmcapture_cctx_ochecksum_file & writer, const char * filename,
    time_t start_sec, time_t stop_sec,
    wrmcapture_hash_type const * hash
);

static inline int wrmcapture_write_meta_file_cctx(
    wrmcapture_cctx_ochecksum_file & writer, const char * filename,
    time_t start_sec, time_t stop_sec,
    wrmcapture_hash_type const * hash
) {
    struct stat stat;
    int err = ::stat(filename, &stat);
    if (err){
        return err;
    }
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
    len += std::sprintf(
        mes + len,
        " %lld %lld",
        ll(start_sec),
        ll(stop_sec)
    );

    char * p = mes + len;
    auto write = [&p](unsigned char const * hash) {
        *p++ = ' ';                // 1 octet
        for (unsigned c : iter(hash, MD_HASH_LENGTH)) {
            sprintf(p, "%02x", c); // 64 octets (hash)
            p += 2;
        }
    };
    write(reinterpret_cast<const unsigned char *>(&hash[0]));
    write(reinterpret_cast<const unsigned char *>(&hash[MD_HASH_LENGTH]));
    *p++ = '\n';

    ssize_t res = writer.write(mes, p-mes);

    if (res < p-mes) {
        return res < 0 ? res : 1;
    }
    return 0;
}





class wrmcapture_out_meta_sequence_filename_buf_impl_ocrypto_filename_buf
{
    char current_filename_[1024];
    wrmcapture_FilenameGenerator filegen_;
    iofdbuf buf_;
    unsigned num_file_;
    int groupid_;

public:
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

    const wrmcapture_FilenameGenerator & seqgen() const noexcept
    { return this->filegen_; }

    iofdbuf & buf() noexcept
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
// =======================================================================

public:
    wrmcapture_ocrypto_filename_buf meta_buf_;

protected:
    wrmcapture_MetaFilename mf_;
    wrmcapture_MetaFilename hf_;
    time_t start_sec_;
    time_t stop_sec_;

public:
    explicit wrmcapture_out_meta_sequence_filename_buf_impl_ocrypto_filename_buf(
        wrmcapture_out_meta_sequence_filename_buf_param const & params
    )
    : filegen_(params.sq_params.format, params.sq_params.prefix, params.sq_params.filename, params.sq_params.extension)
    , buf_()
    , num_file_(0)
    , groupid_(params.sq_params.groupid)
    , meta_buf_(params.meta_buf_params)
    , mf_(params.sq_params.prefix, params.sq_params.filename, params.sq_params.format)
    , hf_(params.hash_prefix, params.sq_params.filename, params.sq_params.format)
    , start_sec_(params.sec)
    , stop_sec_(params.sec)
    {
        this->current_filename_[0] = 0;
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
        const int res2 = (this->meta_buf_.is_open() ? this->meta_buf_.close() : 0);
        int err = res1 ? res1 : res2;
        if (!err) {
            char const * hash_filename = this->hf_.filename;
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


            // TODO: CGR make that a function and use it to write hash file
            int fd = ::open(hash_filename, O_WRONLY | O_CREAT, S_IRUSR|S_IRGRP);

            if (fd < 0) {
                int e = errno;
                LOG(LOG_ERR, "Open to transport failed: code=%d", e);
                errno = e;
                return 1;
            }

            char header[] = "v2\n\n\n";

            size_t remaining_len = sizeof(header)-1;
            size_t total_sent = 0;
            while (remaining_len) {
                ssize_t ret = ::write(fd,  header + total_sent, remaining_len);
                if (ret <= 0){
                    if (errno == EINTR){
                        continue;
                    }
                    ::close(fd);
                    LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                    return 1;
                }
                remaining_len -= ret;
                total_sent += ret;
            }

            struct stat stat;
            int err = ::stat(meta_filename, &stat);
            if (err) {
                ::close(fd);
                LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                return 1;
            }

            auto pfile = filename;
            auto epfile = filename;
            for (; *epfile; ++epfile) {
                if (*epfile == '\\') {
                    ssize_t len = epfile - pfile + 1;
                    size_t remaining_len = len;
                    size_t total_sent = 0;
                    while (remaining_len) {
                        ssize_t ret = ::write(fd,  pfile + total_sent, remaining_len);
                        if (ret <= 0){
                            if (errno == EINTR){
                                continue;
                            }
                            ::close(fd);
                            LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                            return 1;
                        }
                        remaining_len -= ret;
                        total_sent += ret;
                    }
                    pfile = epfile;
                }
                if (*epfile == ' ') {
                    ssize_t len = epfile - pfile;
                    size_t remaining_len = len;
                    size_t total_sent = 0;
                    while (remaining_len) {
                        ssize_t ret = ::write(fd,  pfile + total_sent, remaining_len);
                        if (ret <= 0){
                            if (errno == EINTR){
                                continue;
                            }
                            ::close(fd);
                            LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                            return 1;
                        }
                        remaining_len -= ret;
                        total_sent += ret;
                    }
                    remaining_len = 1u;
                    total_sent = 0;
                    for (;;) {
                        ssize_t ret = ::write(fd,  "\\" + total_sent, 1u);
                        if (ret <= 0){
                            if (errno == EINTR){
                                continue;
                            }
                            ::close(fd);
                            LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                            return 1;
                        }
                    }
                    pfile = epfile;
                }
            }

            if (pfile != epfile) {
                ssize_t len = epfile - pfile;
                size_t remaining_len = len;
                size_t total_sent = 0;
                while (remaining_len) {
                    ssize_t ret = ::write(fd,  pfile + total_sent, remaining_len);
                    if (ret <= 0){
                        if (errno == EINTR){
                            continue;
                        }
                        ::close(fd);
                        LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                        return 1;
                    }
                    remaining_len -= ret;
                    total_sent += ret;
                }
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

            char * p = mes + len;
            *p++ = '\n';

            remaining_len = p-mes;
            total_sent = 0;
            while (remaining_len) {
                ssize_t ret = ::write(fd,  mes + total_sent, remaining_len);
                if (ret <= 0){
                    if (errno == EINTR){
                        continue;
                    }
                    ::close(fd);
                    LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                    return 1;
                }
                remaining_len -= ret;
                total_sent += ret;
            }
            ::close(fd);
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

        auto & writer = this->meta_buf_;
        auto start_sec = this->start_sec_;
        auto stop_sec = this->stop_sec_+1;

        struct stat stat;
        int err = ::stat(filename, &stat);
        if (err){
            return err;
        }
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
        len += std::sprintf(
            mes + len,
            " %lld %lld",
            ll(start_sec),
            ll(stop_sec)
        );

        char * p = mes + len;
        if (hash){
            auto write = [&p](unsigned char const * hash) {
                *p++ = ' ';                // 1 octet
                for (unsigned c : iter(hash, MD_HASH_LENGTH)) {
                    sprintf(p, "%02x", c); // 64 octets (hash)
                    p += 2;
                }
            };
            write(&(*hash)[0]);
            write(&(*hash)[MD_HASH_LENGTH]);
        }
        *p++ = '\n';

        ssize_t res = writer.write(mes, p-mes);

        if (res < p-mes) {
            return res < 0 ? res : 1;
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
        unsigned i = this->num_file_ + 1;
        while (i > 0 && !::unlink(this->filegen_.get(--i))) {
        }
        if (this->buf_.is_open()) {
            this->buf_.close();
        }
        ::unlink(this->mf_.filename);
    }

    void update_sec(time_t sec)
    { this->stop_sec_ = sec; }
};



// meta_buf_meta_buf_meta_buf_meta_buf_meta_buf_
class wrmcapture_out_meta_sequence_filename_buf_impl_ofile_buf_out
{
    char current_filename_[1024];
    wrmcapture_FilenameGenerator filegen_;
    iofdbuf buf_;
    unsigned num_file_;
    int groupid_;

public:

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


    const wrmcapture_FilenameGenerator & seqgen() const noexcept
    { return this->filegen_; }

    iofdbuf & buf() noexcept
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

    int meta_buf_fd;
public:

    int meta_buf_open(const char * filename, mode_t mode)
    {
        this->meta_buf_close();
        this->meta_buf_fd = ::open(filename, O_WRONLY | O_CREAT, mode);
        return this->meta_buf_fd;
    }

    int meta_buf_close()
    {
        if (this->meta_buf_is_open()) {
            const int ret = ::close(this->meta_buf_fd);
            this->meta_buf_fd = -1;
            return ret;
        }
        return 0;
    }

    ssize_t meta_buf_write(const void * data, size_t len)
    {
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            ssize_t ret = ::write(this->meta_buf_fd,
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

    bool meta_buf_is_open() const noexcept
    { return -1 != this->meta_buf_fd; }

    int meta_buf_flush() const
    { return 0; }

    wrmcapture_MetaFilename mf_;
    wrmcapture_MetaFilename hf_;
    time_t start_sec_;
    time_t stop_sec_;

public:
    explicit wrmcapture_out_meta_sequence_filename_buf_impl_ofile_buf_out(
        wrmcapture_out_meta_sequence_filename_buf_noparam const & params
    )
    : filegen_(params.sq_params.format, params.sq_params.prefix, params.sq_params.filename, params.sq_params.extension)
    , buf_()
    , num_file_(0)
    , groupid_(params.sq_params.groupid)
    , meta_buf_fd(-1)
    , mf_(params.sq_params.prefix, params.sq_params.filename, params.sq_params.format)
    , hf_(params.hash_prefix, params.sq_params.filename, params.sq_params.format)
    , start_sec_(params.sec)
    , stop_sec_(params.sec)
    {
        this->current_filename_[0] = 0;
        if (this->meta_buf_open(this->mf_.filename, S_IRUSR | S_IRGRP | S_IWUSR) < 0) {
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

    ~wrmcapture_out_meta_sequence_filename_buf_impl_ofile_buf_out()
    {
        this->meta_buf_close();
    }

    int close()
    {
        const int res1 = this->next();
        const int res2 = (this->meta_buf_is_open() ? this->meta_buf_close() : 0);
        int err = res1 ? res1 : res2;
        if (!err) {
            char const * hash_filename = this->hf_.filename;
            char const * meta_filename = this->meta_filename();
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

                int flush() const
                { return 0; }
            } crypto_hash;

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
                    auto & writer = crypto_hash;
                    auto pfile = filename;
                    auto epfile = filename;
                    for (; *epfile; ++epfile) {
                        if (*epfile == '\\') {
                            ssize_t len = epfile - pfile + 1;
                            auto res = writer.write(pfile, len);
                            if (res < len) {
                                err = res < 0 ? res : 1;
                                LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n",
                                    hash_filename, err);
                                return 1;
                            }
                            pfile = epfile;
                        }
                        if (*epfile == ' ') {
                            ssize_t len = epfile - pfile;
                            auto res = writer.write(pfile, len);
                            if (res < len) {
                                err = res < 0 ? res : 1;
                                LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n",
                                    hash_filename, err);
                                return 1;
                            }
                            res = writer.write("\\", 1u);
                            if (res < 1) {
                                err = res < 0 ? res : 1;
                                LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n",
                                    hash_filename, err);
                                return 1;
                            }
                            pfile = epfile;
                        }
                    }

                    if (pfile != epfile) {
                        ssize_t len = epfile - pfile;
                        auto res = writer.write(pfile, len);
                        if (res < len) {
                            err = res < 0 ? res : 1;
                            LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n",
                                hash_filename, err);
                            return 1;
                        }
                    }
                    
                    if (!err) {
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

                        char * p = mes + len;
                        *p++ = '\n';

                        ssize_t res = writer.write(mes, p-mes);

                        if (res < p-mes) {
                            err = res < 0 ? res : 1;
                        }
                    }
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

        struct stat stat;
        int err = ::stat(filename, &stat);

        if (err){
            return err;
        }
        auto pfile = filename;
        auto epfile = filename;
        for (; *epfile; ++epfile) {
            if (*epfile == '\\') {
                ssize_t len = epfile - pfile + 1;
                auto res = this->meta_buf_write(pfile, len);
                if (res < len) {
                    return res < 0 ? res : 1;
                }
                pfile = epfile;
            }
            if (*epfile == ' ') {
                ssize_t len = epfile - pfile;
                auto res = this->meta_buf_write(pfile, len);
                if (res < len) {
                    return res < 0 ? res : 1;
                }
                res = this->meta_buf_write("\\", 1u);
                if (res < 1) {
                    return res < 0 ? res : 1;
                }
                pfile = epfile;
            }
        }

        if (pfile != epfile) {
            ssize_t len = epfile - pfile;
            auto res = this->meta_buf_write(pfile, len);
            if (res < len) {
                return res < 0 ? res : 1;
            }
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
        len += std::sprintf(
            mes + len,
            " %lld %lld",
            ll(this->start_sec_),
            ll(this->stop_sec_+1)
        );

        char * p = mes + len;
        if (hash) {
            auto write = [&p](unsigned char const * hash) {
                *p++ = ' ';                // 1 octet
                for (unsigned c : iter(hash, MD_HASH_LENGTH)) {
                    sprintf(p, "%02x", c); // 64 octets (hash)
                    p += 2;
                }
            };
            write(&(*hash[0]));
            write(&(*hash[MD_HASH_LENGTH]));
        }
        *p++ = '\n';

        ssize_t res = this->meta_buf_write(mes, p-mes);

        if (res < p-mes) {
            return res < 0 ? res : 1;
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
        unsigned i = this->num_file_ + 1;
        while (i > 0 && !::unlink(this->filegen_.get(--i))) {
        }
        if (this->buf_.is_open()) {
            this->buf_.close();
        }
        ::unlink(this->mf_.filename);
    }

    void update_sec(time_t sec)
    { this->stop_sec_ = sec; }
};

// meta_buf_meta_buf_meta_buf_

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

    int open(iofdbuf &, char const * /*filename*/) {
        return this->sum_buf.open();
    }

    int write(iofdbuf & buf, const void * data, size_t len) {
        this->sum_buf.write(data, len);
        return buf.write(data, len);
    }

    int close(iofdbuf &, wrmcapture_hash_type & hash, unsigned char const (&)[MD_HASH_LENGTH]) {
        return this->sum_buf.close(hash);
    }
};



class wrmcapture_out_meta_sequence_filename_buf_impl_cctx
{

// =====================================================================

        char current_filename_[1024];
        wrmcapture_FilenameGenerator filegen_;
        iofdbuf buf_;
        unsigned num_file_;
        int groupid_;

    public:

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

        const wrmcapture_FilenameGenerator & seqgen() const noexcept
        { return this->filegen_; }

        iofdbuf & buf() noexcept
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

// =====================================================================
    wrmcapture_cctx_ochecksum_file meta_buf_;
    wrmcapture_MetaFilename mf_;
    wrmcapture_MetaFilename hf_;
    time_t start_sec_;
    time_t stop_sec_;

public:
    explicit wrmcapture_out_meta_sequence_filename_buf_impl_cctx(
        wrmcapture_out_meta_sequence_filename_buf_param_cctx const & params
    )
    : filegen_(params.sq_params.format, params.sq_params.prefix, params.sq_params.filename, params.sq_params.extension)
    , buf_()
    , num_file_(0)
    , groupid_(params.sq_params.groupid)
    , meta_buf_(params.meta_buf_params)
    , mf_(params.sq_params.prefix, params.sq_params.filename, params.sq_params.format)
    , hf_(params.hash_prefix, params.sq_params.filename, params.sq_params.format)
    , start_sec_(params.sec)
    , stop_sec_(params.sec)
    {
        this->current_filename_[0] = 0;
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
        int err = this->next();
        if (err) {
            return err;
        }
        if (this->meta_buf().is_open()){
            err = this->meta_buf_.close();
            if (err) {
                return err;
            }
        }
        char const * hash_filename = this->hf_.filename;
        char const * meta_filename = this->meta_filename();
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

            int flush() const
            { return 0; }
        } crypto_hash;

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

        err = crypto_hash.open(hash_filename, S_IRUSR|S_IRGRP) < 0;
        if (err) {
            int e = errno;
            LOG(LOG_ERR, "Open to transport failed: code=%d", e);
            errno = e;
            return 1;
        }
        char header[] = "v2\n\n\n";
        crypto_hash.write(header, sizeof(header)-1);

        struct stat stat;
        err = ::stat(meta_filename, &stat);
        if (err) {
            LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
            return 1;
        }
        
        auto & writer = crypto_hash;
        
        auto pfile = filename;
        auto epfile = filename;
        for (; *epfile; ++epfile) {
            if (*epfile == '\\') {
                ssize_t len = epfile - pfile + 1;
                auto res = writer.write(pfile, len);
                if (res < len) {
                    err = res < 0 ? res : 1;
                    LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                    return 1;
                }
                pfile = epfile;
            }
            if (*epfile == ' ') {
                ssize_t len = epfile - pfile;
                auto res = writer.write(pfile, len);
                if (res < len) {
                    err = res < 0 ? res : 1;
                    LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                    return 1;
                }
                res = writer.write("\\", 1u);
                if (res < 1) {
                    err = res < 0 ? res : 1;
                    LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                    return 1;
                }
                pfile = epfile;
            }
        }

        if (pfile != epfile) {
            ssize_t len = epfile - pfile;
            auto res = writer.write(pfile, len);
            if (res < len) {
                err = res < 0 ? res : 1;
                LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                return 1;
            }
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

        char * p = mes + len;
        *p++ = '\n';

        ssize_t res = writer.write(mes, p-mes);

        if (res < p-mes) {
            LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
            return 1;
        }
        err = crypto_hash.close(/*hash*/);
        return 0;
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

        if (int err = wrmcapture_write_meta_file_cctx(
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
        unsigned i = this->num_file_ + 1;
        while (i > 0 && !::unlink(this->filegen_.get(--i))) {
        }
        if (this->buf_.is_open()) {
            this->buf_.close();
        }
        ::unlink(this->mf_.filename);
    }

    wrmcapture_cctx_ochecksum_file & meta_buf() noexcept
    { return this->meta_buf_; }

    void update_sec(time_t sec)
    { this->stop_sec_ = sec; }
};


class wrmcapture_out_hash_meta_sequence_filename_buf_impl_cctx
: public wrmcapture_out_meta_sequence_filename_buf_impl_cctx
{
    CryptoContext & cctx;
    CryptoContext& hash_ctx;
    wrmcapture_ochecksum_filter wrm_filter;

public:
    explicit wrmcapture_out_hash_meta_sequence_filename_buf_impl_cctx(
        wrmcapture_out_hash_meta_sequence_filename_buf_param_cctx const & params
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

            int flush() const
            { return 0; }
        } hash_buf;

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

        if (hash_buf.open(hash_filename, S_IRUSR|S_IRGRP) < 0) {
            int e = errno;
            LOG(LOG_ERR, "Open to transport failed: code=%d", e);
            errno = e;
            return 1;
        }
        char header[] = "v2\n\n\n";
        hash_buf.write(header, sizeof(header)-1);

        struct stat stat;
        int err = ::stat(meta_filename, &stat);
        if (err) {
            LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
            return 1;
        }
        
        auto & writer = hash_buf;
        auto pfile = filename;
        auto epfile = filename;
        for (; *epfile; ++epfile) {
            if (*epfile == '\\') {
                ssize_t len = epfile - pfile + 1;
                auto res = writer.write(pfile, len);
                if (res < len) {
                    err = res < 0 ? res : 1;
                    LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                    return 1;
                }
                pfile = epfile;
            }
            if (*epfile == ' ') {
                ssize_t len = epfile - pfile;
                auto res = writer.write(pfile, len);
                if (res < len) {
                    err = res < 0 ? res : 1;
                    LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                    return 1;
                }
                res = writer.write("\\", 1u);
                if (res < 1) {
                    err = res < 0 ? res : 1;
                    LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                    return 1;
                }
                pfile = epfile;
            }
        }

        if (pfile != epfile) {
            ssize_t len = epfile - pfile;
            auto res = writer.write(pfile, len);
            if (res < len) {
                err = res < 0 ? res : 1;
                LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                return 1;
            }
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

        char * p = mes + len;
        auto write = [&p](unsigned char const * hash) {
            *p++ = ' ';                // 1 octet
            for (unsigned c : iter(hash, MD_HASH_LENGTH)) {
                sprintf(p, "%02x", c); // 64 octets (hash)
                p += 2;
            }
        };
        write(&hash[0]);
        write(&hash[MD_HASH_LENGTH]);
        *p++ = '\n';

        ssize_t res = writer.write(mes, p-mes);

        if (res < p-mes) {
            err = res < 0 ? res : 1;
            LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
            return 1;
        }

        err = hash_buf.close(/*hash*/);
        if (err) {
            LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
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


class wrmcapture_out_hash_meta_sequence_filename_buf_impl_crypto
: public wrmcapture_out_meta_sequence_filename_buf_impl_ocrypto_filename_buf
{
    CryptoContext & cctx;
    wrmcapture_ocrypto_filename_params hash_ctx;
    wrmcapture_ocrypto_filter wrm_filter;

public:
    explicit wrmcapture_out_hash_meta_sequence_filename_buf_impl_crypto(
        wrmcapture_out_hash_meta_sequence_filename_buf_param_ocrypto const & params
    )
    : wrmcapture_out_meta_sequence_filename_buf_impl_ocrypto_filename_buf(params.meta_sq_params)
    , cctx(params.cctx)
    , hash_ctx(params.filter_params)
    , wrm_filter(this->buf(), params.filter_params)
    {}

    ssize_t write(const void * data, size_t len)
    {
        if (!this->buf().is_open()) {
            const char * filename = this->get_filename_generate();
            const int res = this->open_filename(filename);
            if (res < 0) {
                return res;
            }
            if (int err = this->wrm_filter.open(filename)) {
                return err;
            }
        }
        return this->wrm_filter.write(data, len);
    }

    int close()
    {
        if (this->buf().is_open()) {
            if (this->next()) {
                return 1;
            }
        }

        wrmcapture_ocrypto_filename_buf hash_buf(this->hash_ctx);

        if (!this->meta_buf_.is_open()) {
            return 1;
        }

        wrmcapture_hash_type hash;

        if (this->meta_buf_.close(hash)) {
            return 1;
        }

        char const * hash_filename = this->hf_.filename;
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

        if (hash_buf.open(hash_filename, S_IRUSR|S_IRGRP) < 0) {
            int e = errno;
            LOG(LOG_ERR, "Open to transport failed: code=%d", e);
            errno = e;
            return 1;
        }

        char header[] = "v2\n\n\n";
        hash_buf.write(header, sizeof(header)-1);

        struct stat stat;
        int err = ::stat(meta_filename, &stat);
        if (err) {
            LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n",
                hash_filename, err);
            return 1;
        }

        auto & writer = hash_buf;
        
        auto pfile = filename;
        auto epfile = filename;
        for (; *epfile; ++epfile) {
            if (*epfile == '\\') {
                ssize_t len = epfile - pfile + 1;
                auto res = writer.write(pfile, len);
                if (res < len) {
                    err = res < 0 ? res : 1;
                    LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n",
                        hash_filename, err);
                    return 1;
                }
                pfile = epfile;
            }
            if (*epfile == ' ') {
                ssize_t len = epfile - pfile;
                auto res = writer.write(pfile, len);
                if (res < len) {
                    err = res < 0 ? res : 1;
                    LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n",
                        hash_filename, err);
                    return 1;
            }
                res = writer.write("\\", 1u);
                if (res < 1) {
                    err = res < 0 ? res : 1;
                    LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n",
                        hash_filename, err);
                    return 1;
                }
                pfile = epfile;
            }
        }

        if (pfile != epfile) {
            ssize_t len = epfile - pfile;
            auto res = writer.write(pfile, len);
            if (res < len) {
                err = res < 0 ? res : 1;
                LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
                return 1;
            }
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

        char * p = mes + len;
        auto write = [&p](unsigned char const * hash) {
            *p++ = ' ';                // 1 octet
            for (unsigned c : iter(hash, MD_HASH_LENGTH)) {
                sprintf(p, "%02x", c); // 64 octets (hash)
                p += 2;
            }
        };
        write(&hash[0]);
        write(&hash[MD_HASH_LENGTH]);
        *p++ = '\n';

        ssize_t res = writer.write(mes, p-mes);

        if (res < p-mes) {
            err = res < 0 ? res : 1;
            LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
            return 1;
        }

        err = hash_buf.close(/*hash*/);
        if (err) {
            LOG(LOG_ERR, "Failed writing signature to hash file %s [err %d]\n", hash_filename, err);
            return 1;
        }

        return 0;
    }

    int next()
    {
        if (this->buf().is_open()) {
            wrmcapture_hash_type hash;
            {
                const int res1 = this->wrm_filter.close(hash, this->cctx.get_hmac_key());
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
        wrmcapture_FilenameFormat format = wrmcapture_FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
    : buf(wrmcapture_out_meta_sequence_filename_buf_noparam(
        now.tv_sec, format, hash_path, path, basename, ".wrm", groupid
    ))
    {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        char header1[3 + ((std::numeric_limits<unsigned>::digits10 + 1) * 2 + 2) + (10 + 1) + 2 + 1];
        const int len = sprintf(
            header1,
            "v2\n"
            "%u %u\n"
            "%s\n"
            "\n\n",
            unsigned(width),
            unsigned(height),
            "nochecksum"
        );
        const ssize_t res = this->buffer().meta_buf_write(header1, len);
        if (res < 0) {
            int err = errno;
            LOG(LOG_ERR, "Write to transport failed (M): code=%d", err);

            if (err == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|%s", path);
                this->authentifier->report("FILESYSTEM_FULL", message);

                throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, err);
            }
            else {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, err);
            }
        }
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

    wrmcapture_out_meta_sequence_filename_buf_impl_ofile_buf_out & buffer() noexcept
    { return this->buf; }

    const wrmcapture_out_meta_sequence_filename_buf_impl_ofile_buf_out & buffer() const noexcept
    { return this->buf; }

    wrmcapture_out_meta_sequence_filename_buf_impl_ofile_buf_out buf;

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
        wrmcapture_FilenameFormat format = wrmcapture_FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
    : buf(
        wrmcapture_out_hash_meta_sequence_filename_buf_param_cctx(
            crypto_ctx,
            now.tv_sec, format, hash_path, path, basename, ".wrm", groupid,
            crypto_ctx
        )
    ) {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        auto & writer = this->buffer().meta_buf();
        char header1[3 + ((std::numeric_limits<unsigned>::digits10 + 1) * 2 + 2) + (10 + 1) + 2 + 1];
        const int len = sprintf(
            header1,
            "v2\n"
            "%u %u\n"
            "%s\n"
            "\n\n",
            unsigned(width),
            unsigned(height),
            "checksum"
        );
        const ssize_t res = writer.write(header1, len);
        if (res < 0) {
            int err = errno;
            LOG(LOG_ERR, "Write to transport failed (M): code=%d", err);

            if (err == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|%s", path);
                this->authentifier->report("FILESYSTEM_FULL", message);

                throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, err);
            }
            else {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, err);
            }
        }
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



struct wrmcapture_CryptoOutMetaSequenceTransport
: public Transport {

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
        wrmcapture_FilenameFormat format = wrmcapture_FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
    : buf(
        wrmcapture_out_hash_meta_sequence_filename_buf_param_ocrypto(
            crypto_ctx,
            now.tv_sec, format, hash_path, path, basename, ".wrm", groupid,
            {crypto_ctx, rnd}
        )) {
        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        wrmcapture_write_meta_headers(this->buffer().meta_buf_, path, width, height, this->authentifier, true);
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

    wrmcapture_out_hash_meta_sequence_filename_buf_impl_crypto & buffer() noexcept
    { return this->buf; }

    const wrmcapture_out_hash_meta_sequence_filename_buf_impl_crypto & buffer() const noexcept
    { return this->buf; }

    typedef wrmcapture_CryptoOutMetaSequenceTransport TransportType;

    wrmcapture_out_hash_meta_sequence_filename_buf_impl_crypto buf;
};


// TODO temporary
struct DumpPng24FromRDPDrawableAdapter : gdi::DumpPng24Api  {
    RDPDrawable & drawable;

    explicit DumpPng24FromRDPDrawableAdapter(RDPDrawable & drawable) : drawable(drawable) {}

    void dump_png24(Transport& trans, bool bgr) const override {
      ::dump_png24(this->drawable.impl(), trans, bgr);
    }
};


inline void wrmcapture_send_wrm_chunk(Transport & t, uint16_t chunktype, uint16_t data_size, uint16_t count)
{
    StaticOutStream<8> header;
    header.out_uint16_le(chunktype);
    header.out_uint32_le(8 + data_size);
    header.out_uint16_le(count);
    t.send(header.get_data(), header.get_offset());
}


inline void wrmcapture_send_meta_chunk(
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

    wrmcapture_send_wrm_chunk(t, META_FILE, payload.get_offset(), 1);
    t.send(payload.get_data(), payload.get_offset());
}


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
            wrmcapture_send_wrm_chunk(this->trans, LAST_IMAGE_CHUNK, this->stream.get_offset(), 1);
            this->trans.send(this->stream.get_data(), this->stream.get_offset());
            this->stream = OutStream(buf);
        }
    }

private:
    void do_send(const uint8_t * const buffer, size_t len) override {
        size_t to_buffer_len = len;
        while (this->stream.get_offset() + to_buffer_len > this->max) {
            wrmcapture_send_wrm_chunk(this->trans, PARTIAL_IMAGE_CHUNK, this->max, 1);
            this->trans.send(this->stream.get_data(), this->stream.get_offset());
            size_t to_send = this->max - this->stream.get_offset();
            this->trans.send(buffer + len - to_buffer_len, to_send);
            to_buffer_len -= to_send;
            this->stream = OutStream(buf);
        }
        this->stream.out_copy_bytes(buffer + len - to_buffer_len, to_buffer_len);
    }
};


class wrmcapture_SaveStateChunk {
public:
    wrmcapture_SaveStateChunk() {}

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

        wrmcapture_send_meta_chunk(
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
        wrmcapture_send_wrm_chunk(this->trans, RESET_CHUNK, 0, 1);
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

        wrmcapture_send_wrm_chunk(this->trans, TIMESTAMP, payload.get_offset(), 1);
        this->trans.send(payload.get_data(), payload.get_offset());

        this->last_sent_timer = this->timer;
    }

    void send_save_state_chunk()
    {
        StaticOutStream<4096> payload;
        wrmcapture_SaveStateChunk ssc;
        ssc.send(payload, this->ssc);

        //------------------------------ missing variable length ---------------

        wrmcapture_send_wrm_chunk(this->trans, SAVE_STATE, payload.get_offset(), 1);
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
        wrmcapture_send_wrm_chunk(this->trans, RDP_UPDATE_ORDERS, this->stream_orders.get_offset(), this->order_count);
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
        wrmcapture_send_wrm_chunk(this->trans, RDP_UPDATE_BITMAP, this->stream_bitmaps.get_offset(), this->bitmap_count);
        this->trans.send(this->stream_bitmaps.get_data(), this->stream_bitmaps.get_offset());
        this->bitmap_count = 0;
        this->stream_bitmaps.rewind();
    }

protected:
    void send_pointer(int cache_idx, const Pointer & cursor) override {
        if ((cursor.width != 32) || (cursor.height != 32)) {
            this->send_pointer2(cache_idx, cursor);
            return;
        }

        size_t size =   2           // mouse x
                      + 2           // mouse y
                      + 1           // cache index
                      + 1           // hotspot x
                      + 1           // hotspot y
                      + 32 * 32 * 3 // data
                      + 128         // mask
                      ;
        wrmcapture_send_wrm_chunk(this->trans, POINTER, size, 0);

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

    void send_pointer2(int cache_idx, const Pointer & cursor) {
        size_t size =   2                   // mouse x
                      + 2                   // mouse y
                      + 1                   // cache index

                      + 1                   // mouse width
                      + 1                   // mouse height
                      + 1                   // mouse bpp

                      + 1                   // hotspot x
                      + 1                   // hotspot y

                      + 2                   // data_size
                      + 2                   // mask_size

                      + cursor.data_size()  // data
                      + cursor.mask_size()  // mask
                      ;
        wrmcapture_send_wrm_chunk(this->trans, POINTER2, size, 0);

        StaticOutStream<32> payload;
        payload.out_uint16_le(this->mouse_x);
        payload.out_uint16_le(this->mouse_y);
        payload.out_uint8(cache_idx);

        payload.out_uint8(cursor.width);
        payload.out_uint8(cursor.height);
        payload.out_uint8(24);

        payload.out_uint8(cursor.x);
        payload.out_uint8(cursor.y);

        payload.out_uint16_le(cursor.data_size());
        payload.out_uint16_le(cursor.mask_size());

        this->trans.send(payload.get_data(), payload.get_offset());

        this->trans.send(cursor.data, cursor.data_size());
        this->trans.send(cursor.mask, cursor.mask_size());
    }

    void set_pointer(int cache_idx) override {
        size_t size =   2                   // mouse x
                      + 2                   // mouse y
                      + 1                   // cache index
                      ;
        wrmcapture_send_wrm_chunk(this->trans, POINTER, size, 0);

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

        wrmcapture_send_wrm_chunk(this->trans, SESSION_UPDATE, payload.get_offset() + message_length, 1);
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

