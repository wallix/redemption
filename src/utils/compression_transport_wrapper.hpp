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
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_UTILS_COMPRESSION_TRANSPORT_WRAPPER_HPP
#define REDEMPTION_UTILS_COMPRESSION_TRANSPORT_WRAPPER_HPP

#include "transport/gzip_compression_transport.hpp"
#include "transport/snappy_compression_transport.hpp"

struct CompressionTransportBase
{
    enum class Algorithm : unsigned { None, Gzip, Snappy, NUNMBER };

    static const unsigned min_algorithm = 0;
    static const unsigned max_algorithm = static_cast<unsigned>(Algorithm::NUNMBER);

    explicit CompressionTransportBase(unsigned compression_algorithm)
    : algorithm(static_cast<Algorithm>(compression_algorithm < max_algorithm ? compression_algorithm : 0))
    {}

    Algorithm get_algorithm() const {
        return this->algorithm;
    }

    unsigned get_index_algorithm() const {
        return static_cast<unsigned>(this->algorithm);
    }

private:
    Algorithm algorithm;
};

template<class GZipTransport, class SnappyTransport>
struct CompressionTransportWrapper :  CompressionTransportBase
{
    CompressionTransportWrapper(Transport & trans, Algorithm compression_algorithm, uint32_t verbose = 0)
    : CompressionTransportWrapper(trans, static_cast<unsigned>(compression_algorithm), verbose)
    {}

    CompressionTransportWrapper(Transport & trans, unsigned compression_algorithm, uint32_t verbose = 0)
    : CompressionTransportBase(compression_algorithm)
    , compressors(trans)
    {
        if (this->get_algorithm() == Algorithm::Gzip) {
            new (&this->compressors.gzip_trans) GZipTransport(trans, verbose);
        }
        else if (this->get_algorithm() == Algorithm::Snappy) {
            new (&this->compressors.snappy_trans) SnappyTransport(trans, verbose);
        }
    }

    ~CompressionTransportWrapper() {
        if (this->get_algorithm() == Algorithm::Gzip) {
            this->compressors.gzip_trans.~GZipTransport();
        }
        else if (this->get_algorithm() == Algorithm::Snappy) {
            this->compressors.snappy_trans.~SnappyTransport();
        }
    }

    Transport & get() {
        if (this->get_algorithm() == Algorithm::Gzip) {
            return this->compressors.gzip_trans;
        }
        else if (this->get_algorithm() == Algorithm::Snappy) {
            return this->compressors.snappy_trans;
        }
        return *this->compressors.trans;
    }

private:
    union CompressionTransport {
        GZipTransport   gzip_trans;
        SnappyTransport snappy_trans;
        Transport *     trans;

        explicit CompressionTransport(Transport & trans)
        : trans(&trans)
        {}

        ~CompressionTransport()
        {}
    } compressors;
};

typedef CompressionTransportWrapper<GZipCompressionOutTransport, SnappyCompressionOutTransport> CompressionOutTransportWrapper;
typedef CompressionTransportWrapper<GZipCompressionInTransport, SnappyCompressionInTransport> CompressionInTransportWrapper;

#endif
