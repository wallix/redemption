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

#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <openssl/pem.h>
#include <openssl/dsa.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>

#include "utils/log.hpp"
#include "sashimi/libssh/libssh.h"

#include "sashimi/buffer.hpp"
#include "sashimi/channels.hpp"
#include "utils/log.hpp"
#include "sashimi/pki.hpp"

static char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                         "abcdefghijklmnopqrstuvwxyz"
                         "0123456789+/";

/* Transformations */
#define SET_A(n, i) do { (n) |= ((i) & 63) <<18; } while (0)
#define SET_B(n, i) do { (n) |= ((i) & 63) <<12; } while (0)
#define SET_C(n, i) do { (n) |= ((i) & 63) << 6; } while (0)
#define SET_D(n, i) do { (n) |= ((i) & 63); } while (0)

#define GET_A(n) static_cast<unsigned char>(((n) & 0xff0000) >> 16)
#define GET_B(n) static_cast<unsigned char>(((n) & 0xff00) >> 8)
#define GET_C(n) static_cast<unsigned char>((n) & 0xff)

static int _base64_to_bin(unsigned char dest[3], const char *source, int num);
static int get_equals(char *string);

/* First part: base64 to binary */

/**
 * @internal
 *
 * @brief Translates a base64 string into a binary one.
 *
 * @returns A buffer containing the decoded string, nullptr if something went
 *          wrong (e.g. incorrect char).
 */

// TODO: refactor that shit
ssh_buffer_struct* base64_to_bin(const char *source) {
  ssh_buffer_struct* buffer = nullptr;
  unsigned char block[3];
  char *base64;
  char *ptr;
  size_t len;
  int equals;

  base64 = strdup(source);
  if (base64 == nullptr) {
    return nullptr;
  }
  ptr = base64;

  /* Get the number of equals signs, which mirrors the padding */
  equals = get_equals(ptr);
  if (equals > 2) {
    free(base64);
    return nullptr;
  }

  buffer = new ssh_buffer_struct;
  if (buffer == nullptr) {
    free(base64);
    return nullptr;
  }

  len = strlen(ptr);
  while (len > 4) {
    if (_base64_to_bin(block, ptr, 3) < 0) {
        free(base64);
        delete buffer;
        return nullptr;
    }
    buffer->out_blob(block, 3);
    len -= 4;
    ptr += 4;
  }

  /*
   * Depending on the number of bytes resting, there are 3 possibilities
   * from the RFC.
   */
  switch (len) {
    /*
     * (1) The final quantum of encoding input is an integral multiple of
     *     24 bits. Here, the final unit of encoded output will be an integral
     *     multiple of 4 characters with no "=" padding
     */
    case 4:
      if (equals != 0) {
        break;
      }
      if (_base64_to_bin(block, ptr, 3) < 0) {
        break;
      }
      buffer->out_blob(block, 3);
      free(base64);

      return buffer;
    /*
     * (2) The final quantum of encoding input is exactly 8 bits; here, the
     *     final unit of encoded output will be two characters followed by
     *     two "=" padding characters.
     */
    case 2:
      if (equals != 2){
        break;
      }

      if (_base64_to_bin(block, ptr, 1) < 0) {
        break;
      }
      buffer->out_blob(block, 1);
      free(base64);

      return buffer;
    /*
     * The final quantum of encoding input is exactly 16 bits. Here, the final
     * unit of encoded output will be three characters followed by one "="
     * padding character.
     */
    case 3:
      if (equals != 1) {
        break;
      }
      if (_base64_to_bin(block, ptr, 2) < 0) {
        break;
      }
      buffer->out_blob(block,2);
      free(base64);

      return buffer;
    default:
      /* 4,3,2 are the only padding size allowed */
      break;
  }

  free(base64);
  delete buffer;
  return nullptr;
}

#define BLOCK(letter, n) do {ptr = strchr(alphabet, source[n]); \
                             if(!ptr) return -1; \
                             i = ptr - alphabet; \
                             SET_##letter(*block, i); \
                         } while(0)

/* Returns 0 if ok, -1 if not (ie invalid char into the stuff) */
static int to_block4(unsigned long *block, const char *source, int num) {
  char *ptr;
  unsigned int i;

  *block = 0;
  if (num < 1) {
    return 0;
  }

  BLOCK(A, 0); /* 6 bit */
  BLOCK(B,1); /* 12 bit */

  if (num < 2) {
    return 0;
  }

  BLOCK(C, 2); /* 18 bit */

  if (num < 3) {
    return 0;
  }

  BLOCK(D, 3); /* 24 bit */

  return 0;
}

/* num = numbers of final bytes to be decoded */
static int _base64_to_bin(unsigned char dest[3], const char *source, int num) {
  unsigned long block;

  if (to_block4(&block, source, num) < 0) {
    return -1;
  }
  dest[0] = GET_A(block);
  dest[1] = GET_B(block);
  dest[2] = GET_C(block);

  return 0;
}

/* Count the number of "=" signs and replace them by zeroes */
static int get_equals(char *string) {
  char *ptr = string;
  int num = 0;

  while ((ptr=strchr(ptr,'=')) != nullptr) {
    num++;
    *ptr = '\0';
    ptr++;
  }

  return num;
}

