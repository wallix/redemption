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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   Network related utility functions

*/

#include "utils/netutils.hpp"

#include "regex/regex.hpp"
#include "utils/log.hpp"
#include "utils/select.hpp"

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/un.h>


bool try_again(int errnum)
{
    int res = false;
    // TODO Check wich signals are actually necessary depending on what we are doing
    // looks like EINPROGRESS or EALREADY only occurs when calling connect()
    // EAGAIN is when blocking IO would block (other name for EWOULDBLOCK)
    // EINTR when an interruption stopped system call (and we could do it again)
    switch (errnum){
        case EAGAIN:
        /* case EWOULDBLOCK: */ // same as EAGAIN on Linux
        case EINPROGRESS:
        case EALREADY:
        case EBUSY:
        case EINTR:
            res = true;
            break;
        default:
            ;
    }
    return res;
}

namespace
{
    bool set_snd_buffer(int sck, int buffer_size)
    {
        /* set snd buffer to at least 32 Kbytes */
        int snd_buffer_size = buffer_size;
        socklen_t option_len = static_cast<socklen_t>(sizeof(snd_buffer_size));
        if (0 == getsockopt(sck, SOL_SOCKET, SO_SNDBUF, &snd_buffer_size, &option_len)) {
            if (snd_buffer_size < buffer_size) {
                snd_buffer_size = buffer_size;
                if (-1 == setsockopt(sck,
                        SOL_SOCKET,
                        SO_SNDBUF,
                        &snd_buffer_size, sizeof(snd_buffer_size))){
                    LOG(LOG_WARNING, "setsockopt failed with errno = %d (%s)", errno, strerror(errno));
                    return false;
                }
            }
        }
        else {
            LOG(LOG_WARNING, "getsockopt failed with errno = %d (%s)", errno, strerror(errno));
            return false;
        }

        return true;
    }

    unique_fd connect_sck(int sck, int nbretry, int retry_delai_ms, sockaddr & addr,
                          socklen_t addr_len, const char * target, char const** error_result = nullptr)
    {
        fcntl(sck, F_SETFL, fcntl(sck, F_GETFL) | O_NONBLOCK);

        int trial = 0;
        for (; trial < nbretry ; trial++){
            int const res = ::connect(sck, &addr, addr_len);
            if (-1 != res){
                // connection suceeded
                break;
            }

            int const err =  errno;
            if (trial > 0){
                char const* errmes = strerror(err);
                if (error_result) {
                    *error_result = errmes;
                }
                LOG(LOG_INFO, "Connection to %s failed with errno = %d (%s)", target, err, errmes);
            }

            if ((err == EINPROGRESS) || (err == EALREADY)){
                // try again
                fd_set fds;
                io_fd_zero(fds);
                io_fd_set(sck, fds);
                struct timeval timeout = {
                    retry_delai_ms / 1000,
                    1000 * (retry_delai_ms % 1000)
                };
                // exit select on timeout or connect or error
                // connect will catch the actual error if any,
                // no need to care of select result
                select(sck+1, nullptr, &fds, nullptr, &timeout);
            }
            else {
                // real failure
                trial = nbretry;
                break;
            }
        }

        if (trial >= nbretry){
            if (error_result) {
                *error_result = "All trials done";
            }
            LOG(LOG_ERR, "All trials done connecting to %s", target);
            return unique_fd{-1};
        }

        LOG(LOG_INFO, "connection to %s succeeded : socket %d", target, sck);
        return unique_fd{sck};
    }
} // namespace

char const* resolve_ipv4_address(const char* ip, in_addr & s4_sin_addr)
{
    if (!inet_aton(ip, &s4_sin_addr)) {
        struct addrinfo * addr_info = nullptr;
        int               result    = getaddrinfo(ip, nullptr, nullptr, &addr_info);
        if (result) {
            char const* error = (result == EAI_SYSTEM) ? strerror(errno) : gai_strerror(result);
            LOG(LOG_ERR, "DNS resolution failed for %s with errno = %d (%s)",
                ip, (result == EAI_SYSTEM) ? errno : result, error);
            return error;
        }
        s4_sin_addr.s_addr = (reinterpret_cast<sockaddr_in *>(addr_info->ai_addr))->sin_addr.s_addr; /*NOLINT*/
        freeaddrinfo(addr_info);
    }
    return nullptr;
}

