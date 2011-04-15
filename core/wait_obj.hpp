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

    wait_obj(const char* name) : obj(0)
    {
        #warning sun_path is limited in size (107 char), check for buffer space to throw som exception if necessary, and use snprintf instead of sprintf
        struct sockaddr_un sa;
        int len;
        int sck;
        int i;

        sck = socket(PF_UNIX, SOCK_DGRAM, 0);
        if (sck < 0) {
            throw Error(ERR_WAIT_OBJ_SOCKET);
        }
        memset(&sa, 0, sizeof(sa));
        sa.sun_family = AF_UNIX;
        if ((name == 0) || (strlen(name) == 0)) {
            //g_random((char*)&i, sizeof(i));
            int fd;

            memset((char*)&i, 0x44,  sizeof(i));
            fd = open("/dev/urandom", O_RDONLY);
            if (fd == -1) {
                fd = open("/dev/random", O_RDONLY);
            }
            if (fd != -1) {
                if (read(fd, (char*)&i,  sizeof(i)) !=  sizeof(i)) {
                }
                close(fd);
            }

            sprintf(sa.sun_path, "/tmp/auto%8.8x", i);
            while ((0 == access(sa.sun_path, F_OK))) {
                //g_random((char*)&i, sizeof(i));
                int fd;

                memset((char*)&i, 0x44,  sizeof(i));
                fd = open("/dev/urandom", O_RDONLY);
                if (fd == -1) {
                    fd = open("/dev/random", O_RDONLY);
                }
                if (fd != -1) {
                    if (read(fd, (char*)&i,  sizeof(i)) !=  sizeof(i)) {
                    }
                    close(fd);
                }
                sprintf(sa.sun_path, "/tmp/auto%8.8x", i);
            }
        } else {
            sprintf(sa.sun_path, "/tmp/%s", name);
        }
        unlink(sa.sun_path);
        len = sizeof(sa);
        if (bind(sck, (struct sockaddr*)&sa, len) < 0) {
            close(sck);
            throw Error(ERR_WAIT_OBJ_SOCKET);
        }
        this->obj = sck;
    }

    wait_obj(int sck)
    {
        this->obj = sck;
    }

    ~wait_obj()
    {
        struct sockaddr_un sa;
        socklen_t sa_size = sizeof(sa);

        if (getsockname(this->obj, (struct sockaddr*)&sa, &sa_size) < 0) {
            /* socket is in error state : can't close */
            return;
        }
        close(this->obj);
        unlink(sa.sun_path);
    }

    void add_to_fd_set(fd_set & rfds, unsigned & max)
    {
        FD_SET(this->obj, &rfds);
        max = ((unsigned)this->obj > max)?this->obj:max;
    }

    void reset()
    {
        char buf[64];
        while (this->can_recv()) {
            recvfrom(this->obj, &buf, 64, 0, 0, 0);
        }
    }

    bool is_set()
    {
        return this->can_recv();
    }

    int set()
    {
        socklen_t sa_size;
        int s;
        struct sockaddr_un sa;

        if (this->can_recv()) {
            /* already signalled */
            return false;
        }
        sa_size = sizeof(sa);
        if (getsockname(this->obj, (struct sockaddr*)&sa, &sa_size) < 0) {
            return true;
        }
        s = socket(PF_UNIX, SOCK_DGRAM, 0);
        if (s < 0) {
            return true;
        }
        sendto(s, "sig", 4, 0, (struct sockaddr*)&sa, sa_size);
        close(s);
        return false;
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
