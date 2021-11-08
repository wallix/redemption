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

REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wzero-as-null-pointer-constant")
#if REDEMPTION_COMP_CLANG_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wzero-as-null-pointer-constant")
#endif

class Krb5Creds final
{
    krb5_context ctx;

public:
    Krb5Creds()
    {
        krb5_error_code ret;
        
        // initialize context
        ret = krb5_init_context(&this->ctx);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to initialize Kerberos context");
        }   
    }

    ~Krb5Creds()
    {
        // release context
        krb5_free_context(this->ctx);
    }

public:

    int get_credentials_keytab(
        const std::string &principal_name, const char *keytab_name,
        const char *cache_name, const char *fast_cache_name)
    {
        krb5_error_code ret;
        krb5_keytab keytab(nullptr);
        krb5_creds creds {};
        krb5_principal princ(nullptr);
        krb5_ccache ccache(nullptr);
        krb5_get_init_creds_opt *opts(nullptr);

        // allocate initial credentials options
        ret = krb5_get_init_creds_opt_alloc(this->ctx, &opts);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to allocate initial credentials options structure (%d)", ret);

            goto cleanup;
        } 

        // resolve keytab
        if (keytab_name)
        {
            ret = krb5_kt_resolve(this->ctx, keytab_name, &keytab);
            if (ret)
            {
                LOG(LOG_ERR, "Failed to resolve keytab '%s' (%d)", keytab_name, ret);

                goto cleanup;
            }
        }
        else
        {
            ret = krb5_kt_client_default(this->ctx, &keytab);
            if (ret)
            {
                LOG(LOG_ERR, "Failed to resolve default keytab (%d)", ret);

                goto cleanup;
            }
        }

        // resolve cache name
        ret = resolve_cache_name(cache_name, &ccache);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to resolve cache name");

            goto cleanup;
        } 

        // configure FAST
        ret = configure_fast(fast_cache_name, opts);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to configure FAST");

            goto cleanup;
        }

        // resolve principal name
        ret = resolve_principal_name(principal_name.c_str(), &princ);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to configure FAST");

            goto cleanup;
        }

        // get credentials
        ret = krb5_get_init_creds_keytab(this->ctx, &creds, princ,
            keytab, 0, nullptr, opts);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to get credentials from keytab (%d)", ret);

            goto cleanup;
        }

        // cache credentials
        ret = cache_credentials(&creds, ccache, princ);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to cache credentials");

            goto cleanup;
        }
        
        LOG(LOG_INFO, "Credentials cached to %s", cache_name ? cache_name : "default cache");
        
    cleanup:
        if (opts) krb5_get_init_creds_opt_free(this->ctx, opts);
        if (ccache) krb5_cc_close(this->ctx, ccache);
        if (princ) krb5_free_principal(this->ctx, princ);
        krb5_free_cred_contents(this->ctx, &creds);

        return ret;
    }

    int get_credentials_password(
        const char *principal_name, const char *password,
        const char *cache_name, const char *fast_cache_name)
    {
        krb5_error_code ret;
        krb5_creds creds {};
        krb5_principal princ(nullptr);
        krb5_ccache ccache(nullptr);
        krb5_get_init_creds_opt *opts(nullptr);

        // allocate initial credentials options
        ret = krb5_get_init_creds_opt_alloc(this->ctx, &opts);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to allocate initial credentials options structure (%d)", ret);

            goto cleanup;
        } 

        // resolve cache name
        ret = resolve_cache_name(cache_name, &ccache);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to resolve cache name");

            goto cleanup;
        } 

        // configure FAST
        ret = configure_fast(fast_cache_name, opts);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to configure FAST");

            goto cleanup;
        }

        // resolve principal name
        ret = resolve_principal_name(principal_name, &princ);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to configure FAST");

            goto cleanup;
        }

        // get credentials
        // CAUTION: 4th argument should be const char * !!!
        ret = krb5_get_init_creds_password(this->ctx, &creds, princ,
            password, nullptr, nullptr, 0, nullptr, opts);
        if (ret)
        {
            LOG(LOG_INFO, "Failed to get credentials from password (%d)", ret);

            goto cleanup;
        }

        // cache credentials
        ret = cache_credentials(&creds, ccache, princ);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to cache credentials");

            goto cleanup;
        }
        
        LOG(LOG_INFO, "Credentials cached to %s", cache_name ? cache_name : "default cache");

    cleanup:
        if (opts) krb5_get_init_creds_opt_free(this->ctx, opts);
        if (ccache) krb5_cc_close(this->ctx, ccache);
        if (princ) krb5_free_principal(this->ctx, princ);
        krb5_free_cred_contents(this->ctx, &creds);

        return ret;
    }

    int destroy_credentials(const char *cache_name)
    {
        krb5_error_code ret;
        krb5_ccache ccache(nullptr);

        // resolve cache name
        ret = resolve_cache_name(cache_name, &ccache);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to resolve cache name");

            return ret;
        } 

        // destroy credentials cache
        ret = krb5_cc_destroy(this->ctx, ccache);
        if (ret)
        {
            if (ret != KRB5_FCC_NOFILE)
            {
                LOG(LOG_ERR, "Failed to destroy credentials cache (%d)", ret);
            }
            else
            {
                LOG(LOG_INFO, "No credentials cache to destroy");
            }
        }
        else
        {
            LOG(LOG_INFO, "Credentials cache destroyed");
        }

        return ret;
    }

