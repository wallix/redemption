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
 *   Author(s): Christophe Grosjean, Xiaopeng Zhou, Jonathan Poelen,
 *              Meng Tan, Jennifer Inthavong
 */

#pragma once

#include "configs/config_access.hpp"
#include "mod/internal/rail_mod_base.hpp"
#include "mod/internal/widget/dialog2.hpp"


using DialogMod2Variables = vcfg::variables<
    vcfg::var<cfg::context::display_message, vcfg::accessmode::set>,
    vcfg::var<cfg::translation::language,    vcfg::accessmode::get>
>;


class DialogMod2 : public RailInternalModBase
{
public:
    DialogMod2(
        DialogMod2Variables vars,
        gdi::GraphicApi & drawable,
        uint16_t width, uint16_t height,
        Rect const widget_rect, const char * caption, const char * message,
        const char * link_value, const char * link_label,
        ClientExecute & rail_client_execute,
        Font const& font, Theme const& theme, CopyPaste& copy_paste); /*NOLINT*/

    void move_size_widget(int16_t left, int16_t top, uint16_t width, uint16_t height) override
    {
        this->dialog_widget.move_size_widget(left, top, width, height);
    }

    void acl_update(AclFieldMask const&/* acl_fields*/) override {}

private:
    void accepted();

    void refused();

    WidgetDialog2 dialog_widget;

    DialogMod2Variables vars;
};
