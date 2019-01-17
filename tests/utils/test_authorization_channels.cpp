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

#define RED_TEST_MODULE TestAuthorizationChannels
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "utils/authorization_channels.hpp"

RED_AUTO_TEST_CASE(TestAuthorizationChannels)
{
    AuthorizationChannels authorization_channels("a,b,c", "b,d");
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("a")), true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("b")), false);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("c")), true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("d")), false);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("e")), false);
}

RED_AUTO_TEST_CASE(TestAuthorizationChannels2)
{
    AuthorizationChannels authorization_channels("a, b   , c", "   b,d  ");
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("a")), true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("b")), false);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("c")), true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("d")), false);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("e")), false);
}

RED_AUTO_TEST_CASE(TestAuthorizationChannelsAllDeny)
{
    AuthorizationChannels authorization_channels("a,b,c", "*");
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("a")), true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("b")), true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("c")), true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("d")), false);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("d")), false);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
}

RED_AUTO_TEST_CASE(TestAuthorizationChannelsAllAllow)
{
    AuthorizationChannels authorization_channels("*", "a,b,c");
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("a")), !true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("b")), !true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("c")), !true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(CHANNELS::ChannelNameId("d")), !false);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), !false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), !false);
}

CHANNELS::ChannelNameId const rdpdrid("rdpdr");
CHANNELS::ChannelNameId const cliprdr("cliprdr");
CHANNELS::ChannelNameId const rdpsnd("rdpsnd");
CHANNELS::ChannelNameId const drdynvc("drdynvc");

RED_AUTO_TEST_CASE(TestAuthorizationChannelsCliprdr)
{
    CHANNELS::ChannelNameId const d("d");

    AuthorizationChannels authorization_channels("cliprdr", "*");
    RED_CHECK_EQUAL(authorization_channels.is_authorized(cliprdr), true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(d), false);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), true);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), true);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_file_is_authorized(), true);

    authorization_channels = AuthorizationChannels("cliprdr_up,cliprdr_down", "*");
    RED_CHECK_EQUAL(authorization_channels.is_authorized(cliprdr), true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(d), false);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), true);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), true);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_file_is_authorized(), false);

    authorization_channels = AuthorizationChannels("cliprdr_down", "*");
    RED_CHECK_EQUAL(authorization_channels.is_authorized(cliprdr), true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(d), false);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), true);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), false);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_file_is_authorized(), false);

    authorization_channels = AuthorizationChannels("cliprdr_down", "cliprdr_up");
    RED_CHECK_EQUAL(authorization_channels.is_authorized(cliprdr), true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(d), false);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), true);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), false);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_file_is_authorized(), false);

    authorization_channels = AuthorizationChannels("*", "cliprdr_up");
    RED_CHECK_EQUAL(authorization_channels.is_authorized(cliprdr), true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(d), true);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), true);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), false);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_file_is_authorized(), true);
}

RED_AUTO_TEST_CASE(TestAuthorizationChannelsRdpdr)
{
    AuthorizationChannels authorization_channels("*", "rdpdr_printer");
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), true);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(rdpdrid), true);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_drive_read_is_authorized(), true);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_drive_write_is_authorized(), true);

    authorization_channels = AuthorizationChannels("rdpdr_port", "rdpdr_printer");
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), true);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), true);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(rdpdrid), true);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_drive_read_is_authorized(), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_drive_write_is_authorized(), false);

    authorization_channels = AuthorizationChannels("*", "");
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), true);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), true);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), true);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), true);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), true);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(rdpdrid), true);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(cliprdr), true);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_drive_read_is_authorized(), true);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_drive_write_is_authorized(), true);
}

RED_AUTO_TEST_CASE(TestAuthorizationChannelsAllAll)
{
    AuthorizationChannels authorization_channels("*", "*");
    RED_CHECK_EQUAL(authorization_channels.is_authorized(rdpdrid), false);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(cliprdr), false);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(rdpsnd), false);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(drdynvc), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), false);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), false);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_file_is_authorized(), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_drive_read_is_authorized(), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_drive_write_is_authorized(), false);

    authorization_channels = AuthorizationChannels("*,drdynvc", "*");
    RED_CHECK_EQUAL(authorization_channels.is_authorized(rdpdrid), false);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(cliprdr), false);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(rdpsnd), false);
    RED_CHECK_EQUAL(authorization_channels.is_authorized(drdynvc), true);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), false);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), false);
    RED_CHECK_EQUAL(authorization_channels.cliprdr_file_is_authorized(), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_drive_read_is_authorized(), false);
    RED_CHECK_EQUAL(authorization_channels.rdpdr_drive_write_is_authorized(), false);
}

