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

#define RED_TEST_MODULE TestSnappyCompressionTransport
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/compression_transport_builder.hpp"

#include <string>

namespace
{
    std::string str;
}


RED_AUTO_TEST_CASE(TestCompressionTransportBuilder)
{
    struct NoneTransport : Transport {
        void flush() override { str += "none\n"; }
    };
    struct GzipTransport : Transport {
        GzipTransport(Transport &, uint32_t) {}
        void flush() override { str += "gzip\n"; }
    };
    struct SnappyTransport : Transport {
        SnappyTransport(Transport &, uint32_t) {}
        void flush() override { str += "snappy\n"; }
    };

    NoneTransport trans;

    using CompressionTestTransportBuilder = CompressionTransportBuilder<GzipTransport, SnappyTransport>;

    CompressionTestTransportBuilder(trans, WrmCompressionAlgorithm::no_compression).get().flush();
    CompressionTestTransportBuilder(trans, WrmCompressionAlgorithm::gzip).get().flush();
    CompressionTestTransportBuilder(trans, WrmCompressionAlgorithm::snappy).get().flush();

    RED_CHECK_EQUAL(str, "none\ngzip\nsnappy\n");

    CompressionTestTransportBuilder t(trans, WrmCompressionAlgorithm::gzip);
    RED_CHECK_EQUAL(t.get_algorithm(), WrmCompressionAlgorithm::gzip);
}
