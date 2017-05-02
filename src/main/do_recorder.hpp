/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redver video verifier program
*/

#pragma once

#include <algorithm>
#include <snappy-c.h>
#include <iostream>
#include <memory>

#include "capture/cryptofile.hpp"
#include "utils/sugar/local_fd.hpp"
#include "utils/chex_to_int.hpp"
#include "utils/parse.hpp"
#include "utils/fileutils.hpp"
#include "transport/in_crypto_transport.hpp"
#include "cxx/cxx.hpp"

extern "C" {
    REDEMPTION_LIB_EXPORT
    int recmemcpy(char * dest, char * source, int len);

    REDEMPTION_LIB_EXPORT
    int do_main(int argc, char const ** argv,
            get_hmac_key_prototype * hmac_fn,
            get_trace_key_prototype * trace_fn);
}


enum { QUICK_CHECK_LENGTH = 4096 };


// Compute HmacSha256
// up to check_size or end of file whicherver happens first
// if check_size == 0, checks to eof
// return 0 on success and puts signature in provided buffer
// return -1 if some system error occurs, errno contains actual error
static inline int file_start_hmac_sha256(const char * filename,
                     uint8_t const * crypto_key,
                     size_t          key_len,
                     size_t          check_size,
                     uint8_t (& hash)[SslSha256::DIGEST_LENGTH])
{
    local_fd file(filename, O_RDONLY);
    int fd = file.fd();
    if (fd < 0) {
        return fd;
    }

    SslHMAC_Sha256 hmac(crypto_key, key_len);

    uint8_t buf[4096] = {};
    ssize_t ret = ::read(fd, buf, sizeof(buf));
    for (size_t  number_of_bytes_read = 0 ; ret ; number_of_bytes_read += ret){
        // number_of_bytes_read < check_size
        if (ret < 0){
            // interruption signal, not really an error
            if (errno == EINTR){
                continue;
            }
            return -1;
        }
        if (check_size && number_of_bytes_read + ret > check_size){
            hmac.update(buf, check_size - number_of_bytes_read);
            break;
        }
        if (ret == 0){ break; }
        hmac.update(buf, ret);
        ret = ::read(fd, buf, sizeof(buf));
    }
    hmac.final(hash);
    return 0;
}

class LineReader
{
public:
    constexpr static std::size_t line_max = 1024 * 4 - 1;

    char buf[line_max + 1]; // This is to avoid for the bug to be too visible
    char * eof;
    char * eol;
    char * cur;
    InCryptoTransport & ibuf;

public:
    LineReader(InCryptoTransport & reader_buf) noexcept
    : buf{}
    , eof(buf)
    , eol(buf)
    , cur(buf)
    , ibuf(reader_buf)
    {}

    // buffer must be able to contain line
    // if no line at end of buffer apply some memmove
    /// \return  true if ok, false if end of file
    /// \exception Error : ERR_TRANSPORT_READ_FAILED
    bool next_line()
    {
        this->cur = this->eol;
        if (this->cur == this->eof) // empty buffer
        {
            ssize_t ret = this->ibuf.partial_read(this->buf, sizeof(this->buf)-1);
            this->cur = this->buf;
            this->eof = this->buf + ret;
            this->eof[0] = 0;
        }

        char * pos = std::find(this->cur, this->eof, '\n');
        if (pos == this->eof) {
            // move remaining data to beginning of buffer
            size_t len = this->eof - this->cur;
            ::memmove(this->buf, this->cur, len);
            this->cur = this->buf;
            this->eof = this->cur + len;

            do { // read and append to buffer
                ssize_t ret = this->ibuf.partial_read(this->eof, std::end(this->buf)-1-this->eof);
                if (ret == 0) {
                    break;
                }
                pos = std::find(this->eof, this->eof + ret, '\n');
                this->eof += ret;
                this->eof[0] = 0;
            } while (pos == this->eof);
        }

        this->eol = (pos == this->eof) ? pos : pos + 1;

        // end of file
        if (this->buf == this->eof) {
            return false;
        }

        // line without \n is a error
        if (pos == this->eof) {
            throw Error(ERR_TRANSPORT_READ_FAILED, 0);
        }

        return true;
    }

    array_view_char get_buf() const
    { return {this->cur, this->eol}; }
};

struct MetaHeader {
    unsigned version;
    //unsigned witdh;
    //unsigned height;
    bool has_checksum;
};

struct MetaLine2
{
    char    filename[PATH_MAX + 1];
    off_t   size;
    mode_t  mode;
    uid_t   uid;
    gid_t   gid;
    dev_t   dev;
    ino_t   ino;
    time_t  mtime;
    time_t  ctime;
    time_t  start_time;
    time_t  stop_time;
    unsigned char hash1[MD_HASH::DIGEST_LENGTH];
    unsigned char hash2[MD_HASH::DIGEST_LENGTH];
};

