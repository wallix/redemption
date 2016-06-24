/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redrec video verifier program
*/

#include <iostream>

#include <utility>
#include <string>

#include "main/version.hpp"
#include "transport/in_filename_transport.hpp"
#include "transport/in_meta_sequence_transport.hpp"
#include "system/ssl_calls.hpp"
#include "configs/config.hpp"

#include "program_options/program_options.hpp"

#ifndef HASH_LEN
#define HASH_LEN 64
#endif

#define QUICK_CHECK_LENGTH 4096

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
    unsigned char hash1[MD_HASH_LENGTH];
    unsigned char hash2[MD_HASH_LENGTH];
};


struct HashHeader {
    unsigned version;
};

static inline char const * sread_filename2(char * p, char const * e, char const * pline)
{
    e -= 1;
    for (; p < e && *pline && *pline != ' ' && (*pline == '\\' ? *++pline : true); ++pline, ++p) {
        *p = *pline;
    }
    *p = 0;
    return pline;
}


namespace transbuf {

    class ifile_buf
    {
    public:
        CryptoContext * cctx;
        int cfb_file_fd;
        char           cfb_decrypt_buf[CRYPTO_BUFFER_SIZE]; //
        EVP_CIPHER_CTX cfb_decrypt_ectx;                    // [en|de]cryption context
        uint32_t       cfb_decrypt_pos;                     // current position in buf
        uint32_t       cfb_decrypt_raw_size;                // the unciphered/uncompressed file size
        uint32_t       cfb_decrypt_state;                   // enum crypto_file_state
        unsigned int   cfb_decrypt_MAX_CIPHERED_SIZE;       // = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

        int encryption;


        ssize_t cfb_decrypt_decrypt_read(void * data, size_t len)
        {
            if (this->cfb_decrypt_state & CF_EOF) {
                return 0;
            }

            unsigned int requested_size = len;

            while (requested_size > 0) {
                // Check how much we have decoded
                if (!this->cfb_decrypt_raw_size) {
                    uint8_t tmp_buf[4] = {};
                    ssize_t err = this->cfb_file_read(tmp_buf, 4);
                    if (err != 4) {
                        return err < 0 ? err : -1;
                    }

                    uint32_t ciphered_buf_size = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);
                    if (ciphered_buf_size == WABCRYPTOFILE_EOF_MAGIC) { // end of file
                        this->cfb_decrypt_state |= CF_EOF;
                        this->cfb_decrypt_pos = 0;
                        this->cfb_decrypt_raw_size = 0;
                        break;
                    }

                    if (ciphered_buf_size > this->cfb_decrypt_MAX_CIPHERED_SIZE) {
                        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Integrity error, erroneous chunk size!\n", ::getpid());
                        return -1;
                    }

                    uint32_t compressed_buf_size = ciphered_buf_size + AES_BLOCK_SIZE;

                    //char ciphered_buf[ciphered_buf_size];
                    unsigned char ciphered_buf[65536];
                    //char compressed_buf[compressed_buf_size];
                    unsigned char compressed_buf[65536];


                    err = this->cfb_file_read(ciphered_buf, ciphered_buf_size);

                    // len ?
                    if (err != ssize_t(ciphered_buf_size)){
                        return err < 0 ? err : -1;
                    }

                    if (this->cfb_decrypt_xaes_decrypt(ciphered_buf,
                                    ciphered_buf_size,
                                    compressed_buf,
                                    &compressed_buf_size)) {
                        return -1;
                    }

                    size_t chunk_size = CRYPTO_BUFFER_SIZE;
                    const snappy_status status = snappy_uncompress(
                            reinterpret_cast<char *>(compressed_buf),
                            compressed_buf_size, this->cfb_decrypt_buf, &chunk_size);

                    switch (status)
                    {
                        case SNAPPY_OK:
                            break;
                        case SNAPPY_INVALID_INPUT:
                            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy decompression failed with status code INVALID_INPUT!\n", getpid());
                            return -1;
                        case SNAPPY_BUFFER_TOO_SMALL:
                            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy decompression failed with status code BUFFER_TOO_SMALL!\n", getpid());
                            return -1;
                        default:
                            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy decompression failed with unknown status code (%d)!\n", getpid(), status);
                            return -1;
                    }

                    this->cfb_decrypt_pos = 0;
                    // When reading, raw_size represent the current chunk size
                    this->cfb_decrypt_raw_size = chunk_size;

                    // TODO: check that
                    if (!this->cfb_decrypt_raw_size) { // end of file reached
                        break;
                    }
                }
                // remaining_size is the amount of data available in decoded buffer
                unsigned int remaining_size = this->cfb_decrypt_raw_size - this->cfb_decrypt_pos;
                // Check how much we can copy
                unsigned int copiable_size = MIN(remaining_size, requested_size);
                // Copy buffer to caller
                ::memcpy(static_cast<char*>(data) + (len - requested_size), this->cfb_decrypt_buf + this->cfb_decrypt_pos, copiable_size);
                this->cfb_decrypt_pos      += copiable_size;
                requested_size -= copiable_size;
                // Check if we reach the end
                if (this->cfb_decrypt_raw_size == this->cfb_decrypt_pos) {
                    this->cfb_decrypt_raw_size = 0;
                }
            }
            return len - requested_size;
        }

