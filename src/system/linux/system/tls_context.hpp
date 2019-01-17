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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan

   Transport layer abstraction, socket implementation with TLS support
*/


#pragma once

#include "core/server_notifier_api.hpp"
#include "core/app_path.hpp"
#include "core/error.hpp"
#include "utils/fileutils.hpp"
#include "utils/file.hpp"
#include "utils/log.hpp"
#include "utils/sugar/algostring.hpp"

#include "transport/transport.hpp" // Transport::TlsResult

#include "cxx/diagnostic.hpp"

#include <memory>
#include <cstring>

#include <fcntl.h>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>


REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wzero-as-null-pointer-constant")
#if REDEMPTION_COMP_CLANG_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wzero-as-null-pointer-constant")
#endif

inline bool tls_ctx_print_error(char const* error_msg, std::string* error_message)
{
    LOG(LOG_ERR, "TLSContext::enable_client_tls: %s", error_msg);
    unsigned long error;
    char buf[1024];
    while ((error = ERR_get_error()) != 0) {
        ERR_error_string_n(error, buf, sizeof(buf));
        LOG(LOG_ERR, "print_error %s", buf);
        if (error_message) {
            *error_message += buf;
        }
    }
    return false;
}

class TLSContext
{
    bool tls = false;
    SSL_CTX * allocated_ctx = nullptr;
    SSL     * allocated_ssl = nullptr;
    SSL     * io = nullptr;
    std::unique_ptr<uint8_t[]> public_key;
    size_t public_key_length = 0;

public:
    TLSContext() = default;

    ~TLSContext()
    {
        if (this->allocated_ssl) {
            //SSL_shutdown(this->allocated_ssl);
            SSL_free(this->allocated_ssl);
        }

        if (this->allocated_ctx) {
            SSL_CTX_free(this->allocated_ctx);
        }
    }

    int pending_data() const
    {
        return SSL_pending(this->allocated_ssl);
    }

    array_view_const_u8 get_public_key() const noexcept
    {
        return {this->public_key.get(), this->public_key_length};
    }

    static inline std::unique_ptr<char[]> crypto_print_name(X509_NAME* name)
    {
        std::unique_ptr<char[]> buffer;
        BIO* outBIO = BIO_new(BIO_s_mem());

        if (X509_NAME_print_ex(outBIO, name, 0, XN_FLAG_ONELINE) > 0)
        {
            unsigned long size = BIO_number_written(outBIO);
            buffer = std::make_unique<char[]>(size + 1);
            memset(buffer.get(), 0, size + 1);
            BIO_read(outBIO, buffer.get(), size);
        }
        BIO_free(outBIO);
        return buffer;
    }

    // TODO we should be able to simplify that to just put expected value in a provided buffer
    static inline std::unique_ptr<char[]> crypto_cert_fingerprint(X509* xcert)
    {
        uint32_t fp_len;
        uint8_t fp[EVP_MAX_MD_SIZE];

        X509_digest(xcert, EVP_sha1(), fp, &fp_len);

        auto fp_buffer = std::make_unique<char[]>(3 * fp_len);
        memset(fp_buffer.get(), 0, 3 * fp_len);

        char * p = fp_buffer.get();

        int i = 0;
        for (i = 0; i < int(fp_len - 1); i++)
        {
            sprintf(p, "%02x:", unsigned(fp[i]));
            p = &fp_buffer[(i + 1) * 3];
        }
        sprintf(p, "%02x", unsigned(fp[i]));

        return fp_buffer;
    }

    bool enable_client_tls_start(int sck, std::string* error_message)
    {
        SSL_CTX* ctx = SSL_CTX_new(SSLv23_client_method());

        if (ctx == nullptr) {
            return tls_ctx_print_error("SSL_CTX_new returned NULL", error_message);
        }

        this->allocated_ctx = ctx;

        /*
         * This is necessary, because the Microsoft TLS implementation is not perfect.
         * SSL_OP_ALL enables a couple of workarounds for buggy TLS implementations,
         * but the most important workaround being SSL_OP_TLS_BLOCK_PADDING_BUG.
         * As the size of the encrypted payload may give hints about its contents,
         * block padding is normally used, but the Microsoft TLS implementation
         * won't recognize it and will disconnect you after sending a TLS alert.
         */

        // LOG(LOG_INFO, "TLSContext::SSL_CTX_set_options()");
        SSL_CTX_set_options(ctx, SSL_OP_ALL);

        SSL* ssl = SSL_new(ctx);

        if (ctx == nullptr) {
            return tls_ctx_print_error("SSL_new returned NULL", error_message);
        }

        this->allocated_ssl = ssl;

        if (0 == SSL_set_fd(ssl, sck)) {
            return tls_ctx_print_error("SSL_set_fd failed", error_message);
        }

        LOG(LOG_INFO, "SSL_connect()");

        return true;
    }

