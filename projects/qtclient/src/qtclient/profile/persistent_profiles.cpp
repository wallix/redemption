/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qtclient/profile/persistent_profiles.hpp"
#include "qtclient/profile/profile_as_options.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/sugar/split.hpp"
#include "utils/fileutils.hpp"
#include "utils/strutils.hpp"
#include "utils/sugar/finally.hpp"
#include "utils/redemption_info_version.hpp"
#include "utils/log.hpp"

#include <iostream>


namespace qtclient
{

bool save_profiles(char const* filename, Profiles const& profiles)
{
    unique_fd file = unique_fd(filename, O_WRONLY | O_CREAT | O_TRUNC, 0664);

    if (!file.is_open()) {
        return false;
    }

    std::string str;
    str.reserve(512 * profiles.size());

    str_append(str, "current-profile ", profiles.current_profile().profile_name, '\n');

    auto to_str = [](auto const& value) {
        using T = std::decay_t<decltype(value)>;
        /**/ if constexpr (std::is_same_v<bool, T>) return value ? '1' : '0';
        else if constexpr (std::is_integral_v<T>) return int_to_decimal_chars(value);
        else if constexpr (std::is_enum_v<T>) return int_to_decimal_chars(underlying_cast(value));
        else return value;
    };

    for (auto const& profile : profiles) {
        str_append(
            str, "\n"
            "profile-name ", profile.profile_name, "\n"
            "rdp ", to_str(profile.protocol == ProtocolMod::RDP), "\n"
            "port ", to_str(profile.target_port), "\n"
            "size ",
                to_str(profile.screen_info.width), 'x',
                to_str(profile.screen_info.height), 'x',
                to_str(profile.screen_info.bpp), "\n"
            "span ", to_str(profile.is_spanning), "\n"
            "enable-clipboard ", to_str(profile.enable_clipboard), "\n"
            "tls-min-level ", to_str(profile.tls_min_level), "\n"
            "tls-max-level ", to_str(profile.tls_max_level), "\n"
            "cipher ", profile.cipher_string, "\n"
            "enable-sound ", to_str(profile.enable_sound),"\n"
            "rdp-performance-flags ", to_str(profile.rdp5_performance_flags), "\n"
            "enable-nla ", to_str(profile.enable_nla), "\n"
            "enable-tls ", to_str(profile.enable_tls), "\n"
            "rdp-verbose ", to_str(profile.rdp_verbose), "\n"
            "remote-app ", to_str(profile.enable_remote_app), "\n"
            "remote-cmd ", profile.remote_app_cmd, "\n"
            "remote-dir ", profile.remote_app_working_directory, "\n"
            "enable-drive ", to_str(profile.enable_drive), "\n"
            "drive-dir ", profile.drive_path, "\n"
            "layout ", to_str(profile.key_layout), "\n"
            "enable-recording ", to_str(profile.enable_recording), "\n"
        );
    }

    chars_view data = str;
    ssize_t n = 0;
    while (0 < (n = ::write(file.fd(), data.data(), data.size()))) {
        data = data.drop_front(checked_int(n));
        if (data.empty()) {
            return true;
        }
    }
    return false;
}

Profiles load_profiles(char const* filename)
{
    Profiles profiles;

    std::string contents;
    switch (append_file_contents(filename, contents, 0xffff)) {
        case FileContentsError::None: {
            if (contents.empty()) {
                break;
            }

            // reserve a new line for '\0' transformation
            if (contents.back() != '\n') {
                contents += '\n';
            }

            struct RebuildableOptions
            {
                using CliOptions = decltype(profile_as_options(std::declval<Profile&>()));

                RebuildableOptions(Profile& profile)
                : u{.options = profile_as_options(profile)}
                {}

                ~RebuildableOptions()
                {
                    if (!is_initialized) {
                        u.options.~CliOptions();
                    }
                }

                void rebuild(Profile& profile)
                {
                    u.options.~CliOptions();
                    is_initialized = false;
                    new (&u.options) CliOptions(profile_as_options(profile));
                    is_initialized = true;
                }

                union U
                {
                    CliOptions options;
                    char dummy;
                };

                U u;
                bool is_initialized = true;
            };

            RebuildableOptions rebuildable_options(profiles.front());

            auto parse_option = [](std::string_view name, cli::ParseResult& pr, auto const& opt, cli::Res& res){
                if constexpr (!std::is_same_v<cli::Helper const&, decltype(opt)>) {
                    if constexpr (!std::is_same_v<cli::detail::uninit_t, decltype(opt._long_name)>) {
                        if (opt._long_name != name) {
                            return true;
                        }
                        #ifndef IN_IDE_PARSER
                        res = opt._parser(pr);
                        #endif
                        return false;
                    }
                }
                return true;
            };

            int iline = 0;
            cli::ParseResult pr {};
            std::string_view current_profile;

            for (auto line : get_lines(contents)) {
                ++iline;

                if (line.empty()) {
                    continue;
                }

                auto p = std::find(line.begin(), line.end(), ' ');

                // replace '\n'
                *line.end() = '\0';
                auto key = chars_view(line.begin(), p).as<std::string_view>();
                auto value = chars_view(p + (line.end() != p), line.end()).as<std::string_view>();

                using namespace std::string_view_literals;

                if (key == "profile-name"sv) {
                    auto* profile = profiles.find(value);
                    rebuildable_options.rebuild(profile ? *profile : profiles.add_profile(value));
                    continue;
                }

                if (key == "current-profile"sv) {
                    current_profile = value;
                    continue;
                }

                using cli::Res;
                auto parser = [&](auto const&... opts) {
                    Res r = Res::Ok;
                    pr.str = value.data();
                    return (... && (r == Res::Ok && parse_option(key, pr, opts, r))) ? Res::BadOption : r;
                };

                if (rebuildable_options.u.options(parser) != Res::Ok) {
                    LOG(LOG_WARNING, "Parse error %s:%d: \"%s\".", filename, iline, line.data());
                }
            }

            profiles.choice_profile(current_profile);
            break;
        }

        case FileContentsError::Read:
        case FileContentsError::Open:
        case FileContentsError::Stat:
            LOG(LOG_WARNING, "Parse error %s: %s", filename, strerror(errno));
            break;
    }

    return profiles;
}

} // namespace qtclient
