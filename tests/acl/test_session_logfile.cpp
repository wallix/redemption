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

#include "acl/session_logfile.hpp"
#include "core/log_id.hpp"
#include "configs/config.hpp"
#include "main/version.hpp"
#include "test_only/lcg_random.hpp"
#include "test_only/log_buffered.hpp"


// Class ACL Serializer is used to Modify config file content from a remote ACL manager
// - Send given fields from config
// - Recover fields from network and update Config

inline void init_keys(CryptoContext & cctx)
{
    cctx.set_master_key(
        "\x61\x1f\xd4\xcd\xe5\x95\xb7\xfd"
        "\xa6\x50\x38\xfc\xd8\x86\x51\x4f"
        "\x59\x7e\x8e\x90\x81\xf6\xf4\x48"
        "\x9c\x77\x41\x51\x0f\x53\x0e\xe8"
        ""_av);
    cctx.set_hmac_key(
        "\x86\x41\x05\x58\xc4\x95\xcc\x4e"
        "\x49\x21\x57\x87\x47\x74\x08\x8a"
        "\x33\xb0\x2a\xb8\x65\xcc\x38\x41"
        "\x20\xfe\xc2\xc9\xb8\x72\xc8\x2c"
        ""_av);
}

RED_AUTO_TEST_CASE_WD(TestSessionLogFileAndSiemLogger, wd)
{
    Inifile ini;
    ini.clear_acl_fields_changed();

    LCGRandom rnd;
    CryptoContext cctx;

    ini.set_acl<cfg::globals::auth_user>("admin");
    ini.set_acl<cfg::globals::target_user>("user1");
    ini.set_acl<cfg::globals::host>("10.10.13.12");

    auto logfile = wd.add_file("log5_6.log");
    auto hashlog = wd.add_file("hash_log5_6.log");

    auto session_type = "RDP"_av;

    auto notify_error = [](const Error & /*error*/) { RED_REQUIRE(false); };

    SessionLogFile log_file(
        cctx, rnd,
        SessionLogFile::Debug(false),
        notify_error);

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);          // for localtime

    log_file.open_session_log(
        SessionLogFormat::SIEM | SessionLogFormat::ArcSight,
        SessionLogFile::SaveToFile(true),
        logfile.c_str(), hashlog.c_str(),
        FilePermissions(0664), "log5_6.log"_av);

    {
        ut::log_buffered logbuf;
        const time_t now = 0;

        log_file.log(now, ini, session_type, LogId::INPUT_LANGUAGE, {
            KVLog("identifier"_av,   "ident"_av),
            KVLog("display_name"_av, "name"_av),
        });

        auto expected =
            "[RDP Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"INPUT_LANGUAGE\" identifier=\"ident\" display_name=\"name\"\n"
            "Jan 01 1970 00:00:00 host message CEF:1|Wallix|Bastion|" VERSION "|24|INPUT_LANGUAGE|5|WallixBastionSessionType=RDP WallixBastionSessionId= WallixBastionHost=10.10.13.12 WallixBastionTargetIP= WallixBastionUser=admin WallixBastionDevice= WallixBastionService= WallixBastionAccount=user1 identifier=ident display_name=name\n"
            ""_av;
        RED_CHECK(logbuf.buf() == expected);
    }

    session_type = ""_av;

    {
        ut::log_buffered logbuf;
        const time_t now = 10;

        log_file.log(now, ini, session_type, LogId::CONNECTION_FAILED, {
            KVLog("msg"_av, "long long\nmessage=|x\\y\"z"_av),
            KVLog("msg2"_av, "vnc"_av),
        });

        auto expected =
            "[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"CONNECTION_FAILED\" msg=\"long long\\nmessage=|x\\\\y\\\"z\" msg2=\"vnc\"\n"
            "Jan 01 1970 00:00:10 host message CEF:1|Wallix|Bastion|" VERSION "|11|CONNECTION_FAILED|5|WallixBastionSessionType=Neutral WallixBastionSessionId= WallixBastionHost=10.10.13.12 WallixBastionTargetIP= WallixBastionUser=admin WallixBastionDevice= WallixBastionService= WallixBastionAccount=user1 msg=long long\\nmessage\\=|x\\\\y\"z msg2=vnc\n"
            ""_av;
        RED_CHECK(logbuf.buf() == expected);
    }

    {
        ut::log_buffered logbuf;
        const time_t now = 3033;

        log_file.log(now, ini, session_type, LogId::DRIVE_REDIRECTION_RENAME, {
            KVLog("app"_av, "rdp"_av),
            KVLog("oldFilePath"_av, "/dir/old_file.ext"_av),
            KVLog("filePath"_av, "/dir/new_file.ext"_av),
        });

        auto expected =
            "[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"DRIVE_REDIRECTION_RENAME\" app=\"rdp\" oldFilePath=\"/dir/old_file.ext\" filePath=\"/dir/new_file.ext\"\n"
            "Jan 01 1970 00:50:33 host message CEF:1|Wallix|Bastion|" VERSION "|15|DRIVE_REDIRECTION_RENAME|5|WallixBastionSessionType=Neutral WallixBastionSessionId= WallixBastionHost=10.10.13.12 WallixBastionTargetIP= WallixBastionUser=admin WallixBastionDevice= WallixBastionService= WallixBastionAccount=user1 app=rdp oldFilePath=/dir/old_file.ext filePath=/dir/new_file.ext\n"
            ""_av;
        RED_CHECK(logbuf.buf() == expected);
    }

    {
        ut::log_buffered logbuf;
        const time_t now = 3033;

        log_file.set_control_owner_ctx("guest-1"_av);
        log_file.log(now, ini, session_type, LogId::BUTTON_CLICKED, {
            KVLog("label"_av, "bbb"_av),
        });

        log_file.set_control_owner_ctx({});
        log_file.log(now, ini, session_type, LogId::BUTTON_CLICKED, {
            KVLog("label"_av, "bbb"_av),
        });

        auto expected =
            "[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"BUTTON_CLICKED\" label=\"bbb\" control_owner=\"guest-1\"\n"
            "Jan 01 1970 00:50:33 host message CEF:1|Wallix|Bastion|" VERSION "|0|BUTTON_CLICKED|5|WallixBastionSessionType=Neutral WallixBastionSessionId= WallixBastionHost=10.10.13.12 WallixBastionTargetIP= WallixBastionUser=admin WallixBastionDevice= WallixBastionService= WallixBastionAccount=user1 label=bbb\n"

            "[Neutral Session] session_id=\"\" client_ip=\"10.10.13.12\" target_ip=\"\" user=\"admin\" device=\"\" service=\"\" account=\"user1\" type=\"BUTTON_CLICKED\" label=\"bbb\"\n"
            "Jan 01 1970 00:50:33 host message CEF:1|Wallix|Bastion|" VERSION "|0|BUTTON_CLICKED|5|WallixBastionSessionType=Neutral WallixBastionSessionId= WallixBastionHost=10.10.13.12 WallixBastionTargetIP= WallixBastionUser=admin WallixBastionDevice= WallixBastionService= WallixBastionAccount=user1 label=bbb\n"
            ""_av;
        RED_CHECK(logbuf.buf() == expected);
    }

    log_file.close_session_log();

    RED_CHECK_FILE_CONTENTS(logfile,
        "1970-01-01 01:00:00 type=\"INPUT_LANGUAGE\" identifier=\"ident\" display_name=\"name\"\n"
        "1970-01-01 01:00:10 type=\"CONNECTION_FAILED\" msg=\"long long\\nmessage=|x\\\\y\\\"z\" msg2=\"vnc\"\n"
        "1970-01-01 01:50:33 type=\"DRIVE_REDIRECTION_RENAME\" app=\"rdp\" oldFilePath=\"/dir/old_file.ext\" filePath=\"/dir/new_file.ext\"\n"
        "1970-01-01 01:50:33 type=\"BUTTON_CLICKED\" label=\"bbb\" control_owner=\"guest-1\"\n"
        "1970-01-01 01:50:33 type=\"BUTTON_CLICKED\" label=\"bbb\"\n"
        ""_av);
}

