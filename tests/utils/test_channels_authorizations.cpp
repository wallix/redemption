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
*   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/channels_authorizations.hpp"
#include "core/RDP/channels/rdpdr.hpp"


RED_AUTO_TEST_CASE(TestChannelsAuthorizations)
{
    ChannelsAuthorizations channels_authorizations("a,b,c"_zv, "b,d"_zv);
    RED_CHECK( channels_authorizations.is_authorized(CHANNELS::ChannelNameId("a")));
    RED_CHECK(!channels_authorizations.is_authorized(CHANNELS::ChannelNameId("b")));
    RED_CHECK( channels_authorizations.is_authorized(CHANNELS::ChannelNameId("c")));
    RED_CHECK(!channels_authorizations.is_authorized(CHANNELS::ChannelNameId("d")));
    RED_CHECK(!channels_authorizations.is_authorized(CHANNELS::ChannelNameId("e")));
}

RED_AUTO_TEST_CASE(TestChannelsAuthorizations2)
{
    ChannelsAuthorizations channels_authorizations("a, b   , c"_zv, "   b,d  "_zv);
    RED_CHECK( channels_authorizations.is_authorized(CHANNELS::ChannelNameId("a")));
    RED_CHECK(!channels_authorizations.is_authorized(CHANNELS::ChannelNameId("b")));
    RED_CHECK( channels_authorizations.is_authorized(CHANNELS::ChannelNameId("c")));
    RED_CHECK(!channels_authorizations.is_authorized(CHANNELS::ChannelNameId("d")));
    RED_CHECK(!channels_authorizations.is_authorized(CHANNELS::ChannelNameId("e")));
}

