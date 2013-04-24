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
   Author(s): Christophe Grosjean, Raphael Zhou

   Transport layer abstraction, socket implementation with TLS support
*/

#ifndef _REDEMPTION_TRANSPORT_SOCKETTRANSPORT_HPP_
#define _REDEMPTION_TRANSPORT_SOCKETTRANSPORT_HPP_

#include "transport.hpp"
#include "rio/rio.h"

    // X509_NAME_print_ex() prints a human readable version of nm to BIO out.
    // Each line (for multiline formats) is indented by indent spaces.
    // The output format can be extensively customised by use of the flags parameter.

    TODO("we should be able to simplify that to just put expected value in a provided buffer");
    static inline char* crypto_print_name(X509_NAME* name)
    {
        char* buffer = NULL;
        BIO* outBIO = BIO_new(BIO_s_mem());

        if (X509_NAME_print_ex(outBIO, name, 0, XN_FLAG_ONELINE) > 0)
        {
            unsigned long size = BIO_number_written(outBIO);
            buffer = (char*)malloc(size + 1);
            memset(buffer, 0, size + 1);
            BIO_read(outBIO, buffer, size);
        }
        BIO_free(outBIO);
        return buffer;
    }

    static inline char* crypto_cert_fingerprint(X509* xcert)
    {
        uint32_t fp_len;
        uint8_t fp[EVP_MAX_MD_SIZE];

        X509_digest(xcert, EVP_sha1(), fp, &fp_len);

        char * fp_buffer = (char*) malloc(3 * fp_len);
        memset(fp_buffer, 0, 3 * fp_len);

        char * p = fp_buffer;

        int i = 0;
        for (i = 0; i < (int) (fp_len - 1); i++)
        {
            sprintf(p, "%02x:", fp[i]);
            p = &fp_buffer[(i + 1) * 3];
        }
        sprintf(p, "%02x", fp[i]);

        return fp_buffer;
    }


static inline int password_cb0(char *buf, int num, int rwflag, void *userdata)
{
    printf("password cb num=%u\n", num);
    const char * pass = (char*)userdata;
    if(num < (int)strlen(pass)+1){
      return(0);
    }

    strcpy(buf, pass);
    return strlen(pass);
}


class SocketTransport : public Transport {
    public:
        RIO rio;
        bool tls;
        int sck;
        int sck_closed;
        const char * name;
        uint32_t verbose;

        char ip_address[128];
        int  port;

        char * error_message_buffer;
        size_t error_message_len;

        

    SocketTransport(const char * name, int sck, const char *ip_address, int port, uint32_t verbose, char * error_message_buffer = NULL, size_t error_message_len = 0)
        : Transport(), tls(false), name(name), verbose(verbose), error_message_buffer(error_message_buffer), error_message_len(error_message_len)
    {
        RIO_ERROR res = rio_init_socket(&this->rio, sck);
        this->sck = sck;
        this->sck_closed = 0;
        if (res != RIO_ERROR_OK){ 
            throw Error(ERR_TRANSPORT, 0);
        }
        strncpy(this->ip_address, ip_address, 127);
        this->ip_address[127] = 0;

        this->port = port;
    }

    virtual ~SocketTransport(){
        if (!this->sck_closed){
            this->disconnect();
        }
    }

