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
#include "core/front_api.hpp"
#include "configs/config.hpp"
#include "core/RDP/slowpath.hpp"
#include "RAIL/client_execute.hpp"


namespace
{
    struct temporary_login
    {
        char buffer[256];

        explicit temporary_login(SelectorModVariables vars) {
            this->buffer[0] = 0;
            snprintf(
                this->buffer, sizeof(this->buffer),
                "%s@%s",
                vars.get<cfg::globals::auth_user>().c_str(),
                vars.get<cfg::globals::host>().c_str()
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
} // namespace



void SelectorMod::rdp_input_invalidate(Rect r)
{
    this->screen.rdp_input_invalidate(r);

    if (this->rail_enabled) {
        this->rail_client_execute.input_invalidate(r);
    }
}

void SelectorMod::rdp_input_mouse(int device_flags, int x, int y, Keymap2 * keymap)
{
    if (device_flags & (MOUSE_FLAG_WHEEL | MOUSE_FLAG_HWHEEL)) {
        x = this->old_mouse_x;
        y = this->old_mouse_y;
    }
    else {
        this->old_mouse_x = x;
        this->old_mouse_y = y;
    }

    if (!this->rail_enabled) {
        this->screen.rdp_input_mouse(device_flags, x, y, keymap);
    }
    else {
        bool out_mouse_captured = false;
        if (!this->rail_client_execute.input_mouse(device_flags, x, y, out_mouse_captured)) {
            switch (this->dc_state) {
                case DCState::Wait:
                    if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                        this->dc_state = DCState::FirstClickDown;

                        if (this->first_click_down_timer) {
                            this->first_click_down_timer->set_delay(std::chrono::seconds(1));
                        }
                        else {
                            this->first_click_down_timer = timer_events_
                            .create_timer_executor(this->session_reactor)
                            .set_delay(std::chrono::seconds(1))
                            .on_action(jln::one_shot([this]{
                                this->dc_state = DCState::Wait;
                            }));
                        }
                    }
                break;

                case DCState::FirstClickDown:
                    if (device_flags == SlowPath::PTRFLAGS_BUTTON1) {
                        this->dc_state = DCState::FirstClickRelease;
                    }
                    else if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                    }
                    else {
                        this->cancel_double_click_detection();
                    }
                break;

                case DCState::FirstClickRelease:
                    if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                        this->dc_state = DCState::SecondClickDown;
                    }
                    else {
                        this->cancel_double_click_detection();
                    }
                break;

                case DCState::SecondClickDown:
                    if (device_flags == SlowPath::PTRFLAGS_BUTTON1) {
                        this->dc_state = DCState::Wait;

                        bool out_mouse_captured_2 = false;

                        this->rail_client_execute.input_mouse(PTRFLAGS_EX_DOUBLE_CLICK, x, y, out_mouse_captured_2);

                        this->cancel_double_click_detection();
                    }
                    else if (device_flags == (SlowPath::PTRFLAGS_DOWN | SlowPath::PTRFLAGS_BUTTON1)) {
                    }
                    else {
                        this->cancel_double_click_detection();
                    }
                break;

                default:
                    assert(false);

                    this->cancel_double_click_detection();
                break;
            }

            if (out_mouse_captured) {
                this->allow_mouse_pointer_change(false);

                this->current_mouse_owner = MouseOwner::ClientExecute;
            }
            else {
                if (MouseOwner::WidgetModule != this->current_mouse_owner) {
                    this->redo_mouse_pointer_change(x, y);
                }

                this->current_mouse_owner = MouseOwner::WidgetModule;
            }
        }

        this->screen.rdp_input_mouse(device_flags, x, y, keymap);

        if (out_mouse_captured) {
            this->allow_mouse_pointer_change(true);
        }
    }
}

void SelectorMod::refresh(Rect r)
{
    this->screen.refresh(r);

    if (this->rail_enabled) {
        this->rail_client_execute.input_invalidate(r);
    }
}

void SelectorMod::cancel_double_click_detection()
{
    assert(this->rail_enabled);

    this->first_click_down_timer.reset();

    this->dc_state = DCState::Wait;
}

bool SelectorMod::is_resizing_hosted_desktop_allowed() const
{
    assert(this->rail_enabled);

    return false;
}

