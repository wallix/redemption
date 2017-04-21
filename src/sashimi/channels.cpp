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
   Copyright (c) 2014-2016 by Christophe Grosjean, Meng Tan
   Author(s): Christophe Grosjean

   Adapted from parts of the SSH Library
   Copyright (c) 2003-2013 by Aris Adamantiadis
*/

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdint.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <gssapi/gssapi.h>
#include <zlib.h>
#include <netdb.h>
#include <assert.h>
#include <sys/time.h>
#include <assert.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/types.h>
#include <netdb.h> // for NI_MAXHOST

#include <openssl/blowfish.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/pem.h>
#include <openssl/dsa.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>

#include "utils/log.hpp"


// ex: sftp -vvvvv cgrosjean@dummy:SSH:toto@10.10.43.12:/etc
//-> above broken
// ex: ssh cgrosjean@rj32:toto@10.10.43.12 find /
// ex: sftp -vvvvv cgrosjean@rj64:SSH:toto@10.10.43.12:/tmp
// sftp -vvvvv jenkins#rj64+SSH+cgrosjean@10.10.43.12:/etc
// -> above working
// ex: ssh -L 8888:10.10.43.14:8080 cgrosjean@dummy:SSH:cgrosjean@10.10.43.12
// ex: scp /home/cgrosjean/Documents/Far.avi cgrosjean@10.10.43.12:cgrosjean@dummy:SSH:/tmp
// ex: ssh -A cgrosjean@rj32:cgrosjean@10.10.43.12
// ex: ssh -o ForwardX11=No -A cgrosjean@10.10.43.12
// ex: scp README.rst jenkins@rj64+SSH+cgrosjean@10.10.43.12:

// ex: ssh -ND 9999 pion@Xtarget:SSH:cgrosjean@10.10.43.12
// ex: google-chrome --proxy-server='socks://localhost:9999'
///usr/bin/ssh-agent /usr/bin/ck-launch-session /usr/bin/dbus-launch --exit-with-session gnome-session

// On a kerberos configured client:
// kinit -f cgrosjean
// ssh -vv -A -K -o GSSAPIAuthentication=yes -o GSSAPIDelegateCredentials=yes -o GSSAPIKeyExchange=yes -o GSSAPITrustDNS=no -o PasswordAuthentication=no -o PubkeyAuthentication=no  -p 22 cgrosjean@10.10.43.12

// scp /home/cgrosjean/Documents/Far.avi  jenkins@rj64+SSH+cgrosjean@10.10.43.12:
// -> echec a 36%

// scp /home/cgrosjean/Documents/Far.avi cgrosjean@rj64+cgrosjean@10.10.43.12:
// ssh cgrosjean#rj64:cgrosjean@10.10.43.12 find .
// ssh jenkins@rj64+SSH+cgrosjean@10.10.43.12 ls

// Test X11
// ssh -A -X cgrosjean@rj32:cgrosjean@10.10.43.12
// Puis une fois connecté lancer une appli X


// Lancer une connection avec l'option -D sur une cible
// (X11 doit etre autorisé pour l'ouverture des channels)

// ssh -A -D 8888 cgrosjean@rj32:cgrosjean@10.10.43.12

// Maintenant lancer chrome avec l'option proxy socks

// google-chrome --proxy-server="socks://localhost:8888"

// Et naviguer, les ressources associées aux channels augmentent
// (fd locaux du client ssh)

// A la fermeture des channels elles devraient toutes se libérer
// Malheureusement ce n'est pas le cas.



#include "sashimi/libssh/libssh.h"

#include "sashimi/buffer.hpp"
#include "sashimi/channels.hpp"
#include "sashimi/pki.hpp"




#define RSA_HEADER_BEGIN "-----BEGIN RSA PRIVATE KEY-----"
//#define RSA_HEADER_END "-----END RSA PRIVATE KEY-----"
#define DSA_HEADER_BEGIN "-----BEGIN DSA PRIVATE KEY-----"
//#define DSA_HEADER_END "-----END DSA PRIVATE KEY-----"
#define ECDSA_HEADER_BEGIN "-----BEGIN EC PRIVATE KEY-----"
//#define ECDSA_HEADER_END "-----END EC PRIVATE KEY-----"



struct ssh_timestamp {
  long seconds;
  long useconds;
};


inline char *ssh_get_local_username(void) {
    struct passwd pwd;
    struct passwd *pwdbuf;
    char buf[NSS_BUFLEN_PASSWD];
    char *name;
    int rc;

    rc = getpwuid_r(getuid(), &pwd, buf, NSS_BUFLEN_PASSWD, &pwdbuf);
    if (rc != 0) {
        return nullptr;
    }

    name = strdup(pwd.pw_name);

    if (name == nullptr) {
        return nullptr;
    }

    return name;
}