private:

    krb5_error_code resolve_cache_name(const char *cache_name, krb5_ccache * const cache)
    {
        krb5_error_code ret;

        assert (cache);

        if (cache_name)
        {
            ret = krb5_cc_resolve(this->ctx, cache_name, cache);
            if (ret)
            {
                LOG(LOG_ERR, "Failed to resolve credentials cache name '%s' (%d)", cache_name, ret);
            }
        }
        else
        {
            ret = krb5_cc_default(this->ctx, cache);
            if (ret)
            {
                LOG(LOG_ERR, "Failed to resolve default credentials cache name (%d)", ret);
            }
        }

        return ret;
    }

    krb5_error_code configure_fast(const char *fast_cache_name, krb5_get_init_creds_opt *options)
    {
        krb5_error_code ret;

        assert (options);

        // skip if no FAST cache name provided
        if (!fast_cache_name)
        {
            return 0;
        }

        // resolve and set FAST cache
        ret = krb5_get_init_creds_opt_set_fast_ccache_name(this->ctx, options, fast_cache_name);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to set FAST cache (%d)", ret);

            return ret;
        }

        // require FAST usage
        ret = krb5_get_init_creds_opt_set_fast_flags(this->ctx, options, KRB5_FAST_REQUIRED);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to set FAST flags (%d)", ret);
        }

        return ret;
    }

    krb5_error_code resolve_principal_name(const char *principal_name, krb5_principal *principal)
    {
        krb5_error_code ret;
        char *name;

        assert (principal_name);
        assert (principal);

        // parse principal name
        ret = krb5_parse_name(this->ctx, principal_name, principal);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to parse principal name '%s' (%d)", principal_name, ret);
            
            return ret;
        }

        // unparse principal name (for logging only)
        ret = krb5_unparse_name(this->ctx, *principal, &name);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to unparse principal name (%d)", ret);

            return ret;
        }

        LOG(LOG_INFO, "Resolved principal name: %s", name);

        // release unparsed principal name
        krb5_free_unparsed_name(this->ctx, name);

        return ret;
    }

    krb5_error_code cache_credentials(krb5_creds *credentials,
        krb5_ccache &cache, krb5_principal &principal)
    {
        krb5_error_code ret;

        assert (cache);

        // initialize credentials cache
        ret = krb5_cc_initialize(this->ctx, cache, principal);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to initialize credentials cache (%d)", ret);
            return ret;
        }

        // store credentials in cache
        ret = krb5_cc_store_cred(this->ctx, cache, credentials);
        if (ret)
        {
            LOG(LOG_ERR, "Failed to store credentials in cache (%d)", ret);
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



REDEMPTION_DIAGNOSTIC_POP()
