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

#include "RAIL/client_execute.hpp"
#include "core/app_path.hpp"
#include "core/channel_list.hpp"
#include "core/channel_names.hpp"
#include "core/front_api.hpp"
#include "core/misc.hpp"
#include "core/RDP/capabilities/window.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/rdp_pointer.hpp"
#include "core/RDP/remote_programs.hpp"
#include "core/stream_throw_helpers.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/text_metrics.hpp"
#include "mod/mod_api.hpp"
#include "mod/rdp/channels/rail_window_id_manager.hpp"
#include "utils/bitmap_from_file.hpp"
#include "utils/stream.hpp"
#include "utils/region.hpp"
#include "utils/strutils.hpp"
#include "mod/rdp/channels/virtual_channel_data_sender.hpp" // msgdump_c
#include "mod/rdp/channels/rail_window_id_manager.hpp"      // TODO only for RemoteProgramsWindowIdManager::INVALID_WINDOW_ID
#include "core/RDP/slowpath.hpp"
#include "utils/sugar/create_enum_map.hpp"
#include "utils/timebase.hpp"


#define INTERNAL_MODULE_WINDOW_ID    40000

#define AUXILIARY_WINDOW_ID          40001


constexpr uint16_t MINIMUM_WINDOW_WIDTH = 640;
constexpr uint16_t MINIMUM_WINDOW_HEIGHT = 480;

namespace
{

struct RectComputation
{
    bool substract_cx_in_x;
    bool substract_cy_in_y;
    bool substract_cx_in_cx;
    bool substract_cy_in_cy;
    int16_t x;
    int16_t y;
    uint16_t cx;
    uint16_t cy;

    constexpr RectComputation(
        bool substract_cx_in_x = false, int16_t x = 0,
        bool substract_cy_in_y = false, int16_t y = 0,
        bool substract_cx_in_cx = false, uint16_t cx = 0,
        bool substract_cy_in_cy = false, uint16_t cy = 0
    ) noexcept
    : substract_cx_in_x(substract_cx_in_x)
    , substract_cy_in_y(substract_cy_in_y)
    , substract_cx_in_cx(substract_cx_in_cx)
    , substract_cy_in_cy(substract_cy_in_cy)
    , x(x)
    , y(y)
    , cx(cx)
    , cy(cy)
    {}

    Rect compute(Rect const& w) const
    {
        return Rect(
            w.x + (substract_cx_in_x ? w.cx - x : x),
            w.y + (substract_cy_in_y ? w.cy - y : y),
            (substract_cx_in_cx ? w.cx - cx : cx),
            (substract_cy_in_cy ? w.cy - cy : cy)
        );
    }
};

using WindowArea = ClientExecute::WindowArea;
using Zone = ClientExecute::Zone;

constexpr RectComputation area_compute_button(int n, bool with_border = true)
{
    return RectComputation(
        true,  int16_t(with_border + Zone::button_width * n),
        false, int16_t(with_border),
        false, Zone::button_width,
        false, Zone::corner - 1
    );
}

// d[0] 0=left, 1=middle, 2=right
// d[1] 0=top, 1=middle, 2=bottom
// d[2] 0=horizontal 1=vertical
constexpr RectComputation area_compute_side(int horizontal, int vertical, int diagonal)
{
    int16_t dx = diagonal ? Zone::thickness : Zone::corner;
    int16_t dy = diagonal ? Zone::corner : Zone::thickness;
    return RectComputation(
        (horizontal == 2), (horizontal == 0) ? 0 : (horizontal == 1) ? Zone::corner : dx,
        (vertical   == 2), (vertical   == 0) ? 0 : (vertical   == 1) ? Zone::corner : dy,
        (horizontal == 1), uint16_t((horizontal == 1) ? 2*Zone::corner : dx),
        (vertical   == 1), uint16_t((vertical   == 1) ? 2*Zone::corner : dy)
    );
}

DECLARE_ENUM_MAP(area_to_render_zone, RectComputation,
    (WindowArea::Title, RectComputation(
        false, 21 + 1,
        false, 1,
        true,  21 + 2 + Zone::button_width*3,
        false, Zone::corner - 1))

    (WindowArea::Icon, RectComputation(
        false, 1,
        false, 1,
        false, 21,
        false, Zone::corner - 1))

    (WindowArea::Close, area_compute_button(1))
    (WindowArea::Maxi, area_compute_button(2))
    (WindowArea::Mini, area_compute_button(3))
    (WindowArea::Resize, area_compute_button(4))

    (WindowArea::N, area_compute_side(1, 0, 0))
    (WindowArea::S, area_compute_side(1, 2, 0))
    (WindowArea::W, area_compute_side(0, 1, 1))
    (WindowArea::E, area_compute_side(2, 1, 1))

    (WindowArea::NWN, area_compute_side(0, 0, 0))
    (WindowArea::NWW, area_compute_side(0, 0, 1))
    (WindowArea::NEE, area_compute_side(2, 0, 1))
    (WindowArea::NEN, area_compute_side(2, 0, 0))

    (WindowArea::SES, area_compute_side(2, 2, 0))
    (WindowArea::SEE, area_compute_side(2, 2, 1))
    (WindowArea::SWW, area_compute_side(0, 2, 1))
    (WindowArea::SWS, area_compute_side(0, 2, 0))

    (WindowArea::NUMBER_OF_AREAS_OR_INVALID, RectComputation())
);

DECLARE_ENUM_MAP(area_to_render_zone_without_border, RectComputation,
    (WindowArea::Title, RectComputation(
        false, 21,
        false, 0,
        true,  21 + Zone::button_width*3,
        false, Zone::corner - 1))

    (WindowArea::Icon, RectComputation(
        false, 0,
        false, 0,
        false, 21,
        false, Zone::corner - 1))

    (WindowArea::Close, area_compute_button(1, false))
    (WindowArea::Maxi, area_compute_button(2, false))
    (WindowArea::Mini, area_compute_button(3, false))
    (WindowArea::Resize, area_compute_button(4, false))

    (WindowArea::N, RectComputation())
    (WindowArea::S, RectComputation())
    (WindowArea::W, RectComputation())
    (WindowArea::E, RectComputation())

    (WindowArea::NWN, RectComputation())
    (WindowArea::NWW, RectComputation())
    (WindowArea::NEE, RectComputation())
    (WindowArea::NEN, RectComputation())

    (WindowArea::SES, RectComputation())
    (WindowArea::SEE, RectComputation())
    (WindowArea::SWW, RectComputation())
    (WindowArea::SWS, RectComputation())

    (WindowArea::NUMBER_OF_AREAS_OR_INVALID, RectComputation())
);

constexpr RectComputation desktop_zone(
    false, 1,
    false, 1 + Zone::corner - 1,
    true, 2,
    true, 1 + Zone::corner - 1 + 1
);

constexpr RectComputation desktop_zone_without_border(
    false, 0,
    false, 1 + Zone::corner - 1,
    true, 0,
    true, Zone::corner - 1
);

} // anonymous namespace

Rect ClientExecute::Zone::get_zone(WindowArea area, Rect w)
{
    return area_to_render_zone(area).compute(w);
}

namespace
{
    struct RectZone
    {
        Rect rect;
        ClientExecute::WindowArea area;
    };

    RectZone get_rect_area(Rect const& w, bool is_maximized, bool resizable_hosted, int16_t x, int16_t y)
    {
        #define RETURN_IF_CONTAINS(area) do {         \
            Rect rect = render_zone(area).compute(w); \
            if (rect.contains_pt(x, y)) {             \
                return {rect, area};                  \
            }                                         \
        } while (0)

        if (!is_maximized) {
            auto r = desktop_zone.compute(w);
            if (r.contains_pt(x, y)) {
                return {Rect(), WindowArea::NUMBER_OF_AREAS_OR_INVALID};
            }

            auto const & render_zone = area_to_render_zone;

            RETURN_IF_CONTAINS(WindowArea::Close);
            RETURN_IF_CONTAINS(WindowArea::Maxi);
            RETURN_IF_CONTAINS(WindowArea::Mini);
            if (resizable_hosted) {
                RETURN_IF_CONTAINS(WindowArea::Resize);
            }
            RETURN_IF_CONTAINS(WindowArea::Icon);

            RETURN_IF_CONTAINS(WindowArea::Title);
            RETURN_IF_CONTAINS(WindowArea::N);
            RETURN_IF_CONTAINS(WindowArea::NWN);
            RETURN_IF_CONTAINS(WindowArea::NWW);
            RETURN_IF_CONTAINS(WindowArea::W);
            RETURN_IF_CONTAINS(WindowArea::SWW);
            RETURN_IF_CONTAINS(WindowArea::SWS);
            RETURN_IF_CONTAINS(WindowArea::S);
            RETURN_IF_CONTAINS(WindowArea::SES);
            RETURN_IF_CONTAINS(WindowArea::SEE);
            RETURN_IF_CONTAINS(WindowArea::E);
            RETURN_IF_CONTAINS(WindowArea::NEE);
            RETURN_IF_CONTAINS(WindowArea::NEN);
        }
        else {
            auto r = desktop_zone_without_border.compute(w);
            if (r.contains_pt(x, y)) {
                return {Rect(), WindowArea::NUMBER_OF_AREAS_OR_INVALID};
            }

            auto const & render_zone = area_to_render_zone_without_border;

            RETURN_IF_CONTAINS(WindowArea::Close);
            RETURN_IF_CONTAINS(WindowArea::Maxi);
            RETURN_IF_CONTAINS(WindowArea::Mini);
            if (resizable_hosted) {
                RETURN_IF_CONTAINS(WindowArea::Resize);
            }
            RETURN_IF_CONTAINS(WindowArea::Icon);

            RETURN_IF_CONTAINS(WindowArea::Title);

            // no border
        }

        #undef RETURN_IF_CONTAINS

        return {Rect(), WindowArea::NUMBER_OF_AREAS_OR_INVALID};
    }
}