SelectorMod::SelectorMod(
    Inifile & ini,
    SelectorModVariables vars,
    SessionReactor& session_reactor,
    TimerContainer& timer_events_,
    GraphicEventContainer& graphic_events_,
    gdi::GraphicApi & drawable, FrontAPI & front, uint16_t width, uint16_t height,
    Rect const widget_rect, ClientExecute & rail_client_execute,
    Font const& font, Theme const& theme
)
    : front_width(width)
    , front_height(height)
    , front(front)
    , screen(drawable, width, height, font, nullptr, theme)
    , rail_client_execute(rail_client_execute)
    , dvc_manager(false)
    , dc_state(DCState::Wait)
    , rail_enabled(rail_client_execute.is_rail_enabled())
    , current_mouse_owner(MouseOwner::WidgetModule)
    , session_reactor(session_reactor)
    , timer_events_(timer_events_)
    , graphic_events_(graphic_events_)
    , language_button(
        vars.get<cfg::client::keyboard_layout_proposals>(),
        this->selector, drawable, front, font, theme)

    , selector_params([&]() {
        WidgetSelectorParams params;

        params.nb_columns = 3;
        params.weight[0] = 20;
        params.weight[1] = 70;
        params.weight[2] = 10;

        Translator tr(language(vars.get<cfg::translation::language>()));
        params.label[0] = tr(trkeys::authorization);
        params.label[1] = tr(trkeys::target);
        params.label[2] = tr(trkeys::protocol);

        return params;
    }())
    , selector(
        drawable, temporary_login(vars).buffer,
        widget_rect.x, widget_rect.y, widget_rect.cx, widget_rect.cy,
        this->screen, this,
        vars.is_asked<cfg::context::selector_current_page>()
            ? ""
            : configs::make_zstr_buffer(vars.get<cfg::context::selector_current_page>()).get(),
        vars.is_asked<cfg::context::selector_number_of_pages>()
            ? ""
            : configs::make_zstr_buffer(vars.get<cfg::context::selector_number_of_pages>()).get(),
        &this->language_button, this->selector_params, font, theme, language(vars))

    , current_page(atoi(this->selector.current_page.get_text())) /*NOLINT*/
    , number_page(atoi(this->selector.number_page.get_text()+1)) /*NOLINT*/
    , ini(ini)
    , vars(vars)
    , copy_paste(vars.get<cfg::debug::mod_internal>() != 0)
{
    this->screen.set_wh(front_width, front_height);
    if (this->rail_enabled) {
        this->graphic_event = graphic_events_.create_action_executor(session_reactor)
        .on_action(jln::one_shot([this](gdi::GraphicApi&){
            if (!this->rail_client_execute) {
                this->rail_client_execute.ready(
                    *this, this->front_width, this->front_height, this->font(),
                    this->is_resizing_hosted_desktop_allowed());

                this->dvc_manager.ready(this->front);
            }
        }));
    }
    this->selector.set_widget_focus(&this->selector.selector_lines, Widget::focus_reason_tabkey);
    this->screen.add_widget(&this->selector);
    this->screen.set_widget_focus(&this->selector, Widget::focus_reason_tabkey);

    uint16_t available_height = (this->selector.first_page.y() - 10) - this->selector.selector_lines.y();
    gdi::TextMetrics tm(font, "Édp");
    uint16_t line_height = tm.height + 2 * (
                            this->selector.selector_lines.border
                            +  this->selector.selector_lines.y_padding_label);

    this->selector_lines_per_page_saved = std::min<int>(available_height / line_height, nb_max_row);
    this->vars.set_acl<cfg::context::selector_lines_per_page>(this->selector_lines_per_page_saved);
    this->ask_page();
    this->selector.rdp_input_invalidate(this->selector.get_rect());

    this->started_copy_past_event = graphic_events_.create_action_executor(session_reactor)
    .on_action(jln::one_shot([this](gdi::GraphicApi&){
        this->copy_paste.ready(this->front);
    }));
}


void SelectorMod::acl_update()
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
    this->vars.set_acl<cfg::context::selector_current_page>(static_cast<unsigned>(this->current_page));

    this->vars.set_acl<cfg::context::selector_group_filter>(this->selector.edit_filters[0].get_text());
    this->vars.set_acl<cfg::context::selector_device_filter>(this->selector.edit_filters[1].get_text());
    this->vars.set_acl<cfg::context::selector_proto_filter>(this->selector.edit_filters[2].get_text());

    this->vars.ask<cfg::globals::target_user>();
    this->vars.ask<cfg::globals::target_device>();
    this->vars.ask<cfg::context::selector>();

    // TODO replace BACK_EVENT_REFRESH by session_reactor.create_graphic_event ?
    this->set_mod_signal(BACK_EVENT_REFRESH);
}

