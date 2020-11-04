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
  Copyright (C) Wallix 2010
  Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
             Raphael Zhou, Meng Tan

*/

#pragma once

#include "utils/translation.hpp"
#include "utils/log.hpp"
#include "utils/sugar/algostring.hpp"

#include <array>
#include <algorithm>


class EndSessionWarning
{
    static constexpr std::array<unsigned, 4> timers{{ 30*60, 10*60, 5*60, 1*60 }};

    const unsigned OSD_STATE_INVALID = timers.size();
    const unsigned OSD_STATE_NOT_YET_COMPUTED = OSD_STATE_INVALID + 1;

    unsigned osd_state = OSD_STATE_NOT_YET_COMPUTED;

public:
    void initialize()
    {
        this->osd_state = OSD_STATE_NOT_YET_COMPUTED;
    }

    std::string update_osd_state(Language lang, time_t start_time, time_t end_time, time_t now)
    {
        LOG(LOG_INFO, "Update OSD STATE");

        if (this->osd_state == OSD_STATE_NOT_YET_COMPUTED) {
            this->osd_state = (
                      (end_time <= now)
                    ? OSD_STATE_INVALID
                    : timers.size() - (std::lower_bound(timers.rbegin(), timers.rend(), end_time - start_time) - timers.rbegin())
                );
        }
        else if (this->osd_state < OSD_STATE_INVALID
              && end_time - now <= timers[this->osd_state]) {
            const unsigned minutes = (end_time - now + 30) / 60;
            ++this->osd_state;
            return str_concat(
                std::to_string(minutes),
                ' ',
                TR(trkeys::minute, lang),
                (minutes > 1) ? "s " : " ",
                TR(trkeys::before_closing, lang)
            );
        }

        return std::string();
    }
};

