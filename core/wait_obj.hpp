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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Synchronisation objects

*/

#if !defined(__WAIT_OBJS_HPP__)
#define __WAIT_OBJS_HPP__

#include "error.hpp"

class wait_obj
{
    public:
    int obj;
    bool set_state;
    wait_obj(int sck) : obj(sck), set_state(false) {}

    ~wait_obj()
    {
        if (this->obj > 0){
            struct sockaddr_un sa;
            socklen_t sa_size = sizeof(sa);

            if (getsockname(this->obj, (struct sockaddr*)&sa, &sa_size) < 0) {
                /* socket is in error state : can't close */
                return;
            }
            close(this->obj);
        }
    }

    void add_to_fd_set(fd_set & rfds, unsigned & max)
    {
        if (this->obj > 0){
            FD_SET(this->obj, &rfds);
            max = ((unsigned)this->obj > max)?this->obj:max;
        }
    }

    void reset()
    {
        set_state = false;
        if (this->obj > 0){
            char buf[64];
            while (this->can_recv()) {
                #warning if I actually read data ? May I not lose data doint that ?
                recvfrom(this->obj, &buf, 64, 0, 0, 0);
            }
        }
    }

    bool is_set()
    {
        if (this->obj > 0){
            return this->can_recv();
        }
        else {
            return set_state;
        }
    }

    void set()
    {
        set_state = true;
        if (this->obj) {
            socklen_t sa_size;
            int s;
            struct sockaddr_un sa;

            if (this->can_recv()) {
                /* already signalled */
                throw Error(ERR_SOCKET_ERROR);
            }
            sa_size = sizeof(sa);
            if (getsockname(this->obj, (struct sockaddr*)&sa, &sa_size) < 0) {
                return;
            }
            s = socket(PF_UNIX, SOCK_DGRAM, 0);
            if (s < 0) {
                throw Error(ERR_SOCKET_ERROR);
            }
            sendto(s, "sig", 4, 0, (struct sockaddr*)&sa, sa_size);
            close(s);
            return;
        }
    }

    private:
    bool can_recv()
    {
        fd_set rfds;
        struct timeval time;
        int rv = false;

        time.tv_sec = 0;
        time.tv_usec = 0;
        FD_ZERO(&rfds);
        if (this->obj > 0) {
            FD_SET((this->obj), &rfds);
            rv = select(this->obj + 1, &rfds, 0, 0, &time); /* don't wait */
            if (rv > 0) {
                int opt;
                unsigned int opt_len = sizeof(opt);

                if (getsockopt(this->obj, SOL_SOCKET, SO_ERROR, (char*)(&opt), &opt_len) == 0) {
                    rv = (opt == 0);
                }
            }
        }
        return rv;
    }

};

#endif