    private:

        int cfb_decrypt_xaes_decrypt(const unsigned char *src_buf, uint32_t src_sz, unsigned char *dst_buf, uint32_t *dst_sz)
        {
            int safe_size = *dst_sz;
            int remaining_size = 0;

            /* allows reusing of ectx for multiple encryption cycles */
            if (EVP_DecryptInit_ex(&this->cfb_decrypt_ectx, nullptr, nullptr, nullptr, nullptr) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not prepare decryption context!\n", getpid());
                return -1;
            }
            if (EVP_DecryptUpdate(&this->cfb_decrypt_ectx, dst_buf, &safe_size, src_buf, src_sz) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not decrypt data!\n", getpid());
                return -1;
            }
            if (EVP_DecryptFinal_ex(&this->cfb_decrypt_ectx, dst_buf + safe_size, &remaining_size) != 1){
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not finish decryption!\n", getpid());
                return -1;
            }
            *dst_sz = safe_size + remaining_size;
            return 0;
        }

        int cfb_file_close()
        {
            if (this->is_open()) {
                const int ret = ::close(this->cfb_file_fd);
                this->cfb_file_fd = -1;
                return ret;
            }
            return 0;
        }

        bool cfb_file_is_open() const noexcept
        { return -1 != this->cfb_file_fd; }

        ssize_t cfb_file_read(void * data, size_t len)
        {
            TODO("this is blocking read, add support for timeout reading");
            TODO("add check for O_WOULDBLOCK, as this is is blockig it would be bad");
            size_t remaining_len = len;
            while (remaining_len) {
                ssize_t ret = ::read(this->cfb_file_fd, static_cast<char*>(data) + (len - remaining_len), remaining_len);
                if (ret < 0){
                    if (errno == EINTR){
                        continue;
                    }
                    // Error should still be there next time we try to read
                    if (remaining_len != len){
                        return len - remaining_len;
                    }
                    return ret;
                }
                // We must exit loop or we will enter infinite loop
                if (ret == 0){
                    break;
                }
                remaining_len -= ret;
            }
            return len - remaining_len;
        }


    public:
        explicit ifile_buf(CryptoContext * cctx, int encryption)
        : cctx(cctx)
        , cfb_file_fd(-1)
        , encryption(encryption)
        {
        }

        ~ifile_buf()
        {
            this->cfb_file_close();
        }

        int open(const char * filename, mode_t mode = 0600)
        {
            if (this->encryption){

                this->cfb_file_close();
                this->cfb_file_fd = ::open(filename, O_RDONLY);

                if (this->cfb_file_fd < 0) {
                    return this->cfb_file_fd;
                }

                size_t base_len = 0;
                const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(filename, base_len));

                unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
                cctx->get_derived_key(trace_key, base, base_len);

                ::memset(this->cfb_decrypt_buf, 0, sizeof(this->cfb_decrypt_buf));
                ::memset(&this->cfb_decrypt_ectx, 0, sizeof(this->cfb_decrypt_ectx));

                this->cfb_decrypt_pos = 0;
                this->cfb_decrypt_raw_size = 0;
                this->cfb_decrypt_state = 0;
                const size_t MAX_COMPRESSED_SIZE = ::snappy_max_compressed_length(CRYPTO_BUFFER_SIZE);
                this->cfb_decrypt_MAX_CIPHERED_SIZE = MAX_COMPRESSED_SIZE + AES_BLOCK_SIZE;

                unsigned char tmp_buf[40];
                const ssize_t err = this->cfb_file_read(tmp_buf, 40);
                if (err != 40) {
                    return err < 0 ? err : -1;
                }

                // Check magic
                const uint32_t magic = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);
                if (magic != WABCRYPTOFILE_MAGIC) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Wrong file type %04x != %04x\n",
                        ::getpid(), magic, WABCRYPTOFILE_MAGIC);
                    return -1;
                }

                const int version = tmp_buf[4] + (tmp_buf[5] << 8) + (tmp_buf[6] << 16) + (tmp_buf[7] << 24);
                if (version > WABCRYPTOFILE_VERSION) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Unsupported version %04x > %04x\n",
                        ::getpid(), version, WABCRYPTOFILE_VERSION);
                    return -1;
                }

                unsigned char * const iv = tmp_buf + 8;

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

                ::EVP_CIPHER_CTX_init(&this->cfb_decrypt_ectx);
                if(::EVP_DecryptInit_ex(&this->cfb_decrypt_ectx, cipher, nullptr, key, iv) != 1) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize decrypt context\n", ::getpid());
                    return -1;
                }

                return 0;
            }
            else {
                    this->cfb_file_close();
                    this->cfb_file_fd = ::open(filename, O_RDONLY);
                    if (this->cfb_file_fd < 0){
                        return -1;
                    }
                    return 0;
            }
        }

        ssize_t read(void * data, size_t len)
        {
            if (this->encryption){
                return this->cfb_decrypt_decrypt_read(data, len);
            }
            else {
                return this->cfb_file_read(data, len);
            }
        }

        bool is_open() const noexcept
        {
            return this->cfb_file_is_open();
        }
    };
}

