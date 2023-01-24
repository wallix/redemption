/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "utils/uninit_buffer.hpp"
#include "acl/auth_api.hpp"


struct HeadlessSessionLog : SessionLogApi
{
    void log6(LogId id, KVLogList kv_list) override;

    void report(const char * reason, const char * message) override;

    void set_control_owner_ctx([[maybe_unused]] chars_view name) override
    {
    }

private:
    UninitDynamicBuffer buffer;
};
