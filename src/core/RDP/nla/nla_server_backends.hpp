/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2021
   Author(s): David Fort

   NLA server authentication backends
*/

#pragma once

#include "core/RDP/nla/nla_server.hpp"
#include "core/RDP/nla/credssp.hpp"
#include "core/RDP/nla/kerberos.hpp"

/** @brief authentication handler using SPNego */
class NtlmNlaBackend : public INlaAuthBackend {
public:
    NtlmNlaBackend(NlaServer & parent);
    virtual ~NtlmNlaBackend() = default;

    virtual std::vector<uint8_t> authenticate_next(bytes_view auth_data, bytes_view in_mic_data,
            std::vector<uint8_t> & out_mic_data, credssp::State & next_state) override;

protected:
};


/** @brief authentication handler using SPNego */
class SpNegoNlaBackend : public INlaAuthBackend {
public:
    SpNegoNlaBackend(NlaServer & parent);
    virtual ~SpNegoNlaBackend();

    virtual std::vector<uint8_t> authenticate_next(bytes_view auth_data, bytes_view in_mic_data,
            std::vector<uint8_t> & out_mic_data, credssp::State & next_state) override;
protected:
    KnownOid mBackendType;
    KnownOid mAnswerOid;
    INlaAuthBackend *mSubBackend;
};

/** @brief authentication handler using Kerberos */
class GssapiNlaAuthBackend : public INlaAuthBackend {
public:
    GssapiNlaAuthBackend(NlaServer & parent, bool spNego, bool u2u);
    virtual ~GssapiNlaAuthBackend() = default;

    bool init(const char *serviceName);

    virtual std::vector<uint8_t> authenticate_next(bytes_view auth_data, bytes_view in_mic_data,
            std::vector<uint8_t> & out_mic_data, credssp::State & next_state) override;

public:
    static bool mech_available(gss_OID mech);
    static void report_error(OM_uint32 code, const char *str, OM_uint32 major_status, OM_uint32 minor_status);
    static bool sanityCheck();

protected:
    bool sp_nego;
    bool u2u;
    std::unique_ptr<KERBEROSContext> krb_ctx;
};