    virtual void enable_server_tls(const char * certificate_password) throw (Error)
    {
        if (this->tls) {
            TODO("this should be an error, no need to commute two times to TLS")
            return;
        }
        LOG(LOG_INFO, "RIO *::enable_server_tls() start");

        rio_clear(&this->rio);

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
        // If we accept a netscape connection, demand a client cert, have a non-self-signed CA
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

        LOG(LOG_INFO, "RIO *::SSL_CTX_set_options()");
        SSL_CTX_set_options(ctx, SSL_OP_ALL);

        // -------- End of system wide SSL_Ctx option ----------------------------------

        // --------Start of session specific init code ---------------------------------

        /* Load our keys and certificates*/
        if(!(SSL_CTX_use_certificate_chain_file(ctx, CFG_PATH "/rdpproxy.crt")))
        {
            BIO_printf(bio_err, "Can't read certificate file\n");
            ERR_print_errors(bio_err);
            exit(0);
        }

        SSL_CTX_set_default_passwd_cb(ctx, password_cb0);
//        SSL_CTX_set_default_passwd_cb_userdata(ctx, (void*)"inquisition");
LOG(LOG_INFO, "%s", certificate_password);
        SSL_CTX_set_default_passwd_cb_userdata(ctx, (void*)certificate_password);
        if(!(SSL_CTX_use_PrivateKey_file(ctx, CFG_PATH "/rdpproxy.key", SSL_FILETYPE_PEM)))
        {
            BIO_printf(bio_err,"Can't read key file\n");
            ERR_print_errors(bio_err);
            exit(0);
        }

        DH *ret=0;
        BIO *bio;

        if ((bio=BIO_new_file(CFG_PATH "/dh1024.pem","r")) == NULL){
            BIO_printf(bio_err,"Couldn't open DH file\n");
            ERR_print_errors(bio_err);
            exit(0);
        }

        ret=PEM_read_bio_DHparams(bio, NULL, NULL, NULL);
        BIO_free(bio);
        if(SSL_CTX_set_tmp_dh(ctx, ret)<0)
        {
            BIO_printf(bio_err,"Couldn't set DH parameters\n");
            ERR_print_errors(bio_err);
            exit(0);
        }

        // SSL_new() creates a new SSL structure which is needed to hold the data for a TLS/SSL
        // connection. The new structure inherits the settings of the underlying context ctx:
        // - connection method (SSLv2/v3/TLSv1),
        // - options,
        // - verification settings,
        // - timeout settings.

        // return value: NULL: The creation of a new SSL structure failed. Check the error stack
        // to find out the reason.
        TODO("add error management")
        BIO * sbio = BIO_new_socket(this->sck, BIO_NOCLOSE);
        SSL * ssl = SSL_new(ctx);

        TODO("I should probably not be doing that here ? Is it really necessary")
        int flags = fcntl(this->sck, F_GETFL);
        fcntl(this->sck, F_SETFL, flags & ~(O_NONBLOCK));

        SSL_set_bio(ssl, sbio, sbio);
        
        int r = SSL_accept(ssl);
        if(r <= 0)
        {
            BIO_printf(bio_err, "SSL accept error\n");
            ERR_print_errors(bio_err);
            exit(0);
        }

        RIO_ERROR res = rio_init_socket_tls(&this->rio, ssl);
        if (res != RIO_ERROR_OK){ 
            throw Error(ERR_TRANSPORT, 0);
        }
        this->tls = true;

        LOG(LOG_INFO, "RIO *::enable_server_tls() done");
        return;
    }

