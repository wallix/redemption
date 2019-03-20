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
  Copyright (C) Wallix 2010 - 2012
  Author(s): Christophe Grosjean, Raphael Zhou

  Main loop
*/

#include "core/listen.hpp"
#include "core/mainloop.hpp"
#include "core/session.hpp"
#include "main/version.hpp"
#include "utils/difftimeval.hpp"
#include "utils/log.hpp"
#include "utils/log_siem.hpp"
#include "utils/netutils.hpp"
#include "utils/strutils.hpp"

#include "configs/config.hpp"

#include <cerrno>
#include <csignal>
#include <cstring>

#include <unistd.h>

#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/stat.h>


namespace {
/*****************************************************************************/
[[noreturn]]
void shutdown(int sig)
{
    LOG(LOG_INFO, "shutting down : signal %d pid=%d", sig, getpid());
    exit(1);
}

/*****************************************************************************/
void sigpipe(int sig)
{
    LOG(LOG_INFO, "got SIGPIPE(%d) : ignoring", sig);
}

/*****************************************************************************/
//void sigsegv(int sig)
//{
//    LOG(LOG_INFO, "Ignoring SIGSEGV : signal %d", sig);
//}

void sighup(int sig)
{
    LOG(LOG_INFO, "Ignoring SIGHUP : signal %d", sig);
}

//void sigchld(int sig)
//{
//    // triggered when a child close. For now we will just ignore this signal
//    // because there is no child termination management yet.
//    // When there will be child management code, we will have to setup
//    // some communication protocol to discuss with childs.
//    LOG(LOG_INFO, "Ignoring SIGCHLD : signal %d pid %d", sig, getpid());
//}

void init_signals()
{
    struct sigaction sa;

    sa.sa_flags = 0;

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGBUS);
    sigaddset(&sa.sa_mask, SIGTERM);
    sigaddset(&sa.sa_mask, SIGHUP);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGPIPE);
    sigaddset(&sa.sa_mask, SIGCHLD);
    sigaddset(&sa.sa_mask, SIGALRM);
    sigaddset(&sa.sa_mask, SIGUSR1);
    sigaddset(&sa.sa_mask, SIGUSR2);

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wzero-as-null-pointer-constant")
#if REDEMPTION_COMP_CLANG_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wzero-as-null-pointer-constant")
#endif
    sa.sa_handler = SIG_DFL;
    sigaction(SIGBUS, &sa, nullptr);

    sa.sa_handler = shutdown;
    sigaction(SIGTERM, &sa, nullptr);

    sa.sa_handler = sighup;
    sigaction(SIGHUP, &sa, nullptr);

    sa.sa_handler = shutdown;
    sigaction(SIGINT, &sa, nullptr);

    sa.sa_handler = sigpipe;
    sigaction(SIGPIPE, &sa, nullptr);

    sa.sa_handler = SIG_IGN; /*NOLINT*/
    sigaction(SIGCHLD, &sa, nullptr);

    sa.sa_handler = SIG_DFL;
    sigaction(SIGALRM, &sa, nullptr);

    sa.sa_handler = SIG_IGN; /*NOLINT*/
    sigaction(SIGUSR1, &sa, nullptr);

    sa.sa_handler = SIG_IGN; /*NOLINT*/
    sigaction(SIGUSR2, &sa, nullptr);
REDEMPTION_DIAGNOSTIC_POP
}

}  // namespace

//void reset_signals(void)
//{
//    struct sigaction sa;
//
//    sa.sa_flags = 0;
//    sa.sa_handler = SIG_DFL;
//
//    sigemptyset(&sa.sa_mask);
//    sigaddset(&sa.sa_mask, SIGSEGV);
//    sigaddset(&sa.sa_mask, SIGBUS);
//    sigaddset(&sa.sa_mask, SIGTERM);
//    sigaddset(&sa.sa_mask, SIGHUP);
//    sigaddset(&sa.sa_mask, SIGINT);
//    sigaddset(&sa.sa_mask, SIGPIPE);
//    sigaddset(&sa.sa_mask, SIGCHLD);
//    sigaddset(&sa.sa_mask, SIGALRM);
//    sigaddset(&sa.sa_mask, SIGUSR1);
//    sigaddset(&sa.sa_mask, SIGUSR2);
//
//    sigaction(SIGSEGV, &sa, nullptr);
//    sigaction(SIGBUS, &sa, nullptr);
//    sigaction(SIGTERM, &sa, nullptr);
//    sigaction(SIGHUP, &sa, nullptr);
//    sigaction(SIGINT, &sa, nullptr);
//    sigaction(SIGPIPE, &sa, nullptr);
//    sigaction(SIGCHLD, &sa, nullptr);
//    sigaction(SIGALRM, &sa, nullptr);
//    sigaction(SIGUSR1, &sa, nullptr);
//    sigaction(SIGUSR2, &sa, nullptr);
//}

namespace
{
    enum SocketType : bool
    {
        Ws,
        Tls,
    };