/* thanks sysk for debugging my mess :) */
#define BITS(n) ((1 << (n)) - 1)
static void _bin_to_base64(unsigned char *dest, const unsigned char source[3],
    int len) {
  switch (len) {
    case 1:
      dest[0] = alphabet[(source[0] >> 2)];
      dest[1] = alphabet[((source[0] & BITS(2)) << 4)];
      dest[2] = '=';
      dest[3] = '=';
      break;
    case 2:
      dest[0] = alphabet[source[0] >> 2];
      dest[1] = alphabet[(source[1] >> 4) | ((source[0] & BITS(2)) << 4)];
      dest[2] = alphabet[(source[1] & BITS(4)) << 2];
      dest[3] = '=';
      break;
    case 3:
      dest[0] = alphabet[(source[0] >> 2)];
      dest[1] = alphabet[(source[1] >> 4) | ((source[0] & BITS(2)) << 4)];
      dest[2] = alphabet[ (source[2] >> 6) | (source[1] & BITS(4)) << 2];
      dest[3] = alphabet[source[2] & BITS(6)];
      break;
  }
}

/**
 * @internal
 *
 * @brief Converts binary data to a base64 string.
 *
 * @returns the converted string
 */
std::vector<uint8_t> bin_to_base64(const unsigned char *source, int len);

std::vector<uint8_t> bin_to_base64(const unsigned char *source, int len)
{
  int flen = len + (3 - (len % 3)); /* round to upper 3 multiple */
  flen = (4 * flen) / 3 + 1;

  std::vector<uint8_t> base64(flen);
  unsigned char *ptr = &base64[0];

  while(len > 0){
    _bin_to_base64(ptr, source, len > 3 ? 3 : len);
    ptr += 4;
    source += 3;
    len -= 3;
  }
  ptr[0] = '\0';

  return base64;
}


/**
 * @brief deallocate a SSH key
 * @param [in] pubkey ssh_key_struct *handle to free
 */
void ssh_key_free (ssh_key_struct *pubkey){
    if(pubkey != nullptr){
        if(pubkey->dsa) DSA_free(pubkey->dsa);
        if(pubkey->rsa) RSA_free(pubkey->rsa);
        if(pubkey->ecdsa) EC_KEY_free(pubkey->ecdsa);
        pubkey->flags = SSH_KEY_FLAG_EMPTY;
        pubkey->type = SSH_KEYTYPE_UNKNOWN;
        pubkey->ecdsa_nid = 0;
        pubkey->dsa = nullptr;
        pubkey->rsa = nullptr;
        pubkey->ecdsa = nullptr;
        delete pubkey;
    }
}

void ssh_signature_free(ssh_signature_struct * sig)
{
    if (sig == nullptr) {
        return;
    }

    switch(sig->sig_type) {
        case SSH_KEYTYPE_DSS:
            DSA_SIG_free(sig->dsa_sig);
            break;
        case SSH_KEYTYPE_RSA:
        case SSH_KEYTYPE_RSA1:
            break;
        case SSH_KEYTYPE_ECDSA:
            ECDSA_SIG_free(sig->ecdsa_sig);
            break;
        case SSH_KEYTYPE_UNKNOWN:
            break;
    }

    delete sig;
}

/**
 * @brief Convert a key type to a string.
 *
 * @param[in]  type     The type to convert.
 *
 * @return              A string for the keytype or nullptr if unknown.
 */
const char *ssh_key_type_to_char(enum ssh_keytypes_e type) {
  switch (type) {
    case SSH_KEYTYPE_DSS:
      return "ssh-dss";
    case SSH_KEYTYPE_RSA:
      return "ssh-rsa";
    case SSH_KEYTYPE_RSA1:
      return "ssh-rsa1";
    case SSH_KEYTYPE_ECDSA:
      return "ssh-ecdsa";
    case SSH_KEYTYPE_UNKNOWN:
      return "";
  }
  return "";
}

/**
 * @internal
 *
 * @brief Import a public key from a ssh string.
 *
 * @param[in]  buffer The key blob to import as specified in RFC 4253 section
 *                      6.6 "Public Key Algorithms".
 *
 * @param[out] pkey     A pointer where the allocated key can be stored. You
 *                      need to free the memory.
 *
 * @return              SSH_OK on success, SSH_ERROR on error.
 *
 * @see ssh_key_free()
 */
