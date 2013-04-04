/* A simple TLS client
   It connects to the server
   sends a hello packet and waits for the response
*/

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
#include "rio/rio_impl.h"

#include <openssl/ssl.h>
#include <openssl/err.h>

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
    
    struct hostent *hp = gethostbyname(host);
    
    if(!hp){
        fprintf(stderr,"Failed searching DNS for host");
        exit(0);
    }

    ucs.s4.sin_addr = *(struct in_addr*)hp->h_addr_list[0];

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

static int http_request(SSL * ssl, const char * host, int port, BIO *bio_err)
{
    char buf[1024];
    int len;
    
    const char *request = "REDEMPTION\r\n\r\n";

    /* Find the exact request_len */
    int request_len = strlen(request);

    printf("writing %d bytes\n", request_len);
    int r = SSL_write(ssl, request, request_len);
    switch(SSL_get_error(ssl,r)){      
      case SSL_ERROR_NONE:
        if(request_len != r)
        {
            fprintf(stderr, "Incomplete write!\n");
            exit(0);
        }
        break;
        default:
        {
            BIO_printf(bio_err,"SSL write problem\n");
            ERR_print_errors(bio_err);
            exit(0);
        }
    }
    
    /* Now read the server's response, assuming that it's terminated by a close */
    while(1){
      r = SSL_read(ssl, buf, sizeof(buf));
      switch(SSL_get_error(ssl,r)){
        case SSL_ERROR_NONE:
          len=r;
          break;
        case SSL_ERROR_ZERO_RETURN:
          goto shutdown;
        case SSL_ERROR_SYSCALL:
          fprintf(stderr, "SSL Error: Premature close\n");
          goto done;
        default:
        {
            BIO_printf(bio_err,"SSL read problem\n");
            ERR_print_errors(bio_err);
            exit(0);
        }
      }

      fwrite(buf,1,len,stdout);
    }
    
  shutdown:
    r=SSL_shutdown(ssl);
    switch(r){
      case 1:
        break; /* Success */
      case 0:
      case -1:
      default:
      {
        BIO_printf(bio_err, "Shutdown failed\n");
        ERR_print_errors(bio_err);
        exit(0);
      }
    }
    
  done:
    SSL_free(ssl);
    return(0);
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



int main(int argc, char ** argv)
{
    const char *host = "localhost";
    int port = 4433;
    int require_server_auth = 1;

    SSL_library_init();
    SSL_load_error_strings();
  
    BIO *bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);
   
    SSL_CTX *ctx = SSL_CTX_new(SSLv23_method());

    /* Connect the TCP socket*/
    int sock = tcp_connect(host, port);


    TODO("test behavior if we send some data on unencrypted socket before commuting to SSL")

    /* Connect the SSL socket */
    SSL *ssl = SSL_new(ctx);
    BIO *sbio = BIO_new_socket(sock, BIO_NOCLOSE);
    SSL_set_bio(ssl, sbio, sbio);

    if(SSL_connect(ssl)<=0){
        BIO_printf(bio_err, "SSL connect error\n");
        ERR_print_errors(bio_err);
        exit(0);
    }

    if(require_server_auth){
        X509 * px509 = SSL_get_peer_certificate(ssl);
        if (!px509) {
            fprintf(stderr, "RIO *::crypto_cert_get_public_key: SSL_get_peer_certificate() failed");
            exit(0);
        }

        fprintf(stderr, "dumping X509 peer certificate\n");
        FILE * fp = fopen("X509.pem", "w+");
        PEM_write_X509(fp, px509);
        fclose(fp);
        fprintf(stderr, "dumped X509 peer certificate\n");
        
//        // extract the public key
//        EVP_PKEY* pkey = X509_get_pubkey(px509);
//        if (!pkey){
//            fprintf(stderr, "RIO *::crypto_cert_get_public_key: X509_get_pubkey() failed");
//            exit(0);
//        }

/*        int r = SSL_get_verify_result(ssl);*/
/*        printf("verify_result=%d\n", r);*/
/*        if(r != X509_V_OK)*/
/*        {*/
/*            int error = SSL_get_error(ssl, r);*/
/*            BIO_printf(bio_err,"Certificate doesn't verify, r = %d error=%d\n", r, error);*/
/*            uint32_t errcount = 0;*/
/*            error = ERR_get_error();*/
/*            while (error != 0){*/
/*                errcount++;*/
/*                fprintf(stderr, "%u:%s", errcount, ERR_error_string(error, NULL));*/
/*                error = ERR_get_error();*/
/*            }*/
/*            exit(0);*/
/*        }*/

        /*Check the cert chain. The chain length
          is automatically checked by OpenSSL when
          we set the verify depth in the ctx */

    /* Check the common name*/
    /*    X509 *peer;*/
    /*    char peer_CN[256];*/
    /*    peer=SSL_get_peer_certificate(ssl);*/
    /*    X509_NAME_get_text_by_NID(X509_get_subject_name(peer), NID_commonName, peer_CN, 256);*/
    /*    if(strcasecmp(peer_CN,host)) {*/
    /*        fprintf(stderr, "Common name doesn't match host name\n");*/
    /*        exit(0);*/
    /*    }*/
    }
 
    /* Now make our HTTP request */
    http_request(ssl, host, port, bio_err);

    /* Shutdown the socket */
    SSL_CTX_free(ctx);
    close(sock);

    exit(0);
  }

