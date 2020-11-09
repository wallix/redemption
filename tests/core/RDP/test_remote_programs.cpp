/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/RDP/remote_programs.hpp"
#include <string_view>

using namespace std::string_view_literals;


/*
RED_AUTO_TEST_CASE(TestRAILPDUHeader)
{
    StaticOutStream<128> out_stream;

    const uint8_t order_data[] = "0123456789";

    RAILPDUHeader_Send header_s(out_stream);
    header_s.emit_begin(TS_RAIL_ORDER_EXEC);

    out_stream.out_copy_bytes(order_data, sizeof(order_data));

    header_s.emit_end();

    InStream in_stream(buf, out_stream.get_offset());
    RAILPDUHeader_Recv header_r(in_stream);

    RED_CHECK_EQUAL(header_r.orderType(), TS_RAIL_ORDER_EXEC);
    RED_CHECK_EQUAL(header_r.orderLength(),
        sizeof(order_data) + 4 // orderType(2) + orderLength(2)
        );
}

RED_AUTO_TEST_CASE(TestHandshakePDU)
{
    StaticOutStream<128> out_stream;
    HandshakePDU_Send handshake_pdu_s(out_stream, 0x01020304);

    InStream in_stream(buf, out_stream.get_offset());
    HandshakePDU_Recv handshake_pdu_r(in_stream);

    RED_CHECK_EQUAL(handshake_pdu_r.buildNumber(), 0x01020304);
}

RED_AUTO_TEST_CASE(ClientExecutePDU)
{
    StaticOutStream<128> out_stream;
    ClientExecutePDU_Send client_execute_pdu_s(out_stream,
        TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY,
        "%%SystemRoot%%\\system32\\notepad.exe", "%%HOMEDRIVE%%%%HOMEPATH%%",
        "");

    InStream in_stream(buf, out_stream.get_offset());
    ClientExecutePDU_Recv client_execute_pdu_r(in_stream);

    RED_CHECK_EQUAL(client_execute_pdu_r.Flags(),
        TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY);
    RED_CHECK_EQUAL(client_execute_pdu_r.exe_or_file(),
        "%%SystemRoot%%\\system32\\notepad.exe");
    RED_CHECK_EQUAL(client_execute_pdu_r.working_dir(),
        "%%HOMEDRIVE%%%%HOMEPATH%%");
    RED_CHECK_EQUAL(client_execute_pdu_r.arguments(),
        "");
}

RED_AUTO_TEST_CASE(ClientSystemParametersUpdatePDU)
{
    StaticOutStream<128> out_stream;
    ClientSystemParametersUpdatePDU_Send client_system_parameters_update_pdu_s(out_stream, SPI_SETHIGHCONTRAST);

    InStream in_stream(buf, out_stream.get_offset());
    ClientSystemParametersUpdatePDU_Recv client_system_parameters_update_pdu_r(in_stream);

    RED_CHECK_EQUAL(client_system_parameters_update_pdu_r.SystemParam(), SPI_SETHIGHCONTRAST);
}
*/

RED_AUTO_TEST_CASE(TestProcessClientExecutePdu)
{
// (..........F
    uint8_t buf[] = {0x08, 0x00, // flags 8
                     0x1c, 0x00, // ExeOrFile Length 8
                     0x00, 0x00, // WorkingDirLength 28
                     0x00, 0x00, // Arguments Length 0
                     // |.|.W.A.B.R.e.m.o.t.e.A.p.p.
                     0x7c, 0x00, 0x7c, 0x00, 0x57, 0x00, 0x41, 0x00,
                     0x42, 0x00, 0x52, 0x00, 0x65, 0x00, 0x6d, 0x00,
                     0x6f, 0x00, 0x74, 0x00, 0x65, 0x00, 0x41, 0x00,
                     0x70, 0x00, 0x70, 0x00
                    };

    InStream chunk{buf};

    ClientExecutePDU cepdu;
    cepdu.receive(chunk);
    RED_CHECK(chunk.in_remain() == 0);

    WindowsExecuteShellParams params = cepdu.get_windows_execute_shell_params();
    RED_CHECK(params.flags == 8);
    RED_CHECK(params.exe_or_file == "||WABRemoteApp");
    RED_CHECK(params.working_dir == "");
    RED_CHECK(params.arguments == "");

    StaticOutStream<64> out;
    cepdu.emit(out);
    RED_CHECK(out.get_produced_bytes() == chunk.get_consumed_bytes());
}