    virtual void enable_client_tls() throw (Error)
    {
        if (this->tls) {
            TODO("this should be an error, no need to commute two times to TLS")
            return;
        }
        LOG(LOG_INFO, "Client TLS start");
 
        rio_clear(&this->rio);


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

        SSL_CTX* ctx = SSL_CTX_new(TLSv1_client_method());

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
        // If we accept a netscape connection, demand a client cert, have a non-self-signed CA
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

//        LOG(LOG_INFO, "RIO *::SSL_CTX_set_options()");
        SSL_CTX_set_options(ctx, SSL_OP_ALL);

        // -------- End of system wide SSL_Ctx option ----------------------------------

        // --------Start of session specific init code ---------------------------------

        // SSL_new() creates a new SSL structure which is needed to hold the data for a TLS/SSL
        // connection. The new structure inherits the settings of the underlying context ctx:
        // - connection method (SSLv2/v3/TLSv1),
        // - options,
        // - verification settings,
        // - timeout settings.

        // return value: NULL: The creation of a new SSL structure failed. Check the error stack
        // to find out the reason.
        TODO("add error management")
        SSL * ssl = SSL_new(ctx);

        TODO("I should probably not be doing that here ? Is it really necessary")
        int flags = fcntl(this->sck, F_GETFL);
        fcntl(this->sck, F_SETFL, flags & ~(O_NONBLOCK));

        // SSL_set_fd - connect the SSL object with a file descriptor
        // ==========================================================

        // SSL_set_fd() sets the file descriptor fd as the input/output facility for the TLS/SSL (encrypted)
        // side of ssl. fd will typically be the socket file descriptor of a network connection.

        // When performing the operation, a socket BIO is automatically created to interface between the ssl
        // and fd. The BIO and hence the SSL engine inherit the behaviour of fd. If fd is non-blocking, the ssl
        // will also have non-blocking behaviour.

        // If there was already a BIO connected to ssl, BIO_free() will be called (for both the reading and
        // writing side, if different).

        // SSL_set_rfd() and SSL_set_wfd() perform the respective action, but only for the read channel or the
        //  write channel, which can be set independently.

        // The following return values can occur:
        // 0 : The operation failed. Check the error stack to find out why.
        // 1 : The operation succeeded.

        TODO("add error management")
        SSL_set_fd(ssl, this->sck);

        LOG(LOG_INFO, "SSL_connect()");
    again:
        // SSL_connect - initiate the TLS/SSL handshake with an TLS/SSL server
        // -------------------------------------------------------------------

        // SSL_connect() initiates the TLS/SSL handshake with a server. The
        // communication channel must already have been set and assigned to the
        // ssl by setting an underlying BIO.

        // The behaviour of SSL_connect() depends on the underlying BIO.

        // If the underlying BIO is blocking, SSL_connect() will only return once
        // the handshake has been finished or an error occurred.

        // If the underlying BIO is non-blocking, SSL_connect() will also return
        // when the underlying BIO could not satisfy the needs of SSL_connect()
        // to continue the handshake, indicating the problem by the return value -1.
        // In this case a call to SSL_get_error() with the return value of SSL_connect()
        // will yield SSL_ERROR_WANT_READ or SSL_ERROR_WANT_WRITE. The calling process
        // then must repeat the call after taking appropriate action to satisfy the needs
        // of SSL_connect(). The action depends on the underlying BIO. When using a
        // non-blocking socket, nothing is to be done, but select() can be used to check
        // for the required condition. When using a buffering BIO, like a BIO pair,
        // data must be written into or retrieved out of the BIO before being able to
        // continue.

        // RETURN VALUES, The following return values can occur:
        // - 0 : The TLS/SSL handshake was not successful but was shut down controlled
        // and by the specifications of the TLS/SSL protocol. Call SSL_get_error()
        // with the return value ret to find out the reason.
        // - 1 : The TLS/SSL handshake was successfully completed, a TLS/SSL connection
        // has been established.
        // - <0 : The TLS/SSL handshake was not successful, because a fatal error occurred
        // either at the protocol level or a connection failure occurred. The shutdown
        // was not clean. It can also occur if action is need to continue the operation
        // for non-blocking BIOs. Call SSL_get_error() with the return value ret to find
        // out the reason

        int connection_status = SSL_connect(ssl);

        if (connection_status <= 0)
        {
            unsigned long error;

            switch (SSL_get_error(ssl, connection_status))
            {
                case SSL_ERROR_ZERO_RETURN:
                    LOG(LOG_INFO, "Server closed TLS connection\n");
                    LOG(LOG_INFO, "tls::tls_print_error SSL_ERROR_ZERO_RETURN done\n");
                    return;

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
                    return;

                case SSL_ERROR_SSL:
                    LOG(LOG_INFO, "Failure in SSL library (protocol error?)\n");
                    while ((error = ERR_get_error()) != 0)
                        LOG(LOG_INFO, "%s\n", ERR_error_string(error, NULL));
                    LOG(LOG_INFO, "tls::tls_print_error SSL_ERROR_SSL done\n");
                    return;

                default:
                    LOG(LOG_INFO, "Unknown error\n");
                    while ((error = ERR_get_error()) != 0){
                        LOG(LOG_INFO, "%s\n", ERR_error_string(error, NULL));
                    }
                    LOG(LOG_INFO, "tls::tls_print_error %s [%u]", strerror(errno), errno);
                    LOG(LOG_INFO, "tls::tls_print_error Unknown error done\n");
                    return;
            }
        }

        LOG(LOG_INFO, "SSL_get_peer_certificate()");

        // SSL_get_peer_certificate - get the X509 certificate of the peer
        // ---------------------------------------------------------------

        // SSL_get_peer_certificate() returns a pointer to the X509 certificate
        // the peer presented. If the peer did not present a certificate, NULL
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

        // NULL : no certificate was presented by the peer or no connection was established.
        // Pointer to an X509 certificate : the return value points to the certificate
        // presented by the peer.

        X509 * px509 = SSL_get_peer_certificate(ssl);
        if (!px509) {
            LOG(LOG_INFO, "SSL_get_peer_certificate() failed");
            return;
        }


        // ensures the certificate directory exists
        if (recursive_create_directory(CERTIF_PATH "/", S_IRWXU|S_IRWXG, 0) != 0) {
            LOG(LOG_ERR, "Failed to create certificate directory: " CERTIF_PATH "/");
            strcpy(error_message_buffer, "Failed to create certificate directory: " CERTIF_PATH "/");
            throw Error(ERR_TRANSPORT, 0);
        }

        char filename[1024];

        // generates the name of certificate file associated with RDP target
        snprintf(filename, sizeof(filename) - 1, CERTIF_PATH "/X509-%s-%d.pem", this->ip_address, this->port);
        filename[sizeof(filename) - 1] = '\0';

        FILE *fp = ::fopen(filename, "r");

        if (!fp) {
            if (errno != ENOENT) {
                // failed to open stored certificate file
                LOG(LOG_ERR, "Failed to open stored certificate: \"%s\"\n", filename);
                sprintf(error_message_buffer, "Failed to open stored certificate: \"%s\"\n", filename);
                throw Error(ERR_TRANSPORT, 0);
            }
            else {
                LOG(LOG_INFO, "dumping X509 peer certificate\n");
                fp = ::fopen(filename, "w+");
                PEM_write_X509(fp, px509);
                ::fclose(fp);
                LOG(LOG_INFO, "dumped X509 peer certificate\n");
            }
        }
        else {
            X509 *px509Existing = PEM_read_X509(fp, NULL, NULL, NULL);
            if (!px509Existing) {
                // failed to read stored certificate file
                LOG(LOG_ERR, "Failed to read stored certificate: \"%s\"\n", filename);
                sprintf(error_message_buffer, "Failed to read stored certificate: \"%s\"\n", filename);
                throw Error(ERR_TRANSPORT, 0);
            }
            ::fclose(fp);

            char tmpfilename[1024];
            // temp file for certificate binary check
            snprintf(tmpfilename, sizeof(tmpfilename) - 1, "/tmp/X509-%s-%dXXXXXX", this->ip_address, this->port);
            tmpfilename[sizeof(tmpfilename) - 1] = 0;
            int tmpfd = ::mkostemp(tmpfilename, O_RDWR|O_CREAT);
            FILE * tmpfp = ::fdopen(tmpfd, "w+");
            PEM_write_X509(tmpfp, px509);

            ::fclose(tmpfp);

            fp = ::fopen(filename, "r");
            tmpfp = ::fopen(tmpfilename, "r");
            
//            ::rewind(fp);
//            ::rewind(tmpfp);

            char buffer1[2048];
            char buffer2[2048];
            int binary_check_failed = false;

            for (;;){
                size_t nb1 = fread(buffer1, sizeof(buffer1[0]), sizeof(buffer1)/sizeof(buffer1[0]), fp);
                size_t nb2 = fread(buffer2, sizeof(buffer2[0]), sizeof(buffer2)/sizeof(buffer1[1]), tmpfp);
                LOG(LOG_INFO, "nb1=%u nb2=%u\n", nb1, nb2);
                if ((nb1 != nb2) || (0 != memcmp(buffer1, buffer2, nb1 * sizeof(buffer1[0])))) {
                    binary_check_failed = true;
                    break;
                }
                if (feof(tmpfp) && feof(fp)){
                    break;
                } 
                if (ferror(tmpfp)||ferror(fp)||feof(tmpfp)||feof(fp)){
                    binary_check_failed = true;
                    break;
                }
            }
            ::fclose(tmpfp);
            ::unlink(tmpfilename);
            ::fclose(fp);

            char * issuer               = NULL;
            char * issuer_existing      = NULL;
            char * subject              = NULL;
            char * subject_existing     = NULL;
            char * fingerprint          = NULL;
            char * fingerprint_existing = NULL;

            issuer               = crypto_print_name(X509_get_issuer_name(px509));
            issuer_existing      = crypto_print_name(X509_get_issuer_name(px509Existing));
            subject              = crypto_print_name(X509_get_subject_name(px509));
            subject_existing     = crypto_print_name(X509_get_subject_name(px509Existing));
            fingerprint          = crypto_cert_fingerprint(px509);
            fingerprint_existing = crypto_cert_fingerprint(px509Existing);

            LOG(LOG_INFO, "TLS::X509 existing::issuer=%s", issuer_existing);
            LOG(LOG_INFO, "TLS::X509 existing::subject=%s", subject_existing);
            LOG(LOG_INFO, "TLS::X509 existing::fingerprint=%s", fingerprint_existing);

            if (binary_check_failed 
            || (0 != strcmp(issuer_existing, issuer)) 
            || (0 != strcmp(subject_existing, subject)) 
            || (0 != strcmp(fingerprint_existing, fingerprint))) {
                if (this->error_message_buffer && this->error_message_len) {
                    snprintf(this->error_message_buffer, this->error_message_len,
                        "The certificate for host %s:%d has changed\n\n"
                        "Previous Certificate\n"
                        "\tIssuer = \"%s\"\n"
                        "\tSubject = \"%s\"\n"
                        "\tFingerprint = \"%s\"\n\n"
                        "New Certificate\n"
                        "\tIssuer = \"%s\"\n"
                        "\tSubject = \"%s\"\n"
                        "\tFingerprint = \"%s\"\n",
                        this->ip_address, this->port,
                        issuer_existing, subject_existing, fingerprint_existing,
                        issuer, subject, fingerprint);
                    this->error_message_buffer[this->error_message_len - 1] = 0;
                }
                LOG(LOG_ERR, "The certificate for host %s:%d has changed Previous=\"%s\" \"%s\" \"%s\", New=\"%s\" \"%s\" \"%s\"\n",
                    this->ip_address, this->port,
                    issuer_existing, subject_existing, fingerprint_existing, issuer, subject, fingerprint);
                throw Error(ERR_TRANSPORT, 0);
            }

            if (issuer               != NULL) { free(issuer              ); }
            if (issuer_existing      != NULL) { free(issuer_existing     ); }
            if (subject              != NULL) { free(subject             ); }
            if (subject_existing     != NULL) { free(subject_existing    ); }
            if (fingerprint          != NULL) { free(fingerprint         ); }
            if (fingerprint_existing != NULL) { free(fingerprint_existing); }

            X509_free(px509Existing);
        }


//        SSL_get_verify_result();

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


        LOG(LOG_INFO, "RIO *::X509_get_pubkey()");
        // extract the public key
        EVP_PKEY* pkey = X509_get_pubkey(px509);
        if (!pkey)
        {
            LOG(LOG_INFO, "RIO *::crypto_cert_get_public_key: X509_get_pubkey() failed");
            return;
        }

        LOG(LOG_INFO, "RIO *::i2d_PublicKey()");

        // i2d_X509() encodes the structure pointed to by x into DER format.
        // If out is not NULL is writes the DER encoded data to the buffer at *out,
        // and increments it to point after the data just written. 
        // If the return value is negative an error occurred, otherwise it returns
        // the length of the encoded data. 

        // export the public key to DER format
        int public_key_length = i2d_PublicKey(pkey, NULL);
        uint8_t * public_key_data = (uint8_t *)malloc(public_key_length);
        LOG(LOG_INFO, "RIO *::i2d_PublicKey()");
        uint8_t * tmp = public_key_data;
        i2d_PublicKey(pkey, &tmp);


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

                                  X509_LOOKUP_add_dir(lookup, NULL, X509_FILETYPE_DEFAULT);
//                                  X509_LOOKUP_add_dir(lookup, certificate_store_path, X509_FILETYPE_ASN1);

                X509_STORE_CTX* csc = X509_STORE_CTX_new();
                                          X509_STORE_set_flags(cert_ctx, 0);
                                          X509_STORE_CTX_init(csc, cert_ctx, xcert, 0);
                                          X509_verify_cert(csc);
                                      X509_STORE_CTX_free(csc);

        X509_STORE_free(cert_ctx);

//        int index = X509_NAME_get_index_by_NID(subject_name, NID_commonName, -1);
//        X509_NAME_ENTRY *entry = X509_NAME_get_entry(subject_name, index);
//        ASN1_STRING * entry_data = X509_NAME_ENTRY_get_data(entry);
//        void * subject_alt_names = X509_get_ext_d2i(xcert, NID_subject_alt_name, 0, 0);

       X509_NAME * issuer_name = X509_get_issuer_name(xcert);
       char * issuer = crypto_print_name(issuer_name);
       LOG(LOG_INFO, "TLS::X509::issuer=%s", issuer);
       free(issuer);

       X509_NAME * subject_name = X509_get_subject_name(xcert);
       char * subject = crypto_print_name(subject_name);
       LOG(LOG_INFO, "TLS::X509::subject=%s", subject);
       free(subject);

       char * fingerprint = crypto_cert_fingerprint(xcert);
       LOG(LOG_INFO, "TLS::X509::fingerprint=%s", fingerprint);
       free(fingerprint);

       X509_free(px509);

       RIO_ERROR res = rio_init_socket_tls(&this->rio, ssl);
       if (res != RIO_ERROR_OK){ 
           throw Error(ERR_TRANSPORT, 0);
       }
       this->tls = true;

       LOG(LOG_INFO, "RIO *::enable_tls() done");
       return;
    }

