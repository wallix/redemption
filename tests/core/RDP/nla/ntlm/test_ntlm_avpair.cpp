/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestNtlmAvPair
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "RDP/nla/credssp.hpp"
#include "RDP/nla/ntlm/ntlm_avpair.hpp"

#include "check_sig.hpp"

BOOST_AUTO_TEST_CASE(TestAvPair)
{
    NtlmAvPairList listAvPair;

    BOOST_CHECK_EQUAL(listAvPair.length(), 1);
    BOOST_CHECK_EQUAL(listAvPair.packet_length(), 4);

    const uint8_t tartempion[] = "NomDeDomaine";

    listAvPair.add(MsvAvNbDomainName, tartempion, sizeof(tartempion));

    BOOST_CHECK_EQUAL(listAvPair.length(), 2);
    BOOST_CHECK_EQUAL(listAvPair.packet_length(), 21);

    BStream stream;

    listAvPair.emit(stream);
    BOOST_CHECK_EQUAL(listAvPair.packet_length(), stream.size());
    listAvPair.print();


}
