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
  Copyright (C) Wallix 2021
  Author(s): Proxy Team
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "acl/kv_list_from_strings.hpp"
#include "utils/strutils.hpp"

#include <array>

namespace
{
    std::string kv_list_to_str(array_view<KVLog> kv_list)
    {
        std::string s;
        for (auto kv : kv_list) {
            str_append(s, kv.key, '=', kv.value, " | ");
        }
        return s;
    }

    const auto mk_array = [](auto... strs){
        return [](auto... strs2){
            return std::array<std::string, sizeof...(strs2)>{strs2...};
        }(
            strs..., strs..., strs..., strs..., strs...,
            strs..., strs..., strs..., strs..., strs...,
            strs...
        );
    };
} // namespace anonymous

RED_AUTO_TEST_CASE(TestKVListFromStrings)
{
    std::string strings[]{
        "Operation", "201",
        "LogType", "BLOCKED",
        "ProductName", "SBS",
        "ProductModule", "SBSAGENT",
        "ProductVersion", "v4.2.0.24",
        "UTCDate", "2021.03.26UTC",
        "Time", "10:37:11",
        "LocalDate", "2021.03.26",
        "LocalTime", "11:37:11",
        "IPAddress", "10.99.16.5",
        "OperationCode", "BLOCKED",
        "HostName", "WIN10",
        "UserName", "WALLIX\\NormalUser",
        "ResultCode", "0x00000000",
        "RuleIndex", "1/15",
        "ProcessName", "mstsc.exe",
        "AdminYN", "No",
        "ExeDescription", "Remote Desktop Connection",
        "ExeVersion", "10.0.19041.1 (WinBuild.160101.0800)",
        "ExeOriginalName", "mstsc.exe",
        "ProcessCommandLine", "C:\\WINDOWS\\system32\\mstsc.exe",
        "ProcessID", "9812",
        "ProcessParentID", "6780",
        "ProcessParentCommandLine", "C:\\WINDOWS\\Explorer.EXE",
        "RuleGuid", "{E1E15A84-8890-474B-A8FE-F9C8A49B248D}",
    };

    KVListFromStrings builder(strings);
    auto kv_list = builder.next();
    RED_CHECK(kv_list.size() == std::size(strings)/2);
    RED_CHECK(bool(kv_list));
    RED_TEST(kv_list_to_str(kv_list) ==
        "Operation=201 | "
        "LogType=BLOCKED | "
        "ProductName=SBS | "
        "ProductModule=SBSAGENT | "
        "ProductVersion=v4.2.0.24 | "
        "UTCDate=2021.03.26UTC | "
        "Time=10:37:11 | "
        "LocalDate=2021.03.26 | "
        "LocalTime=11:37:11 | "
        "IPAddress=10.99.16.5 | "
        "OperationCode=BLOCKED | "
        "HostName=WIN10 | "
        "UserName=WALLIX\\NormalUser | "
        "ResultCode=0x00000000 | "
        "RuleIndex=1/15 | "
        "ProcessName=mstsc.exe | "
        "AdminYN=No | "
        "ExeDescription=Remote Desktop Connection | "
        "ExeVersion=10.0.19041.1 (WinBuild.160101.0800) | "
        "ExeOriginalName=mstsc.exe | "
        "ProcessCommandLine=C:\\WINDOWS\\system32\\mstsc.exe | "
        "ProcessID=9812 | "
        "ProcessParentID=6780 | "
        "ProcessParentCommandLine=C:\\WINDOWS\\Explorer.EXE | "
        "RuleGuid={E1E15A84-8890-474B-A8FE-F9C8A49B248D} | "
        ""_av);
    kv_list = builder.next();
    RED_CHECK(kv_list.size() == 0);
    RED_CHECK(!bool(kv_list));
}