RED_AUTO_TEST_CASE(TestUpdateAuthorizedChannels)
{
    std::string allow;
    std::string deny;

    allow = "cliprdr,drdynvc,cliprdr_down,echo,rdpdr,rdpsnd,blah";
    deny = "rdpdr_port";
    AuthorizationChannels::update_authorized_channels(allow, deny, "");
    RED_CHECK_EQUAL(allow, "drdynvc,echo,blah");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "cliprdr,drdynvc,cliprdr_down,rdpsnd";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_DRIVE");
    RED_CHECK_EQUAL(allow, "drdynvc,rdpdr_drive_read,rdpdr_drive_write");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "cliprdr,drdynvc,cliprdr_down,rdpsnd";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_DRIVE_READ");
    RED_CHECK_EQUAL(allow, "drdynvc,rdpdr_drive_read");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "cliprdr,drdynvc,cliprdr_down,rdpsnd";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_DRIVE_READ, RDP_DRIVE");
    RED_CHECK_EQUAL(allow, "drdynvc,rdpdr_drive_read,rdpdr_drive_write");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN");
    RED_CHECK_EQUAL(allow, "cliprdr_down");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN,RDP_CLIPBOARD_UP");
    RED_CHECK_EQUAL(allow, "cliprdr_up,cliprdr_down");
    RED_CHECK_EQUAL(deny, "cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN,RDP_CLIPBOARD_UP,RDP_CLIPBOARD_FILE");
    RED_CHECK_EQUAL(allow, "cliprdr_up,cliprdr_down,cliprdr_file");
    RED_CHECK_EQUAL(deny, "rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "*";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN");
    RED_CHECK_EQUAL(allow, "*,cliprdr_down");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "rdpdr_port,tsmf";
    deny = "rdpdr";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN,RDP_COM_PORT");
    RED_CHECK_EQUAL(allow, "tsmf,cliprdr_down,rdpdr_port");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_file,rdpdr_printer,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "*,rdpdr_port,rdpdr_port,tsmf,tsmf";
    deny = "rdpdr,rdpdr,rdpdr";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN,RDP_CLIPBOARD_DOWN,RDP_CLIPBOARD_DOWN");
    RED_CHECK_EQUAL(allow, "*,tsmf,tsmf,cliprdr_down");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "*";
    deny = "cliprdr";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_UP,RDP_CLIPBOARD_DOWN");
    RED_CHECK_EQUAL(allow, "*,cliprdr_up,cliprdr_down");
    RED_CHECK_EQUAL(deny, "cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "*";
    deny = "rdpdr";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_PRINTER");
    RED_CHECK_EQUAL(allow, "*,rdpdr_printer");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "cliprdr,rdpdr";
    deny = "*";
    AuthorizationChannels::update_authorized_channels(allow, deny, "");
    RED_CHECK_EQUAL(allow, "");
    RED_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "*";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "");
    RED_CHECK_EQUAL(allow, "*");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "";
    deny = "*";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_PRINTER");
    RED_CHECK_EQUAL(allow, "rdpdr_printer");
    RED_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "";
    deny = "*";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_AUDIO_OUTPUT");
    RED_CHECK_EQUAL(allow, "rdpsnd_audio_output");
    RED_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard");

    allow = "*";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_UP,RDP_CLIPBOARD_DOWN,RDP_DRIVE");
    AuthorizationChannels authorization(allow, deny);
    RED_CHECK_EQUAL(authorization.cliprdr_down_is_authorized(), true);
    RED_CHECK_EQUAL(authorization.cliprdr_up_is_authorized(), true);
    RED_CHECK_EQUAL(authorization.cliprdr_file_is_authorized(), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), true);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);
    RED_CHECK_EQUAL(authorization.is_authorized(cliprdr), true);
    RED_CHECK_EQUAL(authorization.is_authorized(drdynvc), true);
    RED_CHECK_EQUAL(authorization.is_authorized(rdpdrid), true);
    RED_CHECK_EQUAL(authorization.is_authorized(rdpsnd), true);
    RED_CHECK_EQUAL(authorization.rdpdr_drive_read_is_authorized(), true);
    RED_CHECK_EQUAL(authorization.rdpdr_drive_write_is_authorized(), true);
    RED_CHECK_EQUAL(authorization.rdpsnd_audio_output_is_authorized(), false);


    allow = "*";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "");
    AuthorizationChannels authorization1(allow, deny);
    RED_CHECK_EQUAL(authorization1.cliprdr_down_is_authorized(), false);
    RED_CHECK_EQUAL(authorization1.cliprdr_up_is_authorized(), false);
    RED_CHECK_EQUAL(authorization1.cliprdr_file_is_authorized(), false);
    RED_CHECK_EQUAL(authorization1.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    RED_CHECK_EQUAL(authorization1.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), false);
    RED_CHECK_EQUAL(authorization1.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    RED_CHECK_EQUAL(authorization1.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    RED_CHECK_EQUAL(authorization1.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);
    RED_CHECK_EQUAL(authorization1.is_authorized(cliprdr), false);
    RED_CHECK_EQUAL(authorization1.is_authorized(drdynvc), true);
    RED_CHECK_EQUAL(authorization1.is_authorized(rdpdrid), false);
    RED_CHECK_EQUAL(authorization1.is_authorized(rdpsnd), false);
    RED_CHECK_EQUAL(authorization1.rdpdr_drive_read_is_authorized(), false);
    RED_CHECK_EQUAL(authorization1.rdpdr_drive_write_is_authorized(), false);
    RED_CHECK_EQUAL(authorization1.rdpsnd_audio_output_is_authorized(), false);
}

RED_AUTO_TEST_CASE(TestUpdateAuthorizedChannels2)
{
    std::string allow;
    std::string deny;

    allow = "*";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_SMARTCARD,RDP_CLIPBOARD_UP");
    RED_CHECK_EQUAL(allow, "*,cliprdr_up,rdpdr_smartcard");
    RED_CHECK_EQUAL(deny, "cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpsnd_audio_output");
    AuthorizationChannels authorization(allow, deny);
    RED_CHECK_EQUAL(authorization.cliprdr_down_is_authorized(), false);
    RED_CHECK_EQUAL(authorization.cliprdr_up_is_authorized(), true);
    RED_CHECK_EQUAL(authorization.cliprdr_file_is_authorized(), false);

    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), true);

    RED_CHECK_EQUAL(authorization.is_authorized(cliprdr), true);
    RED_CHECK_EQUAL(authorization.is_authorized(drdynvc), true);
    RED_CHECK_EQUAL(authorization.is_authorized(rdpdrid), true);
    RED_CHECK_EQUAL(authorization.is_authorized(rdpsnd), true);

    RED_CHECK_EQUAL(authorization.rdpdr_drive_read_is_authorized(), false);
    RED_CHECK_EQUAL(authorization.rdpdr_drive_write_is_authorized(), false);

    RED_CHECK_EQUAL(authorization.rdpsnd_audio_output_is_authorized(), false);
}