RED_AUTO_TEST_CASE(TestProcessClientInformationPdu)
{
    uint8_t buf[] = {0xe5, 0x01,
                     0x00, 0x00, // order Flags 0x1e5
                     };

    InStream chunk{buf};
    ClientInformationPDU cipdu;
    cipdu.receive(chunk);
    RED_CHECK(chunk.in_remain() == 0);

    RED_CHECK(cipdu.get_flags() == (0x100
                                  | TS_RAIL_CLIENTSTATUS_HIGH_DPI_ICONS_SUPPORTED
                                  | TS_RAIL_CLIENTSTATUS_APPBAR_REMOTING_SUPPORTED
                                  | TS_RAIL_CLIENTSTATUS_POWER_DISPLAY_REQUEST_SUPPORTED
                                  | TS_RAIL_CLIENTSTATUS_ALLOWLOCALMOVESIZE
                                  // | TS_RAIL_CLIENTSTATUS_AUTORECONNECT
                                  | TS_RAIL_CLIENTSTATUS_ZORDER_SYNC));

    StaticOutStream<16> out;
    cipdu.emit(out);
    RED_CHECK(out.get_produced_bytes() == chunk.get_consumed_bytes());
}

RED_AUTO_TEST_CASE(TestHighContrastSystemInformationStructure)
{
    StaticOutStream<2048> out_stream;

    {
        HighContrastSystemInformationStructure
            high_contrast_system_information_structure(0x10101010, "ColorScheme");

        high_contrast_system_information_structure.emit(out_stream);
    }

    InStream in_stream(out_stream.get_produced_bytes());

    {
        HighContrastSystemInformationStructure
            high_contrast_system_information_structure;

        high_contrast_system_information_structure.receive(in_stream);

        RED_CHECK_EQUAL(high_contrast_system_information_structure.Flags(),
            0x10101010);
        RED_CHECK_EQUAL(high_contrast_system_information_structure.ColorScheme(),
            "ColorScheme"sv);
    }
}

RED_AUTO_TEST_CASE(TestHighContrastSystemInformationStructure1)
{
    auto reference_data =
/* 0008 */                                 "\x7f\x00\x00\x00\x28\x00\x00\x00" //         ....(...
/* 0010 */ "\x48\x00\x69\x00\x67\x00\x68\x00\x20\x00\x43\x00\x6f\x00\x6e\x00" // H.i.g.h. .C.o.n.
/* 0020 */ "\x74\x00\x72\x00\x61\x00\x73\x00\x74\x00\x20\x00\x42\x00\x6c\x00" // t.r.a.s.t. .B.l.
/* 0030 */ "\x61\x00\x63\x00\x6b\x00\x00\x00"                                 // a.c.k...
            ""_av
        ;

    StaticOutStream<2048> out_stream;

    HighContrastSystemInformationStructure
        high_contrast_system_information_structure(0x7f, "High Contrast Black");

    high_contrast_system_information_structure.emit(out_stream);

    RED_CHECK(out_stream.get_produced_bytes() == reference_data);
}

RED_AUTO_TEST_CASE(TestHighContrastSystemInformationStructure2)
{
    auto reference_data =
/* 0008 */                                 "\x7f\x00\x00\x00\x28\x00\x00\x00" //         ....(...
/* 0010 */ "\x48\x00\x69\x00\x67\x00\x68\x00\x20\x00\x43\x00\x6f\x00\x6e\x00" // H.i.g.h. .C.o.n.
/* 0020 */ "\x74\x00\x72\x00\x61\x00\x73\x00\x74\x00\x20\x00\x42\x00\x6c\x00" // t.r.a.s.t. .B.l.
/* 0030 */ "\x61\x00\x63\x00\x6b\x00\x00\x00"_av                              // a.c.k...
        ;

    InStream in_stream(reference_data);

    HighContrastSystemInformationStructure high_contrast_system_information_structure;

    high_contrast_system_information_structure.receive(in_stream);

    RED_CHECK_EQUAL(high_contrast_system_information_structure.Flags(), 0x7f);
    RED_CHECK_EQUAL(high_contrast_system_information_structure.ColorScheme(),
        "High Contrast Black"sv);
}


RED_AUTO_TEST_CASE(TestHighContrastSystemInformationStructure3)
{
    auto reference_data =
/* 0008 */                                 "\x7e\x00\x00\x00\x02\x00\x00\x00" //         ....(...
/* 0010 */ "\x00\x00"                                                         // ..
            ""_av
        ;

    StaticOutStream<2048> out_stream;

    HighContrastSystemInformationStructure
        high_contrast_system_information_structure(0x7e, "");

    high_contrast_system_information_structure.emit(out_stream);

    RED_CHECK(out_stream.get_produced_bytes() == reference_data);
}

RED_AUTO_TEST_CASE(TestHighContrastSystemInformationStructure4)
{
    auto reference_data =
/* 0008 */                                 "\x7e\x00\x00\x00\x02\x00\x00\x00" //         ....(...
/* 0010 */ "\x00\x00"                                                         // ..
            ""_av
        ;

    InStream in_stream(reference_data);

    HighContrastSystemInformationStructure
        high_contrast_system_information_structure;

    high_contrast_system_information_structure.receive(in_stream);

    RED_CHECK_EQUAL(high_contrast_system_information_structure.Flags(),
        0x7e);
    RED_CHECK_EQUAL(high_contrast_system_information_structure.ColorScheme(),
        ""sv);
}
