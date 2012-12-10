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
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Transport layer abstraction
*/

#if !defined(__TRANSPORT_HPP__)
#define __TRANSPORT_HPP__

#include <sys/types.h> // recv, send
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/un.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include </usr/include/openssl/ssl.h>
#include </usr/include/openssl/err.h>

#include "error.hpp"
#include "log.hpp"


static inline int filesize(const char * path)
{
    struct stat sb;
    int status = stat(path, &sb);
    if (status >= 0){
        return sb.st_size;
    }
    return -1;
}

static inline void canonical_path(const char * fullpath, char * path, size_t path_len, char * basename, size_t basename_len)
{
    TODO("add parameters values for default path and basename. From inifile ?")
    TODO("add extraction of extension")
    TODO("add overflow checking of path and basename len")
    const char * end_of_path = strrchr(fullpath, '/');
    if (end_of_path){
        memcpy(path, fullpath, end_of_path + 1 - fullpath);
        path[end_of_path + 1 - fullpath] = 0;
        const char * start_of_extension = strrchr(end_of_path + 1, '.');
        if (start_of_extension){
            memcpy(basename, end_of_path + 1, start_of_extension - end_of_path - 1);
            basename[start_of_extension - end_of_path - 1] = 0;
        }
        else {
            if (end_of_path[0]){
                strcpy(basename, end_of_path + 1);
            }
	    else {
	      strcpy(basename, "no_name");
	    }
        }
    }
    else {
      strcpy(path, "./");
        const char * start_of_extension = strrchr(fullpath, '.');
        if (start_of_extension){
            memcpy(basename, fullpath, start_of_extension - fullpath);
            basename[start_of_extension - fullpath] = 0;
        }
        else {
            if (fullpath[0]){
                strcpy(basename, fullpath);
            }
	    else {
	        strcpy(basename, "no_name");
	    }
        }
    }
    LOG(LOG_INFO, "canonical_path : %s%s\n", path, basename);
}

class Transport {
public:
    timeval future;
    uint32_t seqno;
    uint64_t total_received;
    uint64_t last_quantum_received;
    uint64_t total_sent;
    uint64_t last_quantum_sent;
    uint64_t quantum_count;
    bool status;

    Transport() :
        seqno(0),
        total_received(0),
        last_quantum_received(0),
        total_sent(0),
        last_quantum_sent(0),
        quantum_count(0),
        status(true)
    {}

    virtual ~Transport() {}

    void tick() {
        quantum_count++;
        last_quantum_received = 0;
        last_quantum_sent = 0;
    }

    virtual void enable_tls()
    {
        // default enable_tls do nothing
    }

    void recv(uint8_t ** pbuffer, size_t len) throw (Error) {
        this->recv(reinterpret_cast<char **>(pbuffer), len);
    }
    virtual void recv(char ** pbuffer, size_t len) throw (Error) = 0;
    virtual void send(const char * const buffer, size_t len) throw (Error) = 0;
    void send(const uint8_t * const buffer, size_t len) throw (Error) {
        this->send(reinterpret_cast<const char * const>(buffer), len);
    }
    virtual void disconnect(){}
    virtual bool connect()
    {
        return true;
    }

    virtual void flush()
    {
    }

    virtual void timestamp(timeval now)
    {
        this->future = now;
    }

    virtual bool next()
    REDOC("Some transports are splitted between sequential discrete units"
          "(it may be block, chunk, numbered files, directory entries, whatever)."
          "Calling next means flushing the current unit and start the next one."
          "seqno countains the current sequence number, starting from 0.")
    {
        this->seqno++;
        return true;
    }

};

class GeneratorTransport : public Transport {

    public:
    size_t current;
    char * data;
    size_t len;


    GeneratorTransport(const char * data, size_t len)
        : Transport(), current(0), data(0), len(len)
    {
        this->data = (char *)malloc(len);
        memcpy(this->data, data, len);
    }

    void reset(const char * data, size_t len)
    {
        delete this->data;
        current = 0;
        this->len = len;
        this->data = (char *)malloc(len);
        memcpy(this->data, data, len);
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        if (current + len > this->len){
            size_t available_len = this->len - this->current;
            memcpy(*pbuffer, (const char *)(&this->data[this->current]),
                                            available_len);
            *pbuffer += available_len;
            this->current += available_len;
            LOG(LOG_INFO, "Generator transport has no more data");
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
        }
        memcpy(*pbuffer, (const char *)(&this->data[current]), len);
        *pbuffer += len;
        current += len;
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        // send perform like a /dev/null and does nothing in generator transport
    }
};

class CheckTransport : public Transport {

    public:
    size_t current;
    char * data;
    size_t len;
    uint32_t verbose;


    CheckTransport(const char * data, size_t len, uint32_t verbose = 0)
        : Transport(), current(0), data(0), len(len), verbose(verbose)
    {
        this->data = (char *)malloc(len);
        memcpy(this->data, data, len);
    }

