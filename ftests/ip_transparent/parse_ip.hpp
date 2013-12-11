/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_FTESTS_IP_TRANSPARENT_PARSE_IP_HPP
#define REDEMPTION_FTESTS_IP_TRANSPARENT_PARSE_IP_HPP

#include <unistd.h>
#include <signal.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <stdio.h>

struct LineBuffer
{
    char buffer[20480];
    int end_buffer;

    int fd;
    int begin_line;
    int eol;
    int eollen;

    LineBuffer(int fd)
        : end_buffer(0)
        , fd(fd)
        , begin_line(0)
        , eol(0)
        , eollen(1)
    {
    }

    int readline()
    {
        for (int i = this->begin_line; i < this->end_buffer; i++){
            if (this->buffer[i] == '\n'){
                this->eol = i+1;
                this->eollen = 1;
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
                this->eollen = 1;
                return 1;
            }
        }
        this->eol = this->end_buffer;
        this->eollen = 0;
        return 1;
    }
};

#include  "../regex/regex.hpp"

// return 0 if found, -1 not found or error
static inline int parse_ip_conntrack(int fd, const char * source, const char * dest, int sport, int dport, char * transparent_dest, int sz_transparent_dest, uint32_t /*verbose*/ = 0)
{
    LineBuffer line(fd);
    int status = line.readline();
    //"tcp      6 299 ESTABLISHED src=10.10.43.13 dst=10.10.47.93 sport=36699 dport=22 packets=5256 bytes=437137 src=10.10.47.93 dst=10.10.43.13 sport=22 dport=36699 packets=3523 bytes=572101 [ASSURED] mark=0 secmark=0 use=2\n"

    char strre[512];
#define RE_IP_DEF "\\d\\d?\\d?\\.\\d\\d?\\d?\\.\\d\\d?\\d?\\.\\d\\d?\\d?"
    sprintf(strre,
        "^tcp +6 +\\d+ +ESTABLISHED +"
        "src="RE_IP_DEF" +"
        "dst=("RE_IP_DEF") +"
        "sport=\\d+ +dport=\\d+ +packets=\\d+ bytes=\\d+ +"
        "src=%s +"
        "dst=%s +"
        "sport=%d +dport=%d +packets=\\d+ bytes=\\d+ +"
        "\\[ASSURED] +mark=\\d+ +secmark=\\d+ use=\\d+$",
        source, dest, sport, dport
    );
#undef RE_IP_DEF
    re::Regex regex(strre);

//     char tmp_transparent_dest[64] = {};
//     int len_tmp_transparent_dest = 0;

    for ( ; status == 1 ; (line.begin_line = line.eol), (status = line.readline())) {
//         if (verbose) {
            fprintf(stderr, "Line: %.*s", line.eol - line.begin_line, &line.buffer[line.begin_line]);
//         }

        line.buffer[line.end_buffer-1] = 0;
        re::Regex::range_matches matches = regex.exact_match(line.buffer);
        if ( ! matches.empty() ) {
            const size_t sz = std::min<size_t>(matches[0].second - matches[0].first, sz_transparent_dest - 1);
            memcpy(transparent_dest, matches[0].first, sz);
            transparent_dest[sz] = 0;
            return 0;
        }
    }
    // transparent ip route not found in ip_conntrack
    return -1;
}

#endif