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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan, Raphael Zhou
*/


#pragma once

#include <ctime>
#include <vector>

#include "core/callback.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/wait_obj.hpp"
#include "gdi/graphic_api.hpp"
#include "utils/sugar/not_null_ptr.hpp"

class EventHandler
{
public:
    class CB
    {
    public:
        virtual ~CB() = default;

        virtual void operator()(time_t now, wait_obj& event, gdi::GraphicApi& drawable) = 0;
    };

public:
    EventHandler(not_null_ptr<wait_obj> event, not_null_ptr<CB> cb, int fd = INVALID_SOCKET)
    : event_(event)
    , cb_(cb.get())
    , fd_(fd)
    {}

    void operator()(time_t now, gdi::GraphicApi& drawable)
    {
        (*this->cb_)(now, *this->event_, drawable);
    }

    wait_obj & get_event() const
    {
        return *this->event_;
    }

    int get_fd() const
    {
        return this->fd_;
    }

private:
    wait_obj* event_;

    CB* cb_;

    int fd_;
};

class mod_api : public Callback
{
protected:
    wait_obj event;

public:
    enum class AsynchronousGraphicTask {
        none,
        clear_screen,
        freeze_screen
    };

private:
    wait_obj asynchronous_graphic_task_event;

    class AsynchronousGraphicTaskEventHandler : public EventHandler::CB {
        mod_api& mod_;

    public:
        AsynchronousGraphicTaskEventHandler(mod_api& mod)
        : mod_(mod)
        {}

        void operator()(time_t now, wait_obj& event, gdi::GraphicApi& drawable) override {
            this->mod_.process_asynchronous_graphic_task_event(now, event, drawable);
        }
    } asynchronous_graphic_task_event_handler;

    void process_asynchronous_graphic_task_event(time_t, wait_obj&, gdi::GraphicApi&) {
        this->asynchronous_graphic_task_event.full_reset();

        this->asynchronous_graphic_task_event.signal = BACK_EVENT_CALL_DRAW_EVENT;
    }

protected:
    AsynchronousGraphicTask asynchronous_graphic_task = AsynchronousGraphicTask::none;

    void perform_asynchronous_graphic_task(gdi::GraphicApi& drawable) {
        switch (this->asynchronous_graphic_task)
        {
            case AsynchronousGraphicTask::clear_screen:
            {
                Dimension dim = this->get_dim();
                Rect r(0, 0, dim.w, dim.h);
                RDPOpaqueRect cmd(r, color_encode(BGRColor(BLACK), 24));
                drawable.begin_update();
                drawable.draw(cmd, r, gdi::ColorCtx::depth24());
                drawable.end_update();
            }
            break;

            case AsynchronousGraphicTask::freeze_screen:
            {
                Dimension dim = this->get_dim();
                Rect r(0, 0, dim.w, dim.h);
                RDPPatBlt cmd(r, 0xA0, color_encode(BGRColor(BLACK), 24), color_encode(BGRColor(WHITE), 24),
                        RDPBrush(0, 0, 3, 0xaa, reinterpret_cast<const uint8_t *>("\x55\xaa\x55\xaa\x55\xaa\x55"))
                    );
                drawable.begin_update();
                drawable.draw(cmd, r, gdi::ColorCtx::depth24());
                drawable.end_update();
            }
            break;

            case AsynchronousGraphicTask::none:
            break;
        }

        this->asynchronous_graphic_task = AsynchronousGraphicTask::none;
    }

public:
    void invoke_asynchronous_graphic_task(AsynchronousGraphicTask task)
    {
        this->asynchronous_graphic_task = task;

        this->asynchronous_graphic_task_event.set_trigger_time(0);
    }

public:
    enum : bool {
        CLIENT_UNLOGGED,
        CLIENT_LOGGED
    };
    bool logged_on = CLIENT_UNLOGGED; // TODO suspicious

    mod_api() : asynchronous_graphic_task_event_handler(*this)
    {
        this->event.set_trigger_time(wait_obj::NOW);
    }

    ~mod_api() override {}

    virtual wait_obj& get_event() { return this->event; }

    virtual int get_fd() const { return INVALID_SOCKET; }

    virtual void get_event_handlers(std::vector<EventHandler>& out_event_handlers) {
        if (AsynchronousGraphicTask::none != this->asynchronous_graphic_task) {
            out_event_handlers.emplace_back(
                &this->asynchronous_graphic_task_event,
                &this->asynchronous_graphic_task_event_handler,
                INVALID_SOCKET
            );
        }
    }

    virtual void send_to_front_channel(CHANNELS::ChannelNameId mod_channel_name,
        uint8_t const * data, size_t length, size_t chunk_size, int flags) = 0;

    // draw_event is run when mod socket received some data (drawing order)
    // or auto-generated by modules, say to comply to some refresh.
    // draw event decodes incoming traffic from backend and eventually calls front to draw things
    // may raise an exception (say if connection to server is closed), but returns nothings
    virtual void draw_event(time_t now, gdi::GraphicApi & drawable) = 0;

    // used when context changed to avoid creating a new module
    // it usually perform some task identical to what constructor does
    // henceforth it should often be called by constructors
    virtual void refresh_context() {}

    virtual bool is_up_and_running() { return false; }

    // support auto-reconnection
    virtual bool is_auto_reconnectable() { return false; }

    virtual void disconnect(time_t now) { (void)now; }

    virtual void display_osd_message(std::string const &) {}

    virtual void move_size_widget(int16_t/* left*/, int16_t/* top*/, uint16_t/* width*/, uint16_t/* height*/) {}

    virtual bool disable_input_event_and_graphics_update(
            bool disable_input_event, bool disable_graphics_update) {
        (void)disable_input_event;
        (void)disable_graphics_update;
        return false;
    }

    virtual void send_input(int/* time*/, int/* message_type*/, int/* device_flags*/, int/* param1*/, int/* param2*/) {}

    virtual Dimension get_dim() const { return Dimension(); }
};
