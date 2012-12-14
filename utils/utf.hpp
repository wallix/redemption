#include <stdint.h>
#include "log.hpp"

//#define U_SENTINEL (-1)
//static uint32_t utf8_minLegal[4]={ 0, 0x80, 0x800, 0x10000 };
//#define U_IS_SURROGATE(c) (((c)&0xfffff800)==0xd800)

//#define UTF8_ERROR_VALUE_1 0x15
//#define UTF8_ERROR_VALUE_2 0x9f
//#define UTF_ERROR_VALUE 0xffff

//static const uint32_t
//utf8_errorValue[6]={
//    UTF8_ERROR_VALUE_1, UTF8_ERROR_VALUE_2, UTF_ERROR_VALUE, 0x10ffff,
//    0x3ffffff, 0x7fffffff
//};


///**
// * Is this code point a Unicode noncharacter?
// * @param c 32-bit code point
// * @return TRUE or FALSE
// * @stable ICU 2.4
// */
//#define U_IS_UNICODE_NONCHAR(c) \
//    ((c)>=0xfdd0 && \
//     ((uint32_t)(c)<=0xfdef || ((c)&0xfffe)==0xfffe) && \
//     (uint32_t)(c)<=0x10ffff)

///**
// * Counts the trail bytes for a UTF-8 lead byte.
// * Returns 0 for 0..0xbf as well as for 0xfe and 0xff.
// *
// * This is internal since it is not meant to be called directly by external clients;
// * however it is called by public macros in this file and thus must remain stable.
// *
// * Note: Beginning with ICU 50, the implementation uses a multi-condition expression
// * which was shown in 2012 (on x86-64) to compile to fast, branch-free code.
// * leadByte is evaluated multiple times.
// *
// * The pre-ICU 50 implementation used the exported array utf8_countTrailBytes:
// * #define U8_COUNT_TRAIL_BYTES(leadByte) (utf8_countTrailBytes[leadByte])
// * leadByte was evaluated exactly once.
// *
// * @param leadByte The first byte of a UTF-8 sequence. Must be 0..0xff.
// * @internal
// */
//#define U8_COUNT_TRAIL_BYTES(leadByte) \
//    ((leadByte)<0xf0 ? \
//        ((leadByte)>=0xc0)+((leadByte)>=0xe0) : \
//        (leadByte)<0xfe ? 3+((leadByte)>=0xf8)+((leadByte)>=0xfc) : 0)

///**
// * Counts the trail bytes for a UTF-8 lead byte of a valid UTF-8 sequence.
// * The maximum supported lead byte is 0xf4 corresponding to U+10FFFF.
// * leadByte might be evaluated multiple times.
// *
// * This is internal since it is not meant to be called directly by external clients;
// * however it is called by public macros in this file and thus must remain stable.
// *
// * @param leadByte The first byte of a UTF-8 sequence. Must be 0..0xff.
// * @internal
// */
//#define U8_COUNT_TRAIL_BYTES_UNSAFE(leadByte) \
//    (((leadByte)>=0xc0)+((leadByte)>=0xe0)+((leadByte)>=0xf0))

///**
// * Mask a UTF-8 lead byte, leave only the lower bits that form part of the code point value.
// *
// * This is internal since it is not meant to be called directly by external clients;
// * however it is called by public macros in this file and thus must remain stable.
// * @internal
// */
//#define U8_MASK_LEAD_BYTE(leadByte, countTrailBytes) ((leadByte)&=(1<<(6-(countTrailBytes)))-1)

///**
// * Does this code unit (byte) encode a code point by itself (US-ASCII 0..0x7f)?
// * @param c 8-bit code unit (byte)
// * @return TRUE or FALSE
// * @stable ICU 2.4
// */
//#define U8_IS_SINGLE(c) (((c)&0x80)==0)