    void session_server_start(
        int incoming_sck, CryptoContext& cctx, Random& rnd, Fstat& fstat, bool forkable,
        unsigned uid, unsigned gid, std::string const& config_filename, bool debug_config,
        SocketType socket_type)
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

        int const sck = accept(incoming_sck, &u.s, &sin_size);
        if (-1 == sck) {
            LOG(LOG_ERR, "Accept failed on socket %d (%s)", incoming_sck, strerror(errno));
            _exit(1);
        }

        /* start new process */
        const pid_t pid = forkable ? fork() : 0;
        switch (pid) {
        case 0: /* child */ {
        // TODO: see exit status of child, we could use it to diagnose session behaviours
        // TODO: we could probably use some session launcher object here. Something like
        // an abstraction layer that would manage either forking of threading behavior
        // this would also likely have some effect on network ressources management
        // (that means the select() on ressources could be managed by that layer)
            close(incoming_sck);

            char source_ip[256];
            utils::strlcpy(source_ip, inet_ntoa(u.s4.sin_addr));
            const bool source_is_localhost = (0 == strcmp(source_ip, "127.0.0.1"));
            REDEMPTION_DIAGNOSTIC_PUSH
            REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast") // only to release
            const int source_port = ntohs(u.s4.sin_port);
            REDEMPTION_DIAGNOSTIC_POP

            Inifile ini;

            configuration_load(ini.configuration_holder(), config_filename);
            ini.set<cfg::debug::config>(debug_config);

            if (ini.get<cfg::debug::session>()){
                LOG(LOG_INFO, "Setting new session socket to %d", sck);
            }

            {
                long long const sec = tvtime().tv_sec;
                int const pid = getpid();
                char psid[128];
                std::sprintf(psid, "%lld%d", sec, pid);
                psid[sizeof(psid)-1] = '\0';
                ini.set_acl<cfg::context::psid>(psid);
                log_proxy::init(psid, source_ip, source_port);
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
                LOG(LOG_ERR, "getsockname failed error=%s", strerror(errno));
                _exit(1);
            }

            REDEMPTION_DIAGNOSTIC_PUSH
            REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast") // only to release
            const int target_port = ntohs(localAddress.s4.sin_port);
            REDEMPTION_DIAGNOSTIC_POP
            char target_ip[256];
            utils::strlcpy(target_ip, inet_ntoa(localAddress.s4.sin_addr));
            if (!ini.get<cfg::debug::fake_target_ip>().empty()){
                utils::strlcpy(target_ip, ini.get<cfg::debug::fake_target_ip>().c_str());
                LOG(LOG_INFO, "fake_target_ip='%s'", target_ip);
            }

            if (!source_is_localhost){
                // do not log early messages for localhost (to avoid tracing in watchdog)
                LOG(LOG_INFO, "Redemption " VERSION);
                LOG(LOG_INFO, "src=%s sport=%d dst=%s dport=%d", source_ip, source_port, target_ip, target_port);
            }

            char real_target_ip[256];
            if (ini.get<cfg::globals::enable_transparent_mode>() && !source_is_localhost) {
                int fd = open("/proc/net/ip_conntrack", O_RDONLY);
                // source and dest are inverted because we get the information we want from reply path rule
                int res = parse_ip_conntrack(fd, target_ip, source_ip, target_port, source_port, real_target_ip, sizeof(real_target_ip), 1);
                if (res){
                    LOG(LOG_WARNING, "Failed to get transparent proxy target from ip_conntrack: %d", fd);
                }
                close(fd);

                if (setgid(gid) != 0){
                    LOG(LOG_ERR, "Changing process group to %u failed with error: %s", gid, strerror(errno));
                    _exit(1);
                }
                if (setuid(uid) != 0){
                    LOG(LOG_ERR, "Changing process user to %u failed with error: %s", uid, strerror(errno));
                    _exit(1);
                }

                LOG(LOG_INFO, "src=%s sport=%d dst=%s dport=%d", source_ip, source_port, real_target_ip, target_port);
            }
            else {
                ::memset(real_target_ip, 0, sizeof(real_target_ip));
            }

            int nodelay = 1;
            if (socket_type == SocketType::Ws
             || 0 == setsockopt(sck, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay))
            ){
                // Create session file
                int child_pid = getpid();
                char session_file[256];
                sprintf(session_file, "%s/session_%d.pid", app_path(AppPath::LockDir), child_pid);
                int fd = open(session_file, O_WRONLY | O_CREAT, S_IRWXU);
                if (fd == -1) {
                    LOG(LOG_ERR, "Writing process id to SESSION ID FILE failed. Maybe no rights ?:%d:%s", errno, strerror(errno));
                    _exit(1);
                }
                char text[256];
                const size_t lg = snprintf(text, 255, "%d", child_pid);
                if (write(fd, text, lg) == -1) {
                    LOG(LOG_ERR, "Couldn't write pid to %s/session_<pid>.pid: %s", app_path(AppPath::LockDir), strerror(errno));
                    _exit(1);
                }
                close(fd);

                // Launch session
                if (!source_is_localhost){
                    // do not log early messages for localhost (to avoid tracing in watchdog)
                    LOG(LOG_INFO,
                        "New session on %d (pid=%d) from %s to %s",
                        sck, child_pid, source_ip, (real_target_ip[0] ? real_target_ip : target_ip));
                }
                ini.set_acl<cfg::globals::host>(source_ip);
                ini.set_acl<cfg::globals::target>(target_ip);
                if (ini.get<cfg::globals::enable_transparent_mode>()
                    && 0 != strncmp(target_ip, real_target_ip, strlen(real_target_ip))) {
                    ini.set_acl<cfg::context::real_target_device>(real_target_ip);
                }

                switch (socket_type) {
                    case SocketType::Ws:
                        session_start_ws(unique_fd{sck}, ini, cctx, rnd, fstat);
                        [[fallthrough]];
                    case SocketType::Tls:
                        session_start_tls(unique_fd{sck}, ini, cctx, rnd, fstat);
                }

                // Suppress session file
                unlink(session_file);

                if (ini.get<cfg::debug::session>()){
                    LOG(LOG_INFO, "Session::end of Session(%d)", sck);
                }
            }
            else {
                LOG(LOG_ERR, "Failed to set socket TCP_NODELAY option on client socket: %d: %s", errno, strerror(errno));
            }

            _exit(0);
        }
        default: /* father */
            close(sck);
            break;
        case -1:
            // error forking
            LOG(LOG_ERR, "Error creating process for new session : %s", strerror(errno));
            break;
        }
    }

    unique_fd create_ws_server(
        uint32_t s_addr, char const* ws_addr,
        EnableTransparentMode enable_transparent_mode)
    {
        // "[:]port"
        {
            const unsigned pos = (ws_addr[0] == ':' ? 1 : 0);
            char* end = nullptr;
            long port = std::strtol(ws_addr + pos, &end, 10);
            if (*end == '\0') {
                return create_server(s_addr, port, enable_transparent_mode);
            }
        }

        // "addr:port"
        const char* ws_port = strchr(ws_addr, ':');
        if (ws_port) {
            std::string listen_addr(ws_addr, ws_port);
            uint32_t ws_iaddr = inet_addr(listen_addr.c_str());
            REDEMPTION_DIAGNOSTIC_PUSH
            REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
            REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wuseless-cast")
            if (ws_iaddr == INADDR_NONE) { ws_iaddr = INADDR_ANY; }
            REDEMPTION_DIAGNOSTIC_POP
            char* end = nullptr;
            long port = std::strtol(ws_port + 1, &end, 10);
            if (*end == '\0') {
                return create_server(ws_iaddr, port, enable_transparent_mode);
            }
        }

        // removed previous websocket
        if (struct stat buf; 0 == stat(ws_addr, &buf) && S_ISSOCK(buf.st_mode)) {
            unlink(ws_addr);
        }
        return create_unix_server(ws_addr, enable_transparent_mode);
    }
} // anonymous namespace

