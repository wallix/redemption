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

#include "composite.hpp"

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
        DrawingPolicy(DrawApi & drawable) : drawable(drawable), bg_color(BLACK), fg_color(WHITE) {}

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
                         , size_t current_item_index) = 0;

        virtual Rect get_child_area(const Rect & rect_tab) = 0;

        virtual void process_input_mouse( WidgetTab & tab
                                        , const Rect & rect_tab
                                        , WidgetTab::Item ** items
                                        , size_t item_count
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

        DrawingPolicy & drawing_policy;

        CompositeArray child_array;

    public:
        Item( const char * text
            , DrawApi & drawable
            , DrawingPolicy & drawing_policy
            , const Rect & rect
            , Widget2 & parent
            , NotifyApi * notifier)
        : WidgetParent(drawable, rect, parent, notifier)
        , drawing_policy(drawing_policy) {
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
    WidgetTab( DrawApi & drawable, DrawingPolicy & drawing_policy, int16_t x, int16_t y
             , uint16_t cx, uint16_t cy, Widget2 & parent
             , NotifyApi * notifier
             , int group_id, int fgcolor, int bgcolor)
    : Widget2(drawable, Rect(x, y, cx, cy), parent, notifier, group_id)
    , drawing_policy(drawing_policy)
    , items()
    , item_count(0)
    , current_item_index(0) {
        this->drawing_policy.set_color(bgcolor, fgcolor);
    }

    virtual ~WidgetTab() {
        this->clear();
    }

    size_t add_item(const char * name) {
        REDASSERT(name && (*name));
        REDASSERT(this->item_count < MAX_ITEM_COUNT);
        Rect rect_child_area  = this->drawing_policy.get_child_area(this->rect);
        Rect rect_item        = rect_child_area.offset(-this->rect.x, -this->rect.y);

        this->items[this->item_count] = new Item( name, this->drawable
                                                , this->drawing_policy
                                                , rect_item
                                                , *this
                                                , this->notifier);
        return this->item_count++;
    }
    Widget2 & get_item(size_t item_index) {
        REDASSERT(this->item_count > item_index);
        return *(this->items[item_index]);
    }

    void add_widget(size_t item_index, Widget2 * w) {
        REDASSERT(this->item_count > item_index);
        this->items[item_index]->add_widget(w);
    }

    virtual void blur() {
        if (this->has_focus) {
            this->has_focus = false;
            this->send_notify(NOTIFY_FOCUS_END);
/*
            if (this->current_focus) {
                this->current_focus->blur();
            }
*/
            this->refresh(this->rect);
        }
    }
    virtual void focus() {
        if (!this->has_focus) {
            this->has_focus = true;
            this->send_notify(NOTIFY_FOCUS_BEGIN);
/*
            if (this->current_focus) {
                this->current_focus->focus();
            }
*/
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

    virtual void draw(const Rect & clip) {
        this->drawing_policy.draw( this->rect
                                 , clip
                                 , this->items
                                 , this->item_count
                                 , this->current_item_index);
/*
        Rect rect_intersect = clip.intersect(this->drawing_policy.get_child_area(this->rect));

        if (this->item_count > 0) {
            this->items[this->current_item_index]->draw_inner_free(rect_intersect, GREEN);
        }

        this->drawable.draw(RDPOpaqueRect(this->rect, this->drawing_policy.get_bg_color()), clip);
*/
/*
        // Box.
        const uint16_t border           = 6;
        const uint16_t text_margin      = 6;
        const uint16_t text_indentation = border + text_margin + 4;
        const uint16_t x_offset         = 1;

        int w, h, tmp;
        this->drawable.text_metrics("bp", tmp, h);
        this->drawable.text_metrics(this->buffer, w, tmp);

        BStream deltaPoints(256);

        deltaPoints.out_sint16_le(border - (text_indentation - text_margin + 1));
        deltaPoints.out_sint16_le(0);

        deltaPoints.out_sint16_le(0);
        deltaPoints.out_sint16_le(this->rect.cy - h / 2 - border);

        deltaPoints.out_sint16_le(this->rect.cx - border * 2 + x_offset);
        deltaPoints.out_sint16_le(0);

        deltaPoints.out_sint16_le(0);
        deltaPoints.out_sint16_le(-(this->rect.cy - h / 2 - border));    // OK

        deltaPoints.out_sint16_le(-(this->rect.cx - border * 2 - w - text_indentation + x_offset));
        deltaPoints.out_sint16_le(0);

        deltaPoints.mark_end();
        deltaPoints.rewind();

        RDPPolyline polyline_box( this->rect.x + text_indentation - text_margin
                                , this->rect.y + h / 2
                                , 0x0D, 0, this->fg_color, 5, deltaPoints);
        this->drawable.draw(polyline_box, clip);


        // Label.
        this->drawable.server_draw_text( this->rect.x + text_indentation
                                       , this->rect.y
                                       , this->buffer
                                       , this->fg_color
                                       , this->bg_color
                                       , rect_intersect
                                       );
*/

        if (this->item_count > 0) {
            this->items[this->current_item_index]->draw_children(rect);
        }
    }

    virtual void rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap) {
        Widget2 * w = this->widget_at_pos(x, y);

        if (w == this) {
            this->drawing_policy.process_input_mouse( *this
                                                    , this->rect
                                                    , this->items
                                                    , this->item_count
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

    virtual void rdp_input_scancode( long param1, long param2, long param3
                                   , long param4, Keymap2 * keymap) {
        if (keymap->nb_kevent_available() > 0) {
            switch (keymap->top_kevent()) {
            case Keymap2::KEVENT_TAB:
                keymap->get_kevent();
            break;
            case Keymap2::KEVENT_BACKTAB:
                keymap->get_kevent();
            break;
            default:
            break;
            }
        }
    }

    void set_current_item(size_t item_index) {
        REDASSERT(this->item_count > item_index);
        if (this->current_item_index != item_index) {
            this->current_item_index = item_index;

            this->rdp_input_invalidate(this->rect);
        }
    }

    virtual Widget2 * widget_at_pos(int16_t x, int16_t y) {
        if (this->item_count) {
            REDASSERT(this->item_count > this->current_item_index);
            if (Widget2 * w = this->items[this->current_item_index]->widget_at_pos(x, y)) {
                return w;
            }
        }
        return Widget2::widget_at_pos(x, y);
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

public:
    WidgetTabDPDefault(DrawApi & drawable) : WidgetTab::DrawingPolicy(drawable), text_height(0), item_index_height(0) {
        int w, h;
        this->drawable.text_metrics("bp", w, h);

        this->text_height       = h;
        this->item_index_height =   border_width_height
                                  + selection_marker_height
                                  + border_width_height
                                  + text_padding_y
                                  + this->text_height
                                  + text_padding_y;
    }

    virtual void draw( const Rect & rect_tab
                     , const Rect & clip
                     , WidgetTab::Item ** items
                     , size_t item_count
                     , size_t current_item_index) {
        Rect rect_intersect = clip.intersect(rect_tab);
this->drawable.draw(RDPOpaqueRect(rect_intersect, RED), clip);

        uint16_t item_index_offset = first_item_index_offset_left;
        for (size_t item_index = 0; item_index < item_count; item_index++) {
            WidgetTab::Item * item = items[item_index];

            int text_width, h;
            this->drawable.text_metrics(item->get_text(), text_width, h);

            uint16_t item_index_width =   border_width_height
                                        + text_padding_x
                                        + text_width
                                        + text_padding_x;

            this->drawable.draw(
                  RDPOpaqueRect( rect_intersect.intersect(Rect( rect_tab.x + item_index_offset + border_width_height
                                                              , rect_tab.y + border_width_height
                                                              , item_index_width - border_width_height
                                                              ,   this->item_index_height
                                                                - ((current_item_index == item_index) ? 0 : border_width_height)))
                               , this->get_bg_color())
                , clip);

            // Text.
            this->drawable.server_draw_text(
                  rect_tab.x + item_index_offset + border_width_height + text_padding_x
                , rect_tab.y + border_width_height + selection_marker_height + border_width_height + text_padding_y
                , item->get_text()
                , this->get_fg_color()
                , this->get_bg_color()
                , rect_intersect);

            item_index_offset += item_index_width;
        }

        this->drawable.draw(
              RDPOpaqueRect( rect_intersect.intersect(Rect( rect_tab.x + border_width_height
                                                          , rect_tab.y + this->item_index_height + border_width_height
                                                          , rect_tab.cx - border_width_height * 2
                                                          , rect_tab.cy - this->item_index_height - border_width_height * 2))
                           , this->get_bg_color())
            , clip);
    }

    virtual Rect get_child_area(const Rect & rect_tab) {
        return Rect( rect_tab.x + border_width_height
                   , rect_tab.y + this->item_index_height + border_width_height
                   , rect_tab.cx - border_width_height * 2
                   , rect_tab.cy - this->item_index_height - border_width_height * 2);
    }

    virtual void process_input_mouse( WidgetTab & tab
                                    , const Rect & rect_tab
                                    , WidgetTab::Item ** items
                                    , size_t item_count
                                    , int device_flags
                                    , int x
                                    , int y
                                    , Keymap2 * keymap) {
        uint16_t item_index_offset = first_item_index_offset_left;
        for (size_t item_index = 0; item_index < item_count; item_index++) {
            WidgetTab::Item * item = items[item_index];

            int text_width, h;
            this->drawable.text_metrics(item->get_text(), text_width, h);

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
                    tab.set_current_item(item_index);
                }

                return;
            }

            item_index_offset += item_index_width;
        }
    }
};

#endif  // #ifndef WidgetTabREDEMPTION_MOD_WIDGET2_TAB_HPP
