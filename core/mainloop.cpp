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
   Copyright (C) Wallix 2010 - 2012
   Author(s): Christophe Grosjean

   Main loop

*/

#include <unistd.h>
#include <signal.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "config.hpp"
#include "mainloop.hpp"
#include "log.hpp"
#include "listen.hpp"
#include "wait_obj.hpp"
#include "session_server.hpp"

/*****************************************************************************/
void shutdown(int sig)
{
    LOG(LOG_INFO, "shutting down : signal %d pid=%d\n", sig, getpid());
    exit(1);
}

/*****************************************************************************/
void sigpipe(int sig)
{
    LOG(LOG_INFO, "got SIGPIPE(%d) : ignoring\n", sig);
}

/*****************************************************************************/
void sigsegv(int sig)
{
    LOG(LOG_INFO, "Ignoring SIGSEGV : signal %d \n", sig);
}

void sigchld(int sig)
{
    // triggered when a child close. For now we will just ignore this signal
    // because there is no child termination management yet.
    // When there will be child management code, we will have to setup
    // some communication protocol to discuss with childs.
    LOG(LOG_INFO, "Ignoring SIGCHLD : signal %d pid %d\n", sig, getpid());
}

int refreshconf;

void sighup(int sig)
{
    refreshconf |= 1;
}

void init_signals(void)
{
    struct sigaction sa;

    sa.sa_flags = 0;

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGSEGV);
    sigaddset(&sa.sa_mask, SIGBUS);
    sigaddset(&sa.sa_mask, SIGTERM);
    sigaddset(&sa.sa_mask, SIGHUP);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGPIPE);
    sigaddset(&sa.sa_mask, SIGCHLD);
    sigaddset(&sa.sa_mask, SIGALRM);
    sigaddset(&sa.sa_mask, SIGUSR1);
    sigaddset(&sa.sa_mask, SIGUSR2);

    sa.sa_handler = SIG_IGN;
    sigaction(SIGSEGV, &sa, NULL);

    sa.sa_handler = SIG_DFL;
    sigaction(SIGBUS, &sa, NULL);

    sa.sa_handler = shutdown;
    sigaction(SIGTERM, &sa, NULL);

    sa.sa_handler = sighup;
    sigaction(SIGHUP, &sa, NULL);

    sa.sa_handler = shutdown;
    sigaction(SIGINT, &sa, NULL);

    sa.sa_handler = sigpipe;
    sigaction(SIGPIPE, &sa, NULL);

    sa.sa_handler = SIG_IGN;
    sigaction(SIGCHLD, &sa, NULL);

    sa.sa_handler = SIG_DFL;
    sigaction(SIGALRM, &sa, NULL);

    sa.sa_handler = SIG_IGN;
    sigaction(SIGUSR1, &sa, NULL);

    sa.sa_handler = SIG_IGN;
    sigaction(SIGUSR2, &sa, NULL);
}

void reset_signals(void)
{
    struct sigaction sa;

    sa.sa_flags = 0;
    sa.sa_handler = SIG_DFL;

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGSEGV);
    sigaddset(&sa.sa_mask, SIGBUS);
    sigaddset(&sa.sa_mask, SIGTERM);
    sigaddset(&sa.sa_mask, SIGHUP);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGPIPE);
    sigaddset(&sa.sa_mask, SIGCHLD);
    sigaddset(&sa.sa_mask, SIGALRM);
    sigaddset(&sa.sa_mask, SIGUSR1);
    sigaddset(&sa.sa_mask, SIGUSR2);

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
}

void redemption_new_session()
{
    char text[256];
    char ip_source[256];

    struct sockaddr_in from;
    int sock_len = sizeof(from);

    Inifile ini(CFG_PATH "/" RDPPROXY_INI);

    init_signals();
    snprintf(text, 255, "redemption_%8.8x_main_term", getpid());

    getpeername(0, (struct sockaddr *)&from, (socklen_t *)&sock_len);
    strcpy(ip_source, inet_ntoa(from.sin_addr));

    int sck = 0;
    if (ini.globals.debug.session){
        LOG(LOG_INFO, "Setting new session socket to %d\n", sck);
    }

    int nodelay = 1;
    if (0 == setsockopt(sck, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay))){
        wait_obj front_event(sck);
        SocketTransport front_trans("RDP Client", sck, ini.globals.debug.front);

        Session session(front_event, front_trans, ip_source, &refreshconf, &ini);

        if (ini.globals.debug.session){
            LOG(LOG_INFO, "Session::end of Session(%u)", sck);
        }

        shutdown(sck, 2);
        close(sck);
    }
    else {
        LOG(LOG_INFO, "Failed to set socket TCP_NODELAY option on client socket");
    }

}

void redemption_main_loop()
{
    init_signals();

    SessionServer ss(&refreshconf);
    Inifile ini(CFG_PATH "/" RDPPROXY_INI);
    int port = ini.globals.port;
    Listen listener(ss, port);
}
