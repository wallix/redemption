/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2014
*   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
*/


#define RED_TEST_MODULE TestLocalFd
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/unique_fd.hpp"

#include <string>

RED_AUTO_TEST_CASE(TestLocalFd)
{
    std::string const unknown_file = "/tmp/unique_fd_unknown_file";
    RED_CHECK(!unique_fd(unknown_file, O_RDONLY).is_open());
    RED_CHECK(!bool(unique_fd(unknown_file, O_RDONLY)));
    RED_CHECK(!unique_fd(unknown_file, O_RDONLY));

    unique_fd fd(unknown_file, O_RDONLY | O_CREAT, 0666);
    RED_CHECK(fd.is_open());
    RED_CHECK_GE(fd.fd(), 0);

    unlink(unknown_file.c_str());
}
