/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "capture/rail_screen_computation.hpp"
#include "core/window_constants.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"


struct RailScreenVisibility::WindowRecord
{
    uint32_t window_id;
    uint32_t fields_present_flags;
    uint32_t style;
    uint8_t show_state;
    int32_t visible_offset_x;
    int32_t visible_offset_y;

    std::string title_info;
};

struct RailScreenVisibility::WindowVisibilityRectRecord
{
    uint32_t window_id;
    Rect rect;
};

template<class C>
static void erase_window_id_from_visibility_rect(C& c, const uint32_t window_id)
{
    c.erase(
        std::remove_if(
            c.begin(),
            c.end(),
            [window_id](auto& window_visibility_rect) {
                return (window_visibility_rect.window_id == window_id);
            }
        ),
        c.end()
    );
}

RailScreenVisibility::RailScreenVisibility(bool verbose) noexcept
: verbose(verbose)
{}

RailScreenVisibility::~RailScreenVisibility() = default;

void RailScreenVisibility::update(const RDP::RAIL::NewOrExistingWindow & cmd)
{
    if (REDEMPTION_UNLIKELY(this->verbose)) {
        LOG(LOG_INFO, "RailScreenVisibility::update(NewOrExistingWindow): ...");
        cmd.log(LOG_INFO);
    }

    uint32_t const fields_present_flags = cmd.header.FieldsPresentFlags();
    uint32_t const window_id            = cmd.header.WindowId();
    uint32_t const style                = cmd.Style();
    uint8_t  const show_state           = cmd.ShowState();
    int32_t  const visible_offset_x     = cmd.VisibleOffsetX();
    int32_t  const visible_offset_y     = cmd.VisibleOffsetY();
    std::string const& title_info       = cmd.TitleInfo();

    const auto flag_mask = ( RDP::RAIL::WINDOW_ORDER_FIELD_STYLE
                           | RDP::RAIL::WINDOW_ORDER_FIELD_SHOW
                           | RDP::RAIL::WINDOW_ORDER_FIELD_TITLE
                           | RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET
                           );

    if (fields_present_flags & flag_mask) {
        auto iter = std::find_if(
            this->windows.begin(),
            this->windows.end(),
            [window_id](WindowRecord& window) -> bool {
                return (window.window_id == window_id);
            }
        );
        if (iter != this->windows.end()) {
            if (fields_present_flags & RDP::RAIL::WINDOW_ORDER_FIELD_STYLE) {
                iter->style = style;
                iter->fields_present_flags |= RDP::RAIL::WINDOW_ORDER_FIELD_STYLE;
            }
            if (fields_present_flags & RDP::RAIL::WINDOW_ORDER_FIELD_SHOW) {
                iter->show_state = show_state;
                iter->fields_present_flags |= RDP::RAIL::WINDOW_ORDER_FIELD_SHOW;
            }
            if (fields_present_flags & RDP::RAIL::WINDOW_ORDER_FIELD_TITLE) {
                iter->title_info = title_info;
                iter->fields_present_flags |= RDP::RAIL::WINDOW_ORDER_FIELD_TITLE;
            }
            if (fields_present_flags & RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET) {
                iter->visible_offset_x = visible_offset_x;
                iter->visible_offset_y = visible_offset_y;
                iter->fields_present_flags |= RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET;
            }
        }
        else {
            this->windows.emplace_back(WindowRecord{
                window_id,
                (fields_present_flags & flag_mask),
                style, show_state, visible_offset_x, visible_offset_y,
                title_info
            });
        }
    }

    if (fields_present_flags & RDP::RAIL::WINDOW_ORDER_FIELD_VISIBILITY) {
        erase_window_id_from_visibility_rect(this->window_visibility_rects, window_id);

        for (auto const& rect : cmd.VisibilityRects()) {
            this->window_visibility_rects.emplace_back(
                WindowVisibilityRectRecord{window_id, rect});
        }
    }

    LOG_IF(this->verbose, LOG_INFO, "RailScreenVisibility::update(NewOrExistingWindow): Done.");
}

