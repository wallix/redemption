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
    Author(s): Christophe Grosjean, Raphael Zhou

    rdp transparent module main header file
*/

#ifndef _REDEMPTION_MOD_RDP_TRANSPARENT_HPP_
#define _REDEMPTION_MOD_RDP_TRANSPARENT_HPP_

#include "front_api.hpp"
#include "genrandom.hpp"
#include "mod_api.hpp"
#include "RDP/gcc.hpp"
#include "RDP/lic.hpp"
#include "RDP/nego.hpp"

struct mod_rdp_transparent : public mod_api {
    FrontAPI & front;

    CHANNELS::ChannelDefArray mod_channel_list;

    bool use_rdp5;

    uint8_t   lic_layer_license_key[16];
    uint8_t   lic_layer_license_sign_key[16];
    uint8_t * lic_layer_license_data;
    size_t    lic_layer_license_size;

    uint16_t userid;

    char hostname[16];
    char username[128];
    char password[256];
    char domain[256];
    char program[512];
    char directory[512];

    int          encryptionLevel;
    int          encryptionMethod;
    uint32_t     server_public_key_len;
    uint8_t      client_crypt_random[512];
    CryptContext encrypt, decrypt;

    enum {
          EARLY
        , WAITING_SYNCHRONIZE
        , WAITING_CTL_COOPERATE
        , WAITING_GRANT_CONTROL_COOPERATE
        , WAITING_FONT_MAP
        , UP_AND_RUNNING
    } connection_finalization_state;

    enum {
          MOD_RDP_NEGO
        , MOD_RDP_BASIC_SETTINGS_EXCHANGE
        , MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER
        , MOD_RDP_GET_LICENSE
        , MOD_RDP_WAITING_DEMAND_ACTIVE_PDU
        , MOD_RDP_CONNECTED
    } state;

    const bool     console_session;
    const uint8_t  front_bpp;
    const int      keylayout;
    const uint32_t performanceFlags;

    Random & gen;

    uint32_t verbose;

    char auth_channel[8];

    RdpNego nego;

    char client_addr[512];

    mod_rdp_transparent( Transport & trans
                       , const char * target_user
                       , const char * target_password
                       , const char * client_ip
                       , FrontAPI & front
                       , const char * hostname
                       , const bool tls
                       , const ClientInfo & info
                       , Random & gen
                       , const char * auth_channel
                       , const char * alternate_shell
                       , const char * shell_working_directory
                       , uint32_t verbose = 0)
            : mod_api(0, 0)
            , front(front)
            , use_rdp5(true)
            , userid(0)
            , encryptionLevel(0)
            , encryptionMethod(0)
            , server_public_key_len(0)
            , connection_finalization_state(EARLY)
            , state(MOD_RDP_NEGO)
            , console_session(info.console_session)
            , front_bpp(info.bpp)
            , keylayout(info.keylayout)
            , performanceFlags(info.rdp5_performanceflags)
            , gen(gen)
            , verbose(verbose)
            , nego(tls, &trans, target_user) {
        if (this->verbose & 1) {
            LOG(LOG_INFO, "Creation of new mod 'RDP Transparent'");
        }

        ::memset(this->auth_channel, 0, sizeof(this->auth_channel));
        ::strncpy(this->auth_channel, auth_channel, sizeof(this->auth_channel));

        ::memset(this->client_addr, 0, sizeof(this->client_addr));
        ::strncpy(this->client_addr, client_ip, sizeof(this->client_addr));

        this->lic_layer_license_data = 0;
        this->lic_layer_license_size = 0;
        ::memset(this->lic_layer_license_key,      0, 16);
        ::memset(this->lic_layer_license_sign_key, 0, 16);
        TODO("CGR: license loading should be done before creating protocol layers")
        struct stat st;
        char        path[256];
        ::snprintf(path, sizeof(path), LICENSE_PATH "/license.%s", hostname);
        int fd = ::open(path, O_RDONLY);
        if (fd != -1) {
            if (::fstat(fd, &st) != 0) {
                this->lic_layer_license_data = (uint8_t *)malloc(this->lic_layer_license_size);
                if (this->lic_layer_license_data) {
                    size_t lic_size = read( fd, this->lic_layer_license_data
                                          , this->lic_layer_license_size);
                    if (lic_size != this->lic_layer_license_size) {
                        LOG( LOG_ERR, "license file truncated : expected %u, got %u"
                           , this->lic_layer_license_size, lic_size);
                    }
                }
            }
            close(fd);
        }

        // from rdp_sec
        memset(this->client_crypt_random, 0, sizeof(this->client_crypt_random));

        // shared
        memset(this->decrypt.key, 0, 16);
        this->decrypt.encryptionMethod = 2; /* 128 bits */
        memset(this->decrypt.update_key, 0, 16);

        memset(this->encrypt.key, 0, 16);
        memset(this->encrypt.update_key, 0, 16);
        this->encrypt.encryptionMethod = 2; /* 128 bits */

        size_t length;

        TODO("CGR: and if hostname is really larger what happens ? We should at least emit a warning log")
        length = ::strlen(hostname);
        if (length >= sizeof(this->hostname)) {
            LOG(LOG_INFO, "mod_rdp_transparent: hostname too long! %u > %u", length, sizeof(this->hostname));
        }
        length = sizeof(this->hostname) - 1;
        strncpy(this->hostname, hostname, length);
        this->hostname[length] = 0;

        TODO("CGR: and if username is really larger what happens ? We should at least emit a warning log")
        length = ::strlen(target_user);
        if (length >= sizeof(this->username)) {
            LOG(LOG_INFO, "mod_rdp_transparent: username too long! %u > %u", length, sizeof(this->username));
        }
        length = sizeof(this->username) - 1;
        strncpy(this->username, target_user, length);
        this->username[length] = 0;

        TODO("CGR: and if password is really larger what happens ? We should at least emit a warning log")
        length = ::strlen(target_password);
        if (length >= sizeof(this->password)) {
            LOG(LOG_INFO, "mod_rdp_transparent: password too long! %u > %u", length, sizeof(this->password));
        }
        length = sizeof(this->password) - 1;
        strncpy(this->password, target_password, length);
        this->password[length] = 0;

        memset(this->domain, 0, sizeof(this->domain));

        length = sizeof(this->program) - 1;
        strncpy(this->program, alternate_shell, length);
        this->program[length] = 0;
        length = sizeof(this->directory) - 1;
        strncpy(this->directory, shell_working_directory, length);
        this->directory[length] = 0;

        while (this->connection_finalization_state != UP_AND_RUNNING) {
            this->draw_event();
            if (this->event.signal != BACK_EVENT_NONE) {
                LOG(LOG_INFO, "Creation of new mod 'RDP' failed");
                throw Error(ERR_SESSION_UNKNOWN_BACKEND);
            }
        }
    }

