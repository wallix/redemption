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

#define RED_TEST_MODULE TestRemotePrograms
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "utils/log.hpp"
#include "core/RDP/remote_programs.hpp"

/*
RED_AUTO_TEST_CASE(TestRAILPDUHeader)
{
    uint8_t buf[128];

    OutStream out_stream(buf);

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
    uint8_t buf[128];

    OutStream out_stream(buf);
    HandshakePDU_Send handshake_pdu_s(out_stream, 0x01020304);

    InStream in_stream(buf, out_stream.get_offset());
    HandshakePDU_Recv handshake_pdu_r(in_stream);

    RED_CHECK_EQUAL(handshake_pdu_r.buildNumber(), 0x01020304);
}

RED_AUTO_TEST_CASE(ClientExecutePDU)
{
    uint8_t buf[2048];

    OutStream out_stream(buf);
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
    uint8_t buf[128];

    OutStream out_stream(buf);
    ClientSystemParametersUpdatePDU_Send client_system_parameters_update_pdu_s(out_stream, SPI_SETHIGHCONTRAST);

    InStream in_stream(buf, out_stream.get_offset());
    ClientSystemParametersUpdatePDU_Recv client_system_parameters_update_pdu_r(in_stream);

    RED_CHECK_EQUAL(client_system_parameters_update_pdu_r.SystemParam(), SPI_SETHIGHCONTRAST);
}
*/

RED_AUTO_TEST_CASE(TestHighContrastSystemInformationStructure)
{
    uint8_t buf[2048];
    OutStream out_stream(buf);

    {
        HighContrastSystemInformationStructure
            high_contrast_system_information_structure(0x10101010, "ColorScheme");

        high_contrast_system_information_structure.emit(out_stream);
    }

    InStream in_stream(buf, out_stream.get_offset());

    {
        HighContrastSystemInformationStructure
            high_contrast_system_information_structure;

        high_contrast_system_information_structure.receive(in_stream);

        RED_CHECK_EQUAL(high_contrast_system_information_structure.Flags(),
            0x10101010);
        RED_CHECK_EQUAL(high_contrast_system_information_structure.ColorScheme(),
            "ColorScheme");
    }
}
