/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "qtclient/profile/profiles.hpp"

namespace qtclient
{

Profiles load_profiles(char const* filename);
bool save_profiles(char const* filename, Profiles const& profiles);

} // namespace qtclient
