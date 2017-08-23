/* A simple TLS client
   It connects to the server
   sends a hello packet and waits for the response
*/

#include <iostream>
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
#include "transport/socket_transport.hpp"

#include "openssl_tls.hpp"


// TODO -Wold-style-cast is ignored
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

namespace
{

void rdp_request(SocketTransport & sockettransport)
{
    const char *request = "REDEMPTION\r\n\r\n";

    /* Find the exact request_len */
    int request_len = strlen(request);

    char buf[1024];

    sockettransport.send(request, request_len);

    printf("HELLO sent, going TLS\n");

    sockettransport.tls = new TLSContext();


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

    sockettransport.tls->allocated_ctx = ctx;
    sockettransport.tls->allocated_ssl = ssl;
    sockettransport.tls->io = ssl;

    NullServerNotifier null_server_notifier;

    const bool server_cert_store = true;
    sockettransport.tls->enable_client_tls(
            sockettransport.sck,
            server_cert_store,
            true,
            false,
            null_server_notifier,
            CERTIF_PATH,
            nullptr,
            "127.0.0.1",
            331
        );

    char * pbuf = buf;
    sockettransport.recv_boom(pbuf, 29);
    //(std::cout << "receive: ").write(buf, 29).flush();

    pbuf = buf;
    sockettransport.recv_boom(pbuf, 18);
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

    struct addrinfo * addr_info = nullptr;
    int               result    = getaddrinfo(host, nullptr, nullptr, &addr_info);

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

}

int main()
{
    const char *host = "localhost";
    int port = 4433;

    SSL_library_init();
    SSL_load_error_strings();

    try {
        SocketTransport sockettransport(
            "TestTLSClient", tcp_connect(host, port), host, port,
            to_verbose_flags(0xffffff)
        );

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

#pragma GCC diagnostic pop
