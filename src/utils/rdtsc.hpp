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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

*/


#pragma once

#include <chrono>

#if defined(__i386__)

static __inline__ std::chrono::microseconds rdtsc(void)
{
  unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return std::chrono::microseconds(x);
}
#elif defined(__x86_64__)

static __inline__ std::chrono::microseconds rdtsc()
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return std::chrono::microseconds(
      static_cast<unsigned long long>(lo)
    | (static_cast<unsigned long long>(hi) << 32)
  );
}

#elif defined(__powerpc__)

static __inline__ std::chrono::microseconds rdtsc(void)
{
  unsigned long long int result=0;
  unsigned long int upper, lower,tmp;
  __asm__ volatile(
                "0:                  \n"
                "\tmftbu   %0           \n"
                "\tmftb    %1           \n"
                "\tmftbu   %2           \n"
                "\tcmpw    %2,%0        \n"
                "\tbne     0b         \n"
                : "=r"(upper),"=r"(lower),"=r"(tmp)
                );
  result = upper;
  result = result<<32;
  result = result|lower;

  return std::chrono::microseconds(result);
}

#else

#error "No tick counter is available!"

#endif


/*  $RCSfile:  $   $Author: kazutomo $
 *  $Revision: 1.6 $  $Date: 2005/04/13 18:49:58 $
 */
