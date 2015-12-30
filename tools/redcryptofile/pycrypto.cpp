#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <memory>

// this is to silent warning as Python.h will redefine this constant
#undef _POSIX_C_SOURCE
#include "Python.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include <unistd.h>
#include <genrandom.hpp>

#include <new>

#include "fdbuf.hpp"
#include "filter/crypto_filter.hpp"

#include "cryptofile.h"

struct crypto_file;

/**********************************************
 *                Public API                  *
 **********************************************/

enum Priv_crypto_type {
    CRYPTO_DECRYPT_TYPE,
    CRYPTO_ENCRYPT_TYPE
};

class Priv_crypto_type_base
{
    Priv_crypto_type type;

public:
    Priv_crypto_type_base(Priv_crypto_type t)
    : type(t)
    {}

    bool is_encrypt() const
    { return CRYPTO_ENCRYPT_TYPE == type; }

    bool is_decrypt() const
    { return CRYPTO_DECRYPT_TYPE == type; }
};

struct Priv_crypto_file_decrypt
: Priv_crypto_type_base
{
  transfil::decrypt_filter decrypt;
  io::posix::fdbuf file;

  Priv_crypto_file_decrypt(int fd)
  : Priv_crypto_type_base(CRYPTO_DECRYPT_TYPE)
  , file(fd)
  {}
};

struct Priv_crypto_file_encrypt
: Priv_crypto_type_base
{
  transfil::encrypt_filter encrypt;
  io::posix::fdbuf file;

  Priv_crypto_file_encrypt(int fd)
  : Priv_crypto_type_base(CRYPTO_ENCRYPT_TYPE)
  , file(fd)
  {}
};


crypto_file * crypto_open_read(int systemfd, unsigned char * trace_key,  CryptoContext * cctx)
{
    (void)cctx;
    Priv_crypto_file_decrypt * cf_struct = new (std::nothrow) Priv_crypto_file_decrypt(systemfd);

    if (!cf_struct) {
        return nullptr;
    }

    if (-1 == cf_struct->decrypt.open(cf_struct->file, trace_key)) {
        delete cf_struct;
        return nullptr;
    }

    return reinterpret_cast<crypto_file*>(cf_struct);
}

crypto_file * crypto_open_write(int systemfd, unsigned char * trace_key, CryptoContext * cctx, const unsigned char * iv)
{
    Priv_crypto_file_encrypt * cf_struct = new (std::nothrow) Priv_crypto_file_encrypt(systemfd);

    if (!cf_struct) {
        return nullptr;
    }

    if (-1 == cf_struct->encrypt.open(cf_struct->file, trace_key, cctx, iv)) {
        delete cf_struct;
        return nullptr;
    }

    return reinterpret_cast<crypto_file*>(cf_struct);
}

/* Flush procedure (compression, encryption, effective file writing)
 * Return 0 on success, -1 on error
 */
int crypto_flush(crypto_file * cf)
{
    if (reinterpret_cast<Priv_crypto_type_base*>(cf)->is_decrypt()) {
        return -1;
    }
    Priv_crypto_file_encrypt * cf_struct = reinterpret_cast<Priv_crypto_file_encrypt*>(cf);
    return cf_struct->encrypt.flush(cf_struct->file);
}

/* The actual read method. Read chunks until we reach requested size.
 * Return the actual size read into buf, -1 on error
 */
int crypto_read(crypto_file * cf, char * buf, unsigned int buf_size)
{
    if (reinterpret_cast<Priv_crypto_type_base*>(cf)->is_decrypt()) {
        Priv_crypto_file_decrypt * cf_struct = reinterpret_cast<Priv_crypto_file_decrypt*>(cf);
        return cf_struct->decrypt.read(cf_struct->file, buf, buf_size);
    }
    return -1;
}

/* Actually appends data to crypto_file buffer, flush if buffer gets full
 * Return the written size, -1 on error
 */
int crypto_write(crypto_file *cf, const char * buf, unsigned int size)
{
    if (reinterpret_cast<Priv_crypto_type_base*>(cf)->is_decrypt()) {
        return -1;
    }
    Priv_crypto_file_encrypt * cf_struct = reinterpret_cast<Priv_crypto_file_encrypt*>(cf);
    return cf_struct->encrypt.write(cf_struct->file, buf, size);
}

int crypto_close(crypto_file *cf, unsigned char hash[MD_HASH_LENGTH << 1], unsigned char * hmac_key)
{
    int nResult = 0;

    if (reinterpret_cast<Priv_crypto_type_base*>(cf)->is_decrypt()) {
        Priv_crypto_file_decrypt * cf_struct = reinterpret_cast<Priv_crypto_file_decrypt*>(cf);
        delete cf_struct;
    }
    else {
        Priv_crypto_file_encrypt * cf_struct = reinterpret_cast<Priv_crypto_file_encrypt*>(cf);
        nResult = cf_struct->encrypt.close(cf_struct->file, hash, hmac_key);
        delete cf_struct;
    }

    return nResult;
}

#include "crypto_impl.h"

extern "C" {
    UdevRandom * get_rnd();
    CryptoContext * get_cctx();
}

UdevRandom * get_rnd(){
    static UdevRandom * rnd = nullptr;
    if (rnd == nullptr){
        rnd = new UdevRandom;
    }
    return rnd;
}


CryptoContext * get_cctx()
{
    static CryptoContext * cctx = nullptr;
    if (cctx == nullptr){
        cctx = new CryptoContext(*get_rnd());
    }
    return cctx;
}