    Transport::TlsResult enable_client_tls_loop(std::string* error_message)
    {
        int const connection_status = SSL_connect(this->allocated_ssl);
        if (connection_status <= 0) {
            char const* error_msg;
            switch (SSL_get_error(this->allocated_ssl, connection_status))
            {
                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                    return Transport::TlsResult::Want;
                case SSL_ERROR_ZERO_RETURN:
                    error_msg = "Server closed TLS connection";
                    break;
                case SSL_ERROR_SYSCALL:
                    error_msg = "I/O error";
                    break;
                case SSL_ERROR_SSL:
                    error_msg = "Failure in SSL library (protocol error?)";
                    break;
                default:
                    error_msg = "Unknown error";
                    break;
            }
            tls_ctx_print_error(error_msg, error_message);
            return Transport::TlsResult::Fail;
        }

        return Transport::TlsResult::Ok;
    }

    Transport::TlsResult check_certificate(
        bool server_cert_store,
        bool ensure_server_certificate_match,
        bool ensure_server_certificate_exists,
        ServerNotifier& server_notifier,
        const char* certif_path,
        std::string* error_message,
        const char* ip_address,
        int port)
    {
        LOG(LOG_INFO, "SSL_get_peer_certificate()");

        // SSL_get_peer_certificate - get the X509 certificate of the peer
        // ---------------------------------------------------------------

        // SSL_get_peer_certificate() returns a pointer to the X509 certificate
        // the peer presented. If the peer did not present a certificate, nullptr
        // is returned.

        // Due to the protocol definition, a TLS/SSL server will always send a
        // certificate, if present. A client will only send a certificate when
        // explicitly requested to do so by the server (see SSL_CTX_set_verify(3)).
        // If an anonymous cipher is used, no certificates are sent.

        // That a certificate is returned does not indicate information about the
        // verification state, use SSL_get_verify_result(3) to check the verification
        // state.

        // The reference count of the X509 object is incremented by one, so that
        // it will not be destroyed when the session containing the peer certificate
        // is freed. The X509 object must be explicitly freed using X509_free().

        // RETURN VALUES The following return values can occur:

        // nullptr : no certificate was presented by the peer or no connection was established.
        // Pointer to an X509 certificate : the return value points to the certificate
        // presented by the peer.

        X509 * px509 = SSL_get_peer_certificate(this->allocated_ssl);
        if (!px509) {
            LOG(LOG_WARNING, "SSL_get_peer_certificate() failed");
            server_notifier.server_cert_error(strerror(errno));
            return Transport::TlsResult::Fail;
        }

        // TODO("Before to have default value certificate doesn't exists")
        bool bad_certificate_path = false;
        error_type checking_exception = NO_ERROR;

        // ensures the certificate directory exists
        if (recursive_create_directory(certif_path, S_IRWXU|S_IRWXG, -1) != 0) {
            LOG(LOG_WARNING, "Failed to create certificate directory: %s ", certif_path);
            if (error_message) {
                str_assign(*error_message, "Failed to create certificate directory: \"", certif_path, "\"\n");
            }
            bad_certificate_path = true;

            server_notifier.server_cert_error(strerror(errno));
            checking_exception = ERR_TRANSPORT_TLS_CERTIFICATE_INACCESSIBLE;
        }

        char filename[1024];

        // generates the name of certificate file associated with RDP target
        snprintf(filename, sizeof(filename) - 1, "%s/rdp,%s,%d,X509.pem",
            certif_path, ip_address, port);
        filename[sizeof(filename) - 1] = '\0';

        bool certificate_exists  = false;
        bool certificate_matches = false;

        if (!bad_certificate_path) {
            File fp(filename, "r");
            if (!fp) {
                switch (errno) {
                default: {
                    // failed to open stored certificate file
                    LOG(LOG_WARNING, "Failed to open stored certificate: \"%s\"", filename);
                    if (error_message) {
                        str_assign(*error_message, "Failed to open stored certificate: \"", filename, "\"\n");
                    }
                    server_notifier.server_cert_error(strerror(errno));
                    checking_exception = ERR_TRANSPORT_TLS_CERTIFICATE_INACCESSIBLE;
                }
                break;
                case ENOENT:
                {
                    LOG(LOG_WARNING, "There's no stored certificate: \"%s\"", filename);
                    if (error_message) {
                        str_assign(*error_message, "There's no stored certificate: \"", filename, "\"\n");
                    }

                    if (ensure_server_certificate_exists) {
                        server_notifier.server_cert_failure();
                        checking_exception = ERR_TRANSPORT_TLS_CERTIFICATE_MISSED;
                    }
                }
                break;
                }
            } // fp
            else {
                certificate_exists  = true;
                X509 *px509Existing = PEM_read_X509(fp.get(), nullptr, nullptr, nullptr);
                fp.close();
                if (!px509Existing) {
                    // failed to read stored certificate file
                    LOG(LOG_WARNING, "Failed to read stored certificate: \"%s\"", filename);
                    if (error_message) {
                        str_assign(*error_message, "Failed to read stored certificate: \"", filename, "\"\n");
                    }
                    certificate_matches = false;

                    server_notifier.server_cert_error(strerror(errno));
                    checking_exception = ERR_TRANSPORT_TLS_CERTIFICATE_CORRUPTED;
                }
                else  {
                    char tmpfilename[1024];
                    // temp file for certificate binary check
                    snprintf(tmpfilename, sizeof(tmpfilename) - 1, "/tmp/rdp,%s,%d,X509,XXXXXX", ip_address, port);
                    tmpfilename[sizeof(tmpfilename) - 1] = 0;
                    int tmpfd = ::mkostemp(tmpfilename, O_RDWR|O_CREAT);
                    PEM_write_X509(File(::fdopen(tmpfd, "w+")).get(), px509);

                    certificate_matches = file_equals(filename, tmpfilename);
                    ::unlink(tmpfilename);

                    const std::unique_ptr<char[]> issuer_existing      = this->crypto_print_name(X509_get_issuer_name(px509Existing));
                    const std::unique_ptr<char[]> subject_existing     = this->crypto_print_name(X509_get_subject_name(px509Existing));
                    const std::unique_ptr<char[]> fingerprint_existing = this->crypto_cert_fingerprint(px509Existing);

                    LOG(LOG_INFO, "TLS::X509 existing::issuer=%s", issuer_existing.get());
                    LOG(LOG_INFO, "TLS::X509 existing::subject=%s", subject_existing.get());
                    LOG(LOG_INFO, "TLS::X509 existing::fingerprint=%s", fingerprint_existing.get());

                    const std::unique_ptr<char[]> issuer               = this->crypto_print_name(X509_get_issuer_name(px509));
                    const std::unique_ptr<char[]> subject              = this->crypto_print_name(X509_get_subject_name(px509));
                    const std::unique_ptr<char[]> fingerprint          = this->crypto_cert_fingerprint(px509);

                    if (!certificate_matches
                        // Read certificate fields to ensure change is not irrelevant
                        // Relevant changes are either:
                        // - issuer changed
                        // - subject changed (or appears, or disappears)
                        // - fingerprint changed
                        // other changes are ignored (expiration date for instance,
                        //  and revocation list is not checked)
                        && ((0 != strcmp(issuer_existing.get(), issuer.get()))
                        // Only one of subject_existing and subject is null
                        || ((!subject_existing || !subject) && (subject_existing != subject))
                        // All of subject_existing and subject are not null
                        || (subject && (0 != strcmp(subject_existing.get(), subject.get())))
                        || (0 != strcmp(fingerprint_existing.get(), fingerprint.get())))) {
                        if (error_message) {
                            char buff[256];
                            snprintf(buff, sizeof(buff), "The certificate for host %s:%d has changed!",
                                     ip_address, port);
                            *error_message = buff;
                        }
                        LOG(LOG_WARNING, "The certificate for host %s:%d has changed Previous=\"%s\" \"%s\" \"%s\", New=\"%s\" \"%s\" \"%s\"",
                            ip_address, port,
                            issuer_existing.get(), subject_existing.get(),
                            fingerprint_existing.get(), issuer.get(),
                            subject.get(), fingerprint.get());
                        if (error_message) {
                            str_assign(*error_message, "The certificate has changed: \"", filename, "\"\n");
                        }
                        certificate_exists  = true;
                        certificate_matches = false;

                        if (ensure_server_certificate_match) {
                            server_notifier.server_cert_failure();
                            checking_exception = ERR_TRANSPORT_TLS_CERTIFICATE_CHANGED;
                        }
                    }
                    else {
                        server_notifier.server_cert_success();
                    }

                    X509_free(px509Existing);
                }
            }
        }

        if ((certificate_exists || !ensure_server_certificate_exists)
        && (!certificate_exists || certificate_matches || !ensure_server_certificate_match))
        {
            if ((!certificate_exists || !certificate_matches) && server_cert_store) {
                ::unlink(filename);

                LOG(LOG_INFO, "Dumping X509 peer certificate: \"%s\"", filename);
                if (File fp{filename, "w+"}) {
                    PEM_write_X509(fp.get(), px509);
                    fp.close();
                    LOG(LOG_INFO, "Dumped X509 peer certificate");
                    server_notifier.server_cert_create();
                }
                else {
                    LOG(LOG_WARNING, "Failed to dump X509 peer certificate");
                }
            }

            server_notifier.server_access_allowed();

            // SSL_get_verify_result();

            // SSL_get_verify_result - get result of peer certificate verification
            // -------------------------------------------------------------------
            // SSL_get_verify_result() returns the result of the verification of the X509 certificate
            // presented by the peer, if any.

            // SSL_get_verify_result() can only return one error code while the verification of
            // a certificate can fail because of many reasons at the same time. Only the last
            // verification error that occurred during the processing is available from
            // SSL_get_verify_result().

            // The verification result is part of the established session and is restored when
            // a session is reused.

            // bug: If no peer certificate was presented, the returned result code is X509_V_OK.
            // This is because no verification error occurred, it does however not indicate
            // success. SSL_get_verify_result() is only useful in connection with SSL_get_peer_certificate(3).

            // RETURN VALUES The following return values can currently occur:

            // X509_V_OK: The verification succeeded or no peer certificate was presented.
            // Any other value: Documented in verify(1).


            //  A X.509 certificate is a structured grouping of information about an individual,
            // a device, or anything one can imagine. A X.509 CRL (certificate revocation list)
            // is a tool to help determine if a certificate is still valid. The exact definition
            // of those can be found in the X.509 document from ITU-T, or in RFC3280 from PKIX.
            // In OpenSSL, the type X509 is used to express such a certificate, and the type
            // X509_CRL is used to express a CRL.

            // A related structure is a certificate request, defined in PKCS#10 from RSA Security,
            // Inc, also reflected in RFC2896. In OpenSSL, the type X509_REQ is used to express
            // such a certificate request.

            // To handle some complex parts of a certificate, there are the types X509_NAME
            // (to express a certificate name), X509_ATTRIBUTE (to express a certificate attributes),
            // X509_EXTENSION (to express a certificate extension) and a few more.

            // Finally, there's the supertype X509_INFO, which can contain a CRL, a certificate
            // and a corresponding private key.


            LOG(LOG_INFO, "TLSContext::X509_get_pubkey()");
            // extract the public key
            EVP_PKEY* pkey = X509_get_pubkey(px509);
            if (!pkey)
            {
                LOG(LOG_WARNING, "TLSContext::crypto_cert_get_public_key: X509_get_pubkey() failed");
                return Transport::TlsResult::Fail;
            }

            LOG(LOG_INFO, "TLSContext::i2d_PublicKey()");

            // i2d_X509() encodes the structure pointed to by x into DER format.
            // If out is not nullptr is writes the DER encoded data to the buffer at *out,
            // and increments it to point after the data just written.
            // If the return value is negative an error occurred, otherwise it returns
            // the length of the encoded data.

            // export the public key to DER format
            this->public_key_length = i2d_PublicKey(pkey, nullptr);
            this->public_key = std::make_unique<uint8_t[]>(this->public_key_length);
            LOG(LOG_INFO, "TLSContext::i2d_PublicKey()");
            // hexdump_c(this->public_key, this->public_key_length);

            {
                uint8_t * tmp = this->public_key.get();
                i2d_PublicKey(pkey, &tmp);
            }

            EVP_PKEY_free(pkey);


            X509* xcert = px509;
            X509_STORE* cert_ctx = X509_STORE_new();

            // OpenSSL_add_all_algorithms(3SSL)
            // --------------------------------

            // OpenSSL keeps an internal table of digest algorithms and ciphers. It uses this table
            // to lookup ciphers via functions such as EVP_get_cipher_byname().

            // OpenSSL_add_all_digests() adds all digest algorithms to the table.

            // OpenSSL_add_all_algorithms() adds all algorithms to the table (digests and ciphers).

            // OpenSSL_add_all_ciphers() adds all encryption algorithms to the table including password
            // based encryption algorithms.

            // EVP_cleanup() removes all ciphers and digests from the table.

            OpenSSL_add_all_algorithms();

            X509_LOOKUP* lookup = X509_STORE_add_lookup(cert_ctx, X509_LOOKUP_file());
            lookup = X509_STORE_add_lookup(cert_ctx, X509_LOOKUP_hash_dir());

            X509_LOOKUP_add_dir(lookup, nullptr, X509_FILETYPE_DEFAULT);
            //X509_LOOKUP_add_dir(lookup, certificate_store_path, X509_FILETYPE_ASN1);

            X509_STORE_CTX* csc = X509_STORE_CTX_new();
            X509_STORE_set_flags(cert_ctx, 0);
            X509_STORE_CTX_init(csc, cert_ctx, xcert, nullptr);
            X509_verify_cert(csc);
            X509_STORE_CTX_free(csc);

            X509_STORE_free(cert_ctx);

        //        int index = X509_NAME_get_index_by_NID(subject_name, NID_commonName, -1);
        //        X509_NAME_ENTRY *entry = X509_NAME_get_entry(subject_name, index);
        //        ASN1_STRING * entry_data = X509_NAME_ENTRY_get_data(entry);
        //        void * subject_alt_names = X509_get_ext_d2i(xcert, NID_subject_alt_name, 0, 0);

           X509_NAME * issuer_name = X509_get_issuer_name(xcert);
           LOG(LOG_INFO, "TLS::X509::issuer=%s", this->crypto_print_name(issuer_name).get());

           X509_NAME * subject_name = X509_get_subject_name(xcert);
           LOG(LOG_INFO, "TLS::X509::subject=%s", this->crypto_print_name(subject_name).get());

           LOG(LOG_INFO, "TLS::X509::fingerprint=%s", this->crypto_cert_fingerprint(xcert).get());

           X509_free(px509);

           // TODO: Probably to be set by caller if everything successfull
           this->io = this->allocated_ssl;
           this->tls = true;
        }

        if ((!certificate_exists && ensure_server_certificate_exists)
         || ( certificate_exists && ensure_server_certificate_match && !certificate_matches)) {
            throw Error(checking_exception);
        }

        if (error_message) {
            error_message->clear();
        }

        LOG(LOG_INFO, "TLSContext::enable_client_tls() done");
        return Transport::TlsResult::Ok;
    }