    void reset(const char * data, size_t len)
    {
        delete this->data;
        current = 0;
        this->len = len;
        this->data = (char *)malloc(len);
        memcpy(this->data, data, len);
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        // CheckTransport does never receive anything
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        if (this->verbose & 0x100){
            LOG(LOG_INFO, "Check Transport (Test Data) sending %u bytes", len);
            hexdump_c(buffer, len);
            LOG(LOG_INFO, "Dump done (Test Data) sending %u bytes", len);
        }
        size_t available_len = (this->current + len > this->len)?this->len - this->current:len;
        if (0 != memcmp(buffer, (const char *)(&this->data[this->current]), available_len)){
            // data differs
            this->status = false;
            // find where
            uint32_t differs = 0;
            for (size_t i = 0; i < available_len ; i++){
                if (buffer[i] != ((const char *)(&this->data[this->current]))[i]){
                    differs = i;
                    break;
                }
            }
            LOG(LOG_INFO, "=============== Common Part =======");
            hexdump_c(buffer, differs);
            LOG(LOG_INFO, "=============== Expected ==========");
            hexdump((const char *)(&this->data[this->current]) + differs, available_len - differs);
            LOG(LOG_INFO, "=============== Got ===============");
            hexdump_c(buffer+differs, available_len - differs);
            throw Error(ERR_TRANSPORT_DIFFERS, 0);
        }
        this->current += available_len;
        if (available_len != len){
            LOG(LOG_INFO, "Check transport out of reference data available=%u len=%u", available_len, len);
            LOG(LOG_INFO, "=============== Unexpected Missing ==========");
            hexdump_c((const char *)(buffer + available_len), len - available_len);
            this->status = false;
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
        }
    }

    size_t remaining(){
        return this->len - this->current;
    }

};

class TestTransport : public Transport {

    GeneratorTransport out;
    CheckTransport in;
    public:
    char name[256];
    uint32_t verbose;

    TestTransport(const char * name, const char * outdata, size_t outlen, const char * indata, size_t inlen, uint32_t verbose = 0)
        : out(outdata, outlen), in(indata, inlen), verbose(verbose)
    {
        strncpy(this->name, name, 254);
        this->name[255]=0;
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        if (this->status){
            try {
                this->out.recv(pbuffer, len);
            } catch (const Error & e){
                this->status = this->out.status;
                throw;
            }
            this->status = this->out.status;
            if (this->verbose & 0x100){
                LOG(LOG_INFO, "Recv done on %s (Test Data) %u bytes", this->name, len);
                hexdump_c(*pbuffer - len, len);
                LOG(LOG_INFO, "Dump done on %s (Test Data) %u bytes", this->name, len);
            }
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        if (this->status){
            if (this->verbose & 0x100){
                LOG(LOG_INFO, "Test Transport %s (Test Data) sending %u bytes", this->name, len);
                hexdump_c(buffer, len);
                LOG(LOG_INFO, "Dump done %s (Test Data) sending %u bytes", this->name, len);
            }
            try {
                this->in.send(buffer, len);
                this->status = this->in.status;
            } catch (const Error & e){
                this->status = this->in.status;
                throw;
            }
        }
    }
};

class OutFileTransport : public Transport {
    public:
    int fd;
    uint32_t verbose;

    OutFileTransport(int fd, unsigned verbose = 0)
        : Transport()
        , fd(fd)
        , verbose(verbose) {}

    virtual ~OutFileTransport() {}

    // recv is not implemented for OutFileTransport
    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        LOG(LOG_INFO, "OutFileTransport used for recv");
        throw Error(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND, 0);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        if (this->verbose & 0x100){
            LOG(LOG_INFO, "File (%u) sending %u bytes", this->fd, len);
            hexdump_c(buffer, len);
        }
        ssize_t ret = 0;
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            ret = ::write(this->fd, buffer + total_sent, remaining_len);
            if (ret > 0){
                remaining_len -= ret;
                total_sent += ret;
            }
            else {
                if (errno == EINTR){
                    continue;
                }
                LOG(LOG_INFO, "Outfile transport write failed with error %s", strerror(errno));
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        if (this->verbose & 0x100){
            LOG(LOG_INFO, "File (%u) sent %u bytes", this->fd, len);
        }
    }

};

class InFileTransport : public Transport {

    public:
    int fd;

    InFileTransport(int fd)
        : Transport(), fd(fd)
    {
    }

    virtual ~InFileTransport()
    {
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        size_t ret = 0;
        size_t remaining_len = len;
        char * & buffer = *pbuffer;
        while (remaining_len) {
            ret = ::read(this->fd, buffer, remaining_len);
            if (ret > 0){
                remaining_len -= ret;
                buffer += ret;
            }
            else {
                if (errno == EINTR){
                    continue;
                }
                if (ret == 0){
                    throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
                }
                LOG(LOG_INFO, "Infile transport read failed with error %u %s ret=%u", errno, strerror(errno), ret);
                throw Error(ERR_TRANSPORT_READ_FAILED, 0);
            }
        }
    }

    // send is not implemented for InFileTransport
    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        LOG(LOG_INFO, "InFileTransport used for writing");
        throw Error(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV, 0);
    }

};

TODO("for now loop transport is not yet implemented, it's a null transport")

class LoopTransport : public Transport {
    public:
    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
    }
    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
    }
};


class SocketTransport : public Transport {
        bool tls;
        SSL * ssl;
    public:
        int sck;
        int sck_closed;
        const char * name;
        uint32_t verbose;

    SocketTransport(const char * name, int sck, uint32_t verbose)
        : Transport(), name(name), verbose(verbose)
    {
        this->ssl = NULL;
        this->tls = false;
        this->sck = sck;
        this->sck_closed = 0;
    }

    virtual ~SocketTransport(){
        if (!this->sck_closed){
            this->disconnect();
        }
    }