extern "C" {

int gl_nb_files = 0;
struct crypto_file * gl_file_store[1024];
unsigned char iv[32] = {0}; //  not used for reading

static PyObject *python_redcryptofile_open(PyObject* self, PyObject* args)
{
    char *path = NULL;
    char *omode = NULL;
    if (!PyArg_ParseTuple(args, "ss", &path, &omode))
        return NULL;
    unsigned char derivator[DERIVATOR_LENGTH];
    get_cctx()->get_derivator(path, derivator, DERIVATOR_LENGTH);
    unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
    if (compute_hmac(trace_key, get_cctx()->crypto_key, derivator) == -1){
        return NULL;
    }

    struct crypto_file * result = NULL;
    if (omode[0] == 'r') {
        int system_fd = open(path, O_RDONLY, 0600);
        if (system_fd == -1){
            printf("failed opening=%s\n", path);
            return NULL;
        }

        result = crypto_open_read(system_fd, trace_key, get_cctx());

        if (!result) {
            close(system_fd);
            return NULL;
        }
    } else if (omode[0] == 'w') {
        unsigned i = 0;
        for (i = 0; i < sizeof(iv) ; i++){ iv[i] = i; }

        int system_fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, 0600);
        if (system_fd == -1){
            printf("failed opening=%s\n", path);
            return NULL;
        }

        result = crypto_open_write(system_fd, trace_key, get_cctx(), iv);
        if (!result) {
            close(system_fd);
            return NULL;
        }

    } else {
        return NULL;
    }

    int fd = 0;
    for (; fd < gl_nb_files ; fd++){
        if (gl_file_store[fd] == NULL){
            break;
        }
    }
    if (fd == gl_nb_files){
        gl_nb_files += 1;
    }
    gl_file_store[fd] = result;
    return Py_BuildValue("i", fd);
}

static PyObject *python_redcryptofile_flush(PyObject* self, PyObject* args)
{
    int fd = 0;
    if (!PyArg_ParseTuple(args, "i", &fd)){
        return NULL;
    }
    if (fd >= gl_nb_files){
        return NULL;
    }
    int result = crypto_flush(gl_file_store[fd]);
    return Py_BuildValue("i", result);
}

static PyObject *python_redcryptofile_close(PyObject* self, PyObject* args)
{
    int fd = 0;
    unsigned char hash[MD_HASH_LENGTH<<1];
    char hash_digest[(MD_HASH_LENGTH*4)+1];

    if (!PyArg_ParseTuple(args, "i", &fd))
        return NULL;

    if (fd >= gl_nb_files){
        return NULL;
    }

    if (!gl_file_store[fd]){
        return NULL;
    }

    int result = crypto_close(gl_file_store[fd], hash, get_cctx()->hmac_key);

    gl_file_store[fd] = NULL;
    if (fd + 1 == gl_nb_files){
        gl_nb_files--;
    }

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
    int buf_len;
    char *buf;

    if (!PyArg_ParseTuple(args, "iS", &fd, &python_buf))
        return NULL;
    buf_len = PyString_Size(python_buf);
    if (buf_len > 2147483647 || buf_len < 0)
        return Py_BuildValue("i", -1);
    buf = PyString_AsString(python_buf);

    if (fd >= gl_nb_files){
        return NULL;
    }
    int result = crypto_write(gl_file_store[fd], buf, buf_len);

    return Py_BuildValue("i", result);
}

static PyObject *python_redcryptofile_read(PyObject* self, PyObject* args)
{
    int fd;
    int buf_len;

    if (!PyArg_ParseTuple(args, "ii", &fd, &buf_len))
        return NULL;
    if (buf_len > 2147483647 || buf_len <= 0)
        return Py_BuildValue("i", -1);


    if (fd >= gl_nb_files){
        return NULL;
    }

//    char buf[buf_len];
    std::unique_ptr<char[]> buf(new char[buf_len]);
//    int result = crypto_read(gl_file_store[fd], buf, buf_len);
    int result = crypto_read(gl_file_store[fd], buf.get(), buf_len);
    if (result >= 0){
//        return PyString_FromStringAndSize(buf, result);
        return PyString_FromStringAndSize(buf.get(), result);
    }
    return Py_BuildValue("i", -1);
}

static PyMethodDef redcryptoFileMethods[] = {
    {"open", python_redcryptofile_open, METH_VARARGS, ""},
    {"flush", python_redcryptofile_flush, METH_VARARGS, ""},
    {"close", python_redcryptofile_close, METH_VARARGS, ""},
    {"write", python_redcryptofile_write, METH_VARARGS, ""},
    {"read", python_redcryptofile_read, METH_VARARGS, ""},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initredcryptofile(void)
{
    Py_InitModule("redcryptofile", redcryptoFileMethods);

    //TODO: this one is default key if shm reading fails
    // Add a way to add this key from outside (for tests)
    memcpy(get_cctx()->crypto_key,
        "\x01\x02\x03\x04\x05\x06\x07\x08"
        "\x01\x02\x03\x04\x05\x06\x07\x08"
        "\x01\x02\x03\x04\x05\x06\x07\x08"
        "\x01\x02\x03\x04\x05\x06\x07\x08",
        CRYPTO_KEY_LENGTH);

    if (-1 == get_crypto_key((char *)get_cctx()->crypto_key)){
        //TODO: we should LOG something here
    }
    if (-1 == derivate_crypto_key((char *)get_cctx()->crypto_key, (char *)get_cctx()->hmac_key)){
        //TODO: we should LOG something here
    }
    OpenSSL_add_all_digests();
}

}
