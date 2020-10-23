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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Meng Tan, Jennifer Inthavong

   Unit tests for Acl Serializer
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/test_framework/file.hpp"

#include "core/log_id.hpp"
#include "configs/config.hpp"
#include "main/version.hpp"
#include "utils/genfstat.hpp"
#include "utils/sugar/algostring.hpp"
#include "transport/file_transport.hpp"
#include "transport/mwrm_reader.hpp"
#include "test_only/transport/test_transport.hpp"
#include "test_only/lcg_random.hpp"
#include "test_only/log_buffered.hpp"
#include "acl/session_logfile.hpp"

// Class ACL Serializer is used to Modify config file content from a remote ACL manager
// - Send given fields from config
// - Recover fields from network and update Config

inline void init_keys(CryptoContext & cctx)
{
    cctx.set_master_key(cstr_array_view(
        "\x61\x1f\xd4\xcd\xe5\x95\xb7\xfd"
        "\xa6\x50\x38\xfc\xd8\x86\x51\x4f"
        "\x59\x7e\x8e\x90\x81\xf6\xf4\x48"
        "\x9c\x77\x41\x51\x0f\x53\x0e\xe8"
    ));
    cctx.set_hmac_key(cstr_array_view(
         "\x86\x41\x05\x58\xc4\x95\xcc\x4e"
         "\x49\x21\x57\x87\x47\x74\x08\x8a"
         "\x33\xb0\x2a\xb8\x65\xcc\x38\x41"
         "\x20\xfe\xc2\xc9\xb8\x72\xc8\x2c"
    ));
}

