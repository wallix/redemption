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

#include "main/redcryptofile.hpp"
#include "transport/out_crypto_transport.hpp"
#include "transport/in_crypto_transport.hpp"

#include <memory>

#include "test_only/lcg_random.hpp"

#include "main/version.hpp"

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

#define SCOPED_TRACE Trace trace_l_ {__FUNCTION__}
#define EXIT_ON_EXCEPTION() trace_l_.exit_on_exception()
#define EXIT_ON_ERROR(e) trace_l_.exit_on_error(e)

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

    CryptoContextWrapper(get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn)
    {
        cctx.set_get_hmac_key_cb(hmac_fn);
        cctx.set_get_trace_key_cb(trace_fn);
    }
};

struct RedCryptoErrrorContext
{
    RedCryptoErrrorContext() noexcept
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
    : cctx_wrapper(hmac_fn, trace_fn)
    , random_wrapper(random_type)
    , out_crypto_transport(with_encryption, with_checksum, cctx_wrapper.cctx, *random_wrapper.rnd)
    {}

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

    CryptoContextWrapper cctx_wrapper;
    RandomWrapper random_wrapper;

public:
    HashHexArray qhashhex;
    HashHexArray fhashhex;

    OutCryptoTransport out_crypto_transport;
    RedCryptoErrrorContext error_ctx;
};


struct RedCryptoReaderHandle
{
    RedCryptoReaderHandle(InCryptoTransport::EncryptionMode encryption
                        , get_hmac_key_prototype * hmac_fn
                        , get_trace_key_prototype * trace_fn)
    : cctxw(hmac_fn, trace_fn)
    , in_crypto_transport(cctxw.cctx, encryption)
    {}

private:
    CryptoContextWrapper cctxw;

public:
    InCryptoTransport in_crypto_transport;
    RedCryptoErrrorContext error_ctx;
};


using HashArray = uint8_t[MD_HASH::DIGEST_LENGTH];
static_assert(sizeof(HashArray) * 2 + 1 == sizeof(HashHexArray), "");

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


const char* redcryptofile_version() {
    return VERSION;
}


const char * redcryptofile_writer_qhashhex(RedCryptoWriterHandle * handle) {
    return handle->qhashhex;
}

const char * redcryptofile_writer_fhashhex(RedCryptoWriterHandle * handle) {
    return handle->fhashhex;
}


RedCryptoWriterHandle * redcryptofile_writer_new(int with_encryption
                                               , int with_checksum
                                               , get_hmac_key_prototype * hmac_fn
                                               , get_trace_key_prototype * trace_fn) {
    SCOPED_TRACE;
    CHECK_NOTHROW_R(
        return new (std::nothrow) RedCryptoWriterHandle(
            RedCryptoWriterHandle::LCG /* TODO UDEV */, with_encryption, with_checksum, hmac_fn, trace_fn
        ),
        nullptr,
        RedCryptoErrrorContext(),
        ERR_TRANSPORT
    );
}

int redcryptofile_writer_open(RedCryptoWriterHandle * handle, const char * path) {
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    handle->error_ctx.set_error(Error(NO_ERROR));
    CHECK_NOTHROW(handle->out_crypto_transport.open(path, 0 /* TODO groupid */), ERR_TRANSPORT_OPEN_FAILED);
    return 0;
}


int redcryptofile_writer_write(RedCryptoWriterHandle * handle, uint8_t const * buffer, unsigned long len) {
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    CHECK_NOTHROW(handle->out_crypto_transport.send(buffer, len), ERR_TRANSPORT_WRITE_FAILED);
    return len;
}


int redcryptofile_writer_close(RedCryptoWriterHandle * handle) {
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    HashArray qhash;
    HashArray fhash;
    CHECK_NOTHROW(handle->out_crypto_transport.close(qhash, fhash), ERR_TRANSPORT_CLOSED);
    hash_to_hashhex(qhash, handle->qhashhex);
    hash_to_hashhex(fhash, handle->fhashhex);
    return 0;
}


void redcryptofile_writer_delete(RedCryptoWriterHandle * handle) {
    SCOPED_TRACE;
    delete handle;
}

char const * redcryptofile_writer_error_message(RedCryptoWriterHandle * handle)
{
    return handle ? handle->error_ctx.message() : RedCryptoErrrorContext::handle_error_message();
}


RedCryptoReaderHandle * redcryptofile_reader_new(get_hmac_key_prototype* hmac_fn
                                               , get_trace_key_prototype* trace_fn) {
    SCOPED_TRACE;
    CHECK_NOTHROW_R(
        return new (std::nothrow) RedCryptoReaderHandle(
            InCryptoTransport::EncryptionMode::Auto, hmac_fn, trace_fn
        ),
        nullptr,
        RedCryptoErrrorContext(),
        ERR_TRANSPORT
    );
}

int redcryptofile_reader_open(RedCryptoReaderHandle * handle, char const * path) {
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    handle->error_ctx.set_error(Error(NO_ERROR));
    CHECK_NOTHROW(handle->in_crypto_transport.open(path), ERR_TRANSPORT_OPEN_FAILED);
    return 0;
}


// 0: if end of file, len: if data was read, negative number on error
int redcryptofile_reader_read(RedCryptoReaderHandle * handle, uint8_t * buffer, unsigned long len) {
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    CHECK_NOTHROW(return int(handle->in_crypto_transport.partial_read(buffer, len)), ERR_TRANSPORT_READ_FAILED);
}

int redcryptofile_reader_close(RedCryptoReaderHandle * handle) {
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    CHECK_NOTHROW(handle->in_crypto_transport.close(), ERR_TRANSPORT_CLOSED);
    return 0;
}

void redcryptofile_reader_delete(RedCryptoReaderHandle * handle) {
    SCOPED_TRACE;
    delete handle;
}

char const * redcryptofile_reader_error_message(RedCryptoReaderHandle * handle)
{
    return handle ? handle->error_ctx.message() : RedCryptoErrrorContext::handle_error_message();
}

}
