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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean, Clement Moroldo

*/

#pragma once

typedef struct asn1_object_st
{
    const char *sn,*ln;
    int nid;
    int length;
    const unsigned char *data;
    int flags; 
} ASN1_OBJECT;

struct X509_algor_st
{
ASN1_OBJECT *algorithm;
};
typedef struct X509_algor_st X509_ALGOR;

struct X509_pubkey_st
{
X509_ALGOR *algor;

};
typedef struct X509_pubkey_st X509_PUBKEY;

struct x509_cinf_st
{
    X509_PUBKEY *key;
};
typedef struct x509_cinf_st X509_CINF;

struct X509 {
    X509_CINF *cert_info;
};

static inline void X509_free(X509 *) {}

static inline X509 * d2i_X509(void *, void *, size_t) { return nullptr; }

typedef int OBJ_obj2nid;

struct engine_st
{};
typedef struct engine_st ENGINE;

struct bignum_st
{
     //BN_ULONG *d;
     int top;
     int dmax;
     int neg;
     int flags;
};
typedef struct bignum_st BIGNUM;

struct evp_pkey_st
{};
typedef struct evp_pkey_st EVP_PKEY;

struct rsa_meth_st
{
};
typedef struct rsa_meth_st RSA_METHOD;

struct crypto_ex_data_st
{
};
typedef struct crypto_ex_data_st CRYPTO_EX_DATA;

struct rsa_st
{
    int pad;
    long version;
    const RSA_METHOD *meth;
    ENGINE *engine;
    BIGNUM *n;
    BIGNUM *e;
    BIGNUM *d;
    BIGNUM *p;
    BIGNUM *q;
    BIGNUM *dmp1;
    BIGNUM *dmq1;
    BIGNUM *iqmp;

    CRYPTO_EX_DATA ex_data;
    int references;
    int flags;


    char *bignum_data;

};
typedef struct rsa_st RSA;


int BN_bn2bin(const BIGNUM *a, unsigned char *to) {return 0;}
int BN_num_bits(const BIGNUM *a) {return 0;}
void RSA_free(RSA * rsa) {}
int RSA_size(const RSA *r) {return 0;}
void ASN1_OBJECT_free(ASN1_OBJECT *a) {}
ASN1_OBJECT *OBJ_nid2obj(int n) {return nullptr;}
EVP_PKEY * X509_get_pubkey(X509 *cert) {return nullptr;}
void EVP_PKEY_free(EVP_PKEY * epk) {}
RSA *  EVP_PKEY_get1_RSA(EVP_PKEY * epk) {return nullptr;}


#define BN_num_bytes(a) ((BN_num_bits(a)+7)/8)
#define NID_rsaEncryption       6


