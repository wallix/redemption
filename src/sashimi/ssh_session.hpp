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

#include <vector>

#define GZIP_BLOCKSIZE 4092

#include "sashimi/event.hpp"

#include "sashimi/pki.hpp"
#include "sashimi/libcrypto.hpp"

#include "cxx/cxx.hpp"
#include "cxx/diagnostic.hpp"

#include <gssapi/gssapi.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")

struct ssh_session_struct {
    struct error_struct error;
    struct ssh_poll_handle_struct * poll;
    int log_verbosity; /* verbosity of the log functions */

    struct ssh_socket_struct *socket;
    char serverbanner[128];
    char clientbanner[128];
    int protoversion;
    int openssh;
    uint32_t send_seq;
    uint32_t recv_seq;

    int connected;
    /* two previous are deprecated */
    /* int auth_service_asked; */

    /* session flags (SSH_SESSION_FLAG_*) */
    int flags;

    SSHString banner; /* that's the issue banner from the server */
    char *discon_msg; /* disconnect message from
                         the remote host */
    ssh_buffer_struct* in_buffer;

    uint8_t in_packet_type;
    uint8_t in_packet_valid;
    uint32_t in_packet_len;

    ssh_buffer_struct* out_buffer;

    /* the states are used by the nonblocking stuff to remember */
    /* where it was before being interrupted */
    enum ssh_pending_call_e pending_call_state;
    enum ssh_session_state_e session_state;
    int packet_state;
    enum ssh_dh_state_e dh_handshake_state;
    enum ssh_auth_service_state_e auth_service_state;
    enum ssh_auth_state_e auth_state;
    enum ssh_channel_request_state_e global_req_state;
    struct ssh_auth_auto_state_struct *auth_auto_state;

    unsigned char * session_id;
    struct ssh_crypto_struct *current_crypto;
    struct ssh_crypto_struct *next_crypto;  /* next_crypto is going to be used after a SSH_MSG_NEWKEYS */

    unsigned nbchannels;
    ssh_channel_struct * channels[4096];
    int maxchannel;
    int exec_channel_opened; /* version 1 only. more
                                info in channels1.c */
    struct ssh_agent_state_struct *agent_state;
    ssh_agent_struct* agent; /* ssh agent */

/* keyb interactive data */
    struct ssh_kbdint_struct *tmp_kbdint;
    struct ssh_kbdint_struct *kbdint;
    struct ssh_gssapi_struct *gssapi;
    int version; /* 1 or 2 */

    /* server host keys */
    ssh_key_struct *server_rsa_key;
    ssh_key_struct *server_dsa_key;
    ssh_key_struct *server_ecdsa_key;
    enum ssh_keytypes_e server_negociated_hostkey;

//    bool ssh_connection_is_client;

    enum class data_type_e {
        HANDLE_RECEIVED_DATA = 0,
        CALLBACK_RECEIVE_BANNER,
    };

    /**
     * This function will be called each time data appears on socket. The data
     * not consumed will appear on the next data event.
     */
    data_type_e socket_callbacks_data_type;
    ssh_poll_ctx_struct * ctx;
    /* options */
    struct options_struct opts;

    char showbuffer[1024];

    std::vector<Event*> waiting_list;

    ssh_session_struct(ssh_poll_ctx_struct * ctx, ssh_socket_struct * socket) :
      poll(nullptr)
    , log_verbosity(0)
    , socket(socket)
    , serverbanner{}
    , clientbanner{}
    , protoversion(0)
    , openssh(0)
    , send_seq(0)
    , recv_seq(0)
    , connected(0)
    // session flags (SSH_SESSION_FLAG_*)
    , flags(SSH_SESSION_FLAG_BLOCKING)
    , banner{} // that's the issue banner from the server
    , discon_msg(nullptr) // disconnect message from the remote host
    , in_buffer(new ssh_buffer_struct)

    , in_packet_type(0)
    , in_packet_valid(0)
    , in_packet_len(0)

    , out_buffer(new ssh_buffer_struct)

    // the states are used by the nonblocking stuff to remember
    // what it was doing before being interrupted
    , pending_call_state(ssh_pending_call_e::SSH_PENDING_CALL_NONE)
    , session_state(ssh_session_state_e::SSH_SESSION_STATE_NONE)
    , packet_state(0)
    , dh_handshake_state(ssh_dh_state_e::DH_STATE_INIT)
    , auth_service_state(ssh_auth_service_state_e::SSH_AUTH_SERVICE_NONE)
    , auth_state(ssh_auth_state_e::SSH_AUTH_STATE_NONE)
    , global_req_state(ssh_channel_request_state_e::SSH_CHANNEL_REQ_STATE_NONE)
    , auth_auto_state(nullptr)
    , session_id(nullptr)
    , current_crypto(nullptr)
    , next_crypto(new ssh_crypto_struct())  // next_crypto is going to be used after a SSH_MSG_NEWKEYS
    , nbchannels(0)
    , maxchannel(FIRST_CHANNEL)
    , exec_channel_opened(0) // ssh version 1 only. more info in channels1.c
    , agent_state(nullptr)
    , agent(nullptr)
    , tmp_kbdint(nullptr)
    , kbdint(nullptr)
    , gssapi(nullptr)
    , version(2)
    , server_rsa_key(nullptr)
    , server_dsa_key(nullptr)
    , server_ecdsa_key(nullptr)
    , server_negociated_hostkey(SSH_KEYTYPE_UNKNOWN)

    , socket_callbacks_data_type(data_type_e::CALLBACK_RECEIVE_BANNER)
    , ctx(ctx)
    , showbuffer{}
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        this->agent = new ssh_agent_struct(this, new ssh_socket_struct());

        this->opts.StrictHostKeyChecking = 1;
        this->opts.port = 22;
        this->opts.fd = -1;
        this->opts.ssh2 = 1;
        this->opts.compressionlevel=7;
        this->opts.ssh1 = 0;
    }

    /** @internal
     * @brief Starts ecdh-sha2-nistp256 key exchange
     */

    /** @internal
     * @brief parses a SSH_MSG_KEX_ECDH_REPLY packet and sends back
     * a SSH_MSG_NEWKEYS
     */

    /* return channel with corresponding local id, or nullptr if not found */
    ssh_channel ssh_channel_from_local(uint32_t id){
        for (unsigned i = 0 ; i < this->nbchannels ; i++){
            if (this->channels[i]->local_channel == id){
                return this->channels[i];
            }
        }
        ssh_set_error(this->error, SSH_FATAL,"Channel %d not found",id);
        return nullptr;
    }

    void add_event(Event * event)
    {
        this->waiting_list.insert(this->waiting_list.begin(), event);
    }

    /**
     * @internal
     *
     * @brief Verify the hmac of a packet
     *
     * @param  buffer       The buffer to verify the hmac from.
     * @param  mac          The mac to compare with the hmac.
     *
     * @return              0 if hmac and mac are equal, < 0 if not or an error
     *                      occurred.
     */
    int packet_hmac_verify(ssh_buffer_struct* buffer, unsigned char *mac) {
        unsigned char hmacbuf[SslSha1::DIGEST_LENGTH] = {0};

        uint32_t seq = htonl(this->recv_seq);

        SslHMAC_Sha1 hmac_sha1(this->current_crypto->decryptMAC, SHA_DIGEST_LENGTH);
        hmac_sha1.update(reinterpret_cast<uint8_t *>(&seq), sizeof(uint32_t));
        hmac_sha1.update(buffer->get_pos_ptr(), buffer->in_remain());
        hmac_sha1.final(hmacbuf);

//      hexa("received mac",mac,SHA_DIGEST_LENGTH);
//      hexa("Computed mac",hmacbuf,SHA_DIGEST_LENGTH);
//      hexa("seq",static_cast<unsigned char *>(&seq),sizeof(uint32_t));
        if (memcmp(mac, hmacbuf, sizeof(hmacbuf)) == 0) {
            return 0;
        }
        return -1;
    }

    int make_sessionid()
    {

        // compute secret hash
        ssh_buffer_struct buf;
        buf.out_length_prefixed_cstr(this->clientbanner);
        buf.out_length_prefixed_cstr(this->serverbanner);

        ssh_buffer_struct client_hash;
        client_hash.out_uint8(20);
        client_hash.out_blob(this->next_crypto->client_kex.cookie, 16);
        client_hash.out_sshstring(this->next_crypto->client_kex.methods[SSH_KEX]);
        client_hash.out_sshstring(this->next_crypto->client_kex.methods[SSH_HOSTKEYS]);
        client_hash.out_sshstring(this->next_crypto->client_kex.methods[SSH_CRYPT_C_S]);
        client_hash.out_sshstring(this->next_crypto->client_kex.methods[SSH_CRYPT_S_C]);
        client_hash.out_sshstring(this->next_crypto->client_kex.methods[SSH_MAC_C_S]);
        client_hash.out_sshstring(this->next_crypto->client_kex.methods[SSH_MAC_S_C]);
        client_hash.out_sshstring(this->next_crypto->client_kex.methods[SSH_COMP_C_S]);
        client_hash.out_sshstring(this->next_crypto->client_kex.methods[SSH_COMP_S_C]);
        client_hash.out_sshstring(this->next_crypto->client_kex.methods[SSH_LANG_C_S]);
        client_hash.out_sshstring(this->next_crypto->client_kex.methods[SSH_LANG_S_C]);
        client_hash.out_uint32_be(0);
        client_hash.out_uint8(0);

        buf.out_uint32_be(client_hash.in_remain());
        buf.out_blob(client_hash.get_pos_ptr(), client_hash.in_remain());

        ssh_buffer_struct server_hash;
        server_hash.out_uint8(20);
        server_hash.out_blob(this->next_crypto->server_kex.cookie, 16);
        server_hash.out_sshstring(this->next_crypto->server_kex.methods[SSH_KEX]);
        server_hash.out_sshstring(this->next_crypto->server_kex.methods[SSH_HOSTKEYS]);
        server_hash.out_sshstring(this->next_crypto->server_kex.methods[SSH_CRYPT_C_S]);
        server_hash.out_sshstring(this->next_crypto->server_kex.methods[SSH_CRYPT_S_C]);
        server_hash.out_sshstring(this->next_crypto->server_kex.methods[SSH_MAC_C_S]);
        server_hash.out_sshstring(this->next_crypto->server_kex.methods[SSH_MAC_S_C]);
        server_hash.out_sshstring(this->next_crypto->server_kex.methods[SSH_COMP_C_S]);
        server_hash.out_sshstring(this->next_crypto->server_kex.methods[SSH_COMP_S_C]);
        server_hash.out_sshstring(this->next_crypto->server_kex.methods[SSH_LANG_C_S]);
        server_hash.out_sshstring(this->next_crypto->server_kex.methods[SSH_LANG_S_C]);
        server_hash.out_uint32_be(0);
        server_hash.out_uint8(0);

        buf.out_uint32_be(server_hash.in_remain());
        buf.out_blob(server_hash.get_pos_ptr(), server_hash.in_remain());

        buf.out_uint32_be(this->next_crypto->server_pubkey.size());
        buf.out_blob(&this->next_crypto->server_pubkey[0], this->next_crypto->server_pubkey.size());

        switch(this->next_crypto->kex_type){
        case SSH_KEX_DH_GROUP1_SHA1:
        { // e, f, k are included in hash
            {
                unsigned int len1 = BN_num_bytes(this->next_crypto->e);
                unsigned int bits1 = BN_num_bits(this->next_crypto->e);
                /* If the first bit is set we have a negative number, padding needed */
                int pad1 = ((bits1 % 8) == 0 && BN_is_bit_set(this->next_crypto->e, bits1 - 1))?1:0;
                std::vector<uint8_t> num1;
                num1.resize(len1 + pad1);
                /* We have a negative number henceforth we need a leading zero */
                num1[0] = 0;
                BN_bn2bin(this->next_crypto->e, &num1[pad1]);

                buf.out_uint32_be(num1.size());
                buf.out_blob(&num1[0], num1.size());
            }

            {
                unsigned int len2 = BN_num_bytes(this->next_crypto->f);
                unsigned int bits2 = BN_num_bits(this->next_crypto->f);
                /* If the first bit is set we have a negative number, padding needed */
                int pad2 = ((bits2 % 8) == 0 && BN_is_bit_set(this->next_crypto->f, bits2 - 1))?1:0;
                std::vector<uint8_t> num2;
                num2.resize(len2 + pad2);
                /* We have a negative number henceforth we need a leading zero */
                num2[0] = 0;
                BN_bn2bin(this->next_crypto->f, &num2[pad2]);

                buf.out_uint32_be(num2.size());
                buf.out_blob(&num2[0], num2.size());
            }

            {
                unsigned int len3 = BN_num_bytes(this->next_crypto->k);
                unsigned int bits3 = BN_num_bits(this->next_crypto->k);
                /* If the first bit is set we have a negative number, padding needed */
                int pad3 = ((bits3 % 8) == 0 && BN_is_bit_set(this->next_crypto->k, bits3 - 1))?1:0;
                std::vector<uint8_t> num3;
                num3.resize(len3 + pad3);
                /* We have a negative number henceforth we need a leading zero */
                num3[0] = 0;
                BN_bn2bin(this->next_crypto->k, &num3[pad3]);

                buf.out_uint32_be(num3.size());
                buf.out_blob(&num3[0], num3.size());
            }

//          hexa("hash buffer", buf.get_pos_ptr(), buf.in_remain());

            this->next_crypto->digest_len = SHA_DIGEST_LENGTH;
            this->next_crypto->mac_type = SSH_MAC_SHA1;
            // Check memory allocation
            SslSha1 sha1;
            sha1.update(buf.get_pos_ptr(), buf.in_remain());
            sha1.final(this->next_crypto->secret_hash);
        }
        break;
        case SSH_KEX_DH_GROUP14_SHA1:
        { // e, f, k are included in hash
            {
                unsigned int len1 = BN_num_bytes(this->next_crypto->e);
                unsigned int bits1 = BN_num_bits(this->next_crypto->e);
                /* If the first bit is set we have a negative number, padding needed */
                int pad1 = ((bits1 % 8) == 0 && BN_is_bit_set(this->next_crypto->e, bits1 - 1))?1:0;
                std::vector<uint8_t> num1;
                num1.resize(len1 + pad1);
                /* We have a negative number henceforth we need a leading zero */
                num1[0] = 0;
                BN_bn2bin(this->next_crypto->e, &num1[pad1]);

                buf.out_uint32_be(num1.size());
                buf.out_blob(&num1[0], num1.size());
            }

            {
                unsigned int len2 = BN_num_bytes(this->next_crypto->f);
                unsigned int bits2 = BN_num_bits(this->next_crypto->f);
                /* If the first bit is set we have a negative number, padding needed */
                int pad2 = ((bits2 % 8) == 0 && BN_is_bit_set(this->next_crypto->f, bits2 - 1))?1:0;
                std::vector<uint8_t> num2;
                num2.resize(len2 + pad2);
                /* We have a negative number henceforth we need a leading zero */
                num2[0] = 0;
                BN_bn2bin(this->next_crypto->f, &num2[pad2]);

                buf.out_uint32_be(num2.size());
                buf.out_blob(&num2[0], num2.size());
            }

            {
                unsigned int len3 = BN_num_bytes(this->next_crypto->k);
                unsigned int bits3 = BN_num_bits(this->next_crypto->k);
                /* If the first bit is set we have a negative number, padding needed */
                int pad3 = ((bits3 % 8) == 0 && BN_is_bit_set(this->next_crypto->k, bits3 - 1))?1:0;
                std::vector<uint8_t> num3;
                num3.resize(len3 + pad3);
                /* We have a negative number henceforth we need a leading zero */
                num3[0] = 0;
                BN_bn2bin(this->next_crypto->k, &num3[pad3]);

                buf.out_uint32_be(num3.size());
                buf.out_blob(&num3[0], num3.size());
            }

//          hexa("hash buffer", buf.get_pos_ptr(), buf.in_remain());

            this->next_crypto->digest_len = SHA_DIGEST_LENGTH;
            this->next_crypto->mac_type = SSH_MAC_SHA1;
            // Check memory allocation
            SslSha1 sha1;
            sha1.update(buf.get_pos_ptr(), buf.in_remain());
            sha1.final(this->next_crypto->secret_hash);
        }
        break;
        case SSH_KEX_ECDH_SHA2_NISTP256:
        { // ecdh.client_pubkey, k are included in hash
            buf.out_uint32_be(this->next_crypto->ecdh.client_pubkey.size());
            buf.out_blob(&this->next_crypto->ecdh.client_pubkey[0],
                          this->next_crypto->ecdh.client_pubkey.size());

            buf.out_uint32_be(this->next_crypto->ecdh.server_pubkey.size());
            buf.out_blob(&this->next_crypto->ecdh.server_pubkey[0],
                          this->next_crypto->ecdh.server_pubkey.size());

            {
                unsigned int len3 = BN_num_bytes(this->next_crypto->k);
                unsigned int bits3 = BN_num_bits(this->next_crypto->k);
                /* If the first bit is set we have a negative number, padding needed */
                int pad3 = ((bits3 % 8) == 0 && BN_is_bit_set(this->next_crypto->k, bits3 - 1))?1:0;
                std::vector<uint8_t> num3;
                num3.resize(len3 + pad3);
                /* We have a negative number henceforth we need a leading zero */
                num3[0] = 0;
                BN_bn2bin(this->next_crypto->k, &num3[pad3]);

                buf.out_uint32_be(num3.size());
                buf.out_blob(&num3[0], num3.size());
            }

//          hexa("hash buffer", buf.get_pos_ptr(), buf.in_remain());

            this->next_crypto->digest_len = SHA256_DIGEST_LENGTH;
            this->next_crypto->mac_type = SSH_MAC_SHA256;
            // check memory allocation
            SslSha256 sha256;
            sha256.update(buf.get_pos_ptr(), buf.in_remain());
            sha256.final(this->next_crypto->secret_hash);
        }
        break;
        case SSH_KEX_CURVE25519_SHA256_LIBSSH_ORG:
        { // curve_25519.client_pubkey, k included in hash
            buf.out_uint32_be(CURVE25519_PUBKEY_SIZE);
            buf.out_blob(this->next_crypto->curve_25519.client_pubkey, CURVE25519_PUBKEY_SIZE);

            buf.out_uint32_be(CURVE25519_PUBKEY_SIZE);
            buf.out_blob(this->next_crypto->curve_25519.server_pubkey, CURVE25519_PUBKEY_SIZE);

            {
                unsigned int len3 = BN_num_bytes(this->next_crypto->k);
                unsigned int bits3 = BN_num_bits(this->next_crypto->k);
                /* If the first bit is set we have a negative number, padding needed */
                int pad3 = ((bits3 % 8) == 0 && BN_is_bit_set(this->next_crypto->k, bits3 - 1))?1:0;
                std::vector<uint8_t> num3;
                num3.resize(len3 + pad3);
                /* We have a negative number henceforth we need a leading zero */
                num3[0] = 0;
                BN_bn2bin(this->next_crypto->k, &num3[pad3]);

                buf.out_uint32_be(num3.size());
                buf.out_blob(&num3[0], num3.size());
            }

//          hexa("hash buffer", buf.get_pos_ptr(), buf.in_remain());

            this->next_crypto->digest_len = SHA256_DIGEST_LENGTH;
            this->next_crypto->mac_type = SSH_MAC_SHA256;
            SslSha256 sha256;
            sha256.update(buf.get_pos_ptr(), buf.in_remain());
            sha256.final(this->next_crypto->secret_hash);
        }
        break;
        }

        /* During the first kex, secret hash and session ID are equal. However, after
         * a key re-exchange, a new secret hash is calculated. This hash will not replace
         * but complement existing session id.
         */
        if (!this->session_id){
            // TODO: optimize that session_id should keep it's length
            this->session_id = static_cast<uint8_t*>(malloc(this->next_crypto->digest_len));
            memcpy(this->session_id, this->next_crypto->secret_hash,  this->next_crypto->digest_len);
        }

//      hexa("secret hash", this->next_crypto->secret_hash, this->next_crypto->digest_len);
//      hexa("session id", this->session_id, this->next_crypto->digest_len);
        return SSH_OK;
    }



    void out_msg_ignore(const char * data)
    {
        this->out_buffer->out_uint8(SSH_MSG_IGNORE);
        this->out_buffer->out_length_prefixed_cstr(data);
    }


    void out_msg_channel_request(int remote_channel,
                                 const char * sig,
                                 uint8_t core,
                                 const char * errmsg,
                                 const char * lang)
    {
        this->out_buffer->out_uint8(SSH_MSG_CHANNEL_REQUEST);
        this->out_buffer->out_uint32_be(remote_channel);
        this->out_buffer->out_length_prefixed_cstr("signal");
        this->out_buffer->out_uint8(0);
        this->out_buffer->out_length_prefixed_cstr(sig);
        this->out_buffer->out_uint8(core);
        this->out_buffer->out_length_prefixed_cstr(errmsg);
        this->out_buffer->out_length_prefixed_cstr(lang);
    }


    int packet_send()
    {
        syslog(LOG_INFO, "--- %s seq=%d %s", __FUNCTION__, this->send_seq, this->show());
        unsigned int blocksize = (this->current_crypto?this->current_crypto->out_cipher->blocksize : 8);

        unsigned char *hmac = nullptr;

        if (this->current_crypto
        && this->current_crypto->do_compress_out
        && this->out_buffer->in_remain()) {
            this->compress_buffer(this->out_buffer);
        }

        uint8_t padding = (blocksize - ((this->out_buffer->in_remain() + 5) % blocksize));
        if(padding < 4) {
            padding += blocksize;
        }

        uint8_t padstring[32] = {0};
        if (this->current_crypto) {
            RAND_pseudo_bytes(padstring, padding);
        }

        uint32_t finallen = htonl(this->out_buffer->in_remain() + padding + 1);

        this->out_buffer->buffer_prepend_data(&padding, sizeof(uint8_t));
        this->out_buffer->buffer_prepend_data(&finallen, sizeof(uint32_t));
        this->out_buffer->out_blob(padstring, padding);

        if (this->current_crypto) {
            hmac = this->current_crypto->packet_encrypt(htonl(this->send_seq),
                    this->out_buffer->get_pos_ptr(),
                    this->out_buffer->in_remain(),
                    this->error);
            if (hmac) {
                this->out_buffer->out_blob(hmac, 20); // TODO Magic number
            }
        }

        this->socket->out_buffer->out_blob(this->out_buffer->get_pos_ptr(), this->out_buffer->in_remain());

        this->send_seq++;

        //    syslog(LOG_INFO,
        //        "packet: wrote [len=%d,padding=%hhd,comp=%d,payload=%d]",
        //        ntohl(finallen), padding, compsize, payloadsize);

        // TODO: that should not be necessary
        this->out_buffer->buffer_reinit();
        syslog(LOG_INFO, "--- %s DONE seq=%d %s", __FUNCTION__, this->send_seq-1, this->show());
        return SSH_OK;
    }


    int compress_buffer(ssh_buffer_struct* buf)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        ssh_buffer_struct* dest = this->gzip_compress(buf, this->opts.compressionlevel);
        buf->buffer_reinit();
        buf->out_blob(dest->get_pos_ptr(), dest->in_remain());
        delete dest;
        return 0;
    }

    ssh_buffer_struct* gzip_compress(ssh_buffer_struct* source,int level)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        z_stream *zout = static_cast<z_stream_s*>(this->current_crypto->compress_out_ctx);
        void *in_ptr = source->get_pos_ptr();
        unsigned long in_size = source->in_remain();
        ssh_buffer_struct* dest = nullptr;
        unsigned char out_buf[GZIP_BLOCKSIZE] = {0};
        unsigned long len;
        int status;

        if(zout == nullptr) {
            zout =  static_cast<z_stream_s*>(this->current_crypto->compress_out_ctx = this->initcompress(level));
            if (zout == nullptr) {
                return nullptr;
            }
        }

        dest = new ssh_buffer_struct;
        if (dest == nullptr) {
            return nullptr;
        }

        zout->next_out = out_buf;
        zout->next_in =  static_cast<uint8_t*>(in_ptr);
        zout->avail_in = in_size;
        do {
            zout->avail_out = GZIP_BLOCKSIZE;
            status = deflate(zout, Z_PARTIAL_FLUSH);
            if (status != Z_OK) {
                delete dest;
                ssh_set_error(this->error,  SSH_FATAL,
                  "status %d deflating zlib packet", status);
                return nullptr;
            }
            len = GZIP_BLOCKSIZE - zout->avail_out;
            dest->out_blob(out_buf, len);
            zout->next_out = out_buf;
        } while (zout->avail_out == 0);

        return dest;
    }


    z_stream *initcompress(int level) {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        z_stream *stream = static_cast<z_stream *>(malloc(sizeof(z_stream)));
        if (stream == nullptr) {
            return nullptr;
        }
        memset(stream, 0, sizeof(z_stream));

        int const status = deflateInit(stream, level);
        if (status != Z_OK) {
            free(stream);
            stream = nullptr;
            ssh_set_error(this->error,  SSH_FATAL,
                "status %d inititalising zlib deflate", status);
            return nullptr;
        }

        return stream;
    }

    virtual ~ssh_session_struct()
    {
        while (this->nbchannels > 0){
            this->nbchannels--;
            delete this->channels[this->nbchannels];
        }
        delete this->socket;
        this->socket = nullptr;

        delete this->in_buffer;
        delete this->out_buffer;
        this->in_buffer = this->out_buffer = nullptr;

        delete this->current_crypto;
        delete this->next_crypto;

        if (this->agent) {
            delete this->agent->sock;
            delete this->agent;
            this->agent = nullptr;
        }

        /* options */
        delete this->auth_auto_state;
        free(this->opts.bindaddr);
        free(this->opts.username);
        free(this->opts.host);
        free(this->opts.gss_server_identity);
        free(this->opts.gss_client_identity);
        free(this->opts.wanted_methods[SSH_KEX]);
        free(this->opts.wanted_methods[SSH_HOSTKEYS]);
        free(this->opts.wanted_methods[SSH_CRYPT_C_S]);
        free(this->opts.wanted_methods[SSH_CRYPT_S_C]);
        free(this->opts.wanted_methods[SSH_MAC_C_S]);
        free(this->opts.wanted_methods[SSH_MAC_S_C]);
        free(this->opts.wanted_methods[SSH_COMP_C_S]);
        free(this->opts.wanted_methods[SSH_COMP_S_C]);
        free(this->opts.wanted_methods[SSH_LANG_C_S]);
        free(this->opts.wanted_methods[SSH_LANG_S_C]);
    }

    uint32_t new_channel_id()
    {
        return ++(this->maxchannel);
    }

    virtual const char * session_type() { return "Generic"; }
    char * show()
    {
        sprintf(this->showbuffer, "Session<%s> PK(%s), STATE(%s)",
              this->session_type(),
              this->packet_state == PACKET_STATE_INIT ? "INIT"
            : this->packet_state == PACKET_STATE_SIZEREAD ? "SIZEREAD"
            : this->packet_state == PACKET_STATE_PROCESSING ? "PROCESSING"
            : "???",
              this->session_state == SSH_SESSION_STATE_NONE ? "NONE"
            : this->session_state == SSH_SESSION_STATE_CONNECTING ? "CONNECTING"
            : this->session_state == SSH_SESSION_STATE_SOCKET_CONNECTED ? "SOCKET_CONNECTED"
            : this->session_state == SSH_SESSION_STATE_BANNER_RECEIVED ? "BANNER_RECEIVED"
            : this->session_state == SSH_SESSION_STATE_INITIAL_KEX ? "STATE_INITIAL_KEX"
            : this->session_state == SSH_SESSION_STATE_KEXINIT_RECEIVED ? "KEXINIT_RECEIVED"
            : this->session_state == SSH_SESSION_STATE_DH ? "DH"
            : this->session_state == SSH_SESSION_STATE_AUTHENTICATING ? "AUTHENTICATING"
            : this->session_state == SSH_SESSION_STATE_AUTHENTICATED ? "AUTHENTICATED"
            : this->session_state == SSH_SESSION_STATE_ERROR ? "ERROR"
            : this->session_state == SSH_SESSION_STATE_DISCONNECTED ? "DISCONNECTED"
            : "???"
            );
        return this->showbuffer;
    }




};



static inline void ssh_analyze_banner(const char * banner, int & version, int & openssh_version)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    /*
     * Typical banners e.g. are:
     *
     * SSH-1.5-openSSH_5.4
     * SSH-1.99-openSSH_3.0
     *
     * SSH-2.0-something
     * 012345678901234567890
     */
    if (strlen(banner) < 6 ||
        strncmp(banner, "SSH-", 4) != 0) {
        return;
    }

    syslog(LOG_ERR, "Analyzing banner: %s", banner);

    version = (banner[4] == '2') ? 2 :
              (banner[4] == '1') ?
                    (((strlen(banner) > 6)
                  && (banner[5] == '.')
                  && (banner[6] == '9')) ? 2 : 1) :
              -1;

    const char *openssh = strstr(banner, "OpenSSH");
    if ((openssh != nullptr) && (strlen(openssh) > 9)){
        /*
        * The banner is typically:
        * OpenSSH_5.4
        * 012345678901234567890
        */
        char * endptr = nullptr;
        openssh_version = (strtol(openssh + 8, &endptr, 10) & 0xF) << 16;
        if (endptr && endptr[0] == '.'){
            openssh_version |= ((strtol(endptr + 1, static_cast<char **>(nullptr), 10) & 0xF) << 8);
        }
    }
}

enum {
   REQUEST_STRING_UNKNOWN = 0,
   REQUEST_STRING_EXIT_STATUS,
   REQUEST_STRING_SIGNAL,
   REQUEST_STRING_EXIT_SIGNAL,
   REQUEST_STRING_AUTH_AGENT_REQ_AT_OPENSSH_DOT_COM,
   REQUEST_STRING_PTY_REQ,
   REQUEST_STRING_WINDOW_CHANGE,
   REQUEST_STRING_SUBSYSTEM,
   REQUEST_STRING_SHELL,
   REQUEST_STRING_EXEC,
   REQUEST_STRING_ENV,
   REQUEST_STRING_X11_REQ
};

#define MACSIZE SHA_DIGEST_LENGTH


static inline uint8_t get_request_code(const char * request)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    if (strcmp(request, "exit-status") == 0) {
        return REQUEST_STRING_EXIT_STATUS;
    }
    if (strcmp(request, "signal") == 0) {
        return REQUEST_STRING_SIGNAL;
    }
    if (strcmp(request, "exit-signal") == 0) {
        return REQUEST_STRING_EXIT_SIGNAL;
    }
    if (strcmp(request, "auth-agent-req@openssh.com") == 0) {
        return REQUEST_STRING_AUTH_AGENT_REQ_AT_OPENSSH_DOT_COM;
    }
    if (strcmp(request, "pty-req") == 0) {
        return REQUEST_STRING_PTY_REQ;
    }
    if (strcmp(request, "window-change") == 0) {
        return REQUEST_STRING_WINDOW_CHANGE;
    }
    if (strcmp(request, "subsystem") == 0) {
        return REQUEST_STRING_SUBSYSTEM;
    }
    if (strcmp(request, "shell") == 0) {
        return REQUEST_STRING_SHELL;
    }
    if (strcmp(request, "exec") == 0) {
        return REQUEST_STRING_EXEC;
    }
    if (strcmp(request, "env") == 0) {
        return REQUEST_STRING_ENV;
    }
    if (strcmp(request, "x11-req") == 0) {
        return REQUEST_STRING_X11_REQ;
    }
    return REQUEST_STRING_UNKNOWN;
}


enum {
    REQUEST_STRING_CHANNEL_OPEN_SESSION,
    REQUEST_STRING_CHANNEL_OPEN_DIRECT_TCPIP,
    REQUEST_STRING_CHANNEL_OPEN_FORWARDED_TCPIP,
    REQUEST_STRING_CHANNEL_OPEN_X11,
    REQUEST_STRING_CHANNEL_OPEN_AUTH_AGENT,
    REQUEST_STRING_CHANNEL_OPEN_UNKNOWN,
};

static inline uint8_t get_channel_open_request_code(const char * request)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    if (strcmp(request, "session") == 0) {
        return REQUEST_STRING_CHANNEL_OPEN_SESSION;
    }
    if (strcmp(request, "direct-tcpip") == 0) {
        return REQUEST_STRING_CHANNEL_OPEN_DIRECT_TCPIP;
    }
    if (strcmp(request, "forwarded-tcpip") == 0) {
        return REQUEST_STRING_CHANNEL_OPEN_FORWARDED_TCPIP;
    }
    if (strcmp(request, "x11") == 0) {
        return REQUEST_STRING_CHANNEL_OPEN_X11;
    }
    if (strcmp(request, "auth-agent@openssh.com") == 0) {
        return REQUEST_STRING_CHANNEL_OPEN_AUTH_AGENT;
    }
    return REQUEST_STRING_CHANNEL_OPEN_UNKNOWN;
}



/** current state of an GSSAPI authentication */
enum ssh_gssapi_state_e {
    SSH_GSSAPI_STATE_NONE, /* no status */
    SSH_GSSAPI_STATE_RCV_TOKEN, /* Expecting a token */
    SSH_GSSAPI_STATE_RCV_MIC, /* Expecting a MIC */
};

struct ssh_gssapi_struct{
    enum ssh_gssapi_state_e state; /* current state */
    struct gss_OID_desc_struct mech; /* mechanism being elected for auth */
    gss_cred_id_t server_creds; /* credentials of server */
    gss_cred_id_t client_creds; /* creds delegated by the client */
    gss_ctx_id_t ctx; /* the authentication context */
    gss_name_t client_name; /* Identity of the client */
    char *user; /* username of client */
    char *canonic_user; /* canonic form of the client's username */
    char *service; /* name of the service */
    struct ssh_gssapi_struct_client {
        gss_name_t server_name; /* identity of server */
        OM_uint32 flags; /* flags used for init context */
        gss_OID oid; /* mech being used for authentication */
        gss_cred_id_t creds; /* creds used to initialize context */
        gss_cred_id_t client_deleg_creds; /* delegated creds (const, not freeable) */
        ssh_gssapi_struct_client() = default;
    } client;

    ssh_gssapi_struct()
        : state(SSH_GSSAPI_STATE_NONE)
        , mech{0,nullptr}
        , server_creds(GSS_C_NO_CREDENTIAL)
        , client_creds(GSS_C_NO_CREDENTIAL)
        , ctx(GSS_C_NO_CONTEXT)
        , client_name(nullptr)
        , user(nullptr)
        , canonic_user(nullptr)
        , service(nullptr)
    {
    }
};


static inline char *ssh_gssapi_name_to_char(gss_name_t name){
    syslog(LOG_INFO, "%s ---", __FUNCTION__);

    gss_buffer_desc buffer;
    OM_uint32 min_stat;
    OM_uint32 maj_stat = gss_display_name(&min_stat, name, &buffer, nullptr);

    gss_buffer_desc buffer2;
    OM_uint32 dummy;
    OM_uint32 message_context;
    gss_display_status(&dummy, maj_stat,GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer2);
    syslog(LOG_INFO, "GSSAPI(%s): %s", "converting name", static_cast<const char *>(buffer2.value));


    char * ptr = static_cast<char*>(malloc(buffer.length + 1));
    memcpy(ptr, buffer.value, buffer.length);
    ptr[buffer.length] = 0;
    gss_release_buffer(&min_stat, &buffer);
    return ptr;

}


static inline gss_OID ssh_gssapi_oid_from_string(const std::vector<uint8_t> & oid_s){
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    gss_OID ret = static_cast<gss_OID_desc*>(malloc(sizeof (gss_OID_desc)));
    if(oid_s.size() > 256
    || oid_s.size() <= 2
    || oid_s[0] != SSH_OID_TAG
    || oid_s[1] != (int)oid_s.size() - 2){
        free(ret);
        return nullptr;
    }
    ret->elements = malloc(oid_s.size() - 2);
    memcpy(ret->elements, &oid_s[2], oid_s.size()-2);
    ret->length = oid_s.size()-2;
    return ret;
}



enum {
    REQUEST_AUTH_UNKNOWN,
    REQUEST_AUTH_NONE,
    REQUEST_AUTH_PASSWORD,
    REQUEST_AUTH_KEYBOARD_INTERACTIVE,
    REQUEST_AUTH_PUBLICKEY,
    REQUEST_AUTH_GSSAPI_WITH_MIC
};

static inline uint8_t get_request_auth_code(const char * request)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    if (strcmp(request, "none") == 0) {
        return REQUEST_AUTH_NONE;
    }
    if (strcmp(request, "password") == 0) {
        return REQUEST_AUTH_PASSWORD;
    }
    if (strcmp(request, "keyboard-interactive") == 0) {
        return REQUEST_AUTH_KEYBOARD_INTERACTIVE;
    }
    if (strcmp(request, "publickey") == 0) {
        return REQUEST_AUTH_PUBLICKEY;
    }
    if (strcmp(request, "gssapi-with-mic") == 0) {
        return REQUEST_AUTH_GSSAPI_WITH_MIC;
    }
    return REQUEST_AUTH_UNKNOWN;
}



struct SshServerSession : public ssh_session_struct
{
    SshServerSession(
        ssh_poll_ctx_struct * ctx,
        ssh_server_callbacks server_callbacks,
        ssh_socket_struct * socket)
        : ssh_session_struct(ctx, socket)
        , server_callbacks(server_callbacks)
    {
    }

    virtual const char * session_type() override { return "Server"; }

    /* auths accepted by server */
    int auth_methods;
    ssh_server_callbacks server_callbacks;

    void do_delayed_sending(){
        if(this->socket->out_buffer->in_remain() == 0){
            size_t q = 0;
            for (q = 0 ; q < this->nbchannels; q++){
                if (this->channels[q]->server_outbuffer->in_remain() > 0){
                    syslog(LOG_INFO, "Data %u waiting to be sent on channel %u",
                        static_cast<unsigned>(this->channels[q]->server_outbuffer->in_remain()),
                        static_cast<unsigned>(q));
                    this->ssh_channel_write_server(this->channels[q], nullptr, 0);
                    break;
                }
                else if (this->channels[q]->local_eof_to_send){
                    syslog(LOG_INFO, "EOF to be sent on channel %u", static_cast<unsigned>(q));
                    this->socket->out_buffer->out_uint8(SSH_MSG_CHANNEL_EOF);
                    this->socket->out_buffer->out_uint32_be(this->channels[q]->remote_channel);
                    this->channels[q]->local_eof_to_send = false;
                }
                else if (this->channels[q]->local_close_to_send){
                    syslog(LOG_INFO, "CLOSE to be sent on channel %u", static_cast<unsigned>(q));
                    this->socket->out_buffer->out_uint8(SSH_MSG_CHANNEL_CLOSE);
                    this->socket->out_buffer->out_uint32_be(this->channels[q]->remote_channel);
                    this->channels[q]->local_close_to_send = false;
                    this->channels[q]->state =
                        ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_CLOSED;
                }
                break;
            }
        }
        else {
            syslog(LOG_INFO, "socket out buffer still has data to send");
        }
    }

