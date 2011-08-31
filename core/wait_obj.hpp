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
#include "urt.hpp"

class wait_obj
{
    public:
    int obj;
    bool set_state;
    URT trigger_time;
    wait_obj(int sck) : obj(sck), set_state(false), trigger_time(URT()) {}

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
        this->set_state = false;
    }

    bool is_set()
    {
        if (this->obj > 0){
            return this->can_recv();
        }
        else{
            if (this->set_state){
                URT now;
                if(now > this->trigger_time){
                    return true;
                }
            }
        }
        return false;
    }

    // Idle time in millisecond
    void set(uint64_t idle_usec = 0)
    {
        this->set_state = true;
        URT idle_time(idle_usec);
        URT now;
        this->trigger_time = now + idle_time;
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
