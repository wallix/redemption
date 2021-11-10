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

#include "core/RDP/nla/nla_server_backends.hpp"
#include "core/RDP/nla/credssp.hpp"
#include "utils/log.hpp"

#include <gssapi/gssapi_krb5.h>

NtlmNlaBackend::NtlmNlaBackend(NlaServer & parent)
: INlaAuthBackend(parent)
{
}

std::vector<uint8_t> NtlmNlaBackend::authenticate_next(bytes_view auth_data, bytes_view in_mic_data,
        std::vector<uint8_t> &out_mic_data, credssp::State & next_state)
{
    std::vector<uint8_t> ret;
    return ret;
}

SpNegoNlaBackend::SpNegoNlaBackend(NlaServer & parent)
: INlaAuthBackend(parent)
, mBackendType(OID_UNKNOWN)
, mAnswerOid(OID_UNKNOWN)
, mSubBackend(nullptr)
{
}

SpNegoNlaBackend::~SpNegoNlaBackend() {
    delete mSubBackend;
}

std::vector<uint8_t> SpNegoNlaBackend::authenticate_next(bytes_view auth_data, bytes_view in_mic_data,
        std::vector<uint8_t> &out_mic_data, credssp::State & next_state) {
    std::vector<uint8_t> ret;
    SpNegoToken token = recvSpNego(auth_data, true);

    if (token.isError) {
        // TODO: warn and error
        return ret;
    }

    bytes_view authToken = token.isInit ? token.negTokenInit.mechToken : token.negTokenResp.responseToken;
    bytes_view micData = token.isInit ? token.negTokenInit.mechListMic : token.negTokenResp.mechListMic;
#if 0
    LOG(LOG_DEBUG, "mechToken=");
    hexdump_d(authToken);
#endif


    if (!mSubBackend) {
        if (!token.isInit) {
            // TODO:
            return ret;
        }

        KnownOid oid = guessAuthTokenType(authToken);
        KnownOid toSearch = oid;
        switch (oid) {
        case OID_NTLM:
        case OID_KRB5:
        case OID_KRB5_KILE:
            break;
        case OID_KRB5_U2U:
            toSearch = OID_KRB5;
            break;
        case OID_SPNEGO:
            LOG(LOG_ERR, "error got SPNego token inside SPNego packet");
            throw Error(ERR_CREDSSP_TS_REQUEST);
        default:
            // TODO: error and warn
            return ret;
        }

        SpNegoTokenInit & tokenInit = token.negTokenInit;
        if (tokenInit.mechTypes.size()) {
            /* We'd better like to see the reported tokenType as first item, but we
             * also scan the rest of the list */
            if (tokenInit.mechTypes[0].mechType != toSearch) {
                bool found = false;
                for (size_t i = 1; i < tokenInit.mechTypes.size(); i++) {
                    SpNegoMech mech = tokenInit.mechTypes[i];
                    if (mech.mechType == toSearch) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    LOG(LOG_INFO, "strange we have not found");
                    // TODO: log about not found mech
                }
            }
        }

        bool u2u = false;
        mAnswerOid = oid;
        switch (oid) {
        case OID_NTLM:
            mSubBackend = new NtlmNlaBackend(mParent);
            break;
        case OID_KRB5_U2U:
            mAnswerOid = OID_KRB5_KILE;
            u2u = true;
            REDEMPTION_CXX_FALLTHROUGH;
        case OID_KRB5:
        case OID_KRB5_KILE:
            mSubBackend = new GssapiNlaAuthBackend(mParent, true, u2u);
            break;
        default:
            // TODO: assert(false)
            break;
        }
        mBackendType = toSearch;
    }

    //credssp::State state;
    SpNegoNegstate negState = SPNEGO_STATE_INVALID;
    std::vector<uint8_t> mic_data;
    std::vector<uint8_t> payload = mSubBackend->authenticate_next(authToken, micData, mic_data, next_state);
    switch (next_state) {
    case credssp::State::Cont:
        negState = SPNEGO_STATE_ACCEPT_INCOMPLETE;
        break;
    case credssp::State::Err:
        negState = SPNEGO_STATE_REJECT;
        break;
    case credssp::State::Finish:
        negState = SPNEGO_STATE_ACCEPT_COMPLETED;
        break;
    }

    return emitNegTokenResp(negState, mAnswerOid, payload, mic_data, false);
}