unique_fd ip_connect(const char* ip, int port, int nbretry /* 3 */, int retry_delai_ms /*1000*/, char const** error_result)
{
    LOG(LOG_INFO, "connecting to %s:%d", ip, port);

    // we will try connection several time
    // the trial process include "socket opening, hostname resolution, etc
    // because some problems can come from the local endpoint,
    // not necessarily from the remote endpoint.
    int sck = socket(PF_INET, SOCK_STREAM, 0);

    union
    {
      sockaddr s;
      sockaddr_storage ss;
      sockaddr_in s4;
      sockaddr_in6 s6;
    } u;

    memset(&u, 0, sizeof(u));
    u.s4.sin_family = AF_INET;
    REDEMPTION_DIAGNOSTIC_PUSH
    REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast") // only to release
    u.s4.sin_port = htons(port);
    REDEMPTION_DIAGNOSTIC_POP
    if (auto error = resolve_ipv4_address(ip, u.s4.sin_addr)){
        if (error_result) {
            *error_result = error;
        }
        LOG(LOG_ERR, "Connecting to %s:%d failed", ip, port);
        close(sck);
        return unique_fd{-1};
    }

    /* set snd buffer to at least 32 Kbytes */
    if (!set_snd_buffer(sck, 32768)) {
        if (error_result) {
            *error_result = "Cannot set socket buffer size";
        }
        LOG(LOG_ERR, "Connecting to %s:%d failed : cannot set socket buffer size", ip, port);
        close(sck);
        return unique_fd{-1};
    }

    char text_target[256];
    snprintf(text_target, sizeof(text_target), "%s:%d (%s)", ip, port, inet_ntoa(u.s4.sin_addr));

    return connect_sck(sck, nbretry, retry_delai_ms, u.s, sizeof(u), text_target, error_result);
}


// TODO int retry_delai_ms -> std::milliseconds
unique_fd local_connect(const char* sck_name, int nbretry, int retry_delai_ms)
{
    char target[1024] = {};
    snprintf(target, sizeof(target), "%s", sck_name);

    LOG(LOG_INFO, "connecting to %s", sck_name);
    // we will try connection several time
    // the trial process include "ocket opening, hostname resolution, etc
    // because some problems can come from the local endpoint,
    // not necessarily from the remote endpoint.
    int sck = socket(AF_UNIX, SOCK_STREAM, 0);

    /* set snd buffer to at least 32 Kbytes */
    if (!set_snd_buffer(sck, 32768)) {
        return unique_fd{-1};
    }

    union
    {
      sockaddr_un s;
      sockaddr addr;
    } u;

    auto len = strnlen(sck_name, sizeof(u.s.sun_path)-1u);
    memcpy(u.s.sun_path, sck_name, len);
    u.s.sun_path[len] = 0;
    u.s.sun_family = AF_UNIX;

    return connect_sck(sck, nbretry, retry_delai_ms, u.addr, static_cast<int>(offsetof(sockaddr_un, sun_path) + strlen(u.s.sun_path) + 1u), target);
}



struct LineBuffer
{
    char buffer[20480];
    int end_buffer;

    int fd;
    int begin_line;
    int eol;

    explicit LineBuffer(int fd)
    : end_buffer(0)
    , fd(fd)
    , begin_line(0)
    , eol(0)
    {
    }

