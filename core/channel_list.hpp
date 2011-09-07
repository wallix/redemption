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

   Channels descriptors

*/


#if !defined(CORE_CHANNEL_LIST_HPP__)
#define CORE_CHANNEL_LIST_HPP__

struct McsChannelItem {
    char name[16];
    int flags;
    int chanid;
    McsChannelItem(){
        this->name[0] = 0;
        this->flags = 0;
        this->chanid = 0;
    }
};

class ChannelList
{
    size_t num;
    #warning replace this 100 with dynamically allocated number of channels someday
    McsChannelItem items[100];

public:

    const McsChannelItem & operator[](size_t index) const {
        return this->items[index];
    }

    void set_chanid(size_t index, int chanid){
        this->items[index].chanid = chanid;
    }

    size_t size() const {
        return this->num;
    }

    void push_back(const McsChannelItem & item){
        this->items[this->num] = item;
        this->num++;
        LOG(LOG_INFO, "push_back <-------------------------------- %p ----------- num = %u", this, this->num);
    }

    ChannelList() : num(0) {}
};


#endif