#if 0
bool get_service_name(chars_view server, gss_name_t * name) {
    gss_buffer_desc output;
    const char* service_name = "TERMSRV";
    gss_OID type = GSS_C_NT_HOSTBASED_SERVICE;
    auto size = (strlen(service_name) + 1 + server.size() + 1);
    auto output_value = std::make_unique<char[]>(size);
    output.value = output_value.get();
    snprintf(static_cast<char*>(output.value), size, "%s@%.*s",
        service_name, int(server.size()), server.data());
    output.length = strlen(static_cast<char*>(output.value)) + 1;
    LOG(LOG_INFO, "GSS IMPORT NAME : %s", static_cast<char*>(output.value));
    OM_uint32 minor_status = 0;
    OM_uint32 major_status = gss_import_name(&minor_status, &output, type, name);
    if (GSS_ERROR(major_status)) {
        LOG(LOG_ERR, "Failed to create service principal name");
        return false;
    }
    return true;
}
#endif

GssapiNlaAuthBackend::GssapiNlaAuthBackend(NlaServer & parent, bool spNego, bool u2u)
: INlaAuthBackend(parent)
, sp_nego(spNego)
, u2u(u2u)
, krb_ctx(new KERBEROSContext())
{
    setenv("KRB5_KTNAME", "/home/david/dev/git/redemption/RDP.keytab", 1);
    this->init("TERMSRV/djinn64.hardening.com@HARDENING.COM");
}

//#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

bool GssapiNlaAuthBackend::init(const char *serviceName)
{
    gss_buffer_desc name_buf;
    gss_name_t server_name;
    OM_uint32 maj_status, minor_stat;


    name_buf.value = const_cast<char*>(serviceName);
    name_buf.length = strlen(serviceName) + 1;

    maj_status = gss_import_name(&minor_stat, &name_buf, (gss_OID)GSS_KRB5_NT_PRINCIPAL_NAME, &server_name);
    if (maj_status != GSS_S_COMPLETE) {
        report_error(GSS_C_GSS_CODE, "importing name", maj_status, minor_stat);
        return false;
    }

    gss_OID_set both_supported = GSS_C_NULL_OID_SET;
    maj_status = gss_create_empty_oid_set(&minor_stat, &both_supported);
    if (maj_status != GSS_S_COMPLETE) {
        this->report_error(GSS_C_GSS_CODE, "creating empty oid_set", maj_status, minor_stat);
        return false;
    }

    gss_OID_desc kerb_u2u_descr = { 10, (void*)(KnownOidHelper::user2userOid) };
    const gss_OID candOid[] = {
            /*gss_mech_krb5,*/ &kerb_u2u_descr/*, gss_mech_krb5, gss_mech_krb5_old, gss_mech_krb5_wrong,*/
    };

    for (int i = 0; i < ARRAY_SIZE(candOid); i++) {
        maj_status = gss_add_oid_set_member(&minor_stat, candOid[i], &both_supported);
        if (maj_status != GSS_S_COMPLETE) {
            this->report_error(GSS_C_GSS_CODE, "unable to add oid to set", maj_status, minor_stat);
            return false;
        }
    }


    maj_status = gss_acquire_cred(&minor_stat, server_name, 0, both_supported, GSS_C_ACCEPT,
                                   &this->krb_ctx->server_cred, nullptr, nullptr);
    if (maj_status != GSS_S_COMPLETE) {
        this->report_error(GSS_C_GSS_CODE, "acquiring credentials", maj_status, minor_stat);
        return false;
    }

    (void)gss_release_name(&minor_stat, &server_name);
    gss_release_oid_set(&minor_stat, &both_supported);

    return true;
}

