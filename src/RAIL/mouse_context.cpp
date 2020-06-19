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


void ClientExecute::update_rects()
{
   auto & self = this->mouse_context;

    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::update_rects()");
    if ((self.window_rect.cx - 2) % 4) {
        self.window_rect.cx -= ((self.window_rect.cx - 2) % 4);
    }

    self.title_bar_rect = self.window_rect;
    self.title_bar_rect.cy = TITLE_BAR_HEIGHT;
    self.title_bar_rect.x++;
    self.title_bar_rect.y++;
    self.title_bar_rect.cx -= 2;
    self.title_bar_rect.cy--;

    self.title_bar_icon_rect    = self.title_bar_rect;
    self.title_bar_icon_rect.cx = 3 + 16 + 2;

    if (this->allow_resize_hosted_desktop_) {
        self.resize_hosted_desktop_box_rect     = self.title_bar_rect;
        self.resize_hosted_desktop_box_rect.x  += self.title_bar_rect.cx - TITLE_BAR_BUTTON_WIDTH * 4;
        self.resize_hosted_desktop_box_rect.cx  = TITLE_BAR_BUTTON_WIDTH;
    }

    self.minimize_box_rect     = self.title_bar_rect;
    self.minimize_box_rect.x  += self.title_bar_rect.cx - TITLE_BAR_BUTTON_WIDTH * 3;
    self.minimize_box_rect.cx  = TITLE_BAR_BUTTON_WIDTH;

    self.maximize_box_rect     = self.title_bar_rect;
    self.maximize_box_rect.x  += self.title_bar_rect.cx - TITLE_BAR_BUTTON_WIDTH * 2;
    self.maximize_box_rect.cx  = TITLE_BAR_BUTTON_WIDTH;

    self.close_box_rect     = self.title_bar_rect;
    self.close_box_rect.x  += self.title_bar_rect.cx - TITLE_BAR_BUTTON_WIDTH;
    self.close_box_rect.cx  = TITLE_BAR_BUTTON_WIDTH;

    self.title_bar_rect.cx -= TITLE_BAR_BUTTON_WIDTH * (3 + (this->allow_resize_hosted_desktop_ ? 1 : 0));

    self.title_bar_rect.x  += 3 + 16 + 2;
    self.title_bar_rect.cx -= 3 + 16 + 2;

    self.north.x  = self.window_rect.x + TITLE_BAR_HEIGHT;
    self.north.y  = self.window_rect.y;
    self.north.cx = self.window_rect.cx - TITLE_BAR_HEIGHT * 2;
    self.north.cy = BORDER_WIDTH_HEIGHT;

    self.north_west_north.x  = self.window_rect.x;
    self.north_west_north.y  = self.window_rect.y;
    self.north_west_north.cx = TITLE_BAR_HEIGHT;
    self.north_west_north.cy = BORDER_WIDTH_HEIGHT;

    self.north_west_west.x  = self.window_rect.x;
    self.north_west_west.y  = self.window_rect.y;
    self.north_west_west.cx = BORDER_WIDTH_HEIGHT;
    self.north_west_west.cy = TITLE_BAR_HEIGHT;

    self.west.x  = self.window_rect.x;
    self.west.y  = self.window_rect.y + TITLE_BAR_HEIGHT;
    self.west.cx = BORDER_WIDTH_HEIGHT;
    self.west.cy = self.window_rect.cy - TITLE_BAR_HEIGHT * 2;

    self.south_west_west.x  = self.window_rect.x;
    self.south_west_west.y  = self.window_rect.y + self.window_rect.cy - TITLE_BAR_HEIGHT;
    self.south_west_west.cx = BORDER_WIDTH_HEIGHT;
    self.south_west_west.cy = TITLE_BAR_HEIGHT;

    self.south_west_south.x  = self.window_rect.x;
    self.south_west_south.y  = self.window_rect.y + self.window_rect.cy - BORDER_WIDTH_HEIGHT;
    self.south_west_south.cx = TITLE_BAR_HEIGHT;
    self.south_west_south.cy = BORDER_WIDTH_HEIGHT;

    self.south.x  = self.window_rect.x + TITLE_BAR_HEIGHT;
    self.south.y  = self.window_rect.y + self.window_rect.cy - BORDER_WIDTH_HEIGHT;
    self.south.cx = self.window_rect.cx - TITLE_BAR_HEIGHT * 2;
    self.south.cy = BORDER_WIDTH_HEIGHT;

    self.south_east_south.x  = self.window_rect.x + self.window_rect.cx - TITLE_BAR_HEIGHT;
    self.south_east_south.y  = self.window_rect.y + self.window_rect.cy - BORDER_WIDTH_HEIGHT;
    self.south_east_south.cx = TITLE_BAR_HEIGHT;
    self.south_east_south.cy = BORDER_WIDTH_HEIGHT;

    self.south_east_east.x  = self.window_rect.x + self.window_rect.cx - BORDER_WIDTH_HEIGHT;
    self.south_east_east.y  = self.window_rect.y + self.window_rect.cy - TITLE_BAR_HEIGHT;
    self.south_east_east.cx = BORDER_WIDTH_HEIGHT;
    self.south_east_east.cy = TITLE_BAR_HEIGHT;

    self.east.x  = self.window_rect.x + self.window_rect.cx - BORDER_WIDTH_HEIGHT;
    self.east.y  = self.window_rect.y + TITLE_BAR_HEIGHT;
    self.east.cx = BORDER_WIDTH_HEIGHT;
    self.east.cy = self.window_rect.cy - TITLE_BAR_HEIGHT * 2;

    self.north_east_east.x  = self.window_rect.x + self.window_rect.cx - BORDER_WIDTH_HEIGHT;
    self.north_east_east.y  = self.window_rect.y;
    self.north_east_east.cx = BORDER_WIDTH_HEIGHT;
    self.north_east_east.cy = TITLE_BAR_HEIGHT;

    self.north_east_north.x  = self.window_rect.x + self.window_rect.cx - TITLE_BAR_HEIGHT;
    self.north_east_north.y  = self.window_rect.y;
    self.north_east_north.cx = TITLE_BAR_HEIGHT;
    self.north_east_north.cy = BORDER_WIDTH_HEIGHT;
}   // update_rects


