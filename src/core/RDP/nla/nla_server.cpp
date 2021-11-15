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
  Copyright (C) Wallix 2021
  Author(s): David Fort
*/
#include "nla_server.hpp"
#include "nla_server_backends.hpp"
#include "utils/log.hpp"



INlaAuthBackend::INlaAuthBackend(NlaServer & parent)
: mParent(parent)
{
}


NlaServer::NlaServer(Random & rand,
        const TimeBase & time_base,
        uint32_t credssp_version,
        bool credssp_verbose, bool verbose)
: credssp_verbose(credssp_verbose)
, verbose(verbose)
, credssp_version(credssp_version)
, rand(rand)
, time_base(time_base)
, nla_state(NLA_STATE_INIT)
{
}


std::vector<uint8_t> NlaServer::authenticate_next(bytes_view in_data) {
    std::vector<uint8_t> ret;

    TSRequest req = recvTSRequest(in_data, this->credssp_verbose);
    std::vector<uint8_t> mic_data;

    switch (this->nla_state) {
    case NLA_STATE_INIT: {
        bool u2u = false;
        KnownOid r = guessAuthTokenType(req.negoTokens);
        switch (r) {
        case OID_NTLM:
            backend = new NtlmNlaBackend(*this);
            break;
        case OID_SPNEGO:
            backend = new SpNegoNlaBackend(*this);
            break;
        case OID_KRB5_U2U:
            u2u = true;
            [[fallthrough]];
        case OID_KRB5:
        case OID_KRB5_KILE:
            backend = new GssapiNlaAuthBackend(*this, false, u2u);
            break;
        default:
            // TODO: error
            break;
        }

        this->nla_state = NLA_STATE_AUTHENTICATING;
        ret = backend->authenticate_next(req.negoTokens, bytes_view(), mic_data, this->state);
        return emitTSRequest(req.version, /*emitMechTokensEnvelop(*/ret/*)*/, bytes_view(), bytes_view(), 0, bytes_view(), false, this->verbose);
    }
    case NLA_STATE_AUTHENTICATING:
        ret = backend->authenticate_next(req.negoTokens, bytes_view(), mic_data, this->state);
        return emitTSRequest(req.version, /*emitMechTokensEnvelop(*/ret/*)*/, bytes_view(), bytes_view(), 0, bytes_view(), false, this->verbose);

    case NLA_STATE_AUTHENTICATED:
    case NLA_STATE_ERR:
        // TODO: assert we should not be called in this state
        return ret;
    }

}
