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
#include "utils/sugar/algostring.hpp"
#include "mod/rdp/channels/virtual_channel_data_sender.hpp" // msgdump_c
#include "mod/rdp/channels/rail_window_id_manager.hpp"      // TODO only for RemoteProgramsWindowIdManager::INVALID_WINDOW_ID
#include "mod/internal/mouse_state.hpp"


#define INTERNAL_MODULE_WINDOW_ID    40000
#define INTERNAL_MODULE_WINDOW_TITLE "Wallix AdminBastion"

#define AUXILIARY_WINDOW_ID          40001

#define TITLE_BAR_HEIGHT       24

#define INTERNAL_MODULE_MINIMUM_WINDOW_WIDTH  640
#define INTERNAL_MODULE_MINIMUM_WINDOW_HEIGHT 480

ClientExecute::ClientExecute(
    EventContainer& events,
    gdi::GraphicApi & drawable, FrontAPI & front,
    WindowListCaps const & window_list_caps, bool verbose)
: front_(front)
, drawable_(drawable)
, verbose(verbose)
, wallix_icon_min(bitmap_from_file(app_path(AppPath::WallixIconMin), BLACK))
, auxiliary_window_id(RemoteProgramsWindowIdManager::INVALID_WINDOW_ID)
, window_title(INTERNAL_MODULE_WINDOW_TITLE)
, window_level_supported_ex(window_list_caps.WndSupportLevel & TS_WINDOW_LEVEL_SUPPORTED_EX)
, events_guard(events)
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
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::adjust_rect(Rect %s)", rect);
    if (!this->rail_enabled) {
        return rect;
    }

    if (this->window_rect.isempty()) {
        this->window_rect = Rect(rect.x + rect.cx * 10 / 100,
                                 rect.y + rect.cy * 10 / 100,
                                 rect.cx * 80 / 100,
                                 rect.cy * 80 / 100);
        this->window_offset_x = -rect.x;
        this->window_offset_y = -rect.y;

        this->update_rects(this->allow_resize_hosted_desktop_);
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
            this->window_offset_x, this->window_offset_y);

        size_t current_surface_size = 0;
        Rect current_work_area = this->work_areas[0];
        for (unsigned int i = 0; i < this->work_area_count; ++i) {
            Rect intersect_rect = this->work_areas[i].intersect(adjusted_window_rect);
            if (!intersect_rect.isempty()) {
                size_t surface_size = intersect_rect.cx * intersect_rect.cy;
                if (current_surface_size < surface_size) {
                    current_surface_size = surface_size;
                    current_work_area = this->work_areas[i];
                }
            }
        }

        return current_work_area.offset(-this->window_offset_x, -this->window_offset_y);
    }

    return this->work_areas[0].offset(-this->window_offset_x, -this->window_offset_y);
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


void ClientExecute::draw_resize_hosted_desktop_box(bool mouse_over, const Rect r)
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::draw_resize_hosted_desktop_box()");
    RDPColor const bg_color = encode_color24()(BGRColor(mouse_over ? 0xCBCACA : 0xFFFFFF));

    auto const depth = gdi::ColorCtx::depth24();

    RDPOpaqueRect order(this->zone.get_zone(Zone::ZONE_RESIZE, this->window_rect), bg_color);

    this->drawable_.draw(order, r, depth);

    if (this->enable_resizing_hosted_desktop_) {
        Rect rect = this->zone.get_zone(Zone::ZONE_RESIZE, this->window_rect);

        rect.x  += 22;
        rect.y  += 8;
        rect.cx  = 2;
        rect.cy  = 7;

        {
            RDPOpaqueRect order(rect, encode_color24()(BLACK));

            this->drawable_.draw(order, r, depth);
        }

        rect.x  -= 4;
        rect.y  += 2;
        rect.cx  = 4;
        rect.cy  = 3;

        {
            RDPOpaqueRect order(rect, encode_color24()(BLACK));

            this->drawable_.draw(order, r, depth);
        }

        rect.x  -= 2;
        rect.y  -= 3;
        rect.cx  = 2;
        rect.cy  = 9;

        {
            RDPOpaqueRect order(rect, encode_color24()(BLACK));

            this->drawable_.draw(order, r, depth);
        }

        rect.x  -= 4;
        rect.y  += 4;
        rect.cx  = 4;
        rect.cy  = 1;

        {
            RDPOpaqueRect order(rect, encode_color24()(BLACK));

            this->drawable_.draw(order, r, depth);
        }
    }
    else {
        Rect rect = this->zone.get_zone(Zone::ZONE_RESIZE, this->window_rect);

        rect.x  += 15;
        rect.y  += 6;
        rect.cx  = 7;
        rect.cy  = 2;

        {
            RDPOpaqueRect order(rect, encode_color24()(BLACK));

            this->drawable_.draw(order, r, depth);
        }

        rect.x  += 2;
        rect.y  += 2;
        rect.cx  = 3;
        rect.cy  = 4;

        {
            RDPOpaqueRect order(rect, encode_color24()(BLACK));

            this->drawable_.draw(order, r, depth);
        }

        rect.x  -= 3;
        rect.y  += 4;
        rect.cx  = 9;
        rect.cy  = 2;

        {
            RDPOpaqueRect order(rect, encode_color24()(BLACK));

            this->drawable_.draw(order, r, depth);
        }

        rect.x  += 4;
        rect.y  += 2;
        rect.cx  = 1;
        rect.cy  = 4;

        {
            RDPOpaqueRect order(rect, encode_color24()(BLACK));

            this->drawable_.draw(order, r, depth);
        }
    }
}   // draw_resize_hosted_desktop_box

void ClientExecute::draw_maximize_box(bool mouse_over, const Rect r)
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::draw_maximize_box()");
    RDPColor const bg_color = encode_color24()(BGRColor(mouse_over ? 0xCBCACA : 0xFFFFFF));

    auto const depth = gdi::ColorCtx::depth24();

    auto rect_maxi = this->zone.get_zone(Zone::ZONE_MAXI, this->window_rect);
    RDPOpaqueRect order(rect_maxi, bg_color);

    this->drawable_.draw(order, r, depth);

    if (this->maximized) {
        Rect rect = rect_maxi;

        rect.x  += 14 + 2;
        rect.y  += 7;
        rect.cx -= 14 * 2 + 2;
        rect.cy -= 7 * 2 + 2;

        {
            RDPOpaqueRect order(rect, encode_color24()(BLACK));
            this->drawable_.draw(order, r, depth);
        }

        rect = rect.shrink(1);

        {
            RDPOpaqueRect order(rect, bg_color);
            this->drawable_.draw(order, r, depth);
        }

        rect = rect_maxi;

        rect.x  += 14;
        rect.y  += 7 + 2;
        rect.cx -= 14 * 2 + 2;
        rect.cy -= 7 * 2 + 2;

        {
            RDPOpaqueRect order(rect, encode_color24()(BLACK));
            this->drawable_.draw(order, r, depth);
        }

        rect = rect.shrink(1);

        {
            RDPOpaqueRect order(rect, bg_color);
            this->drawable_.draw(order, r, depth);
        }
    }
    else {
        Rect rect = rect_maxi;

        rect.x  += 14;
        rect.y  += 7;
        rect.cx -= 14 * 2;
        rect.cy -= 7 * 2;

        {
            RDPOpaqueRect order(rect, encode_color24()(BLACK));
            this->drawable_.draw(order, r, depth);
        }

        rect = rect.shrink(1);

        {
            RDPOpaqueRect order(rect, bg_color);
            this->drawable_.draw(order, r, depth);
        }
    }
}   // draw_maximize_box

void ClientExecute::input_invalidate(const Rect r)
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::input_invalidate(): r=%s", r);

    if (!this->channel_) return;

    auto const depth = gdi::ColorCtx::depth24();
    bool is_updated = false;

    if (auto icon_rect = this->zone.get_zone(Zone::ZONE_ICON, this->window_rect)
      ; r.has_intersection(icon_rect))
    {
        RDPOpaqueRect order(icon_rect, encode_color24()(WHITE));

        this->drawable_.draw(order, r, gdi::ColorCtx::depth24());

        this->drawable_.draw(
            RDPMemBlt(
                0,
                Rect(icon_rect.x + 3, icon_rect.y + 4, 16, 16),
                0xCC,
                0,
                0,
                0
            ),
            r,
            this->wallix_icon_min
        );

        is_updated = true;
    }

    if (auto title_rect = this->zone.get_zone(Zone::ZONE_TITLE, this->window_rect)
      ; r.has_intersection(title_rect))
    {
        RDPOpaqueRect order(title_rect, encode_color24()(WHITE));

        this->drawable_.draw(order, r, gdi::ColorCtx::depth24());

        if (this->font_) {
            gdi::server_draw_text(this->drawable_,
                                    *this->font_,
                                    title_rect.x + 1,
                                    title_rect.y + 3,
                                    this->window_title.c_str(),
                                    encode_color24()(BLACK),
                                    encode_color24()(WHITE),
                                    depth,
                                    r
                                    );
        }

        is_updated = true;
    }

    if (this->allow_resize_hosted_desktop_) {
        this->draw_resize_hosted_desktop_box(false, r);
    }

    if (auto rect_minimize = this->zone.get_zone(Zone::ZONE_MINI, this->window_rect)
      ; r.has_intersection(rect_minimize))
    {
        RDPOpaqueRect order(rect_minimize, encode_color24()(WHITE));

        this->drawable_.draw(order, r, gdi::ColorCtx::depth24());

        if (this->font_) {
            gdi::server_draw_text(this->drawable_,
                                    *this->font_,
                                    rect_minimize.x + 12,
                                    rect_minimize.y + 3,
                                    "−",
                                    encode_color24()(BLACK),
                                    encode_color24()(WHITE),
                                    depth,
                                    r
                                    );
        }

        is_updated = true;
    }

    this->draw_maximize_box(false, r);

    if (auto rect_close = this->zone.get_zone(Zone::ZONE_CLOSE, this->window_rect)
      ; r.has_intersection(rect_close))
    {
        RDPOpaqueRect order(rect_close, encode_color24()(WHITE));

        this->drawable_.draw(order, r, gdi::ColorCtx::depth24());

        if (this->font_) {
            gdi::server_draw_text(this->drawable_,
                                    *this->font_,
                                    rect_close.x + 13,
                                    rect_close.y + 3,
                                    "x",
                                    encode_color24()(BLACK),
                                    encode_color24()(WHITE),
                                    depth,
                                    r
                                    );
        }

        is_updated = true;
    }

    if (is_updated) {
        this->drawable_.sync();
    }
}   // input_invalidate

