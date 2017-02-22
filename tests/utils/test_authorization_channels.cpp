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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestAuthorizationChannels
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT

#include "utils/authorization_channels.hpp"

BOOST_AUTO_TEST_CASE(TestAuthorizationChannels)
{
    AuthorizationChannels authorization_channels("a,b,c", "b,d");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("a"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("b"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("c"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("e"), false);
}

BOOST_AUTO_TEST_CASE(TestAuthorizationChannels2)
{
    AuthorizationChannels authorization_channels("a, b   , c", "   b,d  ");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("a"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("b"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("c"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("e"), false);
}

BOOST_AUTO_TEST_CASE(TestAuthorizationChannelsAllDeny)
{
    AuthorizationChannels authorization_channels("a,b,c", "*");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("a"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("b"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("c"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
}

BOOST_AUTO_TEST_CASE(TestAuthorizationChannelsAllAllow)
{
    AuthorizationChannels authorization_channels("*", "a,b,c");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("a"), !true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("b"), !true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("c"), !true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), !false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), !false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), !false);
}

BOOST_AUTO_TEST_CASE(TestAuthorizationChannelsCliprdr)
{
    AuthorizationChannels authorization_channels("cliprdr", "*");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_up"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_down"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_file_is_authorized(), true);

    authorization_channels = AuthorizationChannels("cliprdr_up,cliprdr_down", "*");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_up"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_down"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_file_is_authorized(), false);

    authorization_channels = AuthorizationChannels("cliprdr_down", "*");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_up"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_down"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_file_is_authorized(), false);

    authorization_channels = AuthorizationChannels("cliprdr_down", "cliprdr_up");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_up"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_down"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_file_is_authorized(), false);

    authorization_channels = AuthorizationChannels("*", "cliprdr_up");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_up"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr_down"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("d"), true);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_file_is_authorized(), true);
}

BOOST_AUTO_TEST_CASE(TestAuthorizationChannelsRdpdr)
{
    AuthorizationChannels authorization_channels("*", "rdpdr_printer");
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), true);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("rdpdr"), true);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_drive_read_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_drive_write_is_authorized(), true);

    authorization_channels = AuthorizationChannels("rdpdr_port", "rdpdr_printer");
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), true);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), true);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("rdpdr"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("rdpdr_port"), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_drive_read_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_drive_write_is_authorized(), false);

    authorization_channels = AuthorizationChannels("*", "");
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), true);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), true);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), true);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), true);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), true);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("rdpdr"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr"), true);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("rdpdr_port"), true);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_drive_read_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_drive_write_is_authorized(), true);
}

