/* 
 * C++ Program to Implement Modular Exponentiation Algorithm
 */


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestModEx
#include <boost/test/auto_unit_test.hpp>


#include "system/big_integer.hpp"

#include <iostream>
#include <stdlib.h> 



BOOST_AUTO_TEST_CASE(TestModEx)
{
 
  BigInteger x("5"), y("1063");
  BigInteger mod("2159");
  BigInteger res("123273");
BigInteger calc;
 // BOOST_CHECK_EQUAL((x.powAssignUnderMod(y, mod)).getNumber(), 588);
 // printf("x.powAssignUnderMod(y, mod) = %s", x.powAssignUnderMod(y, mod).getNumber().c_str());

  x.setNumber("98765");
  y.setNumber("1234");
  mod.setNumber("123557");
 // long long res = atoi( (x.powAssignUnderMod(y, mod)).getNumber().c_str() );

calc = BigInteger(x.powAssignUnderMod(y, mod));

/*
  printf("x.powAssignUnderMod(y, mod) = %s", x.powAssignUnderMod(y, mod).getNumber().c_str());
  std::cout << "x.powAssignUnderMod(y, mod) = " << x.powAssignUnderMod(y, mod).getNumber() << endl;
*/
//  BOOST_CHECK(0 == memcmp(target1, target, sizeof(target)));
  //BOOST_CHECK_EQUAL(res, 123273);
//  BOOST_CHECK_EQUAL(x.powAssignUnderMod(y, mod), res);
//   BOOST_CHECK(0 == memcmp(calc, res, sizeof(calc)));

        BOOST_CHECK_EQUAL(memcmp(res, calc, sizeof(res)), 0);




/*
  x.setNumber("876823746827");
  y.setNumber("18446744073709");
  mod.setNumber("234892734");
  printf("x.powAssignUnderMod(y, mod) = %s", x.powAssignUnderMod(y, mod).getNumber().c_str());
//  BOOST_CHECK_EQUAL(x.powAssignUnderMod(y, mod), 123273);
*/
}