int options_set(options_struct & opts, enum ssh_options_e type, const void *value, error_struct & error)
{
    const char *v;
    char *p, *q;
    long int i;

    if (value == nullptr) {
        switch (type){
        case SSH_OPTIONS_USER:
            opts.username = ssh_get_local_username();
        break;
        case SSH_OPTIONS_FD:
            opts.fd = INVALID_SOCKET;
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_HOST:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_PORT:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_PORT_STR:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_SSH_DIR:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_IDENTITY:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_ADD_IDENTITY:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_KNOWNHOSTS:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_TIMEOUT:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_TIMEOUT_USEC:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_SSH1:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_SSH2:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_CIPHERS_C_S:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_CIPHERS_S_C:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_COMPRESSION_C_S:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_COMPRESSION_S_C:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_PROXYCOMMAND:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_BINDADDR:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_STRICTHOSTKEYCHECK:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_COMPRESSION:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_COMPRESSION_LEVEL:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_KEY_EXCHANGE:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_HOSTKEYS:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_GSSAPI_SERVER_IDENTITY:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_GSSAPI_CLIENT_IDENTITY:
            REDEMPTION_CXX_FALLTHROUGH;
        case SSH_OPTIONS_GSSAPI_DELEGATE_CREDENTIALS:
            REDEMPTION_CXX_FALLTHROUGH;
        default:
            ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
            return -1;
        }
    }


    switch (type) {
        case SSH_OPTIONS_HOST:
            // options_set_host(opts, type, value, error);
            v = static_cast<const char*>(value);
            if (v[0] == '\0') {
                ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                return -1;
            }
            else {
                q = strdup(static_cast<const char*>(value));
                // TODO: check memory allocation
                p = strchr(q, '@');

                free(opts.host);
                opts.host = nullptr;

                if (p) {
                    *p = '\0';
                    opts.host = strdup(static_cast<const char*>(p + 1));
                    // TODO: check memory allocation

                    free(opts.username);
                    opts.username = nullptr;
                    opts.username = strdup(static_cast<const char*>(q));
                    free(q);
                    q = nullptr;
                    // TODO: check memory allocation
                }
                else {
                    opts.host = q;
                }
            }
            break;
        case SSH_OPTIONS_PORT:
            // options_set_port(opts, type, value, error);
            {
                const int *x = reinterpret_cast<const int *>(value);
                if (*x <= 0) {
                    ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                    return -1;
                }

                opts.port = *x & 0xffff;
            }
            break;
        case SSH_OPTIONS_PORT_STR:
            // options_set_port_str(opts, type, value, error);

            v = static_cast<const char*>(value);
            if (v[0] == '\0') {
                ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                return -1;
            } else {
                q = strdup(v);
                // TODO: check memory allocation
                i = strtol(q, &p, 10);
                if (q == p) {
                    free(q);
                    q = nullptr;
                }
                free(q);
                q = nullptr;
                if (i <= 0) {
                    ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                    return -1;
                }

                opts.port = i & 0xffff;
            }
            break;
        case SSH_OPTIONS_FD:
            // options_set_fd(opts, type, value, error);
            {
                const socket_t *x = reinterpret_cast<const socket_t *>(value);
                if (*x < 0) {
                    opts.fd = INVALID_SOCKET;
                    ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                    return -1;
                }

                opts.fd = *x & 0xffff;
            }
            break;
        case SSH_OPTIONS_BINDADDR:
            // options_set_bindaddr(opts, type, value, error);

            v = static_cast<const char*>(value);
            if (v[0] == '\0') {
                ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                return -1;
            }

            q = strdup(v);
            if (q == nullptr) {
                return -1;
            }
            free(opts.bindaddr);
            opts.bindaddr = q;
            break;
        case SSH_OPTIONS_USER:
            // options_set_user(opts, type, value, error);

            v = static_cast<const char*>(value);
            free(opts.username);
            opts.username = nullptr;
            if (v == nullptr) {
                q = ssh_get_local_username();
                // TODO: check memory allocation
                opts.username = q;
            }
            else if (v[0] == 0) {
                return -1;
            }
            else { /* username provided */
                opts.username = strdup(static_cast<const char*>(value));
               // TODO: check memory allocation
            }
            break;
        case SSH_OPTIONS_TIMEOUT:
            // options_set_timeout(opts, type, value, error);

            {
                const long *x = reinterpret_cast<const long *>(value);
                if (*x < 0) {
                    ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                    return -1;
                }

                opts.timeout = *x & 0xffffffff;
            }
            break;
        case SSH_OPTIONS_TIMEOUT_USEC:
            // options_set_timeout_usec(opts, type, value, error);

            {
                const long *x = reinterpret_cast<const long *>(value);
                if (*x < 0) {
                    ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                    return -1;
                }

                opts.timeout_usec = *x & 0xffffffff;
            }
            break;
        case SSH_OPTIONS_SSH1:
            // options_set_ssh1(opts, type, value, error);
            {
                int x = *reinterpret_cast<const int*>(value);
                if (x < 0) {
                    ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                    return -1;
                }
                opts.ssh1 = x;
            }
            break;
        case SSH_OPTIONS_SSH2:
            // options_set_ssh2(opts, type, value, error);

            {
                int x = *reinterpret_cast<const int*>(value);
                if (x < 0) {
                    ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                    return -1;
                }
                opts.ssh2 = x & 0xffff;
            }
            break;
        case SSH_OPTIONS_CIPHERS_C_S:
            // options_set_ciphers_c_s_(opts, type, value, error);

            v = static_cast<const char*>(value);
            if (v[0] == '\0') {
                ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                return -1;
            }
            else {
                const char * supported_ssh_crypt_c_s = "aes256-ctr,aes192-ctr,aes128-ctr,aes256-cbc,aes192-cbc,aes128-cbc,blowfish-cbc,3des-cbc,des-cbc-ssh1";

                SSHString tmp = find_matching(supported_ssh_crypt_c_s, v, ',');
                if (tmp.size() == 0){
                    ssh_set_error(error, SSH_REQUEST_DENIED,
                        "Setting method: no algorithm for method \"%s\" (%s)\n",
                        "encryption client->server", v);
                    return -1;
                }

                free(opts.wanted_methods[SSH_CRYPT_C_S]);
                opts.wanted_methods[SSH_CRYPT_C_S] = strdup(v);
            }
            break;
        case SSH_OPTIONS_CIPHERS_S_C:
            // options_set_ciphers_s_c(opts, type, value, error);

            v = static_cast<const char*>(value);
            if (v[0] == '\0') {
                ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                return -1;
            }
            else {
                const char * supported_ssh_crypt_s_c = "aes256-ctr,aes192-ctr,aes128-ctr,aes256-cbc,aes192-cbc,aes128-cbc,blowfish-cbc,3des-cbc,des-cbc-ssh1";

                SSHString tmp = find_matching(supported_ssh_crypt_s_c, v, ',');
                if (tmp.size() == 0){
                    ssh_set_error(error, SSH_REQUEST_DENIED,
                        "Setting method: no algorithm for method \"%s\" (%s)\n",
                        "encryption server->client", v);
                    return -1;
                }

                free(opts.wanted_methods[SSH_CRYPT_S_C]);
                opts.wanted_methods[SSH_CRYPT_S_C] = strdup(v);
            }
            break;
        case SSH_OPTIONS_KEY_EXCHANGE:
            // options_set_key_exchange(opts, type, value, error);

            v = static_cast<const char*>(value);
            if (v[0] == '\0') {
                ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                return -1;
            } else {
                const char * supported_ssh_kex =       "curve25519-sha256@libssh.org,ecdh-sha2-nistp256,diffie-hellman-group14-sha1,diffie-hellman-group1-sha1";

                SSHString tmp = find_matching(supported_ssh_kex, v, ',');
                if (tmp.size() == 0){
                    ssh_set_error(error, SSH_REQUEST_DENIED,
                        "Setting method: no algorithm for method \"%s\" (%s)\n",
                        "kex algos", v);
                    return -1;
                }

                free(opts.wanted_methods[SSH_KEX]);
                opts.wanted_methods[SSH_KEX] = strdup(v);
            }
            break;
        case SSH_OPTIONS_HOSTKEYS:
            // options_set_hostkeys(opts, type, value, error);
            v = static_cast<const char*>(value);
            if (v[0] == '\0') {
                ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                return -1;
            }
            else {
                const char * supported_ssh_hostkeys = "ecdsa-sha2-nistp256,ssh-rsa,ssh-dss";

                SSHString tmp = find_matching(supported_ssh_hostkeys, v, ',');
                if (tmp.size() == 0){
                    ssh_set_error(error, SSH_REQUEST_DENIED,
                        "Setting method: no algorithm for method \"%s\" (%s)\n",
                        "server host key algo", v);
                    return -1;
                }

                free(opts.wanted_methods[SSH_HOSTKEYS]);
                opts.wanted_methods[SSH_HOSTKEYS] = strdup(v);
            }
            break;
        case SSH_OPTIONS_COMPRESSION_C_S:
            // options_set_compression_c_s(opts, type, value, error);
            v = static_cast<const char*>(value);
            if (v[0] == '\0') {
                ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                return -1;
            }
            else {
                if (strcasecmp(static_cast<const char*>(value),"yes")==0){
                    const char * supported_ssh_comp_c_s = "none,zlib,zlib@openssh.com";
                    SSHString tmp = find_matching(supported_ssh_comp_c_s, "zlib@openssh.com,zlib", ',');
                    if (tmp.size() == 0){
                        ssh_set_error(error, SSH_REQUEST_DENIED,
                            "Setting method: no algorithm for method \"%s\" (%s)\n",
                            "compression algo client->server",
                            "zlib@openssh.com,zlib");
                        return -1;
                    }
                    free(opts.wanted_methods[SSH_COMP_C_S]);
                    opts.wanted_methods[SSH_COMP_C_S] = strdup("zlib@openssh.com,zlib");
                } else if (strcasecmp(static_cast<const char*>(value),"no")==0){
                    const char * supported_ssh_comp_c_s = "none,zlib,zlib@openssh.com";
                    SSHString tmp = find_matching(supported_ssh_comp_c_s, "none", ',');
                    if (tmp.size() == 0){
                        ssh_set_error(error, SSH_REQUEST_DENIED,
                            "Setting method: no algorithm for method \"%s\" (%s)\n",
                            "compression algo client->server",
                            "none");
                        return -1;
                    }
                    free(opts.wanted_methods[SSH_COMP_C_S]);
                    opts.wanted_methods[SSH_COMP_C_S] = strdup("none");
                } else {
                    const char * supported_ssh_comp_c_s = "none,zlib,zlib@openssh.com";
                    SSHString tmp = find_matching(supported_ssh_comp_c_s, v, ',');
                    if (tmp.size() == 0){
                        ssh_set_error(error, SSH_REQUEST_DENIED,
                            "Setting method: no algorithm for method \"%s\" (%s)\n",
                            "compression algo client->server", v);
                        return -1;
                    }
                    free(opts.wanted_methods[SSH_COMP_C_S]);
                    opts.wanted_methods[SSH_COMP_C_S] = strdup(v);
                }
            }
            break;
        case SSH_OPTIONS_COMPRESSION_S_C:
            // options_set_compression_s_c(opts, type, value, error);
            v = static_cast<const char*>(value);
            if (v[0] == '\0') {
                ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                return -1;
            }
            else {
                if (strcasecmp(static_cast<const char*>(value),"yes")==0){
                    const char * supported_ssh_comp_s_c = "none,zlib,zlib@openssh.com";
                    SSHString tmp = find_matching(supported_ssh_comp_s_c, "zlib@openssh.com,zlib", ',');
                    if (tmp.size() == 0){
                        ssh_set_error(error, SSH_REQUEST_DENIED,
                            "Setting method: no algorithm for method \"%s\" (%s)\n",
                            "compression algo server->client",
                            "zlib@openssh.com,zlib");
                        return -1;
                    }
                    free(opts.wanted_methods[SSH_COMP_S_C]);
                    opts.wanted_methods[SSH_COMP_S_C] = strdup("zlib@openssh.com,zlib");
                } else if (strcasecmp(static_cast<const char*>(value),"no")==0){
                    const char * supported_ssh_comp_s_c = "none,zlib,zlib@openssh.com";
                    SSHString tmp = find_matching(supported_ssh_comp_s_c, "none", ',');
                    if (tmp.size() == 0){
                        ssh_set_error(error, SSH_REQUEST_DENIED,
                            "Setting method: no algorithm for method \"%s\" (%s)\n",
                            "compression algo server->client",
                            "none");
                        return -1;
                    }
                    free(opts.wanted_methods[SSH_COMP_S_C]);
                    opts.wanted_methods[SSH_COMP_S_C] = strdup("none");
                } else {
                    const char * supported_ssh_comp_s_c = "none,zlib,zlib@openssh.com";
                    SSHString tmp = find_matching(supported_ssh_comp_s_c, v, ',');
                    if (tmp.size() == 0){
                        ssh_set_error(error, SSH_REQUEST_DENIED,
                            "Setting method: no algorithm for method \"%s\" (%s)\n",
                            "compression algo server->client", v);
                        return -1;
                    }
                    free(opts.wanted_methods[SSH_COMP_S_C]);
                    opts.wanted_methods[SSH_COMP_S_C] = strdup(v);
                }
            }
            break;
        case SSH_OPTIONS_COMPRESSION:
            // options_set_compression(opts, type, value, error);
            v = static_cast<const char*>(value);
            if (v[0] == '\0') {
                ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                return -1;
            }
            if(options_set(opts, SSH_OPTIONS_COMPRESSION_C_S, v, error) < 0)
                return -1;
            if(options_set(opts, SSH_OPTIONS_COMPRESSION_S_C, v, error) < 0)
                return -1;
            break;
        case SSH_OPTIONS_COMPRESSION_LEVEL:
            {
                int x = *reinterpret_cast<const int*>(value);
                if (x < 1 || x > 9) {
                    ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                    return -1;
                }
                opts.compressionlevel = x & 0xff;
            }
            break;
        case SSH_OPTIONS_STRICTHOSTKEYCHECK:
            // options_set_stricthostkeycheck(opts, value, error);
            opts.StrictHostKeyChecking = *reinterpret_cast<const int*>(value);
            break;
        case SSH_OPTIONS_GSSAPI_SERVER_IDENTITY:
            // options_set_gssapi_server_identity(opts, type, value, error);
            v = static_cast<const char*>(value);
            if (v[0] == '\0') {
                ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                return -1;
            } else {
                free(opts.gss_server_identity);
                opts.gss_server_identity = nullptr;
                opts.gss_server_identity = strdup(v);
                // TODO: check memory allocation
            }
            break;
        case SSH_OPTIONS_GSSAPI_CLIENT_IDENTITY:
            // options_set_gssapi_client_identity(opts, type, value, error);
            v = static_cast<const char*>(value);
            if (v[0] == '\0') {
                ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
                return -1;
            } else {
                free(opts.gss_client_identity);
                opts.gss_client_identity = nullptr;
                opts.gss_client_identity = strdup(v);
                // TODO: check memory allocation
            }
            break;
        case SSH_OPTIONS_GSSAPI_DELEGATE_CREDENTIALS:
            // options_set_gssapi_delegate_credentials(opts, value, error);
            {
                int x = *reinterpret_cast<const int *>(value);
                opts.gss_delegate_creds = (x & 0xff);
            }
            break;

        default:
            ssh_set_error(error, SSH_REQUEST_DENIED, "Unknown ssh option %d", type);
            return -1;
    }

    return 0;
}


