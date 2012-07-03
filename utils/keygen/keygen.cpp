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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   rsa key generator for xrdp

*/

/*
   references:

   http://www.securiteam.com/windowsntfocus/5EP010KG0G.html

*/

using namespace std;

#include <stdlib.h>


#include "ssl_calls.hpp"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include <inttypes.h>
#include "rsa_keys.hpp"


#include <iostream>
#include <iomanip>

#if defined(OPENSSL_VERSION_NUMBER) && (OPENSSL_VERSION_NUMBER >= 0x0090800f)
#undef OLD_RSA_GEN1
#else
#define OLD_RSA_GEN1
#endif

#if defined(OLD_RSA_GEN1)
/*****************************************************************************/
/* returns error
   generates a new rsa key
   exp is passed in and mod and pri are passed out */
int ssl_gen_key_xrdp1(int key_size_in_bits, uint8_t* exp, int exp_len,
                  uint8_t* mod, int mod_len, uint8_t* pri, int pri_len)
{

    if ((exp_len != 4) || (mod_len != 64) || (pri_len != 64)) {
        return 1;
    }
    uint8_t *lmod = malloc(mod_len * sizeof(char));
    uint8_t *lpri = malloc(pri_len * sizeof(char));
    uint8_t *lexp = (uint8_t*)exp;
    int my_e = lexp[0];
    my_e |= lexp[1] << 8;
    my_e |= lexp[2] << 16;
    my_e |= lexp[3] << 24;
    /* srand is in stdlib.h */
    srand(time(0));
    RSA *my_key = RSA_generate_key(key_size_in_bits, my_e, 0, 0);
    int error = (my_key == 0);
    int len = 0;
    if (error == 0) {
        len = BN_num_bytes(my_key->n);
        error = len != mod_len;
    }
    if (error == 0) {
        BN_bn2bin(my_key->n, (uint8_t*)lmod);
        ssl_reverse_it(lmod, mod_len);
    }
    if (error == 0) {
        len = BN_num_bytes(my_key->d);
        error = len != pri_len;
    }
    if (error == 0) {
        BN_bn2bin(my_key->d, (uint8_t*)lpri);
        ssl_reverse_it(lpri, pri_len);
    }
    if (error == 0) {
        memcpy(mod, lmod, mod_len);
        memcpy(pri, lpri, pri_len);
    }
    RSA_free(my_key);
    free(lmod);
    free(lpri);
    return error;
}
#else
/*****************************************************************************/
/* returns error
   generates a new rsa key
   exp is passed in and mod and pri are passed out */
int ssl_gen_key_xrdp1(int key_size_in_bits, uint8_t* exp, int exp_len,
                  uint8_t* mod, int mod_len, uint8_t* pri, int pri_len)
{
    if ((exp_len != 4) || (mod_len != 64) || (pri_len != 64)) {
        return 1;
    }
    uint8_t *lexp = (uint8_t *)malloc(exp_len * sizeof(uint8_t));
    uint8_t *lmod = (uint8_t *)malloc(mod_len * sizeof(uint8_t));
    uint8_t *lpri = (uint8_t *)malloc(pri_len * sizeof(uint8_t));
    memcpy(lexp, exp, exp_len);
    ssl_reverse_it(lexp, exp_len);
    BIGNUM *my_e = BN_new();
    BN_bin2bn((uint8_t*)lexp, exp_len, my_e);
    RSA* my_key = RSA_new();
    int error = RSA_generate_key_ex(my_key, key_size_in_bits, my_e, 0) == 0;
    int len = 0;
    if (error == 0) {
        len = BN_num_bytes(my_key->n);
        error = len != mod_len;
    }
    if (error == 0) {
        BN_bn2bin(my_key->n, (uint8_t*)lmod);
        ssl_reverse_it(lmod, mod_len);
    }
    if (error == 0) {
        len = BN_num_bytes(my_key->d);
        error = len != pri_len;
    }
    if (error == 0) {
        BN_bn2bin(my_key->d, (uint8_t*)lpri);
        ssl_reverse_it(lpri, pri_len);
    }
    if (error == 0) {
        memcpy(mod, lmod, mod_len);
        memcpy(pri, lpri, pri_len);
    }
    BN_free(my_e);
    RSA_free(my_key);
    free(lexp);
    free(lmod);
    free(lpri);
    return error;
}
#endif


using namespace std;

