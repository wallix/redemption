/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

    Generic Payload abstraction Class

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestPDU
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "stream.hpp"
#include "transport.hpp"
#include "payload.hpp"

struct BufferSend
{
    Stream & stream;
    BufferSend(Stream & stream) : stream(stream) {
        stream.out_string("BODY.");
        stream.mark_end();
    }
}; 

struct BufferRecv
{
    Stream & stream;
    size_t payload_offset;
    BufferRecv(Transport & t, Stream & stream, size_t size)
    : stream(stream)
    {
        t.recv(&stream.end, size - (stream.size()));
        // here there is not payload, hence payload offset is at the end
        this->payload_offset = 5;
    }    
};

struct MiddleSend
{
    Stream & stream;
    Stream & payload;
    MiddleSend(Stream & stream, Stream & payload) 
    : stream(stream), payload(payload) 
    {
        stream.out_string("MID:");
        stream.mark_end();
    }
};

struct MiddleRecv
{
    Stream & stream;
    size_t payload_offset;

    MiddleRecv(Transport & t, Stream & stream, size_t size)
    : stream(stream)
    {
        t.recv(&stream.end, size - (stream.size()));
        this->payload_offset = 4;

    }
    size_t get_payload(SubStream & s)
    {
        s.reset(this->stream, payload_offset);
        return this->stream.end - this->stream.data - this->payload_offset;
    }
};

struct CarrierSend
{
    Stream & stream;
    Stream & payload;
    CarrierSend(Stream & stream, Stream & payload) 
    : stream(stream), payload(payload) 
    {
        stream.out_string("HEADER:");
        stream.mark_end();
    }
};


struct CarrierRecvFactory
{
    enum {
        CARRIER_TYPE_1EADER,
        CARRIER_TYPE_HEADER
    };

    int type;
    CarrierRecvFactory(Transport & t, Stream & stream)
    {
        t.recv(&stream.end, 8 - (stream.size()));
        if (stream.data[7] == 'B'){
            this->type = CARRIER_TYPE_1EADER; // t.recv(&stream.end, 12 - (stream.size()));
        }
        else {
            this->type = CARRIER_TYPE_HEADER; // t.recv(&stream.end, 16 - (stream.size()));
        }
    }
};

struct Carrier1eaderRecv
{
    Stream & stream;
    size_t payload_offset;
    Carrier1eaderRecv(Transport & t, Stream & stream)
    : stream(stream)
    {
        t.recv(&stream.end, 12 - (stream.size()));
        this->payload_offset = 7;
    }
    size_t get_payload(SubStream & s)
    {
        s.reset(this->stream, this->payload_offset);
        return this->stream.end - this->stream.data - this->payload_offset;
    }
};

struct CarrierHeaderRecv
{
    Stream & stream;
    size_t payload_offset;
    CarrierHeaderRecv(Transport & t, Stream & stream)
    : stream(stream)
    {
        t.recv(&stream.end, 16 - (stream.size()));
        this->payload_offset = 7;
    }
    size_t get_payload(SubStream & s)
    {
        s.reset(this->stream, this->payload_offset);
        return this->stream.end - this->stream.data - this->payload_offset;
    }
};

BOOST_AUTO_TEST_CASE(Test_send_payload)
{
    BStream stream;
    BufferSend buffer(stream);
    CheckTransport t("BODY.", 5);
    t.send(stream.data, stream.size());
    BOOST_CHECK_EQUAL(0, t.remaining());
}

BOOST_AUTO_TEST_CASE(Test_send_header_payload)
{
    BStream b;
    BufferSend buffer(b);
    BStream c;
    CarrierSend carrier(c, b);

    CheckTransport t("HEADER:BODY.", 12);
    t.send(c.data, c.end - c.data);
    t.send(b.data, b.end - b.data);
    BOOST_CHECK_EQUAL(0, t.remaining());
}


BOOST_AUTO_TEST_CASE(Test_send_header_payload_layered)
{
    BStream b;
    BufferSend buf(b);
    BStream m;
    MiddleSend mid(m, b);
    BStream c;
    CarrierSend carrier(c, m);

    CheckTransport t("HEADER:MID:BODY.", 16);
    t.send(c.data, c.end - c.data);
    t.send(m.data, m.end - m.data);
    t.send(b.data, b.end - b.data);
    BOOST_CHECK_EQUAL(0, t.remaining());
}



BOOST_AUTO_TEST_CASE(Test_recv_payload)
{
    GeneratorTransport t("BODY.", 5);
    BStream b;
    BufferRecv buffer(t, b, 5);
    BOOST_CHECK_EQUAL(0, memcmp("BODY.", b.data, buffer.payload_offset));
}

