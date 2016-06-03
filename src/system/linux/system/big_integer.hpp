/*
 * Big Integer library in C++, single file implementation.
 */

#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

#define LOGPRINT
#include "utils/log.hpp"
 
#define MAX 10000 // for strings

using namespace std;
class BigInteger {
private:
    string number;
    bool sign;
public:
    BigInteger() { // empty constructor initializes zero
        number = "0";
        sign = false;
    }


    BigInteger(string s) { // "string" constructor
        if( isdigit(s[0]) ) { // if not signed
            setNumber(s);
            sign = false; // +ve
        } else {
            setNumber( s.substr(1) );
            sign = (s[0] == '-');
        }
    }


    BigInteger(string s, bool sin) { // "string" constructor
        setNumber( s );
        setSign( sin );
    }



    BigInteger(int n) { // "int" constructor
        stringstream ss;
        string s;
        ss << n;
        ss >> s;


        if( isdigit(s[0]) ) { // if not signed
            setNumber( s );
            setSign( false ); // +ve
        } else {
            setNumber( s.substr(1) );
            setSign( s[0] == '-' );
        }
    }

    void setNumber(string s) {
        number = s;
    }

    const string& getNumber() { // retrieves the number
        return number;
    }
    void setSign(bool s) {
        sign = s;
    }

    const bool& getSign() {
        return sign;
    }

    BigInteger absolute() {
        return BigInteger( getNumber() ); // +ve by default
    }

    void operator = (BigInteger b) {
        setNumber( b.getNumber() );
        setSign( b.getSign() );
    }

    bool operator == (BigInteger b) {
        return equals((*this) , b);
    }

    bool operator != (BigInteger b) {
        return ! equals((*this) , b);
    }

    bool operator > (BigInteger b) {
        return greater((*this) , b);
    }

    bool operator < (BigInteger b) {
        return less((*this) , b);
    }

    bool operator >= (BigInteger b) {
        return equals((*this) , b)
            || greater((*this), b);
    }

    bool operator <= (BigInteger b) {
        return equals((*this) , b)
            || less((*this) , b);
    }

    BigInteger& operator ++() { // prefix
        (*this) = (*this) + 1;
        return (*this);
    }

    BigInteger  operator ++(int) { // postfix
        BigInteger before = (*this);

        (*this) = (*this) + 1;

        return before;
    }

    BigInteger& operator --() { // prefix
        (*this) = (*this) - 1;
        return (*this);

    }


    BigInteger  operator --(int) { // postfix
        BigInteger before = (*this);

        (*this) = (*this) - 1;

        return before;
    }


    BigInteger operator + (BigInteger b) {
        BigInteger addition;
        if( getSign() == b.getSign() ) { // both +ve or -ve
            addition.setNumber( add(getNumber(), b.getNumber() ) );
            addition.setSign( getSign() );
        } else { // sign different
            if( absolute() > b.absolute() ) {
                addition.setNumber( subtract(getNumber(), b.getNumber() ) );
                addition.setSign( getSign() );
            } else {
                addition.setNumber( subtract(b.getNumber(), getNumber() ) );
                addition.setSign( b.getSign() );
            }
        }
        if(addition.getNumber() == "0") // avoid (-0) problem
            addition.setSign(false);

        return addition;
    }


    BigInteger operator - (BigInteger b) {
        b.setSign( ! b.getSign() ); // x - y = x + (-y)
        return (*this) + b;
    }


    BigInteger operator * (BigInteger b) {
        BigInteger mul;

        mul.setNumber( multiply(getNumber(), b.getNumber() ) );
        mul.setSign( getSign() != b.getSign() );

        if(mul.getNumber() == "0") // avoid (-0) problem
            mul.setSign(false);

        return mul;
    }

    // Warning: Denomerator must be within "long long" size not "BigInteger"
    BigInteger operator / (BigInteger b) {
        long long den = toInt( b.getNumber() );
        BigInteger div;

        div.setNumber( divide(getNumber(), den).first );
        div.setSign( getSign() != b.getSign() );

        if(div.getNumber() == "0") // avoid (-0) problem
            div.setSign(false);

        return div;
    }


    // Warning: Denomerator must be within "long long" size not "BigInteger"
    BigInteger operator % (BigInteger b) {
        long long den = toInt( b.getNumber() );

        BigInteger rem;
        long long rem_int = divide(number, den).second;
        rem.setNumber( toString(rem_int) );
        rem.setSign( getSign() != b.getSign() );

        if(rem.getNumber() == "0") // avoid (-0) problem
            rem.setSign(false);

        return rem;
    }


    BigInteger& operator += (BigInteger b) {
        (*this) = (*this) + b;
        return (*this);
    }

    
    BigInteger& operator -= (BigInteger b){
        (*this) = (*this) - b;
        return (*this);
    }


    BigInteger& operator *= (BigInteger b) {
        (*this) = (*this) * b;
        return (*this);
    }

    BigInteger& operator /= (BigInteger b) {
        (*this) = (*this) / b;
        return (*this);
    }

    BigInteger& operator %= (BigInteger b) {
        (*this) = (*this) % b;
        return (*this);
    }

    BigInteger& operator [] (int n) {
        return *(this + (n*sizeof(BigInteger)));
    }

    BigInteger operator -(){ // unary minus sign
        return (*this) * -1;
    }

    operator string() { // for conversion from BigInteger to string
        string signedString = ( getSign() ) ? "-" : ""; // if +ve, don't print + sign
        signedString += number;
        return signedString;
    }


private:

    bool equals(BigInteger n1, BigInteger n2) {
        return n1.getNumber() == n2.getNumber()
               && n1.getSign() == n2.getSign();
    }