RED_AUTO_TEST_CASE_WD(TestSessionLogfileLog, wd)
{
    Inifile ini;
    ini.clear_acl_fields_changed();

    LCGRandom rnd;
    Fstat fstat;
    CryptoContext cctx;
    TimeBase timebase({0, 0});

    ini.set_acl<cfg::globals::auth_user>("admin");
    ini.set_acl<cfg::globals::target_user>("user1");
    ini.set_acl<cfg::globals::host>("10.10.13.12");
    ini.set<cfg::session_log::enable_arcsight_log>(true);
    // ini.set_acl<cfg::globals::target_host>("13.12.10.10");
    // ini.set_acl<cfg::globals::ip_target>("13.12.10.10");
    // ini.set_acl<cfg::globals::session_id>("0x520");

    auto logfile = wd.add_file("log5_6.log");
    auto hashdir = wd.create_subdirectory("hash");
    auto hashlog = hashdir.add_file("log5_6.log");

    ini.set<cfg::capture::record_filebase>("log5_6");
    ini.set<cfg::video::record_path>(wd.dirname().string());
    ini.set<cfg::video::hash_path>(hashdir.dirname().string());

    GeneratorTransport trans(""_av);
    std::string session_type;

    auto notify_error = [](const Error & /*error*/) { RED_REQUIRE(false); };

    SessionLogFile log_file(ini, timebase, cctx, rnd, fstat, notify_error);

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);          // for localtime

    log_file.open_session_log();
    {
        tu::log_buffered logbuf;

        log_file.log6(LogId::INPUT_LANGUAGE, {
            KVLog("identifier"_av,   "ident"_av),
            KVLog("display_name"_av, "name"_av),
        });

        log_siem_syslog(LogId::INPUT_LANGUAGE, {
            KVLog("identifier"_av,   "ident"_av),
            KVLog("display_name"_av, "name"_av),
        }, ini, session_type);

        auto now = timebase.get_current_time();
        log_siem_arcsight(now.tv_sec,
            LogId::INPUT_LANGUAGE, {
            KVLog("identifier"_av,   "ident"_av),
            KVLog("display_name"_av, "name"_av),
        }, ini, session_type);

        auto expected6 = cstr_array_view("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"INPUT_LANGUAGE\" identifier=\"ident\" display_name=\"name\"\nJan 01 1970 00:00:00 host message CEF:1|Wallix|Bastion|" VERSION "|24|INPUT_LANGUAGE|5|WallixBastionSessionType=Neutral WallixBastionSessionId= WallixBastionHost=10.10.13.12 WallixBastionTargetIP= WallixBastionUser=admin WallixBastionDevice= WallixBastionService= WallixBastionAccount=user1 identifier=ident display_name=name\n");
        RED_CHECK(logbuf.buf() == expected6);
    }

    timebase.increment_sec(10);

    {
        tu::log_buffered logbuf;

        log_file.log6(LogId::CONNECTION_FAILED, {
            KVLog("msg"_av, "long long\nmessage=|x\\y\"z"_av),
            KVLog("msg2"_av, "vnc"_av),
        });

        log_siem_syslog(LogId::CONNECTION_FAILED, {
            KVLog("msg"_av, "long long\nmessage=|x\\y\"z"_av),
            KVLog("msg2"_av, "vnc"_av),
        }, ini, session_type);

        auto now = timebase.get_current_time();
        log_siem_arcsight(now.tv_sec,LogId::CONNECTION_FAILED, {
            KVLog("msg"_av, "long long\nmessage=|x\\y\"z"_av),
            KVLog("msg2"_av, "vnc"_av),
        }, ini, session_type);

        auto expected6 = cstr_array_view("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"CONNECTION_FAILED\" msg=\"long long\\nmessage=|x\\\\y\\\"z\" msg2=\"vnc\"\n"
                                         "Jan 01 1970 00:00:10 host message CEF:1|Wallix|Bastion|" VERSION "|11|CONNECTION_FAILED|5|WallixBastionSessionType=Neutral WallixBastionSessionId= WallixBastionHost=10.10.13.12 WallixBastionTargetIP= WallixBastionUser=admin WallixBastionDevice= WallixBastionService= WallixBastionAccount=user1 msg=long long\\nmessage\\=|x\\\\y\"z msg2=vnc\n");

        RED_CHECK(logbuf.buf() == expected6);
    }

    timebase.increment_sec(3023);

    {
        tu::log_buffered logbuf;

        log_siem_syslog(LogId::DRIVE_REDIRECTION_RENAME, {
            KVLog("app"_av, "rdp"_av),
            KVLog("oldFilePath"_av, "/dir/old_file.ext"_av),
            KVLog("filePath"_av, "/dir/new_file.ext"_av),
        }, ini, session_type);

        auto now = timebase.get_current_time();
        log_siem_arcsight(now.tv_sec,LogId::DRIVE_REDIRECTION_RENAME, {
            KVLog("app"_av, "rdp"_av),
            KVLog("oldFilePath"_av, "/dir/old_file.ext"_av),
            KVLog("filePath"_av, "/dir/new_file.ext"_av),
        }, ini, session_type);

        log_file.log6(LogId::DRIVE_REDIRECTION_RENAME, {
            KVLog("app"_av, "rdp"_av),
            KVLog("oldFilePath"_av, "/dir/old_file.ext"_av),
            KVLog("filePath"_av, "/dir/new_file.ext"_av),
        });

        auto expected6 = cstr_array_view("[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"DRIVE_REDIRECTION_RENAME\" app=\"rdp\" oldFilePath=\"/dir/old_file.ext\" filePath=\"/dir/new_file.ext\"\n"
                                         "Jan 01 1970 00:50:33 host message CEF:1|Wallix|Bastion|" VERSION "|15|DRIVE_REDIRECTION_RENAME|5|WallixBastionSessionType=Neutral WallixBastionSessionId= WallixBastionHost=10.10.13.12 WallixBastionTargetIP= WallixBastionUser=admin WallixBastionDevice= WallixBastionService= WallixBastionAccount=user1 app=rdp oldFilePath=/dir/old_file.ext filePath=/dir/new_file.ext\n");

        RED_CHECK(logbuf.buf() == expected6);
    }

    log_file.close_session_log();

    RED_CHECK_FILE_CONTENTS(logfile,
        "1970-01-01 01:00:00 type=\"INPUT_LANGUAGE\" identifier=\"ident\" display_name=\"name\"\n"
        "1970-01-01 01:00:10 type=\"CONNECTION_FAILED\" msg=\"long long\\nmessage=|x\\\\y\\\"z\" msg2=\"vnc\"\n"
        "1970-01-01 01:50:33 type=\"DRIVE_REDIRECTION_RENAME\" app=\"rdp\" oldFilePath=\"/dir/old_file.ext\" filePath=\"/dir/new_file.ext\"\n"_av);
}


RED_AUTO_TEST_CASE_WD(TestSessionLogFile, wd)
{
    std::string const logname = "acl_log.txt";
    auto filename = wd.add_file(logname);
    auto hashdir = wd.create_subdirectory("hash");
    auto hashname = hashdir.add_file(logname);

    LCGRandom rnd;
    Fstat fstat;
    CryptoContext cctx;
    init_keys(cctx);
    TimeBase timebase({0, 0});
    Inifile ini;
    SessionLogFile log_file(ini, timebase, cctx, rnd, fstat, [&](const Error & e){
        RED_FAIL(e.errmsg());
    });

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);          // for localtime

    log_file.open(filename, hashname, 0, logname);
    log_file.write_line(1512484183, cstr_array_view("test first"));
    log_file.write_line(1512484185, cstr_array_view("test second"));
    log_file.close_session_log();

    RED_CHECK_FILE_CONTENTS(filename,
        "2017-12-05 15:29:43 test first\n"
        "2017-12-05 15:29:45 test second\n"_av);

    InFileTransport t(unique_fd{open(hashname.c_str(), O_RDONLY)});
    MwrmReader mwrm_reader(t);
    MetaLine meta_line;
    mwrm_reader.set_header({WrmVersion::v2, false});
    mwrm_reader.read_meta_hash_line(meta_line);
    RED_CHECK_EQ(meta_line.filename, logname);
    RED_CHECK(not meta_line.with_hash);
    RED_CHECK_EQ(meta_line.size, 63);
}