    void enable_server_tls(int sck, const char * certificate_password, const char * ssl_cipher_list)
    {
        // SSL_CTX_new - create a new SSL_CTX object as framework for TLS/SSL enabled functions
        // ------------------------------------------------------------------------------------

        // SSL_CTX_new() creates a new SSL_CTX object as framework to establish TLS/SSL enabled
        // connections.

        // The SSL_CTX data structure is the global context structure which is created by a server
        // or client *once* per program life-time and which holds mainly default values for the SSL
        // structures which are later created for the connections.

        // Various options regarding certificates, algorithms, etc. can be set in this object.

        // SSL_CTX_new() receive a data structure of type SSL_METHOD (SSL Method), which is
        // a dispatch structure describing the internal ssl library methods/functions which
        // implement the various protocol versions (SSLv1, SSLv2 and TLSv1). An SSL_METHOD
        // is necessary to create an SSL_CTX.

        // The SSL_CTX object uses method as connection method. The methods exist in a generic
        // type (for client and server use), a server only type, and a client only type. method
        // can be of several types (server, client, generic, support SSLv2, TLSv1, TLSv1.1, etc.)

        // TLSv1_client_method(void): A TLS/SSL connection established with this methods will
        // only understand the TLSv1 protocol. A client will send out TLSv1 client hello messages
        // and will indicate that it only understands TLSv1.

        BIO * bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);

