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

        using Payload::send;
        void send(Transport & trans)
        {
            trans.send(stream.data, stream.get_offset(0));
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

        using Payload::send;
        void send(Transport & trans)
        {
            trans.send(stream.data, stream.get_offset(0));
        }
    } buffer;

    struct Wrapper : public Payload
    {
        BStream stream;
        Payload & wrapped;
        Wrapper(Payload & wrapped) 
        : stream(64) 
        , wrapped(wrapped)
        {
            stream.out_string("HEADER:");
        }

        using Payload::send;
        void send(Transport & trans)
        {
            trans.send(stream.data, stream.get_offset(0));
            this->wrapped.send(trans);
        }
    } wrapper(buffer);

    CheckTransport t("HEADER:BODY.", 12);
    wrapper.send(t);
    BOOST_CHECK_EQUAL(0, t.remaining());
}


BOOST_AUTO_TEST_CASE(Test_send_header_payload_layered)
{
    struct BufferEmit : public Payload
    {
        BStream stream;
        BufferEmit() : stream(64) 
        {
            stream.out_string("BODY.");
        }

        using Payload::send;
        void send(Transport & trans)
        {
            trans.send(stream.data, stream.get_offset(0));
        }
    } buffer;

    struct MiddleWrapperEmit : public Payload
    {
        BStream stream;
        Payload & wrapped;
        MiddleWrapperEmit(Payload & wrapped) 
        : stream(64) 
        , wrapped(wrapped)
        {
            stream.out_string("MID:");
        }

        using Payload::send;
        void send(Transport & trans)
        {
            trans.send(stream.data, stream.get_offset(0));
            this->wrapped.send(trans);
        }
    } middle_wrapper(buffer);

    struct ExternalWrapperEmit : public Payload
    {
        BStream stream;
        Payload & wrapped;
        ExternalWrapperEmit(Payload & wrapped) 
        : stream(64) 
        , wrapped(wrapped)
        {
            stream.out_string("HEADER:");
        }

        using Payload::send;
        void send(Transport & trans)
        {
            trans.send(stream.data, stream.get_offset(0));
            this->wrapped.send(trans);
        }
    } external_wrapper(middle_wrapper);

    CheckTransport t("HEADER:MID:BODY.", 16);
    external_wrapper.send(t);
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

BOOST_AUTO_TEST_CASE(Test_recv_payload_layered)
{
    struct ExternalWrapper 
    {
        BStream stream;
        SubStream payload;
        ExternalWrapper() : stream(64), payload(stream, 7) {
            stream.out_string("HEADER:MID:BODY.");
        }
    } external_wrapper;

    struct MiddleWrapper
    {
        Stream & stream;
        SubStream payload;
        MiddleWrapper(Stream & stream) : stream(stream), payload(stream, 4) {
        }
    } middle_wrapper(external_wrapper.payload);

    struct Buffer
    {
        Stream & stream;
        Buffer(Stream & stream) : stream(stream) 
        {
        }
    } buffer(middle_wrapper.payload);

    BOOST_CHECK_EQUAL(0, memcmp("HEADER:MID:BODY.",external_wrapper.stream.data, 16));
    BOOST_CHECK_EQUAL(0, memcmp("MID:BODY.",middle_wrapper.stream.data, 9));
    BOOST_CHECK_EQUAL(0, memcmp("BODY.",buffer.stream.data, 5));
}