inline int options_set_host(options_struct & opts, const char *value, error_struct & error)
{
    if (value[0] == '\0') {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }
    else {
        char *q = strdup(value);
        // TODO: check memory allocation
        char *p = strchr(q, '@');

        free(opts.host);
        opts.host = nullptr;

        if (p) {
            *p = '\0';
            opts.host = strdup(static_cast<const char*>(p + 1));
            // TODO: check memory allocation

            free(opts.username);
            opts.username = strdup(static_cast<const char*>(q));
            free(q);
            q = nullptr;
            // TODO: check memory allocation
        }
        else {
            opts.host = q;
        }
    }
    return 0;
}


inline int options_set_port(options_struct & opts, int value, error_struct & error)
{

    if (value <= 0) {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }
    opts.port = value & 0xffff;
    return 0;
}


inline int options_set_port_str(options_struct & opts, const void *value, error_struct & error)
{
    const char *v = static_cast<const char*>(value);
    if (v[0] == '\0') {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    } else {
        char * q = strdup(v);
        char * p;
        long int i = strtol(q, &p, 10);
        free(q);
        q = nullptr;
        if (i <= 0) {
            ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
            return -1;
        }

        opts.port = i & 0xffff;
    }

    return 0;
}

inline int options_set_fd(options_struct & opts, socket_t value, error_struct & error)
{
    if (value < 0) {
        opts.fd = INVALID_SOCKET;
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }

    opts.fd = value & 0xffff;
    return 0;
}

inline int options_set_bindaddr(options_struct & opts, const void *value, error_struct & error)
{
    const char *v = static_cast<const char*>(value);
    if (v[0] == '\0') {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }

    char * q = strdup(v);
    if (q == nullptr) {
        return -1;
    }
    free(opts.bindaddr);
    opts.bindaddr = q;
    return 0;
}


inline int options_set_user(options_struct & opts, const void *value, error_struct & error)
{
    (void)error;
    const char *v = static_cast<const char*>(value);
    free(opts.username);
    opts.username = nullptr;
    if (v == nullptr) {
        // TODO: check memory allocation
        opts.username = ssh_get_local_username();
    }
    else if (v[0] == 0) {
        return -1;
    }
    else { /* username provided */
        opts.username = strdup(static_cast<const char*>(value));
       // TODO: check memory allocation
    }

    return 0;
}

inline int options_set_timeout(options_struct & opts, int value, error_struct & error)
{

    if (value < 0) {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }

    opts.timeout = value & 0xffffffff;
    return 0;
}

inline int options_set_timeout_usec(options_struct & opts, int value, error_struct & error)
{
    if (value < 0) {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }

    opts.timeout_usec = value & 0xffffffff;
    return 0;
}


inline int options_set_ssh1(options_struct & opts, int value, error_struct & error)
{
    if (value < 0) {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }

    opts.ssh1 = value;
    return 0;
}


inline int options_set_ssh2(options_struct & opts, int value, error_struct & error)
{
    if (value < 0) {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }

    opts.ssh2 = value & 0xffff;
    return 0;
}

inline int options_set_cipher_c_s(options_struct & opts, const void *value, error_struct & error)
{
    const char *v = static_cast<const char*>(value);
    if (v[0] == '\0') {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }
    else {
        const char * supported_ssh_crypt_c_s = "aes256-ctr,aes192-ctr,aes128-ctr,aes256-cbc,aes192-cbc,aes128-cbc,blowfish-cbc,3des-cbc,des-cbc-ssh1";

        SSHString tmp = find_matching(supported_ssh_crypt_c_s, v, ',');
        if (tmp.size() == 0){
            ssh_set_error(error, SSH_REQUEST_DENIED,
                "Setting method: no algorithm for method \"%s\" (%s)\n",
                "encryption client->server", v);
            return -1;
        }

        free(opts.wanted_methods[SSH_CRYPT_C_S]);
        opts.wanted_methods[SSH_CRYPT_C_S] = strdup(v);
    }
    return 0;
}


inline int options_set_cipher_s_c(options_struct & opts, const void *value, error_struct & error)
{
    const char *v = static_cast<const char*>(value);
    if (v[0] == '\0') {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }
    else {
        const char * supported_ssh_crypt_s_c = "aes256-ctr,aes192-ctr,aes128-ctr,aes256-cbc,aes192-cbc,aes128-cbc,blowfish-cbc,3des-cbc,des-cbc-ssh1";

        SSHString tmp = find_matching(supported_ssh_crypt_s_c, v, ',');
        if (tmp.size() == 0){
            ssh_set_error(error, SSH_REQUEST_DENIED,
                "Setting method: no algorithm for method \"%s\" (%s)\n",
                "encryption server->client", v);
            return -1;
        }

        free(opts.wanted_methods[SSH_CRYPT_S_C]);
        opts.wanted_methods[SSH_CRYPT_S_C] = strdup(v);
    }

    return 0;
}

inline int options_set_keyexchange(options_struct & opts, const void *value, error_struct & error)
{
    const char *v = static_cast<const char*>(value);
    if (v[0] == '\0') {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    } else {
        const char * supported_ssh_kex =       "curve25519-sha256@libssh.org,ecdh-sha2-nistp256,diffie-hellman-group14-sha1,diffie-hellman-group1-sha1";

        SSHString tmp = find_matching(supported_ssh_kex, v, ',');
        if (tmp.size() == 0){
            ssh_set_error(error, SSH_REQUEST_DENIED,
                "Setting method: no algorithm for method \"%s\" (%s)\n",
                "kex algos", v);
            return -1;
        }

        free(opts.wanted_methods[SSH_KEX]);
        opts.wanted_methods[SSH_KEX] = strdup(v);
    }

    return 0;
}


inline int options_set_hostkeys(options_struct & opts, const void *value, error_struct & error)
{
    const char *v = static_cast<const char*>(value);
    if (v[0] == '\0') {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }
    else {
        const char * supported_ssh_hostkeys = "ecdsa-sha2-nistp256,ssh-rsa,ssh-dss";

        SSHString tmp = find_matching(supported_ssh_hostkeys, v, ',');
        if (tmp.size() == 0){
            ssh_set_error(error, SSH_REQUEST_DENIED,
                "Setting method: no algorithm for method \"%s\" (%s)\n",
                "server host key algo", v);
            return -1;
        }

        free(opts.wanted_methods[SSH_HOSTKEYS]);
        opts.wanted_methods[SSH_HOSTKEYS] = strdup(v);
    }
    return 0;
}


inline int options_set_compression_c_s(options_struct & opts, const void *value, error_struct & error)
{
    const char *v = static_cast<const char*>(value);
    if (v[0] == '\0') {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }
    else {
        if (strcasecmp(static_cast<const char*>(value),"yes")==0){
            const char * supported_ssh_comp_c_s = "none,zlib,zlib@openssh.com";
            SSHString tmp = find_matching(supported_ssh_comp_c_s, "zlib@openssh.com,zlib", ',');
            if (tmp.size() == 0){
                ssh_set_error(error, SSH_REQUEST_DENIED,
                    "Setting method: no algorithm for method \"%s\" (%s)\n",
                    "compression algo client->server",
                    "zlib@openssh.com,zlib");
                return -1;
            }
            free(opts.wanted_methods[SSH_COMP_C_S]);
            opts.wanted_methods[SSH_COMP_C_S] = strdup("zlib@openssh.com,zlib");
        } else if (strcasecmp(static_cast<const char*>(value),"no")==0){
            const char * supported_ssh_comp_c_s = "none,zlib,zlib@openssh.com";
            SSHString tmp = find_matching(supported_ssh_comp_c_s, "none", ',');
            if (tmp.size() == 0){
                ssh_set_error(error, SSH_REQUEST_DENIED,
                    "Setting method: no algorithm for method \"%s\" (%s)\n",
                    "compression algo client->server",
                    "none");
                return -1;
            }
            free(opts.wanted_methods[SSH_COMP_C_S]);
            opts.wanted_methods[SSH_COMP_C_S] = strdup("none");
        } else {
            const char * supported_ssh_comp_c_s = "none,zlib,zlib@openssh.com";
            SSHString tmp = find_matching(supported_ssh_comp_c_s, v, ',');
            if (tmp.size() == 0){
                ssh_set_error(error, SSH_REQUEST_DENIED,
                    "Setting method: no algorithm for method \"%s\" (%s)\n",
                    "compression algo client->server", v);
                return -1;
            }
            free(opts.wanted_methods[SSH_COMP_C_S]);
            opts.wanted_methods[SSH_COMP_C_S] = strdup(v);
        }
    }

    return 0;
}


