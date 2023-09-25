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

#include "core/RDP/nla/kerberos.hpp"
#include "utils/log.hpp"
#include "cxx/diagnostic.hpp"

#include <cassert>

#include <gssapi/gssapi.h>
#include <krb5.h>


static char const* get_krb_err_message(krb5_error_code ret)
{
#define CASE(constant) case constant: return #constant
#define CASE_MSG(constant, msg) case constant: return "[" #constant "]: " msg
    switch (ret)
    {
    CASE_MSG(KRB5KDC_ERR_NONE, "No error");
    CASE_MSG(KRB5KDC_ERR_NAME_EXP, "Client's entry in database has expired");
    CASE_MSG(KRB5KDC_ERR_SERVICE_EXP, "Server's entry in database has expired");
    CASE_MSG(KRB5KDC_ERR_BAD_PVNO, "Requested protocol version not supported");
    CASE_MSG(KRB5KDC_ERR_C_OLD_MAST_KVNO, "Client's key is encrypted in an old master key");
    CASE_MSG(KRB5KDC_ERR_S_OLD_MAST_KVNO, "Server's key is encrypted in an old master key");
    CASE_MSG(KRB5KDC_ERR_C_PRINCIPAL_UNKNOWN, "Client not found in Kerberos database");
    CASE_MSG(KRB5KDC_ERR_S_PRINCIPAL_UNKNOWN, "Server not found in Kerberos database");
    CASE_MSG(KRB5KDC_ERR_PRINCIPAL_NOT_UNIQUE, "Principal has multiple entries in Kerberos database");
    CASE_MSG(KRB5KDC_ERR_NULL_KEY, "Client or server has a null key");
    CASE_MSG(KRB5KDC_ERR_CANNOT_POSTDATE, "Ticket is ineligible for postdating");
    CASE_MSG(KRB5KDC_ERR_NEVER_VALID, "Requested effective lifetime is negative or too short");
    CASE_MSG(KRB5KDC_ERR_POLICY, "KDC policy rejects request");
    CASE_MSG(KRB5KDC_ERR_BADOPTION, "KDC can't fulfill requested option");
    CASE_MSG(KRB5KDC_ERR_ETYPE_NOSUPP, "KDC has no support for encryption type");
    CASE_MSG(KRB5KDC_ERR_SUMTYPE_NOSUPP, "KDC has no support for checksum type");
    CASE_MSG(KRB5KDC_ERR_PADATA_TYPE_NOSUPP, "KDC has no support for padata type");
    CASE_MSG(KRB5KDC_ERR_TRTYPE_NOSUPP, "KDC has no support for transited type");
    CASE_MSG(KRB5KDC_ERR_CLIENT_REVOKED, "Client's credentials have been revoked");
    CASE_MSG(KRB5KDC_ERR_SERVICE_REVOKED, "Credentials for server have been revoked");
    CASE_MSG(KRB5KDC_ERR_TGT_REVOKED, "TGT has been revoked");
    CASE_MSG(KRB5KDC_ERR_CLIENT_NOTYET, "Client not yet valid &mdash; try again later");
    CASE_MSG(KRB5KDC_ERR_SERVICE_NOTYET, "Server not yet valid &mdash; try again later");
    CASE_MSG(KRB5KDC_ERR_KEY_EXP, "Password has expired");
    CASE_MSG(KRB5KDC_ERR_PREAUTH_FAILED, "Preauthentication failed");
    CASE_MSG(KRB5KDC_ERR_PREAUTH_REQUIRED, "Additional pre-authentication required");
    CASE_MSG(KRB5KDC_ERR_SERVER_NOMATCH, "Requested server and ticket don't match");
    CASE_MSG(KRB5KRB_AP_ERR_BAD_INTEGRITY, "Decrypt integrity check failed");
    CASE_MSG(KRB5KRB_AP_ERR_TKT_EXPIRED, "Ticket expired");
    CASE_MSG(KRB5KRB_AP_ERR_TKT_NYV, "Ticket not yet valid");
    CASE_MSG(KRB5KRB_AP_ERR_REPEAT, "Request is a replay");
    CASE_MSG(KRB5KRB_AP_ERR_NOT_US, "The ticket isn't for us");
    CASE_MSG(KRB5KRB_AP_ERR_BADMATCH, "Ticket/authenticator don't match");
    CASE_MSG(KRB5KRB_AP_ERR_SKEW, "Clock skew too great");
    CASE_MSG(KRB5KRB_AP_ERR_BADADDR, "Incorrect net address");
    CASE_MSG(KRB5KRB_AP_ERR_BADVERSION, "Protocol version mismatch");
    CASE_MSG(KRB5KRB_AP_ERR_MSG_TYPE, "Invalid message type");
    CASE_MSG(KRB5KRB_AP_ERR_MODIFIED, "Message stream modified");
    CASE_MSG(KRB5KRB_AP_ERR_BADORDER, "Message out of order");
    CASE_MSG(KRB5KRB_AP_ERR_ILL_CR_TKT, "Illegal cross-realm ticket");
    CASE_MSG(KRB5KRB_AP_ERR_BADKEYVER, "Key version is not available");
    CASE_MSG(KRB5KRB_AP_ERR_NOKEY, "Service key not available");
    CASE_MSG(KRB5KRB_AP_ERR_MUT_FAIL, "Mutual authentication failed");
    CASE_MSG(KRB5KRB_AP_ERR_BADDIRECTION, "Incorrect message direction");
    CASE_MSG(KRB5KRB_AP_ERR_METHOD, "Alternative authentication method required");
    CASE_MSG(KRB5KRB_AP_ERR_BADSEQ, "Incorrect sequence number in message");
    CASE_MSG(KRB5KRB_AP_ERR_INAPP_CKSUM, "Inappropriate type of checksum in message");
    CASE_MSG(KRB5KRB_ERR_GENERIC, "Generic error");
    CASE_MSG(KRB5KRB_ERR_FIELD_TOOLONG, "Field is too long for this implementation");
    CASE_MSG(KRB5_LIBOS_BADLOCKFLAG, "Invalid flag for file lock mode");
    CASE_MSG(KRB5_LIBOS_CANTREADPWD, "Cannot read password");
    CASE_MSG(KRB5_LIBOS_BADPWDMATCH, "Password mismatch");
    CASE_MSG(KRB5_LIBOS_PWDINTR, "Password read interrupted");
    CASE_MSG(KRB5_PARSE_ILLCHAR, "Illegal character in component name");
    CASE_MSG(KRB5_PARSE_MALFORMED, "Malformed representation of principal");
    CASE_MSG(KRB5_CONFIG_CANTOPEN, "Can't open/find Kerberos /etc/krb5/krb5 configuration file");
    CASE_MSG(KRB5_CONFIG_BADFORMAT, "Improper format of Kerberos /etc/krb5/krb5 configuration file");
    CASE_MSG(KRB5_CONFIG_NOTENUFSPACE, "Insufficient space to return complete information");
    CASE_MSG(KRB5_BADMSGTYPE, "Invalid message type specified for encoding");
    CASE_MSG(KRB5_CC_BADNAME, "Credential cache name malformed");
    CASE_MSG(KRB5_CC_UNKNOWN_TYPE, "Unknown credential cache type");
    CASE_MSG(KRB5_CC_NOTFOUND, "Matching credential not found");
    CASE_MSG(KRB5_CC_END, "End of credential cache reached");
    CASE_MSG(KRB5_NO_TKT_SUPPLIED, "Request did not supply a ticket");
    CASE_MSG(KRB5KRB_AP_WRONG_PRINC, "Wrong principal in request");
    CASE_MSG(KRB5KRB_AP_ERR_TKT_INVALID, "Ticket has invalid flag set");
    CASE_MSG(KRB5_PRINC_NOMATCH, "Requested principal and ticket don't match");
    CASE_MSG(KRB5_KDCREP_MODIFIED, "KDC reply did not match expectations");
    CASE_MSG(KRB5_KDCREP_SKEW, "Clock skew too great in KDC reply");
    CASE_MSG(KRB5_IN_TKT_REALM_MISMATCH, "Client/server realm mismatch in initial ticket request");
    CASE_MSG(KRB5_PROG_ETYPE_NOSUPP, "Program lacks support for encryption type");
    CASE_MSG(KRB5_PROG_KEYTYPE_NOSUPP, "Program lacks support for key type");
    CASE_MSG(KRB5_WRONG_ETYPE, "Requested encryption type not used in message");
    CASE_MSG(KRB5_PROG_SUMTYPE_NOSUPP, "Program lacks support for checksum type");
    CASE_MSG(KRB5_REALM_UNKNOWN, "Cannot find KDC for requested realm");
    CASE_MSG(KRB5_SERVICE_UNKNOWN, "Kerberos service unknown");
    CASE_MSG(KRB5_KDC_UNREACH, "Cannot contact any KDC for requested realm");
    CASE_MSG(KRB5_NO_LOCALNAME, "No local name found for principal name");
    CASE_MSG(KRB5_MUTUAL_FAILED, "Mutual authentication failed");
    CASE_MSG(KRB5_RC_TYPE_EXISTS, "Replay cache type is already registered");
    CASE_MSG(KRB5_RC_MALLOC, "No more memory to allocate (in replay cache code)");
    CASE_MSG(KRB5_RC_TYPE_NOTFOUND, "Replay cache type is unknown");
    CASE_MSG(KRB5_RC_UNKNOWN, "Generic unknown RC error");
    CASE_MSG(KRB5_RC_REPLAY, "Message is a replay");
    CASE_MSG(KRB5_RC_IO, "Replay I/O operation failed");
    CASE_MSG(KRB5_RC_NOIO, "Replay cache type does not support non-volatile storage");
    CASE_MSG(KRB5_RC_PARSE, "Replay cache name parse/format error");
    CASE_MSG(KRB5_RC_IO_EOF, "End-of-file on replay cache I/O");
    CASE_MSG(KRB5_RC_IO_MALLOC, "No more memory to allocate (in replay cache I/O code)");
    CASE_MSG(KRB5_RC_IO_PERM, "Permission denied in replay cache code");
    CASE_MSG(KRB5_RC_IO_IO, "I/O error in replay cache i/o code");
    CASE_MSG(KRB5_RC_IO_UNKNOWN, "Generic unknown RC/IO error");
    CASE_MSG(KRB5_RC_IO_SPACE, "Insufficient system space to store replay information");
    CASE_MSG(KRB5_TRANS_CANTOPEN, "Can't open/find realm translation file");
    CASE_MSG(KRB5_TRANS_BADFORMAT, "Improper format of realm translation file");
    CASE_MSG(KRB5_LNAME_CANTOPEN, "Can't open/find lname translation database");
    CASE_MSG(KRB5_LNAME_NOTRANS, "No translation available for requested principal");
    CASE_MSG(KRB5_LNAME_BADFORMAT, "Improper format of translation database entry");
    CASE_MSG(KRB5_CRYPTO_INTERNAL, "Cryptosystem internal error");
    CASE_MSG(KRB5_KT_BADNAME, "Key table name malformed");
    CASE_MSG(KRB5_KT_UNKNOWN_TYPE, "Unknown Key table type");
    CASE_MSG(KRB5_KT_NOTFOUND, "Key table entry not found");
    CASE_MSG(KRB5_KT_END, "End of key table reached");
    CASE_MSG(KRB5_KT_NOWRITE, "Cannot write to specified key table");
    CASE_MSG(KRB5_KT_IOERR, "Error writing to key table");
    CASE_MSG(KRB5_NO_TKT_IN_RLM, "Cannot find ticket for requested realm");
    CASE_MSG(KRB5DES_BAD_KEYPAR, "DES key has bad parity");
    CASE_MSG(KRB5DES_WEAK_KEY, "DES key is a weak key");
    CASE_MSG(KRB5_BAD_ENCTYPE, "Bad encryption type");
    CASE_MSG(KRB5_BAD_KEYSIZE, "Key size is incompatible with encryption type");
    CASE_MSG(KRB5_BAD_MSIZE, "Message size is incompatible with encryption type");
    CASE_MSG(KRB5_CC_TYPE_EXISTS, "Credentials cache type is already registered");
    CASE_MSG(KRB5_KT_TYPE_EXISTS, "Key table type is already registered");
    CASE_MSG(KRB5_CC_IO, " Credentials cache I/O operation failed");
    CASE_MSG(KRB5_FCC_PERM, "Credentials cache file permissions incorrect");
    CASE_MSG(KRB5_FCC_NOFILE, "No credentials cache file found");
    CASE_MSG(KRB5_FCC_INTERNAL, "Internal file credentials cache error");
    CASE_MSG(KRB5_CC_WRITE, "Error writing to credentials cache file");
    CASE_MSG(KRB5_CC_NOMEM, "No more memory to allocate (in credentials cache code)");
    CASE_MSG(KRB5_CC_FORMAT, "Bad format in credentials cache");
    CASE_MSG(KRB5_INVALID_FLAGS, "Invalid KDC option combination (library internal error)");
    CASE_MSG(KRB5_NO_2ND_TKT, "Request missing second ticket");
    CASE_MSG(KRB5_NOCREDS_SUPPLIED, "No credentials supplied to library routine");
    CASE_MSG(KRB5_SENDAUTH_BADAUTHVERS, "Bad sendauth version was sent");
    CASE_MSG(KRB5_SENDAUTH_BADAPPLVERS, "Bad application version was sent (by sendauth)");
    CASE_MSG(KRB5_SENDAUTH_BADRESPONSE, "Bad response (during sendauth exchange)");
    CASE_MSG(KRB5_SENDAUTH_REJECTED, "Server rejected authentication (during sendauth exchange)");
    CASE_MSG(KRB5_PREAUTH_BAD_TYPE, "Unsupported pre-authentication type");
    CASE_MSG(KRB5_PREAUTH_NO_KEY, "Required pre-authentication key not supplied");
    CASE_MSG(KRB5_PREAUTH_FAILED, "Generic preauthentication failure");
    CASE_MSG(KRB5_RCACHE_BADVNO, "Unsupported replay cache format version number");
    CASE_MSG(KRB5_CCACHE_BADVNO, "Unsupported credentials cache format version number");
    CASE_MSG(KRB5_KEYTAB_BADVNO, "Unsupported key table format version number");
    CASE_MSG(KRB5_PROG_ATYPE_NOSUPP, "Program lacks support for address type");
    CASE_MSG(KRB5_RC_REQUIRED, "Message replay detection requires rcache parameter");
    CASE_MSG(KRB5_ERR_BAD_HOSTNAME, "Host name cannot be canonicalized");
    CASE_MSG(KRB5_ERR_HOST_REALM_UNKNOWN, "Cannot determine realm for host");
    CASE_MSG(KRB5_SNAME_UNSUPP_NAMETYPE, "Conversion to service principal undefined for name type");
    CASE_MSG(KRB5KRB_AP_ERR_V4_REPLY, "Initial Ticket response appears to be Version 4 error");
    CASE_MSG(KRB5_REALM_CANT_RESOLVE, "Cannot resolve KDC for requested realm");
    CASE_MSG(KRB5_TKT_NOT_FORWARDABLE, "Requesting ticket can't get forwardable tickets");
    CASE_MSG(KRB5_FWD_BAD_PRINCIPAL, "Bad principal name while trying to forward credentials");
    CASE_MSG(KRB5_GET_IN_TKT_LOOP, "Looping detected inside krb5_get_in_tkt");
    CASE_MSG(KRB5_CONFIG_NODEFREALM, "Configuration file /etc/krb5/krb5.conf does not specify default realm");
    CASE_MSG(KRB5_SAM_UNSUPPORTED, "Bad SAM flags in obtain_sam_padata");
    CASE_MSG(KRB5_KT_NAME_TOOLONG, "Keytab name too long");
    CASE_MSG(KRB5_KT_KVNONOTFOUND, "Key version number for principal in key table is incorrect");

    CASE(KRB5_APPL_EXPIRED);
    CASE(KRB5_CC_NOSUPP);
    CASE(KRB5_CC_NOT_KTYPE);
    CASE(KRB5_CC_READONLY);
    CASE(KRB5_CHPW_FAIL);
    CASE(KRB5_CHPW_PWDNULL);
    CASE(KRB5_CONFIG_ETYPE_NOSUPP);
    CASE(KRB5_DELTAT_BADFORMAT);
    CASE(KRB5_EAI_FAIL);
    CASE(KRB5_EAI_NODATA);
    CASE(KRB5_EAI_NONAME);
    CASE(KRB5_EAI_SERVICE);
    CASE(KRB5_ERR_BAD_S2K_PARAMS);
    CASE(KRB5_ERR_FAST_REQUIRED);
    CASE(KRB5_ERR_INVALID_UTF8);
    CASE(KRB5_ERR_NO_SERVICE);
    CASE(KRB5_ERR_NUMERIC_REALM);
    CASE(KRB5_ERR_RCSID);
    CASE(KRB5KDC_ERR_CANT_VERIFY_CERTIFICATE);
    CASE(KRB5KDC_ERR_CERTIFICATE_MISMATCH);
    CASE(KRB5KDC_ERR_CLIENT_NAME_MISMATCH);
    CASE(KRB5KDC_ERR_CLIENT_NOT_TRUSTED);
    CASE(KRB5KDC_ERR_DH_KEY_PARAMETERS_NOT_ACCEPTED);
    CASE(KRB5KDC_ERR_DIGEST_IN_CERT_NOT_ACCEPTED);
    CASE(KRB5KDC_ERR_DIGEST_IN_SIGNED_DATA_NOT_ACCEPTED);
    CASE(KRB5KDC_ERR_INCONSISTENT_KEY_PURPOSE);
    CASE(KRB5KDC_ERR_INVALID_CERTIFICATE);
    CASE(KRB5KDC_ERR_INVALID_SIG);
    CASE(KRB5KDC_ERR_KDC_NAME_MISMATCH);
    CASE(KRB5KDC_ERR_KDC_NOT_TRUSTED);
    CASE(KRB5KDC_ERR_MORE_PREAUTH_DATA_REQUIRED);
    CASE(KRB5KDC_ERR_MUST_USE_USER2USER);
    CASE(KRB5KDC_ERR_NO_ACCEPTABLE_KDF);
    CASE(KRB5KDC_ERR_PA_CHECKSUM_MUST_BE_INCLUDED);
    CASE(KRB5KDC_ERR_PATH_NOT_ACCEPTED);
    CASE(KRB5KDC_ERR_PREAUTH_EXPIRED);
    CASE(KRB5KDC_ERR_PUBLIC_KEY_ENCRYPTION_NOT_SUPPORTED);
    CASE(KRB5KDC_ERR_REVOCATION_STATUS_UNAVAILABLE);
    CASE(KRB5KDC_ERR_REVOCATION_STATUS_UNKNOWN);
    CASE(KRB5KDC_ERR_REVOKED_CERTIFICATE);
    CASE(KRB5KDC_ERR_SVC_UNAVAILABLE);
    CASE(KRB5KDC_ERR_UNKNOWN_CRITICAL_FAST_OPTION);
    CASE(KRB5KDC_ERR_WRONG_REALM);
    CASE(KRB5KRB_AP_ERR_IAKERB_KDC_NO_RESPONSE);
    CASE(KRB5KRB_AP_ERR_IAKERB_KDC_NOT_FOUND);
    CASE(KRB5KRB_AP_ERR_NO_TGT);
    CASE(KRB5KRB_AP_ERR_USER_TO_USER_REQUIRED);
    CASE(KRB5KRB_AP_PATH_NOT_ACCEPTED);
    CASE(KRB5KRB_ERR_RESPONSE_TOO_BIG);
    CASE(KRB5_KT_FORMAT);
    CASE(KRB5_LIB_EXPIRED);
    CASE(KRB5_LOCAL_ADDR_REQUIRED);
    CASE(KRB5_NOPERM_ETYPE);
    CASE(KRB5_OBSOLETE_FN);
    CASE(KRB5_PLUGIN_NO_HANDLE);
    CASE(KRB5_PLUGIN_OP_NOTSUPP);
    CASE(KRB5_REMOTE_ADDR_REQUIRED);
    CASE(KRB5_SAM_BAD_CHECKSUM);
    CASE(KRB5_SAM_INVALID_ETYPE);
    CASE(KRB5_SAM_NO_CHECKSUM);
    CASE(KRB5_TRACE_NOSUPP);
    }
#undef CASE
#undef CASE_MSG

    return "<unknown>";
}

