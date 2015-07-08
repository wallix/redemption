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
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Meng Tan, Raphael Zhou
*/

#ifndef WidgetTabREDEMPTION_MOD_WIDGET2_TAB_HPP
#define WidgetTabREDEMPTION_MOD_WIDGET2_TAB_HPP

#include "widget.hpp"
#include "composite.hpp"
#include "RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"

class WidgetTab : public Widget2
{
public:
    class Item;

    class DrawingPolicy {
    protected:
        DrawApi & drawable;

    private:
        uint32_t bg_color;
        uint32_t fg_color;

    public:
        explicit DrawingPolicy(DrawApi & drawable) : drawable(drawable), bg_color(BLACK), fg_color(WHITE) {}

        virtual ~DrawingPolicy() {}

        uint32_t get_bg_color() { return this->bg_color; }
        uint32_t get_fg_color() { return this->fg_color; }
        void set_color(uint32_t bg_color, uint32_t fg_color) {
            this->bg_color = bg_color;
            this->fg_color = fg_color;
        }

        virtual void draw( const Rect & rect_tab
                         , const Rect & clip
                         , Item ** items
                         , size_t item_count
                         , size_t current_item_index
                         , bool draw_focus) = 0;

        virtual Rect get_child_area(const Rect & rect_tab) = 0;

        virtual void process_input_mouse( WidgetTab & tab
                                        , const Rect & rect_tab
                                        , WidgetTab::Item ** items
                                        , size_t item_count
                                        , size_t current_item_index
                                        , int device_flags
                                        , int x
                                        , int y
                                        , Keymap2 * keymap) = 0;
    };

    DrawingPolicy & drawing_policy;

public:
    class Item : public WidgetParent {
    private:
        enum {
            MAX_TEXT = 256
        };

        char text[MAX_TEXT];

//        DrawingPolicy & drawing_policy;

        CompositeArray child_array;

    public:
        Item( const char * text
            , DrawApi & drawable
//            , DrawingPolicy & drawing_policy
            , const Rect & rect
            , Widget2 & parent
            , NotifyApi * notifier)
        : WidgetParent(drawable, rect, parent, notifier)
//        , drawing_policy(drawing_policy)
 {
            this->impl = &child_array;

            this->set_text(text);
        }

        const char * get_text() const {
            return this->text;
        }
        void set_text(const char * text) {
            this->text[0] = 0;
            if (text) {
                const size_t max = std::min<int>(MAX_TEXT - 1, strlen(text));
                memcpy(this->text, text, max);
                this->text[max] = 0;
            }
        }
    };

private:
    enum {
        MAX_ITEM_COUNT = 128
    };

    Item   * items[MAX_ITEM_COUNT];
    size_t   item_count;
    size_t   current_item_index;

public:
    bool child_has_focus;

public:
    WidgetTab( DrawApi & drawable, DrawingPolicy & drawing_policy, int16_t x, int16_t y
             , uint16_t cx, uint16_t cy, Widget2 & parent
             , NotifyApi * notifier
             , int group_id, int fgcolor, int bgcolor)
    : Widget2(drawable, Rect(x, y, cx, cy), parent, notifier, group_id)
    , drawing_policy(drawing_policy)
    , items()
    , item_count(0)
    , current_item_index(0)
    , child_has_focus(false) {
        this->drawing_policy.set_color(bgcolor, fgcolor);
    }

    ~WidgetTab() override {
        this->clear();
    }

    size_t add_item(const char * name) {
        REDASSERT(name && (*name));
        REDASSERT(this->item_count < MAX_ITEM_COUNT);
        Rect rect_child_area  = this->drawing_policy.get_child_area(this->rect);
        Rect rect_item        = rect_child_area.offset(-this->rect.x, -this->rect.y);

        this->items[this->item_count] = new Item( name, this->drawable
//                                                , this->drawing_policy
                                                , rect_item
                                                , *this
                                                , this);

        this->items[this->item_count]->set_bg_color(this->drawing_policy.get_bg_color());

        this->refresh(this->rect);

        return this->item_count++;
    }
    WidgetParent & get_item(size_t item_index) {
        REDASSERT(this->item_count > item_index);
        return *(this->items[item_index]);
    }

    void add_widget(size_t item_index, Widget2 * w) {
        REDASSERT(this->item_count > item_index);
        this->items[item_index]->add_widget(w);
    }

