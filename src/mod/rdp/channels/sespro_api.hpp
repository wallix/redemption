/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

struct sespro_api
{
    virtual void rail_new_or_existing_window(uint32_t window_id) = 0;
    virtual void rail_deleted_window(uint32_t window_id) = 0;

    virtual void rail_new_or_existing_notification_icon(uint32_t window_id, uint32_t notification_icon_id) = 0;
    virtual void rail_deleted_notification_icon(uint32_t window_id, uint32_t notification_icon_id) = 0;

    virtual ~sespro_api() = default;
};
