/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARIO *ICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   new Socket RedTransport class

*/

#ifndef _REDEMPTION_LIBS_RIO_SOCKET_TLS_H_
#define _REDEMPTION_LIBS_RIO_SOCKET_TLS_H_

#include "rio_constants.h"
#include "netutils.hpp"
#include </usr/include/openssl/ssl.h>
#include </usr/include/openssl/err.h>

struct RIOSocketTLS {
    bool tls;
    SSL * ssl;
    int sck;
};

extern "C" {
    /* This method does not allocate space for object itself, 
        but initialize it's properties
        and allocate and initialize it's subfields if necessary
    */
    static inline RIO_ERROR rio_m_RIOSocketTLS_constructor(RIOSocketTLS * self, int sck)
    {
        self->tls = false;
        self->ssl = NULL;
        self->sck = sck;
        return RIO_ERROR_OK;
    }

    /* This method deallocate any space used for subfields if any
    */
    static inline RIO_ERROR rio_m_RIOSocketTLS_destructor(RIOSocketTLS * self)
    {
        return RIO_ERROR_OK;
    }

    static inline void rio_m_RIOSocketTLS_close(RIOSocketTLS * self)
    {
    }


    static inline size_t rio_m_RIOSocketTLS_recv_tls(RIOSocketTLS * self, void * data, size_t len)
    {
        char * pbuffer = (char*)data;
        size_t remaining_len = len;
        while (remaining_len > 0) {
            ssize_t rcvd = ::SSL_read(self->ssl, pbuffer, remaining_len);
            unsigned long error = SSL_get_error(self->ssl, rcvd);
            switch (error) {
                case SSL_ERROR_NONE:
                    pbuffer += rcvd;
                    remaining_len -= rcvd;
                    break;

                case SSL_ERROR_WANT_READ:
                    LOG(LOG_INFO, "recv_tls WANT READ");
                    continue;

                case SSL_ERROR_WANT_WRITE:
                    LOG(LOG_INFO, "recv_tls WANT WRITE");
                    continue;

                case SSL_ERROR_WANT_CONNECT:
                    LOG(LOG_INFO, "recv_tls WANT CONNECT");
                    continue;

                case SSL_ERROR_WANT_ACCEPT:
                    LOG(LOG_INFO, "recv_tls WANT ACCEPT");
                    continue;

                case SSL_ERROR_WANT_X509_LOOKUP:
                    LOG(LOG_INFO, "recv_tls WANT X509 LOOKUP");
                    continue;

                case SSL_ERROR_ZERO_RETURN:
                    return remaining_len - len;
                default:
                {
                    uint32_t errcount = 0;
                    errcount++;
                    LOG(LOG_INFO, "%s", ERR_error_string(error, NULL));
                    while ((error = ERR_get_error()) != 0){
                        errcount++;
                        LOG(LOG_INFO, "%s", ERR_error_string(error, NULL));
                    }
                    TODO("Manage actual errors, check possible values")
                    return (RIO_ERROR)-RIO_ERROR_ANY;
                }
                break;
            }
        }
        return len;
    }


    /* This method receive len bytes of data into buffer
       target buffer *MUST* be large enough to contains len data
       returns len actually received (may be 0),
       or negative value to signal some error.
       If an error occurs after reading some data, the return buffer
       has been changed but an error is returned anyway
       and an error returned on subsequent call.
    */
    static inline ssize_t rio_m_RIOSocketTLS_recv(RIOSocketTLS * self, void * data, size_t len)
    {
        if (self->tls) {
            return rio_m_RIOSocketTLS_recv_tls(self, data, len);
        }
        char * pbuffer = (char*)data;
        size_t remaining_len = len;

        while (remaining_len > 0) {
            ssize_t res = ::recv(self->sck, pbuffer, len, 0);
            switch (res) {
                case -1: /* error, maybe EAGAIN */
                    if (try_again(errno)) {
                        fd_set fds;
                        struct timeval time = { 0, 100000 };
                        FD_ZERO(&fds);
                        FD_SET(self->sck, &fds);
                        select(self->sck + 1, &fds, NULL, NULL, &time);
                        continue;
                    }
                    TODO("replace this with actual error management, EOF is not even an option for sockets")
                    return -RIO_ERROR_EOF;
                case 0: /* no data received, socket closed */
                    return -RIO_ERROR_EOF;
                default: /* some data received */
                    pbuffer += res;
                    remaining_len -= res;
                break;
            }
        }
        return len;
    }

    static inline ssize_t rio_m_RIOSocketTLS_send_tls(RIOSocketTLS * self, const void * data, size_t len)
    {
        const char * const buffer = (const char * const)data;
        size_t remaining_len = len;
        size_t offset = 0;
        while (remaining_len > 0){
            int ret = SSL_write(self->ssl, buffer + offset, remaining_len);

            unsigned long error = SSL_get_error(self->ssl, ret);
            switch (error)
            {
                case SSL_ERROR_NONE:
                    remaining_len -= ret;
                    offset += ret;
                    break;

                case SSL_ERROR_WANT_READ:
                    LOG(LOG_INFO, "send_tls WANT READ");
                    continue;

                case SSL_ERROR_WANT_WRITE:
                    LOG(LOG_INFO, "send_tls WANT WRITE");
                    continue;

                default:
                {
                    LOG(LOG_INFO, "Failure in SSL library");
                    uint32_t errcount = 0;
                    errcount++;
                    LOG(LOG_INFO, "%s", ERR_error_string(error, NULL));
                    while ((error = ERR_get_error()) != 0){
                        errcount++;
                        LOG(LOG_INFO, "%s", ERR_error_string(error, NULL));
                    }
                    return (RIO_ERROR)-RIO_ERROR_ANY;
                }
            }
        }
        return len;
    }

    /* This method send len bytes of data from buffer to current transport
       buffer must actually contains the amount of data requested to send.
       returns len actually sent (may be 0),
       or negative value to signal some error.
       If an error occurs after sending some data the amount sent will be returned
       and an error returned on subsequent call.
    */
    static inline ssize_t rio_m_RIOSocketTLS_send(RIOSocketTLS * self, const void * data, size_t len)
    {
        if (self->tls){
            return rio_m_RIOSocketTLS_send_tls(self, data, len);
        }
        size_t total = 0;
        while (total < len) {
            ssize_t sent = ::send(self->sck, &(((uint8_t*)data)[total]), len - total, 0);
            switch (sent){
            case -1:
                if (try_again(errno)) {
                    fd_set wfds;
                    struct timeval time = { 0, 10000 };
                    FD_ZERO(&wfds);
                    FD_SET(self->sck, &wfds);
                    select(self->sck + 1, NULL, &wfds, NULL, &time);
                    continue;
                }
                return RIO_ERROR_EOF;
            case 0:
                return RIO_ERROR_EOF;
            default:
                total = total + sent;
            }
        }
        return len;
    }

    static inline RIO_ERROR rio_m_RIOSocketTLS_enableTLS(RIOSocketTLS * self)
    {
        LOG(LOG_INFO, "RIO *::enable_tls()");
        SSL_load_error_strings();
        SSL_library_init();

        LOG(LOG_INFO, "RIO *::SSL_CTX_new()");
        SSL_CTX* ctx = SSL_CTX_new(TLSv1_client_method());

        /*
         * This is necessary, because the Microsoft TLS implementation is not perfect.
         * SSL_OP_ALL enables a couple of workarounds for buggy TLS implementations,
         * but the most important workaround being SSL_OP_TLS_BLOCK_PADDING_BUG.
         * As the size of the encrypted payload may give hints about its contents,
         * block padding is normally used, but the Microsoft TLS implementation
         * won't recognize it and will disconnect you after sending a TLS alert.
         */
        LOG(LOG_INFO, "RIO *::SSL_CTX_set_options()");
        SSL_CTX_set_options(ctx, SSL_OP_ALL);
        LOG(LOG_INFO, "RIO *::SSL_new()");
        self->ssl = SSL_new(ctx);

        int flags = fcntl(self->sck, F_GETFL);
        fcntl(self->sck, F_SETFL, flags & ~(O_NONBLOCK));

        LOG(LOG_INFO, "RIO *::SSL_set_fd()");
        SSL_set_fd(self->ssl, self->sck);
        LOG(LOG_INFO, "RIO *::SSL_connect()");
    again:
        int connection_status = SSL_connect(self->ssl);

        if (connection_status <= 0)
        {
            unsigned long error;

            switch (SSL_get_error(self->ssl, connection_status))
            {
                case SSL_ERROR_ZERO_RETURN:
                    LOG(LOG_INFO, "Server closed TLS connection\n");
                    LOG(LOG_INFO, "tls::tls_print_error SSL_ERROR_ZERO_RETURN done\n");
                    return RIO_ERROR_TLS_CONNECT_FAILED;

                case SSL_ERROR_WANT_READ:
                    LOG(LOG_INFO, "SSL_ERROR_WANT_READ\n");
                    LOG(LOG_INFO, "tls::tls_print_error SSL_ERROR_WANT_READ done\n");
                    goto again;

                case SSL_ERROR_WANT_WRITE:
                    LOG(LOG_INFO, "SSL_ERROR_WANT_WRITE\n");
                    LOG(LOG_INFO, "tls::tls_print_error SSL_ERROR_WANT_WRITE done\n");
                    goto again;

                case SSL_ERROR_SYSCALL:
                    LOG(LOG_INFO, "I/O error\n");
                    while ((error = ERR_get_error()) != 0)
                        LOG(LOG_INFO, "%s\n", ERR_error_string(error, NULL));
                    LOG(LOG_INFO, "tls::tls_print_error SSL_ERROR_SYSCLASS done\n");
                    return RIO_ERROR_TLS_CONNECT_FAILED;

                case SSL_ERROR_SSL:
                    LOG(LOG_INFO, "Failure in SSL library (protocol error?)\n");
                    while ((error = ERR_get_error()) != 0)
                        LOG(LOG_INFO, "%s\n", ERR_error_string(error, NULL));
                    LOG(LOG_INFO, "tls::tls_print_error SSL_ERROR_SSL done\n");
                    return RIO_ERROR_TLS_CONNECT_FAILED;

                default:
                    LOG(LOG_INFO, "Unknown error\n");
                    while ((error = ERR_get_error()) != 0){
                        LOG(LOG_INFO, "%s\n", ERR_error_string(error, NULL));
                    }
                    LOG(LOG_INFO, "tls::tls_print_error %s [%u]", strerror(errno), errno);
                    LOG(LOG_INFO, "tls::tls_print_error Unknown error done\n");
                    break;
            }
        }

        LOG(LOG_INFO, "RIO *::SSL_get_peer_certificate()");
        X509 * px509 = SSL_get_peer_certificate(self->ssl);
        if (!px509)
        {
            LOG(LOG_INFO, "RIO *::crypto_cert_get_public_key: SSL_get_peer_certificate() failed");
            return RIO_ERROR_TLS_CONNECT_FAILED;
        }

        LOG(LOG_INFO, "RIO *::X509_get_pubkey()");
        EVP_PKEY* pkey = X509_get_pubkey(px509);
        if (!pkey)
        {
            LOG(LOG_INFO, "RIO *::crypto_cert_get_public_key: X509_get_pubkey() failed");
            return RIO_ERROR_TLS_CONNECT_FAILED;
        }

        LOG(LOG_INFO, "RIO *::i2d_PublicKey()");
        int public_key_length = i2d_PublicKey(pkey, NULL);
        LOG(LOG_INFO, "RIO *::i2d_PublicKey() -> length = %u", public_key_length);
        uint8_t * public_key_data = (uint8_t *)malloc(public_key_length);
        LOG(LOG_INFO, "RIO *::i2d_PublicKey()");
        i2d_PublicKey(pkey, &public_key_data);
        // verify_certificate -> ignore for now

             //            tls::tls_verify_certificate
            //            crypto::x509_verify_certificate

            //                X509_STORE_CTX* csc;
            //                X509_STORE* cert_ctx = NULL;
            //                X509_LOOKUP* lookup = NULL;
            //                X509* xcert = cert->px509;
            //                cert_ctx = X509_STORE_new();
            //                OpenSSL_add_all_algorithms();
            //                lookup = X509_STORE_add_lookup(cert_ctx, X509_LOOKUP_file());
            //                lookup = X509_STORE_add_lookup(cert_ctx, X509_LOOKUP_hash_dir());
            //                X509_LOOKUP_add_dir(lookup, NULL, X509_FILETYPE_DEFAULT);
            //                X509_LOOKUP_add_dir(lookup, certificate_store_path, X509_FILETYPE_ASN1);
            //                csc = X509_STORE_CTX_new();
            //                X509_STORE_set_flags(cert_ctx, 0);
            //                X509_STORE_CTX_init(csc, cert_ctx, xcert, 0);
            //                X509_verify_cert(csc);
            //                X509_STORE_CTX_free(csc);
            //                X509_STORE_free(cert_ctx);

            //            crypto::x509_verify_certificate done
            //            crypto::crypto_get_certificate_data
            //            crypto::crypto_cert_fingerprint

            //                X509_digest(xcert, EVP_sha1(), fp, &fp_len);

            //            crypto::crypto_cert_fingerprint done
            //            crypto::crypto_get_certificate_data done
            //            crypto::crypto_cert_subject_common_name

            //                subject_name = X509_get_subject_name(xcert);
            //                index = X509_NAME_get_index_by_NID(subject_name, NID_commonName, -1);
            //                entry = X509_NAME_get_entry(subject_name, index);
            //                entry_data = X509_NAME_ENTRY_get_data(entry);

            //            crypto::crypto_cert_subject_common_name done
            //            crypto::crypto_cert_subject_alt_name

            //                subject_alt_names = X509_get_ext_d2i(xcert, NID_subject_alt_name, 0, 0);

            //            crypto::crypto_cert_subject_alt_name (!subject_alt_names) done
            //            crypto::crypto_cert_issuer

            //                char * res = crypto_print_name(X509_get_issuer_name(xcert));

            //            crypto::crypto_print_name

            //                BIO* outBIO = BIO_new(BIO_s_mem());
            //                X509_NAME_print_ex(outBIO, name, 0, XN_FLAG_ONELINE)
            //                BIO_read(outBIO, buffer, size);
            //                BIO_free(outBIO);

            //            crypto::crypto_print_name done
            //            crypto::crypto_cert_issuer done
            //            crypto::crypto_cert_subject

            //                char * res = crypto_print_name(X509_get_subject_name(xcert));

            //            crypto::crypto_print_name

            //                BIO* outBIO = BIO_new(BIO_s_mem());
            //                X509_NAME_print_ex(outBIO, name, 0, XN_FLAG_ONELINE)
            //                BIO_read(outBIO, buffer, size);
            //                BIO_free(outBIO);

            //            crypto::crypto_print_name done
            //            crypto::crypto_cert_subject done
            //            crypto::crypto_cert_fingerprint


            //                X509_digest(xcert, EVP_sha1(), fp, &fp_len);

            //            crypto::crypto_cert_fingerprint done
            //            tls::tls_verify_certificate verification_status=1 done
            //            tls::tls_free_certificate

            //                X509_free(cert->px509);

            //            tls::tls_free_certificate done
            //            tls::tls_connect -> true done
        self->tls = true;
        LOG(LOG_INFO, "RIO *::enable_tls() done");
        return RIO_ERROR_OK;
    }
};

#endif