namespace
{

DECLARE_ENUM_MAP(area_to_rail_wmsz, uint16_t,
    (WindowArea::N, RAIL_WMSZ_TOP)
    (WindowArea::S, RAIL_WMSZ_BOTTOM)
    (WindowArea::W, RAIL_WMSZ_LEFT)
    (WindowArea::E, RAIL_WMSZ_RIGHT)

    (WindowArea::NWN, RAIL_WMSZ_TOPLEFT)
    (WindowArea::NWW, RAIL_WMSZ_TOPLEFT)

    (WindowArea::SES, RAIL_WMSZ_BOTTOMRIGHT)
    (WindowArea::SEE, RAIL_WMSZ_BOTTOMRIGHT)

    (WindowArea::SWW, RAIL_WMSZ_BOTTOMLEFT)
    (WindowArea::SWS, RAIL_WMSZ_BOTTOMLEFT)

    (WindowArea::NEE, RAIL_WMSZ_TOPRIGHT)
    (WindowArea::NEN, RAIL_WMSZ_TOPRIGHT)

    (WindowArea::Title, RAIL_WMSZ_MOVE)

    (WindowArea::Icon, 0)
    (WindowArea::Close, 0)
    (WindowArea::Maxi, 0)
    (WindowArea::Mini, 0)
    (WindowArea::Resize, 0)
    (WindowArea::NUMBER_OF_AREAS_OR_INVALID, 0)
);

DECLARE_ENUM_MAP(area_to_predefined_pointer, PredefinedPointer,
    (WindowArea::N, PredefinedPointer::NS)
    (WindowArea::S, PredefinedPointer::NS)

    (WindowArea::W, PredefinedPointer::WE)
    (WindowArea::E, PredefinedPointer::WE)

    (WindowArea::NWN, PredefinedPointer::NWSE)
    (WindowArea::NWW, PredefinedPointer::NWSE)
    (WindowArea::SES, PredefinedPointer::NWSE)
    (WindowArea::SEE, PredefinedPointer::NWSE)

    (WindowArea::SWW, PredefinedPointer::NESW)
    (WindowArea::SWS, PredefinedPointer::NESW)
    (WindowArea::NEE, PredefinedPointer::NESW)
    (WindowArea::NEN, PredefinedPointer::NESW)

    (WindowArea::Icon, PredefinedPointer::Normal)
    (WindowArea::Title, PredefinedPointer::Normal)
    (WindowArea::Close, PredefinedPointer::Normal)
    (WindowArea::Maxi, PredefinedPointer::Normal)
    (WindowArea::Mini, PredefinedPointer::Normal)
    (WindowArea::Resize, PredefinedPointer::Normal)
    (WindowArea::NUMBER_OF_AREAS_OR_INVALID, PredefinedPointer::Normal)
);

void update_adjusted_window_rect(
    RDP::RAIL::NewOrExistingWindow& order,
    Rect const& adjusted_window_rect)
{
    order.ClientOffsetX(adjusted_window_rect.x + 6);
    order.ClientOffsetY(adjusted_window_rect.y + 25);
    order.WindowOffsetX(adjusted_window_rect.x);
    order.WindowOffsetY(adjusted_window_rect.y);
    order.WindowWidth(adjusted_window_rect.cx);
    order.WindowHeight(adjusted_window_rect.cy);
    order.VisibleOffsetX(adjusted_window_rect.x);
    order.VisibleOffsetY(adjusted_window_rect.y);
    order.NumVisibilityRects(1);
    order.VisibilityRects(
        0,
        RDP::RAIL::Rectangle(0, 0, adjusted_window_rect.cx, adjusted_window_rect.cy));
}

RDP::RAIL::NewOrExistingWindow create_window_with_title_pdu(
    uint32_t fieldsPresentFlags,
    std::string_view window_title,
    Rect const& adjusted_window_rect)
{
    RDP::RAIL::NewOrExistingWindow order;

    order.header.FieldsPresentFlags(
        RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
      | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTDELTA
      | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
      | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
      | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
      | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
      | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
      | RDP::RAIL::WINDOW_ORDER_FIELD_SHOW
      | RDP::RAIL::WINDOW_ORDER_FIELD_STYLE
      | RDP::RAIL::WINDOW_ORDER_FIELD_TITLE
      | RDP::RAIL::WINDOW_ORDER_FIELD_OWNER
      | fieldsPresentFlags
    );
    order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

    order.OwnerWindowId(0x0);
    order.Style(0x14EE0000);
    order.ExtendedStyle(0x40310);
    order.ShowState(5);
    order.TitleInfo(window_title);
    order.WindowClientDeltaX(6);
    order.WindowClientDeltaY(25);

    update_adjusted_window_rect(order, adjusted_window_rect);

    return order;
}

RDP::RAIL::NewOrExistingWindow create_move_resize_window_pdu(
    bool window_level_supported_ex,
    Rect const& adjusted_window_rect)
{
    RDP::RAIL::NewOrExistingWindow order;

    order.header.FieldsPresentFlags(
        RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
      | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
      | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
      | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
      | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
      | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
      | (window_level_supported_ex ? RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREASIZE : 0)
    );
    order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

    order.ClientAreaWidth(adjusted_window_rect.cx - 6 * 2);
    order.ClientAreaHeight(adjusted_window_rect.cy - 25 - 6);

    update_adjusted_window_rect(order, adjusted_window_rect);

    return order;
}

RDP::RAIL::NewOrExistingWindow create_resize_window_pdu(
    bool window_level_supported_ex,
    Rect const& adjusted_window_rect)
{
    RDP::RAIL::NewOrExistingWindow order;

    order.header.FieldsPresentFlags(
        RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
      | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
      | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
      | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
      | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
      | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
      | RDP::RAIL::WINDOW_ORDER_FIELD_SHOW
      | RDP::RAIL::WINDOW_ORDER_FIELD_STYLE
      | (window_level_supported_ex ? RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREASIZE : 0)
    );
    order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

    order.ClientAreaWidth(adjusted_window_rect.cx - 6 * 2);
    order.ClientAreaHeight(adjusted_window_rect.cy - 25 - 6);

    update_adjusted_window_rect(order, adjusted_window_rect);

    order.ShowState(5);
    order.Style(0x16CF0000);
    order.ExtendedStyle(0x110);

    return order;
}

RDP::RAIL::NewOrExistingWindow create_minimized_window_pdu(
    bool window_level_supported_ex)
{
    RDP::RAIL::NewOrExistingWindow order;

    order.header.FieldsPresentFlags(
        RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
      | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
      | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
      | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
      | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
      | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
      | RDP::RAIL::WINDOW_ORDER_FIELD_SHOW
      | RDP::RAIL::WINDOW_ORDER_FIELD_STYLE
      | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTDELTA
      | (window_level_supported_ex ? RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREASIZE : 0)
    );
    order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

    order.ClientOffsetY(800);
    order.VisibleOffsetY(800);
    order.WindowWidth(160);
    order.WindowHeight(ClientExecute::TITLE_BAR_HEIGHT);
    order.WindowOffsetX(0);
    order.WindowOffsetY(800);
    order.NumVisibilityRects(1);
    order.VisibilityRects(
        0,
        RDP::RAIL::Rectangle(0, 0, 160, ClientExecute::TITLE_BAR_HEIGHT));
    order.ShowState(2);
    order.Style(0x34EE0000);
    order.ExtendedStyle(0x40310);

    return order;
}

RDP::RAIL::NewOrExistingWindow create_maximized_window_pdu(
    bool window_level_supported_ex,
    Rect const& adjusted_window_rect)
{
    RDP::RAIL::NewOrExistingWindow order;

    order.header.FieldsPresentFlags(
        RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
      | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
      | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
      | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
      | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
      | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
      | RDP::RAIL::WINDOW_ORDER_FIELD_SHOW
      | RDP::RAIL::WINDOW_ORDER_FIELD_STYLE
      | (window_level_supported_ex ? RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREASIZE : 0)
    );
    order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

    order.ClientAreaWidth(adjusted_window_rect.cx);
    order.ClientAreaHeight(adjusted_window_rect.cy - 25);
    order.WindowWidth(adjusted_window_rect.cx + 2);
    order.WindowHeight(adjusted_window_rect.cy + 2);
    order.NumVisibilityRects(1);
    order.VisibilityRects(
        0,
        RDP::RAIL::Rectangle(0, 0, adjusted_window_rect.cx, adjusted_window_rect.cy + 1));

    order.ClientOffsetX(adjusted_window_rect.x/* + 0*/);
    order.ClientOffsetY(adjusted_window_rect.y + 25);
    order.WindowOffsetX(adjusted_window_rect.x + -1);
    order.WindowOffsetY(adjusted_window_rect.y + -1);
    order.VisibleOffsetX(adjusted_window_rect.x/* + 0*/);
    order.VisibleOffsetY(adjusted_window_rect.y/* + 0*/);

    order.ShowState(3);
    order.Style(0x17CF0000);
    order.ExtendedStyle(0x110);

    return order;
}

void log_new_or_existing_window(
    RDP::RAIL::NewOrExistingWindow const& order,
    bool verbose,
    char const* function_name)
{
    if (REDEMPTION_UNLIKELY(verbose)) {
        StaticOutStream<1024> out_s;
        order.emit(out_s);
        order.log(LOG_INFO);
        LOG(LOG_INFO,
            "ClientExecute::%s: Send NewOrExistingWindow to client: size=%zu (0)",
            function_name, out_s.get_offset() - 1);
    }
}

void refresh_window_rect(mod_api* mod, Rect const& window_rect)
{
    if (mod) {
        mod->rdp_input_invalidate(Rect(
            window_rect.x,
            window_rect.y,
            checked_int(window_rect.x + window_rect.cx),
            checked_int(window_rect.y + window_rect.cy)
        ));
    }
}

constexpr auto black = encode_color24()(BLACK);
constexpr auto white = encode_color24()(WHITE);

constexpr auto button_normal_color = encode_color24()(WHITE);
constexpr auto button_hover_color  = encode_color24()(BGRColor(0xE7EBE7));
constexpr auto button_active_color = encode_color24()(BGRColor(0xCBCACA));
constexpr auto close_normal_color  = button_normal_color;
constexpr auto close_hover_color   = encode_color24()(BGRColor(0x2311E8));
constexpr auto close_active_color  = encode_color24()(BGRColor(0x7B73F7));


template<class PDUBuilder>
void send_to_channel(
    FrontAPI& front, CHANNELS::ChannelDef const* channel,
    bool verbose, char const* function_name, char const* desc,
    uint16_t order_type, PDUBuilder&& pdu_builder)
{
    assert(channel);

    StaticOutStream<1024> out_s;
    RAILPDUHeader header;
    header.emit_begin(out_s, order_type);

    auto pdu = pdu_builder();
    pdu.emit(out_s);

    header.emit_end();

    const size_t   length = out_s.get_offset();
    const uint32_t flags  = CHANNELS::CHANNEL_FLAG_FIRST
                          | CHANNELS::CHANNEL_FLAG_LAST;

    if (verbose) {
        const bool send              = true;
        const bool from_or_to_client = true;
        ::msgdump_c(
            send, from_or_to_client, uint32_t(length),
            flags, out_s.get_produced_bytes());
        LOG_IF(verbose, LOG_INFO, "ClientExecute::%s: Send to client - %s", function_name, desc);
        pdu.log(LOG_INFO);
    }

    front.send_to_channel(*channel, out_s.get_produced_bytes(), length, flags);
}

void draw_icon(
    gdi::GraphicApi& drawable, Rect const& rect,
    Bitmap const& wallix_icon_min, Rect const& clip)
{
    drawable.draw(RDPOpaqueRect(rect, white), clip, gdi::ColorCtx::depth24());
    drawable.draw(RDPMemBlt(0, Rect(rect.x + 3, rect.y + 4, 16, 16), 0xCC, 0, 0, 0),
                  clip, wallix_icon_min);
}

void draw_text_region(
    gdi::GraphicApi& drawable, Font const* font, Rect const& rect,
    RDPColor fg_color, RDPColor bg_color,
    int16_t offset_x, int16_t offset_y, zstring_view text, Rect const& clip)
{
    drawable.draw(RDPOpaqueRect(rect, bg_color), clip, gdi::ColorCtx::depth24());
    if (font) {
        gdi::server_draw_text(
            drawable,
            *font,
            rect.x + offset_x,
            rect.y + offset_y,
            text.c_str(),
            fg_color,
            bg_color,
            gdi::ColorCtx::depth24(),
            rect
        );
    }
}

enum class ButtonStyle
{
    Normal,
    Hover,
    Active,
};

constexpr RDPColor normal_button_bg_colors[]{
    button_normal_color,
    button_hover_color,
    button_active_color,
};

constexpr RDPColor close_button_bg_colors[]{
    close_normal_color,
    close_hover_color,
    close_active_color,
};

constexpr RDPColor close_button_fg_colors[]{
    black,
    white,
    white,
};

void draw_title(
    gdi::GraphicApi& drawable, Font const* font, Rect const& rect,
    zstring_view window_title, Rect const& clip)
{
    auto fg_color = black;
    auto bg_color = white;
    draw_text_region(drawable, font, rect, fg_color, bg_color, 1, 3, window_title, clip);
}

void draw_button_mini(
    gdi::GraphicApi& drawable, Font const* font, Rect const& rect,
    ButtonStyle button_style, Rect const& clip)
{
    auto fg_color = black;
    auto bg_color = normal_button_bg_colors[underlying_cast(button_style)];
    draw_text_region(drawable, font, rect, fg_color, bg_color, 12, 3, "−"_zv, clip);
}

void draw_button_maxi(
    gdi::GraphicApi& drawable, bool is_maximized, Rect const& rect_maxi,
    ButtonStyle button_style, Rect const& clip)
{
    auto bg_color = normal_button_bg_colors[underlying_cast(button_style)];
    auto fg_color = black;
    auto const depth = gdi::ColorCtx::depth24();

    auto rect = rect_maxi;

    drawable.draw(RDPOpaqueRect(rect, bg_color), clip, depth);

    if (is_maximized) {
        rect.x  += 14 + 2;
        rect.y  += 7;
        rect.cx -= 14 * 2 + 2;
        rect.cy -= 7 * 2 + 2;

        drawable.draw(RDPOpaqueRect(rect, fg_color), clip, depth);

        rect = rect.shrink(1);

        drawable.draw(RDPOpaqueRect(rect, bg_color), clip, depth);

        rect = rect_maxi;

        rect.x  += 14;
        rect.y  += 7 + 2;
        rect.cx -= 14 * 2 + 2;
        rect.cy -= 7 * 2 + 2;

        drawable.draw(RDPOpaqueRect(rect, fg_color), clip, depth);
    }
    else {
        rect.x  += 14;
        rect.y  += 7;
        rect.cx -= 14 * 2;
        rect.cy -= 7 * 2;

        drawable.draw(RDPOpaqueRect(rect, fg_color), clip, depth);
    }

    rect = rect.shrink(1);

    drawable.draw(RDPOpaqueRect(rect, bg_color), clip, depth);
}

void draw_button_resize_hosted_desktop(
    gdi::GraphicApi& drawable, bool enable_resizing_hosted_desktop,
    Rect rect, ButtonStyle button_style, Rect const& clip)
{
    auto bg_color = normal_button_bg_colors[underlying_cast(button_style)];
    auto fg_color = black;
    auto const depth = gdi::ColorCtx::depth24();

    drawable.draw(RDPOpaqueRect(rect, bg_color), clip, depth);

    if (enable_resizing_hosted_desktop) {
        rect.x  += 22;
        rect.y  += 8;
        rect.cx  = 2;
        rect.cy  = 7;

        drawable.draw(RDPOpaqueRect(rect, fg_color), clip, depth);

        rect.x  -= 4;
        rect.y  += 2;
        rect.cx  = 4;
        rect.cy  = 3;

        drawable.draw(RDPOpaqueRect(rect, fg_color), clip, depth);

        rect.x  -= 2;
        rect.y  -= 3;
        rect.cx  = 2;
        rect.cy  = 9;

        drawable.draw(RDPOpaqueRect(rect, fg_color), clip, depth);

        rect.x  -= 4;
        rect.y  += 4;
        rect.cx  = 4;
        rect.cy  = 1;

        drawable.draw(RDPOpaqueRect(rect, fg_color), clip, depth);
    }
    else {
        rect.x  += 15;
        rect.y  += 6;
        rect.cx  = 7;
        rect.cy  = 2;

        drawable.draw(RDPOpaqueRect(rect, fg_color), clip, depth);

        rect.x  += 2;
        rect.y  += 2;
        rect.cx  = 3;
        rect.cy  = 4;

        drawable.draw(RDPOpaqueRect(rect, fg_color), clip, depth);

        rect.x  -= 3;
        rect.y  += 4;
        rect.cx  = 9;
        rect.cy  = 2;

        drawable.draw(RDPOpaqueRect(rect, fg_color), clip, depth);

        rect.x  += 4;
        rect.y  += 2;
        rect.cx  = 1;
        rect.cy  = 4;

        drawable.draw(RDPOpaqueRect(rect, fg_color), clip, depth);
    }
}

void draw_button_close(
    gdi::GraphicApi& drawable, Font const* font, Rect const& rect,
    ButtonStyle button_style, Rect const& clip)
{
    auto bg_color = close_button_bg_colors[underlying_cast(button_style)];
    auto fg_color = close_button_fg_colors[underlying_cast(button_style)];
    draw_text_region(drawable, font, rect, fg_color, bg_color, 13, 3, "x"_zv, clip);
}


constexpr std::string_view INTERNAL_MODULE_WINDOW_TITLE = "Wallix AdminBastion";

#define CASE(name) (ClientExecute::MouseAction::EventAction::name, #name)
DECLARE_ENUM_MAP(event_action_to_cstr_map, char const*,
    CASE(CapturedClick)
    CASE(ResizeHostedDesktop)
    CASE(Minimize)
    CASE(Maximaze)
    CASE(MaximazeVertical)
    CASE(Close)
    CASE(StartMoveResize)
    CASE(StopMoveResize)
    CASE(MoveResize)
    CASE(ActiveButton)
    CASE(UnactiveButton)
    CASE(Nothing)
);
#undef CASE

#define CASE(name) (name, #name)
DECLARE_ENUM_MAP(area_to_cstr, char const*,
    CASE(WindowArea::N)
    CASE(WindowArea::NWN)
    CASE(WindowArea::NWW)
    CASE(WindowArea::W)
    CASE(WindowArea::SWW)
    CASE(WindowArea::SWS)
    CASE(WindowArea::S)
    CASE(WindowArea::SES)
    CASE(WindowArea::SEE)
    CASE(WindowArea::E)
    CASE(WindowArea::NEE)
    CASE(WindowArea::NEN)
    CASE(WindowArea::Icon)
    CASE(WindowArea::Title)
    CASE(WindowArea::Close)
    CASE(WindowArea::Maxi)
    CASE(WindowArea::Mini)
    CASE(WindowArea::Resize)
    CASE(WindowArea::NUMBER_OF_AREAS_OR_INVALID)
);
#undef CASE

} // anonymous namespace