inline int options_set_compression_s_c(options_struct & opts, const void *value, error_struct & error)
{
    const char *v = static_cast<const char*>(value);
    if (v[0] == '\0') {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }
    else {
        if (strcasecmp(static_cast<const char*>(value),"yes")==0){
            const char * supported_ssh_comp_s_c = "none,zlib,zlib@openssh.com";
            SSHString tmp = find_matching(supported_ssh_comp_s_c, "zlib@openssh.com,zlib", ',');
            if (tmp.size() == 0){
                ssh_set_error(error, SSH_REQUEST_DENIED,
                    "Setting method: no algorithm for method \"%s\" (%s)\n",
                    "compression algo server->client",
                    "zlib@openssh.com,zlib");
                return -1;
            }
            free(opts.wanted_methods[SSH_COMP_S_C]);
            opts.wanted_methods[SSH_COMP_S_C] = strdup("zlib@openssh.com,zlib");
        } else if (strcasecmp(static_cast<const char*>(value),"no")==0){
            const char * supported_ssh_comp_s_c = "none,zlib,zlib@openssh.com";
            SSHString tmp = find_matching(supported_ssh_comp_s_c, "none", ',');
            if (tmp.size() == 0){
                ssh_set_error(error, SSH_REQUEST_DENIED,
                    "Setting method: no algorithm for method \"%s\" (%s)\n",
                    "compression algo server->client",
                    "none");
                return -1;
            }
            free(opts.wanted_methods[SSH_COMP_S_C]);
            opts.wanted_methods[SSH_COMP_S_C] = strdup("none");
        } else {
            const char * supported_ssh_comp_s_c = "none,zlib,zlib@openssh.com";
            SSHString tmp = find_matching(supported_ssh_comp_s_c, v, ',');
            if (tmp.size() == 0){
                ssh_set_error(error, SSH_REQUEST_DENIED,
                    "Setting method: no algorithm for method \"%s\" (%s)\n",
                    "compression algo server->client", v);
                return -1;
            }
            free(opts.wanted_methods[SSH_COMP_S_C]);
            opts.wanted_methods[SSH_COMP_S_C] = strdup(v);
        }
    }

    return 0;
}


inline int options_set_compression(options_struct & opts, const void *value, error_struct & error)
{
    const char *v = static_cast<const char*>(value);
    if (v[0] == '\0') {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }
    if(options_set_compression_c_s(opts, v, error) < 0)
        return -1;
    if(options_set_compression_s_c(opts, v, error) < 0)
        return -1;

    return 0;
}


inline int options_set_compression_level(options_struct & opts, int value, error_struct & error)
{
    if (value < 1 || value > 9) {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    }
    opts.compressionlevel = value & 0xff;
    return 0;
}

inline int options_set_stricthostkeycheck(options_struct & opts, int value, error_struct & error)
{
    (void)error;
    opts.StrictHostKeyChecking = value;
    return 0;
}


inline int options_set_gssapi_server_identity(options_struct & opts, const void *value, error_struct & error)
{
    const char *v = static_cast<const char*>(value);
    if (v[0] == '\0') {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    } else {
        free(opts.gss_server_identity);
        opts.gss_server_identity = nullptr;
        opts.gss_server_identity = strdup(v);
        // TODO: check memory allocation
    }

    return 0;
}