    int readline()
    {
        for (int i = this->begin_line; i < this->end_buffer; i++){
            if (this->buffer[i] == '\n'){
                this->eol = i+1;
                return 1;
            }
        }
        size_t trailing_room = sizeof(this->buffer) - this->end_buffer;
        // reframe buffer if no trailing room left
        if (trailing_room == 0){
            size_t used_len = this->end_buffer - this->begin_line;
            memmove(this->buffer, &(this->buffer[this->begin_line]), used_len);
            this->end_buffer = used_len;
            this->begin_line = 0;
        }

        ssize_t res = read(this->fd, &(this->buffer[this->end_buffer]), sizeof(this->buffer) - this->end_buffer);
        if (res < 0){
            return res;
        }
        this->end_buffer += res;
        if (this->begin_line == this->end_buffer) {
            return 0;
        }
        for (int i = this->begin_line; i < this->end_buffer; i++){
            if (this->buffer[i] == '\n'){
                this->eol = i+1;
                return 1;
            }
        }
        this->eol = this->end_buffer;
        return 1;
    }
};

// return 0 if found, -1 not found or error
int parse_ip_conntrack(int fd, const char * source, const char * dest, int sport, int dport,
                       char * transparent_dest, size_t sz_transparent_dest, uint32_t verbose)
{
    LineBuffer line(fd);
    //"tcp      6 299 ESTABLISHED src=10.10.43.13 dst=10.10.47.93 sport=36699 dport=22 packets=5256 bytes=437137 src=10.10.47.93 dst=10.10.43.13 sport=22 dport=36699 packets=3523 bytes=572101 [ASSURED] mark=0 secmark=0 use=2\n"

    char strre[512];
#define RE_IP_DEF "\\d\\d?\\d?\\.\\d\\d?\\d?\\.\\d\\d?\\d?\\.\\d\\d?\\d?"
    // sprintf(strre,
    //         "^ *6 +\\d+ +ESTABLISHED +"
    //         "src=" RE_IP_DEF " +"
    //         "dst=(" RE_IP_DEF ") +"
    //         "sport=\\d+ +dport=\\d+( +packets=\\d+ bytes=\\d+)? +"
    //         "src=%s +"
    //         "dst=%s +"
    //         "sport=%d +dport=%d( +packets=\\d+ bytes=\\d+)? +"
    //         "\\[ASSURED] +mark=\\d+ +secmark=\\d+ use=\\d+$",
    //         source, dest, sport, dport
    // );
    sprintf(strre,
            "^ *6 +\\d+ +ESTABLISHED +"
            "src=" RE_IP_DEF " +"
            "dst=(" RE_IP_DEF ") +"
            "sport=\\d+ +dport=\\d+ .*"
            "src=%s +"
            "dst=%s +"
            "sport=%d +dport=%d .*"
            "\\[ASSURED].*",
            source, dest, sport, dport
    );
#undef RE_IP_DEF
    re::Regex regex(strre);

    int status = line.readline();
    for (; status == 1 ; (line.begin_line = line.eol), (status = line.readline())) {
        if (verbose) {
            fprintf(stderr, "Line: %.*s", line.eol - line.begin_line, &line.buffer[line.begin_line]);
        }

        if (line.eol - line.begin_line < 100) {
            continue;
        }

        const char * s = line.buffer + line.begin_line;
        if (s[0] != 't' || s[1] != 'c' || s[2] != 'p' || s[3] != ' ') {
            continue ;
        }
        s += 4;

        const bool contains_endl = line.buffer[line.eol-1] == '\n';
        if (contains_endl) {
            line.buffer[line.eol-1] = 0;
        }

        re::Regex::range_matches matches = regex.exact_match(s);
        if ( ! matches.empty() ) {
            const size_t match_size = matches[0].second - matches[0].first;
            if (match_size >= sz_transparent_dest){
                LOG(LOG_WARNING, "No enough space to store transparent ip target address");
                return -1;
            }

            memcpy(transparent_dest, matches[0].first, match_size);
            transparent_dest[match_size] = 0;

            if (verbose) {
                LOG(LOG_INFO, "Match found: %s", transparent_dest);
            }

            return 0;
        }

        if (contains_endl) {
            line.buffer[line.eol-1] = '\n';
        }
    }
    // transparent ip route not found in ip_conntrack
    return -1;
}