void Krb5Creds::KrbErrLogger::log_if_error()
{
    if (!ret) {
        return ;
    }

    LOG(LOG_ERR, "Krb5Creds: %s%s%s%s (%d) %s", ctx_msg,
        extra_param ? " '" : "",
        extra_param ? extra_param : "",
        extra_param ? "'" : "",
        ret,
        get_krb_err_message(ret)
    );
}

#define CHECK_OR_SET_ERR(err_expr, err_obj, expr, msg, extra) \
    do { if ((err_obj.ret = expr)) {                          \
        err_obj.ctx_msg = msg;                                \
        err_obj.extra_param = extra;                          \
        err_expr;                                             \
    } } while (0)


REDEMPTION_DIAGNOSTIC_PUSH()
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wzero-as-null-pointer-constant")
#if REDEMPTION_COMP_CLANG_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wzero-as-null-pointer-constant")
#endif


Krb5Creds::Krb5Creds()
{
    krb5_error_code ret;

    // initialize context
    ret = krb5_init_context(&this->ctx);
    if (ret)
    {
        LOG(LOG_ERR, "Failed to initialize Kerberos context");
    }
}

Krb5Creds::~Krb5Creds()
{
    // release context
    krb5_free_context(this->ctx);
}

int Krb5Creds::get_credentials_keytab(
    const std::string &principal_name, const char *keytab_name,
    const char *cache_name, const char *fast_cache_name)
{
    KrbErrLogger err;
    krb5_keytab keytab(nullptr);
    krb5_creds creds {};
    krb5_principal princ(nullptr);
    krb5_ccache ccache(nullptr);
    krb5_get_init_creds_opt *opts(nullptr);

    // allocate initial credentials options
    CHECK_OR_SET_ERR(goto cleanup, err, krb5_get_init_creds_opt_alloc(this->ctx, &opts),
        "Failed to allocate initial credentials options structure", nullptr);

    // resolve keytab
    if (keytab_name)
    {
        CHECK_OR_SET_ERR(goto cleanup, err, krb5_kt_resolve(this->ctx, keytab_name, &keytab),
            "Failed to resolve keytab", keytab_name);
    }
    else
    {
        CHECK_OR_SET_ERR(goto cleanup, err, krb5_kt_client_default(this->ctx, &keytab),
            "Failed to resolve default keytab", nullptr);
    }

    if (not resolve_cache_name(err, cache_name, &ccache)
     || not configure_fast(err, fast_cache_name, opts)
     || not resolve_principal_name(err, principal_name.c_str(), &princ)
    ) {
        goto cleanup;
    }

    // get credentials
    CHECK_OR_SET_ERR(goto cleanup, err,
        krb5_get_init_creds_keytab(this->ctx, &creds, princ, keytab, 0, nullptr, opts),
        "Failed to get credentials from keytab", nullptr);

    // cache credentials
    if (not cache_credentials(err, &creds, ccache, princ))
    {
        goto cleanup;
    }

    LOG(LOG_INFO, "Credentials cached to %s", cache_name ? cache_name : "default cache");

cleanup:
    if (opts) krb5_get_init_creds_opt_free(this->ctx, opts);
    if (ccache) krb5_cc_close(this->ctx, ccache);
    if (princ) krb5_free_principal(this->ctx, princ);
    krb5_free_cred_contents(this->ctx, &creds);

    err.log_if_error();
    return err.ret;
}

