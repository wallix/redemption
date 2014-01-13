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
  Copyright (C) Wallix 2012
  Author(s): Christophe Grosjean
*/

#ifndef _REDEMPTION_CORE_SESSION_SERVER_HPP_
#define _REDEMPTION_CORE_SESSION_SERVER_HPP_

#include "config.hpp"
#include "ssl_calls.hpp"
#include "server.hpp"
#include "session.hpp"

class SessionServer : public Server
{
    // Used for enable transparent proxying on accepted socket (ini.globals.enable_ip_transparent = true).
    unsigned uid;
    unsigned gid;

    const char * key0;
    const char * key1;

public:
    SessionServer(unsigned uid, unsigned gid, const char * key0, const char * key1)
        : uid(uid)
        , gid(gid)
        , key0(key0)
        , key1(key1) {
    }

    virtual Server_status start(int incoming_sck)
    {
        union
        {
            struct sockaddr s;
            struct sockaddr_storage ss;
            struct sockaddr_in s4;
            struct sockaddr_in6 s6;
        } u;
        unsigned int sin_size = sizeof(u);
        memset(&u, 0, sin_size);

        int sck = accept(incoming_sck, &u.s, &sin_size);
        if (-1 == sck) {
            LOG(LOG_INFO, "Accept failed on socket %u (%s)", incoming_sck, strerror(errno));
            _exit(1);
        }

        char text[256];
        char source_ip[256];
        int source_port = 0;
        char target_ip[256];
        int target_port = 0;
        char real_target_ip[256];

        strcpy(source_ip, inet_ntoa(u.s4.sin_addr));
        source_port = ntohs(u.s4.sin_port);
        /* start new process */
        pid_t pid = fork();
        switch (pid) {
        case 0: /* child */
            {
                close(incoming_sck);

                Inifile ini;
                ConfigurationLoader cfg_loader(ini, CFG_PATH "/" RDPPROXY_INI);

                memcpy(ini.crypto.key0, this->key0, sizeof(ini.crypto.key0));
                memcpy(ini.crypto.key1, this->key1, sizeof(ini.crypto.key1));

                if (ini.debug.session){
                    LOG(LOG_INFO, "Setting new session socket to %d\n", sck);
                }

                union
                {
                    struct sockaddr s;
                    struct sockaddr_storage ss;
                    struct sockaddr_in s4;
                    struct sockaddr_in6 s6;
                } localAddress;
                socklen_t addressLength = sizeof(localAddress);


                if (-1 == getsockname(sck, &localAddress.s, &addressLength)){
                    LOG(LOG_INFO, "getsockname failed error=%s", strerror(errno));
                    _exit(1);
                }

                target_port = ntohs(localAddress.s4.sin_port);
                strcpy(real_target_ip, inet_ntoa(localAddress.s4.sin_addr));

                if (ini.globals.enable_ip_transparent) {
                    strcpy(target_ip, inet_ntoa(localAddress.s4.sin_addr));

                    LOG(LOG_INFO, "src=%s sport=%d dst=%s dport=%d", source_ip, source_port, target_ip, target_port);

                    int fd = open("/proc/net/ip_conntrack", O_RDONLY);
                    // source and dest are inverted because we get the information we want from reply path rule
                    int res = parse_ip_conntrack(fd, target_ip, source_ip, target_port, source_port, real_target_ip, sizeof(real_target_ip), 1);
                    if (res){
                        LOG(LOG_WARNING, "Failed to get transparent proxy target from ip_conntrack: %d", fd);
                    }
                    close(fd);

                    if (setgid(this->gid) != 0){
                        LOG(LOG_WARNING, "Changing process group to %u failed with error: %s\n", this->gid, strerror(errno));
                        _exit(1);
                    }
                    if (setuid(this->uid) != 0){
                        LOG(LOG_WARNING, "Changing process group to %u failed with error: %s\n", this->gid, strerror(errno));
                        _exit(1);
                    }
                }

                LOG(LOG_INFO, "src=%s sport=%d dst=%s dport=%d", source_ip, source_port, real_target_ip, target_port);

                int nodelay = 1;
                if (0 == setsockopt(sck, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay))){
                    // Create session file
                    int child_pid = getpid();
                    char session_file[256];
                    sprintf(session_file, "%s/redemption/session_%d.pid", PID_PATH, child_pid);
                    int fd = open(session_file, O_WRONLY | O_CREAT, S_IRWXU);
                    if (fd == -1) {
                        LOG(LOG_ERR, "Writing process id to SESSION ID FILE failed. Maybe no rights ?:%d:%d\n", errno, strerror(errno));
                        _exit(1);
                    }
                    size_t lg = snprintf(text, 255, "%d", child_pid);
                    if (write(fd, text, lg) == -1) {
                        LOG(LOG_ERR, "Couldn't write pid to %s: %s", PID_PATH "/redemption/session_<pid>.pid", strerror(errno));
                        _exit(1);
                    }
                    close(fd);

                    // Launch session
                    LOG(LOG_INFO, "New session on %u (pid=%u) from %s to %s", (unsigned)sck, (unsigned)child_pid, source_ip, real_target_ip);
                    ini.context_set_value(AUTHID_HOST, source_ip);
                    ini.context_set_value(AUTHID_TARGET, real_target_ip);
                    if (ini.globals.enable_ip_transparent
                        &&  strncmp(target_ip, real_target_ip, strlen(real_target_ip))) {
                        ini.context_set_value(AUTHID_REAL_TARGET_DEVICE, real_target_ip);
                    }
                    Session session(sck, &ini);

                    // Suppress session file
                    unlink(session_file);

                    if (ini.debug.session){
                        LOG(LOG_INFO, "Session::end of Session(%u)", sck);
                    }

                    shutdown(sck, 2);
                    close(sck);
                }
                else {
                    LOG(LOG_ERR, "Failed to set socket TCP_NODELAY option on client socket");
                }
                return START_WANT_STOP;
            }
            break;
        default: /* father */
            {
                close(sck);
            }
            break;
        case -1:
            // error forking
            LOG(LOG_ERR, "Error creating process for new session : %s\n", strerror(errno));
            break;
        }
        return START_FAILED;
    }
};

#endif
