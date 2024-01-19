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
  Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#pragma once

#include "utils/sugar/zstring_view.hpp"

#include <string>

#include <gssapi/gssapi.h>
#include <krb5.h>


class Krb5Creds final
{
    krb5_context ctx;

public:
    Krb5Creds();

    ~Krb5Creds();

    int get_credentials_keytab(
        const std::string &principal_name, const char *keytab_name,
        const char *cache_name, const char *fast_cache_name);

    int get_credentials_password(
        const char *principal_name, const char *password,
        const char *cache_name, const char *fast_cache_name);

    int destroy_credentials(const char *cache_name);

private:
    struct KrbErrLogger
    {
        krb5_error_code ret = 0;
        char const* ctx_msg = nullptr;
        char const* extra_param = nullptr;

        void log_if_error() const;
    };

    bool resolve_cache_name(
        KrbErrLogger& err_ctx, const char *cache_name, krb5_ccache * const cache);

    bool configure_fast(
        KrbErrLogger& err_ctx, const char *fast_cache_name, krb5_get_init_creds_opt *options);

    bool resolve_principal_name(
        KrbErrLogger& err_ctx, const char *principal_name, krb5_principal *principal);

    bool cache_credentials(
        KrbErrLogger& err_ctx, krb5_creds *credentials,
        krb5_ccache cache, krb5_principal principal);
};


//const char* KERBEROS_PACKAGE_NAME = "KERBEROS";
// const char Kerberos_Name[] = "Kerberos";
// const char Kerberos_Comment[] = "Kerberos Security Package";
// const SecPkgInfo KERBEROS_SecPkgInfo = {
//     0x00082B37,             // fCapabilities
//     1,                      // wVersion
//     0x000A,                 // wRPCID
//     0x00000B48,             // cbMaxToken
//     Kerberos_Name,          // Name
//     Kerberos_Comment        // Comment
// };


inline gss_OID_desc gss_spnego_krb5_mechanism_oid_desc()
{
    void const* oid = "\x2a\x86\x48\x86\xf7\x12\x01\x02\x02";
    return { 9, const_cast<void *>(oid) }; /*NOLINT*/
}

// SecPkgContext_Sizes ContextSizes;
// ContextSizes.cbMaxToken = 4096;
// ContextSizes.cbMaxSignature = 0;
// ContextSizes.cbBlockSize = 0;
// ContextSizes.cbSecurityTrailer = 16;


struct KERBEROSContext final
{
    gss_ctx_id_t gss_ctx;
    gss_name_t target_name;
    OM_uint32 actual_services;
    OM_uint32 actual_time;
    OM_uint32 actual_flag;
    gss_OID actual_mech;
    gss_cred_id_t deleg_cred;
    KERBEROSContext()
        : gss_ctx(GSS_C_NO_CONTEXT)
        , target_name(GSS_C_NO_NAME)
        , deleg_cred(GSS_C_NO_CREDENTIAL)
    {}

    ~KERBEROSContext() {
        OM_uint32 major_status;
        OM_uint32 minor_status;
        if (this->target_name != GSS_C_NO_NAME) {
            major_status = gss_release_name(&minor_status, &this->target_name);
            (void) major_status;
            this->target_name = GSS_C_NO_NAME;
        }
        if (this->gss_ctx != GSS_C_NO_CONTEXT) {
            major_status = gss_delete_sec_context(&minor_status, &this->gss_ctx,
                                                  GSS_C_NO_BUFFER);
            (void) major_status;
            this->gss_ctx = GSS_C_NO_CONTEXT;
        }
        if (this->deleg_cred != GSS_C_NO_CREDENTIAL) {
            major_status = gss_release_cred(&minor_status, &this->deleg_cred);
            (void) major_status;
            this->deleg_cred = GSS_C_NO_CREDENTIAL;
        }
    }
};
