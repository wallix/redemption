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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean
*/

#define RED_TEST_MODULE TestRecorderTransport
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "transport/recorder_transport.hpp"
#include "transport/replay_transport.hpp"
#include "test_only/transport/test_transport.hpp"
#include "test_only/get_file_contents.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "utils/difftimeval.hpp"
#include "utils/select.hpp"


class ZeroTime : public TimeObj
{
    timeval get_time() override
    {
        return {};
    }
};


RED_AUTO_TEST_CASE(TestRecorderTransport)
{
    char const* filename = "/tmp/recorder_test.out";
    unlink(filename);
    SCOPE_EXIT(unlink(filename));

    using Pck = RecorderFile::PacketType;

    struct {
        Pck type;
        array_view_const_char s;
    } a[] {
        {Pck::DataOut,    cstr_array_view("abc")},
        {Pck::DataOut,    cstr_array_view("defg")},
        {Pck::DataIn,     cstr_array_view("")},
        {Pck::DataIn,     cstr_array_view("")},
        {Pck::DataOut,    cstr_array_view("h")},
        {Pck::DataOut,    cstr_array_view("ijklm")},
        {Pck::Disconnect, cstr_array_view("")},
        {Pck::Connect,    cstr_array_view("")},
        {Pck::DataOut,    cstr_array_view("no")},
        {Pck::ServerCert, cstr_array_view("")},
        {Pck::DataOut,    cstr_array_view("p")},
        {Pck::DataIn,     cstr_array_view("")},
        {Pck::DataOut,    cstr_array_view("q")},
        {Pck::DataOut,    cstr_array_view("rstuvw")},
        {Pck::DataOut,    cstr_array_view("xyz")},
    };

    {
        ZeroTime timeobj;
        TestTransport socket(
            cstr_array_view("123456789"),
            cstr_array_view("abcdefghijklmnopqrstuvwxyz"));
        RecorderTransport trans(socket, timeobj, filename);
        char buf[10];

        for (auto m : a) {
            switch (m.type) {
                case Pck::DataOut: trans.send(m.s); break;
                case Pck::DataIn: (void)trans.partial_read(buf, 3); break;
                case Pck::ServerCert: trans.enable_server_tls("", ""); break;
                case Pck::Disconnect: trans.disconnect(); break;
                case Pck::Connect: trans.connect(); break;
                case Pck::NlaClientIn:
                case Pck::NlaClientOut:
                case Pck::NlaServerIn:
                case Pck::NlaServerOut:
                case Pck::ClientCert:
                case Pck::Info:
                case Pck::Eof:
                    RED_FAIL("unreacheable");
            }
        }
    }

    auto s = get_file_contents(filename);
    RED_CHECK_MEM_C(s,
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00""abc"
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00""defg"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00""123"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00""456"
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00""h"
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00""ijklm"
        // Disconnect
        "\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        // Connect
        "\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00""no"
        // Cert
        "\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00""p"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00""789"
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00""q"
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00""rstuvw"
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00""xyz"
        // Eof
        "\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");

    {
        GeneratorTransport trans(s);
        RecorderTransportHeader header;
        auto it = std::begin(a);
        while (
            (void)(header = read_recorder_transport_header(trans)),
            header.type != RecorderFile::PacketType::Eof
        ) {
            RED_CHECK_EQ(((it->type == Pck::DataIn) ? 3 : it->s.size()), header.data_size);
            RED_CHECK_EQ(Transport::Read::Ok, trans.atomic_read({s.data(), header.data_size}));
            ++it;
        }
        RED_CHECK_EQ(it-std::begin(a), std::size(a));
    }

    {
        ZeroTime timeobj;
        ReplayTransport trans(
            filename, "ip", 0/*port*/, timeobj, ReplayTransport::FdType::AlwaysReady,
            ReplayTransport::FirstPacket::DisableTimer, ReplayTransport::UncheckedPacket::Send);
        RED_CHECK_EXCEPTION_ERROR_ID(trans.send("!@#", 3), ERR_TRANSPORT_DIFFERS);
    }

    // Replay
    {
        ZeroTime timeobj;
        ReplayTransport trans(filename, "", 0, timeobj, ReplayTransport::FdType::AlwaysReady);
        char buf[10];
        auto av = make_array_view(buf);
        auto in = cstr_array_view("123456789");
        timeval timeout{0, 0};
        fd_set rfd;
        int fd = trans.get_fd();

        for (auto m : a) {

            switch (m.type) {
                case Pck::DataIn:
                    io_fd_zero(rfd);
                    io_fd_set(fd, rfd);

                    RED_REQUIRE_EQ(1, select(fd+1, &rfd, nullptr, nullptr, &timeout));
                    RED_CHECK_EQ(3, trans.partial_read(av));
                    RED_CHECK_MEM(in.first(3), make_array_view(buf, 3));
                    in = in.array_from_offset(3);
                    break;
                case Pck::DataOut: trans.send(m.s); break;
                case Pck::ServerCert: trans.enable_server_tls("", ""); break;
                case Pck::Disconnect: trans.disconnect(); break;
                case Pck::Connect: trans.connect(); break;
                case Pck::NlaClientIn:
                case Pck::NlaClientOut:
                case Pck::NlaServerIn:
                case Pck::NlaServerOut:
                case Pck::ClientCert:
                case Pck::Info:
                case Pck::Eof:
                    break;
            }
        }
    }

    // Replay real time
    {
        ZeroTime timeobj;
        ReplayTransport trans(filename, "", 0, timeobj, ReplayTransport::FdType::Timer);
        char buf[10];
        auto av = make_array_view(buf);
        auto in = cstr_array_view("123456789");
        fd_set rfd;
        int fd = trans.get_fd();
        timeval timeout{1, 0};

        for (auto m : a) {
            switch (m.type) {
                case Pck::DataIn:
                    io_fd_zero(rfd);
                    io_fd_set(fd, rfd);
                    RED_REQUIRE_EQ(1, select(fd+1, &rfd, nullptr, nullptr, &timeout));
                    RED_CHECK_EQ(3, trans.partial_read(av));
                    RED_CHECK_MEM(in.first(3), make_array_view(buf, 3));
                    in = in.array_from_offset(3);
                    break;
                case Pck::DataOut: trans.send(m.s); break;
                case Pck::ServerCert: trans.enable_server_tls("", ""); break;
                case Pck::Disconnect: trans.disconnect(); break;
                case Pck::Connect: trans.connect(); break;
                case Pck::NlaClientIn:
                case Pck::NlaClientOut:
                case Pck::NlaServerIn:
                case Pck::NlaServerOut:
                case Pck::ClientCert:
                case Pck::Info:
                case Pck::Eof:
                    break;
            }
        }
    }
}
