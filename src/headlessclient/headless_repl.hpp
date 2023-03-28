/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "utils/sugar/noncopyable.hpp"
#include "utils/sugar/array_view.hpp"

struct HeadlessRepl
{
    bool is_eof() const
    {
        return is_eof_;
    }

    chars_view read_command(int fd);

    struct CommandBuffer : private noncopyable
    {
        enum class ResultType
        {
            Extracted,
            Incomplete,
            Error,
            Eof,
        };

        struct Result
        {
            ResultType type;
            chars_view cmd;
        };

        CommandBuffer()
        {
            start_line = inbuf;
            end_buffer = start_line;
        }

        Result read_line(int fd);

    private:
        char* end_buffer;
        char* start_line;
        char inbuf[1024];
    };

private:
    bool is_eof_ = false;
    bool is_incomplete_ = false;
    CommandBuffer command_buffer_;
};
