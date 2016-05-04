/* 
 * C++ Program to Implement Modular Exponentiation Algorithm
 */


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestModEx
#include <boost/test/auto_unit_test.hpp>


#include <iostream>
#define ll long long
using namespace std; 
 



/* 
 * Function to calculate modulus of x raised to the power y 
 */
ll modular_exp(ll base, ll exponent, int modulus)
{
    ll result = 1;
    while (exponent > 0)
    {
        if (exponent % 2 == 1)
            result = (result * base) % modulus;
        exponent = exponent >> 1;
        base = (base * base) % modulus;
    }
    return result;
}

unsigned int pModulaire2(unsigned int base, unsigned int exponent, unsigned int modulus){
   unsigned result = 1;

   while (exponent > 0) {
      if ((exponent & 1) > 0) result = (result * base) % modulus;
      exponent >>= 1;
      base = (base * base) % modulus;
   }

   return result;
}

BOOST_AUTO_TEST_CASE(TestModEx)
{

    ll x = 5, y = 1063;
    int mod = 2159;

    BOOST_CHECK_EQUAL(modular_exp(x, y, mod), 588);


    x = 98765;
    y = 1234;
    mod = 123557;
	BOOST_CHECK_EQUAL(pModulaire2(x, y, mod), 123273);

}