        SSL_CTX* ctx = SSL_CTX_new(SSLv23_server_method());
        this->allocated_ctx = ctx;

        /*
         * This is necessary, because the Microsoft TLS implementation is not perfect.
         * SSL_OP_ALL enables a couple of workarounds for buggy TLS implementations,
         * but the most important workaround being SSL_OP_TLS_BLOCK_PADDING_BUG.
         * As the size of the encrypted payload may give hints about its contents,
         * block padding is normally used, but the Microsoft TLS implementation
         * won't recognize it and will disconnect you after sending a TLS alert.
         */

        // SSL_CTX_set_options() adds the options set via bitmask in options to ctx.
        // Options already set before are not cleared!

         // During a handshake, the option settings of the SSL object are used. When
         // a new SSL object is created from a context using SSL_new(), the current
         // option setting is copied. Changes to ctx do not affect already created
         // SSL objects. SSL_clear() does not affect the settings.

         // The following bug workaround options are available:

         // SSL_OP_MICROSOFT_SESS_ID_BUG

         // www.microsoft.com - when talking SSLv2, if session-id reuse is performed,
         // the session-id passed back in the server-finished message is different
         // from the one decided upon.

         // SSL_OP_NETSCAPE_CHALLENGE_BUG

         // Netscape-Commerce/1.12, when talking SSLv2, accepts a 32 byte challenge
         // but then appears to only use 16 bytes when generating the encryption keys.
         // Using 16 bytes is ok but it should be ok to use 32. According to the SSLv3
         // spec, one should use 32 bytes for the challenge when operating in SSLv2/v3
         // compatibility mode, but as mentioned above, this breaks this server so
         // 16 bytes is the way to go.

         // SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG

         // As of OpenSSL 0.9.8q and 1.0.0c, this option has no effect.

        // SSL_OP_SSLREF2_REUSE_CERT_TYPE_BUG

        //  ...

        // SSL_OP_MICROSOFT_BIG_SSLV3_BUFFER

        // ...

        // SSL_OP_MSIE_SSLV2_RSA_PADDING

        // As of OpenSSL 0.9.7h and 0.9.8a, this option has no effect.

        // SSL_OP_SSLEAY_080_CLIENT_DH_BUG
        // ...

        // SSL_OP_TLS_D5_BUG
        //    ...

        // SSL_OP_TLS_BLOCK_PADDING_BUG
        //   ...

        // SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS

        // Disables a countermeasure against a SSL 3.0/TLS 1.0 protocol vulnerability
        // affecting CBC ciphers, which cannot be handled by some broken SSL implementations.
        // This option has no effect for connections using other ciphers.

        // SSL_OP_ALL
        // All of the above bug workarounds.

        // It is usually safe to use SSL_OP_ALL to enable the bug workaround options if
        // compatibility with somewhat broken implementations is desired.

        // The following modifying options are available:

        // SSL_OP_TLS_ROLLBACK_BUG

        // Disable version rollback attack detection.

        // During the client key exchange, the client must send the same information about
        // acceptable SSL/TLS protocol levels as during the first hello. Some clients violate
        // this rule by adapting to the server's answer. (Example: the client sends a SSLv2
        // hello and accepts up to SSLv3.1=TLSv1, the server only understands up to SSLv3.
        // In this case the client must still use the same SSLv3.1=TLSv1 announcement. Some
        // clients step down to SSLv3 with respect to the server's answer and violate the
        // version rollback protection.)

