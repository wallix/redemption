#ifndef _DIFFTIMEVAL_HPP
#define _DIFFTIMEVAL_HPP

#include <sys/time.h>
#include <stdint.h>

inline uint64_t difftimeval(const struct timeval& endtime, const struct timeval& starttime)
{
    uint64_t sec = (endtime.tv_sec  - starttime.tv_sec ) * 1000000
                 + (endtime.tv_usec - starttime.tv_usec);
    return sec;
}

#endif
