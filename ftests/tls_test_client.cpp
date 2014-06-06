/* A simple TLS client
   It connects to the server
   sends a hello packet and waits for the response
*/

// #define LOGPRINT

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "sockettransport.hpp"

#include <openssl/ssl.h>
#include <openssl/err.h>

static void rdp_request(SocketTransport & sockettransport)
{
    const char *request = "REDEMPTION\r\n\r\n";

    /* Find the exact request_len */
    int request_len = strlen(request);

    char buf[1024];

    sockettransport.send(request, request_len);

    printf("HELLO sent, going TLS\n");

    BIO *bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);
    SSL_CTX *ctx = SSL_CTX_new(SSLv23_method());

    SSL *ssl = SSL_new(ctx);
    BIO *sbio = BIO_new_socket(sockettransport.sck, BIO_NOCLOSE);
    SSL_set_bio(ssl, sbio, sbio);

    if(SSL_connect(ssl)<=0){
        BIO_printf(bio_err, "SSL connect error\n");
        ERR_print_errors(bio_err);
        exit(0);
    }

    sockettransport.allocated_ctx = ctx;
    sockettransport.allocated_ssl = ssl;
    sockettransport.tls = true;
    sockettransport.io = ssl;

    sockettransport.enable_client_tls(false);

    char * pbuf = buf;
    sockettransport.recv(&pbuf, 29);
    (std::cout << "receive: ").write(buf, 29).flush();

    pbuf = buf;
    sockettransport.recv(&pbuf, 18);
}

static X509 *load_cert(const char *file)
{
    X509 *x=NULL;
    BIO *cert;

    if ((cert=BIO_new(BIO_s_file())) == NULL)
        goto end;

    if (BIO_read_filename(cert,file) <= 0)
        goto end;

    x=PEM_read_bio_X509_AUX(cert,NULL, NULL, NULL);
end:
    if (cert != NULL) BIO_free(cert);
    return(x);
}

static int check(X509_STORE *ctx, const char *file)
{
    X509 *x=NULL;
    int i=0,ret=0;
    X509_STORE_CTX *csc;

    x = load_cert(file);
    if (x == NULL)
        goto end;

    csc = X509_STORE_CTX_new();
    if (csc == NULL)
        goto end;
    X509_STORE_set_flags(ctx, 0);
    if(!X509_STORE_CTX_init(csc,ctx,x,0))
        goto end;
    i=X509_verify_cert(csc);
    X509_STORE_CTX_free(csc);

    ret=0;
end:
    ret = (i > 0);
    if (x != NULL)
        X509_free(x);

    return(ret);
}


int verify(const char* certfile, const char* CAfile)
{
    int ret=0;
    X509_STORE *cert_ctx=NULL;
    X509_LOOKUP *lookup=NULL;

    cert_ctx=X509_STORE_new();
    if (cert_ctx == NULL) goto end;

    OpenSSL_add_all_algorithms();

    lookup=X509_STORE_add_lookup(cert_ctx,X509_LOOKUP_file());
    if (lookup == NULL)
        goto end;

    if(!X509_LOOKUP_load_file(lookup,CAfile,X509_FILETYPE_PEM))
        goto end;

    lookup=X509_STORE_add_lookup(cert_ctx,X509_LOOKUP_hash_dir());
    if (lookup == NULL)
        goto end;

    X509_LOOKUP_add_dir(lookup,NULL,X509_FILETYPE_DEFAULT);

    ret = check(cert_ctx, certfile);
end:
    if (cert_ctx != NULL) X509_STORE_free(cert_ctx);

    return ret;
}

int tcp_connect(const char *host, int port)
{
    union
    {
        struct sockaddr s;
        struct sockaddr_storage ss;
        struct sockaddr_in s4;
        struct sockaddr_in6 s6;
    } ucs;
    memset(&ucs, 0, sizeof(ucs));
    ucs.s4.sin_family = AF_INET;
    ucs.s4.sin_port = htons(port);

    struct addrinfo * addr_info = NULL;
    int               result    = getaddrinfo(host, NULL, NULL, &addr_info);

    if (result) {
        int          _error;
        const char * _strerror;

        if (result == EAI_SYSTEM) {
            _error    = errno;
            _strerror = strerror(errno);
        }
        else {
            _error    = result;
            _strerror = gai_strerror(result);
        }
        fprintf(stderr, "DNS resolution failed for %s with errno =%d (%s)\n",
                host, _error, _strerror);
        exit(0);
    }
    ucs.s4.sin_addr = (reinterpret_cast<sockaddr_in *>(addr_info->ai_addr))->sin_addr;

    int sock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0){
        fprintf(stderr, "Couldn't create socket\n");
        exit(0);
    }

    if (connect(sock, &ucs.s, sizeof(ucs)) < 0){
        fprintf(stderr, "Couldn't connect socket\n");
        exit(0);
    }

    return sock;
}


int main(int argc, char ** argv)
{
    const char *host = "localhost";
    int port = 4433;

    SSL_library_init();
    SSL_load_error_strings();

    try {
        SocketTransport sockettransport("TestTLSClient", tcp_connect(host, port), host, port, 0xffffff);

        if ( sockettransport.sck < 0 ){
            std::cerr << "Couldn't connect socket\n";
            return 1;
        }

        /* Now make our HTTP request */
        rdp_request(sockettransport);
    }
    catch(Error & e)
    {
        std::cerr << e.errmsg() << std::endl;
        return e.errnum ? e.errnum : 1;
    }

    return 0;
}

