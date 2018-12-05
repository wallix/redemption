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
   Copyright (C) Wallix 2015
   Author(s): Christophe Grosjean, Jonathan Poelen,
              Meng Tan, Raphael Zhou

*/

#define RED_TEST_MODULE TestServerRedirection
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/ServerRedirection.hpp"

RED_AUTO_TEST_CASE(TestServerRedirectionPDU)
{
    RedirectionInfo rinfo_init;
    rinfo_init.valid = true;
    rinfo_init.session_id = 3333;
    memcpy(rinfo_init.host, "machine.domaine.lan", sizeof("machine.domaine.lan"));
    rinfo_init.host_is_fqdn = true;
    memcpy(rinfo_init.username, "utilisateur", sizeof("utilisateur"));
    memcpy(rinfo_init.domain, "domaine", sizeof("domaine"));
    rinfo_init.smart_card_logon = true;

    ServerRedirectionPDU srv_redir_init;
    srv_redir_init.import_from_redirection_info(rinfo_init);

    srv_redir_init.log(LOG_INFO, "test server_redirection_pdu");

    uint8_t buf[65536];
    OutStream out_buffer(buf);
    srv_redir_init.emit(out_buffer);

    ServerRedirectionPDU srv_redir_target;
    InStream in_stream(buf, out_buffer.get_offset());
    srv_redir_target.receive(in_stream);

    srv_redir_target.log(LOG_INFO, "test 2 server_redirection_pdu");

    RedirectionInfo rinfo_target;
    srv_redir_target.export_to_redirection_info(rinfo_target);
    rinfo_target.log(LOG_INFO, "test 2 redirection_info");

    RED_CHECK_EQUAL(rinfo_init.valid, rinfo_target.valid);
    RED_CHECK_EQUAL(rinfo_init.session_id, rinfo_target.session_id);
    RED_CHECK_EQUAL(rinfo_init.host_is_fqdn, rinfo_target.host_is_fqdn);
    RED_CHECK_EQUAL(rinfo_init.smart_card_logon, rinfo_target.smart_card_logon);

    RED_CHECK_EQUAL(0, strcmp(char_ptr_cast(rinfo_init.host),
                                char_ptr_cast(rinfo_target.host)));
    RED_CHECK_EQUAL(0, strcmp(char_ptr_cast(rinfo_init.username),
                                char_ptr_cast(rinfo_target.username)));
    RED_CHECK_EQUAL(0, strcmp(char_ptr_cast(rinfo_init.domain),
                                char_ptr_cast(rinfo_target.domain)));
}