RED_AUTO_TEST_CASE(TestChannelsAuthorizationsAllDeny)
{
    ChannelsAuthorizations channels_authorizations("a,b,c"_zv, "*"_zv);
    RED_CHECK( channels_authorizations.is_authorized(CHANNELS::ChannelNameId("a")));
    RED_CHECK( channels_authorizations.is_authorized(CHANNELS::ChannelNameId("b")));
    RED_CHECK( channels_authorizations.is_authorized(CHANNELS::ChannelNameId("c")));
    RED_CHECK(!channels_authorizations.is_authorized(CHANNELS::ChannelNameId("d")));
    RED_CHECK(!channels_authorizations.is_authorized(CHANNELS::ChannelNameId("d")));

    RED_CHECK(!channels_authorizations.cliprdr_up_is_authorized());
    RED_CHECK(!channels_authorizations.cliprdr_down_is_authorized());
    RED_CHECK(!channels_authorizations.cliprdr_file_is_authorized());

    RED_CHECK(!channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
}

RED_AUTO_TEST_CASE(TestChannelsAuthorizationsAllAllow)
{
    ChannelsAuthorizations channels_authorizations("*"_zv, "a,b,c"_zv);
    RED_CHECK(!channels_authorizations.is_authorized(CHANNELS::ChannelNameId("a")));
    RED_CHECK(!channels_authorizations.is_authorized(CHANNELS::ChannelNameId("b")));
    RED_CHECK(!channels_authorizations.is_authorized(CHANNELS::ChannelNameId("c")));
    RED_CHECK( channels_authorizations.is_authorized(CHANNELS::ChannelNameId("d")));

    RED_CHECK( channels_authorizations.cliprdr_up_is_authorized());
    RED_CHECK( channels_authorizations.cliprdr_down_is_authorized());
    RED_CHECK( channels_authorizations.cliprdr_file_is_authorized());

    RED_CHECK(channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
}

CHANNELS::ChannelNameId const rdpdrid("rdpdr");
CHANNELS::ChannelNameId const cliprdr("cliprdr");
CHANNELS::ChannelNameId const rdpsnd("rdpsnd");
CHANNELS::ChannelNameId const drdynvc("drdynvc");

RED_AUTO_TEST_CASE(TestChannelsAuthorizationsCliprdr)
{
    CHANNELS::ChannelNameId const d("d");

    ChannelsAuthorizations channels_authorizations("cliprdr"_zv, "*"_zv);
    RED_CHECK( channels_authorizations.is_authorized(cliprdr));
    RED_CHECK(!channels_authorizations.is_authorized(d));

    RED_CHECK(channels_authorizations.cliprdr_up_is_authorized());
    RED_CHECK(channels_authorizations.cliprdr_down_is_authorized());
    RED_CHECK(channels_authorizations.cliprdr_file_is_authorized());

    channels_authorizations = ChannelsAuthorizations("cliprdr_up,cliprdr_down"_zv, "*"_zv);
    RED_CHECK( channels_authorizations.is_authorized(cliprdr));
    RED_CHECK(!channels_authorizations.is_authorized(d));

    RED_CHECK( channels_authorizations.cliprdr_up_is_authorized());
    RED_CHECK( channels_authorizations.cliprdr_down_is_authorized());
    RED_CHECK(!channels_authorizations.cliprdr_file_is_authorized());

    channels_authorizations = ChannelsAuthorizations("cliprdr_down"_zv, "*"_zv);
    RED_CHECK( channels_authorizations.is_authorized(cliprdr));
    RED_CHECK(!channels_authorizations.is_authorized(d));

    RED_CHECK(!channels_authorizations.cliprdr_up_is_authorized());
    RED_CHECK( channels_authorizations.cliprdr_down_is_authorized());
    RED_CHECK(!channels_authorizations.cliprdr_file_is_authorized());

    channels_authorizations = ChannelsAuthorizations("cliprdr_down"_zv, "cliprdr_up"_zv);
    RED_CHECK( channels_authorizations.is_authorized(cliprdr));
    RED_CHECK(!channels_authorizations.is_authorized(d));

    RED_CHECK(!channels_authorizations.cliprdr_up_is_authorized());
    RED_CHECK( channels_authorizations.cliprdr_down_is_authorized());
    RED_CHECK(!channels_authorizations.cliprdr_file_is_authorized());

    channels_authorizations = ChannelsAuthorizations("*"_zv, "cliprdr_up"_zv);
    RED_CHECK( channels_authorizations.is_authorized(cliprdr));
    RED_CHECK( channels_authorizations.is_authorized(d));

    RED_CHECK(!channels_authorizations.cliprdr_up_is_authorized());
    RED_CHECK( channels_authorizations.cliprdr_down_is_authorized());
    RED_CHECK( channels_authorizations.cliprdr_file_is_authorized());
}

RED_AUTO_TEST_CASE(TestChannelsAuthorizationsRdpdr)
{
    ChannelsAuthorizations channels_authorizations("*"_zv, "rdpdr_printer"_zv);
    RED_CHECK(!channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
    RED_CHECK( channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM));
    RED_CHECK( channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD));
    RED_CHECK( channels_authorizations.is_authorized(rdpdrid));
    RED_CHECK( channels_authorizations.rdpdr_drive_read_is_authorized());
    RED_CHECK( channels_authorizations.rdpdr_drive_write_is_authorized());

    channels_authorizations = ChannelsAuthorizations("rdpdr_port"_zv, "rdpdr_printer"_zv);
    RED_CHECK( channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL));
    RED_CHECK( channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL));
    RED_CHECK(!channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
    RED_CHECK(!channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM));
    RED_CHECK(!channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD));
    RED_CHECK( channels_authorizations.is_authorized(rdpdrid));
    RED_CHECK(!channels_authorizations.rdpdr_drive_read_is_authorized());
    RED_CHECK(!channels_authorizations.rdpdr_drive_write_is_authorized());

    channels_authorizations = ChannelsAuthorizations("*"_zv, ""_zv);
    RED_CHECK( channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL));
    RED_CHECK( channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL));
    RED_CHECK( channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
    RED_CHECK( channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM));
    RED_CHECK( channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD));

    RED_CHECK(channels_authorizations.cliprdr_up_is_authorized());
    RED_CHECK(channels_authorizations.cliprdr_down_is_authorized());
    RED_CHECK(channels_authorizations.cliprdr_file_is_authorized());

    RED_CHECK( channels_authorizations.is_authorized(rdpdrid));
    RED_CHECK( channels_authorizations.is_authorized(cliprdr));
    RED_CHECK( channels_authorizations.rdpdr_drive_read_is_authorized());
    RED_CHECK( channels_authorizations.rdpdr_drive_write_is_authorized());
}

