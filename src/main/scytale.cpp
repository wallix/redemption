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
   Author(s): Christophe Grosjean, Jonathan Poelen

*/

#include "main/scytale.hpp"
#include "transport/crypto_transport.hpp"

#include <memory>

#include "test_only/lcg_random.hpp"
#include "utils/fileutils.hpp"
#include "utils/genfstat.hpp"

#include "main/version.hpp"


#ifdef SHOW_DEBUG_TRACES

namespace
{
    struct Trace
    {
        Trace(char const * func_name) noexcept
        : func_name(func_name)
        {
            LOG(LOG_INFO, "%s()", func_name);
        }

        ~Trace()
        {
            LOG(LOG_INFO, "%s() done", func_name);
        }

        void exit_on_exception() noexcept
        {
            LOG(LOG_ERR, "%s() exit with exception", func_name);
        }

        void exit_on_error(Error const & e) noexcept
        {
            LOG(LOG_ERR, "%s() exit with exception Error: %s", func_name, e.errmsg());
        }

    private:
        char const * func_name;
};
}

#define SCOPED_TRACE Trace trace_l_ {__FUNCTION__}
#define EXIT_ON_EXCEPTION() trace_l_.exit_on_exception()
#define EXIT_ON_ERROR(e) trace_l_.exit_on_error(e)

#else

#define SCOPED_TRACE
#define EXIT_ON_EXCEPTION()
#define EXIT_ON_ERROR(e)

#endif

#define CHECK_HANDLE(handle) if (!handle) return -1

#define CHECK_NOTHROW_R(expr, return_err, error_ctx, errid) \
    do {                                                    \
        try { expr; }                                       \
        catch (Error const& err) {                          \
            EXIT_ON_ERROR(err);                             \
            error_ctx.set_error(err);                       \
            return return_err;                              \
        }                                                   \
        catch (...) {                                       \
            EXIT_ON_EXCEPTION();                            \
            error_ctx.set_error(Error{errid});              \
            return return_err;                              \
        }                                                   \
    } while (0)

#ifdef IN_IDE_PARSER
# define CHECK_NOTHROW(expr, errid) expr; errid
#else
# define CHECK_NOTHROW(expr, errid) CHECK_NOTHROW_R(expr, -1, handle->error_ctx, errid)
#endif


