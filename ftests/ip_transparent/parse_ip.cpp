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

#include "parse_ip.hpp"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>

int main()
{
    const char source_ip[] = "10.10.43.13";
    const char target_ip[] = "10.10.47.23";
    const int source_port = 36699;
    const int target_port = 22;
    char real_target_ip[] = "   .   .   .   ";
    const int fd = open("ip_conntrack", O_RDONLY);
    const int res = parse_ip_conntrack(fd, target_ip, source_ip, target_port, source_port, real_target_ip, sizeof(real_target_ip));
    if (res){
        std::cerr << "Failed to get transparent proxy target from ip_conntrack\n";
    }
    else {
        std::cout << "ip: " << real_target_ip << "\n";
    }
    close(fd);
}