RED_AUTO_TEST_CASE(TestChannelsAuthorizationsAllAll)
{
    ChannelsAuthorizations channels_authorizations("*"_zv, "*"_zv);
    RED_CHECK(!channels_authorizations.is_authorized(rdpdrid));
    RED_CHECK(!channels_authorizations.is_authorized(cliprdr));
    RED_CHECK(!channels_authorizations.is_authorized(rdpsnd));
    RED_CHECK(!channels_authorizations.is_authorized(drdynvc));
    RED_CHECK(!channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL));
    RED_CHECK(!channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL));
    RED_CHECK(!channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
    RED_CHECK(!channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM));
    RED_CHECK(!channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD));

    RED_CHECK(!channels_authorizations.cliprdr_up_is_authorized());
    RED_CHECK(!channels_authorizations.cliprdr_down_is_authorized());
    RED_CHECK(!channels_authorizations.cliprdr_file_is_authorized());

    RED_CHECK(!channels_authorizations.rdpdr_drive_read_is_authorized());
    RED_CHECK(!channels_authorizations.rdpdr_drive_write_is_authorized());

    channels_authorizations = ChannelsAuthorizations("*,drdynvc"_zv, "*"_zv);
    RED_CHECK(!channels_authorizations.is_authorized(rdpdrid));
    RED_CHECK(!channels_authorizations.is_authorized(cliprdr));
    RED_CHECK(!channels_authorizations.is_authorized(rdpsnd));
    RED_CHECK( channels_authorizations.is_authorized(drdynvc));
    RED_CHECK(!channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL));
    RED_CHECK(!channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL));
    RED_CHECK(!channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
    RED_CHECK(!channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM));
    RED_CHECK(!channels_authorizations.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD));

    RED_CHECK(!channels_authorizations.cliprdr_up_is_authorized());
    RED_CHECK(!channels_authorizations.cliprdr_down_is_authorized());
    RED_CHECK(!channels_authorizations.cliprdr_file_is_authorized());

    RED_CHECK(!channels_authorizations.rdpdr_drive_read_is_authorized());
    RED_CHECK(!channels_authorizations.rdpdr_drive_write_is_authorized());
}


