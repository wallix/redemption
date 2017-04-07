 /* A simple TLS server */

#define LOGPRINT

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

#include "openssl_tls.hpp"

#include "transport/socket_transport.hpp"

// TODO -Wold-style-cast is ignored
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

static int rdp_serve(SSL_CTX * ctx, int sock, BIO *bio_err)
{
    // TODO test behavior if we wai for receiving some data on unencrypted socket before commuting to SSL

    SocketTransport sockettransport("TestTLSServer", sock, "", 0, to_verbose_flags(0xFFFFFFF));

    char buf[1024];
    sockettransport.recv_atomic(buf, 14);

    if(0 != strcmp(buf, "REDEMPTION\r\n\r\n")){
        fprintf(stderr,"failed to read expected hello\n");
        exit(0);
    }

    printf("received plain text HELLO, going TLS\n");

    sockettransport.tls = new TLSContext();

    BIO * sbio = BIO_new_socket(sock, BIO_NOCLOSE);
    SSL * ssl = SSL_new(ctx);
    SSL_set_bio(ssl, sbio, sbio);

    int r = SSL_accept(ssl);
    if(r <= 0)
    {
        BIO_printf(bio_err, "SSL accept error\n");
        ERR_print_errors(bio_err);
        exit(0);
    }


    sockettransport.tls->allocated_ctx = ctx;
    sockettransport.tls->allocated_ssl = ssl;
    sockettransport.tls->io = ssl;

    sockettransport.send("Server: Redemption Server\r\n\r\n", 29);
    sockettransport.send("Server test page\r\n", 18);

//    r = SSL_shutdown(ssl);
//     if(!r){
      /* If we called SSL_shutdown() first then we always get return value of '0'.
         In this case, try again, but first send a TCP FIN to trigger the other side's close_notify*/
//       shutdown(sock, 1);
//      r=SSL_shutdown(ssl);
//     }

//    SSL_free(ssl);

    return(0);
}

int main()
{
    SSL_library_init();
    SSL_load_error_strings();

    BIO * bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);

    /* Set up a SIGPIPE handler */
    signal(SIGPIPE, SIG_IGN);

    /* Create our context*/
    SSL_CTX * ctx = SSL_CTX_new(SSLv23_method());

    /* Load our keys and certificates*/
    if(!(SSL_CTX_use_certificate_chain_file(ctx, "ftests/fixtures/rdpproxy-cert.pem")))
    {
        BIO_printf(bio_err, "Can't read certificate file\n");
        ERR_print_errors(bio_err);
        exit(0);
    }

    SSL_CTX_set_default_passwd_cb(ctx, password_cb0);
    SSL_CTX_set_default_passwd_cb_userdata(ctx, const_cast<char *>("inquisition"));
    if(!(SSL_CTX_use_PrivateKey_file(ctx, "ftests/fixtures/rdpproxy-key.pem", SSL_FILETYPE_PEM)))
    {
        BIO_printf(bio_err,"Can't read key file\n");
        ERR_print_errors(bio_err);
        exit(0);
    }

    DH *ret=nullptr;
    BIO *bio;

    if ((bio=BIO_new_file("ftests/fixtures/dh1024.pem","r")) == nullptr){
        BIO_printf(bio_err,"Couldn't open DH file\n");
        ERR_print_errors(bio_err);
        exit(0);
    }

    ret=PEM_read_bio_DHparams(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if(SSL_CTX_set_tmp_dh(ctx, ret)<0)
    {
        BIO_printf(bio_err,"Couldn't set DH parameters\n");
        ERR_print_errors(bio_err);
        exit(0);
    }


    union
    {
        struct sockaddr s;
        struct sockaddr_storage ss;
        struct sockaddr_in s4;
        struct sockaddr_in6 s6;
    } ucs;
    memset(&ucs, 0, sizeof(ucs));

    int val=1;

    int sock = socket(AF_INET, SOCK_STREAM,0);
    if(sock < 0) {
        fprintf(stderr, "Failed to make socket\n");
        exit(0);
    }

    memset(&ucs.s4, 0, sizeof(ucs));
    ucs.s4.sin_addr.s_addr = INADDR_ANY;
    ucs.s4.sin_family = AF_INET;
    ucs.s4.sin_port = htons(4433);
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR, &val,sizeof(val));

    int bind_res = bind(sock,&ucs.s,  sizeof(ucs));
    if(bind_res < 0){
        fprintf(stderr, "Failed to bind\n");
        exit(0);
    }
    listen(sock,5);

    for (;;) {
        int s = accept(sock,nullptr,nullptr);
        if(s < 0){
            fprintf(stderr,"Problem accepting\n");
            break;
        }

        pid_t pid = fork();

        if(pid){
            close(s);
        }
        else {
            rdp_serve(ctx, s, bio_err);
            break;
        }
    }
    SSL_CTX_free(ctx);

    return 0;
}

#pragma GCC diagnostic pop