void ClientExecute::adjust_window_to_mod() {
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::adjust_window_to_mod()");
    this->maximized = false;
    Rect work_area_rect = this->get_current_work_area_rect();

    Dimension module_dimension;
    if (this->mod_) {
        module_dimension = this->mod_->get_dim();
    }

    Dimension prefered_window_dimension(module_dimension.w + 2, module_dimension.h + 2 + TITLE_BAR_HEIGHT);

    if (((this->window_rect.cx != prefered_window_dimension.w)||(this->window_rect.cy != prefered_window_dimension.h))
    && (work_area_rect.cx > prefered_window_dimension.w)
    && (work_area_rect.cy > prefered_window_dimension.h)) {
        this->window_rect.cx = prefered_window_dimension.w;
        this->window_rect.cy = prefered_window_dimension.h;
    }

    this->update_rects(this->allow_resize_hosted_desktop_);

    const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset_x, this->window_offset_y);

    {
        RDP::RAIL::NewOrExistingWindow order;

        order.header.FieldsPresentFlags(
                    RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                | (this->window_level_supported_ex ? RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREASIZE : 0)
                | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
                | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
                | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
                | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
                | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
                | RDP::RAIL::WINDOW_ORDER_FIELD_SHOW
                | RDP::RAIL::WINDOW_ORDER_FIELD_STYLE
            );
        order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

        order.ClientAreaWidth(adjusted_window_rect.cx - 6 * 2);
        order.ClientAreaHeight(adjusted_window_rect.cy - 25 - 6);
        order.WindowWidth(adjusted_window_rect.cx);
        order.WindowHeight(adjusted_window_rect.cy);
        order.NumVisibilityRects(1);
        order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, adjusted_window_rect.cx, adjusted_window_rect.cy));

        order.ClientOffsetX(adjusted_window_rect.x + 6);
        order.ClientOffsetY(adjusted_window_rect.y + 25);
        order.WindowOffsetX(adjusted_window_rect.x);
        order.WindowOffsetY(adjusted_window_rect.y);
        order.VisibleOffsetX(adjusted_window_rect.x);
        order.VisibleOffsetY(adjusted_window_rect.y);

        order.ShowState(5);
        order.Style(0x16CF0000);
        order.ExtendedStyle(0x110);

        if (this->verbose) {
            StaticOutStream<1024> out_s;
            order.emit(out_s);
            order.log(LOG_INFO);
            LOG(LOG_INFO, "ClientExecute::adjust_window_to_mod: Send NewOrExistingWindow to client: size=%zu (0)", out_s.get_offset() - 1);
        }

        this->drawable_.draw(order);
    }

    this->update_widget();

    this->on_new_or_existing_window(adjusted_window_rect);
}

void ClientExecute::maximize_restore_window()
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::maximize_restore_window()");
    if (this->maximized) {
        this->maximized = false;

        this->window_rect = this->window_rect_normal;

        Rect work_area_rect = this->get_current_work_area_rect();

        Dimension module_dimension;
        if (this->mod_) {
            module_dimension = this->mod_->get_dim();
        }

        Dimension prefered_window_dimension(
                module_dimension.w + 2,
                module_dimension.h + 2 + TITLE_BAR_HEIGHT
            );
        if (((this->window_rect.cx != prefered_window_dimension.w) ||
                (this->window_rect.cy != prefered_window_dimension.h)) &&
            (work_area_rect.cx > prefered_window_dimension.w) &&
            (work_area_rect.cy > prefered_window_dimension.h)) {
            this->window_rect.cx = prefered_window_dimension.w;
            this->window_rect.cy = prefered_window_dimension.h;
        }

        this->update_rects(this->allow_resize_hosted_desktop_);

        const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset_x, this->window_offset_y);

        {
            RDP::RAIL::NewOrExistingWindow order;

            order.header.FieldsPresentFlags(
                        RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                    | (this->window_level_supported_ex ? RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREASIZE : 0)
                    | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
                    | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
                    | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
                    | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
                    | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
                    | RDP::RAIL::WINDOW_ORDER_FIELD_SHOW
                    | RDP::RAIL::WINDOW_ORDER_FIELD_STYLE
                );
            order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

            order.ClientAreaWidth(adjusted_window_rect.cx - 6 * 2);
            order.ClientAreaHeight(adjusted_window_rect.cy - 25 - 6);
            order.WindowWidth(adjusted_window_rect.cx);
            order.WindowHeight(adjusted_window_rect.cy);
            order.NumVisibilityRects(1);
            order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, adjusted_window_rect.cx, adjusted_window_rect.cy));

            order.ClientOffsetX(adjusted_window_rect.x + 6);
            order.ClientOffsetY(adjusted_window_rect.y + 25);
            order.WindowOffsetX(adjusted_window_rect.x);
            order.WindowOffsetY(adjusted_window_rect.y);
            order.VisibleOffsetX(adjusted_window_rect.x);
            order.VisibleOffsetY(adjusted_window_rect.y);

            order.ShowState(5);
            order.Style(0x16CF0000);
            order.ExtendedStyle(0x110);

            if (this->verbose) {
                StaticOutStream<1024> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "ClientExecute::maximize_restore_window: Send NewOrExistingWindow to client: size=%zu (0)", out_s.get_offset() - 1);
            }

            this->drawable_.draw(order);
        }

        this->update_widget();

        this->on_new_or_existing_window(adjusted_window_rect);
    }   // if (this->maximized)
    else {
        this->maximized = true;

        this->window_rect_normal = this->window_rect;

        Rect work_area_rect = this->get_current_work_area_rect();

        this->window_rect = Rect(work_area_rect.x - 1,
                                 work_area_rect.y - 1,
                                 work_area_rect.cx + 1 * 2,
                                 work_area_rect.cy + 1 * 2);

        this->update_rects(this->allow_resize_hosted_desktop_);

        const Rect adjusted_window_rect = work_area_rect.offset(this->window_offset_x, this->window_offset_y);

        {
            RDP::RAIL::NewOrExistingWindow order;

            order.header.FieldsPresentFlags(
                        RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                    | (this->window_level_supported_ex ? RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREASIZE : 0)
                    | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
                    | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
                    | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
                    | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
                    | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
                    | RDP::RAIL::WINDOW_ORDER_FIELD_SHOW
                    | RDP::RAIL::WINDOW_ORDER_FIELD_STYLE
                );
            order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

            order.ClientAreaWidth(adjusted_window_rect.cx);
            order.ClientAreaHeight(adjusted_window_rect.cy - 25);
            order.WindowWidth(adjusted_window_rect.cx + 2);
            order.WindowHeight(adjusted_window_rect.cy + 2);
            order.NumVisibilityRects(1);
            order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, adjusted_window_rect.cx, adjusted_window_rect.cy + 1));

            order.ClientOffsetX(adjusted_window_rect.x/* + 0*/);
            order.ClientOffsetY(adjusted_window_rect.y + 25);
            order.WindowOffsetX(adjusted_window_rect.x + -1);
            order.WindowOffsetY(adjusted_window_rect.y + -1);
            order.VisibleOffsetX(adjusted_window_rect.x/* + 0*/);
            order.VisibleOffsetY(adjusted_window_rect.y/* + 0*/);

            order.ShowState(3);
            order.Style(0x17CF0000);
            order.ExtendedStyle(0x110);

            if (this->verbose) {
                StaticOutStream<1024> out_s;
                order.emit(out_s);
                order.log(LOG_INFO);
                LOG(LOG_INFO, "ClientExecute::maximize_restore_window: Send NewOrExistingWindow to client: size=%zu (1)", out_s.get_offset() - 1);
            }

            this->drawable_.draw(order);
        }

        this->update_widget();

        this->on_new_or_existing_window(adjusted_window_rect);
    }   // if (!this->maximized)
}   // maximize_restore_window

