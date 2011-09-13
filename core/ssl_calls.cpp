/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   ssl calls based on openssl

*/

#include <string.h>
#include <stdint.h>

#include "ssl_calls.hpp"

#if defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER >= 0x0090800f)
#undef OLD_RSA_GEN1
#else
#define OLD_RSA_GEN1
#endif

/* rc4 stuff */

/*****************************************************************************/
uint8_t* ssl_rc4_info_create(void)
{
    #warning we should manage what happen when we have an allocation error
    return (uint8_t*)calloc(1, sizeof(RC4_KEY));
}

/*****************************************************************************/
void ssl_rc4_info_delete(uint8_t* rc4_info)
{
    free(rc4_info);
}

/*****************************************************************************/
void ssl_rc4_set_key(uint8_t* rc4_info, uint8_t* key, int len)
{
    RC4_set_key((RC4_KEY*)rc4_info, len, key);
}

/*****************************************************************************/
void ssl_rc4_crypt(uint8_t* rc4_info, uint8_t* data, int len)
{
    RC4((RC4_KEY*)rc4_info, len, data, data);
}

/* sha1 stuff */

/*****************************************************************************/
uint8_t* ssl_sha1_info_create(void)
{
    #warning we should manage what happen when we have an allocation error
    return (uint8_t*)calloc(1, sizeof(SHA_CTX));
}

/*****************************************************************************/
void ssl_sha1_info_delete(uint8_t* sha1_info)
{
    free(sha1_info);
}

/* md5 stuff */

/*****************************************************************************/
uint8_t* ssl_md5_info_create(void)
{
    #warning we should manage what happen when we have an allocation error
    return (uint8_t*)calloc(1, sizeof(MD5_CTX));
}

/*****************************************************************************/
void ssl_md5_info_delete(uint8_t* md5_info)
{
    free(md5_info);
}

/*****************************************************************************/
static void ssl_reverse_it(uint8_t* p, int len)
{
    char temp;

    int i = 0;
    int j = len - 1;
    while (i < j) {
        temp = p[i];
        p[i] = p[j];
        p[j] = temp;
        i++;
        j--;
    }
}

/*****************************************************************************/
int ssl_mod_exp(uint8_t* out, int out_len, uint8_t* in, int in_len,
                uint8_t* mod, int mod_len, uint8_t* exp, int exp_len)
{
    BN_CTX* ctx;
    BIGNUM lmod;
    BIGNUM lexp;
    BIGNUM lin;
    BIGNUM lout;
    int rv;
    uint8_t* l_out;
    uint8_t* l_in;
    uint8_t* l_mod;
    uint8_t* l_exp;

    #warning replace these fucking new / delete by objects on stack
    l_out = new uint8_t[out_len];
    memset(l_out, 0, out_len);
    l_in = new uint8_t[in_len];
    memset(l_in, 0, in_len);
    l_mod = new uint8_t[mod_len];
    memset(l_mod, 0, mod_len);
    l_exp = new uint8_t[exp_len];
    memset(l_exp, 0, exp_len);
    memcpy(l_in, in, in_len);
    memcpy(l_mod, mod, mod_len);
    memcpy(l_exp, exp, exp_len);
    ssl_reverse_it(l_in, in_len);
    ssl_reverse_it(l_mod, mod_len);
    ssl_reverse_it(l_exp, exp_len);
    ctx = BN_CTX_new();
    BN_init(&lmod);
    BN_init(&lexp);
    BN_init(&lin);
    BN_init(&lout);
    BN_bin2bn((uint8_t*)l_mod, mod_len, &lmod);
    BN_bin2bn((uint8_t*)l_exp, exp_len, &lexp);
    BN_bin2bn((uint8_t*)l_in, in_len, &lin);
    BN_mod_exp(&lout, &lin, &lexp, &lmod, ctx);
    rv = BN_bn2bin(&lout, (uint8_t*)l_out);
    if (rv <= out_len) {
        ssl_reverse_it(l_out, rv);
        memcpy(out, l_out, out_len);
    } else {
        rv = 0;
    }
    BN_free(&lin);
    BN_free(&lout);
    BN_free(&lexp);
    BN_free(&lmod);
    BN_CTX_free(ctx);
    delete [] l_out;
    delete [] l_in;
    delete [] l_mod;
    delete [] l_exp;
    return rv;
}

#if defined(OLD_RSA_GEN1)
/*****************************************************************************/
/* returns error
   generates a new rsa key
   exp is passed in and mod and pri are passed out */
int ssl_gen_key_xrdp1(int key_size_in_bits, uint8_t* exp, int exp_len,
                  uint8_t* mod, int mod_len, uint8_t* pri, int pri_len)
{

    if ((exp_len != 4) || (mod_len != 64) || (pri_len != 64)) {
        return 1;
    }
    uint8_t *lmod = malloc(mod_len * sizeof(char));
    uint8_t *lpri = malloc(pri_len * sizeof(char));
    uint8_t *lexp = (uint8_t*)exp;
    int my_e = lexp[0];
    my_e |= lexp[1] << 8;
    my_e |= lexp[2] << 16;
    my_e |= lexp[3] << 24;
    /* srand is in stdlib.h */
    srand(time(0));
    RSA *my_key = RSA_generate_key(key_size_in_bits, my_e, 0, 0);
    int error = (my_key == 0);
    int len = 0;
    if (error == 0) {
        len = BN_num_bytes(my_key->n);
        error = len != mod_len;
    }
    if (error == 0) {
        BN_bn2bin(my_key->n, (uint8_t*)lmod);
        ssl_reverse_it(lmod, mod_len);
    }
    if (error == 0) {
        len = BN_num_bytes(my_key->d);
        error = len != pri_len;
    }
    if (error == 0) {
        BN_bn2bin(my_key->d, (uint8_t*)lpri);
        ssl_reverse_it(lpri, pri_len);
    }
    if (error == 0) {
        memcpy(mod, lmod, mod_len);
        memcpy(pri, lpri, pri_len);
    }
    RSA_free(my_key);
    free(lmod);
    free(lpri);
    return error;
}
#else
/*****************************************************************************/
/* returns error
   generates a new rsa key
   exp is passed in and mod and pri are passed out */
