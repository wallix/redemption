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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean
*/

#define RED_TEST_MODULE TestVNC
#include "system/redemption_unit_tests.hpp"
#include "test_only/fake_graphic.hpp"
#include "test_only/check_sig.hpp"

#include "core/client_info.hpp"
#include "core/font.hpp"
#include "mod/vnc/vnc.hpp"
#include "test_only/transport/test_transport.hpp"
#include "mod/vnc/encoder/zrle.hpp"

RED_AUTO_TEST_CASE(TestFillEncodingTypesBuffer)
{
    struct testable_mod_vnc : mod_vnc {
        static void testable_fill_encoding_types_buffer(
                const char * encodings, OutStream & stream,
                uint16_t & number_of_encodings) {
            fill_encoding_types_buffer(encodings, stream, number_of_encodings,
                 VNCVerbose{});
        }
    };

    {
        StaticOutStream <512> stream;
        uint16_t number_of_encodings = 0;
        testable_mod_vnc::testable_fill_encoding_types_buffer("16,2,0,1,-239",
            stream, number_of_encodings);
        RED_CHECK(!memcmp(stream.get_data(),
                            "\x00\x00\x00\x10\x00\x00\x00\x02"
                            "\x00\x00\x00\x00\x00\x00\x00\x01"
                            "\xFF\xFF\xFF\x11",
                            20));
        RED_CHECK_EQUAL(number_of_encodings, 5);
    }

    {
        StaticOutStream <512> stream;
        uint16_t number_of_encodings = 0;
        testable_mod_vnc::testable_fill_encoding_types_buffer(
            "\t16 , 2 , 0 , 1 , -239 ", stream, number_of_encodings);
        RED_CHECK(!memcmp(stream.get_data(),
                            "\x00\x00\x00\x10\x00\x00\x00\x02"
                            "\x00\x00\x00\x00\x00\x00\x00\x01"
                            "\xFF\xFF\xFF\x11",
                            20));
        RED_CHECK_EQUAL(number_of_encodings, 5);
    }
}


RED_AUTO_TEST_CASE(TestVncMouse)
{
    char data[] =
        "\x05\x00\x00\x0a\x00\x0a"                          // move 10, 10
        "\x05\x08\x00\x0a\x00\x0a\x05\x00\x00\x0a\x00\x0a"  // scrool up
        "\x05\x01\x00\x0a\x00\x0a"                          // up left click
        "\x05\x03\x00\x0a\x00\x0a"                          // up right click
        "\x05\x02\x00\x0a\x00\x0a"                          // down left click
        "\x05\x02\x00\x0f\x00\x11"                          // move 15, 17
        "\x05\x00\x00\x0f\x00\x12"                          // down right click + move 15, 18
    ;
    CheckTransport t(data, sizeof(data)-1);
    mod_vnc::Mouse mouse;
    mouse.move(t, 10, 10);
    mouse.scroll(t, 8);
    mouse.click(t, 10, 10, 1, 1);
    mouse.click(t, 10, 10, 2, 1);
    mouse.click(t, 10, 10, 1, 0);
    mouse.move(t, 15, 17);
    mouse.click(t, 15, 18, 2, 0);
}


#include "vnc_zrle_slice1_0_34.hpp"

namespace VNC
{ 
    namespace Encoder {
        enum class State
        {
            Header,
            Encoding,
            Data,
            RreData,
            Exit
        };
    }
}

RED_AUTO_TEST_CASE(TestZrle)
{
    Zdecompressor<> zd;

    const uint8_t rect1_header[] = {/* 0000 */ 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x22, 0x00, 0x00, 0x00, 0x10,};
    const uint8_t compressed_len1[] = { 0x00, 0x00, 0xff, 0xad};
    const_byte_array datas[5] = {
         make_array_view(rect1_header),
         make_array_view(compressed_len1),
         make_array_view(slice1_0_34_p1),
         make_array_view(slice1_0_34_p2),
         make_array_view(slice1_0_34_p3)
    };
    Buf64k buf;
    class BlockWrap : public PartialReaderAPI
    {
        const_byte_array & t;
        size_t pos;
    public:
        BlockWrap(const_byte_array & t) : t(t), pos(0) {}
        
        size_t partial_read(byte_ptr buffer, size_t len) override
        {
            const size_t available = this->t.size() - this->pos;
            if (len >= available){
                std::memcpy(&buffer[0], &this->t[this->pos], available);
                this->pos += available;
                return available;
            }
            std::memcpy(&buffer[0], &this->t[this->pos], len);
            this->pos += len;
            return len;
        }
        bool empty() const {
            return this->t.size() == this->pos;
        }
    };

    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 16;
    info.width = 1920;
    info.height = 34;

    FakeGraphic drawable(info, 20);


    auto state = VNC::Encoder::State::Encoding;
    VNC::Encoder::Zrle * encoder = nullptr;
    for (auto t: datas){
        BlockWrap bw(t);
                
        while (!bw.empty()){
            buf.read_from(bw);
            switch (state){
                default:
                    LOG(LOG_INFO, "ignoring %zu\n", buf.av().size());
                    buf.advance(buf.av().size());
                    break;
                case VNC::Encoder::State::Encoding:
                {
                    const size_t sz = 12;
                    if (buf.remaining() < sz){ break; /* need more data */ }
                    InStream stream(buf.av(sz));
                    uint16_t x = stream.in_uint16_be();
                    uint16_t y = stream.in_uint16_be();
                    uint16_t cx = stream.in_uint16_be();
                    uint16_t cy = stream.in_uint16_be();
                    int32_t encoding = stream.in_sint32_be();
                    encoder = new VNC::Encoder::Zrle(info.bpp, nbbytes(info.bpp), x, y, cx, cy, zd, VNCVerbose::basic_trace);
                    LOG(LOG_INFO, "Encoding: (%u, %u, %u, %u) : %d", x, y, cx, cy, encoding);
                    buf.advance(sz);
                    // Post Assertion: we have an encoder
                    state = VNC::Encoder::State::Data;
                    break;
                }
                case VNC::Encoder::State::Data:
                {
                        // Pre Assertion: we have an encoder
                        if (encoder->consume(buf, drawable)){
                            // consume returns true if encoder is finished (ready to be resetted)
                            state = VNC::Encoder::State::Exit;
                            LOG(LOG_INFO, "End of encoder");
                        }
                }
                break;
            }
        }
    }
    LOG(LOG_INFO, "All data consumed");
    drawable.save_to_png("vnc_first_len.png");
    char message[4096] = {};
    if (!redemption_unit_test__::check_sig(drawable.gd, message,
                            "\xd6\x38\xee\x6a\xa7\x49\x9e\x06\xa3\x6d\x08\xd1\xf3\x82\x8d\x63\xad\x23\x9d\x2f")){
        LOG(LOG_INFO, "signature mismatch: %s", message);
        BOOST_CHECK(false);
    }

}


