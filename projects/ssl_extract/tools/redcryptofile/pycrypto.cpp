#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#include <memory>

// this is to silent warning as Python.h will redefine this constant
#undef _XOPEN_SOURCE
// this is to silent warning as Python.h will redefine this constant
#undef _POSIX_C_SOURCE
#include "Python.h"
#include <structmember.h>
typedef PyObject * __attribute__((__may_alias__)) AlPyObject;
#include <algorithm>
#include <unistd.h>
#include <new>

#include "utils/genrandom.hpp"

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#include <cerrno>
#include <cstddef>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <snappy-c.h>

#include "utils/apps/cryptofile.hpp"
#include "transport/in_filename_transport.hpp"

struct crypto_file_write
{
    int fdbuf_fd;

    char           encrypt_filter3_buf[CRYPTO_BUFFER_SIZE]; //
    EVP_CIPHER_CTX encrypt_filter3_ectx;                    // [en|de]cryption context
    EVP_MD_CTX     encrypt_filter3_hctx;                    // hash context
    EVP_MD_CTX     encrypt_filter3_hctx4k;                  // hash context
    uint32_t       encrypt_filter3_pos;                     // current position in buf
    uint32_t       encrypt_filter3_raw_size;                // the unciphered/uncompressed file size
    uint32_t       encrypt_filter3_file_size;               // the current file size

    int fdbuf_open(int fd)
    {
        this->fdbuf_close();
        this->fdbuf_fd = fd;
        return fd;
    }

    int fdbuf_close()
    {
        if (-1 != this->fdbuf_fd) {
            const int ret = ::close(this->fdbuf_fd);
            this->fdbuf_fd = -1;
            return ret;
        }
        return 0;
    }