/* fix for solaris 10 with gcc 3.3.2 problem */
#if defined(sun) || defined(__sun)
#define ctid_t id_t
#endif
// from <unistd.h> import setuid, _exit, getuid, fork, execlp,
// execvp, unlink, rmdir, access, chdir, getcwd, lseek, write, read, close,
// usleep
#include <unistd.h>
// from <errno.h> import errno, EAGAIN, EWOULDBLOCK, EINPROGRESS, EALREADY,
// EBUSY, EINTR
#include <errno.h>
// from <netinet/tcp.h> import TCP_NODELAY
#include <netinet/tcp.h>
// from <sys/un.h> import struct sockaddr_un s,
#include <sys/un.h>
// from <sys/time.h> import gettimeofday
#include <sys/time.h>
// from <sys/times.h> import tms tm
#include <sys/times.h>
// from <sys/wait.h> import waitpid
#include <sys/wait.h>
// from <signal.h> import SIGCHLD, signal, SIGHUP, SIGINT, SIGKILL, SIGTERM,
// SIGPIPE, kill
#include <signal.h>
// from <arpa/inet.h> import inet_addr
#include <arpa/inet.h>
// from <netdb.h> import gethostbyname, struct hostent,
#include <netdb.h>
// from <fcntl.h> import F_GETFL, fcntl, O_NONBLOCK, F_SETFL, O_RDONLY, open,
// O_RDWR, O_CREAT, struct flock, F_WRLCK, F_SETLK
#include <fcntl.h>
// from <pwd.h> import struct passwd,
#include <pwd.h>
// from <grp.h> import initgroups, getgrnam, struct group, getgrgid, time
#include <grp.h>
// from <time.h> import time
#include <time.h>

// from <stdlib> import malloc, free, atoi, abs, mbstowcs, system,
// setenv, getenv
#include <stdlib.h>
// from <stdarg.h> import va_start, va_end
#include <stdarg.h>
// from <stdio.h> import stdout, vfprintf, vsprintf, vsnprintf, getchar, sprintf
#include <stdio.h>
// from <locale.h> import LC_CTYPE, setlocale
#include <locale.h>
#include <syslog.h>
#include "log.hpp"

#include <assert.h>

/* for clearenv() */
extern char** environ;

/* for solaris */
#if !defined(PF_LOCAL)
#define PF_LOCAL AF_UNIX
#endif
#if !defined(INADDR_NONE)
#define INADDR_NONE ((unsigned long)-1)
#endif


/*****************************************************************************/
/* produce a hex dump */
void hexdump(ostream & str, const char* p, int len)
{
    if (len > 1024){ // no need to dump the world
        len = 1024;
    }
    unsigned char *line = (unsigned char*)p;
    int offset = 0;
    while (offset < len) {
        str.flags(ios::hex);
        str.width(4);
        str.precision(4);
        str.fill('0');

        str << offset << ' ';
        int thisline = len - offset;
        if (thisline > 16) { thisline = 16; }
        str.flags(ios::right);
        str.width(3);
        str.precision(2);
        str.fill('0');
        int i = 0;
        for (i = 0; i < thisline; i++) {
            str << hex << setw(2) << setfill('0') << (int)line[i] << " ";
        }
        for (; i < 16; i++) {
            str << "   ";
        }
        for (i = 0; i < thisline; i++) {
            str.put((line[i] >= 0x20 && line[i] < 0x7f) ? line[i] : '.');
        }
        str << '\n';
        offset += thisline;
        line += thisline;
    }
    str << '\n' << endl;
}


#define MY_KEY_SIZE 512

static uint8_t g_exponent[4] = {
    0x01, 0x00, 0x01, 0x00
};

static uint8_t g_ppk_e[4] = {
    0x5B, 0x7B, 0x88, 0xC0
};

