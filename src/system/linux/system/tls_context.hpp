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

inline void init_TLS()
{
        // init TLS

        // -------- Start of system wide SSL_Ctx option ------------------------------

        // ERR_load_crypto_strings() registers the error strings for all libcrypto
        // functions. SSL_load_error_strings() does the same, but also registers the
        // libssl error strings.

        // One of these functions should be called before generating textual error
        // messages. However, this is not required when memory usage is an issue.

        // ERR_free_strings() frees all previously loaded error strings.

        SSL_load_error_strings();

        // SSL_library_init() registers the available SSL/TLS ciphers and digests.
        // OpenSSL_add_ssl_algorithms() and SSLeay_add_ssl_algorithms() are synonyms
        // for SSL_library_init().

        // - SSL_library_init() must be called before any other action takes place.
        // - SSL_library_init() is not reentrant.
        // - SSL_library_init() always returns "1", so it is safe to discard the return
        // value.

        // Note: OpenSSL 0.9.8o and 1.0.0a and later added SHA2 algorithms to
        // SSL_library_init(). Applications which need to use SHA2 in earlier versions
        // of OpenSSL should call OpenSSL_add_all_algorithms() as well.

        SSL_library_init();
}

inline bool tls_ctx_print_error(char const* funcname, char const* error_msg, std::string* error_message)
{
    LOG(LOG_ERR, "TLSContext::%s: %s", funcname, error_msg);
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
    SSL_CTX * allocated_ctx = nullptr;
    SSL     * allocated_ssl = nullptr;
    SSL     * io = nullptr;
    std::unique_ptr<uint8_t[]> public_key;
    size_t public_key_length = 0;

    struct X509_deleter
    {
        void operator()(X509* px509) noexcept
        {
            X509_free(px509);
        }
    };
    using X509UniquePtr = std::unique_ptr<X509, X509_deleter>;
    X509UniquePtr cert_external_validation_wait_ctx;

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

    bool enable_client_tls_start(int sck, std::string* error_message, const TLSClientParams & tls_client_params)
    {
        SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());

        if (ctx == nullptr) {
            return tls_ctx_print_error("enable_client_tls", "SSL_CTX_new returned NULL", error_message);
        }

        // reference doc: https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_config.html


        this->allocated_ctx = ctx;
        SSL_CTX_set_mode(ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER/* | SSL_MODE_ENABLE_PARTIAL_WRITE*/);

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
        SSL_CTX_set_min_proto_version(ctx, tls_client_params.tls_min_level);
        if (tls_client_params.tls_max_level){
            SSL_CTX_set_max_proto_version(ctx, tls_client_params.tls_max_level);
        }

        // https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_set_ciphersuites.html
        // "DEFAULT@SEC_LEVEL=1"
        if (tls_client_params.cipher_string.size() > 0) { // if parameter is not defined, use system default
            LOG(LOG_INFO, "TLS Client cipher list: %s", tls_client_params.cipher_string.c_str());
            SSL_CTX_set_cipher_list(ctx, tls_client_params.cipher_string.c_str());
            SSL_CTX_set_security_level(ctx, 1);
        }

        SSL* ssl = SSL_new(ctx);

        if (ssl == nullptr) {
            return tls_ctx_print_error("enable_client_tls", "SSL_new returned NULL", error_message);
        }

        this->allocated_ssl = ssl;

        if (0 == SSL_set_fd(ssl, sck)) {
            return tls_ctx_print_error("enable_client_tls", "SSL_set_fd failed", error_message);
        }

        if (tls_client_params.show_common_cipher_list){
            int priority = 0;
            while(1){
                 const char * cipher_name = SSL_get_cipher_list(this->allocated_ssl, priority);
                 if (not cipher_name) { break; }
                 priority++;
                 LOG(LOG_INFO, "TLSContext::Client cipher %d: %s", priority, cipher_name);
            }
            if (priority == 0){
                 LOG(LOG_INFO, "TLSContext::Client negotiated cipher list empty");
            }
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
            tls_ctx_print_error("enable_client_tls", error_msg, error_message);
            return Transport::TlsResult::Fail;
        }

        return Transport::TlsResult::Ok;
    }

