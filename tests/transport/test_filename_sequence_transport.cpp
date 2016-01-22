/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestOutFilenameSequenceTransport
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include <stdlib.h>
#include <unistd.h>

#include "transport/out_filename_sequence_transport.hpp"
#include "fileutils.hpp"

BOOST_AUTO_TEST_CASE(TestOutFilenameSequenceTransport)
{
    OutFilenameSequenceTransport fnt(FilenameGenerator::PATH_FILE_PID_COUNT_EXTENSION, "/tmp/", "test_outfilenametransport", ".txt", getgid());
    fnt.send("We write, ", 10);
    fnt.send("and again, ", 11);
    fnt.send("and so on.", 10);

    fnt.next();
    fnt.send(" ", 1);
    fnt.send("A new file.", 11);

    BOOST_CHECK_EQUAL(filesize(fnt.seqgen()->get(0)), 31);
    BOOST_CHECK_EQUAL(filesize(fnt.seqgen()->get(1)), 12);

    fnt.disconnect();
    unlink(fnt.seqgen()->get(0));
    unlink(fnt.seqgen()->get(1));
}
