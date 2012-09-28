/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2010-2012
 * Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 *
 * recorder main program
 *
 */

#include "recorder/recorder_app.hpp"
#include "recorder/recorder_option.hpp"
#include "recorder/adapter/to_png_adapter.hpp"
#include "recorder/adapter/to_wrm_adapter.hpp"

int main(int argc, char** argv)
{
    RecorderOption opt;
    ToPngAdapter to_png(opt);
    ToWrmAdapter to_wrm(opt);
    ToPngListAdapter to_png_list(opt);
    RecorderAction recorder_actions[] = {
        RecorderAction("png", &to_png),
        RecorderAction("wrm", &to_wrm),
        RecorderAction("png.list", &to_png_list),
    };

    return recorder_app(opt, argc, argv, recorder_actions,
                        sizeof(recorder_actions) / sizeof(recorder_actions[0]));
}