RED_AUTO_TEST_CASE(TestUpdateAuthorizedChannels3)
{
    std::string allow;
    std::string deny;

    allow = "*";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_UP");
    RED_CHECK_EQUAL(allow, "*,cliprdr_up");
    RED_CHECK_EQUAL(deny, "cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");
    AuthorizationChannels authorization(allow, deny);
    RED_CHECK_EQUAL(authorization.cliprdr_down_is_authorized(), false);
    RED_CHECK_EQUAL(authorization.cliprdr_up_is_authorized(), true);
    RED_CHECK_EQUAL(authorization.cliprdr_file_is_authorized(), false);

    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);

    RED_CHECK_EQUAL(authorization.is_authorized(cliprdr), true);
    RED_CHECK_EQUAL(authorization.is_authorized(drdynvc), true);
    RED_CHECK_EQUAL(authorization.is_authorized(rdpdrid), false);
    RED_CHECK_EQUAL(authorization.is_authorized(rdpsnd), false);

    RED_CHECK_EQUAL(authorization.rdpdr_drive_read_is_authorized(), false);
    RED_CHECK_EQUAL(authorization.rdpdr_drive_write_is_authorized(), false);

    RED_CHECK_EQUAL(authorization.rdpsnd_audio_output_is_authorized(), false);
}

RED_AUTO_TEST_CASE(TestUpdateAuthorizedChannels4)
{
    std::string allow;
    std::string deny;

    allow = "*";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_DRIVE_READ,RDP_DRIVE_WRITE");
    RED_CHECK_EQUAL(allow, "*,rdpdr_drive_read,rdpdr_drive_write");
    RED_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output");
    AuthorizationChannels authorization(allow, deny);
    RED_CHECK_EQUAL(authorization.cliprdr_down_is_authorized(), false);
    RED_CHECK_EQUAL(authorization.cliprdr_up_is_authorized(), false);
    RED_CHECK_EQUAL(authorization.cliprdr_file_is_authorized(), false);

    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), true);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);

    RED_CHECK_EQUAL(authorization.is_authorized(cliprdr), false);
    RED_CHECK_EQUAL(authorization.is_authorized(drdynvc), true);
    RED_CHECK_EQUAL(authorization.is_authorized(rdpdrid), true);
    RED_CHECK_EQUAL(authorization.is_authorized(rdpsnd), true);

    RED_CHECK_EQUAL(authorization.rdpdr_drive_read_is_authorized(), true);
    RED_CHECK_EQUAL(authorization.rdpdr_drive_write_is_authorized(), true);

    RED_CHECK_EQUAL(authorization.rdpsnd_audio_output_is_authorized(), false);
}