    virtual ~mod_rdp_transparent() {
        if (this->lic_layer_license_data) {
            free(this->lic_layer_license_data);
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) {}

    virtual void rdp_input_scancode( long param1, long param2, long param3, long param4
                                   , Keymap2 * keymap) {}

    virtual void rdp_input_synchronize( uint32_t time, uint16_t device_flags, int16_t param1
                                      , int16_t param2) {}

    virtual void rdp_input_invalidate(const Rect & r) {}

    void send_data_request(uint16_t channelId, HStream & stream) {
        BStream x224_header(256);
        BStream mcs_header(256);

        MCS::SendDataRequest_Send mcs( mcs_header, this->userid, channelId, 1, 3, stream.size()
                                     , MCS::PER_ENCODING);

        X224::DT_TPDU_Send(x224_header, stream.size() + mcs_header.size());

        this->nego.trans->send(x224_header, mcs_header, stream);
    }

    // management of module originated event ("data received from server")
    // return non zero if module is "finished", 0 if it's still running
    // the null module never finish and accept any incoming event
    virtual void draw_event(void) {
        switch (this->state) {
        case MOD_RDP_NEGO:
            if (this->verbose & 1) {
                LOG(LOG_INFO, "mod_rdp_transparent::draw_event: Early TLS Security Exchange");
            }

            switch (this->nego.state) {
            default:
                this->nego.server_event();
                break;
            case RdpNego::NEGO_STATE_FINAL:
                {
                    // Basic Settings Exchange
                    // -----------------------

                    // Basic Settings Exchange: Basic settings are exchanged between the client and
                    // server by using the MCS Connect Initial and MCS Connect Response PDUs. The
                    // Connect Initial PDU contains a GCC Conference Create Request, while the
                    // Connect Response PDU contains a GCC Conference Create Response.

                    // These two Generic Conference Control (GCC) packets contain concatenated
                    // blocks of settings data (such as core data, security data and network data)
                    // which are read by client and server


                    // Client                                                     Server
                    //    |--------------MCS Connect Initial PDU with-------------> |
                    //                   GCC Conference Create Request
                    //    | <------------MCS Connect Response PDU with------------- |
                    //                   GCC conference Create Response

                    /* Generic Conference Control (T.124) ConferenceCreateRequest */

                    HStream stream(1024, 65536);
                    // ------------------------------------------------------------
                    GCC::UserData::CSCore cs_core;

                    cs_core.version        = this->use_rdp5 ? 0x00080004 : 0x00080001;
                    cs_core.desktopWidth   = this->front_width;
                    cs_core.desktopHeight  = this->front_height;
                    cs_core.highColorDepth = this->front_bpp;
                    cs_core.keyboardLayout = this->keylayout;

                    uint16_t hostlen    = strlen(hostname);
                    uint16_t maxhostlen = std::min((uint16_t)15, hostlen);

                    for (size_t i = 0; i < maxhostlen ; i++) {
                        cs_core.clientName[i] = hostname[i];
                    }
                    bzero(&(cs_core.clientName[hostlen]), 16-hostlen);
                    if (this->nego.tls) {
                        cs_core.serverSelectedProtocol = 1;
                    }
                    if (this->verbose) {
                        cs_core.log("Sending to Server");
                    }
                    cs_core.emit(stream);

                    // ------------------------------------------------------------
                    GCC::UserData::CSCluster cs_cluster;
                    TODO("CGR: values used for setting console_session looks crazy. It's old code and actual validity of these values should be checked. It should only be about REDIRECTED_SESSIONID_FIELD_VALID and shouldn't touch redirection version. Shouldn't it ?")

                    if (!this->nego.tls) {
                        if (this->console_session) {
                            cs_cluster.flags = GCC::UserData::CSCluster::REDIRECTED_SESSIONID_FIELD_VALID | (3 << 2);   // REDIRECTION V4
                        }
                        else {
                            cs_cluster.flags = GCC::UserData::CSCluster::REDIRECTION_SUPPORTED            | (2 << 2);   // REDIRECTION V3
                        }
                    }
                    else {
                        cs_cluster.flags = GCC::UserData::CSCluster::REDIRECTION_SUPPORTED * ((3 << 2) | 1);  // REDIRECTION V4
                        if (this->console_session) {
                            cs_cluster.flags |= GCC::UserData::CSCluster::REDIRECTED_SESSIONID_FIELD_VALID;
                        }
                    }
                    if (this->verbose) {
                        cs_cluster.log("Sending to server");
                    }
                    cs_cluster.emit(stream);
                    // ------------------------------------------------------------

                    GCC::UserData::CSSecurity cs_security;
                    if (this->verbose) {
                        cs_security.log("Sending to server");
                    }
                    cs_security.emit(stream);
                    // ------------------------------------------------------------

                    const CHANNELS::ChannelDefArray & channel_list = this->front.get_channel_list();
                    size_t                            num_channels = channel_list.size();
                    if (num_channels > 0) {
                        /* Here we need to put channel information in order to redirect channel data
                           from client to server passing through the "proxy" */
                        GCC::UserData::CSNet cs_net;
                        cs_net.channelCount = num_channels;
                        for (size_t index = 0; index < num_channels; index++) {
                            const CHANNELS::ChannelDef & channel_item = channel_list[index];
                            memcpy(cs_net.channelDefArray[index].name, channel_list[index].name, 8);
                            cs_net.channelDefArray[index].options = channel_item.flags;
                            CHANNELS::ChannelDef def;
                            memcpy(def.name, cs_net.channelDefArray[index].name, 8);
                            def.flags = channel_item.flags;
                            if (this->verbose & 16) {
                                def.log(index);
                            }
                            this->mod_channel_list.push_back(def);
                        }

                        // Inject a new channel for auth_channel virtual channel (wablauncher)
                        if (this->auth_channel[0]/* && this->acl*/) {
                            memcpy(cs_net.channelDefArray[num_channels].name, this->auth_channel, 8);
                            TODO("CGR: We should figure out what value options should actually have, not just get any channel option and copy it");
                            cs_net.channelDefArray[num_channels].options =
                                cs_net.channelDefArray[num_channels - 1].options;
                            cs_net.channelCount++;
                            CHANNELS::ChannelDef def;
                            memcpy(def.name, this->auth_channel, 8);
                            def.flags = cs_net.channelDefArray[num_channels].options;
                            if (this->verbose & 16) {
                                def.log(num_channels);
                            }
                            this->mod_channel_list.push_back(def);
                        }

                        if (this->verbose) {
                            cs_net.log("Sending to server");
                        }
                        cs_net.emit(stream);
                    }
                    // ------------------------------------------------------------

                    BStream gcc_header(256);
                    GCC::Create_Request_Send(gcc_header, stream.size());

                    BStream mcs_header(256);
                    MCS::CONNECT_INITIAL_Send mcs( mcs_header, gcc_header.size() + stream.size()
                                                 , MCS::BER_ENCODING);

                    BStream x224_header(256);
                    X224::DT_TPDU_Send(x224_header, mcs_header.size() + gcc_header.size() + stream.size());

                    this->nego.trans->send(x224_header, mcs_header, gcc_header, stream);

                    this->state = MOD_RDP_BASIC_SETTINGS_EXCHANGE;
                }
                break;  // case RdpNego::NEGO_STATE_FINAL:
            }   // switch (this->nego.state)
            break;  // case MOD_RDP_NEGO:

        case MOD_RDP_BASIC_SETTINGS_EXCHANGE:
            if (this->verbose & 1) {
                LOG(LOG_INFO, "mod_rdp_transparent::draw_event: Basic Settings Exchange");
            }
            {
                BStream            x224_data(65536);
                X224::RecvFactory  f(*this->nego.trans, x224_data);
                X224::DT_TPDU_Recv x224(*this->nego.trans, x224_data);

                SubStream                      & mcs_data(x224.payload);
                MCS::CONNECT_RESPONSE_PDU_Recv   mcs(mcs_data, MCS::BER_ENCODING);

                GCC::Create_Response_Recv gcc_cr(mcs.payload);

                while (gcc_cr.payload.in_check_rem(4)) {
                    GCC::UserData::RecvFactory f(gcc_cr.payload);
                    switch (f.tag) {
                    case SC_CORE:
                        {
                            GCC::UserData::SCCore sc_core;
                            sc_core.recv(f.payload);
                            if (this->verbose) {
                                sc_core.log("Received from server");
                            }
                            if (0x0080001 == sc_core.version) { // can't use rdp5
                                this->use_rdp5 = false;
                            }
                        }
                        break;
                    case SC_SECURITY:
                        {
                            GCC::UserData::SCSecurity sc_sec1;
                            sc_sec1.recv(f.payload);
                            if (this->verbose) {
                                sc_sec1.log("Received from server");
                            }

                            this->encryptionLevel  = sc_sec1.encryptionLevel;
                            this->encryptionMethod = sc_sec1.encryptionMethod;
                            if (   (sc_sec1.encryptionLevel  == 0)
                                && (sc_sec1.encryptionMethod == 0)) {   /* no encryption */
                                LOG(LOG_INFO, "No encryption");
                            }
                            else {
                                uint8_t serverRandom[SEC_RANDOM_SIZE] = {};

                                uint8_t modulus[SEC_MAX_MODULUS_SIZE];
                                memset(modulus, 0, sizeof(modulus));
                                uint8_t exponent[SEC_EXPONENT_SIZE];
                                memset(exponent, 0, sizeof(exponent));

                                memcpy(serverRandom, sc_sec1.serverRandom, sc_sec1.serverRandomLen);

                                // serverCertificate (variable): The variable-length certificate containing the
                                //  server's public key information. The length in bytes is given by the
                                // serverCertLen field. If the encryptionMethod and encryptionLevel fields are
                                // both set to 0 then this field MUST NOT be present.

                                /* RSA info */
                                if (sc_sec1.dwVersion ==
                                    GCC::UserData::SCSecurity::CERT_CHAIN_VERSION_1) {
                                    memcpy( exponent, sc_sec1.proprietaryCertificate.RSAPK.pubExp
                                          , SEC_EXPONENT_SIZE);
                                    memcpy( modulus, sc_sec1.proprietaryCertificate.RSAPK.modulus
                                          ,   sc_sec1.proprietaryCertificate.RSAPK.keylen
                                            - SEC_PADDING_SIZE);

                                    this->server_public_key_len =
                                        sc_sec1.proprietaryCertificate.RSAPK.keylen - SEC_PADDING_SIZE;
                                }
                                else {
                                    uint32_t certcount = sc_sec1.x509.certCount;
                                    if (certcount < 2) {
                                        LOG(LOG_WARNING, "Server didn't send enough X509 certificates");
                                        throw Error(ERR_SEC);
                                    }

                                    uint32_t   cert_len = sc_sec1.x509.cert[certcount - 1].len;
                                    X509     * cert     = sc_sec1.x509.cert[certcount - 1].cert;

                                    TODO("CGR: Currently, we don't use the CA Certificate, we should"
                                         "*) Verify the server certificate (server_cert) with the CA certificate."
                                         "*) Store the CA Certificate with the hostname of the server we are connecting"
                                         " to as key, and compare it when we connect the next time, in order to prevent"
                                         " MITM-attacks.")

                                    /* By some reason, Microsoft sets the OID of the Public RSA key to
                                       the oid for "MD5 with RSA Encryption" instead of "RSA Encryption"

                                       Kudos to Richard Levitte for the following (. intuitive .)
                                       lines of code that resets the OID and let's us extract the key. */

                                    int nid = OBJ_obj2nid(cert->cert_info->key->algor->algorithm);
                                    if (   (nid == NID_md5WithRSAEncryption)
                                        || (nid == NID_shaWithRSAEncryption)) {
                                        ASN1_OBJECT_free(cert->cert_info->key->algor->algorithm);
                                        cert->cert_info->key->algor->algorithm =
                                            OBJ_nid2obj(NID_rsaEncryption);
                                    }

                                    EVP_PKEY * epk = X509_get_pubkey(cert);
                                    if (NULL == epk) {
                                        LOG( LOG_WARNING
                                           , "Failed to extract public key from certificate\n");
                                        throw Error(ERR_SEC);
                                    }

                                    RSA * server_public_key = RSAPublicKey_dup((RSA *)epk->pkey.ptr);
                                    EVP_PKEY_free(epk);
                                    this->server_public_key_len = RSA_size(server_public_key);

                                    if (NULL == server_public_key) {
                                        LOG(LOG_WARNING, "Failed to parse X509 server key");
                                        throw Error(ERR_SEC);
                                    }

                                    if (   (this->server_public_key_len < SEC_MODULUS_SIZE)
                                        || (this->server_public_key_len > SEC_MAX_MODULUS_SIZE)) {
                                        LOG( LOG_WARNING, "Wrong server public key size (%u bits)"
                                           , this->server_public_key_len * 8);
                                        throw Error(ERR_SEC_PARSE_CRYPT_INFO_MOD_SIZE_NOT_OK);
                                    }

                                    if (   (BN_num_bytes(server_public_key->e) > SEC_EXPONENT_SIZE)
                                        || (BN_num_bytes(server_public_key->n) > SEC_MAX_MODULUS_SIZE)) {
                                        LOG(LOG_WARNING, "Failed to extract RSA exponent and modulus");
                                        throw Error(ERR_SEC);
                                    }
                                    int len_e = BN_bn2bin(server_public_key->e, (unsigned char*)exponent);
                                    reverseit(exponent, len_e);
                                    int len_n = BN_bn2bin(server_public_key->n, (unsigned char*)modulus);
                                    reverseit(modulus, len_n);
                                    RSA_free(server_public_key);
                                }

                                uint8_t client_random[SEC_RANDOM_SIZE];
                                memset(client_random, 0, sizeof(SEC_RANDOM_SIZE));

                                /* Generate a client random, and determine encryption keys */
                                this->gen.random(client_random, SEC_RANDOM_SIZE);

                                ssllib ssl;

                                ssl.rsa_encrypt( client_crypt_random, client_random, SEC_RANDOM_SIZE
                                               , this->server_public_key_len, modulus, exponent);
                                SEC::KeyBlock key_block(client_random, serverRandom);
                                memcpy(encrypt.sign_key, key_block.blob0, 16);
                                if (sc_sec1.encryptionMethod == 1) {
                                    ssl.sec_make_40bit(encrypt.sign_key);
                                }
                                this->decrypt.generate_key(key_block.key1, sc_sec1.encryptionMethod);
                                this->encrypt.generate_key(key_block.key2, sc_sec1.encryptionMethod);
                            }
                        }
                        break;
                    case SC_NET:
                        {
                            GCC::UserData::SCNet sc_net;
                            sc_net.recv(f.payload);

                            /* We assume that the channel_id array is confirmed in the same order
                               that it has been sent. If there are any channels not confirmed, they're
                               going to be the last channels on the array sent in MCS Connect Initial */
                            if (this->verbose & 16) {
                                LOG( LOG_INFO, "server_channels_count=%u sent_channels_count=%u"
                                   , sc_net.channelCount, mod_channel_list.channelCount);
                            }
                            for (uint32_t index = 0; index < sc_net.channelCount; index++) {
                                if (this->verbose & 16) {
                                    this->mod_channel_list.items[index].log(index);
                                }
                                this->mod_channel_list.set_chanid( index
                                                                 , sc_net.channelDefArray[index].id);
                            }
                            if (this->verbose) {
                                sc_net.log("Received from server");
                            }
                        }
                        break;
                    default:
                        LOG(LOG_WARNING, "unsupported GCC UserData response tag 0x%x", f.tag);
                        throw Error(ERR_GCC);
                    }   // switch (f.tag)
                }   // while (gcc_cr.payload.in_check_rem(4))
                if (gcc_cr.payload.in_check_rem(1)) {
                    LOG(LOG_WARNING, "Error while parsing GCC UserData : short header");
                    throw Error(ERR_GCC);
                }
            }

            if (this->verbose & (1 | 16)) {
                LOG(LOG_INFO, "mod_rdp_transparent::draw_event: Channel Connection");
            }

            // Channel Connection
            // ------------------
            // Channel Connection: The client sends an MCS Erect Domain Request PDU,
            // followed by an MCS Attach User Request PDU to attach the primary user
            // identity to the MCS domain.

            // The server responds with an MCS Attach User Response PDU containing the user
            // channel ID.

            // The client then proceeds to join the :
            // - user channel,
            // - the input/output (I/O) channel
            // - and all of the static virtual channels

            // (the I/O and static virtual channel IDs are obtained from the data embedded
            //  in the GCC packets) by using multiple MCS Channel Join Request PDUs.

            // The server confirms each channel with an MCS Channel Join Confirm PDU.
            // (The client only sends a Channel Join Request after it has received the
            // Channel Join Confirm for the previously sent request.)

            // From this point, all subsequent data sent from the client to the server is
            // wrapped in an MCS Send Data Request PDU, while data sent from the server to
            //  the client is wrapped in an MCS Send Data Indication PDU. This is in
            // addition to the data being wrapped by an X.224 Data PDU.

            // Client                                                     Server
            //    |-------MCS Erect Domain Request PDU--------------------> |
            //    |-------MCS Attach User Request PDU---------------------> |

            //    | <-----MCS Attach User Confirm PDU---------------------- |

            //    |-------MCS Channel Join Request PDU--------------------> |
            //    | <-----MCS Channel Join Confirm PDU--------------------- |

            if (this->verbose & 1) {
                LOG(LOG_INFO, "Send MCS::ErectDomainRequest");
            }
            {
                BStream x224_header(256);
                HStream mcs_data(256, 512);

                MCS::ErectDomainRequest_Send mcs(mcs_data, 0, 0, MCS::PER_ENCODING);

                X224::DT_TPDU_Send(x224_header, mcs_data.size());
                this->nego.trans->send(x224_header, mcs_data);
            }
            if (this->verbose & 1) {
                LOG(LOG_INFO, "Send MCS::AttachUserRequest");
            }
            {
                BStream x224_header(256);
                HStream mcs_data(256, 512);

                MCS::AttachUserRequest_Send mcs(mcs_data, MCS::PER_ENCODING);

                X224::DT_TPDU_Send(x224_header, mcs_data.size());
                this->nego.trans->send(x224_header, mcs_data);
            }
            this->state = MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER;
            break;  // case MOD_RDP_BASIC_SETTINGS_EXCHANGE:

        case MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER:
            if (this->verbose & 1) {
                LOG(LOG_INFO, "mod_rdp_transparent::draw_event: Channel Connection Attach User");
            }
            {
                {
                    BStream stream(65536);
                    X224::RecvFactory  f(*this->nego.trans, stream);
                    X224::DT_TPDU_Recv x224(*this->nego.trans, stream);
                    SubStream & payload = x224.payload;

                    MCS::AttachUserConfirm_Recv mcs(payload, MCS::PER_ENCODING);
                    if (mcs.initiator_flag) {
                        this->userid = mcs.initiator;
                    }
                }

                {
                    size_t num_channels = this->mod_channel_list.size();
                    uint16_t channels_id[CHANNELS::MAX_STATIC_VIRTUAL_CHANNELS];
                    channels_id[0] = this->userid + GCC::MCS_USERCHANNEL_BASE;
                    channels_id[1] = GCC::MCS_GLOBAL_CHANNEL;
                    for (size_t index = 0; index < num_channels; index++) {
                        channels_id[index+2] = this->mod_channel_list[index].chanid;
                    }

                    for (size_t index = 0; index < num_channels+2; index++) {
                        BStream x224_header(256);
                        HStream mcs_cjrq_data(256, 512);
                        if (this->verbose & 16) {
                            LOG(LOG_INFO, "cjrq[%u] = %u", index, channels_id[index]);
                        }
                        MCS::ChannelJoinRequest_Send( mcs_cjrq_data, this->userid
                                                    , channels_id[index], MCS::PER_ENCODING);
                        X224::DT_TPDU_Send(x224_header, mcs_cjrq_data.size());
                        this->nego.trans->send(x224_header, mcs_cjrq_data);

                        BStream x224_data(256);
                        X224::RecvFactory  f(*this->nego.trans, x224_data);
                        X224::DT_TPDU_Recv x224(*this->nego.trans, x224_data);
                        SubStream & mcs_cjcf_data = x224.payload;
                        MCS::ChannelJoinConfirm_Recv mcs(mcs_cjcf_data, MCS::PER_ENCODING);
                        TODO("If mcs.result is negative channel is not confirmed and should be removed from mod_channel list")
                        if (this->verbose & 16) {
                            LOG(LOG_INFO, "cjcf[%u] = %u", index, mcs.channelId);
                        }
                    }
                }

                // RDP Security Commencement
                // -------------------------

                // RDP Security Commencement: If standard RDP security methods are being
                // employed and encryption is in force (this is determined by examining the data
                // embedded in the GCC Conference Create Response packet) then the client sends
                // a Security Exchange PDU containing an encrypted 32-byte random number to the
                // server. This random number is encrypted with the public key of the server
                // (the server's public key, as well as a 32-byte server-generated random
                // number, are both obtained from the data embedded in the GCC Conference Create
                //  Response packet).

                // The client and server then utilize the two 32-byte random numbers to generate
                // session keys which are used to encrypt and validate the integrity of
                // subsequent RDP traffic.

                // From this point, all subsequent RDP traffic can be encrypted and a security
                // header is include " with the data if encryption is in force (the Client Info
                // and licensing PDUs are an exception in that they always have a security
                // header). The Security Header follows the X.224 and MCS Headers and indicates
                // whether the attached data is encrypted.

                // Even if encryption is in force server-to-client traffic may not always be
                // encrypted, while client-to-server traffic will always be encrypted by
                // Microsoft RDP implementations (encryption of licensing PDUs is optional,
                // however).

                // Client                                                     Server
                //    |------Security Exchange PDU ---------------------------> |
                if (this->verbose & 1) {
                    LOG(LOG_INFO, "mod_rdp_transparent::draw_event: RDP Security Commencement");
                }

                if (this->encryptionLevel) {
                    if (this->verbose & 1) {
                        LOG( LOG_INFO, "mod_rdp_transparent::draw_event: SecExchangePacket keylen=%u"
                           , this->server_public_key_len);
                    }
                    HStream stream(512, 512 + this->server_public_key_len + 32);
                    SEC::SecExchangePacket_Send mcs( stream, client_crypt_random
                                                   , this->server_public_key_len);
                    this->send_data_request(GCC::MCS_GLOBAL_CHANNEL, stream);
                }

                // Secure Settings Exchange
                // ------------------------

                // Secure Settings Exchange: Secure client data (such as the username,
                // password and auto-reconnect cookie) is sent to the server using the Client
                // Info PDU.

                // Client                                                     Server
                //    |------ Client Info PDU      ---------------------------> |

                if (this->verbose & 1) {
                    LOG(LOG_INFO, "mod_rdp_transparent::draw_event: Secure Settings Exchange");
                }

                this->send_client_info_pdu(this->userid, this->password);

                this->state = MOD_RDP_GET_LICENSE;
            }
            break;  // case MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER:

        case MOD_RDP_GET_LICENSE:
            if (this->verbose & 2) {
                LOG(LOG_INFO, "mod_rdp_transparent::draw_event: Licensing");
            }
            // Licensing
            // ---------

            // Licensing: The goal of the licensing exchange is to transfer a
            // license from the server to the client.

            // The client should store this license and on subsequent
            // connections send the license to the server for validation.
            // However, in some situations the client may not be issued a
            // license to store. In effect, the packets exchanged during this
            // phase of the protocol depend on the licensing mechanisms
            // employed by the server. Within the context of this document
            // we will assume that the client will not be issued a license to
            // store. For details regarding more advanced licensing scenarios
            // that take place during the Licensing Phase, see [MS-RDPELE].

            // Client                                                     Server
            //    | <------ License Error PDU Valid Client ---------------- |

            // 2.2.1.12 Server License Error PDU - Valid Client
            // ================================================

            // The License Error (Valid Client) PDU is an RDP Connection Sequence PDU sent
            // from server to client during the Licensing phase of the RDP Connection
            // Sequence (see section 1.3.1.1 for an overview of the RDP Connection Sequence
            // phases). This licensing PDU indicates that the server will not issue the
            // client a license to store and that the Licensing Phase has ended
            // successfully. This is one possible licensing PDU that may be sent during the
            // Licensing Phase (see [MS-RDPELE] section 2.2.2 for a list of all permissible
            // licensing PDUs).

            // tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

            // x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224] section 13.7.

            // mcsSDin (variable): Variable-length PER-encoded MCS Domain PDU (DomainMCSPDU)
            // which encapsulates an MCS Send Data Indication structure (SDin, choice 26
            // from DomainMCSPDU), as specified in [T125] section 11.33 (the ASN.1 structure
            // definitions are given in [T125] section 7, parts 7 and 10). The userData
            // field of the MCS Send Data Indication contains a Security Header and a Valid
            // Client License Data (section 2.2.1.12.1) structure.

            // securityHeader (variable): Security header. The format of the security header
            // depends on the Encryption Level and Encryption Method selected by the server
            // (sections 5.3.2 and 2.2.1.4.3).

            // This field MUST contain one of the following headers:
            //  - Basic Security Header (section 2.2.8.1.1.2.1) if the Encryption Level
            // selected by the server is ENCRYPTION_LEVEL_NONE (0) or ENCRYPTION_LEVEL_LOW
            // (1) and the embedded flags field does not contain the SEC_ENCRYPT (0x0008)
            // flag.
            //  - Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption Method
            // selected by the server is ENCRYPTION_METHOD_40BIT (0x00000001),
            // ENCRYPTION_METHOD_56BIT (0x00000008), or ENCRYPTION_METHOD_128BIT
            // (0x00000002) and the embedded flags field contains the SEC_ENCRYPT (0x0008)
            // flag.
            //  - FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Method
            // selected by the server is ENCRYPTION_METHOD_FIPS (0x00000010) and the
            // embedded flags field contains the SEC_ENCRYPT (0x0008) flag.

            // If the Encryption Level is set to ENCRYPTION_LEVEL_CLIENT_COMPATIBLE (2),
            // ENCRYPTION_LEVEL_HIGH (3), or ENCRYPTION_LEVEL_FIPS (4) and the flags field
            // of the security header does not contain the SEC_ENCRYPT (0x0008) flag (the
            // licensing PDU is not encrypted), then the field MUST contain a Basic Security
            // Header. This MUST be the case if SEC_LICENSE_ENCRYPT_SC (0x0200) flag was not
            // set on the Security Exchange PDU (section 2.2.1.10).

            // The flags field of the security header MUST contain the SEC_LICENSE_PKT
            // (0x0080) flag (see Basic (TS_SECURITY_HEADER)).

            // validClientLicenseData (variable): The actual contents of the License Error
            // (Valid Client) PDU, as specified in section 2.2.1.12.1.

            {
                const char * hostname = this->hostname;
                const char * username = this->username;
                // read tpktHeader (4 bytes = 3 0 len)
                // TPDU class 0    (3 bytes = LI F0 PDU_DT)

                BStream stream(65536);
                X224::RecvFactory  f(*this->nego.trans, stream);
                X224::DT_TPDU_Recv x224(*this->nego.trans, stream);
                SubStream & mcs_data = x224.payload;
                MCS::SendDataIndication_Recv mcs(mcs_data, MCS::PER_ENCODING);

                SEC::SecSpecialPacket_Recv sec( mcs.payload, this->decrypt
                                              , this->encryptionLevel);

                if (sec.flags & SEC::SEC_LICENSE_PKT) {
                    LIC::RecvFactory flic(sec.payload);

                    switch (flic.tag) {
                    case LIC::LICENSE_REQUEST:
                        if (this->verbose & 2) {
                            LOG(LOG_INFO, "mod_rdp_transparent::draw_event: License Request");
                        }
                        {
                            LIC::LicenseRequest_Recv lic(sec.payload);
                            uint8_t null_data[SEC_MODULUS_SIZE];
                            memset(null_data, 0, sizeof(null_data));
                            /* We currently use null client keys. This is a bit naughty but, hey,
                               the security of license negotiation isn't exactly paramount. */
                            SEC::SessionKey keyblock(null_data, null_data, lic.server_random);

                            /* Store first 16 bytes of session key as MAC secret */
                            memcpy( this->lic_layer_license_sign_key
                                  , keyblock.get_MAC_salt_key(), 16);
                            memcpy(this->lic_layer_license_key
                                  , keyblock.get_LicensingEncryptionKey(), 16);

                            BStream sec_header(256);
                            HStream lic_data(1024, 65535);

                            if (this->lic_layer_license_size > 0) {
                                uint8_t hwid[LIC::LICENSE_HWID_SIZE];
                                buf_out_uint32(hwid, 2);
                                memcpy(hwid + 4, hostname, LIC::LICENSE_HWID_SIZE - 4);

                                ssllib ssl;
                                /* Generate a signature for the HWID buffer */
                                uint8_t signature[LIC::LICENSE_SIGNATURE_SIZE];

                                FixedSizeStream sig(signature, sizeof(signature));
                                FixedSizeStream key( this->lic_layer_license_sign_key
                                                   , sizeof(this->lic_layer_license_sign_key));
                                FixedSizeStream data(hwid, sizeof(hwid));

                                ssl.sign(sig, key, data);
                                /* Now encrypt the HWID */

                                SslRC4 rc4;
                                rc4.set_key(FixedSizeStream(this->lic_layer_license_key, 16));

                                FixedSizeStream hwid_stream(hwid, sizeof(hwid));
                                rc4.crypt(hwid_stream);

                                LIC::ClientLicenseInfo_Send( lic_data, this->use_rdp5 ? 3 : 2
                                                           , this->lic_layer_license_size
                                                           , this->lic_layer_license_data
                                                           , hwid, signature);
                            }
                            else {
                                LIC::NewLicenseRequest_Send( lic_data, this->use_rdp5 ? 3 : 2
                                                           , username, hostname);
                            }

                            SEC::Sec_Send sec( sec_header, lic_data, SEC::SEC_LICENSE_PKT
                                             , this->encrypt, 0);
                            lic_data.copy_to_head(sec_header);

                            this->send_data_request(GCC::MCS_GLOBAL_CHANNEL, lic_data);
                        }
                        break;  // case LIC::LICENSE_REQUEST:
                    case LIC::PLATFORM_CHALLENGE:
                        if (this->verbose & 2) {
                            LOG(LOG_INFO, "mod_rdp_transparent::draw_event: Platform Challenge");
                        }
                        {
                            LIC::PlatformChallenge_Recv lic(sec.payload);

                            uint8_t out_token[LIC::LICENSE_TOKEN_SIZE];
                            uint8_t decrypt_token[LIC::LICENSE_TOKEN_SIZE];
                            uint8_t hwid[LIC::LICENSE_HWID_SIZE];
                            uint8_t crypt_hwid[LIC::LICENSE_HWID_SIZE];
                            uint8_t out_sig[LIC::LICENSE_SIGNATURE_SIZE];

                            memcpy( out_token, lic.encryptedPlatformChallenge.blob
                                  , LIC::LICENSE_TOKEN_SIZE);
                            /* Decrypt the token. It should read TEST in Unicode. */
                            memcpy( decrypt_token, lic.encryptedPlatformChallenge.blob
                                  , LIC::LICENSE_TOKEN_SIZE);
                            SslRC4 rc4_decrypt_token;
                            rc4_decrypt_token.set_key(
                                FixedSizeStream(this->lic_layer_license_key, 16));
                            FixedSizeStream decrypt_token_stream( decrypt_token
                                                                , LIC::LICENSE_TOKEN_SIZE);
                            rc4_decrypt_token.crypt(decrypt_token_stream);

                            /* Generate a signature for a buffer of token and HWID */
                            buf_out_uint32(hwid, 2);
                            memcpy(hwid + 4, hostname, LIC::LICENSE_HWID_SIZE - 4);

                            uint8_t sealed_buffer[
                                LIC::LICENSE_TOKEN_SIZE + LIC::LICENSE_HWID_SIZE];
                            memcpy(sealed_buffer, decrypt_token, LIC::LICENSE_TOKEN_SIZE);
                            memcpy( sealed_buffer + LIC::LICENSE_TOKEN_SIZE, hwid
                                  , LIC::LICENSE_HWID_SIZE);

                            ssllib ssl;

                            FixedSizeStream sig(out_sig, sizeof(out_sig));
                            FixedSizeStream key( this->lic_layer_license_sign_key
                                               , sizeof(this->lic_layer_license_sign_key));
                            FixedSizeStream data(sealed_buffer, sizeof(sealed_buffer));

                            ssl.sign(sig, key, data);

                            /* Now encrypt the HWID */
                            memcpy(crypt_hwid, hwid, LIC::LICENSE_HWID_SIZE);
                            SslRC4 rc4_hwid;
                            rc4_hwid.set_key(FixedSizeStream(this->lic_layer_license_key, 16));
                            FixedSizeStream crypt_hwid_stream( crypt_hwid
                                                             , LIC::LICENSE_HWID_SIZE);
                            rc4_hwid.crypt(crypt_hwid_stream);

                            BStream sec_header(256);
                            HStream lic_data(1024, 65535);

                            LIC::ClientPlatformChallengeResponse_Send( lic_data
                                                                     , this->use_rdp5 ? 3 : 2
                                                                     , out_token, crypt_hwid
                                                                     , out_sig);
                            SEC::Sec_Send sec( sec_header, lic_data, SEC::SEC_LICENSE_PKT
                                             , this->encrypt, 0);
                            lic_data.copy_to_head(sec_header);
                            this->send_data_request(GCC::MCS_GLOBAL_CHANNEL, lic_data);
                        }
                        break;
                    case LIC::NEW_LICENSE:
                        {
                            if (this->verbose & 2) {
                                LOG(LOG_INFO, "mod_rdp_transparent::draw_event: New License");
                            }

                            LIC::NewLicense_Recv lic(sec.payload, this->lic_layer_license_key);

                            TODO("CGR: Save license to keep a local copy of the license of a remote server thus avoiding to ask it every time we connect. Not obvious files is the best choice to do that")
                            this->state = MOD_RDP_CONNECTED;

                            LOG(LOG_WARNING, "New license not saved");
                        }
                        break;
                    case LIC::UPGRADE_LICENSE:
                        {
                            if (this->verbose & 2) {
                                LOG(LOG_INFO, "mod_rdp_transparent::draw_event: Upgrade License");
                            }
                            LIC::UpgradeLicense_Recv lic( sec.payload
                                                        , this->lic_layer_license_key);

                            LOG(LOG_WARNING, "Upgraded license not saved");
                        }
                        break;
                    case LIC::ERROR_ALERT:
                        {
                            if (this->verbose & 2) {
                                LOG(LOG_INFO, "mod_rdp_transparent::draw_event: Get license status");
                            }
                            LIC::ErrorAlert_Recv lic(sec.payload);
                            if (   (lic.validClientMessage.dwErrorCode       ==
                                    LIC::STATUS_VALID_CLIENT)
                                && (lic.validClientMessage.dwStateTransition ==
                                    LIC::ST_NO_TRANSITION)) {
                                this->state = MOD_RDP_CONNECTED;
                            }
                            else {
                                LOG( LOG_ERR, "mod_rdp_transparent::draw_event: License Alert: error=%u transition=%u"
                                   , lic.validClientMessage.dwErrorCode
                                   , lic.validClientMessage.dwStateTransition);
                            }
                            this->state = MOD_RDP_CONNECTED;
                        }
                        break;
                    default:
                        {
                            LOG( LOG_WARNING
                               , "Unexpected license tag sent from server (tag = %x)"
                               , flic.tag);
                            throw Error(ERR_SEC);
                        }
                        break;
                    }   // switch (flic.tag)
                    TODO("CGR: check if moving end is still necessary all data should have been consumed")
                    if (sec.payload.p != sec.payload.end) {
                        LOG( LOG_ERR, "all data should have been consumed %s:%u tag = %x"
                           , __FILE__, __LINE__, flic.tag);
                        throw Error(ERR_SEC);
                    }
                }   // if (sec.flags & SEC::SEC_LICENSE_PKT)
                else {
                    LOG(LOG_ERR, "Failed to get expected license negotiation PDU");
                    hexdump(x224.payload.get_data(), x224.payload.size());
                    //                throw Error(ERR_SEC);
                    this->state   = MOD_RDP_CONNECTED;
                    sec.payload.p = sec.payload.end;
                    hexdump(sec.payload.get_data(), sec.payload.size());
                }   // if (sec.flags & SEC::SEC_LICENSE_PKT)
            }
            break;  // case MOD_RDP_GET_LICENSE:
        }   // switch (this->state)
    }   // virtual void draw_event(void)

    void send_client_info_pdu(int userid, const char * password) {
        if (this->verbose & 1) {
            LOG(LOG_INFO, "mod_rdp_transparent::send_client_info_pdu");
        }
        HStream stream(1024, 2048);

        InfoPacket infoPacket( this->use_rdp5
                             , this->domain
                             , this->username
                             , password
                             , this->program
                             , this->directory
                             , this->performanceFlags
                             , this->client_addr
                             );

        if (this->verbose) {
            infoPacket.log("Sending to server: ");
        }
        infoPacket.emit(stream);
        stream.mark_end();

        BStream sec_header(256);

        SEC::Sec_Send sec( sec_header, stream, SEC::SEC_INFO_PKT, this->encrypt
                         , this->encryptionLevel);
        stream.copy_to_head(sec_header);

        this->send_data_request(GCC::MCS_GLOBAL_CHANNEL, stream);

        if (this->verbose & 1) {
            LOG(LOG_INFO, "mod_rdp_transparent::send_client_info_pdu done");
        }
    }

    virtual void begin_update() {}
    virtual void end_update() {}

    virtual void draw(const RDPOpaqueRect & cmd, const Rect & clip) {}
    virtual void draw(const RDPScrBlt     & cmd, const Rect & clip) {}
    virtual void draw(const RDPDestBlt    & cmd, const Rect & clip) {}
    virtual void draw(const RDPPatBlt     & cmd, const Rect & clip) {}
    virtual void draw(const RDPMemBlt     & cmd, const Rect & clip, const Bitmap & bmp) {}
    virtual void draw(const RDPMem3Blt    & cmd, const Rect & clip, const Bitmap & bmp) {}
    virtual void draw(const RDPLineTo     & cmd, const Rect & clip) {}
    virtual void draw(const RDPGlyphIndex & cmd, const Rect & clip) {}

    virtual void server_draw_text( int16_t x, int16_t y, const char * text, uint32_t fgcolor
                                 , uint32_t bgcolor, const Rect & clip) {}

    virtual void text_metrics(const char * text, int & width, int & height) {}

    virtual void send_to_front_channel( const char * const mod_channel_name, uint8_t * data
                                      , size_t length, size_t chunk_size, int flags) {}

    virtual void send_to_mod_channel( const char * const front_channel_name
                                    , Stream & chunk
                                    , size_t length
                                    , uint32_t flags) {
        if (this->verbose & 16) {
            LOG(LOG_INFO, "mod_rdp_transparent::send_to_mod_channel");
            LOG(LOG_INFO, "sending to channel %s", front_channel_name);
        }
    }
};  // struct mod_rdp_transparent

#endif  // #ifndef _REDEMPTION_MOD_RDP_TRANSPARENT_HPP_
