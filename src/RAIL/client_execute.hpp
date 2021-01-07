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
namespace CHANNELS { class ChannelDef; }
namespace gdi { class GraphicApi; }

enum {BORDER_WIDTH_HEIGHT = 3 };

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
    void set_mouse_pointer(uint16_t xPos, uint16_t yPos, bool& mouse_captured_ref);

    enum {
        MOUSE_BUTTON_PRESSED_NONE,

        MOUSE_BUTTON_PRESSED_NORTH,
        MOUSE_BUTTON_PRESSED_NORTHWEST,
        MOUSE_BUTTON_PRESSED_WEST,
        MOUSE_BUTTON_PRESSED_SOUTHWEST,
        MOUSE_BUTTON_PRESSED_SOUTH,
        MOUSE_BUTTON_PRESSED_SOUTHEAST,
        MOUSE_BUTTON_PRESSED_EAST,
        MOUSE_BUTTON_PRESSED_NORTHEAST,

        MOUSE_BUTTON_PRESSED_TITLEBAR,
        MOUSE_BUTTON_PRESSED_RESIZEHOSTEDDESKTOPBOX,
        MOUSE_BUTTON_PRESSED_MINIMIZEBOX,
        MOUSE_BUTTON_PRESSED_MAXIMIZEBOX,
        MOUSE_BUTTON_PRESSED_CLOSEBOX,
    };

    int window_offset_x = 0;
    int window_offset_y = 0;

    Rect virtual_screen_rect;

    Rect task_bar_rect;

    Rect window_rect;
    Rect window_rect_saved;
    Rect window_rect_normal;
    Rect window_rect_old;

//    Rect minimize_box_rect;
//    Rect maximize_box_rect;
    Rect resize_hosted_desktop_box_rect;

    int button_1_down_timer;

    int button_1_down_x = 0;
    int button_1_down_y = 0;

    int button_1_down = MOUSE_BUTTON_PRESSED_NONE;

    uint16_t captured_mouse_x = 0;
    uint16_t captured_mouse_y = 0;

    int pressed_mouse_button = MOUSE_BUTTON_PRESSED_NONE;
    bool move_size_initialized = false;
    bool verbose = false;

    void update_rects(const bool allow_resize_hosted_desktop);

public:
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

        static const uint16_t corner = 24; // TITLE_BAR_HEIGHT
        static const uint16_t thickness = 3; // BORDER_WIDTH_HEIGHT
        static const uint16_t button_width = 37; // TITLE_BAR_BUTTON_WIDTH

        enum {
            ZONE_N, ZONE_NWN, ZONE_NWW,
            ZONE_W, ZONE_SWW, ZONE_SWS,
            ZONE_S, ZONE_SES, ZONE_SEE,
            ZONE_E, ZONE_NEE, ZONE_NEN,

            ZONE_ICON, ZONE_TITLE, ZONE_CLOSE,
            ZONE_MAXI, ZONE_MINI, ZONE_RESIZE,

            NUMBER_OF_ZONES
        };

        static inline Rect get_zone(size_t zone, Rect w)
        {
            // if (allow_resize_hosted_desktop)
            if (zone >= ZONE_CLOSE && zone <= ZONE_RESIZE){
                return Rect(w.x + w.cx-1-button_width*(zone-ZONE_CLOSE+1), w.y + 1, button_width, corner-1);
            }
            if (zone == ZONE_TITLE){
                return Rect(w.x+22, w.y+1, w.cx-23-button_width*3, corner-1);
            }
            if (zone == ZONE_ICON){
                return Rect(w.x+1, w.y+1, 21, corner-1);
            }
            if (zone >= ZONE_N && zone <= ZONE_NEN){
                static constexpr uint8_t data[12][4] ={
                    { 1, 0, 0}, // North
                    { 0, 0, 0}, // North West North
                    { 0, 0, 1}, // North West West
                    { 0, 1, 1}, // West
                    { 0, 2, 1}, // South West West
                    { 0, 2, 0}, // South West South
                    { 1, 2, 0}, // South
                    { 2, 2, 0}, // South East South
                    { 2, 2, 1}, // South East East
                    { 2, 1, 1}, // East
                    { 2, 0, 1}, // North East East
                    { 2, 0, 0}, // North East North
                };

                // d[0] 0=left or 1=middle, 2=right
                // d[1] 0=top or 1=middle or 2=bottom
                // d[2] 0=horizontal 1=vectical

                auto & d = data[zone];

                return Rect(
                    w.x + ((d[0]==0)?0:(d[0]==1)?corner:(w.cx-((d[2]==0)?corner:thickness))),
                    w.y + ((d[1]==0)?0:(d[1]==1)?corner:(w.cy-((d[2]==1)?corner:thickness))),
                    (d[0]==1)?w.cx-2*corner:(d[2]==0)?corner:thickness,
                    (d[1]==1)?w.cy-2*corner:(d[2]==1)?corner:thickness
                );
            }
            return Rect(0,0,0,0);
        }

        static inline int get_button(int zone)
        {
            switch (zone){
            case ZONE_N  : return MOUSE_BUTTON_PRESSED_NORTH;
            case ZONE_NWN:
            case ZONE_NWW: return MOUSE_BUTTON_PRESSED_NORTHWEST;
            case ZONE_W  : return MOUSE_BUTTON_PRESSED_WEST;
            case ZONE_SWW:
            case ZONE_SWS: return MOUSE_BUTTON_PRESSED_SOUTHWEST;
            case ZONE_S  : return MOUSE_BUTTON_PRESSED_SOUTH;
            case ZONE_SES:
            case ZONE_SEE: return MOUSE_BUTTON_PRESSED_SOUTHEAST;
            case ZONE_E  : return MOUSE_BUTTON_PRESSED_EAST;
            case ZONE_NEE:
            case ZONE_NEN: return MOUSE_BUTTON_PRESSED_NORTHEAST;
            }
            return MOUSE_BUTTON_PRESSED_NONE;
        }

        static inline Pointer const& get_pointer(int zone)
        {
            switch (zone){
            case ZONE_N  :
            case ZONE_S  : return size_NS_pointer();
            case ZONE_E  :
            case ZONE_W  : return size_WE_pointer();
            case ZONE_NWN:
            case ZONE_NWW:
            case ZONE_SWW:
            case ZONE_SWS:
            case ZONE_SES:
            case ZONE_SEE:
            case ZONE_NEE:
            case ZONE_NEN: return size_NESW_pointer();
            }
            return normal_pointer();
        }
    } zone;

    /// \return true when \c xPos and \c yPos are captured
    bool input_mouse(uint16_t pointerFlags, uint16_t xPos, uint16_t yPos);

private:

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
    bool const window_level_supported_ex;
    bool allow_resize_hosted_desktop_    = false;
    bool enable_resizing_hosted_desktop_ = false;
    Pointer const* current_mouse_pointer = nullptr;
    bool rail_enabled = false;

    EventsGuard events_guard;
    Rect protocol_window_rect;

public:
    ClientExecute(
        EventContainer& events,
        gdi::GraphicApi & drawable, FrontAPI & front,
        WindowListCaps const & window_list_caps, bool verbose);

    ~ClientExecute();

    void set_verbose(bool verbose);

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

private:
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

