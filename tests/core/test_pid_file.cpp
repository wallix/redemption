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
  Copyright (C) Wallix 2020
  Author(s): Proxy Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/file.hpp"

#include "core/pid_file.hpp"
#include "core/app_path.hpp"
#include "utils/sugar/algostring.hpp"

#include <charconv>

#include <sys/types.h>
#include <unistd.h>


RED_AUTO_TEST_CASE(TestPidFile)
{
    const int pid = getpid();
    char session_id[32];
    auto r = std::to_chars(std::begin(session_id), std::end(session_id), pid);
    auto dirname = app_path(AppPath::LockDir);
    chars_view s_pid{session_id, r.ptr};

    recursive_create_directory(dirname.c_str(), 0755, -1);

    auto filename = str_concat(dirname, "/session_", s_pid, ".pid");

    {
        RED_TEST(!tu::fexists(filename));
        PidFile pid_file(pid);
        RED_TEST(pid_file.is_open());
        RED_TEST(tu::fexists(filename));
    }

    {
        RED_TEST(!tu::fexists(filename));
        PidFile pid_file(pid);
        RED_TEST(pid_file.is_open());
        RED_TEST(tu::fexists(filename));

        auto new_filename = str_concat(dirname, "/session_abc.pid");
        pid_file.rename("abc"_av);
        RED_TEST(!tu::fexists(filename));
        RED_TEST(tu::fexists(new_filename));

        filename = str_concat(dirname, "/session_xyz.pid");
        pid_file.rename("xyz"_av);
        RED_TEST(!tu::fexists(new_filename));
        RED_TEST(tu::fexists(filename));
    }

    RED_TEST(!tu::fexists(filename));
}