void ClientExecute::initialize_move_size(uint16_t xPos, uint16_t yPos, int pressed_mouse_button_)
{
    auto & self = this->mouse_context;

    LOG_IF(this->verbose, LOG_INFO, "ClientExecute::initialize_move_size(%d,%d,%d)", xPos, yPos, self.pressed_mouse_button);
    assert(!self.move_size_initialized);

    self.pressed_mouse_button = pressed_mouse_button_;

    self.captured_mouse_x = xPos;
    self.captured_mouse_y = yPos;

    self.window_rect_saved = self.window_rect;

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

    int move_size_type = 0;
    uint16_t PosX = xPos;
    uint16_t PosY = yPos;
    switch (pressed_mouse_button_) {
        case MouseContext::MOUSE_BUTTON_PRESSED_NORTH:     move_size_type = RAIL_WMSZ_TOP;         break;
        case MouseContext::MOUSE_BUTTON_PRESSED_NORTHWEST: move_size_type = RAIL_WMSZ_TOPLEFT;     break;
        case MouseContext::MOUSE_BUTTON_PRESSED_WEST:      move_size_type = RAIL_WMSZ_LEFT;        break;
        case MouseContext::MOUSE_BUTTON_PRESSED_SOUTHWEST: move_size_type = RAIL_WMSZ_BOTTOMLEFT;  break;
        case MouseContext::MOUSE_BUTTON_PRESSED_SOUTH:     move_size_type = RAIL_WMSZ_BOTTOM;      break;
        case MouseContext::MOUSE_BUTTON_PRESSED_SOUTHEAST: move_size_type = RAIL_WMSZ_BOTTOMRIGHT; break;
        case MouseContext::MOUSE_BUTTON_PRESSED_EAST:      move_size_type = RAIL_WMSZ_RIGHT;       break;
        case MouseContext::MOUSE_BUTTON_PRESSED_NORTHEAST: move_size_type = RAIL_WMSZ_TOPRIGHT;    break;
        case MouseContext::MOUSE_BUTTON_PRESSED_TITLEBAR:
            PosX = xPos - self.window_rect.x;
            PosY = yPos - self.window_rect.y;
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

    self.move_size_initialized = true;
}   // initialize_move_size


// Return true if event is consumed.
bool ClientExecute::input_mouse(uint16_t pointerFlags, uint16_t xPos, uint16_t yPos, bool& mouse_captured_ref)
{
    auto & self = this->mouse_context;

    LOG_IF(this->verbose, LOG_INFO,
        "ClientExecute::input_mouse: pointerFlags=0x%X xPos=%u yPos=%u pressed_mouse_button=%d",
        pointerFlags, xPos, yPos, self.pressed_mouse_button);

    if (self.button_1_down_timer) {
        if (SlowPath::PTRFLAGS_BUTTON1 != pointerFlags) {
            this->initialize_move_size(self.button_1_down_x, self.button_1_down_y,
                self.button_1_down);
        }

        self.button_1_down_timer.reset();
    }

    // Mouse pointer managment
    using SetPointerMode = gdi::GraphicApi::SetPointerMode;

    if (!self.move_size_initialized) {
        if (self.north.contains_pt(xPos, yPos)
        ||  self.south.contains_pt(xPos, yPos)) {
            if (Pointer::POINTER_SIZENS != this->current_mouse_pointer_type) {
                    this->current_mouse_pointer_type = Pointer::POINTER_SIZENS;
                    this->drawable_.set_pointer(0, size_NS_pointer(), SetPointerMode::Insert);
            }
        }
        else if (self.north_west_north.contains_pt(xPos, yPos)
             ||  self.north_west_west.contains_pt(xPos, yPos)
             ||  self.south_east_south.contains_pt(xPos, yPos)
             ||  self.south_east_east.contains_pt(xPos, yPos)) {
            if (Pointer::POINTER_SIZENWSE != this->current_mouse_pointer_type) {
                    this->current_mouse_pointer_type = Pointer::POINTER_SIZENWSE;
                    this->drawable_.set_pointer(0, size_NESW_pointer(), SetPointerMode::Insert);
            }
        }
        else if (self.west.contains_pt(xPos, yPos)
             ||  self.east.contains_pt(xPos, yPos)) {
            if (Pointer::POINTER_SIZEWE != this->current_mouse_pointer_type) {
                    this->current_mouse_pointer_type = Pointer::POINTER_SIZEWE;
                    this->drawable_.set_pointer(0, size_WE_pointer(), SetPointerMode::Insert);
            }
        }
        else if (self.south_west_west.contains_pt(xPos, yPos)
             ||  self.south_west_south.contains_pt(xPos, yPos)
             ||  self.north_east_east.contains_pt(xPos, yPos)
             ||  self.north_east_north.contains_pt(xPos, yPos)) {
            if (Pointer::POINTER_SIZENESW != this->current_mouse_pointer_type) {
                    this->current_mouse_pointer_type = Pointer::POINTER_SIZENESW;
                    this->drawable_.set_pointer(0, size_NESW_pointer(), SetPointerMode::Insert);
            }
        }
        else if ((self.title_bar_rect.contains_pt(xPos, yPos))
             ||  (this->enable_resizing_hosted_desktop_ && self.resize_hosted_desktop_box_rect.contains_pt(xPos, yPos))
             ||  (self.minimize_box_rect.contains_pt(xPos, yPos))
             ||  (self.maximize_box_rect.contains_pt(xPos, yPos))
             ||  (self.close_box_rect.contains_pt(xPos, yPos))) {
            if (Pointer::POINTER_NORMAL != this->current_mouse_pointer_type) {
                this->current_mouse_pointer_type = Pointer::POINTER_NORMAL;
                this->drawable_.set_pointer(0, normal_pointer(), SetPointerMode::Insert);
            }
        }
        else {
                this->current_mouse_pointer_type = Pointer::POINTER_NULL;
                mouse_captured_ref = false;
        }
    }
    // Mouse action managment

    if ((SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1) == pointerFlags) {
        if (MouseContext::MOUSE_BUTTON_PRESSED_NONE == self.pressed_mouse_button) {
            if (!this->maximized) {
                if (self.north.contains_pt(xPos, yPos))
                {
                    self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_NORTH;
                }
                else if (self.north_west_north.contains_pt(xPos, yPos)
                     || self.north_west_west.contains_pt(xPos, yPos))
                {
                    self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_NORTHWEST;
                }
                else if (self.west.contains_pt(xPos, yPos)) {
                    self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_WEST;
                }
                else if (self.south_west_west.contains_pt(xPos, yPos)
                     || self.south_west_south.contains_pt(xPos, yPos))
                {
                    self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_SOUTHWEST;
                }
                else if (self.south.contains_pt(xPos, yPos))
                {
                    self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_SOUTH;
                }
                else if (self.south_east_south.contains_pt(xPos, yPos)
                     || self.south_east_east.contains_pt(xPos, yPos))
                {
                    self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_SOUTHEAST;
                }
                else if (self.east.contains_pt(xPos, yPos))
                {
                    self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_EAST;
                }
                else if (self.north_east_east.contains_pt(xPos, yPos)
                     || self.north_east_north.contains_pt(xPos, yPos))
                {
                    self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_NORTHEAST;
                }
                else if (self.title_bar_rect.contains_pt(xPos, yPos))
                {
                    self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_TITLEBAR;
                }
            }

            if (MouseContext::MOUSE_BUTTON_PRESSED_NONE != self.pressed_mouse_button) {
                if ((MouseContext::MOUSE_BUTTON_PRESSED_NORTH == self.pressed_mouse_button) ||
                    (MouseContext::MOUSE_BUTTON_PRESSED_SOUTH == self.pressed_mouse_button) ||
                    (MouseContext::MOUSE_BUTTON_PRESSED_TITLEBAR == self.pressed_mouse_button)) {
                    self.button_1_down = self.pressed_mouse_button;

                    Event event_down_timer("Double Click Down Timer", this);
                    event_down_timer.alarm.set_timeout(this->time_base.get_current_time()
                        +std::chrono::milliseconds{400});
                    event_down_timer.actions.on_timeout = [this](Event &){
                        assert(this->is_ready());
                        this->initialize_move_size(
                            this->mouse_context.button_1_down_x,
                            this->mouse_context.button_1_down_y,
                            this->mouse_context.button_1_down);
                    };
                    this->events.push_back(std::move(event_down_timer));

                    self.button_1_down_x = xPos;
                    self.button_1_down_y = yPos;

                    self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_NONE;

                    LOG_IF(this->verbose, LOG_INFO,
                        "ClientExecute::input_mouse: Mouse button 1 pressed on %s delayed",
                        ((self.button_1_down == MouseContext::MOUSE_BUTTON_PRESSED_NORTH)
                            ? "north edge"
                            : ((self.button_1_down == MouseContext::MOUSE_BUTTON_PRESSED_TITLEBAR)
                            ? "title bar"
                            : "south edge")));
                }
                else {
                    this->initialize_move_size(xPos, yPos, self.pressed_mouse_button);
                }
            }   // if (MouseContext::MOUSE_BUTTON_PRESSED_NONE != self.pressed_mouse_button)
            else if (this->allow_resize_hosted_desktop_
                && self.resize_hosted_desktop_box_rect.contains_pt(xPos, yPos))
            {
                this->draw_resize_hosted_desktop_box(true, self.resize_hosted_desktop_box_rect);
                this->drawable_.sync();
                self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_RESIZEHOSTEDDESKTOPBOX;
            }   // else if (self.maximize_box_rect.contains_pt(xPos, yPos))
            else if (self.minimize_box_rect.contains_pt(xPos, yPos))
            {
                RDPOpaqueRect order(self.minimize_box_rect, encode_color24()(BGRColor{0xCBCACA}));

                this->drawable_.draw(order, self.minimize_box_rect, gdi::ColorCtx::depth24());

                if (this->font_) {
                    gdi::server_draw_text(this->drawable_,
                                            *this->font_,
                                            self.minimize_box_rect.x + 12,
                                            self.minimize_box_rect.y + 3,
                                            "−",
                                            encode_color24()(BLACK),
                                            encode_color24()(BGRColor{0xCBCACA}),
                                            gdi::ColorCtx::depth24(),
                                            self.minimize_box_rect
                                            );
                }

                this->drawable_.sync();

                self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_MINIMIZEBOX;
            }   // else if (self.minimize_box_rect.contains_pt(xPos, yPos))
            else if (self.maximize_box_rect.contains_pt(xPos, yPos)) {
                this->draw_maximize_box(true, self.maximize_box_rect);

                this->drawable_.sync();

                self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_MAXIMIZEBOX;
            }   // else if (self.maximize_box_rect.contains_pt(xPos, yPos))
            else if (self.close_box_rect.contains_pt(xPos, yPos)) {
                RDPOpaqueRect order(self.close_box_rect, encode_color24()(BGRColor{0x2311E8}));

                this->drawable_.draw(order, self.close_box_rect, gdi::ColorCtx::depth24());

                if (this->font_) {
                    gdi::server_draw_text(this->drawable_,
                                            *this->font_,
                                            self.close_box_rect.x + 13,
                                            self.close_box_rect.y + 3,
                                            "x",
                                            encode_color24()(WHITE),
                                            encode_color24()(BGRColor{0x2311E8}),
                                            gdi::ColorCtx::depth24(),
                                            self.close_box_rect
                                            );
                }

                this->drawable_.sync();

                self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_CLOSEBOX;
            }   // else if (self.close_box_rect.contains_pt(xPos, yPos))
        }   // if (MouseContext::MOUSE_BUTTON_PRESSED_NONE == self.pressed_mouse_button)
    }   // if ((SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1) == pointerFlags)
    else if (SlowPath::PTRFLAGS_MOVE == pointerFlags) {
        if (((MouseContext::MOUSE_BUTTON_PRESSED_TITLEBAR == self.pressed_mouse_button) ||
                (MouseContext::MOUSE_BUTTON_PRESSED_NORTH == self.pressed_mouse_button) ||
                (MouseContext::MOUSE_BUTTON_PRESSED_NORTHWEST == self.pressed_mouse_button) ||
                (MouseContext::MOUSE_BUTTON_PRESSED_WEST == self.pressed_mouse_button) ||
                (MouseContext::MOUSE_BUTTON_PRESSED_SOUTHWEST == self.pressed_mouse_button) ||
                (MouseContext::MOUSE_BUTTON_PRESSED_SOUTH == self.pressed_mouse_button) ||
                (MouseContext::MOUSE_BUTTON_PRESSED_SOUTHEAST == self.pressed_mouse_button) ||
                (MouseContext::MOUSE_BUTTON_PRESSED_EAST == self.pressed_mouse_button) ||
                (MouseContext::MOUSE_BUTTON_PRESSED_NORTHEAST == self.pressed_mouse_button)) &&
            !this->maximized) {

            if (this->full_window_drag_enabled) {
                int offset_x  = 0;
                int offset_y  = 0;
                int offset_cx = 0;
                int offset_cy = 0;

                switch (self.pressed_mouse_button) {
                    case MouseContext::MOUSE_BUTTON_PRESSED_TITLEBAR:
                        offset_x = xPos - self.captured_mouse_x;
                        offset_y = yPos - self.captured_mouse_y;
                    break;

                    case MouseContext::MOUSE_BUTTON_PRESSED_NORTH: {
                        const int offset_y_max = self.window_rect_saved.cy - INTERNAL_MODULE_MINIMUM_WINDOW_HEIGHT;

                        offset_y = std::min(yPos - self.captured_mouse_y, offset_y_max);
                        offset_cy = -offset_y;
                    }
                    break;

                    case MouseContext::MOUSE_BUTTON_PRESSED_NORTHWEST: {
                        const int offset_x_max = self.window_rect_saved.cx - INTERNAL_MODULE_MINIMUM_WINDOW_WIDTH;
                        const int offset_y_max = self.window_rect_saved.cy - INTERNAL_MODULE_MINIMUM_WINDOW_HEIGHT;

                        offset_x = std::min(xPos - self.captured_mouse_x, offset_x_max);
                        offset_cx = -offset_x;

                        offset_y = std::min(yPos - self.captured_mouse_y, offset_y_max);
                        offset_cy = -offset_y;
                    }
                    break;

                    case MouseContext::MOUSE_BUTTON_PRESSED_WEST: {
                        const int offset_x_max = self.window_rect_saved.cx - INTERNAL_MODULE_MINIMUM_WINDOW_WIDTH;

                        offset_x = std::min(xPos - self.captured_mouse_x, offset_x_max);
                        offset_cx = -offset_x;
                    }
                    break;

                    case MouseContext::MOUSE_BUTTON_PRESSED_SOUTHWEST: {
                        const int offset_x_max = self.window_rect_saved.cx - INTERNAL_MODULE_MINIMUM_WINDOW_WIDTH;

                        offset_x = std::min(xPos - self.captured_mouse_x, offset_x_max);
                        offset_cx = -offset_x;

                        const int offset_cy_min = INTERNAL_MODULE_MINIMUM_WINDOW_HEIGHT - self.window_rect_saved.cy;

                        offset_cy = std::max(yPos - self.captured_mouse_y, offset_cy_min);
                    }
                    break;

                    case MouseContext::MOUSE_BUTTON_PRESSED_SOUTH : {
                        const int offset_cy_min = INTERNAL_MODULE_MINIMUM_WINDOW_HEIGHT - self.window_rect_saved.cy;

                        offset_cy = std::max(yPos - self.captured_mouse_y, offset_cy_min);
                    }
                    break;

                    case MouseContext::MOUSE_BUTTON_PRESSED_SOUTHEAST: {
                        const int offset_cy_min = INTERNAL_MODULE_MINIMUM_WINDOW_HEIGHT - self.window_rect_saved.cy;

                        offset_cy = std::max(yPos - self.captured_mouse_y, offset_cy_min);

                        const int offset_cx_min = INTERNAL_MODULE_MINIMUM_WINDOW_WIDTH - self.window_rect_saved.cx;

                        offset_cx = std::max(xPos - self.captured_mouse_x, offset_cx_min);
                    }
                    break;

                    case MouseContext::MOUSE_BUTTON_PRESSED_EAST: {
                        const int offset_cx_min = INTERNAL_MODULE_MINIMUM_WINDOW_WIDTH - self.window_rect_saved.cx;

                        offset_cx = std::max(xPos - self.captured_mouse_x, offset_cx_min);
                    }
                    break;

                    case MouseContext::MOUSE_BUTTON_PRESSED_NORTHEAST: {
                        const int offset_y_max = self.window_rect_saved.cy - INTERNAL_MODULE_MINIMUM_WINDOW_HEIGHT;

                        offset_y = std::min(yPos - self.captured_mouse_y, offset_y_max);
                        offset_cy = -offset_y;

                        const int offset_cx_min = INTERNAL_MODULE_MINIMUM_WINDOW_WIDTH - self.window_rect_saved.cx;

                        offset_cx = std::max(xPos - self.captured_mouse_x, offset_cx_min);
                    }
                    break;
                }

                self.window_rect = self.window_rect_saved;

                self.window_rect.x  += offset_x;
                self.window_rect.y  += offset_y;
                self.window_rect.cx += offset_cx;
                self.window_rect.cy += offset_cy;

                this->update_rects();

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

                const Rect adjusted_window_rect = self.window_rect.offset(this->window_offset_x, this->window_offset_y);

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
        else if (this->allow_resize_hosted_desktop_ &&
                    (MouseContext::MOUSE_BUTTON_PRESSED_RESIZEHOSTEDDESKTOPBOX == self.pressed_mouse_button)) {
            this->draw_resize_hosted_desktop_box(
                self.resize_hosted_desktop_box_rect.contains_pt(xPos, yPos),
                self.resize_hosted_desktop_box_rect);

            this->drawable_.sync();
        }   // else if (MouseContext::MOUSE_BUTTON_PRESSED_MINIMIZEBOX == self.pressed_mouse_button)
        else if (MouseContext::MOUSE_BUTTON_PRESSED_MINIMIZEBOX == self.pressed_mouse_button) {
            if (self.minimize_box_rect.contains_pt(xPos, yPos)) {
                RDPOpaqueRect order(self.minimize_box_rect, encode_color24()(BGRColor{0xCBCACA}));

                this->drawable_.draw(order, self.minimize_box_rect, gdi::ColorCtx::depth24());

                if (this->font_) {
                    gdi::server_draw_text(this->drawable_,
                                            *this->font_,
                                            self.minimize_box_rect.x + 12,
                                            self.minimize_box_rect.y + 3,
                                            "−",
                                            encode_color24()(BLACK),
                                            encode_color24()(BGRColor{0xCBCACA}),
                                            gdi::ColorCtx::depth24(),
                                            self.minimize_box_rect
                                            );
                }

                this->drawable_.sync();
            }
            else {
                RDPOpaqueRect order(self.minimize_box_rect, encode_color24()(WHITE));

                this->drawable_.draw(order, self.minimize_box_rect, gdi::ColorCtx::depth24());

                if (this->font_) {
                    gdi::server_draw_text(this->drawable_,
                                            *this->font_,
                                            self.minimize_box_rect.x + 12,
                                            self.minimize_box_rect.y + 3,
                                            "−",
                                            encode_color24()(BLACK),
                                            encode_color24()(WHITE),
                                            gdi::ColorCtx::depth24(),
                                            self.minimize_box_rect
                                            );
                }

                this->drawable_.sync();
            }
        }   // else if (MouseContext::MOUSE_BUTTON_PRESSED_MINIMIZEBOX == self.pressed_mouse_button)
        else if (MouseContext::MOUSE_BUTTON_PRESSED_MAXIMIZEBOX == self.pressed_mouse_button) {
            this->draw_maximize_box(self.maximize_box_rect.contains_pt(xPos, yPos), self.maximize_box_rect);

            this->drawable_.sync();
        }   // else if (MouseContext::MOUSE_BUTTON_PRESSED_MINIMIZEBOX == self.pressed_mouse_button)
        else if (MouseContext::MOUSE_BUTTON_PRESSED_CLOSEBOX == self.pressed_mouse_button) {
            if (self.close_box_rect.contains_pt(xPos, yPos)) {
                RDPOpaqueRect order(self.close_box_rect, encode_color24()(BGRColor{0x2311E8}));

                this->drawable_.draw(order, self.close_box_rect, gdi::ColorCtx::depth24());

                if (this->font_) {
                    gdi::server_draw_text(this->drawable_,
                                            *this->font_,
                                            self.close_box_rect.x + 13,
                                            self.close_box_rect.y + 3,
                                            "x",
                                            encode_color24()(WHITE),
                                            encode_color24()(BGRColor{0x2311E8}),
                                            gdi::ColorCtx::depth24(),
                                            self.close_box_rect
                                            );
                }

                this->drawable_.sync();
            }
            else {
                RDPOpaqueRect order(self.close_box_rect, encode_color24()(WHITE));

                this->drawable_.draw(order, self.close_box_rect, gdi::ColorCtx::depth24());

                if (this->font_) {
                    gdi::server_draw_text(this->drawable_,
                                            *this->font_,
                                            self.close_box_rect.x + 13,
                                            self.close_box_rect.y + 3,
                                            "x",
                                            encode_color24()(BLACK),
                                            encode_color24()(WHITE),
                                            gdi::ColorCtx::depth24(),
                                            self.close_box_rect
                                            );
                }

                this->drawable_.sync();
            }
        }   // else if (MouseContext::MOUSE_BUTTON_PRESSED_CLOSEBOX == self.pressed_mouse_button)
    }   // else if (SlowPath::PTRFLAGS_MOVE == pointerFlags)
    else if (SlowPath::PTRFLAGS_BUTTON1 == pointerFlags) {
        if (this->allow_resize_hosted_desktop_ && (MouseContext::MOUSE_BUTTON_PRESSED_RESIZEHOSTEDDESKTOPBOX == self.pressed_mouse_button)) {
            self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_NONE;

            this->enable_resizing_hosted_desktop_ = (!this->enable_resizing_hosted_desktop_);

            this->draw_resize_hosted_desktop_box(false, self.resize_hosted_desktop_box_rect);

            this->drawable_.sync();

            if (this->enable_resizing_hosted_desktop_) {
                this->update_widget();
            }
        }   // if (this->allow_resize_hosted_desktop_ &&
        else if (MouseContext::MOUSE_BUTTON_PRESSED_MINIMIZEBOX == self.pressed_mouse_button) {
            self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_NONE;

            {
                RDPOpaqueRect order(self.minimize_box_rect, encode_color24()(WHITE));

                this->drawable_.draw(order, self.minimize_box_rect, gdi::ColorCtx::depth24());

                if (this->font_) {
                    gdi::server_draw_text(this->drawable_,
                                            *this->font_,
                                            self.minimize_box_rect.x + 12,
                                            self.minimize_box_rect.y + 3,
                                            "−",
                                            encode_color24()(BLACK),
                                            encode_color24()(WHITE),
                                            gdi::ColorCtx::depth24(),
                                            self.minimize_box_rect
                                            );
                }

                this->drawable_.sync();
            }

            if (self.minimize_box_rect.contains_pt(xPos, yPos)) {
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
                                self.window_rect.x,
                                self.window_rect.y,
                                self.window_rect.x + self.window_rect.cx,
                                self.window_rect.y + self.window_rect.cy
                            ));
                }
            }
        }   // if (MouseContext::MOUSE_BUTTON_PRESSED_MINIMIZEBOX == self.pressed_mouse_button)
        else if (MouseContext::MOUSE_BUTTON_PRESSED_MAXIMIZEBOX == self.pressed_mouse_button) {
            self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_NONE;

            this->draw_maximize_box(false, self.maximize_box_rect);

            this->drawable_.sync();

            if (self.maximize_box_rect.contains_pt(xPos, yPos)) {
                this->maximize_restore_window();
            }
        }   // else if (MouseContext::MOUSE_BUTTON_PRESSED_MAXIMIZEBOX == self.pressed_mouse_button)
        else if (MouseContext::MOUSE_BUTTON_PRESSED_CLOSEBOX == self.pressed_mouse_button) {
            self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_NONE;

            {
                RDPOpaqueRect order(self.close_box_rect, encode_color24()(WHITE));

                this->drawable_.draw(order, self.close_box_rect, gdi::ColorCtx::depth24());

                if (this->font_) {
                    gdi::server_draw_text(this->drawable_,
                                            *this->font_,
                                            self.close_box_rect.x + 13,
                                            self.close_box_rect.y + 3,
                                            "x",
                                            encode_color24()(BLACK),
                                            encode_color24()(WHITE),
                                            gdi::ColorCtx::depth24(),
                                            self.close_box_rect
                                            );
                }

                this->drawable_.sync();
            }

            if (self.close_box_rect.contains_pt(xPos, yPos)) {
                LOG(LOG_INFO, "ClientExecute::input_mouse: Close by user (Close Box)");
                throw Error(ERR_WIDGET);    // Close Box pressed
            }
        }   // else if (MouseContext::MOUSE_BUTTON_PRESSED_CLOSEBOX == self.pressed_mouse_button)
        else if ((MouseContext::MOUSE_BUTTON_PRESSED_NONE != self.pressed_mouse_button) &&
                    !this->maximized) {
            if (MouseContext::MOUSE_BUTTON_PRESSED_TITLEBAR == self.pressed_mouse_button) {
                LOG_IF(this->verbose, LOG_INFO, "ClientExecute::input_mouse: Mouse button 1 released from title bar");

                int const diff_x = (xPos - self.captured_mouse_x);
                int const diff_y = (yPos - self.captured_mouse_y);

                self.window_rect.x = self.window_rect_saved.x + diff_x;
                self.window_rect.y = self.window_rect_saved.y + diff_y;

                this->update_rects();
            }   // if (MouseContext::MOUSE_BUTTON_PRESSED_TITLEBAR == self.pressed_mouse_button)

            int move_size_type = 0;
            switch (self.pressed_mouse_button) {
                case MouseContext::MOUSE_BUTTON_PRESSED_NORTH:     move_size_type = RAIL_WMSZ_TOP;         break;
                case MouseContext::MOUSE_BUTTON_PRESSED_NORTHWEST: move_size_type = RAIL_WMSZ_TOPLEFT;     break;
                case MouseContext::MOUSE_BUTTON_PRESSED_WEST:      move_size_type = RAIL_WMSZ_LEFT;        break;
                case MouseContext::MOUSE_BUTTON_PRESSED_SOUTHWEST: move_size_type = RAIL_WMSZ_BOTTOMLEFT;  break;
                case MouseContext::MOUSE_BUTTON_PRESSED_SOUTH:     move_size_type = RAIL_WMSZ_BOTTOM;      break;
                case MouseContext::MOUSE_BUTTON_PRESSED_SOUTHEAST: move_size_type = RAIL_WMSZ_BOTTOMRIGHT; break;
                case MouseContext::MOUSE_BUTTON_PRESSED_EAST:      move_size_type = RAIL_WMSZ_RIGHT;       break;
                case MouseContext::MOUSE_BUTTON_PRESSED_NORTHEAST: move_size_type = RAIL_WMSZ_TOPRIGHT;    break;
                case MouseContext::MOUSE_BUTTON_PRESSED_TITLEBAR:  move_size_type = RAIL_WMSZ_MOVE;        break;
            }

            if (0 != move_size_type) {
                StaticOutStream<256> out_s;
                RAILPDUHeader header;
                header.emit_begin(out_s, TS_RAIL_ORDER_LOCALMOVESIZE);

                const Rect adjusted_window_rect = self.window_rect.offset(this->window_offset_x, this->window_offset_y);

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

                self.move_size_initialized = false;
            }   // if (0 != move_size_type)

            const Rect adjusted_window_rect = self.window_rect.offset(this->window_offset_x, this->window_offset_y);

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

            if (MouseContext::MOUSE_BUTTON_PRESSED_TITLEBAR == self.pressed_mouse_button) {
                this->update_widget();
            }   // if (MouseContext::MOUSE_BUTTON_PRESSED_TITLEBAR == self.pressed_mouse_button)

            this->on_new_or_existing_window(adjusted_window_rect);

            if (0 != move_size_type) {
                self.pressed_mouse_button = MouseContext::MOUSE_BUTTON_PRESSED_NONE;
            }
        }   // else if ((MouseContext::MOUSE_BUTTON_PRESSED_NONE != self.pressed_mouse_button) &&
    }   // else if (SlowPath::PTRFLAGS_BUTTON1 == pointerFlags)
    else if (PTRFLAGS_EX_DOUBLE_CLICK == pointerFlags) {
        if ((self.north.contains_pt(xPos, yPos) || self.south.contains_pt(xPos, yPos)) && !this->maximized) {
            Rect work_area_rect = this->get_current_work_area_rect();

            self.window_rect.y  = 0;
            self.window_rect.cy = work_area_rect.cy - 1;

            this->update_rects();

            const Rect adjusted_window_rect = self.window_rect.offset(this->window_offset_x, this->window_offset_y);

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
        }   // if (self.south.contains_pt(xPos, yPos))
        else if (self.title_bar_rect.contains_pt(xPos, yPos)) {
            this->maximize_restore_window();
        }   // else if (self.title_bar_rect.contains_pt(xPos, yPos))
        else if (self.title_bar_icon_rect.contains_pt(xPos, yPos)) {
            LOG(LOG_INFO, "ClientExecute::input_mouse: Close by user (Title Bar Icon)");
            throw Error(ERR_WIDGET);    // Title Bar Icon Double-clicked
        }   // else if (self.title_bar_icon_rect.contains_pt(xPos, yPos))
    }   // else if (PTRFLAGS_EX_DOUBLE_CLICK == pointerFlags)

    return false;
}   // input_mouse

