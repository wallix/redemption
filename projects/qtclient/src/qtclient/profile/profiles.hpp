/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "qtclient/profile/profile.hpp"

#include <vector>


namespace qtclient
{

struct Profiles : std::vector<Profile>
{
    Profiles()
    : std::vector<Profile>(1)
    {
        front().profile_name = "Default";
    }

    Profile& add_profile(std::string_view name, bool selected = false)
    {
        if (selected) {
            current_index = size();
        }

        auto& profile = emplace_back();
        profile.profile_name = name;
        return profile;
    }

    bool choice_profile(std::size_t id) noexcept
    {
        if (id < size()) {
            current_index = id;
            return true;
        }
        return false;
    }

    bool choice_profile(std::string_view name) noexcept
    {
        for (auto const& profile : *this) {
            if (profile.profile_name == name) {
                current_index = checked_int(&profile - data());
                return true;
            }
        }
        return false;
    }

    Profile* find(std::string_view name) noexcept
    {
        for (auto& profile : *this) {
            if (profile.profile_name == name) {
                return &profile;
            }
        }
        return nullptr;
    }

    Profile& current_profile() noexcept
    {
        return operator[](current_index);
    }

    Profile const& current_profile() const noexcept
    {
        return operator[](current_index);
    }

    std::size_t current_index = 0;
};

Profiles load_profiles(char const* filename);
bool save_profiles(char const* filename, Profiles const& profiles);
bool cli_parse_options(char const* const argv[], int argc, Profile& profile);

} // namespace qtclient
