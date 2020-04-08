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
  Copyright (C) Wallix 2013
  Author(s): Christophe Grosjean, Meng Tan
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "acl/end_session_warning.hpp"
#include "acl/time_before_closing.hpp"


RED_AUTO_TEST_CASE(TestStrTimeBeforeClosing)
{
    auto to_s = [](uint32_t t) {
        return time_before_closing(t, Translator(Translation::EN));
    };
    RED_CHECK_EQ("23 seconds before closing", to_s(23));
    RED_CHECK_EQ("36 minutes, 23 seconds before closing", to_s(23 + 36*60));
    RED_CHECK_EQ("3 hours, 36 minutes, 23 seconds before closing", to_s(23 + 216*60));
    RED_CHECK_EQ("1 hour, 1 minute, 1 second before closing", to_s(1 + 60 + 60*60));
    RED_CHECK_EQ("1 hour, 0 minute, 1 second before closing", to_s(1 + 60*60));
}


RED_AUTO_TEST_CASE(TestEndSessionWarning)
{
    EndSessionWarning end_session_warning;
    std::string       message;

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1330, 1000);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1330, 1010);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1330, 1030);
    RED_CHECK_EQ(message, "5 minutes before closing");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1330, 1060);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1330, 1270);
    RED_CHECK_EQ(message, "1 minute before closing");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1330, 1300);
    RED_CHECK_EQ(message, "");
}

RED_AUTO_TEST_CASE(TestEndSessionWarning1)
{
    EndSessionWarning end_session_warning;
    std::string       message;

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 2860, 1030);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 2860, 1060);
    RED_CHECK_EQ(message, "30 minutes before closing");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 2860, 2230);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 2860, 2260);
    RED_CHECK_EQ(message, "10 minutes before closing");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 2860, 2530);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 2860, 2560);
    RED_CHECK_EQ(message, "5 minutes before closing");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 2860, 2770);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 2860, 2800);
    RED_CHECK_EQ(message, "1 minute before closing");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 2860, 2830);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 2860, 2860);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 2860, 2890);
    RED_CHECK_EQ(message, "");
}

RED_AUTO_TEST_CASE(TestEndSessionWarning2)
{
    EndSessionWarning end_session_warning;
    std::string       message;

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 0, 330, 331);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 0, 330, 340);
    RED_CHECK_EQ(message, "");
}

RED_AUTO_TEST_CASE(TestEndSessionWarning3)
{
    EndSessionWarning end_session_warning;
    std::string       message;

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1330, 1330);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1330, 1331);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1330, 1340);
    RED_CHECK_EQ(message, "");
}

RED_AUTO_TEST_CASE(TestEndSessionWarning4)
{
    EndSessionWarning end_session_warning;
    std::string       message;

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1620, 1000);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1620, 1020);
    RED_CHECK_EQ(message, "10 minutes before closing");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1620, 1290);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1620, 1320);
    RED_CHECK_EQ(message, "5 minutes before closing");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1620, 1530);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1620, 1560);
    RED_CHECK_EQ(message, "1 minute before closing");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1620, 1590);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1620, 1620);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1620, 1650);
    RED_CHECK_EQ(message, "");
}

RED_AUTO_TEST_CASE(TestEndSessionWarning5)
{
    EndSessionWarning end_session_warning;

    RED_CHECK_EQ("", 
        end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1090, 1000));

    RED_CHECK_EQ("", 
        end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1090, 1010));

    RED_CHECK_EQ("1 minute before closing",
        end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1090, 1030));

    RED_CHECK_EQ("", 
        end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1090, 1060));
}

RED_AUTO_TEST_CASE(TestEndSessionWarning6)
{
    EndSessionWarning end_session_warning;
    std::string       message;

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1090, 1000);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1090, 1010);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1090, 1030);
    RED_CHECK_EQ(message, "1 minute before closing");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1090, 1060);
    RED_CHECK_EQ(message, "");

    end_session_warning.initialize();

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1090, 1000);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1090, 1010);
    RED_CHECK_EQ(message, "");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1090, 1030);
    RED_CHECK_EQ(message, "1 minute before closing");

    message = end_session_warning.update_osd_state(Translation::language_t::EN, 1000, 1090, 1060);
    RED_CHECK_EQ(message, "");
}