int ssh_pki_import_pubkey_blob(ssh_buffer_struct & buffer, ssh_key_struct **pkey) {
    LOG(LOG_INFO, "%s common 1", __FUNCTION__);
    if (sizeof(uint32_t) > buffer.in_remain()) {
        LOG(LOG_INFO, "%s common error 2", __FUNCTION__);
        //ERRRRRRRRRRRRRRRRRRRRRRRRRR
    }
    LOG(LOG_INFO, "%s common 2", __FUNCTION__);
    uint32_t str_len = buffer.in_uint32_be();
    if (str_len > buffer.in_remain()) {
        LOG(LOG_INFO, "%s error 2", __FUNCTION__);
        //ERRRRRRRRRRRRRRRRRRRRRRRRRR
    }
    SSHString name;
    name.resize(str_len);
    buffer.buffer_get_data(&name[0], str_len);

    std::initializer_list<std::pair<const char *, enum ssh_keytypes_e>> l = {
         {"rsa1", SSH_KEYTYPE_RSA1},
         {"ssh-rsa1", SSH_KEYTYPE_RSA1},
         {"rsa", SSH_KEYTYPE_RSA},
         {"ssh-rsa", SSH_KEYTYPE_RSA},
         {"dsa", SSH_KEYTYPE_DSS},
         {"ssh-dss", SSH_KEYTYPE_DSS},
         {"ecdsa", SSH_KEYTYPE_ECDSA},
         {"ssh-ecdsa", SSH_KEYTYPE_ECDSA},
         {"ecdsa-sha2-nistp256", SSH_KEYTYPE_ECDSA},
         {"ecdsa-sha2-nistp384", SSH_KEYTYPE_ECDSA},
         {"ecdsa-sha2-nistp521", SSH_KEYTYPE_ECDSA},
        };

    enum ssh_keytypes_e type = SSH_KEYTYPE_UNKNOWN;
    for(auto &p:l){
        if (strcmp(p.first, name.c_str()) == 0){
            type = p.second;
            break;
        }
    }

    *pkey = new ssh_key_struct(type, SSH_KEY_FLAG_PUBLIC);

    switch (type){
    case SSH_KEYTYPE_RSA1:
    case SSH_KEYTYPE_RSA:
    {
        LOG(LOG_INFO, "%s RSA", __FUNCTION__);
        if (sizeof(uint32_t) > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        uint32_t e_len = buffer.in_uint32_be();
        if (e_len > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        std::vector<uint8_t> e;
        e.resize(e_len);
        buffer.buffer_get_data(&e[0],e_len);

        if (sizeof(uint32_t) > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        uint32_t n_len = buffer.in_uint32_be();
        if (n_len > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        std::vector<uint8_t> n;
        n.resize(n_len);
        buffer.buffer_get_data(&n[0],n_len);

        (*pkey)->rsa = RSA_new();
        if ((*pkey)->rsa == nullptr) {
            LOG(LOG_INFO, "%s RSA err", __FUNCTION__);
            ssh_key_free(*pkey);
            return SSH_ERROR;
        }

        (*pkey)->rsa->e = BN_bin2bn(&e[0], e.size(), nullptr);
        (*pkey)->rsa->n = BN_bin2bn(&n[0], n.size(), nullptr);
        if ((*pkey)->rsa->e == nullptr || (*pkey)->rsa->n == nullptr) {
            RSA_free((*pkey)->rsa);
            LOG(LOG_INFO, "%s RSA err", __FUNCTION__);
            ssh_key_free(*pkey);
            return SSH_ERROR;
        }

        LOG(LOG_INFO, "%s RSA OK", __FUNCTION__);
        return SSH_OK;
    }
    case SSH_KEYTYPE_DSS:
    {
        LOG(LOG_INFO, "%s DSS", __FUNCTION__);
        if (sizeof(uint32_t) > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        uint32_t p_len = buffer.in_uint32_be();
        if (p_len > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        std::vector<uint8_t> p;
        p.resize(p_len);
        buffer.buffer_get_data(&p[0],p_len);

        if (sizeof(uint32_t) > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        uint32_t q_len = buffer.in_uint32_be();
        if (q_len > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        std::vector<uint8_t> q;
        q.resize(q_len);
        buffer.buffer_get_data(&q[0],q_len);

        if (sizeof(uint32_t) > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        uint32_t g_len = buffer.in_uint32_be();
        if (g_len > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        std::vector<uint8_t> g;
        g.resize(g_len);
        buffer.buffer_get_data(&g[0],g_len);

        if (sizeof(uint32_t) > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        uint32_t pubkey_len = buffer.in_uint32_be();
        if (pubkey_len > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        std::vector<uint8_t> pubkey;
        pubkey.resize(pubkey_len);
        buffer.buffer_get_data(&pubkey[0],pubkey_len);

        (*pkey)->dsa = DSA_new();
        if ((*pkey)->dsa == nullptr) {
            LOG(LOG_INFO, "%s DSS ERR", __FUNCTION__);
            ssh_key_free(*pkey);
            return SSH_ERROR;
        }

        (*pkey)->dsa->p = BN_bin2bn(&p[0], p.size(), nullptr);
        (*pkey)->dsa->q = BN_bin2bn(&q[0], q.size(), nullptr);
        (*pkey)->dsa->g = BN_bin2bn(&g[0], g.size(), nullptr);
        (*pkey)->dsa->pub_key = BN_bin2bn(&pubkey[0], pubkey.size(), nullptr);
        if ((*pkey)->dsa->p == nullptr || (*pkey)->dsa->q == nullptr
         || (*pkey)->dsa->g == nullptr || (*pkey)->dsa->pub_key == nullptr) {
            DSA_free((*pkey)->dsa);
            LOG(LOG_INFO, "%s DSS ERR", __FUNCTION__);
            ssh_key_free(*pkey);
            return SSH_ERROR;
        }

        LOG(LOG_INFO, "%s DSS OK", __FUNCTION__);
        return SSH_OK;
    }
    case SSH_KEYTYPE_ECDSA:
    {
        LOG(LOG_INFO, "%s ECDSA", __FUNCTION__);
        if (sizeof(uint32_t) > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        uint32_t i_len = buffer.in_uint32_be();
        if (i_len > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        char * name = reinterpret_cast<char *>(buffer.get_pos_ptr());
        int nid = (strncmp(name, "nistp256", i_len) == 0)? NID_X9_62_prime256v1
                : (strncmp(name, "nistp384", i_len) == 0)? NID_secp384r1
                : (strncmp(name, "nistp521", i_len) == 0)? NID_secp521r1
                : -1;
        if (nid == -1) {
            LOG(LOG_INFO, "%s ECDSA ERR", __FUNCTION__);
            ssh_key_free(*pkey);
            return SSH_ERROR;
        }
        buffer.in_skip_bytes(i_len);

        if (sizeof(uint32_t) > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        uint32_t e_len = buffer.in_uint32_be();
        if (e_len > buffer.in_remain()) {
            //ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        std::vector<uint8_t> e;
        e.resize(e_len);
        buffer.buffer_get_data(&e[0],e.size());

        (*pkey)->ecdsa_nid = nid;
        (*pkey)->ecdsa = EC_KEY_new_by_curve_name((*pkey)->ecdsa_nid);
        if ((*pkey)->ecdsa == nullptr) {
            return -1;
        }

        const EC_GROUP *g = EC_KEY_get0_group((*pkey)->ecdsa);

        EC_POINT *p = EC_POINT_new(g);
        if (p == nullptr) {
            LOG(LOG_INFO, "%s ECDSA ERR 2", __FUNCTION__);
            ssh_key_free(*pkey);
            return SSH_ERROR;
        }

        int ok = EC_POINT_oct2point(g, p, &e[0], e.size(), nullptr);
        if (!ok) {
            EC_POINT_free(p);
            LOG(LOG_INFO, "%s ECDSA ERR 2", __FUNCTION__);
            ssh_key_free(*pkey);
            return SSH_ERROR;
        }

        /* EC_KEY_set_public_key duplicates p */
        ok = EC_KEY_set_public_key((*pkey)->ecdsa, p);
        EC_POINT_free(p);
        if (!ok) {
            LOG(LOG_INFO, "%s ECDSA ERR 2", __FUNCTION__);
            ssh_key_free(*pkey);
            return SSH_ERROR;
        }

        LOG(LOG_INFO, "%s ECDSA OK", __FUNCTION__);
        return SSH_OK;
    }
    case SSH_KEYTYPE_UNKNOWN:
    default:
        LOG(LOG_INFO, "%s Unknown key type found!", __FUNCTION__);
        break;
    }
    LOG(LOG_INFO, "Unknown key type found!");
    return SSH_ERROR;
}



/**
 * @brief Convert a public key to a base64 hased key.
 *
 * @param[in] pubkey       The key to hash
 *
 * @param[out] b64_key  A pointer to store the allocated base64 hashed key. You
 *                      need to free the buffer.
 *
 * @return              SSH_OK on success, SSH_ERROR on error.
 *
 * @see free_char()
 */
inline int ssh_pki_export_pubkey_base64(const ssh_key_struct *pubkey, std::vector<uint8_t> & b64_key)
{
    LOG(LOG_INFO, "%s", __FUNCTION__);

    if (pubkey == nullptr) {
        return SSH_ERROR;
    }

    std::vector<uint8_t> key_blob;
    switch (pubkey->type) {
        case SSH_KEYTYPE_DSS:
        {
            ssh_buffer_struct buffer;
            buffer.out_length_prefixed_cstr(pubkey->type_c());
            LOG(LOG_INFO, "%s SSH_KEYTYPE_DSS", __FUNCTION__);
            buffer.out_bignum(pubkey->dsa->p); // p
            buffer.out_bignum(pubkey->dsa->q); // q
            buffer.out_bignum(pubkey->dsa->g); // g
            buffer.out_bignum(pubkey->dsa->pub_key); // n
            key_blob.resize(static_cast<uint32_t>(buffer.in_remain()));
            memcpy(&key_blob[0], buffer.get_pos_ptr(), key_blob.size());
        }
        break;
        case SSH_KEYTYPE_RSA:
        case SSH_KEYTYPE_RSA1:
        {
            ssh_buffer_struct buffer;
            buffer.out_length_prefixed_cstr(pubkey->type_c());
            LOG(LOG_INFO, "%s SSH_KEYTYPE_RSA", __FUNCTION__);
            buffer.out_bignum(pubkey->rsa->e); // e
            buffer.out_bignum(pubkey->rsa->n); // n
            key_blob.resize(static_cast<uint32_t>(buffer.in_remain()));
            memcpy(&key_blob[0], buffer.get_pos_ptr(), key_blob.size());
        }
        break;
        case SSH_KEYTYPE_ECDSA:
        {
            LOG(LOG_INFO, "%s SSH_KEYTYPE_ECDSA", __FUNCTION__);
            ssh_buffer_struct buffer;
            buffer.out_length_prefixed_cstr(pubkey->type_c());

            buffer.out_length_prefixed_cstr(
                (pubkey->ecdsa_nid == NID_X9_62_prime256v1) ? "nistp256" :
                (pubkey->ecdsa_nid == NID_secp384r1)        ? "nistp384" :
                (pubkey->ecdsa_nid == NID_secp521r1)        ? "nistp521" :
                "unknown");

            const EC_GROUP *g = EC_KEY_get0_group(pubkey->ecdsa);
            const EC_POINT *p = EC_KEY_get0_public_key(pubkey->ecdsa);

            size_t len_ec = EC_POINT_point2oct(g, p, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0, nullptr);
            if (len_ec == 0) {
                return SSH_ERROR;
            }

            std::vector<uint8_t> e;
            e.resize(static_cast<uint32_t>(len_ec));
            if (e.size() != EC_POINT_point2oct(g, p, POINT_CONVERSION_UNCOMPRESSED, &e[0], e.size(), nullptr)){
                return SSH_ERROR;
            }

            buffer.out_uint32_be(e.size());
            buffer.out_blob(&e[0], e.size());
            key_blob.resize(static_cast<uint32_t>(buffer.in_remain()));
            memcpy(&key_blob[0], buffer.get_pos_ptr(), key_blob.size());
        }
        break;
        case SSH_KEYTYPE_UNKNOWN:
            LOG(LOG_INFO, "%s SSH_KEYTYPE_UNKNOWN", __FUNCTION__);
        return SSH_ERROR;
    }

    b64_key = bin_to_base64(&key_blob[0], key_blob.size());
    return SSH_OK;
}

int ssh_pki_export_pubkey_base64_p(const ssh_key_struct *key, char *b64, int b64_len)
{
    std::vector<uint8_t> buf;
    int rc = ssh_pki_export_pubkey_base64(key, buf);
    if (rc != SSH_OK) {
        return SSH_ERROR;
    }
    if (static_cast<unsigned>(b64_len) < buf.size()){
        return SSH_ERROR;
    }
    memcpy(b64, &buf[0], buf.size());
    return SSH_OK;
}



inline std::vector<uint8_t> pki_signature_to_blob(const ssh_signature_struct * sig)
{
    switch(sig->sig_type) {
        case SSH_KEYTYPE_DSS:
        {
            std::vector<uint8_t> sig_blob;
            sig_blob.resize(40);

            unsigned int len3 = BN_num_bytes(sig->dsa_sig->r);
            unsigned int bits3 = BN_num_bits(sig->dsa_sig->r);
            /* If the first bit is set we have a negative number, padding needed */
            int pad3 = ((bits3 % 8) == 0 && BN_is_bit_set(sig->dsa_sig->r, bits3 - 1))?1:0;
            std::vector<uint8_t> r;
            r.resize(len3 + pad3);
            /* We have a negative number henceforth we need a leading zero */
            r[0] = 0;
            BN_bn2bin(sig->dsa_sig->r, &r[pad3]);

            // TODO: check that, suspicious
            int r_offset_in  = (r.size() > 20) ? (r.size() - 20) : 0;
            int r_offset_out = (r.size() < 20) ? (20 - r.size()) : 0;

            memcpy(&sig_blob[r_offset_out], &r[r_offset_in], r.size() - r_offset_in);

            unsigned int len4 = BN_num_bytes(sig->dsa_sig->s);
            unsigned int bits4 = BN_num_bits(sig->dsa_sig->s);
            /* If the first bit is set we have a negative number, padding needed */
            int pad4 = ((bits4 % 8) == 0 && BN_is_bit_set(sig->dsa_sig->s, bits4 - 1))?1:0;
            std::vector<uint8_t> s;
            s.resize(len4 + pad4);
            /* if pad we have a negative number henceforth we need a leading zero */
            s[0] = 0;
            BN_bn2bin(sig->dsa_sig->s, &s[pad4]);

            // TODO: check that, suspicious
            int s_offset_in  = (s.size() > 20) ? (s.size() - 20) : 0;
            int s_offset_out = (s.size() < 20) ? (20 - s.size()) : 0;

            memcpy(&sig_blob[20 + s_offset_out], &s[s_offset_in], s.size() - s_offset_in);

            return sig_blob;
        }
        case SSH_KEYTYPE_RSA:
        case SSH_KEYTYPE_RSA1:
        {
            // TODO: check that, very suspicious, why don't we compute sig_blob like for dsa or ecdsa ?
            // copy signature blob
            std::vector<uint8_t> sig_blob;
            sig_blob.resize(sig->rsa_sig.size());
            memcpy(&sig_blob[0], &sig->rsa_sig[0], sig->rsa_sig.size());
            return sig_blob;
        }
        case SSH_KEYTYPE_ECDSA:
        {
            ssh_buffer_struct b;

            unsigned int len3 = BN_num_bytes(sig->ecdsa_sig->r);
            unsigned int bits3 = BN_num_bits(sig->ecdsa_sig->r);
            /* If the first bit is set we have a negative number, padding needed */
            int pad3 = ((bits3 % 8) == 0 && BN_is_bit_set(sig->ecdsa_sig->r, bits3 - 1))?1:0;
            std::vector<uint8_t> r;
            r.resize(len3 + pad3);
            /* if pad we have a negative number henceforth we need a leading zero */
            r[0] = 0;
            BN_bn2bin(sig->ecdsa_sig->r, &r[pad3]);

            b.out_uint32_be(r.size());
            b.out_blob(&r[0], r.size());

            unsigned int len4 = BN_num_bytes(sig->ecdsa_sig->s);
            unsigned int bits4 = BN_num_bits(sig->ecdsa_sig->s);
            /* If the first bit is set we have a negative number, padding needed */
            int pad4 = ((bits4 % 8) == 0 && BN_is_bit_set(sig->ecdsa_sig->s, bits4 - 1))?1:0;
            std::vector<uint8_t> s;
            s.resize(len4 + pad4);
            /* if pad we have a negative number henceforth we need a leading zero */
            s[0] = 0;
            BN_bn2bin(sig->ecdsa_sig->s, &s[pad4]);

            b.out_uint32_be(s.size());
            b.out_blob(&s[0], s.size());

            std::vector<uint8_t> sig_blob;
            sig_blob.resize(static_cast<uint32_t>(b.in_remain()));
            memcpy(&sig_blob[0], b.get_pos_ptr(), b.in_remain());

            return sig_blob;
        }
        case SSH_KEYTYPE_UNKNOWN:
            LOG(LOG_INFO, "Unknown signature key type: %s", sig->type_c());
            return std::vector<uint8_t>{};
    }
    return std::vector<uint8_t>{};
}

std::vector<uint8_t> ssh_pki_export_signature_blob(const ssh_key_struct *key, const unsigned char *hash, size_t hlen)
{
    ssh_signature_struct * sig = new ssh_signature_struct;
    sig->sig_type = key->type;

    switch(key->type) {
        case SSH_KEYTYPE_DSS:
            sig->dsa_sig = DSA_do_sign(hash, hlen, key->dsa);
            if (sig->dsa_sig == nullptr) {
                ssh_signature_free(sig);
                return std::vector<uint8_t>();
            }
            break;
        case SSH_KEYTYPE_RSA:
        case SSH_KEYTYPE_RSA1:
        {
            unsigned int slen;
            std::vector<uint8_t> sig1(RSA_size(key->rsa));
            RSA_sign(NID_sha1, hash, hlen, &sig1[0], &slen, key->rsa);
            sig->rsa_sig = std::move(sig1);
        }
        break;
        case SSH_KEYTYPE_ECDSA:
            sig->ecdsa_sig = ECDSA_do_sign(hash, hlen, key->ecdsa);
            if (sig->ecdsa_sig == nullptr) {
                ssh_signature_free(sig);
                return std::vector<uint8_t>();
            }
            break;
        case SSH_KEYTYPE_UNKNOWN:
            ssh_signature_free(sig);
            return std::vector<uint8_t>();
    }

    ssh_buffer_struct* buf = new ssh_buffer_struct;

    const char * type_c = sig->type_c();
    SSHString str(type_c);

    buf->out_uint32_be(str.size());
    buf->out_blob(&str[0], str.size());

    std::vector<uint8_t> str2 = pki_signature_to_blob(sig);

    buf->out_uint32_be(str2.size());
    buf->out_blob(&str2[0], str2.size());

    std::vector<uint8_t> sig_blob(static_cast<uint32_t>(buf->in_remain()));
    memcpy(&sig_blob[0], buf->get_pos_ptr(), buf->in_remain());

    delete buf;
    delete sig;

    return sig_blob;
}

static ssh_signature_struct * pki_signature_from_blob(const ssh_key_struct *pubkey, const SSHString & sig_blob, enum ssh_keytypes_e type)
{
    ssh_signature_struct * sig = new ssh_signature_struct;
    sig->sig_type = type;

    switch(type) {
        case SSH_KEYTYPE_DSS:
        {
            /* 40 is the dual signature blob len. */
            if (sig_blob.size() != 40) {
                LOG(LOG_INFO, "Signature has wrong size: %lu", sig_blob.size());
                ssh_signature_free(sig);
                return nullptr;
            }

            sig->dsa_sig = DSA_SIG_new();
            if (sig->dsa_sig == nullptr) {
                ssh_signature_free(sig);
                return nullptr;
            }

            std::vector<uint8_t> r;
            r.resize(20);
            memcpy(&r[0], &sig_blob[0], 20);

            sig->dsa_sig->r = BN_bin2bn(&r[0], r.size(), nullptr);
            if (sig->dsa_sig->r == nullptr) {
                ssh_signature_free(sig);
                return nullptr;
            }

            std::vector<uint8_t> s(20);
            memcpy(&s[0], &sig_blob[20], 20);

            sig->dsa_sig->s = BN_bin2bn(&s[0], s.size(), nullptr);
            if (sig->dsa_sig->s == nullptr) {
                ssh_signature_free(sig);
                return nullptr;
            }
        }
        break;
        case SSH_KEYTYPE_RSA:
        case SSH_KEYTYPE_RSA1:
        {
            uint32_t pad_len = 0;
            size_t rsalen = RSA_size(pubkey->rsa);

            if (sig_blob.size() > rsalen) {
                LOG(LOG_INFO, "Signature is too big: %lu > %lu", sig_blob.size(), rsalen);
                ssh_signature_free(sig);
            }

            #ifdef DEBUG_CRYPTO
                LOG(LOG_INFO, "RSA signature len: %lu", static_cast<unsigned char>(sig_blob.size()));
                ssh_print_hexa("RSA signature", &sig_blob[0], sig_blob.size());
            #endif

            if (sig_blob.size() == rsalen) {
                    sig->rsa_sig.resize(sig_blob.size());
                    memcpy(&sig->rsa_sig[0], &sig_blob[0], sig_blob.size());
            }
            else {
                /* pad the blob to the expected rsalen size */
                LOG(LOG_INFO, "RSA signature len %lu < %lu", sig_blob.size(), rsalen);

                pad_len = rsalen - sig_blob.size();

                std::vector<uint8_t> sig_blob_padded(static_cast<uint32_t>(rsalen));

                /* front-pad the buffer with zeroes */
                memset(&sig_blob_padded[0], 0, pad_len);
                /* fill the rest with the actual signature blob */
                memcpy(&sig_blob_padded[pad_len], &sig_blob[0], sig_blob.size());
                sig->rsa_sig = std::move(sig_blob_padded);
            }
        }
        break;
        case SSH_KEYTYPE_ECDSA:
            sig->ecdsa_sig = ECDSA_SIG_new();
            if (sig->ecdsa_sig == nullptr) {
                ssh_signature_free(sig);
                return nullptr;
            }

            { /* build ecdsa signature */
                ssh_buffer_struct* b;

                b = new ssh_buffer_struct;
                if (b == nullptr) {
                    ssh_signature_free(sig);
                    return nullptr;
                }

                b->out_blob(&sig_blob[0], sig_blob.size());

                if (sizeof(uint32_t) > b->in_remain()){
                    delete b;
                    ssh_signature_free(sig);
                    return nullptr;
                }
                uint32_t r_len = b->in_uint32_be();
                if (r_len > b->in_remain()){
                    delete b;
                    ssh_signature_free(sig);
                    return nullptr;
                }
                uint8_t * r = new uint8_t[r_len];
                b->buffer_get_data(r, r_len);
                // TODO: is there error management for BN_bin2bn
                sig->ecdsa_sig->r = BN_bin2bn(r, r_len, nullptr);

                if (r){
                  delete [] r;
                }

                if (sig->ecdsa_sig->r == nullptr) {
                    delete b;
                    ssh_signature_free(sig);
                    return nullptr;
                }

                if (sizeof(uint32_t) > b->in_remain()){
                    delete b;
                    ssh_signature_free(sig);
                    return nullptr;
                }
                uint32_t s_len = b->in_uint32_be();
                if (s_len > b->in_remain()){
                    delete b;
                    ssh_signature_free(sig);
                    return nullptr;
                }

                uint8_t * s = new uint8_t[s_len];
                b->buffer_get_data(s, s_len);
                // TODO: is there error management for BN_bin2bn
                sig->ecdsa_sig->s = BN_bin2bn(s, s_len, nullptr);
                if (s){
                  delete [] s;
                }

                if (sig->ecdsa_sig->s == nullptr) {
                    delete b;
                    ssh_signature_free(sig);
                    return nullptr;
                }

                if (b->in_remain() != 0) {
                    LOG(LOG_INFO, "Signature has remaining bytes in inner "
                                "sigblob: %lu",
                                b->in_remain());
                    delete b;
                    ssh_signature_free(sig);
                    return nullptr;
                }
                delete b;
            }

            break;
        case SSH_KEYTYPE_UNKNOWN:
            LOG(LOG_INFO, "Unknown signature type");
            ssh_signature_free(sig);
            return nullptr;
    }

    return sig;
}


int ssh_pki_signature_verify_blob(const std::vector<uint8_t> & sig_blob,
                                  const ssh_key_struct *key,
                                  unsigned char *digest,
                                  size_t dlen,
                                  struct error_struct & error)
{
    ssh_buffer_struct* buf = new ssh_buffer_struct;
    buf->out_blob(&sig_blob[0], sig_blob.size());

    if (sizeof(uint32_t) > buf->in_remain()) {
        //ERRRRRRRRRRRRRRRRRRRRRRRRRR
    }
    uint32_t str_len = buf->in_uint32_be();
    if (str_len > buf->in_remain()) {
        //ERRRRRRRRRRRRRRRRRRRRRRRRRR
    }
    SSHString str;
    str.resize(str_len);
    buf->buffer_get_data(&str[0], str_len);

    std::initializer_list<std::pair<const char *, enum ssh_keytypes_e>> l = {
         {"rsa1", SSH_KEYTYPE_RSA1},
         {"ssh-rsa1", SSH_KEYTYPE_RSA1},
         {"rsa", SSH_KEYTYPE_RSA},
         {"ssh-rsa", SSH_KEYTYPE_RSA},
         {"dsa", SSH_KEYTYPE_DSS},
         {"ssh-dss", SSH_KEYTYPE_DSS},
         {"ecdsa", SSH_KEYTYPE_ECDSA},
         {"ssh-ecdsa", SSH_KEYTYPE_ECDSA},
         {"ecdsa-sha2-nistp256", SSH_KEYTYPE_ECDSA},
         {"ecdsa-sha2-nistp384", SSH_KEYTYPE_ECDSA},
         {"ecdsa-sha2-nistp521", SSH_KEYTYPE_ECDSA},
        };

    enum ssh_keytypes_e type = SSH_KEYTYPE_UNKNOWN;
    for(auto &p:l){
        if (strcmp(p.first, str.c_str()) == 0){
            type = p.second;
            break;
        }
    }

    if (sizeof(uint32_t) > buf->in_remain()) {
        //ERRRRRRRRRRRRRRRRRRRRRRRRRR
    }
    uint32_t tmp_len = buf->in_uint32_be();
    if (tmp_len > buf->in_remain()) {
        //ERRRRRRRRRRRRRRRRRRRRRRRRRR
    }
    SSHString tmp;
    tmp.resize(tmp_len);
    buf->buffer_get_data(&tmp[0],tmp_len);

    ssh_signature_struct * sig = pki_signature_from_blob(key, std::move(tmp), type);
    delete buf;

    LOG(LOG_INFO, "Going to verify a %s type signature", key->type_c());

    // max(SHA_DIGEST_LENGTH, SHA256_DIGEST_LENGTH, EVP_DIGEST_LEN)

    union any_hash {
        unsigned char evp[SHA256_DIGEST_LENGTH];
        unsigned char sha256[SHA256_DIGEST_LENGTH];
        unsigned char sha1[SHA256_DIGEST_LENGTH];
    };
    unsigned char hash[sizeof(any_hash)] = {0};

    uint32_t elen = 0;

    switch (key->type)
    {
    case SSH_KEYTYPE_ECDSA:
    {
        EVP_MD_CTX md;

        switch (key->ecdsa_nid) {
            case NID_X9_62_prime256v1:
            {
                const EVP_MD *evp_md = EVP_sha256();
                EVP_DigestInit(&md, evp_md);
                EVP_DigestUpdate(&md, digest, dlen);
                EVP_DigestFinal(&md, hash, &elen);
            }
            break;
            case NID_secp384r1:
            {
                const EVP_MD *evp_md = EVP_sha384();
                EVP_DigestInit(&md, evp_md);
                EVP_DigestUpdate(&md, digest, dlen);
                EVP_DigestFinal(&md, hash, &elen);
            }
            break;
            case NID_secp521r1:
            {
                const EVP_MD *evp_md = EVP_sha512();
                EVP_DigestInit(&md, evp_md);
                EVP_DigestUpdate(&md, digest, dlen);
                EVP_DigestFinal(&md, hash, &elen);
            }
            break;
            default:
                ;
        }

        int rc = ECDSA_do_verify(hash,
                                 elen,
                                 sig->ecdsa_sig,
                                 key->ecdsa);
        if (rc <= 0) {
            ssh_set_error(error,
                          SSH_FATAL,
                          "ECDSA error: %s",
                          ERR_error_string(ERR_get_error(), nullptr));
            return SSH_ERROR;
        }

        ssh_signature_free(sig);
        return rc;
    }
    case SSH_KEYTYPE_DSS:
    {
        SslSha1 sha1;
        sha1.update(digest, dlen);
        sha1.final(hash);
        //hexa("Hash to be verified with dsa", hash, SHA_DIGEST_LENGTH);
        int rc = DSA_do_verify(hash,
                           SHA_DIGEST_LENGTH,
                           sig->dsa_sig,
                           key->dsa);
        if (rc <= 0) {
            ssh_set_error(error,
                          SSH_FATAL,
                          "DSA error: %s",
                          ERR_error_string(ERR_get_error(), nullptr));
            return SSH_ERROR;
        }
        ssh_signature_free(sig);
    }
    break;
    case SSH_KEYTYPE_RSA:
    case SSH_KEYTYPE_RSA1:
    {
        SslSha1 sha1;
        sha1.update(digest, dlen);
        sha1.final(hash);
        //hexa("Hash to be verified with dsa", hash, SHA_DIGEST_LENGTH);

        int rc = RSA_verify(NID_sha1,
                        hash,
                        SHA_DIGEST_LENGTH,
                        &sig->rsa_sig[0],
                        sig->rsa_sig.size(),
                        key->rsa);
        if (rc <= 0) {
            ssh_set_error(error,
                          SSH_FATAL,
                          "RSA error: %s",
                          ERR_error_string(ERR_get_error(), nullptr));
            return SSH_ERROR;
        }
        ssh_signature_free(sig);
    }
    break;
    case SSH_KEYTYPE_UNKNOWN:
    default:
    {
        ssh_set_error(error, SSH_FATAL, "Unknown public key type");
        ssh_signature_free(sig);
        return SSH_ERROR;
    }
    }
    return SSH_OK;
}


/**
 * @}
 */