private:
    Transport::TlsResult final_check_certificate(X509& x509)
    {
        auto* px509 = &x509;

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
        // hexdump_c(this->public_key, this->public_key_length);

        {
            uint8_t * tmp = this->public_key.get();
            i2d_PublicKey(pkey, &tmp);
        }

        EVP_PKEY_free(pkey);

        this->io = this->allocated_ssl;

        // LOG(LOG_INFO, "TLSContext::enable_client_tls() done");
        LOG(LOG_INFO, "Connected to target using TLS version %s", SSL_get_version(this->allocated_ssl));

        return Transport::TlsResult::Ok;
    }

public:
    Transport::TlsResult certificate_external_validation(
        ServerNotifier& server_notifier,
        std::string* error_message,
        const char* ip_address,
        int port)
    {
        // local scope for exception and destruction
        std::unique_ptr px509 = std::exchange(this->cert_external_validation_wait_ctx, nullptr);
        switch (server_notifier.server_cert_callback(*px509, error_message, ip_address, port))
        {
            case CertificateResult::wait:
                this->cert_external_validation_wait_ctx = std::move(px509);
                return Transport::TlsResult::WaitExternalEvent;
            case CertificateResult::valid:
                return this->final_check_certificate(*px509);
            case CertificateResult::invalid:
                LOG(LOG_WARNING, "server_cert_callback() failed");
                return Transport::TlsResult::Fail;
        }

        REDEMPTION_UNREACHABLE();
    }

    Transport::TlsResult check_certificate(
        ServerNotifier& server_notifier,
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
            server_notifier.server_cert_status(ServerNotifier::Status::CertError, strerror(errno));
            return Transport::TlsResult::Fail;
        }

        X509UniquePtr x509_uptr{px509};

        switch (server_notifier.server_cert_callback(*px509, error_message, ip_address, port))
        {
            case CertificateResult::wait:
                this->cert_external_validation_wait_ctx = std::move(x509_uptr);
                return Transport::TlsResult::WaitExternalEvent;
            case CertificateResult::valid:
                return this->final_check_certificate(*px509);
            case CertificateResult::invalid:
                LOG(LOG_WARNING, "server_cert_callback() failed");
                return Transport::TlsResult::Fail;
        }

        return Transport::TlsResult::Fail;
    }

    bool enable_server_tls(int sck, const char * certificate_password, const char * ssl_cipher_list, uint32_t tls_min_level, uint32_t tls_max_level, bool show_common_cipher_list)
    {
        // reference doc: https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_new.html

        SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
        this->allocated_ctx = ctx;
        SSL_CTX_set_mode(ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER/* | SSL_MODE_ENABLE_PARTIAL_WRITE*/);

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

        SSL_CTX_set_min_proto_version(ctx, tls_min_level);

        if (tls_max_level){
            SSL_CTX_set_max_proto_version(ctx, tls_max_level);
        }

        // LOG(LOG_INFO, "TLSContext::SSL_CTX_set_ciphers(HIGH:!ADH:!3DES)");
        // SSL_CTX_set_cipher_list(ctx, "ALL:!aNULL:!eNULL:!ADH:!EXP");
        // Not compatible with MSTSC 6.1 on XP and W2K3
        // SSL_CTX_set_cipher_list(ctx, "HIGH:!ADH:!3DES");
        if (ssl_cipher_list && *ssl_cipher_list) {
            LOG(LOG_INFO, "TLSContext::enable_server_tls() set SSL cipher list");
            SSL_CTX_set_cipher_list(ctx, ssl_cipher_list);
        }

        // -------- End of system wide SSL_Ctx option ----------------------------------

        // --------Start of session specific init code ---------------------------------

        /* Load our keys and certificates*/
        if(!SSL_CTX_use_certificate_chain_file(ctx, app_path(AppPath::CfgCrt)))
        {
            return tls_ctx_print_error("enable_server_tls", "Can't read certificate file", nullptr);
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
        SSL_CTX_set_default_passwd_cb_userdata(ctx, const_cast<char*>(certificate_password)); /*NOLINT*/
        if(!SSL_CTX_use_PrivateKey_file(ctx, app_path(AppPath::CfgKey), SSL_FILETYPE_PEM))
        {
            return tls_ctx_print_error("enable_server_tls", "Can't read key file", nullptr);
        }

        BIO* bio = BIO_new_file(app_path(AppPath::CfgDhPem), "r");
        if (bio == nullptr){
            return tls_ctx_print_error("enable_server_tls", "Couldn't open DH file", nullptr);
        }

        DH* ret = PEM_read_bio_DHparams(bio, nullptr, nullptr, nullptr);
        BIO_free(bio);
        if(ret == nullptr)
        {
            return tls_ctx_print_error("enable_server_tls", "Can't read DH parameters", nullptr);
        }

        if(SSL_CTX_set_tmp_dh(ctx, ret) < 0) /*NOLINT*/
        {
            DH_free(ret);
            return tls_ctx_print_error("enable_server_tls", "Couldn't set DH parameters", nullptr);
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
        BIO* sbio = BIO_new_socket(sck, BIO_NOCLOSE);
        if (bio == nullptr){
            return tls_ctx_print_error("enable_server_tls", "Couldn't open socket", nullptr);
        }

        this->allocated_ssl = SSL_new(ctx);

        // get public_key
        {
            X509* px509 = SSL_get_certificate(this->allocated_ssl);
            LOG(LOG_INFO, "TLSContext::X509_get_pubkey()");
            // extract the public key
            EVP_PKEY* pkey = X509_get_pubkey(px509);
            if (!pkey)
            {
                tls_ctx_print_error("X509_get_pubkey()", "failed", nullptr);
                BIO_free(sbio);
                return false;
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

        SSL_set_bio(this->allocated_ssl, sbio, sbio);

        int r = SSL_accept(this->allocated_ssl);
        if(r <= 0)
        {
            return tls_ctx_print_error("enable_server_tls", "SSL accept error", nullptr);
        }
        this->io = this->allocated_ssl;

        LOG(LOG_INFO, "Incoming connection to Bastion using TLS version %s", SSL_get_version(this->allocated_ssl));

        if (show_common_cipher_list){
            int priority = 0;
            while(1){
                 const char * cipher_name = SSL_get_cipher_list(this->allocated_ssl, priority);
                 if (not cipher_name) { break; }
                 priority++;
                 LOG(LOG_INFO, "TLSContext::Server cipher %d: %s", priority, cipher_name);
            }
            if (priority == 0){
                 LOG(LOG_INFO, "TLSContext::Server cipher list empty");
            }
        }

        LOG(LOG_INFO, "TLSContext::Negociated cipher used %s", SSL_CIPHER_get_name(SSL_get_current_cipher(this->allocated_ssl)));

        return true;
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
                    LOG(LOG_INFO, "recv_tls SSL_ERROR_NONE");
                    return rcvd;

                case SSL_ERROR_WANT_WRITE:
                    LOG(LOG_INFO, "recv_tls WANT WRITE");
                    return 0;

                case SSL_ERROR_WANT_READ:
                    LOG(LOG_INFO, "recv_tls WANT READ");
                    return 0;

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
                    // Other side closed TLS connection
                    return -1;

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
                    return 0;

                case SSL_ERROR_WANT_WRITE:
                    LOG(LOG_INFO, "send_tls WANT WRITE");
                    return 0;

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