RED_AUTO_TEST_CASE(TestKVListFromStringsLongKV)
{
    auto strings = mk_array(
        "Operation", "201",
        "LogType", "BLOCKED",
        "ProductName", "SBS",
        "ProductModule", "SBSAGENT",
        "ProductVersion", "v4.2.0.24",
        "UTCDate", "2021.03.26UTC",
        "Time", "10:37:11",
        "LocalDate", "2021.03.26",
        "LocalTime", "11:37:11",
        "IPAddress", "10.99.16.5",
        "OperationCode", "BLOCKED",
        "HostName", "WIN10",
        "UserName", "WALLIX\\NormalUser",
        "ResultCode", "0x00000000",
        "RuleIndex", "1/15",
        "ProcessName", "mstsc.exe",
        "AdminYN", "No",
        "ExeDescription", "Remote Desktop Connection",
        "ExeVersion", "10.0.19041.1 (WinBuild.160101.0800)",
        "ExeOriginalName", "mstsc.exe",
        "ProcessCommandLine", "C:\\WINDOWS\\system32\\mstsc.exe",
        "ProcessID", "9812",
        "ProcessParentID", "6780",
        "ProcessParentCommandLine", "C:\\WINDOWS\\Explorer.EXE",
        "RuleGuid", "{E1E15A84-8890-474B-A8FE-F9C8A49B248D}"
    );

    auto s =
        "Operation=201 | "
        "LogType=BLOCKED | "
        "ProductName=SBS | "
        "ProductModule=SBSAGENT | "
        "ProductVersion=v4.2.0.24 | "
        "UTCDate=2021.03.26UTC | "
        "Time=10:37:11 | "
        "LocalDate=2021.03.26 | "
        "LocalTime=11:37:11 | "
        "IPAddress=10.99.16.5 | "
        "OperationCode=BLOCKED | "
        "HostName=WIN10 | "
        "UserName=WALLIX\\NormalUser | "
        "ResultCode=0x00000000 | "
        "RuleIndex=1/15 | "
        "ProcessName=mstsc.exe | "
        "AdminYN=No | "
        "ExeDescription=Remote Desktop Connection | "
        "ExeVersion=10.0.19041.1 (WinBuild.160101.0800) | "
        "ExeOriginalName=mstsc.exe | "
        "ProcessCommandLine=C:\\WINDOWS\\system32\\mstsc.exe | "
        "ProcessID=9812 | "
        "ProcessParentID=6780 | "
        "ProcessParentCommandLine=C:\\WINDOWS\\Explorer.EXE | "
        "RuleGuid={E1E15A84-8890-474B-A8FE-F9C8A49B248D} | "

        ""_av;

    KVListFromStrings builder(strings);
    auto kv_list = builder.next();
    RED_CHECK(kv_list.size() == 255);
    RED_CHECK(bool(kv_list));
    RED_TEST(kv_list_to_str(kv_list) == str_concat(s, s, s, s, s, s, s, s, s, s,
        "Operation=201 | "
        "LogType=BLOCKED | "
        "ProductName=SBS | "
        "ProductModule=SBSAGENT | "
        "ProductVersion=v4.2.0.24 | "

        ""_av));

    kv_list = builder.next();
    RED_CHECK(kv_list.size() == 20);
    RED_CHECK(bool(kv_list));
    RED_TEST(kv_list_to_str(kv_list) ==
        "UTCDate=2021.03.26UTC | "
        "Time=10:37:11 | "
        "LocalDate=2021.03.26 | "
        "LocalTime=11:37:11 | "
        "IPAddress=10.99.16.5 | "
        "OperationCode=BLOCKED | "
        "HostName=WIN10 | "
        "UserName=WALLIX\\NormalUser | "
        "ResultCode=0x00000000 | "
        "RuleIndex=1/15 | "
        "ProcessName=mstsc.exe | "
        "AdminYN=No | "
        "ExeDescription=Remote Desktop Connection | "
        "ExeVersion=10.0.19041.1 (WinBuild.160101.0800) | "
        "ExeOriginalName=mstsc.exe | "
        "ProcessCommandLine=C:\\WINDOWS\\system32\\mstsc.exe | "
        "ProcessID=9812 | "
        "ProcessParentID=6780 | "
        "ProcessParentCommandLine=C:\\WINDOWS\\Explorer.EXE | "
        "RuleGuid={E1E15A84-8890-474B-A8FE-F9C8A49B248D} | "
        ""_av
    );

    kv_list = builder.next();
    RED_CHECK(kv_list.size() == 0);
    RED_CHECK(!bool(kv_list));
}

