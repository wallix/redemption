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
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "core/session_reactor.hpp"
#include "core/RDP/rdp_pointer.hpp"
#include "mod/rdp/channels/rail_window_id_manager.hpp"
#include "mod/rdp/windowing_api.hpp"
#include "utils/bitmap.hpp"
#include "utils/rect.hpp"
#include "core/RDP/windows_execute_shell_params.hpp"
#include "core/events.hpp"


#include <string>


class FrontAPI;
class mod_api;
class Font;
class InStream;
class WindowListCaps;
namespace CHANNELS { class ChannelDef; }
namespace gdi { class GraphicApi; }

#include "RAIL/mouse_context.hpp"

class ClientExecute : public windowing_api
{
          FrontAPI             & front_;
          gdi::GraphicApi      & drawable_;
          mod_api              * mod_     = nullptr;
    const CHANNELS::ChannelDef * channel_ = nullptr;
    const Font                 * font_    = nullptr;

    uint16_t client_order_type = 0;

    WindowsExecuteShellParams windows_execute_shell_params;

public:
    bool should_ignore_first_client_execute_ = true;

private:
    bool server_execute_result_sent = false;

    void initialize_move_size(uint16_t xPos, uint16_t yPos, int pressed_mouse_button_);

    MouseContext mouse_context;

public:
    // Return true if event is consumed.
    bool input_mouse(uint16_t pointerFlags, uint16_t xPos, uint16_t yPos, bool& mouse_captured_ref);

private:

    uint16_t front_width  = 0;
    uint16_t front_height = 0;

    bool full_window_drag_enabled = false;

    bool internal_module_window_created = false;

    bool maximized = false;

    Bitmap wallix_icon_min;

    uint32_t auxiliary_window_id = RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;

    Rect auxiliary_window_rect;

    const static unsigned int max_work_area   = 32;
                 unsigned int work_area_count = 0;

    Rect work_areas[max_work_area];

    std::string window_title;

    bool const window_level_supported_ex;

    bool allow_resize_hosted_desktop_    = false;
    bool enable_resizing_hosted_desktop_ = false;

    int current_mouse_pointer_type = Pointer::POINTER_NULL;

    bool rail_enabled = false;

    bool verbose;

    TimeBase& time_base;
    EventContainer& events;

    void set_mouse_pointer(uint16_t xPos, uint16_t yPos, bool& mouse_captured_ref);

public:
    ClientExecute(
        TimeBase& time_base, EventContainer& events,
        gdi::GraphicApi & drawable, FrontAPI & front,
        WindowListCaps const & window_list_caps, bool verbose);

    ~ClientExecute();

    void set_verbose(bool verbose);

    void enable_remote_program(bool enable);

    Rect adjust_rect(Rect rect);

    [[nodiscard]] Rect get_current_work_area_rect() const;

    [[nodiscard]] Rect get_window_rect() const
    {
        return this->mouse_context.get_window_rect();
    }

    [[nodiscard]] Point get_window_offset() const
    {
        return this->mouse_context.get_window_offset();
    }

    [[nodiscard]] Rect get_auxiliary_window_rect() const;

    void ready(mod_api & mod, uint16_t front_width, uint16_t front_height, Font const & font, bool allow_resize_hosted_desktop);

    bool is_ready() const noexcept;

    void reset(bool soft);

    void send_to_mod_rail_channel(size_t length, InStream & chunk, uint32_t flags);

    void input_invalidate(const Rect r);

    const WindowsExecuteShellParams & get_windows_execute_shell_params();

    [[nodiscard]] uint16_t Flags() const;

    [[nodiscard]] const char * ExeOrFile() const;

    [[nodiscard]] const char * WorkingDir() const;

    [[nodiscard]] const char * Arguments() const;

    void create_auxiliary_window(Rect const window_rect) override;

    void destroy_auxiliary_window() override;

    void set_target_info(chars_view ti);

    [[nodiscard]] bool is_rail_enabled() const;

    [[nodiscard]] bool is_resizing_hosted_desktop_enabled() const;

private:
    void update_widget();

    void draw_resize_hosted_desktop_box(bool mouse_over, const Rect r);

    void draw_maximize_box(bool mouse_over, const Rect r);

    void check_is_unit_throw(uint32_t total_length, uint32_t flags, InStream& chunk, const char * message);

public:
    void adjust_window_to_mod();

private:
    void maximize_restore_window();

    void process_client_system_command_pdu(InStream& chunk);

    void process_client_get_application_id_pdu(InStream& chunk);

    void process_client_system_parameters_update_pdu(InStream& chunk);

    void process_client_activate_pdu(InStream& chunk);

    void process_client_window_move_pdu(InStream& chunk);

    Rect protocol_window_rect;

    void on_new_or_existing_window(Rect const & window_rect);

    void on_delete_window();
};  // class ClientExecute