BOOST_AUTO_TEST_CASE(Test_recv_payload_2)
{
    try {
        GeneratorTransport t("1EADER:BODY.", 12);
        BStream c;
        Carrier1eaderRecv carrier(t, c);
        SubStream b;
        size_t length = carrier.get_payload(b);
        BufferRecv buffer(t, b, length);

        BOOST_CHECK_EQUAL(7, carrier.payload_offset);
        BOOST_CHECK_EQUAL(0, memcmp("1EADER:", c.data, carrier.payload_offset));

        BOOST_CHECK_EQUAL(5, length);
        BOOST_CHECK_EQUAL(5, b.end - b.data);
        BOOST_CHECK_EQUAL(0, memcmp("BODY.", b.data, b.end - b.data));
    } catch(Error & e){
        LOG(LOG_ERR, "eid=%u", e.id);
        BOOST_CHECK(0);
    };
}

BOOST_AUTO_TEST_CASE(Test_recv_payload_3)
{
    try {
        GeneratorTransport t("HEADER:MID:BODY.", 16);

        BStream c;
        CarrierHeaderRecv carrier(t, c);

        SubStream m;
        size_t length_mid = carrier.get_payload(m);
        MiddleRecv middle(t, m, length_mid);

        SubStream b;
        size_t length_buf = middle.get_payload(b);
        BufferRecv buffer(t, b, length_buf);

        BOOST_CHECK_EQUAL(7, carrier.payload_offset);
        BOOST_CHECK_EQUAL(0, memcmp("HEADER:", c.data, carrier.payload_offset));

        BOOST_CHECK_EQUAL(4+5, length_mid);
        BOOST_CHECK_EQUAL(4, middle.payload_offset);
        BOOST_CHECK_EQUAL(0, memcmp("MID:", m.data, middle.payload_offset));

        BOOST_CHECK_EQUAL(5, length_buf);
        BOOST_CHECK_EQUAL(5, buffer.payload_offset);
        BOOST_CHECK_EQUAL(0, memcmp("BODY.", b.data, buffer.payload_offset));

    } catch(Error & e){
        LOG(LOG_ERR, "eid=%u", e.id);
        BOOST_CHECK(0);
    };
}

BOOST_AUTO_TEST_CASE(Test_recv_payload_polymorphic_header)
{
    try {
        GeneratorTransport t("HEADER:MID:BODY.", 16);

        BStream c;

        CarrierRecvFactory f(t, c);

        switch (f.type){
        
        case CarrierRecvFactory::CARRIER_TYPE_1EADER:{
            Carrier1eaderRecv carrier(t, c);

            SubStream m;
            size_t length_mid = carrier.get_payload(m);
            MiddleRecv middle(t, m, length_mid);

            SubStream b;
            size_t length_buf = middle.get_payload(b);
            BufferRecv buffer(t, b, length_buf);

            BOOST_CHECK_EQUAL(7, c.end - c.data);
            BOOST_CHECK_EQUAL(0, memcmp("HEADER:", c.data, c.end - c.data));

            BOOST_CHECK_EQUAL(4+5, length_mid);
            BOOST_CHECK_EQUAL(4, m.end - m.data);
            BOOST_CHECK_EQUAL(0, memcmp("MID:", m.data, m.end - m.data));

            BOOST_CHECK_EQUAL(5, length_buf);
            BOOST_CHECK_EQUAL(5, b.end - b.data);
            BOOST_CHECK_EQUAL(0, memcmp("BODY.", b.data, b.end - b.data));
        }
        break;

        case CarrierRecvFactory::CARRIER_TYPE_HEADER:{
            CarrierHeaderRecv carrier(t, c);

            SubStream m;
            size_t length_mid = carrier.get_payload(m);
            MiddleRecv middle(t, m, length_mid);

            SubStream b;
            size_t length_buf = middle.get_payload(b);
            BufferRecv buffer(t, b, length_buf);

            BOOST_CHECK_EQUAL(7, carrier.payload_offset);
            BOOST_CHECK_EQUAL(0, memcmp("HEADER:", c.data, carrier.payload_offset));

            BOOST_CHECK_EQUAL(4+5, length_mid);
            BOOST_CHECK_EQUAL(4, middle.payload_offset);
            BOOST_CHECK_EQUAL(0, memcmp("MID:", m.data, middle.payload_offset));

            BOOST_CHECK_EQUAL(5, length_buf);
            BOOST_CHECK_EQUAL(5, buffer.payload_offset);
            BOOST_CHECK_EQUAL(0, memcmp("BODY.", b.data, buffer.payload_offset));
        }
        break;
        default:
            BOOST_CHECK(0);
        break;
        }
    } catch(Error & e){
        LOG(LOG_ERR, "eid=%u", e.id);
        BOOST_CHECK(0);
    };
}