        // SSL_OP_SINGLE_DH_USE

        // Always create a new key when using temporary/ephemeral DH parameters (see
        // SSL_CTX_set_tmp_dh_callback(3)). This option must be used to prevent small subgroup
        // attacks, when the DH parameters were not generated using ``strong'' primes (e.g.
        // when using DSA-parameters, see dhparam(1)). If ``strong'' primes were used, it is
        // not strictly necessary to generate a new DH key during each handshake but it is
        // also recommended. SSL_OP_SINGLE_DH_USE should therefore be enabled whenever
        // temporary/ephemeral DH parameters are used.

        // SSL_OP_EPHEMERAL_RSA

        // Always use ephemeral (temporary) RSA key when doing RSA operations (see
        // SSL_CTX_set_tmp_rsa_callback(3)). According to the specifications this is only done,
        // when a RSA key can only be used for signature operations (namely under export ciphers
        // with restricted RSA keylength). By setting this option, ephemeral RSA keys are always
        // used. This option breaks compatibility with the SSL/TLS specifications and may lead
        // to interoperability problems with clients and should therefore never be used. Ciphers
        // with EDH (ephemeral Diffie-Hellman) key exchange should be used instead.

        // SSL_OP_CIPHER_SERVER_PREFERENCE

        // When choosing a cipher, use the server's preferences instead of the client preferences.
        // When not set, the SSL server will always follow the clients preferences. When set, the
        // SSLv3/TLSv1 server will choose following its own preferences. Because of the different
        // protocol, for SSLv2 the server will send its list of preferences to the client and the
        // client chooses.

        // SSL_OP_PKCS1_CHECK_1
        //  ...