RED_AUTO_TEST_CASE(TestUpdateAuthorizedChannels)
{
    std::string allow;
    std::string deny;

    allow = "cliprdr,drdynvc,cliprdr_down,echo,rdpdr,rdpsnd,blah";
    deny = "rdpdr_port";
    auto result = compute_authorized_channels(allow, deny, "");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "drdynvc,echo,blah");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    allow = "cliprdr,drdynvc,cliprdr_down,rdpsnd";
    deny = "";
    result = compute_authorized_channels(allow, deny, "RDP_DRIVE");
    allow = result.first;
    deny = result.second;

    RED_CHECK_EQUAL(allow, "drdynvc,rdpdr_drive_read,rdpdr_drive_write");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    allow = "cliprdr,drdynvc,cliprdr_down,rdpsnd";
    deny = "";
    result = compute_authorized_channels(allow, deny, "RDP_DRIVE_READ");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "drdynvc,rdpdr_drive_read");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    allow = "cliprdr,drdynvc,cliprdr_down,rdpsnd";
    deny = "";
    result = compute_authorized_channels(allow, deny, "RDP_DRIVE_READ, RDP_DRIVE");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "drdynvc,rdpdr_drive_read,rdpdr_drive_write");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    allow = "";
    deny = "";
    result = compute_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "cliprdr_down");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    allow = "";
    deny = "";
    result = compute_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN,RDP_CLIPBOARD_UP");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "cliprdr_up,cliprdr_down");
    RED_CHECK_EQUAL(deny, "cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    allow = "";
    deny = "";
    result = compute_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN,RDP_CLIPBOARD_UP,RDP_CLIPBOARD_FILE");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "cliprdr_up,cliprdr_down,cliprdr_file");
    RED_CHECK_EQUAL(deny, "rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    allow = "*";
    deny = "";
    result = compute_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "*,cliprdr_down");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    allow = "rdpdr_port,tsmf";
    deny = "rdpdr";
    result = compute_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN,RDP_COM_PORT");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "tsmf,cliprdr_down,rdpdr_port");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_file,rdpdr_printer,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    allow = "*,rdpdr_port,rdpdr_port,tsmf,tsmf";
    deny = "rdpdr,rdpdr,rdpdr";
    result = compute_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN,RDP_CLIPBOARD_DOWN,RDP_CLIPBOARD_DOWN");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "*,tsmf,tsmf,cliprdr_down");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    allow = "*";
    deny = "cliprdr";
    result = compute_authorized_channels(allow, deny, "RDP_CLIPBOARD_UP,RDP_CLIPBOARD_DOWN");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "*,cliprdr_up,cliprdr_down");
    RED_CHECK_EQUAL(deny, "cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    allow = "*";
    deny = "rdpdr";
    result = compute_authorized_channels(allow, deny, "RDP_PRINTER");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "*,rdpdr_printer");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    allow = "cliprdr,rdpdr";
    deny = "*";
    result = compute_authorized_channels(allow, deny, "");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "");
    RED_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    allow = "*";
    deny = "";
    result = compute_authorized_channels(allow, deny, "");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "*");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    allow = "";
    deny = "*";
    result = compute_authorized_channels(allow, deny, "RDP_PRINTER");
    allow = result.first;
    deny = result.second;

    RED_CHECK_EQUAL(allow, "rdpdr_printer");
    RED_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    allow = "";
    deny = "*";
    result = compute_authorized_channels(allow, deny, "RDP_AUDIO_OUTPUT");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "rdpsnd_audio_output");
    RED_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpcap_audio_input");

    allow = "*";
    deny = "";
    result = compute_authorized_channels(allow, deny, "RDP_CLIPBOARD_UP,RDP_CLIPBOARD_DOWN,RDP_DRIVE");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "*,cliprdr_up,cliprdr_down,rdpdr_drive_read,rdpdr_drive_write");
    RED_CHECK_EQUAL(deny, "cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");

    ChannelsAuthorizations authorization(allow, deny);

    RED_CHECK( authorization.cliprdr_up_is_authorized());
    RED_CHECK( authorization.cliprdr_down_is_authorized());
    RED_CHECK(!authorization.cliprdr_file_is_authorized());

    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
    RED_CHECK( authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD));
    RED_CHECK( authorization.is_authorized(cliprdr));
    RED_CHECK( authorization.is_authorized(drdynvc));
    RED_CHECK( authorization.is_authorized(rdpdrid));
    RED_CHECK( authorization.is_authorized(rdpsnd));
    RED_CHECK( authorization.rdpdr_drive_read_is_authorized());
    RED_CHECK( authorization.rdpdr_drive_write_is_authorized());
    RED_CHECK(!authorization.rdpsnd_audio_output_is_authorized());
    RED_CHECK(!authorization.rdpsnd_audio_input_is_authorized());

    allow = "*";
    deny = "";
    result = compute_authorized_channels(allow, deny, "");
    allow = result.first;
    deny = result.second;
    ChannelsAuthorizations authorization1(allow, deny);

    RED_CHECK(!authorization1.cliprdr_up_is_authorized());
    RED_CHECK(!authorization1.cliprdr_down_is_authorized());
    RED_CHECK(!authorization1.cliprdr_file_is_authorized());

    RED_CHECK(!authorization1.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
    RED_CHECK(!authorization1.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM));
    RED_CHECK(!authorization1.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL));
    RED_CHECK(!authorization1.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL));
    RED_CHECK(!authorization1.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD));
    RED_CHECK(!authorization1.is_authorized(cliprdr));
    RED_CHECK( authorization1.is_authorized(drdynvc));
    RED_CHECK(!authorization1.is_authorized(rdpdrid));
    RED_CHECK(!authorization1.is_authorized(rdpsnd));
    RED_CHECK(!authorization1.rdpdr_drive_read_is_authorized());
    RED_CHECK(!authorization1.rdpdr_drive_write_is_authorized());
    RED_CHECK(!authorization1.rdpsnd_audio_output_is_authorized());
    RED_CHECK(!authorization1.rdpsnd_audio_input_is_authorized());


    allow = "*";
    deny = "";
    result = compute_authorized_channels(allow, deny, "RDP_CLIPBOARD_UP,RDP_CLIPBOARD_DOWN,RDP_CLIPBOARD_FILE,RDP_COM_PORT,RDP_DRIVE,RDP_PRINTER,RDP_SMARTCARD");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "*,cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard");
    RED_CHECK_EQUAL(deny, "rdpsnd_audio_output,rdpcap_audio_input");

    ChannelsAuthorizations authorization2(allow, deny);

    RED_CHECK(authorization2.cliprdr_up_is_authorized());
    RED_CHECK(authorization2.cliprdr_down_is_authorized());
    RED_CHECK(authorization2.cliprdr_file_is_authorized());

    RED_CHECK(authorization2.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
    RED_CHECK(authorization2.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM));
    RED_CHECK(authorization2.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL));
    RED_CHECK(authorization2.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL));
    RED_CHECK(authorization2.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD));
    RED_CHECK(authorization2.is_authorized(cliprdr));
    RED_CHECK(authorization2.is_authorized(drdynvc));
    RED_CHECK(authorization2.is_authorized(rdpdrid));
    RED_CHECK(authorization2.is_authorized(rdpsnd));
    RED_CHECK(authorization2.rdpdr_drive_read_is_authorized());
    RED_CHECK(authorization2.rdpdr_drive_write_is_authorized());
    RED_CHECK(!authorization2.rdpsnd_audio_output_is_authorized());
    RED_CHECK(!authorization2.rdpsnd_audio_input_is_authorized());


    allow = "*";
    deny = "";
    result = compute_authorized_channels(allow, deny, "RDP_AUDIO_INPUT");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "*,rdpcap_audio_input");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    ChannelsAuthorizations authorization3(allow, deny);

    RED_CHECK(!authorization3.cliprdr_up_is_authorized());
    RED_CHECK(!authorization3.cliprdr_down_is_authorized());
    RED_CHECK(!authorization3.cliprdr_file_is_authorized());

    RED_CHECK(!authorization3.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
    RED_CHECK(!authorization3.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM));
    RED_CHECK(!authorization3.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL));
    RED_CHECK(!authorization3.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL));
    RED_CHECK(!authorization3.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD));
    RED_CHECK(!authorization3.is_authorized(cliprdr));
    RED_CHECK(authorization3.is_authorized(drdynvc));
    RED_CHECK(!authorization3.is_authorized(rdpdrid));
    RED_CHECK(!authorization3.is_authorized(rdpsnd));
    RED_CHECK(!authorization3.rdpdr_drive_read_is_authorized());
    RED_CHECK(!authorization3.rdpdr_drive_write_is_authorized());
    RED_CHECK(!authorization3.rdpsnd_audio_output_is_authorized());
    RED_CHECK(authorization3.rdpsnd_audio_input_is_authorized());
}

