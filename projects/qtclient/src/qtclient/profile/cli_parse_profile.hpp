/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

namespace qtclient
{

class Profile;

bool cli_parse_profile(char const* const argv[], int argc, Profile& profile);

} // namespace qtclient
