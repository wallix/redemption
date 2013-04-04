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

#define PORT	4433
#define PASSWORD "password"

static char *pass;

static int password_cb0(char *buf, int num, int rwflag, void *userdata)
{
    printf("password cb\n");
    if(num<strlen(pass)+1){
      return(0);
    }

    strcpy(buf,pass);
    return(strlen(pass));
}

static int http_serve(SSL * ssl, int s, BIO *bio_err)
{
    char buf[1024];
    
    BIO * io=BIO_new(BIO_f_buffer());
    BIO * ssl_bio=BIO_new(BIO_f_ssl());
    BIO_set_ssl(ssl_bio, ssl, BIO_CLOSE);
    BIO_push(io, ssl_bio);
    
    int r = SSL_ERROR_NONE;
    while(1){
      r = BIO_gets(io, buf, sizeof(buf)-1);
      int error = SSL_get_error(ssl, r);
      switch(error){
        case SSL_ERROR_NONE:
          break;
        default:
        {
            uint32_t errcount = 0;
            errcount++;
            fprintf(stderr, "%s", ERR_error_string(error, NULL));
            while ((error = ERR_get_error()) != 0){
                errcount++;
                fprintf(stderr, "%s", ERR_error_string(error, NULL));
            }
            exit(0);
        }
      }
      /* Look for the blank line that signals the end of Client Hello */
      if(!strcmp(buf,"\r\n"))
        break;
    }

    if((r=BIO_puts(io,"Server: Redemption Server\r\n\r\n"))<=0)
    {
        fprintf(stderr,"Write error\n");
        exit(0);
    }

    if((r=BIO_puts(io,"Server test page\r\n"))<=0)
    {
        fprintf(stderr,"Write error\n");
        exit(0);
    }
    
    if((r=BIO_flush(io))<0)
    {
        fprintf(stderr,"Error flushing BIO\n");
        exit(0);
    }
    
    r=SSL_shutdown(ssl);
    if(!r){
      /* If we called SSL_shutdown() first then
         we always get return value of '0'. In
         this case, try again, but first send a
         TCP FIN to trigger the other side's
         close_notify*/
      shutdown(s,1);
      r=SSL_shutdown(ssl);
    }
      
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

    pass=PASSWORD;
    SSL_CTX_set_default_passwd_cb(ctx, password_cb0);
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

    int sock = sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0) {
        fprintf(stderr,"Couldn't make socket\n");
        exit(0);
    }

    memset(&sin,0,sizeof(sin));
    sin.sin_addr.s_addr=INADDR_ANY;
    sin.sin_family=AF_INET;
    sin.sin_port=htons(PORT);
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR, &val,sizeof(val));

    if(bind(sock,(struct sockaddr *)&sin,  sizeof(sin))<0){
        fprintf(stderr, "Couldn't bind\n");
        exit(0);
    }
    listen(sock,5);  

    while(1){
      if((s=accept(sock,0,0))<0)
      {
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