        // SSL_OP_PKCS1_CHECK_2
        //  ...

        // SSL_OP_NETSCAPE_CA_DN_BUG
        // If we accept a netscape connection, demand a client cert, have a non-this-signed CA
        // which does not have its CA in netscape, and the browser has a cert, it will crash/hang.
        // Works for 3.x and 4.xbeta

        // SSL_OP_NETSCAPE_DEMO_CIPHER_CHANGE_BUG
        //    ...

        // SSL_OP_NO_SSLv2
        // Do not use the SSLv2 protocol.

        // SSL_OP_NO_SSLv3
        // Do not use the SSLv3 protocol.

        // SSL_OP_NO_TLSv1

        // Do not use the TLSv1 protocol.
        // SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION

        // When performing renegotiation as a server, always start a new session (i.e., session
        // resumption requests are only accepted in the initial handshake). This option is not
        // needed for clients.

        // SSL_OP_NO_TICKET
        // Normally clients and servers will, where possible, transparently make use of RFC4507bis
        // tickets for stateless session resumption.

        // If this option is set this functionality is disabled and tickets will not be used by
        // clients or servers.

        // SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION

        // Allow legacy insecure renegotiation between OpenSSL and unpatched clients or servers.
        // See the SECURE RENEGOTIATION section for more details.

        // SSL_OP_LEGACY_SERVER_CONNECT
        // Allow legacy insecure renegotiation between OpenSSL and unpatched servers only: this option
        // is currently set by default. See the SECURE RENEGOTIATION section for more details.

        LOG(LOG_INFO, "TLSContext::enable_server_tls() set SSL options");
        SSL_CTX_set_options(ctx, SSL_OP_ALL);
        SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
        SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv3);

