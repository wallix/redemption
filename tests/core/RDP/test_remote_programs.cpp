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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRemotePrograms
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
//#define LOGPRINT

#include "log.hpp"
#include "RDP/remote_programs.hpp"

BOOST_AUTO_TEST_CASE(TestRAILPDUHeader)
{
    BStream stream(128);

    RAILPDUHeader_Send header_s(stream, TS_RAIL_ORDER_EXEC);
    header_s.set_orderLength(24);

    stream.rewind();

    RAILPDUHeader_Recv header_r(stream);

    BOOST_CHECK_EQUAL(header_r.orderType(),   TS_RAIL_ORDER_EXEC);
    BOOST_CHECK_EQUAL(header_r.orderLength(), 24                );
}

BOOST_AUTO_TEST_CASE(TestHandshakePDU)
{
    BStream stream(128);

    HandshakePDU_Send handshake_pdu_s(stream, 0x01020304);

    stream.rewind();

    HandshakePDU_Recv handshake_pdu_r(stream);

    BOOST_CHECK_EQUAL(handshake_pdu_r.buildNumber(), 0x01020304);
}

BOOST_AUTO_TEST_CASE(ClientExecutePDU)
{
    BStream stream(2048);

    ClientExecutePDU_Send client_execute_pdu_s(stream,
        TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY,
        "%%SystemRoot%%\\system32\\notepad.exe", "%%HOMEDRIVE%%%%HOMEPATH%%",
        "");

    stream.rewind();

    ClientExecutePDU_Recv client_execute_pdu_r(stream);

    BOOST_CHECK_EQUAL(client_execute_pdu_r.Flags(),
        TS_RAIL_EXEC_FLAG_EXPAND_WORKINGDIRECTORY);
    BOOST_CHECK_EQUAL(client_execute_pdu_r.exe_or_file(),
        "%%SystemRoot%%\\system32\\notepad.exe");
    BOOST_CHECK_EQUAL(client_execute_pdu_r.working_dir(),
        "%%HOMEDRIVE%%%%HOMEPATH%%");
    BOOST_CHECK_EQUAL(client_execute_pdu_r.arguments(),
        "");
}

BOOST_AUTO_TEST_CASE(ClientSystemParametersUpdatePDU)
{
    BStream stream(128);

    ClientSystemParametersUpdatePDU_Send client_system_parameters_update_pdu_s(stream, SPI_SETHIGHCONTRAST);

    stream.rewind();

    ClientSystemParametersUpdatePDU_Recv client_system_parameters_update_pdu_r(stream);

    BOOST_CHECK_EQUAL(client_system_parameters_update_pdu_r.SystemParam(), SPI_SETHIGHCONTRAST);
}

BOOST_AUTO_TEST_CASE(HighContrastSystemInformationStructure)
{
    BStream stream(2048);

    HighContrastSystemInformationStructure_Send high_contrast_system_information_structure_s(
        stream, 0x10101010, "ColorScheme");

    stream.rewind();

    HighContrastSystemInformationStructure_Recv high_contrast_system_information_structure_r(
        stream);

    BOOST_CHECK_EQUAL(high_contrast_system_information_structure_r.Flags(),
        0x10101010);
    BOOST_CHECK_EQUAL(high_contrast_system_information_structure_r.ColorScheme(),
        "ColorScheme");
}