ClientExecute::ClientExecute(
    CRef<TimeBase> time_base, gdi::GraphicApi & drawable, FrontAPI & front,
    WindowListCaps const & window_list_caps, bool verbose)
: front_(front)
, drawable_(drawable)
, verbose(verbose)
, wallix_icon_min(bitmap_from_file(app_path(AppPath::WallixIconMin), BLACK))
, auxiliary_window_id(RemoteProgramsWindowIdManager::INVALID_WINDOW_ID)
, window_title(INTERNAL_MODULE_WINDOW_TITLE)
, current_mouse_pointer(PredefinedPointer::Normal)
, window_level_supported_ex(window_list_caps.WndSupportLevel & TS_WINDOW_LEVEL_SUPPORTED_EX)
, time(time_base)
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::ClientExecute()");
}

ClientExecute::~ClientExecute()
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::~ClientExecute()");
    this->reset(false);
}

void ClientExecute::enable_remote_program(bool enable)
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::enable_remote_program()");
    this->rail_enabled = enable;
}

Rect ClientExecute::adjust_rect(Rect rect)
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::adjust_rect(%s)", rect);

    if (!this->rail_enabled) {
        return rect;
    }

    if (this->window_rect.isempty()) {
        this->window_rect = Rect(rect.x + rect.cx * 10 / 100,
                                 rect.y + rect.cy * 10 / 100,
                                 rect.cx * 80 / 100,
                                 rect.cy * 80 / 100);
        this->window_offset.x = -rect.x;
        this->window_offset.y = -rect.y;

        this->update_rects();
    }

    Rect result_rect = this->window_rect.shrink(1);

    result_rect.y  += TITLE_BAR_HEIGHT;
    result_rect.cy -= TITLE_BAR_HEIGHT;

    return result_rect;
}   // adjust_rect

Rect ClientExecute::get_current_work_area_rect() const
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::get_current_work_area_rect()");
    assert(this->work_area_count);

    if (!this->window_rect.isempty()) {
        const Rect adjusted_window_rect = this->window_rect.offset(
            this->window_offset.x, this->window_offset.y);

        size_t current_surface_size = 0;
        Rect current_work_area = this->work_areas[0];
        for (Rect const& work_area : array_view{this->work_areas, this->work_area_count}) {
            Rect intersect_rect = work_area.intersect(adjusted_window_rect);
            if (!intersect_rect.isempty()) {
                size_t surface_size = intersect_rect.cx * intersect_rect.cy;
                if (current_surface_size < surface_size) {
                    current_surface_size = surface_size;
                    current_work_area = work_area;
                }
            }
        }

        return current_work_area.offset(-this->window_offset.x, -this->window_offset.y);
    }

    return this->work_areas[0].offset(-this->window_offset.x, -this->window_offset.y);
}

Rect ClientExecute::get_auxiliary_window_rect() const
{
    LOG_IF(this->verbose, LOG_INFO, "get_window_offset(%u) -> (%s)", this->auxiliary_window_id, this->auxiliary_window_rect);
    if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID == this->auxiliary_window_id) {
        LOG_IF(this->verbose, LOG_INFO, "get_window_offset(%u) -> ()", this->auxiliary_window_id);
        return Rect();
    }
    return this->auxiliary_window_rect;
}

void ClientExecute::input_invalidate(const Rect r)
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::input_invalidate(): r=%s", r);

    if (!this->channel_) return;

    bool is_updated = false;

    auto draw_area = [this, &r, &is_updated](WindowArea area, auto f){
        if (auto area_rect = Zone::get_zone(area, this->window_rect)
          ; r.has_intersection(area_rect))
        {
            f(area_rect);
            is_updated = true;
        }
    };

    draw_area(WindowArea::Icon, [&](Rect const& area_rect){
        draw_icon(this->drawable_, area_rect, this->wallix_icon_min, r);
    });

    draw_area(WindowArea::Title, [&](Rect const& area_rect){
        draw_title(this->drawable_, this->font_, area_rect, this->window_title, r);
    });

    if (this->allow_resize_hosted_desktop_) {
        draw_area(WindowArea::Resize, [&](Rect const& area_rect){
            draw_button_resize_hosted_desktop(
                this->drawable_, this->enable_resizing_hosted_desktop_,
                area_rect, ButtonStyle::Normal, r);
        });
    }

    draw_area(WindowArea::Mini, [&](Rect const& area_rect){
        draw_button_mini(this->drawable_, this->font_, area_rect, ButtonStyle::Normal, r);
    });

    draw_area(WindowArea::Maxi, [&](Rect const& area_rect){
        bool const is_maximized = this->maximized_state == MaximizedState::FullScreen;
        draw_button_maxi(this->drawable_, is_maximized, area_rect, ButtonStyle::Normal, r);
    });

    draw_area(WindowArea::Close, [&](Rect const& area_rect){
        draw_button_close(this->drawable_, this->font_, area_rect, ButtonStyle::Normal, r);
    });

    if (is_updated) {
        this->drawable_.sync();
    }
}   // input_invalidate

void ClientExecute::adjust_window_to_mod()
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::adjust_window_to_mod()");
    this->maximized_state = MaximizedState::No;
    Rect const work_area_rect = this->get_current_work_area_rect();

    Dimension const module_dimension = this->mod_ ? this->mod_->get_dim() : Dimension{};

    Dimension const prefered_window_dimension(
        module_dimension.w + 2,
        module_dimension.h + 2 + TITLE_BAR_HEIGHT);

    if ((work_area_rect.cx > prefered_window_dimension.w)
     && (work_area_rect.cy > prefered_window_dimension.h)
    ) {
        this->window_rect.cx = prefered_window_dimension.w;
        this->window_rect.cy = prefered_window_dimension.h;
    }

    this->update_rects();

    const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset);

    {
        RDP::RAIL::NewOrExistingWindow order = create_resize_window_pdu(
            this->window_level_supported_ex,
            adjusted_window_rect);

        log_new_or_existing_window(order, this->verbose, "adjust_window_to_mod");

        this->drawable_.draw(order);
    }

    this->update_widget();

    this->on_new_or_existing_window(adjusted_window_rect);
}

void ClientExecute::maximize_restore_window()
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::maximize_restore_window()");
    if (this->maximized_state == MaximizedState::FullScreen) {
        this->window_rect = this->window_rect_normal;
        this->maximized_state = MaximizedState::No;
        this->adjust_window_to_mod();
    }
    else {
        this->maximized_state = MaximizedState::FullScreen;

        this->window_rect_normal = this->window_rect;

        Rect work_area_rect = this->get_current_work_area_rect();

        this->window_rect = Rect(work_area_rect.x - 1,
                                 work_area_rect.y - 1,
                                 work_area_rect.cx + 1 * 2,
                                 work_area_rect.cy + 1 * 2);

        this->update_rects();

        const Rect adjusted_window_rect = work_area_rect.offset(this->window_offset);

        {
            RDP::RAIL::NewOrExistingWindow order = create_maximized_window_pdu(
                this->window_level_supported_ex,
                adjusted_window_rect);

            log_new_or_existing_window(order, this->verbose, "maximize_restore_window");

            this->drawable_.draw(order);
        }

        this->update_widget();

        this->on_new_or_existing_window(adjusted_window_rect);
    }   // if (!this->maximized_state)
}   // maximize_restore_window

