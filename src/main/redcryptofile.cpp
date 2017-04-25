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

#include "../tests/includes/test_only/lcg_random.hpp" // TODO

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
                case LCG:
                    new (&u.lcg) LCGRandom(0);
                    rnd = &u.lcg;
                    break;
                case UDEV:
                    new (&u.udev) UdevRandom();
                    rnd = &u.udev;
                    break;
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
};


struct RedCryptoReaderHandle
{
    RedCryptoReaderHandle(
        InCryptoTransport::EncryptionMode encryption,
        get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn)
    : cctxw(hmac_fn, trace_fn)
    , in_crypto_transport(cctxw.cctx, encryption)
    {}

private:
    CryptoContextWrapper cctxw;

public:
    InCryptoTransport in_crypto_transport;
};


RedCryptoWriterHandle * redcryptofile_new_writer(int with_encryption, int with_checksum, get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn)
{
    LOG(LOG_INFO, "redcryptofile_new_writer()");
    try {
        auto handler = new (std::nothrow) RedCryptoWriterHandle(
            RedCryptoWriterHandle::LCG /* TODO UDEV */, with_encryption, with_checksum, hmac_fn, trace_fn
        );
        std::unique_ptr<RedCryptoWriterHandle> u(handler);
        LOG(LOG_INFO, "redcryptofile_new_writer() -> exit");
        return u.release();
    }
    catch (...) {
        LOG(LOG_INFO, "redcryptofile_new_writer() -> exit exception");
        return nullptr;
    }
}


#define CHECK_HANDLE(handle) if (!handle) return -1
#define CHECK_NOTHROW(exp) do { try { exp; } catch (...) { return -1; } } while (0)

int redcryptofile_open_writer(RedCryptoWriterHandle * handle, const char * path)
{
    LOG(LOG_INFO, "redcryptofile_open_writer()");
    CHECK_HANDLE(handle);
    CHECK_NOTHROW(handle->out_crypto_transport.open(path, 0 /* TODO groupid */));
    return 0;
}

RedCryptoReaderHandle * redcryptofile_open_reader(
    char const * path,
    get_hmac_key_prototype* hmac_fn, get_trace_key_prototype* trace_fn)
{
    try {
        auto handler = new (std::nothrow) RedCryptoReaderHandle(
            InCryptoTransport::EncryptionMode::Auto, hmac_fn, trace_fn
        );
        std::unique_ptr<RedCryptoReaderHandle> u(handler);
        handler->in_crypto_transport.open(path);
        return u.release();
    }
    catch (...) {
        return nullptr;
    }
}

int redcryptofile_write(RedCryptoWriterHandle * handle, uint8_t const * buffer, unsigned long len)
{
    LOG(LOG_INFO, "redcryptofile_write()");
    CHECK_HANDLE(handle);
    try {
        handle->out_crypto_transport.send(buffer, len);
    }
    catch (...)
    {
        return -1;
    }
    LOG(LOG_INFO, "redcryptofile_write() done");
    return len;
}

// 0: if end of file, len: if data was read, negative number on error
int redcryptofile_read(RedCryptoReaderHandle * handle, uint8_t * buffer, unsigned long len)
{
    CHECK_HANDLE(handle);
    try {
        return handle->in_crypto_transport.partial_read(buffer, len);
    }
    catch (Error e) {
        return -e.id;
    }
    catch (...) {
        return -1;
    }
}

using HashArray = uint8_t[MD_HASH::DIGEST_LENGTH];

inline void hash_to_hashhex(HashArray const & hash, HashHexArray hashhex) noexcept
{
    char const * t = "0123456789ABCDEF";
    static_assert(sizeof(hash) * 2 + 1 == sizeof(HashHexArray), "");
    auto phex = hashhex;
    for (uint8_t c : hash) {
        *phex++ = t[c >> 4];
        *phex++ = t[c & 0xf];
    }
    *phex = '\0';
}


const char * redcryptofile_qhashhex_writer(RedCryptoWriterHandle * handle)
{
    return handle->qhashhex;
}

const char * redcryptofile_fhashhex_writer(RedCryptoWriterHandle * handle)
{
    return handle->fhashhex;
}

int redcryptofile_close_writer(RedCryptoWriterHandle * handle)
{
    LOG(LOG_INFO, "redcryptofile_close_writer()");
    CHECK_HANDLE(handle);
    std::unique_ptr<RedCryptoWriterHandle> u(handle);
    HashArray qhash;
    HashArray fhash;
    CHECK_NOTHROW(handle->out_crypto_transport.close(qhash, fhash));
    if (handle) {
        hash_to_hashhex(qhash, handle->qhashhex);
    }
    if (handle) {
        hash_to_hashhex(fhash, handle->fhashhex);
    }
    LOG(LOG_INFO, "redcryptofile_close_writer() done");
    return 0;
}

int redcryptofile_close_reader(RedCryptoReaderHandle * handle)
{
    CHECK_HANDLE(handle);
    std::unique_ptr<RedCryptoReaderHandle> u(handle);
    CHECK_NOTHROW(handle->in_crypto_transport.close());
    return 0;
}

}
