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
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/file.hpp"

#include "core/pid_file.hpp"
#include "core/app_path.hpp"
#include "utils/strutils.hpp"
#include "utils/sugar/int_to_chars.hpp"

#include <sys/types.h>
#include <unistd.h>


RED_AUTO_TEST_CASE_WD(TestPidFile, wd)
{
    const int pid = getpid();
    auto const s_pid = int_to_decimal_chars(pid);
    auto const dirname = app_path(AppPath::LockDir);

    recursive_create_directory(dirname.c_str(), 0755);

    auto subdir = wd.create_subdirectory("LockDir");
    auto filename = str_concat("session_"_av, s_pid, ".pid"_av);
    auto file = subdir.add_file(filename);
    auto path = file.string();

    {
        RED_TEST(!ut::fexists(path));
        PidFile pid_file(pid);
        RED_TEST(pid_file.is_open());
        RED_TEST(ut::fexists(path));
    }

    {
        RED_TEST(!ut::fexists(path));
        PidFile pid_file(pid);
        RED_TEST(pid_file.is_open());
        RED_TEST(ut::fexists(path));

        auto new_path = str_concat(dirname, "/session_abc.pid");
        pid_file.rename("abc"_av);
        RED_TEST(!ut::fexists(path));
        RED_TEST(ut::fexists(new_path));

        path = str_concat(dirname, "/session_xyz.pid");
        pid_file.rename("xyz"_av);
        RED_TEST(!ut::fexists(new_path));
        RED_TEST(ut::fexists(path));
    }

    RED_TEST(!ut::fexists(path));

    subdir.remove_files({std::string_view(filename)});
}
