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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   mcs layer at core module

*/

#if !defined(__MCS_HPP__)
#define __MCS_HPP__

/* used in mcs */
struct mcs_channel_item {
    char name[16];
    int flags;
    int chanid;
    mcs_channel_item(){
        this->name[0] = 0;
        this->flags = 0;
        this->chanid = 0;
    }
};

struct Mcs {

    int userid;
    vector<struct mcs_channel_item *> channel_list;

    Mcs() : userid(1)
    {
    }

    ~Mcs()
    {
        int count = (int) this->channel_list.size();
        for (int index = 0; index < count; index++) {
            mcs_channel_item* channel_item = this->channel_list[index];
            if (0 != channel_item) {
                delete channel_item;
            }
        }
    }



    /* returns a zero based index of the channel,
      -1 if error or if it dosen't exist */
    int mcs_get_channel_id(const char * name)
    {
//        LOG(LOG_INFO, "mcs_get_channel_id");

        int rv = -1;

        int count = (int) this->channel_list.size();

        for (int index = 0; index < count; index++) {
            mcs_channel_item* channel_item = this->channel_list[index];
            if (0 != channel_item) {
                if (0 == strcasecmp(name, channel_item->name)) {
                    rv = index;
                    break;
                }
            }
        }
        return rv;
    }

};

#endif
