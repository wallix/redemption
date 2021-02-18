/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen, Meng Tan, Jennifer Inthavong
 */

#include "mod/internal/selector_mod.hpp"
#include "configs/config.hpp"
#include "gdi/text_metrics.hpp"
#include "keyboard/keymap2.hpp"

#include <charconv>
#include <cassert>


namespace
{
    struct temporary_login
    {
        char buffer[256];

        explicit temporary_login(SelectorModVariables ini)
        {
            this->buffer[0] = 0;
            snprintf(
                this->buffer, sizeof(this->buffer),
                "%s@%s",
                ini.get<cfg::globals::auth_user>().c_str(),
                ini.get<cfg::globals::host>().c_str()
            );
        }
    };

    inline size_t proceed_item(const char * list)
    {
        const char * p = list;
        while (*p != '\x01' && *p != '\n' && *p) {
            p++;
        }
        return p - list;
    }

    constexpr int nb_max_row = 1024;

    struct lexical_string
    {
        char buf[32];

        template<class T>
        lexical_string(T x)
        {
            std::to_chars_result result = std::to_chars(std::begin(buf), std::end(buf), x);
            assert(result.ec == std::errc());
            assert(result.ptr != std::end(buf));
            *result.ptr = '\0';
        }

        char const* c_str() const
        {
            return this->buf;
        }
    };
} // namespace


SelectorMod::SelectorMod(
    SelectorModVariables ini,
    EventContainer& events,
    gdi::GraphicApi & drawable, FrontAPI & front, uint16_t width, uint16_t height,
    Rect const widget_rect, ClientExecute & rail_client_execute,
    Font const& font, Theme const& theme
)
    : RailModBase(
        events, drawable, front,
        width, height, rail_client_execute, font, theme)
    , ini(ini)
    , language_button(
        ini.get<cfg::client::keyboard_layout_proposals>(),
        this->selector, drawable, front, font, theme)

    , selector_params([&]() {
        WidgetSelectorParams params;

        params.nb_columns = 3;
        params.weight[0] = 20;
        params.weight[1] = 70;
        params.weight[2] = 10;

        Translator tr(ini.get<cfg::translation::language>());
        params.label[0] = tr(trkeys::authorization);
        params.label[1] = tr(trkeys::target);
        params.label[2] = tr(trkeys::protocol);

        return params;
    }())
    , selector(
        drawable, temporary_login(ini).buffer,
        widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy,
        this->screen, this,
        ini.is_asked<cfg::context::selector_current_page>()
            ? ""
            : lexical_string(ini.get<cfg::context::selector_current_page>()).c_str(),
        ini.is_asked<cfg::context::selector_number_of_pages>()
            ? ""
            : lexical_string(ini.get<cfg::context::selector_number_of_pages>()).c_str(),
        &this->language_button, this->selector_params, font, theme, language(ini))

    , current_page(atoi(this->selector.current_page.get_text())) /*NOLINT*/
    , number_page(atoi(this->selector.number_page.get_text()+1)) /*NOLINT*/
    , copy_paste(ini.get<cfg::debug::mod_internal>() != 0)
{
    this->selector.set_widget_focus(&this->selector.selector_lines, Widget::focus_reason_tabkey);
    this->screen.add_widget(&this->selector);
    this->screen.set_widget_focus(&this->selector, Widget::focus_reason_tabkey);

    uint16_t available_height = (this->selector.first_page.y() - 10) - this->selector.selector_lines.y();
    gdi::TextMetrics tm(font, "Édp");
    uint16_t line_height = tm.height + 2 * (
                            this->selector.selector_lines.border
                            +  this->selector.selector_lines.y_padding_label);

    this->selector_lines_per_page_saved = std::min<int>(available_height / line_height, nb_max_row);
    this->ini.set_acl<cfg::context::selector_lines_per_page>(this->selector_lines_per_page_saved);
    this->selector.rdp_input_invalidate(this->selector.get_rect());
    this->ask_page();
}

void SelectorMod::init()
{
    RailModBase::init();
    this->copy_paste.ready(this->front);
}

