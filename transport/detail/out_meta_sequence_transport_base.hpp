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
 *   Copyright (C) Wallix 2010-2014
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_TRANSPORT_DETAIL_OUT_META_SEQUENCE_TRANSPORT_BASE_HPP
#define REDEMPTION_TRANSPORT_DETAIL_OUT_META_SEQUENCE_TRANSPORT_BASE_HPP

#include "buffer_transport.hpp"
#include "sequence_generator.hpp"
#include "meta_writer.hpp"

namespace detail
{
    template<class Buf, class Nexter>
    class OutMetaSequenceTransportBase
    : public OutBufferTransport<Buf, Nexter>
    {
    protected:
        detail::MetaFilename mf;

    protected:
        typedef OutMetaSequenceTransportBase OutMetaSequenceBase;

    public:
        template<class BufParams, class NexterParams>
        OutMetaSequenceTransportBase(
            const BufParams & buf_params,
            const NexterParams & nexter_params,
            const char * path,
            const char * basename,
            FilenameFormat format,
            uint16_t width,
            uint16_t height,
            auth_api * authentifier = NULL,
            unsigned verbose = 0)
        : OutMetaSequenceTransportBase::TransportType(buf_params, nexter_params)
        , mf(path, basename, format)
        {
            (void)verbose;

            if (this->nexter().open(this->mf.filename, S_IRUSR) < 0) {
                throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
            }

            if (authentifier) {
                this->set_authentifier(authentifier);
            }

            detail::write_meta_headers(this->nexter(), path, width, height, this->authentifier);
        }

        virtual void timestamp(timeval now) /*noexcept*/
        {
            this->nexter().update_sec(now.tv_sec);
        }

        const FilenameGenerator * seqgen() const /*noexcept*/
        {
            return &(this->buffer().policy().seqgen());
        }

        virtual void request_full_cleaning()
        {
            this->buffer().policy().request_full_cleaning();
            ::unlink(this->mf.filename);
        }
    };
}

#endif