    void do_front_event(int revents)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        int ret = 0;
        /* avoid having any event caught during callback */
        this->poll->lock = 1;

        switch (1){
        default:
        {
            error_struct & error = this->error;
            char buffer[4096];

            syslog(LOG_INFO, "%s polling FRONT checkin POLLERR", __FUNCTION__);

            if(revents & POLLERR || revents & POLLHUP){
                syslog(LOG_INFO, "%s polling FRONT POLLERR", __FUNCTION__);

                /* Check if we are in a connecting state */
                if(this->socket->state == SSH_SOCKET_CONNECTING){
                    syslog(LOG_ERR, "Front socket connection");

                    this->socket->state=SSH_SOCKET_ERROR;
                    int err=0;
                    socklen_t errlen=sizeof(err);
                    int r = getsockopt(this->socket->fd_in,
                                SOL_SOCKET, SO_ERROR, &err, &errlen);
                    if (r < 0) {
                        err = errno;
                    }
                    this->socket->last_errno = err;
                    this->socket->close();
                    syslog(LOG_ERR, "Socket connection callback error: (%d)", err);
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    ssh_set_error(error, SSH_FATAL,"%s",strerror(err));
                    this->socket->close();
                    ret = SSH_ERROR;
                    break;
                }

                /* Then we are in a more standard kind of error */
                /* force a read to get an explanation */
                revents |= POLLIN;
            }

            syslog(LOG_INFO, "%s polling FRONT checkin POLLIN lock=%d (%d == %d)"
                , __FUNCTION__
                , this->poll->lock
                , this->socket->state, SSH_SOCKET_CONNECTED);

            if((revents & POLLIN) && this->socket->state == SSH_SOCKET_CONNECTED){

                syslog(LOG_INFO, "%s polling FRONT POLLIN", __FUNCTION__);

                this->socket->read_wontblock=1;
                int r = read(this->socket->fd_in, buffer, sizeof(buffer));
                this->socket->last_errno = errno;
                this->socket->read_wontblock = 0;
                if (r < 0) {
                    this->socket->data_except = true;
                    int except = SSH_SOCKET_EXCEPTION_ERROR;
                    // Not listening anymore incoming data events
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    /** executed when an error/exception (timeout, ...) occurs on socket. */
                    syslog(LOG_ERR,"Socket exception: %d (%d)", except, this->socket->last_errno);
                    ssh_set_error(error, SSH_FATAL, "Socket error: %s", strerror(this->socket->last_errno));
                    this->socket->close();
                    return;
                }

                if (r == 0) {
                    this->socket->data_except = true;
                    int except = SSH_SOCKET_EXCEPTION_EOF;
                    this->socket->last_errno = 0;
                    // Not listening anymore incoming data events
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    /** executed when an error/exception (timeout, ...) occurs on socket. */
                    syslog(LOG_ERR,"Socket exception: %d EOF", except);
                    ssh_set_error(error, SSH_FATAL, "Socket error: disconnected");
                    this->socket->close();
                    return;
                }

                /* Bufferize the data and then call the callback */
                this->socket->in_buffer->out_blob(buffer, r);

                switch (this->socket_callbacks_data_type){
                default:
                case ssh_session_struct::data_type_e::HANDLE_RECEIVED_DATA:
                    while ((r = this->handle_received_data_server(
                                    this->socket->in_buffer->get_pos_ptr(),
                                    this->socket->in_buffer->in_remain())) > 0) {
                        this->socket->in_buffer->in_skip_bytes(r);
                        if (this->session_state == SSH_SESSION_STATE_ERROR) {
                            break;
                        }
                    }
                break;
                case ssh_session_struct::data_type_e::CALLBACK_RECEIVE_BANNER:
                    while ((r = this->callback_receive_banner_server(
                                this->socket->in_buffer->get_pos_ptr(),
                                this->socket->in_buffer->in_remain(),
                                error)) >= 0) {
                        this->socket->in_buffer->in_skip_bytes(r);
                        if (r > 0){
                            break;
                        }
                    }
                break;
                }
            }

            syslog(LOG_INFO, "%s polling SERVER checkin POLLOUT", __FUNCTION__);


            if(revents & POLLOUT){

                syslog(LOG_INFO, "%s polling SERVER POLLOUT state=%d (%d)", __FUNCTION__,
                    this->socket->state,
                    SSH_SOCKET_CONNECTING);

                /* First, POLLOUT is a sign we may be connected */
                if(this->socket->state == SSH_SOCKET_CONNECTING){
                    syslog(LOG_INFO, "Received POLLOUT in connecting state");
                    syslog(LOG_INFO, "POLLOUT for write CONNECTED");
                    int r = fcntl(this->socket->fd_in, F_SETFL, 0);
                    if (r < 0) {
                        syslog(LOG_INFO, "%s -- fcntl error %s", __FUNCTION__, strerror(errno));
                        ret = SSH_ERROR;
                        break;
                    }
                    this->socket->write_wontblock = 1;
                    this->socket->state = SSH_SOCKET_CONNECTED;
                    this->session_state = SSH_SESSION_STATE_SOCKET_CONNECTED;
                }

                syslog(LOG_INFO, "POLLOUT for write remain=%d except=%d wontblock=%d",
                        static_cast<unsigned>(this->socket->out_buffer->in_remain()),
                        this->socket->data_except,
                        this->socket->write_wontblock
                        );

                /* So, we can write data */
                this->socket->write_wontblock = 1;

                /* If buffered data is pending, write it */
                if(this->socket->out_buffer->in_remain() > 0){
                    if (this->socket->fd_in == INVALID_SOCKET) {
                        ssh_set_error(error, SSH_FATAL,
                          "[A]Writing packet: error on socket %d (or connection closed): %s",
                          this->socket->fd_in, strerror(this->socket->last_errno));
                        return;
                    }

                    if (this->socket->write_wontblock) {
                        if (this->socket->data_except) {
                            this->socket->close();
                            ssh_set_error(error, SSH_FATAL,
                              "[B] Writing packet: error on socket (or connection closed): %s",
                              strerror(this->socket->last_errno));
                            return;
                        }
                        syslog(LOG_INFO, "Sending %d bytes",
                            static_cast<unsigned>(this->socket->out_buffer->in_remain()));
                        int w = write(this->socket->fd_out,
                                      this->socket->out_buffer->get_pos_ptr(),
                                      this->socket->out_buffer->in_remain());
                        this->socket->last_errno = errno;
                        this->socket->write_wontblock = 0;

                        if (w < 0) {
                            this->socket->data_except = 1;
                            this->socket->close();
                            ssh_set_error(error, SSH_FATAL,
                              "[C] Writing packet: error on socket (or connection closed): %s",
                              strerror(this->socket->last_errno));
                            return;
                        }
                        syslog(LOG_INFO, "sent %d bytes", w);
                        this->socket->out_buffer->in_skip_bytes(w);
                    }
                }
                /* TODO: Find a way to put back POLLOUT when buffering occurs */
            }
            ret = SSH_OK;
        }
        break;
        }
        // do not unlock if error
        this->poll->lock = (ret == SSH_ERROR)?1:0;
    }


    int ssh_channel_write_server(ssh_channel channel, const uint8_t *data0, uint32_t len)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        /*    syslog(LOG_INFO, "channel write len=%d", len);*/
        /*
         * Handle the max packet len from remote side, be nice
         * 10 bytes for the headers
         */
        size_t maxpacketlen = channel->remote_maxpacket - 10;

        if (len){
            channel->server_outbuffer->out_blob(data0, len);
        }
        // TODO: is there need to do the actual sending now ?
        uint32_t effectivelen = std::min<uint32_t>(maxpacketlen,
                                std::min<uint32_t>(channel->server_outbuffer->in_remain(),
                                         channel->remote_window));
        if (effectivelen > 0) {
            this->out_buffer->out_uint8(SSH_MSG_CHANNEL_DATA);
            this->out_buffer->out_uint32_be(channel->remote_channel);
            this->out_buffer->out_uint32_be(effectivelen);
            this->out_buffer->out_blob(channel->server_outbuffer->get_pos_ptr(), effectivelen);
            channel->server_outbuffer->in_skip_bytes(effectivelen);
            channel->remote_window -= effectivelen;
            syslog(LOG_INFO, "wrote 9 bytes header + %u bytes data", effectivelen);
            this->packet_send();
        }
        // TODO: We are always saying to caller we have sent everything
        // that is not good, we should notify it later when datas
        // are actually sent. Trouble is that is probably means having
        // a sent notification API and not merely returning length like now
        return len;
    }


    //5.3.  Closing a Channel

    //   When a party will no longer send more data to a channel, it SHOULD
    //   send SSH_MSG_CHANNEL_EOF.

    //      byte      SSH_MSG_CHANNEL_EOF
    //      uint32    recipient channel

    //   No explicit response is sent to this message.  However, the
    //   application may send EOF to whatever is at the other end of the
    //   channel.  Note that the channel remains open after this message, and
    //   more data may still be sent in the other direction.  This message
    //   does not consume window space and can be sent even if no window space
    //   is available.

    //   When either party wishes to terminate the channel, it sends
    //   SSH_MSG_CHANNEL_CLOSE.  Upon receiving this message, a party MUST
    //   send back an SSH_MSG_CHANNEL_CLOSE unless it has already sent this
    //   message for the channel.  The channel is considered closed for a
    //   party when it has both sent and received SSH_MSG_CHANNEL_CLOSE, and
    //   the party may then reuse the channel number.  A party MAY send
    //   SSH_MSG_CHANNEL_CLOSE without having sent or received
    //   SSH_MSG_CHANNEL_EOF.

    //      byte      SSH_MSG_CHANNEL_CLOSE
    //      uint32    recipient channel

    //   This message does not consume window space and can be sent even if no
    //   window space is available.

    //   It is RECOMMENDED that all data sent before this message be delivered
    //   to the actual destination, if possible.

    int ssh_channel_send_eof_server(ssh_channel channel){
        if (!channel->local_eof && !channel->local_closed){
            channel->local_eof_to_send = true;
        }
        channel->local_eof = true;
        return SSH_OK;
    }


//6.10  Returning Exit Status
// ==========================

//   When the command running at the other end terminates, the following
//   message can be sent to return the exit status of the command.
//   Returning the status is RECOMMENDED.  No acknowledgment is sent for
//   this message.  The channel needs to be closed with
//   SSH_MSG_CHANNEL_CLOSE after this message.

//   The client MAY ignore these messages.

//            byte      SSH_MSG_CHANNEL_REQUEST
//            uint32    recipient_channel
//            string    "exit-status"
//            boolean   FALSE
//            uint32    exit_status

//   The remote command may also terminate violently due to a signal.
//   Such a condition can be indicated by the following message.  A zero
//   'exit_status' usually means that the command terminated successfully.
//   o  byte      SSH_MSG_CHANNEL_REQUEST
//   o  uint32    recipient channel
//   o  string    "exit-signal"
//   o  boolean   FALSE
//   o  string    signal name without the "SIG" prefix.
//   o  boolean   core dumped
//   o  string    error message in ISO-10646 UTF-8 encoding
//   o  string    language tag as defined in [RFC3066]

//   The 'signal name' is one of the following (these are from [POSIX])

//            ABRT
//            ALRM
//            FPE
//            HUP
//            ILL
//            INT
//            KILL
//            PIPE
//            QUIT
//            SEGV
//            TERM
//            USR1
//            USR2

//   Additional 'signal name' values MAY be sent in the format
//   "sig-name@xyz", where "sig-name" and "xyz" may be anything a
//   particular implementor wants (except the "@" sign).  However, it is
//   suggested that if a 'configure' script is used, any non-standard
//   'signal name' values it finds be encoded as "SIG@xyz.config.guess",
//   where "SIG" is the 'signal name' without the "SIG" prefix, and "xyz"
//   be the host type, as determined by "config.guess".

//   The 'error message' contains an additional textual explanation of the
//   error message.  The message may consist of multiple lines.  The
//   client software MAY display this message to the user.  If this is
//   done, the client software should take the precautions discussed in
//   [SSH-ARCH].

//5.3  Closing a Channel
// =====================

//   When a party will no longer send more data to a channel, it SHOULD
//   send SSH_MSG_CHANNEL_EOF.

//            byte      SSH_MSG_CHANNEL_EOF
//            uint32    recipient_channel

//   No explicit response is sent to this message.  However, the
//   application may send EOF to whatever is at the other end of the
//   channel.  Note that the channel remains open after this message, and
//   more data may still be sent in the other direction.  This message
//   does not consume window space and can be sent even if no window space
//   is available.

//   When either party wishes to terminate the channel, it sends
//   SSH_MSG_CHANNEL_CLOSE.  Upon receiving this message, a party MUST
//   send back a SSH_MSG_CHANNEL_CLOSE unless it has already sent this
//   message for the channel.  The channel is considered closed for a
//   party when it has both sent and received SSH_MSG_CHANNEL_CLOSE, and
//   the party may then reuse the channel number.  A party MAY send
//   SSH_MSG_CHANNEL_CLOSE without having sent or received
//   SSH_MSG_CHANNEL_EOF.

//            byte      SSH_MSG_CHANNEL_CLOSE
//            uint32    recipient_channel

//   This message does not consume window space and can be sent even if no
//   window space is available.