void SelectorMod::acl_update(AclFieldMask const& /*acl_fields*/)
{
    char buffer[16];

    this->current_page = this->ini.get<cfg::context::selector_current_page>();
    snprintf(buffer, sizeof(buffer), "%d", this->current_page);
    this->selector.current_page.set_text(buffer);

    this->number_page = this->ini.get<cfg::context::selector_number_of_pages>();
    snprintf(buffer, sizeof(buffer), "%d", this->number_page);
    this->selector.number_page.set_text(WidgetSelector::temporary_number_of_page(buffer).buffer);

    this->selector.selector_lines.clear();

    this->refresh_device();

    this->selector.rdp_input_invalidate(this->selector.get_rect());

    this->selector.current_page.rdp_input_invalidate(this->selector.current_page.get_rect());
    this->selector.number_page.rdp_input_invalidate(this->selector.number_page.get_rect());
}

void SelectorMod::ask_page()
{
    this->ini.set_acl<cfg::context::selector_current_page>(this->current_page);

    this->ini.set_acl<cfg::context::selector_group_filter>(this->selector.edit_filters[0].get_text());
    this->ini.set_acl<cfg::context::selector_device_filter>(this->selector.edit_filters[1].get_text());
    this->ini.set_acl<cfg::context::selector_proto_filter>(this->selector.edit_filters[2].get_text());

    this->ini.ask<cfg::globals::target_user>();
    this->ini.ask<cfg::globals::target_device>();
    this->ini.ask<cfg::context::selector>();
}

void SelectorMod::notify(Widget* widget, notify_event_t event)
{
    switch (event) {
    case NOTIFY_CANCEL: {
        this->ini.ask<cfg::globals::auth_user>();
        this->ini.ask<cfg::context::password>();
        this->ini.set<cfg::context::selector>(false);
        this->set_mod_signal(BACK_EVENT_NEXT);
        // throw Error(ERR_BACK_EVENT_NEXT);
        break;
    }
    case NOTIFY_SUBMIT: {
        if (widget == &this->selector.connect) {
            char buffer[1024] = {};
            uint16_t row_index = 0;
            uint16_t column_index = 0;
            this->selector.selector_lines.get_selection(row_index, column_index);
            if (static_cast<uint16_t>(-1u) != row_index)
            {
                const char * target = this->selector.selector_lines.get_cell_text(row_index, WidgetSelector::IDX_TARGET);
                const char * groups = this->selector.selector_lines.get_cell_text(row_index, WidgetSelector::IDX_TARGETGROUP);
                snprintf(buffer, sizeof(buffer), "%s:%s:%s",
                            target, groups, this->ini.get<cfg::globals::auth_user>().c_str());
                this->ini.set_acl<cfg::globals::auth_user>(buffer);
                this->ini.ask<cfg::globals::target_user>();
                this->ini.ask<cfg::globals::target_device>();
                this->ini.ask<cfg::context::target_protocol>();

                this->set_mod_signal(BACK_EVENT_NEXT);
                // throw Error(ERR_BACK_EVENT_NEXT);
            }
        }
        else if (widget->group_id == this->selector.apply.group_id) {
            this->ask_page();
        }
        else if (widget == &this->selector.first_page) {
            if (this->current_page > 1) {
                this->current_page = 1;
                this->ask_page();
            }
        }
        else if (widget == &this->selector.prev_page) {
            if (this->current_page > 1) {
                --this->current_page;
                this->ask_page();
            }
        }
        else if (widget == &this->selector.current_page) {
            int page = atoi(this->selector.current_page.get_text()); /*NOLINT*/
            if (page != this->current_page) {
                this->current_page = page;
                this->ask_page();
            }
        }
        else if (widget == &this->selector.next_page) {
            if (this->current_page < this->number_page) {
                ++this->current_page;
                this->ask_page();
            }
        }
        else if (widget == &this->selector.last_page) {
            if (this->current_page < this->number_page) {
                this->current_page = this->number_page;
                this->ask_page();
            }
        }

        break;
    }

    case NOTIFY_PASTE:
    case NOTIFY_COPY:
    case NOTIFY_CUT:
        if (this->copy_paste) {
            copy_paste_process_event(this->copy_paste, *widget, event);
        }
        break;

    default:;
    }
}