    virtual void enable_tls() throw (Error)
    {
        LOG(LOG_INFO, "Transport::enable_tls()");
        SSL_load_error_strings();
        SSL_library_init();

        LOG(LOG_INFO, "Transport::SSL_CTX_new()");
        SSL_CTX* ctx = SSL_CTX_new(TLSv1_client_method());

        /*
         * This is necessary, because the Microsoft TLS implementation is not perfect.
         * SSL_OP_ALL enables a couple of workarounds for buggy TLS implementations,
         * but the most important workaround being SSL_OP_TLS_BLOCK_PADDING_BUG.
         * As the size of the encrypted payload may give hints about its contents,
         * block padding is normally used, but the Microsoft TLS implementation
         * won't recognize it and will disconnect you after sending a TLS alert.
         */
        LOG(LOG_INFO, "Transport::SSL_CTX_set_options()");
        SSL_CTX_set_options(ctx, SSL_OP_ALL);
        LOG(LOG_INFO, "Transport::SSL_new()");
        this->ssl = SSL_new(ctx);

        int flags = fcntl(this->sck, F_GETFL);
        fcntl(this->sck, F_SETFL, flags & ~(O_NONBLOCK));

        LOG(LOG_INFO, "Transport::SSL_set_fd()");
        SSL_set_fd(this->ssl, this->sck);
        LOG(LOG_INFO, "Transport::SSL_connect()");
    again:
        int connection_status = SSL_connect(ssl);

        if (connection_status <= 0)
        {
            unsigned long error;

            switch (SSL_get_error(this->ssl, connection_status))
            {
                case SSL_ERROR_ZERO_RETURN:
                    LOG(LOG_INFO, "Server closed TLS connection\n");
                    LOG(LOG_INFO, "tls::tls_print_error SSL_ERROR_ZERO_RETURN done\n");
                    throw Error(ERR_TRANSPORT_TLS_CONNECT_FAILED, 0);
                    break;

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
                    throw Error(ERR_TRANSPORT_TLS_CONNECT_FAILED, 0);
                    break;

                case SSL_ERROR_SSL:
                    LOG(LOG_INFO, "Failure in SSL library (protocol error?)\n");
                    while ((error = ERR_get_error()) != 0)
                        LOG(LOG_INFO, "%s\n", ERR_error_string(error, NULL));
                    LOG(LOG_INFO, "tls::tls_print_error SSL_ERROR_SSL done\n");
                    throw Error(ERR_TRANSPORT_TLS_CONNECT_FAILED, 0);
                    break;

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

        LOG(LOG_INFO, "Transport::SSL_get_peer_certificate()");
        X509 * px509 = SSL_get_peer_certificate(ssl);
        if (!px509)
        {
            LOG(LOG_INFO, "Transport::crypto_cert_get_public_key: SSL_get_peer_certificate() failed");
            throw Error(ERR_TRANSPORT_TLS_CONNECT_FAILED, 0);
        }

        LOG(LOG_INFO, "Transport::X509_get_pubkey()");
        EVP_PKEY* pkey = X509_get_pubkey(px509);
        if (!pkey)
        {
            LOG(LOG_INFO, "Transport::crypto_cert_get_public_key: X509_get_pubkey() failed");
            throw Error(ERR_TRANSPORT_TLS_CONNECT_FAILED, 0);
        }

        LOG(LOG_INFO, "Transport::i2d_PublicKey()");
        int public_key_length = i2d_PublicKey(pkey, NULL);
        LOG(LOG_INFO, "Transport::i2d_PublicKey() -> length = %u", public_key_length);
        uint8_t * public_key_data = (uint8_t *)malloc(public_key_length);
        LOG(LOG_INFO, "Transport::i2d_PublicKey()");
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

        this->tls = true;
        LOG(LOG_INFO, "Transport::enable_tls() done");
    }

    static bool try_again(int errnum){
        int res = false;
        switch (errnum){
            case EAGAIN:
            /* case EWOULDBLOCK: */ // same as EAGAIN on Linux
            case EINPROGRESS:
            case EALREADY:
            case EBUSY:
            case EINTR:
                res = true;
                break;
            default:
                ;
        }
        return res;
    }

    void disconnect(){
        LOG(LOG_INFO, "Socket %s (%d) : closing connection\n", this->name, this->sck);
        if (this->sck != 0) {
            shutdown(this->sck, 2);
            close(this->sck);
        }
        this->sck = 0;
        this->sck_closed = 1;
    }

    enum direction_t {
        NONE = 0,
        RECV = 1,
        SEND = 2
    };

    using Transport::recv;

    virtual void recv(char ** pbuffer, size_t len) throw (Error)
    {
        if (this->tls){
            this->recv_tls(pbuffer, len);
        }
        else {
            this->recv_tcp(pbuffer, len);
        }
    }

    void recv_tls(char ** input_buffer, size_t total_len) throw (Error)
    {
        if (this->verbose & 0x100){
            LOG(LOG_INFO, "TLS Socket %s (%u) receiving %u bytes", this->name, this->sck, total_len);
        }
        char * start = *input_buffer;
        size_t len = total_len;
        char * pbuffer = *input_buffer;
        unsigned long error;

        if (this->sck_closed) {
            LOG(LOG_INFO, "TLS Socket %s (%u) already closed", this->name, this->sck);
            throw Error(ERR_SOCKET_ALLREADY_CLOSED);
        }

        while (len > 0) {
            ssize_t rcvd = ::SSL_read(this->ssl, pbuffer, len);
            switch (SSL_get_error(this->ssl, rcvd)) {
                case SSL_ERROR_NONE:
//                    LOG(LOG_INFO, "recv_tls ERROR NONE");
                    pbuffer += rcvd;
                    len -= rcvd;
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
                    LOG(LOG_INFO, "recv_tls ZERO RETURN");
                    LOG(LOG_INFO, "No data received. TLS Socket %s (%u) closed on recv", this->name, this->sck);
                    this->sck_closed = 1;
                    throw Error(ERR_SOCKET_CLOSED);
                    break;

                default:
                {
                    LOG(LOG_INFO, "Failure in SSL library");
                    uint32_t errcount = 0;
                    while ((error = ERR_get_error()) != 0){
                        errcount++;
                        LOG(LOG_INFO, "%s", ERR_error_string(error, NULL));
                    }
                    if (!errcount && rcvd == -1){
                        LOG(LOG_INFO, "%s [%u]", strerror(errno), errno);
                    }
                    LOG(LOG_INFO, "Closing socket %s (%u) on recv", this->name, this->sck);
                    this->sck_closed = 1;
                    throw Error(ERR_SOCKET_ERROR, errno);
                }
                break;
            }
        }

        if (this->verbose & 0x100){
            LOG(LOG_INFO, "Recv done on %s (%u) %u bytes", this->name, this->sck, total_len);
            hexdump_c(start, total_len);
            LOG(LOG_INFO, "Dump done on %s (%u) %u bytes", this->name, this->sck, total_len);
        }
        *input_buffer = pbuffer;
        total_received += total_len;
        last_quantum_received += total_len;
    }

    void recv_tcp(char ** input_buffer, size_t total_len) throw (Error)
    {
        if (this->verbose & 0x100){
            LOG(LOG_INFO, "Socket %s (%u) receiving %u bytes", this->name, this->sck, total_len);
        }
        char * start = *input_buffer;
        size_t len = total_len;
        char * pbuffer = *input_buffer;

        if (this->sck_closed) {
            LOG(LOG_INFO, "Socket %s (%u) already closed", this->name, this->sck);
            throw Error(ERR_SOCKET_ALLREADY_CLOSED);
        }

        while (len > 0) {
            ssize_t rcvd = ::recv(this->sck, pbuffer, len, 0);
            switch (rcvd) {
                case -1: /* error, maybe EAGAIN */
                    if (!this->try_again(errno)) {
                        LOG(LOG_INFO, "Closing socket %s (%u) on recv (%s)", this->name, this->sck, strerror(errno));
                        this->sck_closed = 1;
                        throw Error(ERR_SOCKET_ERROR, errno);
                    }
                    else {
                        fd_set fds;
                        struct timeval time = { 0, 100000 };
                        FD_ZERO(&fds);
                        FD_SET(this->sck, &fds);
                        select(this->sck + 1, &fds, NULL, NULL, &time);
                    }
                    break;
                case 0: /* no data received, socket closed */
                    LOG(LOG_INFO, "No data received. Socket %s (%u) closed on recv", this->name, this->sck);
                    this->sck_closed = 1;
                    throw Error(ERR_SOCKET_CLOSED);
                default: /* some data received */
                    pbuffer += rcvd;
                    len -= rcvd;
            }
        }

        if (this->verbose & 0x100){
            LOG(LOG_INFO, "Recv done on %s (%u) %u bytes", this->name, this->sck, total_len);
            hexdump_c(start, total_len);
            LOG(LOG_INFO, "Dump done on %s (%u) %u bytes", this->name, this->sck, total_len);
        }

        *input_buffer = pbuffer;
        total_received += total_len;
        last_quantum_received += total_len;
    }



    using Transport::send;

    virtual void send(const char * const buffer, size_t len) throw (Error)
    {
        if (this->tls){
            this->send_tls(buffer, len);
        }
        else {
            this->send_tcp(buffer, len);
        }
    }

    void send_tls(const char * const buffer, size_t len) throw (Error)
    {
        if (this->verbose & 0x100){
            LOG(LOG_INFO, "TLS Socket %s (%u) sending %u bytes", this->name, this->sck, len);
            hexdump_c(buffer, len);
            LOG(LOG_INFO, "TLS Dump done %s (%u) sending %u bytes", this->name, this->sck, len);
        }

        if (this->sck_closed) {
            LOG(LOG_INFO, "Socket already closed on %s (%u)", this->name, this->sck);
            throw Error(ERR_SOCKET_ALLREADY_CLOSED);
        }

        size_t offset = 0;
        while (len > 0){
            int ret = SSL_write(this->ssl, buffer + offset, len);

            unsigned long error;
            switch (SSL_get_error(this->ssl, ret))
            {
                case SSL_ERROR_NONE:
//                    LOG(LOG_INFO, "send_tls ERROR NONE ret=%u", ret);
                    total_sent += ret;
                    last_quantum_sent += ret;
                    len -= ret;
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
                    while ((error = ERR_get_error()) != 0){
                        errcount++;
                        LOG(LOG_INFO, "%s", ERR_error_string(error, NULL));
                    }
                    if (!errcount && ret == -1){
                        LOG(LOG_INFO, "%s [%u]", strerror(errno), errno);
                    }
                    LOG(LOG_INFO, "Closing socket %s (%u) on recv", this->name, this->sck);
                    this->sck_closed = 1;
                    throw Error(ERR_SOCKET_ERROR, errno);
                    break;
                }
            }
        }
        if (this->verbose & 0x100){
            LOG(LOG_INFO, "TLS Send done on %s (%u)", this->name, this->sck);
        }

    }

    void send_tcp(const char * const buffer, size_t len) throw (Error)
    {
        if (this->verbose & 0x100){
            LOG(LOG_INFO, "Socket %s (%u) sending %u bytes", this->name, this->sck, len);
            hexdump_c(buffer, len);
            LOG(LOG_INFO, "Dump done %s (%u) sending %u bytes", this->name, this->sck, len);
        }
        if (this->sck_closed) {
            LOG(LOG_INFO, "Socket already closed on %s (%u)", this->name, this->sck);
            throw Error(ERR_SOCKET_ALLREADY_CLOSED);
        }
        size_t total = 0;
        while (total < len) {
            ssize_t sent = ::send(this->sck, buffer + total, len - total, 0);
            switch (sent){
            case -1:
                if (!this->try_again(errno)) {
                    this->sck_closed = 1;
                    LOG(LOG_INFO, "Socket %s (%u) : %s", this->name, this->sck, strerror(errno));
                    throw Error(ERR_SOCKET_ERROR, errno);
                }
                else {
                    fd_set fds;
                    struct timeval time = { 0, 100000 };
                    FD_ZERO(&fds);
                    FD_SET(this->sck, &fds);
                    select(this->sck + 1, NULL, &fds, NULL, &time);
                }
                break;
            case 0:
                this->sck_closed = 1;
                LOG(LOG_INFO, "Socket %s (%u) closed on sending : %s", this->name, this->sck, strerror(errno));
                throw Error(ERR_SOCKET_CLOSED, errno);
            default:
                total = total + sent;
            }
        }
        total_sent += len;
        last_quantum_sent += len;
        if (this->verbose & 0x100){
            LOG(LOG_INFO, "Send done on %s (%u)", this->name, this->sck);
        }
    }

    private:

};


class ClientSocketTransport : public Transport {
        const char * name;
        SocketTransport * st;
        char ip[256];
        int port;
        int nbretry;
        int retry_delai_ms;
        uint32_t verbose;

