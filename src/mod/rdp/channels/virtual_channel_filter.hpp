/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  Product name: redemption, a FLOSS RDP proxy
  Copyright (C) Wallix 2010
  Author(s): Christophe Grosjean, Javier Caverni, Dominique Lafages,
             Raphael Zhou, Meng Tan, Clément Moroldo
  Based on xrdp Copyright (C) Jay Sorg 2004-2010

  rdp module main header file
*/

#pragma once

#include "core/RDP/clipboard.hpp"

template <typename T>
class VirtualChannelFilter : public T
{
public:
    VirtualChannelFilter() = default;

    VirtualChannelFilter(VirtualChannelFilter& previous_filter,
        VirtualChannelFilter& next_filter)
    {
        this->previous_filter_ptr = &previous_filter;
        this->next_filter_ptr = &next_filter;
    }

    virtual ~VirtualChannelFilter() = default;

    virtual void insert_after(VirtualChannelFilter& filter_ref)
    {
        if (this->next_filter_ptr)
        {
            this->next_filter_ptr->previous_filter_ptr = &filter_ref;
        }

        filter_ref.previous_filter_ptr = this;
        filter_ref.next_filter_ptr = this->next_filter_ptr;

        this->next_filter_ptr = &filter_ref;
    }

    virtual void insert_before(VirtualChannelFilter& filter_ref)
    {
        if (this->previous_filter_ptr)
        {
            this->previous_filter_ptr->next_filter_ptr = &filter_ref;
        }

        filter_ref.previous_filter_ptr = this->previous_filter_ptr;
        filter_ref.next_filter_ptr     = this;

        this->previous_filter_ptr = &filter_ref;
    }

    VirtualChannelFilter* get_previous_filter_ptr()
    {
        return this->previous_filter_ptr;
    }

    void set_previous_filter_ptr(VirtualChannelFilter& filter_ref)
    {
        this->previous_filter_ptr = &filter_ref;
    }

    VirtualChannelFilter* get_next_filter_ptr()
    {
        return this->next_filter_ptr;
    }

    void set_next_filter_ptr(VirtualChannelFilter& filter_ref)
    {
        this->next_filter_ptr = &filter_ref;
    }

private:
    VirtualChannelFilter* previous_filter_ptr = nullptr;

    VirtualChannelFilter* next_filter_ptr = nullptr;
};

template <typename T>
class RemovableVirtualChannelFilter : public VirtualChannelFilter<T>
{
public:
    void remove_self()
    {
        if (!this->get_previous_filter_ptr())
        {
            assert(!this->get_next_filter_ptr());

            return;
        }

        assert(this->get_next_filter_ptr());

        VirtualChannelFilter<T>* previous_filter_ptr = this->get_previous_filter_ptr();
        VirtualChannelFilter<T>* next_filter_ptr = this->get_next_filter_ptr();

        previous_filter_ptr->set_next_filter_ptr(*next_filter_ptr);
        next_filter_ptr->set_previous_filter_ptr(*previous_filter_ptr);
    }
};

class CliprdrVirtualChannelProcessor
{
public:
    virtual ~CliprdrVirtualChannelProcessor() = default;

    virtual void process_client_message(uint32_t total_length, uint32_t flags,
        bytes_view chunk_data, RDPECLIP::CliprdrHeader const* decoded_header) = 0;

    virtual void process_server_message(uint32_t total_length, uint32_t flags,
        bytes_view chunk_data, RDPECLIP::CliprdrHeader const* decoded_header) = 0;
};