int Krb5Creds::get_credentials_password(
    const char *principal_name, const char *password,
    const char *cache_name, const char *fast_cache_name)
{
    KrbErrLogger err;
    krb5_creds creds {};
    krb5_principal princ(nullptr);
    krb5_ccache ccache(nullptr);
    krb5_get_init_creds_opt *opts(nullptr);

    // allocate initial credentials options
    CHECK_OR_SET_ERR(goto cleanup, err, krb5_get_init_creds_opt_alloc(this->ctx, &opts),
        "Failed to allocate initial credentials options structure", nullptr);

    if (not resolve_cache_name(err, cache_name, &ccache)
     || not configure_fast(err, fast_cache_name, opts)
     || not resolve_principal_name(err, principal_name, &princ)
    ) {
        goto cleanup;
    }

    // get credentials
    // CAUTION: 4th argument should be const char * !!!
    CHECK_OR_SET_ERR(goto cleanup, err,
        krb5_get_init_creds_password(this->ctx, &creds, princ, password, nullptr, nullptr, 0, nullptr, opts),
        "Failed to get credentials from password", nullptr);

    // cache credentials
    if (not cache_credentials(err, &creds, ccache, princ))
    {
        goto cleanup;
    }

    LOG(LOG_INFO, "Credentials cached to %s", cache_name ? cache_name : "default cache");

cleanup:
    if (opts) krb5_get_init_creds_opt_free(this->ctx, opts);
    if (ccache) krb5_cc_close(this->ctx, ccache);
    if (princ) krb5_free_principal(this->ctx, princ);
    krb5_free_cred_contents(this->ctx, &creds);

    err.log_if_error();
    return err.ret;
}

