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
  Copyright (C) Wallix 2010
  Author(s): Wallix Team

  Protocol layer for communication with ACL
  Updating context dictionnary from incoming acl traffic
*/


#pragma once

#include "transport/crypto_transport.hpp"
#include "acl/auth_api.hpp"

#include <string>
#include <ctime>


class Random;
class Fstat;
class Inifile;


class SessionLogFile
{
public:
    SessionLogFile(
        const Inifile & ini, CryptoContext & cctx, Random & rnd, Fstat & fstat,
        std::function<void(const Error & error)> notify_error);

    ~SessionLogFile();

    void log6(std::time_t time_now, LogId id, KVLogList kv_list);

    void open_session_log();

    void close_session_log();

private:
    const Inifile & ini;
    CryptoContext & cctx;
    OutCryptoTransport ct;
    std::string log6_buffer;
};


struct SiemLogger
{
    explicit SiemLogger();

    void log_syslog_format(
        LogId id, KVLogList kv_list, const Inifile & ini, chars_view session_type);

    void log_arcsight_format(
        std::time_t time_now,
        LogId id, KVLogList kv_list, const Inifile & ini, chars_view session_type);

private:
    std::string buffer_;
};