void ClientExecute::maximize_vertical_restore_window()
{
    if (this->maximized_state == MaximizedState::VerticalScreen) {
        this->maximized_state = MaximizedState::No;
        this->window_rect = this->window_rect_normal;
    }
    else {
        this->maximized_state = MaximizedState::VerticalScreen;
        this->window_rect_normal = this->window_rect;

        const Rect work_area_rect = this->get_current_work_area_rect();
        this->window_rect = Rect(this->window_rect.x, 0,
                                    this->window_rect.cx, work_area_rect.cy - 1);
        this->update_rects();
    }

    const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset);

    {
        RDP::RAIL::NewOrExistingWindow order = create_move_resize_window_pdu(
            this->window_level_supported_ex,
            adjusted_window_rect);

        log_new_or_existing_window(order, this->verbose, "mazimize_vertical_restore_window");

        this->drawable_.draw(order);
    }

    this->update_widget();

    this->on_new_or_existing_window(adjusted_window_rect);
}   // maximize_vertical_restore_window

void ClientExecute::ready(mod_api & mod, Font const & font, bool allow_resize_hosted_desktop)
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::ready");

    this->mod_  = &mod;
    this->font_ = &font;

    this->allow_resize_hosted_desktop_    = allow_resize_hosted_desktop;
    this->enable_resizing_hosted_desktop_ = true;

    this->update_rects();

    Rect rect = this->window_rect;
    rect.cy = TITLE_BAR_HEIGHT;

    this->input_invalidate(rect);

    if (this->channel_) {
        return;
    }

    this->channel_ = this->front_.get_channel_list().get_by_name(channel_names::rail);

    if (!this->channel_) {
        return;
    }

    send_to_channel(
        this->front_, this->channel_, this->verbose, "ready",
        "Server Handshake PDU",
        TS_RAIL_ORDER_HANDSHAKE, []{
            HandshakePDU handshake_pdu;
            handshake_pdu.buildNumber(7601);
            return handshake_pdu;
        });
    // send_to_channel(
    //     this->front_, this->channel_, this->verbose, "ready", "Server HandshakeEx PDU",
    //     TS_RAIL_ORDER_HANDSHAKE_EX, []{
    //         HandshakeExPDU handshakeex_pdu;
    //         handshakeex_pdu.buildNumber(7601);
    //         handshakeex_pdu.railHandshakeFlags(TS_RAIL_ORDER_HANDSHAKEEX_FLAGS_HIDEF);
    //         return handshakeex_pdu;
    //     });

    auto send_server_system_paramters_update = [this](uint32_t systemParam){
        send_to_channel(
            this->front_, this->channel_, this->verbose, "ready",
            "Server System Parameters Update PDU",
            TS_RAIL_ORDER_SYSPARAM, [systemParam]{
                ServerSystemParametersUpdatePDU server_system_parameters_update_pdu;
                server_system_parameters_update_pdu.SystemParam(systemParam);
                server_system_parameters_update_pdu.Body(0);
                return server_system_parameters_update_pdu;
            });
    };

    send_server_system_paramters_update(SPI_SETSCREENSAVESECURE);
    send_server_system_paramters_update(SPI_SETSCREENSAVEACTIVE);
}   // ready

bool ClientExecute::is_ready() const noexcept
{
    return this->channel_ && this->mod_;
}   // bool

void ClientExecute::reset(bool soft)
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::reset (%s)", (soft ? "Soft" : "Hard"));
    if (soft) {
        this->mod_ = nullptr;

        return;
    }

    if (!this->channel_) return;

    if (this->internal_module_window_created) {
        RDP::RAIL::DeletedWindow order;

        order.header.FieldsPresentFlags(
            uint32_t(RDP::RAIL::WINDOW_ORDER_STATE_DELETED)
          | uint32_t(RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW));
        order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

        if (this->verbose) {
            StaticOutStream<256> out_s;
            order.emit(out_s);
            order.log(LOG_INFO);
            LOG(LOG_INFO, "ClientExecute::reset: Send DeletedWindow to client: size=%zu", out_s.get_offset() - 1);
        }

        this->drawable_.draw(order);
        this->on_delete_window();

        this->internal_module_window_created = false;
    }

    this->auxiliary_window_id = RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;

    this->work_area_count = 0;

    this->set_target_info(nullptr);

    this->channel_ = nullptr;
}   // reset



// Check if a PDU chunk is a "unit" on the channel, which means
// - it has both FLAG_FIRST and FLAG_LAST
// - it contains at least two bytes (to read the chunk length)
// - its total length is the same as the chunk length
static void check_is_unit_throw(
    size_t total_length, uint32_t flags, InStream& chunk, const char * message, bool verbose)
{
    LOG_IF(verbose, LOG_INFO, "ClientExecute::check_is_unit_throw(%s)", message);

    if ((flags & (CHANNELS::CHANNEL_FLAG_FIRST|CHANNELS::CHANNEL_FLAG_LAST))
              != (CHANNELS::CHANNEL_FLAG_FIRST|CHANNELS::CHANNEL_FLAG_LAST)){
        LOG(LOG_ERR, "ClientExecute::%s unexpected fragmentation flags=%.4x", message, flags);
        throw Error(ERR_RDP_DATA_CHANNEL_FRAGMENTATION);
    }

    // orderLength(2)
    if (!chunk.in_check_rem(2)) {
        LOG(LOG_ERR, "Truncated ClientExecute::%s::orderLength: expected=2 remains=%zu", message, chunk.in_remain());
        throw Error(ERR_RDP_DATA_TRUNCATED);
    }

    auto order_length = chunk.in_uint16_le(); // orderLength(2)
    if (total_length != order_length){
        LOG(LOG_ERR, "ClientExecute::%s unexpected fragmentation chunk=%u total=%zu", message, order_length, total_length);
        throw Error(ERR_RDP_DATA_CHANNEL_FRAGMENTATION);
    }
}

static void send_activate_window(uint32_t flag, gdi::GraphicApi & drawable_, bool verbose)
{
    RDP::RAIL::ActivelyMonitoredDesktop order;
    order.header.FieldsPresentFlags(RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP|flag);

    switch (flag) {
        case RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ACTIVEWND:
            order.ActiveWindowId(0xFFFFFFFF);
        break;
        case RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ZORDER:
            order.NumWindowIds(1);
            order.window_ids(0, INTERNAL_MODULE_WINDOW_ID);
        break;
        default:
            LOG(LOG_ERR, "Unexpected flag %u in ClientExecute::send_activate_window", flag);
    }
    if (verbose) {
        order.log(LOG_INFO);
        LOG(LOG_INFO, "process_client_activate_pdu: Send ActivelyMonitoredDesktop to client");
    }
    drawable_.draw(order);
}

void ClientExecute::on_new_or_existing_window(Rect const & window_rect)
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::on_new_or_existing_window()");
    if (!this->protocol_window_rect.isempty())
    {
        SubRegion sub_region;

        sub_region.add_rect(this->protocol_window_rect);

        sub_region.subtract_rect(window_rect);

        auto const depth = gdi::ColorCtx::depth24();
        for (Rect const & rect : sub_region.rects) {
            RDPOpaqueRect order(rect, black);

            this->drawable_.draw(order, rect, depth);
        }
    }

    this->protocol_window_rect = window_rect;
}   // on_new_or_existing_window

void ClientExecute::on_delete_window()
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::on_delete_window()");
    assert(!this->protocol_window_rect.isempty());

    auto const depth = gdi::ColorCtx::depth24();
    RDPOpaqueRect order(this->protocol_window_rect, black);

    this->drawable_.draw(order, this->protocol_window_rect, depth);

    this->protocol_window_rect = Rect();
}   // on_delete_window



const WindowsExecuteShellParams & ClientExecute::get_windows_execute_shell_params()
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::get_windows_execute_shell_params() ->");
    this->windows_execute_shell_params.log(LOG_INFO);
    return this->windows_execute_shell_params;
}

void ClientExecute::create_auxiliary_window(Rect const window_rect)
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::create_auxiliary_window(%s)", window_rect);
    if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID != this->auxiliary_window_id) return;

    this->auxiliary_window_id = AUXILIARY_WINDOW_ID;

    const Rect adjusted_window_rect = window_rect.offset(this->window_offset);

    RDP::RAIL::NewOrExistingWindow order = create_window_with_title_pdu(
        RDP::RAIL::WINDOW_ORDER_STATE_NEW,
        "Dialog box",
        adjusted_window_rect);
    order.header.WindowId(this->auxiliary_window_id);
    order.ExtendedStyle(0x40310 | 0x8);

    log_new_or_existing_window(
        order,
        this->verbose,
        "create_auxiliary_window");

    this->drawable_.draw(order);

    this->auxiliary_window_rect = this->window_rect;
}

void ClientExecute::destroy_auxiliary_window()
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::destroy_auxiliary_window()");
    if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID == this->auxiliary_window_id) return;

    {
        RDP::RAIL::DeletedWindow order;

        order.header.FieldsPresentFlags(
            uint32_t(RDP::RAIL::WINDOW_ORDER_STATE_DELETED)
          | uint32_t(RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW)
        );
        order.header.WindowId(this->auxiliary_window_id);

        if (this->verbose) {
            StaticOutStream<1024> out_s;
            order.emit(out_s);
            order.log(LOG_INFO);
            LOG(LOG_INFO, "ClientExecute::destroy_auxiliary_window: Send DeletedWindow to client: size=%zu", out_s.get_offset() - 1);
        }

        this->drawable_.draw(order);
    }

    this->auxiliary_window_id = RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;
}

void ClientExecute::set_target_info(chars_view ti)
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::set_target_info()");
    str_append(this->window_title, ti, (ti.empty() ? ""_av : " - "_av), INTERNAL_MODULE_WINDOW_TITLE);
}

void ClientExecute::update_widget()
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::update_widget()");

    if (this->mod_) {
        Rect widget_rect_new = this->window_rect.shrink(1);
        widget_rect_new.y  += TITLE_BAR_HEIGHT;
        widget_rect_new.cy -= TITLE_BAR_HEIGHT;

        this->mod_->move_size_widget(widget_rect_new.x, widget_rect_new.y,
            widget_rect_new.cx, widget_rect_new.cy);

        this->mod_->refresh(this->window_rect);
    }
}

bool ClientExecute::is_rail_enabled() const
{
   LOG_IF(this->verbose, LOG_INFO, "ClientExecute::is_rail_enabled() -> %d", this->rail_enabled);
   return this->rail_enabled;
}

bool ClientExecute::is_resizing_hosted_desktop_enabled() const
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::is_resizing_hosted_desktop_enabled() -> %d, %d",
        this->allow_resize_hosted_desktop_,
        this->enable_resizing_hosted_desktop_
    );
    return (this->allow_resize_hosted_desktop_ && this->enable_resizing_hosted_desktop_);
}

void ClientExecute::initialize_move_size(uint16_t xPos, uint16_t yPos)
{
    assert(!this->move_size_initialized);

    this->window_rect_saved = this->window_rect;

    // TS_RAIL_ORDER_MINMAXINFO
    // Send to client - Server Min Max Info PDU (0)
    send_to_channel(
        this->front_, this->channel_, this->verbose, "initialize_move_size",
        "Server Min Max Info PDU (0)",
        TS_RAIL_ORDER_MINMAXINFO, [this]{
            ServerMinMaxInfoPDU smmipdu;

            const Rect adjusted_virtual_sreen_rect = this->virtual_screen_rect.offset(this->window_offset);

            smmipdu.WindowId(INTERNAL_MODULE_WINDOW_ID);
            smmipdu.MaxWidth(adjusted_virtual_sreen_rect.cx - 1);
            smmipdu.MaxHeight(adjusted_virtual_sreen_rect.cy - 1);
            smmipdu.MaxPosX(adjusted_virtual_sreen_rect.eright());
            smmipdu.MaxPosY(adjusted_virtual_sreen_rect.ebottom());
            smmipdu.MinTrackWidth(MINIMUM_WINDOW_WIDTH);
            smmipdu.MinTrackHeight(MINIMUM_WINDOW_HEIGHT);
            smmipdu.MaxTrackWidth(adjusted_virtual_sreen_rect.cx - 1);
            smmipdu.MaxTrackHeight(adjusted_virtual_sreen_rect.cy - 1);

            return smmipdu;
        });

    // Send to client - Server Move/Size Start PDU (0)

    uint16_t PosX = xPos;
    uint16_t PosY = yPos;
    auto area = this->mouse_action.pressed_mouse_button();
    if (area == WindowArea::Title) {
        PosX = checked_int(xPos - this->window_rect.x);
        PosY = checked_int(yPos - this->window_rect.y);
    }

    uint16_t const move_size_type = area_to_rail_wmsz(area);
    assert(move_size_type);

    send_to_channel(
        this->front_, this->channel_, this->verbose, "initialize_move_size",
        "Server Move/Size Start PDU (0)",
        TS_RAIL_ORDER_LOCALMOVESIZE, [&]{
            ServerMoveSizeStartOrEndPDU smssoepdu;

            smssoepdu.WindowId(INTERNAL_MODULE_WINDOW_ID);
            smssoepdu.IsMoveSizeStart(1);
            smssoepdu.MoveSizeType(move_size_type);
            smssoepdu.PosXOrTopLeftX(PosX);
            smssoepdu.PosYOrTopLeftY(PosY);

            return smssoepdu;
        });

    this->move_size_initialized = true;
}   // initialize_move_size


