/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/file.hpp"

#include "qtclient/profile/cli_parse_profile.hpp"
#include "qtclient/profile/persistent_profiles.hpp"
#include "utils/strutils.hpp"
#include "utils/sugar/split.hpp"

#include <cstring>
#include <cerrno>


RED_AUTO_TEST_CASE(TestCliParser)
{
    // some values
    char const * argv[] = {
        "cmd",
        "-u", "user",
        "-p", "password",
        "-t", "10.10.13.12",
        "-P", "1793",
        "--size", "1600x900"
    };

    qtclient::Profile profile;

    RED_CHECK(profile.user_name == ""_av);
    RED_CHECK(profile.user_password == ""_av);
    RED_CHECK(profile.target_address == ""_av);
    RED_CHECK(profile.target_port == -1);
    RED_CHECK(profile.screen_info.width == 800);
    RED_CHECK(profile.screen_info.height == 600);
    RED_CHECK(profile.screen_info.bpp == BitsPerPixel::BitsPP16);

    qtclient::cli_parse_profile(argv, std::size(argv), profile);

    RED_CHECK(profile.user_name == "user"_av);
    RED_CHECK(profile.user_password == "password"_av);
    RED_CHECK(profile.target_address == "10.10.13.12"_av);
    RED_CHECK(profile.target_port == 1793);
    RED_CHECK(profile.screen_info.width == 1600);
    RED_CHECK(profile.screen_info.height == 900);
    RED_CHECK(profile.screen_info.bpp == BitsPerPixel::BitsPP16);
}

RED_AUTO_TEST_CASE_WF(TestLoadSaveProfiles, wf)
{
    qtclient::Profiles profiles;

    RED_CHECK(profiles.current_index == 0);
    RED_CHECK(profiles.choice_profile(0));
    RED_CHECK(!profiles.choice_profile(1));
    RED_CHECK(!profiles.choice_profile(2));
    RED_CHECK(profiles.choice_profile("Default"));
    RED_CHECK(!profiles.choice_profile("test"));
    RED_CHECK(!profiles.choice_profile("unknown"));
    RED_CHECK(profiles.current_index == 0);


    // save and get profile 1
    //@{
    RED_REQUIRE_MESSAGE(qtclient::save_profiles(wf, profiles), strerror(errno));

    std::string p1;
    RED_REQUIRE(FileContentsError::None == append_file_contents(wf.c_str(), p1));

    RED_CHECK(*get_lines(p1).begin() == "current-profile Default"_av);
    //@}


    auto& profile1 = profiles.add_profile("test");
    profile1.screen_info.width = 1234;

    RED_REQUIRE(profiles.size() == 2);

    // save and get profile 1 and 2 then compare
    //@{
    RED_REQUIRE_MESSAGE(qtclient::save_profiles(wf, profiles), strerror(errno));

    std::string p2;
    RED_REQUIRE(FileContentsError::None == append_file_contents(wf.c_str(), p2));

    RED_CHECK(chars_view(p2) == str_concat(
        p1, "\nprofile-name test\nrdp 1\nport -1\nsize 1234x600x16"_av,
        chars_view(p1).drop_front(75)
    ));
    //@}


    // select profile
    RED_CHECK(profiles.current_index == 0);
    RED_CHECK(profiles.choice_profile(0));
    RED_CHECK(profiles.current_index == 0);
    RED_CHECK(profiles.choice_profile(1));
    RED_CHECK(profiles.current_index == 1);
    RED_CHECK(!profiles.choice_profile(2));
    RED_CHECK(profiles.current_index == 1);
    RED_CHECK(profiles.choice_profile("Default"));
    RED_CHECK(profiles.current_index == 0);
    RED_CHECK(profiles.choice_profile("test"));
    RED_CHECK(profiles.current_index == 1);
    RED_CHECK(!profiles.choice_profile("unknown"));
    RED_CHECK(profiles.current_index == 1);


    // compare current-profile value (Default -> test)
    //@{
    RED_REQUIRE_MESSAGE(qtclient::save_profiles(wf, profiles), strerror(errno));

    p2.clear();
    RED_REQUIRE(FileContentsError::None == append_file_contents(wf.c_str(), p2));

    RED_CHECK(*get_lines(p2).begin() == "current-profile test"_av);
    RED_CHECK(chars_view(p2) == str_concat(
        "current-profile test\n"_av, chars_view(p1).drop_front(24),
        "\nprofile-name test\nrdp 1\nport -1\nsize 1234x600x16"_av,
        chars_view(p1).drop_front(75)
    ));
    //@}


    // load profiles and compare
    qtclient::Profiles profiles2 = qtclient::load_profiles(wf);
    RED_REQUIRE(profiles.size() == profiles2.size());
    RED_REQUIRE(profiles[0].screen_info.width == profiles2[0].screen_info.width);
    RED_REQUIRE(profiles[1].screen_info.width == profiles2[1].screen_info.width);


    wf.set_removed();
}