BOOST_AUTO_TEST_CASE(TestAuthorizationChannelsAllAll)
{
    AuthorizationChannels authorization_channels("*", "*");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("rdpdr"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("rdpsnd"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("drdynvc"), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_file_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_drive_read_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_drive_write_is_authorized(), false);

    authorization_channels = AuthorizationChannels("*,drdynvc", "*");
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("rdpdr"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("cliprdr"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("rdpsnd"), false);
    BOOST_CHECK_EQUAL(authorization_channels.is_authorized("drdynvc"), true);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_up_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_down_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization_channels.cliprdr_file_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_drive_read_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization_channels.rdpdr_drive_write_is_authorized(), false);
}

BOOST_AUTO_TEST_CASE(TestUpdateAuthorizedChannels)
{
    std::string allow;
    std::string deny;

    allow = "cliprdr,drdynvc,cliprdr_down,echo,rdpdr,rdpsnd,blah";
    deny = "rdpdr_port";
    AuthorizationChannels::update_authorized_channels(allow, deny, "");
    BOOST_CHECK_EQUAL(allow, "drdynvc,echo,blah");
    BOOST_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "cliprdr,drdynvc,cliprdr_down,rdpsnd";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_DRIVE");
    BOOST_CHECK_EQUAL(allow, "drdynvc,rdpdr_drive_read,rdpdr_drive_write");
    BOOST_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "cliprdr,drdynvc,cliprdr_down,rdpsnd";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_DRIVE_READ");
    BOOST_CHECK_EQUAL(allow, "drdynvc,rdpdr_drive_read");
    BOOST_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "cliprdr,drdynvc,cliprdr_down,rdpsnd";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_DRIVE_READ, RDP_DRIVE");
    BOOST_CHECK_EQUAL(allow, "drdynvc,rdpdr_drive_read,rdpdr_drive_write");
    BOOST_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN");
    BOOST_CHECK_EQUAL(allow, "cliprdr_down");
    BOOST_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN,RDP_CLIPBOARD_UP");
    BOOST_CHECK_EQUAL(allow, "cliprdr_up,cliprdr_down");
    BOOST_CHECK_EQUAL(deny, "cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN,RDP_CLIPBOARD_UP,RDP_CLIPBOARD_FILE");
    BOOST_CHECK_EQUAL(allow, "cliprdr_up,cliprdr_down,cliprdr_file");
    BOOST_CHECK_EQUAL(deny, "rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "*";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN");
    BOOST_CHECK_EQUAL(allow, "*,cliprdr_down");
    BOOST_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "rdpdr_port,tsmf";
    deny = "rdpdr";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN,RDP_COM_PORT");
    BOOST_CHECK_EQUAL(allow, "tsmf,cliprdr_down,rdpdr_port");
    BOOST_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_file,rdpdr_printer,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "*,rdpdr_port,rdpdr_port,tsmf,tsmf";
    deny = "rdpdr,rdpdr,rdpdr";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_DOWN,RDP_CLIPBOARD_DOWN,RDP_CLIPBOARD_DOWN");
    BOOST_CHECK_EQUAL(allow, "*,tsmf,tsmf,cliprdr_down");
    BOOST_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "*";
    deny = "cliprdr";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_UP,RDP_CLIPBOARD_DOWN");
    BOOST_CHECK_EQUAL(allow, "*,cliprdr_up,cliprdr_down");
    BOOST_CHECK_EQUAL(deny, "cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "*";
    deny = "rdpdr";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_PRINTER");
    BOOST_CHECK_EQUAL(allow, "*,rdpdr_printer");
    BOOST_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "cliprdr,rdpdr";
    deny = "*";
    AuthorizationChannels::update_authorized_channels(allow, deny, "");
    BOOST_CHECK_EQUAL(allow, "");
    BOOST_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "*";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "");
    BOOST_CHECK_EQUAL(allow, "*");
    BOOST_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "";
    deny = "*";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_PRINTER");
    BOOST_CHECK_EQUAL(allow, "rdpdr_printer");
    BOOST_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");

    allow = "";
    deny = "*";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_AUDIO_OUTPUT");
    BOOST_CHECK_EQUAL(allow, "rdpsnd_audio_output");
    BOOST_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard");

    allow = "*";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_UP,RDP_CLIPBOARD_DOWN,RDP_DRIVE");
    AuthorizationChannels authorization(allow, deny);
    BOOST_CHECK_EQUAL(authorization.cliprdr_down_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization.cliprdr_up_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization.cliprdr_file_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), true);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);
    BOOST_CHECK_EQUAL(authorization.is_authorized("cliprdr"), true);
    BOOST_CHECK_EQUAL(authorization.is_authorized("drdynvc"), true);
    BOOST_CHECK_EQUAL(authorization.is_authorized("rdpdr"), true);
    BOOST_CHECK_EQUAL(authorization.is_authorized("rdpsnd"), true);
    BOOST_CHECK_EQUAL(authorization.rdpdr_drive_read_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization.rdpdr_drive_write_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization.rdpsnd_audio_output_is_authorized(), false);


    allow = "*";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "");
    AuthorizationChannels authorization1(allow, deny);
    BOOST_CHECK_EQUAL(authorization1.cliprdr_down_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization1.cliprdr_up_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization1.cliprdr_file_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization1.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    BOOST_CHECK_EQUAL(authorization1.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), false);
    BOOST_CHECK_EQUAL(authorization1.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    BOOST_CHECK_EQUAL(authorization1.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    BOOST_CHECK_EQUAL(authorization1.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);
    BOOST_CHECK_EQUAL(authorization1.is_authorized("cliprdr"), false);
    BOOST_CHECK_EQUAL(authorization1.is_authorized("drdynvc"), true);
    BOOST_CHECK_EQUAL(authorization1.is_authorized("rdpdr"), false);
    BOOST_CHECK_EQUAL(authorization1.is_authorized("rdpsnd"), false);
    BOOST_CHECK_EQUAL(authorization1.rdpdr_drive_read_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization1.rdpdr_drive_write_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization1.rdpsnd_audio_output_is_authorized(), false);
}

BOOST_AUTO_TEST_CASE(TestUpdateAuthorizedChannels2)
{
    std::string allow;
    std::string deny;

    allow = "*";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_SMARTCARD,RDP_CLIPBOARD_UP");
    BOOST_CHECK_EQUAL(allow, "*,cliprdr_up,rdpdr_smartcard");
    BOOST_CHECK_EQUAL(deny, "cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpsnd_audio_output");
    AuthorizationChannels authorization(allow, deny);
    BOOST_CHECK_EQUAL(authorization.cliprdr_down_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization.cliprdr_up_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization.cliprdr_file_is_authorized(), false);

    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), true);

    BOOST_CHECK_EQUAL(authorization.is_authorized("cliprdr"), true);
    BOOST_CHECK_EQUAL(authorization.is_authorized("drdynvc"), true);
    BOOST_CHECK_EQUAL(authorization.is_authorized("rdpdr"), true);
    BOOST_CHECK_EQUAL(authorization.is_authorized("rdpsnd"), true);

    BOOST_CHECK_EQUAL(authorization.rdpdr_drive_read_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_drive_write_is_authorized(), false);

    BOOST_CHECK_EQUAL(authorization.rdpsnd_audio_output_is_authorized(), false);
}