bool ClientExecute::input_mouse(uint16_t pointerFlags, uint16_t xPos, uint16_t yPos)
{
    bool const is_maximized = (this->maximized_state == MaximizedState::FullScreen);

    auto draw_button = [this, is_maximized](
        WindowArea area, Rect const& rect, ButtonStyle button_style
    ){
        switch (area) {
            case WindowArea::Mini:
                draw_button_mini(this->drawable_, this->font_, rect, button_style, rect);
                return true;

            case WindowArea::Maxi:
                draw_button_maxi(this->drawable_, is_maximized, rect, button_style, rect);
                return true;

            case WindowArea::Close:
                draw_button_close(this->drawable_, this->font_, rect, button_style, rect);
                return true;

            case WindowArea::Resize:
                if (this->allow_resize_hosted_desktop_) {
                    draw_button_resize_hosted_desktop(
                        this->drawable_, this->enable_resizing_hosted_desktop_,
                        rect, button_style, rect);
                    return true;
                }
                break;

            case WindowArea::NWN:
            case WindowArea::NWW:
            case WindowArea::W:
            case WindowArea::SWW:
            case WindowArea::SWS:
            case WindowArea::SES:
            case WindowArea::SEE:
            case WindowArea::E:
            case WindowArea::NEE:
            case WindowArea::NEN:
            case WindowArea::Title:
            case WindowArea::S:
            case WindowArea::N:
            case WindowArea::Icon:
            case WindowArea::NUMBER_OF_AREAS_OR_INVALID:
                return false;
        }

        REDEMPTION_UNREACHABLE();
    };

    using EventAction = MouseAction::EventAction;

    auto previous_pressed_area = this->mouse_action.pressed_mouse_button();

    auto const event = this->mouse_action.next_mouse_action(
        this->window_rect, MouseAction::IsMaximized(is_maximized),
        MouseAction::ResizableHosted(this->allow_resize_hosted_desktop_),
        this->time.monotonic_time, pointerFlags, xPos, yPos);

    LOG_IF(this->verbose, LOG_INFO,
        "ClientExecute::input_mouse: action=%s area=%s rect=%s pointerFlags=0x%X xPos=%u yPos=%u previous_pressed=%s",
        event_action_to_cstr_map(event.action), area_to_cstr(event.area), event.rect,
        pointerFlags, xPos, yPos, area_to_cstr(previous_pressed_area));

    // resize/move then click down without release is possible -> EventAction::StopMoveResize
    if (this->move_size_initialized
     && event.action != EventAction::MoveResize
     && (pointerFlags & SlowPath::PTRFLAGS_BUTTON1)
    ) {
        this->move_size_initialized = false;
        uint16_t const move_size_type = area_to_rail_wmsz(previous_pressed_area);
        assert(move_size_type);

        if (WindowArea::Title == previous_pressed_area) {
            LOG_IF(this->verbose, LOG_INFO, "ClientExecute::input_mouse: Mouse button 1 released from title bar");

            this->window_rect.x += xPos - this->mouse_action.pressed_button_down_x();
            this->window_rect.y += yPos - this->mouse_action.pressed_button_down_y();

            this->update_rects();
        }

        const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset);

        send_to_channel(
            this->front_, this->channel_, this->verbose, "input_mouse",
            "Server Move/Size End PDU (1)",
            TS_RAIL_ORDER_LOCALMOVESIZE, [&]{
                ServerMoveSizeStartOrEndPDU smssoepdu;

                smssoepdu.WindowId(INTERNAL_MODULE_WINDOW_ID);
                smssoepdu.IsMoveSizeStart(0);
                smssoepdu.MoveSizeType(move_size_type);
                smssoepdu.PosXOrTopLeftX(adjusted_window_rect.x);
                smssoepdu.PosYOrTopLeftY(adjusted_window_rect.y);

                return smssoepdu;
            });

        {
            RDP::RAIL::NewOrExistingWindow order = create_move_resize_window_pdu(
                this->window_level_supported_ex,
                adjusted_window_rect);

            log_new_or_existing_window(order, this->verbose, "input_mouse");

            this->drawable_.draw(order);
        }

        if (WindowArea::Title == previous_pressed_area) {
            this->update_widget();
        }

        this->on_new_or_existing_window(adjusted_window_rect);

        // possibly VerticalScreen
        this->maximized_state = MaximizedState::No;
    }

    bool resized = false;

    REDEMPTION_DIAGNOSTIC_PUSH()
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wcovered-switch-default")
    switch (event.action) {
        case EventAction::Nothing:
        case EventAction::CapturedClick:
            if (this->previous_area != event.area) {
                bool update = false;
                auto style
                    = (event.area == this->mouse_action.pressed_mouse_button())
                    ? ButtonStyle::Active
                    : (event.action == EventAction::CapturedClick)
                    ? ButtonStyle::Normal
                    : ButtonStyle::Hover;
                update |= draw_button(event.area, event.rect, style);
                update |= draw_button(this->previous_area, this->previous_rect, ButtonStyle::Normal);
                if (update) {
                    this->drawable_.sync();
                }
            }
            break;

        case EventAction::ResizeHostedDesktop:
            this->enable_resizing_hosted_desktop_ = !this->enable_resizing_hosted_desktop_;

            draw_button_resize_hosted_desktop(
                this->drawable_, this->enable_resizing_hosted_desktop_,
                event.rect, ButtonStyle::Hover, event.rect);

            this->drawable_.sync();

            if (this->enable_resizing_hosted_desktop_) {
                this->update_widget();
            }
            break;

        case EventAction::Minimize: {
            resized = true;
            RDP::RAIL::NewOrExistingWindow order = create_minimized_window_pdu(
                this->window_level_supported_ex);

            log_new_or_existing_window(order, this->verbose, "input_mouse");

            this->drawable_.draw(order);
            this->on_delete_window();

            this->drawable_.sync();

            refresh_window_rect(this->mod_, this->window_rect);

            break;
        }

        case EventAction::Maximaze:
            resized = true;
            if (this->maximized_state == MaximizedState::VerticalScreen
             && event.area == WindowArea::Title
            ) {
                this->maximize_vertical_restore_window();
            }
            else {
                this->maximize_restore_window();
            }
            break;

        case EventAction::MaximazeVertical:
            resized = true;
            this->maximize_vertical_restore_window();
            break;

        case EventAction::Close: {
            resized = true;
            draw_button_close(
                this->drawable_, this->font_, event.rect,
                ButtonStyle::Normal, event.rect);
            this->drawable_.sync();

            LOG(LOG_INFO, "ClientExecute::input_mouse: Close by user (Close Box)");
            throw Error(ERR_WIDGET);
        }

        case EventAction::StartMoveResize:
            if (this->maximized_state == MaximizedState::FullScreen) {
                this->mouse_action.remove_move_resize_state();
            }
            else {
                this->initialize_move_size(
                    this->mouse_action.pressed_button_down_x(),
                    this->mouse_action.pressed_button_down_y());
            }
            break;

        case EventAction::StopMoveResize:
            // stop called before switch, do nothing
            break;

        case EventAction::MoveResize:
            resized = true;
            if (this->full_window_drag_enabled) {
                uint16_t const captured_mouse_x = this->mouse_action.pressed_button_down_x();
                uint16_t const captured_mouse_y = this->mouse_action.pressed_button_down_y();

                this->window_rect = move_resize_rect(
                    this->mouse_action.pressed_mouse_button(),
                    xPos - captured_mouse_x,
                    yPos - captured_mouse_y,
                    this->window_rect_saved
                );

                this->update_rects();

                const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset);

                RDP::RAIL::NewOrExistingWindow order = create_window_with_title_pdu(
                    0,
                    this->window_title,
                    adjusted_window_rect);

                log_new_or_existing_window(order, this->verbose, "input_mouse");

                this->drawable_.draw(order);

                this->update_widget();

                this->on_new_or_existing_window(adjusted_window_rect);
            }
            break;

        case EventAction::ActiveButton:
            draw_button(event.area, event.rect, ButtonStyle::Active);
            if (this->previous_area != event.area) {
                draw_button(this->previous_area, this->previous_rect, ButtonStyle::Normal);
            }
            this->drawable_.sync();
            break;

        case EventAction::UnactiveButton:
            draw_button(event.area, event.rect, ButtonStyle::Hover);
            if (this->previous_area != event.area) {
                draw_button(this->previous_area, this->previous_rect, ButtonStyle::Normal);
            }
            this->drawable_.sync();
            break;

        default:
            REDEMPTION_UNREACHABLE();
    }
    REDEMPTION_DIAGNOSTIC_POP()

    this->previous_area = resized ? WindowArea::NUMBER_OF_AREAS_OR_INVALID : event.area;
    this->previous_rect = event.rect;

    if (event.area == WindowArea::NUMBER_OF_AREAS_OR_INVALID) {
        this->current_mouse_pointer = PredefinedPointer::Null;
    }
    else {
        PredefinedPointer pointer = area_to_predefined_pointer(event.area);
        if (pointer != this->current_mouse_pointer) {
            this->current_mouse_pointer = pointer;
            this->drawable_.cached_pointer(pointer);
        }
    }

    return event.area != WindowArea::NUMBER_OF_AREAS_OR_INVALID
        || event.action == EventAction::MoveResize
        ;
}   // input_mouse

void ClientExecute::update_rects()
{
    auto padding = (this->window_rect.cx - 2) % 4;
    this->window_rect.cx -= padding;
}   // update_rects

Rect ClientExecute::get_window_rect() const
{
    return this->window_rect;
}

Point ClientExecute::get_window_offset() const
{
    return this->window_offset;
}

// ==========================================================
//
// Actions on RAIL Windows from Client to Server
//
// ==========================================================