RED_AUTO_TEST_CASE(TestUpdateAuthorizedChannels5)
{
    std::string allow;
    std::string deny;

    allow = "";
    deny = "*";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_DRIVE_READ,RDP_DRIVE_WRITE");
    RED_CHECK_EQUAL(allow, "rdpdr_drive_read,rdpdr_drive_write");
    RED_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output");
    AuthorizationChannels authorization(allow, deny);
    RED_CHECK_EQUAL(authorization.cliprdr_down_is_authorized(), false);
    RED_CHECK_EQUAL(authorization.cliprdr_up_is_authorized(), false);
    RED_CHECK_EQUAL(authorization.cliprdr_file_is_authorized(), false);

    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), true);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);

    RED_CHECK_EQUAL(authorization.is_authorized(cliprdr), false);
    RED_CHECK_EQUAL(authorization.is_authorized(drdynvc), false);
    RED_CHECK_EQUAL(authorization.is_authorized(rdpdrid), true);
    RED_CHECK_EQUAL(authorization.is_authorized(rdpsnd), true);

    RED_CHECK_EQUAL(authorization.rdpdr_drive_read_is_authorized(), true);
    RED_CHECK_EQUAL(authorization.rdpdr_drive_write_is_authorized(), true);

    RED_CHECK_EQUAL(authorization.rdpsnd_audio_output_is_authorized(), false);
}

RED_AUTO_TEST_CASE(TestUpdateAuthorizedChannels6)
{
    std::string allow;
    std::string deny;

    allow = "";
    deny = "*";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_FILE,RDP_DRIVE_READ,RDP_DRIVE_WRITE");
    RED_CHECK_EQUAL(allow, "cliprdr_file,rdpdr_drive_read,rdpdr_drive_write");
    RED_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output");
    AuthorizationChannels authorization(allow, deny);
    RED_CHECK_EQUAL(authorization.cliprdr_down_is_authorized(), false);
    RED_CHECK_EQUAL(authorization.cliprdr_up_is_authorized(), false);
    RED_CHECK_EQUAL(authorization.cliprdr_file_is_authorized(), true);

    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), true);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);

    RED_CHECK_EQUAL(authorization.is_authorized(cliprdr), false);
    RED_CHECK_EQUAL(authorization.is_authorized(drdynvc), false);
    RED_CHECK_EQUAL(authorization.is_authorized(rdpdrid), true);
    RED_CHECK_EQUAL(authorization.is_authorized(rdpsnd), true);

    RED_CHECK_EQUAL(authorization.rdpdr_drive_read_is_authorized(), true);
    RED_CHECK_EQUAL(authorization.rdpdr_drive_write_is_authorized(), true);

    RED_CHECK_EQUAL(authorization.rdpsnd_audio_output_is_authorized(), false);
}

RED_AUTO_TEST_CASE(TestUpdateAuthorizedChannels7)
{
    std::string allow;
    std::string deny;

    allow = "";
    deny = "*";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_FILE,RDP_DRIVE_READ,RDP_DRIVE_WRITE,RDP_AUDIO_OUTPUT");
    RED_CHECK_EQUAL(allow, "cliprdr_file,rdpdr_drive_read,rdpdr_drive_write,rdpsnd_audio_output");
    RED_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,rdpdr_printer,rdpdr_port,rdpdr_smartcard");
    AuthorizationChannels authorization(allow, deny);
    RED_CHECK_EQUAL(authorization.cliprdr_down_is_authorized(), false);
    RED_CHECK_EQUAL(authorization.cliprdr_up_is_authorized(), false);
    RED_CHECK_EQUAL(authorization.cliprdr_file_is_authorized(), true);

    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), true);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    RED_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);

    RED_CHECK_EQUAL(authorization.is_authorized(cliprdr), false);
    RED_CHECK_EQUAL(authorization.is_authorized(drdynvc), false);
    RED_CHECK_EQUAL(authorization.is_authorized(rdpdrid), true);
    RED_CHECK_EQUAL(authorization.is_authorized(rdpsnd), true);

    RED_CHECK_EQUAL(authorization.rdpdr_drive_read_is_authorized(), true);
    RED_CHECK_EQUAL(authorization.rdpdr_drive_write_is_authorized(), true);

    RED_CHECK_EQUAL(authorization.rdpsnd_audio_output_is_authorized(), true);
}
