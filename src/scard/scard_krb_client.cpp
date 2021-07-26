#include <cassert>
#include <cstring>

#include "scard/scard_common.hpp"
#include "utils/log.hpp"

#include "scard/scard_krb_client.hpp"


///////////////////////////////////////////////////////////////////////////////


scard_krb_client::scard_krb_client(scard_krb_client_handler *handler_ptr)
    :
    _handler_ptr(handler_ptr),
    _context(nullptr)
{
    krb5_error_code ret;

    ret = krb5_init_context(&_context);
    if (ret)
    {
        throw scard_exception("Failed to initialize Kerberos context.");
    }
}

scard_krb_client::~scard_krb_client()
{
    if (_context)
    {
        krb5_free_context(_context);
    }
}

bool scard_krb_client::get_credentials(
    std::string x509_identity, std::string pin,
    std::string principal_name, std::string password)
{
    krb5_error_code ret;
    krb5_creds creds{};
    krb5_principal princ(nullptr);
    krb5_ccache ccache(nullptr);
    krb5_get_init_creds_opt *pa_opts(nullptr);

    // initialize pre-authentication options
    ret = krb5_get_init_creds_opt_alloc(_context, &pa_opts);
    if (ret)
    {
        LOG(LOG_INFO, "scard_krb_client::get_credentials: "
            "failed to initialize pre-authentication options (%d).", ret);

        goto cleanup;
    }

    // set up pre-authentication
    {
        const char *attr = "X509_user_identity";

        // set pre-authentication options
        ret = krb5_get_init_creds_opt_set_pa(_context, pa_opts,
            attr, x509_identity.c_str());
        if (ret)
        {
            LOG(LOG_INFO, "scard_krb_client::get_credentials: "
                "failed to set up pre-authentication (%d).", ret);

            goto cleanup;
        }
    }

    // resolve default credentials cache
    ret = krb5_cc_default(_context, &ccache);
    if (ret)
    {
        LOG(LOG_ERR, "scard_krb_client::get_credentials: "
            "failed to resolve default credentials cache (%d).", ret);

        goto cleanup;
    }

    // parse principal name
    ret = krb5_parse_name(_context, principal_name.c_str(), &princ);
    if (ret)
    {
        LOG(LOG_ERR, "scard_krb_client::get_credentials: "
            "failed to parse principal name (%d).", ret);

        goto cleanup;
    }

    // get initial credentials, performing pre-authentication
    ret = krb5_get_init_creds_password(_context, &creds, princ,
        password.c_str(), &prompt_password, const_cast<char *>(pin.c_str()),
        0, nullptr, pa_opts);
    if (ret)
    {
        const char *message = krb5_get_error_message(_context, ret);
        if (message)
        {
            LOG(LOG_INFO, "scard_krb_client::get_credentials: "
                "failed to get initial credentials (%d): %s.", ret, message);

            krb5_free_error_message(_context, message);
        }
        else
        {
            LOG(LOG_ERR, "scard_krb_client::get_credentials: "
                "failed to get error message.");
        }
        
        goto cleanup;
    }

    // initialize credentials cache
    ret = krb5_cc_initialize(_context, ccache, princ);
    if (ret)
    {
        LOG(LOG_ERR, "scard_krb_client::get_credentials: "
            "failed to initialize credentials cache (%d).", ret);

        goto cleanup;
    }

    // cache credentials
    ret = krb5_cc_store_cred(_context, ccache, &creds);
    if (ret)
    {
        LOG(LOG_ERR, "scard_krb_client::get_credentials: "
            "failed to cache credentials (%d).", ret);

        goto cleanup;
    }
    
    LOG(LOG_INFO, "scard_krb_client::get_credentials: "
        "credentials cached.");

cleanup:
    if (pa_opts) krb5_get_init_creds_opt_free(_context, pa_opts);
    if (ccache) krb5_cc_close(_context, ccache);
    if (princ) krb5_free_principal(_context, princ);
    krb5_free_cred_contents(_context, &creds);

    // notify
    if (_handler_ptr)
    {
        _handler_ptr->handle_krb_client_result(!ret);
    }

    return !ret;
}

krb5_error_code scard_krb_client::prompt_password(
    krb5_context context, void *data,
    const char */*name*/, const char */*banner*/, int num_prompts,
    krb5_prompt prompts[])
{
    const char *password = reinterpret_cast<const char *>(data);
    const size_t password_length = strlen(password);
    krb5_prompt_type *ptypes;
    
    ptypes = krb5_get_prompt_types(context);
    for (int i = 0; i < num_prompts; i++)
    {
        if (ptypes && ptypes[i] == KRB5_PROMPT_TYPE_PREAUTH)
        {
            memset(prompts[i].reply->data, 0, prompts[i].reply->length);
            memcpy(prompts[i].reply->data, password, password_length);
            prompts[i].reply->length = password_length;
        }
    }
    
    return 0;
}