void ClientExecute::send_to_mod_rail_channel(size_t length, InStream & chunk, uint32_t flags)
{
    LOG_IF(this->verbose, LOG_INFO,
        "ClientExecute::send_to_mod_rail_channel: total_length=%zu flags=0x%08X chunk_data_length=%zu",
        length, flags, chunk.get_capacity());

    if (this->verbose) {
        const bool send              = false;
        const bool from_or_to_client = true;
        ::msgdump_c(send, from_or_to_client, length, flags,
            {chunk.get_data(), chunk.get_capacity()});
    }

    if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
        // orderType(2)
        ::check_throw(chunk,  2, "ClientExecute::SendToModRailChannel::orderType", ERR_RDP_DATA_TRUNCATED);
        this->client_order_type = chunk.in_uint16_le();
    }

    switch (this->client_order_type)
    {
        case TS_RAIL_ORDER_ACTIVATE:
            LOG_IF(this->verbose, LOG_INFO, "ClientExecute::send_to_mod_rail_channel:Client Activate PDU");
        check_is_unit_throw(length, flags, chunk, "ProcessClientActivatePDU", this->verbose);
            this->process_client_activate_pdu(chunk);
        break;

        case TS_RAIL_ORDER_CLIENTSTATUS:
            LOG_IF(this->verbose, LOG_INFO, "ClientExecute::send_to_mod_rail_channel:Client Information PDU");
            if (this->channel_){
                check_is_unit_throw(length, flags, chunk, "ProcessClientInformationPDU", this->verbose);

                ClientInformationPDU cipdu;
                cipdu.receive(chunk);

                if (this->verbose) { cipdu.log(LOG_INFO); }
            }
        break;

        //case TS_RAIL_ORDER_COMPARTMENTINFO:
        //    if (this->verbose) {
        //        LOG(LOG_INFO,"ClientExecute::send_to_mod_rail_channel: Client Compartment Status Information PDU");
        //    }
        //
        //    this->process_client_compartment_status_information_pdu(length, flags, chunk);
        //break;

        //case TS_RAIL_ORDER_CLOAK:
        //    if (this->verbose) {
        //        LOG(LOG_INFO, "ClientExecute::send_to_mod_rail_channel: Client Window Cloak State Change PDU");
        //    }
        //
        //    this->process_client_window_cloak_state_change_pdu(length, flags, chunk);
        //break;

        case TS_RAIL_ORDER_EXEC:
            LOG_IF(this->verbose, LOG_INFO, "ClientExecute::send_to_mod_rail_channel: Client Execute PDU");

            if (this->channel_) {
                check_is_unit_throw(length, flags, chunk, "ProcessClientExecutePDU", this->verbose);

                ClientExecutePDU cepdu;
                cepdu.receive(chunk);

                if (this->verbose) {
                    cepdu.log(LOG_INFO);
                }

                const char* exe_of_file = cepdu.get_windows_execute_shell_params().exe_or_file.c_str();

                if (0 != ::strcasecmp(exe_of_file, DUMMY_REMOTEAPP)
                 && exe_of_file != ::strcasestr(exe_of_file, DUMMY_REMOTEAPP ":")
                ) {
                    this->windows_execute_shell_params = cepdu.get_windows_execute_shell_params();
                }
                this->should_ignore_first_client_execute_ = false;
            }
        break;

        case TS_RAIL_ORDER_GET_APPID_REQ:
            LOG_IF(this->verbose, LOG_INFO, "ClientExecute::send_to_mod_rail_channel:Client Get Application ID PDU");

            if (this->channel_){
                check_is_unit_throw(length, flags, chunk, "ApplicationIdPDU", this->verbose);
                this->process_client_get_application_id_pdu(chunk);
            }
        break;

        case TS_RAIL_ORDER_HANDSHAKE:
            LOG_IF(this->verbose, LOG_INFO, "ClientExecute::send_to_mod_rail_channel:Client Handshake PDU");

            if (this->channel_) {
                check_is_unit_throw(length, flags, chunk, "ProcessClientHandshakePDU", this->verbose);

                HandshakePDU hspdu;
                hspdu.receive(chunk);

                if (this->verbose) { hspdu.log(LOG_INFO); }
            }
        break;

        //case TS_RAIL_ORDER_LANGBARINFO:
        //    if (this->verbose) {
        //        LOG(LOG_INFO,
        //            "ClientExecute::send_to_mod_rail_channel: "
        //                "Client Language Bar Information PDU");
        //    }
        //
        //    this->process_client_language_bar_information_pdu(
        //        length, flags, chunk);
        //break;

        //case TS_RAIL_ORDER_LANGUAGEIMEINFO:
        //    if (this->verbose) {
        //        LOG(LOG_INFO,
        //            "ClientExecute::send_to_mod_rail_channel: "
        //                "Client Language Profile Information PDU");
        //    }
        //
        //    this->process_client_language_profile_information_pdu(
        //        length, flags, chunk);
        //break;

        //case TS_RAIL_ORDER_NOTIFY_EVENT:
        //    if (this->verbose) {
        //        LOG(LOG_INFO,
        //            "ClientExecute::send_to_mod_rail_channel: "
        //                "Client Notify Event PDU");
        //    }
        //
        //    this->process_client_notify_event_pdu(
        //        length, flags, chunk);
        //break;

        case TS_RAIL_ORDER_SYSCOMMAND:
            LOG_IF(this->verbose, LOG_INFO,
                "ClientExecute::send_to_mod_rail_channel:Client System Command PDU");
            check_is_unit_throw(length, flags, chunk, "ProcessClientSystemCommandPDU", this->verbose);
            this->process_client_system_command_pdu(chunk);
        break;

        case TS_RAIL_ORDER_SYSPARAM:
            LOG_IF(this->verbose, LOG_INFO,
                "ClientExecute::send_to_mod_rail_channel:Client System Parameters Update PDU");

            if (this->channel_) {
                check_is_unit_throw(length, flags, chunk, "ProcessClientSystemParametersUpdatePDU", this->verbose);
                this->process_client_system_parameters_update_pdu(chunk);
            }
        break;

        //case TS_RAIL_ORDER_SYSMENU:
        //    if (this->verbose) {
        //        LOG(LOG_INFO,
        //            "ClientExecute::send_to_mod_rail_channel: "
        //                "Client System Menu PDU");
        //    }
        //
        //    this->process_client_system_menu_pdu(
        //        length, flags, chunk);
        //break;

        case TS_RAIL_ORDER_WINDOWMOVE:
            LOG_IF(this->verbose, LOG_INFO,
                "ClientExecute::send_to_mod_rail_channel:Client Window Move PDU");
            check_is_unit_throw(length, flags, chunk, "ProcessClientWindowMovePDU", this->verbose);
            this->process_client_window_move_pdu(chunk);
        break;

        default:
            LOG_IF(this->verbose, LOG_INFO,
                "ClientExecute::send_to_mod_rail_channel: Delivering unprocessed messages"
                " %s(%u) to server.",
                get_RAIL_orderType_name(this->client_order_type),
                static_cast<unsigned>(this->client_order_type));
        break;
    }   // switch (this->client_order_type)
}   // send_to_mod_rail_channel

// TS_RAIL_ORDER_ACTIVATE
void ClientExecute::process_client_activate_pdu(InStream& chunk)
{
    ClientActivatePDU capdu;
    capdu.receive(chunk);
    if (this->verbose) { capdu.log(LOG_INFO); }

    if ((capdu.WindowId() == INTERNAL_MODULE_WINDOW_ID) && (capdu.Enabled() == 0)) {
        send_activate_window(RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ACTIVEWND,
            this->drawable_, this->verbose);
        send_activate_window(RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ZORDER,
            this->drawable_, this->verbose);
   }
}   // process_client_activate_pdu

// TS_RAIL_ORDER_CLIENTSTATUS

// TS_RAIL_ORDER_COMPARTMENTINFO

// TS_RAIL_ORDER_CLOAK

// TS_RAIL_ORDER_EXEC

// TS_RAIL_ORDER_GET_APPID_REQ

void ClientExecute::process_client_get_application_id_pdu(InStream& chunk)
{
    ClientGetApplicationIDPDU cgaipdu;
    cgaipdu.receive(chunk);

    if (this->verbose) {
        cgaipdu.log(LOG_INFO);
    }

    send_to_channel(
        this->front_, this->channel_, this->verbose, "process_client_get_application_id_pdu",
        "Server Get Application ID Response PDU",
        TS_RAIL_ORDER_GET_APPID_RESP, [this]{
            ServerGetApplicationIDResponsePDU server_get_application_id_response_pdu;
            server_get_application_id_response_pdu.WindowId(INTERNAL_MODULE_WINDOW_ID);
            server_get_application_id_response_pdu.ApplicationId(this->window_title);
            return server_get_application_id_response_pdu;
        });

}   // process_client_get_application_id_pdu

// TS_RAIL_ORDER_HANDSHAKE

// TS_RAIL_ORDER_LANGBARINFO:

// TS_RAIL_ORDER_LANGUAGEIMEINFO:

// TS_RAIL_ORDER_NOTIFY_EVENT:

// TS_RAIL_ORDER_SYSCOMMAND:

void ClientExecute::process_client_system_command_pdu(InStream& chunk)
{
    ClientSystemCommandPDU cscpdu;
    cscpdu.receive(chunk);

    if (this->verbose) { cscpdu.log(LOG_INFO); }

    switch (cscpdu.Command()) {
        case SC_CLOSE:
            LOG(LOG_INFO, "ClientExecute::process_client_system_command_pdu: Close by user (System Command)");
            throw Error(ERR_WIDGET);    // F4 key pressed
        case SC_MINIMIZE:
            {
                {
                    RDP::RAIL::ActivelyMonitoredDesktop order;

                    order.header.FieldsPresentFlags(
                            RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP |
                            RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ACTIVEWND
                        );

                    order.ActiveWindowId(0xFFFFFFFF);

                    if (this->verbose) { order.log(LOG_INFO); }

                    this->drawable_.draw(order);
                }

                {
                    RDP::RAIL::NewOrExistingWindow order = create_minimized_window_pdu(
                        this->window_level_supported_ex);

                    log_new_or_existing_window(order, this->verbose, "process_client_system_command_pdu");

                    this->drawable_.draw(order);
                    this->on_delete_window();
                }

                refresh_window_rect(this->mod_, this->window_rect);
            }
            break;

        case SC_RESTORE:
            {
                const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset);

                RDP::RAIL::NewOrExistingWindow order = create_window_with_title_pdu(
                    0,
                    this->window_title,
                    adjusted_window_rect);

                log_new_or_existing_window(order, this->verbose, "process_client_system_command_pdu");

                this->drawable_.draw(order);
                this->on_new_or_existing_window(adjusted_window_rect);

                refresh_window_rect(this->mod_, this->window_rect);
            }
            break;
    }
}   // process_client_system_command_pdu

// TS_RAIL_ORDER_SYSPARAM:

void ClientExecute::process_client_system_parameters_update_pdu(InStream& chunk)
{
    ClientSystemParametersUpdatePDU cspupdu;
    cspupdu.receive(chunk);

    if (this->verbose) {
        cspupdu.log(LOG_INFO);
    }

    if (cspupdu.SystemParam() == SPI_SETWORKAREA) {
        if (this->work_area_count >= max_work_area) {
            LOG(LOG_ERR, "ClientExecute::process_client_system_parameters_update_pdu: work_area_count overflow");
            throw Error(ERR_WIDGET);
        }

        RDP::RAIL::Rectangle const & body_r = cspupdu.body_r();

        Rect& work_area = this->work_areas[this->work_area_count];

        work_area.x  = body_r.iLeft();
        work_area.y  = body_r.iTop();
        work_area.cx = body_r.eRight() - body_r.iLeft();
        work_area.cy = body_r.eBottom() - body_r.iTop();

        LOG_IF(this->verbose, LOG_INFO, "WorkArea%s", work_area);

        this->virtual_screen_rect = this->virtual_screen_rect.disjunct(work_area);

        this->work_area_count++;

        {
            RDP::RAIL::ActivelyMonitoredDesktop order;

            order.header.FieldsPresentFlags(
                    RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP |
                    RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ACTIVEWND |
                    RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ZORDER |
                    RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_HOOKED |
                    RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ARC_BEGAN
                );

            order.ActiveWindowId(0xFFFFFFFF);
            order.NumWindowIds(0);

            if (this->verbose) { order.log(LOG_INFO); }

            this->drawable_.draw(order);
        }

        {
            const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset);

            RDP::RAIL::NewOrExistingWindow order = create_window_with_title_pdu(
                RDP::RAIL::WINDOW_ORDER_STATE_NEW,
                this->window_title,
                adjusted_window_rect);

            order.ShowState(this->maximized_state == MaximizedState::FullScreen ? 3 : 5);

            log_new_or_existing_window(order, this->verbose, "process_client_system_parameters_update_pdu");

            this->drawable_.draw(order);
            this->on_new_or_existing_window(adjusted_window_rect);
        }

        {
            RDP::RAIL::ActivelyMonitoredDesktop order;

            order.header.FieldsPresentFlags(
                    RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP |
                    RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ARC_COMPLETED
                );

            if (this->verbose) { order.log(LOG_INFO); }

            this->drawable_.draw(order);
        }

        {
            RDP::RAIL::ActivelyMonitoredDesktop order;

            order.header.FieldsPresentFlags(
                        RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP
                    | RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ACTIVEWND
                );

            order.ActiveWindowId(INTERNAL_MODULE_WINDOW_ID);
            order.NumWindowIds(0);

            if (this->verbose) { order.log(LOG_INFO); }

            this->drawable_.draw(order);
        }

        {
            RDP::RAIL::ActivelyMonitoredDesktop order;

            order.header.FieldsPresentFlags(
                        RDP::RAIL::WINDOW_ORDER_TYPE_DESKTOP
                    | RDP::RAIL::WINDOW_ORDER_FIELD_DESKTOP_ZORDER
                );

            order.NumWindowIds(1);
            order.window_ids(0, INTERNAL_MODULE_WINDOW_ID);

            if (this->verbose) { order.log(LOG_INFO); }

            this->drawable_.draw(order);
        }

        this->internal_module_window_created = true;

        {
            RDP::RAIL::WindowIcon order;

            order.header.FieldsPresentFlags(
                uint32_t(RDP::RAIL::WINDOW_ORDER_ICON)
              | uint32_t(RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW)
              | uint32_t(RDP::RAIL::WINDOW_ORDER_FIELD_ICON_BIG)
            );
            order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

            order.icon_info.CacheEntry(65535);
            order.icon_info.CacheId(255);
            order.icon_info.Bpp(16);
            order.icon_info.Width(32);
            order.icon_info.Height(32);

            static uint8_t const BitsMask[] = {
/* 0000 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //................
/* 0010 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //................
/* 0020 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //................
/* 0030 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //................
/* 0040 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //................
/* 0050 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //................
/* 0060 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //................
/* 0070 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  //................
                };
            order.icon_info.BitsMask(BitsMask, sizeof(BitsMask));

            static uint8_t const BitsColor[] = {
/* 0000 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0010 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0020 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0030 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0040 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0050 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0060 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0070 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xe5, 0x75, 0xc3, 0x75, //.u.u.u.u.u.u.u.u
/* 0080 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0090 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 00a0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 00b0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa2, 0x75, 0x06, 0x7a, 0xf2, 0x7a, 0xfe, 0x7f, 0x27, 0x7a, //.u.u.u.u.z.z..'z
/* 00c0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 00d0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 00e0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 00f0 */ 0xa1, 0x75, 0xa1, 0x75, 0x49, 0x7a, 0x35, 0x7f, 0xdd, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x27, 0x7a, //.u.uIz5.......'z
/* 0100 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0110 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0120 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xc3, 0x75, //.u.u.u.u.u.u.u.u
/* 0130 */ 0x28, 0x7a, 0x57, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xfe, 0x7f, 0x07, 0x7a, //(zW............z
/* 0140 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0150 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0160 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa2, 0x75, 0x6c, 0x7a, 0x57, 0x7f, //.u.u.u.u.u.ulzW.
/* 0170 */ 0xfe, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xdd, 0x7f, 0x13, 0x7b, 0x29, 0x7a, 0xa2, 0x75, //...........{)z.u
/* 0180 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0190 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 01a0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xc3, 0x75, 0x6b, 0x7a, 0xbb, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.ukz......
/* 01b0 */ 0xff, 0x7f, 0xff, 0x7f, 0xbb, 0x7f, 0xf1, 0x7a, 0xc4, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.......z.u.u.u.u
/* 01c0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 01d0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 01e0 */ 0xa1, 0x75, 0xc4, 0x75, 0xd0, 0x7a, 0x9a, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.z..........
/* 01f0 */ 0x36, 0x7f, 0x6b, 0x7a, 0xe4, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //6.kz.u.u.u.u.u.u
/* 0200 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0210 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa2, 0x75, 0xe5, 0x75, //.u.u.u.u.u.u.u.u
/* 0220 */ 0xd0, 0x7a, 0xde, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xdd, 0x7f, 0x07, 0x7a, //.z.............z
/* 0230 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0240 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0250 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x06, 0x7a, 0x14, 0x7f, 0xbc, 0x7f, //.u.u.u.u.u.z....
/* 0260 */ 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x99, 0x7f, 0xdd, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xae, 0x7a, //...............z
/* 0270 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0280 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0290 */ 0xa1, 0x75, 0xa1, 0x75, 0xa2, 0x75, 0xe6, 0x75, 0x14, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u........
/* 02a0 */ 0xff, 0x7f, 0xbb, 0x7f, 0x8e, 0x7a, 0xa2, 0x75, 0xf2, 0x7a, 0xff, 0x7f, 0xff, 0x7f, 0xbc, 0x7f, //.....z.u.z......
/* 02b0 */ 0xa2, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 02c0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 02d0 */ 0xa2, 0x75, 0x4a, 0x7a, 0x56, 0x7f, 0xde, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x9a, 0x7f, //.uJzV...........
/* 02e0 */ 0x8c, 0x7a, 0xc4, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x4a, 0x7a, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.z.u.u.uJz......
/* 02f0 */ 0x28, 0x7a, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //(z.u.u.u.u.u.u.u
/* 0300 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xc3, 0x75, 0x28, 0x7a, //.u.u.u.u.u.u.u(z
/* 0310 */ 0x78, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xbc, 0x7f, 0xaf, 0x7a, 0xa2, 0x75, //x............z.u
/* 0320 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xe5, 0x75, 0xdc, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.u......
/* 0330 */ 0xd0, 0x7a, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.z.u.u.u.u.u.u.u
/* 0340 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xc3, 0x75, 0xaf, 0x7a, 0x79, 0x7f, 0xff, 0x7f, //.u.u.u.u.u.zy...
/* 0350 */ 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xbb, 0x7f, 0x8d, 0x7a, 0xe4, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.........z.u.u.u
/* 0360 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xc2, 0x75, 0x9a, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.u......
/* 0370 */ 0x35, 0x7f, 0xa2, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //5..u.u.u.u.u.u.u
/* 0380 */ 0xa1, 0x75, 0xa1, 0x75, 0xc4, 0x75, 0x8d, 0x7a, 0xbc, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.z........
/* 0390 */ 0xff, 0x7f, 0xbc, 0x7f, 0xd0, 0x7a, 0xa2, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.....z.u.u.u.u.u
/* 03a0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x57, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.uW.....
/* 03b0 */ 0x78, 0x7f, 0xc2, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //x..u.u.u.u.u.u.u
/* 03c0 */ 0xe4, 0x75, 0xf2, 0x7a, 0x9b, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xbc, 0x7f, //.u.z............
/* 03d0 */ 0xae, 0x7a, 0xe4, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.z.u.u.u.u.u.u.u
/* 03e0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x14, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.u......
/* 03f0 */ 0x99, 0x7f, 0xc3, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //...u.u.u.u.u.u.u
/* 0400 */ 0x6b, 0x7a, 0xdd, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xd0, 0x7a, //kz.............z
/* 0410 */ 0xe4, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0420 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x13, 0x7b, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.u.{....
/* 0430 */ 0x99, 0x7f, 0xc3, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //...u.u.u.u.u.u.u
/* 0440 */ 0xa2, 0x75, 0x07, 0x7a, 0xd0, 0x7a, 0xdd, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.z.z..........
/* 0450 */ 0xbc, 0x7f, 0x13, 0x7f, 0x06, 0x7a, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.....z.u.u.u.u.u
/* 0460 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x56, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.uV.....
/* 0470 */ 0x79, 0x7f, 0xc3, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //y..u.u.u.u.u.u.u
/* 0480 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xc3, 0x75, 0xd1, 0x7a, 0xbc, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.z......
/* 0490 */ 0xff, 0x7f, 0xff, 0x7f, 0xfe, 0x7f, 0x13, 0x7f, 0x06, 0x7a, 0xa2, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.........z.u.u.u
/* 04a0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa2, 0x75, 0x99, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.u......
/* 04b0 */ 0x56, 0x7f, 0xa2, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //V..u.u.u.u.u.u.u
/* 04c0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xe4, 0x75, 0x6c, 0x7a, 0x99, 0x7f, //.u.u.u.u.u.ulz..
/* 04d0 */ 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xdd, 0x7f, 0x36, 0x7f, 0x49, 0x7a, 0xa1, 0x75, //..........6.Iz.u
/* 04e0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xe4, 0x75, 0xbc, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.u.u.u......
/* 04f0 */ 0x13, 0x7f, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //...u.u.u.u.u.u.u
/* 0500 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0510 */ 0x4b, 0x7a, 0x99, 0x7f, 0xfe, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x57, 0x7f, //Kz............W.
/* 0520 */ 0x27, 0x7a, 0xc2, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x49, 0x7a, 0xfe, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //'z.u.u.uIz......
/* 0530 */ 0x6b, 0x7a, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //kz.u.u.u.u.u.u.u
/* 0540 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0550 */ 0xa1, 0x75, 0xc3, 0x75, 0x49, 0x7a, 0x34, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.uIz4.........
/* 0560 */ 0xfe, 0x7f, 0x57, 0x7f, 0x6c, 0x7a, 0xa2, 0x75, 0xf2, 0x7a, 0xff, 0x7f, 0xff, 0x7f, 0xdd, 0x7f, //..W.lz.u.z......
/* 0570 */ 0xc3, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0580 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0590 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x06, 0x7a, 0x56, 0x7f, 0xdd, 0x7f, 0xff, 0x7f, //.u.u.u.u.zV.....
/* 05a0 */ 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x9a, 0x7f, 0xfe, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xf1, 0x7a, //...............z
/* 05b0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 05c0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 05d0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x07, 0x7a, 0xd0, 0x7a, //.u.u.u.u.u.u.z.z
/* 05e0 */ 0xdc, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xde, 0x7f, 0x07, 0x7a, //...............z
/* 05f0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0600 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0610 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0620 */ 0xc3, 0x75, 0xd0, 0x7a, 0xbb, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xde, 0x7f, //.u.z............
/* 0630 */ 0xd1, 0x7a, 0xe5, 0x75, 0xa2, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.z.u.u.u.u.u.u.u
/* 0640 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0650 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0660 */ 0xa1, 0x75, 0xa1, 0x75, 0xc4, 0x75, 0x6b, 0x7a, 0x78, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, //.u.u.ukzx.......
/* 0670 */ 0xff, 0x7f, 0xdd, 0x7f, 0x35, 0x7f, 0x49, 0x7a, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //....5.Iz.u.u.u.u
/* 0680 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0690 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 06a0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0x4a, 0x7a, 0x79, 0x7f, 0xfe, 0x7f, //.u.u.u.u.uJzy...
/* 06b0 */ 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x78, 0x7f, 0x29, 0x7a, 0xc3, 0x75, 0xa1, 0x75, //........x.)z.u.u
/* 06c0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 06d0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 06e0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xc2, 0x75, 0x28, 0x7a, //.u.u.u.u.u.u.u(z
/* 06f0 */ 0x13, 0x7f, 0xfe, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x79, 0x7f, 0xe5, 0x75, //............y..u
/* 0700 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0710 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0720 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0730 */ 0xa1, 0x75, 0xe5, 0x75, 0x35, 0x7f, 0xdd, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0xff, 0x7f, 0x27, 0x7a, //.u.u5.........'z
/* 0740 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0750 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0760 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0770 */ 0xa1, 0x75, 0xa1, 0x75, 0xa2, 0x75, 0x06, 0x7a, 0xaf, 0x7a, 0xbc, 0x7f, 0xff, 0x7f, 0x27, 0x7a, //.u.u.u.z.z....'z
/* 0780 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 0790 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 07a0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 07b0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa2, 0x75, 0xae, 0x7a, 0x06, 0x7a, //.u.u.u.u.u.u.z.z
/* 07c0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 07d0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 07e0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, //.u.u.u.u.u.u.u.u
/* 07f0 */ 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75, 0xa1, 0x75  //.u.u.u.u.u.u.u.u
                };
            order.icon_info.BitsColor(BitsColor, sizeof(BitsColor));

            if (this->verbose) { order.log(LOG_INFO); }

            this->drawable_.draw(order);

            order.header.FieldsPresentFlags(
                uint32_t(RDP::RAIL::WINDOW_ORDER_ICON)
              | uint32_t(RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW)
            );

            if (this->verbose) { order.log(LOG_INFO); }

            this->drawable_.draw(order);
        }

        refresh_window_rect(this->mod_, this->window_rect);
    }   // if (cspupdu.SystemParam() == SPI_SETWORKAREA)
    else if (cspupdu.SystemParam() == RAIL_SPI_TASKBARPOS) {
        RDP::RAIL::Rectangle const & body_r = cspupdu.body_r();

        // task_bar_rect
        LOG_IF(this->verbose, LOG_INFO,
            "ClientExecute::process_client_system_parameters_update_pdu: TaskBarRect(%d, %d, %d, %d)",
            body_r.iLeft(), body_r.iTop(),
            body_r.eRight() - body_r.iLeft(), body_r.eBottom() - body_r.iTop());
    }   // else if (cspupdu.SystemParam() == RAIL_SPI_TASKBARPOS)
    else if (cspupdu.SystemParam() == SPI_SETDRAGFULLWINDOWS) {
        this->full_window_drag_enabled = (cspupdu.body_b() != 0);
        LOG_IF(this->verbose, LOG_INFO,
            "ClientExecute::process_client_system_parameters_update_pdu: Full Window Drag is %s",
            (this->full_window_drag_enabled ? "enabled" : "disabled"));
    }   // else if (cspupdu.SystemParam() == SPI_SETDRAGFULLWINDOWS)
}   // process_client_system_parameters_update_pdu