struct MetaHeader2 {
    unsigned version;
    //unsigned witdh;
    //unsigned height;
    bool has_checksum;
};


struct FileChecker
{
    const std::string & full_filename;
    bool failed;
    explicit FileChecker(const std::string & full_filename) noexcept 
        : full_filename(full_filename)
        , failed(false) 
    {
    }
    void check_hash_sha256(uint8_t const * crypto_key,
                           size_t          key_len,
                           uint8_t const * hash_buf,
                           size_t          hash_len,
                           bool quick_check)
    {

        if (!this->failed){
            struct fdwrap
            {
                int fd;
                fdwrap(int fd) : fd(fd) {}
                ~fdwrap(){ if (fd >=0) {::close(fd);} }
            } file(::open(this->full_filename.c_str(), O_RDONLY));

            if (file.fd < 0) {
                LOG(LOG_ERR, "failed opening=%s", this->full_filename.c_str());
                std::cerr << "Error opening file \"" << this->full_filename << std::endl << std::endl;
                this->failed = true;
                return;
            }

            REDASSERT(SHA256_DIGEST_LENGTH == hash_len);
            SslHMAC_Sha256 hmac(crypto_key, key_len);

            uint8_t buf[4096] = {};
            size_t  number_of_bytes_read = 0;
            for (; !quick_check || (number_of_bytes_read < QUICK_CHECK_LENGTH) ; ){
                ssize_t ret = ::read(file.fd, buf, sizeof(buf)- quick_check*number_of_bytes_read);
                // interruption signal, not really an error
                if ((ret < 0) && (errno == EINTR)){
                    continue;
                }
                // error
                if (ret < 0){
                    LOG(LOG_ERR, "failed reading %s", this->full_filename.c_str());
                    this->failed = true;
                    return;
                }
                // end_of_file, exit loop
                if (ret == 0){ break; }
                hmac.update(buf, ret);
                number_of_bytes_read += ret;
            }

            uint8_t         hash[SHA256_DIGEST_LENGTH];
            hmac.final(&hash[0], SHA256_DIGEST_LENGTH);
            this->failed = 0 != memcmp(hash, hash_buf, hash_len);
        }
    }
    
    void check_short_stat(const MetaLine2 & meta_line)
    {
        // we just check the size match
        // used when checksum is active
        if (!this->failed){
            struct stat64 sb;
            memset(&sb, 0, sizeof(sb));
            lstat64(full_filename.c_str(), &sb);
            this->failed = (meta_line.size != sb.st_size);
        }
    }

    void check_full_stat(const MetaLine2 & meta_line)
    {
        if (!this->failed){
            struct stat64 sb;
            memset(&sb, 0, sizeof(sb));
            lstat64(full_filename.c_str(), &sb);
            this->failed = ((meta_line.dev   != sb.st_dev  ) 
                        ||  (meta_line.ino   != sb.st_ino  ) 
                        ||  (meta_line.mode  != sb.st_mode ) 
                        ||  (meta_line.uid   != sb.st_uid  ) 
                        ||  (meta_line.gid   != sb.st_gid  ) 
                        ||  (meta_line.size  != sb.st_size ) 
                        ||  (meta_line.mtime != sb.st_mtime) 
                        ||  (meta_line.ctime != sb.st_ctime)
                        );
        }
    }
};