void RailScreenVisibility::update(const RDP::RAIL::DeletedWindow & cmd)
{
    if (REDEMPTION_UNLIKELY(this->verbose)) {
        LOG(LOG_INFO, "RailScreenVisibility::update(DeletedWindow): ...");
        cmd.log(LOG_INFO);
    }

    uint32_t const window_id = cmd.header.WindowId();

    auto it = std::find_if(
        this->windows.begin(),
        this->windows.end(),
        [window_id](WindowRecord const& window) {
            return (window.window_id == window_id);
        }
    );

    if (it != this->windows.end()) {
        LOG_IF(this->verbose, LOG_INFO,
            "RailScreenVisibility::update(DeletedWindow): Title=\"%s\"",
            it->title_info);
        *it = std::move(this->windows.back());
        this->windows.pop_back();
    }

    erase_window_id_from_visibility_rect(this->window_visibility_rects, window_id);

    LOG_IF(this->verbose, LOG_INFO, "RailScreenVisibility::update(DeletedWindow): Done.");
}

Rect RailScreenVisibility::get_join_visibility_rect() const
{
    LOG_IF(this->verbose, LOG_INFO, "Capture::get_join_visibility_rect(): ...");

    Rect join_visibility_rect;

    for (const WindowRecord& window : this->windows) {
        for (const WindowVisibilityRectRecord& window_visibility_rect : this->window_visibility_rects) {
            if (window.window_id != window_visibility_rect.window_id) {
                continue;
            }

            assert(window.fields_present_flags & RDP::RAIL::WINDOW_ORDER_FIELD_VISOFFSET);

            char state = ' ';

            if (// Window is not IME icon.
                window.title_info != std::string_view{"CiceroUIWndFrame-TF_FloatingLangBar_WndTitle"}
             && (
                 ( (window.style & (WS_DISABLED | WS_SYSMENU | WS_VISIBLE))
                && !(window.style & WS_ICONIC)
                && (window.show_state != SW_FORCEMINIMIZE)
                && (window.show_state != SW_HIDE)
                && (window.show_state != SW_MINIMIZE)
                )
             || // window is created before recording starts
                (!window.style && !window.show_state)
             )
            ) {
                state = '+';
                join_visibility_rect = join_visibility_rect.disjunct(
                    window_visibility_rect.rect.offset(
                        window.visible_offset_x, window.visible_offset_y));
            }

            LOG_IF(this->verbose, LOG_INFO,
                "VisibilityRect::get_join_visibility_rect():"
                " %c Title=\"%s\" Rect=%s ShowState=0x%X Style=0x%X",
                state,
                window.title_info,
                window_visibility_rect.rect.offset(
                    window.visible_offset_x, window.visible_offset_y),
                window.show_state, window.style);
        }
    }

    LOG_IF(this->verbose, LOG_INFO, "Capture::get_join_visibility_rect(): Done.");

    return join_visibility_rect;
}


void RailScreenComputation::visibility_rect_event(Rect rect)
{
    if (!rect.isempty()) {
        this->update_rects(rect);
    }
}

void RailScreenComputation::resize(uint16_t width, uint16_t height)
{
    this->width = width;
    this->height = height;
}

void RailScreenComputation::draw(const RDP::RAIL::NewOrExistingWindow & cmd)
{
    this->screen.update(cmd);
    this->update_rects(this->screen.get_join_visibility_rect());
}

void RailScreenComputation::draw(const RDP::RAIL::DeletedWindow & cmd)
{
    this->screen.update(cmd);
    this->update_rects(this->screen.get_join_visibility_rect());
}

void RailScreenComputation::draw(const RDP::RAIL::NonMonitoredDesktop & cmd)
{
    (void)cmd;
    this->update_rects(Rect(0, 0, this->width, this->height));
}

void RailScreenComputation::update_rects(Rect rect)
{
    this->max_image_frame_rect = this->max_image_frame_rect.disjunct(rect);

    this->min_image_frame_dim.w = std::max(this->min_image_frame_dim.w, rect.cx);
    this->min_image_frame_dim.h = std::max(this->min_image_frame_dim.h, rect.cy);
}
