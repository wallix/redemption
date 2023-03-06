/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qtclient/profile/persistent_profiles.hpp"
#include "qtclient/profile/profile_as_cli_options.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/sugar/split.hpp"
#include "utils/fileutils.hpp"
#include "utils/strutils.hpp"
#include "utils/sugar/finally.hpp"
#include "utils/redemption_info_version.hpp"
#include "utils/log.hpp"

#include <iostream>

namespace
{

static_string<15> screen_info_to_chars(ScreenInfo screen_info)
{
    static_string<15> str;
    str.delayed_build([&](auto& array){
        auto* p = array.data();
        auto cpy = [&](chars_view str){
            memcpy(p, str.data(), str.size());
            return p + str.size();
        };
        p = cpy(int_to_decimal_chars(screen_info.width));
        *p++ = 'x';
        p = cpy(int_to_decimal_chars(screen_info.height));
        *p++ = 'x';
        p = cpy(int_to_decimal_chars(underlying_cast(screen_info.bpp)));
        return checked_int(p - array.data());
    });
    return str;
}

template<std::size_t Int, class T>
T const& option_value(cli::detail::indexed_option<Int, T> const& value)
{
    return value.option;
}

template<std::size_t... Ints, class Tuple>
void concat_tuple_option(std::string& str, chars_view profile_name, std::index_sequence<Ints...>, Tuple const& t)
{
    str_append(str, "\nprofile-name "_av, profile_name, option_value<Ints>(t)..., '\n');
}

template<class T>
std::string_view extract_long_name(T const& option)
{
    if constexpr (qtclient::serializable_option<T>::value) return option._long_name;
    else return {};
}

template<class T>
decltype(auto) extract_option_as_string(T const& option)
{
    if constexpr (qtclient::serializable_option<T>::value) {
        auto& value = *option._parser.value;
        using U = std::decay_t<decltype(value)>;
        /**/ if constexpr (std::is_same_v<bool, U>) return value ? "1"_av : "0"_av;
        else if constexpr (std::is_integral_v<U>) return int_to_decimal_chars(value);
        else if constexpr (std::is_enum_v<U>) return int_to_decimal_chars(underlying_cast(value));
        else if constexpr (std::is_same_v<ScreenInfo, U>) return screen_info_to_chars(value);
        else return value;
    }
    else {
        return chars_view();
    }
}

template<std::size_t... Ints, class... Options>
void concat_options(std::string& str, chars_view profile_name, std::index_sequence<Ints...>, Options const&... options)
{
    struct Tuple
    : cli::detail::indexed_option<Ints*4, chars_view>...
    , cli::detail::indexed_option<Ints*4+1, std::string_view>...
    , cli::detail::indexed_option<Ints*4+2, chars_view>...
    , cli::detail::indexed_option<Ints*4+3, chars_view>...
    {};

    concat_tuple_option(str, profile_name, std::make_index_sequence<sizeof...(Ints) * 4>(), Tuple{
        {{qtclient::serializable_option<Options>::value ? "\n"_av : ""_av}}...,
        {{extract_long_name(options)}}...,
        {{qtclient::serializable_option<Options>::value ? " "_av : ""_av}}...,
        {{extract_option_as_string(options)}}...
    });
}

}

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

    for (auto const& profile : profiles) {
        profile_as_cli_options(const_cast<Profile&>(profile))([&](auto&&... options) {
            concat_options(str, profile.profile_name, std::make_index_sequence<sizeof...(options)>(), options...);
        });
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
                using CliOptions = decltype(profile_as_cli_options(std::declval<Profile&>()));

                RebuildableOptions(Profile& profile)
                : u{.options = profile_as_cli_options(profile)}
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
                    new (&u.options) CliOptions(profile_as_cli_options(profile));
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
