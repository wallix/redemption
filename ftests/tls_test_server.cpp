 /* A simple TLS server */

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

#include <openssl/ssl.h>
#include <openssl/err.h>
#include "rio/rio_impl.h"

static int password_cb0(char *buf, int num, int rwflag, void *userdata)
{
    printf("password cb num=%u\n", num);
    const char * pass = (char*)userdata;
    if(num < strlen(pass)+1){
      return(0);
    }

    strcpy(buf, pass);
    return strlen(pass);
}

static int http_serve(SSL * ssl, int s, BIO *bio_err)
{
    char buf[1024];
    RIO rio;
    RIO_ERROR status = rio_init_socket_tls(&rio, ssl);
    ssize_t r = rio_recv(&rio, buf, 14);

    if(r != 14 || (0 != strcmp(buf, "REDEMPTION\r\n\r\n"))){
        fprintf(stderr,"failed to read expected hello\n");
        exit(0);
    }

    r = rio_send(&rio, "Server: Redemption Server\r\n\r\n", 29);
    if(r < 0){
        fprintf(stderr, "Write error 1\n");
        exit(0);
    }
    r = rio_send(&rio, "Server test page\r\n", 18);
    if(r < 0){
        fprintf(stderr, "Write error 2\n");
        exit(0);
    }
    
    r = SSL_shutdown(ssl);
    if(!r){
      /* If we called SSL_shutdown() first then we always get return value of '0'. 
         In this case, try again, but first send a TCP FIN to trigger the other side's close_notify*/
      shutdown(s, 1);
      r=SSL_shutdown(ssl);
    }
      
    SSL_free(ssl);
    close(s);

    return(0);
}

int main(int argc, char **argv)
{
    int s;
    BIO *sbio;
    SSL_CTX *ctx;
    SSL *ssl;
    int r;
    pid_t pid;
    BIO *bio_err=0;

    
    if(!bio_err){
      /* Global system initialization*/
      SSL_library_init();
      SSL_load_error_strings();
      
      /* An error write context */
      bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);
    }

    /* Set up a SIGPIPE handler */
    signal(SIGPIPE, SIG_IGN);
    
    /* Create our context*/
    ctx=SSL_CTX_new(SSLv23_method());

    /* Load our keys and certificates*/
    if(!(SSL_CTX_use_certificate_chain_file(ctx, "ftests/fixtures/rdpproxy-cert.pem")))
    {
        BIO_printf(bio_err, "Can't read certificate file\n");
        ERR_print_errors(bio_err);
        exit(0);
    }

    SSL_CTX_set_default_passwd_cb(ctx, password_cb0);
    SSL_CTX_set_default_passwd_cb_userdata(ctx, (void*)"inquisition");
    if(!(SSL_CTX_use_PrivateKey_file(ctx, "ftests/fixtures/rdpproxy-key.pem", SSL_FILETYPE_PEM)))
    {
        BIO_printf(bio_err,"Can't read key file\n");
        ERR_print_errors(bio_err);
        exit(0);
    }
   
  {
    DH *ret=0;
    BIO *bio;

    if ((bio=BIO_new_file("ftests/fixtures/dh1024.pem","r")) == NULL){
        BIO_printf(bio_err,"Couldn't open DH file\n");
        ERR_print_errors(bio_err);
        exit(0);
    }

    ret=PEM_read_bio_DHparams(bio, NULL, NULL, NULL);
    BIO_free(bio);
    if(SSL_CTX_set_tmp_dh(ctx,ret)<0)
    {
        BIO_printf(bio_err,"Couldn't set DH parameters\n");
        ERR_print_errors(bio_err);
        exit(0);
    }
  }
 
    struct sockaddr_in sin;
    int val=1;

    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0) {
        fprintf(stderr, "Failed to make socket\n");
        exit(0);
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(4433);
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR, &val,sizeof(val));

    int bind_res = bind(sock,(struct sockaddr *)&sin,  sizeof(sin));
    if(bind_res < 0){
        fprintf(stderr, "Failed to bind\n");
        exit(0);
    }
    listen(sock,5);  

    while(1){
      s = accept(sock,0,0);
      if(s < 0){
        fprintf(stderr,"Problem accepting\n");
        exit(0);
      }

      if((pid=fork())){
        close(s);
      }
      else {
        sbio=BIO_new_socket(s,BIO_NOCLOSE);
        ssl=SSL_new(ctx);
        SSL_set_bio(ssl,sbio,sbio);
        
        if((r=SSL_accept(ssl)<=0))
        {
            BIO_printf(bio_err, "SSL accept error\n");
            ERR_print_errors(bio_err);
            exit(0);
        }
        
        http_serve(ssl, s, bio_err);
        exit(0);
      }
    }
    SSL_CTX_free(ctx);
    exit(0);
  }