static inline int check_encrypted_or_checksumed(
                                       std::string const & input_filename,
                                       std::string const & mwrm_path,
                                       std::string const & hash_path,
                                       bool quick_check,
                                       uint32_t verbose,
                                       CryptoContext * cctx,
                                       bool infile_is_encrypted) {
    unsigned infile_version = 1;
    bool infile_is_checksumed = false;
    std::string const full_mwrm_filename = mwrm_path + input_filename;

    if (infile_is_encrypted){
        OpenSSL_add_all_digests();
    }

    {
        class ReaderLine2ReaderBuf1
        {
            char buf[1024];
            char * eof;
            char * cur;
            transbuf::ifile_buf ibuf;
        public:
            MetaHeader2 meta_header;

            explicit ReaderLine2ReaderBuf1(CryptoContext * cctx, int encryption, 
                const std::string & full_mwrm_filename) noexcept
            : eof(buf)
            , cur(buf)
            , ibuf(cctx, encryption)
            , meta_header{1, false}
            {
                int res = ibuf.open(full_mwrm_filename.c_str());
                if (res < 0){
                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                }
            }

            ssize_t read_line(char * dest, size_t len, int err)
            {
                ssize_t total_read = 0;
                while (1) {
                    char * pos = std::find(this->cur, this->eof, '\n');
                    if (len < size_t(pos - this->cur)) {
                        total_read += len;
                        memcpy(dest, this->cur, len);
                        this->cur += len;
                        break;
                    }
                    total_read += pos - this->cur;
                    memcpy(dest, this->cur, pos - this->cur);
                    dest += pos - this->cur;
                    this->cur = pos + 1;
                    if (pos != this->eof) {
                        break;
                    }

                    ssize_t ret = this->ibuf.read(this->buf, sizeof(this->buf));

                    if (ret < 0 && errno != EINTR) {
                        return -ERR_TRANSPORT_READ_FAILED;
                    }
                    if (ret == 0) {
                        return -err;
                    }
                    this->eof = this->buf + ret;
                    this->cur = this->buf;
                }
                return total_read;
            }

            int next_line()
            {
                char * pos;
                while ((pos = std::find(this->cur, this->eof, '\n')) == this->eof) {

                    ssize_t ret = this->ibuf.read(this->buf, sizeof(this->buf));

                    if (ret < 0 && errno != EINTR) {
                        return -ERR_TRANSPORT_READ_FAILED;
                    }
                    if (ret == 0) {
                        return -ERR_TRANSPORT_READ_FAILED;
                    }
                    this->eof = this->buf + ret;
                    this->cur = this->buf;
                }
                this->cur = pos+1;
                return 0;
            }
            
            void read_meta(){
                char line[32];
                auto sz = this->read_line(line, sizeof(line), ERR_TRANSPORT_READ_FAILED);
                if (sz < 0) {
                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                }

                // v2
                if (line[0] == 'v') {
                    if (this->next_line()
                     || (sz = this->read_line(line, sizeof(line), ERR_TRANSPORT_READ_FAILED)) < 0
                    ) {
                        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                    }
                    this->meta_header.version = 2;
                    this->meta_header.has_checksum = (line[0] == 'c');
                }
                // else v1

                if (this->next_line()
                 || this->next_line()
                ) {
                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                }
            }

        } reader(cctx, infile_is_encrypted, full_mwrm_filename);
        
        reader.read_meta();

        infile_version       = reader.meta_header.version;
        infile_is_checksumed = reader.meta_header.has_checksum;
    }

    if (verbose) {
        LOG(LOG_INFO, "file_version=%d is_checksumed=%s", infile_version,
            (infile_is_checksumed ? "yes" : "no"));
    }

    // TODO: check compatibility of version and encryption
    if (infile_version < 2) {
        std::cout << "Input file is unencrypted.\n";
        return 0;
    }

    if (infile_is_checksumed == 0) {
        std::cout << "Input file don't include checksum\n";
        return 0;
    }

    /*****************
    * Load file hash *
    *****************/
    MetaLine2 hash_line = {{}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}};

    {
        struct HashLoad
        {
            MetaLine2 & hash_line;
            bool hash_ok;

            HashLoad(const std::string & full_hash_path, const std::string & input_filename,
                     unsigned int infile_version, bool infile_is_checksumed,
                     MetaLine2 & hash_line,
                    CryptoContext * cctx, bool infile_is_encrypted, int verbose)
                : hash_line(hash_line)
                , hash_ok(false)
            {
                transbuf::ifile_buf in_hash_fb(cctx, infile_is_encrypted);
                in_hash_fb.open(full_hash_path.c_str());
                if (verbose) {
                    LOG(LOG_INFO, "File buffer created");
                }

                char temp_buffer[8192];
                memset(temp_buffer, 0, sizeof(temp_buffer));

                ssize_t number_of_bytes_read = in_hash_fb.read(temp_buffer, sizeof(temp_buffer));
                if (verbose) {
                    LOG(LOG_INFO, "Hash data received. Length=%zd", number_of_bytes_read);
                }

                ssize_t filename_len = input_filename.length();

                if (infile_version == 1) {
                    if (verbose) {
                        LOG(LOG_INFO, "Hash data v1");
                    }

                    // Filename HASH_64_BYTES
                    //         ^
                    //         |
                    //     separator
                    if (0 == memcmp(temp_buffer, input_filename.c_str(), filename_len)
                    // Separator
                    &&  (temp_buffer[filename_len] == ' ')) {

                        if (verbose) {
                            LOG(LOG_INFO, "Copy hash");
                        }

                        memcpy(this->hash_line.hash1, 
                               &temp_buffer[filename_len + 1], 
                               sizeof(this->hash_line.hash1));
                        memcpy(this->hash_line.hash2, 
                               &temp_buffer[filename_len + 1 + sizeof(this->hash_line.hash1)],
                               sizeof(this->hash_line.hash2));
                        this->hash_ok = true;
                    }
                    else {
                        std::cerr << "File name mismatch: \"" << full_hash_path << "\"" << std::endl << std::endl;
                    }
                }
                else {
                    if (verbose) {
                        LOG(LOG_INFO, "Hash data v2 or higher");
                    }

                    class ReaderLine2ReaderBuf2
                    {

                    public:
                        ReaderLine2ReaderBuf2(const std::string & full_hash_path, const std::string & input_filename, char * remaining_data_buf, ssize_t remaining_data_length, bool infile_is_checksumed, MetaLine2 & hash_line, bool & hash_ok)
                        {
                            char buf[1024];
                            char * eof = &remaining_data_buf[remaining_data_length];
                            char * cur = &remaining_data_buf[0];
                            // v2
                            REDASSERT(remaining_data_buf[0] == 'v');

                            // skip 3 lines
                            for (auto i = 0 ; i < 3 ; i++)
                            {
                                char * pos = std::find(cur, eof, '\n');
                                if (pos == eof) {
                                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                                }
                                remaining_data_buf += pos - cur;
                                remaining_data_length -= pos - cur;
                                cur = pos + 1;
                            }

                            char * line = cur;
                            ssize_t len = eof-cur-1;
                            
                            if (len <= 0) {
                                throw Error(ERR_TRANSPORT_READ_FAILED);
                            }
                                
                            // Line format "fffff
                            // st_size st_mode st_uid st_gid st_dev st_ino st_mtime st_ctime
                            // sssss eeeee hhhhh HHHHH"
                            //            ^  ^  ^  ^
                            //            |  |  |  |
                            //            |hash1|  |
                            //            |     |  |
                            //        space3    |hash2
                            //                  |
                            //                space4
                            //
                            // filename(1 or >) + space(1) + stat_info(ll|ull * 8) +
                            //     space(1) + start_sec(1 or >) + space(1) + stop_sec(1 or >) +
                            //     space(1) + hash1(64) + space(1) + hash2(64) >= 135

                            // replace final \n by 0 
                            // (to be able to use strtoxxx functions)
                            line[len] = 0;

                            using std::begin;
                            using std::end;

                            printf("========='%s'============================\n", cur);
                            // filename(1 or >) followed by space
                            {
                                char * pos = std::find(cur, eof, ' ');
                                if (pos == eof){
                                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                                }
                                memcpy(hash_line.filename, cur, pos - cur);
                                cur = pos + 1;
                            }
                            printf("========='%s'============================\n", cur);
                            // stat_info(ll|ull * 8) + space(1)
                            {
                                char * pos = std::find(cur, eof, ' ');
                                if (pos == eof || (pos - cur < 2)){
                                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                                }
                                char * pend = nullptr;
                                hash_line.size = strtoll(cur, &pend, 10);
                                if (pend != pos){
                                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                                }
                                cur = pos + 1;
                            }

                            printf("========='%s'============================\n", cur);
                            // stat_info(ll|ull * 8) + space(1)
                            {
                                char * pos = std::find(cur, eof, ' ');
                                if (pos == eof || (pos - cur < 2)){
                                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                                }
                                char * pend = nullptr;
                                hash_line.mode = strtoll(cur, &pend, 10);
                                if (pend != pos){
                                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                                }
                                cur = pos + 1;
                            }

                            auto pline = cur;
                            int err = 0;
                            auto pend = pline + 1;
                            
                            hash_line.uid = strtoll (pline, &pend, 10);
                            err |= (*pend != ' ');
                            pline = pend;
                            
                            hash_line.gid = strtoll (pline, &pend, 10);
                            err |= (*pend != ' ');
                            pline = pend;
                            
                            hash_line.dev = strtoull(pline, &pend, 10);
                            err |= (*pend != ' ');
                            pline = pend;
                            
                            hash_line.ino = strtoll (pline, &pend, 10);
                            err |= (*pend != ' ');
                            pline = pend;
                            
                            hash_line.mtime = strtoll(pline, &pend, 10);
                            err |= (*pend != ' ');
                            pline = pend;
                            
                            hash_line.ctime = strtoll (pline, &pend, 10);

                            if (infile_is_checksumed
                             && !(err |= (len - (pend - line) != (sizeof(hash_line.hash1) + sizeof(hash_line.hash2)) * 2 + 2))
                            ) {
                                err |= (*pend != ' ');
                                ++pend;
                                for (int i = 0 ; i < MD_HASH_LENGTH ; i++){
                                    hash_line.hash1[i] = (chex_to_int(pend[i*2u], err)*16)
                                                       + chex_to_int(pend[i*2u+1], err);
                                }
                                pend += 2*MD_HASH_LENGTH;
                                err |= (*pend != ' ');
                                ++pend;
                                for (int i = 0 ; i < MD_HASH_LENGTH ; i++){
                                    hash_line.hash2[i] = (chex_to_int(pend[i*2u], err)<<4)
                                                       + chex_to_int(pend[i*2u+1], err);
                                }
                                pend += 2*MD_HASH_LENGTH;
                            }
                            err |= bool(*pend);

                            if (err) {
                                throw Error(ERR_TRANSPORT_READ_FAILED);
                            }

                            ssize_t filename_len = input_filename.length();
                            if (0 == memcmp(hash_line.filename, input_filename.c_str(), filename_len)) 
                            {
                                hash_ok = true;
                            }
                            else {
                                std::cerr << "File name mismatch: \"" 
                                          << full_hash_path 
                                          << "\"" << std::endl 
                                          << std::endl;
                            }
                        }
                    } reader(full_hash_path, input_filename, temp_buffer, number_of_bytes_read, infile_is_checksumed, hash_line, this->hash_ok);
                }
            }
        };

        std::string const full_hash_path = hash_path + input_filename;

        try {
            HashLoad meta(full_hash_path, input_filename, infile_version, infile_is_checksumed, hash_line, cctx, infile_is_encrypted, verbose);
        if (!meta.hash_ok) {
            return 1;
        }

        }
        catch (Error const & e) {
            std::cerr << "Exception code (hash): " << e.id << std::endl << std::endl;
        }
        catch (...) {
            std::cerr << "Cannot read hash file: \"" << full_hash_path << "\"" << std::endl << std::endl;
        }

    }

    /******************
    * Check mwrm file *
    ******************/
    const bool is_status_enabled = (infile_version > 1);
    bool result = false;

    FileChecker check(full_mwrm_filename);
    if (infile_is_checksumed){
        check.check_hash_sha256(cctx->get_hmac_key(), sizeof(cctx->get_hmac_key()),
                    (quick_check ? hash_line.hash1 : hash_line.hash2),
                    (quick_check ? sizeof(hash_line.hash1) : sizeof(hash_line.hash2)),
                    quick_check);
        check.check_short_stat(hash_line);
    }
    else {
        check.check_full_stat(hash_line);
    }
    
    if (!check.failed)
    {
        transbuf::ifile_buf ifile(cctx, infile_is_encrypted);
        if (ifile.open(full_mwrm_filename.c_str()) < 0) {
            LOG(LOG_ERR, "failed opening=%s", full_mwrm_filename.c_str());
            std::cerr << "Failed opening file " << full_mwrm_filename << std::endl; 
            std::cerr << "File \"" << full_mwrm_filename << "\" is invalid!" << std::endl << std::endl;
            return 1;;
        }

        struct ReaderBuf3
        {
            private:
            transbuf::ifile_buf & buf;

            public:

            ReaderBuf3(transbuf::ifile_buf & buf) : buf(buf) {}

            ssize_t reader_read(char * buf, size_t len) const {
                return this->buf.read(buf, len);
            }
        } rb(ifile);

        class ReaderLine2ReaderBuf3
        {
            char buf[1024];
            char * eof;
            char * cur;
            ReaderBuf3 reader;

            int read(int err)
            {
                ssize_t ret = this->reader.reader_read(this->buf, sizeof(this->buf));

                if (ret < 0 && errno != EINTR) {
                    return -ERR_TRANSPORT_READ_FAILED;
                }
                if (ret == 0) {
                    return -err;
                }
                this->eof = this->buf + ret;
                this->cur = this->buf;
                return 0;
            }

        public:
            ReaderLine2ReaderBuf3(ReaderBuf3 reader) noexcept
            : eof(buf)
            , cur(buf)
            , reader(reader)
            {
            }

            int read_meta_file2(MetaHeader2 const & meta_header, MetaLine2 & meta_line) {
                if (meta_header.version == 1) {
                    return this->read_meta_file_v1(meta_line);
                }
                else {
                    return this->read_meta_file_v2(meta_header, meta_line);
                }
            }

            MetaHeader2 read_meta_headers()
            {
                MetaHeader2 header{1, false};

                char line[32];
                if (this->read_line(line, sizeof(line), ERR_TRANSPORT_READ_FAILED) < 0) {
                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                }

                // v2
                if (line[0] == 'v') {
                    if (this->next_line()){
                        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                    }
                    if (this->read_line(line, sizeof(line), ERR_TRANSPORT_READ_FAILED) < 0)
                    {
                        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                    }
                    header.version = 2;
                    header.has_checksum = (line[0] == 'c');
                }
                // else v1
                    if (this->next_line()){
                        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                    }
                    if (this->next_line()){
                        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                    }
                    return header;
            }

            int read_meta_file_v1(MetaLine2 & meta_line)
            {
                char line[1024 + (std::numeric_limits<unsigned>::digits10 + 1) * 2 + 4 + 64 * 2 + 2];
                ssize_t len = this->read_line(line, sizeof(line) - 1, ERR_TRANSPORT_NO_MORE_DATA);
                if (len < 0) {
                    return -len;
                }
                line[len] = 0;

                // Line format "fffff sssss eeeee hhhhh HHHHH"
                //                               ^  ^  ^  ^
                //                               |  |  |  |
                //                               |hash1|  |
                //                               |     |  |
                //                           space3    |hash2
                //                                     |
                //                                   space4
                //
                // filename(1 or >) + space(1) + start_sec(1 or >) + space(1) + stop_sec(1 or >) +
                //     space(1) + hash1(64) + space(1) + hash2(64) >= 135
                typedef std::reverse_iterator<char*> reverse_iterator;

                using std::begin;

                reverse_iterator last(line);
                reverse_iterator first(line + len);
                reverse_iterator e1 = std::find(first, last, ' ');
                if (e1 - first == 64) {
                    int err = 0;
                    auto phash = begin(meta_line.hash2);
                    for (char * b = e1.base(), * e = b + 64; e != b; ++b, ++phash) {
                        *phash = (chex_to_int(*b, err) << 4);
                        *phash |= chex_to_int(*++b, err);
                    }
                    REDASSERT(!err);
                }

                reverse_iterator e2 = (e1 == last) ? e1 : std::find(e1 + 1, last, ' ');
                if (e2 - (e1 + 1) == 64) {
                    int err = 0;
                    auto phash = begin(meta_line.hash1);
                    for (char * b = e2.base(), * e = b + 64; e != b; ++b, ++phash) {
                        *phash = (chex_to_int(*b, err) << 4);
                        *phash |= chex_to_int(*++b, err);
                    }
                    REDASSERT(!err);
                }

                if (e1 - first == 64 && e2 != last) {
                    first = e2 + 1;
                    e1 = std::find(first, last, ' ');
                    e2 = (e1 == last) ? e1 : std::find(e1 + 1, last, ' ');
                }

                meta_line.stop_time = meta_parse_sec(e1.base(), first.base());
                if (e1 != last) {
                    ++e1;
                }
                meta_line.start_time = meta_parse_sec(e2.base(), e1.base());

                if (e2 != last) {
                    *e2 = 0;
                }

                auto path_len = std::min(int(e2.base() - line), PATH_MAX);
                memcpy(meta_line.filename, line, path_len);
                meta_line.filename[path_len] = 0;

                return 0;
            }

            int read_meta_file_v2_impl2(bool has_checksum, MetaLine2 & meta_line, bool read_start_stop_time) {
                char line[
                    PATH_MAX + 1 + 1 +
                    (std::numeric_limits<long long>::digits10 + 1 + 1) * 8 +
                    (std::numeric_limits<unsigned long long>::digits10 + 1 + 1) * 2 +
                    (1 + MD_HASH_LENGTH*2) * 2 +
                    2
                ];
                ssize_t len = this->read_line(line, sizeof(line) - 1, ERR_TRANSPORT_NO_MORE_DATA);
                if (len < 0) {
                    return -len;
                }
                line[len] = 0;

                // Line format "fffff
                // st_size st_mode st_uid st_gid st_dev st_ino st_mtime st_ctime
                // sssss eeeee hhhhh HHHHH"
                //            ^  ^  ^  ^
                //            |  |  |  |
                //            |hash1|  |
                //            |     |  |
                //        space3    |hash2
                //                  |
                //                space4
                //
                // filename(1 or >) + space(1) + stat_info(ll|ull * 8) +
                //     space(1) + start_sec(1 or >) + space(1) + stop_sec(1 or >) +
                //     space(1) + hash1(64) + space(1) + hash2(64) >= 135

                using std::begin;
                using std::end;

                auto pline = line + (sread_filename2(begin(meta_line.filename), end(meta_line.filename), line) - line);

                int err = 0;
                auto pend = pline;                   meta_line.size       = strtoll (pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.mode       = strtoull(pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.uid        = strtoll (pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.gid        = strtoll (pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.dev        = strtoull(pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.ino        = strtoll (pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.mtime      = strtoll (pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.ctime      = strtoll (pline, &pend, 10);
                if (read_start_stop_time) {
                err |= (*pend != ' '); pline = pend; meta_line.start_time = strtoll (pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.stop_time  = strtoll (pline, &pend, 10);
                }

                if (has_checksum
                 && !(err |= (len - (pend - line) != (sizeof(meta_line.hash1) + sizeof(meta_line.hash2)) * 2 + 2))
                ) {
                    auto read = [&](unsigned char (&hash)[MD_HASH_LENGTH]) {
                        auto phash = begin(hash);
                        for (auto e = ++pend + sizeof(hash) * 2u; pend != e; ++pend, ++phash) {
                            *phash = (chex_to_int(*pend, err) << 4);
                            *phash |= chex_to_int(*++pend, err);
                        }
                    };
                    read(meta_line.hash1);
                    err |= (*pend != ' ');
                    read(meta_line.hash2);
                }

                err |= bool(*pend);

                if (err) {
                    throw Error(ERR_TRANSPORT_READ_FAILED);
                }

                return 0;
            }

            int read_meta_file_v2(MetaHeader2 const & meta_header, MetaLine2 & meta_line) {
                return this->read_meta_file_v2_impl2(meta_header.has_checksum, meta_line, true);
            }

            ssize_t read_line(char * dest, size_t len, int err)
            {
                ssize_t total_read = 0;
                while (1) {
                    char * pos = std::find(this->cur, this->eof, '\n');
                    if (len < size_t(pos - this->cur)) {
                        total_read += len;
                        memcpy(dest, this->cur, len);
                        this->cur += len;
                        break;
                    }
                    total_read += pos - this->cur;
                    memcpy(dest, this->cur, pos - this->cur);
                    dest += pos - this->cur;
                    this->cur = pos + 1;
                    if (pos != this->eof) {
                        break;
                    }
                    if (int e = this->read(err)) {
                        return e;
                    }
                }
                return total_read;
            }

            int next_line()
            {
                char * pos;
                while ((pos = std::find(this->cur, this->eof, '\n')) == this->eof) {
                    if (int e = this->read(ERR_TRANSPORT_READ_FAILED)) {
                        return e;
                    }
                }
                this->cur = pos+1;
                return 0;
            }
        } reader(rb);

        auto meta_header = reader.read_meta_headers();

        MetaLine2 meta_line_wrm;

        result = true;
        while (reader.read_meta_file2(meta_header, meta_line_wrm) !=
               ERR_TRANSPORT_NO_MORE_DATA) {

            size_t tmp_wrm_filename_len = 0;
            const char * tmp_wrm_filename = basename_len(meta_line_wrm.filename, tmp_wrm_filename_len);
            std::string const meta_line_wrm_filename = std::string(tmp_wrm_filename, tmp_wrm_filename_len);
            std::string const full_meta_mwrm_filename = mwrm_path + meta_line_wrm_filename;

            FileChecker check(full_meta_mwrm_filename);
            if (infile_is_checksumed){
                check.check_hash_sha256(cctx->get_hmac_key(), sizeof(cctx->get_hmac_key()),
                            (quick_check ? meta_line_wrm.hash1 : meta_line_wrm.hash2),
                            (quick_check ? sizeof(meta_line_wrm.hash1) : sizeof(meta_line_wrm.hash2)),
                            quick_check);
                if (check.failed){
                     std::cerr << "Bad checksum for part file \"" 
                               << full_meta_mwrm_filename << "\""
                               << std::endl << std::endl;
                }
                check.check_short_stat(meta_line_wrm);
            }
            else {
                check.check_full_stat(meta_line_wrm);
            }
            
            if (check.failed)
            {
                result = false;
                break;
            }
        }
    }

    if (!result){
        std::cerr << "File \"" << full_mwrm_filename << "\" is invalid!" << std::endl << std::endl;
        return 1;
    }

    std::cout << "No error detected during the data verification." << std::endl << std::endl;
    return 0;
}


static inline int is_file_encrypted(const std::string & full_filename)
{
    uint8_t tmp_buf[4] ={};
    int fd = open(full_filename.c_str(), O_RDONLY);
    if (fd == -1){
        std::cerr << "Input file missing.\n";
        return -1;
    }
    struct fdbuf
    {
        int fd;
        explicit fdbuf(int fd) noexcept : fd(fd){}
        ~fdbuf() {::close(this->fd);}
    } file(fd);

    const size_t len = sizeof(tmp_buf);
    size_t remaining_len = len;
    while (remaining_len) {
        ssize_t ret = ::read(fd, &tmp_buf[len - remaining_len], remaining_len);
        if (ret < 0){
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

    const uint32_t magic = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);
    if (magic == WABCRYPTOFILE_MAGIC) {
        std::cout << "Input file is encrypted.\n";
        return 1;
    }
    return 0;
}


static inline int app_verifier(Inifile & ini, int argc, char const * const * argv, const char * copyright_notice, CryptoContext & cctx)
{
    openlog("verifier", LOG_CONS | LOG_PERROR, LOG_USER);

    std::string hash_path      = ini.get<cfg::video::hash_path>().c_str()  ;
    std::string mwrm_path      = ini.get<cfg::video::record_path>().c_str();
    std::string input_filename                                ;
    bool        quick_check    = false                        ;
    uint32_t    verbose        = 0                            ;

    program_options::options_description desc({
        {'h', "help",    "produce help message"},
        {'v', "version", "show software version"},
        {'q', "quick",   "quick check only"},
        {'s', "hash-path",  &hash_path,         "hash file path"       },
        {'m', "mwrm-path",  &mwrm_path,         "mwrm file path"       },
        {'i', "input-file", &input_filename,    "input mwrm file name" },
        {"verbose",         &verbose,           "more logs"            },
    })
    ;

    auto options = program_options::parse_command_line(argc, argv, desc);

    if (options.count("help") > 0) {
        std::cout << copyright_notice;
        std::cout << "Usage: redver [options]\n\n";
        std::cout << desc << std::endl;
        return 0;
    }

    if (options.count("version") > 0) {
        std::cout << copyright_notice;
        return 0;
    }

    if (options.count("quick") > 0) {
        quick_check = true;
    }

    if (hash_path.c_str()[0] == 0) {
        std::cerr << "Missing hash-path : use -h path\n\n";
        return 1;
    }

    if (mwrm_path.c_str()[0] == 0) {
        std::cerr << "Missing mwrm-path : use -m path\n\n";
        return 1;
    }

    if (input_filename.c_str()[0] == 0) {
        std::cerr << "Missing input mwrm file name : use -i filename\n\n";
        return 1;
    }

    {
        char temp_path[1024]     = {};
        char temp_basename[1024] = {};
        char temp_extension[256] = {};

        canonical_path(input_filename.c_str(), temp_path, sizeof(temp_path), temp_basename, sizeof(temp_basename), temp_extension, sizeof(temp_extension), verbose);

        if (strlen(temp_path) > 0) {
            mwrm_path       = temp_path;
            input_filename  = temp_basename;
            input_filename += temp_extension;
        }
        if (mwrm_path.back() != '/'){
            mwrm_path.push_back('/');
        }
        if (hash_path.back() != '/'){
            hash_path.push_back('/');
        }

    }
    std::cout << "Input file is \"" << mwrm_path << input_filename << "\".\n";

    if (verbose) {
        LOG(LOG_INFO, "hash_path=\"%s\"", hash_path.c_str());
        LOG(LOG_INFO, "mwrm_path=\"%s\"", mwrm_path.c_str());
        LOG(LOG_INFO, "file_name=\"%s\"", input_filename.c_str());
    }

    bool infile_is_encrypted = false;
    std::string const full_mwrm_filename = mwrm_path + input_filename;

    switch (is_file_encrypted(full_mwrm_filename)){
    case -1: // error
        return 1;
    case 1:
        infile_is_encrypted = true;
        break;
    case 0:
        break;
    }

    return check_encrypted_or_checksumed(
                input_filename, mwrm_path, hash_path,
                quick_check, verbose, &cctx, infile_is_encrypted);
}