bool GssapiNlaAuthBackend::mech_available(gss_OID mech)
{
    int mech_found;
    OM_uint32 major_status;
    OM_uint32 minor_status;
    gss_OID_set mech_set;

    mech_found = 0;

    if (mech == GSS_C_NO_OID) {
        return true;
    }

    major_status = gss_indicate_mechs(&minor_status, &mech_set);
    if (!mech_set) {
        return false;
    }
    if (GSS_ERROR(major_status)) {
        report_error(GSS_C_GSS_CODE,
                "Failed to get available mechs on system", major_status,
                minor_status);
        gss_release_oid_set(&minor_status, &mech_set);
        return false;
    }

    gss_test_oid_set_member(&minor_status, mech, mech_set, &mech_found);

    gss_release_oid_set(&minor_status, &mech_set);
    if (GSS_ERROR(major_status)) {
        report_error(GSS_C_GSS_CODE, "Failed to match mechanism in set",
                major_status, minor_status);
        return false;
    }

    return mech_found != 0;
}

void GssapiNlaAuthBackend::report_error(OM_uint32 code, const char *str, OM_uint32 major_status,
        OM_uint32 minor_status)
{
    OM_uint32 msgctx = 0;
    OM_uint32 ms;
    gss_buffer_desc status_string;

    LOG(LOG_ERR, "GSS error [%u:%u:%u]: %s",
            (major_status & 0xff000000) >> 24, // Calling error
            (major_status & 0xff0000) >> 16,// Routine error
            major_status & 0xffff,// Supplementary info bits
            str);

    LOG(LOG_ERR, "GSS Minor status error [%u:%u:%u]:%u %s",
            (minor_status & 0xff000000) >> 24,    // Calling error
            (minor_status & 0xff0000) >> 16,// Routine error
            minor_status & 0xffff,// Supplementary info bits
            minor_status, str);

    do {
        ms = gss_display_status(&minor_status, major_status, code,
                GSS_C_NULL_OID, &msgctx, &status_string);
        if (ms != GSS_S_COMPLETE) {
            continue;
        }

        LOG(LOG_ERR, " - %s", static_cast<char const*>(status_string.value));
    } while (ms == GSS_S_COMPLETE && msgctx);
}

bool GssapiNlaAuthBackend::sanityCheck()
{
    gss_OID_desc desired_mech = _gss_spnego_krb5_mechanism_oid_desc();
    return mech_available(&desired_mech);
}

static void vectorAssign(std::vector<uint8_t> &r1, gss_buffer_t buf)
{
    OM_uint32 minor_status = 0;
    const uint8_t * v = static_cast<uint8_t const*>(buf->value);
    r1.assign(v, v + buf->length);

    (void) gss_release_buffer(&minor_status, buf);
}

std::vector<uint8_t> GssapiNlaAuthBackend::authenticate_next(bytes_view auth_data, bytes_view in_mic_data,
        std::vector<uint8_t> & out_mic_data, credssp::State & next_state)
{
    std::vector<uint8_t> ret;
    //gss_OID_desc desired_mech = _gss_spnego_krb5_mechanism_oid_desc();

    gss_buffer_desc input_tok;
    input_tok.length = auth_data.size();
    input_tok.value = const_cast<uint8_t*>(auth_data.data()); /*NOLINT*/

    gss_buffer_desc output_tok;
    output_tok.length = 0;

    gss_cred_id_t gss_no_cred = GSS_C_NO_CREDENTIAL;

    OM_uint32 minor_status = 0;
    OM_uint32 major_status = gss_accept_sec_context(&minor_status,
                &this->krb_ctx->gss_ctx,
                this->krb_ctx->server_cred,
                &input_tok,
                GSS_C_NO_CHANNEL_BINDINGS,
                &this->krb_ctx->target_name,
                /*&this->krb_ctx->actual_mech,*/ nullptr,
                &output_tok,
                &this->krb_ctx->actual_flag,
                nullptr,
                &this->krb_ctx->deleg_cred
    );

    if (GSS_ERROR(major_status)) {
        LOG(LOG_INFO, "MAJOR ERROR");
        report_error(GSS_C_GSS_CODE, "CredSSP: GSSAPI negotiation failed.",
                           major_status, minor_status);
        //return SEC_E_OUT_OF_SEQUENCE;
    }

    vectorAssign(ret, &output_tok);

    if (this->krb_ctx->actual_flag & GSS_C_INTEG_FLAG) {
        LOG(LOG_DEBUG, "have MIC");
#if 0
        gss_buffer_desc mic_tok;
        major_status = gss_get_mic(&minor_status, this->krb_ctx->gss_ctx,
                GSS_C_QOP_DEFAULT, )
#endif
    }
    return ret;
}