int Krb5Creds::destroy_credentials(const char *cache_name)
{
    KrbErrLogger err;
    krb5_ccache ccache(nullptr);

    // resolve cache name
    if (not resolve_cache_name(err, cache_name, &ccache))
    {
        LOG(LOG_ERR, "Failed to resolve cache name");
    }
    // destroy credentials cache
    else if ((err.ret = krb5_cc_destroy(this->ctx, ccache)))
    {
        if (err.ret != KRB5_FCC_NOFILE)
        {
            err.ctx_msg = "Failed to destroy credentials cache";
        }
        else
        {
            err.ctx_msg = "No credentials cache to destroy";
        }
    }
    else
    {
        LOG(LOG_INFO, "Credentials cache destroyed");
        return 0;
    }

    err.log_if_error();
    return err.ret;
}

bool Krb5Creds::resolve_cache_name(KrbErrLogger& err_ctx, const char *cache_name, krb5_ccache * const cache)
{
    assert(cache);

    if (cache_name)
    {
        CHECK_OR_SET_ERR(return false, err_ctx, krb5_cc_resolve(this->ctx, cache_name, cache),
            "Failed to resolve credentials cache name", cache_name);
    }
    else
    {
        CHECK_OR_SET_ERR(return false, err_ctx, krb5_cc_default(this->ctx, cache),
            "Failed to resolve default credentials cache name", nullptr);
    }

    return true;
}