static inline void in_copy_bytes(uint8_t * hash, int len, char * & cur, char * eol, int err)
{
    if (eol - cur < len){
        throw Error(err);
    }
    memcpy(hash, cur, len);
    cur += len;
}

static inline void in_hex256(uint8_t * hash, int len, char * & cur, char * eol, char sep, int exc)
{
    int err = 0;
    char * pos = std::find(cur, eol, sep);
    if (pos == eol || (pos - cur != 2*len)){
        throw Error(exc);
    }
    for (int i = 0 ; i < len ; i++){
        hash[i] = (chex_to_int(cur[i*2u], err)*16)
                 + chex_to_int(cur[i*2u+1], err);
    }
    if (err){
        throw Error(err);
    }
    cur = pos + 1;
}

static inline long long int get_ll(char * & cur, char * eol, char sep, int err)
{
    char * pos = std::find(cur, eol, sep);
    if (pos == eol || (pos - cur < 1)){
        throw Error(err);
    }
    char * pend = nullptr;
    long long int res = strtoll(cur, &pend, 10);
    if (pend != pos){
        throw Error(err);
    }
    cur = pos + 1;
    return res;
}

struct MwrmReader
{
    LineReader line_reader;
    MetaHeader header;

public:
    MwrmReader(InCryptoTransport & ibuf) noexcept
    : line_reader(ibuf)
    {}

    void read_meta_headers(bool encrypted)
    {
        auto next_line = [this]{
            if (!this->line_reader.next_line()) {
                throw Error(ERR_TRANSPORT_READ_FAILED, 0);
            }
        };
        next_line(); // v2
        this->header.version = (this->line_reader.get_buf()[0] == 'v')?2:1;
        if (header.version == 2) {
            next_line(); // 800 600
            next_line(); // checksum or nochecksum
        }
        this->header.has_checksum
          = encrypted
          || ((this->header.version > 1) && (this->line_reader.get_buf()[0] == 'c'));
        // else v1
        next_line(); // blank
        next_line(); // blank
    }

    /// \return false if end of file
    bool read_meta_file(MetaLine2 & meta_line)
    {
//        LOG(LOG_INFO, "read_meta_file %s", this->line_reader.get_buf().begin());

        if (!this->line_reader.next_line()) {
            return false;
        }
//        LOG(LOG_INFO, "read_meta_file: after reading line %s", this->line_reader.get_buf().begin());

        auto cur = this->line_reader.get_buf().begin();
        auto eol = this->line_reader.get_buf().end();

//        LOG(LOG_INFO, "read_meta_file [1]");

        // Line format "filename
        // [st_size st_mode st_uid st_gid st_dev st_ino st_mtime st_ctime]
        // start_sec stop_sen [has1 has2]"
        //
        // filename(1 or >) + space(1)
        // + [stat_info(ll|ull * 8) + space(1)*8]
        // + start_sec(1 or >) + space(1) + stop_sec(1 or >)
        // + [space(1) + hash1(64) + space(1) + hash2(64)] >= 135

        using reverse_iterator = std::reverse_iterator<char*>;
        reverse_iterator first(cur);
        reverse_iterator space(eol);
//        LOG(LOG_INFO, "read_meta_file [2] has_checksum=%d", header.has_checksum);
        for(int i = 0; i < ((header.version==1)?2:10) + 2*header.has_checksum; ++i){
            space = std::find(space, first, ' ');
            ++space;
        }
        int path_len = first-space;
        in_copy_bytes(
            reinterpret_cast<uint8_t*>(meta_line.filename),
            path_len, cur, eol, ERR_TRANSPORT_READ_FAILED
        );

        ++cur;
        meta_line.filename[path_len] = 0;

//        LOG(LOG_INFO, "read_meta_file [3] filename='%s' %.*s", meta_line.filename, static_cast<int>(eol-cur), cur);

        if (this->header.version == 1){
//            LOG(LOG_INFO, "read_meta_file [4]");

            meta_line.size = 0;
            meta_line.mode = 0;
            meta_line.uid = 0;
            meta_line.gid = 0;
            meta_line.dev = 0;
            meta_line.ino = 0;
            meta_line.mtime = 0;
            meta_line.ctime = 0;
        }
        else{ // v2
//            LOG(LOG_INFO, "read_meta_file [5] %.*s", static_cast<int>(eol-cur), cur);

            meta_line.size = get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
            meta_line.mode = get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
            meta_line.uid = get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
            meta_line.gid = get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
            meta_line.dev = get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
            meta_line.ino = get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
            meta_line.mtime = get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
            meta_line.ctime = get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
        }

//        LOG(LOG_INFO, "read_meta_file [6] %.*s", static_cast<int>(eol-cur), cur);

        meta_line.start_time = get_ll(cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
        meta_line.stop_time = get_ll(cur, eol, this->header.has_checksum?' ':'\n',
                                            ERR_TRANSPORT_READ_FAILED);

//        LOG(LOG_INFO, "read_meta_file [7] %.*s", static_cast<int>(eol-cur), cur);
        if (header.has_checksum){
            // HASH1 + space
            in_hex256(meta_line.hash1, MD_HASH::DIGEST_LENGTH, cur, eol, ' ', ERR_TRANSPORT_READ_FAILED);
            // HASH1 + CR
            in_hex256(meta_line.hash2, MD_HASH::DIGEST_LENGTH, cur, eol, '\n', ERR_TRANSPORT_READ_FAILED);
        }
        else {
            memset(meta_line.hash1, 0, sizeof(meta_line.hash1));
            memset(meta_line.hash2, 0, sizeof(meta_line.hash2));
        }

//        LOG(LOG_INFO, "read_meta_file [8] %.*s", static_cast<int>(eol-cur), cur);
        // TODO: check the whole line has been consumed (or it's an error)
//        LOG(LOG_INFO, "read_meta_file: done %s", this->line_reader.get_buf().begin());
        return true;
    }
};

static inline int encryption_type(const std::string & full_filename, CryptoContext & cctx)
{
    uint8_t tmp_buf[4] ={};
    int fd = open(full_filename.c_str(), O_RDONLY);
    if (fd == -1){
        std::cerr << "Input file missing.\n";
        return -1;
    }

    {
        local_fd file(fd);

        const size_t len = sizeof(tmp_buf);
        size_t remaining_len = len;
        while (remaining_len) {
            ssize_t ret = ::read(fd, &tmp_buf[len - remaining_len], remaining_len);
            if (ret < 0){
                if (ret == 0){
                    std::cerr << "Input file truncated\n";
                    return -1;
                }
                if (errno == EINTR){
                    continue;
                }
                // Error should still be there next time we try to read
                std::cerr << "Input file error\n";
                return -1;
            }
            // We must exit loop or we will enter infinite loop
            remaining_len -= ret;
        }
    }

    const uint32_t magic = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);
    if (magic == WABCRYPTOFILE_MAGIC) {
        InCryptoTransport in_test(cctx, InCryptoTransport::EncryptionMode::Encrypted);
        in_test.open(full_filename.c_str());
        char mem[4096];
        try {
            (void)in_test.partial_read(mem, sizeof(mem));
        } catch (Error const&) {
            cctx.old_encryption_scheme = 1;
            return 1;
        }
        return 2;
    }
    return 0;
}

