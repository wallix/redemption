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
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(REDEMPTION_MOD_INTERNAL_WIDGET2_WIDGET2_MOD_HPP)
#define REDEMPTION_MOD_INTERNAL_WIDGET2_WIDGET2_MOD_HPP

// #include "widget.hpp"
#include "front.hpp"
#include "mod_api.hpp"
#include "internal/internal_mod.hpp"
#include "selector.hpp"

TODO("Old SelectorMod Test")

class widget2_mod : public internal_mod
{
    class Notifier : public NotifyApi
    {
    public:
        Notifier()
        {}

        virtual void notify(Widget2 * sender, notify_event_t event,
                            unsigned long param, unsigned long param2)
        {}
    } notifier;

    WidgetSelector selector;

public:
    widget2_mod(ModContext& context, Front& front, uint16_t width, uint16_t height)
    : internal_mod(front, width, height)
    , notifier()
    , selector(this, "bidule", width, height, &this->notifier,
               context.get(STRAUTHID_SELECTOR_CURRENT_PAGE),
               context.get(STRAUTHID_SELECTOR_NUMBER_OF_PAGES))
    {
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");
        this->selector.add_device("dsq", "dqfdfdfsfds", "fd", "fdsfsfd");

        this->front.begin_update();
        this->selector.refresh(this->selector.rect);
        this->front.end_update();
    }

    virtual ~widget2_mod()
    {}

    virtual void rdp_input_invalidate(const Rect& r)
    {
        this->front.begin_update();
        this->selector.rdp_input_invalidate(r);
        this->front.end_update();
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2* keymap)
    {
        this->selector.rdp_input_mouse(device_flags, x, y, keymap);
    }

    virtual void rdp_input_scancode(long int param1, long int param2, long int param3, long int param4, Keymap2* keymap)
    {
        this->selector.rdp_input_scancode(param1, param2, param3, param4, keymap);
    }

    virtual void rdp_input_synchronize(uint32_t time, uint16_t device_flags, int16_t param1, int16_t param2)
    {}

    void refresh()
    {
        this->front.begin_update();
        this->selector.refresh(this->selector.rect);
        this->front.end_update();
    }

    virtual BackEvent_t draw_event()
    {
        this->event.reset();
        return this->signal;
    }
};

#endif