    void blur() override {
        if (this->has_focus) {
            this->has_focus = false;
            this->send_notify(NOTIFY_FOCUS_END);
            if (this->item_count) {
                this->items[this->current_item_index]->blur();
            }
            this->refresh(this->rect);
        }
    }
    void focus(int reason) override {
        if (!this->has_focus) {
            this->has_focus = true;
            this->send_notify(NOTIFY_FOCUS_BEGIN);

/*
            if (reason == focus_reason_tabkey) {
                this->child_has_focus = false;
            }
            else if (this->item_count) {
                this->child_has_focus = true;
                this->items[this->current_item_index]->focus(reason);
            }
*/
            if (reason == focus_reason_backtabkey) {
                if (this->item_count) {
                    if (!this->items[this->current_item_index]->get_previous_focus(nullptr, false)) {
                        this->child_has_focus = false;
                    }
                    else {
                        this->child_has_focus = true;
                        this->items[this->current_item_index]->focus(reason);
                    }
                }
            }
            else {
                this->child_has_focus = false;
            }
            this->refresh(this->rect);
        }
    }

    void clear() {
        for (size_t i = 0; i < item_count; i++) {
            REDASSERT(this->items[i]);
            delete this->items[i];
        }
        this->item_count = 0;
    }

    void draw(const Rect & clip) override {
        this->drawable.begin_update();
        this->drawing_policy.draw( this->rect
                                 , clip
                                 , this->items
                                 , this->item_count
                                 , this->current_item_index
                                 , this->has_focus && (!this->child_has_focus)
                                 );

        if (this->item_count) {
            this->items[this->current_item_index]->draw_children(rect);
        }
        this->drawable.end_update();
    }

    bool next_focus() override {
        if (!this->child_has_focus) {
            if (this->item_count) {
                if (!this->items[this->current_item_index]->get_next_focus(nullptr, false)) {
                    return false;
                }
                this->child_has_focus = true;
                this->items[this->current_item_index]->focus(focus_reason_tabkey);
                this->refresh(this->rect);
                return true;
            }
        }

        if (this->item_count) {
            REDASSERT(this->item_count > this->current_item_index);
            return this->items[this->current_item_index]->next_focus();
        }

        return false;
    }
    bool previous_focus() override {
        if (this->child_has_focus) {
            REDASSERT(this->item_count);
            REDASSERT(this->item_count > this->current_item_index);
            if (this->items[this->current_item_index]->previous_focus()) {
                return true;
            }

            this->child_has_focus = false;
            this->refresh(this->rect);
            return true;
        }

        return false;
    }

    void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) override {
        Widget2 * w = this->widget_at_pos(x, y);

        if (w == this) {
            this->drawing_policy.process_input_mouse( *this
                                                    , this->rect
                                                    , this->items
                                                    , this->item_count
                                                    , this->current_item_index
                                                    , device_flags
                                                    , x
                                                    , y
                                                    , keymap);
            return;
        }

        if (this->item_count) {
            REDASSERT(this->item_count > this->current_item_index);
            return this->items[this->current_item_index]->rdp_input_mouse(device_flags, x, y, keymap);
        }
    }

    void rdp_input_scancode( long param1, long param2, long param3
                                   , long param4, Keymap2 * keymap) override {
        if (keymap->nb_kevent_available() > 0) {
            if (this->child_has_focus && this->item_count) {
                REDASSERT(this->item_count > this->current_item_index);
                this->items[this->current_item_index]->rdp_input_scancode(param1, param2, param3, param4, keymap);
            }
            else {
                switch (keymap->top_kevent()) {
                case Keymap2::KEVENT_LEFT_ARROW:
                    keymap->get_kevent();
                    if (this->current_item_index > 0) {
                        this->current_item_index--;
                    }
                    else {
                        this->current_item_index = (this->item_count ? (this->item_count - 1) : 0);
                    }
                    this->refresh(this->rect);
                    break;
                case Keymap2::KEVENT_RIGHT_ARROW:
                    keymap->get_kevent();
                    this->current_item_index++;
                    if (this->current_item_index >= this->item_count) {
                        this->current_item_index = 0;
                    }
                    this->refresh(this->rect);
                    break;
                }
            }
        }
    }

    void set_current_item(size_t item_index) {
        REDASSERT(this->item_count > item_index);
        if (this->current_item_index != item_index) {
            this->current_item_index = item_index;

            this->refresh(this->rect);
        }
    }

    Widget2 * widget_at_pos(int16_t x, int16_t y) override {
        if (this->item_count) {
            REDASSERT(this->item_count > this->current_item_index);
            if (Widget2 * w = this->items[this->current_item_index]->widget_at_pos(x, y)) {
                return w;
            }
        }
        return Widget2::widget_at_pos(x, y);
    }

    void notify(Widget2 * w, NotifyApi::notify_event_t event) override {
        if (event == NOTIFY_FOCUS_BEGIN) {
            this->child_has_focus = true;
            this->refresh(this->rect);
        }
        Widget2::notify(w, event);
    }
};