    public:
        int sck;

    ClientSocketTransport(const char * name, const char* ip, int port,
                          int nbretry = 3, int retry_delai_ms = 1000,
                          uint32_t verbose = 0)
        : Transport(), name(name),
          st(NULL),
          port(port),
          nbretry(nbretry),
          retry_delai_ms(retry_delai_ms),
          verbose(verbose),
          sck(0)
    {
        strcpy(this->ip, ip);
    }

    virtual ~ClientSocketTransport(){
        delete this->st;
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error)
    {
        if (!this->st){
            LOG(LOG_INFO, "send failed : not connected to %s (%s:%d)\n", this->name, this->ip, this->port);
            return;
        }
        this->st->send(buffer, len);
    }

    using Transport::recv;

    virtual void recv(char ** pbuffer, size_t len) throw (Error)
    {
        if (!this->st){
            LOG(LOG_INFO, "recv failed : not connected to %s (%s:%d)\n", this->name, this->ip, this->port);
            return;
        }
        this->st->recv(pbuffer, len);
    }

    virtual void enable_tls() throw (Error)
    {
        if (!this->st){
            LOG(LOG_INFO, "enable_tls failed : not connected to %s (%s:%d)\n", this->name, this->ip, this->port);
            return;
        }
        this->st->enable_tls();
    }

