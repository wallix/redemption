/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_TRANSPORT_META_SEQUENCE_TRANSPORT_HPP
#define REDEMPTION_TRANSPORT_META_SEQUENCE_TRANSPORT_HPP

#include "transport/detail/meta_writer.hpp"
#include "transport/mixin_transport.hpp"
#include "transport/buffer/file_buf.hpp"
#include "fdbuf.hpp"
#include "transport/buffer/crypto_filename_buf.hpp"
#include "utils/genrandom.hpp"
#include "fileutils.hpp"
#include "transport/detail/meta_hash.hpp"
#include "transport/buffer/checksum_buf.hpp"
#include "transport/buffer/null_buf.hpp"
#include "transport/filter/crypto_filter.hpp"
#include "urandom_read.hpp"

namespace detail
{
    struct ocrypto_filter
    : transfil::encrypt_filter
    {
        ocrypto_filter(CryptoContext &)
        {}

        template<class Buf>
        int open(Buf & buf, CryptoContext & cctx, char const * filename) {
            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            unsigned char derivator[DERIVATOR_LENGTH];

//            printf("ocrypto_filter: cctx.get_derivator\n");
            cctx.get_derivator(filename, derivator, DERIVATOR_LENGTH);
            if (-1 == cctx.compute_hmac(trace_key, derivator)) {
                return -1;
            }

            unsigned char iv[32];
            cctx.random(iv, 32);
            return transfil::encrypt_filter::open(buf, trace_key, &cctx, iv);
        }
    };
}

namespace detail
{
    template<class Buf, class Params>
    struct OutHashedMetaSequenceTransport
    : // FlushingTransport<
    RequestCleaningTransport<OutputNextTransport<CloseWrapper<Buf>, detail::GetCurrentPath>>
    // >
    {
        OutHashedMetaSequenceTransport(
            CryptoContext * crypto_ctx,
            const char * path,
            const char * hash_path,
            const char * basename,
            timeval now,
            uint16_t width,
            uint16_t height,
            const int groupid,
            auth_api * authentifier = nullptr,
            unsigned verbose = 0,
            FilenameFormat format = FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
        : OutHashedMetaSequenceTransport::TransportType(Params(
            *crypto_ctx,
            now.tv_sec, format, hash_path, path, basename, ".wrm", groupid,
            *crypto_ctx,
            verbose
        ))
        {
            this->verbose = verbose;

            if (authentifier) {
                this->set_authentifier(authentifier);
            }

            detail::write_meta_headers(this->buffer().meta_buf(), path, width, height, this->authentifier, true);
        }

        void timestamp(timeval now) override {
            this->buffer().update_sec(now.tv_sec);
        }

        const FilenameGenerator * seqgen() const noexcept
        {
            return &(this->buffer().seqgen());
        }
    };


    class ochecksum_filter
    {
        transbuf::ochecksum_buf<transbuf::null_buf> sum_buf;

    public:
        ochecksum_filter(CryptoContext & cctx)
        : sum_buf(cctx.get_hmac_key())
        {}

        template<class Buf>
        int open(Buf &, CryptoContext &, char const * /*filename*/) {
            return this->sum_buf.open();
        }

        template<class Buf>
        int write(Buf & buf, const void * data, size_t len) {
            this->sum_buf.write(data, len);
            return buf.write(data, len);
        }

        template<class Buf>
        int close(Buf &, hash_type & hash, unsigned char (&)[MD_HASH_LENGTH]) {
            return this->sum_buf.close(hash);
        }
    };

    struct cctx_ofile_buf
    : transbuf::ofile_buf
    {
        cctx_ofile_buf(CryptoContext &)
        {}
    };

    struct cctx_ochecksum_file
    : transbuf::ochecksum_buf<transbuf::ofile_buf>
    {
        cctx_ochecksum_file(CryptoContext & cctx)
        : transbuf::ochecksum_buf<transbuf::ofile_buf>(cctx.get_hmac_key())
        {}
    };
}

#include "transport/buffer/file_buf.hpp"
#include "transport/filter/crypto_filter.hpp"
#include "urandom_read.hpp"

namespace transbuf {
    class ocrypto_filename_buf
    {
        transfil::encrypt_filter encrypt;
        CryptoContext * cctx;
        ofile_buf file;