    bool less(BigInteger n1, BigInteger n2) {
        bool sign1 = n1.getSign();
        bool sign2 = n2.getSign();

        if(sign1 && ! sign2) // if n1 is -ve and n2 is +ve
            return true;

        else if(! sign1 && sign2)
            return false;

        else if(! sign1) { // both +ve
            if(n1.getNumber().length() < n2.getNumber().length() )
                return true;
            if(n1.getNumber().length() > n2.getNumber().length() )
                return false;
            return n1.getNumber() < n2.getNumber();
        } else { // both -ve
            if(n1.getNumber().length() > n2.getNumber().length())
                return true;
            if(n1.getNumber().length() < n2.getNumber().length())
                return false;
            return n1.getNumber().compare( n2.getNumber() ) > 0; // greater with -ve sign is LESS
        }
    }

    bool greater(BigInteger n1, BigInteger n2){
        return ! equals(n1, n2) && ! less(n1, n2);
    }

    string add(string number1, string number2){
        string add = (number1.length() > number2.length()) ?  number1 : number2;
        char carry = '0';
        int differenceInLength = abs( static_cast<int> (number1.size() - number2.size()) );

        if(number1.size() > number2.size())
            number2.insert(0, differenceInLength, '0'); // put zeros from left

        else// if(number1.size() < number2.size())
            number1.insert(0, differenceInLength, '0');

        for(int i=number1.size()-1; i>=0; --i) {
            add[i] = ((carry-'0')+(number1[i]-'0')+(number2[i]-'0')) + '0';

            if(i != 0) {
                if(add[i] > '9') {
                    add[i] -= 10;
                    carry = '1';
                } else
                    carry = '0';
            }
        }
        if(add[0] > '9') {
            add[0]-= 10;
            add.insert(0,1,'1');
        }
        return add;
    }


    string subtract(string number1, string number2) {
        string sub = (number1.length()>number2.length())? number1 : number2;
        int differenceInLength = abs( static_cast<int>(number1.size() - number2.size()) );

        if(number1.size() > number2.size())
            number2.insert(0, differenceInLength, '0');

        else
            number1.insert(0, differenceInLength, '0');

        for(int i=number1.length()-1; i>=0; --i) {
            if(number1[i] < number2[i]) {
                number1[i] += 10;
                number1[i-1]--;
            }
            sub[i] = ((number1[i]-'0')-(number2[i]-'0')) + '0';
        }

        while(sub[0]=='0' && sub.length()!=1) // erase leading zeros
            sub.erase(0,1);

        return sub;
    }


    string multiply(string n1, string n2){
        if(n1.length() > n2.length())
            n1.swap(n2);

        string res = "0";
        for(int i=n1.length()-1; i>=0; --i) {
            string temp = n2;
            int currentDigit = n1[i]-'0';
            int carry = 0;

            for(int j=temp.length()-1; j>=0; --j) {
                temp[j] = ((temp[j]-'0') * currentDigit) + carry;

                if(temp[j] > 9) {
                    carry = (temp[j]/10);
                    temp[j] -= (carry*10);
                } else
                    carry = 0;

                temp[j] += '0'; // back to string mood
            }

            if(carry > 0)
                temp.insert(0, 1, (carry+'0'));

            temp.append((n1.length()-i-1), '0'); // as like mult by 10, 100, 1000, 10000 and so on

            res = add(res, temp); // O(n)
        }

        while(res[0] == '0' && res.length()!=1) // erase leading zeros
            res.erase(0,1);

        return res;
    }


    pair<string, long long> divide(string n, long long den) {
        long long rem = 0;
        string result;
        result.resize(MAX);

        for(int indx=0, len = n.length(); indx<len; ++indx) {
            rem = (rem * 10) + (n[indx] - '0');
            result[indx] = rem / den + '0';
            rem %= den;
        }
        result.resize( n.length() );

        while( result[0] == '0' && result.length() != 1)
            result.erase(0,1);

        if(result.length() == 0)
            result = "0";

        return make_pair(result, rem);
    }

    string toString(long long n) {
        stringstream ss;
        string temp;

        ss << n;
        ss >> temp;

        return temp;
    }


    long long toInt(string s) {
       long long sum = 0;

        for(unsigned int i=0; i<s.length(); i++)
            sum = (sum*10) + (s[i] - '0');

        return sum;
    }

    int cmpWithoutSign(const BigInteger& a, const BigInteger& b) {
        int n = static_cast<int>(a.number.size());
        if (n < static_cast<int>(b.number.size()))
            n = static_cast<int>(b.number.size());

        for (int i = n-1; i >= 0; --i) {
            unsigned short a_digit = 0;
            unsigned short b_digit = 0;

            if (i < static_cast<int>(a.number.size())) {
                a_digit = a.number[i];
            }

            if (i < static_cast<int>(b.number.size())) {
                b_digit = b.number[i];
            }

            if (a_digit < b_digit) {
                return -1;
            } else if (a_digit > b_digit) {
                return +1;
            }
        }

        return 0;
    }



public:

    BigInteger& powAssignUnderMod(const BigInteger& exponent, const BigInteger& modulus) {
        BigInteger zero("0");
        BigInteger e = exponent;
        BigInteger base = *this;
        *this = BigInteger("1");

        while (cmpWithoutSign(e, zero) != 0) {
            BigInteger odd = e % BigInteger(2);

            if (strcmp(odd.number.c_str(),"1") == 0) {
                *this *= base;
                *this %= modulus;
            }

            e /= BigInteger("2");
            base *= base;
            base %= modulus;
        }
        return *this;
    }



};


