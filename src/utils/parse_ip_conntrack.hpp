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
*   Copyright (C) Wallix 2010-2014
*   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
*/


#pragma once

#include "utils/log.hpp"
#include "regex/regex.hpp"

#include <unistd.h>
#include <cstring>
#include <cstdio>


struct LineBuffer
{
    char buffer[20480];
    int end_buffer;

    int fd;
    int begin_line;
    int eol;

    LineBuffer(int fd)
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
static inline int parse_ip_conntrack(int fd, const char * source, const char * dest, int sport, int dport, char * transparent_dest, size_t sz_transparent_dest, uint32_t verbose = 0)
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

