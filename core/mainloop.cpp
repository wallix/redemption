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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat, Olivier Hervieu
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Main loop

*/

#include <unistd.h>
#include <locale.h>
#include <stdio.h>
#include <signal.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>

#include "mainloop.hpp"
#include "log.hpp"
#include "listen.hpp"
#include "wait_obj.hpp"
#include "session.hpp"


/*****************************************************************************/
void xrdp_shutdown(int sig)
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

    sa.sa_handler = SIG_DFL;
    sigaction(SIGSEGV, &sa, NULL);

    sa.sa_handler = SIG_DFL;
    sigaction(SIGBUS, &sa, NULL);

    sa.sa_handler = xrdp_shutdown;
    sigaction(SIGTERM, &sa, NULL);

    sa.sa_handler = xrdp_shutdown;
    sigaction(SIGHUP, &sa, NULL);

    sa.sa_handler = xrdp_shutdown;
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

    try {
        Session session(0, ip_source, &ini);
        session.session_main_loop();
    } catch(...) {
    };
}

void redemption_main_loop()
{

    init_signals();

    { /* block to ensure destructor is called immediately */
        Listen listener;
        listener.listen_main_loop();
    }
}