//   It is recommended that any data sent before this message is delivered
//   to the actual destination, if possible.

    /**
     * @brief Close a channel.
     *
     * This sends an end of file and then closes the channel. You won't be able
     * to recover any data the server was going to send or was in buffers.
     *
     * @param[in]  channel  The channel to close.
     *
     * @return              SSH_OK on success, SSH_ERROR if an error occurred.
     *
     * @see ssh_channel_free()
     * @see ssh_channel_is_eof()
     */
    int ssh_channel_close_server(ssh_channel channel){
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        if (!channel->local_eof) {
            channel->local_eof = true;
            channel->local_eof_to_send = true;
        }

        channel->local_closed = true;
        channel->local_close_to_send = true;
        return SSH_OK;
    }


    //    [RFC4253] 11.2.  Ignored Data Message
    //    ======================================

    //      byte      SSH_MSG_IGNORE
    //      string    data

    //   All implementations MUST understand (and ignore) this message at any
    //   time (after receiving the identification string).  No implementation
    //   is required to send them.  This message can be used as an additional
    //   protection measure against advanced traffic analysis techniques.

    int ssh_send_ignore_server(const char *data)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        this->out_buffer->out_uint8(SSH_MSG_IGNORE);
        this->out_buffer->out_length_prefixed_cstr(data);
        this->packet_send();
        return SSH_OK;
    }


    int ssh_channel_is_open_server(ssh_channel_struct * channel) {
        syslog(LOG_INFO, "%s --- channel_state=%d", __FUNCTION__, static_cast<int>(channel->state));
        return (channel->state == ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN);
    }

    int global_request_server(const char *request, ssh_buffer_struct* buffer, int reply);

    /**
     * @brief Sends the "tcpip-forward" global request to ask the server to begin
     *        listening for inbound connections.
     *
     * @param[in]  address  The address to bind to on the server. Pass nullptr to bind
     *                      to all available addresses on all protocol families
     *                      supported by the server.
     *
     * @param[in]  port     The port to bind to on the server. Pass 0 to ask the
     *                      server to allocate the next available unprivileged port
     *                      number
     *
     * @param[in]  bound_port The pointer to get actual bound port. Pass nullptr to
     *                        ignore.
     *
     * @return              SSH_OK on success,
     *                      SSH_ERROR if an error occurred,
     *                      SSH_AGAIN if in nonblocking mode and call has
     *                      to be done again.
     **/
    int ssh_forward_listen_server(const char *address, int port, int *bound_port)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        // TODO: check how and when this function is called to ensure we are actually in NONE
        // state when calling it. See how to manage non blocking (in global request)
        if(this->global_req_state != SSH_CHANNEL_REQ_STATE_NONE){
            return SSH_ERROR;
        }

        ssh_buffer_struct* buffer = new ssh_buffer_struct;
        buffer->out_length_prefixed_cstr(address ? address : "");
        buffer->out_uint32_be(port);

        int rc = this->global_request_server("tcpip-forward", buffer, 1);

        /* TODO: FIXME no guarantee the last packet we received contains that info */
        if (rc == SSH_OK && port == 0 && bound_port) {
            *bound_port = this->in_buffer->in_uint32_be();
        }
        delete buffer;
        return rc;
    }

    // [RFC4253] 11.3.  Debug Message
    // ==============================

    //      byte      SSH_MSG_DEBUG
    //      boolean   always_display
    //      string    message in ISO-10646 UTF-8 encoding [RFC3629]
    //      string    language tag [RFC3066]

    //   All implementations MUST understand this message, but they are
    //   allowed to ignore it.  This message is used to transmit information
    //   that may help debugging.  If 'always_display' is TRUE, the message
    //   SHOULD be displayed.  Otherwise, it SHOULD NOT be displayed unless
    //   debugging information has been explicitly requested by the user.

    //   The 'message' doesn't need to contain a newline.  It is, however,
    //   allowed to consist of multiple lines separated by CRLF (Carriage
    //   Return - Line Feed) pairs.

    //   If the 'message' string is displayed, the terminal control character
    //   filtering discussed in [SSH-ARCH] should be used to avoid attacks by
    //   sending terminal control characters.

    public:
    int ssh_send_debug_server(const char *message, int always_display)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        this->out_buffer->out_uint8(SSH_MSG_DEBUG);
        this->out_buffer->out_uint8(always_display);
        this->out_buffer->out_length_prefixed_cstr(message);
        // Empty language tag
        this->out_buffer->out_uint32_be(0);
        this->packet_send();
        return SSH_OK;
    }


    /**
     * @brief Sends the "cancel-tcpip-forward" global request to ask the server to
     *        cancel the tcpip-forward request.
     *
     * @param[in]  address  The bound address on the server.
     *
     * @param[in]  port     The bound port on the server.
     *
     * @return              SSH_OK on success,
     *                      SSH_ERROR if an error occurred,
     *                      SSH_AGAIN if in nonblocking mode and call has
     *                      to be done again.
     */
    int ssh_forward_cancel_server(const char *address, int port)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        // TODO: check how and when this function is called to ensure we are actually in NONE
        // state when calling it. See how to manage non blocking (in global request)
        if(this->global_req_state != SSH_CHANNEL_REQ_STATE_NONE){
            return SSH_ERROR;
        }

        ssh_buffer_struct* buffer = new ssh_buffer_struct;
        buffer->out_length_prefixed_cstr(address ? address : "");
        buffer->out_uint32_be(port);

        int rc = this->global_request_server("cancel-tcpip-forward", buffer, 1);

        delete buffer;
        return rc;
    }

    int ssh_channel_is_closed_server(ssh_channel_struct * channel) {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        return (channel->state == ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_CLOSED);
    }

    /**
     * @brief Check if remote has sent an EOF.
     *
     * @param[in]  channel  The channel to check.
     *
     * @return              0 if there is no EOF, nonzero otherwise.
     */
    int ssh_channel_is_eof_server(ssh_channel channel) {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        if ((channel->stdout_buffer && channel->stdout_buffer->in_remain() > 0) ||
            (channel->stderr_buffer && channel->stderr_buffer->in_remain() > 0)) {
            return 0;
        }

        return (channel->remote_eof != 0);
    }

    int ssh_channel_open_reverse_forward_server(ssh_channel channel, const char *remotehost,
                                     int remoteport, const char *sourcehost, int localport);

    int ssh_channel_open_auth_agent_server(ssh_channel channel);


    /**
     * @brief Send an exit signal to remote process (as described in RFC 4254, section 6.10).
     *
     * Sends a signal 'sig' to the remote process.
     * Note, that remote system may not support signals concept.
     * In such a case this request will be silently ignored.
     * Only SSH-v2 is supported (I'm not sure about SSH-v1).
     *
     * @param[in]  channel  The channel to send signal.
     *
     * @param[in]  sig      The signal to send (without SIG prefix)
     *                      (e.g. "TERM" or "KILL").
     * @param[in]  core     A boolean to tell if a core was dumped
     * @param[in]  errmsg   A CRLF explanation text about the error condition
     * @param[in]  lang     The language used in the message (format: RFC 3066)
     *
     * @return              SSH_OK on success, SSH_ERROR if an error occurred
     *                      (including attempts to send signal via SSH-v1 session).
     */
    int ssh_channel_request_send_exit_signal_server(ssh_channel channel, const char *sig,
                                             int core, const char *errmsg, const char *lang)
     {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        syslog(LOG_INFO, ">>>>>>>>>>>> CHANNEL_REQUEST_SEND_EXIT %s", channel->show());

        if(channel == nullptr || sig == nullptr || errmsg == nullptr || lang == nullptr) {
            ssh_set_error(this->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
            return SSH_ERROR;
        }

        if (channel->request_state != SSH_CHANNEL_REQ_STATE_NONE){
            ssh_set_error(this->error, SSH_FATAL, "Invalid state in %s", __FUNCTION__);
            return SSH_ERROR;
        }

        this->out_msg_channel_request(channel->remote_channel, sig, core?1:0, errmsg, lang);
        return SSH_OK;
    }




    /**
     * @brief Send the exit status to the remote process
     *
     * Sends the exit status to the remote process (as described in RFC 4254,
     * section 6.10).
     * Only SSH-v2 is supported (I'm not sure about SSH-v1).
     *
     * @param[in]  channel  The channel to send exit status.
     *
     * @param[in]  exit_status  The exit status to send
     *
     * @return     SSH_OK on success, SSH_ERROR if an error occurred.
     *             (including attempts to send exit status via SSH-v1 session).
     */
    int ssh_channel_request_send_exit_status_server(ssh_channel channel, int exit_status) {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        syslog(LOG_INFO, ">>>>>>>>>>>> CHANNEL_REQUEST_SEND_EXIT_STATUS %s", channel->show());

        if(channel == nullptr) {
            return SSH_ERROR;
        }

        switch(channel->request_state){
        case SSH_CHANNEL_REQ_STATE_NONE:
        {
            uint8_t want_reply = 0;
            this->out_buffer->out_uint8(SSH_MSG_CHANNEL_REQUEST);
            this->out_buffer->out_uint32_be(channel->remote_channel);
            this->out_buffer->out_length_prefixed_cstr("exit-status");
            this->out_buffer->out_uint8(want_reply);

            this->out_buffer->out_uint32_be(exit_status);
            this->packet_send();
            if (!want_reply) {
                channel->request_state = SSH_CHANNEL_REQ_STATE_NONE;
                return SSH_OK;
            }
            else {
                channel->request_state = SSH_CHANNEL_REQ_STATE_PENDING;
                return channel->channel_request(this);
            }
        }
        break;
        case SSH_CHANNEL_REQ_STATE_PENDING:
        case SSH_CHANNEL_REQ_STATE_ACCEPTED:
        case SSH_CHANNEL_REQ_STATE_DENIED:
        case SSH_CHANNEL_REQ_STATE_ERROR:
        default:
            break;
        }
        return channel->channel_request(this);
    }


    int ssh_channel_write_stderr_server(ssh_channel channel, const uint8_t *data, uint32_t len);

    /**
     * @internal
     *
     * @brief Gets the banner from socket and saves it in session.
     * Updates the session state
     *
     * @param  buffer pointer to the beginning of header
     * @param  len size of the banner
     * @param  error structure to fill in case of error
     * @returns Number of bytes processed, or zero if the banner is not complete.
     */
     // TODO: intermittent segfault here, see what happen
     private:
    int callback_receive_banner_server(uint8_t *buffer, size_t len, error_struct & error) {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        size_t i = 0;
        for (i = 0; i < len; i++) {
            if (buffer[i] == '\n') {
                buffer[i]='\0';
                if ((i > 1) && (buffer[i-1] == '\r')){
                    buffer[i-1]= 0;
                }
                /* number of bytes read */
                memcpy(this->clientbanner, buffer, i);
                syslog(LOG_INFO,"Received banner: %s", buffer);
                this->server_callbacks->connect_status_server_cb(this->server_callbacks->userdata, 0.4f);
                syslog(LOG_ERR, "SSH client banner: %s", this->clientbanner);

                /* Here we analyze the different protocols the server allows. */
                int ssh_version = 0;
                ssh_analyze_banner(this->clientbanner, ssh_version, this->openssh);
                /* Here we decide which version of the protocol to use. */
                if (ssh_version != 2) {
                    ssh_set_error(error, SSH_FATAL, "SSH-2 is the only supported protocol");
                    this->socket->close();
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    syslog(LOG_INFO, "%s --- [A]", __FUNCTION__);
                    return -1;
                }
                this->version = 2;

                this->socket_callbacks_data_type = ssh_session_struct::data_type_e::HANDLE_RECEIVED_DATA;
                this->server_callbacks->connect_status_server_cb(this->server_callbacks->userdata, 0.5f);
                this->session_state = SSH_SESSION_STATE_INITIAL_KEX;

                this->out_buffer->out_uint8(SSH_MSG_KEXINIT);
                this->out_buffer->out_blob(this->next_crypto->server_kex.cookie, 16);

                this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_KEX]);
                this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_HOSTKEYS]);
                this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_CRYPT_C_S]);
                this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_CRYPT_S_C]);
                this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_MAC_C_S]);
                this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_MAC_S_C]);
                this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_COMP_C_S]);
                this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_COMP_S_C]);
                this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_LANG_C_S]);
                this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_LANG_S_C]);
                this->out_buffer->out_uint8(0);
                this->out_buffer->out_uint32_be(0);

                this->packet_send();
                return i+1;
            }
            if(i > 128) {
                /* Too big banner */
                this->session_state = SSH_SESSION_STATE_ERROR;
                ssh_set_error(error,  SSH_FATAL,
                    "Receiving banner: too large banner (%u) %s", static_cast<unsigned>(i), buffer);
                syslog(LOG_INFO, "%s --- [D]", __FUNCTION__);
                return 0;
            }
        }
        syslog(LOG_INFO, "%s --- [E]", __FUNCTION__);
        return -1;
    }

    private:
    int handle_received_data_server(const void *data, size_t receivedlen)
    {
        syslog(LOG_INFO, "%s ---", __PRETTY_FUNCTION__);

        syslog(LOG_INFO, "%s --- [A]", __PRETTY_FUNCTION__);

        unsigned int blocksize = (this->current_crypto ?
                                  this->current_crypto->in_cipher->blocksize : 8);

        syslog(LOG_INFO, "%s --- [B]", __PRETTY_FUNCTION__);

        unsigned current_macsize = this->current_crypto ? MACSIZE : 0;
        unsigned char mac[30] = {0};
        char buffer[16] = {0};
        const uint8_t *packet;
        unsigned to_be_read;
        int rc;
    //    uint32_t compsize;
    //    uint32_t payloadsize;
        unsigned processed = 0; /* number of byte processed from the callback */
        uint8_t padding = 0;

        syslog(LOG_INFO, "%s --- [C]", __PRETTY_FUNCTION__);

        if (this->session_state == SSH_SESSION_STATE_ERROR) {
            syslog(LOG_INFO, "%s session in error state: exiting ", __FUNCTION__);
            this->session_state = SSH_SESSION_STATE_ERROR;
            return 0;
        }

        syslog(LOG_INFO, "%s --- [D]", __PRETTY_FUNCTION__);

        switch(this->packet_state) {
            case PACKET_STATE_INIT:
                syslog(LOG_INFO, "%s --- [E]", __PRETTY_FUNCTION__);

                if (receivedlen < blocksize) {
                    /*
                     * We didn't received enough data to read
                     * we need at least one block size, give up
                     */
                    syslog(LOG_INFO, "%s Need more data (got %d, need %d)",
                         __FUNCTION__, static_cast<int>(receivedlen), static_cast<int>(blocksize) );
                    return 0;
                }
                this->in_packet_type = 0;
                this->in_packet_valid = 0;

                if (this->in_buffer) {
                    delete this->in_buffer;
                }
                this->in_buffer = new ssh_buffer_struct;
                memcpy(buffer, data, blocksize);
                processed += blocksize;

                if (this->current_crypto) {
                  if (packet_decrypt(*(this->current_crypto), buffer,
                      this->current_crypto->in_cipher->blocksize, this->error) < 0) {
                      this->in_packet_len = 0;
                     // TODO: check that suspicious. What will happen if decrypt fail ?
                  }
                }
                {
                    Parse p(reinterpret_cast<uint8_t *>(buffer));
                    this->in_packet_len = p.in_uint32_be();
                }

                // TODO: why not reading from buffer instead of copying to in_buffer ?
                // (actually copying to in_buffer could be done inside packet_decrypt)
                this->in_buffer->out_blob(buffer, blocksize);

                if (this->in_packet_len > MAX_PACKET_LEN) {
                    ssh_set_error(this->error,
                                  SSH_FATAL,
                                  "read_packet(): Packet len larger than allowed max (%.4x, %u > %u)",
                                  this->in_packet_len, this->in_packet_len, MAX_PACKET_LEN);
                    this->session_state= SSH_SESSION_STATE_ERROR;
                    return processed;
                }

                this->packet_state = PACKET_STATE_SIZEREAD;
                REDEMPTION_CXX_FALLTHROUGH;
            case PACKET_STATE_SIZEREAD:
                syslog(LOG_INFO, "%s --- [F]", __PRETTY_FUNCTION__);

                // some cases are looking wrong here, seems like not crypted/crypted cases
                // are mixed. It works by sheer luck...
                to_be_read = this->in_packet_len - blocksize + sizeof(uint32_t) + current_macsize;

                syslog(LOG_INFO, "%s --- [G]", __PRETTY_FUNCTION__);

                /* if to_be_read is zero, the whole packet was blocksize bytes. */
                // TODO: I wonder what would happen is to_be_read is negative in above expression
                // => will be seen as a large positive number and accepted as valid ?
                if (to_be_read != 0) {
                    if ((receivedlen < to_be_read + processed) || (to_be_read < current_macsize)
                    ) {

                        /* give up, not enough data in buffer */
                        syslog(LOG_INFO,"packet: partial packet (read len) [len=%d] receivedlen=%u - process=%u < to_be_read=%u",
                            this->in_packet_len, static_cast<unsigned int>(receivedlen), processed, to_be_read);
                        return processed;
                    }

                    packet = (reinterpret_cast<const uint8_t*>(data)) + processed;
                    this->in_buffer->out_blob(packet, to_be_read - current_macsize);
                    processed += to_be_read - current_macsize;
                }


                if (this->current_crypto) {
                    /*
                     * Decrypt the rest of the packet (blocksize bytes already
                     * have been decrypted)
                     */
                    uint32_t buffer_len = this->in_buffer->in_remain();

                    /* The following check avoids decrypting zero bytes */
                    if (buffer_len > blocksize) {
                        uint8_t *payload = this->in_buffer->get_pos_ptr() + blocksize;
                        uint32_t plen = buffer_len - blocksize;

                        rc = packet_decrypt(*this->current_crypto, payload, plen, this->error);
                        if (rc < 0) {
                            syslog(LOG_INFO, "%s Decrypt error", __FUNCTION__);
                            ssh_set_error(this->error,  SSH_FATAL, "Decrypt error");
                            this->session_state= SSH_SESSION_STATE_ERROR;
                            return processed;
                        }
                    }

                    /* copy the last part from the incoming buffer */
                    packet = (reinterpret_cast<const uint8_t*>(data)) + processed;
                    memcpy(mac, packet, MACSIZE);

                    rc = this->packet_hmac_verify(this->in_buffer, mac);
                    if (rc < 0) {
                        syslog(LOG_INFO, "%s HMAC error", __FUNCTION__);
                        ssh_set_error(this->error, SSH_FATAL, "HMAC error");
                        this->session_state= SSH_SESSION_STATE_ERROR;
                        return processed;
                    }
                    processed += current_macsize;
                }

                /* skip the size field which has been processed before */
                this->in_buffer->in_skip_bytes(sizeof(uint32_t));
                padding = this->in_buffer->in_uint8();
                // check padding

                if (padding > this->in_buffer->in_remain()) {
                    ssh_set_error(this->error,
                                  SSH_FATAL,
                                  "Invalid padding: %d (%d left)",
                                  padding,
                                  static_cast<uint32_t>(this->in_buffer->in_remain()));
                    this->session_state= SSH_SESSION_STATE_ERROR;
                    return processed;
                }

                // TODO: see this, why do we remove data at end of buffer instead of beginning ?
                // I would be more expecting a call to skip_bytes!
                if (this->in_buffer->used >= padding) {
                    this->in_buffer->used -= padding;
                }

                if (this->current_crypto
                    && this->current_crypto->do_compress_in
                    && this->in_buffer->in_remain() > 0) {
                    rc = this->decompress_buffer_server(this->in_buffer, MAX_PACKET_LEN);
                    if (rc < 0) {
                        this->session_state= SSH_SESSION_STATE_ERROR;
                        return processed;
                    }
                }
    //            payloadsize = this->in_buffer->in_remain();
                this->recv_seq++;

                /*
                 * We don't want to rewrite a new packet while still executing the
                 * packet callbacks
                 */
                this->in_packet_type = this->in_buffer->in_uint8();
                this->in_packet_valid = 1;
                this->in_packet_len = 0;

    //            syslog(LOG_INFO,
    //                    "====> packet: read type %hhd [len=%d, padding=%hhd, comp=%d, payload=%d]",
    //                    this->in_packet_type, this->in_packet_len, padding, compsize, payloadsize);
                {
                    switch (this->in_packet_type) {
                    case SSH_MSG_DISCONNECT:
                        syslog(LOG_INFO, "%s --- SSH_MSG_DISCONNECT", __FUNCTION__);
                        this->handle_ssh_packet_disconnect_server(this->in_buffer, this->error);
                    break;
                    case SSH_MSG_IGNORE:
                        syslog(LOG_INFO, "%s --- ssh2_msg_ignore", __FUNCTION__);
                    break;
                    case SSH_MSG_UNIMPLEMENTED:
                        syslog(LOG_INFO, "%s --- SSH_MSG_UNIMPLEMENTED", __FUNCTION__);
                        this->handle_ssh_packet_unimplemented_server(this->in_buffer, this->error);
                    break;
                    case SSH_MSG_DEBUG:
                        syslog(LOG_INFO, "%s --- ssh2_msg_debug", __FUNCTION__);
                    break;
                    case SSH_MSG_SERVICE_REQUEST:
                        syslog(LOG_INFO, "%s --- SSH_MSG_SERVICE_REQUEST", __FUNCTION__);
                        this->handle_ssh_packet_service_request_server(this->in_buffer, this->error);
                    break;
                    case SSH_MSG_SERVICE_ACCEPT:
                        syslog(LOG_INFO, "%s --- SSH_MSG_SERVICE_ACCEPT", __FUNCTION__);
                        this->auth_service_state = SSH_AUTH_SERVICE_ACCEPTED;
                    //    this->flags &= ~SSH_SESSION_FLAG_BLOCKING;
                    break;
                    case SSH_MSG_KEXINIT:
                        syslog(LOG_INFO, "%s --- SSH_MSG_KEXINIT", __FUNCTION__);
                        this->handle_ssh_packet_kexinit_server(this->in_buffer);
                    break;
                    case SSH_MSG_NEWKEYS:
                        syslog(LOG_INFO, "%s --- SSH_MSG_NEWKEYS", __FUNCTION__);
                        this->ssh_packet_newkeys_server(this->error);
                    break;

    //       First, the client sends the following:

    //         byte      SSH_MSG_KEXDH_INIT
    //         mpint     e

    // Implies This message can be received Server side only

                    case SSH_MSG_KEXDH_INIT:
                        syslog(LOG_INFO, "%s --- SSH_MSG_KEXDH_INIT", __FUNCTION__);
                        this->ssh_packet_kexdh_init_server(this->in_buffer);
                        syslog(LOG_INFO, "%s --- SSH_MSG_KEXDH_INIT DONE", __FUNCTION__);
                    break;

    //       The server responds with the following:

    //         byte      SSH_MSG_KEXDH_REPLY
    //         string    server public host key and certificates (K_S)
    //         mpint     f
    //         string    signature of H

    // Implies This message can be received Client side only

                    case SSH_MSG_KEXDH_REPLY:
                        syslog(LOG_INFO, "%s --- SSH_MSG_KEXDH_REPLY", __FUNCTION__);
                    break;
                    case SSH_MSG_USERAUTH_REQUEST:
                        syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_REQUEST", __FUNCTION__);
                        this->ssh_packet_userauth_request_server(this->in_buffer);
                    break;
                    case SSH_MSG_USERAUTH_FAILURE:
                        syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_FAILURE", __FUNCTION__);
                        this->ssh_packet_userauth_failure_server(this->in_packet_type, this->in_buffer);
                    break;
                    case SSH_MSG_USERAUTH_SUCCESS:
                        syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_SUCCESS", __FUNCTION__);
                        this->ssh_packet_userauth_success_server(this->in_packet_type, this->in_buffer);
                    break;
                    case SSH_MSG_USERAUTH_BANNER:
                        syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_BANNER", __FUNCTION__);
                        this->ssh_packet_userauth_banner_server(this->in_buffer);
                    break;
                    case SSH_MSG_USERAUTH_PK_OK:
                        syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_PK_OK", __FUNCTION__);
                        this->ssh_packet_userauth_pk_ok_server(this->in_packet_type, this->in_buffer);
                    break;
                    case SSH_MSG_USERAUTH_INFO_RESPONSE:
    //                case SSH_MSG_USERAUTH_GSSAPI_TOKEN:
                        // TODO: replace this with a better test where the accepted answer
                        // depends on the type of the previously sent request
                        if (this->gssapi != nullptr) {
                            syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_GSSAPI_TOKEN", __FUNCTION__);
                            this->ssh_packet_userauth_gssapi_token_server(this->in_buffer);
                        }
                        else {
                            syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_INFO_RESPONSE", __FUNCTION__);
                            this->ssh_packet_userauth_info_response_server(this->in_buffer);
                        }
                    break;
                    case SSH_MSG_USERAUTH_GSSAPI_MIC:
                        syslog(LOG_INFO, "%s --- SSH_MSG_USERAUTH_GSSAPI_MIC", __FUNCTION__);
                        this->ssh_packet_userauth_gssapi_mic_server(this->in_packet_type, this->in_buffer);
                    break;
                    case SSH_MSG_GLOBAL_REQUEST:
                        syslog(LOG_INFO, "%s --- SSH_MSG_GLOBAL_REQUEST", __FUNCTION__);
                        this->ssh_packet_global_request_server(this->in_buffer);
                    break;
                    case SSH_MSG_REQUEST_SUCCESS:
                        syslog(LOG_INFO, "%s --- SSH_MSG_REQUEST_SUCCESS", __FUNCTION__);
                        if(this->global_req_state != SSH_CHANNEL_REQ_STATE_PENDING){
                            syslog(LOG_ERR, "SSH_REQUEST_SUCCESS received in incorrect state %d",
                                this->global_req_state);
                        }
                        this->global_req_state=SSH_CHANNEL_REQ_STATE_ACCEPTED;
                    break;
                    case SSH_MSG_REQUEST_FAILURE:
                        syslog(LOG_INFO, "%s --- SSH_MSG_REQUEST_FAILURE", __FUNCTION__);
                        if(this->global_req_state != SSH_CHANNEL_REQ_STATE_PENDING){
                            syslog(LOG_ERR, "SSH_REQUEST_DENIED received in incorrect state %d",
                                this->global_req_state);
                        }
                        this->global_req_state = SSH_CHANNEL_REQ_STATE_DENIED;
                    break;
                    case SSH_MSG_CHANNEL_OPEN:
                        syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_OPEN", __FUNCTION__);
                        if (this->session_state != SSH_SESSION_STATE_AUTHENTICATED){
                            ssh_set_error(this->error,  SSH_FATAL,
                            "Invalid state when receiving channel open request (must be authenticated)");
                            this->session_state = SSH_SESSION_STATE_ERROR;
                            return processed;
                        }
                        this->handle_channel_open_server(this->in_buffer);
                    break;
                    case SSH_MSG_CHANNEL_OPEN_CONFIRMATION:
                        syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_OPEN_CONFIRMATION", __FUNCTION__);
                        this->ssh2_msg_channel_open_confirmation_server(this->in_buffer);
                    break;
                    case SSH_MSG_CHANNEL_OPEN_FAILURE:
                    syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_OPEN_FAILURE", __FUNCTION__);
                    {
                        uint32_t chan = this->in_buffer->in_uint32_be();
                        ssh_channel channel = this->ssh_channel_from_local(chan);
                        if (channel == nullptr) {
                            ssh_set_error(this->error,  SSH_FATAL,
                                          "Server specified invalid channel %lu",
                                          static_cast<long unsigned int>(ntohl(chan)));
                            syslog(LOG_ERR,"Invalid channel in packet");
                        }
                        else {
                            this->ssh_packet_channel_open_fail_server(channel, this->in_buffer);
                        }
                    }
                    break;
                    case SSH_MSG_CHANNEL_WINDOW_ADJUST:
                        syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_WINDOW_ADJUST", __FUNCTION__);
                        this->channel_rcv_change_window_server(this->in_buffer, this->error);
                    break;
                    case SSH_MSG_CHANNEL_DATA:
                        syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_DATA", __FUNCTION__);
                        this->channel_rcv_data_server(this->in_buffer, this->error);
                    break;
                    case SSH_MSG_CHANNEL_EXTENDED_DATA:
                        syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_EXTENDED_DATA", __FUNCTION__);
                        this->channel_rcv_data_stderr_server(this->in_buffer, this->error);
                    break;
                    case SSH_MSG_CHANNEL_EOF:
                        syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_EOF", __FUNCTION__);
                        this->channel_rcv_eof_server(this->in_buffer);
    //                    this->session_state = SSH_SESSION_STATE_ERROR;
                    break;
                    case SSH_MSG_CHANNEL_CLOSE:
                        syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_CLOSE", __FUNCTION__);
                        this->channel_rcv_close_server(this->in_buffer);
    //                    this->session_state = SSH_SESSION_STATE_ERROR; // breaks agent forwarding
                        syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_CLOSE DONE", __FUNCTION__);
                    break;
                    case SSH_MSG_CHANNEL_REQUEST:
    //                    syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_REQUEST", __FUNCTION__);
                        this->channel_rcv_request_server(this->in_buffer, this->error);
                    break;
                    case SSH_MSG_CHANNEL_SUCCESS:
                    {
                        syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_SUCCESS", __FUNCTION__);
                        uint32_t chan = this->in_buffer->in_uint32_be();
                        ssh_channel channel = this->ssh_channel_from_local(chan);
                        if(channel->request_state != SSH_CHANNEL_REQ_STATE_PENDING){
                            syslog(LOG_ERR, "SSH_MSG_CHANNEL_SUCCESS received in incorrect state %d",
                                    channel->request_state);
                        } else {
                            channel->request_state=SSH_CHANNEL_REQ_STATE_ACCEPTED;
                        }
                    }
                    break;
                    case SSH_MSG_CHANNEL_FAILURE:
                    {
                        syslog(LOG_INFO, "%s --- SSH_MSG_CHANNEL_FAILURE", __FUNCTION__);
                        uint32_t chan = this->in_buffer->in_uint32_be();
                        ssh_channel channel = this->ssh_channel_from_local(chan);
                        if(channel->request_state != SSH_CHANNEL_REQ_STATE_PENDING){
                            syslog(LOG_ERR, "SSH_MSG_CHANNEL_FAILURE received in incorrect state %d",
                                    channel->request_state);
                        } else {
                            channel->request_state=SSH_CHANNEL_REQ_STATE_DENIED;
                        }
                    }
                    break;
                    default:
                        this->ssh_send_unimplemented_server();
                    break;
                    }
                }
                syslog(LOG_INFO, "Packet used processed=%d receivedLen=%d",
                    static_cast<int>(processed), static_cast<int>(receivedlen));

                this->packet_state = PACKET_STATE_INIT;
                return processed;
        }
        syslog(LOG_INFO, "%s OTHER ", __FUNCTION__);

        ssh_set_error(this->error,
                      SSH_FATAL,
                      "Invalid state into packet_read2(): %d",
                      this->packet_state);
        syslog(LOG_INFO, "%s OTHER.1 ", __FUNCTION__);
        this->session_state = SSH_SESSION_STATE_ERROR;
        return 0;
    }


    z_stream *initdecompress_server()
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        int status;

        z_stream *stream = static_cast<z_stream_s*>(malloc(sizeof(z_stream)));
        if (stream == nullptr) {
            return nullptr;
        }
        memset(stream,0,sizeof(z_stream));

        status = inflateInit(stream);
        if (status != Z_OK) {
            free(stream);
            stream = nullptr;
            ssh_set_error(this->error,  SSH_FATAL,
                "Status = %d initiating inflate context!", status);
            return nullptr;
        }

        return stream;
    }

    ssh_buffer_struct* gzip_decompress_server(ssh_buffer_struct* source, size_t maxlen)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        z_stream *zin = static_cast<z_stream_s*>(this->current_crypto->compress_in_ctx);
        void *in_ptr = source->get_pos_ptr();
        unsigned long in_size = source->in_remain();
        unsigned char out_buf[GZIP_BLOCKSIZE] = {0};
        ssh_buffer_struct* dest = nullptr;
        unsigned long len;
        int status;

        if (zin == nullptr) {
            this->current_crypto->compress_in_ctx = this->initdecompress_server();
            zin = static_cast<z_stream_s*>(this->current_crypto->compress_in_ctx);
            if (zin == nullptr) {
                return nullptr;
            }
        }

        dest = new ssh_buffer_struct;
        if (dest == nullptr) {
            return nullptr;
        }

        zin->next_out = out_buf;
        zin->next_in = static_cast<uint8_t*>(in_ptr);
        zin->avail_in = in_size;

        do {
            zin->avail_out = GZIP_BLOCKSIZE;
            status = inflate(zin, Z_PARTIAL_FLUSH);
            if (status != Z_OK && status != Z_BUF_ERROR) {
                ssh_set_error(this->error,  SSH_FATAL,
                  "status %d inflating zlib packet", status);
                delete dest;
                return nullptr;
            }

            len = GZIP_BLOCKSIZE - zin->avail_out;
            dest->out_blob(out_buf,len);
            if (dest->in_remain() > maxlen){
                /* Size of packet exceeded, avoid a denial of service attack */
                delete dest;
                return nullptr;
            }
            zin->next_out = out_buf;
        } while (zin->avail_out == 0);

        return dest;
    }

    int decompress_buffer_server(ssh_buffer_struct* buf, size_t maxlen)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        ssh_buffer_struct* dest = this->gzip_decompress_server(buf, maxlen);
        if (dest == nullptr) {
            return -1;
        }

        if (buf->buffer_reinit() < 0) {
            delete dest;
            return -1;
        }

        buf->out_blob(dest->get_pos_ptr(), dest->in_remain());

        delete dest;
        return 0;
    }


    int ssh_packet_newkeys_server(error_struct & error)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        if (this->session_state != SSH_SESSION_STATE_DH
        && this->dh_handshake_state != DH_STATE_NEWKEYS_SENT){
            ssh_set_error(error,  SSH_FATAL,"ssh_packet_newkeys called in wrong state : %d:%d",
                this->session_state, this->dh_handshake_state);
            this->session_state=SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        this->dh_handshake_state = DH_STATE_FINISHED;
        this->ssh_connection_callback_server(error);

        return SSH_PACKET_USED;
    }

    void ssh_connection_callback_server(error_struct & error){
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        switch(this->session_state){
            case SSH_SESSION_STATE_NONE:
            case SSH_SESSION_STATE_CONNECTING:
            case SSH_SESSION_STATE_SOCKET_CONNECTED:
                break;
            case SSH_SESSION_STATE_INITIAL_KEX:
            /* TODO: This state should disappear in favor of get_key handle */
                break;
            case SSH_SESSION_STATE_KEXINIT_RECEIVED:
            {
                this->server_callbacks->connect_status_server_cb(this->server_callbacks->userdata, 0.6f);

                syslog(LOG_ERR, "SSH_SESSION_STATE_KEXINIT_RECEIVED: !!!!");

                if (this->next_crypto->server_kex.methods[0].size() == 0){

                    syslog(LOG_ERR, "Initializing server_kex !!!! [F]");

                    char hostkeys[64] = {0};
                    size_t len = 0;

                    const char * ssh_key_methods = "curve25519-sha256@libssh.org,ecdh-sha2-nistp256,diffie-hellman-group14-sha1,diffie-hellman-group1-sha1";
                    this->next_crypto->server_kex.methods[SSH_KEX] = SSHString(ssh_key_methods, strlen(ssh_key_methods));

                    memset(this->next_crypto->server_kex.cookie, 0, 16);
                    RAND_pseudo_bytes(this->next_crypto->server_kex.cookie, 16);

                    if (this->server_ecdsa_key) {
                        syslog(LOG_INFO,"Server ECDSA key loaded");
                        const char * key = this->server_ecdsa_key->type_c();
                        int keylen = strlen(key);
                        memcpy(hostkeys+len, key, keylen);
                        len += keylen;
                    }
                    if (len > 0){
                        hostkeys[len] = ',';
                        len++;
                    }
                    if (this->server_dsa_key) {
                        syslog(LOG_INFO,"Server DSA key loaded");
                        const char * key = this->server_dsa_key->type_c();
                        int keylen = strlen(key);
                        memcpy(hostkeys+len, key, keylen);
                        len += keylen;
                    }
                    if (len > 0){
                        hostkeys[len] = ',';
                        len++;
                    }
                    if (this->server_rsa_key) {
                        syslog(LOG_INFO,"Server RSA key loaded");
                        const char * key = this->server_rsa_key->type_c();
                        int keylen = strlen(key);
                        memcpy(hostkeys+len, key, keylen);
                        len += keylen;
                    }
                    hostkeys[len] = 0;

                    const char * supported_hostkeys = "ecdsa-sha2-nistp256,ssh-rsa,ssh-dss";

                    SSHString tmp = find_matching(supported_hostkeys, hostkeys, ',');
                    if (tmp.size() == 0){
                        ssh_set_error(error, SSH_REQUEST_DENIED,
                            "Setting method: no algorithm for method \"%s\" (%s)\n",
                            "server host key algo", hostkeys);
                        this->socket->close();
                        this->session_state = SSH_SESSION_STATE_ERROR;
                        return;
                    }

                    free(this->opts.wanted_methods[SSH_HOSTKEYS]);
                    this->opts.wanted_methods[SSH_HOSTKEYS] = strdup(hostkeys);

                    this->next_crypto->server_kex.methods[SSH_HOSTKEYS] = tmp;

                    const char * ssh_crypt_cs_methods = "aes256-ctr,aes192-ctr,aes128-ctr,aes256-cbc,aes192-cbc,aes128-cbc,blowfish-cbc,3des-cbc,des-cbc-ssh1";
                    this->next_crypto->server_kex.methods[SSH_CRYPT_C_S] = SSHString(ssh_crypt_cs_methods, strlen(ssh_crypt_cs_methods));
                    this->next_crypto->server_kex.methods[SSH_CRYPT_S_C] = "aes256-ctr,aes192-ctr,aes128-ctr,aes256-cbc,aes192-cbc,aes128-cbc,blowfish-cbc,3des-cbc,des-cbc-ssh1";
                    this->next_crypto->server_kex.methods[SSH_MAC_C_S] = "hmac-sha1";
                    this->next_crypto->server_kex.methods[SSH_MAC_S_C] = "hmac-sha1";
                    this->next_crypto->server_kex.methods[SSH_COMP_C_S] = "none,zlib,zlib@openssh.com";
                    this->next_crypto->server_kex.methods[SSH_COMP_S_C] = "none,zlib,zlib@openssh.com";
                    this->next_crypto->server_kex.methods[SSH_LANG_S_C] = "";
                    this->next_crypto->server_kex.methods[SSH_LANG_C_S] = "";

                    /* We are in a rekeying, so we need to send the server kex */
                    this->out_buffer->out_uint8(SSH_MSG_KEXINIT);
                    this->out_buffer->out_blob(this->next_crypto->server_kex.cookie, 16);

                    this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_KEX]);
                    this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_HOSTKEYS]);
                    this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_CRYPT_C_S]);
                    this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_CRYPT_S_C]);
                    this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_MAC_C_S]);
                    this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_MAC_S_C]);
                    this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_COMP_C_S]);
                    this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_COMP_S_C]);
                    this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_LANG_C_S]);
                    this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_LANG_S_C]);
                    this->out_buffer->out_uint8(0);
                    this->out_buffer->out_uint32_be(0);

                    this->packet_send();
                }
                else {
                    syslog(LOG_ERR, "Server Kex already initialized !!!! [F]");
                }

                this->next_crypto->kex_methods[SSH_KEX] = find_matching(
                    this->next_crypto->server_kex.methods[SSH_KEX].c_str(),
                    this->next_crypto->client_kex.methods[SSH_KEX].c_str(),
                    ',');

                if (this->next_crypto->kex_methods[SSH_KEX].size() == 0){
                    ssh_set_error(this->error, SSH_FATAL,
                        "kex error : no match for method %s: server [%s], client [%s]",
                        "kex algos",
                        this->next_crypto->server_kex.methods[SSH_KEX].c_str(),
                        this->next_crypto->client_kex.methods[SSH_KEX].c_str());
                    this->socket->close();
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return;
                }

                // TODO: create a general purpose method to find a value in a string keyed dictionnary
                // and use it there and whenever necessary
                if(strcmp(this->next_crypto->kex_methods[SSH_KEX].c_str(), "diffie-hellman-group1-sha1") == 0){
                  this->next_crypto->kex_type = SSH_KEX_DH_GROUP1_SHA1;
                }
                else if(strcmp(this->next_crypto->kex_methods[SSH_KEX].c_str(), "diffie-hellman-group14-sha1") == 0){
                  this->next_crypto->kex_type = SSH_KEX_DH_GROUP14_SHA1;
                }
                else if(strcmp(this->next_crypto->kex_methods[SSH_KEX].c_str(), "ecdh-sha2-nistp256") == 0){
                  this->next_crypto->kex_type = SSH_KEX_ECDH_SHA2_NISTP256;
                }
                else if(strcmp(this->next_crypto->kex_methods[SSH_KEX].c_str(), "curve25519-sha256@libssh.org") == 0){
                  this->next_crypto->kex_type = SSH_KEX_CURVE25519_SHA256_LIBSSH_ORG;
                }

                this->next_crypto->kex_methods[SSH_HOSTKEYS] = find_matching(
                    this->next_crypto->server_kex.methods[SSH_HOSTKEYS].c_str(),
                    this->next_crypto->client_kex.methods[SSH_HOSTKEYS].c_str(),
                    ',');

                syslog(LOG_ERR, "%s HOSTKEYS %s %s",
                    __FUNCTION__,
                    this->next_crypto->server_kex.methods[SSH_HOSTKEYS].c_str(),
                    this->next_crypto->client_kex.methods[SSH_HOSTKEYS].c_str());

                if (this->next_crypto->kex_methods[SSH_HOSTKEYS].size() == 0){
                    ssh_set_error(this->error, SSH_FATAL,
                        "kex error : no match for method %s: server [%s], client [%s]",
                        "server host key algo",
                        this->next_crypto->server_kex.methods[SSH_HOSTKEYS].c_str(),
                        this->next_crypto->client_kex.methods[SSH_HOSTKEYS].c_str());
                    this->socket->close();
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return;
                }

                this->next_crypto->kex_methods[SSH_CRYPT_C_S] = find_matching(
                        this->next_crypto->server_kex.methods[SSH_CRYPT_C_S].c_str(),
                        this->next_crypto->client_kex.methods[SSH_CRYPT_C_S].c_str(),
                        ',');

                if (this->next_crypto->kex_methods[SSH_CRYPT_C_S].size() == 0){
                    ssh_set_error(this->error, SSH_FATAL,
                        "kex error : no match for method %s: server [%s], client [%s]",
                        "encryption client->server",
                        this->next_crypto->server_kex.methods[SSH_CRYPT_C_S].c_str(),
                        this->next_crypto->client_kex.methods[SSH_CRYPT_C_S].c_str());
                    this->socket->close();
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return;
                }
                this->next_crypto->kex_methods[SSH_CRYPT_S_C] = find_matching(
                    this->next_crypto->server_kex.methods[SSH_CRYPT_S_C].c_str(),
                    this->next_crypto->client_kex.methods[SSH_CRYPT_S_C].c_str(),
                    ',');

                if (this->next_crypto->kex_methods[SSH_CRYPT_S_C].size() == 0){
                    ssh_set_error(this->error, SSH_FATAL,
                        "kex error : no match for method %s: server [%s], client [%s]",
                        "encryption server->client",
                        this->next_crypto->server_kex.methods[SSH_CRYPT_S_C].c_str(),
                        this->next_crypto->client_kex.methods[SSH_CRYPT_S_C].c_str());
                    this->socket->close();
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return;
                }

                this->next_crypto->kex_methods[SSH_MAC_C_S] = find_matching(
                    this->next_crypto->server_kex.methods[SSH_MAC_C_S].c_str(),
                    this->next_crypto->client_kex.methods[SSH_MAC_C_S].c_str(),
                    ',');

                if (this->next_crypto->kex_methods[SSH_MAC_C_S].size() == 0){
                    ssh_set_error(this->error, SSH_FATAL,
                        "kex error : no match for method %s: server [%s], client [%s]",
                        "mac algo client->server",
                        this->next_crypto->server_kex.methods[SSH_MAC_C_S].c_str(),
                        this->next_crypto->client_kex.methods[SSH_MAC_C_S].c_str());
                    this->socket->close();
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return;
                }

                this->next_crypto->kex_methods[SSH_MAC_S_C] = find_matching(
                    this->next_crypto->server_kex.methods[SSH_MAC_S_C].c_str(),
                    this->next_crypto->client_kex.methods[SSH_MAC_S_C].c_str(),
                    ',');

                if (this->next_crypto->kex_methods[SSH_MAC_S_C].size() == 0){
                    ssh_set_error(this->error, SSH_FATAL,
                        "kex error : no match for method %s: server [%s], client [%s]",
                        "mac algo server->client",
                        this->next_crypto->server_kex.methods[SSH_MAC_S_C].c_str(),
                        this->next_crypto->client_kex.methods[SSH_MAC_S_C].c_str());
                    this->socket->close();
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return;
                }
                this->next_crypto->kex_methods[SSH_COMP_C_S] = find_matching(
                    this->next_crypto->server_kex.methods[SSH_COMP_C_S].c_str(),
                    this->next_crypto->client_kex.methods[SSH_COMP_C_S].c_str(),
                    ',');

                if (this->next_crypto->kex_methods[SSH_COMP_C_S].size() == 0){
                    ssh_set_error(this->error,SSH_FATAL,
                        "kex error : no match for method %s: server [%s], client [%s]",
                        "compression algo client->server",
                        this->next_crypto->server_kex.methods[SSH_COMP_C_S].c_str(),
                        this->next_crypto->client_kex.methods[SSH_COMP_C_S].c_str());
                    this->socket->close();
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return;
                }
                this->next_crypto->kex_methods[SSH_COMP_S_C] = find_matching(
                    this->next_crypto->server_kex.methods[SSH_COMP_S_C].c_str(),
                    this->next_crypto->client_kex.methods[SSH_COMP_S_C].c_str(),
                    ',');

                if (this->next_crypto->kex_methods[SSH_COMP_S_C].size() == 0){
                    ssh_set_error(this->error,SSH_FATAL,
                        "kex error : no match for method %s: server [%s], client [%s]",
                        "compression algo server->client",
                        this->next_crypto->server_kex.methods[SSH_COMP_S_C].c_str(),
                        this->next_crypto->client_kex.methods[SSH_COMP_S_C].c_str());
                    this->socket->close();
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return;
                }
                this->next_crypto->kex_methods[SSH_LANG_C_S] = find_matching(
                    this->next_crypto->server_kex.methods[SSH_LANG_C_S].c_str(),
                    this->next_crypto->client_kex.methods[SSH_LANG_C_S].c_str(),
                    ',');

                this->next_crypto->kex_methods[SSH_LANG_S_C] = find_matching(
                    this->next_crypto->server_kex.methods[SSH_LANG_S_C].c_str(),
                    this->next_crypto->client_kex.methods[SSH_LANG_S_C].c_str(),
                    ',');

                this->next_crypto->out_cipher = cipher_new_by_name(this->next_crypto->kex_methods[SSH_CRYPT_S_C]);
                this->next_crypto->in_cipher = cipher_new_by_name(this->next_crypto->kex_methods[SSH_CRYPT_C_S]);

                const char * method_in = this->next_crypto->kex_methods[SSH_COMP_C_S].c_str();
                if(strcmp(method_in,"zlib") == 0){
                    syslog(LOG_INFO,"enabling C->S compression");
                    this->next_crypto->do_compress_in=1;
                }

                if(strcmp(method_in,"zlib@openssh.com") == 0){
                    syslog(LOG_INFO,"enabling C->S delayed compression");
                    if (this->flags & SSH_SESSION_FLAG_AUTHENTICATED) {
                        this->next_crypto->do_compress_in = 1;
                    } else {
                        this->next_crypto->delayed_compress_in = 1;
                    }
                }

                const char * method_out = this->next_crypto->kex_methods[SSH_COMP_S_C].c_str();
                if(strcmp(method_out,"zlib") == 0){
                    syslog(LOG_INFO, "enabling S->C compression\n");
                    this->next_crypto->do_compress_out=1;
                }
                if(strcmp(method_out,"zlib@openssh.com") == 0){
                    syslog(LOG_INFO,"enabling S->C delayed compression\n");
                    if (this->flags & SSH_SESSION_FLAG_AUTHENTICATED) {
                        this->next_crypto->do_compress_out = 1;
                    }
                    else {
                        this->next_crypto->delayed_compress_out = 1;
                    }
                }

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

            syslog(LOG_INFO,"%s Finding hostkey type in = %s", __FUNCTION__, this->next_crypto->kex_methods[SSH_HOSTKEYS].c_str());
                this->server_negociated_hostkey = SSH_KEYTYPE_UNKNOWN;
                for(auto &p:l){
                    if (strcmp(p.first, this->next_crypto->kex_methods[SSH_HOSTKEYS].c_str()) == 0){
                        syslog(LOG_INFO,"%s negociated hostkey = %s %d", __FUNCTION__, p.first, p.second);
                        this->server_negociated_hostkey = p.second;
                        break;
                    }
                }

                this->server_callbacks->connect_status_server_cb(this->server_callbacks->userdata, 0.8f);
                this->session_state = SSH_SESSION_STATE_DH;
                return;
            }
            break;
            case SSH_SESSION_STATE_DH:
            {
                if(this->dh_handshake_state == DH_STATE_FINISHED){
                    if (this->next_crypto->generate_session_keys_server(this->session_id, this->error) < 0) {
                        this->socket->close();
                        this->session_state = SSH_SESSION_STATE_ERROR;
                        return;
                    }

                    /*
                     * Once we got SSH_MSG_NEWKEYS we can switch next_crypto and
                     * current_crypto
                     */
                    if (this->current_crypto) {
                      delete this->current_crypto;
                    }

                    /* FIXME TODO later, include a function to change keys */
                    this->current_crypto = this->next_crypto;
                    this->next_crypto = new ssh_crypto_struct();
                    this->server_callbacks->connect_status_server_cb(this->server_callbacks->userdata, 1.0f);
                    this->connected = 1;
                    this->session_state = SSH_SESSION_STATE_AUTHENTICATING;
                    if (this->flags & SSH_SESSION_FLAG_AUTHENTICATED){
                        this->session_state = SSH_SESSION_STATE_AUTHENTICATED;
                    }
                }
            }
            break;
            case SSH_SESSION_STATE_AUTHENTICATING:
                break;
            case SSH_SESSION_STATE_BANNER_RECEIVED:
            REDEMPTION_CXX_FALLTHROUGH;
            case SSH_SESSION_STATE_AUTHENTICATED:
            REDEMPTION_CXX_FALLTHROUGH;
            case SSH_SESSION_STATE_DISCONNECTED:
            REDEMPTION_CXX_FALLTHROUGH;
            case SSH_SESSION_STATE_ERROR:
                this->socket->close();
                this->session_state = SSH_SESSION_STATE_ERROR;
                return;
            default:
                ssh_set_error(error, SSH_FATAL,"Invalid state %d",this->session_state);
        }
        return;
    }

    int ssh_packet_kexdh_init_server(ssh_buffer_struct* packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        if(this->dh_handshake_state != DH_STATE_INIT){
            syslog(LOG_ERR,"Invalid state for SSH_MSG_KEXDH_INIT");
            syslog(LOG_INFO, "%s --- DONE ERROR", __FUNCTION__);
            this->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }

        switch(this->next_crypto->kex_type){
            case SSH_KEX_DH_GROUP1_SHA1:
            {
                syslog(LOG_INFO, "%s --- SSH_KEX_DH_GROUP1_SHA1 ", __FUNCTION__);
                if (sizeof(uint32_t) > packet->in_remain()){
                    ssh_set_error(this->error,  SSH_FATAL, "Cannot import e number");
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return SSH_ERROR;
                }
                uint32_t e_len = packet->in_uint32_be();
                if (e_len > packet->in_remain()){
                    ssh_set_error(this->error,  SSH_FATAL, "Cannot import e number");
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return SSH_ERROR;
                }
                uint8_t * e = new uint8_t[e_len];
                packet->buffer_get_data(e, e_len);
                // TODO: is there error management for BN_bin2bn
                this->next_crypto->e = BN_bin2bn(e, e_len, nullptr);
                if (this->next_crypto->e == nullptr) {
                    ssh_set_error(this->error, SSH_FATAL, "Cannot import e number");
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    free(e);
                    return SSH_ERROR;
                }

                this->dh_handshake_state = DH_STATE_INIT_SENT;

                this->next_crypto->y = BN_new();
                BN_rand(this->next_crypto->y, 128, 0, -1);

                BN_CTX * ctx1 = BN_CTX_new();
                this->next_crypto->f = BN_new();
                BIGNUM * g = BN_new();

                /* G is defined as 2 by the ssh2 standards */
                BN_set_word(g, 2);
                BIGNUM * p_group1 = BN_new();
                unsigned char p_group1_value[] = {
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2,
                0x21, 0x68, 0xC2, 0x34, 0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
                0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74, 0x02, 0x0B, 0xBE, 0xA6,
                0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
                0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D,
                0xF2, 0x5F, 0x14, 0x37, 0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
                0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6, 0xF4, 0x4C, 0x42, 0xE9,
                0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
                0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11,
                0x7C, 0x4B, 0x1F, 0xE6, 0x49, 0x28, 0x66, 0x51, 0xEC, 0xE6, 0x53, 0x81,
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

                enum { P_GROUP1_LEN = 128 }; /* Size in bytes of the p number for group 1 */
                BN_bin2bn(p_group1_value, P_GROUP1_LEN, p_group1);
                BN_mod_exp(this->next_crypto->f, g, this->next_crypto->y, p_group1, ctx1);
                BN_clear_free(p_group1);

                BN_CTX_free(ctx1);

                BN_clear_free(g);

                ssh_key_struct *privkey = nullptr;

                syslog(LOG_INFO,"%s negociated hostkey is %d", __FUNCTION__, this->server_negociated_hostkey);

                switch(this->server_negociated_hostkey) {
                case SSH_KEYTYPE_DSS:
                {
                    syslog(LOG_INFO, "%s ------ SSH_KEYTYPE_DSS ", __FUNCTION__);
                    privkey = this->server_dsa_key;
                    /*
                     * p        = public prime number
                     * q        = public 160-bit subprime, q | p-1
                     * g        = public generator of subgroup
                     * pub_key  = public key y = g^x
                     * priv_key = private key x
                     */
                    ssh_buffer_struct buffer;
                    buffer.out_length_prefixed_cstr("ssh-dss");
                    buffer.out_bignum(this->server_dsa_key->dsa->p); // p
                    buffer.out_bignum(this->server_dsa_key->dsa->q); // q
                    buffer.out_bignum(this->server_dsa_key->dsa->g); // g
                    buffer.out_bignum(this->server_dsa_key->dsa->pub_key); // n

                    this->next_crypto->server_pubkey.resize(static_cast<uint32_t>(buffer.in_remain()));
                    memcpy(&this->next_crypto->server_pubkey[0],
                           buffer.get_pos_ptr(), this->next_crypto->server_pubkey.size());
                  }
                  break;
                  case SSH_KEYTYPE_RSA:
                  case SSH_KEYTYPE_RSA1:
                  {
                    syslog(LOG_INFO, "%s ------ SSH_KEYTYPE_RSA ", __FUNCTION__);
                    privkey = this->server_rsa_key;

                    /*
                     * n    = public modulus
                     * e    = public exponent
                     * d    = private exponent
                     * p    = secret prime factor
                     * q    = secret prime factor
                     * dmp1 = d mod (p-1)
                     * dmq1 = d mod (q-1)
                     * iqmp = q^-1 mod p
                     */
                    ssh_buffer_struct buffer;
                    buffer.out_length_prefixed_cstr("ssh-rsa");
                    buffer.out_bignum(this->server_rsa_key->rsa->e); // e
                    buffer.out_bignum(this->server_rsa_key->rsa->n); // n

                    this->next_crypto->server_pubkey.resize(static_cast<uint32_t>(buffer.in_remain()));
                    memcpy(&this->next_crypto->server_pubkey[0],
                        buffer.get_pos_ptr(), this->next_crypto->server_pubkey.size());
                  }
                  break;
                  case SSH_KEYTYPE_ECDSA:
                  {
                    syslog(LOG_INFO, "%s ------ SSH_KEYTYPE_ECDSA ", __FUNCTION__);
                    privkey = this->server_ecdsa_key;
                    ssh_key_struct *pubkey = new ssh_key_struct(privkey->type, SSH_KEY_FLAG_PUBLIC);
                    pubkey->ecdsa_nid = privkey->ecdsa_nid;
                    pubkey->ecdsa = EC_KEY_new_by_curve_name(privkey->ecdsa_nid);

                    if (pubkey->ecdsa == nullptr) {
                        ssh_key_free(pubkey);
                        return SSH_ERROR;
                    }

                    const EC_POINT *p0 = EC_KEY_get0_public_key(privkey->ecdsa);
                    if (p0 == nullptr) {
                        ssh_key_free(pubkey);
                        return SSH_ERROR;
                    }

                    int ok = EC_KEY_set_public_key(pubkey->ecdsa, p0);
                    if (!ok) {
                        ssh_key_free(pubkey);
                        return SSH_ERROR;
                    }

                    ssh_buffer_struct buffer;
                    buffer.out_length_prefixed_cstr(pubkey->type_c());

                    buffer.out_length_prefixed_cstr(
                        (pubkey->ecdsa_nid == NID_X9_62_prime256v1) ? "nistp256" :
                        (pubkey->ecdsa_nid == NID_secp384r1)        ? "nistp384" :
                        (pubkey->ecdsa_nid == NID_secp521r1)        ? "nistp521" :
                        "unknown");

                    const EC_GROUP *g = EC_KEY_get0_group(pubkey->ecdsa);
                    const EC_POINT *p1 = EC_KEY_get0_public_key(pubkey->ecdsa);

                    size_t len_ec = EC_POINT_point2oct(g, p1, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0, nullptr);
                    if (len_ec == 0) {
                        this->next_crypto->server_pubkey.resize(0);
                        ssh_set_error(this->error,  SSH_FATAL, "Could not create a session id");
                        return SSH_ERROR;
                    }

                    std::vector<uint8_t> e;
                    e.resize(static_cast<uint32_t>(len_ec));
                    if (e.size() != EC_POINT_point2oct(g, p1, POINT_CONVERSION_UNCOMPRESSED, &e[0], e.size(), nullptr)){
                        this->next_crypto->server_pubkey.resize(0);
                        ssh_set_error(this->error,  SSH_FATAL, "Could not create a session id");
                        return SSH_ERROR;
                    }

                    buffer.out_uint32_be(e.size());
                    buffer.out_blob(&e[0], e.size());
                    this->next_crypto->server_pubkey.resize(static_cast<uint32_t>(buffer.in_remain()));
                    memcpy(&this->next_crypto->server_pubkey[0],
                           buffer.get_pos_ptr(),
                           this->next_crypto->server_pubkey.size());
                    delete pubkey;
                  }
                  break;
                  case SSH_KEYTYPE_UNKNOWN:
                  syslog(LOG_INFO, "%s ------ SSH_KEYTYPE_UNKNOWN ", __FUNCTION__);

                    privkey = nullptr;
                    ssh_set_error(this->error,  SSH_FATAL,
                        "Private key type unknown");
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    free(e);
                    return SSH_ERROR;
                }

                /* the server and clients don't use the same numbers */
                {
                    BN_CTX* ctx = BN_CTX_new();
                    BIGNUM * p_group1 = BN_new();
                    unsigned char p_group1_value[] = {
                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2,
                    0x21, 0x68, 0xC2, 0x34, 0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
                    0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74, 0x02, 0x0B, 0xBE, 0xA6,
                    0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
                    0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D,
                    0xF2, 0x5F, 0x14, 0x37, 0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
                    0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6, 0xF4, 0x4C, 0x42, 0xE9,
                    0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
                    0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11,
                    0x7C, 0x4B, 0x1F, 0xE6, 0x49, 0x28, 0x66, 0x51, 0xEC, 0xE6, 0x53, 0x81,
                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                    enum { P_GROUP1_LEN = 128 }; /* Size in bytes of the p number for group 1 */
                    BN_bin2bn(p_group1_value, P_GROUP1_LEN, p_group1);
                    this->next_crypto->k = BN_new();
                    BN_mod_exp(this->next_crypto->k,
                               this->next_crypto->e,
                               this->next_crypto->y, p_group1, ctx);
                    BN_clear_free(p_group1);
                    BN_CTX_free(ctx);
                }

                int rc = this->make_sessionid();

                if (rc != SSH_OK) {
                    ssh_set_error(this->error,  SSH_FATAL, "Could not create a session id");
                    return -1;
                }

                struct ssh_crypto_struct *crypto = this->next_crypto
                                                 ? this->next_crypto
                                                 : this->current_crypto;

                unsigned char hash[SHA_DIGEST_LENGTH] = {0};
                SslSha1 sha1;
                sha1.update(crypto->secret_hash, crypto->digest_len);
                sha1.final(hash);

                std::vector<uint8_t> sig_blob = ssh_pki_export_signature_blob(privkey, hash, SHA_DIGEST_LENGTH);

                this->out_buffer->out_uint8(SSH_MSG_KEXDH_REPLY);

                this->out_buffer->out_uint32_be(this->next_crypto->server_pubkey.size());
                this->out_buffer->out_blob(&this->next_crypto->server_pubkey[0],
                                              this->next_crypto->server_pubkey.size());

                {
                    unsigned int len3 = BN_num_bytes(this->next_crypto->f);
                    unsigned int bits3 = BN_num_bits(this->next_crypto->f);
                    /* If the first bit is set we have a negative number, padding needed */
                    int pad3 = ((bits3 % 8) == 0 && BN_is_bit_set(this->next_crypto->f, bits3 - 1))?1:0;
                    std::vector<uint8_t> num3;
                    num3.resize(len3 + pad3);
                    /* We have a negative number henceforth we need a leading zero */
                    num3[0] = 0;
                    BN_bn2bin(this->next_crypto->f, &num3[pad3]);

                    this->out_buffer->out_uint32_be(num3.size());
                    this->out_buffer->out_blob(&num3[0], num3.size());

                }

                this->out_buffer->out_uint32_be(sig_blob.size());
                this->out_buffer->out_blob(&sig_blob[0], sig_blob.size());

                this->packet_send();

                this->out_buffer->out_uint8(SSH_MSG_NEWKEYS);

                this->packet_send();

                syslog(LOG_INFO, "SSH_MSG_NEWKEYS sent");
                this->dh_handshake_state = DH_STATE_NEWKEYS_SENT;
                free(e);
            }
            break;

            case SSH_KEX_DH_GROUP14_SHA1:
            {
                syslog(LOG_INFO, "%s --- SSH_KEX_DH_GROUP14_SHA1 ", __FUNCTION__);
                if (sizeof(uint32_t) > packet->in_remain()){
                    ssh_set_error(this->error,  SSH_FATAL, "Cannot import e number");
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return SSH_ERROR;
                }
                uint32_t e_len = packet->in_uint32_be();
                if (e_len > packet->in_remain()){
                    ssh_set_error(this->error,  SSH_FATAL, "Cannot import e number");
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return SSH_ERROR;
                }
                uint8_t * e = new uint8_t[e_len];
                packet->buffer_get_data(e, e_len);
                // TODO: is there error management for BN_bin2bn
                this->next_crypto->e = BN_bin2bn(e, e_len, nullptr);
                if (this->next_crypto->e == nullptr) {
                    ssh_set_error(this->error, SSH_FATAL, "Cannot import e number");
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    free(e);
                    return SSH_ERROR;
                }

                this->dh_handshake_state = DH_STATE_INIT_SENT;

                this->next_crypto->y = BN_new();
                BN_rand(this->next_crypto->y, 128, 0, -1);

                BN_CTX* ctx0 = BN_CTX_new();
                this->next_crypto->f = BN_new();
                BIGNUM * g = BN_new();
                /* G is defined as 2 by the ssh2 standards */
                BN_set_word(g, 2);
                BIGNUM * p_group14 = BN_new();
                unsigned char p_group14_value[] = {
                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2,
                    0x21, 0x68, 0xC2, 0x34, 0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
                    0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74, 0x02, 0x0B, 0xBE, 0xA6,
                    0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
                    0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D,
                    0xF2, 0x5F, 0x14, 0x37, 0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
                    0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6, 0xF4, 0x4C, 0x42, 0xE9,
                    0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
                    0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11,
                    0x7C, 0x4B, 0x1F, 0xE6, 0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
                    0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05, 0x98, 0xDA, 0x48, 0x36,
                    0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
                    0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96, 0x1C, 0x62, 0xF3, 0x56,
                    0x20, 0x85, 0x52, 0xBB, 0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
                    0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04, 0xF1, 0x74, 0x6C, 0x08,
                    0xCA, 0x18, 0x21, 0x7C, 0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
                    0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03, 0x9B, 0x27, 0x83, 0xA2,
                    0xEC, 0x07, 0xA2, 0x8F, 0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9,
                    0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18, 0x39, 0x95, 0x49, 0x7C,
                    0xEA, 0x95, 0x6A, 0xE5, 0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
                    0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAC, 0xAA, 0x68, 0xFF, 0xFF, 0xFF, 0xFF,
                    0xFF, 0xFF, 0xFF, 0xFF};

                enum { P_GROUP14_LEN = 256 }; /* Size in bytes of the p number for group 14 */

                BN_bin2bn(p_group14_value, P_GROUP14_LEN, p_group14);

                BN_mod_exp(this->next_crypto->f, g, this->next_crypto->y, p_group14, ctx0);
                BN_clear_free(p_group14);

                BN_CTX_free(ctx0);
                BN_clear_free(g);


                ssh_key_struct *privkey = nullptr;

                syslog(LOG_INFO,"%s negociated hostkey is %d", __FUNCTION__, this->server_negociated_hostkey);

                switch(this->server_negociated_hostkey) {
                  case SSH_KEYTYPE_DSS:
                  {
                    syslog(LOG_INFO, "%s ------ SSH_KEYTYPE_DSS", __FUNCTION__);

                    privkey = this->server_dsa_key;
                    /*
                     * p        = public prime number
                     * q        = public 160-bit subprime, q | p-1
                     * g        = public generator of subgroup
                     * pub_key  = public key y = g^x
                     * priv_key = private key x
                     */
                    ssh_buffer_struct buffer;
                    buffer.out_length_prefixed_cstr("ssh-dss");
                    buffer.out_bignum(this->server_dsa_key->dsa->p); // p
                    buffer.out_bignum(this->server_dsa_key->dsa->q); // q
                    buffer.out_bignum(this->server_dsa_key->dsa->g); // g
                    buffer.out_bignum(this->server_dsa_key->dsa->pub_key); // n
                    this->next_crypto->server_pubkey.resize(static_cast<uint32_t>(buffer.in_remain()));
                    memcpy(&this->next_crypto->server_pubkey[0], buffer.get_pos_ptr(), this->next_crypto->server_pubkey.size());
                  }
                  break;
                  case SSH_KEYTYPE_RSA:
                  case SSH_KEYTYPE_RSA1:
                  {
                    syslog(LOG_INFO, "%s ------ SSH_KEYTYPE_RSA1", __FUNCTION__);

                    privkey = this->server_rsa_key;
                    /*
                     * n    = public modulus
                     * e    = public exponent
                     * d    = private exponent
                     * p    = secret prime factor
                     * q    = secret prime factor
                     * dmp1 = d mod (p-1)
                     * dmq1 = d mod (q-1)
                     * iqmp = q^-1 mod p
                     */
                    ssh_buffer_struct buffer;
                    buffer.out_length_prefixed_cstr("ssh-rsa");
                    buffer.out_bignum(this->server_rsa_key->rsa->e); // e
                    buffer.out_bignum(this->server_rsa_key->rsa->n); // n
                    this->next_crypto->server_pubkey.resize(buffer.in_remain());
                    memcpy(&this->next_crypto->server_pubkey[0], buffer.get_pos_ptr(), this->next_crypto->server_pubkey.size());
                  }
                  break;
                  case SSH_KEYTYPE_ECDSA:
                  {
                    syslog(LOG_INFO, "%s ------ SSH_KEYTYPE_ECDSA", __FUNCTION__);

                    privkey = this->server_ecdsa_key;
                    syslog(LOG_INFO,"%s negociated hostkey is %d", __FUNCTION__, this->server_negociated_hostkey);

                    ssh_key_struct * pubkey = new ssh_key_struct(this->server_negociated_hostkey, SSH_KEY_FLAG_PUBLIC);
                    pubkey->type = this->server_ecdsa_key->type;
                    pubkey->flags = SSH_KEY_FLAG_PUBLIC;
                    /* privkey -> pubkey */

                    pubkey->ecdsa_nid = this->server_ecdsa_key->ecdsa_nid;

                    pubkey->ecdsa = EC_KEY_new_by_curve_name(this->server_ecdsa_key->ecdsa_nid);
                    if (pubkey->ecdsa == nullptr) {
                        ssh_key_free(pubkey);
                        return SSH_ERROR;
                    }

                    const EC_POINT *p0 = EC_KEY_get0_public_key(this->server_ecdsa_key->ecdsa);
                    if (p0 == nullptr) {
                        ssh_key_free(pubkey);
                        return SSH_ERROR;
                    }

                    int ok = EC_KEY_set_public_key(pubkey->ecdsa, p0);
                    if (!ok) {
                        ssh_key_free(pubkey);
                        return SSH_ERROR;
                    }

                    ssh_buffer_struct buffer;
                    buffer.out_length_prefixed_cstr(pubkey->type_c());

                    buffer.out_length_prefixed_cstr(
                        (pubkey->ecdsa_nid == NID_X9_62_prime256v1) ? "nistp256" :
                        (pubkey->ecdsa_nid == NID_secp384r1)        ? "nistp384" :
                        (pubkey->ecdsa_nid == NID_secp521r1)        ? "nistp521" :
                        "unknown");

                    const EC_GROUP *g = EC_KEY_get0_group(pubkey->ecdsa);
                    const EC_POINT *p1 = EC_KEY_get0_public_key(pubkey->ecdsa);

                    size_t len_ec = EC_POINT_point2oct(g, p1, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0, nullptr);
                    if (len_ec == 0) {
                        return SSH_ERROR;
                    }

                    std::vector<uint8_t> e;
                    e.resize(static_cast<uint32_t>(len_ec));
                    if (e.size() != EC_POINT_point2oct(g, p1, POINT_CONVERSION_UNCOMPRESSED, &e[0], e.size(), nullptr)){
                        return SSH_ERROR;
                    }

                    buffer.out_uint32_be(e.size());
                    buffer.out_blob(&e[0], e.size());
                    this->next_crypto->server_pubkey.resize(static_cast<uint32_t>(buffer.in_remain()));
                    memcpy(&this->next_crypto->server_pubkey[0],
                           buffer.get_pos_ptr(),
                           this->next_crypto->server_pubkey.size());
                    ssh_key_free(pubkey);
                  }
                  break;
                  case SSH_KEYTYPE_UNKNOWN:
                  syslog(LOG_INFO, "%s ------ SSH_KEYTYPE_UNKNOWN", __FUNCTION__);

                    ssh_set_error(this->error,  SSH_FATAL,
                        "Private key type unknown");
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    free(e);
                    return SSH_ERROR;
                }


                /* the server and clients don't use the same numbers */
                {
                    BN_CTX* ctx = BN_CTX_new();
                    BIGNUM * p_group14 = BN_new();
                    unsigned char p_group14_value[] = {
                        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC9, 0x0F, 0xDA, 0xA2,
                        0x21, 0x68, 0xC2, 0x34, 0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
                        0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74, 0x02, 0x0B, 0xBE, 0xA6,
                        0x3B, 0x13, 0x9B, 0x22, 0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
                        0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B, 0x30, 0x2B, 0x0A, 0x6D,
                        0xF2, 0x5F, 0x14, 0x37, 0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
                        0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6, 0xF4, 0x4C, 0x42, 0xE9,
                        0xA6, 0x37, 0xED, 0x6B, 0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
                        0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5, 0xAE, 0x9F, 0x24, 0x11,
                        0x7C, 0x4B, 0x1F, 0xE6, 0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
                        0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05, 0x98, 0xDA, 0x48, 0x36,
                        0x1C, 0x55, 0xD3, 0x9A, 0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
                        0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96, 0x1C, 0x62, 0xF3, 0x56,
                        0x20, 0x85, 0x52, 0xBB, 0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
                        0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04, 0xF1, 0x74, 0x6C, 0x08,
                        0xCA, 0x18, 0x21, 0x7C, 0x32, 0x90, 0x5E, 0x46, 0x2E, 0x36, 0xCE, 0x3B,
                        0xE3, 0x9E, 0x77, 0x2C, 0x18, 0x0E, 0x86, 0x03, 0x9B, 0x27, 0x83, 0xA2,
                        0xEC, 0x07, 0xA2, 0x8F, 0xB5, 0xC5, 0x5D, 0xF0, 0x6F, 0x4C, 0x52, 0xC9,
                        0xDE, 0x2B, 0xCB, 0xF6, 0x95, 0x58, 0x17, 0x18, 0x39, 0x95, 0x49, 0x7C,
                        0xEA, 0x95, 0x6A, 0xE5, 0x15, 0xD2, 0x26, 0x18, 0x98, 0xFA, 0x05, 0x10,
                        0x15, 0x72, 0x8E, 0x5A, 0x8A, 0xAC, 0xAA, 0x68, 0xFF, 0xFF, 0xFF, 0xFF,
                        0xFF, 0xFF, 0xFF, 0xFF};
                    enum { P_GROUP14_LEN = 256 }; /* Size in bytes of the p number for group 14 */
                    BN_bin2bn(p_group14_value, P_GROUP14_LEN, p_group14);
                    this->next_crypto->k = BN_new();
                    BN_mod_exp(this->next_crypto->k,
                               this->next_crypto->e,
                               this->next_crypto->y, p_group14, ctx);
                    BN_clear_free(p_group14);
                    BN_CTX_free(ctx);
                }

                int rc = this->make_sessionid();

                if (rc != SSH_OK) {
                    ssh_set_error(this->error,  SSH_FATAL, "Could not create a session id");
                    return -1;
                }

                /* add signature blob */
                struct ssh_crypto_struct *crypto = this->next_crypto
                                                 ? this->next_crypto
                                                 : this->current_crypto;

                unsigned char hash[SHA_DIGEST_LENGTH] = {0};
                SslSha1 sha1;
                sha1.update(crypto->secret_hash, crypto->digest_len);
                sha1.final(hash);

                std::vector<uint8_t> sig_blob = ssh_pki_export_signature_blob(privkey, hash, SHA_DIGEST_LENGTH);

                this->out_buffer->out_uint8(SSH_MSG_KEXDH_REPLY);

                this->out_buffer->out_uint32_be(this->next_crypto->server_pubkey.size());
                this->out_buffer->out_blob(&this->next_crypto->server_pubkey[0],
                                              this->next_crypto->server_pubkey.size());

                {
                    unsigned int len3 = BN_num_bytes(this->next_crypto->f);
                    unsigned int bits3 = BN_num_bits(this->next_crypto->f);
                    /* If the first bit is set we have a negative number, padding needed */
                    int pad3 = ((bits3 % 8) == 0 && BN_is_bit_set(this->next_crypto->f, bits3 - 1))?1:0;
                    std::vector<uint8_t> num3;
                    num3.resize(len3 + pad3);
                    /* We have a negative number henceforth we need a leading zero */
                    num3[0] = 0;
                    BN_bn2bin(this->next_crypto->f, &num3[pad3]);

                    this->out_buffer->out_uint32_be(num3.size());
                    this->out_buffer->out_blob(&num3[0], num3.size());
                }


                this->out_buffer->out_uint32_be(sig_blob.size());
                this->out_buffer->out_blob(&sig_blob[0], sig_blob.size());

                this->packet_send();

                this->out_buffer->out_uint8(SSH_MSG_NEWKEYS);

                this->packet_send();

                syslog(LOG_INFO, "SSH_MSG_NEWKEYS sent");
                this->dh_handshake_state = DH_STATE_NEWKEYS_SENT;
                free(e);
            }
            break;
            case SSH_KEX_ECDH_SHA2_NISTP256:
            {
                syslog(LOG_INFO, "%s --- SSH_KEX_ECDH_SHA2_NISTP256 ", __FUNCTION__);

                /* SSH host keys (rsa,dsa,ecdsa) */
                /* Extract the client pubkey from the init packet */
                if (sizeof(uint32_t) > packet->in_remain()) {
                    // ERRRRRRRRRRRRRRRRRRRRRRRRRR
                }
                uint32_t q_c_string_len = packet->in_uint32_be();
                if (q_c_string_len > packet->in_remain()) {
                    // ERRRRRRRRRRRRRRRRRRRRRRRRRR
                }
                std::vector<uint8_t> q_c_string;
                q_c_string.resize(q_c_string_len);
                packet->buffer_get_data(&q_c_string[0],q_c_string_len);

                this->next_crypto->ecdh.client_pubkey = std::move(q_c_string);
                /* Build server's keypair */

                BN_CTX* ctx = BN_CTX_new();
                EC_KEY *ecdh_key = EC_KEY_new_by_curve_name(NISTP256);
                // TODO: check memory allocation
                const EC_GROUP * group = EC_KEY_get0_group(ecdh_key);
                EC_KEY_generate_key(ecdh_key);
                const EC_POINT *ecdh_pubkey = EC_KEY_get0_public_key(ecdh_key);
                int len = EC_POINT_point2oct(group,
                                         ecdh_pubkey,
                                         POINT_CONVERSION_UNCOMPRESSED,
                                         nullptr,
                                         0,
                                         ctx);
                std::vector<uint8_t> q_s_string;
                q_s_string.resize(len);
                EC_POINT_point2oct(group,
                                   ecdh_pubkey,
                                   POINT_CONVERSION_UNCOMPRESSED,
                                   &q_s_string[0],
                                   len,
                                   ctx);
                BN_CTX_free(ctx);
                this->next_crypto->ecdh.privkey = ecdh_key;
                this->next_crypto->ecdh.server_pubkey = std::move(q_s_string);

                this->out_buffer->out_uint8(SSH_MSG_KEXDH_REPLY);

                /* build k and session_id */
                // This line is different between _client and _server
                this->next_crypto->ecdh.build_k(this->next_crypto->k, this->next_crypto->ecdh.client_pubkey);

                syslog(LOG_INFO,"%s negociated hostkey is %d", __FUNCTION__, this->server_negociated_hostkey);

                switch(this->server_negociated_hostkey) {
                case SSH_KEYTYPE_DSS:
                {
                    syslog(LOG_INFO, "%s --- SSH_KEYTYPE_DSS ", __FUNCTION__);
                    /*
                     * p        = public prime number
                     * q        = public 160-bit subprime, q | p-1
                     * g        = public generator of subgroup
                     * pub_key  = public key y = g^x
                     * priv_key = private key x
                     */

                    ssh_buffer_struct buffer;
                    buffer.out_length_prefixed_cstr("ssh-dss");
                    buffer.out_bignum(this->server_dsa_key->dsa->p); // p
                    buffer.out_bignum(this->server_dsa_key->dsa->q); // q
                    buffer.out_bignum(this->server_dsa_key->dsa->g); // g
                    buffer.out_bignum(this->server_dsa_key->dsa->pub_key); // n

                    this->next_crypto->server_pubkey.resize(static_cast<uint32_t>(buffer.in_remain()));
                    memcpy(&this->next_crypto->server_pubkey[0], buffer.get_pos_ptr(), this->next_crypto->server_pubkey.size());
                }
                break;
                case SSH_KEYTYPE_RSA:
                case SSH_KEYTYPE_RSA1:
                {
                    syslog(LOG_INFO, "%s --- SSH_KEYTYPE_RSA ", __FUNCTION__);
                    /*
                     * n    = public modulus
                     * e    = public exponent
                     * d    = private exponent
                     * p    = secret prime factor
                     * q    = secret prime factor
                     * dmp1 = d mod (p-1)
                     * dmq1 = d mod (q-1)
                     * iqmp = q^-1 mod p
                     */
                    ssh_buffer_struct buffer;
                    // TODO: see when to use ssh-rsa or ssh-rsa1
                    buffer.out_length_prefixed_cstr("ssh-rsa");
                    buffer.out_bignum(this->server_rsa_key->rsa->e); // e
                    buffer.out_bignum(this->server_rsa_key->rsa->n); // n
                    this->next_crypto->server_pubkey.resize(static_cast<uint32_t>(buffer.in_remain()));
                    memcpy(&this->next_crypto->server_pubkey[0], buffer.get_pos_ptr(), this->next_crypto->server_pubkey.size());
                }
                break;
                case SSH_KEYTYPE_ECDSA:
                {
                    syslog(LOG_INFO, "%s --- SSH_KEYTYPE_ECDSA ", __FUNCTION__);
                    ssh_key_struct *pubkey = new ssh_key_struct(SSH_KEYTYPE_ECDSA, SSH_KEY_FLAG_PUBLIC);
                    pubkey->ecdsa_nid = this->server_ecdsa_key->ecdsa_nid;

                    pubkey->ecdsa = EC_KEY_new_by_curve_name(this->server_ecdsa_key->ecdsa_nid);
                    if (pubkey->ecdsa == nullptr) {
                        ssh_key_free(pubkey);
                        return SSH_ERROR;
                    }

                    // TODO: check if it is the same as the other p below
                    const EC_POINT *p0 = EC_KEY_get0_public_key(this->server_ecdsa_key->ecdsa);
                    if (p0 == nullptr) {
                        ssh_key_free(pubkey);
                        return SSH_ERROR;
                    }

                    int ok = EC_KEY_set_public_key(pubkey->ecdsa, p0);
                    if (!ok) {
                        ssh_key_free(pubkey);
                        return SSH_ERROR;
                    }

                    syslog(LOG_INFO, "%s SSH_KEYTYPE_ECDSA", __FUNCTION__);
                    ssh_buffer_struct buffer;
                    buffer.out_length_prefixed_cstr(pubkey->type_c());

                    buffer.out_length_prefixed_cstr(
                        (pubkey->ecdsa_nid == NID_X9_62_prime256v1) ? "nistp256" :
                        (pubkey->ecdsa_nid == NID_secp384r1)        ? "nistp384" :
                        (pubkey->ecdsa_nid == NID_secp521r1)        ? "nistp521" :
                        "unknown");

                    const EC_GROUP *g = EC_KEY_get0_group(pubkey->ecdsa);
                    // TODO: check if it is the same as the other p above
                    const EC_POINT *p1 = EC_KEY_get0_public_key(pubkey->ecdsa);

                    size_t len_ec = EC_POINT_point2oct(g, p1, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0, nullptr);
                    if (len_ec == 0) {
                        return SSH_ERROR;
                    }

                    std::vector<uint8_t> e;
                    e.resize(static_cast<uint32_t>(len_ec));
                    if (e.size() != EC_POINT_point2oct(g, p1, POINT_CONVERSION_UNCOMPRESSED, &e[0], e.size(), nullptr)){
                        return SSH_ERROR;
                    }

                    buffer.out_uint32_be(e.size());
                    buffer.out_blob(&e[0], e.size());
                    this->next_crypto->server_pubkey.resize(static_cast<uint32_t>(buffer.in_remain()));
                    memcpy(&this->next_crypto->server_pubkey[0],
                           buffer.get_pos_ptr(),
                           this->next_crypto->server_pubkey.size());

                    ssh_key_free(pubkey);
                }
                break;
                case SSH_KEYTYPE_UNKNOWN:
                    ssh_set_error(this->error,  SSH_FATAL,
                        "Private key type unknown");
                    this->out_buffer->buffer_reinit();
                    return SSH_ERROR;
                }

                this->make_sessionid();

                // TODO: check memory allocation

                /* add host's public key */
                this->out_buffer->out_uint32_be(this->next_crypto->server_pubkey.size());
                this->out_buffer->out_blob(&this->next_crypto->server_pubkey[0],
                                           this->next_crypto->server_pubkey.size());
                /* add ecdh public key */
                this->out_buffer->out_uint32_be(this->next_crypto->ecdh.server_pubkey.size());

                this->out_buffer->out_blob(&this->next_crypto->ecdh.server_pubkey[0], this->next_crypto->ecdh.server_pubkey.size());

                /* add signature blob */
                // TODO: check memory allocation

                struct ssh_crypto_struct *crypto = this->next_crypto
                                                 ? this->next_crypto
                                                 : this->current_crypto;

                unsigned char hash[SHA_DIGEST_LENGTH] = {0};
                SslSha1 sha1;
                sha1.update(crypto->secret_hash, crypto->digest_len);
                sha1.final(hash);

                ssh_key_struct *privkey = nullptr;
                syslog(LOG_INFO,"%s negociated hostkey is %d", __FUNCTION__, this->server_negociated_hostkey);

                switch (this->server_negociated_hostkey) {
                case SSH_KEYTYPE_DSS:
                    privkey = this->server_dsa_key;
                    break;
                case SSH_KEYTYPE_RSA:
                case SSH_KEYTYPE_RSA1:
                    privkey = this->server_rsa_key;
                    break;
                case SSH_KEYTYPE_ECDSA:
                    privkey = this->server_ecdsa_key;
                    break;
                case SSH_KEYTYPE_UNKNOWN:
                    privkey = nullptr;
                break;
                }

                std::vector<uint8_t> sig_blob = ssh_pki_export_signature_blob(privkey, hash, SHA_DIGEST_LENGTH);

                this->out_buffer->out_uint32_be(sig_blob.size());
                this->out_buffer->out_blob(&sig_blob[0], sig_blob.size());

                syslog(LOG_INFO, "SSH_MSG_KEXDH_REPLY sent");
                this->packet_send();

                /* Send the MSG_NEWKEYS */
                this->out_buffer->out_uint8(SSH_MSG_NEWKEYS);
                this->dh_handshake_state = DH_STATE_NEWKEYS_SENT;
                this->packet_send();
                syslog(LOG_INFO, "SSH_MSG_NEWKEYS sent");


                syslog(LOG_INFO, "%s --- ZB", __FUNCTION__);
            }
            break;
            case SSH_KEX_CURVE25519_SHA256_LIBSSH_ORG:
            {
                syslog(LOG_INFO, "%s --- SSH_KEX_CURVE25519_SHA256_LIBSSH_ORG ", __FUNCTION__);

                /* ECDH keys */
                /* SSH host keys (rsa,dsa,ecdsa) */
                /* Extract the client pubkey from the init packet */
                if (sizeof(uint32_t) > packet->in_remain()) {
                    // ERRRRRRRRRRRRRRRRRRRRRRRRRR
                }
                uint32_t q_c_string_len = packet->in_uint32_be();
                if (q_c_string_len > packet->in_remain()) {
                    // ERRRRRRRRRRRRRRRRRRRRRRRRRR
                }
                std::vector<uint8_t> q_c_string;
                q_c_string.resize(q_c_string_len);
                packet->buffer_get_data(&q_c_string[0], q_c_string_len);

                if (q_c_string.size() != CURVE25519_PUBKEY_SIZE){
                    ssh_set_error(this->error, SSH_FATAL, "Incorrect size for server Curve25519 public key: %d",
                            static_cast<int>(q_c_string.size()));
                    this->out_buffer->buffer_reinit();
                    syslog(LOG_INFO, "%s --- error", __FUNCTION__);
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return SSH_ERROR;
                }

                memcpy(this->next_crypto->curve_25519.client_pubkey, &q_c_string[0], CURVE25519_PUBKEY_SIZE);

                this->next_crypto->curve_25519.init(this->next_crypto->curve_25519.server_pubkey);

                /* build k and session_id */
                this->next_crypto->curve_25519.build_k(this->next_crypto->k,
                                                       this->next_crypto->curve_25519.client_pubkey);

                /* privkey is not allocated */
                ssh_key_struct *privkey = nullptr;

                syslog(LOG_INFO,"%s negociated hostkey is %d", __FUNCTION__, this->server_negociated_hostkey);

                switch(this->server_negociated_hostkey) {
                case SSH_KEYTYPE_DSS:
                {
                    syslog(LOG_INFO, "%s --- SSH_KEYTYPE_DSS ", __FUNCTION__);
                    privkey = this->server_dsa_key;
                    /*
                     * p        = public prime number
                     * q        = public 160-bit subprime, q | p-1
                     * g        = public generator of subgroup
                     * pub_key  = public key y = g^x
                     * priv_key = private key x
                     */

                    ssh_buffer_struct buffer;
                    buffer.out_length_prefixed_cstr("ssh-dss");
                    buffer.out_bignum(this->server_dsa_key->dsa->p); // p
                    buffer.out_bignum(this->server_dsa_key->dsa->q); // q
                    buffer.out_bignum(this->server_dsa_key->dsa->g); // g
                    buffer.out_bignum(this->server_dsa_key->dsa->pub_key); // n
                    this->next_crypto->server_pubkey.resize(static_cast<uint32_t>(buffer.in_remain()));
                    memcpy(&this->next_crypto->server_pubkey[0],
                           buffer.get_pos_ptr(),
                           this->next_crypto->server_pubkey.size());
                  }
                  break;
                  case SSH_KEYTYPE_RSA:
                  case SSH_KEYTYPE_RSA1:
                  {
                    syslog(LOG_INFO, "%s --- SSH_KEYTYPE_RSA ", __FUNCTION__);
                    privkey = this->server_rsa_key;
                    ssh_key_struct *pubkey = new ssh_key_struct(this->server_rsa_key->type, SSH_KEY_FLAG_PUBLIC);
                    pubkey->rsa = RSA_new();
                    if (pubkey->rsa == nullptr) {
                        ssh_key_free(pubkey);
                        return SSH_ERROR;
                    }

                    /*
                     * n    = public modulus
                     * e    = public exponent
                     * d    = private exponent
                     * p    = secret prime factor
                     * q    = secret prime factor
                     * dmp1 = d mod (p-1)
                     * dmq1 = d mod (q-1)
                     * iqmp = q^-1 mod p
                     */
                    ssh_buffer_struct buffer;
                    buffer.out_length_prefixed_cstr("ssh-rsa");
                    buffer.out_bignum(this->server_rsa_key->rsa->e); // e
                    buffer.out_bignum(this->server_rsa_key->rsa->n); // n
                    this->next_crypto->server_pubkey.resize(static_cast<uint32_t>(buffer.in_remain()));
                    memcpy(&this->next_crypto->server_pubkey[0], buffer.get_pos_ptr(), this->next_crypto->server_pubkey.size());
                  }
                  break;
                  case SSH_KEYTYPE_ECDSA:
                  {
                    syslog(LOG_INFO, "%s --- SSH_KEYTYPE_ECDSA ", __FUNCTION__);
                    privkey = this->server_ecdsa_key;
                    ssh_key_struct *pubkey = new ssh_key_struct(privkey->type, SSH_KEY_FLAG_PUBLIC);
                    /* privkey -> pubkey */
                    pubkey->ecdsa_nid = privkey->ecdsa_nid;

                    pubkey->ecdsa = EC_KEY_new_by_curve_name(privkey->ecdsa_nid);
                    if (pubkey->ecdsa == nullptr) {
                        ssh_key_free(pubkey);
                        return SSH_ERROR;
                    }

                    const EC_POINT *p0 = EC_KEY_get0_public_key(privkey->ecdsa);
                    if (p0 == nullptr) {
                        ssh_key_free(pubkey);
                        return SSH_ERROR;
                    }

                    int ok = EC_KEY_set_public_key(pubkey->ecdsa, p0);
                    if (!ok) {
                        ssh_key_free(pubkey);
                        return SSH_ERROR;
                    }
                    syslog(LOG_INFO, "%s SSH_KEYTYPE_ECDSA", __FUNCTION__);
                    ssh_buffer_struct buffer;
                    buffer.out_length_prefixed_cstr(pubkey->type_c());

                    buffer.out_length_prefixed_cstr(
                        (pubkey->ecdsa_nid == NID_X9_62_prime256v1) ? "nistp256" :
                        (pubkey->ecdsa_nid == NID_secp384r1)        ? "nistp384" :
                        (pubkey->ecdsa_nid == NID_secp521r1)        ? "nistp521" :
                        "unknown");

                    const EC_GROUP *g = EC_KEY_get0_group(pubkey->ecdsa);
                    const EC_POINT *p1 = EC_KEY_get0_public_key(pubkey->ecdsa);

                    size_t len_ec = EC_POINT_point2oct(g, p1, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0, nullptr);
                    if (len_ec == 0) {
                        return SSH_ERROR;
                    }

                    std::vector<uint8_t> e;
                    e.resize(static_cast<uint32_t>(len_ec));
                    if (e.size() != EC_POINT_point2oct(g, p1, POINT_CONVERSION_UNCOMPRESSED, &e[0], e.size(), nullptr)){
                        return SSH_ERROR;
                    }

                    buffer.out_uint32_be(e.size());
                    buffer.out_blob(&e[0], e.size());
                    this->next_crypto->server_pubkey.resize(static_cast<uint32_t>(buffer.in_remain()));
                    memcpy(&this->next_crypto->server_pubkey[0],
                           buffer.get_pos_ptr(),
                           this->next_crypto->server_pubkey.size());

                    ssh_key_free(pubkey);
                  }
                  break;
                  case SSH_KEYTYPE_UNKNOWN:
                    privkey = nullptr;
                    this->out_buffer->buffer_reinit();
                    syslog(LOG_INFO, "%s --- error", __FUNCTION__);
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return SSH_ERROR;
                }


                this->out_buffer->out_uint8(SSH_MSG_KEX_ECDH_REPLY);

                // session line is different between _server and _client
                int rc = this->make_sessionid();

                if (rc != SSH_OK) {
                    ssh_set_error(this->error, SSH_FATAL, "Could not create a session id");
                    this->out_buffer->buffer_reinit();
                    syslog(LOG_INFO, "%s --- error", __FUNCTION__);
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return SSH_ERROR;
                }

                /* add host's public key */
                this->out_buffer->out_uint32_be(this->next_crypto->server_pubkey.size());
                this->out_buffer->out_blob(&this->next_crypto->server_pubkey[0],
                                           this->next_crypto->server_pubkey.size());

                /* add ecdh public key */
                this->out_buffer->out_uint32_be(CURVE25519_PUBKEY_SIZE);
                this->out_buffer->out_blob(this->next_crypto->curve_25519.server_pubkey, CURVE25519_PUBKEY_SIZE);

                /* add signature blob */
                struct ssh_crypto_struct *crypto = this->next_crypto
                                                 ? this->next_crypto
                                                 : this->current_crypto;

                unsigned char hash[SHA_DIGEST_LENGTH] = {0};
                SslSha1 sha1;
                sha1.update(crypto->secret_hash, crypto->digest_len);
                sha1.final(hash);

                std::vector<uint8_t> sig_blob = ssh_pki_export_signature_blob(privkey, hash, SHA_DIGEST_LENGTH);

                this->out_buffer->out_uint32_be(sig_blob.size());
                this->out_buffer->out_blob(&sig_blob[0], sig_blob.size());

                syslog(LOG_INFO, "SSH_MSG_KEX_ECDH_REPLY sent");
                this->packet_send();

                /* Send the MSG_NEWKEYS */
                this->out_buffer->out_uint8(SSH_MSG_NEWKEYS);

                this->dh_handshake_state = DH_STATE_NEWKEYS_SENT;
                this->packet_send();
                syslog(LOG_INFO, "SSH_MSG_NEWKEYS sent");
            }
            break;
          default:
            ssh_set_error(this->error,  SSH_FATAL,"Wrong kex type in ssh_packet_kexdh_init_server");
            syslog(LOG_INFO, "%s --- error", __FUNCTION__);
            this->session_state = SSH_SESSION_STATE_ERROR;
        }

        syslog(LOG_INFO, "%s --- DONE", __FUNCTION__);
        return SSH_PACKET_USED;
    }



    int ssh_packet_userauth_failure_server(uint8_t type, ssh_buffer_struct* packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        char *auth_methods = nullptr;
        uint8_t partial = 0;
        (void) type;

        if (sizeof(uint32_t) > packet->in_remain()) {
            ssh_set_error(this->error,  SSH_FATAL, "Invalid USERAUTH_FAILURE msg");
            return SSH_PACKET_USED;
        }
        uint32_t auth_len = packet->in_uint32_be();
        if (auth_len > packet->in_remain()) {
            ssh_set_error(this->error,  SSH_FATAL, "Invalid USERAUTH_FAILURE msg");
            return SSH_PACKET_USED;
        }
        std::vector<uint8_t> auth;
        auth.resize(auth_len);
        packet->buffer_get_data(&auth[0], auth_len);


        partial = packet->in_uint8();

        auth_methods = new char [auth.size() + 1];
        memcpy(auth_methods, &auth[0], auth.size());
        auth_methods[auth.size()] = 0;

        if (partial) {
            this->auth_state = SSH_AUTH_STATE_PARTIAL;
            syslog(LOG_INFO,
                "Partial success. Authentication that can continue: %s",
                auth_methods);
        }
        else {
            this->auth_state=SSH_AUTH_STATE_FAILED;
            syslog(LOG_INFO,
                "Access denied. Authentication that can continue: %s",
                auth_methods);
            ssh_set_error(this->error,  SSH_REQUEST_DENIED,
                    "Access denied. Authentication that can continue: %s",
                    auth_methods);

            this->auth_methods = 0;
        }
        if (strstr(auth_methods, "password") != nullptr) {
            this->auth_methods |= SSH_AUTH_METHOD_PASSWORD;
        }
        if (strstr(auth_methods, "keyboard-interactive") != nullptr) {
            this->auth_methods |= SSH_AUTH_METHOD_INTERACTIVE;
        }
        if (strstr(auth_methods, "publickey") != nullptr) {
            this->auth_methods |= SSH_AUTH_METHOD_PUBLICKEY;
        }
        if (strstr(auth_methods, "hostbased") != nullptr) {
            this->auth_methods |= SSH_AUTH_METHOD_HOSTBASED;
        }
        if (strstr(auth_methods, "gssapi-with-mic") != nullptr) {
              this->auth_methods |= SSH_AUTH_METHOD_GSSAPI_MIC;
        }

        free(auth_methods);
        auth_methods = nullptr;

        return SSH_PACKET_USED;
    }


    int ssh_packet_userauth_success_server(uint8_t type, ssh_buffer_struct* packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
      (void)packet;
      (void)type;

      syslog(LOG_DEBUG, "Authentication successful");
      syslog(LOG_INFO, "Received SSH_USERAUTH_SUCCESS");

      this->auth_state = SSH_AUTH_STATE_SUCCESS;
      this->session_state = SSH_SESSION_STATE_AUTHENTICATED;
      this->flags |= SSH_SESSION_FLAG_AUTHENTICATED;

      if(this->current_crypto && this->current_crypto->delayed_compress_out){
          syslog(LOG_DEBUG, "Enabling delayed compression OUT");
          this->current_crypto->do_compress_out=1;
      }
      if(this->current_crypto && this->current_crypto->delayed_compress_in){
          syslog(LOG_DEBUG, "Enabling delayed compression IN");
          this->current_crypto->do_compress_in=1;
      }

      return SSH_PACKET_USED;
    }


    int ssh_packet_userauth_banner_server(ssh_buffer_struct* packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        if (sizeof(uint32_t) > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        uint32_t banner_len = packet->in_uint32_be();
        if (banner_len > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        this->banner.resize(banner_len);
        packet->buffer_get_data(&this->banner[0], banner_len);
        return SSH_PACKET_USED;
    }



    int ssh_packet_userauth_request_server(ssh_buffer_struct* packet){
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        int rc = SSH_PACKET_USED;
        SSHString username = packet->in_strdup_cstr();
        SSHString service = packet->in_strdup_cstr();
        SSHString method = packet->in_strdup_cstr();
        uint8_t method_code = get_request_auth_code(&method[0]);
        switch (method_code)
        {
        case REQUEST_AUTH_NONE:
        {
            this->handle_userauth_request_none_server(&username[0]);
        }
        break;

        case REQUEST_AUTH_PASSWORD:
        {
            this->handle_userauth_request_password_server(&username[0], packet);
        }
        break;
        case REQUEST_AUTH_KEYBOARD_INTERACTIVE:
            this->handle_userauth_request_keyboard_interactive_server(&username[0], packet);
        break;
        case REQUEST_AUTH_PUBLICKEY:
            this->handle_userauth_request_publickey_server(&service[0], &username[0], packet);
        break;
        case REQUEST_AUTH_GSSAPI_WITH_MIC:
            this->handle_userauth_gssapi_with_mic_server(&username[0], packet);
        break;
        default:
            // TODO: we should probably reject authentication
            syslog(LOG_INFO, "%s --- Unknown authentication method %s", __FUNCTION__, method.c_str());
        break;
        }
        return rc;
    }


    //5.2.  The "none" Authentication Request

    //   A client may request a list of authentication 'method name' values
    //   that may continue by using the "none" authentication 'method name'.

    //   If no authentication is needed for the user, the server MUST return
    //   SSH_MSG_USERAUTH_SUCCESS.  Otherwise, the server MUST return
    //   SSH_MSG_USERAUTH_FAILURE and MAY return with it a list of methods
    //   that may continue in its 'authentications that can continue' value.

    //   This 'method name' MUST NOT be listed as supported by the server.

    // PREASSERTION: server_request (no need to check server callbacks)

    void handle_userauth_request_none_server(char * username)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        // SSH_REQUEST_AUTH SSH_AUTH_METHOD_NONE
        syslog(LOG_INFO, "Auth request for service %s, method %s for user '%s'",
            "ssh-connection", "none", username);

        int rc = this->server_callbacks->auth_none_server_cb(username,
                                                             this->server_callbacks->userdata);
        switch (rc) {
        case SSH_AUTH_PARTIAL:
            this->ssh_auth_reply_denied_server(true);
            break;
        case SSH_AUTH_SUCCESS:
            this->session_state = SSH_SESSION_STATE_AUTHENTICATED;
            this->flags |= SSH_SESSION_FLAG_AUTHENTICATED;
            this->out_buffer->out_uint8(SSH_MSG_USERAUTH_SUCCESS);
            this->packet_send();
            if (this->current_crypto){
                if(this->current_crypto->delayed_compress_out){
                    syslog(LOG_INFO,"Enabling delayed compression OUT");
                    this->current_crypto->do_compress_out=1;
                }
                if (this->current_crypto->delayed_compress_in){
                    syslog(LOG_INFO,"Enabling delayed compression IN");
                    this->current_crypto->do_compress_in=1;
                }
            }
            REDEMPTION_CXX_FALLTHROUGH;
        default:
            ssh_auth_reply_denied_server(this);
        }
    }


    //8.  Password Authentication Method: "password"

    //   Password authentication uses the following packets.  Note that a
    //   server MAY request that a user change the password.  All
    //   implementations SHOULD support password authentication.

    //      byte      SSH_MSG_USERAUTH_REQUEST
    //      string    user name
    //      string    service name
    //      string    "password"
    //      boolean   FALSE
    //      string    plaintext password in ISO-10646 UTF-8 encoding [RFC3629]

    //   Note that the 'plaintext password' value is encoded in ISO-10646
    //   UTF-8.  It is up to the server how to interpret the password and
    //   validate it against the password database.  However, if the client
    //   reads the password in some other encoding (e.g., ISO 8859-1 - ISO
    //   Latin1), it MUST convert the password to ISO-10646 UTF-8 before
    //   transmitting, and the server MUST convert the password to the
    //   encoding used on that system for passwords.

    //   From an internationalization standpoint, it is desired that if a user
    //   enters their password, the authentication process will work
    //   regardless of what OS and client software the user is using.  Doing
    //   so requires normalization.  Systems supporting non-ASCII passwords
    //   SHOULD always normalize passwords and user names whenever they are
    //   added to the database, or compared (with or without hashing) to
    //   existing entries in the database.  SSH implementations that both
    //   store the passwords and compare them SHOULD use [RFC4013] for
    //   normalization.

    //   Note that even though the cleartext password is transmitted in the
    //   packet, the entire packet is encrypted by the transport layer.  Both
    //   the server and the client should check whether the underlying
    //   transport layer provides confidentiality (i.e., if encryption is
    //   being used).  If no confidentiality is provided ("none" cipher),
    //   password authentication SHOULD be disabled.  If there is no
    //   confidentiality or no MAC, password change SHOULD be disabled.

    //   Normally, the server responds to this message with success or
    //   failure.  However, if the password has expired, the server SHOULD
    //   indicate this by responding with SSH_MSG_USERAUTH_PASSWD_CHANGEREQ.
    //   In any case, the server MUST NOT allow an expired password to be used
    //   for authentication.

    //      byte      SSH_MSG_USERAUTH_PASSWD_CHANGEREQ
    //      string    prompt in ISO-10646 UTF-8 encoding [RFC3629]
    //      string    language tag [RFC3066]

    //   In this case, the client MAY continue with a different authentication
    //   method, or request a new password from the user and retry password
    //   authentication using the following message.  The client MAY also send
    //   this message instead of the normal password authentication request
    //   without the server asking for it.

    //      byte      SSH_MSG_USERAUTH_REQUEST
    //      string    user name
    //      string    service name
    //      string    "password"
    //      boolean   TRUE
    //      string    plaintext old password in ISO-10646 UTF-8 encoding
    //                 [RFC3629]
    //      string    plaintext new password in ISO-10646 UTF-8 encoding
    //                 [RFC3629]

    //   The server must reply to each request message with
    //   SSH_MSG_USERAUTH_SUCCESS, SSH_MSG_USERAUTH_FAILURE, or another
    //   SSH_MSG_USERAUTH_PASSWD_CHANGEREQ.  The meaning of these is as
    //   follows:

    //      SSH_MSG_USERAUTH_SUCCESS - The password has been changed, and
    //      authentication has been successfully completed.

    //      SSH_MSG_USERAUTH_FAILURE with partial success - The password has
    //      been changed, but more authentications are needed.

    //      SSH_MSG_USERAUTH_FAILURE without partial success - The password
    //      has not been changed.  Either password changing was not supported,
    //      or the old password was bad.  Note that if the server has already
    //      sent SSH_MSG_USERAUTH_PASSWD_CHANGEREQ, we know that it supports
    //      changing the password.

    //      SSH_MSG_USERAUTH_CHANGEREQ - The password was not changed because
    //      the new password was not acceptable (e.g., too easy to guess).

    //   The following method-specific message numbers are used by the
    //   password authentication method.

    //      SSH_MSG_USERAUTH_PASSWD_CHANGEREQ   60

    void handle_userauth_request_password_server(char * username, ssh_buffer_struct * packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        // SSH_REQUEST_AUTH SSH_AUTH_METHOD_PASSWORD
        syslog(LOG_INFO, "Auth request for service %s, method %s for user '%s'",
            "ssh-connection", "password", username);
        packet->in_uint8(); /* skip one byte */
        SSHString password = packet->in_strdup_cstr();
        int rc = this->server_callbacks->auth_password_server_cb(this,
                                                   &username[0],
                                                   &password[0],
                                                   this->server_callbacks->userdata);
        if (rc == SSH_AUTH_SUCCESS || rc == SSH_AUTH_PARTIAL) {
            if (rc == SSH_AUTH_PARTIAL){
                this->ssh_auth_reply_denied_server(true);
            }
            else {
                this->session_state = SSH_SESSION_STATE_AUTHENTICATED;
                this->flags |= SSH_SESSION_FLAG_AUTHENTICATED;
                this->out_buffer->out_uint8(SSH_MSG_USERAUTH_SUCCESS);
                this->packet_send();
                if (this->current_crypto){
                    if(this->current_crypto->delayed_compress_out){
                        syslog(LOG_INFO,"Enabling delayed compression OUT");
                        this->current_crypto->do_compress_out=1;
                    }
                    if (this->current_crypto->delayed_compress_in){
                        syslog(LOG_INFO,"Enabling delayed compression IN");
                        this->current_crypto->do_compress_in=1;
                    }
                }
            }
        }
        else {
            this->ssh_auth_reply_denied_server(false);
        }
    }


    inline void handle_userauth_request_keyboard_interactive_server(char * username, ssh_buffer_struct * packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        // SSH_REQUEST_AUTH SSH_AUTH_METHOD_INTERACTIVE
        syslog(LOG_INFO, "Auth request for service %s, method %s for user '%s'",
            "ssh-connection", "keyboard-interactive", username);

        // from the RFC 4256
        //3.1 Initial Exchange

        //   The authentication starts with the client sending the following
        //   packet:

        //      byte      SSH_MSG_USERAUTH_REQUEST
        //      string    user name (ISO-10646 UTF-8, as defined in [RFC-3629])
        //      string    service name (US-ASCII)
        //      string    "keyboard-interactive" (US-ASCII)
        //      string    language tag (as defined in [RFC-3066])
        //      string    submethods (ISO-10646 UTF-8)

        //   The language tag is deprecated and SHOULD be the empty string.  It
        //   may be removed in a future revision of this specification.  The
        //   server SHOULD instead select the language used based on the tags
        //   communicated during key exchange [SSH-TRANS].

        //   If the language tag is not the empty string, the server SHOULD use
        //   the specified language for any messages sent to the client as part of
        //   this protocol.  The language tag SHOULD NOT be used for language
        //   selection for messages outside of this protocol.  The language to be
        //   used if the server does not support the requested language is
        //   implementation-dependent.

        //   The submethods field is included so the user can give a hint of which
        //   actual methods he wants to use.  It is a comma-separated list of
        //   authentication submethods (software or hardware) which the user
        //   prefers.  If the client has knowledge of the submethods preferred by
        //   the user, presumably through a configuration setting, it MAY use the
        //   submethods field to pass this information to the server.  Otherwise
        //   it MUST send the empty string.

        //   The actual names of the submethods is something which the user and
        //   the server need to agree upon.

        //   Server interpretation of the submethods field is implementation-
        //   dependent.

        //   One possible implementation strategy of the submethods field on the
        //   server is that, unless the user may use multiple different
        //   submethods, the server ignores this field.  If the user may
        //   authenticate using one of several different submethods the server
        //   should treat the submethods field as a hint on which submethod the
        //   user wants to use this time.

        //   Note that when this message is sent to the server, the client has not
        //   yet prompted the user for a password, and so that information is NOT
        //   included with this initial message (unlike the "password" method).

        //   The server MUST reply with either a SSH_MSG_USERAUTH_SUCCESS,
        //   SSH_MSG_USERAUTH_FAILURE, or SSH_MSG_USERAUTH_INFO_REQUEST message.

        //   The server SHOULD NOT reply with the SSH_MSG_USERAUTH_FAILURE message
        //   if the failure is based on the user name or service name; instead it
        //   SHOULD send SSH_MSG_USERAUTH_INFO_REQUEST message(s) which look just
        //   like the one(s) which would have been sent in cases where
        //   authentication should proceed, and then send the failure message
        //   (after a suitable delay, as described below).  The goal is to make it
        //   impossible to find valid usernames by just comparing the results when
        //   authenticating as different users.

        //   The server MAY reply with a SSH_MSG_USERAUTH_SUCCESS message if no
        //   authentication is required for the user in question, however a better
        //   approach, for reasons discussed above, might be to reply with a
        //   SSH_MSG_USERAUTH_INFO_REQUEST message and ignore (don't validate) the
        //   response.

        SSHString lang = packet->in_strdup_cstr();
        (void)lang;
        SSHString submethods = packet->in_strdup_cstr();
        (void)submethods;

        // TODO: we should have two different callbacks instead of the
        // second parameter
        int rc = this->server_callbacks->auth_interactive_server_cb(
                                                username,
                                                // 0 at first call (first instance of object)
                                                0, // 1 when we are coming from INFO_RESPONSE
                                                // msg->auth_request.kbdint_response,
                                                this->server_callbacks->userdata);

        switch (rc)
        {
        case SSH_AUTH_PARTIAL:
            syslog(LOG_INFO, "%s : SSH_AUTH_PARTIAL", __FUNCTION__);
            if (this->kbdint == nullptr) {
                this->kbdint = this->tmp_kbdint;
            }
            this->ssh_auth_interactive_request_server(this->kbdint->name,
                                        this->kbdint->instruction,
                                        this->kbdint->nprompts,
                                        const_cast<const char **>(this->kbdint->prompts),
                                        reinterpret_cast<char*>(this->kbdint->echo));
        break;
        case SSH_AUTH_SUCCESS:
            syslog(LOG_INFO, "%s : SSH_AUTH_SUCCESS", __FUNCTION__);
            this->session_state = SSH_SESSION_STATE_AUTHENTICATED;
            this->flags |= SSH_SESSION_FLAG_AUTHENTICATED;
            this->out_buffer->out_uint8(SSH_MSG_USERAUTH_SUCCESS);
            this->packet_send();
            if (this->current_crypto){
                if(this->current_crypto->delayed_compress_out){
                    syslog(LOG_INFO,"Enabling delayed compression OUT");
                    this->current_crypto->do_compress_out=1;
                }
                if (this->current_crypto->delayed_compress_in){
                    syslog(LOG_INFO,"Enabling delayed compression IN");
                    this->current_crypto->do_compress_in=1;
                }
            }
        break;
        default:
            syslog(LOG_INFO, "%s : SSH_AUTH_??? %d", __FUNCTION__, rc);
            this->ssh_auth_reply_denied_server(false);
        break;
        }

        return;
    }


    void handle_userauth_request_publickey_server(char * service, char * username, ssh_buffer_struct * packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        // SSH_REQUEST_AUTH SSH_AUTH_METHOD_PUBLICKEY
        uint8_t has_sign = packet->in_uint8();
        SSHString algo = packet->in_strdup_cstr();

        syslog(LOG_INFO, "Auth request for service %s, method %s for user '%s' has_sign=%u algo=%s",
            service, "publickey", username, has_sign, &algo[0]);

        if (sizeof(uint32_t) > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        uint32_t pubkey_blob_len = packet->in_uint32_be();
        if (pubkey_blob_len > packet->in_remain()) {
            // ERRRRRRRRRRRRRRRRRRRRRRRRRR
        }
        std::vector<uint8_t> pubkey_blob;
        pubkey_blob.resize(pubkey_blob_len);
        packet->buffer_get_data(&pubkey_blob[0], pubkey_blob_len);

        ssh_key_struct *pubkey;

        ssh_buffer_struct buffer;
        buffer.out_blob(&pubkey_blob[0], pubkey_blob.size());
        int rc = ssh_pki_import_pubkey_blob(buffer, &pubkey);
        if (rc < 0) { return; }
        int signature_state = SSH_PUBLICKEY_STATE_NONE;
        // has a valid signature ?
        if(has_sign) {
            syslog(LOG_INFO, "%s has_sign ---", __FUNCTION__);

            if (sizeof(uint32_t) > packet->in_remain()) {
                // ERRRRRRRRRRRRRRRRRRRRRRRRRR
            }
            uint32_t sig_blob_len = packet->in_uint32_be();
            if (sig_blob_len > packet->in_remain()) {
                // ERRRRRRRRRRRRRRRRRRRRRRRRRR
            }
            std::vector<uint8_t> sig_blob;
            sig_blob.resize(sig_blob_len);
            packet->buffer_get_data(&sig_blob[0],sig_blob_len);

            struct ssh_crypto_struct *crypto =
                this->current_crypto ? this->current_crypto :
                                          this->next_crypto;

            ssh_buffer_struct* digest = new ssh_buffer_struct;

            /* Add session id */
            std::vector<uint8_t> str;
            str.resize(static_cast<uint32_t>(crypto->digest_len));
            memcpy(&str[0], this->session_id, crypto->digest_len);
            digest->out_uint32_be(str.size());
            digest->out_blob(&str[0], str.size());

            syslog(LOG_INFO, "%s building SSH_MSG_USERAUTH_REQUEST ---", __FUNCTION__);

            digest->out_uint8(SSH_MSG_USERAUTH_REQUEST);
            digest->out_length_prefixed_cstr(username);
            digest->out_length_prefixed_cstr("ssh-connection");
            digest->out_length_prefixed_cstr("publickey");
            digest->out_uint8(1); /* Has been signed (TRUE) */

            /* Add the public key algorithm */
            digest->out_length_prefixed_cstr(pubkey->type_c());

            /* Add the publickey as blob */
            SSHString pubkey_blob;

            switch (pubkey->type) {
                case SSH_KEYTYPE_DSS:
                {
                    ssh_buffer_struct buffer;
                    buffer.out_length_prefixed_cstr(pubkey->type_c());
                    syslog(LOG_INFO, "%s SSH_KEYTYPE_DSS", __FUNCTION__);
                    buffer.out_bignum(pubkey->dsa->p); // p
                    buffer.out_bignum(pubkey->dsa->q); // q
                    buffer.out_bignum(pubkey->dsa->g); // g
                    buffer.out_bignum(pubkey->dsa->pub_key); // n
                    std::vector<uint8_t> pubkey_blob;
                    pubkey_blob.resize(static_cast<uint32_t>(buffer.in_remain()));
                    memcpy(&pubkey_blob[0], buffer.get_pos_ptr(), pubkey_blob.size());
                }
                break;
                case SSH_KEYTYPE_RSA:
                case SSH_KEYTYPE_RSA1:
                {
                    ssh_buffer_struct buffer;
                    buffer.out_length_prefixed_cstr(pubkey->type_c());
                    syslog(LOG_INFO, "%s SSH_KEYTYPE_RSA", __FUNCTION__);
                    buffer.out_bignum(pubkey->rsa->e); // e
                    buffer.out_bignum(pubkey->rsa->n); // n
                    std::vector<uint8_t> pubkey_blob;
                    pubkey_blob.resize(static_cast<uint32_t>(buffer.in_remain()));
                    memcpy(&pubkey_blob[0], buffer.get_pos_ptr(), pubkey_blob.size());
                }
                break;
                case SSH_KEYTYPE_ECDSA:
                {
                    syslog(LOG_INFO, "%s SSH_KEYTYPE_ECDSA", __FUNCTION__);
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
                        return;
                    }

                    std::vector<uint8_t> e;
                    e.resize(static_cast<uint32_t>(len_ec));
                    if (e.size() != EC_POINT_point2oct(g, p, POINT_CONVERSION_UNCOMPRESSED, &e[0], e.size(), nullptr)){
                        return;
                    }

                    buffer.out_uint32_be(e.size());
                    buffer.out_blob(&e[0], e.size());
                    std::vector<uint8_t> pubkey_blob;
                    pubkey_blob.resize(static_cast<uint32_t>(buffer.in_remain()));
                    memcpy(&pubkey_blob[0], buffer.get_pos_ptr(), pubkey_blob.size());
                }
                break;
                case SSH_KEYTYPE_UNKNOWN:
                    syslog(LOG_INFO, "%s SSH_KEYTYPE_UNKNOWN", __FUNCTION__);
            }


            digest->out_uint32_be(pubkey_blob.size());
            digest->out_blob(&pubkey_blob[0], pubkey_blob.size());

            // TODO: put error message in a result buffer to use here
            // instead of managing it inside signature_verify_blob
            rc = ssh_pki_signature_verify_blob(sig_blob,
                                               pubkey,
                                               digest->get_pos_ptr(),
                                               digest->in_remain(),
                                               this->error);
            delete digest;
            if (rc < 0) {
                syslog(LOG_INFO, "%s error invalid peer signature ---", __FUNCTION__);
                syslog(LOG_INFO,
                    "Received an invalid  signature from peer");
                // TODO: shouldn't we close connection if this occurs
                signature_state = SSH_PUBLICKEY_STATE_WRONG;
                return;
            }
            syslog(LOG_INFO, "Valid signature received");
            signature_state = SSH_PUBLICKEY_STATE_VALID;
        }

        syslog(LOG_INFO, "%s python callback auth pubkey---", __FUNCTION__);

        rc = this->server_callbacks->auth_pubkey_server_cb(
                                        username,
                                        pubkey,
                                        signature_state,
                                        this->server_callbacks->userdata);

        syslog(LOG_INFO, "userauth_request_publickey %d", rc);
        switch (signature_state){
        default:
        case SSH_PUBLICKEY_STATE_NONE:
            if (rc == SSH_AUTH_SUCCESS) {
                syslog(LOG_INFO, "%s SSH_PUBLICKEY_STATE_NONE -> SSH_AUTH_SUCCESS", __FUNCTION__);
                syslog(LOG_INFO, "userauth_request_publickey SSH_AUTH_SUCCESS 2");

                SSHString algo(pubkey->type_c());
                this->out_buffer->out_uint8(SSH_MSG_USERAUTH_PK_OK);
                this->out_buffer->out_uint32_be(algo.size());
                this->out_buffer->out_blob(&algo[0], algo.size());
                this->out_buffer->out_uint32_be(pubkey_blob.size());
                this->out_buffer->out_blob(&pubkey_blob[0], pubkey_blob.size());

                syslog(LOG_INFO, "%s send SSH_MSG_USERAUTH_PK_OK", __FUNCTION__);

                this->packet_send();
            }
            else {
                syslog(LOG_INFO, "userauth_request_publickey SSH_AUTH_DENIED");
                this->ssh_auth_reply_denied_server(false);
            }
        break;
        case SSH_PUBLICKEY_STATE_VALID:
            if (rc == SSH_AUTH_PARTIAL){
                syslog(LOG_INFO, "%s SSH_PUBLICKEY_STATE_VALID -> SSH_AUTH_PARTIAL", __FUNCTION__);
                syslog(LOG_INFO, "userauth_request_publickey SSH_AUTH_PARTIAL");
                this->ssh_auth_reply_denied_server(true);
            }
            else if (rc == SSH_AUTH_SUCCESS){
                syslog(LOG_INFO, "%s SSH_PUBLICKEY_STATE_VALID -> SUCCESS", __FUNCTION__);
                this->session_state = SSH_SESSION_STATE_AUTHENTICATED;
                this->flags |= SSH_SESSION_FLAG_AUTHENTICATED;
                syslog(LOG_INFO, "userauth_request_publickey SSH_AUTH_SUCCESS");
                this->out_buffer->out_uint8(SSH_MSG_USERAUTH_SUCCESS);
                this->packet_send();
                if (this->current_crypto){
                    if(this->current_crypto->delayed_compress_out){
                        syslog(LOG_INFO,"Enabling delayed compression OUT");
                        this->current_crypto->do_compress_out=1;
                    }
                    if (this->current_crypto->delayed_compress_in){
                        syslog(LOG_INFO,"Enabling delayed compression IN");
                        this->current_crypto->do_compress_in=1;
                    }
                }
            }
            else {
                syslog(LOG_INFO, "%s SSH_PUBLICKEY_STATE_VALID -> SSH_AUTH_DENIED", __FUNCTION__);
                syslog(LOG_INFO, "userauth_request_publickey SSH_AUTH_DENIED");
                this->ssh_auth_reply_denied_server(false);
            }
        break;
        case SSH_PUBLICKEY_STATE_WRONG:
            if (rc == SSH_AUTH_PARTIAL){
                syslog(LOG_INFO, "%s SSH_PUBLICKEY_STATE_VALID -> SSH_AUTH_PARTIAL", __FUNCTION__);
                syslog(LOG_INFO, "userauth_request_publickey SSH_AUTH_PARTIAL");
                this->ssh_auth_reply_denied_server(true);
            }
            else if (rc == SSH_AUTH_SUCCESS){
                syslog(LOG_INFO, "%s SSH_PUBLICKEY_STATE_VALID -> SSH_AUTH_SUCCESS", __FUNCTION__);
                this->session_state = SSH_SESSION_STATE_AUTHENTICATED;
                this->flags |= SSH_SESSION_FLAG_AUTHENTICATED;
                syslog(LOG_INFO, "userauth_request_publickey SSH_AUTH_SUCCESS");
                this->out_buffer->out_uint8(SSH_MSG_USERAUTH_SUCCESS);
                this->packet_send();
                if (this->current_crypto){
                    if(this->current_crypto->delayed_compress_out){
                        syslog(LOG_INFO,"Enabling delayed compression OUT");
                        this->current_crypto->do_compress_out=1;
                    }
                    if (this->current_crypto->delayed_compress_in){
                        syslog(LOG_INFO,"Enabling delayed compression IN");
                        this->current_crypto->do_compress_in=1;
                    }
                }
            }
            else {
                syslog(LOG_INFO, "%s SSH_PUBLICKEY_STATE_VALID -> SSH_AUTH_DENIED", __FUNCTION__);
                syslog(LOG_INFO, "userauth_request_publickey SSH_AUTH_DENIED");
                this->ssh_auth_reply_denied_server(false);
            }
        break;
        }
    }

    void handle_userauth_gssapi_with_mic_server(char * username, ssh_buffer_struct * packet)
    {
        // SSH_REQUEST_AUTH;
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        syslog(LOG_INFO, "Auth request for service %s, method %s for user '%s'",
            "ssh-connection", "gss-api-with-mic", username);
        ;
        uint32_t n_oid = packet->in_uint32_be();
        if(n_oid > 100){
         ssh_set_error(this->error,  SSH_FATAL, "USERAUTH_REQUEST: gssapi-with-mic OID count too big (%d)",n_oid);
            return;
        }

        syslog(LOG_INFO, "gssapi: %d OIDs", n_oid);
        std::vector<SSHString> oids;
        oids.resize(n_oid);

        for (auto & oid : oids ){
            uint32_t oid_len = packet->in_uint32_be();
            oid.resize(oid_len);
            packet->buffer_get_data(&oid[0], oid_len);
        }
        this->ssh_gssapi_handle_userauth_server(username, n_oid, oids);
        return;
    }

    int ssh_gssapi_handle_userauth_server(const char *user, uint32_t n_oid, std::vector<SSHString> oids){
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        char service_name[]="host";
        gss_buffer_desc name_buf;
        gss_name_t server_name; /* local server fqdn */
        OM_uint32 maj_stat, min_stat;
        unsigned int i;
        gss_OID_set supported; /* oids supported by server */
        gss_OID_set both_supported; /* oids supported by both client and server */
        gss_OID_set selected; /* oid selected for authentication */
        int present=0;
        int oid_count=0;
        struct gss_OID_desc_struct oid;

        gss_create_empty_oid_set(&min_stat, &both_supported);
        maj_stat = gss_indicate_mechs(&min_stat, &supported);
        for (i=0; i < supported->count; ++i){
            // TODO: avoid too long buffers, we can make this one static and truncate it
            char *hexa = new char[supported->elements[i].length * 3 + 1];
            size_t q = 0;
            size_t j = 0;
            for (q = 0; q < supported->elements[q].length; q++) {
                const uint8_t cl = reinterpret_cast<uint8_t *>(supported->elements[q].elements)[q] >> 4;
                const uint8_t ch = reinterpret_cast<uint8_t *>(supported->elements[q].elements)[q] & 0x0F;
                hexa[j] = (ch < 10?'0':'a')+ch;
                hexa[j+1] = (cl < 10?'0':'a')+cl;
                hexa[j+2] = ':';
                j+= 3;
            }
            hexa[j>0?(j-1):0] = 0;
            delete[] hexa;
        }

        for (auto & oid_s : oids){
            syslog(LOG_INFO,"GSSAPI: i=%u n_oid=%u", i, n_oid);
            size_t len = oid_s.size();
            syslog(LOG_INFO,"GSSAPI: oid_len=%d %u %u %u", static_cast<int>(len), SSH_OID_TAG, oid_s[0], oid_s[1]);
            if(len < 2){
                syslog(LOG_WARNING,"GSSAPI: received invalid OID 1");
                continue;
            }
            if(oid_s[0] != SSH_OID_TAG){
                syslog(LOG_WARNING,"GSSAPI: received invalid OID 2");
                continue;
            }
            if((static_cast<size_t>(oid_s[1])) != len - 2){
                syslog(LOG_WARNING,"GSSAPI: received invalid OID 3");
                continue;
            }
            oid.elements = &oid_s[2];
            oid.length = len - 2;
            gss_test_oid_set_member(&min_stat, &oid, supported,&present);
            if(present){
                gss_add_oid_set_member(&min_stat,&oid,&both_supported);
                oid_count++;
            }
        }
        syslog(LOG_INFO,"GSSAPI: i=%u loop done", i);
        gss_release_oid_set(&min_stat, &supported);
        if (oid_count == 0){
            syslog(LOG_INFO,"GSSAPI: no OID match");
            this->ssh_auth_reply_denied_server(false);
            gss_release_oid_set(&min_stat, &both_supported);
            return SSH_OK;
        }
        /* from now we have room for context */
        if (this->gssapi == nullptr){
            this->gssapi = new ssh_gssapi_struct;
        }

        name_buf.value = service_name;
        name_buf.length = strlen(static_cast<const char*>(name_buf.value)) + 1;
        maj_stat = gss_import_name(&min_stat, &name_buf, GSS_C_NT_HOSTBASED_SERVICE, &server_name);
        if (maj_stat != GSS_S_COMPLETE) {
            syslog(LOG_WARNING, "importing name %d, %d", maj_stat, min_stat);
            gss_buffer_desc buffer;
            OM_uint32 dummy;
            OM_uint32 message_context;
            gss_display_status(&dummy, maj_stat,GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
            syslog(LOG_INFO, "GSSAPI(%s): %s", "importing name", static_cast<const char *>(buffer.value));
            return -1;
        }

        maj_stat = gss_acquire_cred(&min_stat, server_name, 0,
                both_supported, GSS_C_ACCEPT,
                &this->gssapi->server_creds, &selected, nullptr);
        gss_release_name(&min_stat, &server_name);
        gss_release_oid_set(&min_stat, &both_supported);

        if (maj_stat != GSS_S_COMPLETE) {
            syslog(LOG_WARNING, "error acquiring credentials %d, %d", maj_stat, min_stat);

            gss_buffer_desc buffer;
            OM_uint32 dummy;
            OM_uint32 message_context;
            gss_display_status(&dummy, maj_stat,GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
            syslog(LOG_INFO, "GSSAPI(%s): %s", "acquiring creds", static_cast<const char *>(buffer.value));

            this->ssh_auth_reply_denied_server(false);
            return SSH_ERROR;
        }

        syslog(LOG_INFO, "acquiring credentials %d, %d", maj_stat, min_stat);

        /* finding which OID from client we selected */
        for (auto & oid_s : oids){
            size_t len = oid_s.size();
            if(len < 2){
                syslog(LOG_WARNING,"GSSAPI: received invalid OID 1");
                continue;
            }
            if(oid_s[0] != SSH_OID_TAG){
                syslog(LOG_WARNING,"GSSAPI: received invalid OID 2");
                continue;
            }
            if((static_cast<size_t>(oid_s[1])) != len - 2){
                syslog(LOG_WARNING,"GSSAPI: received invalid OID 3");
                continue;
            }
            oid.elements = &oid_s[2];
            oid.length = len - 2;
            gss_test_oid_set_member(&min_stat,&oid,selected,&present);
            if(present){
                syslog(LOG_INFO, "Selected oid %d", i);
                break;
            }
        }
        this->gssapi->mech.length = oid.length;
        this->gssapi->mech.elements = malloc(oid.length);
        // TODO : check memory allocation
        memcpy(this->gssapi->mech.elements, oid.elements, oid.length);
        gss_release_oid_set(&min_stat, &selected);
        this->gssapi->user = strdup(user);
        this->gssapi->service = service_name;
        this->gssapi->state = SSH_GSSAPI_STATE_RCV_TOKEN;

        this->out_buffer->out_uint8(SSH_MSG_USERAUTH_GSSAPI_RESPONSE);
        this->out_buffer->out_uint32_be(oids[i].size());
        this->out_buffer->out_blob(&oids[i][0], oids[i].size());
        this->packet_send();

        return SSH_OK;
    }



/**
 * @internal
 *
 * @brief Handles a SSH_USERAUTH_PK_OK or SSH_USERAUTH_INFO_REQUEST packet.
 *
 * Since the two types of packets share the same code, additional work is done
 * to understand if we are in a public key or keyboard-interactive context.
 */
   int ssh_packet_userauth_pk_ok_server(uint8_t type, ssh_buffer_struct* packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        int rc;

        syslog(LOG_INFO, "Received SSH_USERAUTH_PK_OK/INFO_REQUEST/GSSAPI_RESPONSE");

        if(this->auth_state == SSH_AUTH_STATE_KBDINT_SENT){
            /* Assuming we are in keyboard-interactive context */
            syslog(LOG_INFO,
                    "keyboard-interactive context, assuming SSH_USERAUTH_INFO_REQUEST");
            rc = this->ssh_packet_userauth_info_request_server(type,packet);
        }
        else if (this->auth_state == SSH_AUTH_STATE_GSSAPI_REQUEST_SENT){
            rc = this->ssh_packet_userauth_gssapi_response_server(type, packet);
        }
        else {
            this->auth_state=SSH_AUTH_STATE_PK_OK;
            syslog(LOG_INFO, "Assuming SSH_USERAUTH_PK_OK");
            rc = SSH_PACKET_USED;
        }

        return rc;
    }

    int ssh_packet_userauth_info_request_server(uint8_t type, ssh_buffer_struct* packet)
    {
      syslog(LOG_INFO, "%s ---", __FUNCTION__);
      uint32_t i;
      (void)type;

        /* name of the "asking" window showed to client */
        if (sizeof(uint32_t) > packet->in_remain()) {
            ssh_set_error(this->error,  SSH_FATAL, "Invalid USERAUTH_INFO_REQUEST msg");
            return SSH_PACKET_USED;
        }
        uint32_t name_len = packet->in_uint32_be();
        if (name_len > packet->in_remain()) {
            ssh_set_error(this->error,  SSH_FATAL, "Invalid USERAUTH_INFO_REQUEST msg");
            return SSH_PACKET_USED;
        }
        std::vector<uint8_t> name;
        name.resize(name_len);
        packet->buffer_get_data(&name[0], name_len);

        if (sizeof(uint32_t) > packet->in_remain()) {
            ssh_set_error(this->error,  SSH_FATAL, "Invalid USERAUTH_INFO_REQUEST msg");
            return SSH_PACKET_USED;
        }
        uint32_t instruction_len = packet->in_uint32_be();
        if (instruction_len > packet->in_remain()) {
            ssh_set_error(this->error,  SSH_FATAL, "Invalid USERAUTH_INFO_REQUEST msg");
            return SSH_PACKET_USED;
        }
        std::vector<uint8_t> instruction;
        instruction.resize(instruction_len);
        packet->buffer_get_data(&instruction[0], instruction_len);

        if (sizeof(uint32_t) > packet->in_remain()) {
            ssh_set_error(this->error,  SSH_FATAL, "Invalid USERAUTH_INFO_REQUEST msg");
            return SSH_PACKET_USED;
        }
        uint32_t tmp_len = packet->in_uint32_be();
        if (tmp_len > packet->in_remain()) {
            ssh_set_error(this->error,  SSH_FATAL, "Invalid USERAUTH_INFO_REQUEST msg");
            return SSH_PACKET_USED;
        }
        std::vector<uint8_t> tmp;
        tmp.resize(tmp_len);
        packet->buffer_get_data(&tmp[0], tmp_len);

        uint32_t nprompts = packet->in_uint32_be();

        if (this->kbdint == nullptr) {
            this->kbdint = new ssh_kbdint_struct();
        }
        else {
            ssh_kbdint_clean(this->kbdint);
        }

        this->kbdint->name = new char [name.size() + 1];
        memcpy(this->kbdint->name, &name[0], name.size());
        this->kbdint->name[name.size()] = 0;

        this->kbdint->instruction = new char[instruction.size() + 1];
        memcpy(this->kbdint->instruction, &instruction[0], instruction.size());
        this->kbdint->instruction[instruction.size()] = 0;

        syslog(LOG_DEBUG, "%d keyboard-interactive prompts", nprompts);
        if (nprompts > KBDINT_MAX_PROMPT) {
            ssh_set_error(this->error,  SSH_FATAL,
                "Too much prompts requested by the server: %u (0x%.4x)",
                nprompts, nprompts);
            ssh_kbdint_free(this->kbdint);
            this->kbdint = nullptr;

        return SSH_PACKET_USED;
        }

        this->kbdint->nprompts = nprompts;
        this->kbdint->nanswers = nprompts;
        this->kbdint->prompts = static_cast<char**>(malloc(nprompts * sizeof(char *)));
        // TODO : check memory allocation
        memset(this->kbdint->prompts, 0, nprompts * sizeof(char *));

        this->kbdint->echo = static_cast<unsigned char*>(malloc(nprompts));
        // TODO : check memory allocation
        memset(this->kbdint->echo, 0, nprompts);

        for (i = 0; i < nprompts; i++) {
            if (sizeof(uint32_t) > packet->in_remain()) {
                // TODO : check memory allocation
                this->kbdint->nprompts = i-1;
                ssh_kbdint_free(this->kbdint);
                this->kbdint = nullptr;
                return SSH_PACKET_USED;
            }
            uint32_t tmp2_len = packet->in_uint32_be();
            if (tmp2_len > packet->in_remain()) {
               // TODO : check memory allocation
                this->kbdint->nprompts = i-1;
                ssh_kbdint_free(this->kbdint);
                this->kbdint = nullptr;

                return SSH_PACKET_USED;
            }
            std::vector<uint8_t> tmp2;
            tmp2.resize(tmp2_len);
            packet->buffer_get_data(&tmp2[0], tmp2_len);

            this->kbdint->echo[i] = packet->in_uint8();

            this->kbdint->prompts[i] = new char [tmp2.size() + 1];
            // TODO : check memory allocation
            memcpy(this->kbdint->prompts[i], &tmp2[0], tmp2.size());
            this->kbdint->prompts[i][tmp2.size()] = 0;

        }
        this->auth_state=SSH_AUTH_STATE_INFO;

        return SSH_PACKET_USED;
    }

    int ssh_packet_userauth_gssapi_response_server(uint8_t type, ssh_buffer_struct* packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        gss_uint32 maj_stat, min_stat;
        gss_buffer_desc input_token = GSS_C_EMPTY_BUFFER;
        gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
        (void)type;

        syslog(LOG_INFO, "Received SSH_USERAUTH_GSSAPI_RESPONSE");
        if (this->auth_state != SSH_AUTH_STATE_GSSAPI_REQUEST_SENT){
            ssh_set_error(this->error,  SSH_FATAL, "Invalid state in ssh_packet_userauth_gssapi_response");
            return SSH_PACKET_USED;
        }

        if (sizeof(uint32_t) > packet->in_remain()) {
            ssh_set_error(this->error,  SSH_FATAL, "parse error");
            return SSH_PACKET_USED;
        }
        uint32_t oid_s_len = packet->in_uint32_be();
        if (oid_s_len > packet->in_remain()) {
            ssh_set_error(this->error,  SSH_FATAL, "parse error");
            return SSH_PACKET_USED;
        }
        std::vector<uint8_t> oid_s;
        oid_s.resize(oid_s_len);
        packet->buffer_get_data(&oid_s[0],oid_s_len);
        this->gssapi->client.oid = ssh_gssapi_oid_from_string(oid_s);

        if (!this->gssapi->client.oid) {
            ssh_set_error(this->error,  SSH_FATAL, "Invalid OID");
            return SSH_PACKET_USED;
        }

        this->gssapi->client.flags = GSS_C_MUTUAL_FLAG | GSS_C_INTEG_FLAG;
        if (this->opts.gss_delegate_creds) {
            this->gssapi->client.flags |= GSS_C_DELEG_FLAG;
        }

        /* prepare the first TOKEN response */
        maj_stat = gss_init_sec_context(&min_stat,
                                        this->gssapi->client.creds,
                                        &this->gssapi->ctx,
                                        this->gssapi->client.server_name,
                                        this->gssapi->client.oid,
                                        this->gssapi->client.flags,
                                        0, nullptr, &input_token, nullptr,
                                        &output_token, nullptr, nullptr);
        if(GSS_ERROR(maj_stat)){
            gss_buffer_desc buffer;
            OM_uint32 dummy;
            OM_uint32 message_context;
            gss_display_status(&dummy, maj_stat,GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
            syslog(LOG_WARNING, "GSSAPI(%s): %s", "Initializing gssapi context", static_cast<const char *>(buffer.value));

            return SSH_PACKET_USED;
        }
        if (output_token.length != 0){
            // TODO: avoid too long buffers, we can make this one static and truncate it
            char *hexa = new char[output_token.length * 3 + 1];
            size_t q = 0;
            size_t j = 0;
            for (q = 0; q < output_token.length; q++) {
                const uint8_t cl = reinterpret_cast<uint8_t *>(output_token.value)[q] >> 4;
                const uint8_t ch = reinterpret_cast<uint8_t *>(output_token.value)[q] & 0x0F;
                hexa[j] = (ch < 10?'0':'a')+ch;
                hexa[j+1] = (cl < 10?'0':'a')+cl;
                hexa[j+2] = ':';
                j+= 3;
            }
            hexa[j>0?(j-1):0] = 0;
            syslog(LOG_INFO, "GSSAPI: sending token %s",hexa);
            delete[] hexa;

            std::vector<uint8_t> token;
            token.resize(static_cast<uint32_t>(output_token.length));
            memcpy(&token[0], output_token.value, output_token.length);
            this->out_buffer->out_uint8(SSH_MSG_USERAUTH_GSSAPI_TOKEN);
            this->out_buffer->out_uint32_be(token.size());
            this->out_buffer->out_blob(&token[0], token.size());
            this->packet_send();
            this->auth_state = SSH_AUTH_STATE_GSSAPI_TOKEN;
        }
        return SSH_PACKET_USED;
    }


    int ssh_packet_userauth_info_response_server(ssh_buffer_struct* packet)
    {
        // SSH_REQUEST_AUTH SSH_AUTH_METHOD_INTERACTIVE
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        uint32_t i;

    //    int kbdint_response = 1;
        uint32_t nanswers = packet->in_uint32_be();

        if (this->kbdint == nullptr) {
            syslog(LOG_INFO, "Warning: Got a keyboard-interactive "
                                "response but it seems we didn't send the request.");

            this->kbdint = new ssh_kbdint_struct;
        }

        syslog(LOG_INFO,"%s kbdint: %d answers", __FUNCTION__, nanswers);
        if (nanswers > KBDINT_MAX_PROMPT) {
            ssh_set_error(this->error,  SSH_FATAL,
                "Too many answers received from client: %u (0x%.4x)",
                nanswers, nanswers);
            ssh_kbdint_free(this->kbdint);
            this->kbdint = nullptr;
            return SSH_PACKET_USED;
        }

        if(nanswers != this->kbdint->nprompts) {
            /* warn but let the application handle this case */
            syslog(LOG_INFO, "Warning: Number of prompts and answers"
                        " mismatch: p=%u a=%u", this->kbdint->nprompts, nanswers);
        }

        syslog(LOG_INFO,"%s kbdint (2): %d answers", __FUNCTION__, nanswers);

        this->kbdint->nanswers = nanswers;
        this->kbdint->answers = static_cast<char**>(malloc(nanswers * sizeof(char *)));
        memset(this->kbdint->answers, 0, nanswers * sizeof(char *));

        for (i = 0; i < nanswers; i++) {
            this->kbdint->answers[i] = strdup(&packet->in_strdup_cstr()[0]);
        }

        int rc = this->server_callbacks->auth_interactive_server_cb(
                                               this->kbdint->name,
                                                  // 0 at first call (first instance of object)
                                               1, // 1 when we are coming from INFO_RESPONSE
                                               // msg->auth_request.kbdint_response,
                                               this->server_callbacks->userdata);

        switch (rc)
        {
        case SSH_AUTH_PARTIAL:
            syslog(LOG_INFO, "%s : SSH_AUTH_PARTIAL", __FUNCTION__);
            if (this->kbdint == nullptr) {
                this->kbdint = this->tmp_kbdint;
            }
            this->ssh_auth_interactive_request_server(this->kbdint->name,
                                        this->kbdint->instruction,
                                        this->kbdint->nprompts,
                                        const_cast<const char**>(this->kbdint->prompts),
                                        reinterpret_cast<char*>(this->kbdint->echo));
        break;
        case SSH_AUTH_SUCCESS:
            syslog(LOG_INFO, "%s : SSH_AUTH_SUCCESS", __FUNCTION__);
            this->session_state = SSH_SESSION_STATE_AUTHENTICATED;
            this->flags |= SSH_SESSION_FLAG_AUTHENTICATED;
            this->out_buffer->out_uint8(SSH_MSG_USERAUTH_SUCCESS);
            this->packet_send();
            if (this->current_crypto){
                if(this->current_crypto->delayed_compress_out){
                    syslog(LOG_INFO,"Enabling delayed compression OUT");
                    this->current_crypto->do_compress_out=1;
                }
                if (this->current_crypto->delayed_compress_in){
                    syslog(LOG_INFO,"Enabling delayed compression IN");
                    this->current_crypto->do_compress_in=1;
                }
            }
        break;
        default:
            syslog(LOG_INFO, "%s : SSH_AUTH_??? %d", __FUNCTION__, rc);
            this->ssh_auth_reply_denied_server(false);
        break;
        }

        return SSH_PACKET_USED;
    }


    int ssh_auth_interactive_request_server(const char *name,
                                const char *instruction, unsigned int num_prompts,
                                const char **prompts, char *echo)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        unsigned int i = 0;
        if(name == nullptr) {
            syslog(LOG_INFO, "%s name is null---", __FUNCTION__);
            return SSH_ERROR;
        }
        if(instruction == nullptr) {
            syslog(LOG_INFO, "%s instruction is null---", __FUNCTION__);
            return SSH_ERROR;
        }
        if(num_prompts > 0 && (prompts == nullptr || echo == nullptr)) {
            syslog(LOG_INFO, "%s prompts = %p echo = %p is null---", __FUNCTION__, static_cast<void*>(prompts), static_cast<void*>(echo));
            return SSH_ERROR;
        }

        this->out_buffer->out_uint8(SSH_MSG_USERAUTH_INFO_REQUEST);
        this->out_buffer->out_length_prefixed_cstr(name);
        this->out_buffer->out_length_prefixed_cstr(instruction);
        this->out_buffer->out_length_prefixed_cstr(""); /* language tag */
        this->out_buffer->out_uint32_be(num_prompts); /* num prompts */

        for(i = 0; i < num_prompts; i++) {
            this->out_buffer->out_length_prefixed_cstr(prompts[i]); // prompt[i]
            this->out_buffer->out_uint8(echo[i]); /* echo[i] */
        }

        this->packet_send();

        syslog(LOG_INFO, "------ %s A0", __FUNCTION__);

        /* fill in the kbdint structure */
        if (this->kbdint == nullptr) {
            syslog(LOG_INFO, "Warning: Got a "
                             "keyboard-interactive response but it "
                             "seems we didn't send the request.");

            this->kbdint = new ssh_kbdint_struct;
            // TODO : check memory allocation
        }
        else {
            ssh_kbdint_clean(this->kbdint);
        }

        this->kbdint->name = strdup(name);
        // TODO : check memory allocation
        this->kbdint->instruction = strdup(instruction);
        // TODO : check memory allocation

        this->kbdint->nprompts = num_prompts;

        if(num_prompts > 0) {
            this->kbdint->prompts = static_cast<char**>(malloc(num_prompts * sizeof(char *)));
            // TODO : check memory allocation
            this->kbdint->echo = static_cast<unsigned char*>(malloc(num_prompts * sizeof(unsigned char)));
            // TODO : check memory allocation
            for (i = 0; i < num_prompts; i++) {
                this->kbdint->echo[i] = echo[i];
                this->kbdint->prompts[i] = strdup(prompts[i]);
                // TODO : check memory allocation
            }
        }
        else {
            this->kbdint->prompts = nullptr;
            this->kbdint->echo = nullptr;
        }
        return SSH_OK;
    }


    void ssh_gssapi_free_server()
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        OM_uint32 min;
        if (this->gssapi == nullptr)
            return;
        free(this->gssapi->user);
        this->gssapi->user = nullptr;
        free(this->gssapi->mech.elements);
        this->gssapi->mech.elements = nullptr;
        gss_release_cred(&min,&this->gssapi->server_creds);
        if (this->gssapi->client.creds !=
                        this->gssapi->client.client_deleg_creds) {
            gss_release_cred(&min, &this->gssapi->client.creds);
        }
        free(this->gssapi);
        this->gssapi = nullptr;
    }



    // RFC-4252:
    // The Secure Shell (SSH) Authentication Protocol
    // ==============================================

    //4.  The Authentication Protocol Framework

    //   The server drives the authentication by telling the client which
    //   authentication methods can be used to continue the exchange at any
    //   given time.  The client has the freedom to try the methods listed by
    //   the server in any order.  This gives the server complete control over
    //   the authentication process if desired, but also gives enough
    //   flexibility for the client to use the methods it supports or that are
    //   most convenient for the user, when multiple methods are offered by
    //   the server.

    //   Authentication methods are identified by their name, as defined in
    //   [SSH-ARCH].  The "none" method is reserved, and MUST NOT be listed as
    //   supported.  However, it MAY be sent by the client.  The server MUST
    //   always reject this request, unless the client is to be granted access
    //   without any authentication, in which case, the server MUST accept
    //   this request.  The main purpose of sending this request is to get the
    //   list of supported methods from the server.

    //   The server SHOULD have a timeout for authentication and disconnect if
    //   the authentication has not been accepted within the timeout period.
    //   The RECOMMENDED timeout period is 10 minutes.  Additionally, the
    //   implementation SHOULD limit the number of failed authentication
    //   attempts a client may perform in a single session (the RECOMMENDED
    //   limit is 20 attempts).  If the threshold is exceeded, the server
    //   SHOULD disconnect.

    //   Additional thoughts about authentication timeouts and retries may be
    //   found in [ssh-1.2.30].

    //5.  Authentication Requests

    //   All authentication requests MUST use the following message format.
    //   Only the first few fields are defined; the remaining fields depend on
    //   the authentication method.

    //      byte      SSH_MSG_USERAUTH_REQUEST
    //      string    user name in ISO-10646 UTF-8 encoding [RFC3629]
    //      string    service name in US-ASCII
    //      string    method name in US-ASCII
    //      ....      method specific fields

    //   The 'user name' and 'service name' are repeated in every new
    //   authentication attempt, and MAY change.  The server implementation
    //   MUST carefully check them in every message, and MUST flush any
    //   accumulated authentication states if they change.  If it is unable to

    //   flush an authentication state, it MUST disconnect if the 'user name'
    //   or 'service name' changes.

    //   The 'service name' specifies the service to start after
    //   authentication.  There may be several different authenticated
    //   services provided.  If the requested service is not available, the
    //   server MAY disconnect immediately or at any later time.  Sending a
    //   proper disconnect message is RECOMMENDED.  In any case, if the
    //   service does not exist, authentication MUST NOT be accepted.

    //   If the requested 'user name' does not exist, the server MAY
    //   disconnect, or MAY send a bogus list of acceptable authentication
    //   'method name' values, but never accept any.  This makes it possible
    //   for the server to avoid disclosing information on which accounts
    //   exist.  In any case, if the 'user name' does not exist, the
    //   authentication request MUST NOT be accepted.

    //   While there is usually little point for clients to send requests that
    //   the server does not list as acceptable, sending such requests is not
    //   an error, and the server SHOULD simply reject requests that it does
    //   not recognize.

    //   An authentication request MAY result in a further exchange of
    //   messages.  All such messages depend on the authentication 'method
    //   name' used, and the client MAY at any time continue with a new
    //   SSH_MSG_USERAUTH_REQUEST message, in which case the server MUST
    //   abandon the previous authentication attempt and continue with the new
    //   one.

    //   The following 'method name' values are defined.

    //      "publickey"             REQUIRED
    //      "password"              OPTIONAL
    //      "hostbased"             OPTIONAL
    //      "none"                  NOT RECOMMENDED

    //   Additional 'method name' values may be defined as specified in
    //   [SSH-ARCH] and [SSH-NUMBERS].

    //5.1.  Responses to Authentication Requests

    //   If the server rejects the authentication request, it MUST respond
    //   with the following:

    //      byte         SSH_MSG_USERAUTH_FAILURE
    //      name-list    authentications that can continue
    //      boolean      partial success

    //   The 'authentications that can continue' is a comma-separated name-
    //   list of authentication 'method name' values that may productively
    //   continue the authentication dialog.

    //   It is RECOMMENDED that servers only include those 'method name'
    //   values in the name-list that are actually useful.  However, it is not
    //   illegal to include 'method name' values that cannot be used to
    //   authenticate the user.

    //   Already successfully completed authentications SHOULD NOT be included
    //   in the name-list, unless they should be performed again for some
    //   reason.

    //   The value of 'partial success' MUST be TRUE if the authentication
    //   request to which this is a response was successful.  It MUST be FALSE
    //   if the request was not successfully processed.

    //   When the server accepts authentication, it MUST respond with the
    //   following:

    //      byte      SSH_MSG_USERAUTH_SUCCESS

    //   Note that this is not sent after each step in a multi-method
    //   authentication sequence, but only when the authentication is
    //   complete.

    //   The client MAY send several authentication requests without waiting
    //   for responses from previous requests.  The server MUST process each
    //   request completely and acknowledge any failed requests with a
    //   SSH_MSG_USERAUTH_FAILURE message before processing the next request.

    //   A request that requires further messages to be exchanged will be
    //   aborted by a subsequent request.  A client MUST NOT send a subsequent
    //   request if it has not received a response from the server for a
    //   previous request.  A SSH_MSG_USERAUTH_FAILURE message MUST NOT be
    //   sent for an aborted method.

    //   SSH_MSG_USERAUTH_SUCCESS MUST be sent only once.  When
    //   SSH_MSG_USERAUTH_SUCCESS has been sent, any further authentication
    //   requests received after that SHOULD be silently ignored.

    //   Any non-authentication messages sent by the client after the request
    //   that resulted in SSH_MSG_USERAUTH_SUCCESS being sent MUST be passed
    //   to the service being run on top of this protocol.  Such messages can
    //   be identified by their message numbers (see Section 6).

    int ssh_auth_reply_denied_server(bool partial)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        char methods[256] = {0};
        uint32_t auth_methods_flags =
            (this->auth_methods == 0)?(SSH_AUTH_METHOD_PUBLICKEY|SSH_AUTH_METHOD_PASSWORD)
            : this->auth_methods & (
              SSH_AUTH_METHOD_PUBLICKEY
            | SSH_AUTH_METHOD_GSSAPI_MIC
            | SSH_AUTH_METHOD_INTERACTIVE
            | SSH_AUTH_METHOD_PASSWORD
            | SSH_AUTH_METHOD_HOSTBASED
            );

        bool coma = false;
        if (auth_methods_flags & SSH_AUTH_METHOD_PUBLICKEY){
            strcat(methods, "publickey");
            coma = true;
        }
        if (auth_methods_flags & SSH_AUTH_METHOD_GSSAPI_MIC){
            if (coma) { strcat(methods, ","); }
            strcat(methods, "gssapi-with-mic");
            coma = true;
        }
        if (auth_methods_flags & SSH_AUTH_METHOD_INTERACTIVE){
            if (coma) { strcat(methods, ","); }
            strcat(methods, "keyboard-interactive");
            coma = true;
        }
        if (auth_methods_flags & SSH_AUTH_METHOD_PASSWORD){
            if (coma) { strcat(methods, ","); }
            strcat(methods, "password");
            coma = true;
        }
        if (auth_methods_flags & SSH_AUTH_METHOD_HOSTBASED){
            if (coma) { strcat(methods, ","); }
            strcat(methods, "hostbased");
        }

        syslog(LOG_INFO, "Sending %s auth failure. Methods that can continue: %s",
            partial?"a partial":"an", methods);
        this->out_buffer->out_uint8(SSH_MSG_USERAUTH_FAILURE);
        this->out_buffer->out_length_prefixed_cstr(methods);
        this->out_buffer->out_uint8(partial?1:0);
        this->packet_send();

        return SSH_OK;
    }

    int ssh_packet_userauth_gssapi_mic_server(uint8_t type, ssh_buffer_struct* packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        OM_uint32 maj_stat, min_stat;
        gss_buffer_desc mic_buf = GSS_C_EMPTY_BUFFER;
        gss_buffer_desc mic_token_buf = GSS_C_EMPTY_BUFFER;

        (void)type;
        ssh_buffer_struct * mic_buffer = nullptr;

        syslog(LOG_INFO,"Received SSH_MSG_USERAUTH_GSSAPI_MIC");

        if (sizeof(uint32_t) > packet->in_remain()) {
            this->ssh_auth_reply_denied_server(false);
            this->ssh_gssapi_free_server();
            if (mic_buffer != nullptr) {
                delete mic_buffer;
            }
            return SSH_PACKET_USED;
        }
        uint32_t mic_token_len = packet->in_uint32_be();
        if (mic_token_len > packet->in_remain()) {
            this->ssh_auth_reply_denied_server(false);
            this->ssh_gssapi_free_server();
            if (mic_buffer != nullptr) {
                delete mic_buffer;
            }
            return SSH_PACKET_USED;
        }
        std::vector<uint8_t> mic_token;
        mic_token.resize(mic_token_len);
        packet->buffer_get_data(&mic_token[0],mic_token_len);


        if (this->gssapi == nullptr
        || this->gssapi->state != SSH_GSSAPI_STATE_RCV_MIC) {
            ssh_set_error(this->error,  SSH_FATAL, "Received SSH_MSG_USERAUTH_GSSAPI_MIC in invalid state");
            this->ssh_auth_reply_denied_server(false);
            this->ssh_gssapi_free_server();
            if (mic_buffer != nullptr) {
                delete mic_buffer;
            }
            return SSH_PACKET_USED;
        }
        mic_buffer = new ssh_buffer_struct;
        if (this->ssh_gssapi_build_mic_server(mic_buffer) == nullptr){
            // TODO : check memory allocation
            this->ssh_auth_reply_denied_server(false);
            this->ssh_gssapi_free_server();
            if (mic_buffer != nullptr) {
                delete mic_buffer;
            }
            return SSH_PACKET_USED;
        }
        mic_buf.length = mic_buffer->in_remain();
        mic_buf.value = mic_buffer->get_pos_ptr();
        mic_token_buf.length = mic_token.size();
        mic_token_buf.value = &mic_token[0];

        maj_stat = gss_verify_mic(&min_stat, this->gssapi->ctx, &mic_buf, &mic_token_buf, nullptr);

        {
            gss_buffer_desc buffer;
            OM_uint32 dummy;
            OM_uint32 message_context;
            gss_display_status(&dummy, maj_stat,GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
            syslog(LOG_INFO, "GSSAPI(%s): %s", "verifying MIC", static_cast<const char *>(buffer.value));
        }

        {
            gss_buffer_desc buffer;
            OM_uint32 dummy;
            OM_uint32 message_context;
            gss_display_status(&dummy, min_stat,GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
            syslog(LOG_INFO, "GSSAPI(%s): %s", "verifying MIC (min stat)", static_cast<const char *>(buffer.value));
        }

        if (maj_stat == GSS_S_DEFECTIVE_TOKEN || GSS_ERROR(maj_stat)) {
            this->ssh_auth_reply_denied_server(false);
            this->ssh_gssapi_free_server();
            if (mic_buffer != nullptr) {
                delete mic_buffer;
            }
            return SSH_PACKET_USED;
        }

        // TODO: callbacks are now mandatory
        if (this->server_callbacks->auth_gssapi_mic_server_cb){
            switch(this->server_callbacks->auth_gssapi_mic_server_cb(
                        this->gssapi->user,
                        this->gssapi->canonic_user,
                        this->server_callbacks->userdata)){
                // TODO: see why this was missing
                case SSH_AUTH_SUCCESS:
                    // TODO: check this, auth_state was initially set to SSH_SESSION_STATE_AUTHENTICATED
                    this->auth_state = SSH_AUTH_STATE_SUCCESS;
                    this->session_state = SSH_SESSION_STATE_AUTHENTICATED;
                    this->flags |= SSH_SESSION_FLAG_AUTHENTICATED;

                    syslog(LOG_INFO,"auth_gssapi_mic_server_cb -> SSH_AUTH_SUCCESS");
                    this->out_buffer->out_uint8(SSH_MSG_USERAUTH_SUCCESS);
                    this->packet_send();
                    break;
                case SSH_AUTH_PARTIAL:
                    syslog(LOG_INFO,"auth_gssapi_mic_server_cb -> SSH_AUTH_PARTIAL");
                    this->ssh_auth_reply_denied_server(true);
                    break;
                default:
                    syslog(LOG_INFO,"auth_gssapi_mic_server_cb -> SSH_AUTH_DENIED");
                    this->ssh_auth_reply_denied_server(false);
                    break;
            }
        }

        this->ssh_gssapi_free_server();
        if (mic_buffer != nullptr) {
            delete mic_buffer;
        }
        return SSH_PACKET_USED;
    }



    ssh_buffer_struct* ssh_gssapi_build_mic_server(ssh_buffer_struct * mic_buffer)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        std::vector<uint8_t> str;
        str.resize(static_cast<uint32_t>(this->current_crypto->digest_len));
        memcpy(&str[0], this->session_id, this->current_crypto->digest_len);

        mic_buffer->out_uint32_be(str.size());
        mic_buffer->out_blob(&str[0], str.size());

        mic_buffer->out_uint8(SSH_MSG_USERAUTH_REQUEST);
        mic_buffer->out_length_prefixed_cstr(this->gssapi->user);
        mic_buffer->out_length_prefixed_cstr("ssh-connection");
        mic_buffer->out_length_prefixed_cstr("gssapi-with-mic");

        return mic_buffer;
    }

    int ssh_packet_userauth_gssapi_token_server(ssh_buffer_struct* packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        OM_uint32 maj_stat, min_stat;
        gss_buffer_desc input_token, output_token = GSS_C_EMPTY_BUFFER;
        gss_name_t client_name = GSS_C_NO_NAME;
        OM_uint32 ret_flags=0;
        gss_channel_bindings_t input_bindings=GSS_C_NO_CHANNEL_BINDINGS;

        syslog(LOG_INFO,"Received SSH_MSG_USERAUTH_GSSAPI_TOKEN");
        if (!this->gssapi || this->gssapi->state != SSH_GSSAPI_STATE_RCV_TOKEN){
            ssh_set_error(this->error,  SSH_FATAL, "Received SSH_MSG_USERAUTH_GSSAPI_TOKEN in invalid state");
            return SSH_PACKET_USED;
        }

        if (sizeof(uint32_t) > packet->in_remain()) {
            this->ssh_auth_reply_denied_server(false);
            this->ssh_gssapi_free_server();
            this->gssapi=nullptr;
            return SSH_PACKET_USED;
        }
        uint32_t token_len = packet->in_uint32_be();
        if (token_len > packet->in_remain()) {
            this->ssh_auth_reply_denied_server(false);
            this->ssh_gssapi_free_server();
            this->gssapi=nullptr;
            return SSH_PACKET_USED;
        }
        std::vector<uint8_t> token;
        token.resize(token_len);
        packet->buffer_get_data(&token[0],token_len);

        char *hexa = new char[token_len * 3 + 1];
        size_t q = 0;
        size_t j = 0;
        for (q = 0; q < token_len; q++) {
            const uint8_t cl = token[q] >> 4;
            const uint8_t ch = token[q] & 0x0F;
            hexa[j] = (ch < 10?'0':'a')+ch;
            hexa[j+1] = (cl < 10?'0':'a')+cl;
            hexa[j+2] = ':';
            j+= 3;
        }
        hexa[j>0?(j-1):0] = 0;
        syslog(LOG_INFO, "GSSAPI Token : %s",hexa);
        delete[] hexa;

        input_token.length = token.size();
        input_token.value = &token[0];

        maj_stat = gss_accept_sec_context(&min_stat, &this->gssapi->ctx, this->gssapi->server_creds,
                &input_token, input_bindings, &client_name, nullptr /*mech_oid*/, &output_token, &ret_flags,
                nullptr /*time*/, &this->gssapi->client_creds);

        gss_buffer_desc buffer;
        OM_uint32 dummy;
        OM_uint32 message_context;
        gss_display_status(&dummy, maj_stat,GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
        syslog(LOG_INFO, "GSSAPI(%s): %s", "accepting token", static_cast<const char *>(buffer.value));

        if (client_name != GSS_C_NO_NAME){
            this->gssapi->client_name = client_name;
            this->gssapi->canonic_user = ssh_gssapi_name_to_char(client_name);
        }
        if (GSS_ERROR(maj_stat)){
            gss_buffer_desc buffer;
            OM_uint32 dummy;
            OM_uint32 message_context;
            gss_display_status(&dummy, maj_stat,GSS_C_GSS_CODE, GSS_C_NO_OID, &message_context, &buffer);
            syslog(LOG_INFO, "GSSAPI(%s): %s", "Gssapi error", static_cast<const char *>(buffer.value));

            this->ssh_auth_reply_denied_server(false);
            this->ssh_gssapi_free_server();
            this->gssapi=nullptr;
            return SSH_PACKET_USED;
        }

        if (output_token.length != 0){
            char *hexa = new char[output_token.length * 3 + 1];
            size_t q = 0;
            size_t j = 0;
            for (q = 0; q < output_token.length; q++) {
                const uint8_t cl = reinterpret_cast<uint8_t *>(output_token.value)[q] >> 4;
                const uint8_t ch = reinterpret_cast<uint8_t *>(output_token.value)[q] & 0x0F;
                hexa[j] = (ch < 10?'0':'a')+ch;
                hexa[j+1] = (cl < 10?'0':'a')+cl;
                hexa[j+2] = ':';
                j+= 3;
            }
            hexa[j>0?(j-1):0] = 0;
            syslog(LOG_INFO, "GSSAPI: sending token %s",hexa);
            delete[] hexa;

            std::vector<uint8_t> token2;
            token2.resize(static_cast<uint32_t>(output_token.length));
            memcpy(&token2[0], output_token.value, output_token.length);
            this->out_buffer->out_uint8(SSH_MSG_USERAUTH_GSSAPI_TOKEN);
            this->out_buffer->out_uint32_be(token2.size());
            this->out_buffer->out_blob(&token2[0], token2.size());
            this->packet_send();
        }
        if(maj_stat == GSS_S_COMPLETE){
            this->gssapi->state = SSH_GSSAPI_STATE_RCV_MIC;
        }
        return SSH_PACKET_USED;
    }

    int ssh_packet_global_request_server(ssh_buffer_struct* packet)
    {
        // SSH_REQUEST_GLOBAL
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        int rc = SSH_PACKET_USED;

        SSHString request = packet->in_strdup_cstr();
        uint8_t want_reply = packet->in_uint8();

        if (strcmp(&request[0], "tcpip-forward") == 0) {
            SSHString bind_address = packet->in_strdup_cstr();
            uint32_t bind_port = packet->in_uint32_be();

            syslog(LOG_INFO, "Received SSH_MSG_GLOBAL_REQUEST %s %d %s:%d",
                "SSH_GLOBAL_REQUEST_TCPIP_FORWARD",
                want_reply, &bind_address[0], bind_port);
            this->server_callbacks->global_request_server_cb(this,
                                            SSH_GLOBAL_REQUEST_TCPIP_FORWARD,
                                            want_reply,
                                            &bind_address[0],
                                            bind_port,
                                            this->server_callbacks->userdata);

            syslog(LOG_INFO, "Accepting a global request");
            if (want_reply) {
                this->out_buffer->out_uint8(SSH_MSG_REQUEST_SUCCESS);
                if(bind_port == 0) {
                    // TODO: see what is supposed to be put here, certainly not 0
                    uint32_t bound_port = 0;
                    this->out_buffer->out_uint32_be(bound_port);
                }
                this->packet_send();
            }
            else {
                syslog(LOG_INFO, "No ack asked");
                if(bind_port == 0) {
                    syslog(LOG_INFO, "The client doesn't want to know the remote port!");
                }
            }
        } else if (strcmp(&request[0], "cancel-tcpip-forward") == 0) {
            SSHString bind_address = packet->in_strdup_cstr();
            uint32_t bind_port = packet->in_uint32_be();

            syslog(LOG_INFO, "Received SSH_MSG_GLOBAL_REQUEST %s %d %s:%d",
                "SSH_GLOBAL_REQUEST_CANCEL_TCPIP_FORWARD",
                want_reply, &bind_address[0], bind_port);

            this->server_callbacks->global_request_server_cb(this,
                SSH_GLOBAL_REQUEST_CANCEL_TCPIP_FORWARD,
                want_reply, &bind_address[0], bind_port, this->server_callbacks->userdata);
            if (want_reply) {
                this->out_buffer->out_uint8(SSH_MSG_REQUEST_SUCCESS);
                this->packet_send();
            }
            else {
                syslog(LOG_INFO, "No ack asked");
            }
        } else {
            syslog(LOG_INFO, "UNKNOWN SSH_MSG_GLOBAL_REQUEST %s %d", &request[0], want_reply);
            if (want_reply) {
                this->out_buffer->out_uint8(SSH_MSG_REQUEST_FAILURE);
                this->packet_send();
            }
            return SSH_PACKET_NOT_USED;
        }
        return rc;
    }



    void handle_ssh_packet_unimplemented_server(ssh_buffer_struct* packet, error_struct & error)
    {
        (void)packet;
        (void)error;
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        uint32_t seq = this->in_buffer->in_uint32_be();
        syslog(LOG_ERR, "Received SSH_MSG_UNIMPLEMENTED (sequence number %d)", seq);
    }

    void handle_ssh_packet_disconnect_server(ssh_buffer_struct* packet, error_struct & error)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        uint32_t code = packet->in_uint32_be();
        // TODO: error should be read directly from stream
        // and should be filtered to avoid some attacks
        SSHString tmp_error = packet->in_strdup_cstr();
        syslog(LOG_INFO, "Received SSH_MSG_DISCONNECT %d", code);
        ssh_set_error(error,  SSH_FATAL, "Received SSH_MSG_DISCONNECT: %d (%s)", code, &tmp_error[0]);
        this->socket->close();
        this->session_state = SSH_SESSION_STATE_ERROR;
        /* TODO: handle a graceful disconnect */
    }


    int handle_ssh_packet_service_request_server(ssh_buffer_struct* packet, error_struct & error)
    {
        (void)error;
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        // SSH_REQUEST_SERVICE
        SSHString service = packet->in_strdup_cstr();
        syslog(LOG_INFO, "Received a SERVICE_REQUEST for service %s", &service[0]);
        int rc = 0;
        if (this->server_callbacks->service_request_server_cb){
            rc = this->server_callbacks->service_request_server_cb(
                    &service[0],
                    this->server_callbacks->userdata);
            if (rc != 0) {
                ssh_disconnect_server(this);
            }
        }

        this->ssh_send_service_accept_server(&service[0]);
        return SSH_PACKET_USED;
    }


    // cf to [RFC4253] 10.  Service Request
    // ====================================

    // ...

    //   If the server supports the service (and permits the client to use
    //   it), it MUST respond with the following:

    //      byte      SSH_MSG_SERVICE_ACCEPT
    //      string    service name

    // ...

    int ssh_send_service_accept_server(const char *service)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        /* The only return code accepted by specifications are success or disconnect */
        syslog(LOG_INFO, "Sending a SERVICE_ACCEPT for service %s", service);
        this->out_buffer->out_uint8(SSH_MSG_SERVICE_ACCEPT);
        this->out_buffer->out_length_prefixed_cstr(service);
        return this->packet_send();
    }


    int handle_ssh_packet_kexinit_server(ssh_buffer_struct* packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        switch (this->session_state){
        case SSH_SESSION_STATE_AUTHENTICATED:
            syslog(LOG_WARNING, "Other side initiating key re-exchange");
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_SESSION_STATE_INITIAL_KEX:
            break;
        case SSH_SESSION_STATE_NONE:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_SESSION_STATE_CONNECTING:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_SESSION_STATE_SOCKET_CONNECTED:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_SESSION_STATE_BANNER_RECEIVED:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_SESSION_STATE_KEXINIT_RECEIVED:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_SESSION_STATE_DH:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_SESSION_STATE_AUTHENTICATING:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_SESSION_STATE_ERROR:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_SESSION_STATE_DISCONNECTED:
        REDEMPTION_CXX_FALLTHROUGH;
        default:
            ssh_set_error(this->error,  SSH_FATAL,"SSH_KEXINIT received in wrong state");
            this->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }

        if (packet->buffer_get_data(this->next_crypto->client_kex.cookie, 16) != 16) {
            ssh_set_error(this->error, SSH_FATAL, "ssh_packet_kexinit_server: no cookie in packet");
            this->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }

        syslog(LOG_INFO,"Initialising client methods");

        this->next_crypto->client_kex.methods[SSH_KEX] = packet->in_strdup_cstr();
        this->next_crypto->client_kex.methods[SSH_HOSTKEYS] = packet->in_strdup_cstr();

        syslog(LOG_INFO,"Client hostkeys method : %s", this->next_crypto->client_kex.methods[SSH_HOSTKEYS].c_str());

        this->next_crypto->client_kex.methods[SSH_CRYPT_C_S] = packet->in_strdup_cstr();
        this->next_crypto->client_kex.methods[SSH_CRYPT_S_C] = packet->in_strdup_cstr();
        this->next_crypto->client_kex.methods[SSH_MAC_C_S] = packet->in_strdup_cstr();
        this->next_crypto->client_kex.methods[SSH_MAC_S_C] = packet->in_strdup_cstr();
        this->next_crypto->client_kex.methods[SSH_COMP_C_S] = packet->in_strdup_cstr();
        this->next_crypto->client_kex.methods[SSH_COMP_S_C] = packet->in_strdup_cstr();
        this->next_crypto->client_kex.methods[SSH_LANG_S_C] = packet->in_strdup_cstr();
        this->next_crypto->client_kex.methods[SSH_LANG_C_S] = packet->in_strdup_cstr();

        this->session_state = SSH_SESSION_STATE_KEXINIT_RECEIVED;
        this->dh_handshake_state = DH_STATE_INIT;
        this->server_callbacks->connect_status_server_cb(this->server_callbacks->userdata, 0.6f);


        if (this->next_crypto->server_kex.methods[0].size() == 0){
            syslog(LOG_INFO,"Initialising server methods");

            this->next_crypto->server_kex.methods[SSH_KEX] = "curve25519-sha256@libssh.org,ecdh-sha2-nistp256,diffie-hellman-group14-sha1,diffie-hellman-group1-sha1";

            // Set server HOSTKEYS (depending on loaded keys)
            char hostkeys[64] = {0};
            size_t len = 0;

            memset(this->next_crypto->server_kex.cookie, 0, 16);
            RAND_pseudo_bytes(this->next_crypto->server_kex.cookie, 16);

            if (this->server_ecdsa_key) {
                syslog(LOG_INFO,"Server ECDSA key loaded");
                const char * key = this->server_ecdsa_key->type_c();
                int keylen = strlen(key);
                memcpy(hostkeys+len, key, keylen);
                len += keylen;
            }
            if (len > 0){
                hostkeys[len] = ',';
                len++;
            }
            if (this->server_dsa_key) {
                syslog(LOG_INFO,"Server DSA key loaded");
                const char * key = this->server_dsa_key->type_c();
                int keylen = strlen(key);
                memcpy(hostkeys+len, key, keylen);
                len += keylen;
            }
            if (len > 0){
                hostkeys[len] = ',';
                len++;
            }
            if (this->server_rsa_key) {
                syslog(LOG_INFO,"Server RSA key loaded");
                const char * key = this->server_rsa_key->type_c();
                int keylen = strlen(key);
                memcpy(hostkeys+len, key, keylen);
                len += keylen;
            }
            hostkeys[len] = 0;

            const char * supported_hostkeys = "ecdsa-sha2-nistp256,ssh-rsa,ssh-dss";

            SSHString tmp = find_matching(supported_hostkeys, hostkeys, ',');

            if (tmp.size() == 0){
                ssh_set_error(this->error, SSH_REQUEST_DENIED,
                    "Setting method: no algorithm for method \"%s\" (%s)\n",
                    "server host key algo", hostkeys);
                this->socket->close();
                this->session_state = SSH_SESSION_STATE_ERROR;
                return SSH_PACKET_USED;
            }

            free(this->opts.wanted_methods[SSH_HOSTKEYS]);
            this->opts.wanted_methods[SSH_HOSTKEYS] = strdup(hostkeys);

            this->next_crypto->server_kex.methods[SSH_HOSTKEYS] = strdup(tmp.c_str());

            this->next_crypto->server_kex.methods[SSH_CRYPT_C_S] = "aes256-ctr,aes192-ctr,aes128-ctr,aes256-cbc,aes192-cbc,aes128-cbc,blowfish-cbc,3des-cbc,des-cbc-ssh1";
            this->next_crypto->server_kex.methods[SSH_CRYPT_S_C] = "aes256-ctr,aes192-ctr,aes128-ctr,aes256-cbc,aes192-cbc,aes128-cbc,blowfish-cbc,3des-cbc,des-cbc-ssh1";
            this->next_crypto->server_kex.methods[SSH_MAC_C_S] = "hmac-sha1";
            this->next_crypto->server_kex.methods[SSH_MAC_S_C] = "hmac-sha1";
            this->next_crypto->server_kex.methods[SSH_COMP_C_S] = "none,zlib,zlib@openssh.com";
            this->next_crypto->server_kex.methods[SSH_COMP_S_C] = "none,zlib,zlib@openssh.com";
            this->next_crypto->server_kex.methods[SSH_LANG_S_C] = "";
            this->next_crypto->server_kex.methods[SSH_LANG_C_S] = "";

            syslog(LOG_INFO, "kex algos: %s",
                this->next_crypto->server_kex.methods[SSH_KEX].c_str());
            syslog(LOG_INFO, "server host key algo: %s",
                this->next_crypto->server_kex.methods[SSH_HOSTKEYS].c_str());
            syslog(LOG_INFO, "encryption client->server: %s",
                this->next_crypto->server_kex.methods[SSH_CRYPT_C_S].c_str());
            syslog(LOG_INFO, "encryption server->client: %s",
                this->next_crypto->server_kex.methods[SSH_CRYPT_S_C].c_str());
            syslog(LOG_INFO, "mac algo client->server: %s",
                this->next_crypto->server_kex.methods[SSH_MAC_C_S].c_str());
            syslog(LOG_INFO, "mac algo server->client: %s",
                this->next_crypto->server_kex.methods[SSH_MAC_S_C].c_str());
            syslog(LOG_INFO, "compression algo client->server: %s",
                this->next_crypto->server_kex.methods[SSH_COMP_C_S].c_str());
            syslog(LOG_INFO, "compression algo server->client: %s",
                this->next_crypto->server_kex.methods[SSH_COMP_S_C].c_str());
            syslog(LOG_INFO, "languages client->server: %s",
                this->next_crypto->server_kex.methods[SSH_LANG_C_S].c_str());
            syslog(LOG_INFO, "languages server->client: %s",
                this->next_crypto->server_kex.methods[SSH_LANG_S_C].c_str());



            /* We are in a rekeying, so we need to send the server kex */
            this->out_buffer->out_uint8(SSH_MSG_KEXINIT);
            this->out_buffer->out_blob(this->next_crypto->server_kex.cookie, 16);
            this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_KEX]);
            this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_HOSTKEYS]);
            this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_CRYPT_C_S]);
            this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_CRYPT_S_C]);
            this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_MAC_C_S]);
            this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_MAC_S_C]);
            this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_COMP_C_S]);
            this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_COMP_S_C]);
            this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_LANG_C_S]);
            this->out_buffer->out_sshstring(this->next_crypto->server_kex.methods[SSH_LANG_S_C]);
            this->out_buffer->out_uint8(0);
            this->out_buffer->out_uint32_be(0);

            this->packet_send();
        }
        else {
            syslog(LOG_INFO,"%s Server methods are already initialised", __FUNCTION__);

            syslog(LOG_INFO, "kex algos: %s",
                this->next_crypto->server_kex.methods[SSH_KEX].c_str());
            syslog(LOG_INFO, "server host key algo: %s",
                this->next_crypto->server_kex.methods[SSH_HOSTKEYS].c_str());
            syslog(LOG_INFO, "encryption client->server: %s",
                this->next_crypto->server_kex.methods[SSH_CRYPT_C_S].c_str());
            syslog(LOG_INFO, "encryption server->client: %s",
                this->next_crypto->server_kex.methods[SSH_CRYPT_S_C].c_str());
            syslog(LOG_INFO, "mac algo client->server: %s",
                this->next_crypto->server_kex.methods[SSH_MAC_C_S].c_str());
            syslog(LOG_INFO, "mac algo server->client: %s",
                this->next_crypto->server_kex.methods[SSH_MAC_S_C].c_str());
            syslog(LOG_INFO, "compression algo client->server: %s",
                this->next_crypto->server_kex.methods[SSH_COMP_C_S].c_str());
            syslog(LOG_INFO, "compression algo server->client: %s",
                this->next_crypto->server_kex.methods[SSH_COMP_S_C].c_str());
            syslog(LOG_INFO, "languages client->server: %s",
                this->next_crypto->server_kex.methods[SSH_LANG_C_S].c_str());
            syslog(LOG_INFO, "languages server->client: %s",
                this->next_crypto->server_kex.methods[SSH_LANG_S_C].c_str());
        }

        if (this->session_state != SSH_SESSION_STATE_ERROR){

    //        syslog(LOG_INFO, "kex algos: %s",
    //            this->next_crypto->client_kex.methods[SSH_KEX].c_str());
    //        syslog(LOG_INFO, "server host key algo: %s",
    //            this->next_crypto->client_kex.methods[SSH_HOSTKEYS].c_str());
    //        syslog(LOG_INFO, "encryption client->server: %s",
    //            this->next_crypto->client_kex.methods[SSH_CRYPT_C_S].c_str());
    //        syslog(LOG_INFO, "encryption server->client: %s",
    //            this->next_crypto->client_kex.methods[SSH_CRYPT_S_C].c_str());
    //        syslog(LOG_INFO, "mac algo client->server: %s",
    //            this->next_crypto->client_kex.methods[SSH_MAC_C_S].c_str());
    //        syslog(LOG_INFO, "mac algo server->client: %s",
    //            this->next_crypto->client_kex.methods[SSH_MAC_S_C].c_str());
    //        syslog(LOG_INFO, "compression algo client->server: %s",
    //            this->next_crypto->client_kex.methods[SSH_COMP_C_S].c_str());
    //        syslog(LOG_INFO, "compression algo server->client: %s",
    //            this->next_crypto->client_kex.methods[SSH_COMP_S_C].c_str());
    //        syslog(LOG_INFO, "languages client->server: %s",
    //            this->next_crypto->client_kex.methods[SSH_LANG_C_S].c_str());
    //        syslog(LOG_INFO, "languages server->client: %s",
    //            this->next_crypto->client_kex.methods[SSH_LANG_S_C].c_str());

            this->next_crypto->kex_methods[SSH_KEX] = find_matching(
                this->next_crypto->server_kex.methods[SSH_KEX].c_str(),
                this->next_crypto->client_kex.methods[SSH_KEX].c_str(),
                ',');

            if (this->next_crypto->kex_methods[SSH_KEX].size() == 0){
                ssh_set_error(this->error, SSH_FATAL,
                    "kex error : no match for method %s: server [%s], client [%s]",
                    "kex algos",
                    this->next_crypto->server_kex.methods[SSH_KEX].c_str(),
                    this->next_crypto->client_kex.methods[SSH_KEX].c_str());
                this->socket->close();
                this->session_state = SSH_SESSION_STATE_ERROR;
                return SSH_PACKET_USED;
            }

            // TODO: create a general purpose method to find a value in a string keyed dictionnary
            // and use it there and whenever necessary
            if(strcmp(this->next_crypto->kex_methods[SSH_KEX].c_str(), "diffie-hellman-group1-sha1") == 0){
              this->next_crypto->kex_type = SSH_KEX_DH_GROUP1_SHA1;
            }
            else if(strcmp(this->next_crypto->kex_methods[SSH_KEX].c_str(), "diffie-hellman-group14-sha1") == 0)
            {
              this->next_crypto->kex_type = SSH_KEX_DH_GROUP14_SHA1;
            }
            else if(strcmp(this->next_crypto->kex_methods[SSH_KEX].c_str(), "ecdh-sha2-nistp256") == 0){
              this->next_crypto->kex_type = SSH_KEX_ECDH_SHA2_NISTP256;
            }
            else if(strcmp(this->next_crypto->kex_methods[SSH_KEX].c_str(), "curve25519-sha256@libssh.org") == 0)
            {
              this->next_crypto->kex_type = SSH_KEX_CURVE25519_SHA256_LIBSSH_ORG;
            }

            this->next_crypto->kex_methods[SSH_HOSTKEYS] = find_matching(
                this->next_crypto->server_kex.methods[SSH_HOSTKEYS].c_str(),
                this->next_crypto->client_kex.methods[SSH_HOSTKEYS].c_str(),
                ',');

            syslog(LOG_INFO,"%s [A] Setting HOSKTEYS: server=%s client=%s -> methods=%s",
                __FUNCTION__,
                this->next_crypto->server_kex.methods[SSH_HOSTKEYS].c_str(),
                this->next_crypto->client_kex.methods[SSH_HOSTKEYS].c_str(),
                this->next_crypto->kex_methods[SSH_HOSTKEYS].c_str()
            );


            if (this->next_crypto->kex_methods[SSH_HOSTKEYS].size() == 0){
                ssh_set_error(this->error, SSH_FATAL,
                    "kex error : no match for method %s: server [%s], client [%s]",
                    "server host key algo",
                    this->next_crypto->server_kex.methods[SSH_HOSTKEYS].c_str(),
                    this->next_crypto->client_kex.methods[SSH_HOSTKEYS].c_str());
                this->socket->close();
                this->session_state = SSH_SESSION_STATE_ERROR;
                return SSH_PACKET_USED;
            }

            this->next_crypto->kex_methods[SSH_CRYPT_C_S] = find_matching(
                    this->next_crypto->server_kex.methods[SSH_CRYPT_C_S].c_str(),
                    this->next_crypto->client_kex.methods[SSH_CRYPT_C_S].c_str(),
                    ',');

            if (this->next_crypto->kex_methods[SSH_CRYPT_C_S].size() == 0){
                ssh_set_error(this->error, SSH_FATAL,
                    "kex error : no match for method %s: server [%s], client [%s]",
                    "encryption client->server",
                    this->next_crypto->server_kex.methods[SSH_CRYPT_C_S].c_str(),
                    this->next_crypto->client_kex.methods[SSH_CRYPT_C_S].c_str());
                this->socket->close();
                this->session_state = SSH_SESSION_STATE_ERROR;
                return SSH_PACKET_USED;
            }
            this->next_crypto->kex_methods[SSH_CRYPT_S_C] = find_matching(
                this->next_crypto->server_kex.methods[SSH_CRYPT_S_C].c_str(),
                this->next_crypto->client_kex.methods[SSH_CRYPT_S_C].c_str(),
                ',');

        if (this->next_crypto->kex_methods[SSH_CRYPT_S_C].size() == 0){
            ssh_set_error(this->error, SSH_FATAL,
                "kex error : no match for method %s: server [%s], client [%s]",
                "encryption server->client",
                this->next_crypto->server_kex.methods[SSH_CRYPT_S_C].c_str(),
                this->next_crypto->client_kex.methods[SSH_CRYPT_S_C].c_str());
            this->socket->close();
            this->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }

        this->next_crypto->kex_methods[SSH_MAC_C_S] = find_matching(
            this->next_crypto->server_kex.methods[SSH_MAC_C_S].c_str(),
            this->next_crypto->client_kex.methods[SSH_MAC_C_S].c_str(),
            ',');

        if (this->next_crypto->kex_methods[SSH_MAC_C_S].size() == 0){
            ssh_set_error(this->error, SSH_FATAL,
                "kex error : no match for method %s: server [%s], client [%s]",
                "compression algo client->server",
                this->next_crypto->server_kex.methods[SSH_MAC_C_S].c_str(),
                this->next_crypto->client_kex.methods[SSH_MAC_C_S].c_str());
            this->socket->close();
            this->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }

        this->next_crypto->kex_methods[SSH_MAC_S_C] = find_matching(
            this->next_crypto->server_kex.methods[SSH_MAC_S_C].c_str(),
            this->next_crypto->client_kex.methods[SSH_MAC_S_C].c_str(),
            ',');

        if (this->next_crypto->kex_methods[SSH_MAC_S_C].size() == 0){
            ssh_set_error(this->error, SSH_FATAL,
                "kex error : no match for method %s: server [%s], client [%s]",
                "compression algo server->client",
                this->next_crypto->server_kex.methods[SSH_MAC_S_C].c_str(),
                this->next_crypto->client_kex.methods[SSH_MAC_S_C].c_str());
            this->socket->close();
            this->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        this->next_crypto->kex_methods[SSH_COMP_C_S] = find_matching(
            this->next_crypto->server_kex.methods[SSH_COMP_C_S].c_str(),
            this->next_crypto->client_kex.methods[SSH_COMP_C_S].c_str(),
            ',');

        if (this->next_crypto->kex_methods[SSH_COMP_C_S].size() == 0){
            ssh_set_error(this->error,SSH_FATAL,
                "kex error : no match for method %s: server [%s], client [%s]",
                "mac algo client->server",
                this->next_crypto->server_kex.methods[SSH_COMP_C_S].c_str(),
                this->next_crypto->client_kex.methods[SSH_COMP_C_S].c_str());
            this->socket->close();
            this->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        this->next_crypto->kex_methods[SSH_COMP_S_C] = find_matching(
            this->next_crypto->server_kex.methods[SSH_COMP_S_C].c_str(),
            this->next_crypto->client_kex.methods[SSH_COMP_S_C].c_str(),
            ',');

        if (this->next_crypto->kex_methods[SSH_COMP_S_C].size() == 0){
            ssh_set_error(this->error,SSH_FATAL,"kex error : no match for method %s: server [%s], client [%s]",
                "mac algo server->client",
                this->next_crypto->server_kex.methods[SSH_COMP_S_C].c_str(),
                this->next_crypto->client_kex.methods[SSH_COMP_S_C].c_str());
            this->socket->close();
            this->session_state = SSH_SESSION_STATE_ERROR;
            return SSH_PACKET_USED;
        }
        this->next_crypto->kex_methods[SSH_LANG_C_S] = find_matching(
            this->next_crypto->server_kex.methods[SSH_LANG_C_S].c_str(),
            this->next_crypto->client_kex.methods[SSH_LANG_C_S].c_str(),
            ',');

        this->next_crypto->kex_methods[SSH_LANG_S_C] = find_matching(
            this->next_crypto->server_kex.methods[SSH_LANG_S_C].c_str(),
            this->next_crypto->client_kex.methods[SSH_LANG_S_C].c_str(),
            ',');


            this->next_crypto->out_cipher = cipher_new_by_name(this->next_crypto->kex_methods[SSH_CRYPT_S_C]);
            this->next_crypto->in_cipher = cipher_new_by_name(this->next_crypto->kex_methods[SSH_CRYPT_C_S]);

            const char * method_in = this->next_crypto->kex_methods[SSH_COMP_C_S].c_str();
            if(strcmp(method_in,"zlib") == 0){
                syslog(LOG_INFO,"enabling C->S compression");
                this->next_crypto->do_compress_in=1;
            }

            if(strcmp(method_in,"zlib@openssh.com") == 0){
                syslog(LOG_INFO,"enabling C->S delayed compression");
                if (this->flags & SSH_SESSION_FLAG_AUTHENTICATED) {
                    this->next_crypto->do_compress_in = 1;
                } else {
                    this->next_crypto->delayed_compress_in = 1;
                }
            }

            const char * method_out = this->next_crypto->kex_methods[SSH_COMP_S_C].c_str();
            if(strcmp(method_out,"zlib") == 0){
                syslog(LOG_INFO, "enabling S->C compression\n");
                this->next_crypto->do_compress_out=1;
            }
            if(strcmp(method_out,"zlib@openssh.com") == 0){
                syslog(LOG_INFO,"enabling S->C delayed compression\n");
                if (this->flags & SSH_SESSION_FLAG_AUTHENTICATED) {
                    this->next_crypto->do_compress_out = 1;
                }
                else {
                    this->next_crypto->delayed_compress_out = 1;
                }
            }

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

            syslog(LOG_INFO,"%s Finding hostkey type in = %s", __FUNCTION__, this->next_crypto->kex_methods[SSH_HOSTKEYS].c_str());
            this->server_negociated_hostkey = SSH_KEYTYPE_UNKNOWN;
            for(auto &p:l){
                if (strcmp(p.first, this->next_crypto->kex_methods[SSH_HOSTKEYS].c_str()) == 0){
                    syslog(LOG_INFO,"%s negociated hostkey = %s %d", __FUNCTION__, p.first, p.second);
                    this->server_negociated_hostkey = p.second;
                    break;
                }
            }

            this->server_callbacks->connect_status_server_cb(this->server_callbacks->userdata, 0.8f);
            this->session_state = SSH_SESSION_STATE_DH;
        }
        return SSH_PACKET_USED;
    }


    int handle_channel_open_server(ssh_buffer_struct* packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        SSHString type_c = packet->in_strdup_cstr();
        syslog(LOG_INFO, "%s --- channel type = %s", __FUNCTION__, &type_c[0]);
        uint8_t channel_open_code = get_channel_open_request_code(&type_c[0]);
        switch (channel_open_code) {
        case REQUEST_STRING_CHANNEL_OPEN_SESSION:
            syslog(LOG_INFO, "%s --- REQUEST_CHANNEL_OPEN_SESSION", __FUNCTION__);
            this->handle_channel_open_session_request_server(packet);
        break;
        case REQUEST_STRING_CHANNEL_OPEN_DIRECT_TCPIP:
            syslog(LOG_INFO, "%s --- REQUEST_CHANNEL_OPEN_DIRECT_TCPIP", __FUNCTION__);
            this->handle_channel_open_direct_tcpip_request_server(packet);
        break;
        case REQUEST_STRING_CHANNEL_OPEN_FORWARDED_TCPIP:
            syslog(LOG_INFO, "%s --- REQUEST_CHANNEL_OPEN_FORWARDED_TCPIP", __FUNCTION__);
            this->handle_channel_open_forwarded_tcpip_request_server(packet);
        break;
        case REQUEST_STRING_CHANNEL_OPEN_X11:
            syslog(LOG_INFO, "%s --- REQUEST_CHANNEL_OPEN_X11", __FUNCTION__);
            this->handle_channel_open_x11_request_server(packet);
        break;
        case REQUEST_STRING_CHANNEL_OPEN_AUTH_AGENT:
            syslog(LOG_INFO, "%s --- REQUEST_CHANNEL_OPEN_AUTH_AGENT", __FUNCTION__);
            this->handle_channel_open_auth_agent_request_server(packet);
        break;
        case REQUEST_STRING_CHANNEL_OPEN_UNKNOWN:
            syslog(LOG_INFO, "%s --- REQUEST_CHANNEL_OPEN_UNKNOWN", __FUNCTION__);
            REDEMPTION_CXX_FALLTHROUGH;
        default:
        break;
        }
        return SSH_PACKET_USED;
    }


    void handle_channel_open_session_request_server(ssh_buffer_struct * packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        // SSH_REQUEST_CHANNEL_OPEN SSH_CHANNEL_SESSION;
        syslog(LOG_INFO, "Clients wants to open a %s channel", "session");
        uint32_t sender = packet->in_uint32_be();
        uint32_t window = packet->in_uint32_be();
        uint32_t packet_size = packet->in_uint32_be();
        ssh_channel channel = this->server_callbacks->channel_open_request_session_server_cb(
            this->server_callbacks->userdata);
        if (channel != nullptr) {
            syslog(LOG_INFO, "Accepting a channel request_open for chan %d", channel->remote_channel);
            channel->local_channel = this->new_channel_id();
            channel->local_window = 32000;
            channel->local_maxpacket = CHANNEL_TOTAL_PACKET_SIZE;

            channel->remote_channel = sender;
            channel->remote_window = window;
            channel->remote_maxpacket = packet_size;
            channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN;

            this->out_buffer->out_uint8(SSH_MSG_CHANNEL_OPEN_CONFIRMATION);
            this->out_buffer->out_uint32_be(sender);
            this->out_buffer->out_uint32_be(channel->local_channel);
            this->out_buffer->out_uint32_be(channel->local_window);
            this->out_buffer->out_uint32_be(channel->local_maxpacket);
        }
        else {
            syslog(LOG_INFO, "Refusing a channel");

            this->out_buffer->out_uint8(SSH_MSG_CHANNEL_OPEN_FAILURE);
            this->out_buffer->out_uint32_be(sender);
            this->out_buffer->out_uint32_be(SSH2_OPEN_ADMINISTRATIVELY_PROHIBITED);
            this->out_buffer->out_uint32_be(0); /* reason is an empty string */
            this->out_buffer->out_uint32_be(0); /* language too */
        }
        this->packet_send();
    }


    void handle_channel_open_direct_tcpip_request_server(ssh_buffer_struct * packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        // SSH_REQUEST_CHANNEL_OPEN SSH_CHANNEL_DIRECT_TCPIP
        syslog(LOG_INFO, "Clients wants to open a %s channel", "direct-tcpip");
        uint32_t sender = packet->in_uint32_be();
        uint32_t window = packet->in_uint32_be();
        uint32_t packet_size = packet->in_uint32_be();
        SSHString destination = packet->in_strdup_cstr();
        uint32_t destination_port = packet->in_uint32_be();
        SSHString originator = packet->in_strdup_cstr();
        uint32_t originator_port = packet->in_uint32_be();

        // Why leave other side of callback create channel ?
        // We initialize all relevant channel informations here anyway
        ssh_channel channel = this->server_callbacks->channel_open_request_direct_tcpip_server_cb(
                            &destination[0], destination_port,
                            &originator[0], originator_port,
                            this->server_callbacks->userdata);
        if (channel != nullptr) {
            channel->local_channel = this->new_channel_id();
            channel->local_maxpacket = CHANNEL_TOTAL_PACKET_SIZE;
            channel->local_window = 32000;
            channel->remote_channel = sender;
            channel->remote_maxpacket = packet_size;
            channel->remote_window = window;
            channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN;

            this->out_buffer->out_uint8(SSH_MSG_CHANNEL_OPEN_CONFIRMATION);
            this->out_buffer->out_uint32_be(channel->remote_channel);
            this->out_buffer->out_uint32_be(channel->local_channel);
            this->out_buffer->out_uint32_be(channel->local_window);
            this->out_buffer->out_uint32_be(channel->local_maxpacket);
        }
        else {
            this->out_buffer->out_uint8(SSH_MSG_CHANNEL_OPEN_FAILURE);
            this->out_buffer->out_uint32_be(sender);
            this->out_buffer->out_uint32_be(SSH2_OPEN_ADMINISTRATIVELY_PROHIBITED);
            this->out_buffer->out_uint32_be(0); /* reason is an empty string */
            this->out_buffer->out_uint32_be(0); /* language too */
        }
        this->packet_send();
    }

    void handle_channel_open_forwarded_tcpip_request_server(ssh_buffer_struct * packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        // SSH_REQUEST_CHANNEL_OPEN SSH_CHANNEL_FORWARDED_TCPIP
        syslog(LOG_INFO, "Clients wants to open a %s channel", "forwarded-tcpip");
        uint32_t sender = packet->in_uint32_be();
        uint32_t window = packet->in_uint32_be();
        uint32_t packet_size = packet->in_uint32_be();
        SSHString destination = packet->in_strdup_cstr();
        uint32_t destination_port = packet->in_uint32_be();
        SSHString originator = packet->in_strdup_cstr();
        uint32_t originator_port = packet->in_uint32_be();
        ssh_channel channel = this->server_callbacks->channel_open_request_forwarded_tcpip_function(
                        this,
                        &destination[0],
                        destination_port,
                        &originator[0],
                        originator_port,
                        this->server_callbacks->userdata);
        if (channel != nullptr) {
            channel->local_channel = this->new_channel_id();
            channel->local_window = 32000;
            channel->local_maxpacket = CHANNEL_TOTAL_PACKET_SIZE;
            channel->remote_channel = sender;
            channel->remote_window = window;
            channel->remote_maxpacket = packet_size;
            channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN;

            this->out_buffer->out_uint8(SSH_MSG_CHANNEL_OPEN_CONFIRMATION);
            this->out_buffer->out_uint32_be(sender);
            this->out_buffer->out_uint32_be(channel->local_channel);
            this->out_buffer->out_uint32_be(channel->local_window);
            this->out_buffer->out_uint32_be(channel->local_maxpacket);
        }
        else {
            this->out_buffer->out_uint8(SSH_MSG_CHANNEL_OPEN_FAILURE);
            this->out_buffer->out_uint32_be(sender);
            this->out_buffer->out_uint32_be(SSH2_OPEN_ADMINISTRATIVELY_PROHIBITED);
            this->out_buffer->out_uint32_be(0); /* reason is an empty string */
            this->out_buffer->out_uint32_be(0); /* language too */
        }
        this->packet_send();
    }


    void handle_channel_open_x11_request_server(ssh_buffer_struct * packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        // SSH_REQUEST_CHANNEL_OPEN SSH_CHANNEL_X11
        syslog(LOG_INFO, "Clients wants to open a %s channel", "x11");
        /* uint32_t sender = */          (void)packet->in_uint32_be();
        /* uint32_t window = */          (void)packet->in_uint32_be();
        /* uint32_t packet_size = */     (void)packet->in_uint32_be();
        SSHString originator =  packet->in_strdup_cstr();
        /* uint32_t originator_port = */ (void)packet->in_uint32_be();
        syslog(LOG_INFO, "No python callback to x11");
    }


    void handle_channel_open_auth_agent_request_server(ssh_buffer_struct * packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        // SSH_REQUEST_CHANNEL_OPEN SSH_CHANNEL_X11
        syslog(LOG_INFO, "Client wants to open a %s channel", "auth-agent@openssh.com");
        /* uint32_t sender = */      (void)packet->in_uint32_be();
        /* uint32_t window = */      (void)packet->in_uint32_be();
        /* uint32_t packet_size = */ (void)packet->in_uint32_be();
        syslog(LOG_INFO, "No python callback to auth_agent on server side");
    }



    int ssh2_msg_channel_open_confirmation_server(ssh_buffer_struct* packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        syslog(LOG_WARNING,"Received SSH_MSG_CHANNEL_OPEN_CONFIRMATION");

        uint32_t chan = packet->in_uint32_be();
        ssh_channel channel = this->ssh_channel_from_local(chan);
        if(channel==nullptr){
            ssh_set_error(this->error,  SSH_FATAL,
                          "Unknown channel id %lu",
                          static_cast<long unsigned int>(chan));
            /* TODO: Set error marking in channel object */

            return SSH_PACKET_USED;
        }

        channel->remote_channel = packet->in_uint32_be();
        channel->remote_window = packet->in_uint32_be();
        channel->remote_maxpacket = packet->in_uint32_be();

        syslog(LOG_INFO,
                "Received a CHANNEL_OPEN_CONFIRMATION for channel %d:%d",
                channel->local_channel,
                channel->remote_channel);
        syslog(LOG_INFO,
                "Remote window : %lu, maxpacket : %lu",
                static_cast<long unsigned int>(channel->remote_window),
                static_cast<long unsigned int>(channel->remote_maxpacket));

        channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN;
        channel->flags &= ~SSH_CHANNEL_FLAG_NOT_BOUND;

        return SSH_PACKET_USED;
    }

    int ssh_packet_channel_open_fail_server(ssh_channel_struct * channel, ssh_buffer_struct* packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        uint32_t code =  packet->in_uint32_be();

        if (sizeof(uint32_t) > packet->in_remain()) {
            // TODO: malformed, should be an error
            channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN_DENIED;
            return SSH_PACKET_USED;
        }
        uint32_t error_s_len = packet->in_uint32_be();
        if (error_s_len > packet->in_remain()) {
            // TODO: malformed, should be an error
            channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN_DENIED;
            return SSH_PACKET_USED;
        }
        SSHString error_s;
        error_s.resize(error_s_len);
        packet->buffer_get_data(&error_s[0],error_s_len);

        ssh_set_error(this->error,  SSH_REQUEST_DENIED,
                      "Channel opening failure: channel %u error (%lu) %*s",
                      channel->local_channel,
                      static_cast<long unsigned int>(ntohl(code)),
                      (int)error_s.size(), &error_s[0]);
        channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN_DENIED;
        return SSH_PACKET_USED;
    }

    int channel_rcv_change_window_server(ssh_buffer_struct* packet, error_struct & error)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        uint32_t chan = packet->in_uint32_be();
        ssh_channel channel = this->ssh_channel_from_local(chan);
        if (channel == nullptr) {
            ssh_set_error(error, SSH_FATAL, "Server specified invalid channel %lu", static_cast<long unsigned int>(ntohl(chan)));
            syslog(LOG_INFO, "%s", ssh_get_error(&error));
            syslog(LOG_INFO, "Error getting a window adjust message: invalid packet");

            return SSH_PACKET_USED;
        }
        uint32_t bytes = packet->in_uint32_be();
        channel->remote_window += bytes;

        return SSH_PACKET_USED;
    }

    int channel_rcv_data_stderr_server(ssh_buffer_struct* packet, error_struct & error)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        uint32_t chan = packet->in_uint32_be();
        ssh_channel channel = this->ssh_channel_from_local(chan);
        if (channel == nullptr) {
            ssh_set_error(error, SSH_FATAL,
                "Server specified invalid channel %lu", static_cast<long unsigned int>(ntohl(chan)));
            syslog(LOG_INFO, "%s", ssh_get_error(&error));

            return SSH_PACKET_USED;
        }

        /* uint32 data type code. we can ignore it */
        /* uint32_t ignore = */ packet->in_uint32_be();

        if (sizeof(uint32_t) > packet->in_remain()) {
            return -1;
        }
        uint32_t str_len = packet->in_uint32_be();
        if (str_len > packet->in_remain()) {
            return -1;
        }
        std::vector<uint8_t> str;
        str.resize(str_len);
        packet->buffer_get_data(&str[0], str_len);

        // TODO: see that, we read full packet then drop what the local_window can't hold
        // as this looks really like a forbidden case we should probably close the connection
        if (str_len > channel->local_window) {
            syslog(LOG_INFO,
                   "%s Data packet too big for our window(%u vs %d)",
                   __FUNCTION__,
                   str_len,
                   channel->local_window);
        }

        channel->stderr_buffer->out_blob(&str[0], str_len);

        if (str_len <= channel->local_window) {
            channel->local_window -= str_len;
        } else {
            channel->local_window = 0; /* buggy remote */
        }

        syslog(LOG_INFO,
                "Channel windows are now (local win=%d remote win=%d)",
                channel->local_window,
                channel->remote_window);

        if (channel->callbacks && channel->callbacks->channel_data_function){
            int used = channel->callbacks->channel_data_function(this,
                                                             channel,
                                                             channel->stderr_buffer->get_pos_ptr(),
                                                             channel->stderr_buffer->in_remain(),
                                                             1,
                                                             channel->callbacks->userdata);
            if(used > 0) {
                channel->stderr_buffer->in_skip_bytes(used);
            }
            if (channel->local_window + channel->stderr_buffer->in_remain() < WINDOWLIMIT) {
                if (WINDOWBASE > channel->local_window){
                    /* WINDOW_ADJUST packet needs a relative increment rather than an absolute
                     * value, so we give here the missing bytes needed to reach new_window
                     */
                    this->out_buffer->out_uint8(SSH_MSG_CHANNEL_WINDOW_ADJUST);
                    this->out_buffer->out_uint32_be(channel->remote_channel);
                    this->out_buffer->out_uint32_be(WINDOWBASE - channel->local_window);
                    this->packet_send();
                    channel->local_window = WINDOWBASE - channel->local_window ;
                }
            }
        }

        return SSH_PACKET_USED;
    }

    int channel_rcv_data_server(ssh_buffer_struct* packet, error_struct & error)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        uint32_t chan = packet->in_uint32_be();
        ssh_channel channel = this->ssh_channel_from_local(chan);
        if (channel == nullptr) {
            ssh_set_error(error, SSH_FATAL,
                "Server specified invalid channel %lu", static_cast<long unsigned int>(ntohl(chan)));
            syslog(LOG_INFO, "%s", ssh_get_error(&error));
            return SSH_PACKET_USED;
        }

        if (sizeof(uint32_t) > packet->in_remain()) {
            return -1;
        }
        uint32_t str_len = packet->in_uint32_be();
        if (str_len > packet->in_remain()) {
            return -1;
        }
        std::vector<uint8_t> str;
        str.resize(str_len);
        packet->buffer_get_data(&str[0], str_len);

        // TODO: see that, we read full packet then drop what the local_window can't hold
        // as this looks really like a forbidden case we should probably close the connection
        if (str_len > channel->local_window) {
            syslog(LOG_INFO,
                   "%s Data packet too big for our window(%u vs %d)",
                   __FUNCTION__,
                   str_len,
                   channel->local_window);
        }

        channel->stdout_buffer->out_blob(&str[0], str_len);

        if (str_len <= channel->local_window) {
            channel->local_window -= str_len;
        } else {
            channel->local_window = 0; /* buggy remote */
        }

        syslog(LOG_INFO,
                "Channel windows are now (local win=%d remote win=%d)",
                channel->local_window,
                channel->remote_window);

        if (channel->callbacks && channel->callbacks->channel_data_function){
            int used = channel->callbacks->channel_data_function(this,
                                                             channel,
                                                             channel->stdout_buffer->get_pos_ptr(),
                                                             channel->stdout_buffer->in_remain(),
                                                             0,
                                                             channel->callbacks->userdata);
            if(used > 0) {
                channel->stdout_buffer->in_skip_bytes(used);
            }
            if (channel->local_window + channel->stdout_buffer->in_remain() < WINDOWLIMIT) {
                if (WINDOWBASE > channel->local_window){
                    /* WINDOW_ADJUST packet needs a relative increment rather than an absolute
                     * value, so we give here the missing bytes needed to reach new_window
                     */
                    this->out_buffer->out_uint8(SSH_MSG_CHANNEL_WINDOW_ADJUST);
                    this->out_buffer->out_uint32_be(channel->remote_channel);
                    this->out_buffer->out_uint32_be(WINDOWBASE - channel->local_window);
                    this->packet_send();
                    channel->local_window = WINDOWBASE - channel->local_window ;
                }
            }
        }

        return SSH_PACKET_USED;
    }



    int channel_rcv_eof_server(ssh_buffer_struct* packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        uint32_t chan = packet->in_uint32_be();
        ssh_channel channel = this->ssh_channel_from_local(chan);
        if (channel == nullptr) {
            ssh_set_error(this->error,  SSH_FATAL,
                          "Server specified invalid channel %lu",
                          static_cast<long unsigned int>(ntohl(chan)));
            syslog(LOG_INFO, "%s", ssh_get_error(&this->error));
            return SSH_PACKET_USED;
        }

        syslog(LOG_INFO,
                "Received eof on channel (%d:%d)",
                channel->local_channel,
                channel->remote_channel);
        /* channel->remote_window = 0; */
        channel->remote_eof = 1;

        if (channel->callbacks && channel->callbacks->channel_eof_function){
            channel->callbacks->channel_eof_function(this,
                                                     channel,
                                                     channel->callbacks->userdata);
        }

        return SSH_PACKET_USED;
    }


    int channel_rcv_close_server(ssh_buffer_struct* packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        uint32_t chan = packet->in_uint32_be();
        ssh_channel channel = this->ssh_channel_from_local(chan);
        if (channel == nullptr) {
            ssh_set_error(this->error,  SSH_FATAL,
                          "Server specified invalid channel %lu",
                          static_cast<long unsigned int>(ntohl(chan)));
            syslog(LOG_INFO, "%s", ssh_get_error(&this->error));

            return SSH_PACKET_USED;
        }

        syslog(LOG_INFO, "Received close on channel (%d:%d)",
                channel->local_channel,
                channel->remote_channel);

        if ((channel->stdout_buffer && channel->stdout_buffer->in_remain() > 0) ||
            (channel->stderr_buffer && channel->stderr_buffer->in_remain() > 0)) {
            channel->delayed_close = 1;
        } else {
            channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_CLOSED;
        }
        if (channel->remote_eof == 0) {
            syslog(LOG_INFO,
                    "Remote host not polite enough to send an eof before close");
        }
        channel->remote_eof = 1;

        /*
         * The remote eof doesn't break things if there was still data into read
         * buffer because the eof is ignored until the buffer is empty.
         */

        if(channel->callbacks && channel->callbacks->channel_close_function){
            channel->callbacks->channel_close_function(this,
                                                       channel,
                                                       channel->callbacks->userdata);
        }
        syslog(LOG_INFO,
                "Return from user channel_close function");

        channel->flags |= SSH_CHANNEL_FLAG_CLOSED_REMOTE;
        if(channel->flags & SSH_CHANNEL_FLAG_FREED_LOCAL){
            for (unsigned i = 0; i < this->nbchannels ; i++){
                if (this->channels[i] == channel){
                    this->nbchannels--;
                    this->channels[i] = this->channels[this->nbchannels];
                    delete channel;
                    break;
                }
            }
        }

        return SSH_PACKET_USED;
    }


    void channel_rcv_request_server(ssh_buffer_struct* packet, error_struct & error)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
    //    syslog(LOG_INFO, "%s ---", __FUNCTION__);

        uint32_t chan = packet->in_uint32_be();
        ssh_channel channel = this->ssh_channel_from_local(chan);
        if (channel == nullptr) {
            ssh_set_error(error,  SSH_FATAL,
                          "Server specified invalid channel %lu",
                          static_cast<long unsigned int>(ntohl(chan)));
            syslog(LOG_INFO,"%s", ssh_get_error(&error));
            return;
        }

        const SSHString request = packet->in_strdup_cstr();
        uint8_t request_code = get_request_code(&request[0]);
        if (request_code == REQUEST_STRING_UNKNOWN){
          syslog(LOG_INFO, "%s --- Unknown channel request %s %s",
            __FUNCTION__, &request[0], channel->show());
          return;
        }
        uint8_t want_reply = packet->in_uint8();

        switch (request_code){
        case REQUEST_STRING_EXIT_STATUS:
            this->handle_exit_status_request_server(channel, want_reply, packet);
            break;
        case REQUEST_STRING_SIGNAL:
            this->handle_signal_request_server(channel, want_reply, packet);
            break;
        case REQUEST_STRING_EXIT_SIGNAL:
            this->handle_exit_signal_request_server(channel, want_reply, packet);
            break;
        case REQUEST_STRING_AUTH_AGENT_REQ_AT_OPENSSH_DOT_COM:
            this->handle_auth_agent_req_at_openssh_dot_com_request_server(channel, want_reply);
            break;
        case REQUEST_STRING_PTY_REQ:
            this->handle_pty_req_request_server(channel, want_reply, packet);
            break;
        case REQUEST_STRING_WINDOW_CHANGE:
            this->handle_window_change_request_server(channel, want_reply, packet);
            break;
        case REQUEST_STRING_SUBSYSTEM:
            this->handle_subsystem_request_server(channel, want_reply, packet);
            break;
        case REQUEST_STRING_SHELL:
            this->handle_shell_request_server(channel, want_reply, packet);
            break;
        case REQUEST_STRING_EXEC:
            this->handle_exec_request_server(channel, want_reply, packet);
            break;
        case REQUEST_STRING_ENV:
            this->handle_env_request_server(channel, want_reply, packet);
            break;
        case REQUEST_STRING_X11_REQ:
            this->handle_x11_req_request_server(channel, want_reply, packet);
            break;
        default:
            ;
        }
    }


    void handle_exit_status_request_server(ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        channel->exit_status = packet->in_uint32_be();

        syslog(LOG_INFO,
          "SSH_MSG_CHANNEL_REQUEST '%s' <%d> for channel %s wr=%d",
          "exit-status", channel->exit_status, channel->show(), want_reply);

        if (channel->callbacks && channel->callbacks->channel_exit_status_function) {
            channel->callbacks->channel_exit_status_function(this,
                                                             channel,
                                                             channel->exit_status,
                                                             channel->callbacks->userdata);
        }
    }

    void handle_signal_request_server(ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        SSHString sig = packet->in_strdup_cstr();
        syslog(LOG_INFO,
          "SSH_MSG_CHANNEL_REQUEST '%s' <%s> for channel %s wr=%d",
          "signal", &sig[0], channel->show(), want_reply);

        if (channel->callbacks && channel->callbacks->channel_signal_function) {
            channel->callbacks->channel_signal_function(this,
                                                        channel,
                                                        &sig[0],
                                                        channel->callbacks->userdata);
        }
    }


    void handle_exit_signal_request_server(ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        SSHString sig = packet->in_strdup_cstr();
        uint8_t i = packet->in_uint8();
        const char *core = i?"(core dumped)":"";
        SSHString errmsg = packet->in_strdup_cstr();
        SSHString lang = packet->in_strdup_cstr();

        syslog(LOG_INFO,
          "SSH_MSG_CHANNEL_REQUEST '%s' <%s %s %s %s> for channel %s wr=%d",
          "exit-signal", &sig[0], core, &errmsg[0], &lang[0], channel->show(), want_reply);

        if (channel->callbacks && channel->callbacks->channel_exit_signal_function) {
            channel->callbacks->channel_exit_signal_function(this,
                                                             channel,
                                                             &sig[0], i, &errmsg[0], &lang[0],
                                                             channel->callbacks->userdata);
        }
    }


    void handle_auth_agent_req_at_openssh_dot_com_request_server(ssh_channel channel, int want_reply)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        syslog(LOG_INFO,
          "SSH_MSG_CHANNEL_REQUEST '%s' <> for channel %s wr=%d",
          "auth-agent-req@openssh.com", channel->show(), want_reply);

        if (channel->callbacks && channel->callbacks->channel_auth_agent_req_function) {
            channel->callbacks->channel_auth_agent_req_function(this, channel,
                                                                channel->callbacks->userdata);
            if (want_reply) {
                syslog(LOG_INFO,"Responding to Openssh's auth-agent-req");
                this->out_buffer->out_uint8(SSH_MSG_CHANNEL_SUCCESS);
                this->out_buffer->out_uint32_be(channel->remote_channel);
                this->packet_send();
            }
        }
    }


    // 6.2.  Requesting a Pseudo-Terminal
    // ----------------------------------

    //   A pseudo-terminal can be allocated for the session by sending the
    //   following message.

    //      byte      SSH_MSG_CHANNEL_REQUEST
    //      uint32    recipient channel
    //      string    "pty-req"
    //      boolean   want_reply
    //      string    TERM environment variable value (e.g., vt100)
    //      uint32    terminal width, characters (e.g., 80)
    //      uint32    terminal height, rows (e.g., 24)
    //      uint32    terminal width, pixels (e.g., 640)
    //      uint32    terminal height, pixels (e.g., 480)
    //      string    encoded terminal modes

    //   The 'encoded terminal modes' are described in Section 8.  Zero
    //   dimension parameters MUST be ignored.  The character/row dimensions
    //   override the pixel dimensions (when nonzero).  Pixel dimensions refer
    //   to the drawable area of the window.

    //   The dimension parameters are only informational.

    //   The client SHOULD ignore pty requests.

    void handle_pty_req_request_server(ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        SSHString TERM = packet->in_strdup_cstr();
        uint32_t width = packet->in_uint32_be();
        uint32_t height = packet->in_uint32_be();
        uint32_t pxwidth = packet->in_uint32_be();
        uint32_t pxheight = packet->in_uint32_be();
        SSHString modes = packet->in_strdup_cstr();

        syslog(LOG_INFO,
          "SSH_MSG_CHANNEL_REQUEST '%s' <%s, %d, %d, %d, %d, %s> for channel %s wr=%d",
          "pty-req", &TERM[0], static_cast<int>(width), static_cast<int>(height),
          static_cast<int>(pxwidth), static_cast<int>(pxheight), &modes[0],
          channel->show(), want_reply);

        if (channel->callbacks && channel->callbacks->channel_pty_request_function){
            int rc = channel->callbacks->channel_pty_request_function(this,
                                                                      channel,
                                                                      &TERM[0],
                                                                      width,
                                                                      height,
                                                                      pxwidth,
                                                                      pxheight,
                                                                      channel->callbacks->userdata);
            if (want_reply) {
                syslog(LOG_INFO, "Sending a channel_request success to channel %d",
                    channel->remote_channel);
                this->out_buffer->out_uint8((rc == 0)?SSH_MSG_CHANNEL_SUCCESS:SSH_MSG_CHANNEL_FAILURE);
                this->out_buffer->out_uint32_be(channel->remote_channel);
                this->packet_send();
            }
            else {
              syslog(LOG_INFO, "The client doesn't want to know if the request succeeded");
            }
        }
    }


    void handle_window_change_request_server(ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        uint32_t width = packet->in_uint32_be();
        uint32_t height = packet->in_uint32_be();
        uint32_t pxwidth = packet->in_uint32_be();
        uint32_t pxheight = packet->in_uint32_be();

        syslog(LOG_INFO,
          "SSH_MSG_CHANNEL_REQUEST '%s' <%d, %d, %d, %d> for channel %s wr=%d",
          "window-change", static_cast<int>(width), static_cast<int>(height),
                           static_cast<int>(pxwidth), static_cast<int>(pxheight),
                           channel->show(), want_reply);

        if (channel->callbacks && channel->callbacks->channel_pty_window_change_function){
            channel->callbacks->channel_pty_window_change_function(this,
                                                                channel,
                                                                width,
                                                                height,
                                                                pxwidth,
                                                                pxheight,
                                                                channel->callbacks->userdata);
        }
        // TODO: why is there no support for wantreply ?
    }

    void handle_subsystem_request_server(ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        SSHString subsystem = packet->in_strdup_cstr();

        syslog(LOG_INFO,
          "SSH_MSG_CHANNEL_REQUEST '%s' <%s> for channel %s wr=%d",
          "subsystem", &subsystem[0], channel->show(), want_reply);

        // SSH_REQUEST_CHANNEL SSH_CHANNEL_REQUEST_SUBSYSTEM
        int rc = -1;
        if (channel->callbacks && channel->callbacks->channel_subsystem_request_function){
            rc = channel->callbacks->channel_subsystem_request_function(static_cast<ssh_session_struct*>(this),
                                                                        channel,
                                                                        &subsystem[0],
                                                                        channel->callbacks->userdata);
        }
        if (want_reply) {
            syslog(LOG_INFO, "Sending a channel_request success to channel %d", channel->remote_channel);
            this->out_buffer->out_uint8((rc == 0)?SSH_MSG_CHANNEL_SUCCESS:SSH_MSG_CHANNEL_FAILURE);
            this->out_buffer->out_uint32_be(channel->remote_channel);
            this->packet_send();
        }
        else {
            syslog(LOG_INFO, "The client doesn't want to know if the request succeeded");
        }
    }


    void handle_shell_request_server(ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
    {
        (void)packet;
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        syslog(LOG_INFO,
          "SSH_MSG_CHANNEL_REQUEST '%s' <> for channel %s wr=%d",
          "shell", channel->show(), want_reply);

        if (channel->callbacks &&  channel->callbacks->channel_shell_request_function){
            int rc = channel->callbacks->channel_shell_request_function(this,
                                                                    channel,
                                                                    channel->callbacks->userdata);
            if (want_reply) {
                syslog(LOG_INFO, "Sending a channel_request success to channel %d",
                     channel->remote_channel);
                this->out_buffer->out_uint8((rc == 0)?SSH_MSG_CHANNEL_SUCCESS:SSH_MSG_CHANNEL_FAILURE);
                this->out_buffer->out_uint32_be(channel->remote_channel);
                this->packet_send();
            }
            else {
                syslog(LOG_INFO, "The client doesn't want to know if the request succeeded");
            }
        }
    }

    void handle_exec_request_server(ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        SSHString command = packet->in_strdup_cstr();

        syslog(LOG_INFO,
          "SSH_MSG_CHANNEL_REQUEST '%s' <%s> for channel %s wr=%d",
          "exec", &command[0], channel->show(), want_reply);

        // TODO: should we not send a reply whenever it is asked, even if we do not have any API callback ?

        if (channel->callbacks && channel->callbacks->channel_exec_request_function){
            int rc = channel->callbacks->channel_exec_request_function(this,
                                                                       channel,
                                                                       &command[0],
                                                                       channel->callbacks->userdata);
            if (want_reply) {
                syslog(LOG_INFO, "Sending a channel_request success to channel %d",
                     channel->remote_channel);

                this->out_buffer->out_uint8((rc == 0)?SSH_MSG_CHANNEL_SUCCESS:SSH_MSG_CHANNEL_FAILURE);
                this->out_buffer->out_uint32_be(channel->remote_channel);
                this->packet_send();
            }
            else {
                syslog(LOG_INFO, "The client doesn't want to know if the request succeeded");
            }
        }
    }

    void handle_env_request_server(ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        SSHString var_name = packet->in_strdup_cstr();
        SSHString var_value = packet->in_strdup_cstr();

        syslog(LOG_INFO,
          "SSH_MSG_CHANNEL_REQUEST '%s' <%s=%s> for channel %s wr=%d",
          "env", &var_name[0], &var_value[0], channel->show(), want_reply);

        if (channel->callbacks && channel->callbacks->channel_env_request_function){
            int rc = channel->callbacks->channel_env_request_function(this,
                                                                      channel,
                                                                      &var_name[0],
                                                                      &var_value[0],
                                                                      channel->callbacks->userdata);
            if (want_reply) {
                syslog(LOG_INFO, "Sending a channel_request success to channel %d",
                     channel->remote_channel);

                this->out_buffer->out_uint8((rc == 0)?SSH_MSG_CHANNEL_SUCCESS:SSH_MSG_CHANNEL_FAILURE);
                this->out_buffer->out_uint32_be(channel->remote_channel);
                this->packet_send();
            }
            else {
                syslog(LOG_INFO, "The client doesn't want to know if the request succeeded");
            }
        }
    }


    void handle_x11_req_request_server(ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        syslog(LOG_INFO,
          "Received a %s channel_request for channel (%d:%d) (want_reply=%d)",
          "x11-req", channel->local_channel, channel->remote_channel, want_reply);

        uint8_t x11_single_connection = packet->in_uint8();
        SSHString x11_auth_protocol = packet->in_strdup_cstr();
        SSHString x11_auth_cookie = packet->in_strdup_cstr();
        // TODO: why no network order ?
        uint32_t x11_screen_number = packet->in_uint32_le();
        if (channel->callbacks && channel->callbacks->channel_x11_req_function){
            channel->callbacks->channel_x11_req_function(this,
                                                         channel,
                                                         x11_single_connection,
                                                         &x11_auth_protocol[0],
                                                         &x11_auth_cookie[0],
                                                         x11_screen_number,
                                                         channel->callbacks->userdata);

            int rc = 0;
            if (want_reply) {
                syslog(LOG_INFO, "Sending a channel_request success to channel %d",
                     channel->remote_channel);

                this->out_buffer->out_uint8((rc == 0)?SSH_MSG_CHANNEL_SUCCESS:SSH_MSG_CHANNEL_FAILURE);
                this->out_buffer->out_uint32_be(channel->remote_channel);
                this->packet_send();
            }
            else {
                syslog(LOG_INFO, "The client doesn't want to know if the request succeeded");
            }

        }
    }

    //  [RFC4253] 11.4.  Reserved Messages
    //  ==================================

    //   An implementation MUST respond to all unrecognized messages with an
    //   SSH_MSG_UNIMPLEMENTED message in the order in which the messages were
    //   received.  Such messages MUST be otherwise ignored.  Later protocol
    //   versions may define other meanings for these message types.

    //      byte      SSH_MSG_UNIMPLEMENTED
    //      uint32    packet sequence number of rejected message

    void ssh_send_unimplemented_server()
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        syslog(LOG_ERR,"Can't do anything with packet type %d", this->in_packet_type);
        this->out_buffer->out_uint8(SSH_MSG_UNIMPLEMENTED);
        this->out_buffer->out_uint32_be(this->recv_seq-1);
        this->packet_send();
    }

    public:
    int ssh_set_auth_methods_server(int authmethods)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        return this->auth_methods = authmethods & 0x3f;
    }
};

