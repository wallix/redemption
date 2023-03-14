/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

namespace qtclient
{

class Profile;

enum class CliResult : char
{
    Ok,
    Exit,
    Error,
};

CliResult cli_parse_profile(int argc, char const* const argv[], Profile& profile);

} // namespace qtclient