    void disconnect()
    {
        delete this->st;
        this->st = NULL;
        this->sck = 0;
    }

    bool connect()
    {
        if (this->st){
            LOG(LOG_INFO, "Already connected to %s (%s:%d)\n", this->name, this->ip, this->port);
            return true;
        }
        LOG(LOG_INFO, "connecting to %s (%s:%d)\n", this->name, this->ip, this->port);
        // we will try connection several time
        // the trial process include socket opening, hostname resolution, etc
        // because some problems can come from the local endpoint,
        // not necessarily from the remote endpoint.
        int sck = socket(PF_INET, SOCK_STREAM, 0);

        /* set snd buffer to at least 32 Kbytes */
        int snd_buffer_size = 32768;
        unsigned int option_len = sizeof(snd_buffer_size);
        if (0 == getsockopt(sck, SOL_SOCKET, SO_SNDBUF, &snd_buffer_size, &option_len)) {
            if (snd_buffer_size < 32768) {
                snd_buffer_size = 32768;
                if (-1 == setsockopt(sck,
                        SOL_SOCKET,
                        SO_SNDBUF,
                        &snd_buffer_size, sizeof(snd_buffer_size))){
                    LOG(LOG_WARNING, "setsockopt failed with errno=%d", errno);
                    return false;
                }
            }
        }
        else {
            LOG(LOG_WARNING, "getsockopt failed with errno=%d", errno);
            return false;
        }

        TODO("DNS resolution should probably be done only once in constructor, even if we try several connections with TLS")
        struct sockaddr_in s;
        memset(&s, 0, sizeof(struct sockaddr_in));
        s.sin_family = AF_INET;
        s.sin_port = htons(this->port);
        s.sin_addr.s_addr = inet_addr(this->ip);
        if (s.sin_addr.s_addr == INADDR_NONE) {
        TODO(" gethostbyname is obsolete use new function getnameinfo")
            LOG(LOG_INFO, "Asking ip to DNS for %s\n", this->ip);
            struct hostent *h = gethostbyname(this->ip);
            if (!h) {
                LOG(LOG_ERR, "DNS resolution failed for %s with errno =%d (%s)\n",
                    this->ip, errno, strerror(errno));
                return false;
            }
            s.sin_addr.s_addr = *((int*)(*(h->h_addr_list)));
        }

        fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) | O_NONBLOCK);