static inline void clear_files_flv_meta_png(const char * path, const char * prefix)
{
    struct D {
        DIR * d;

        ~D() { closedir(d); }
        operator DIR * () const { return d; }
    } d{opendir(path)};

    if (d){
//        char static_buffer[8192];
        char buffer[8192];
        size_t path_len = strlen(path);
        size_t prefix_len = strlen(prefix);
        size_t file_len = 1024;
        if (file_len + path_len + 1 > sizeof(buffer)) {
            LOG(LOG_WARNING, "Path len %zu > %zu", file_len + path_len + 1, sizeof(buffer));
            return;
        }
        strncpy(buffer, path, file_len + path_len + 1);
        if (buffer[path_len] != '/'){
            buffer[path_len] = '/'; path_len++; buffer[path_len] = 0;
        }

        // TODO size_t len = offsetof(struct dirent, d_name) + NAME_MAX + 1 ?
        struct dirent * result;
        for (result = readdir(d) ; result ; result = readdir(d)) {
            if ((0 == strcmp(result->d_name, ".")) || (0 == strcmp(result->d_name, ".."))){
                continue;
            }

            if (strncmp(result->d_name, prefix, prefix_len)){
                continue;
            }

            strncpy(buffer + path_len, result->d_name, file_len);
            const char * eob = buffer + path_len + strlen(result->d_name);
            const bool extension = ((strlen(result->d_name) > 4) && (eob[-4] == '.')
                    && (((eob[-3] == 'f') && (eob[-2] == 'l') && (eob[-1] == 'v'))
                      ||((eob[-3] == 'p') && (eob[-2] == 'n') && (eob[-1] == 'g'))
                      ||((eob[-3] == 'p') && (eob[-2] == 'g') && (eob[-1] == 's'))))
                || (((strlen(result->d_name) > 5) && (eob[-5] == '.')
                    && (eob[-4] == 'm') && (eob[-3] == 'e') && (eob[-2] == 't') && (eob[-1] == 'a')))
                ;

            if (!extension){
                continue;
            }

            struct stat st;
            if (stat(buffer, &st) < 0){
                LOG(LOG_WARNING, "Failed to read file %s [%d: %s]", buffer, errno, strerror(errno));
                continue;
            }
            if (unlink(buffer) < 0){
                LOG(LOG_WARNING, "Failed to remove file %s [%d: %s]", buffer, errno, strerror(errno));
            }
        }
    }
    else {
        LOG(LOG_WARNING, "Failed to open directory %s [%d: %s]", path, errno, strerror(errno));
    }
}