struct SshClientSession : public ssh_session_struct
{
    ssh_client_callbacks client_callbacks; /* Callbacks to user functions */
    int auth_methods;

    SshClientSession(
        ssh_poll_ctx_struct * ctx,
        ssh_client_callbacks client_callbacks,
        ssh_socket_struct * socket)
    : ssh_session_struct(ctx, socket)
    , client_callbacks(client_callbacks)
    , auth_methods(0)
    {
    }

    virtual const char * session_type() override { return "Client"; }

    int ssh_channel_is_open_client(ssh_channel_struct * channel) {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        if(channel == nullptr) {
            return 0;
        }
        return (channel->state == ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN);
    }
    int ssh_channel_is_closed_client(ssh_channel_struct * channel) {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        return (channel->state == ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_CLOSED);
    }



    /**
     * @brief Try to authenticate through the "none" method.
     *
     * @returns void
     *
     * The answer will be provided later through a callback
     * to ssh_auth_none_reply_client_cb
     *
     *          SSH_AUTH_ERROR:   A serious error happened.\n
     *          SSH_AUTH_DENIED:  Authentication failed: use another method\n
     *          SSH_AUTH_PARTIAL: You've been partially authenticated, you still
     *                            have to use another method\n
     *          SSH_AUTH_SUCCESS: Authentication success\n
     *          SSH_AUTH_AGAIN:   In nonblocking mode, you've got to call this again
     *                            later.
     *
     */
    public:
    // TODO: we should use username from session context,
    // no need to provide it again and again
    // (and also servers are not supposed to work if username change)
    int ssh_userauth_none_client(const char *username, error_struct * error)
    {
        (void)error;
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        this->out_buffer->out_uint8(SSH_MSG_SERVICE_REQUEST);
        this->out_buffer->out_length_prefixed_cstr("ssh-userauth");
        this->auth_service_state = SSH_AUTH_SERVICE_SENT;
        this->packet_send();

        syslog(LOG_INFO, "%s --- [A]", __FUNCTION__);

        this->out_buffer->out_uint8(SSH_MSG_USERAUTH_REQUEST); /* request */

        syslog(LOG_INFO, "%s --- [A.1]", __FUNCTION__);
        this->out_buffer->out_length_prefixed_cstr(username);
        syslog(LOG_INFO, "%s --- [A.2]", __FUNCTION__);
        this->out_buffer->out_length_prefixed_cstr("ssh-connection"); /* service */
        syslog(LOG_INFO, "%s --- [A.3]", __FUNCTION__);
        this->out_buffer->out_length_prefixed_cstr("none"); /* method */
        this->auth_state = SSH_AUTH_STATE_NONE;
        this->packet_send();

        syslog(LOG_INFO, "%s --- [B]", __FUNCTION__);

        class Event_userauth_none_status_client : public Event
        {
            SshClientSession * client_session;
        public:
            Event_userauth_none_status_client(struct SshClientSession * client_session)
                : client_session(client_session)
            {
                syslog(LOG_INFO, "%s --- [C]", __FUNCTION__);
            }

            virtual ~Event_userauth_none_status_client(void)
            {
                syslog(LOG_INFO, "%s --- [D]", __FUNCTION__);
            }

            int trigger() override
            {
                syslog(LOG_INFO, "%s --- [E]", __FUNCTION__);
                syslog(LOG_INFO, "Event_userauth_none_status_client trigger");
                return this->client_session->auth_state != SSH_AUTH_STATE_NONE;
            }

            void action() override
            {
                syslog(LOG_INFO, "%s --- [F]", __FUNCTION__);

                syslog(LOG_INFO, "Event_userauth_none_status_client action");
                this->client_session->client_callbacks->ssh_auth_none_reply_client_cb(
                                        static_cast<int>(this->client_session->auth_state),
                                        client_session->client_callbacks->userdata);
            }
        } * event = new Event_userauth_none_status_client(this);

        syslog(LOG_INFO, "%s --- [G]", __FUNCTION__);

        this->add_event(event);
        syslog(LOG_INFO, "%s --- [H]", __FUNCTION__);
        return SSH_OK;
    }

