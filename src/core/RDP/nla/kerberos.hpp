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

#include <gssapi/gssapi.h>
#include <cstring>
#include <krb5.h>

#include "utils/log.hpp"

#include "cxx/diagnostic.hpp"
#include <cassert>

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wzero-as-null-pointer-constant")
#if REDEMPTION_COMP_CLANG_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wzero-as-null-pointer-constant")
#endif

class Krb5Creds final
{
    krb5_context ctx;

public:
    Krb5Creds() {
        krb5_error_code ret = krb5_init_context(&this->ctx);
        if (ret) {
            LOG(LOG_ERR, "Initialisation KERBEROS 5 LIB");
        }
    }

    ~Krb5Creds() {
        krb5_free_context(this->ctx);
    }


    int get_credentials(const std::string & princname, const char * password,
                        const char * cache_name)
    {
        char* name;
        krb5_error_code ret;
        krb5_creds creds {};
        krb5_principal client_princ;
        krb5_ccache ccache;
        if (cache_name) {
            ret = krb5_cc_resolve(this->ctx, cache_name, &ccache);
            if (ret) {
                LOG(LOG_ERR, "CC Resolve %d", ret);
            }
        }
        else {
            ret = krb5_cc_default(this->ctx, &ccache);
            if (ret) {
                LOG(LOG_ERR, "CC Default resolve ");
            }
        }

        /** krb5_parse_name (from krb5.h)
         * Convert a string principal name to a krb5_principal structure.
         *
         * @param [in]  context         Library context
         * @param [in]  name            String representation of a principal name
         * @param [out] principal_out   New principal
         *
         * Convert a string representation of a principal name to a krb5_principal
         * structure.
         *
         * A string representation of a Kerberos name consists of one or more principal
         * name components, separated by slashes, optionally followed by the \@
         * character and a realm name.  If the realm name is not specified, the local
         * realm is used.
         *
         * To use the slash and \@ symbols as part of a component (quoted) instead of
         * using them as a component separator or as a realm prefix), put a backslash
         * (\) character in front of the symbol.  Similarly, newline, tab, backspace,
         * and NULL characters can be included in a component by using @c n, @c t, @c b
         * or @c 0, respectively.
         *
         * @note The realm in a Kerberos @a name cannot contain slash, colon,
         * or NULL characters.
         *
         * Use krb5_free_principal() to free @a principal_out when it is no longer
         * needed.
         *
         * @retval
         * 0 Success
         * @return
         * Kerberos error codes
         */

        ret = krb5_parse_name(this->ctx, princname.c_str(), &client_princ);
        LOG(LOG_INFO, "Parse name %s", princname);
        if (ret) {
            LOG(LOG_ERR, "Parse name %s", princname);
            goto cleanup;
        }

        /** krb5_unparse_name (from krb5.h)
         * Convert a krb5_principal structure to a string representation.
         *
         * @param [in]  context         Library context
         * @param [in]  principal       Principal
         * @param [out] name            String representation of principal name
         *
         * The resulting string representation uses the format and quoting conventions
         * described for krb5_parse_name().
         *
         * Use krb5_free_unparsed_name() to free @a name when it is no longer needed.
         *
         * @retval
         * 0 Success
         * @return
         * Kerberos error codes
         */
        ret = krb5_unparse_name(this->ctx, client_princ, &name);
        if (ret) {
            LOG(LOG_ERR, "Unparse name");
            goto cleanup;
        }
        LOG(LOG_INFO, "Using principal: %s", name);
        krb5_free_unparsed_name(this->ctx, name);

        // get TGT
        // 4th argument should be const char * !!!
        ret = krb5_get_init_creds_password(this->ctx, &creds, client_princ,
                                           password, nullptr, nullptr, 0, nullptr, nullptr);

        if (ret) {
            LOG(LOG_INFO, "Init creds password failed: Wrong password or no such user");
            goto cleanup;
        }
        // ret = krb5_verify_init_creds(this->ctx, &creds, nullptr, nullptr, nullptr, nullptr);
        // if (ret) {
        //     LOG(LOG_ERR, "Verify creds");
        //     goto cleanup;
        // }
        ret = krb5_cc_initialize(this->ctx, ccache, client_princ);
        if (ret) {
            LOG(LOG_ERR, "CC INITIALIZE");
            goto cleanup;
        }
        ret = krb5_cc_store_cred(this->ctx, ccache, &creds);
        if (ret) {
            LOG(LOG_ERR, "CC Store Creds");
            goto cleanup;
        } else {
            LOG(LOG_INFO, "Credentials Cache stored in %s", cache_name?cache_name:"Default Cache");
        }

    cleanup:
        krb5_cc_close(this->ctx, ccache);
        krb5_free_principal(this->ctx, client_princ);
        krb5_free_cred_contents(this->ctx, &creds);
        return ret;
    }

    int destroy_credentials(const char * cache_name) {
        krb5_error_code ret;
        krb5_ccache ccache;

        if (cache_name) {
            ret = krb5_cc_resolve(this->ctx, cache_name, &ccache);
            if (ret) {
                LOG(LOG_ERR, "Resolving Cache Name");
            }
        }
        else {
            ret = krb5_cc_default(this->ctx, &ccache);
            if (ret) {
                LOG(LOG_ERR, "CC Default resolve ");
            }
        }

        ret = krb5_cc_destroy(this->ctx, ccache);
        if (ret) {
            if (ret != KRB5_FCC_NOFILE) {
                LOG(LOG_ERR, "Destroying Cache");
            } else {
                LOG(LOG_INFO, "No Credential cache to destroy");
            }
        } else {
            LOG(LOG_INFO, "Credentials Cache Succesfully destroyed");
        }
        return ret;
    }
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


inline gss_OID_desc _gss_spnego_krb5_mechanism_oid_desc()
{
    return { 9, const_cast<void *>(static_cast<const void *>("\x2a\x86\x48\x86\xf7\x12\x01\x02\x02")) }; /*NOLINT*/
}

// SecPkgContext_Sizes ContextSizes;
// ContextSizes.cbMaxToken = 4096;
// ContextSizes.cbMaxSignature = 0;
// ContextSizes.cbBlockSize = 0;
// ContextSizes.cbSecurityTrailer = 16;



struct KERBEROSContext final {
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
        , actual_services(0)
        , actual_time(0)
        , actual_flag(0)
        , actual_mech(GSS_C_NO_OID)
        , deleg_cred(GSS_C_NO_CREDENTIAL)
    {}

    ~KERBEROSContext() {
        OM_uint32 major_status, minor_status;
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



REDEMPTION_DIAGNOSTIC_POP
