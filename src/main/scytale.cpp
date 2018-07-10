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
#include "transport/mwrm_reader.hpp"

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
} // namespace


#define SCOPED_TRACE Trace trace_l_ {__FUNCTION__}
#define EXIT_ON_EXCEPTION() trace_l_.exit_on_exception()
#define EXIT_ON_ERROR(e) trace_l_.exit_on_error(e)

#else

#define SCOPED_TRACE
#define EXIT_ON_EXCEPTION()
#define EXIT_ON_ERROR(e)

#endif

#define CHECK_HANDLE(handle) if (!handle) return -1
#define CHECK_HANDLE_R(handle, return_err) if (!handle) return return_err

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

#define CHECK_NOTHROW(expr, errid) CHECK_NOTHROW_R(expr, -1, handle->error_ctx, errid)
#define CREATE_HANDLE(construct) [&]()->decltype(new construct){ \
    CHECK_NOTHROW_R(                                             \
        auto handle = new (std::nothrow) construct;              \
        return handle,                                           \
        nullptr,                                                 \
        RedCryptoErrorContext(),                                 \
        ERR_MEMORY_ALLOCATION_FAILED                             \
    );                                                           \
}()


extern "C"
{

struct CryptoContextWrapper
{
    CryptoContext cctx;

    CryptoContextWrapper(
        get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn,
        bool with_encryption, bool with_checksum,
        bool old_encryption_scheme, bool one_shot_encryption_scheme,
        char const * filename_derivator)
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

        size_t base_len = 0;
        char const * base = basename_len(filename_derivator, base_len);
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
        if (this->error.errnum) {
            std::snprintf(this->msg, sizeof(msg), "%s, errno = %d: %s", this->error.errmsg(), this->error.errnum, strerror(this->error.errnum));
            this->msg[sizeof(this->msg)-1] = 0;
            return this->msg;
        }

        return this->error.errmsg();
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
    char msg[256];
};

struct RedCryptoWriterHandle
{
    enum RandomType { LCG, UDEV };

    RedCryptoWriterHandle(
        RandomType random_type,
        bool with_encryption, bool with_checksum,
        get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn,
        int old_encryption_scheme , int one_shot_encryption_scheme,
        char const * filename_derivator)
    : random_wrapper(random_type)
    , cctxw(hmac_fn, trace_fn, with_encryption, with_checksum,
            old_encryption_scheme, one_shot_encryption_scheme,
            filename_derivator)
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

    RedCryptoReaderHandle(
        InCryptoTransport::EncryptionMode encryption,
        get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn,
        int old_encryption_scheme, int one_shot_encryption_scheme,
        char const * filename_derivator)
    : cctxw(hmac_fn, trace_fn,
            false /* unused for reading */, false /* unused for reading */,
            old_encryption_scheme, one_shot_encryption_scheme,
            filename_derivator)
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
static_assert(sizeof(HashArray) * 2 + 1 == sizeof(HashHexArray));

namespace
{
    inline void hash_to_hashhex(HashArray const & hash, HashHexArray hashhex) noexcept {
        char const * t = "0123456789ABCDEF";
        static_assert(sizeof(hash) * 2 + 1 == sizeof(HashHexArray));
        auto phex = hashhex;
        for (uint8_t c : hash) {
            *phex++ = t[c >> 4];
            *phex++ = t[c & 0xf];
        }
        *phex = '\0';
    }

    inline void hashex_to_hash(HashHexArray const & hashhex, HashArray hash) noexcept {
        // Undefined Behavior if hashhex is not a valid input hex key
        static_assert(sizeof(HashArray) * 2 + 1 == sizeof(HashHexArray));
        auto phex = hash;
        for (size_t i = 0 ; i < sizeof(hashhex) - 1 ; i += 2) {
            auto c1 = hashhex[i];
            auto c2 = hashhex[i+1];
            *phex++ = ((0xF & (c1 < 'A'? c1 - '0' : c1 < 'a' ? c1 - 'A' : c1 - 'a')) << 4)
                    |  (0xF & (c2 < 'A'? c2 - '0' : c2 < 'a' ? c2 - 'A' : c2 - 'a'));
        }
    }
} // namespace



const char* scytale_version() {
    return VERSION;
}


/// \return HashHexArray
char const * scytale_writer_qhashhex(RedCryptoWriterHandle * handle) {
    SCOPED_TRACE;
    CHECK_HANDLE_R(handle, "");
    return handle->qhashhex;
}

/// \return HashHexArray
char const * scytale_writer_fhashhex(RedCryptoWriterHandle * handle) {
    SCOPED_TRACE;
    CHECK_HANDLE_R(handle, "");
    return handle->fhashhex;
}


RedCryptoWriterHandle * scytale_writer_new(
    int with_encryption, int with_checksum, const char * derivator,
    get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn,
    int old_scheme, int one_shot)
{
    SCOPED_TRACE;
    return CREATE_HANDLE(RedCryptoWriterHandle(
        RedCryptoWriterHandle::UDEV,
        with_encryption, with_checksum,
        hmac_fn, trace_fn,
        old_scheme, one_shot,
        derivator
    ));
}


RedCryptoWriterHandle * scytale_writer_new_with_test_random(
    int with_encryption, int with_checksum, const char * derivator,
    get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn,
    int old_scheme, int one_shot)
{
    SCOPED_TRACE;
    return CREATE_HANDLE(RedCryptoWriterHandle(
        RedCryptoWriterHandle::LCG,
        with_encryption, with_checksum,
        hmac_fn, trace_fn,
        old_scheme, one_shot,
        derivator
    ));
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


void scytale_writer_delete(RedCryptoWriterHandle * handle)
{
    SCOPED_TRACE;
    delete handle;
}

char const * scytale_writer_error_message(RedCryptoWriterHandle * handle)
{
    SCOPED_TRACE;
    return handle ? handle->error_ctx.message() : RedCryptoErrorContext::handle_error_message();
}



RedCryptoReaderHandle * scytale_reader_new(
    const char * derivator,
    get_hmac_key_prototype* hmac_fn, get_trace_key_prototype* trace_fn,
    int old_scheme, int one_shot)
{
    SCOPED_TRACE;
    return CREATE_HANDLE(RedCryptoReaderHandle(
        InCryptoTransport::EncryptionMode::Auto,
        hmac_fn, trace_fn,
        old_scheme, one_shot,
        derivator
    ));
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

int scytale_reader_open_with_auto_detect_encryption_scheme(
    RedCryptoReaderHandle * handle, char const * path, char const * derivator)
{
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    handle->error_ctx.set_error(Error(NO_ERROR));
    const_byte_array const derivator_array{derivator, strlen(derivator)};
    Error out_error{NO_ERROR};
    CHECK_NOTHROW(
        auto const r = open_if_possible_and_get_encryption_scheme_type(
            handle->in_crypto_transport, path, derivator_array, &out_error);
        switch (r)
        {
            case EncryptionSchemeTypeResult::Error:
                handle->error_ctx.set_error(out_error);
                break;
            case EncryptionSchemeTypeResult::OldScheme:
                // repopen file because some data are lost
                handle->cctxw.cctx.old_encryption_scheme = 1;
                handle->in_crypto_transport.open(path, derivator_array);
                break;
            case EncryptionSchemeTypeResult::NewScheme:
            case EncryptionSchemeTypeResult::NoEncrypted:
                break;
        }
        return int(r),
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

int scytale_reader_fhash(RedCryptoReaderHandle * handle, const char * file)
{
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    CHECK_NOTHROW(
        InCryptoTransport::HASH fhash = handle->in_crypto_transport.fhash(file);
        hash_to_hashhex(fhash.hash, handle->fhashhex),
        ERR_TRANSPORT_READ_FAILED
    );
    return 0;
}

int scytale_reader_qhash(RedCryptoReaderHandle * handle, const char * file)
{
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    CHECK_NOTHROW(
        InCryptoTransport::HASH qhash = handle->in_crypto_transport.qhash(file);
        hash_to_hashhex(qhash.hash, handle->qhashhex),
        ERR_TRANSPORT_READ_FAILED
    );
    return 0;
}


const char * scytale_reader_qhashhex(RedCryptoReaderHandle * handle)
{
    SCOPED_TRACE;
    CHECK_HANDLE_R(handle, nullptr);
    return handle->qhashhex;
}

const char * scytale_reader_fhashhex(RedCryptoReaderHandle * handle)
{
    SCOPED_TRACE;
    CHECK_HANDLE_R(handle, nullptr);
    return handle->fhashhex;
}


struct RedCryptoMetaReaderHandle
{
    explicit RedCryptoMetaReaderHandle(RedCryptoReaderHandle & reader)
      : mwrm_reader(reader.in_crypto_transport)
    {}

    RedCryptoMwrmHeader & get_header() noexcept
    {
        auto const & header = this->mwrm_reader.get_header();
        this->c_header.version = static_cast<int>(header.version);
        this->c_header.has_checksum = header.has_checksum;
        return this->c_header;
    }

    RedCryptoMwrmLine & get_meta_line() noexcept
    {
        if (this->meta_line.with_hash) {
            hash_to_hashhex(this->meta_line.hash1, this->hashhex1);
            hash_to_hashhex(this->meta_line.hash2, this->hashhex2);
        }
        else {
            this->hashhex1[0] = 0;
            this->hashhex2[0] = 0;
        }
        this->c_mwrm_line = {
            this->meta_line.filename,
            static_cast<uint64_t>(this->meta_line.size),
            this->meta_line.mode,
            this->meta_line.uid,
            this->meta_line.gid,
            this->meta_line.dev,
            this->meta_line.ino,
            static_cast<uint64_t>(this->meta_line.mtime),
            static_cast<uint64_t>(this->meta_line.ctime),
            static_cast<uint64_t>(this->meta_line.start_time),
            static_cast<uint64_t>(this->meta_line.stop_time),
            this->meta_line.with_hash,
            this->hashhex1,
            this->hashhex2,
        };
        return this->c_mwrm_line;
    }

private:
    HashHexArray hashhex1;
    HashHexArray hashhex2;

    RedCryptoMwrmHeader c_header;
    RedCryptoMwrmLine c_mwrm_line;

public:
    MwrmReader mwrm_reader;
    MetaLine meta_line;
    bool eof = false;

    RedCryptoErrorContext error_ctx;
};


RedCryptoMetaReaderHandle * scytale_meta_reader_new(RedCryptoReaderHandle * reader)
{
    SCOPED_TRACE;
    CHECK_HANDLE_R(reader, nullptr);
    return CREATE_HANDLE(RedCryptoMetaReaderHandle(*reader));
}

char const * scytale_meta_reader_message(RedCryptoMetaReaderHandle * handle)
{
    SCOPED_TRACE;
    return handle ? handle->error_ctx.message() : RedCryptoErrorContext::handle_error_message();
}

int scytale_meta_reader_read_hash(RedCryptoMetaReaderHandle * handle, int version, int has_checksum)
{
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    handle->mwrm_reader.set_header({static_cast<WrmVersion>(version), bool(has_checksum)});
    CHECK_NOTHROW(
        handle->mwrm_reader.read_meta_hash_line(handle->meta_line),
        ERR_TRANSPORT_READ_FAILED);
    return 0;
}

int scytale_meta_reader_read_header(RedCryptoMetaReaderHandle * handle)
{
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    CHECK_NOTHROW(
        handle->mwrm_reader.read_meta_headers(),
        ERR_TRANSPORT_READ_FAILED);
    return 0;
}

int scytale_meta_reader_read_line(RedCryptoMetaReaderHandle * handle)
{
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    CHECK_NOTHROW(
        handle->eof
            = Transport::Read::Eof == handle->mwrm_reader.read_meta_line(handle->meta_line),
        ERR_TRANSPORT_READ_FAILED);
    return handle->eof ? ERR_TRANSPORT_NO_MORE_DATA : 0;
}

int scytale_meta_reader_read_line_eof(RedCryptoMetaReaderHandle * handle)
{
    SCOPED_TRACE;
    CHECK_HANDLE(handle);
    return handle->eof;
}

void scytale_meta_reader_delete(RedCryptoMetaReaderHandle * handle)
{
    SCOPED_TRACE;
    delete handle;
}

RedCryptoMwrmHeader * scytale_meta_reader_get_header(RedCryptoMetaReaderHandle * handle)
{
    SCOPED_TRACE;
    CHECK_HANDLE_R(handle, nullptr);
    return &handle->get_header();
}

RedCryptoMwrmLine * scytale_meta_reader_get_line(RedCryptoMetaReaderHandle * handle)
{
    SCOPED_TRACE;
    CHECK_HANDLE_R(handle, nullptr);
    return &handle->get_meta_line();
}


struct RedCryptoKeyHandle
{
    HashHexArray masterhex;
    HashArray master;
    HashHexArray derivatedhex;
    HashArray derivated;

    explicit RedCryptoKeyHandle(const char * masterkeyhex)
    {
        memcpy(this->masterhex, masterkeyhex, sizeof(this->masterhex));
        hashex_to_hash(this->masterhex, this->master);
    }
};


RedCryptoKeyHandle * scytale_key_new(const char * masterkeyhex)
{
    SCOPED_TRACE;
    constexpr auto key_len = sizeof(HashHexArray) - 1;
    if (!masterkeyhex || strlen(masterkeyhex) != key_len){
        return nullptr;
    }
    for (char c : make_array_view(masterkeyhex, key_len)){
        if (not ((c >= '0' and c <= '9') or (c >= 'A' and c <= 'F') or (c >= 'a' and c <= 'f'))){
            return nullptr;
        }
    }
    return CREATE_HANDLE(RedCryptoKeyHandle(masterkeyhex));
}


const char * scytale_key_derivate(RedCryptoKeyHandle * handle, const uint8_t * derivator, size_t len)
{
    SCOPED_TRACE;
    CHECK_HANDLE_R(handle, "");

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
    CHECK_HANDLE_R(handle, "");
    return handle->masterhex;
}

const char * scytale_key_derivated(RedCryptoKeyHandle * handle) {
    SCOPED_TRACE;
    CHECK_HANDLE_R(handle, "");
    return handle->derivatedhex;
}

}