        int trial = 0;
        for (; trial < this->nbretry ; trial++){
            int res = ::connect(sck, (struct sockaddr*)&s, sizeof(s));
            if (-1 != res){
                // connection suceeded
                break;
            }
            if (trial > 0){
                LOG(LOG_INFO, "Connection to %s failed with errno = %d (%s)",
                    this->ip, errno, strerror(errno));
            }
            if ((errno == EINPROGRESS) || (errno == EALREADY)){
                // try again
                fd_set fds;
                FD_ZERO(&fds);
                struct timeval timeout = {
                    this->retry_delai_ms / 1000,
                    1000 * (this->retry_delai_ms % 1000)
                };
                FD_SET(sck, &fds);
                // exit select on timeout or connect or error
                // connect will catch the actual error if any,
                // no need to care of select result
                select(sck+1, NULL, &fds, NULL, &timeout);
            }
            else {
                // real failure
               trial = this->nbretry;
            }
        }
        if (trial >= this->nbretry){
            LOG(LOG_INFO, "All trials done connecting to %s\n", this->ip);
            return false;
        }
        LOG(LOG_INFO, "connection to %s succeeded : socket %d\n", this->ip, sck);

        this->st = new SocketTransport(this->name, sck, this->verbose);
        this->sck = sck;
        return true;
    }
};

class OutByFilenameTransport : public OutFileTransport {
    char path[1024];
public:
    OutByFilenameTransport(const char * path)
    : OutFileTransport(-1)
    {
        size_t len = strlen(path);
        memcpy(this->path, path, len);
        this->path[len] = 0;
    }

    ~OutByFilenameTransport()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        if (this->fd == -1){
            this->fd = ::creat(this->path, 0777);
            if (this->fd == -1){
                LOG(LOG_INFO, "OutByFilename transport write failed with error : %s on %s", strerror(errno), this->path);
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        OutFileTransport::send(buffer, len);
    }
};


class InByFilenameTransport : public InFileTransport {
    char path[1024];
public:
    InByFilenameTransport(const char * path)
    : InFileTransport(-1)
    {
        size_t len = strlen(path);
        memcpy(this->path, path, len);
        this->path[len] = 0;
    }

    ~InByFilenameTransport()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        if (this->fd == -1){
            this->fd = ::open(this->path, O_RDONLY);
            if (this->fd == -1){
                LOG(LOG_INFO, "InByFilename transport '%s' recv failed with error : %s", this->path, strerror(errno));
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
        }
        InFileTransport::recv(pbuffer, len);
    }
};

class FileSequence
{
    char format[64];
    char prefix[512];
    char filename[512];
    char extension[12];
    uint32_t pid;

public:
    FileSequence(
        const char * const format,
        const char * const prefix,
        const char * const filename,
        const char * const extension)
    : pid(getpid())
    {
        size_t len_format = std::min(strlen(format), sizeof(this->format));
        memcpy(this->format, format, len_format);
        this->format[len_format] = 0;

        size_t len_prefix = std::min(strlen(prefix), sizeof(this->prefix));
        memcpy(this->prefix, prefix, len_prefix);
        this->prefix[len_prefix] = 0;

        size_t len_filename = std::min(strlen(filename), sizeof(this->filename));
        memcpy(this->filename, filename, len_filename);
        this->filename[len_filename] = 0;

        size_t len_extension = std::min(strlen(extension), sizeof(this->extension));
        memcpy(this->extension, extension, len_extension);
        this->extension[len_extension] = 0;

        this->pid = getpid();
    }

    void get_name(char * const buffer, size_t len, uint32_t count) const {
        if (0 == strcmp(this->format, "path file pid count extension")){
            snprintf(buffer, len, "%s%s-%06u-%06u.%s",
            this->prefix, this->filename, this->pid, count, this->extension);
        }
        else if (0 == strcmp(this->format, "path file pid extension")){
            snprintf(buffer, len, "%s%s-%06u.%s",
            this->prefix, this->filename, this->pid, this->extension);
        }
        else {
            LOG(LOG_ERR, "Unsupported sequence format string");
            throw Error(ERR_TRANSPORT);
        }
    }

    ssize_t filesize(uint32_t count) const {
        char filename[1024];
        this->get_name(filename, sizeof(filename), count);
        return ::filesize(filename);
    }

    ssize_t unlink(uint32_t count) const {
        char filename[1024];
        this->get_name(filename, sizeof(filename), count);
        return ::unlink(filename);
    }
};

class OutByFilenameSequenceTransport : public OutFileTransport {
public:
    const FileSequence & sequence;
    char path[1024];

    OutByFilenameSequenceTransport(const FileSequence & sequence, unsigned verbose = 0)
    : OutFileTransport(-1, verbose)
    , sequence(sequence)
    {
    }

