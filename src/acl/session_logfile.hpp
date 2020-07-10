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
  Author(s): Christophe Grosjean, Meng Tauth_rail_exec_an, Jennifer Inthavong

  Protocol layer for communication with ACL
  Updating context dictionnary from incoming acl traffic
*/


#pragma once

#include "core/back_event_t.hpp"
#include "core/report_message_api.hpp"
#include "transport/crypto_transport.hpp"
#include "transport/socket_transport.hpp"
#include "utils/verbose_flags.hpp"
#include "utils/timebase.hpp"
#include "configs/config.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/key_qvalue_pairs.hpp"
#include "utils/fileutils.hpp"

#include <string>
#include <chrono>

#include <ctime>

class SessionLogFile
{
    Inifile & ini;
    TimeBase & time_base;
    CryptoContext & cctx;
    OutCryptoTransport ct;
    bool logfile_is_open = false;

public:
    SessionLogFile(Inifile&ini, TimeBase & time_base, CryptoContext & cctx, Random & rnd, Fstat & fstat, ReportError report_error)
    : ini(ini), time_base(time_base), cctx(cctx), ct(cctx, rnd, fstat, std::move(report_error))
    {}

    ~SessionLogFile()
    {
        try {
            this->close_session_log();
        }
        catch (Error const& e) {
            LOG(LOG_ERR, "~SessionLogFile: %s", e.errmsg());
        }
    }

    bool is_open()
    {
        return this->logfile_is_open;
    }

    void open(std::string const& log_path, std::string const& hash_path, int groupid, bytes_view derivator)
    {
        assert(!this->ct.is_open());
        this->ct.open(log_path.c_str(), hash_path.c_str(), groupid, -1, derivator);
        // force to create the file
        this->ct.send("", 0);
        this->logfile_is_open = true;
    }

    void log6(LogId id, KVList kv_list)
    {
        const timeval time = this->time_base.get_current_time();
        std::string buffer_info;
        buffer_info.reserve(kv_list.size() * 50 + 30);

        time_t const time_now = time.tv_sec;
        this->write_line(time_now, log_format_set_info(buffer_info, id, kv_list));
    }

    void write_line(std::time_t time, chars_view av)
    {
        assert(this->ct.is_open());

        char mbstr[100];
        auto const len = std::strftime(mbstr, sizeof(mbstr), "%F %T ", std::localtime(&time));
        if (len) {
            this->ct.send(mbstr, len);
        }

        this->ct.send(av.data(), av.size());
        this->ct.send("\n", 1);
    }

    void start_session_log()
    {
        this->cctx.set_master_key(this->ini.get<cfg::crypto::key0>());
        this->cctx.set_hmac_key(this->ini.get<cfg::crypto::key1>());
        this->cctx.set_trace_type(this->ini.get<cfg::globals::trace_type>());

        const int groupid = this->ini.get<cfg::video::capture_groupid>();
        auto const& subdir = this->ini.get<cfg::capture::record_subdirectory>();
        auto const& record_dir = this->ini.get<cfg::video::record_path>();
        auto const& hash_dir = this->ini.get<cfg::video::hash_path>();
        auto const& filebase = this->ini.get<cfg::capture::record_filebase>();

        std::string record_path = str_concat(record_dir.as_string(), subdir, '/');
        std::string hash_path = str_concat(hash_dir.as_string(), subdir, '/');

        for (auto* s : {&record_path, &hash_path}) {
            if (recursive_create_directory(s->c_str(), S_IRWXU | S_IRGRP | S_IXGRP, groupid) != 0) {
                LOG(LOG_ERR,
                    "AclSerializer::start_session_log: Failed to create directory: \"%s\"", *s);
            }
        }

        std::string basename = str_concat(filebase, ".log");
        record_path += basename;
        hash_path += basename;
        this->open(record_path, hash_path, groupid, /*derivator=*/basename);
    }

    void close_session_log()
    {
        if (this->ct.is_open()) {
            uint8_t qhash[MD_HASH::DIGEST_LENGTH];
            uint8_t fhash[MD_HASH::DIGEST_LENGTH];
            this->ct.close(qhash, fhash);
            this->logfile_is_open = false;
        }
    }


};

