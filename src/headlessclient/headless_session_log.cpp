/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "headlessclient/headless_session_log.hpp"
#include "utils/key_qvalue_pairs.hpp"


void HeadlessSessionLog::log6(LogId id, KVLogList kv_list)
{
    std::size_t len = safe_size_for_log_format_append_info(id, kv_list);
    char* p = buffer.grow_without_copy(len).as_charp();
    char* end = log_format_append_info(p, id, kv_list);
    fprintf(stderr, "[headless] %.*s\n", int(end - p), p);
}

void HeadlessSessionLog::report(const char * reason, const char * message)
{
    fprintf(stderr, "Report: %s: %s\n", reason, message);
}