    ~OutByFilenameSequenceTransport()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        if (this->fd == -1){
            this->sequence.get_name(this->path, sizeof(this->path), this->seqno);
            this->fd = ::creat(this->path, 0777);
            if (this->fd == -1){
                LOG(LOG_INFO, "OutByFilename transport write failed with error : %s", strerror(errno));
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        OutFileTransport::send(buffer, len);
    }

    virtual bool next()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
        this->OutFileTransport::next();
        return true;
    }
};


class OutByFilenameSequenceWithMetaTransport : public OutFileTransport {
public:
    timeval now;
    const FileSequence & meta;
    const FileSequence & sequence;
    char meta_path[1024];
    char path[1024];

    OutByFilenameSequenceWithMetaTransport(const FileSequence & meta, timeval now, uint16_t width, uint16_t height, const FileSequence & sequence, unsigned verbose = 0)
    : OutFileTransport(-1, verbose)
    , now(now)
    , meta(meta)
    , sequence(sequence)
    {
        this->timestamp(now);
        this->meta.get_name(this->meta_path, sizeof(this->meta_path), 0);
        int mfd = ::creat(this->meta_path, 0777);
        char buffer[2048];
        size_t len = sprintf(buffer, "%u %u\n0\n\n", width, height);
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            int ret = ::write(mfd, buffer + total_sent, remaining_len);
            if (ret > 0){
                remaining_len -= ret;
                total_sent += ret;
            }
            else {
                if (errno == EINTR){
                    continue;
                }
                LOG(LOG_INFO, "Meta write to %s failed with error %s", this->meta_path, strerror(errno));
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        ::close(mfd);
    }

    ~OutByFilenameSequenceWithMetaTransport()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
        int mfd = ::open(this->meta_path, O_APPEND|O_WRONLY, 0777);
        if (mfd < 0){
            LOG(LOG_ERR, "Failed to open meta_file %s : error %s", this->meta_path, strerror(errno));
            throw Error(ERR_TRANSPORT);
        }
        char buffer[2048];
        size_t len = sprintf(buffer, "%s %u %u\n", this->path, (unsigned)this->now.tv_sec, (unsigned)this->future.tv_sec);
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            int ret = ::write(mfd, buffer + total_sent, remaining_len);
            if (ret > 0){
                remaining_len -= ret;
                total_sent += ret;
            }
            else {
                if (errno == EINTR){
                    continue;
                }
                LOG(LOG_INFO, "Meta write to %s failed with error %s", this->meta_path, strerror(errno));
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        ::close(mfd);
    }

    using Transport::send;
    virtual void send(const char * const buffer, size_t len) throw (Error) {
        if (this->fd == -1){
            LOG(LOG_INFO, "next chunk file: path=%s\n", this->path);
            this->sequence.get_name(this->path, sizeof(this->path), this->seqno);
            this->fd = ::creat(this->path, 0777);
            if (this->fd == -1){
                LOG(LOG_INFO, "OutByFilename transport write failed with error : %s", strerror(errno));
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        OutFileTransport::send(buffer, len);
    }

    virtual bool next()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
        int mfd = ::open(this->meta_path, O_APPEND|O_WRONLY, 0777);
        if (mfd < 0){
            LOG(LOG_ERR, "Failed to open meta_file %s : error %s", this->meta_path, strerror(errno));
            throw Error(ERR_TRANSPORT);
        }
        char buffer[2048];
        size_t len = sprintf(buffer, "%s %u %u\n", this->path, (unsigned)this->now.tv_sec, (unsigned)this->future.tv_sec);
        size_t remaining_len = len;
        size_t total_sent = 0;
        while (remaining_len) {
            int ret = ::write(mfd, buffer + total_sent, remaining_len);
            if (ret > 0){
                remaining_len -= ret;
                total_sent += ret;
            }
            else {
                if (errno == EINTR){
                    continue;
                }
                LOG(LOG_INFO, "Meta write to %s failed with error %s", this->meta_path, strerror(errno));
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        this->now = this->future;
        ::close(mfd);
        this->OutFileTransport::next();
        return true;
    }
};


class InByFilenameSequenceTransport : public InFileTransport {
    const FileSequence & sequence;
    char path[1024];
public:
    InByFilenameSequenceTransport(const FileSequence & sequence)
    : InFileTransport(-1)
    , sequence(sequence)
    {
    }

    ~InByFilenameSequenceTransport()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
    }

    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        size_t remaining_len = len;
        while (remaining_len > 0){
            if (this->fd == -1){
                this->sequence.get_name(this->path, sizeof(this->path), this->seqno);
                this->fd = ::open(this->path, O_RDONLY);
                if (this->fd == -1){
                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                }
            }
            char * oldpbuffer = *pbuffer;
            try {
                InFileTransport::recv(pbuffer, remaining_len);
                // if recv returns it has read everything asked for, otherwise it will raise some exception
                remaining_len = 0;
            }
            catch (const Error & e) {
                if (e.id == 1501){
                    remaining_len -= *pbuffer - oldpbuffer;
                    this->next();
                }
                else {
                    throw;
                }
            };
        }
    }

    virtual bool next()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
        this->InFileTransport::next();
        return true;
    }
};

TODO("This readline function could probably move into stream to give some level of support for text oriented files")
static inline bool readline(int fd, char ** begin, char **end, char **eol, char buffer[], size_t len)
{
    for (char * p = *begin; p < *end; p++){
        if (*p == '\n'){
            *eol = p+1;
            return true;
        }
    }
    size_t trailing_space = buffer + len - *end;
    // reframe buffer if no trailing space left
    if (trailing_space == 0){
        size_t used_len = *end - *begin;
        memmove(buffer, *begin, used_len);
        *end = buffer + used_len;
        *begin = buffer;
    }
    ssize_t rcvd = 0;
    do {
        rcvd = ::read(fd, *end, buffer + len - *end);
    } while (rcvd < 0 && errno == EINTR);
    if (rcvd < 0){
        throw Error(ERR_TRANSPORT_READ_FAILED, 0);
    }
    if (rcvd == 0){
        if (*begin != *end) {
            *eol = *end;
            return false;
        }
        throw Error(ERR_TRANSPORT_READ_FAILED, 0);
    }
    *end += rcvd;
    for (char * p = *begin; p < *end; p++){
        if (*p == '\n'){
            *eol = p+1;
            return true;
        }
    }
    *eol = *end;
    return false;
}

class InByMetaSequenceTransport : public InFileTransport {
    char meta_filename[2048];
    int meta_fd;
    char buffer[2048];
    char * begin;
    char * end;
public:
    char path[2048];
    uint32_t begin_chunk_time;
    uint32_t end_chunk_time;
    unsigned chunk_num;

