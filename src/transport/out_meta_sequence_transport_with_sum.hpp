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

#ifndef REDEMPTION_TRANSPORT_META_SEQUENCE_TRANSPORT_WITH_SUM_HPP
#define REDEMPTION_TRANSPORT_META_SEQUENCE_TRANSPORT_WITH_SUM_HPP

#include "detail/meta_writer.hpp"
#include "detail/meta_hash.hpp"
#include "mixin_transport.hpp"
// #include "buffer/buffering_buf.hpp"
#include "buffer/file_buf.hpp"
#include "buffer/checksum_buf.hpp"
#include "buffer/null_buf.hpp"
#include "fdbuf.hpp"

namespace detail
{
    struct out_meta_sequence_filename_buf_with_sum_param
    {
        out_meta_sequence_filename_buf_param<CryptoContext *> meta_sq_params;
        const char * hash_prefix;
        CryptoContext & cctx;
        uint32_t verbose;

        out_meta_sequence_filename_buf_with_sum_param(
            CryptoContext & cctx,
            time_t start_sec,
            FilenameGenerator::Format format,
            const char * const hash_prefix,
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid,
            uint32_t verbose = 0)
        : meta_sq_params(start_sec, format, prefix, filename, extension, groupid, &cctx)
        , hash_prefix(hash_prefix)
        , cctx(cctx)
        , verbose(verbose)
        {}
    };


    template<class BufWrm, class BufMwrm>
    class out_meta_sequence_filename_buf_with_sum
    : public out_meta_sequence_filename_buf<BufWrm, BufMwrm>
    {
        detail::MetaFilename hf_;
        transbuf::ochecksum_buf<transbuf::null_buf> checksum_wrm;
        uint32_t verbose;

        typedef out_meta_sequence_filename_buf<BufWrm, BufMwrm> sequence_base_type;

    public:
        explicit out_meta_sequence_filename_buf_with_sum(out_meta_sequence_filename_buf_with_sum_param const & params)
        : sequence_base_type(params.meta_sq_params)
        , hf_(params.hash_prefix, params.meta_sq_params.sq_params.filename, params.meta_sq_params.sq_params.format)
        , checksum_wrm(&params.cctx)
        , verbose(params.verbose)
        {}

        ~out_meta_sequence_filename_buf_with_sum()
        {
            this->close();
        }

        ssize_t write(const void * data, size_t len)
        {
            if (!this->buf().is_open()) {
                const char * filename = this->get_filename_generate();
                const int res = this->open_filename(filename);
                if (res < 0) {
                    return res;
                }
                this->checksum_wrm.open();
            }
            this->checksum_wrm.write(data, len);
            return this->buf().write(data, len);
        }

        int close()
        {
            if (this->buf().is_open()) {
                if (this->next()) {
                    return 1;
                }
            }

            transbuf::ofile_buf file_hash;
            return close_meta_hash(this->hf_, this->meta_buf(), file_hash, this->verbose);
        }

        int next()
        {
            if (this->buf().is_open()) {
                unsigned char hash[MD_HASH_LENGTH * 2];
                this->checksum_wrm.close(hash);
                if (const int err = this->buf().close()) {
                    return err;
                }

                return this->next_meta_file(MetaHashMaker(hash).c_str());
            }
            return 1;
        }
    };
}

struct OutMetaSequenceTransportWithSum
: //SeekableTransport<
// FlushingTransport<
RequestCleaningTransport<
    OutputNextTransport<detail::out_meta_sequence_filename_buf_with_sum<
        detail::empty_ctor</*transbuf::obuffering_buf<*/io::posix::fdbuf/*>*/>,
        transbuf::ochecksum_buf<transbuf::ofile_buf>
    >, detail::GetCurrentPath>
>
// >
// >
{
    OutMetaSequenceTransportWithSum(
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
    : OutMetaSequenceTransportWithSum::TransportType(
        detail::out_meta_sequence_filename_buf_with_sum_param(
            *crypto_ctx,
            now.tv_sec, format, hash_path, path, basename, ".wrm", groupid, verbose
        )
    ) {
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

#endif
