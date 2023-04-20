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

#include "utils/sugar/unique_fd.hpp"
#include "acl/module_manager/create_module_rdp.cpp" /* NOLINT */

RED_AUTO_TEST_CASE_WD(TestUpdateApplicationDriver, wd)
{
    Inifile ini;
    Font font;
    Theme theme;
    std::array<uint8_t, 28> server_auto_reconnect_packet;
    std::string message;

    ModRDPParams mod_rdp_params(
        "A", "B", "C", "D", kbdtypes::KeyLocks(), font, theme,
        server_auto_reconnect_packet, message, {}, RDPVerbose());

    mod_rdp_params.auth_channel = CHANNELS::ChannelNameId("xxxx");
    mod_rdp_params.application_params.alternate_shell = "__APP_DRIVER_EDGE_CHROMIUM_UIA__";

    char const* uia = "uia_chrome";
    char const* exe_or_file = "app_exe_or_file";

    ini.set<cfg::mod_rdp::application_driver_chrome_uia_script>(uia);
    ini.set<cfg::mod_rdp::application_driver_exe_or_file>(exe_or_file);
    ini.set<cfg::mod_rdp::application_driver_script_argument>("appdriver_shell_args");
    ini.set<cfg::mod_rdp::shell_arguments>("shell_args");

    auto subd1 = wd.create_subdirectory("DriveRedirection");
    auto subd2 = subd1.create_subdirectory("sespro");
    auto file1 = subd2.add_file(str_concat(uia, ".hash"_av));
    auto file2 = subd2.add_file(str_concat(exe_or_file, ".hash"_av));

    RED_CHECK(write(unique_fd(file1.c_str(), O_WRONLY | O_CREAT, 0644).fd(), "123 [abc]", 9) == 9);
    RED_CHECK(write(unique_fd(file2.c_str(), O_WRONLY | O_CREAT, 0644).fd(), "456 [def]", 9) == 9);

    update_application_driver(mod_rdp_params, ini);

    RED_CHECK(mod_rdp_params.application_params.alternate_shell ==
        "\x02\\\\tsclient\\SESPRO\\app_exe_or_file\x02""456\x02[def]"
        "\x02\\\\tsclient\\SESPRO\\uia_chrome\x02""123\x02[abc]\x02"
        ""_av);

    RED_CHECK(mod_rdp_params.application_params.shell_arguments ==
        "appdriver_shell_args /e:UseEdgeChromium=Yes /v:xxxx shell_args"
        ""_av);
}
