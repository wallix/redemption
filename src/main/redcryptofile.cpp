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
    OutCryptoTransport out_crypto_transport;
};


struct RedCryptoReaderHandle
{
    RedCryptoReaderHandle(
        bool with_encryption,
        get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn)
    : cctxw(hmac_fn, trace_fn)
    , in_crypto_transport(cctxw.cctx, with_encryption)
    {}

private:
    CryptoContextWrapper cctxw;

public:
    InCryptoTransport in_crypto_transport;
};


RedCryptoWriterHandle * redcryptofile_open_writer(
    int with_encryption, int with_checksum, const char * path,
    get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn)
{
    try {
        auto handler = new (std::nothrow) RedCryptoWriterHandle(
            RedCryptoWriterHandle::LCG /* TODO UDEV */, with_encryption, with_checksum, hmac_fn, trace_fn
        );
        std::unique_ptr<RedCryptoWriterHandle> u(handler);
        handler->out_crypto_transport.open(path, 0 /* TODO groupid */);
        return u.release();
    }
    catch (...) {
        return nullptr;
    }
}

#define CHECK_HANDLE(handle) if (!handle) return -1
#define CHECK_NOTHROW(exp) do { try { exp; } catch (...) { return -1; } } while (0)

long redcryptofile_write(RedCryptoWriterHandle * handle, uint8_t const * buffer, unsigned long len)
{
    CHECK_HANDLE(handle);
    CHECK_NOTHROW(handle->out_crypto_transport.send(buffer, len));
    return static_cast<long>(len);
}

using HashArray = uint8_t[MD_HASH::DIGEST_LENGTH];

inline void hash_to_hashhex(HashArray const & hash, HashHexArray hashhex) noexcept
{
    static_assert(sizeof(hash) * 2 + 1 == sizeof(HashHexArray), "");
    auto phex = hashhex;
    for (uint8_t c : hash) {
        char const * t = "0123456789ABCDEF";
        *phex++ = t[int(c >> 4)];
        *phex++ = t[int(c & 0xf)];
    }
    *phex = '\0';
}

long int redcryptofile_close_writer(RedCryptoWriterHandle* handle, HashHexArray qhashhex, HashHexArray fhashhex)
{
    CHECK_HANDLE(handle);
    int const ret = [&]{
        HashArray qhash;
        HashArray fhash;
        CHECK_NOTHROW(handle->out_crypto_transport.close(qhash, fhash));
        hash_to_hashhex(qhash, qhashhex);
        hash_to_hashhex(fhash, fhashhex);
        return 0;
    }();
    delete handle;
    return ret;
}

}
