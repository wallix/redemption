/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <string>

class ClientInfo;
class Inifile;

void headless_init_client_info(ClientInfo& client_info);
void headless_init_ini(Inifile& ini);

void load_headless_config_from_file(Inifile& ini, ClientInfo& client_info, char const* filename);
void load_headless_config_from_string(Inifile& ini, ClientInfo& client_info, char* str);

std::string headless_client_info_config_as_string(ClientInfo const& client_info);