    //    [RFC4253] 11.2.  Ignored Data Message
    //    ======================================

    //      byte      SSH_MSG_IGNORE
    //      string    data

    //   All implementations MUST understand (and ignore) this message at any
    //   time (after receiving the identification string).  No implementation
    //   is required to send them.  This message can be used as an additional
    //   protection measure against advanced traffic analysis techniques.

    int ssh_send_ignore_client(const char *data)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        this->out_msg_ignore(data);
        this->packet_send();
        return SSH_OK;
    }

    /**
     * @brief Check if remote has sent an EOF.
     *
     * @param[in]  channel  The channel to check.
     *
     * @return              0 if there is no EOF, nonzero otherwise.
     */
    int ssh_channel_is_eof_client(ssh_channel channel) {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        if ((channel->stdout_buffer && channel->stdout_buffer->in_remain() > 0) ||
            (channel->stderr_buffer && channel->stderr_buffer->in_remain() > 0)) {
            return 0;
        }
        return (channel->remote_eof != 0);
    }

    // After sending this, I expect that the target sshd will create some agent forwarder
    // and create in shell env some variable like
    // SSH_AUTH_SOCK=/tmp/ssh-sUNit15097/agent.15097
    // actually it is not doing that yet
    int ssh_channel_request_auth_agent_client(ssh_channel channel)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        syslog(LOG_INFO, ">>>>>>>>>>>> CHANNEL_REQUEST_AUTH_AGENT_REQ %s", channel->show());

        switch(channel->request_state){
        case SSH_CHANNEL_REQ_STATE_NONE:
        {
            uint8_t want_reply = 1;
            this->out_buffer->out_uint8(SSH_MSG_CHANNEL_REQUEST);
            this->out_buffer->out_uint32_be(channel->remote_channel);
            this->out_buffer->out_length_prefixed_cstr("auth-agent-req@openssh.com");
            this->out_buffer->out_uint8(want_reply);
            this->packet_send();
            if (!want_reply) {
                channel->request_state = SSH_CHANNEL_REQ_STATE_NONE;
                return SSH_OK;
            }
            else {
                channel->request_state = SSH_CHANNEL_REQ_STATE_PENDING;
                return channel->channel_request(this);
            }
        }
        break;
        case SSH_CHANNEL_REQ_STATE_PENDING:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_CHANNEL_REQ_STATE_ACCEPTED:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_CHANNEL_REQ_STATE_DENIED:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_CHANNEL_REQ_STATE_ERROR:
        REDEMPTION_CXX_FALLTHROUGH;
        default:
        break;
        }

        return channel->channel_request(this);
    }

    /**
     * @brief Request a subsystem (for example "sftp").
     *
     * @param[in]  channel  The channel to send the request.
     *
     * @param[in]  subsys   The subsystem to request (for example "sftp").
     *
     * @return              SSH_OK on success,
     *                      SSH_ERROR if an error occurred,
     *                      SSH_AGAIN if in nonblocking mode and call has
     *                      to be done again.
     *
     */
    int ssh_channel_request_subsystem_client(ssh_channel channel, const char *subsys)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        syslog(LOG_INFO, ">>>>>>>>>>>> CHANNEL_REQUEST_SUSYSTEM <%s>", subsys);
        if(channel == nullptr) {
            syslog(LOG_INFO, "%s: missing channel", __FUNCTION__);
            return SSH_ERROR;
        }
        if(subsys == nullptr) {
            syslog(LOG_INFO, "%s: missing subsystem", __FUNCTION__);
            return SSH_ERROR;
        }

        switch(channel->request_state){
        case SSH_CHANNEL_REQ_STATE_NONE:
        {
            uint8_t want_reply = 1;

            this->out_buffer->out_uint8(SSH_MSG_CHANNEL_REQUEST);
            this->out_buffer->out_uint32_be(channel->remote_channel);
            this->out_buffer->out_length_prefixed_cstr("subsystem");
            this->out_buffer->out_uint8(want_reply);

            this->out_buffer->out_length_prefixed_cstr(subsys);
            this->packet_send();
            if (!want_reply) {
                channel->request_state = SSH_CHANNEL_REQ_STATE_NONE;
                return SSH_OK;
            }
            else {
                channel->request_state = SSH_CHANNEL_REQ_STATE_PENDING;
                return channel->channel_request(this);
            }
        }
        break;
        case SSH_CHANNEL_REQ_STATE_PENDING:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_CHANNEL_REQ_STATE_ACCEPTED:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_CHANNEL_REQ_STATE_DENIED:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_CHANNEL_REQ_STATE_ERROR:
        REDEMPTION_CXX_FALLTHROUGH;
        default:
            break;
        }
        return channel->channel_request(this);
    }
    int ssh_channel_write_client(ssh_channel channel, const uint8_t *data, uint32_t len);
    int ssh_channel_write_stderr_client(ssh_channel channel, const uint8_t *data, uint32_t len);

    void ssh_event_x11_requested_channel_client(ssh_channel channel, uint32_t sender, uint32_t window, uint32_t packet_size)
    {
        syslog(LOG_INFO, "x11 Accepting a channel request_open for chan %d", channel->remote_channel);
        channel->local_channel = this->new_channel_id();
        channel->local_window = 32000;
        channel->local_maxpacket = CHANNEL_TOTAL_PACKET_SIZE;
        channel->remote_channel = sender;
        channel->remote_window = window;
        channel->remote_maxpacket = packet_size;
        channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN;

        this->out_buffer->out_uint8(SSH_MSG_CHANNEL_OPEN_CONFIRMATION);
        this->out_buffer->out_uint32_be(sender);
        this->out_buffer->out_uint32_be(channel->local_channel);
        this->out_buffer->out_uint32_be(channel->local_window);
        this->out_buffer->out_uint32_be(channel->local_maxpacket);
        this->packet_send();
    }

    void ssh_event_x11_requested_channel_failure_client(uint32_t sender)
    {
        syslog(LOG_INFO, "x11 Refusing a channel");

        this->out_buffer->out_uint8(SSH_MSG_CHANNEL_OPEN_FAILURE);
        this->out_buffer->out_uint32_be(sender);
        this->out_buffer->out_uint32_be(SSH2_OPEN_ADMINISTRATIVELY_PROHIBITED);
        this->out_buffer->out_uint32_be(0); /* reason is an empty string */
        this->out_buffer->out_uint32_be(0); /* language too */
        this->packet_send();
    }


    // TODO: Should become private when caller will be in class
    int handle_received_data_client(const void *data, size_t receivedlen);

    /**
     * @brief Request a shell.
     *
     * @param[in]  channel  The channel to send the request.
     *
     * @return              SSH_OK on success,
     *                      SSH_ERROR if an error occurred,
     *                      SSH_AGAIN if in nonblocking mode and call has
     *                      to be done again.
     */
    int ssh_channel_request_shell_client(ssh_channel channel) {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);

        switch(channel->request_state){
        case SSH_CHANNEL_REQ_STATE_NONE:
        {
            this->out_buffer->out_uint8(SSH_MSG_CHANNEL_REQUEST);
            this->out_buffer->out_uint32_be(channel->remote_channel);
            this->out_buffer->out_length_prefixed_cstr("shell");
            this->out_buffer->out_uint8(1);
            this->packet_send();
            channel->request_state = SSH_CHANNEL_REQ_STATE_PENDING;
            return channel->channel_request(this);
        }
        break;
        case SSH_CHANNEL_REQ_STATE_PENDING:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_CHANNEL_REQ_STATE_ACCEPTED:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_CHANNEL_REQ_STATE_DENIED:
        REDEMPTION_CXX_FALLTHROUGH;
        case SSH_CHANNEL_REQ_STATE_ERROR:
        REDEMPTION_CXX_FALLTHROUGH;
        default:
            break;
        }
        return channel->channel_request(this);
    }



    void do_target_event(int revents)
    {
        syslog(LOG_INFO, "%s poll event on TARGET (SSH) revent=%d", __FUNCTION__, revents);

        if (this->poll->lock){
            syslog(LOG_INFO, "%s TARGET (SSH)  locked", __FUNCTION__);
        }
        else {
            syslog(LOG_INFO, "%s TARGET (SSH) not locked", __FUNCTION__);
            /* avoid having any event caught during callback */
            this->poll->lock = 1;
            error_struct & error = this->error;
            char buffer[4096];
            int r = 0;
            int err=0;
            socklen_t errlen=sizeof(err);
            /* Do not do anything if this socket was already closed */
            if(this->socket->fd_in == INVALID_SOCKET){
                this->poll->lock = 1;
                return;
            }
            else {
                if(revents & POLLERR || revents & POLLHUP){
                    syslog(LOG_ERR, "POLLERR | POLLHUP");
                    /* Check if we are in a connecting state */
                    if(this->socket->state == SSH_SOCKET_CONNECTING){
                        syslog(LOG_ERR, "Socket connecting");

                        this->socket->state = SSH_SOCKET_ERROR;
                        r = getsockopt(this->socket->fd_in,
                                       SOL_SOCKET, SO_ERROR, &err, &errlen);
                        if (r < 0) {
                            err = errno;
                        }
                        this->socket->last_errno = err;
                        this->socket->close();
                        syslog(LOG_ERR, "TARGET Socket connection callback error: (%d)", err);
                        this->session_state = SSH_SESSION_STATE_ERROR;
                        ssh_set_error(error, SSH_FATAL,"%s",strerror(err));
                        this->socket->close();
                        this->poll->lock = 1;
                        return;
                    }
                    /* Then we are in a more standard kind of error */
                    /* force a read to get an explanation */
                    revents |= POLLIN;
                }

                if((revents & POLLIN) && this->socket->state == SSH_SOCKET_CONNECTED){
                    syslog(LOG_ERR, "Socket connected");
                    this->socket->read_wontblock=1;
                      if (this->socket->data_except) {
                        syslog(LOG_ERR, "data except");
                        r = -1;
                      }
                      else {
                         syslog(LOG_ERR, "TARGET data read %d", static_cast<int>(sizeof(buffer)));
                          r = read(this->socket->fd_in, buffer, sizeof(buffer));
                         syslog(LOG_ERR, "TARGET data read done r=%d", r);
                          this->socket->last_errno = errno;
                          this->socket->read_wontblock = 0;
                          if (r < 0) {
                            this->socket->data_except = 1;
                          }
                      }

                    if(r <= 0){
                        this->session_state = SSH_SESSION_STATE_ERROR;
                        if (r == 0){
                            syslog(LOG_ERR, "TARGET Socket exception: %d (%d)",
                                    SSH_SOCKET_EXCEPTION_EOF,
                                    this->socket->last_errno);
                            ssh_set_error(error,  SSH_FATAL, "Socket error: %s",
                                this->socket->last_errno?strerror(this->socket->last_errno):"disconnected");
                        }
                        else {
                            /** This code is executed each time an error/exception (timeout, ...) occurs on socket. */
                            syslog(LOG_ERR, "TARGET Socket exception: %d (%d)",
                                SSH_SOCKET_EXCEPTION_ERROR,
                                this->socket->last_errno);
                            ssh_set_error(error,  SSH_FATAL, "Socket error: %s",
                                strerror(this->socket->last_errno));
                        }
                        this->socket->close();
                        return;
                    }
                     syslog(LOG_ERR, "TARGET data read %d -> to blob", static_cast<int>(sizeof(buffer)));
                    /* Bufferize the data and then call the callback */
                    this->socket->in_buffer->out_blob(buffer, r);

                    switch (this->socket_callbacks_data_type){
                    default:
                    case ssh_session_struct::data_type_e::HANDLE_RECEIVED_DATA:
                        while (1) {
                            const void *data = this->socket->in_buffer->get_pos_ptr();
                            size_t receivedlen = this->socket->in_buffer->in_remain();
                            r = this->handle_received_data_client(data, receivedlen);
                            if (r == 0 || this->session_state == SSH_SESSION_STATE_ERROR) {
                                break;
                            }
                            this->socket->in_buffer->in_skip_bytes(r);
                        }
                    break;
                    case ssh_session_struct::data_type_e::CALLBACK_RECEIVE_BANNER:
                        while ((r = this->callback_receive_banner_client(
                                    this->socket->in_buffer->get_pos_ptr(),
                                    this->socket->in_buffer->in_remain(), error)) > 0){
                            this->socket->in_buffer->in_skip_bytes(r);
                        }
                        syslog(LOG_INFO,"ssh_session_struct::data_type_e::CALLBACK_RECEIVE_BANNER done");
                    break;
                    }
                }

                if(revents & POLLOUT){
                    /* First, POLLOUT is a sign we may be connected */
                    if(this->socket->state == SSH_SOCKET_CONNECTING){
                        syslog(LOG_INFO,"TARGET Received POLLOUT in connecting state");
                        this->socket->state = SSH_SOCKET_CONNECTED;
            //          set to blocking
                        r = fcntl(this->socket->fd_in, F_SETFL, 0);
                        if (r < 0) {
                            this->poll->lock = 1;
                            return;
                        }
                        this->session_state = SSH_SESSION_STATE_SOCKET_CONNECTED;
                        this->poll->lock = 0;
                        return;
                    }

                    /* We can write data */
                    this->socket->write_wontblock=1;
                    /* If buffered data is pending, write it */
                    if(this->socket->out_buffer->in_remain() > 0){
                        if (this->socket->data_except) {
                            this->socket->close();
                            ssh_set_error(error, SSH_FATAL,
                              "TARGET Writing packet: error on socket (or connection closed): %s",
                              strerror(this->socket->last_errno));
                        }
                        int w = write(this->socket->fd_out,
                                      this->socket->out_buffer->get_pos_ptr(),
                                      this->socket->out_buffer->in_remain());
                        this->socket->last_errno = errno;
                        if (w < 0) {
                            this->socket->data_except = 1;
                            this->socket->close();
                            ssh_set_error(error, SSH_FATAL,
                              "TARGET Writing packet: error on socket (or connection closed): %s",
                              strerror(this->socket->last_errno));
                        }
                        this->socket->out_buffer->in_skip_bytes(w);
                    }
                }
            }
        }
        this->poll->lock = 0;
        return;
    }

    int callback_receive_banner_client(uint8_t *buffer, size_t len, error_struct & error)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        if (this->session_state != SSH_SESSION_STATE_SOCKET_CONNECTED){
            ssh_set_error(error, SSH_FATAL,
                "Wrong state in callback_receive_banner_client : %d", this->session_state);
            return SSH_ERROR;
        }
        syslog(LOG_INFO, "%s --- [A]", __FUNCTION__);

        size_t i = 0;
        for(i=0 ; i<len ; ++i){
            syslog(LOG_INFO, "%s --- [B]", __FUNCTION__);
            if (buffer[i] == '\n') {
                buffer[i]='\0';
                if ((i > 1) && (buffer[i-1] == '\r')){
                    buffer[i-1]= 0;
                }
                /* number of bytes read */
                memcpy(this->serverbanner, buffer, i);
                syslog(LOG_INFO, "%s --- [C]", __FUNCTION__);
                syslog(LOG_INFO,"Received banner: %s", buffer);
                this->client_callbacks->connect_status_function(this->client_callbacks->userdata, 0.4f);
                syslog(LOG_ERR, "SSH server banner: %s", this->serverbanner);

                syslog(LOG_INFO, "%s --- [D]", __FUNCTION__);
                int version = 0;
                /* Here we analyze the different protocols the server allows. */
                ssh_analyze_banner(this->serverbanner, version, this->openssh);
                if (version != 2){
                    ssh_set_error(error, SSH_FATAL,
                        "SSH-2 is the only supported protocol");
                    this->socket->close();
                    this->session_state = SSH_SESSION_STATE_ERROR;
                    return SSH_ERROR;
                }
                this->version = 2;
                /* from now on, the packet layer is handling incoming packets */
                this->socket_callbacks_data_type = ssh_session_struct::data_type_e::HANDLE_RECEIVED_DATA;
                this->session_state = SSH_SESSION_STATE_INITIAL_KEX;

                char buffer[128] = {0};
                memcpy(this->clientbanner, "SSH-2.0-sashimi-4.1.1.0", strlen("SSH-2.0-sashimi-4.1.1.0")+1);
                snprintf(buffer, 128, "%s\n", this->clientbanner);
                this->socket->out_buffer->out_blob(buffer, strlen(buffer));

                this->client_callbacks->connect_status_function(this->client_callbacks->userdata, 0.5f);
                return i+1;
            }
            if(i > 127){
                /* Banner too big */
                this->session_state=SSH_SESSION_STATE_ERROR;
                ssh_set_error(error, SSH_FATAL,"Receiving banner: too large banner");
                return 0;
            }
        }
        return 0;
    }


    private:
    void handle_channel_rcv_request_client(ssh_buffer_struct* packet, error_struct & error);

    void handle_exit_status_request_client(ssh_channel channel, int want_reply, ssh_buffer_struct *packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        channel->exit_status = packet->in_uint32_be();

        syslog(LOG_INFO,
          "SSH_MSG_CHANNEL_REQUEST '%s' <%d> for channel %s wr=%d",
          "exit-status", channel->exit_status, channel->show(), want_reply);

        // TODO: I should never have want_reply log an error if it happens

        // TODO: instead of code below I should have prototype
        // channel->channel_exit_status_callback(exit_status);

        if (channel->callbacks && channel->callbacks->channel_exit_status_function) {
            channel->callbacks->channel_exit_status_function(this,
                                                             channel,
                                                             channel->exit_status,
                                                             channel->callbacks->userdata);
        }
    }


    private:
    int handle_channel_rcv_eof_client(ssh_buffer_struct* packet)
    {
        syslog(LOG_INFO, "%s ---", __FUNCTION__);
        uint32_t chan = packet->in_uint32_be();
        ssh_channel channel = this->ssh_channel_from_local(chan);
        if (channel == nullptr) {
            syslog(LOG_INFO, "%s --- Channel not found", __FUNCTION__);
            ssh_set_error(this->error,  SSH_FATAL,
                          "Server specified invalid channel %lu",
                          static_cast<long unsigned int>(ntohl(chan)));
            syslog(LOG_INFO, "%s", ssh_get_error(&this->error));
            return SSH_PACKET_USED;
        }

        syslog(LOG_INFO, "Received eof on channel (%d:%d)",
                channel->local_channel,
                channel->remote_channel);
        /* channel->remote_window = 0; */
        channel->remote_eof = 1;

        if (channel->callbacks && channel->callbacks->channel_eof_function){
            channel->callbacks->channel_eof_function(this,
                                                     channel,
                                                     channel->callbacks->userdata);
        }

        return SSH_PACKET_USED;
    }

};