// TS_RAIL_ORDER_SYSMENU:

// TS_RAIL_ORDER_WINDOWMOVE:

void ClientExecute::process_client_window_move_pdu(InStream& chunk)
{
    ClientWindowMovePDU cwmpdu;
    cwmpdu.receive(chunk);

    if (this->verbose) { cwmpdu.log(LOG_INFO); }

    if (INTERNAL_MODULE_WINDOW_ID == cwmpdu.WindowId()) {

        this->window_rect = Rect(cwmpdu.iLeft() - this->window_offset.x,
                                 cwmpdu.iTop() - this->window_offset.y,
                                 cwmpdu.eRight() - cwmpdu.iLeft(),
                                 cwmpdu.eBottom() - cwmpdu.iTop());

        this->update_rects();

        const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset);

        {
            RDP::RAIL::NewOrExistingWindow order = create_move_resize_window_pdu(
                this->window_level_supported_ex,
                adjusted_window_rect);

            log_new_or_existing_window(order, this->verbose, "process_client_window_move_pdu");

            this->drawable_.draw(order);
        }

        uint16_t const move_size_type = area_to_rail_wmsz(
            this->mouse_action.pressed_mouse_button());
        if (move_size_type) {
            send_to_channel(
                this->front_, this->channel_, this->verbose, "process_client_window_move_pdu",
                "Server Move/Size End PDU",
                TS_RAIL_ORDER_LOCALMOVESIZE, [&]{
                    ServerMoveSizeStartOrEndPDU smssoepdu;

                    const Rect adjusted_window_rect = this->window_rect.offset(
                        this->window_offset.x, this->window_offset.y);

                    smssoepdu.WindowId(INTERNAL_MODULE_WINDOW_ID);
                    smssoepdu.IsMoveSizeStart(0);
                    smssoepdu.MoveSizeType(move_size_type);
                    smssoepdu.PosXOrTopLeftX(adjusted_window_rect.x);
                    smssoepdu.PosYOrTopLeftY(adjusted_window_rect.y);

                    return smssoepdu;
                });

            this->move_size_initialized = false;
        }

        this->update_widget();
        this->on_new_or_existing_window(adjusted_window_rect);
    }
}   // process_client_window_move_pdu

Rect ClientExecute::move_resize_rect(
    WindowArea pressed_button,
    int original_offset_x, int original_offset_y,
    Rect const& r
) noexcept
{
    int offset_x = 0;
    int offset_y = 0;
    int offset_cx = 0;
    int offset_cy = 0;

    switch (pressed_button) {
        case WindowArea::N: {
            const int offset_y_max = r.cy - MINIMUM_WINDOW_HEIGHT;
            offset_y = std::min(original_offset_x, offset_y_max);
            offset_cy = -offset_y;
            break;
        }

        case WindowArea::NWN:
        case WindowArea::NWW: {
            const int offset_x_max = r.cx - MINIMUM_WINDOW_WIDTH;
            const int offset_y_max = r.cy - MINIMUM_WINDOW_HEIGHT;

            offset_x = std::min(original_offset_x, offset_x_max);
            offset_cx = -offset_x;

            offset_y = std::min(original_offset_y, offset_y_max);
            offset_cy = -offset_y;
            break;
        }

        case WindowArea::W: {
            const int offset_x_max = r.cx - MINIMUM_WINDOW_WIDTH;
            offset_x = std::min(original_offset_x, offset_x_max);
            offset_cx = -offset_x;
            break;
        }

        case WindowArea::SWW:
        case WindowArea::SWS: {
            const int offset_x_max = r.cx - MINIMUM_WINDOW_WIDTH;

            offset_x = std::min(original_offset_x, offset_x_max);
            offset_cx = -offset_x;

            const int offset_cy_min = MINIMUM_WINDOW_HEIGHT - r.cy;

            offset_cy = std::max(original_offset_y, offset_cy_min);
            break;
        }

        case WindowArea::S: {
            const int offset_cy_min = MINIMUM_WINDOW_HEIGHT - r.cy;
            offset_cy = std::max(original_offset_y, offset_cy_min);
            break;
        }

        case WindowArea::SES:
        case WindowArea::SEE: {
            const int offset_cy_min = MINIMUM_WINDOW_HEIGHT - r.cy;
            const int offset_cx_min = MINIMUM_WINDOW_WIDTH - r.cx;

            offset_cy = std::max(original_offset_y, offset_cy_min);
            offset_cx = std::max(original_offset_x, offset_cx_min);
            break;
        }

        case WindowArea::E: {
            const int offset_cx_min = MINIMUM_WINDOW_WIDTH - r.cx;
            offset_cx = std::max(original_offset_x, offset_cx_min);
            break;
        }

        case WindowArea::NEE:
        case WindowArea::NEN: {
            const int offset_y_max = r.cy - MINIMUM_WINDOW_HEIGHT;
            const int offset_cx_min = MINIMUM_WINDOW_WIDTH - r.cx;

            offset_y = std::min(original_offset_y, offset_y_max);
            offset_cy = -offset_y;

            offset_cx = std::max(original_offset_x, offset_cx_min);
            break;
        }

        case WindowArea::Title: {
            const int offset_y_max = r.cy - MINIMUM_WINDOW_HEIGHT;
            const int offset_x_max = r.cx - MINIMUM_WINDOW_WIDTH;

            offset_x = std::min(original_offset_x, offset_x_max);
            offset_y = std::min(original_offset_y, offset_y_max);
            break;
        }

        case WindowArea::Mini:
        case WindowArea::Maxi:
        case WindowArea::Close:
        case WindowArea::Resize:
        case WindowArea::Icon:
        case WindowArea::NUMBER_OF_AREAS_OR_INVALID:
            REDEMPTION_UNREACHABLE();
    }

    return Rect(checked_int(r.x + offset_x),
                checked_int(r.y + offset_y),
                checked_int(r.cx + offset_cx),
                checked_int(r.cy + offset_cy));
}

enum class ClientExecute::MouseAction::State : uint8_t
{
    Default,
    WaitRelease,
    CapturedClick,
    MoveSizeInitialized,
    DelayedStartMoveSize,
    DelayedMaximazeVertical,
    DelayedClose,
    WaitDoubleClick,
};

inline constexpr MonotonicTimePoint::duration double_click_delay = std::chrono::milliseconds(400);

ClientExecute::MouseAction::ActionResult ClientExecute::MouseAction::next_mouse_action(
    Rect const& window_rect, IsMaximized is_maximized, ResizableHosted resizable_hosted,
    MonotonicTimePoint now, uint16_t flags, uint16_t x, uint16_t y)
{
    RectZone rect_area = get_rect_area(window_rect, bool(is_maximized), bool(resizable_hosted),
                                       checked_int(x), checked_int(y));

    return ActionResult{
        rect_area.rect,
        rect_area.area,
        this->_next_action(rect_area.area, now, flags, x, y),
    };
}

void ClientExecute::MouseAction::remove_move_resize_state() noexcept
{
    this->state = State::CapturedClick;
}

inline ClientExecute::MouseAction::EventAction ClientExecute::MouseAction::_next_action(
    WindowArea area, MonotonicTimePoint now,
    uint16_t flags, uint16_t x, uint16_t y) noexcept
{
    if (SlowPath::PTRFLAGS_MOVE == flags)
    {
        switch (this->state)
        {
            case State::MoveSizeInitialized:
                return EventAction::MoveResize;

            case State::DelayedStartMoveSize:
            case State::DelayedMaximazeVertical:
                this->state = State::MoveSizeInitialized;
                return EventAction::StartMoveResize;

            case State::DelayedClose:
                this->state = State::CapturedClick;
                return EventAction::CapturedClick;

            case State::WaitRelease:
            case State::CapturedClick:
                return EventAction::CapturedClick;

            case State::WaitDoubleClick:
                this->state = State::Default;
                return EventAction::Nothing;

            case State::Default:
                return EventAction::Nothing;
        }

        REDEMPTION_UNREACHABLE();
    }

    // left click down
    else if ((SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1) == flags)
    {
        auto old_area = this->pressed_area;
        this->pressed_area = area;

        auto delayed_action = [&](State state, EventAction db_action, State db_state){
            if (this->state == State::WaitDoubleClick
             && old_area == area
             && now - this->button_down_timepoint <= double_click_delay)
            {
                this->state = db_state;
                this->pressed_area = WindowArea::NUMBER_OF_AREAS_OR_INVALID;
                return db_action;
            }
            this->state = state;
            this->button_down_timepoint = now;
            return EventAction::Nothing;
        };

        switch (area) {
            // double click timer for close
            case WindowArea::Icon:
                return delayed_action(State::DelayedClose,
                                      EventAction::Close, State::CapturedClick);

            // move or double click timer for mazimize
            case WindowArea::Title:
                this->safe_button_down_x = x;
                this->safe_button_down_y = y;
                return delayed_action(State::DelayedStartMoveSize,
                                      EventAction::Maximaze, State::Default);

            // vertical resize or double click timer
            case WindowArea::S:
            case WindowArea::N:
                this->safe_button_down_x = x;
                this->safe_button_down_y = y;
                return delayed_action(State::DelayedMaximazeVertical,
                                      EventAction::MaximazeVertical, State::Default);

            // resize
            case WindowArea::NWN:
            case WindowArea::NWW:
            case WindowArea::W:
            case WindowArea::SWW:
            case WindowArea::SWS:
            case WindowArea::SES:
            case WindowArea::SEE:
            case WindowArea::E:
            case WindowArea::NEE:
            case WindowArea::NEN:
                this->state = State::DelayedStartMoveSize;
                this->safe_button_down_x = x;
                this->safe_button_down_y = y;
                return EventAction::Nothing;

            // buttons
            case WindowArea::Close:
            case WindowArea::Maxi:
            case WindowArea::Mini:
            case WindowArea::Resize:
                this->state = State::WaitRelease;
                return EventAction::ActiveButton;

            case WindowArea::NUMBER_OF_AREAS_OR_INVALID:
                return EventAction::Nothing;
        }

        REDEMPTION_UNREACHABLE();
    }

    // left click release
    else if (SlowPath::PTRFLAGS_BUTTON1 == flags) {
        auto current_pressed_area = this->pressed_area;
        this->pressed_area = WindowArea::NUMBER_OF_AREAS_OR_INVALID;

        if (this->state == State::MoveSizeInitialized) {
            this->state = State::Default;
            return EventAction::StopMoveResize;
        }

        auto ret = (State::WaitRelease == this->state)
            ? EventAction::UnactiveButton
            : EventAction::Nothing;

        auto button_action = [&](EventAction action){
            this->state = State::Default;
            return (current_pressed_area == area) ? action : ret;
        };

        switch (area) {
            case WindowArea::NWN:
            case WindowArea::NWW:
            case WindowArea::W:
            case WindowArea::SWW:
            case WindowArea::SWS:
            case WindowArea::SES:
            case WindowArea::SEE:
            case WindowArea::E:
            case WindowArea::NEE:
            case WindowArea::NEN:
                this->state = State::Default;
                return ret;

            // wait double click or move flag
            case WindowArea::Title:
            case WindowArea::S:
            case WindowArea::N:
            case WindowArea::Icon:
                this->state = State::WaitDoubleClick;
                this->pressed_area = current_pressed_area;
                return ret;

            case WindowArea::Close:
                return button_action(EventAction::Close);

            case WindowArea::Maxi:
                return button_action(EventAction::Maximaze);

            case WindowArea::Mini:
                return button_action(EventAction::Minimize);

            case WindowArea::Resize:
                return button_action(EventAction::ResizeHostedDesktop);

            case WindowArea::NUMBER_OF_AREAS_OR_INVALID:
                return EventAction::Nothing;
        }

        REDEMPTION_UNREACHABLE();
    }

    return EventAction::Nothing;
}
