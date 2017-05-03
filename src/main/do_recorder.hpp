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

