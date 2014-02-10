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

#ifndef _REDEMPTION_CORE_RDP_NLA_KERBEROS_CREDENTIALS_HPP_
#define _REDEMPTION_CORE_RDP_NLA_KERBEROS_CREDENTIALS_HPP_

#include <stdio.h>
#include <cstring>
#include <krb5.h>

class Krb5Creds {
public:
    krb5_context ctx;

    Krb5Creds() {
        krb5_error_code ret;
        ret = krb5_init_context(&this->ctx);
        if (ret) {
            printf("ERREUR Initialisation KERBEROS 5 LIB\n");
        }
    }

    virtual ~Krb5Creds() {
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
                printf("ERREUR CC Resolve %d\n", ret);
            }
        }
        else {
            ret = krb5_cc_default(this->ctx, &ccache);
            if (ret) {
                printf("ERREUR CC Default resolve \n");
            }
        }
        memset(&creds, 0, sizeof(creds));
        ret = krb5_parse_name(this->ctx, princname, &client_princ);
        printf("Parse name %s\n", princname);
        if (ret) {
            printf("ERREUR Parse name %s\n", princname);
            goto cleanup;
        }

        ret = krb5_unparse_name(this->ctx, client_princ, &name);
        if (ret) {
            printf("ERREUR Unparse name\n");
            goto cleanup;
        }
        printf("Using principal: %s\n", name);
        krb5_free_unparsed_name(this->ctx, name);

        // get TGT
        // 4th argument should be const char * !!!
        ret = krb5_get_init_creds_password(this->ctx, &creds, client_princ,
                                           (char*)password, NULL, NULL, 0, NULL, NULL);

        if (ret) {
            printf("ERREUR Init creds password %s\n", password);
            goto cleanup;
        }
        // ret = krb5_verify_init_creds(this->ctx, &creds, NULL, NULL, NULL, NULL);
        // if (ret) {
        //     printf("ERREUR Verify creds\n");
        //     goto cleanup;
        // }
        ret = krb5_cc_initialize(this->ctx, ccache, client_princ);
        if (ret) {
            printf("ERREUR CC INITIALIZE\n");
            goto cleanup;
        }
        ret = krb5_cc_store_cred(this->ctx, ccache, &creds);
        if (ret) {
            printf("ERREUR CC Store Creds\n");
            goto cleanup;
        } else {
            printf("Credentials Cache stored in %s\n", cache_name?cache_name:"Default Cache");
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
                printf("ERREUR Resolving Cache Name\n");
            }
        }
        else {
            ret = krb5_cc_default(this->ctx, &ccache);
            if (ret) {
                printf("ERREUR CC Default resolve \n");
            }
        }

        ret = krb5_cc_destroy(this->ctx, ccache);
        if (ret) {
            if (ret != KRB5_FCC_NOFILE) {
                printf("ERREUR Destroying Cache\n");
            } else {
                printf("No Credential cache to destroy\n");
            }
        } else {
            printf("Credential Cache Succesfully destroyed\n");
        }
        return ret;
    }
};

#endif
