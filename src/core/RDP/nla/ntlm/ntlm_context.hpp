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
    Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan, Jennifer Inthavong
*/

#pragma once

#include "utils/genrandom.hpp"
#include "utils/difftimeval.hpp"
#include "utils/utf.hpp"
#include "utils/stream.hpp"

#include "core/RDP/nla/ntlm/ntlm_message.hpp"
#include "core/RDP/nla/ntlm/ntlm_message_negotiate.hpp"
#include "core/RDP/nla/ntlm/ntlm_message_challenge.hpp"
#include "core/RDP/nla/ntlm/ntlm_message_authenticate.hpp"
#include "core/RDP/nla/sspi.hpp"

#include "system/ssl_md5.hpp"
#include "system/ssl_rc4.hpp"
#include "system/ssl_md4.hpp"

enum NtlmState {
    NTLM_STATE_INITIAL,
    NTLM_STATE_NEGOTIATE,
    NTLM_STATE_CHALLENGE,
    NTLM_STATE_AUTHENTICATE,
    NTLM_STATE_WAIT_PASSWORD,
    NTLM_STATE_FINAL
};

// static const uint8_t lm_magic[] = "KGS!@#$%";

static const uint8_t client_sign_magic[] =
    "session key to client-to-server signing key magic constant";
static const uint8_t server_sign_magic[] =
    "session key to server-to-client signing key magic constant";
static const uint8_t client_seal_magic[] =
    "session key to client-to-server sealing key magic constant";
static const uint8_t server_seal_magic[] =
    "session key to server-to-client sealing key magic constant";

