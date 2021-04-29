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
#include "core/RDP/windows_execute_shell_params.hpp"
#include "mod/rdp/windowing_api.hpp"
#include "utils/bitmap.hpp"
#include "utils/rect.hpp"
#include "utils/ref.hpp"
#include "utils/monotonic_clock.hpp"

#include <string>

class FrontAPI;
class mod_api;
class Font;
class InStream;
class WindowListCaps;
class TimeBase;
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
        CRef<TimeBase> time_base,
        gdi::GraphicApi & drawable,
        FrontAPI & front,
        WindowListCaps const & window_list_caps,
        bool verbose);

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

    void ready(mod_api & mod, Font const & font, bool allow_resize_hosted_desktop);

    bool is_ready() const noexcept;

    void reset(bool soft);

    void send_to_mod_rail_channel(size_t length, InStream & chunk, uint32_t flags);

    void input_invalidate(const Rect r);

    const WindowsExecuteShellParams & get_windows_execute_shell_params();

    void create_auxiliary_window(Rect const window_rect) override;

    void destroy_auxiliary_window() override;

    void set_target_info(chars_view ti);

    [[nodiscard]] bool is_rail_enabled() const;

    [[nodiscard]] bool is_resizing_hosted_desktop_enabled() const;

    void adjust_window_to_mod();

    /// \return true when \c xPos and \c yPos are captured
    bool input_mouse(uint16_t pointerFlags, uint16_t xPos, uint16_t yPos);

public:
    //                 title_bar
    //                       |
    // +---------------------|---------------------------------------------------+
    // | +----+              |                       +----+ +----+ +----+ +----+ |
    // | |    |              |                       |    | |    | |    | |    | |
    // | |  | |                                      |  | | |  | | |  | | |  | | |
    // | +--|-+                                      +--|-+ +--|-+ +--|-+ +--|-+ |
    // +----|-------------------------------------------|------|------|------|---+
    //      |                                           |      |      |      |
    //    icon                       resize_hosted_desktop_box |      |      |
    //                                                   minimize_box |      |
    //                                                          maximize_box |
    //                                                                   close_box
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
    enum class WindowArea : uint8_t
    {
        N, NWN, NWW,
        W, SWW, SWS,
        S, SES, SEE,
        E, NEE, NEN,

        Icon, Title, Close,
        Maxi, Mini, Resize,

        NUMBER_OF_AREAS_OR_INVALID
    };

    struct Zone
    {
        static const uint16_t corner = TITLE_BAR_HEIGHT;
        static const uint16_t thickness = BORDER_WIDTH_HEIGHT;
        static const uint16_t button_width = TITLE_BAR_BUTTON_WIDTH;

        static Rect get_zone(WindowArea area, Rect w);
    };

    struct MouseAction
    {
        enum class EventAction : uint8_t
        {
            Nothing,

            // left click pressed
            CapturedClick,

            ResizeHostedDesktop,
            Minimize,
            // double click on title bar or maximize/restore button
            Maximaze,
            // top or bottom border
            MaximazeVertical,
            // double click on icon or close button
            Close,

            StartMoveResize,
            StopMoveResize,
            MoveResize,

            // left click down on button
            ActiveButton,
            // left click release
            UnactiveButton,
        };

        enum class IsMaximized : bool;
        enum class ResizableHosted : bool;

        struct ActionResult
        {
            Rect rect;
            WindowArea area;
            EventAction action;
        };

        ActionResult next_mouse_action(
            Rect const& window_rect,
            IsMaximized is_maximized, ResizableHosted resizable_hosted,
            MonotonicTimePoint now,
            uint16_t flags, uint16_t x, uint16_t y);

        uint16_t pressed_button_down_x() const noexcept { return safe_button_down_x; }
        uint16_t pressed_button_down_y() const noexcept { return safe_button_down_y; }
        WindowArea pressed_mouse_button() const noexcept { return pressed_area; }

    private:
        EventAction _next_action(
            WindowArea area, MonotonicTimePoint now,
            uint16_t flags, uint16_t x, uint16_t y);

        enum class State : uint8_t;

        MonotonicTimePoint button_down_timepoint {};
        WindowArea pressed_area = WindowArea::NUMBER_OF_AREAS_OR_INVALID;
        uint16_t safe_button_down_x {};
        uint16_t safe_button_down_y {};
        State state {};
    };

    static Rect move_resize_rect(
        WindowArea pressed_button,
        int original_offset_x, int original_offset_y, Rect const& r) noexcept;

private:
    enum class MaximizedState : uint8_t
    {
        No,
        VerticalScreen,
        FullScreen,
    };

          FrontAPI             & front_;
          gdi::GraphicApi      & drawable_;
          mod_api              * mod_     = nullptr;
    const CHANNELS::ChannelDef * channel_ = nullptr;
    const Font                 * font_    = nullptr;

    uint16_t client_order_type = 0;

    WindowsExecuteShellParams windows_execute_shell_params;

    bool should_ignore_first_client_execute_ = true;

    Point window_offset {};

    Rect virtual_screen_rect;

    Rect window_rect;
    // rect before start move
    Rect window_rect_saved;
    // rect before resize
    Rect window_rect_normal;

    MouseAction mouse_action;

    bool move_size_initialized = false;
    bool verbose = false;

    bool full_window_drag_enabled = false;
    bool internal_module_window_created = false;
    MaximizedState maximized_state = MaximizedState::No;
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

    TimeBase const& time;
    WindowArea previous_area = WindowArea::NUMBER_OF_AREAS_OR_INVALID;
    Rect previous_rect;
    Rect protocol_window_rect;

private:
    void initialize_move_size(uint16_t xPos, uint16_t yPos);

    void update_rects();

    void update_widget();

    void maximize_restore_window();
    void maximize_vertical_restore_window();

    void process_client_system_command_pdu(InStream& chunk);
    void process_client_get_application_id_pdu(InStream& chunk);
    void process_client_system_parameters_update_pdu(InStream& chunk);
    void process_client_activate_pdu(InStream& chunk);
    void process_client_window_move_pdu(InStream& chunk);

    void on_new_or_existing_window(Rect const & window_rect);
    void on_delete_window();
};  // class ClientExecute