extern "C"
{

struct CryptoContextWrapper
{
    CryptoContext cctx;

    CryptoContextWrapper(get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn, bool with_encryption, bool with_checksum, int old_encryption_scheme = false, int one_shot_encryption_scheme = false)
    {
        cctx.set_get_hmac_key_cb(hmac_fn);
        cctx.set_get_trace_key_cb(trace_fn);
        cctx.set_trace_type(
            with_encryption
            ? TraceType::cryptofile
            : with_checksum
                ? TraceType::localfile_hashed
                : TraceType::localfile);
        cctx.old_encryption_scheme = old_encryption_scheme;
        cctx.one_shot_encryption_scheme = one_shot_encryption_scheme;
    }

    void set_master_derivator_from_file_name(char const * filename)
    {
        size_t base_len = 0;
        char const * base = basename_len(filename, base_len);
        cctx.set_master_derivator({base, base_len});
    }
};

struct RedCryptoErrorContext
{
    RedCryptoErrorContext() noexcept
    : error(NO_ERROR)
    , msg{}
    {}

    char const * message() noexcept
    {
        if (this->error.errnum && this->error.id != NO_ERROR) {
            std::snprintf(this->msg, sizeof(msg), "%s, errno = %d", this->error.errmsg(), this->error.errnum);
        }
        else {
            std::snprintf(this->msg, sizeof(msg), "%s", this->error.errmsg());
        }
        this->msg[sizeof(this->msg)-1] = 0;
        return this->msg;
    }

    static char const * handle_error_message() noexcept
    {
        return "Handle is nullptr";
    }

    void set_error(Error const & err) noexcept
    {
        this->error = err;
    }

private:
    Error error;
    char msg[128];
};

struct RedCryptoWriterHandle
{
    enum RandomType { LCG, UDEV };

    RedCryptoWriterHandle(
        RandomType random_type,
        bool with_encryption, bool with_checksum,
        get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn)
    : random_wrapper(random_type)
    , cctxw(hmac_fn, trace_fn, with_encryption, with_checksum)
    , out_crypto_transport(cctxw.cctx, *random_wrapper.rnd, fstat)
    {

        memset(this->qhashhex, '0', sizeof(this->qhashhex)-1);
        this->qhashhex[sizeof(this->qhashhex)-1] = 0;
        memset(this->fhashhex, '0', sizeof(this->fhashhex)-1);
        this->fhashhex[sizeof(this->fhashhex)-1] = 0;
    }

private:
    struct RandomWrapper
    {
        Random * rnd;

        RandomWrapper(RandomType rnd_type)
        {
            switch (rnd_type) {
                case LCG: rnd = new (&u.lcg) LCGRandom(0); break;
                case UDEV: rnd = new (&u.udev) UdevRandom(); break;
            }
        }

        ~RandomWrapper()
        {
            rnd->~Random();
        }

    private:
        union U {
            LCGRandom lcg; /* for reproductible tests */
            UdevRandom udev;
            char dummy;
            U() : dummy() {}
            ~U() {}
        } u;
    };

    RandomWrapper random_wrapper;
    Fstat fstat;

public:
    CryptoContextWrapper cctxw;

    HashHexArray qhashhex;
    HashHexArray fhashhex;

    OutCryptoTransport out_crypto_transport;
    RedCryptoErrorContext error_ctx;
};


struct RedCryptoReaderHandle
{
    HashHexArray qhashhex;
    HashHexArray fhashhex;

    RedCryptoReaderHandle(InCryptoTransport::EncryptionMode encryption
                        , get_hmac_key_prototype * hmac_fn
                        , get_trace_key_prototype * trace_fn
                        , int old_encryption_scheme = 0
                        , int one_shot_encryption_scheme = 0
                        )
    : cctxw(hmac_fn, trace_fn, 
            false /* unused for reading */, false /* unused for reading */,
            old_encryption_scheme, one_shot_encryption_scheme)
    , in_crypto_transport(cctxw.cctx, encryption, this->fstat)
    {
        memset(this->qhashhex, '0', sizeof(this->qhashhex)-1);
        this->qhashhex[sizeof(this->qhashhex)-1] = 0;
        memset(this->fhashhex, '0', sizeof(this->fhashhex)-1);
        this->fhashhex[sizeof(this->fhashhex)-1] = 0;
    }

    CryptoContextWrapper cctxw;
    Fstat fstat;

    InCryptoTransport in_crypto_transport;
    RedCryptoErrorContext error_ctx;
};


using HashArray = uint8_t[MD_HASH::DIGEST_LENGTH];
static_assert(sizeof(HashArray) * 2 + 1 == sizeof(HashHexArray), "");

namespace
{
    inline void hash_to_hashhex(HashArray const & hash, HashHexArray hashhex) noexcept {
        char const * t = "0123456789ABCDEF";
        static_assert(sizeof(hash) * 2 + 1 == sizeof(HashHexArray), "");
        auto phex = hashhex;
        for (uint8_t c : hash) {
            *phex++ = t[c >> 4];
            *phex++ = t[c & 0xf];
        }
        *phex = '\0';
    }
    
    inline void hashex_to_hash(HashHexArray const & hashhex, HashArray hash) noexcept {
        // Undefined Behavior if hashhex is not a valid input hex key
        static_assert(sizeof(HashArray) * 2 + 1 == sizeof(HashHexArray), "");
        auto phex = hash;
        for (size_t i = 0 ; i < sizeof(hashhex) - 1 ; i += 2) {
            auto c1 = hashhex[i];
            auto c2 = hashhex[i+1];
            *phex++ = ((0xF & (c1 < 'A'? c1 - '0' : c1 < 'a' ? c1 - 'A' : c1 - 'a')) << 4)
                    |  (0xF & (c2 < 'A'? c2 - '0' : c2 < 'a' ? c2 - 'A' : c2 - 'a'));
        }
    }

}


const char* scytale_version() {
    return VERSION;
}


/// \return HashHexArray
char const * scytale_writer_qhashhex(RedCryptoWriterHandle * handle) {
    SCOPED_TRACE;
    return handle->qhashhex;
}

/// \return HashHexArray
char const * scytale_writer_fhashhex(RedCryptoWriterHandle * handle) {
    SCOPED_TRACE;
    return handle->fhashhex;
}


RedCryptoWriterHandle * scytale_writer_new(int with_encryption
                                               , int with_checksum
                                               , const char * derivator
                                               , get_hmac_key_prototype * hmac_fn
                                               , get_trace_key_prototype * trace_fn) {
    SCOPED_TRACE;
    CHECK_NOTHROW_R(
        auto handle = new (std::nothrow) RedCryptoWriterHandle(
            RedCryptoWriterHandle::UDEV, with_encryption, with_checksum, hmac_fn, trace_fn
        );
        handle->cctxw.set_master_derivator_from_file_name(derivator);
        return handle,
        nullptr,
        RedCryptoErrorContext(),
        ERR_TRANSPORT
    );
}


RedCryptoWriterHandle * scytale_writer_new_with_test_random(
    int with_encryption, int with_checksum, const char * derivator,
    get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn
) {
    SCOPED_TRACE;
    CHECK_NOTHROW_R(
        auto handle = new (std::nothrow) RedCryptoWriterHandle(
            RedCryptoWriterHandle::LCG, with_encryption, with_checksum, hmac_fn, trace_fn
        );
        handle->cctxw.set_master_derivator_from_file_name(derivator);
        return handle,
        nullptr,
        RedCryptoErrorContext(),
        ERR_TRANSPORT
    );
}

int scytale_writer_open(RedCryptoWriterHandle * handle, const char * path, char const * hashpath, int groupid) {
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    handle->error_ctx.set_error(Error(NO_ERROR));
    CHECK_NOTHROW(handle->out_crypto_transport.open(path, hashpath, groupid/*, TODO derivator*/), ERR_TRANSPORT_OPEN_FAILED);
    return 0;
}


int scytale_writer_write(RedCryptoWriterHandle * handle, uint8_t const * buffer, unsigned long len) {
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    CHECK_NOTHROW(handle->out_crypto_transport.send(buffer, len), ERR_TRANSPORT_WRITE_FAILED);
    return len;
}


int scytale_writer_close(RedCryptoWriterHandle * handle) {
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    HashArray qhash;
    HashArray fhash;
    CHECK_NOTHROW(handle->out_crypto_transport.close(qhash, fhash), ERR_TRANSPORT_CLOSED);
    hash_to_hashhex(qhash, handle->qhashhex);
    hash_to_hashhex(fhash, handle->fhashhex);
    return 0;
}


void scytale_writer_delete(RedCryptoWriterHandle * handle) {
    SCOPED_TRACE;
    delete handle;
}

char const * scytale_writer_error_message(RedCryptoWriterHandle * handle)
{
    return handle ? handle->error_ctx.message() : RedCryptoErrorContext::handle_error_message();
}


struct RedCryptoKeyHandle
{
    HashHexArray masterhex;
    HashArray master;
    HashHexArray derivatedhex;
    HashArray derivated;
    
    RedCryptoKeyHandle(const char * masterkeyhex)
    {
        memcpy(this->masterhex, masterkeyhex, sizeof(this->masterhex));
        hashex_to_hash(this->masterhex, this->master);
    }
};


RedCryptoKeyHandle * scytale_key_new(const char * masterkeyhex)
{
    constexpr auto key_len = sizeof(HashHexArray) - 1;
    if (!masterkeyhex || strlen(masterkeyhex) != key_len){
        return nullptr;
    }
    for (char c : make_array_view(masterkeyhex, key_len)){
        if (not ((c >= '0' and c <= '9') or (c >= 'A' and c <= 'F') or (c >= 'a' and c <= 'f'))){
            return nullptr;
        }
    }
    auto handle = new (std::nothrow) RedCryptoKeyHandle(masterkeyhex);
    return handle;
}


const char * scytale_key_derivate(RedCryptoKeyHandle * handle, const uint8_t * derivator, size_t len)
{
    std::unique_ptr<uint8_t[]> normalized_derivator_gc;
    auto const new_derivator = CryptoContext::get_normalized_derivator(
        normalized_derivator_gc, {derivator, len});

    CryptoContext cctx;
    cctx.old_encryption_scheme = false;
    cctx.one_shot_encryption_scheme = false;
    cctx.set_master_key(handle->master);
    cctx.get_derived_key(handle->derivated, new_derivator);
    hash_to_hashhex(handle->derivated, handle->derivatedhex);
    return handle->derivatedhex;
}


void scytale_key_delete(RedCryptoKeyHandle * handle) {
    SCOPED_TRACE;
    delete handle;
}


const char * scytale_key_master(RedCryptoKeyHandle * handle) {
    SCOPED_TRACE;
    return handle->masterhex;
}

const char * scytale_key_derivated(RedCryptoKeyHandle * handle) {
    SCOPED_TRACE;
    return handle->derivatedhex;
}


RedCryptoReaderHandle * scytale_reader_new(const char * derivator
                                                , get_hmac_key_prototype* hmac_fn
                                                , get_trace_key_prototype* trace_fn
                                                , int old_scheme
                                                , int one_shot)
{
    SCOPED_TRACE;
    CHECK_NOTHROW_R(
        auto handle = new (std::nothrow) RedCryptoReaderHandle(
            InCryptoTransport::EncryptionMode::Auto, hmac_fn, trace_fn, old_scheme, one_shot
        );
        handle->cctxw.set_master_derivator_from_file_name(derivator);
        return handle,
        nullptr,
        RedCryptoErrorContext(),
        ERR_TRANSPORT
    );
    ;
}

int scytale_reader_open(RedCryptoReaderHandle * handle, char const * path, char const * derivator) {
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    handle->error_ctx.set_error(Error(NO_ERROR));
    CHECK_NOTHROW(
        handle->in_crypto_transport.open(path, {derivator, strlen(derivator)}),
        ERR_TRANSPORT_OPEN_FAILED);
    return 0;
}


// 0: if end of file, len: if data was read, negative number on error
int scytale_reader_read(RedCryptoReaderHandle * handle, uint8_t * buffer, unsigned long len) {
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    CHECK_NOTHROW(return int(handle->in_crypto_transport.partial_read(buffer, len)), ERR_TRANSPORT_READ_FAILED);
}


int scytale_reader_close(RedCryptoReaderHandle * handle) {
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    CHECK_NOTHROW(handle->in_crypto_transport.close(), ERR_TRANSPORT_CLOSED);
    return 0;
}

void scytale_reader_delete(RedCryptoReaderHandle * handle) {
    SCOPED_TRACE;
    delete handle;
}

char const * scytale_reader_error_message(RedCryptoReaderHandle * handle)
{
    return handle ? handle->error_ctx.message() : RedCryptoErrorContext::handle_error_message();
}

int scytale_reader_fhash(RedCryptoReaderHandle * handle, const char * file) {
    SCOPED_TRACE;
    try {
        InCryptoTransport::HASH fhash = handle->in_crypto_transport.fhash(file);
        hash_to_hashhex(fhash.hash, handle->fhashhex);
    }
    catch (Error const& err) {
        EXIT_ON_ERROR(err);
        handle->error_ctx.set_error(err);
        return -1;
    }
    catch (...) {
        EXIT_ON_EXCEPTION();
        handle->error_ctx.set_error(Error{ERR_TRANSPORT_READ_FAILED});
        return -1;
    }
    return 0;
}

int scytale_reader_qhash(RedCryptoReaderHandle * handle, const char * file) {
    SCOPED_TRACE;
    try {
        InCryptoTransport::HASH qhash = handle->in_crypto_transport.qhash(file);
        hash_to_hashhex(qhash.hash, handle->qhashhex);
    }
    catch (Error const& err) {
        EXIT_ON_ERROR(err);
        handle->error_ctx.set_error(err);
        return -1;
    }
    catch (...) {
        EXIT_ON_EXCEPTION();
        handle->error_ctx.set_error(Error{ERR_TRANSPORT_READ_FAILED});
        return -1;
    }
    return 0;
}


const char * scytale_reader_qhashhex(RedCryptoReaderHandle * handle) {
    SCOPED_TRACE;
    return handle->qhashhex;
}

const char * scytale_reader_fhashhex(RedCryptoReaderHandle * handle) {
    SCOPED_TRACE;
    return handle->fhashhex;
}

}