    ssize_t fdbuf_write(const void * data, size_t len) const
    {
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            ssize_t ret = ::write(this->fdbuf_fd, static_cast<const char*>(data) + total_sent, remaining_len);
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

    int encrypt_filter3_open(const unsigned char * trace_key, CryptoContext * cctx, const unsigned char * iv)
    {
        ::memset(this->encrypt_filter3_buf, 0, sizeof(this->encrypt_filter3_buf));
        ::memset(&this->encrypt_filter3_ectx, 0, sizeof(this->encrypt_filter3_ectx));
        ::memset(&this->encrypt_filter3_hctx, 0, sizeof(this->encrypt_filter3_hctx));
        ::memset(&this->encrypt_filter3_hctx4k, 0, sizeof(this->encrypt_filter3_hctx4k));
        this->encrypt_filter3_pos = 0;
        this->encrypt_filter3_raw_size = 0;
        this->encrypt_filter3_file_size = 0;

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

        ::EVP_CIPHER_CTX_init(&this->encrypt_filter3_ectx);
        if (::EVP_EncryptInit_ex(&this->encrypt_filter3_ectx, cipher, nullptr, key, iv) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize encrypt context\n", ::getpid());
            return -1;
        }

        // MD stuff
        const EVP_MD * md = EVP_get_digestbyname(MD_HASH_NAME);
        if (!md) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not find message digest algorithm!\n", ::getpid());
            return -1;
        }

        ::EVP_MD_CTX_init(&this->encrypt_filter3_hctx);
        ::EVP_MD_CTX_init(&this->encrypt_filter3_hctx4k);
        if (::EVP_DigestInit_ex(&this->encrypt_filter3_hctx, md, nullptr) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not initialize MD hash context!\n", ::getpid());
            return -1;
        }
        if (::EVP_DigestInit_ex(&this->encrypt_filter3_hctx4k, md, nullptr) != 1) {
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
                if ( ! ::MD_HASH_FUNC(static_cast<unsigned char *>(cctx->get_hmac_key()), CRYPTO_KEY_LENGTH, keyhash)) {
                    LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not hash crypto key!\n", ::getpid());
                    return -1;
                }
                ::memcpy(key_buf, keyhash, MIN(MD_HASH_LENGTH, blocksize));
            }
            else {
                ::memcpy(key_buf, cctx->get_hmac_key(), CRYPTO_KEY_LENGTH);
            }
            for (int idx = 0; idx <  blocksize; idx++) {
                key_buf[idx] = key_buf[idx] ^ 0x36;
            }
            if (::EVP_DigestUpdate(&this->encrypt_filter3_hctx, key_buf, blocksize) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash!\n", ::getpid());
                return -1;
            }
            if (::EVP_DigestUpdate(&this->encrypt_filter3_hctx4k, key_buf, blocksize) != 1) {
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
        ssize_t err = this->fdbuf_write(tmp_buf, 40);
        if (err < ssize_t(40)){
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: write error! error=%s\n", ::getpid(), ::strerror(errno));
            return (err < 0 ? err : -1);
        }

        // update file_size
        this->encrypt_filter3_file_size += 40;

        return this->encrypt_filter3_xmd_update(tmp_buf, 40);
    }

    ssize_t encrypt_filter3_write(const void * data, size_t len)
    {
        unsigned int remaining_size = len;
        while (remaining_size > 0) {
            // Check how much we can append into buffer
            unsigned int available_size = MIN(CRYPTO_BUFFER_SIZE - this->encrypt_filter3_pos, remaining_size);
            // Append and update pos pointer
            ::memcpy(this->encrypt_filter3_buf + this->encrypt_filter3_pos, static_cast<const char*>(data) + (len - remaining_size), available_size);
            this->encrypt_filter3_pos += available_size;
            // If buffer is full, flush it to disk
            if (this->encrypt_filter3_pos == CRYPTO_BUFFER_SIZE) {
                if (this->encrypt_filter3_flush()) {
                    return -1;
                }
            }
            remaining_size -= available_size;
        }
        // Update raw size counter
        this->encrypt_filter3_raw_size += len;
        return len;
    }

    /* Flush procedure (compression, encryption, effective file writing)
     * Return 0 on success, negatif on error
     */
    int encrypt_filter3_flush()
    {
        // No data to flush
        if (!this->encrypt_filter3_pos) {
            return 0;
        }

        // Compress
        // TODO: check this
        char compressed_buf[65536];
        //char compressed_buf[compressed_buf_sz];
        size_t compressed_buf_sz = ::snappy_max_compressed_length(this->encrypt_filter3_pos);
        snappy_status status = snappy_compress(this->encrypt_filter3_buf, this->encrypt_filter3_pos, compressed_buf, &compressed_buf_sz);

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
            default:
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Snappy compression failed with unknown status code (%d)!\n", getpid(), status);
                return -1;
        }

        // Encrypt
        unsigned char ciphered_buf[4 + 65536];
        //char ciphered_buf[ciphered_buf_sz];
        uint32_t ciphered_buf_sz = compressed_buf_sz + AES_BLOCK_SIZE;
        {
            const unsigned char * src_buf = reinterpret_cast<unsigned char*>(compressed_buf);
            if (this->encrypt_filter3_xaes_encrypt(src_buf, compressed_buf_sz, ciphered_buf + 4, &ciphered_buf_sz)) {
                return -1;
            }
        }

        ciphered_buf[0] = ciphered_buf_sz & 0xFF;
        ciphered_buf[1] = (ciphered_buf_sz >> 8) & 0xFF;
        ciphered_buf[2] = (ciphered_buf_sz >> 16) & 0xFF;
        ciphered_buf[3] = (ciphered_buf_sz >> 24) & 0xFF;

        ciphered_buf_sz += 4;

        ssize_t err = this->fdbuf_write(ciphered_buf, ciphered_buf_sz);
        if (err < ssize_t(ciphered_buf_sz)){
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Write error : %s\n", ::getpid(), ::strerror(errno));
            return (err < 0 ? err : -1);
        }

        if (-1 == this->encrypt_filter3_xmd_update(&ciphered_buf, ciphered_buf_sz)) {
            return -1;
        }
        this->encrypt_filter3_file_size += ciphered_buf_sz;

        // Reset buffer
        this->encrypt_filter3_pos = 0;
        return 0;
    }