BOOST_AUTO_TEST_CASE(TestUpdateAuthorizedChannels3)
{
    std::string allow;
    std::string deny;

    allow = "*";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_UP");
    BOOST_CHECK_EQUAL(allow, "*,cliprdr_up");
    BOOST_CHECK_EQUAL(deny, "cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_drive_read,rdpdr_drive_write,rdpdr_smartcard,rdpsnd_audio_output");
    AuthorizationChannels authorization(allow, deny);
    BOOST_CHECK_EQUAL(authorization.cliprdr_down_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization.cliprdr_up_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization.cliprdr_file_is_authorized(), false);

    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);

    BOOST_CHECK_EQUAL(authorization.is_authorized("cliprdr"), true);
    BOOST_CHECK_EQUAL(authorization.is_authorized("drdynvc"), true);
    BOOST_CHECK_EQUAL(authorization.is_authorized("rdpdr"), false);
    BOOST_CHECK_EQUAL(authorization.is_authorized("rdpsnd"), false);

    BOOST_CHECK_EQUAL(authorization.rdpdr_drive_read_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_drive_write_is_authorized(), false);

    BOOST_CHECK_EQUAL(authorization.rdpsnd_audio_output_is_authorized(), false);
}

BOOST_AUTO_TEST_CASE(TestUpdateAuthorizedChannels4)
{
    std::string allow;
    std::string deny;

    allow = "*";
    deny = "";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_DRIVE_READ,RDP_DRIVE_WRITE");
    BOOST_CHECK_EQUAL(allow, "*,rdpdr_drive_read,rdpdr_drive_write");
    BOOST_CHECK_EQUAL(deny, "cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output");
    AuthorizationChannels authorization(allow, deny);
    BOOST_CHECK_EQUAL(authorization.cliprdr_down_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization.cliprdr_up_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization.cliprdr_file_is_authorized(), false);

    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), true);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);

    BOOST_CHECK_EQUAL(authorization.is_authorized("cliprdr"), false);
    BOOST_CHECK_EQUAL(authorization.is_authorized("drdynvc"), true);
    BOOST_CHECK_EQUAL(authorization.is_authorized("rdpdr"), true);
    BOOST_CHECK_EQUAL(authorization.is_authorized("rdpsnd"), true);

    BOOST_CHECK_EQUAL(authorization.rdpdr_drive_read_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization.rdpdr_drive_write_is_authorized(), true);

    BOOST_CHECK_EQUAL(authorization.rdpsnd_audio_output_is_authorized(), false);
}