//        LOG(LOG_INFO, "TLSContext::SSL_CTX_set_ciphers(HIGH:!ADH:!3DES)");
//        SSL_CTX_set_cipher_list(ctx, "ALL:!aNULL:!eNULL:!ADH:!EXP");
// Not compatible with MSTSC 6.1 on XP and W2K3
//        SSL_CTX_set_cipher_list(ctx, "HIGH:!ADH:!3DES");
        if (ssl_cipher_list && *ssl_cipher_list) {
            LOG(LOG_INFO, "TLSContext::enable_server_tls() set SSL cipher list");
            SSL_CTX_set_cipher_list(ctx, ssl_cipher_list);
        }

        // -------- End of system wide SSL_Ctx option ----------------------------------

        // --------Start of session specific init code ---------------------------------

        /* Load our keys and certificates*/
        if(!(SSL_CTX_use_certificate_chain_file(ctx, app_path(AppPath::CfgCrt))))
        {
            BIO_printf(bio_err, "Can't read certificate file\n");
            ERR_print_errors(bio_err);
            exit(128);
        }

        SSL_CTX_set_default_passwd_cb(
            ctx, [](char *buf, int num, int rwflag, void *userdata) {
                (void)rwflag;
                const char * pass = static_cast<const char*>(userdata);
                size_t pass_len = strlen(pass);
                if(num < static_cast<int>(pass_len+1u)) {
                    return 0;
                }

                memcpy(buf, pass, pass_len);
                buf[pass_len] = 0;
                return int(pass_len);
            }
        );
        SSL_CTX_set_default_passwd_cb_userdata(ctx, const_cast<void*>(static_cast<const void*>(certificate_password))); /*NOLINT*/
        if(!(SSL_CTX_use_PrivateKey_file(ctx, app_path(AppPath::CfgKey), SSL_FILETYPE_PEM)))
        {
            BIO_printf(bio_err,"Can't read key file\n");
            ERR_print_errors(bio_err);
            exit(129);
        }

        BIO *bio = BIO_new_file(app_path(AppPath::CfgDhPem), "r");
        if (bio == nullptr){
            BIO_printf(bio_err,"Couldn't open DH file\n");
            ERR_print_errors(bio_err);
            exit(130);
        }

        DH *ret = PEM_read_bio_DHparams(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        if(SSL_CTX_set_tmp_dh(ctx, ret) < 0) /*NOLINT*/
        {
            BIO_printf(bio_err,"Couldn't set DH parameters\n");
            ERR_print_errors(bio_err);
            exit(131);
        }
        DH_free(ret);
        // SSL_new() creates a new SSL structure which is needed to hold the data for a TLS/SSL
        // connection. The new structure inherits the settings of the underlying context ctx:
        // - connection method (SSLv2/v3/TLSv1),
        // - options,
        // - verification settings,
        // - timeout settings.

        // return value: nullptr: The creation of a new SSL structure failed. Check the error stack
        // to find out the reason.
        // TODO add error management
        BIO * sbio = BIO_new_socket(sck, BIO_NOCLOSE);
        SSL * ssl = SSL_new(ctx);
        this->allocated_ssl = ssl;

        // get public_key
        {
            X509 * px509 = SSL_get_certificate(this->allocated_ssl);
            LOG(LOG_INFO, "TLSContext::X509_get_pubkey()");
            // extract the public key
            EVP_PKEY* pkey = X509_get_pubkey(px509);
            if (!pkey)
            {
                LOG(LOG_WARNING, "TLSContext::crypto_cert_get_public_key: X509_get_pubkey() failed");
                exit(132);
            }

            LOG(LOG_INFO, "TLSContext::i2d_PublicKey()");

            // export the public key to DER format
            this->public_key_length = i2d_PublicKey(pkey, nullptr);
            this->public_key = std::make_unique<uint8_t[]>(this->public_key_length);
            LOG(LOG_INFO, "TLSContext::i2d_PublicKey()");
            // hexdump_c(this->public_key, this->public_key_length);

            {
                uint8_t * tmp = this->public_key.get();
                i2d_PublicKey(pkey, &tmp);
            }

            EVP_PKEY_free(pkey);
        }

        SSL_set_bio(ssl, sbio, sbio);

        int r = SSL_accept(ssl);
        if(r <= 0)
        {
            BIO_printf(bio_err, "SSL accept error\n");
            ERR_print_errors(bio_err);
            exit(133);
        }

        this->io = ssl;
        this->tls = true;

        BIO_free(bio_err);
        LOG(LOG_INFO, "TLSContext::enable_server_tls() done");
    }

    ssize_t privpartial_recv_tls(uint8_t * data, size_t len)
    {
        for (;;) {
            int rcvd = ::SSL_read(this->io, data, len);
            if (rcvd > 0) {
                return rcvd;
            }
            unsigned long error = SSL_get_error(this->io, rcvd);
            switch (error) {
                case SSL_ERROR_NONE:
                    return rcvd;

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
                    return 0;

                default:
                {
                    do {
                        LOG(LOG_INFO, "partial_recv_tls %s", ERR_error_string(error, nullptr));
                    } while ((error = ERR_get_error()) != 0);

                    // TODO if recv fail with partial read we should return the amount of data received, close socket and store some delayed error value that will be sent back next call
                    // TODO replace this with actual error management, EOF is not even an option for sockets
                    // TODO Manage actual errors, check possible values
                    return -1;
                }
            }
        }
    }

    ssize_t privpartial_send_tls(const uint8_t * data, size_t len)
    {
        const uint8_t * const buffer = data;
        size_t remaining_len = len;
        for (;;){
            int ret = SSL_write(this->io, buffer, remaining_len);
            if (ret > 0) {
                return ret;
            }
            unsigned long error = SSL_get_error(this->io, ret);
            switch (error)
            {
                case SSL_ERROR_NONE:
                    return ret;

                case SSL_ERROR_WANT_READ:
                    LOG(LOG_INFO, "send_tls WANT READ");
                    continue;

                case SSL_ERROR_WANT_WRITE:
                    LOG(LOG_INFO, "send_tls WANT WRITE");
                    continue;

                default:
                {
                    LOG(LOG_INFO, "Failure in SSL library, error=%lu, %s [%d]", error, strerror(errno), errno);
                    do {
                        LOG(LOG_INFO, "partial_send_tls %s", ERR_error_string(error, nullptr));
                    } while ((error = ERR_get_error()) != 0);
                    return -1;
                }
            }
        }
    }

    ssize_t privsend_tls(const uint8_t * data, size_t len)
    {
        const uint8_t * const buffer = data;
        size_t remaining_len = len;
        size_t offset = 0;
        while (remaining_len > 0){
            int ret = SSL_write(this->io, buffer + offset, remaining_len);

            unsigned long error = SSL_get_error(this->io, ret);
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
                    LOG(LOG_INFO, "Failure in SSL library, error=%lu, %s [%d]", error, strerror(errno), errno);
                    do {
                        LOG(LOG_INFO, "send_tls %s", ERR_error_string(error, nullptr));
                    } while ((error = ERR_get_error()) != 0);
                    return -1;
                }
            }
        }
        return len;
    }

};

REDEMPTION_DIAGNOSTIC_POP
