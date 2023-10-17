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

#include "configs/autogen/enums.hpp"
#include "transport/crypto_transport.hpp"
#include "utils/uninit_buffer.hpp"
#include "acl/auth_api.hpp"

#include <ctime>


class Random;
class Inifile;

class SessionLogFile
{
public:
    enum class SaveToFile : bool { Disable, Enable, };
    enum class Debug : bool { Disable, Enable, };

    SessionLogFile(
        CryptoContext & cctx, Random & rnd, Debug enable_debug,
        std::function<void(const Error & error)> notify_error);

    ~SessionLogFile();

    void set_control_owner_ctx(chars_view name);

    void log(std::time_t time_now, Inifile& ini,
             chars_view session_type, LogId id, KVLogList kv_list);

    void open_session_log(
        SessionLogFormat syslog_format, SaveToFile save_to_file,
        const char * const record_path, const char * const hash_path,
        FilePermissions file_permissions, bytes_view derivator);

    void close_session_log();

    static std::string const& get_target_ip(Inifile const& ini);
    static std::string const& get_account(Inifile const& ini);

private:
    UninitDynamicBuffer buffer;
    UninitDynamicBuffer control_owner_extra_log;
    std::size_t control_owner_extra_log_len = 0;
    bool enable_file = false;
    bool enable_siem = false;
    bool enable_arcsight = false;
    const bool enable_debug;
    OutCryptoTransport ct;
};