///**
// * Is this code unit (byte) a UTF-8 lead byte?
// * @param c 8-bit code unit (byte)
// * @return TRUE or FALSE
// * @stable ICU 2.4
// */
//#define U8_IS_LEAD(c) ((uint8_t)((c)-0xc0)<0x3e)

///**
// * Is this code unit (byte) a UTF-8 trail byte?
// * @param c 8-bit code unit (byte)
// * @return TRUE or FALSE
// * @stable ICU 2.4
// */
//#define U8_IS_TRAIL(c) (((c)&0xc0)==0x80)

///**
// * How many code units (bytes) are used for the UTF-8 encoding
// * of this Unicode code point?
// * @param c 32-bit code point
// * @return 1..4, or 0 if c is a surrogate or not a Unicode code point
// * @stable ICU 2.4
// */
//#define U8_LENGTH(c) \
//    ((uint32_t)(c)<=0x7f ? 1 : \
//        ((uint32_t)(c)<=0x7ff ? 2 : \
//            ((uint32_t)(c)<=0xd7ff ? 3 : \
//                ((uint32_t)(c)<=0xdfff || (uint32_t)(c)>0x10ffff ? 0 : \
//                    ((uint32_t)(c)<=0xffff ? 3 : 4)\
//                ) \
//            ) \
//        ) \
//    )

///**
// * The maximum number of UTF-8 code units (bytes) per Unicode code point (U+0000..U+10ffff).
// * @return 4
// * @stable ICU 2.4
// */
//#define U8_MAX_LENGTH 4

///*
// * Handle the non-inline part of the U8_NEXT() macro and its obsolete sibling
// * UTF8_NEXT_CHAR_SAFE().
// *
// * The "strict" parameter controls the error behavior:
// * <0  "Safe" behavior of U8_NEXT(): All illegal byte sequences yield a negative
// *     code point result.
// *  0  Obsolete "safe" behavior of UTF8_NEXT_CHAR_SAFE(..., FALSE):
// *     All illegal byte sequences yield a positive code point such that this
// *     result code point would be encoded with the same number of bytes as
// *     the illegal sequence.
// * >0  Obsolete "strict" behavior of UTF8_NEXT_CHAR_SAFE(..., TRUE):
// *     Same as the obsolete "safe" behavior, but non-characters are also treated
// *     like illegal sequences.
// *
// * The special negative (<0) value -2 is used for lenient treatment of surrogate
// * code points as legal. Some implementations use this for roundtripping of
// * Unicode 16-bit strings that are not well-formed UTF-16, that is, they
// * contain unpaired surrogates.
// *
// * Note that a bool is the same as an int8_t.
// */

//uint32_t utf8_nextCharSafeBody(const uint8_t *s, int32_t *pi, int32_t length, uint32_t c, bool strict) {
//    int32_t i=*pi;
//    uint8_t count=U8_COUNT_TRAIL_BYTES(c);
//    printf("count = %u\n", count);
//    //U_ASSERT(count <= 5); /* U8_COUNT_TRAIL_BYTES returns value 0...5 */
//    if((i)+count<=(length)) {
//        uint8_t trail, illegal=0;

//        U8_MASK_LEAD_BYTE((c), count);
//        /* count==0 for illegally leading trail bytes and the illegal bytes 0xfe and 0xff */
//        switch(count) {
//        /* each branch falls through to the next one */
//        case 5:
//        case 4:
//            /* count>=4 is always illegal: no more than 3 trail bytes in Unicode's UTF-8 */
//            illegal=1;
//            break;
//        case 3:
//            trail=s[(i)++];
//            (c)=((c)<<6)|(trail&0x3f);
//            if(c<0x110) {
//                illegal|=(trail&0xc0)^0x80;
//            } else {
//                /* code point>0x10ffff, outside Unicode */
//                illegal=1;
//                break;
//            }
//        case 2:
//            trail=s[(i)++];
//            (c)=((c)<<6)|(trail&0x3f);
//            illegal|=(trail&0xc0)^0x80;
//        case 1:
//            trail=s[(i)++];
//            (c)=((c)<<6)|(trail&0x3f);
//            illegal|=(trail&0xc0)^0x80;
//            break;
//        case 0:
//            if(strict>=0) {
//                return UTF8_ERROR_VALUE_1;
//            } else {
//                return U_SENTINEL;
//            }
//        /* no default branch to optimize switch()  - all values are covered */
//        }

