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
#include "mixin_transport.hpp"
// #include "buffer/buffering_buf.hpp"
#include "buffer/file_buf.hpp"
#include "fdbuf.hpp"
#include "buffer/sum_buf.hpp"

namespace detail
{
}

struct OutMetaSequenceTransportWithSum
: //SeekableTransport<
// FlushingTransport<
RequestCleaningTransport<
    OutputNextTransport<detail::out_meta_sequence_filename_buf<
        transbuf::osum_and_mini_sum_buf</*transbuf::obuffering_buf<*/io::posix::fdbuf/*>*/ >,
        detail::empty_ctor<transbuf::osum_buf<transbuf::ofile_base>>
    >, detail::GetCurrentPath>
>
// >
// >
{
    OutMetaSequenceTransportWithSum(
        const char * path,
        const char * basename,
        timeval now,
        uint16_t width,
        uint16_t height,
        const int groupid,
        auth_api * authentifier = nullptr,
        unsigned verbose = 0,
        FilenameFormat format = FilenameGenerator::PATH_FILE_COUNT_EXTENSION)
    : OutMetaSequenceTransportWithSum::TransportType(
        detail::out_meta_sequence_filename_buf_param<detail::no_param, transbuf::sum_and_mini_sum_buf_param<>>(
            now.tv_sec, format, path, basename, ".wrm", groupid,
            detail::no_param{}, transbuf::sum_and_mini_sum_buf_param<>{1024}
        ))
    {
        (void)verbose;

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