void SelectorMod::refresh_device()
{
    char const* groups    = this->ini.get<cfg::globals::target_user>().c_str();
    char const* targets   = this->ini.get<cfg::globals::target_device>().c_str();
    char const* protocols = this->ini.get<cfg::context::target_protocol>().c_str();
    for (unsigned index = 0; index < this->ini.get<cfg::context::selector_lines_per_page>(); index++) {
        size_t size_groups = proceed_item(groups);
        if (!size_groups) {
            break;
        }
        size_t size_targets = proceed_item(targets);
        size_t size_protocols = proceed_item(protocols);

        chars_view const texts[] {
            {groups, size_groups},
            {targets, size_targets},
            {protocols, size_protocols},
        };
        this->selector.add_device(texts);

        if (groups[size_groups]       == '\n' || !groups[size_groups]
         || targets[size_targets]     == '\n' || !targets[size_targets]
         || protocols[size_protocols] == '\n' || !protocols[size_protocols]
        ){
            break;
        }

        groups += size_groups + 1;
        targets += size_targets + 1;
        protocols += size_protocols + 1;
    }

    if (this->selector.selector_lines.get_nb_rows() == 0) {
        this->selector.selector_lines.tab_flag = Widget::IGNORE_TAB;
        this->selector.selector_lines.focus_flag = Widget::IGNORE_FOCUS;

        auto no_result = TR(trkeys::no_results, language(this->ini));
        chars_view const texts[] {{}, no_result, {}};
        this->selector.add_device(texts);
    }
    else {
        this->selector.selector_lines.tab_flag = Widget::NORMAL_TAB;
        this->selector.selector_lines.focus_flag = Widget::NORMAL_FOCUS;
        this->selector.selector_lines.set_selection(0);
        this->selector.set_widget_focus(&this->selector.selector_lines, Widget::focus_reason_tabkey);
    }

    this->selector.move_size_widget(
        this->selector.x(),
        this->selector.y(),
        this->selector.cx(),
        this->selector.cy());
}

void SelectorMod::rdp_input_scancode(
    long int param1, long int param2,
    long int param3, long int param4, Keymap2* keymap)
{
    RailModBase::rdp_input_scancode(param1, param2, param3, param4, keymap);

    if (&this->selector.selector_lines == this->selector.current_focus
        && keymap->nb_kevent_available() > 0) {
        switch (keymap->top_kevent()){
        case Keymap2::KEVENT_LEFT_ARROW:
            keymap->get_kevent();
            if (this->current_page > 1) {
                --this->current_page;
                this->ask_page();
            }
            else if (this->current_page == 1 && this->number_page > 1) {
                this->current_page = this->number_page;
                this->ask_page();
            }
            break;
        case Keymap2::KEVENT_RIGHT_ARROW:
            keymap->get_kevent();
            if (this->current_page < this->number_page) {
                ++this->current_page;
                this->ask_page();
            }
            else if (this->current_page == this->number_page && this->number_page > 1) {
                this->current_page = 1;
                this->ask_page();
            }
            break;
        default:
            this->screen.rdp_input_scancode(param1, param2, param3, param4, keymap);
            break;
        }
    }
    else {
        this->screen.rdp_input_scancode(param1, param2, param3, param4, keymap);
    }
    this->screen.rdp_input_scancode(param1, param2, param3, param4, keymap);
}

void SelectorMod::send_to_mod_channel(
    CHANNELS::ChannelNameId front_channel_name, InStream& chunk,
    size_t length, uint32_t flags)
{
    RailModBase::send_to_mod_channel(front_channel_name, chunk, length, flags);

    if (this->copy_paste && front_channel_name == CHANNELS::channel_names::cliprdr) {
        this->copy_paste.send_to_mod_channel(chunk, flags);
    }
}

void SelectorMod::move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height)
{
    this->selector.move_size_widget(left, top, width, height);

    uint16_t available_height = (this->selector.first_page.y() - 10) - this->selector.selector_lines.y();
    gdi::TextMetrics tm(this->screen.font, "Édp");
    uint16_t line_height = tm.height + 2 * (
                            this->selector.selector_lines.border
                            +  this->selector.selector_lines.y_padding_label);

    int const selector_lines_per_page = std::min<int>(available_height / line_height, nb_max_row);

    LOG(LOG_INFO, "selector lines per page = %d (%d)", selector_lines_per_page, this->selector_lines_per_page_saved);
    if (this->selector_lines_per_page_saved != selector_lines_per_page) {
        this->selector_lines_per_page_saved = selector_lines_per_page;
        this->ini.set_acl<cfg::context::selector_lines_per_page>(this->selector_lines_per_page_saved);
        this->selector.rdp_input_invalidate(this->selector.get_rect());
        this->ask_page();
    }
}