RED_AUTO_TEST_CASE(TestUpdateAuthorizedChannels2)
{
    std::string allow = "*";
    std::string deny;
    auto result = compute_authorized_channels(allow, deny, "RDP_SMARTCARD,RDP_CLIPBOARD_UP");
    allow = result.first;
    deny = result.second;
    RED_CHECK_EQUAL(allow, "*,cliprdr_up,rdpdr_smartcard");
    RED_CHECK_EQUAL(deny, "cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpsnd_audio_output,rdpcap_audio_input");
    ChannelsAuthorizations authorization(allow, deny);

    RED_CHECK( authorization.cliprdr_up_is_authorized());
    RED_CHECK(!authorization.cliprdr_down_is_authorized());
    RED_CHECK(!authorization.cliprdr_file_is_authorized());

    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL));
    RED_CHECK( authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD));

    RED_CHECK( authorization.is_authorized(cliprdr));
    RED_CHECK( authorization.is_authorized(drdynvc));
    RED_CHECK( authorization.is_authorized(rdpdrid));
    RED_CHECK( authorization.is_authorized(rdpsnd));

    RED_CHECK(!authorization.rdpdr_drive_read_is_authorized());
    RED_CHECK(!authorization.rdpdr_drive_write_is_authorized());

    RED_CHECK(!authorization.rdpsnd_audio_output_is_authorized());
    RED_CHECK(!authorization.rdpsnd_audio_input_is_authorized());
}

