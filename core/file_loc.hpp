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

   default file locations

*/

#if !defined(FILE_LOC_H)
#define FILE_LOC_H

#if !defined(SHARE_PATH)
#define SHARE_PATH "/usr/local/share/rdpproxy"
#endif

#if !defined(CFG_PATH)
#define CFG_PATH "/etc/rdpproxy"
#endif

#if !defined(PID_PATH)
#define PID_PATH "/var/run"
#endif

#if !defined(SBIN_PATH)
#define SBIN_PATH "/usr/local/sbin"
#endif

#define LOGIN_LOGO24 "ad24b.bmp"
#define CURSOR0 "cursor0.cur"
#define CURSOR1 "cursor1.cur"
#define FONT1 "sans-10.fv1"
#define REDEMPTION_LOGO24 "xrdp24b-redemption.bmp"
#define CAPTUREFONT "FreeSans.ttf"
#define LOCKFILE "rdpproxy.pid"

#define RSAKEYS_INI "rsakeys.ini"
#define RDPPROXY_INI "rdpproxy.ini"


#endif
