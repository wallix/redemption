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
  Copyright (C) Wallix 2013
  Author(s): Christophe Grosjean, Meng Tan
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestAuthentifierNew
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT
#include "log.hpp"

#include "authentifier.hpp"

BOOST_AUTO_TEST_CASE(TestAuthentifier)
{
    class FakeModuleManager : public MMApi
    {
        public:
        FakeModuleManager() {}
        ~FakeModuleManager() {}
        virtual void remove_mod() {}
        virtual void new_mod(int target_module) {}
        virtual void record() {}
    } mm;

    

}