class WidgetTabDPDefault : public WidgetTab::DrawingPolicy {
private:
    enum {
          first_item_index_offset_left = 2
        , selection_marker_height      = 2
        , border_width_height          = 1
        , text_padding_x               = 5
        , text_padding_y               = 2
    };

    uint16_t   text_height;
    uint16_t   item_index_height;

    Font const & font;

public:
    WidgetTabDPDefault(DrawApi & drawable, Font const & font)
    : WidgetTab::DrawingPolicy(drawable), text_height(0), item_index_height(0)
    , font(font) {
        int w, h;
        this->drawable.text_metrics(this->font, "bp", w, h);

        this->text_height       = h;
        this->item_index_height =   border_width_height
                                  + selection_marker_height
                                  + border_width_height
                                  + text_padding_y
                                  + this->text_height
                                  + text_padding_y;
    }

    void draw( const Rect & rect_tab
                     , const Rect & clip
                     , WidgetTab::Item ** items
                     , size_t item_count
                     , size_t current_item_index
                     , bool draw_focus) override {
        this->drawable.begin_update();
        this->draw_opaque_rect(rect_tab, this->get_bg_color(), clip);

        uint16_t item_index_offset = first_item_index_offset_left;
        for (size_t item_index = 0; item_index < item_count; item_index++) {
            WidgetTab::Item * item = items[item_index];

            int text_width, h;
            this->drawable.text_metrics(this->font, item->get_text(), text_width, h);

            uint16_t item_index_width =   border_width_height
                                        + text_padding_x
                                        + text_width
                                        + text_padding_x;

            this->drawable.draw(
                  RDPOpaqueRect( Rect( rect_tab.x + item_index_offset + border_width_height
                                     , rect_tab.y + border_width_height
                                     , item_index_width - border_width_height
                                     ,   this->item_index_height
                                       - ((current_item_index == item_index) ? 0 : border_width_height))
                               , this->get_bg_color())
                , clip);

            // Border.
            BStream deltaPoints(256);

            deltaPoints.out_sint16_le(item_index_width);
            deltaPoints.out_sint16_le(0);

            deltaPoints.out_sint16_le(0);
            deltaPoints.out_sint16_le(this->item_index_height - ((current_item_index != item_index) ? 1 : 0));

            if (current_item_index != item_index) {
                deltaPoints.out_sint16_le(-(item_index_width - 1));
                deltaPoints.out_sint16_le(0);
            }

            deltaPoints.mark_end();
            deltaPoints.rewind();

            RDPPolyline polyline_box( rect_tab.x + item_index_offset
                                    , rect_tab.y + ((current_item_index != item_index) ? 1 : 0)
                                    , 0x0D, 0, this->get_fg_color(), ((current_item_index != item_index) ? 3 : 2)
                                    , deltaPoints);
            this->drawable.draw(polyline_box, clip);

            if (draw_focus && (current_item_index == item_index)) {
                this->drawable.draw(RDPLineTo( 1
                                             , rect_tab.x + item_index_offset + border_width_height
                                             , rect_tab.y + border_width_height * 2
                                             , rect_tab.x + item_index_offset + border_width_height + item_index_width - border_width_height * 2
                                             , rect_tab.y + border_width_height * 2
                                             , 0
                                             , 13
                                             , RDPPen(0, 1, this->get_fg_color())
                                             )
                    , clip);
            }

            // Text.
            this->drawable.server_draw_text(
                  this->font
                , rect_tab.x + item_index_offset + border_width_height + text_padding_x
                , rect_tab.y + border_width_height + selection_marker_height + border_width_height + text_padding_y
                , item->get_text()
                , this->get_fg_color()
                , this->get_bg_color()
                , clip);


            item_index_offset += item_index_width;
        }

        this->draw_opaque_rect( Rect( rect_tab.x + border_width_height
                              , rect_tab.y + this->item_index_height + border_width_height
                              , rect_tab.cx - border_width_height * 2
                              , rect_tab.cy - this->item_index_height - border_width_height * 2)
            , this->get_bg_color(), clip);

        BStream deltaPoints(256);

        deltaPoints.out_sint16_le(0);
        deltaPoints.out_sint16_le(this->item_index_height - border_width_height);

        deltaPoints.out_sint16_le(-first_item_index_offset_left);
        deltaPoints.out_sint16_le(0);

        deltaPoints.out_sint16_le(0);
        deltaPoints.out_sint16_le(rect_tab.cy - this->item_index_height - border_width_height);

        deltaPoints.out_sint16_le(rect_tab.cx - border_width_height);
        deltaPoints.out_sint16_le(0);

        deltaPoints.out_sint16_le(0);
        deltaPoints.out_sint16_le(-(rect_tab.cy - this->item_index_height - border_width_height));

        deltaPoints.out_sint16_le(-(rect_tab.cx - item_index_offset - border_width_height * 2));
        deltaPoints.out_sint16_le(0);

        deltaPoints.mark_end();
        deltaPoints.rewind();

        RDPPolyline polyline_box( rect_tab.x + first_item_index_offset_left
                                , rect_tab.y + border_width_height
                                , 0x0D, 0, this->get_fg_color(), 6
                                , deltaPoints);
        this->drawable.draw(polyline_box, clip);

        this->drawable.end_update();
    }

private:
    void draw_opaque_rect(const Rect & rect, int bg_color, const Rect & clip) {
        const uint16_t tile_width_height = 32;

        for (int x_begin = rect.x, x_end = rect.x + rect.cx; x_begin < x_end; x_begin += tile_width_height) {
            uint16_t cx = x_end - x_begin;
            if (cx > tile_width_height) {
                cx = tile_width_height;
            }
            for (int y_begin = rect.y, y_end = rect.y + rect.cy; y_begin < y_end; y_begin += tile_width_height) {
                uint16_t cy = y_end - y_begin;
                if (cy > tile_width_height) {
                    cy = tile_width_height;
                }

                this->drawable.draw(
                      RDPOpaqueRect( Rect( x_begin
                                         , y_begin
                                         , cx
                                         , cy)
                                   , bg_color)
                    , clip);
            }
        }
    }
public:

    Rect get_child_area(const Rect & rect_tab) override {
        return Rect( rect_tab.x + border_width_height
                   , rect_tab.y + this->item_index_height + border_width_height
                   , rect_tab.cx - border_width_height * 2
                   , rect_tab.cy - this->item_index_height - border_width_height * 2);
    }

    void process_input_mouse( WidgetTab & tab
                                    , const Rect & rect_tab
                                    , WidgetTab::Item ** items
                                    , size_t item_count
                                    , size_t current_item_index
                                    , int device_flags
                                    , int x
                                    , int y
                                    , Keymap2 * keymap) override {
        uint16_t item_index_offset = first_item_index_offset_left;
        for (size_t item_index = 0; item_index < item_count; item_index++) {
            WidgetTab::Item * item = items[item_index];

            int text_width, h;
            this->drawable.text_metrics(this->font, item->get_text(), text_width, h);

            uint16_t item_index_width =   border_width_height
                                        + text_padding_x
                                        + text_width
                                        + text_padding_x;

            Rect rect_index = rect_tab.intersect(Rect( rect_tab.x + item_index_offset
                                                     , rect_tab.y
                                                     , item_index_width
                                                     , this->item_index_height));

            if (rect_index.contains_pt(x, y)) {
                if (device_flags & (MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN)) {
/*
                    if (tab.child_has_focus) {
                        items[current_item_index]->blur();
                    }
*/
                    tab.blur();

                    tab.child_has_focus = false;
                    tab.set_current_item(item_index);
                    tab.focus(Widget2::focus_reason_mousebutton1);
                }

                return;
            }

            item_index_offset += item_index_width;
        }
    }
};

#endif  // #ifndef WidgetTabREDEMPTION_MOD_WIDGET2_TAB_HPP
