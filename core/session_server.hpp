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

*/

#if !defined(__CORE_SESSION_SERVER_HPP__)
#define __CORE_SESSION_SERVER_HPP__

#include "session.hpp"

class SessionServer : public Server
{
    virtual Server_status start(int incoming_sck)
    {
        struct sockaddr_in sin;
        unsigned int sin_size = sizeof(struct sockaddr_in);
        memset(&sin, 0, sin_size);
        TODO("We should manage accept errors")
        int sck = accept(incoming_sck, (struct sockaddr*)&sin, &sin_size);
        char ip_source[256];
        strcpy(ip_source, inet_ntoa(sin.sin_addr));
        /* start new process */
        pid_t pid = fork();
        switch (pid) {
        case 0: /* child */
        {
            close(incoming_sck);
            LOG(LOG_INFO, "Setting new session socket to %d\n", sck);
            Inifile ini(CFG_PATH "/" RDPPROXY_INI);
            Session session(sck, ip_source, &ini);
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
            LOG(LOG_INFO, "Error creating process for new session : %s\n", strerror(errno));
        break;
        }
        return START_FAILED;
    }
};

#endif