//        /*
//         * All the error handling should return a value
//         * that needs count bytes so that UTF8_GET_CHAR_SAFE() works right.
//         *
//         * Starting with Unicode 3.0.1, non-shortest forms are illegal.
//         * Starting with Unicode 3.2, surrogate code points must not be
//         * encoded in UTF-8, and there are no irregular sequences any more.
//         *
//         * U8_ macros (new in ICU 2.4) return negative values for error conditions.
//         */

//        /* correct sequence - all trail bytes have (b7..b6)==(10)? */
//        /* illegal is also set if count>=4 */
//        if(illegal || (c)<utf8_minLegal[count] || (U_IS_SURROGATE(c) && strict!=-2)) {
//            /* error handling */
//            uint8_t errorCount=count;
//            /* don't go beyond this sequence */
//            i=*pi;
//            while(count>0 && U8_IS_TRAIL(s[i])) {
//                ++(i);
//                --count;
//            }
//            if(strict>=0) {
//                c=utf8_errorValue[errorCount-count];
//            } else {
//                c=U_SENTINEL;
//            }
//        } else if((strict)>0 && U_IS_UNICODE_NONCHAR(c)) {
//            /* strict: forbid non-characters like U+fffe */
//            c=utf8_errorValue[count];
//        }
//    } else /* too few bytes left */ {
//        /* error handling */
//        int32_t i0=i;
//        /* don't just set (i)=(length) in case there is an illegal sequence */
//        while((i)<(length) && U8_IS_TRAIL(s[i])) {
//            ++(i);
//        }
//        if(strict>=0) {
//            c=utf8_errorValue[i-i0];
//        } else {
//            c=U_SENTINEL;
//        }
//        printf("error handling\n");
//    }
//    *pi=i;
//    return c;
//}


///**
// * Get a code point from a string at a code point boundary offset,
// * and advance the offset to the next code point boundary.
// * (Post-incrementing forward iteration.)
// * "Unsafe" macro, assumes well-formed UTF-8.
// *
// * The offset may point to the lead byte of a multi-byte sequence,
// * in which case the macro will read the whole sequence.
// * The result is undefined if the offset points to a trail byte
// * or an illegal UTF-8 sequence.
// *
// * @param s const uint8_t * string
// * @param i string offset
// * @param c output uint32_t variable
// * @see U8_NEXT
// * @stable ICU 2.4
// */
//#define U8_NEXT_UNSAFE(s, i, c) { \
//    (c)=(uint8_t)(s)[(i)++]; \
//    if((c)>=0x80) { \
//        if((c)<0xe0) { \
//            (c)=(((c)&0x1f)<<6)|((s)[(i)++]&0x3f); \
//        } else if((c)<0xf0) { \
//            /* no need for (c&0xf) because the upper bits are truncated after <<12 in the cast to (uint8_t) */ \
//            (c)=(uint8_t)(((c)<<12)|(((s)[i]&0x3f)<<6)|((s)[(i)+1]&0x3f)); \
//            (i)+=2; \
//        } else { \
//            (c)=(((c)&7)<<18)|(((s)[i]&0x3f)<<12)|(((s)[(i)+1]&0x3f)<<6)|((s)[(i)+2]&0x3f); \
//            (i)+=3; \
//        } \
//    } \
//}

