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

#include "core/RDP/rdp_pointer.hpp"
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
namespace CHANNELS
{
    class ChannelDef;
}
namespace gdi
{
    class GraphicApi;
}

class ClientExecute : public windowing_api
{
public:
    enum {
        BORDER_WIDTH_HEIGHT = 3,
        TITLE_BAR_HEIGHT = 24,
        TITLE_BAR_BUTTON_WIDTH = 37,
    };

    ClientExecute(
        EventContainer& events,
        gdi::GraphicApi & drawable,
        FrontAPI & front,
        WindowListCaps const & window_list_caps, bool verbose);

    ~ClientExecute();

    bool should_ignore_first_client_execute() const noexcept
    {
        return this->should_ignore_first_client_execute_;
    }

    void enable_remote_program(bool enable);

    Rect adjust_rect(Rect rect);

    [[nodiscard]] Rect get_window_rect() const;

    [[nodiscard]] Point get_window_offset() const;

    [[nodiscard]] Rect get_current_work_area_rect() const;

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

    void adjust_window_to_mod();

    /// \return true when \c xPos and \c yPos are captured
    bool input_mouse(uint16_t pointerFlags, uint16_t xPos, uint16_t yPos);

public:
    // TODO private
    enum class MouseButtonPressed
    {
        None,

        North,
        NorthWest,
        West,
        SouthWest,
        South,
        SouthEast,
        East,
        NorthEast,

        TitleBar,
        ResizeHostedDesktopBox,
        MinimizeBox,
        MaximizeBox,
        CloseBox,
    };

    struct Zone
    {
        //                 title_bar_rect
        //                       |
        // +---------------------|---------------------------------------------------+
        // | +----+              |                       +----+ +----+ +----+ +----+ |
        // | |    |              |                       |    | |    | |    | |    | |
        // | |  | |                                      |  | | |  | | |  | | |  | | |
        // | +--|-+                                      +--|-+ +--|-+ +--|-+ +--|-+ |
        // +----|-------------------------------------------|------|------|------|---+
        //      |                                           |      |      |      |
        //  title_bar_icon_rect    resize_hosted desktop_box rect  |      |      |
        //                                            minimize_box rect   |      |
        //                                                   maximize_box_rect   |
        //                                                            close_box_rect


        //                          corner
        //                        |-------|
        //                  cx
        //    |---------------------------|
        //        1         0        11
        //    +--NWN--\-----N-----|--NEN--+      +   +
        //    |                           |      |   |
        // 2 NWW                         NEE 10  |   | corner
        //    |                           |      |   |
        //    +         N = North         +      |   +
        //    |         S = South         |      |
        //    |         E = East          |      |
        // 3  W         W = West          E 9    | cy
        //    |                           |      |
        //    |                           |      |
        //    +                           +      |
        //    |                           |      |
        // 4 SWW                         SEE 8   |
        //    |                           |      |
        //    +--SWS--\-----S-----|--SES--+      °
        //        5         6         7

        static const uint16_t corner = TITLE_BAR_HEIGHT;
        static const uint16_t thickness = BORDER_WIDTH_HEIGHT;
        static const uint16_t button_width = TITLE_BAR_BUTTON_WIDTH;

        enum {
            ZONE_N, ZONE_NWN, ZONE_NWW,
            ZONE_W, ZONE_SWW, ZONE_SWS,
            ZONE_S, ZONE_SES, ZONE_SEE,
            ZONE_E, ZONE_NEE, ZONE_NEN,

            ZONE_ICON, ZONE_TITLE, ZONE_CLOSE,
            ZONE_MAXI, ZONE_MINI, ZONE_RESIZE,

            NUMBER_OF_ZONES
        };

        static Rect get_zone(size_t zone, Rect w);

        static MouseButtonPressed get_button(int zone);

        static PredefinedPointer get_pointer(int zone);
    };

private:
          FrontAPI             & front_;
          gdi::GraphicApi      & drawable_;
          mod_api              * mod_     = nullptr;
    const CHANNELS::ChannelDef * channel_ = nullptr;
    const Font                 * font_    = nullptr;

    uint16_t client_order_type = 0;

    WindowsExecuteShellParams windows_execute_shell_params;

    bool should_ignore_first_client_execute_ = true;
    bool server_execute_result_sent = false;

    Point window_offset {};

    Rect virtual_screen_rect;

    Rect task_bar_rect;

    Rect window_rect;
    Rect window_rect_saved;
    Rect window_rect_normal;

    int button_1_down_timer;

    int button_1_down_x = 0;
    int button_1_down_y = 0;

    MouseButtonPressed button_1_down = MouseButtonPressed::None;

    uint16_t captured_mouse_x = 0;
    uint16_t captured_mouse_y = 0;

    MouseButtonPressed pressed_mouse_button = MouseButtonPressed::None;
    bool move_size_initialized = false;
    bool verbose = false;

    uint16_t front_width  = 0;
    uint16_t front_height = 0;

    bool full_window_drag_enabled = false;
    bool internal_module_window_created = false;
    bool maximized = false;
    Bitmap wallix_icon_min;
    uint32_t auxiliary_window_id;
    Rect auxiliary_window_rect;
    const static unsigned int max_work_area   = 32;
                 unsigned int work_area_count = 0;
    Rect work_areas[max_work_area];
    std::string window_title;
    PredefinedPointer current_mouse_pointer;
    bool const window_level_supported_ex;
    bool allow_resize_hosted_desktop_    = false;
    bool enable_resizing_hosted_desktop_ = false;
    bool rail_enabled = false;

    EventsGuard events_guard;
    Rect protocol_window_rect;

private:
    void initialize_move_size(uint16_t xPos, uint16_t yPos, MouseButtonPressed pressed_mouse_button_);
    void set_mouse_pointer(uint16_t xPos, uint16_t yPos, bool& mouse_captured_ref);

    void update_rects();

    void update_widget();

    void draw_resize_hosted_desktop_box(bool mouse_over, const Rect r);

    void draw_maximize_box(bool mouse_over, const Rect r);

    void check_is_unit_throw(uint32_t total_length, uint32_t flags, InStream& chunk, const char * message);

    void maximize_restore_window();

    void process_client_system_command_pdu(InStream& chunk);
    void process_client_get_application_id_pdu(InStream& chunk);
    void process_client_system_parameters_update_pdu(InStream& chunk);
    void process_client_activate_pdu(InStream& chunk);
    void process_client_window_move_pdu(InStream& chunk);

    void on_new_or_existing_window(Rect const & window_rect);
    void on_delete_window();
};  // class ClientExecute