    int encrypt_filter3_close(unsigned char hash[MD_HASH_LENGTH << 1], const unsigned char * hmac_key)
    {
        int result = this->encrypt_filter3_flush();

        const uint32_t eof_magic = WABCRYPTOFILE_EOF_MAGIC;
        unsigned char tmp_buf[8] = {
            eof_magic & 0xFF,
            (eof_magic >> 8) & 0xFF,
            (eof_magic >> 16) & 0xFF,
            (eof_magic >> 24) & 0xFF,
            uint8_t(this->encrypt_filter3_raw_size & 0xFF),
            uint8_t((this->encrypt_filter3_raw_size >> 8) & 0xFF),
            uint8_t((this->encrypt_filter3_raw_size >> 16) & 0xFF),
            uint8_t((this->encrypt_filter3_raw_size >> 24) & 0xFF),
        };

        ssize_t err = this->fdbuf_write(tmp_buf, 8);
        if (err < ssize_t(8)){
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Write error : %s\n", ::getpid(), ::strerror(errno));
//            return (err < 0 ? err : -1);
        }

        this->encrypt_filter3_file_size += 8;

        this->encrypt_filter3_xmd_update(tmp_buf, 8);

        if (hash) {
            unsigned char tmp_hash[MD_HASH_LENGTH << 1];
            if (::EVP_DigestFinal_ex(&this->encrypt_filter3_hctx4k, tmp_hash, nullptr) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not compute 4k MD digests\n", ::getpid());
                result = -1;
                tmp_hash[0] = '\0';
            }
            if (::EVP_DigestFinal_ex(&this->encrypt_filter3_hctx, tmp_hash + MD_HASH_LENGTH, nullptr) != 1) {
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

    /* Encrypt src_buf into dst_buf. Update dst_sz with encrypted output size
     * Return 0 on success, negative value on error
     */
    int encrypt_filter3_xaes_encrypt(const unsigned char *src_buf, uint32_t src_sz, unsigned char *dst_buf, uint32_t *dst_sz)
    {
        int safe_size = *dst_sz;
        int remaining_size = 0;

        /* allows reusing of ectx for multiple encryption cycles */
        if (EVP_EncryptInit_ex(&this->encrypt_filter3_ectx, nullptr, nullptr, nullptr, nullptr) != 1){
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not prepare encryption context!\n", getpid());
            return -1;
        }
        if (EVP_EncryptUpdate(&this->encrypt_filter3_ectx, dst_buf, &safe_size, src_buf, src_sz) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could encrypt data!\n", getpid());
            return -1;
        }
        if (EVP_EncryptFinal_ex(&this->encrypt_filter3_ectx, dst_buf + safe_size, &remaining_size) != 1){
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not finish encryption!\n", getpid());
            return -1;
        }
        *dst_sz = safe_size + remaining_size;
        return 0;
    }

    /* Update hash context with new data.
     * Returns 0 on success, -1 on error
     */
    int encrypt_filter3_xmd_update(const void * src_buf, uint32_t src_sz)
    {
        if (::EVP_DigestUpdate(&this->encrypt_filter3_hctx, src_buf, src_sz) != 1) {
            LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update hash!\n", ::getpid());
            return -1;
        }
        if (this->encrypt_filter3_file_size < 4096) {
            size_t remaining_size = 4096 - this->encrypt_filter3_file_size;
            size_t hashable_size = MIN(remaining_size, src_sz);
            if (::EVP_DigestUpdate(&this->encrypt_filter3_hctx4k, src_buf, hashable_size) != 1) {
                LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: Could not update 4k hash!\n", ::getpid());
                return -1;
            }
        }
        return 0;
    }

    crypto_file_write(int fd) : fdbuf_fd(fd) {}

    ~crypto_file_write() { this->fdbuf_close();}
};

enum crypto_type {
    CRYPTO_DECRYPT_TYPE,
    CRYPTO_ENCRYPT_TYPE
};

extern "C" {

int gl_read_nb_files = 0;
struct InFilenameTransport * gl_file_store_read[1024];
int gl_write_nb_files = 0;
struct crypto_file_write * gl_file_store_write[1024];

unsigned char iv[32] = {0}; //  not used for reading
}


struct crypto_file
{
    enum crypto_type type;
    int idx;
    crypto_file(): type(CRYPTO_DECRYPT_TYPE), idx(-1) {}

    crypto_file(crypto_type t, CryptoContext * cctx, int fd, const uint8_t * base, size_t base_len)
    : type(t)
    , idx(-1)
    {
        switch (t){
        case CRYPTO_DECRYPT_TYPE:
        {
            auto cf = new InFilenameTransport(cctx, fd, base, base_len);
            int idx = 0;
            for (; idx < gl_read_nb_files ; idx++){
                if (gl_file_store_read[idx] == nullptr){
                    break;
                }
            }
            gl_read_nb_files += 1;
            gl_file_store_read[idx] = cf;
            this->idx = idx;
        }
        break;
        case CRYPTO_ENCRYPT_TYPE:
        {
            auto cf = new crypto_file_write(fd);
            int idx = 0;
            for (; idx < gl_write_nb_files ; idx++){
                if (gl_file_store_write[idx] == nullptr){
                    break;
                }
            }
            gl_write_nb_files += 1;
            gl_file_store_write[idx] = cf;
            this->idx = idx;
        }
        break;
        }
    }
};

extern "C" {
int gl_nb_files = 0;
struct crypto_file gl_file_store[1024];
}


extern "C" {
    UdevRandom * get_rnd();
    CryptoContext * get_cctx();
}

/* File format V1:  ([...] represent an uint32_t)
 *
 * Header:
 *  [WABCRYPTOFILE_MAGIC][FILE_VERSION][Crypto IV]
 *
 * Chunk:
 *  [ciphered chunk size (size it takes on disk)][data]
 *
 * Footer:
 *  [WABCRYPTOFILE_EOF_MAGIC][raw_file_size]
 *
 */


UdevRandom * get_rnd(){
    static UdevRandom * rnd = nullptr;
    if (rnd == nullptr){
        rnd = new UdevRandom;
    }
    return rnd;
}

Inifile * get_ini(){
    static Inifile * ini = nullptr;
    if (ini == nullptr){
        ini = new Inifile;
        ini->set<cfg::crypto::key0>(
            "\x01\x02\x03\x04\x05\x06\x07\x08"
            "\x01\x02\x03\x04\x05\x06\x07\x08"
            "\x01\x02\x03\x04\x05\x06\x07\x08"
            "\x01\x02\x03\x04\x05\x06\x07\x08");
    }
    return ini;
}


CryptoContext * get_cctx()
{
    static CryptoContext * cctx = nullptr;
    if (cctx == nullptr){
        cctx = new CryptoContext(*get_rnd(), *get_ini());
    }
    return cctx;
}

extern "C" {

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
} redcryptofile_NoddyObject;

// This union is work around for
typedef union {
    PyTypeObject pto;
    PyObject po;
} t_PyTyOb;


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wmissing-braces" // CLang
t_PyTyOb redcryptofile_NoddyType = {
    PyObject_HEAD_INIT(nullptr)
    0,                         /*ob_size*/
    "redcryptofile.Noddy",     /*tp_name*/
    sizeof(redcryptofile_NoddyObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    nullptr,                   /*tp_dealloc*/
    nullptr,                   /*tp_print*/
    nullptr,                   /*tp_getattr*/
    nullptr,                   /*tp_setattr*/
    nullptr,                   /*tp_compare*/
    nullptr,                   /*tp_repr*/
    nullptr,                   /*tp_as_number*/
    nullptr,                   /*tp_as_sequence*/
    nullptr,                   /*tp_as_mapping*/
    nullptr,                   /*tp_hash */
    nullptr,                   /*tp_call*/
    nullptr,                   /*tp_str*/
    nullptr,                   /*tp_getattro*/
    nullptr,                   /*tp_setattro*/
    nullptr,                   /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "Noddy objects",           /* tp_doc */
    nullptr,                   /* tp_traverse */
    nullptr,                   /* tp_clear */
    nullptr,                   /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    nullptr,                   /* tp_iter */
    nullptr,                   /* tp_iternext */
    nullptr,                   /* tp_methods */
    nullptr,                   /* tp_members */
    nullptr,                   /* tp_getset */
    nullptr,                   /* tp_base */
    nullptr,                   /* tp_dict */
    nullptr,                   /* tp_descr_get */
    nullptr,                   /* tp_descr_set */
    0,                         /* tp_dictoffset */
    nullptr,                   /* tp_init */
    nullptr,                   /* tp_alloc */
    nullptr,                   /* tp_new */
    nullptr,                   /* tp_free */
    nullptr,                   /* tp_is_gc */
    nullptr,                   /* tp_bases */
    nullptr,                   /* tp_mro */
    nullptr,                   /* tp_cache */
    nullptr,                   /* tp_subclasses */
    nullptr,                   /* tp_weaklist */
    nullptr,                   /* tp_del */
};
#pragma GCC diagnostic pop

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    UdevRandom * rnd;
} PyORandom;

static void Random_dealloc(PyORandom* self) {
    delete self->rnd;
    self->ob_type->tp_free(reinterpret_cast<PyObject*>(self));
}

static PyObject *Random_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    PyORandom *self = reinterpret_cast<PyORandom *>(type->tp_alloc(type, 0));
    if (self != nullptr) {
        self->rnd = new UdevRandom;
    }
    return reinterpret_cast<PyObject*>(self);
}

static int Random_init(PyORandom *self, PyObject *args, PyObject *kwds)
{
    if (self != nullptr) {
        if (self->rnd == nullptr){
            self->rnd = new UdevRandom;
        }
    }
    return 0;
}

static PyObject *
Random_rand(PyORandom* self)
{
    long val = static_cast<long>(self->rnd->rand64());
    PyObject * result = PyInt_FromLong(val);
    return result;
}

static PyMemberDef Random_members[] = {
    {nullptr, 0, 0, 0, nullptr}
};


static PyMethodDef Random_methods[] = {
    {"rand", reinterpret_cast<PyCFunction>(Random_rand), METH_NOARGS, "Return a new random int"},
    {nullptr, nullptr, 0, nullptr}
  /* Sentinel */
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wmissing-braces" // CLang
t_PyTyOb PyTyRandom = {
    PyObject_HEAD_INIT(nullptr)
    0,                         /*ob_size*/
    "redcryptofile.Random",    /*tp_name*/
    sizeof(PyORandom), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    reinterpret_cast<destructor>(Random_dealloc),/*tp_dealloc*/
    nullptr,                   /*tp_print*/
    nullptr,                   /*tp_getattr*/
    nullptr,                   /*tp_setattr*/
    nullptr,                   /*tp_compare*/
    nullptr,                   /*tp_repr*/
    nullptr,                   /*tp_as_number*/
    nullptr,                   /*tp_as_sequence*/
    nullptr,                   /*tp_as_mapping*/
    nullptr,                   /*tp_hash */
    nullptr,                   /*tp_call*/
    nullptr,                   /*tp_str*/
    nullptr,                   /*tp_getattro*/
    nullptr,                   /*tp_setattro*/
    nullptr,                   /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT| Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Random objects",          /* tp_doc */
    nullptr,                   /* tp_traverse */
    nullptr,                   /* tp_clear */
    nullptr,                   /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    nullptr,                   /* tp_iter */
    nullptr,                   /* tp_iternext */
    Random_methods,            /* tp_methods */
    Random_members,            /* tp_members */
    nullptr,                   /* tp_getset */
    nullptr,                   /* tp_base */
    nullptr,                   /* tp_dict */
    nullptr,                   /* tp_descr_get */
    nullptr,                   /* tp_descr_set */
    0,                         /* tp_dictoffset */
    reinterpret_cast<initproc>(Random_init),     /* tp_init */
    nullptr,                   /* tp_alloc */
    Random_new,                /* tp_new */
    nullptr,                   /* tp_free */
    nullptr,                   /* tp_is_gc */
    nullptr,                   /* tp_bases */
    nullptr,                   /* tp_mro */
    nullptr,                   /* tp_cache */
    nullptr,                   /* tp_subclasses */
    nullptr,                   /* tp_weaklist */
    nullptr,                   /* tp_del */
};
#pragma GCC diagnostic pop

namespace {
    inline PyObject * py_return_none() {
#if defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
        Py_RETURN_NONE;
#if defined(__GNUC__) || defined(__clang__)
# pragma GCC diagnostic pop
#endif
    }
}

static PyObject *python_redcryptofile_open(PyObject* self, PyObject* args)
{
    char *path = nullptr;
    char *omode = nullptr;
    if (!PyArg_ParseTuple(args, "ss", &path, &omode)){
        return py_return_none();
    }

    if (omode[0] == 'r') {
        int system_fd = open(path, O_RDONLY, 0600);
        if (system_fd == -1){
            return py_return_none();
        }

        size_t base_len = 0;
        const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(path, base_len));

        auto result = crypto_file(CRYPTO_DECRYPT_TYPE, get_cctx(), system_fd, base, base_len);
        int fdi = 0;
        for (; fdi < gl_nb_files ; fdi++){
            if (gl_file_store[fdi].idx == -1){
                break;
            }
        }
        gl_nb_files += 1;
        gl_file_store[fdi] = result;
        return Py_BuildValue("i", fdi);

    } else if (omode[0] == 'w') {

        size_t base_len = 0;
        const uint8_t * base = reinterpret_cast<const uint8_t *>(basename_len(path, base_len));

        unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
        get_cctx()->get_derived_key(trace_key, base, base_len);

        unsigned i = 0;
        for (i = 0; i < sizeof(iv) ; i++){ iv[i] = i; }

        int system_fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0600);
        if (system_fd == -1){
            return py_return_none();
        }

        auto result = crypto_file(CRYPTO_ENCRYPT_TYPE, get_cctx(), system_fd, base, base_len);
        auto cfw = gl_file_store_write[result.idx];
        cfw->encrypt_filter3_open(trace_key, get_cctx(), iv);

        int fd = 0;
        for (; fd < gl_nb_files ; fd++){
            if (gl_file_store[fd].idx == -1){
                break;
            }
        }
        gl_nb_files += 1;
        gl_file_store[fd] = result;
        return Py_BuildValue("i", fd);
    } else {
        return py_return_none();
    }

    return Py_BuildValue("i", -1);
}

static PyObject *python_redcryptofile_flush(PyObject* self, PyObject* args)
{
    int fd = 0;
    if (!PyArg_ParseTuple(args, "i", &fd)){
        return py_return_none();
    }
    if (fd >= gl_nb_files){
        return py_return_none();
    }
    auto & cf = gl_file_store[fd];
    int result = -1;
    if (cf.type == CRYPTO_ENCRYPT_TYPE){
        auto & cfw = gl_file_store_write[cf.idx];
        result = cfw->encrypt_filter3_flush();
    }
    return Py_BuildValue("i", result);
}

static PyObject *python_redcryptofile_close(PyObject* self, PyObject* args)
{
    int fd = 0;
    unsigned char hash[MD_HASH_LENGTH<<1];
    char hash_digest[(MD_HASH_LENGTH*4)+1];

    if (!PyArg_ParseTuple(args, "i", &fd)){
        return py_return_none();
    }

    if (fd >= static_cast<int>(sizeof(gl_file_store)/sizeof(gl_file_store[0]))){
        return py_return_none();
    }

    auto & cf = gl_file_store[fd];
    if (cf.idx == -1){
        return py_return_none();
    }

    int result = 0;
    switch (cf.type){
    case CRYPTO_DECRYPT_TYPE:
    {
        auto cfr = gl_file_store_read[cf.idx];
        gl_file_store_read[cf.idx] = nullptr;
        gl_read_nb_files--;
        delete cfr;
    }
    break;
    case CRYPTO_ENCRYPT_TYPE:
    {
        auto cfw = gl_file_store_write[cf.idx];
        gl_file_store_write[cf.idx] = nullptr;
        gl_write_nb_files--;
        result = cfw->encrypt_filter3_close(hash, get_cctx()->get_hmac_key());
        delete cfw;
    }
    break;
    }

    cf.idx = -1;
    gl_nb_files--;

    // Crazy API: return error as integer or HASH as string... change that
    if (result){
        return Py_BuildValue("i", result);
    }

    int idx = 0;
    for (idx = 0; idx < MD_HASH_LENGTH; idx++) {
        sprintf(hash_digest + idx * 2, "%02x", hash[idx]);
    }
    for (idx = MD_HASH_LENGTH; idx < (MD_HASH_LENGTH*2); idx++) {
        sprintf(hash_digest + idx * 2, "%02x", hash[idx]);
    }
    hash_digest[MD_HASH_LENGTH*4] = 0;
    return Py_BuildValue("s", hash_digest);
}

static PyObject *python_redcryptofile_write(PyObject* self, PyObject* args)
{
    int fd;
    PyObject *python_buf;
    if (!PyArg_ParseTuple(args, "iS", &fd, &python_buf)){
        return py_return_none();
    }

    int buf_len = PyString_Size(python_buf);
    if (buf_len > 2147483647 || buf_len < 0){
        return Py_BuildValue("i", -1);
    }
    char *buf = PyString_AsString(python_buf);

    if (fd >= gl_nb_files){
        return py_return_none();
    }

    auto & cf = gl_file_store[fd];
    int result = -1;
    if (cf.type == CRYPTO_ENCRYPT_TYPE){
        auto & cfw = gl_file_store_write[cf.idx];
        result = cfw->encrypt_filter3_write(buf, buf_len);
    }

    return Py_BuildValue("i", result);
}

static PyObject *python_redcryptofile_read(PyObject* self, PyObject* args)
{
    int fd;
    int buf_len;
    if (!PyArg_ParseTuple(args, "ii", &fd, &buf_len)){
        return nullptr;
    }
    if (buf_len > 2147483647 || buf_len <= 0){
        return Py_BuildValue("i", -1);
    }
    if (fd >= gl_nb_files){
        LOG(LOG_ERR, "[CRYPTO_ERROR][%d]: fd=%d > gl_nb_files=%d\n", ::getpid(), fd, gl_nb_files);
        return py_return_none();
    }

    std::unique_ptr<char[]> buf(new char[buf_len]);
    char * pbuffer = buf.get();

    auto & cf = gl_file_store[fd];
    if (cf.type ==  CRYPTO_DECRYPT_TYPE) {
        auto & cfr = gl_file_store_read[cf.idx];
        try {
            cfr->recv(&pbuffer, buf_len);
        } catch (...) {
            return PyString_FromStringAndSize("", 0);
        }
    }
    return PyString_FromStringAndSize(buf.get(), pbuffer-buf.get());
}

static PyMethodDef redcryptoFileMethods[] = {
    {"open", python_redcryptofile_open, METH_VARARGS, ""},
    {"flush", python_redcryptofile_flush, METH_VARARGS, ""},
    {"close", python_redcryptofile_close, METH_VARARGS, ""},
    {"write", python_redcryptofile_write, METH_VARARGS, ""},
    {"read", python_redcryptofile_read, METH_VARARGS, ""},
    {nullptr, nullptr, 0, nullptr}
};

PyMODINIT_FUNC
initredcryptofile(void)
{
#if defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
    PyObject* module = Py_InitModule3("redcryptofile", redcryptoFileMethods,
                           "redcryptofile module");
#if defined(__clang__)
# pragma GCC diagnostic pop
#endif

    OpenSSL_add_all_digests();

    size_t idx = 0;
    for (; idx < sizeof(gl_file_store)/sizeof(gl_file_store[0]);idx++)
    {
        gl_file_store[idx].idx = -1;
    }
    size_t idxr = 0;
    for (; idxr < sizeof(gl_file_store_read)/sizeof(gl_file_store_read[0]);idxr++)
    {
        gl_file_store_read[idxr] = nullptr;
    }
    size_t idxw = 0;
    for (; idxw < sizeof(gl_file_store_write)/sizeof(gl_file_store_write[0]);idxw++)
    {
        gl_file_store_write[idxw] = nullptr;
    }

#if defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
    redcryptofile_NoddyType.pto.tp_new = PyType_GenericNew;
    if (PyType_Ready(&redcryptofile_NoddyType.pto) == 0){
        Py_INCREF(&redcryptofile_NoddyType.po);
        PyModule_AddObject(module, "Noddy", &redcryptofile_NoddyType.po);
    }

//    PyTyRandom.pto.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyTyRandom.pto) == 0){
        Py_INCREF(&PyTyRandom.po);
        PyModule_AddObject(module, "Random", &PyTyRandom.po);
    }
#if defined(__GNUC__) || defined(__clang__)
# pragma GCC diagnostic pop
#endif
}

}