int ssl_gen_key_xrdp1(int key_size_in_bits, uint8_t* exp, int exp_len,
                  uint8_t* mod, int mod_len, uint8_t* pri, int pri_len)
{
    if ((exp_len != 4) || (mod_len != 64) || (pri_len != 64)) {
        return 1;
    }
    uint8_t *lexp = (uint8_t *)malloc(exp_len * sizeof(uint8_t));
    uint8_t *lmod = (uint8_t *)malloc(mod_len * sizeof(uint8_t));
    uint8_t *lpri = (uint8_t *)malloc(pri_len * sizeof(uint8_t));
    memcpy(lexp, exp, exp_len);
    ssl_reverse_it(lexp, exp_len);
    BIGNUM *my_e = BN_new();
    BN_bin2bn((uint8_t*)lexp, exp_len, my_e);
    RSA* my_key = RSA_new();
    int error = RSA_generate_key_ex(my_key, key_size_in_bits, my_e, 0) == 0;
    int len = 0;
    if (error == 0) {
        len = BN_num_bytes(my_key->n);
        error = len != mod_len;
    }
    if (error == 0) {
        BN_bn2bin(my_key->n, (uint8_t*)lmod);
        ssl_reverse_it(lmod, mod_len);
    }
    if (error == 0) {
        len = BN_num_bytes(my_key->d);
        error = len != pri_len;
    }
    if (error == 0) {
        BN_bn2bin(my_key->d, (uint8_t*)lpri);
        ssl_reverse_it(lpri, pri_len);
    }
    if (error == 0) {
        memcpy(mod, lmod, mod_len);
        memcpy(pri, lpri, pri_len);
    }
    BN_free(my_e);
    RSA_free(my_key);
    free(lexp);
    free(lmod);
    free(lpri);
    return error;
}
#endif


/*****************************************************************************/
/* returns newly allocated SSL_CERT or NULL */

SSL_CERT * ssl_cert_read(uint8_t* data, int len)
{
  /* this will move the data pointer but we don't care, we don't use it again */
  return d2i_X509(NULL, (D2I_X509_CONST unsigned char **) &data, len);
}

/*****************************************************************************/

/* Free an allocated SSL_CERT */
void ssl_cert_free(SSL_CERT * cert)
{
  X509_free(cert);
}

/*****************************************************************************/

/* returns boolean */
int ssl_certs_ok(SSL_CERT * server_cert, SSL_CERT * cacert)
{
  /* Currently, we don't use the CA Certificate.
  FIXME:
  *) Verify the server certificate (server_cert) with the
  CA certificate.
  *) Store the CA Certificate with the hostname of the
  server we are connecting to as key, and compare it
  when we connect the next time, in order to prevent
  MITM-attacks.
  */
  return 1;
}

/*****************************************************************************/

/* returns newly allocated SSL_RKEY or NULL */
SSL_RKEY *ssl_cert_to_rkey(SSL_CERT* cert, uint32_t & key_len)
{
  EVP_PKEY *epk = NULL;
  SSL_RKEY *lkey;
  int nid;

  /* By some reason, Microsoft sets the OID of the Public RSA key to
  the oid for "MD5 with RSA Encryption" instead of "RSA Encryption"

  Kudos to Richard Levitte for the following (. intiutive .)
  lines of code that resets the OID and let's us extract the key. */

  nid = OBJ_obj2nid(cert->cert_info->key->algor->algorithm);
  if ((nid == NID_md5WithRSAEncryption) || (nid == NID_shaWithRSAEncryption))
  {
    ASN1_OBJECT_free(cert->cert_info->key->algor->algorithm);
    cert->cert_info->key->algor->algorithm = OBJ_nid2obj(NID_rsaEncryption);
  }
  epk = X509_get_pubkey(cert);
  if (NULL == epk){
    printf("Failed to extract public key from certificate\n");
    return 0;
  }

  lkey = RSAPublicKey_dup((RSA *) epk->pkey.ptr);
  EVP_PKEY_free(epk);
  key_len = RSA_size(lkey);
  return lkey;
}


/*****************************************************************************/

/* returns error */
int ssl_rkey_get_exp_mod(SSL_RKEY * rkey, uint8_t* exponent, int max_exp_len,
                     uint8_t* modulus, int max_mod_len)
{
  int len;

  if ((BN_num_bytes(rkey->e) > (int) max_exp_len) ||
     (BN_num_bytes(rkey->n) > (int) max_mod_len))
  {
    return 1;
  }
  len = BN_bn2bin(rkey->e, (unsigned char*)exponent);
  reverse(exponent, len);
  len = BN_bn2bin(rkey->n, (unsigned char*)modulus);
  reverse(modulus, len);
  return 0;
}

/*****************************************************************************/
void reverse(uint8_t* p, int len)
{
  int i;
  int j;
  char temp;

  for (i = 0, j = len - 1; i < j; i++, j--)
  {
    temp = p[i];
    p[i] = p[j];
    p[j] = temp;
  }
}

/*****************************************************************************/