inline int options_set_gssapi_client_identity(options_struct & opts, const void *value, error_struct & error)
{
    const char *v = static_cast<const char*>(value);
    if (v[0] == '\0') {
        ssh_set_error(error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return -1;
    } else {
        free(opts.gss_client_identity);
        opts.gss_client_identity = nullptr;
        opts.gss_client_identity = strdup(v);
        // TODO: check memory allocation
    }

    return 0;
}

inline int options_set_gssapi_delegate_credentials(options_struct & opts, int value, error_struct & error)
{
    (void)error;
    opts.gss_delegate_creds = (value & 0xff);
    return 0;
}



enum ssh_config_opcode_e {
  SOC_UNSUPPORTED = -1,
  SOC_HOST,
  SOC_HOSTNAME,
  SOC_PORT,
  SOC_USERNAME,
  SOC_IDENTITY,
  SOC_CIPHERS,
  SOC_COMPRESSION,
  SOC_TIMEOUT,
  SOC_PROTOCOL,
  SOC_STRICTHOSTKEYCHECK,
  SOC_KNOWNHOSTS,
  SOC_GSSAPISERVERIDENTITY,
  SOC_GSSAPICLIENTIDENTITY,
  SOC_GSSAPIDELEGATECREDENTIALS,
};

struct ssh_config_keyword_table_s {
  const char *name;
  enum ssh_config_opcode_e opcode;
};

void do_fd_target_event(ssh_poll_handle_fd_struct * fd_poll, int revents);

// Sending data buffered in channel when window is full

//while (len > 0) {
//    while (channel->remote_window == 0) {
//        if (this->session_state == SSH_SESSION_STATE_ERROR){
//            LOG(LOG_INFO, "Wait for a growing window message terminated on error: exiting");
//            return SSH_ERROR;
//        }
//        LOG(LOG_WARNING, "Waiting for growing window Call to handle_packets session_state=%d channel_state=%d",
//            this->session_state, channel->state);
//        if (this->socket == nullptr){
//            return SSH_ERROR;
//        }
//
//        // Waiting for input
//        dopoll(this->ctx, (this->flags & SSH_SESSION_FLAG_BLOCKING)?SSH_TIMEOUT_INFINITE:SSH_TIMEOUT_NONBLOCKING);
//
//        if (this->session_state == SSH_SESSION_STATE_ERROR
//        || this->session_state == SSH_SESSION_STATE_ERROR){
//            LOG(LOG_INFO, "Wait for a growing window message terminated on error: exiting");
//            return SSH_ERROR;
//        }
//    }
//
//    uint32_t effectivelen = len > channel->remote_window ? channel->remote_window : len;
//    if (effectivelen > maxpacketlen) {
//        effectivelen = maxpacketlen;
//    }

//    // TODO: datas are buffered in channel buffer but sent only if remote window
//    // has enough free space. Channels can say if they have enough room to store data.
//    // construction of message and sending will be done later (in polling).

//    this->out_buffer->out_uint8(SSH_MSG_CHANNEL_DATA);
//    this->out_buffer->out_uint32_be(channel->remote_channel);
//    this->out_buffer->out_uint32_be(effectivelen);
//    this->out_buffer->out_blob(data, effectivelen);

//    this->packet_send();

//    LOG(LOG_INFO,
//            "channel_write wrote %ld bytes", (long int) effectivelen);

//    channel->remote_window -= effectivelen;
//    len -= effectivelen;
//    data = ((uint8_t*)data + effectivelen);
//}


int dopoll(ssh_poll_ctx_struct * ctx, int timeout)
{
    LOG(LOG_INFO, "%s ---", __FUNCTION__);

    ssh_pollfd_t pollfds[2];
    bool polling_target = false;

    uint8_t n = 0;
    ssh_pollfd_t & front_pollfd = pollfds[n];


    if (ctx->front_session && ctx->front_session->poll) {
        LOG(LOG_INFO, "%s --- POLLING FRONT to_send=%d received=%d", __FUNCTION__,
             static_cast<int>(ctx->front_session->socket->out_buffer->in_remain()),
             static_cast<int>(ctx->front_session->socket->in_buffer->in_remain()));
        front_pollfd.fd = ctx->front_session->poll->socket->fd_in;
        front_pollfd.events = POLLIN;

        /* If no data is pending at session level
           check if channel data is pending
           TODO: we should probably perform some rotation
           on channels or we may privilege some channels
           above the others
           TODO: should we send as much data as possible at once ?
        */

        if(ctx->front_session->socket->out_buffer->in_remain() == 0){
            ctx->front_session->do_delayed_sending();
        }

        if(ctx->front_session->socket->out_buffer->in_remain() > 0){
            front_pollfd.events |= POLLOUT;
        }

        if (ctx->front_session->socket->state == SSH_SOCKET_CONNECTING){
            front_pollfd.events |= POLLOUT;
        }
        if (ctx->front_session->poll->lock){
            front_pollfd.events &= ~POLLIN;
        }

        front_pollfd.revents = 0;
        n++;
    }

    ssh_pollfd_t & target_pollfd = pollfds[n];

    if ((ctx->target_session && ctx->target_session->poll) || ctx->fd_poll){
        polling_target = true;

        target_pollfd.fd = (ctx->fd_poll) ? ctx->fd_poll->x_fd : ctx->target_session->poll->socket->fd_in;
        target_pollfd.events = POLLIN;
        target_pollfd.events &= ~POLLOUT;

        if (ctx->fd_poll){
            LOG(LOG_INFO, "%s --- POLLING FD TARGET", __FUNCTION__);
        }
        else {
            LOG(LOG_INFO, "%s --- POLLING TARGET to_send=%d received=%d", __FUNCTION__,
                 static_cast<int>(ctx->target_session->socket->out_buffer->in_remain()),
                 static_cast<int>(ctx->target_session->socket->in_buffer->in_remain()));

            if(ctx->target_session->socket->out_buffer->in_remain() > 0){
                LOG(LOG_INFO, "%s --- POLLOUT TARGET because %d to send",
                    __FUNCTION__, static_cast<int>(ctx->target_session->socket->out_buffer->in_remain()));
                target_pollfd.events |= POLLOUT;
            }

            if (ctx->target_session->socket->state == SSH_SOCKET_CONNECTING){
                LOG(LOG_INFO, "%s --- POLLOUT TARGET because connecting", __FUNCTION__);
                target_pollfd.events |= POLLOUT;
            }
        }

        target_pollfd.revents = 0;
        n++;
    }

    int rc = poll(&pollfds[0], n, timeout);
    switch (rc) {
    case -1: // poll error (check errno, what about EINTR or other signals)
        LOG(LOG_INFO, "%s --- POLL ERROR", __FUNCTION__);
        return SSH_ERROR;
    case 0: // timeout
        LOG(LOG_INFO, "%s --- POLL AGAIN", __FUNCTION__);
        return SSH_AGAIN;
    default:;

  }

    if (ctx->front_session && ctx->front_session->poll)
    {
        LOG(LOG_INFO, "%s FRONT state rc=%d revent=%d lock=%d",
            __FUNCTION__, rc, front_pollfd.revents, ctx->front_session->poll->lock);

        /* Do not do anything if this socket was already closed */
        if(front_pollfd.fd != INVALID_SOCKET && front_pollfd.revents){
            ctx->front_session->do_front_event(front_pollfd.revents);

            if (ctx->front_session->waiting_list.size() > 0){
                Event * event = ctx->front_session->waiting_list.back();
                if (event->trigger()){
                    event->action();
                    ctx->front_session->waiting_list.pop_back();
                    delete event;
                }
            }
        }
    }

    if (polling_target && ctx->fd_poll){
        if (target_pollfd.revents){
            do_fd_target_event(ctx->fd_poll, target_pollfd.revents);
        }
    }

    if (polling_target && ctx->target_session){
        if (target_pollfd.revents){
            ctx->target_session->do_target_event(target_pollfd.revents);
        }

        if (ctx->target_session->waiting_list.size() > 0){
            Event * event = ctx->target_session->waiting_list.back();
            if (event->trigger()){
                event->action();
                ctx->target_session->waiting_list.pop_back();
                delete event;
            }
        }
    }

    return SSH_OK;
}


// ==================================== END SSH_MSG_DISCONNECT ================================

// ================================= SSH_MSG_SERVICE_REQUEST =================================


//5.3.  Completion of User Authentication

//   Authentication is complete when the server has responded with
//   SSH_MSG_USERAUTH_SUCCESS.  All authentication related messages
//   received after sending this message SHOULD be silently ignored.

//   After sending SSH_MSG_USERAUTH_SUCCESS, the server starts the
//   requested service.

//5.4.  Banner Message

//   In some jurisdictions, sending a warning message before
//   authentication may be relevant for getting legal protection.  Many
//   UNIX machines, for example, normally display text from /etc/issue,
//   use TCP wrappers, or similar software to display a banner before
//   issuing a login prompt.

//   The SSH server may send an SSH_MSG_USERAUTH_BANNER message at any
//   time after this authentication protocol starts and before
//   authentication is successful.  This message contains text to be
//   displayed to the client user before authentication is attempted.  The
//   format is as follows:

//      byte      SSH_MSG_USERAUTH_BANNER
//      string    message in ISO-10646 UTF-8 encoding [RFC3629]
//      string    language tag [RFC3066]

//   By default, the client SHOULD display the 'message' on the screen.
//   However, since the 'message' is likely to be sent for every login
//   attempt, and since some client software will need to open a separate
//   window for this warning, the client software may allow the user to
//   explicitly disable the display of banners from the server.  The
//   'message' may consist of multiple lines, with line breaks indicated
//   by CRLF pairs.

//   If the 'message' string is displayed, control character filtering,
//   discussed in [SSH-ARCH], SHOULD be used to avoid attacks by sending
//   terminal control characters.

//6.  Authentication Protocol Message Numbers

//   All message numbers used by this authentication protocol are in the
//   range from 50 to 79, which is part of the range reserved for
//   protocols running on top of the SSH transport layer protocol.

//   Message numbers of 80 and higher are reserved for protocols running
//   after this authentication protocol, so receiving one of them before
//   authentication is complete is an error, to which the server MUST
//   respond by disconnecting, preferably with a proper disconnect message
//   sent to ease troubleshooting.

//   After successful authentication, such messages are passed to the
//   higher-level service.

//   These are the general authentication message codes:

//      SSH_MSG_USERAUTH_REQUEST            50
//      SSH_MSG_USERAUTH_FAILURE            51
//      SSH_MSG_USERAUTH_SUCCESS            52
//      SSH_MSG_USERAUTH_BANNER             53

//   In addition to the above, there is a range of message numbers (60 to
//   79) reserved for method-specific messages.  These messages are only
//   sent by the server (client sends only SSH_MSG_USERAUTH_REQUEST
//   messages).  Different authentication methods reuse the same message
//   numbers.



// 7.  Public Key Authentication Method: "publickey"

//   The only REQUIRED authentication 'method name' is "publickey"
//   authentication.  All implementations MUST support this method;
//   however, not all users need to have public keys, and most local
//   policies are not likely to require public key authentication for all
//   users in the near future.

//   With this method, the possession of a private key serves as
//   authentication.  This method works by sending a signature created
//   with a private key of the user.  The server MUST check that the key
//   is a valid authenticator for the user, and MUST check that the
//   signature is valid.  If both hold, the authentication request MUST be
//   accepted; otherwise, it MUST be rejected.  Note that the server MAY
//   require additional authentications after successful authentication.

//   Private keys are often stored in an encrypted form at the client
//   host, and the user must supply a passphrase before the signature can
//   be generated.  Even if they are not, the signing operation involves
//   some expensive computation.  To avoid unnecessary processing and user
//   interaction, the following message is provided for querying whether
//   authentication using the "publickey" method would be acceptable.

//      byte      SSH_MSG_USERAUTH_REQUEST
//      string    user name in ISO-10646 UTF-8 encoding [RFC3629]
//      string    service name in US-ASCII
//      string    "publickey"
//      boolean   FALSE
//      string    public key algorithm name
//      string    public key blob

//   Public key algorithms are defined in the transport layer
//   specification [SSH-TRANS].  The 'public key blob' may contain
//   certificates.

//   Any public key algorithm may be offered for use in authentication.
//   In particular, the list is not constrained by what was negotiated
//   during key exchange.  If the server does not support some algorithm,
//   it MUST simply reject the request.

//   The server MUST respond to this message with either
//   SSH_MSG_USERAUTH_FAILURE or with the following:

//      byte      SSH_MSG_USERAUTH_PK_OK
//      string    public key algorithm name from the request
//      string    public key blob from the request

//   To perform actual authentication, the client MAY then send a
//   signature generated using the private key.  The client MAY send the
//   signature directly without first verifying whether the key is
//   acceptable.  The signature is sent using the following packet:

//      byte      SSH_MSG_USERAUTH_REQUEST
//      string    user name
//      string    service name
//      string    "publickey"
//      boolean   TRUE
//      string    public key algorithm name
//      string    public key to be used for authentication
//      string    signature

//   The value of 'signature' is a signature by the corresponding private
//   key over the following data, in the following order:

//      string    session identifier
//      byte      SSH_MSG_USERAUTH_REQUEST
//      string    user name
//      string    service name
//      string    "publickey"
//      boolean   TRUE
//      string    public key algorithm name
//      string    public key to be used for authentication

//   When the server receives this message, it MUST check whether the
//   supplied key is acceptable for authentication, and if so, it MUST
//   check whether the signature is correct.

//   If both checks succeed, this method is successful.  Note that the
//   server may require additional authentications.  The server MUST
//   respond with SSH_MSG_USERAUTH_SUCCESS (if no more authentications are
//   needed), or SSH_MSG_USERAUTH_FAILURE (if the request failed, or more
//   authentications are needed).

//   The following method-specific message numbers are used by the
//   "publickey" authentication method.

//      SSH_MSG_USERAUTH_PK_OK              60

// PREASSERTION: server_request (no need to check server callbacks)


//9.  Host-Based Authentication: "hostbased"

//   Some sites wish to allow authentication based on the host that the
//   user is coming from and the user name on the remote host.  While this
//   form of authentication is not suitable for high-security sites, it
//   can be very convenient in many environments.  This form of
//   authentication is OPTIONAL.  When used, special care SHOULD be taken
//   to prevent a regular user from obtaining the private host key.

//   The client requests this form of authentication by sending the
//   following message.  It is similar to the UNIX "rhosts" and
//   "hosts.equiv" styles of authentication, except that the identity of
//   the client host is checked more rigorously.

//   This method works by having the client send a signature created with
//   the private key of the client host, which the server checks with that
//   host's public key.  Once the client host's identity is established,
//   authorization (but no further authentication) is performed based on
//   the user names on the server and the client, and the client host
//   name.

//      byte      SSH_MSG_USERAUTH_REQUEST
//      string    user name
//      string    service name
//      string    "hostbased"
//      string    public key algorithm for host key
//      string    public host key and certificates for client host
//      string    client host name expressed as the FQDN in US-ASCII
//      string    user name on the client host in ISO-10646 UTF-8 encoding
//                 [RFC3629]
//      string    signature

//   Public key algorithm names for use in 'public key algorithm for host
//   key' are defined in the transport layer specification [SSH-TRANS].
//   The 'public host key and certificates for client host' may include
//   certificates.

//   The value of 'signature' is a signature with the private host key of
//   the following data, in this order:

//      string    session identifier
//      byte      SSH_MSG_USERAUTH_REQUEST
//      string    user name
//      string    service name
//      string    "hostbased"
//      string    public key algorithm for host key
//      string    public host key and certificates for client host
//      string    client host name expressed as the FQDN in US-ASCII
//      string    user name on the client host in ISO-10646 UTF-8 encoding
//                 [RFC3629]

//   The server MUST verify that the host key actually belongs to the
//   client host named in the message, that the given user on that host is
//   allowed to log in, and that the 'signature' value is a valid
//   signature on the appropriate value by the given host key.  The server
//   MAY ignore the client 'user name', if it wants to authenticate only
//   the client host.

//   Whenever possible, it is RECOMMENDED that the server perform
//   additional checks to verify that the network address obtained from
//   the (untrusted) network matches the given client host name.  This
//   makes exploiting compromised host keys more difficult.  Note that
//   this may require special handling for connections coming through a
//   firewall.


// RFC-4256:
// Generic Message Exchange Authentication for the Secure Shell Protocol (SSH)
// ===========================================================================

// TODO: keybaord-interactive, see RFC-4256


// RFC-4254:
// The Secure Shell (SSH) Connection Protocol
// ==========================================

//5.4.  Channel-Specific Requests

//   Many 'channel type' values have extensions that are specific to that
//   particular 'channel type'.  An example is requesting a pty (pseudo
//   terminal) for an interactive session.

//   All channel-specific requests use the following format.

//      byte      SSH_MSG_CHANNEL_REQUEST
//      uint32    recipient channel
//      string    request type in US-ASCII characters only
//      boolean   want reply
//      ....      type-specific data follows

//   If 'want reply' is FALSE, no response will be sent to the request.
//   Otherwise, the recipient responds with either
//   SSH_MSG_CHANNEL_SUCCESS, SSH_MSG_CHANNEL_FAILURE, or request-specific
//   continuation messages.  If the request is not recognized or is not
//   supported for the channel, SSH_MSG_CHANNEL_FAILURE is returned.

//   This message does not consume window space and can be sent even if no
//   window space is available.  The values of 'request type' are local to
//   each channel type.

//   The client is allowed to send further messages without waiting for
//   the response to the request.

//   'request type' names follow the DNS extensibility naming convention
//   outlined in [SSH-ARCH] and [SSH-NUMBERS].

//      byte      SSH_MSG_CHANNEL_SUCCESS
//      uint32    recipient channel


//      byte      SSH_MSG_CHANNEL_FAILURE
//      uint32    recipient channel

//   These messages do not consume window space and can be sent even if no
//   window space is available.


// 6.3.1.  Requesting X11 Forwarding
// ---------------------------------

//   X11 forwarding may be requested for a session by sending a
//   SSH_MSG_CHANNEL_REQUEST message.

//      byte      SSH_MSG_CHANNEL_REQUEST
//      uint32    recipient channel
//      string    "x11-req"
//      boolean   want reply
//      boolean   single connection
//      string    x11 authentication protocol
//      string    x11 authentication cookie
//      uint32    x11 screen number

//   It is RECOMMENDED that the 'x11 authentication cookie' that is sent
//   be a fake, random cookie, and that the cookie be checked and replaced
//   by the real cookie when a connection request is received.

//   X11 connection forwarding should stop when the session channel is
//   closed.  However, already opened forwardings should not be
//   automatically closed when the session channel is closed.

//   If 'single connection' is TRUE, only a single connection should be
//   forwarded.  No more connections will be forwarded after the first, or
//   after the session channel has been closed.

//   The 'x11 authentication protocol' is the name of the X11
//   authentication method used, e.g., "MIT-MAGIC-COOKIE-1".

//   The 'x11 authentication cookie' MUST be hexadecimal encoded.

//   The X Protocol is documented in [SCHEIFLER].


// 6.5.  Starting a Shell or a Command
// -----------------------------------

//   Once the session has been set up, a program is started at the remote
//   end.  The program can be a shell, an application program, or a
//   subsystem with a host-independent name.  Only one of these requests
//   can succeed per channel.

//      byte      SSH_MSG_CHANNEL_REQUEST
//      uint32    recipient channel
//      string    "shell"
//      boolean   want reply

//   This message will request that the user's default shell (typically
//   defined in /etc/passwd in UNIX systems) be started at the other end.

//      byte      SSH_MSG_CHANNEL_REQUEST
//      uint32    recipient channel
//      string    "exec"
//      boolean   want reply
//      string    command

//   This message will request that the server start the execution of the
//   given command.  The 'command' string may contain a path.  Normal
//   precautions MUST be taken to prevent the execution of unauthorized
//   commands.

//      byte      SSH_MSG_CHANNEL_REQUEST
//      uint32    recipient channel
//      string    "subsystem"
//      boolean   want reply
//      string    subsystem name

//   This last form executes a predefined subsystem.  It is expected that
//   these will include a general file transfer mechanism, and possibly
//   other features.  Implementations may also allow configuring more such
//   mechanisms.  As the user's shell is usually used to execute the
//   subsystem, it is advisable for the subsystem protocol to have a
//   "magic cookie" at the beginning of the protocol transaction to
//   distinguish it from arbitrary output generated by shell
//   initialization scripts, etc.  This spurious output from the shell may
//   be filtered out either at the server or at the client.

//   The server SHOULD NOT halt the execution of the protocol stack when
//   starting a shell or a program.  All input and output from these
//   SHOULD be redirected to the channel or to the encrypted tunnel.

//   It is RECOMMENDED that the reply to these messages be requested and
//   checked.  The client SHOULD ignore these messages.

//   Subsystem names follow the DNS extensibility naming convention
//   outlined in [SSH-NUMBERS].






inline void ssh_poll_cleanup(void) {
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    return;
}


void ssh_kbdint_free(ssh_kbdint kbd) {
    LOG(LOG_INFO, "%s ---", __FUNCTION__);

    if (kbd != nullptr) {
        delete kbd->name;
        delete kbd->instruction;
        delete kbd->echo;

        int n = kbd->nprompts;

        if (kbd->prompts) {
            int i = 0;
            for (i = 0; i < n; i++) {
                delete kbd->prompts[i];
            }
            delete kbd->prompts;
        }

        n = kbd->nanswers;
        if (kbd->answers) {
            int i = 0;
            for (i = 0; i < n; i++) {
                delete kbd->answers[i];
            }
            delete kbd->answers;
        }
        delete kbd;
    }

}

void ssh_kbdint_clean(ssh_kbdint kbd) {
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    int i, n;

    if (kbd == nullptr) {
        return;
    }

    free(kbd->name);
    kbd->name = nullptr;
    free(kbd->instruction);
    kbd->instruction = nullptr;
    free(kbd->echo);
    kbd->echo = nullptr;

    n = kbd->nprompts;
    if (kbd->prompts) {
        for (i = 0; i < n; i++) {
            free(kbd->prompts[i]);
            kbd->prompts[i] = nullptr;
        }
        free(kbd->prompts);
        kbd->prompts = nullptr;
    }

    n = kbd->nanswers;

    if (kbd->answers) {
        for (i = 0; i < n; i++) {
            free(kbd->answers[i]);
            kbd->answers[i] = nullptr;
        }
        free(kbd->answers);
        kbd->answers = nullptr;
    }

    kbd->nprompts = 0;
    kbd->nanswers = 0;
}



ssh_poll_ctx_struct * ssh_new_poll_ctx()
{
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    return new ssh_poll_ctx_struct();
}


// TODO: this should be client session constructor
ssh_session_struct * ssh_new_client_session(ssh_client_callbacks cb, ssh_poll_ctx_struct * ctx, char * host, char * port, char * user, char * hostkeys, char * verbosity, error_struct * error)
{
    (void)verbosity;
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    if (!ctx) {
        LOG(LOG_WARNING, "Client must provide context =========================================== done 1.1");
        return nullptr;
    }

    LOG(LOG_INFO, "%s --- [A]", __FUNCTION__);

    SshClientSession * session = new SshClientSession(ctx, cb, new ssh_socket_struct);

    session->opts.port = atoi(port);
    session->opts.username = strdup(user);
    session->opts.wanted_methods[SSH_HOSTKEYS] = strdup(hostkeys);

    LOG(LOG_INFO, "%s --- [B]", __FUNCTION__);

    // TODO: already done in server, extract init to only do once
//    OpenSSL_add_all_algorithms();

    if (!host) {
        ssh_set_error(*error, SSH_FATAL, "Hostname required");
        LOG(LOG_INFO, "%s --- done 3 ERROR", __FUNCTION__);
        return nullptr;
    }

    LOG(LOG_INFO, "%s --- [C]", __FUNCTION__);

    if (session->opts.username == nullptr) {
        ssh_set_error(*error, SSH_FATAL, "Username required");
        LOG(LOG_INFO, "%s --- done 4 ERROR", __FUNCTION__);
        return nullptr;
    }

    LOG(LOG_INFO, "%s --- [D]", __FUNCTION__);

    session->session_state = SSH_SESSION_STATE_CONNECTING;
    if(session->socket->state != SSH_SOCKET_NONE) {
        ssh_set_error(*error, SSH_FATAL,
                "ssh_socket_connect called on socket not unconnected");
        session->pending_call_state = SSH_PENDING_CALL_NONE;
        return nullptr;
    }

    LOG(LOG_INFO, "%s --- [E]", __FUNCTION__);

    socket_t fd = INVALID_SOCKET;
    struct addrinfo *ai;
    struct addrinfo *itr;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags=AI_NUMERICSERV;

    struct in6_addr dest6;
    struct in_addr dest;

    /* TODO link-local (IP:v6:addr%ifname). */
    // say if it's v6 or v4 ip_addr
    if ((strchr(host, ':') && (inet_pton(AF_INET6, host, &dest6) > 0))
    || (inet_pton(AF_INET, host, &dest) > 0)){
        /* this is an IP address */
        LOG(LOG_INFO,"host %s matches an IP address", host);
        hints.ai_flags |= AI_NUMERICHOST;
    }

    LOG(LOG_INFO, "%s --- [F]", __FUNCTION__);

    // TODO : make that RAII
    int rc = getaddrinfo(host, port, &hints, &ai);

    if (rc != 0) {
        ssh_set_error(*error, SSH_FATAL,
            "Failed to resolve hostname %s (%s)", host, gai_strerror(rc));
        LOG(LOG_INFO,"Invalid socket");
        session->pending_call_state = SSH_PENDING_CALL_NONE;
        return nullptr;
    }

    LOG(LOG_INFO, "%s --- [G]", __FUNCTION__);

    for (itr = ai; itr != nullptr; itr = itr->ai_next){
        /* create socket */
        fd = socket(itr->ai_family, itr->ai_socktype, itr->ai_protocol);
        if (fd < 0) {
          ssh_set_error(*error, SSH_FATAL,
              "Socket create failed: %s", strerror(errno));
          continue;
        }

        rc = fcntl(fd, F_SETFL, O_NONBLOCK);
        if (rc < 0) {
            ssh_set_error(*error, SSH_FATAL,
                "Failed to set socket non-blocking for %s:%d", host, session->opts.port);
            ::close(fd);
            fd = INVALID_SOCKET;
            continue;
        }
        break;
    }
    freeaddrinfo(ai);

    LOG(LOG_INFO, "%s --- [H]", __FUNCTION__);

    LOG(LOG_INFO,"Nonblocking connection socket: %d",fd);
    if(fd == INVALID_SOCKET){
        LOG(LOG_INFO,"Invalid socket");
        session->pending_call_state = SSH_PENDING_CALL_NONE;
        return nullptr;
    }

    connect(fd, itr->ai_addr, itr->ai_addrlen);

    session->socket->fd_in = session->socket->fd_out = fd;
    session->socket->state = SSH_SOCKET_CONNECTING;

    /* POLLOUT is the event to wait for in a nonblocking connect */
    LOG(LOG_WARNING, "new target_poll handle");
    session->poll = new ssh_poll_handle_struct(session->socket);
    session->ctx->fd_poll = nullptr;
    session->ctx->target_session = session;

    LOG(LOG_INFO, "%s --- [I]", __FUNCTION__);

    session->client_callbacks->connect_status_function(session->client_callbacks->userdata, 0.2f);

    LOG(LOG_INFO, "%s --- [J]", __FUNCTION__);

  // CGR: See that, code is irregular here, see why
    session->pending_call_state = SSH_PENDING_CALL_CONNECT;

    struct timeval start;
    gettimeofday(&start, nullptr);
    int timeout = TIMEOUT_DEFAULT_MS;

    LOG(LOG_INFO, "%s --- [K]", __FUNCTION__);

    while(!(session->session_state == SSH_SESSION_STATE_ERROR
      || SSH_SESSION_STATE_AUTHENTICATING == session->session_state
      || SSH_SESSION_STATE_DISCONNECTED == session->session_state)) {

        // Waiting for input
        LOG(LOG_WARNING, "Waiting for connection");
        dopoll(session->ctx, timeout);

        if (session->session_state == SSH_SESSION_STATE_ERROR) {
            session->pending_call_state = SSH_PENDING_CALL_NONE;
            break;
        }
        struct timeval now;
        gettimeofday(&now, nullptr);
        long ms =   (now.tv_sec - start.tv_sec) * 1000
                + (now.tv_usec < start.tv_usec) * 1000
                + (now.tv_usec - start.tv_usec) / 1000;

        if (ms >= timeout)
        {
            break;
        }
        timeout -= ms;
    }

    LOG(LOG_INFO, "%s --- [L]", __FUNCTION__);

    if (SSH_SESSION_STATE_AUTHENTICATING != session->session_state
      && SSH_SESSION_STATE_DISCONNECTED != session->session_state) {
        ssh_set_error(session->error, SSH_FATAL, "Timeout connecting to %s", host);
        session->pending_call_state=SSH_PENDING_CALL_NONE;
        session->session_state = SSH_SESSION_STATE_ERROR;
    }

    session->pending_call_state = SSH_PENDING_CALL_NONE;
    if (session->session_state == SSH_SESSION_STATE_DISCONNECTED){
        LOG(LOG_INFO, "%s --- done SSH_ERROR", __FUNCTION__);
        LOG(LOG_INFO, "%s --- done 7", __FUNCTION__);
        return nullptr;
    }
    LOG(LOG_INFO, "%s --- done SSH_OK", __FUNCTION__);
    //    session->flags &= ~SSH_SESSION_FLAG_BLOCKING;
    return session;
}






ssh_session_struct *  ssh_start_new_server_session(ssh_server_callbacks cb_server,
                                          ssh_poll_ctx_struct * ctx,
                                          int fd, const char * filename, int authmethods)
{
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    LOG(LOG_INFO, "%s --- key filename = %s", __FUNCTION__, filename);

    LOG(LOG_INFO, "ssh_start_new_server_session --- cb_server=%p ctx=%p filename=%p", reinterpret_cast<void*>(cb_server), reinterpret_cast<void*>(ctx), reinterpret_cast<const void*>(filename));

    ctx->front_session = new SshServerSession(ctx, cb_server, new ssh_socket_struct);
    
    if (!ctx->front_session) {
        LOG(LOG_INFO, "session allocation failed");
        return nullptr;
    }

    LOG(LOG_INFO, "ssh_start_new_server_session --- A");

    ctx->front_session->socket->fd_in = ctx->front_session->socket->fd_out = fd;
    ctx->front_session->socket->state = SSH_SOCKET_CONNECTING;

    /* copy options */
    ctx->front_session->opts.bindaddr = nullptr;

    LOG(LOG_WARNING, "new front_poll handle");
    ctx->front_session->poll = new ssh_poll_handle_struct(ctx->front_session->socket);
    ctx->target_session = nullptr;
    ctx->fd_poll = nullptr;

    LOG(LOG_INFO, "ssh_start_new_server_session --- B");


    // TODO: I should put the key in a blob, it's not the library role to read the file
    // I should even be able to provide a key structure created by some other library call
    if (filename == nullptr || *filename == '\0') {
        LOG(LOG_INFO, "Failed to import private host key");
        return nullptr;
    }

    LOG(LOG_INFO, "ssh_start_new_server_session --- C filename=%s", filename);

    FILE *file = fopen(filename, "rb");
    if (file == nullptr) {
        LOG(LOG_INFO, "Error opening %s: %s", filename, strerror(errno));
        LOG(LOG_INFO, "Failed to import private host key");
        return nullptr;
    }

    struct stat sb;
    int rc = fstat(fileno(file), &sb);
    if (rc < 0) {
        fclose(file);
        LOG(LOG_INFO, "Error getting stat of %s: %s", filename, strerror(errno));
        switch (errno) {
            case ENOENT:
            case EACCES:
                LOG(LOG_INFO, "Failed to import private host key");
                return nullptr;
        }
        LOG(LOG_INFO, "Failed to import private host key");
        return nullptr;
    }

    if (sb.st_size > MAX_PRIVKEY_SIZE) {
        LOG(LOG_INFO, "Private key is bigger than 4M.");
        fclose(file);
        LOG(LOG_INFO, "Failed to import private host key");
        return nullptr;
    }

    char *b64_key = new char[sb.st_size + 1];

    off_t size = fread(b64_key, 1, sb.st_size, file);
    fclose(file);

    if (size != sb.st_size) {
        delete [] b64_key;
        LOG(LOG_INFO, "Error reading %s: %s", filename, strerror(errno));
        LOG(LOG_INFO, "Failed to import private host key");
        return nullptr;
    }
    b64_key[size] = 0;

    LOG(LOG_INFO, "ssh_start_new_server_session --- D");

    // TODO: needed for openssl initialization, but only needed once, extract it in global init method
    OpenSSL_add_all_algorithms();
    // TODO: we should do this when stopping using lib
    //    EVP_cleanup();
    //    CRYPTO_cleanup_all_ex_data();

    std::initializer_list<std::pair<const char *, enum ssh_keytypes_e>> l = {
         {DSA_HEADER_BEGIN, SSH_KEYTYPE_DSS},
         {RSA_HEADER_BEGIN, SSH_KEYTYPE_RSA},
         {ECDSA_HEADER_BEGIN, SSH_KEYTYPE_ECDSA}
        };

    LOG(LOG_INFO, "ssh_start_new_server_session --- E");

    enum ssh_keytypes_e type = SSH_KEYTYPE_UNKNOWN;
    for(auto &p:l){
        if (strncmp(p.first, b64_key, strlen(p.first)) == 0){
            type = p.second;
            break;
        }
    }

    if (type == SSH_KEYTYPE_UNKNOWN) {
        LOG(LOG_INFO, "Unknown or invalid private key.");
        LOG(LOG_INFO, "Failed to import private host key");
        return nullptr;
    }

    LOG(LOG_INFO, "ssh_start_new_server_session --- F");

    ssh_key_struct *privkey = new ssh_key_struct(type, SSH_KEY_FLAG_PRIVATE | SSH_KEY_FLAG_PUBLIC);
    char hostkeys[64] = {0};

    LOG(LOG_INFO, "ssh_start_new_server_session --- G");

    switch (type) {
    case SSH_KEYTYPE_DSS:
    {
        LOG(LOG_INFO, "SSH_KEYTYPE_DSS");
        BIO *mem = BIO_new_mem_buf(b64_key, -1);
        privkey->dsa = PEM_read_bio_DSAPrivateKey(mem, nullptr, nullptr, nullptr);
        if (privkey->dsa == nullptr) {
            BIO_free(mem);
            LOG(LOG_INFO, "Parsing private key: %s",
                        ERR_error_string(ERR_get_error(), nullptr));
            LOG(LOG_INFO, "Failed to import private host key");
            return nullptr;
        }
        BIO_free(mem);
        ctx->front_session->server_dsa_key = privkey;
        snprintf(hostkeys, sizeof(hostkeys), "ssh-dss");
    }
    break;
    case SSH_KEYTYPE_RSA1:
    case SSH_KEYTYPE_RSA:
    {
        LOG(LOG_INFO, "SSH_KEYTYPE_RSA");
        BIO *mem = BIO_new_mem_buf(b64_key, -1);
        privkey->rsa = PEM_read_bio_RSAPrivateKey(mem, nullptr, nullptr, nullptr);
        if (privkey->rsa == nullptr) {
            BIO_free(mem);
            LOG(LOG_INFO, "Parsing private key: %s",
                        ERR_error_string(ERR_get_error(),nullptr));
            LOG(LOG_INFO, "Failed to import private host key");
            return nullptr;
        }
        BIO_free(mem);
        ctx->front_session->server_rsa_key = privkey;
        snprintf(hostkeys, sizeof(hostkeys), "ssh-rsa");
    }
    break;
    case SSH_KEYTYPE_ECDSA:
    {
        LOG(LOG_INFO, "SSH_KEYTYPE_ECDSA");
        BIO *mem = BIO_new_mem_buf(b64_key, -1);
        privkey->ecdsa = PEM_read_bio_ECPrivateKey(mem, nullptr, nullptr, nullptr);
        if (privkey->ecdsa == nullptr) {
            BIO_free(mem);
            LOG(LOG_INFO, "Parsing private key: %s",
                        ERR_error_string(ERR_get_error(), nullptr));
            LOG(LOG_INFO, "Failed to import private host key");
            return nullptr;
        }
        privkey->ecdsa_nid = EC_GROUP_get_curve_name(EC_KEY_get0_group(privkey->ecdsa));
        BIO_free(mem);
        ctx->front_session->server_ecdsa_key = privkey;
        snprintf(hostkeys, sizeof(hostkeys), "ssh-ecdsa");
    }
    break;
    case SSH_KEYTYPE_UNKNOWN:
        LOG(LOG_INFO, "SSH_KEYTYPE_UNKNOWN");
        LOG(LOG_INFO, "Unkown or invalid private key type %d", type);
        LOG(LOG_INFO, "Failed to import private host key");
        return nullptr;
    }

    delete [] b64_key;

    /* force PRNG to change state in case we fork after ssh_bind_accept */
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    RAND_add(&tv, sizeof(tv), 0.0);
    ctx->front_session->auth_methods = authmethods & 0x3f;

    memset(ctx->front_session->next_crypto->server_kex.cookie, 0, 16);
    RAND_pseudo_bytes(ctx->front_session->next_crypto->server_kex.cookie, 16);

    LOG(LOG_WARNING, "%s --- Setting KEX methods", __FUNCTION__);


    // TODO: it would be easier to keep supported methods as an array of strings
    // or even distinguished contants than as a coma separated list and parse it everytime.

    // find if hostkey is in the list
    unsigned offset = 0;
    char supported_methods[] = "ecdsa-sha2-nistp256,ssh-rsa,ssh-dss";
    for (;;){
        char * ptr = strstr(supported_methods + offset, hostkeys);
        if (!ptr) {
            ssh_set_error(ctx->front_session->error, SSH_REQUEST_DENIED,
                "Setting method: no algorithm for method \"%s\" (%s)\n",
                "server host key algo",
                hostkeys);
            return nullptr;
        }
        unsigned len = strlen(hostkeys);
        if ((ptr == (supported_methods+offset) || ptr[-1] == ',')
        && (ptr[len] == 0 || ptr[len] == ',')){
            break;
        }
        offset += len;
    }

    ctx->front_session->next_crypto->server_kex.methods[SSH_KEX] = "curve25519-sha256@libssh.org,ecdh-sha2-nistp256,diffie-hellman-group14-sha1,diffie-hellman-group1-sha1";

//        ctx->front_session->next_crypto->server_kex.methods[SSH_HOSTKEYS] = "ecdsa-sha2-nistp256,ssh-rsa,ssh-dss";
    ctx->front_session->next_crypto->server_kex.methods[SSH_HOSTKEYS] = hostkeys;

    ctx->front_session->next_crypto->server_kex.methods[SSH_CRYPT_C_S] = "aes256-ctr,aes192-ctr,aes128-ctr,aes256-cbc,aes192-cbc,aes128-cbc,blowfish-cbc,3des-cbc,des-cbc-ssh1";
    ctx->front_session->next_crypto->server_kex.methods[SSH_CRYPT_S_C] = "aes256-ctr,aes192-ctr,aes128-ctr,aes256-cbc,aes192-cbc,aes128-cbc,blowfish-cbc,3des-cbc,des-cbc-ssh1";
    ctx->front_session->next_crypto->server_kex.methods[SSH_MAC_C_S] = "hmac-sha1";
    ctx->front_session->next_crypto->server_kex.methods[SSH_MAC_S_C] = "hmac-sha1";
    ctx->front_session->next_crypto->server_kex.methods[SSH_COMP_C_S] = "none,zlib,zlib@openssh.com";
    ctx->front_session->next_crypto->server_kex.methods[SSH_COMP_S_C] = "none,zlib,zlib@openssh.com";
    ctx->front_session->next_crypto->server_kex.methods[SSH_LANG_S_C] = "";
    ctx->front_session->next_crypto->server_kex.methods[SSH_LANG_C_S] = "";

    LOG(LOG_WARNING, "%s --- Sending Server Banner", __FUNCTION__);

    // Send server banner
    // ------------------
    char buffer[128] = {0};
    memcpy(ctx->front_session->serverbanner, "SSH-2.0-sashimi-4.1.1.0", strlen("SSH-2.0-sashimi-4.1.1.0")+1);
    snprintf(buffer, 128, "%s\n", ctx->front_session->serverbanner);
    ctx->front_session->socket->out_buffer->out_blob(buffer, strlen(buffer));

    LOG(LOG_INFO, "%s --- done ok", __FUNCTION__);
    return ctx->front_session; // SSH_OK;
}

int packet_decrypt(ssh_crypto_struct & crypto, void *data, uint32_t len, error_struct & error)
{
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
  assert(len);

  if(len % crypto.in_cipher->blocksize != 0){
    ssh_set_error(error,  SSH_FATAL, "Cryptographic functions must be set on at least one blocksize (received %d)",len);
    return SSH_ERROR;
  }
  uint8_t *out = static_cast<unsigned char *>(malloc(len));
  if (out == nullptr) {
    return SSH_ERROR;
  }

  if (crypto.in_cipher->set_decrypt_key(crypto.decryptkey, crypto.decryptIV) < 0) {
    free(out);
    out = nullptr;
    return SSH_ERROR;
  }
  // TODO: much too much copying data around, we should be able to decompress to out directly
  crypto.in_cipher->cbc_decrypt(static_cast<const unsigned char *>(data), out, len);

  memcpy(data, out, len);
  memset(out, 0, len);

  free(out);
  out = nullptr;
  return 0;
}

int ssh_channel_struct::channel_request(ssh_session_struct * session)
{
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    if (session->session_state == SSH_SESSION_STATE_ERROR){
        return SSH_ERROR;
    }

    while(1){
        if (this->request_state != SSH_CHANNEL_REQ_STATE_PENDING){
            break;
        }
        if (session->session_state == SSH_SESSION_STATE_ERROR){
            break;
        }

        // Waiting for input
        dopoll(session->ctx,  (session->flags&SSH_SESSION_FLAG_BLOCKING)
                        ? SSH_TIMEOUT_INFINITE
                        : SSH_TIMEOUT_NONBLOCKING);

        if (session->session_state == SSH_SESSION_STATE_ERROR) {
            this->request_state = SSH_CHANNEL_REQ_STATE_ERROR;
            break;
        }
        if (!(session->flags&SSH_SESSION_FLAG_BLOCKING)){
            break;
        }
    }

    if(session->session_state == SSH_SESSION_STATE_ERROR) {
        this->request_state = SSH_CHANNEL_REQ_STATE_ERROR;
    }
    /* we received something */
    switch (this->request_state){
    case SSH_CHANNEL_REQ_STATE_NONE: // this one should never be called
    case SSH_CHANNEL_REQ_STATE_ERROR:
    case SSH_CHANNEL_REQ_STATE_DENIED: // this one is not an error, we were just denied some feature we asked for
        this->request_state=SSH_CHANNEL_REQ_STATE_NONE;
        return SSH_ERROR;
    case SSH_CHANNEL_REQ_STATE_ACCEPTED:
        this->request_state=SSH_CHANNEL_REQ_STATE_NONE;
        return SSH_OK;
    case SSH_CHANNEL_REQ_STATE_PENDING:
        this->request_state=SSH_CHANNEL_REQ_STATE_PENDING;
        return SSH_AGAIN;
    }
    return SSH_ERROR;
}

#include "channels_server.cpp"
#include "channels_client.cpp"