///**
// * Get a code point from a string at a code point boundary offset,
// * and advance the offset to the next code point boundary.
// * (Post-incrementing forward iteration.)
// * "Safe" macro, checks for illegal sequences and for string boundaries.
// *
// * The offset may point to the lead byte of a multi-byte sequence,
// * in which case the macro will read the whole sequence.
// * If the offset points to a trail byte or an illegal UTF-8 sequence, then
// * c is set to a negative value.
// *
// * @param s const uint8_t * string
// * @param i string offset, must be i<length
// * @param length string length
// * @param c output uint32_t variable, set to <0 in case of an error
// * @see U8_NEXT_UNSAFE
// * @stable ICU 2.4
// */
//#define U8_NEXT(s, i, length, c) { \
//    (c)=(uint8_t)(s)[(i)++]; \
//    if((c)>=0x80) { \
//        uint8_t __t1, __t2; \
//        if( /* handle U+1000..U+CFFF inline */ \
//            (0xe0<(c) && (c)<=0xec) && \
//            (((i)+1)<(length)) && \
//            (__t1=(uint8_t)((s)[i]-0x80))<=0x3f && \
//            (__t2=(uint8_t)((s)[(i)+1]-0x80))<= 0x3f \
//        ) { \
//            /* no need for (c&0xf) because the upper bits are truncated after <<12 in the cast to (uint8_t) */ \
//            (c)=(uint8_t)(((c)<<12)|(__t1<<6)|__t2); \
//            (i)+=2; \
//        } else if( /* handle U+0080..U+07FF inline */ \
//            ((c)<0xe0 && (c)>=0xc2) && \
//            ((i)<(length)) && \
//            (__t1=(uint8_t)((s)[i]-0x80))<=0x3f \
//        ) { \
//            (c)=(uint8_t)((((c)&0x1f)<<6)|__t1); \
//            ++(i); \
//        } else if(U8_IS_LEAD(c)) { \
//            /* function call for "complicated" and error cases */ \
//            (c)=utf8_nextCharSafeBody((const uint8_t *)s, &(i), (int32_t)(length), c, -1); \
//        } else { \
//            (c)=U_SENTINEL; \
//        } \
//    } \
//}


// convert some utf8 string to utf16, 
// if some invalide codepoint is encountered stop encoding (this way, the simplest behavior is truncating string)
// after returning this function leaves :
// - *s pointing to the first byte of invalid sequence, or immediately after
// - *t pointing after the last valid utf16 sequence
// it is left to the responsibility of the caller to do whatever is needed is such a sequence occurs
// either putting an end to s and t string and stop encoding, 
// either calling some other function to change or skip somehow the invalid sequence
// and continue proceeding after it.
// it is also to the responsibility of the caller to put a terminating zero or double zero
// at the end of the input and output buffer is needed.
// The function will also return if target buffer is full.
// No error code is returned, it is the responsibility of the caller to check returned pointers values
// and decide by themselve if there is some error or not.

void UTF8toUTF16(uint8_t ** s, size_t s_len, uint16_t ** t, size_t t_len)
{
    uint8_t * source = *s;
    uint16_t * target = *t;
    // naive first implementation, not check for length, not check for error invalid sequences
    size_t i = 0;
    while (i < s_len){
        unsigned c = source[i++];
        switch (c & 0xF0){
            case 0x00:
            break;
            /* handle U+0080..U+07FF inline : 2 bytes sequences */ 
            case 0x80: case 0x90: case 0xA0: case 0xB0:
                c = ((c & 0x3f) << 6) | source[i];
                i+=1;
            break;
             /* handle U+8FFF..U+FFFF inline : 3 bytes sequences */ 
            case 0xD0: case 0xC0:
                c = (c << 12) | ((source[i] & 0x3F) << 6) | (source[i+1] & 0x3F);
            break;
            case 0xE0: case 0xF0:
                LOG(LOG_ERR, "complicated cases not yet supported");
                goto UTF8toUTF16_exit;
            break;
        }
        *target = c;
        target++;
    }
UTF8toUTF16_exit:
    *s = source + i;
    *t = target;
}