REDEMPTION_DIAGNOSTIC_POP


// SshServerSession public methods

void ssh_event_set_session_server(ssh_poll_ctx_struct * ctx, SshServerSession * server_session);
void ssh_disconnect_server(SshServerSession * server_session);
int ssh_userauth_kbdint_settmpprompts_server(SshServerSession * server_session, const char * name,
                          const char * instruction, unsigned int num_prompts,
                          const char ** prompts, unsigned char * echo);

int ssh_userauth_kbdint_settmpprompt_server(SshServerSession * server_session, const char * name,
                         const char * instruction, const char * prompt,
                         unsigned char echo, error_struct * error);
int ssh_userauth_kbdint_getnanswers_server(SshServerSession * server_session, error_struct * error);
const char *ssh_userauth_kbdint_getanswer_server(SshServerSession * server_session, unsigned int i, error_struct * error);
void ssh_channel_free_server(SshServerSession * server_session, ssh_channel channel);
ssh_gssapi_creds ssh_gssapi_get_creds_server(SshServerSession * server_session);
void ssh_channel_open_x11_server(SshServerSession * server_session, ssh_channel channel, const char *orig_addr, int orig_port);

// SshCLientSession public methods

void ssh_event_set_session_client(ssh_poll_ctx_struct * ctx, SshClientSession * client_session);
void ssh_disconnect_client(SshClientSession * client_session);
int ssh_userauth_none_client(SshClientSession * client_session, const char *username, error_struct * error);
int ssh_userauth_list_client(SshClientSession * client_session, error_struct * error);
ssh_auth_e ssh_userauth_try_publickey_client(SshClientSession * client_session, const char *username, const ssh_key_struct *pubkey);
int ssh_userauth_agent_client(SshClientSession * client_session, SshServerSession * front_session, const char *username, error_struct * error);
int ssh_userauth_password_client(SshClientSession * client_session,
                          const char *username,
                          const char *password,
                          error_struct * error);
