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

static inline void load_ssl_digests(bool encryption)
{
    if (encryption){
        OpenSSL_add_all_digests();
    }
}

static inline int check_encrypted_or_checksumed(
                                       std::string const & input_filename,
                                       std::string const & mwrm_path,
                                       std::string const & hash_path,
                                       bool quick_check,
                                       uint32_t verbose,
                                       CryptoContext * cctx) {
                                       
    std::string const full_mwrm_filename = mwrm_path + input_filename;

    bool infile_is_encrypted = false;

    uint8_t tmp_buf[4] ={};
    int fd = open(full_mwrm_filename.c_str(), O_RDONLY);
    if (fd == -1){
        std::cerr << "Input file missing.\n";
        return 1;
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
            return 1;
        }
        // We must exit loop or we will enter infinite loop
        remaining_len -= ret;
    }

    const uint32_t magic = tmp_buf[0] + (tmp_buf[1] << 8) + (tmp_buf[2] << 16) + (tmp_buf[3] << 24);
    if (magic == WABCRYPTOFILE_MAGIC) {
        std::cout << "Input file is encrypted.\n";
        infile_is_encrypted = true;
    }
                                       
    unsigned infile_version = 1;
    bool infile_is_checksumed = false;

    load_ssl_digests(infile_is_encrypted);

    {
        class MwrmHeadersReader
        {
            char buf[1024];
            char * eof;
            char * cur;
            transbuf::ifile_buf ibuf;
        public:
            MetaHeader2 meta_header;

            explicit MwrmHeadersReader(CryptoContext * cctx, int encryption, 
                const std::string & full_mwrm_filename)
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

            int next_line()
            {
                char * pos;
                while ((pos = std::find(this->cur, this->eof, '\n')) == this->eof) {
                    ssize_t ret = this->ibuf.read(this->buf, sizeof(this->buf));

                    if (ret < 0 && errno != EINTR) {
                        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                    }
                    if (ret == 0) {
                        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                    }
                    this->eof = this->buf + ret;
                    this->cur = this->buf;
                }
                this->cur = pos+1;
                return 0;
            }
            
            void read_meta(){
                this->next_line();
                // v2
                if (cur[0] == 'v') {
                    this->next_line();
                    this->next_line();
                    this->meta_header.version = 2;
                    this->meta_header.has_checksum = (cur[0] == 'c');
                }
                // else v1
                // common lines to all versions
                this->next_line();
                this->next_line();
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

            long long int get_ll(char * & cur, char * eof, char sep, int err)
            {
                char * pos = std::find(cur, eof, sep);
                if (pos == eof || (pos - cur < 2)){
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

            void in_copy_bytes(uint8_t * hash, int len, char * & cur, char * eof, int err)
            {
                if (eof - cur < len){
                    throw Error(err);
                }
                memcpy(hash, cur, len);
                cur += len;
            }

            void in_hex256(uint8_t * hash, int len, char * & cur, char * eof, char sep, int exc)
            {
                int err = 0;
                char * pos = std::find(cur, eof, sep);
                if (pos == eof || (pos - cur != 2*len)){
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


            HashLoad(const std::string & full_hash_path, const std::string & input_filename,
                     unsigned int infile_version, bool infile_is_checksumed,
                     MetaLine2 & hash_line,
                    CryptoContext * cctx, bool infile_is_encrypted, int verbose)
                : hash_line(hash_line)
            {
                transbuf::ifile_buf in_hash_fb(cctx, infile_is_encrypted);
                in_hash_fb.open(full_hash_path.c_str());

                char buffer[8192];
                memset(buffer, 0, sizeof(buffer));

                ssize_t len = in_hash_fb.read(buffer, sizeof(buffer));

                char * eof =  &buffer[len];
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
                    this->in_copy_bytes(this->hash_line.hash1, MD_HASH_LENGTH, cur, eof,
                                        ERR_TRANSPORT_READ_FAILED);
                    this->in_copy_bytes(this->hash_line.hash2, MD_HASH_LENGTH, cur, eof,
                                        ERR_TRANSPORT_READ_FAILED);

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
                    // st_size + space
                    hash_line.size = this->get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
                    // st_mode + space
                    hash_line.mode = this->get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
                    // st_uid + space
                    hash_line.uid = this->get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
                    // st_gid + space
                    hash_line.gid = this->get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
                    // st_dev + space
                    hash_line.dev = this->get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
                    // st_ino + space
                    hash_line.ino = this->get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
                    // st_mtime + space
                    hash_line.mtime = this->get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
                    // st_ctime + space
                    hash_line.ctime = this->get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);

                    if (infile_is_checksumed){
                        // HASH1 + space
                        this->in_hex256(hash_line.hash1, MD_HASH_LENGTH, cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
                        // HASH1 + CR
                        this->in_hex256(hash_line.hash2, MD_HASH_LENGTH, cur, eof, '\n', ERR_TRANSPORT_READ_FAILED);
                    }
                }
            }
        };

        std::string const full_hash_path = hash_path + input_filename;

        // if reading hash fails
        try {
            HashLoad meta(full_hash_path, input_filename, infile_version, infile_is_checksumed, hash_line, cctx, infile_is_encrypted, verbose);
        }
        catch (Error const & e) {
            std::cerr << "Exception code (hash): " << e.id << std::endl << std::endl;
            if (infile_is_checksumed){
                return 1;
            }
        }
        catch (...) {
            std::cerr << "Cannot read hash file: \"" << full_hash_path << "\"" << std::endl << std::endl;
            if (infile_is_checksumed){
                return 1;
            }
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

        class ReaderLine2ReaderBuf3
        {
            char buf[1024];
            char * eof;
            char * cur;
            transbuf::ifile_buf & reader_buf;


            long long int get_ll(char * & cur, char * eof, char sep, int err)
            {
                char * pos = std::find(cur, eof, sep);
                if (pos == eof || (pos - cur < 2)){
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

            void in_copy_bytes(uint8_t * hash, int len, char * & cur, char * eof, int err)
            {
                if (eof - cur < len){
                    throw Error(err);
                }
                memcpy(hash, cur, len);
                cur += len;
            }

            void in_hex256(uint8_t * hash, int len, char * & cur, char * eof, char sep, int exc)
            {
                int err = 0;
                char * pos = std::find(cur, eof, sep);
                if (pos == eof || (pos - cur != 2*len)){
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

        public:
            ReaderLine2ReaderBuf3(transbuf::ifile_buf & reader_buf) noexcept
            : eof(buf)
            , cur(buf)
            , reader_buf(reader_buf)
            {
            }

            int read_meta_file2(MetaHeader2 const & meta_header, MetaLine2 & meta_line) {
                if (meta_header.version == 1) {
                    this->read_meta_file_v1(meta_line);
                    return 0;
                }
                else {
                    return this->read_meta_file_v2(meta_header, meta_line);
                }
            }

            MetaHeader2 read_meta_headers()
            {
                MetaHeader2 header{1, false};

                this->next_line();

                // v2
                if (this->cur[0] == 'v') {
                    this->next_line();
                    this->next_line();
                    header.version = 2;
                    header.has_checksum = (this->cur[0] == 'c');
                }
                // else v1
                this->next_line();
                this->next_line();
                return header;
            }

            void read_meta_file_v1(MetaLine2 & meta_line)
            {
                this->next_line();
                this->eof[0] = 0;
                size_t len = this->eof - this->cur;

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
                reverse_iterator first(cur);
                reverse_iterator last(cur + len);
                reverse_iterator space4 = std::find(last, first, ' ');
                space4++;
                reverse_iterator space3 = std::find(space4, first, ' ');
                space3++;
                reverse_iterator space2 = std::find(space3, first, ' ');
                space2++;
                reverse_iterator space1 = std::find(space2, first, ' ');
                space1++;
                int path_len = first-space1;
                this->in_copy_bytes(reinterpret_cast<uint8_t*>(meta_line.filename), path_len, cur, eof, ERR_TRANSPORT_READ_FAILED);
                this->cur += path_len + 1;

                // start_time + ' '
                meta_line.start_time = this->get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
                // stop_time + ' '
                meta_line.stop_time = this->get_ll(cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
                // HASH1 + ' '
                this->in_hex256(meta_line.hash1, MD_HASH_LENGTH, cur, eof, ' ', ERR_TRANSPORT_READ_FAILED);
                // HASH1 + CR
                this->in_hex256(meta_line.hash2, MD_HASH_LENGTH, cur, eof, '\n', ERR_TRANSPORT_READ_FAILED);
            }

            int read_meta_file_v2(MetaHeader2 const & meta_header, MetaLine2 & meta_line) 
            {
                this->next_line();
                this->eof[0] = 0;
                size_t len = this->eof - this->cur;
                this->eof[len] = 0;

                // Line format "fffff
                // st_size st_mode st_uid st_gid st_dev st_ino st_mtime st_ctime
                // sssss eeeee hhhhh HHHHH"
                //            ^  ^  ^  ^
                //            |  |  |  |
                //            |hash1|  |
                //            |     |  |
                //        space11   |hash2
                //                  |
                //                space12
                //
                // filename(1 or >) + space(1) + stat_info(ll|ull * 8) +
                //     space(1) + start_sec(1 or >) + space(1) + stop_sec(1 or >) +
                //     space(1) + hash1(64) + space(1) + hash2(64) >= 135

                using std::begin;
                using std::end;

                auto pline = cur + (sread_filename2(begin(meta_line.filename), end(meta_line.filename), cur) - cur);

                int err = 0;
                auto pend = pline;                   meta_line.size       = strtoll (pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.mode       = strtoull(pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.uid        = strtoll (pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.gid        = strtoll (pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.dev        = strtoull(pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.ino        = strtoll (pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.mtime      = strtoll (pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.ctime      = strtoll (pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.start_time = strtoll (pline, &pend, 10);
                err |= (*pend != ' '); pline = pend; meta_line.stop_time  = strtoll (pline, &pend, 10);

                if (meta_header.has_checksum
                 && !(err |= (len - (pend - cur) != (sizeof(meta_line.hash1) + sizeof(meta_line.hash2)) * 2 + 2))
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


            void next_line()
            {
                char * pos;
                while ((pos = std::find(this->cur, this->eof, '\n')) == this->eof) {
                    ssize_t ret = this->reader_buf.read(this->buf, sizeof(this->buf));

                    if (ret == 0) {
                        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                    }
                    this->eof = this->buf + ret;
                    this->cur = this->buf;
                }
                this->cur = pos+1;
            }
        } reader(ifile);

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

    return check_encrypted_or_checksumed(
                input_filename, mwrm_path, hash_path,
                quick_check, verbose, &cctx);
}