RED_AUTO_TEST_CASE(TestKVListFromStringsManyKV)
{
    auto mk_array = [](auto... strs){
        return [](auto... strs2){
            return std::array<std::string, sizeof...(strs2)>{strs2...};
        }(
            strs..., strs..., strs..., strs..., strs...,
            strs..., strs..., strs..., strs..., strs...,
            strs..., strs..., strs..., strs..., strs...,
            strs..., strs..., strs..., strs..., strs...
        );
    };

    auto strings = mk_array(
        "aaaaabbbbbaaaaabbbbbaaaaabbbbbaaaaaaaaaabbbbbaaaaabbbbbaaaaabbbbbaaaaa",
        "AAAAABBBBBAAAAABBBBBAAAAABBBBBAAAAAAAAAABBBBBAAAAABBBBBAAAAABBBBBAAAAA",
        "cccccdddddcccccdddddcccccdddddccccccccccdddddcccccdddddcccccdddddccccc",
        "CCCCCDDDDDCCCCCDDDDDCCCCCDDDDDCCCCCCCCCCDDDDDCCCCCDDDDDCCCCCDDDDDCCCCC",
        "eeeeefffffeeeeefffffeeeeefffffeeeeeeeeeefffffeeeeefffffeeeeefffffeeeee",
        "EEEEEFFFFFEEEEEFFFFFEEEEEFFFFFEEEEEEEEEEFFFFFEEEEEFFFFFEEEEEFFFFFEEEEE",
        "ggggghhhhhggggghhhhhggggghhhhhgggggggggghhhhhggggghhhhhggggghhhhhggggg",
        "GGGGGHHHHHGGGGGHHHHHGGGGGHHHHHGGGGGGGGGGHHHHHGGGGGHHHHHGGGGGHHHHHGGGGG",
        "iiiiijjjjjiiiiijjjjjiiiiijjjjjiiiiiiiiiijjjjjiiiiijjjjjiiiiijjjjjiiiii",
        "IIIIIJJJJJIIIIIJJJJJIIIIIJJJJJIIIIIIIIIIJJJJJIIIIIJJJJJIIIIIJJJJJIIIII",
        "kkkkklllllkkkkklllllkkkkklllllkkkkkkkkkklllllkkkkklllllkkkkklllllkkkkk",
        "KKKKKLLLLLKKKKKLLLLLKKKKKLLLLLKKKKKKKKKKLLLLLKKKKKLLLLLKKKKKLLLLLKKKKK",
        "mmmmmnnnnnmmmmmnnnnnmmmmmnnnnnmmmmmmmmmmnnnnnmmmmmnnnnnmmmmmnnnnnmmmmm",
        "MMMMMNNNNNMMMMMNNNNNMMMMMNNNNNMMMMMMMMMMNNNNNMMMMMNNNNNMMMMMNNNNNMMMMM",
        "ooooopppppooooopppppooooopppppoooooooooopppppooooopppppooooopppppooooo",
        "OOOOOPPPPPOOOOOPPPPPOOOOOPPPPPOOOOOOOOOOPPPPPOOOOOPPPPPOOOOOPPPPPOOOOO",
        "qqqqqrrrrrqqqqqrrrrrqqqqqrrrrrqqqqqqqqqqrrrrrqqqqqrrrrrqqqqqrrrrrqqqqq",
        "QQQQQRRRRRQQQQQRRRRRQQQQQRRRRRQQQQQQQQQQRRRRRQQQQQRRRRRQQQQQRRRRRQQQQQ",
        "ssssstttttssssstttttssssstttttsssssssssstttttssssstttttssssstttttsssss",
        "SSSSSTTTTTSSSSSTTTTTSSSSSTTTTTSSSSSSSSSSTTTTTSSSSSTTTTTSSSSSTTTTTSSSSS",
        "uuuuuvvvvvuuuuuvvvvvuuuuuvvvvvuuuuuuuuuuvvvvvuuuuuvvvvvuuuuuvvvvvuuuuu",
        "UUUUUVVVVVUUUUUVVVVVUUUUUVVVVVUUUUUUUUUUVVVVVUUUUUVVVVVUUUUUVVVVVUUUUU",
        "wwwwwxxxxxwwwwwxxxxxwwwwwxxxxxwwwwwwwwwwxxxxxwwwwwxxxxxwwwwwxxxxxwwwww",
        "WWWWWXXXXXWWWWWXXXXXWWWWWXXXXXWWWWWWWWWWXXXXXWWWWWXXXXXWWWWWXXXXXWWWWW",
        "yyyyyzzzzzyyyyyzzzzzyyyyyzzzzzyyyyyyyyyyzzzzzyyyyyzzzzzyyyyyzzzzzyyyyy",
        "YYYYYZZZZZYYYYYZZZZZYYYYYZZZZZYYYYYYYYYYZZZZZYYYYYZZZZZYYYYYZZZZZYYYYY"
    );

    auto s =
        "aaaaabbbbbaaaaabbbbbaaaaabbbbbaaaaaaaaaabbbbbaaaaabbbbbaaaaabbbbbaaaaa="
        "AAAAABBBBBAAAAABBBBBAAAAABBBBBAAAAAAAAAABBBBBAAAAABBBBBAAAAABBBBBAAAAA | "
        "cccccdddddcccccdddddcccccdddddccccccccccdddddcccccdddddcccccdddddccccc="
        "CCCCCDDDDDCCCCCDDDDDCCCCCDDDDDCCCCCCCCCCDDDDDCCCCCDDDDDCCCCCDDDDDCCCCC | "
        "eeeeefffffeeeeefffffeeeeefffffeeeeeeeeeefffffeeeeefffffeeeeefffffeeeee="
        "EEEEEFFFFFEEEEEFFFFFEEEEEFFFFFEEEEEEEEEEFFFFFEEEEEFFFFFEEEEEFFFFFEEEEE | "
        "ggggghhhhhggggghhhhhggggghhhhhgggggggggghhhhhggggghhhhhggggghhhhhggggg="
        "GGGGGHHHHHGGGGGHHHHHGGGGGHHHHHGGGGGGGGGGHHHHHGGGGGHHHHHGGGGGHHHHHGGGGG | "
        "iiiiijjjjjiiiiijjjjjiiiiijjjjjiiiiiiiiiijjjjjiiiiijjjjjiiiiijjjjjiiiii="
        "IIIIIJJJJJIIIIIJJJJJIIIIIJJJJJIIIIIIIIIIJJJJJIIIIIJJJJJIIIIIJJJJJIIIII | "
        "kkkkklllllkkkkklllllkkkkklllllkkkkkkkkkklllllkkkkklllllkkkkklllllkkkkk="
        "KKKKKLLLLLKKKKKLLLLLKKKKKLLLLLKKKKKKKKKKLLLLLKKKKKLLLLLKKKKKLLLLLKKKKK | "
        "mmmmmnnnnnmmmmmnnnnnmmmmmnnnnnmmmmmmmmmmnnnnnmmmmmnnnnnmmmmmnnnnnmmmmm="
        "MMMMMNNNNNMMMMMNNNNNMMMMMNNNNNMMMMMMMMMMNNNNNMMMMMNNNNNMMMMMNNNNNMMMMM | "
        "ooooopppppooooopppppooooopppppoooooooooopppppooooopppppooooopppppooooo="
        "OOOOOPPPPPOOOOOPPPPPOOOOOPPPPPOOOOOOOOOOPPPPPOOOOOPPPPPOOOOOPPPPPOOOOO | "
        "qqqqqrrrrrqqqqqrrrrrqqqqqrrrrrqqqqqqqqqqrrrrrqqqqqrrrrrqqqqqrrrrrqqqqq="
        "QQQQQRRRRRQQQQQRRRRRQQQQQRRRRRQQQQQQQQQQRRRRRQQQQQRRRRRQQQQQRRRRRQQQQQ | "
        "ssssstttttssssstttttssssstttttsssssssssstttttssssstttttssssstttttsssss="
        "SSSSSTTTTTSSSSSTTTTTSSSSSTTTTTSSSSSSSSSSTTTTTSSSSSTTTTTSSSSSTTTTTSSSSS | "
        "uuuuuvvvvvuuuuuvvvvvuuuuuvvvvvuuuuuuuuuuvvvvvuuuuuvvvvvuuuuuvvvvvuuuuu="
        "UUUUUVVVVVUUUUUVVVVVUUUUUVVVVVUUUUUUUUUUVVVVVUUUUUVVVVVUUUUUVVVVVUUUUU | "
        "wwwwwxxxxxwwwwwxxxxxwwwwwxxxxxwwwwwwwwwwxxxxxwwwwwxxxxxwwwwwxxxxxwwwww="
        "WWWWWXXXXXWWWWWXXXXXWWWWWXXXXXWWWWWWWWWWXXXXXWWWWWXXXXXWWWWWXXXXXWWWWW | "
        "yyyyyzzzzzyyyyyzzzzzyyyyyzzzzzyyyyyyyyyyzzzzzyyyyyzzzzzyyyyyzzzzzyyyyy="
        "YYYYYZZZZZYYYYYZZZZZYYYYYZZZZZYYYYYYYYYYZZZZZYYYYYZZZZZYYYYYZZZZZYYYYY | "
        ""_av;

    KVListFromStrings builder(strings);
    auto kv_list = builder.next();
    RED_CHECK(kv_list.size() == 114);
    RED_CHECK(bool(kv_list));
    RED_TEST(kv_list_to_str(kv_list) == str_concat(s, s, s, s, s, s, s, s,
        "aaaaabbbbbaaaaabbbbbaaaaabbbbbaaaaaaaaaabbbbbaaaaabbbbbaaaaabbbbbaaaaa="
        "AAAAABBBBBAAAAABBBBBAAAAABBBBBAAAAAAAAAABBBBBAAAAABBBBBAAAAABBBBBAAAAA | "
        "cccccdddddcccccdddddcccccdddddccccccccccdddddcccccdddddcccccdddddccccc="
        "CCCCCDDDDDCCCCCDDDDDCCCCCDDDDDCCCCCCCCCCDDDDDCCCCCDDDDDCCCCCDDDDDCCCCC | "
        "eeeeefffffeeeeefffffeeeeefffffeeeeeeeeeefffffeeeeefffffeeeeefffffeeeee="
        "EEEEEFFFFFEEEEEFFFFFEEEEEFFFFFEEEEEEEEEEFFFFFEEEEEFFFFFEEEEEFFFFFEEEEE | "
        "ggggghhhhhggggghhhhhggggghhhhhgggggggggghhhhhggggghhhhhggggghhhhhggggg="
        "GGGGGHHHHHGGGGGHHHHHGGGGGHHHHHGGGGGGGGGGHHHHHGGGGGHHHHHGGGGGHHHHHGGGGG | "
        "iiiiijjjjjiiiiijjjjjiiiiijjjjjiiiiiiiiiijjjjjiiiiijjjjjiiiiijjjjjiiiii="
        "IIIIIJJJJJIIIIIJJJJJIIIIIJJJJJIIIIIIIIIIJJJJJIIIIIJJJJJIIIIIJJJJJIIIII | "
        "kkkkklllllkkkkklllllkkkkklllllkkkkkkkkkklllllkkkkklllllkkkkklllllkkkkk="
        "KKKKKLLLLLKKKKKLLLLLKKKKKLLLLLKKKKKKKKKKLLLLLKKKKKLLLLLKKKKKLLLLLKKKKK | "
        "mmmmmnnnnnmmmmmnnnnnmmmmmnnnnnmmmmmmmmmmnnnnnmmmmmnnnnnmmmmmnnnnnmmmmm="
        "MMMMMNNNNNMMMMMNNNNNMMMMMNNNNNMMMMMMMMMMNNNNNMMMMMNNNNNMMMMMNNNNNMMMMM | "
        "ooooopppppooooopppppooooopppppoooooooooopppppooooopppppooooopppppooooo="
        "OOOOOPPPPPOOOOOPPPPPOOOOOPPPPPOOOOOOOOOOPPPPPOOOOOPPPPPOOOOOPPPPPOOOOO | "
        "qqqqqrrrrrqqqqqrrrrrqqqqqrrrrrqqqqqqqqqqrrrrrqqqqqrrrrrqqqqqrrrrrqqqqq="
        "QQQQQRRRRRQQQQQRRRRRQQQQQRRRRRQQQQQQQQQQRRRRRQQQQQRRRRRQQQQQRRRRRQQQQQ | "
        "ssssstttttssssstttttssssstttttsssssssssstttttssssstttttssssstttttsssss="
        "SSSSSTTTTTSSSSSTTTTTSSSSSTTTTTSSSSSSSSSSTTTTTSSSSSTTTTTSSSSSTTTTTSSSSS | "
        ""_av));

    kv_list = builder.next();
    RED_CHECK(kv_list.size() == 114);
    RED_CHECK(bool(kv_list));
    RED_TEST(kv_list_to_str(kv_list) == str_concat(
        "uuuuuvvvvvuuuuuvvvvvuuuuuvvvvvuuuuuuuuuuvvvvvuuuuuvvvvvuuuuuvvvvvuuuuu="
        "UUUUUVVVVVUUUUUVVVVVUUUUUVVVVVUUUUUUUUUUVVVVVUUUUUVVVVVUUUUUVVVVVUUUUU | "
        "wwwwwxxxxxwwwwwxxxxxwwwwwxxxxxwwwwwwwwwwxxxxxwwwwwxxxxxwwwwwxxxxxwwwww="
        "WWWWWXXXXXWWWWWXXXXXWWWWWXXXXXWWWWWWWWWWXXXXXWWWWWXXXXXWWWWWXXXXXWWWWW | "
        "yyyyyzzzzzyyyyyzzzzzyyyyyzzzzzyyyyyyyyyyzzzzzyyyyyzzzzzyyyyyzzzzzyyyyy="
        "YYYYYZZZZZYYYYYZZZZZYYYYYZZZZZYYYYYYYYYYZZZZZYYYYYZZZZZYYYYYZZZZZYYYYY | "
        ""_av,
        s, s, s, s, s, s, s, s,
        "aaaaabbbbbaaaaabbbbbaaaaabbbbbaaaaaaaaaabbbbbaaaaabbbbbaaaaabbbbbaaaaa="
        "AAAAABBBBBAAAAABBBBBAAAAABBBBBAAAAAAAAAABBBBBAAAAABBBBBAAAAABBBBBAAAAA | "
        "cccccdddddcccccdddddcccccdddddccccccccccdddddcccccdddddcccccdddddccccc="
        "CCCCCDDDDDCCCCCDDDDDCCCCCDDDDDCCCCCCCCCCDDDDDCCCCCDDDDDCCCCCDDDDDCCCCC | "
        "eeeeefffffeeeeefffffeeeeefffffeeeeeeeeeefffffeeeeefffffeeeeefffffeeeee="
        "EEEEEFFFFFEEEEEFFFFFEEEEEFFFFFEEEEEEEEEEFFFFFEEEEEFFFFFEEEEEFFFFFEEEEE | "
        "ggggghhhhhggggghhhhhggggghhhhhgggggggggghhhhhggggghhhhhggggghhhhhggggg="
        "GGGGGHHHHHGGGGGHHHHHGGGGGHHHHHGGGGGGGGGGHHHHHGGGGGHHHHHGGGGGHHHHHGGGGG | "
        "iiiiijjjjjiiiiijjjjjiiiiijjjjjiiiiiiiiiijjjjjiiiiijjjjjiiiiijjjjjiiiii="
        "IIIIIJJJJJIIIIIJJJJJIIIIIJJJJJIIIIIIIIIIJJJJJIIIIIJJJJJIIIIIJJJJJIIIII | "
        "kkkkklllllkkkkklllllkkkkklllllkkkkkkkkkklllllkkkkklllllkkkkklllllkkkkk="
        "KKKKKLLLLLKKKKKLLLLLKKKKKLLLLLKKKKKKKKKKLLLLLKKKKKLLLLLKKKKKLLLLLKKKKK | "
        "mmmmmnnnnnmmmmmnnnnnmmmmmnnnnnmmmmmmmmmmnnnnnmmmmmnnnnnmmmmmnnnnnmmmmm="
        "MMMMMNNNNNMMMMMNNNNNMMMMMNNNNNMMMMMMMMMMNNNNNMMMMMNNNNNMMMMMNNNNNMMMMM | "
        ""_av));

    kv_list = builder.next();
    RED_CHECK(kv_list.size() == 32);
    RED_CHECK(bool(kv_list));
    RED_TEST(kv_list_to_str(kv_list) == str_concat(
        "ooooopppppooooopppppooooopppppoooooooooopppppooooopppppooooopppppooooo="
        "OOOOOPPPPPOOOOOPPPPPOOOOOPPPPPOOOOOOOOOOPPPPPOOOOOPPPPPOOOOOPPPPPOOOOO | "
        "qqqqqrrrrrqqqqqrrrrrqqqqqrrrrrqqqqqqqqqqrrrrrqqqqqrrrrrqqqqqrrrrrqqqqq="
        "QQQQQRRRRRQQQQQRRRRRQQQQQRRRRRQQQQQQQQQQRRRRRQQQQQRRRRRQQQQQRRRRRQQQQQ | "
        "ssssstttttssssstttttssssstttttsssssssssstttttssssstttttssssstttttsssss="
        "SSSSSTTTTTSSSSSTTTTTSSSSSTTTTTSSSSSSSSSSTTTTTSSSSSTTTTTSSSSSTTTTTSSSSS | "
        "uuuuuvvvvvuuuuuvvvvvuuuuuvvvvvuuuuuuuuuuvvvvvuuuuuvvvvvuuuuuvvvvvuuuuu="
        "UUUUUVVVVVUUUUUVVVVVUUUUUVVVVVUUUUUUUUUUVVVVVUUUUUVVVVVUUUUUVVVVVUUUUU | "
        "wwwwwxxxxxwwwwwxxxxxwwwwwxxxxxwwwwwwwwwwxxxxxwwwwwxxxxxwwwwwxxxxxwwwww="
        "WWWWWXXXXXWWWWWXXXXXWWWWWXXXXXWWWWWWWWWWXXXXXWWWWWXXXXXWWWWWXXXXXWWWWW | "
        "yyyyyzzzzzyyyyyzzzzzyyyyyzzzzzyyyyyyyyyyzzzzzyyyyyzzzzzyyyyyzzzzzyyyyy="
        "YYYYYZZZZZYYYYYZZZZZYYYYYZZZZZYYYYYYYYYYZZZZZYYYYYZZZZZYYYYYZZZZZYYYYY | "
        ""_av,
        s, s));

    kv_list = builder.next();
    RED_CHECK(kv_list.size() == 0);
    RED_CHECK(!bool(kv_list));
}
