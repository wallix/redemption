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

#pragma once

#include "transport/gzip_compression_transport.hpp"
#include "transport/snappy_compression_transport.hpp"
#include "configs/autogen/enums.hpp"

template<class GZipTransport, class SnappyTransport>
struct CompressionTransportBuilder
{
    CompressionTransportBuilder(
        Transport & trans,
        WrmCompressionAlgorithm compression_algorithm,
        uint32_t verbose = 0
    ) {
        this->construct_(trans, compression_algorithm, verbose);
    }

    Transport & reset(
        Transport & trans,
        WrmCompressionAlgorithm compression_algorithm,
        uint32_t verbose = 0
    ) {
        this->destruct_();
        return this->construct_(trans, compression_algorithm, verbose);
    }

    ~CompressionTransportBuilder()
    {
        this->destruct_();
    }

    Transport & get()
    {
        switch (this->get_algorithm()) {
            case WrmCompressionAlgorithm::gzip:
                return this->compressors.gzip_trans;
            case WrmCompressionAlgorithm::snappy:
                return this->compressors.snappy_trans;
            case WrmCompressionAlgorithm::no_compression:
                return *this->compressors.trans;
        }
        assert(false && "unknown algorithm");
        return *this->compressors.trans;
    }

    WrmCompressionAlgorithm get_algorithm() const
    {
        return this->compression_algorithm;
    }

private:
    WrmCompressionAlgorithm compression_algorithm;
    union CompressionTransport
    {
        GZipTransport   gzip_trans;
        SnappyTransport snappy_trans;
        Transport *     trans;
        char            dummy;

        explicit CompressionTransport()
        : dummy()
        {}

        ~CompressionTransport() {} /*NOLINT*/
    } compressors;

    void destruct_()
    {
        switch (this->get_algorithm()) {
            case WrmCompressionAlgorithm::gzip:
                this->compressors.gzip_trans.~GZipTransport();
                break;
            case WrmCompressionAlgorithm::snappy:
                this->compressors.snappy_trans.~SnappyTransport();
                break;
            case WrmCompressionAlgorithm::no_compression:
                break;
        }
    }

    Transport & construct_(Transport & trans, WrmCompressionAlgorithm compression_algorithm, uint32_t verbose)
    {
        this->compression_algorithm = compression_algorithm;
        switch (this->get_algorithm()) {
            case WrmCompressionAlgorithm::gzip:
                new (&this->compressors.gzip_trans) GZipTransport(trans, verbose);
                return this->compressors.gzip_trans;
            case WrmCompressionAlgorithm::snappy:
                new (&this->compressors.snappy_trans) SnappyTransport(trans, verbose);
                return this->compressors.snappy_trans;
            case WrmCompressionAlgorithm::no_compression:
                this->compressors.trans = &trans;
                return trans;
        }
        assert(false && "unknown algorithm");
        this->compressors.trans = &trans;
        return trans;
    }
};

using CompressionOutTransportBuilder = CompressionTransportBuilder<GZipCompressionOutTransport, SnappyCompressionOutTransport>;
using CompressionInTransportBuilder = CompressionTransportBuilder<GZipCompressionInTransport, SnappyCompressionInTransport>;