int ssh_userauth_kbdint_client(SshClientSession * client_session, const char *user, const char *submethods, error_struct * error);
int ssh_userauth_kbdint_getnprompts_client(SshClientSession * client_session);
int ssh_userauth_gssapi_client(SshClientSession * client_session, error_struct * error);
int ssh_userauth_kbdint_setanswer_client(SshClientSession * client_session, unsigned int i, const char *answer, error_struct * error);
const char *ssh_userauth_kbdint_getprompt_client(SshClientSession * client_session, unsigned int i, char *echo, error_struct * error);
int ssh_sessionchannel_open_client(SshClientSession * client_session, ssh_channel channel);
int ssh_channel_open_forward_client(SshClientSession * client_session, ssh_channel channel, const char *remotehost, int remoteport, const char *sourcehost, int localport);
void ssh_channel_free_client(SshClientSession * client_session, ssh_channel channel);
int ssh_channel_send_eof_client(SshClientSession * client_session, ssh_channel channel);
int ssh_channel_close_client(SshClientSession * client_session, ssh_channel channel);
int ssh_get_server_publickey_hash_value_client(const SshClientSession * client_session,
                                        enum ssh_publickey_hash_type type,
                                        unsigned char *buf,
                                        size_t *hlen,
                                        error_struct * error);
int ssh_set_agent_channel_client(SshClientSession * client_session, ssh_channel channel);
void ssh_gssapi_set_creds_client(SshClientSession * client_session, const ssh_gssapi_creds creds);
int ssh_channel_request_env_client(SshClientSession * client_session, ssh_channel channel, const char *name, const char *value);
int ssh_channel_request_x11_client(SshClientSession * client_session, ssh_channel channel, int single_connection, const char *protocol, const char *cookie, int screen_number);
int ssh_channel_request_send_signal_client(SshClientSession * client_session, ssh_channel channel, const char *sig);
int ssh_channel_change_pty_size_client(SshClientSession * client_session, ssh_channel channel, int cols, int rows) ;
int ssh_channel_request_exec_client(SshClientSession * client_session, ssh_channel channel, const char *cmd);
int ssh_channel_request_pty_size_client(SshClientSession * client_session, ssh_channel channel, const char *terminal, int col, int row);


