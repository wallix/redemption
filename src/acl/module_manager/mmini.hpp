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

  Manage Modules Life cycle : creation, destruction and chaining
  find out the next module to run from context reading
*/

#pragma once

#include "acl/mm_api.hpp"
#include "configs/config.hpp"
#include "utils/rect.hpp"


class Inifile;
class SessionReactor;

namespace GCC::UserData
{
    class CSMonitor;
}


Rect get_widget_rect(uint16_t width, uint16_t height, GCC::UserData::CSMonitor const & monitors);


class MMIni : public MMApi
{
protected:
    Inifile& ini;
    SessionReactor& session_reactor;

public:
    explicit MMIni(SessionReactor& session_reactor, Inifile& ini)
    : ini(ini)
    , session_reactor(session_reactor)
    {}

    void remove_mod() override {}

    void new_mod(int target_module, time_t now, AuthApi & /*unused*/, ReportMessageApi & /*unused*/) override;

    void invoke_close_box(const char * auth_error_message, BackEvent_t & signal,
                          time_t now, AuthApi & authentifier, ReportMessageApi & report_message) override;

    int next_module() override;

    void check_module() override;
};
