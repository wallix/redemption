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

   log file including syslog
*/

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>

typedef struct _code {
	const char	*c_name;
	int	c_val;
} CODE;

// LOG_EMERG      system is unusable
// LOG_ALERT      action must be taken immediately
// LOG_CRIT       critical conditions
// LOG_ERR        error conditions
// LOG_WARNING    warning conditions
// LOG_NOTICE     normal, but significant, condition
// LOG_INFO       informational message
// LOG_DEBUG      debug-level message

static inline void LOG(int priority, const char *format, ...)
{
    static CODE prioritynames[] =
    {
        { "ALERT", LOG_ALERT },
        { "CRIT", LOG_CRIT },
        { "DEBUG", LOG_DEBUG },
        { "EMERG", LOG_EMERG },
        { "ERR", LOG_ERR },
        { "INFO", LOG_INFO },
        { "NOTICE", LOG_NOTICE },
        { "WARNING", LOG_WARNING },
        { NULL, -1 }
    };
    char message[8192];
    char formated_message[8192];
    va_list vl;
    va_start (vl, format);
    vsnprintf(message, 8191, format, vl);
    snprintf(formated_message, 8191, "%s (%d/%d) -- %s", prioritynames[priority].c_name, getpid(), getpid(), message);
    va_end(vl);
    syslog(priority, "%s", formated_message);
};

#endif