RED_AUTO_TEST_CASE(TestUpdateAuthorizedChannels3)
{
    std::string allow;
    std::string deny;

    allow = "*";
    deny = "";
    auto result = compute_authorized_channels(allow, deny, "RDP_CLIPBOARD_UP");
    allow = result.first;
    deny = result.second;

    RED_CHECK_EQUAL(allow, "*,cliprdr_up");
    RED_CHECK_EQUAL(deny, "cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");
    ChannelsAuthorizations authorization(allow, deny);

    RED_CHECK( authorization.cliprdr_up_is_authorized());
    RED_CHECK(!authorization.cliprdr_down_is_authorized());
    RED_CHECK(!authorization.cliprdr_file_is_authorized());

    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD));

    RED_CHECK( authorization.is_authorized(cliprdr));
    RED_CHECK( authorization.is_authorized(drdynvc));
    RED_CHECK(!authorization.is_authorized(rdpdrid));
    RED_CHECK(!authorization.is_authorized(rdpsnd));

    RED_CHECK(!authorization.rdpdr_drive_read_is_authorized());
    RED_CHECK(!authorization.rdpdr_drive_write_is_authorized());

    RED_CHECK(!authorization.rdpsnd_audio_output_is_authorized());
}

RED_AUTO_TEST_CASE(TestUpdateAuthorizedChannels4)
{
    std::string allow;
    std::string deny;

    allow = "*";
    deny = "";
    auto result = compute_authorized_channels(allow, deny, "RDP_DRIVE_READ,RDP_DRIVE_WRITE");
    allow = result.first;
    deny = result.second;

    RED_CHECK_EQUAL(allow, "*,rdpdr_drive_read,rdpdr_drive_write");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");
    ChannelsAuthorizations authorization(allow, deny);

    RED_CHECK(!authorization.cliprdr_up_is_authorized());
    RED_CHECK(!authorization.cliprdr_down_is_authorized());
    RED_CHECK(!authorization.cliprdr_file_is_authorized());

    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
    RED_CHECK( authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD));

    RED_CHECK(!authorization.is_authorized(cliprdr));
    RED_CHECK( authorization.is_authorized(drdynvc));
    RED_CHECK( authorization.is_authorized(rdpdrid));
    RED_CHECK( authorization.is_authorized(rdpsnd));

    RED_CHECK( authorization.rdpdr_drive_read_is_authorized());
    RED_CHECK( authorization.rdpdr_drive_write_is_authorized());

    RED_CHECK(!authorization.rdpsnd_audio_output_is_authorized());
}