void ClientExecute::ready(mod_api & mod, uint16_t front_width, uint16_t front_height, Font const & font, bool allow_resize_hosted_desktop)
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::ready(%d,%d)", front_width, front_height);

    this->mod_  = &mod;
    this->font_ = &font;

    this->front_width  = front_width;
    this->front_height = front_height;

    this->allow_resize_hosted_desktop_    = allow_resize_hosted_desktop;
    this->enable_resizing_hosted_desktop_ = true;

    this->update_rects(this->allow_resize_hosted_desktop_);

    Rect rect = this->window_rect;
    rect.cy = TITLE_BAR_HEIGHT;

    this->input_invalidate(rect);

    if (this->channel_) {
        return;
    }

    this->channel_ = this->front_.get_channel_list().get_by_name(channel_names::rail);
    if (!this->channel_) return;

    {
        uint16_t const orderType = TS_RAIL_ORDER_HANDSHAKE/*TS_RAIL_ORDER_HANDSHAKE_EX*/;

        StaticOutStream<256> out_s;
        RAILPDUHeader header;
        header.emit_begin(out_s, orderType);

        HandshakePDU handshake_pdu;
        handshake_pdu.buildNumber(7601);

        HandshakeExPDU handshakeex_pdu;
        handshakeex_pdu.buildNumber(7601);
        handshakeex_pdu.railHandshakeFlags(TS_RAIL_ORDER_HANDSHAKEEX_FLAGS_HIDEF);

        if (TS_RAIL_ORDER_HANDSHAKE == orderType) {
            handshake_pdu.emit(out_s);
        }
        else {
            handshakeex_pdu.emit(out_s);
        }

        header.emit_end();

        const size_t   length     = out_s.get_offset();
        const uint32_t flags      =   CHANNELS::CHANNEL_FLAG_FIRST
                                    | CHANNELS::CHANNEL_FLAG_LAST;

        if (this->verbose) {
            {
                const bool send              = true;
                const bool from_or_to_client = true;
                ::msgdump_c(send, from_or_to_client, length, flags, out_s.get_produced_bytes());
            }
            if (TS_RAIL_ORDER_HANDSHAKE == orderType) {
                LOG(LOG_INFO, "ClientExecute::ready: Send to client - Server Handshake PDU");
                handshake_pdu.log(LOG_INFO);
            }
            else {
                LOG(LOG_INFO, "ClientExecute::ready: Send to client - Server HandshakeEx PDU");
                handshakeex_pdu.log(LOG_INFO);
            }
        }

        this->front_.send_to_channel(*this->channel_, out_s.get_produced_bytes(), length, flags);
    }

    {
        StaticOutStream<256> out_s;
        RAILPDUHeader header;
        header.emit_begin(out_s, TS_RAIL_ORDER_SYSPARAM);

        ServerSystemParametersUpdatePDU server_system_parameters_update_pdu;
        server_system_parameters_update_pdu.SystemParam(SPI_SETSCREENSAVESECURE);
        server_system_parameters_update_pdu.Body(0);
        server_system_parameters_update_pdu.emit(out_s);

        header.emit_end();

        const size_t   length     = out_s.get_offset();
        const uint32_t flags      =   CHANNELS::CHANNEL_FLAG_FIRST
                                    | CHANNELS::CHANNEL_FLAG_LAST;

        if (this->verbose) {
            {
                const bool send              = true;
                const bool from_or_to_client = true;
                ::msgdump_c(send, from_or_to_client, length, flags, out_s.get_produced_bytes());
            }
            LOG(LOG_INFO, "ClientExecute::ready: Send to client - Server System Parameters Update PDU");
            server_system_parameters_update_pdu.log(LOG_INFO);
        }

        this->front_.send_to_channel(*this->channel_, out_s.get_produced_bytes(), length, flags);
    }

    {
        StaticOutStream<256> out_s;
        RAILPDUHeader header;
        header.emit_begin(out_s, TS_RAIL_ORDER_SYSPARAM);

        ServerSystemParametersUpdatePDU server_system_parameters_update_pdu;
        server_system_parameters_update_pdu.SystemParam(SPI_SETSCREENSAVEACTIVE);
        server_system_parameters_update_pdu.Body(0);
        server_system_parameters_update_pdu.emit(out_s);

        header.emit_end();

        const size_t length     = out_s.get_offset();
        const uint32_t flags      =   CHANNELS::CHANNEL_FLAG_FIRST
                                    | CHANNELS::CHANNEL_FLAG_LAST;

        if (this->verbose) {
            {
                const bool send              = true;
                const bool from_or_to_client = true;
                ::msgdump_c(send, from_or_to_client, length, flags, out_s.get_produced_bytes());
            }
            LOG(LOG_INFO, "ClientExecute::ready: Send to client - Server System Parameters Update PDU");
            server_system_parameters_update_pdu.log(LOG_INFO);
        }

        this->front_.send_to_channel(*this->channel_, out_s.get_produced_bytes(), length, flags);
    }
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
void ClientExecute::check_is_unit_throw(uint32_t total_length, uint32_t flags, InStream& chunk, const char * message)
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::check_is_unit_throw(%s)", message);
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
        LOG(LOG_ERR, "ClientExecute::%s unexpected fragmentation chunk=%u total=%u", message, order_length, total_length);
        throw Error(ERR_RDP_DATA_CHANNEL_FRAGMENTATION);
    }
}

static void send_activate_window(uint32_t flag, gdi::GraphicApi & drawable_, bool verbose);

void send_activate_window(uint32_t flag, gdi::GraphicApi & drawable_, bool verbose)
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
            RDPOpaqueRect order(rect, encode_color24()(BLACK));

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
    RDPOpaqueRect order(this->protocol_window_rect, encode_color24()(BLACK));

    this->drawable_.draw(order, this->protocol_window_rect, depth);

    this->protocol_window_rect.empty();
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

    {
        const Rect adjusted_window_rect = window_rect.offset(this->window_offset_x, this->window_offset_y);

        RDP::RAIL::NewOrExistingWindow order;

        order.header.FieldsPresentFlags(
            uint32_t(RDP::RAIL::WINDOW_ORDER_STATE_NEW)
          | uint32_t(RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW)
          | uint32_t(RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTDELTA)
          | uint32_t(RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET)
          | uint32_t(RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET)
          | uint32_t(RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET)
          | uint32_t(RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE)
          | uint32_t(RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY)
          | uint32_t(RDP::RAIL::WINDOW_ORDER_FIELD_SHOW)
          | uint32_t(RDP::RAIL::WINDOW_ORDER_FIELD_STYLE)
          | uint32_t(RDP::RAIL::WINDOW_ORDER_FIELD_TITLE)
          | uint32_t(RDP::RAIL::WINDOW_ORDER_FIELD_OWNER)
        );
        order.header.WindowId(this->auxiliary_window_id);

        order.OwnerWindowId(0x0);
        order.Style(0x14EE0000);
        order.ExtendedStyle(0x40310 | 0x8);
        order.ShowState(5);
        order.TitleInfo("Dialog box");
        order.ClientOffsetX(adjusted_window_rect.x + 6);
        order.ClientOffsetY(adjusted_window_rect.y + 25);
        order.WindowOffsetX(adjusted_window_rect.x);
        order.WindowOffsetY(adjusted_window_rect.y);
        order.WindowClientDeltaX(6);
        order.WindowClientDeltaY(25);
        order.WindowWidth(adjusted_window_rect.cx);
        order.WindowHeight(adjusted_window_rect.cy);
        order.VisibleOffsetX(adjusted_window_rect.x);
        order.VisibleOffsetY(adjusted_window_rect.y);
        order.NumVisibilityRects(1);
        order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, adjusted_window_rect.cx, adjusted_window_rect.cy));

        if (this->verbose) {
            StaticOutStream<1024> out_s;
            order.emit(out_s);
            order.log(LOG_INFO);
            LOG(LOG_INFO, "ClientExecute::dialog_box_create: Send NewOrExistingWindow to client: size=%zu", out_s.get_offset() - 1);
        }

        this->drawable_.draw(order);
    }

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
    str_append(this->window_title, ti, (ti.empty() ? "" : " - "), INTERNAL_MODULE_WINDOW_TITLE);
}

void ClientExecute::update_widget()
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::update_widget()");
    Rect widget_rect_new = this->window_rect.shrink(1);
    widget_rect_new.y  += TITLE_BAR_HEIGHT;
    widget_rect_new.cy -= TITLE_BAR_HEIGHT;

    if (this->mod_) {
        this->mod_->move_size_widget(widget_rect_new.x, widget_rect_new.y,
            widget_rect_new.cx, widget_rect_new.cy);

        this->mod_->refresh(this->window_rect);
    }

    this->window_rect_old = this->window_rect;
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

void ClientExecute::initialize_move_size(uint16_t xPos, uint16_t yPos, const int pressed_mouse_button_)
{
    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::initialize_move_size(%d,%d,%d)", xPos, yPos, this->pressed_mouse_button);
    assert(!this->move_size_initialized);

    this->pressed_mouse_button = pressed_mouse_button_;

    this->captured_mouse_x = xPos;
    this->captured_mouse_y = yPos;

    this->window_rect_saved = this->window_rect;

    // TS_RAIL_ORDER_MINMAXINFO
    // Send to client - Server Min Max Info PDU (0)
    {
        StaticOutStream<256> out_s;
        RAILPDUHeader header;
        header.emit_begin(out_s, TS_RAIL_ORDER_MINMAXINFO);

        const Rect adjusted_virtual_sreen_rect = this->virtual_screen_rect.offset(
            this->window_offset_x, this->window_offset_y);

        ServerMinMaxInfoPDU smmipdu;

        smmipdu.WindowId(INTERNAL_MODULE_WINDOW_ID);
        smmipdu.MaxWidth(adjusted_virtual_sreen_rect.cx - 1);
        smmipdu.MaxHeight(adjusted_virtual_sreen_rect.cy - 1);
        smmipdu.MaxPosX(adjusted_virtual_sreen_rect.eright());
        smmipdu.MaxPosY(adjusted_virtual_sreen_rect.ebottom());
        smmipdu.MinTrackWidth(INTERNAL_MODULE_MINIMUM_WINDOW_WIDTH);
        smmipdu.MinTrackHeight(INTERNAL_MODULE_MINIMUM_WINDOW_HEIGHT);
        smmipdu.MaxTrackWidth(adjusted_virtual_sreen_rect.cx - 1);
        smmipdu.MaxTrackHeight(adjusted_virtual_sreen_rect.cy - 1);

        smmipdu.emit(out_s);

        header.emit_end();

        const size_t   length     = out_s.get_offset();
        const uint32_t flags      =   CHANNELS::CHANNEL_FLAG_FIRST
                                    | CHANNELS::CHANNEL_FLAG_LAST;

        if (this->verbose) {
            {
                const bool send              = true;
                const bool from_or_to_client = true;
                ::msgdump_c(send, from_or_to_client, length, flags, out_s.get_produced_bytes());
            }
            LOG_IF(this->verbose, LOG_INFO, "ClientExecute::initialize_move_size: Send to client - Server Min Max Info PDU (0)");
            smmipdu.log(LOG_INFO);
        }

        this->front_.send_to_channel(*this->channel_, out_s.get_produced_bytes(), length, flags);
    }

    // Send to client - Server Move/Size Start PDU (0)

    int move_size_type = 0;
    uint16_t PosX = xPos;
    uint16_t PosY = yPos;
    switch (pressed_mouse_button_) {
        case MOUSE_BUTTON_PRESSED_NORTH:     move_size_type = RAIL_WMSZ_TOP;         break;
        case MOUSE_BUTTON_PRESSED_NORTHWEST: move_size_type = RAIL_WMSZ_TOPLEFT;     break;
        case MOUSE_BUTTON_PRESSED_WEST:      move_size_type = RAIL_WMSZ_LEFT;        break;
        case MOUSE_BUTTON_PRESSED_SOUTHWEST: move_size_type = RAIL_WMSZ_BOTTOMLEFT;  break;
        case MOUSE_BUTTON_PRESSED_SOUTH:     move_size_type = RAIL_WMSZ_BOTTOM;      break;
        case MOUSE_BUTTON_PRESSED_SOUTHEAST: move_size_type = RAIL_WMSZ_BOTTOMRIGHT; break;
        case MOUSE_BUTTON_PRESSED_EAST:      move_size_type = RAIL_WMSZ_RIGHT;       break;
        case MOUSE_BUTTON_PRESSED_NORTHEAST: move_size_type = RAIL_WMSZ_TOPRIGHT;    break;
        case MOUSE_BUTTON_PRESSED_TITLEBAR:
            PosX = xPos - this->window_rect.x;
            PosY = yPos - this->window_rect.y;
            move_size_type = RAIL_WMSZ_MOVE;
            break;
    }

    if (move_size_type) {
        StaticOutStream<256> out_s;
        RAILPDUHeader header;
        header.emit_begin(out_s, TS_RAIL_ORDER_LOCALMOVESIZE);

        ServerMoveSizeStartOrEndPDU smssoepdu;

        smssoepdu.WindowId(INTERNAL_MODULE_WINDOW_ID);
        smssoepdu.IsMoveSizeStart(1);
        smssoepdu.MoveSizeType(move_size_type);
        smssoepdu.PosXOrTopLeftX(PosX);
        smssoepdu.PosYOrTopLeftY(PosY);

        smssoepdu.emit(out_s);

        header.emit_end();

        const size_t   length     = out_s.get_offset();
        const uint32_t flags      =   CHANNELS::CHANNEL_FLAG_FIRST
                                    | CHANNELS::CHANNEL_FLAG_LAST;

        if (this->verbose) {
            {
                const bool send              = true;
                const bool from_or_to_client = true;
                ::msgdump_c(send, from_or_to_client, length, flags, out_s.get_produced_bytes());
            }
            LOG_IF(this->verbose, LOG_INFO, "ClientExecute::initialize_move_size: Send to client - Server Move/Size Start PDU (0)");
            smssoepdu.log(LOG_INFO);
        }

        this->front_.send_to_channel(*this->channel_, out_s.get_produced_bytes(), length, flags);
    }   // if (move_size_type)

    this->move_size_initialized = true;
}   // initialize_move_size