bool Krb5Creds::configure_fast(KrbErrLogger& err_ctx, const char *fast_cache_name, krb5_get_init_creds_opt *options)
{
    assert(options);

    // skip if no FAST cache name provided
    if (!fast_cache_name)
    {
        return true;
    }

    LOG(LOG_INFO, "Resolve and set FAST cache");
    // resolve and set FAST cache
    CHECK_OR_SET_ERR(return false, err_ctx,
        krb5_get_init_creds_opt_set_fast_ccache_name(this->ctx, options, fast_cache_name),
        "Failed to set FAST cache", nullptr);

    // require FAST usage
    CHECK_OR_SET_ERR(return false, err_ctx,
        krb5_get_init_creds_opt_set_fast_flags(this->ctx, options, KRB5_FAST_REQUIRED),
        "Failed to set FAST flags", nullptr);

    return true;
}

bool Krb5Creds::resolve_principal_name(KrbErrLogger& err_ctx, const char *principal_name, krb5_principal* principal)
{
    char *name;

    assert(principal_name);

    // parse principal name
    CHECK_OR_SET_ERR(return false, err_ctx,
        krb5_parse_name(this->ctx, principal_name, principal),
        "Failed to parse principal name", principal_name);

    // unparse principal name (for logging only)
    CHECK_OR_SET_ERR(return false, err_ctx,
        krb5_unparse_name(this->ctx, *principal, &name),
        "Failed to unparse principal name", nullptr);

    LOG(LOG_INFO, "Resolved principal name: %s", name);

    // release unparsed principal name
    krb5_free_unparsed_name(this->ctx, name);

    return true;
}

bool Krb5Creds::cache_credentials(
    KrbErrLogger& err_ctx, krb5_creds *credentials,
    krb5_ccache cache, krb5_principal principal)
{
    assert(cache);

    // initialize credentials cache
    CHECK_OR_SET_ERR(return false, err_ctx, krb5_cc_initialize(this->ctx, cache, principal),
        "Failed to initialize credentials cache", nullptr);

    // store credentials in cache
    CHECK_OR_SET_ERR(return false, err_ctx, krb5_cc_store_cred(this->ctx, cache, credentials),
        "Failed to store credentials in cache", nullptr);

    return true;
}

#undef CHECK_OR_SET_ERR

REDEMPTION_DIAGNOSTIC_POP()