void redemption_main_loop(
    Inifile & ini, CryptoContext & cctx, Random & rnd, Fstat & fstat,
    unsigned uid, unsigned gid, std::string config_filename, bool forkable)
{
    init_signals();

    // Inifile ini(CFG_PATH "/" RDPPROXY_INI);
    uint32_t s_addr = inet_addr(ini.get<cfg::globals::listen_address>().c_str());
    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
    REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wuseless-cast")
    if (s_addr == INADDR_NONE) { s_addr = INADDR_ANY; }
    REDEMPTION_DIAGNOSTIC_POP

    const bool debug_config = (ini.get<cfg::debug::config>() == Inifile::ENABLE_DEBUG_CONFIG);
    const EnableTransparentMode enable_transparent_mode
      = EnableTransparentMode(ini.get<cfg::globals::enable_transparent_mode>());

    unique_fd sck1 = create_server(s_addr, ini.get<cfg::globals::port>(), enable_transparent_mode);

    if (ini.get<cfg::globals::enable_websocket>())
    {
        unique_fd sck2 = create_ws_server(
            s_addr, ini.get<cfg::globals::websocket_addr>().c_str(), enable_transparent_mode);
        const auto ws_sck = sck2.fd();
        two_server_loop(std::move(sck1), std::move(sck2), [&](int sck)
        {
            auto const socket_type = (ws_sck == sck) ? SocketType::Ws : SocketType::Tls;
            session_server_start(sck, cctx, rnd, fstat, forkable, uid, gid, config_filename, debug_config, socket_type);
            return true;
        });
    }
    else
    {
        unique_server_loop(std::move(sck1), [&](int sck)
        {
            session_server_start(sck, cctx, rnd, fstat, forkable, uid, gid, config_filename, debug_config, SocketType::Tls);
            return true;
        });
    }
}
