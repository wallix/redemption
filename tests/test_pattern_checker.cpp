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
   Copyright (C) Wallix 2010
   Author(s): Jonathan Poelen
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestPatternChercher

//#define LOGNULL
#define LOGPRINT

#include "system/redemption_unit_tests.hpp"

#include "capture/utils/pattern_checker.hpp"

BOOST_AUTO_TEST_CASE(TestPattern)
{
    for (int i = 0; i < 2; ++i) {
        struct Auth : NullAuthentifier
        {
            std::string reason;
            std::string message;

            void report(const char * reason, const char * message) override {
                this->reason = reason;
                this->message = message;
            }
        } authentifier;
        PatternsChecker checker(&authentifier, i ? ".de." : nullptr, i ? nullptr : ".de.");

        auto const reason = i ? "FINDPATTERN_KILL" : "FINDPATTERN_NOTIFY";

        checker(cstr_array_view("Gestionnaire"));

        BOOST_CHECK(authentifier.reason.empty());
        BOOST_CHECK(authentifier.message.empty());

        checker(cstr_array_view("Gestionnaire de serveur"));

        BOOST_CHECK_EQUAL(authentifier.reason,  reason);
        BOOST_CHECK_EQUAL(authentifier.message, "$ocr:.de.|Gestionnaire de serveur");

        checker(cstr_array_view("Gestionnaire de licences TS"));

        BOOST_CHECK_EQUAL(authentifier.reason,  reason);
        BOOST_CHECK_EQUAL(authentifier.message, "$ocr:.de.|Gestionnaire de licences TS");
    }
}