    void disconnect(){
        rio_clear(&this->rio);
        LOG(LOG_INFO, "Socket %s (%d) : closing connection\n", this->name, this->sck);
        TODO("add code to disconnect TLS if needed")
        shutdown(this->sck, 2);
        close(this->sck);
        this->sck = 0;
        this->sck_closed = 1;
    }

    virtual bool connect()
    {
        if (this->sck_closed == 1){
            this->sck = ip_connect(this->ip_address,
                                    this->port,
                                    3, 1000,
                                    this->verbose);
            RIO_ERROR res = rio_init_socket(&this->rio, this->sck);
            if (res != RIO_ERROR_OK){ 
                throw Error(ERR_TRANSPORT, 0);
            }
            this->sck_closed = 0;
        }
        return true;
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error)
    {
//        if (this->verbose & 0x100){
//            LOG(LOG_INFO, "Socket %s (%u) receiving %u bytes", this->name, this->sck, total_len);
//        }

        ssize_t res = rio_recv(&this->rio, *pbuffer, len);
        if (res < 0){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
        }
        *pbuffer += res;

        if ((size_t)res < len){
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
        }

//        if (this->verbose & 0x100){
//            LOG(LOG_INFO, "Recv done on %s (%u) %u bytes", this->name, this->sck, total_len);
//            hexdump_c(start, total_len);
//            LOG(LOG_INFO, "Dump done on %s (%u) %u bytes", this->name, this->sck, total_len);
//        }

        this->total_received += len;
        this->last_quantum_received += len;
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error)
    {
        if (len == 0) { return; }
        ssize_t res = rio_send(&this->rio, buffer, len);
        if (res < 0) {
            throw Error(ERR_TRANSPORT_DIFFERS);
        }
        if (res < (ssize_t)len) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }

        this->total_sent += len;
        this->last_quantum_sent += len;
    }

    virtual bool get_status()
    {
        return rio_get_status(&this->rio) == RIO_ERROR_OK;
    }
};

#endif