RED_AUTO_TEST_CASE_WD(TestSessionLogFileWithDebugOnly, wd)
{
    Inifile ini;
    ini.clear_acl_fields_changed();

    LCGRandom rnd;
    CryptoContext cctx;

    ini.set_acl<cfg::globals::auth_user>("admin");
    ini.set_acl<cfg::globals::target_user>("user1");
    ini.set_acl<cfg::globals::host>("10.10.13.12");

    auto logfile = wd.add_file("log5_6.log");
    auto hashlog = wd.add_file("hash_log5_6.log");

    auto session_type = "RDP"_av;

    auto notify_error = [](const Error & /*error*/) { RED_REQUIRE(false); };

    SessionLogFile log_file(
        cctx, rnd,
        SessionLogFile::Debug(true),
        notify_error);

    log_file.open_session_log(
        SessionLogFormat::disabled,
        SessionLogFile::SaveToFile(false),
        logfile.c_str(), hashlog.c_str(),
        FilePermissions(0664), "log5_6.log"_av);

    {
        ut::log_buffered logbuf;
        const time_t now = 0;

        log_file.log(now, ini, session_type, LogId::INPUT_LANGUAGE, {
            KVLog("identifier"_av,   "ident"_av),
            KVLog("display_name"_av, "name"_av),
        });

        RED_CHECK(logbuf.buf() == "INFO -- <RDP> type=\"INPUT_LANGUAGE\" identifier=\"ident\" display_name=\"name\"\n"_av);
    }

    log_file.close_session_log();

    wd.remove_file(logfile);
    wd.remove_file(hashlog);
}
