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
    Copyright (C) Wallix 2016
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "gdi/graphic_api.hpp"
#include "mod/internal/widget2/widget.hpp"
#include "mod/mod_api.hpp"

class WidgetModuleHost : public Widget2,
    public gdi::GraphicBase<WidgetModuleHost>
{
private:
    class ModuleHolder : public mod_api
    {
    private:
        WidgetModuleHost& host;

        std::unique_ptr<mod_api> managed_mod;

    public:
        ModuleHolder(WidgetModuleHost& host,
                     std::unique_ptr<mod_api> managed_mod)
        : host(host)
        , managed_mod(std::move(managed_mod))
        {}

        // mod_api

        void draw_event(time_t now, gdi::GraphicApi& drawable) override
        {
            if (this->managed_mod)
            {
                this->host.drawable_ptr = &drawable;

                this->managed_mod->draw_event(now, this->host);

                this->host.drawable_ptr = nullptr;
            }
        }

        wait_obj& get_event() override
        {
            if (this->managed_mod)
            {
                return this->managed_mod->get_event();
            }

            REDASSERT(false);

            return mod_api::get_event();
        }

        bool is_up_and_running() override
        {
            if (this->managed_mod)
            {
                return this->managed_mod->is_up_and_running();
            }

            REDASSERT(false);

            return mod_api::is_up_and_running();
        }

        void send_to_front_channel(const char* const mod_channel_name,
                                   const uint8_t* data, size_t length,
                                   size_t chunk_size, int flags) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->send_to_front_channel(mod_channel_name,
                    data, length, chunk_size, flags);
            }
        }

        // RdpInput

        void rdp_input_invalidate(const Rect& r) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->rdp_input_invalidate(r);
            }
        }

        void rdp_input_mouse(int device_flags, int x, int y,
                             Keymap2* keymap) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->rdp_input_mouse(device_flags, x, y,
                    keymap);
            }
        }

        void rdp_input_scancode(long param1, long param2, long param3,
                                long param4, Keymap2* keymap) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->rdp_input_scancode(param1, param2, param3,
                    param4, keymap);
            }
        }

        void rdp_input_synchronize(uint32_t time, uint16_t device_flags,
                                   int16_t param1, int16_t param2) override
        {
            if (this->managed_mod)
            {
                this->managed_mod->rdp_input_synchronize(time, device_flags,
                    param1, param2);
            }
        }
    } module_holder;

    gdi::GraphicApi* drawable_ptr = nullptr;

    gdi::GraphicApi& drawable_ref;

public:
    WidgetModuleHost(gdi::GraphicApi& drawable, Widget2& parent,
                     NotifyApi* notifier,
                     std::unique_ptr<mod_api> managed_mod, int group_id = 0)
    : Widget2(drawable, parent, notifier, group_id)
    , module_holder(*this, std::move(managed_mod))
    , drawable_ref(drawable)
    {
        this->tab_flag   = NORMAL_TAB;
        this->focus_flag = NORMAL_FOCUS;
    }

    mod_api& get_managed_mod()
    {
        return this->module_holder;
    }

private:
    gdi::GraphicApi& get_drawable()
    {
        if (this->drawable_ptr)
        {
            return *this->drawable_ptr;
        }

        return this->drawable_ref;
    }

public:
    using gdi::GraphicBase<WidgetModuleHost>::draw;

public:
    // RdpInput

    void rdp_input_invalidate(const Rect & r) override
    {
        this->module_holder.rdp_input_invalidate(r);
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override
    {
        this->module_holder.rdp_input_mouse(device_flags, x, y, keymap);
    }

    void rdp_input_scancode(long param1, long param2, long param3, long param4, Keymap2 * keymap) override
    {
        this->module_holder.rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2) override
    {
        this->module_holder.rdp_input_synchronize(time, device_flags, param1, param2);
    }

    // Widget2

    void draw(const Rect&/* clip*/) override {}

private:
    // gdi::GraphicBase

    friend gdi::GraphicCoreAccess;

    void begin_update() override
    {
        gdi::GraphicApi& drawable = this->get_drawable();

        drawable.begin_update();
    }

    void end_update() override
    {
        gdi::GraphicApi& drawable = this->get_drawable();

        drawable.end_update();
    }

    template<class Cmd>
    void draw_impl(const Cmd& cmd) {
        gdi::GraphicApi& drawable = this->get_drawable();

        drawable.draw(cmd);
    }

    template<class Cmd, class... Args>
    void draw_impl(const Cmd& cmd, const Rect& clip, const Args&... args)
    {
        gdi::GraphicApi& drawable = this->get_drawable();

        drawable.draw(cmd, clip, args...);
    }

    void draw_impl(const RDPBitmapData& bitmap_data, const Bitmap& bmp)
    {
        gdi::GraphicApi& drawable = this->get_drawable();

        drawable.draw(bitmap_data, bmp);
    }
};