void SelectorMod::notify(Widget* widget, notify_event_t event)
{
    switch (event) {
    case NOTIFY_CANCEL: {
        this->vars.ask<cfg::globals::auth_user>();
        this->vars.ask<cfg::context::password>();
        this->vars.set<cfg::context::selector>(false);
        this->set_mod_signal(BACK_EVENT_NEXT);

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
                            target, groups, this->vars.get<cfg::globals::auth_user>().c_str());
                this->vars.set_acl<cfg::globals::auth_user>(buffer);
                this->vars.ask<cfg::globals::target_user>();
                this->vars.ask<cfg::globals::target_device>();
                this->vars.ask<cfg::context::target_protocol>();

                this->set_mod_signal(BACK_EVENT_NEXT);
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
    } break;
    case NOTIFY_PASTE: case NOTIFY_COPY: case NOTIFY_CUT: {
        if (this->copy_paste) {
            copy_paste_process_event(this->copy_paste, *reinterpret_cast<WidgetEdit*>(widget), event); /*NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)*/
        }
    } break;
    default:;
    }
}

void SelectorMod::refresh_device()
{
    char const* groups    = this->vars.get<cfg::globals::target_user>().c_str();
    char const* targets   = this->vars.get<cfg::globals::target_device>().c_str();
    char const* protocols = this->vars.get<cfg::context::target_protocol>().c_str();
    for (unsigned index = 0; index < this->vars.get<cfg::context::selector_lines_per_page>(); index++) {
        size_t size_groups = proceed_item(groups);
        if (!size_groups) {
            break;
        }
        size_t size_targets = proceed_item(targets);
        size_t size_protocols = proceed_item(protocols);

        array_view_const_char const texts[] {
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

        auto no_result = TR(trkeys::no_results, language(this->vars));
        array_view_const_char const texts[] {{}, no_result, {}};
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

    if (this->rail_enabled) {
        if (!this->alt_key_pressed) {
            if ((param1 == 56) && !(param3 & SlowPath::KBDFLAGS_RELEASE)) {
                this->alt_key_pressed = true;
            }
        }
        else {
//            if ((param1 == 56) && (param3 == (SlowPath::KBDFLAGS_DOWN | SlowPath::KBDFLAGS_RELEASE))) {
            if ((param1 == 56) && (param3 & SlowPath::KBDFLAGS_RELEASE)) {
                this->alt_key_pressed = false;
            }
            else if ((param1 == 62) && !param3) {
                LOG(LOG_INFO, "SelectorMod::rdp_input_scancode: Close by user (Alt+F4)");
                throw Error(ERR_WIDGET);    // F4 key pressed
            }
        }
    }

}

void SelectorMod::send_to_mod_channel(
    CHANNELS::ChannelNameId front_channel_name, InStream& chunk,
    size_t length, uint32_t flags)
{
    if (this->rail_enabled && this->rail_client_execute){
        if (front_channel_name == CHANNELS::channel_names::rail) {
            this->rail_client_execute.send_to_mod_rail_channel(length, chunk, flags);
        }
        else if (front_channel_name == CHANNELS::channel_names::drdynvc) {
            this->dvc_manager.send_to_mod_drdynvc_channel(length, chunk, flags);
        }
    }

    if (this->copy_paste && front_channel_name == CHANNELS::channel_names::cliprdr) {
        this->copy_paste.send_to_mod_channel(chunk, flags);
    }
}

void SelectorMod::move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height)
{
    this->selector.move_size_widget(left, top, width, height);

    uint16_t available_height = (this->selector.first_page.y() - 10) - this->selector.selector_lines.y();
    gdi::TextMetrics tm(this->font(), "Édp");
    uint16_t line_height = tm.height + 2 * (
                            this->selector.selector_lines.border
                            +  this->selector.selector_lines.y_padding_label);

    int const selector_lines_per_page = std::min<int>(available_height / line_height, nb_max_row);

    LOG(LOG_INFO, "selector lines per page = %d (%d)", selector_lines_per_page, this->selector_lines_per_page_saved);
    if (this->selector_lines_per_page_saved != selector_lines_per_page) {
        this->selector_lines_per_page_saved = selector_lines_per_page;
        this->vars.set_acl<cfg::context::selector_lines_per_page>(this->selector_lines_per_page_saved);
        this->ask_page();
        this->selector.rdp_input_invalidate(this->selector.get_rect());
    }
}
