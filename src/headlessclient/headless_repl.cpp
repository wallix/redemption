/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <cstring>
#include <unistd.h>

#include "headlessclient/headless_repl.hpp"


HeadlessRepl::CommandBuffer::Result
HeadlessRepl::CommandBuffer::read_line(int fd)
{
    char * pos = std::find(start_line, end_buffer, '\n');

    if (pos != end_buffer) {
        char* p = start_line;
        start_line = pos + 1;
        return Result{ResultType::Extracted, {p, pos}};
    }

    std::size_t remaining = checked_int(end_buffer - start_line);
    memmove(inbuf, start_line, remaining);
    start_line = inbuf;
    end_buffer = inbuf + remaining;

    ssize_t n = ::read(fd, end_buffer, std::size(inbuf) - remaining);
    if (REDEMPTION_UNLIKELY(n <= 0)) {
        return Result{(n < 0) ? ResultType::Error : ResultType::Eof, {}};
    }

    end_buffer += n;
    pos = std::find(end_buffer - n, end_buffer, '\n');

    if (pos != end_buffer) {
        start_line = pos + 1;
        return Result{ResultType::Extracted, {inbuf, pos}};
    }

    return Result{ResultType::Incomplete, {inbuf, pos}};
}

chars_view HeadlessRepl::read_command()
{
    auto [rtype, cmd] = command_buffer_.read_line();
    switch (rtype) {
        case CommandBuffer::ResultType::Extracted:
            if (is_incomplete_) {
                fprintf(stderr, "Command line too long ; ignored\n");
                is_incomplete_ = false;
            }
            break;
        case CommandBuffer::ResultType::Error: {
            int errnum = errno;
            fprintf(stderr, "Repl read error: %d: %s\n", errnum, strerror(errnum));
            is_eof_ = true;
            break;
        }
        case CommandBuffer::ResultType::Eof:
            is_eof_ = true;
            break;
        case CommandBuffer::ResultType::Incomplete:
            is_incomplete_ = true;
            break;
    }

    return cmd;
}