BOOST_AUTO_TEST_CASE(TestUpdateAuthorizedChannels5)
{
    std::string allow;
    std::string deny;

    allow = "";
    deny = "*";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_DRIVE_READ,RDP_DRIVE_WRITE");
    BOOST_CHECK_EQUAL(allow, "rdpdr_drive_read,rdpdr_drive_write");
    BOOST_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,cliprdr_file,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output");
    AuthorizationChannels authorization(allow, deny);
    BOOST_CHECK_EQUAL(authorization.cliprdr_down_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization.cliprdr_up_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization.cliprdr_file_is_authorized(), false);

    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), true);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);

    BOOST_CHECK_EQUAL(authorization.is_authorized("cliprdr"), false);
    BOOST_CHECK_EQUAL(authorization.is_authorized("drdynvc"), false);
    BOOST_CHECK_EQUAL(authorization.is_authorized("rdpdr"), true);
    BOOST_CHECK_EQUAL(authorization.is_authorized("rdpsnd"), true);

    BOOST_CHECK_EQUAL(authorization.rdpdr_drive_read_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization.rdpdr_drive_write_is_authorized(), true);

    BOOST_CHECK_EQUAL(authorization.rdpsnd_audio_output_is_authorized(), false);
}

BOOST_AUTO_TEST_CASE(TestUpdateAuthorizedChannels6)
{
    std::string allow;
    std::string deny;

    allow = "";
    deny = "*";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_FILE,RDP_DRIVE_READ,RDP_DRIVE_WRITE");
    BOOST_CHECK_EQUAL(allow, "cliprdr_file,rdpdr_drive_read,rdpdr_drive_write");
    BOOST_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,rdpdr_printer,rdpdr_port,rdpdr_smartcard,rdpsnd_audio_output");
    AuthorizationChannels authorization(allow, deny);
    BOOST_CHECK_EQUAL(authorization.cliprdr_down_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization.cliprdr_up_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization.cliprdr_file_is_authorized(), true);

    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), true);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);

    BOOST_CHECK_EQUAL(authorization.is_authorized("cliprdr"), false);
    BOOST_CHECK_EQUAL(authorization.is_authorized("drdynvc"), false);
    BOOST_CHECK_EQUAL(authorization.is_authorized("rdpdr"), true);
    BOOST_CHECK_EQUAL(authorization.is_authorized("rdpsnd"), true);

    BOOST_CHECK_EQUAL(authorization.rdpdr_drive_read_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization.rdpdr_drive_write_is_authorized(), true);

    BOOST_CHECK_EQUAL(authorization.rdpsnd_audio_output_is_authorized(), false);
}

BOOST_AUTO_TEST_CASE(TestUpdateAuthorizedChannels7)
{
    std::string allow;
    std::string deny;

    allow = "";
    deny = "*";
    AuthorizationChannels::update_authorized_channels(allow, deny, "RDP_CLIPBOARD_FILE,RDP_DRIVE_READ,RDP_DRIVE_WRITE,RDP_AUDIO_OUTPUT");
    BOOST_CHECK_EQUAL(allow, "cliprdr_file,rdpdr_drive_read,rdpdr_drive_write,rdpsnd_audio_output");
    BOOST_CHECK_EQUAL(deny, "*,cliprdr_up,cliprdr_down,rdpdr_printer,rdpdr_port,rdpdr_smartcard");
    AuthorizationChannels authorization(allow, deny);
    BOOST_CHECK_EQUAL(authorization.cliprdr_down_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization.cliprdr_up_is_authorized(), false);
    BOOST_CHECK_EQUAL(authorization.cliprdr_file_is_authorized(), true);

    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PRINT), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_FILESYSTEM), true);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SERIAL), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_PARALLEL), false);
    BOOST_CHECK_EQUAL(authorization.rdpdr_type_is_authorized(rdpdr::RDPDR_DTYP_SMARTCARD), false);

    BOOST_CHECK_EQUAL(authorization.is_authorized("cliprdr"), false);
    BOOST_CHECK_EQUAL(authorization.is_authorized("drdynvc"), false);
    BOOST_CHECK_EQUAL(authorization.is_authorized("rdpdr"), true);
    BOOST_CHECK_EQUAL(authorization.is_authorized("rdpsnd"), true);

    BOOST_CHECK_EQUAL(authorization.rdpdr_drive_read_is_authorized(), true);
    BOOST_CHECK_EQUAL(authorization.rdpdr_drive_write_is_authorized(), true);

    BOOST_CHECK_EQUAL(authorization.rdpsnd_audio_output_is_authorized(), true);
}