RED_AUTO_TEST_CASE(TestUpdateAuthorizedChannels5)
{
    std::string allow;
    std::string deny;

    allow = "";
    deny = "*";
    auto result = compute_authorized_channels(allow, deny, "RDP_DRIVE_READ,RDP_DRIVE_WRITE");
    allow = result.first;
    deny = result.second;

    RED_CHECK_EQUAL(allow, "rdpdr_drive_read,rdpdr_drive_write");
    RED_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");
    ChannelsAuthorizations authorization(allow, deny);

    RED_CHECK(!authorization.cliprdr_up_is_authorized());
    RED_CHECK(!authorization.cliprdr_down_is_authorized());
    RED_CHECK(!authorization.cliprdr_file_is_authorized());

    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
    RED_CHECK( authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD));

    RED_CHECK(!authorization.is_authorized(cliprdr));
    RED_CHECK(!authorization.is_authorized(drdynvc));
    RED_CHECK( authorization.is_authorized(rdpdrid));
    RED_CHECK( authorization.is_authorized(rdpsnd));

    RED_CHECK( authorization.rdpdr_drive_read_is_authorized());
    RED_CHECK( authorization.rdpdr_drive_write_is_authorized());

    RED_CHECK(!authorization.rdpsnd_audio_output_is_authorized());
}

RED_AUTO_TEST_CASE(TestUpdateAuthorizedChannels6)
{
    std::string allow;
    std::string deny;

    allow = "";
    deny = "*";
    auto result = compute_authorized_channels(allow, deny, "RDP_CLIPBOARD_FILE,RDP_DRIVE_READ,RDP_DRIVE_WRITE");
    allow = result.first;
    deny = result.second;

    RED_CHECK_EQUAL(allow, "cliprdr_file,rdpdr_drive_read,rdpdr_drive_write");
    RED_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output,rdpcap_audio_input");
    ChannelsAuthorizations authorization(allow, deny);

    RED_CHECK(!authorization.cliprdr_up_is_authorized());
    RED_CHECK(!authorization.cliprdr_down_is_authorized());
    RED_CHECK( authorization.cliprdr_file_is_authorized());

    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
    RED_CHECK( authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD));

    RED_CHECK(!authorization.is_authorized(cliprdr));
    RED_CHECK(!authorization.is_authorized(drdynvc));
    RED_CHECK( authorization.is_authorized(rdpdrid));
    RED_CHECK( authorization.is_authorized(rdpsnd));

    RED_CHECK( authorization.rdpdr_drive_read_is_authorized());
    RED_CHECK( authorization.rdpdr_drive_write_is_authorized());

    RED_CHECK(!authorization.rdpsnd_audio_output_is_authorized());
}

RED_AUTO_TEST_CASE(TestUpdateAuthorizedChannels7)
{
    std::string allow;
    std::string deny;

    allow = "";
    deny = "*";
    auto result = compute_authorized_channels(allow, deny, "RDP_CLIPBOARD_FILE,RDP_DRIVE_READ,RDP_DRIVE_WRITE,RDP_AUDIO_OUTPUT");
    allow = result.first;
    deny = result.second;

    RED_CHECK_EQUAL(allow, "cliprdr_file,rdpdr_drive_read,rdpdr_drive_write,rdpsnd_audio_output");
    RED_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpcap_audio_input");
    ChannelsAuthorizations authorization(allow, deny);

    RED_CHECK(!authorization.cliprdr_up_is_authorized());
    RED_CHECK(!authorization.cliprdr_down_is_authorized());
    RED_CHECK( authorization.cliprdr_file_is_authorized());

    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT));
    RED_CHECK( authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL));
    RED_CHECK(!authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD));

    RED_CHECK(!authorization.is_authorized(cliprdr));
    RED_CHECK(!authorization.is_authorized(drdynvc));
    RED_CHECK( authorization.is_authorized(rdpdrid));
    RED_CHECK( authorization.is_authorized(rdpsnd));

    RED_CHECK( authorization.rdpdr_drive_read_is_authorized());
    RED_CHECK( authorization.rdpdr_drive_write_is_authorized());

    RED_CHECK( authorization.rdpsnd_audio_output_is_authorized());
}
