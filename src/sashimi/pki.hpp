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
   Copyright (c) 2014-2016 by Christophe Grosjean, Meng Tan
   Author(s): Christophe Grosjean

   Adapted from parts of the SSH Library
   Copyright (c) 2003-2009 by Aris Adamantiadis
*/

#pragma once

#include <cassert>
#include "system/ssl_calls.hpp"
#include "openssl_crypto.hpp"

#define MAX_PUBKEY_SIZE 0x100000 /* 1M */
#define MAX_PRIVKEY_SIZE 0x400000 /* 4M */

#define SSH_KEY_FLAG_EMPTY   0x0
#define SSH_KEY_FLAG_PUBLIC  0x0001
#define SSH_KEY_FLAG_PRIVATE 0x0002


struct ssh_public_key_struct {
    int type;
    DSA *dsa_pub;
    RSA *rsa_pub;

    ssh_public_key_struct()
        : type(0)
        , dsa_pub(nullptr)
        , rsa_pub(nullptr)
    {
    }
};

struct ssh_private_key_struct {
    int type;
    DSA *dsa_priv;
    RSA *rsa_priv;
};

struct ssh_key_struct {
    enum ssh_keytypes_e type;
    int flags;
    int ecdsa_nid;
    DSA *dsa;
    RSA *rsa;
    EC_KEY *ecdsa;
    void *cert;

    ssh_key_struct(ssh_keytypes_e type, int flags)
        : type(type)
        , flags(flags)
        , ecdsa_nid(0)
        , dsa(nullptr)
        , rsa(nullptr)
        , ecdsa(nullptr)
        , cert(nullptr)
    {
    }

    const char *type_c() const {
        switch (this->type) {
        case SSH_KEYTYPE_DSS:
        return "ssh-dss";
        case SSH_KEYTYPE_RSA:
        return "ssh-rsa";
        case SSH_KEYTYPE_RSA1:
        return "ssh-rsa1";
        case SSH_KEYTYPE_ECDSA:
        return this->ecdsa_type_c();
        case SSH_KEYTYPE_UNKNOWN:
        default:
            break;
        }
        return "ssh-keytype-unknown";
    }

    const char *ecdsa_type_c() const {
        switch (this->ecdsa_nid) {
            case NID_X9_62_prime256v1:
                return "ecdsa-sha2-nistp256";
            case NID_secp384r1:
                return "ecdsa-sha2-nistp384";
            case NID_secp521r1:
                return "ecdsa-sha2-nistp521";
            default:
                break;
        }
        return "ssh-keytype-ecdsa-unknown";
    }
};

struct ssh_signature_struct {
    enum ssh_keytypes_e sig_type;
    int ecdsa_nid;
    DSA_SIG *dsa_sig;
    std::vector<uint8_t> rsa_sig;
    ECDSA_SIG *ecdsa_sig;

    ssh_signature_struct()
        : sig_type(SSH_KEYTYPE_UNKNOWN)
        , ecdsa_nid(0)
        , dsa_sig(nullptr)
        , rsa_sig{}
        , ecdsa_sig(nullptr)
    {
    }

    const char *type_c() const {
        switch (this->sig_type) {
            case SSH_KEYTYPE_DSS: return "ssh-dss";
            case SSH_KEYTYPE_RSA: return "ssh-rsa";
            case SSH_KEYTYPE_RSA1: return "ssh-rsa1";
            case SSH_KEYTYPE_ECDSA: return this->ecdsa_type_c();
            case SSH_KEYTYPE_UNKNOWN: default: break;
        }
        return "ssh-keytype-unknown";
    }

    const char *ecdsa_type_c() const {
        switch (this->ecdsa_nid) {
            case NID_X9_62_prime256v1:
                return "ecdsa-sha2-nistp256";
            case NID_secp384r1:
                return "ecdsa-sha2-nistp384";
            case NID_secp521r1:
                return "ecdsa-sha2-nistp521";
            default:
                break;
        }
        return "ssh-keytype-ecdsa-unknown";
    }

};

//typedef struct ssh_signature_struct *ssh_signature;

/* SSH Key Functions */
ssh_key_struct *ssh_key_dup(const ssh_key_struct *key);

/* SSH Signature Functions */
void ssh_signature_free(ssh_signature_struct * sign);

int ssh_pki_signature_verify_blob(const std::vector<uint8_t> & sig_blob,
                                  const ssh_key_struct *key,
                                  unsigned char *digest,
                                  size_t dlen,
                                  error_struct & error);

/* SSH Public Key Functions */
int ssh_pki_import_pubkey_blob(ssh_buffer_struct & buffer, ssh_key_struct **pkey);

/* SSH Signing Functions */
std::vector<uint8_t> ssh_pki_export_signature_blob(const ssh_key_struct *key, const unsigned char *hash, size_t hlen);

/* SSH Key Functions */

/* SSH Private Key Functions */
enum ssh_keytypes_e pki_privatekey_type_from_string(const char *privkey);

/* SSH Signature Functions */
int pki_signature_verify(ssh_session_struct * session,
                         const ssh_signature_struct * sig,
                         const ssh_key_struct *key,
                         const unsigned char *hash,
                         size_t hlen);