bool ClientExecute::input_mouse(uint16_t pointerFlags, uint16_t xPos, uint16_t yPos)
{
    const bool allow_resize_hosted_desktop = this->allow_resize_hosted_desktop_;

    LOG_IF(this->verbose, LOG_INFO,
        "ClientExecute::input_mouse: pointerFlags=0x%X xPos=%u yPos=%u pressed_mouse_button=%d",
        pointerFlags, xPos, yPos, this->pressed_mouse_button);

    bool zone_found = false;

    // Mouse pointer managment
    if (!this->move_size_initialized) {
        using SetPointerMode = gdi::GraphicApi::SetPointerMode;
        for (int i = 0; i < Zone::NUMBER_OF_ZONES; i++){
            if (this->zone.get_zone(i, this->window_rect).contains_pt(xPos, yPos)){
                auto& pointer = this->zone.get_pointer(i);
                if (&pointer != this->current_mouse_pointer){
                    this->current_mouse_pointer = &pointer;
                    this->drawable_.set_pointer(0, pointer, SetPointerMode::Insert);
                }
                zone_found = true;
                break;
            }
        }

        if (!zone_found){
            this->current_mouse_pointer = &null_pointer();
        }
    }

    // Mouse action management
    if ((SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1) == pointerFlags) {
        if (MOUSE_BUTTON_PRESSED_NONE == this->pressed_mouse_button) {
            if (!this->maximized) {
                if (this->zone.get_zone(Zone::ZONE_TITLE, this->window_rect).contains_pt(xPos, yPos))
                {
                    this->pressed_mouse_button = MOUSE_BUTTON_PRESSED_TITLEBAR;
                }
                else {
                    for (int i = Zone::ZONE_N ; i <= Zone::ZONE_NEN ; i++){
                        if (this->zone.get_zone(i, this->window_rect).contains_pt(xPos, yPos)){
                            this->pressed_mouse_button = this->zone.get_button(i);
                        }
                    }
                }
            }

            if (MOUSE_BUTTON_PRESSED_NONE != this->pressed_mouse_button) {
                if ((MOUSE_BUTTON_PRESSED_NORTH == this->pressed_mouse_button)
                || (MOUSE_BUTTON_PRESSED_SOUTH == this->pressed_mouse_button)
                || (MOUSE_BUTTON_PRESSED_TITLEBAR == this->pressed_mouse_button)) {
                    this->button_1_down = this->pressed_mouse_button;

                    this->events_guard.create_event_timeout("Double Click Down Timer",
                        400ms, [this](Event &/*e*/)
                        {
                            assert(this->is_ready());
                            this->initialize_move_size(
                                this->button_1_down_x,
                                this->button_1_down_y,
                                this->button_1_down);
                        });

                    this->button_1_down_x = xPos;
                    this->button_1_down_y = yPos;

                    this->pressed_mouse_button = MOUSE_BUTTON_PRESSED_NONE;

                    LOG_IF(this->verbose, LOG_INFO,
                        "ClientExecute::input_mouse: Mouse button 1 pressed on %s delayed",
                        ((this->button_1_down == MOUSE_BUTTON_PRESSED_NORTH)
                            ? "north edge"
                            : ((this->button_1_down == MOUSE_BUTTON_PRESSED_TITLEBAR)
                            ? "title bar"
                            : "south edge")));
                }
                else {
                    this->initialize_move_size(xPos, yPos, this->pressed_mouse_button);
                }

            }   // if (MOUSE_BUTTON_PRESSED_NONE != this->pressed_mouse_button)
            else if (allow_resize_hosted_desktop
            && this->zone.get_zone(Zone::ZONE_RESIZE, this->window_rect).contains_pt(xPos, yPos))
            {
                this->draw_resize_hosted_desktop_box(true, this->zone.get_zone(Zone::ZONE_RESIZE, this->window_rect));
                this->drawable_.sync();
                this->pressed_mouse_button = MOUSE_BUTTON_PRESSED_RESIZEHOSTEDDESKTOPBOX;

            }
            else if (this->zone.get_zone(Zone::ZONE_MINI, this->window_rect).contains_pt(xPos, yPos))
            {
                auto rect_mini = this->zone.get_zone(Zone::ZONE_MINI, this->window_rect);

                RDPOpaqueRect order(rect_mini, encode_color24()(BGRColor{0xCBCACA}));
                this->drawable_.draw(order, rect_mini, gdi::ColorCtx::depth24());

                if (this->font_) {
                    gdi::server_draw_text(this->drawable_,
                                            *this->font_,
                                            rect_mini.x + 12,
                                            rect_mini.y + 3,
                                            "−",
                                            encode_color24()(BLACK),
                                            encode_color24()(BGRColor{0xCBCACA}),
                                            gdi::ColorCtx::depth24(),
                                            rect_mini
                                            );
                }
                this->drawable_.sync();
                this->pressed_mouse_button = MOUSE_BUTTON_PRESSED_MINIMIZEBOX;

            }
            else if (this->zone.get_zone(Zone::ZONE_MAXI, this->window_rect).contains_pt(xPos, yPos)) {
                auto rect_maxi = this->zone.get_zone(Zone::ZONE_MAXI, this->window_rect);
                this->draw_maximize_box(true, rect_maxi);
                this->drawable_.sync();
                this->pressed_mouse_button = MOUSE_BUTTON_PRESSED_MAXIMIZEBOX;

            }
            else if (this->zone.get_zone(Zone::ZONE_CLOSE, this->window_rect).contains_pt(xPos, yPos)) {
                auto rect_close = this->zone.get_zone(Zone::ZONE_CLOSE, this->window_rect);
                RDPOpaqueRect order(rect_close, encode_color24()(BGRColor{0x2311E8}));

                this->drawable_.draw(order, rect_close, gdi::ColorCtx::depth24());

                if (this->font_) {
                    gdi::server_draw_text(this->drawable_,
                                            *this->font_,
                                            rect_close.x + 13,
                                            rect_close.y + 3,
                                            "x",
                                            encode_color24()(WHITE),
                                            encode_color24()(BGRColor{0x2311E8}),
                                            gdi::ColorCtx::depth24(),
                                            rect_close
                                            );
                }

                this->drawable_.sync();

                this->pressed_mouse_button = MOUSE_BUTTON_PRESSED_CLOSEBOX;
            }
        }
    }
    else if (SlowPath::PTRFLAGS_MOVE == pointerFlags) {
        if (((MOUSE_BUTTON_PRESSED_TITLEBAR == this->pressed_mouse_button)
            || (MOUSE_BUTTON_PRESSED_NORTH == this->pressed_mouse_button)
            || (MOUSE_BUTTON_PRESSED_NORTHWEST == this->pressed_mouse_button)
            || (MOUSE_BUTTON_PRESSED_WEST == this->pressed_mouse_button)
            || (MOUSE_BUTTON_PRESSED_SOUTHWEST == this->pressed_mouse_button)
            || (MOUSE_BUTTON_PRESSED_SOUTH == this->pressed_mouse_button)
            || (MOUSE_BUTTON_PRESSED_SOUTHEAST == this->pressed_mouse_button)
            || (MOUSE_BUTTON_PRESSED_EAST == this->pressed_mouse_button)
            || (MOUSE_BUTTON_PRESSED_NORTHEAST == this->pressed_mouse_button))
        && !this->maximized
        ){
            if (this->full_window_drag_enabled) {
                int offset_x  = 0;
                int offset_y  = 0;
                int offset_cx = 0;
                int offset_cy = 0;

                switch (this->pressed_mouse_button) {
                    case MOUSE_BUTTON_PRESSED_TITLEBAR:
                        offset_x = xPos - this->captured_mouse_x;
                        offset_y = yPos - this->captured_mouse_y;
                    break;

                    case MOUSE_BUTTON_PRESSED_NORTH: {
                        const int offset_y_max = this->window_rect_saved.cy - INTERNAL_MODULE_MINIMUM_WINDOW_HEIGHT;

                        offset_y = std::min(yPos - this->captured_mouse_y, offset_y_max);
                        offset_cy = -offset_y;
                    }
                    break;

                    case MOUSE_BUTTON_PRESSED_NORTHWEST: {
                        const int offset_x_max = this->window_rect_saved.cx - INTERNAL_MODULE_MINIMUM_WINDOW_WIDTH;
                        const int offset_y_max = this->window_rect_saved.cy - INTERNAL_MODULE_MINIMUM_WINDOW_HEIGHT;

                        offset_x = std::min(xPos - this->captured_mouse_x, offset_x_max);
                        offset_cx = -offset_x;

                        offset_y = std::min(yPos - this->captured_mouse_y, offset_y_max);
                        offset_cy = -offset_y;
                    }
                    break;

                    case MOUSE_BUTTON_PRESSED_WEST: {
                        const int offset_x_max = this->window_rect_saved.cx - INTERNAL_MODULE_MINIMUM_WINDOW_WIDTH;

                        offset_x = std::min(xPos - this->captured_mouse_x, offset_x_max);
                        offset_cx = -offset_x;
                    }
                    break;

                    case MOUSE_BUTTON_PRESSED_SOUTHWEST: {
                        const int offset_x_max = this->window_rect_saved.cx - INTERNAL_MODULE_MINIMUM_WINDOW_WIDTH;

                        offset_x = std::min(xPos - this->captured_mouse_x, offset_x_max);
                        offset_cx = -offset_x;

                        const int offset_cy_min = INTERNAL_MODULE_MINIMUM_WINDOW_HEIGHT - this->window_rect_saved.cy;

                        offset_cy = std::max(yPos - this->captured_mouse_y, offset_cy_min);
                    }
                    break;

                    case MOUSE_BUTTON_PRESSED_SOUTH : {
                        const int offset_cy_min = INTERNAL_MODULE_MINIMUM_WINDOW_HEIGHT - this->window_rect_saved.cy;

                        offset_cy = std::max(yPos - this->captured_mouse_y, offset_cy_min);
                    }
                    break;

                    case MOUSE_BUTTON_PRESSED_SOUTHEAST: {
                        const int offset_cy_min = INTERNAL_MODULE_MINIMUM_WINDOW_HEIGHT - this->window_rect_saved.cy;

                        offset_cy = std::max(yPos - this->captured_mouse_y, offset_cy_min);

                        const int offset_cx_min = INTERNAL_MODULE_MINIMUM_WINDOW_WIDTH - this->window_rect_saved.cx;

                        offset_cx = std::max(xPos - this->captured_mouse_x, offset_cx_min);
                    }
                    break;

                    case MOUSE_BUTTON_PRESSED_EAST: {
                        const int offset_cx_min = INTERNAL_MODULE_MINIMUM_WINDOW_WIDTH - this->window_rect_saved.cx;

                        offset_cx = std::max(xPos - this->captured_mouse_x, offset_cx_min);
                    }
                    break;

                    case MOUSE_BUTTON_PRESSED_NORTHEAST: {
                        const int offset_y_max = this->window_rect_saved.cy - INTERNAL_MODULE_MINIMUM_WINDOW_HEIGHT;

                        offset_y = std::min(yPos - this->captured_mouse_y, offset_y_max);
                        offset_cy = -offset_y;

                        const int offset_cx_min = INTERNAL_MODULE_MINIMUM_WINDOW_WIDTH - this->window_rect_saved.cx;

                        offset_cx = std::max(xPos - this->captured_mouse_x, offset_cx_min);
                    }
                    break;
                }

                this->window_rect = this->window_rect_saved;

                this->window_rect = Rect(this->window_rect_saved.x + offset_x,
                                         this->window_rect_saved.y + offset_y,
                                         this->window_rect_saved.cx + offset_cx,
                                         this->window_rect_saved.cy + offset_cy);
                this->update_rects(allow_resize_hosted_desktop);

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
                    );
                order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

                const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset_x, this->window_offset_y);

                order.OwnerWindowId(0x0);
                order.Style(0x14EE0000);
                order.ExtendedStyle(0x40310);
                order.ShowState(5);
                order.TitleInfo(this->window_title.c_str());
                order.ClientOffsetX(adjusted_window_rect.x + 6);
                order.ClientOffsetY(adjusted_window_rect.y + 25);
                order.WindowOffsetX(adjusted_window_rect.x);
                order.WindowOffsetY(adjusted_window_rect.y);
                order.WindowClientDeltaX(6);
                order.WindowClientDeltaY(25);
                order.WindowWidth(adjusted_window_rect.cx);
                order.WindowHeight(adjusted_window_rect.cy);
                order.VisibleOffsetX(adjusted_window_rect.x);
                order.VisibleOffsetY(adjusted_window_rect.y);
                order.NumVisibilityRects(1);
                order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, adjusted_window_rect.cx, adjusted_window_rect.cy));

                if (this->verbose) {
                    StaticOutStream<1024> out_s;
                    order.emit(out_s);
                    order.log(LOG_INFO);
                    LOG(LOG_INFO, "ClientExecute::input_mouse: Send NewOrExistingWindow to client: size=%zu (0)", out_s.get_offset() - 1);
                }

                this->drawable_.draw(order);

                this->update_widget();

                this->on_new_or_existing_window(adjusted_window_rect);
            }   // if (this->full_window_drag_enabled)
        }
        else if (allow_resize_hosted_desktop
             && (MOUSE_BUTTON_PRESSED_RESIZEHOSTEDDESKTOPBOX == this->pressed_mouse_button)) {
            auto rect_resize = this->zone.get_zone(Zone::ZONE_RESIZE, this->window_rect);
            this->draw_resize_hosted_desktop_box(rect_resize.contains_pt(xPos, yPos), rect_resize);
            this->drawable_.sync();

        }   // else if (MOUSE_BUTTON_PRESSED_MINIMIZEBOX == this->pressed_mouse_button)
        else if (MOUSE_BUTTON_PRESSED_MINIMIZEBOX == this->pressed_mouse_button) {

            auto rect_mini = this->zone.get_zone(Zone::ZONE_MINI, this->window_rect);
            this->draw_maximize_box(rect_mini.contains_pt(xPos, yPos), rect_mini);
            this->drawable_.sync();
            auto front_color = encode_color24()(BLACK);
            auto back_color = (rect_mini.contains_pt(xPos, yPos))
                             ? encode_color24()(BGRColor{0xCBCACA})
                             : encode_color24()(WHITE);


            RDPOpaqueRect order(rect_mini, back_color);
            this->drawable_.draw(order, rect_mini, gdi::ColorCtx::depth24());

            if (this->font_) {
                gdi::server_draw_text(this->drawable_,
                                        *this->font_,
                                        rect_mini.x + 12,
                                        rect_mini.y + 3,
                                        "−",
                                        front_color,
                                        back_color,
                                        gdi::ColorCtx::depth24(),
                                        rect_mini
                                        );
            }
            this->drawable_.sync();
        }   // else if (MOUSE_BUTTON_PRESSED_MINIMIZEBOX == this->pressed_mouse_button)
        else if (MOUSE_BUTTON_PRESSED_MAXIMIZEBOX == this->pressed_mouse_button) {
            auto rect_maxi = this->zone.get_zone(Zone::ZONE_MAXI, this->window_rect);
            this->draw_maximize_box(rect_maxi.contains_pt(xPos, yPos), rect_maxi);
            this->drawable_.sync();

        }   // else if (MOUSE_BUTTON_PRESSED_MINIMIZEBOX == this->pressed_mouse_button)
        else if (MOUSE_BUTTON_PRESSED_CLOSEBOX == this->pressed_mouse_button) {
            auto rect_close = this->zone.get_zone(Zone::ZONE_CLOSE, this->window_rect);
            auto front_color = (rect_close.contains_pt(xPos, yPos))
                             ? encode_color24()(WHITE)
                             : encode_color24()(BLACK);

            auto back_color = (rect_close.contains_pt(xPos, yPos))
                             ? encode_color24()(BGRColor{0x2311E8})
                             : encode_color24()(WHITE);

            RDPOpaqueRect order(rect_close, back_color);
            this->drawable_.draw(order, rect_close, gdi::ColorCtx::depth24());

            if (this->font_) {
                gdi::server_draw_text(this->drawable_,
                                        *this->font_,
                                        rect_close.x + 13,
                                        rect_close.y + 3,
                                        "x",
                                        front_color,
                                        back_color,
                                        gdi::ColorCtx::depth24(),
                                        rect_close
                                        );
            }
            this->drawable_.sync();

        }   // else if (MOUSE_BUTTON_PRESSED_CLOSEBOX == this->pressed_mouse_button)
    }   // else if (SlowPath::PTRFLAGS_MOVE == pointerFlags)
    else if (SlowPath::PTRFLAGS_BUTTON1 == pointerFlags) {
        if (allow_resize_hosted_desktop
        && (MOUSE_BUTTON_PRESSED_RESIZEHOSTEDDESKTOPBOX == this->pressed_mouse_button)) {
            this->pressed_mouse_button = MOUSE_BUTTON_PRESSED_NONE;

            this->enable_resizing_hosted_desktop_ = (!this->enable_resizing_hosted_desktop_);

            this->draw_resize_hosted_desktop_box(false, this->zone.get_zone(Zone::ZONE_RESIZE, this->window_rect));

            this->drawable_.sync();

            if (this->enable_resizing_hosted_desktop_) {
                this->update_widget();
            }
        }   // if (this->allow_resize_hosted_desktop_ &&
        else if (MOUSE_BUTTON_PRESSED_MINIMIZEBOX == this->pressed_mouse_button) {
            this->pressed_mouse_button = MOUSE_BUTTON_PRESSED_NONE;

            auto rect_mini = this->zone.get_zone(Zone::ZONE_MINI, this->window_rect);

            {
                RDPOpaqueRect order(rect_mini, encode_color24()(WHITE));
                this->drawable_.draw(order, rect_mini, gdi::ColorCtx::depth24());

                if (this->font_) {
                    gdi::server_draw_text(this->drawable_,
                                            *this->font_,
                                            rect_mini.x + 12,
                                            rect_mini.y + 3,
                                            "−",
                                            encode_color24()(BLACK),
                                            encode_color24()(WHITE),
                                            gdi::ColorCtx::depth24(),
                                            rect_mini
                                            );
                }

                this->drawable_.sync();
            }

            if (rect_mini.contains_pt(xPos, yPos)) {
                RDP::RAIL::NewOrExistingWindow order;

                order.header.FieldsPresentFlags(
                            RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                        | (this->window_level_supported_ex ? RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREASIZE : 0)
                        | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTDELTA
                        | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
                        | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
                        | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
                        | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
                        | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
                        | RDP::RAIL::WINDOW_ORDER_FIELD_SHOW
                        | RDP::RAIL::WINDOW_ORDER_FIELD_STYLE
                    );
                order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

                order.ClientAreaWidth(0);
                order.ClientAreaHeight(0);
                order.WindowClientDeltaX(0);
                order.WindowClientDeltaY(0);
                order.ClientOffsetX(0);
                order.ClientOffsetY(800);
                order.VisibleOffsetX(0);
                order.VisibleOffsetY(800);
                order.WindowWidth(160);
                order.WindowHeight(TITLE_BAR_HEIGHT);
                order.WindowOffsetX(0);
                order.WindowOffsetY(800);
                order.NumVisibilityRects(1);
                order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, 160, TITLE_BAR_HEIGHT));
                order.ShowState(2);
                order.Style(0x34EE0000);
                order.ExtendedStyle(0x40310);

                if (this->verbose) {
                    StaticOutStream<1024> out_s;
                    order.emit(out_s);
                    order.log(LOG_INFO);
                    LOG(LOG_INFO, "ClientExecute::input_mouse: Send NewOrExistingWindow to client: size=%zu (1)", out_s.get_offset() - 1);
                }

                this->drawable_.draw(order);
                this->on_delete_window();

                if (this->mod_) {
                    this->mod_->rdp_input_invalidate(
                        Rect(
                                this->window_rect.x,
                                this->window_rect.y,
                                this->window_rect.x + this->window_rect.cx,
                                this->window_rect.y + this->window_rect.cy
                            ));
                }
            }
        }   // if (MOUSE_BUTTON_PRESSED_MINIMIZEBOX == this->pressed_mouse_button)
        else if (MOUSE_BUTTON_PRESSED_MAXIMIZEBOX == this->pressed_mouse_button) {
            this->pressed_mouse_button = MOUSE_BUTTON_PRESSED_NONE;

            auto rect_maxi = this->zone.get_zone(Zone::ZONE_MAXI, this->window_rect);
            this->draw_maximize_box(false, rect_maxi);
            this->drawable_.sync();
            if (rect_maxi.contains_pt(xPos, yPos)) {
                this->maximize_restore_window();
            }
        }   // else if (MOUSE_BUTTON_PRESSED_MAXIMIZEBOX == this->pressed_mouse_button)
        else if (MOUSE_BUTTON_PRESSED_CLOSEBOX == this->pressed_mouse_button) {
            this->pressed_mouse_button = MOUSE_BUTTON_PRESSED_NONE;
            auto rect_close = this->zone.get_zone(Zone::ZONE_CLOSE, this->window_rect);

            {
                RDPOpaqueRect order(rect_close, encode_color24()(WHITE));

                this->drawable_.draw(order, rect_close, gdi::ColorCtx::depth24());

                if (this->font_) {
                    gdi::server_draw_text(this->drawable_,
                                            *this->font_,
                                            rect_close.x + 13,
                                            rect_close.y + 3,
                                            "x",
                                            encode_color24()(BLACK),
                                            encode_color24()(WHITE),
                                            gdi::ColorCtx::depth24(),
                                            rect_close
                                            );
                }

                this->drawable_.sync();
            }

            if (rect_close.contains_pt(xPos, yPos)) {
                LOG(LOG_INFO, "ClientExecute::input_mouse: Close by user (Close Box)");
                throw Error(ERR_WIDGET);    // Close Box pressed
            }
        }   // else if (MOUSE_BUTTON_PRESSED_CLOSEBOX == this->pressed_mouse_button)
        else if ((MOUSE_BUTTON_PRESSED_NONE != this->pressed_mouse_button)
            && !this->maximized) {
            if (MOUSE_BUTTON_PRESSED_TITLEBAR == this->pressed_mouse_button) {
                LOG_IF(this->verbose, LOG_INFO, "ClientExecute::input_mouse: Mouse button 1 released from title bar");

                int const diff_x = (xPos - this->captured_mouse_x);
                int const diff_y = (yPos - this->captured_mouse_y);

                this->window_rect = Rect(this->window_rect_saved.x + diff_x,
                                         this->window_rect_saved.y + diff_y,
                                         this->window_rect.cx,
                                         this->window_rect.cy);

                this->update_rects(allow_resize_hosted_desktop);
            }   // if (MOUSE_BUTTON_PRESSED_TITLEBAR == this->pressed_mouse_button)

            int move_size_type = 0;
            switch (this->pressed_mouse_button) {
                case MOUSE_BUTTON_PRESSED_NORTH:     move_size_type = RAIL_WMSZ_TOP;         break;
                case MOUSE_BUTTON_PRESSED_NORTHWEST: move_size_type = RAIL_WMSZ_TOPLEFT;     break;
                case MOUSE_BUTTON_PRESSED_WEST:      move_size_type = RAIL_WMSZ_LEFT;        break;
                case MOUSE_BUTTON_PRESSED_SOUTHWEST: move_size_type = RAIL_WMSZ_BOTTOMLEFT;  break;
                case MOUSE_BUTTON_PRESSED_SOUTH:     move_size_type = RAIL_WMSZ_BOTTOM;      break;
                case MOUSE_BUTTON_PRESSED_SOUTHEAST: move_size_type = RAIL_WMSZ_BOTTOMRIGHT; break;
                case MOUSE_BUTTON_PRESSED_EAST:      move_size_type = RAIL_WMSZ_RIGHT;       break;
                case MOUSE_BUTTON_PRESSED_NORTHEAST: move_size_type = RAIL_WMSZ_TOPRIGHT;    break;
                case MOUSE_BUTTON_PRESSED_TITLEBAR:  move_size_type = RAIL_WMSZ_MOVE;        break;
            }

            if (0 != move_size_type) {
                StaticOutStream<256> out_s;
                RAILPDUHeader header;
                header.emit_begin(out_s, TS_RAIL_ORDER_LOCALMOVESIZE);

                const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset_x, this->window_offset_y);

                ServerMoveSizeStartOrEndPDU smssoepdu;

                smssoepdu.WindowId(INTERNAL_MODULE_WINDOW_ID);
                smssoepdu.IsMoveSizeStart(0);
                smssoepdu.MoveSizeType(move_size_type);
                smssoepdu.PosXOrTopLeftX(adjusted_window_rect.x);
                smssoepdu.PosYOrTopLeftY(adjusted_window_rect.y);

                smssoepdu.emit(out_s);

                header.emit_end();

                const size_t   length     = out_s.get_offset();
                const uint32_t flags      =   CHANNELS::CHANNEL_FLAG_FIRST
                                            | CHANNELS::CHANNEL_FLAG_LAST;

                if (this->verbose) {
                    {
                        const bool send              = true;
                        const bool from_or_to_client = true;
                        ::msgdump_c(send, from_or_to_client, length, flags, out_s.get_produced_bytes());
                    }
                    LOG(LOG_INFO, "ClientExecute::input_mouse: Send to client - Server Move/Size End PDU (1)");
                    smssoepdu.log(LOG_INFO);
                }

                this->front_.send_to_channel(*this->channel_, out_s.get_produced_bytes(), length, flags);

                this->move_size_initialized = false;
            }   // if (0 != move_size_type)

            const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset_x, this->window_offset_y);

            {
                RDP::RAIL::NewOrExistingWindow order;

                order.header.FieldsPresentFlags(
                            RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                        | (this->window_level_supported_ex ? RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREASIZE : 0)
                        | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
                        | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
                        | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
                        | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
                        | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
                    );
                order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

                order.ClientAreaWidth(adjusted_window_rect.cx - 6 * 2);
                order.ClientAreaHeight(adjusted_window_rect.cy - 25 - 6);
                order.WindowWidth(adjusted_window_rect.cx);
                order.WindowHeight(adjusted_window_rect.cy);
                order.NumVisibilityRects(1);
                order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, adjusted_window_rect.cx, adjusted_window_rect.cy));

                order.ClientOffsetX(adjusted_window_rect.x + 6);
                order.ClientOffsetY(adjusted_window_rect.y + 25);
                order.WindowOffsetX(adjusted_window_rect.x);
                order.WindowOffsetY(adjusted_window_rect.y);
                order.VisibleOffsetX(adjusted_window_rect.x);
                order.VisibleOffsetY(adjusted_window_rect.y);

                if (this->verbose) {
                    StaticOutStream<1024> out_s;
                    order.emit(out_s);
                    order.log(LOG_INFO);
                    LOG(LOG_INFO, "ClientExecute::input_mouse: Send NewOrExistingWindow to client: size=%zu (2)", out_s.get_offset() - 1);
                }

                this->drawable_.draw(order);
            }

            if (MOUSE_BUTTON_PRESSED_TITLEBAR == this->pressed_mouse_button) {
                this->update_widget();
            }   // if (MOUSE_BUTTON_PRESSED_TITLEBAR == this->pressed_mouse_button)

            this->on_new_or_existing_window(adjusted_window_rect);

            if (0 != move_size_type) {
                this->pressed_mouse_button = MOUSE_BUTTON_PRESSED_NONE;
            }
        }   // else if ((MOUSE_BUTTON_PRESSED_NONE != this->pressed_mouse_button) &&
    }   // else if (SlowPath::PTRFLAGS_BUTTON1 == pointerFlags)
    else if (PTRFLAGS_EX_DOUBLE_CLICK == pointerFlags) {
        if ((this->zone.get_zone(Zone::ZONE_N, this->window_rect).contains_pt(xPos, yPos)
        || this->zone.get_zone(Zone::ZONE_S, this->window_rect).contains_pt(xPos, yPos))
        && !this->maximized) {

            Rect work_area_rect = this->get_current_work_area_rect();
            this->window_rect = Rect(this->window_rect.x, 0, this->window_rect.cx, work_area_rect.cy - 1);
            this->update_rects(allow_resize_hosted_desktop);

            const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset_x, this->window_offset_y);

            {
                RDP::RAIL::NewOrExistingWindow order;

                order.header.FieldsPresentFlags(
                            RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                        | (this->window_level_supported_ex ? RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREASIZE : 0)
                        | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
                        | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
                        | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
                        | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
                        | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
                    );
                order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

                order.ClientAreaWidth(adjusted_window_rect.cx - 6 * 2);
                order.ClientAreaHeight(adjusted_window_rect.cy - 25 - 6);
                order.WindowWidth(adjusted_window_rect.cx);
                order.WindowHeight(adjusted_window_rect.cy);
                order.NumVisibilityRects(1);
                order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, adjusted_window_rect.cx, adjusted_window_rect.cy));

                order.ClientOffsetX(adjusted_window_rect.x + 6);
                order.ClientOffsetY(adjusted_window_rect.y + 25);
                order.WindowOffsetX(adjusted_window_rect.x);
                order.WindowOffsetY(adjusted_window_rect.y);
                order.VisibleOffsetX(adjusted_window_rect.x);
                order.VisibleOffsetY(adjusted_window_rect.y);

                if (this->verbose) {
                    StaticOutStream<1024> out_s;
                    order.emit(out_s);
                    order.log(LOG_INFO);
                    LOG(LOG_INFO, "ClientExecute::input_mouse: Send NewOrExistingWindow to client: size=%zu (3)", out_s.get_offset() - 1);
                }

                this->drawable_.draw(order);
            }

            this->update_widget();

            this->on_new_or_existing_window(adjusted_window_rect);
        }   // if (this->zone.get_zone(Zone::ZONE_S, this->window_rect).contains_pt(xPos, yPos))
        else if (this->zone.get_zone(Zone::ZONE_TITLE, this->window_rect).contains_pt(xPos, yPos)) {
            this->maximize_restore_window();
        }   // else if (this->zone.get_zone(Zone::ZONE_TITLE, this->window_rect).contains_pt(xPos, yPos))
        else if (this->zone.get_zone(Zone::ZONE_ICON, this->window_rect).contains_pt(xPos, yPos)) {
            LOG(LOG_INFO, "ClientExecute::input_mouse: Close by user (Title Bar Icon)");
            throw Error(ERR_WIDGET);    // Title Bar Icon Double-clicked
        }   // else if (this->zone.get_zone(Zone::ZONE_ICON, this->window_rect).contains_pt(xPos, yPos))
    }   // else if (PTRFLAGS_EX_DOUBLE_CLICK == pointerFlags)

    return zone_found;
}   // input_mouse

