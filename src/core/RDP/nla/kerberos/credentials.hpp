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

#include <cstring>
#include <krb5.h>

#include "utils/log.hpp"

class Krb5Creds final {
public:
    krb5_context ctx;

    Krb5Creds() {
        krb5_error_code ret;
        ret = krb5_init_context(&this->ctx);
        if (ret) {
            LOG(LOG_ERR, "Initialisation KERBEROS 5 LIB");
        }
    }

    ~Krb5Creds() {
        krb5_free_context(this->ctx);
    }


    int get_credentials(const char * princname, const char * password,
                        const char * cache_name)
    {
        char* name;
        krb5_error_code ret;
        krb5_creds creds;
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
        memset(&creds, 0, sizeof(creds));
        ret = krb5_parse_name(this->ctx, princname, &client_princ);
        LOG(LOG_INFO, "Parse name %s", princname);
        if (ret) {
            LOG(LOG_ERR, "Parse name %s", princname);
            goto cleanup;
        }

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
                                           const_cast<char*>(password), nullptr, nullptr, 0, nullptr, nullptr);

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

