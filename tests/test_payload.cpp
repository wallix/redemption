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

BOOST_AUTO_TEST_CASE(Test_send_payload)
{
    struct Buffer : public Payload
    {
        BStream stream;
        Buffer() : stream(64) {
            stream.out_sz("ABCDEF");
        }

        using Payload::len;
        size_t len(void)
        {
            return stream.get_offset(0);
        }
        using Payload::send;
        void send(Transport & trans)
        {
            trans.send(stream.data, this->len());
        }
    } buffer;

    CheckTransport t("ABCDEF\0", 7);
    buffer.send(t);
    BOOST_CHECK_EQUAL(0, t.remaining());
}

BOOST_AUTO_TEST_CASE(Test_send_header_payload)
{
    struct Buffer : public Payload
    {
        BStream stream;
        Buffer() : stream(64) 
        {
            stream.out_string("BODY.");
        }

        using Payload::len;
        size_t len(void)
        {
            return stream.get_offset(0);
        }
        using Payload::send;
        void send(Transport & trans)
        {
            trans.send(stream.data, this->len());
        }
    } buffer;

    struct Wrapper : public Payload
    {
        BStream stream;
        Payload & payload;
        Wrapper(Payload & payload) 
        : stream(64) 
        , payload(payload)
        {
            stream.out_string("HEADER:");
        }

        using Payload::len;
        size_t len(void)
        {
            return stream.get_offset(0) + this->payload.len();
        }
        using Payload::send;
        void send(Transport & trans)
        {
            trans.send(stream.data, stream.get_offset(0));
            this->payload.send(trans);
        }
    } wrapper(buffer);

    BOOST_CHECK_EQUAL(12, wrapper.len());

    CheckTransport t("HEADER:BODY.", 12);
    wrapper.send(t);
    BOOST_CHECK_EQUAL(0, t.remaining());
}


BOOST_AUTO_TEST_CASE(Test_recv_payload)
{
    struct Wrapper
    {
        BStream stream;
        SubStream payload;
        Wrapper() : stream(64), payload(stream, 7) {
            stream.out_string("HEADER:BODY.");
        }
    } wrapper;

    struct Buffer
    {
        SubStream & stream;
        Buffer(SubStream & stream) 
        : stream(stream) 
        {
        }
    } buffer(wrapper.payload);

    BOOST_CHECK_EQUAL(0, memcmp("HEADER:BODY.",wrapper.stream.data, 12));
    BOOST_CHECK_EQUAL(0, memcmp("BODY.",buffer.stream.data, 5));
}

