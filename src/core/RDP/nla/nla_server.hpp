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

#pragma once

#include "core/RDP/nla/credssp.hpp"

#include "utils/hexdump.hpp"
#include "utils/timebase.hpp"
#include "utils/genrandom.hpp"

#include "transport/transport.hpp"

class NlaServer;

/** @brief abstract NLA authentication handler */
class INlaAuthBackend {
public:
    INlaAuthBackend(NlaServer & parent);
    virtual ~INlaAuthBackend() = default;

    virtual std::vector<uint8_t> authenticate_next(bytes_view auth_data, bytes_view in_mic_data,
            std::vector<uint8_t> & out_mic_data, credssp::State & next_state) = 0;
protected:
    NlaServer & mParent;
};


/** @brief server-side NLA server */
class NlaServer {
public:
    NlaServer(Random & rand,
            const TimeBase & time_base,
            uint32_t credssp_version,
            bool credssp_verbose, bool verbose);

    std::vector<uint8_t> authenticate_next(bytes_view in_data);

    /** @brief state of the NLA server */
    enum NlaState {
        NLA_STATE_INIT,
        NLA_STATE_AUTHENTICATING,
        NLA_STATE_AUTHENTICATED,
        NLA_STATE_ERR
    };


protected:
    bool credssp_verbose;
    bool verbose;
    uint32_t credssp_version;
    ClientNonce SavedClientNonce;
    Random & rand;
    const TimeBase & time_base;

    credssp::State state = credssp::State::Cont;
    TSCredentials ts_credentials;

    NlaState nla_state;
    INlaAuthBackend *backend;
};