void ClientExecute::update_rects(const bool /*allow_resize_hosted_desktop*/)
{
    if ((this->window_rect.cx - 2) % 4) {
        this->window_rect.cx -= ((this->window_rect.cx - 2) % 4);
    }
}   // update_rects

Rect ClientExecute::get_window_rect() const
{
    return this->window_rect;
}

Point ClientExecute::get_window_offset() const
{
    return Point(this->window_offset_x, this->window_offset_y);
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
            this->check_is_unit_throw(length, flags, chunk, "ProcessClientActivatePDU");
            this->process_client_activate_pdu(chunk);
        break;

        case TS_RAIL_ORDER_CLIENTSTATUS:
            LOG_IF(this->verbose, LOG_INFO, "ClientExecute::send_to_mod_rail_channel:Client Information PDU");
            if (this->channel_){
                this->check_is_unit_throw(length, flags, chunk, "ProcessClientInformationPDU");

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
                this->check_is_unit_throw(length, flags, chunk, "ProcessClientExecutePDU");

                ClientExecutePDU cepdu;
                cepdu.receive(chunk);

                if (this->verbose) { cepdu.log(LOG_INFO); }

                const char* exe_of_file = cepdu.get_windows_execute_shell_params().exe_or_file.c_str();

                if (0 != ::strcasecmp(exe_of_file, DUMMY_REMOTEAPP)
                && (::strcasestr(exe_of_file, DUMMY_REMOTEAPP ":") != exe_of_file)) {
                    this->windows_execute_shell_params = cepdu.get_windows_execute_shell_params();
                }
                this->should_ignore_first_client_execute_ = false;
            }
        break;

        case TS_RAIL_ORDER_GET_APPID_REQ:
            LOG_IF(this->verbose, LOG_INFO, "ClientExecute::send_to_mod_rail_channel:Client Get Application ID PDU");

            if (this->channel_){
                this->check_is_unit_throw(length, flags, chunk, "ApplicationIdPDU");
                this->process_client_get_application_id_pdu(chunk);
            }
        break;

        case TS_RAIL_ORDER_HANDSHAKE:
            LOG_IF(this->verbose, LOG_INFO, "ClientExecute::send_to_mod_rail_channel:Client Handshake PDU");

            if (this->channel_) {
                this->check_is_unit_throw(length, flags, chunk, "ProcessClientHandshakePDU");

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
            this->check_is_unit_throw(length, flags, chunk, "ProcessClientSystemCommandPDU");
            this->process_client_system_command_pdu(chunk);
        break;

        case TS_RAIL_ORDER_SYSPARAM:
            LOG_IF(this->verbose, LOG_INFO,
                "ClientExecute::send_to_mod_rail_channel:Client System Parameters Update PDU");

            if (this->channel_) {
                this->check_is_unit_throw(length, flags, chunk, "ProcessClientSystemParametersUpdatePDU");
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
            this->check_is_unit_throw(length, flags, chunk, "ProcessClientWindowMovePDU");
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

    if ((capdu.WindowId() == INTERNAL_MODULE_WINDOW_ID) && (capdu.Enabled() == 0))
    {
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

    if (this->verbose) { cgaipdu.log(LOG_INFO); }

    {
        StaticOutStream<1024> out_s;
        RAILPDUHeader header;
        header.emit_begin(out_s, TS_RAIL_ORDER_GET_APPID_RESP);

        ServerGetApplicationIDResponsePDU server_get_application_id_response_pdu;
        server_get_application_id_response_pdu.WindowId(INTERNAL_MODULE_WINDOW_ID);
        server_get_application_id_response_pdu.ApplicationId(this->window_title.c_str());
        server_get_application_id_response_pdu.emit(out_s);

        header.emit_end();

        const size_t   length     = out_s.get_offset();
        const uint32_t flags      =   CHANNELS::CHANNEL_FLAG_FIRST
                                    | CHANNELS::CHANNEL_FLAG_LAST;

        LOG_IF(this->verbose, LOG_INFO, "ClientExecute::process_client_get_application_id_pdu: Send to client - Server Get Application ID Response PDU");
        if (this->verbose) {
            const bool send              = true;
            const bool from_or_to_client = true;
            ::msgdump_c(send, from_or_to_client, length, flags, out_s.get_produced_bytes());
            server_get_application_id_response_pdu.log(LOG_INFO);
        }
        this->front_.send_to_channel(*this->channel_, out_s.get_produced_bytes(), length, flags);
        this->server_execute_result_sent = true;
    }
}   // process_client_get_application_id_pdu

// TS_RAIL_ORDER_HANDSHAKE

// TS_RAIL_ORDER_LANGBARINFO:

// TS_RAIL_ORDER_LANGUAGEIMEINFO:

// TS_RAIL_ORDER_NOTIFY_EVENT:

//TS_RAIL_ORDER_SYSCOMMAND:

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
                    RDP::RAIL::NewOrExistingWindow order;

                    order.header.FieldsPresentFlags(
                                RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                            | (this->window_level_supported_ex ? RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREASIZE : 0)
                            | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTDELTA
                            | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
                            | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
                            | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
                            | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
                            | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
                            | RDP::RAIL::WINDOW_ORDER_FIELD_SHOW
                            | RDP::RAIL::WINDOW_ORDER_FIELD_STYLE
                        );
                    order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

                    order.ClientAreaWidth(0);
                    order.ClientAreaHeight(0);
                    order.WindowClientDeltaX(0);
                    order.WindowClientDeltaY(0);
                    order.ClientOffsetX(0);
                    order.ClientOffsetY(800);
                    order.VisibleOffsetX(0);
                    order.VisibleOffsetY(800);
                    order.WindowWidth(160);
                    order.WindowHeight(TITLE_BAR_HEIGHT);
                    order.WindowOffsetX(0);
                    order.WindowOffsetY(800);
                    order.NumVisibilityRects(1);
                    order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, 160, TITLE_BAR_HEIGHT));
                    order.ShowState(2);
                    order.Style(0x34EE0000);
                    order.ExtendedStyle(0x40310);

                    if (this->verbose) { order.log(LOG_INFO); }

                    this->drawable_.draw(order);
                    this->on_delete_window();
                }

                if (this->mod_) {
                    this->mod_->rdp_input_invalidate(
                        Rect(
                                this->window_rect.x,
                                this->window_rect.y,
                                this->window_rect.x + this->window_rect.cx,
                                this->window_rect.y + this->window_rect.cy
                            ));
                }
            }
            break;

        case SC_RESTORE:
            {
                const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset_x, this->window_offset_y);

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
                    );
                order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

                order.OwnerWindowId(0x0);
                order.Style(0x14EE0000);
                order.ExtendedStyle(0x40310);
                order.ShowState(5);
                order.TitleInfo(this->window_title.c_str());
                order.ClientOffsetX(adjusted_window_rect.x + 6);
                order.ClientOffsetY(adjusted_window_rect.y + 25);
                order.WindowOffsetX(adjusted_window_rect.x);
                order.WindowOffsetY(adjusted_window_rect.y);
                order.WindowClientDeltaX(6);
                order.WindowClientDeltaY(25);
                order.WindowWidth(adjusted_window_rect.cx);
                order.WindowHeight(adjusted_window_rect.cy);
                order.VisibleOffsetX(adjusted_window_rect.x);
                order.VisibleOffsetY(adjusted_window_rect.y);
                order.NumVisibilityRects(1);
                order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, adjusted_window_rect.cx, adjusted_window_rect.cy));

                if (this->verbose) { order.log(LOG_INFO); }

                this->drawable_.draw(order);
                this->on_new_or_existing_window(adjusted_window_rect);

                if (this->mod_) {
                    this->mod_->rdp_input_invalidate(
                        Rect(
                                this->window_rect.x,
                                this->window_rect.y,
                                this->window_rect.x + this->window_rect.cx,
                                this->window_rect.y + this->window_rect.cy
                            ));
                }
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
        RDP::RAIL::Rectangle const & body_r = cspupdu.body_r();

        this->work_areas[this->work_area_count].x  = body_r.iLeft();
        this->work_areas[this->work_area_count].y  = body_r.iTop();
        this->work_areas[this->work_area_count].cx = body_r.eRight() - body_r.iLeft();
        this->work_areas[this->work_area_count].cy = body_r.eBottom() - body_r.iTop();

        if (this->verbose) {
            LOG(LOG_INFO, "WorkAreaRect: (%d, %d, %u, %u)",
                this->work_areas[this->work_area_count].x, this->work_areas[this->work_area_count].y,
                this->work_areas[this->work_area_count].cx, this->work_areas[this->work_area_count].cy);
        }

        this->virtual_screen_rect = this->virtual_screen_rect.disjunct(this->work_areas[this->work_area_count]);

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
            RDP::RAIL::NewOrExistingWindow order;

            order.header.FieldsPresentFlags(
                        RDP::RAIL::WINDOW_ORDER_STATE_NEW
                    | RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
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
                );
            order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

            const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset_x, this->window_offset_y);

            order.OwnerWindowId(0x0);
            order.Style(0x14EE0000);
            order.ExtendedStyle(0x40310);
            order.ShowState(this->maximized ? 3 : 5);
            order.TitleInfo(this->window_title.c_str());
            order.ClientOffsetX(adjusted_window_rect.x + 6);
            order.ClientOffsetY(adjusted_window_rect.y + 25);
            order.WindowOffsetX(adjusted_window_rect.x);
            order.WindowOffsetY(adjusted_window_rect.y);
            order.WindowClientDeltaX(6);
            order.WindowClientDeltaY(25);
            order.WindowWidth(adjusted_window_rect.cx);
            order.WindowHeight(adjusted_window_rect.cy);
            order.VisibleOffsetX(adjusted_window_rect.x);
            order.VisibleOffsetY(adjusted_window_rect.y);
            order.NumVisibilityRects(1);
            order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, adjusted_window_rect.cx, adjusted_window_rect.cy));

            if (this->verbose) { order.log(LOG_INFO); }

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

        if (this->mod_) {
            this->mod_->rdp_input_invalidate(
                Rect(
                        this->window_rect.x,
                        this->window_rect.y,
                        this->window_rect.x + this->window_rect.cx,
                        this->window_rect.y + this->window_rect.cy
                    ));
        }
    }   // if (cspupdu.SystemParam() == SPI_SETWORKAREA)
    else if (cspupdu.SystemParam() == RAIL_SPI_TASKBARPOS) {
        RDP::RAIL::Rectangle const & body_r = cspupdu.body_r();

        this->task_bar_rect.x  = body_r.iLeft();
        this->task_bar_rect.y  = body_r.iTop();
        this->task_bar_rect.cx = body_r.eRight() - body_r.iLeft();
        this->task_bar_rect.cy = body_r.eBottom() - body_r.iTop();

        if (this->verbose) {
            LOG(LOG_INFO, "ClientExecute::process_client_system_parameters_update_pdu: TaskBarRect(%d, %d, %u, %u)",
                this->task_bar_rect.x, this->task_bar_rect.y,
                this->task_bar_rect.cx, this->task_bar_rect.cy);
        }
    }   // else if (cspupdu.SystemParam() == RAIL_SPI_TASKBARPOS)
    else if (cspupdu.SystemParam() == SPI_SETDRAGFULLWINDOWS) {
        this->full_window_drag_enabled =
            (cspupdu.body_b() != 0);
        if (this->verbose) {
            LOG(LOG_INFO,
                "ClientExecute::process_client_system_parameters_update_pdu: Full Window Drag is %s",
                (this->full_window_drag_enabled ? "enabled" : "disabled"));
        }
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

        this->window_rect = Rect(cwmpdu.iLeft() - this->window_offset_x,
                                 cwmpdu.iTop() - this->window_offset_y,
                                 cwmpdu.eRight() - cwmpdu.iLeft(),
                                 cwmpdu.eBottom() - cwmpdu.iTop());

        this->update_rects(this->allow_resize_hosted_desktop_);

        const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset_x, this->window_offset_y);

        {
            RDP::RAIL::NewOrExistingWindow order;

            order.header.FieldsPresentFlags(
                        RDP::RAIL::WINDOW_ORDER_TYPE_WINDOW
                    | (this->window_level_supported_ex ? RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREASIZE : 0)
                    | RDP::RAIL::WINDOW_ORDER_FIELD_WNDSIZE
                    | RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY
                    | RDP::RAIL::WINDOW_ORDER_FIELD_CLIENTAREAOFFSET
                    | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
                    | RDP::RAIL::WINDOW_ORDER_FIELD_WNDOFFSET
                );
            order.header.WindowId(INTERNAL_MODULE_WINDOW_ID);

            order.ClientAreaWidth(adjusted_window_rect.cx - 6 * 2);
            order.ClientAreaHeight(adjusted_window_rect.cy - 25 - 6);
            order.WindowWidth(adjusted_window_rect.cx);
            order.WindowHeight(adjusted_window_rect.cy);
            order.NumVisibilityRects(1);
            order.VisibilityRects(0, RDP::RAIL::Rectangle(0, 0, adjusted_window_rect.cx, adjusted_window_rect.cy));

            order.ClientOffsetX(adjusted_window_rect.x + 6);
            order.ClientOffsetY(adjusted_window_rect.y + 25);
            order.WindowOffsetX(adjusted_window_rect.x);
            order.WindowOffsetY(adjusted_window_rect.y);
            order.VisibleOffsetX(adjusted_window_rect.x);
            order.VisibleOffsetY(adjusted_window_rect.y);

            if (this->verbose) { order.log(LOG_INFO); }

            this->drawable_.draw(order);
        }

        int move_size_type = 0;
        switch (this->pressed_mouse_button) {
            case MOUSE_BUTTON_PRESSED_NORTH:     move_size_type = RAIL_WMSZ_TOP;         break;
            case MOUSE_BUTTON_PRESSED_NORTHWEST: move_size_type = RAIL_WMSZ_TOPLEFT;     break;
            case MOUSE_BUTTON_PRESSED_WEST:      move_size_type = RAIL_WMSZ_LEFT;        break;
            case MOUSE_BUTTON_PRESSED_SOUTHWEST: move_size_type = RAIL_WMSZ_BOTTOMLEFT;  break;
            case MOUSE_BUTTON_PRESSED_SOUTH:     move_size_type = RAIL_WMSZ_BOTTOM;      break;
            case MOUSE_BUTTON_PRESSED_SOUTHEAST: move_size_type = RAIL_WMSZ_BOTTOMRIGHT; break;
            case MOUSE_BUTTON_PRESSED_EAST:      move_size_type = RAIL_WMSZ_RIGHT;       break;
            case MOUSE_BUTTON_PRESSED_NORTHEAST: move_size_type = RAIL_WMSZ_TOPRIGHT;    break;
            case MOUSE_BUTTON_PRESSED_TITLEBAR:  move_size_type = RAIL_WMSZ_MOVE;        break;
        }

        if (0 != move_size_type) {
            const Rect adjusted_window_rect = this->window_rect.offset(this->window_offset_x, this->window_offset_y);

            StaticOutStream<256> out_s;
            RAILPDUHeader header;
            header.emit_begin(out_s, TS_RAIL_ORDER_LOCALMOVESIZE);

            ServerMoveSizeStartOrEndPDU smssoepdu;

            smssoepdu.WindowId(INTERNAL_MODULE_WINDOW_ID);
            smssoepdu.IsMoveSizeStart(0);
            smssoepdu.MoveSizeType(move_size_type);
            smssoepdu.PosXOrTopLeftX(adjusted_window_rect.x);
            smssoepdu.PosYOrTopLeftY(adjusted_window_rect.y);

            smssoepdu.emit(out_s);

            header.emit_end();

            const size_t   length     = out_s.get_offset();
            const uint32_t flags      =   CHANNELS::CHANNEL_FLAG_FIRST
                                        | CHANNELS::CHANNEL_FLAG_LAST;

            if (this->verbose) {
                {
                    const bool send              = true;
                    const bool from_or_to_client = true;
                    ::msgdump_c(send, from_or_to_client, length, flags, out_s.get_produced_bytes());
                }
                LOG(LOG_INFO, "ClientExecute::process_client_window_move_pdu: Send to client - Server Move/Size End PDU");
                smssoepdu.log(LOG_INFO);
            }

            this->front_.send_to_channel(*this->channel_, out_s.get_produced_bytes(), length, flags);
            this->move_size_initialized = false;
        }

        this->pressed_mouse_button = MOUSE_BUTTON_PRESSED_NONE;
        this->update_widget();
        this->on_new_or_existing_window(adjusted_window_rect);
    }
}   // process_client_window_move_pdu

// ==========================================================
//
// End of Rail Actions received from client via RAIL Channel
//
// ==========================================================