    public:
        explicit ocrypto_filename_buf(CryptoContext * cctx)
        : cctx(cctx)
        {}

        explicit ocrypto_filename_buf(CryptoContext & cctx)
        : cctx(&cctx)
        {}

        ~ocrypto_filename_buf()
        {
            if (this->is_open()) {
                this->close();
            }
        }

        int open(const char * filename, mode_t mode = 0600)
        {
            unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
            unsigned char derivator[DERIVATOR_LENGTH];
//            printf("init_trace_key: cctx.get_derivator\n");

            cctx->get_derivator(filename, derivator, DERIVATOR_LENGTH);
            if (-1 == this->cctx->compute_hmac(trace_key, derivator)) {
                return -1;
            }

            int err = this->file.open(filename, mode);
            if (err < 0) {
                return err;
            }

            unsigned char iv[32];
            this->cctx->random(iv, 32);
//            if (-1 == urandom_read(iv, 32)) {
//                LOG(LOG_ERR, "iv randomization failed for crypto file=%s\n", filename);
//                return -1;
//            }

            return this->encrypt.open(this->file, trace_key, this->cctx, iv);
        }

        ssize_t write(const void * data, size_t len)
        { return this->encrypt.write(this->file, data, len); }

        int close(unsigned char hash[MD_HASH_LENGTH << 1])
        {
            const int res1 = this->encrypt.close(this->file, hash, this->cctx->get_hmac_key());
            const int res2 = this->file.close();
            return res1 < 0 ? res1 : (res2 < 0 ? res2 : 0);
        }

        int close()
        {
            unsigned char hash[MD_HASH_LENGTH << 1];
            return this->close(hash);
        }

        bool is_open() const noexcept
        { return this->file.is_open(); }

        off64_t seek(off64_t offset, int whence) const
        { return this->file.seek(offset, whence); }

        int flush() const
        { return this->file.flush(); }
    };
}


struct OutMetaSequenceTransport
: //SeekableTransport<
// FlushingTransport<
RequestCleaningTransport<
    OutputNextTransport<detail::out_meta_sequence_filename_buf<
        detail::empty_ctor</*transbuf::obuffering_buf<*/io::posix::fdbuf/*>*/ >,
        detail::empty_ctor<transbuf::ofile_buf>
    >, detail::GetCurrentPath>
>
// >
// >
{
    OutMetaSequenceTransport(
        CryptoContext * cctx,
        const char * path,
        const char * hash_path,
        const char * basename,
        timeval now,
        uint16_t width,
        uint16_t height,
        const int groupid,
        auth_api * authentifier = nullptr,
        unsigned verbose = 0,
        FilenameFormat format = FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
    : OutMetaSequenceTransport::TransportType(detail::out_meta_sequence_filename_buf_param<>(
        now.tv_sec, format, hash_path, path, basename, ".wrm", groupid, verbose
    ))
    {
        this->verbose = verbose;

        if (authentifier) {
            this->set_authentifier(authentifier);
        }

        detail::write_meta_headers(this->buffer().meta_buf(), path, width, height, this->authentifier, false);
    }

    void timestamp(timeval now) override {
        this->buffer().update_sec(now.tv_sec);
    }

    const FilenameGenerator * seqgen() const noexcept
    {
        return &(this->buffer().seqgen());
    }
};

using OutMetaSequenceTransportWithSum = detail::OutHashedMetaSequenceTransport<
    detail::out_hash_meta_sequence_filename_buf_impl<
        detail::empty_ctor</*transbuf::obuffering_buf<*/io::posix::fdbuf/*>*/>,
        detail::ochecksum_filter,
        detail::cctx_ochecksum_file,
        detail::cctx_ofile_buf
    >,
    detail::out_hash_meta_sequence_filename_buf_param<CryptoContext&>
>;

using CryptoOutMetaSequenceTransport = detail::OutHashedMetaSequenceTransport<
    detail::out_hash_meta_sequence_filename_buf_impl<
        detail::empty_ctor</*transbuf::obuffering_buf<*/io::posix::fdbuf/*>*/>,
        detail::ocrypto_filter,
        transbuf::ocrypto_filename_buf,
        transbuf::ocrypto_filename_buf
    >,
    detail::out_hash_meta_sequence_filename_buf_param<CryptoContext&>
>;

#endif
