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
   Copyright (C) Wallix 2019
   Author(s): Christophe Grosjean

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "proxy_recorder/extract_user_domain.hpp"

RED_AUTO_TEST_CASE(TestExtractUserDomainFull)
{
    std::string nla_username("qaa16389@n1.hml.cnav");
    auto [user, domain] = extract_user_domain(nla_username.c_str());
    RED_CHECK(user == "qaa16389");
    RED_CHECK(domain == "n1.hml.cnav");
}

RED_AUTO_TEST_CASE(TestExtractUserDomainFullBV)
{
    std::string nla_username("qaa16389@n1.hml.cnav");
    auto [user, domain] = extract_user_domain(nla_username);
    auto expected_user = std::string("qaa16389");
    auto expected_domain = std::string("n1.hml.cnav");
    RED_CHECK_MEM(user, expected_user);
    RED_CHECK_MEM(domain, expected_domain);
}

RED_AUTO_TEST_CASE(TestExtractUserDomainFullNetbios)
{
    std::string nla_username("domain\\qaa16389");
    auto [user, domain] = extract_user_domain(nla_username.c_str());
    RED_CHECK(user == "qaa16389");
    RED_CHECK(domain == "domain");
}

RED_AUTO_TEST_CASE(TestExtractUserDomainFullBVNetbios)
{
    std::string nla_username("domain\\qaa16389");
    auto [user, domain] = extract_user_domain(nla_username);
    auto expected_user = std::string("qaa16389");
    auto expected_domain = std::string("domain");
    RED_CHECK_MEM(user, expected_user);
    RED_CHECK_MEM(domain, expected_domain);
}

RED_AUTO_TEST_CASE(TestExtractUserDomainUserOnly)   
{
    std::string nla_username("user");
    auto [user, domain] = extract_user_domain(nla_username.c_str());
    RED_CHECK(user == "user");
    RED_CHECK(domain == "");
}

RED_AUTO_TEST_CASE(TestExtractUserDomainFullBVUserOnly)
{
    std::string nla_username("user");
    auto [user, domain] = extract_user_domain(nla_username);
    auto expected_user = std::string("user");
    auto expected_domain = std::string("");
    RED_CHECK_MEM(user, expected_user);
    RED_CHECK_MEM(domain, expected_domain);
}
