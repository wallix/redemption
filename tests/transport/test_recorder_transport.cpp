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
#include "system/redemption_unit_tests.hpp"


#include "transport/recorder_transport.hpp"
#include "test_only/transport/test_transport.hpp"
#include "test_only/get_file_contents.hpp"
#include "utils/sugar/scope_exit.hpp"


RED_AUTO_TEST_CASE(TestRecorderTransport)
{
    char const* filename = "/tmp/recorder_test.out";
    SCOPE_EXIT(unlink(filename));

    {
        TestTransport socket(
            cstr_array_view("123456789"),
            cstr_array_view("abcdefghijklmnopqrstuvwxyz"));
        RecorderTransport trans(socket, filename);
        char buf[10];

        trans.send(cstr_array_view("abc"));
        trans.send(cstr_array_view("defg"));
        (void)trans.partial_read(buf, 3);
        (void)trans.partial_read(buf, 3);
        trans.send(cstr_array_view("h"));
        trans.send(cstr_array_view("ijklm"));
        trans.disconnect();
        trans.connect();
        trans.send(cstr_array_view("no"));
        trans.enable_server_tls("", "");
        trans.send(cstr_array_view("p"));
        (void)trans.partial_read(buf, 3);
        trans.send(cstr_array_view("q"));
        trans.send(cstr_array_view("rstuvw"));
        trans.send(cstr_array_view("xyz"));
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
        "\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        // Connect
        "\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00""no"
        // Cert
        "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00""p"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00""789"
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00""q"
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00""rstuvw"
        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00""xyz"
        // Eof
        "\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00");
}