static uint8_t g_ppk_n[72] = {
    0x3D, 0x3A, 0x5E, 0xBD, 0x72, 0x43, 0x3E, 0xC9,
    0x4D, 0xBB, 0xC1, 0x1E, 0x4A, 0xBA, 0x5F, 0xCB,
    0x3E, 0x88, 0x20, 0x87, 0xEF, 0xF5, 0xC1, 0xE2,
    0xD7, 0xB7, 0x6B, 0x9A, 0xF2, 0x52, 0x45, 0x95,
    0xCE, 0x63, 0x65, 0x6B, 0x58, 0x3A, 0xFE, 0xEF,
    0x7C, 0xE7, 0xBF, 0xFE, 0x3D, 0xF6, 0x5C, 0x7D,
    0x6C, 0x5E, 0x06, 0x09, 0x1A, 0xF5, 0x61, 0xBB,
    0x20, 0x93, 0x09, 0x5F, 0x05, 0x6D, 0xEA, 0x87,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t g_ppk_d[108] = {
    0x87, 0xA7, 0x19, 0x32, 0xDA, 0x11, 0x87, 0x55,
    0x58, 0x00, 0x16, 0x16, 0x25, 0x65, 0x68, 0xF8,
    0x24, 0x3E, 0xE6, 0xFA, 0xE9, 0x67, 0x49, 0x94,
    0xCF, 0x92, 0xCC, 0x33, 0x99, 0xE8, 0x08, 0x60,
    0x17, 0x9A, 0x12, 0x9F, 0x24, 0xDD, 0xB1, 0x24,
    0x99, 0xC7, 0x3A, 0xB8, 0x0A, 0x7B, 0x0D, 0xDD,
    0x35, 0x07, 0x79, 0x17, 0x0B, 0x51, 0x9B, 0xB3,
    0xC7, 0x10, 0x01, 0x13, 0xE7, 0x3F, 0xF3, 0x5F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

static uint8_t g_testkey[176] = {
    0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x5c, 0x00,
    0x52, 0x53, 0x41, 0x31, 0x48, 0x00, 0x00, 0x00,
    0x00, 0x02, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x01, 0x00, 0x79, 0x6f, 0xb4, 0xdf,
    0xa6, 0x95, 0xb9, 0xa9, 0x61, 0xe3, 0xc4, 0x5e,
    0xff, 0x6b, 0xd8, 0x81, 0x8a, 0x12, 0x4a, 0x93,
    0x42, 0x97, 0x18, 0x93, 0xac, 0xd1, 0x3a, 0x38,
    0x3c, 0x68, 0x50, 0x19, 0x31, 0xb6, 0x84, 0x51,
    0x79, 0xfb, 0x1c, 0xe7, 0xe3, 0x99, 0x20, 0xc7,
    0x84, 0xdf, 0xd1, 0xaa, 0xb5, 0x15, 0xef, 0x47,
    0x7e, 0xfc, 0x88, 0xeb, 0x29, 0xc3, 0x27, 0x5a,
    0x35, 0xf8, 0xfd, 0xaa, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x48, 0x00,
    0x32, 0x3b, 0xde, 0x6f, 0x18, 0x97, 0x1e, 0xc3,
    0x6b, 0x2b, 0x2d, 0xe4, 0xfc, 0x2d, 0xa2, 0x8e,
    0x32, 0x3c, 0xf3, 0x1b, 0x24, 0x90, 0x57, 0x4d,
    0x8e, 0xe4, 0x69, 0xfc, 0x16, 0x8d, 0x41, 0x92,
    0x78, 0xc7, 0x9c, 0xb4, 0x26, 0xff, 0xe8, 0x3e,
    0xa1, 0x8a, 0xf5, 0x57, 0xc0, 0x7f, 0x3e, 0x21,
    0x17, 0x32, 0x30, 0x6f, 0x79, 0xe1, 0x36, 0xcd,
    0xb6, 0x8e, 0xbe, 0x57, 0x57, 0xd2, 0xa9, 0x36
};

/* this is the installed signature */
char inst_pub_sig[]="0x6a,0x41,0xb1,0x43,0xcf,0x47,0x6f,0xf1,0xe6,0xcc,0xa1,\
0x72,0x97,0xd9,0xe1,0x85,0x15,0xb3,0xc2,0x39,0xa0,0xa6,0x26,0x1a,0xb6,\
0x49,0x01,0xfa,0xa6,0xda,0x60,0xd7,0x45,0xf7,0x2c,0xee,0xe4,0x8e,0x64,\
0x2e,0x37,0x49,0xf0,0x4c,0x94,0x6f,0x08,0xf5,0x63,0x4c,0x56,0x29,0x55,\
0x5a,0x63,0x41,0x2c,0x20,0x65,0x95,0x99,0xb1,0x15,0x7c";


/*****************************************************************************/
static int out_params(void)
{
    printf("\n");
    printf("xrdp rsa key gen utility examples\n");
    printf("  xrdp-keygen xrdp ['path and file name' | auto]\n");
    printf("  xrdp-keygen test\n");
    printf("\n");
    return 0;
}

/*****************************************************************************/
/* this is the special key signing algorithm */
static int sign_key(uint8_t* e_data, int e_len, uint8_t* n_data, int n_len,
         uint8_t* sign_data, int sign_len)
{
    uint8_t* key;
    uint8_t* md5_final;

    if ((e_len != 4) || (n_len != 64) || (sign_len != 64)) {
        return 1;
    }
    key = new uint8_t[176];
    md5_final = new uint8_t[64];
    SSL_MD5 md5;
    /* copy the test key */
    memcpy(key, g_testkey, 176);
    /* replace e and n */
    memcpy(key + 32, e_data, 4);
    memcpy(key + 36, n_data, 64);

    ssllib ssl;

    ssl.md5_init(&md5);
    /* the first 108 bytes */
    ssl.md5_update(&md5, key, 108);
    /* set the whole thing with 0xff */
    memset(md5_final, 0xff, 64);
    /* digest 16 bytes */
    ssl.md5_final(&md5, md5_final);
    /* set non 0xff array items */
    md5_final[16] = 0;
    md5_final[62] = 1;
    md5_final[63] = 0;
    /* encrypt */
    ssl_mod_exp(sign_data, 64, md5_final, 64, g_ppk_n, 64, g_ppk_d, 64);
    /* cleanup */
    delete(key);
    delete(md5_final);
    return 0;
}

/*****************************************************************************/
static int write_out_line(int fd, const char* name, uint8_t* data, int len)
{
    int max;
    int error;
    int index;
    int data_item;
    int buf_pos;
    char* buf;
    char* text;

    text = new char[256];
    max = len;
    max = max * 10;
    buf_pos = strlen(name);
    max = max + buf_pos + 16;
    buf = new char[max];
    strncpy(buf, name, max - 1);
    buf[buf_pos] = '=';
    buf_pos++;
    for (index = 0; index < len; index++) {
        data_item = (uint8_t)(data[index]);
        snprintf(text, 255, "0x%2.2x", data_item);
        if (index != 0) {
            buf[buf_pos] = ',';
            buf_pos++;
        }
        buf[buf_pos] = text[0];
        buf_pos++;
        buf[buf_pos] = text[1];
        buf_pos++;
        buf[buf_pos] = text[2];
        buf_pos++;
        buf[buf_pos] = text[3];
        buf_pos++;
    }
    buf[buf_pos] = '\n';
    buf_pos++;
    buf[buf_pos] = 0;
    error = write(fd, buf, buf_pos) == -1;
    delete(buf);
    delete(text);
    return error;
}

/*****************************************************************************/
static int save_all(uint8_t* e_data, int e_len, uint8_t* n_data, int n_len,
         uint8_t* d_data, int d_len, uint8_t* sign_data, int sign_len,
         const char* path_and_file_name)
{
    int fd;
    char filename[256];

    if (path_and_file_name == 0) {
        strncpy(filename, "rsakeys.ini", 255);
    } else {
        strncpy(filename, path_and_file_name, 255);
    }
    printf("saving to %s\n", filename);
    printf("\n");
    if ((0 == access(filename, F_OK))) {
        if (unlink(filename)) {
            printf("problem deleting %s, maybe no rights\n", filename);
            return 1;
        }
    }
    /*Code related to g_file_open os_call*/
    fd =  open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        /* can't open read / write, try to open read only */
        fd =  open(filename, O_RDONLY);
    }

    if (fd > 0) {
        if (write(fd, "[keys]\n", 7) == -1) {
            printf("problem writing to %s, maybe no rights\n", filename);
            return 1;
        }
        write_out_line(fd, "pub_exp", e_data, e_len);
        write_out_line(fd, "pub_mod", n_data, n_len);
        write_out_line(fd, "pub_sig", sign_data, sign_len);
        write_out_line(fd, "pri_exp", d_data, d_len);
    } else {
        printf("problem opening %s, maybe no rights\n", filename);
        return 1;
    }
    close(fd);
    return 0;
}

/*****************************************************************************/
static int key_gen(const char* path_and_file_name)
{
    uint8_t* e_data;
    uint8_t* n_data;
    uint8_t* d_data;
    uint8_t* sign_data;
    int e_len;
    int n_len;
    int d_len;
    int sign_len;
    int error;

    e_data = (uint8_t*)g_exponent;
    n_data = new uint8_t[64];
    d_data = new uint8_t[64];
    sign_data = new uint8_t[64];
    e_len = 4;
    n_len = 64;
    d_len = 64;
    sign_len = 64;
    error = 0;
    printf("\n");
    printf("Generating %d bit rsa key...\n", MY_KEY_SIZE);
    printf("\n");
    if (error == 0) {
        error = ssl_gen_key_xrdp1(MY_KEY_SIZE, e_data, e_len, n_data, n_len,
                                  d_data, d_len);
        if (error != 0) {
            printf("error %d in key_gen, ssl_gen_key_xrdp1\n", error);
        }
    }
    if (error == 0) {
        printf("ssl_gen_key_xrdp1 ok\n");
        printf("\n");
        error = sign_key(e_data, e_len, n_data, n_len, sign_data, sign_len);
        if (error != 0) {
            printf("error %d in key_gen, sign_key\n", error);
        }
    }
    if (error == 0) {
        error = save_all(e_data, e_len, n_data, n_len, d_data, d_len,
                         sign_data, sign_len, path_and_file_name);
        if (error != 0) {
            printf("error %d in key_gen, save_all\n", error);
        }
    }
    delete(n_data);
    delete(d_data);
    delete(sign_data);
    return error;
}

/*****************************************************************************/
static int key_gen_auto(Rsakeys * rsa_keys)
{
    TODO(": verify that is working  need to see changes!!")

    if (strcasecmp((char*)rsa_keys->pub_sig, inst_pub_sig) == 0) {
        return key_gen("/etc/xrdp/rsakeys.ini");
    }
    printf("xrdp-keygen does not need to run\n");
    return 0;
}

/*****************************************************************************/
static int key_test(void)
{
    uint8_t* md5_final;
    uint8_t* sig;
    SSL_MD5 md5;

    md5_final = new uint8_t[64];
    sig = new uint8_t[64];
    printf("original key is:\n");
    hexdump(std::cout, (char*)g_testkey, 176);
    printf("original exponent is:\n");
    hexdump(std::cout, (char*)g_testkey + 32, 4);
    printf("original modulus is:\n");
    hexdump(std::cout, (char*)g_testkey + 36, 64);
    printf("original signature is:\n");
    hexdump(std::cout, (char*)g_testkey + 112, 64);

    ssllib ssl;

    ssl.md5_init(&md5);
    ssl.md5_update(&md5, g_testkey, 108);
    memset(md5_final, 0xff, 64);
    ssl.md5_final(&md5, md5_final);
    printf("md5 hash of first 108 bytes of this key is:\n");
    hexdump(std::cout, (char*)md5_final, 16);
    md5_final[16] = 0;
    md5_final[62] = 1;
    md5_final[63] = 0;
    ssl_mod_exp(sig, 64, md5_final, 64, g_ppk_n, 64, g_ppk_d, 64);
    printf("produced signature(this should match original signature above) is:\n");
    hexdump(std::cout, (char*)sig, 64);
    memset(md5_final, 0, 64);
    ssl_mod_exp(md5_final, 64, g_testkey + 112, 64, g_ppk_n, 64, g_ppk_e, 4);
    printf("decrypted hash of first 108 bytes of this key is:\n");
    hexdump(std::cout, (char*)md5_final, 64);
    delete(md5_final);
    delete(sig);
    return 0;
}

/*****************************************************************************/
int main(int argc, char** argv)
{
    TODO(": verify that is working")
    char key_file[256];
    snprintf(key_file, 255, "%s/rsakeys.ini", CFG_PATH);
    Rsakeys * rsa_keys = new Rsakeys(key_file);
    if (argc > 1) {
        if (strcasecmp(argv[1], "xrdp") == 0) {
            if (argc > 2) {
                if (strcasecmp(argv[2], "auto") == 0) {
                    if (getuid() != 0) {
                        printf("must run as root\n");
                        return 0;
                    }
                    return key_gen_auto(rsa_keys);
                } else {
                    return key_gen(argv[2]);
                }
            } else {
                return key_gen(0);
            }
        } else if (strcasecmp(argv[1], "test") == 0) {
            return key_test();
        }
    }
    out_params();
    return 0;
}
