/*
 * C++ Program to Implement Modular Exponentiation Algorithm
 */


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Test_ModEx
#include "system/redemption_unit_tests.hpp"

 #define LOGPRINT
#include "utils/log.hpp"

#include "system/big_integer.hpp"

BOOST_AUTO_TEST_CASE(TestModEx)
{
  BigInteger x("98765"), y("1234");
  BigInteger mod("123557");
  int res = 70506;
  BigInteger calc;

  calc = x.powAssignUnderMod(y, mod);
  BOOST_CHECK(BigInteger(res).getNumber() == calc.getNumber());

  x.setNumber("876823746827");
  y.setNumber("18446744073709");
  mod.setNumber("234892734");
  res = 132113723;
  calc = x.powAssignUnderMod(y, mod);
  BOOST_CHECK(BigInteger(res).getNumber() == calc.getNumber());

}


