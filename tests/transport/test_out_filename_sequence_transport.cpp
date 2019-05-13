/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/file.hpp"

#include "transport/out_filename_sequence_transport.hpp"

RED_AUTO_TEST_CASE_WD(TestOutFilenameSequenceTransport, wd)
{
    OutFilenameSequenceTransport fnt(
        FilenameGenerator::PATH_FILE_COUNT_EXTENSION,
        wd.dirname(), "test_outfilenametransport", ".txt", 0, ReportError{});
    fnt.send("We write, ", 10);
    fnt.send("and again, ", 11);
    fnt.send("and so on.", 10);

    fnt.next();
    fnt.send(" ", 1);
    fnt.send("A new file.", 11);

    fnt.disconnect();

    RED_TEST_FSIZE(wd.add_file("test_outfilenametransport-000000.txt"), 31);
    RED_TEST_FSIZE(wd.add_file("test_outfilenametransport-000001.txt"), 12);
}