    InByMetaSequenceTransport(const char * meta_filename)
    : InFileTransport(-1)
    , meta_fd(-1)
    , begin(this->buffer)
    , end(this->buffer)
    , begin_chunk_time(0)
    , end_chunk_time(0)
    , chunk_num(0)
    {
        this->path[0] = 0;
        strcpy(this->meta_filename, meta_filename);
        this->reset_meta();
    }

    ~InByMetaSequenceTransport()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
        if (this->meta_fd != -1){
            ::close(this->meta_fd);
            this->meta_fd = -1;
        }
    }

    void reset_meta(){
        if (this->meta_fd != -1){
            ::close(this->meta_fd);
        }
        this->begin = this->end = this->buffer;
        printf("opening %s\n", this->meta_filename);
        this->meta_fd = ::open(this->meta_filename, O_RDONLY);
        char * eol = NULL;
        if(!readline(this->meta_fd, &this->begin, &this->end, &eol, this->buffer, sizeof(this->buffer))){
            LOG(LOG_INFO, "InByMetaSequenceTransport recv failed with error %s reading meta file line 0", strerror(errno));
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        };
        this->begin = eol;
        if(!readline(this->meta_fd, &this->begin, &this->end, &eol, this->buffer, sizeof(this->buffer))){
            LOG(LOG_INFO, "InByMetaSequenceTransport recv failed with error %s reading meta file line 1", strerror(errno));
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        };
        this->begin = eol;
        if(!readline(this->meta_fd, &this->begin, &this->end, &eol, this->buffer, sizeof(this->buffer))){
            LOG(LOG_INFO, "InByMetaSequenceTransport recv failed with error %s reading meta file line 2", strerror(errno));
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        };
        this->begin = eol;
        this->chunk_num = 0;
    }

    void next_chunk_info()
    {
        char * eol = NULL;
        bool res = readline(this->meta_fd, &this->begin, &this->end, &eol, this->buffer, sizeof(this->buffer));
        if (!res) {
            LOG(LOG_INFO, "InByMetaSequenceTransport recv failed with error %s reading meta file", strerror(errno));
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }
        char *eol2 = strchrnul(this->begin, ' ');
        if (eol2){
            memcpy(this->path, this->begin, eol2 - this->begin);
            this->path[eol2 - this->begin] = 0;
            this->begin = eol;
        }
        else {
            throw Error(ERR_TRANSPORT_READ_FAILED, errno);
        }

        TODO("Make this code harder, input sanity is not checked")
        this->begin_chunk_time = atol(eol2+1);
        char *eol3 = strchrnul(eol2+1, ' ');
        this->end_chunk_time = atol(eol3+1);

        this->chunk_num++;
    }

    TODO("Code below looks insanely complicated for what it is doing. I should probably stop at some point"
         "and *THINK* about the API that transport objects should really provide."
         "For instance I strongly suspect that it should be allowed to stop returning only part of the asked datas"
         "like the file system transport objects. There should also be an easy way to combine several layers of"
         "transports (using templates ?) and clearly define the properties of objects providing the sources of datas"
         "(some abstraction above file ?). The current sequences are easy to use, but somewhat limited")
    using Transport::recv;
    virtual void recv(char ** pbuffer, size_t len) throw (Error) {
        size_t remaining_len = len;
        while (remaining_len > 0){
            if (this->fd == -1){
                this->next_chunk_info();
                printf("opening new source WRM %s\n", this->path);
                this->fd = ::open(this->path, O_RDONLY);
                if (this->fd == -1){
                    LOG(LOG_INFO, "InByMetaSequence transport '%s' recv failed with error : %s", this->path, strerror(errno));
                    throw Error(ERR_TRANSPORT_READ_FAILED, errno);
                }
            }
            char * oldpbuffer = *pbuffer;
            try {
                InFileTransport::recv(pbuffer, remaining_len);
                // if recv returns it has read everything asked for, otherwise it will raise some exception
                remaining_len = 0;
            }
            catch (const Error & e) {
                if (e.id == ERR_TRANSPORT_NO_MORE_DATA){
                    remaining_len -= *pbuffer - oldpbuffer;
                    this->next();
                }
                else {
                    throw;
                }
            };
        }
    }

    virtual bool next()
    {
        if (this->fd != -1){
            ::close(this->fd);
            this->fd = -1;
        }
        this->InFileTransport::next();
        return true;
    }
};

#endif
