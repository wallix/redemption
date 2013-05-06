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
   Copyright (C) Wallix 2012-2013
   Author(s): Christophe Grosjean, Raphael Zhou

   Unit test to config.cpp file
   Using lib boost functions, some tests need to be added
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestConfig
#include <boost/test/auto_unit_test.hpp>
#include <stdio.h>

#include "log.hpp"
#define LOGNULL

#include "config.hpp"
#include <sstream>
#include <string>

BOOST_AUTO_TEST_CASE(TestConfigFromFile)
{
    // test we can read from a file (and not only from a stream)
    Inifile ini(FIXTURES_PATH "/rdpproxy.ini");

    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.movie_path));
    BOOST_CHECK_EQUAL(std::string("flv"),               std::string(ini.globals.codec_id));
    BOOST_CHECK_EQUAL(std::string("medium"),            std::string(ini.globals.video_quality));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.auth_user));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.host));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_device));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_user));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.globals.auth_channel, "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(3389,                             ini.globals.port);
    BOOST_CHECK_EQUAL(0,                                ini.globals.encryptionLevel);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.globals.authip));
    BOOST_CHECK_EQUAL(3350,                             ini.globals.authport);
    BOOST_CHECK_EQUAL(false,                            ini.globals.nomouse);
    BOOST_CHECK_EQUAL(false,                            ini.globals.notimestamp);
    BOOST_CHECK_EQUAL(false,                            ini.globals.autovalidate);
    BOOST_CHECK_EQUAL(std::string("/tmp/rdpproxy/"),    std::string(ini.globals.dynamic_conf_path));

    BOOST_CHECK_EQUAL(1,                                ini.globals.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.globals.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.globals.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.globals.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.globals.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.globals.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.globals.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.globals.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.globals.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.globals.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.globals.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.globals.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.globals.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.globals.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.globals.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.globals.replay_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("0.0.0.0"),           std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("inquisition"),       std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string(PNG_PATH),            std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string(WRM_PATH),            std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.input);
    // log file
    BOOST_CHECK_EQUAL(2,                                ini.globals.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.globals.client.tls_fallback_legacy);
}

BOOST_AUTO_TEST_CASE(TestConfigDefaultEmpty)
{
    // default config
    Inifile ini;

    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.movie_path));
    BOOST_CHECK_EQUAL(std::string("flv"),               std::string(ini.globals.codec_id));
    BOOST_CHECK_EQUAL(std::string("medium"),            std::string(ini.globals.video_quality));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.auth_user));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.host));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_device));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_user));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.globals.auth_channel, "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(3389,                             ini.globals.port);
    BOOST_CHECK_EQUAL(0,                                ini.globals.encryptionLevel);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.globals.authip));
    BOOST_CHECK_EQUAL(3350,                             ini.globals.authport);
    BOOST_CHECK_EQUAL(false,                            ini.globals.nomouse);
    BOOST_CHECK_EQUAL(false,                            ini.globals.notimestamp);
    BOOST_CHECK_EQUAL(false,                            ini.globals.autovalidate);
    BOOST_CHECK_EQUAL(std::string("/tmp/rdpproxy/"),    std::string(ini.globals.dynamic_conf_path));

    BOOST_CHECK_EQUAL(1,                                ini.globals.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.globals.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.globals.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.globals.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.globals.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.globals.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.globals.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.globals.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.globals.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.globals.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.globals.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.globals.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.globals.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.globals.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.globals.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.globals.replay_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("0.0.0.0"),           std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("inquisition"),       std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string(PNG_PATH),            std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string(WRM_PATH),            std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.input);
    // log file
    BOOST_CHECK_EQUAL(2,                                ini.globals.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.globals.client.tls_fallback_legacy);
}

BOOST_AUTO_TEST_CASE(TestConfigDefault)
{
    // test we can read a config file with a global section
    std::stringstream oss("");

    Inifile ini(oss);

    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.movie_path));
    BOOST_CHECK_EQUAL(std::string("flv"),               std::string(ini.globals.codec_id));
    BOOST_CHECK_EQUAL(std::string("medium"),            std::string(ini.globals.video_quality));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.auth_user));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.host));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_device));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_user));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.globals.auth_channel, "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(3389,                             ini.globals.port);
    BOOST_CHECK_EQUAL(0,                                ini.globals.encryptionLevel);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.globals.authip));
    BOOST_CHECK_EQUAL(3350,                             ini.globals.authport);
    BOOST_CHECK_EQUAL(false,                            ini.globals.nomouse);
    BOOST_CHECK_EQUAL(false,                            ini.globals.notimestamp);
    BOOST_CHECK_EQUAL(false,                            ini.globals.autovalidate);
    BOOST_CHECK_EQUAL(std::string("/tmp/rdpproxy/"),    std::string(ini.globals.dynamic_conf_path));

    BOOST_CHECK_EQUAL(1,                                ini.globals.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.globals.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.globals.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.globals.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.globals.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.globals.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.globals.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.globals.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.globals.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.globals.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.globals.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.globals.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.globals.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.globals.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.globals.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.globals.replay_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("0.0.0.0"),           std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("inquisition"),       std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string(PNG_PATH),            std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string(WRM_PATH),            std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.input);
    // log
    BOOST_CHECK_EQUAL(2,                                ini.globals.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.globals.client.tls_fallback_legacy);
}

BOOST_AUTO_TEST_CASE(TestConfig1)
{
    // test we can read a config file with a global section
    std::stringstream oss(
    "[globals]\n"
    "bitmap_cache=yes\n"
    "bitmap_compression=true\n"
    "port=3390\n"
    "encryptionLevel=low\n"
    "enable_file_encryption=yes\n"
    "enable_tls=no\n"
    "listen_address=192.168.1.1\n"
    "enable_ip_transparent=yes\n"
    "certificate_password=redemption\n"
    "png_path=/var/tmp/wab/recorded/rdp\n"
    "wrm_path=/var/wab/recorded/rdp\n"
    "alternate_shell=C:\\WINDOWS\\NOTEPAD.EXE\n"
    "shell_working_directory=C:\\WINDOWS\\\n"
    "\n"
    "[client]\n"
    "ignore_logon_password=yes\n"
    "performance_flags_default=0x00000007\n"
    "performance_flags_force_present=0x1\n"
    "performance_flags_force_not_present=0x0\n"
    "tls_fallback_legacy=yes\n"
    "[debug]\n"
    "log_type=file\n"
    "log_file_path=/var/log/redemption.log\n"
    "\n"
    );

    Inifile ini(oss);

    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.movie_path));
    BOOST_CHECK_EQUAL(std::string("flv"),               std::string(ini.globals.codec_id));
    BOOST_CHECK_EQUAL(std::string("medium"),            std::string(ini.globals.video_quality));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.auth_user));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.host));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_device));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_user));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.globals.auth_channel, "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(3390,                             ini.globals.port);
    BOOST_CHECK_EQUAL(0,                                ini.globals.encryptionLevel);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.globals.authip));
    BOOST_CHECK_EQUAL(3350,                             ini.globals.authport);
    BOOST_CHECK_EQUAL(false,                            ini.globals.nomouse);
    BOOST_CHECK_EQUAL(false,                            ini.globals.notimestamp);
    BOOST_CHECK_EQUAL(false,                            ini.globals.autovalidate);
    BOOST_CHECK_EQUAL(std::string("/tmp/rdpproxy/"),    std::string(ini.globals.dynamic_conf_path));

    BOOST_CHECK_EQUAL(1,                                ini.globals.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.globals.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.globals.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.globals.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.globals.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.globals.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.globals.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.globals.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.globals.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.globals.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.globals.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.globals.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.globals.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.globals.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.globals.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.globals.replay_path));

    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("192.168.1.1"),       std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("redemption"),        std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string("/var/tmp/wab/recorded/rdp"),
                                                        std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string("/var/wab/recorded/rdp"),
                                                        std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string("C:\\WINDOWS\\NOTEPAD.EXE"),
                                                        std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string("C:\\WINDOWS\\"),     std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.input);
    // log
    BOOST_CHECK_EQUAL(3,                                ini.globals.debug.log_type);
    BOOST_CHECK_EQUAL(std::string("/var/log/redemption.log"),
                                                        std::string(ini.globals.debug.log_file_path));

    BOOST_CHECK_EQUAL(true,                             ini.globals.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(7,                                ini.globals.client.performance_flags_default);
    BOOST_CHECK_EQUAL(1,                                ini.globals.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(true,                             ini.globals.client.tls_fallback_legacy);
}

BOOST_AUTO_TEST_CASE(TestConfig1bis)
{
    // test we can read a config file with a global section
    // alternative ways to say yes in file, other values
    std::stringstream oss(
    "[globals]\n"
    "bitmap_cache=true\n"
    "bitmap_compression=on\n"
    "encryptionLevel=medium\n"
    "enable_file_encryption=no\n"
    "enable_tls=yes\n"
    "listen_address=0.0.0.0\n"
    "enable_ip_transparent=no\n"
    "certificate_password=\n"
    "png_path=/var/tmp/wab/recorded/rdp\n"
    "wrm_path=/var/wab/recorded/rdp\n"
    "alternate_shell=\n"
    "shell_working_directory=\n"
    "[client]\n"
    "performance_flags_default=7\n"
    "performance_flags_force_present=1\n"
    "performance_flags_force_not_present=0\n"
    "\n"
    );

    Inifile ini(oss);

    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.movie_path));
    BOOST_CHECK_EQUAL(std::string("flv"),               std::string(ini.globals.codec_id));
    BOOST_CHECK_EQUAL(std::string("medium"),            std::string(ini.globals.video_quality));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.auth_user));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.host));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_device));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_user));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.globals.auth_channel, "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(3389,                             ini.globals.port);
    BOOST_CHECK_EQUAL(1,                                ini.globals.encryptionLevel);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.globals.authip));
    BOOST_CHECK_EQUAL(3350,                             ini.globals.authport);
    BOOST_CHECK_EQUAL(false,                            ini.globals.nomouse);
    BOOST_CHECK_EQUAL(false,                            ini.globals.notimestamp);
    BOOST_CHECK_EQUAL(false,                            ini.globals.autovalidate);
    BOOST_CHECK_EQUAL(std::string("/tmp/rdpproxy/"),    std::string(ini.globals.dynamic_conf_path));

    BOOST_CHECK_EQUAL(1,                                ini.globals.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.globals.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.globals.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.globals.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.globals.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.globals.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.globals.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.globals.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.globals.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.globals.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.globals.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.globals.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.globals.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.globals.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.globals.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.globals.replay_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("0.0.0.0"),           std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string("/var/tmp/wab/recorded/rdp"),
                                                        std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string("/var/wab/recorded/rdp"),
                                                        std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.input);
    // log
    BOOST_CHECK_EQUAL(2,                                ini.globals.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(7,                                ini.globals.client.performance_flags_default);
    BOOST_CHECK_EQUAL(1,                                ini.globals.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.globals.client.tls_fallback_legacy);
}

BOOST_AUTO_TEST_CASE(TestConfig2)
{
    // test we can read a config file with a global section, other values
    std::stringstream oss(
    "[globals]\n"
    "bitmap_cache=no\n"
    "bitmap_compression=false\n"
    "encryptionLevel=high\n"
    "enable_file_encryption=true\n"
    "enable_tls=yes\n"
    "listen_address=127.0.0.1\n"
    "certificate_password=rdpproxy\n"
    "enable_ip_transparent=true\n"
    "png_path=/var/tmp/wab/recorded/rdp\n"
    "wrm_path=/var/wab/recorded/rdp\n"
//    "alternate_shell=C:\\Program\ Files\\Microsoft Visual Studio\\Common\\MSDev98\\Bin\\MSDEV.EXE\n"
    "shell_working_directory=\n"
    "[client]\n"
    "performance_flags_default=07\n"
    "performance_flags_force_present=1\n"
    "performance_flags_force_not_present=0x\n"
    "\n"
    );

    Inifile ini(oss);

    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.movie_path));
    BOOST_CHECK_EQUAL(std::string("flv"),               std::string(ini.globals.codec_id));
    BOOST_CHECK_EQUAL(std::string("medium"),            std::string(ini.globals.video_quality));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.auth_user));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.host));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_device));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_user));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.globals.auth_channel, "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(false,                            ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(false,                            ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(3389,                             ini.globals.port);
    BOOST_CHECK_EQUAL(2,                                ini.globals.encryptionLevel);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.globals.authip));
    BOOST_CHECK_EQUAL(3350,                             ini.globals.authport);
    BOOST_CHECK_EQUAL(false,                            ini.globals.nomouse);
    BOOST_CHECK_EQUAL(false,                            ini.globals.notimestamp);
    BOOST_CHECK_EQUAL(false,                            ini.globals.autovalidate);
    BOOST_CHECK_EQUAL(std::string("/tmp/rdpproxy/"),    std::string(ini.globals.dynamic_conf_path));

    BOOST_CHECK_EQUAL(1,                                ini.globals.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.globals.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.globals.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.globals.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.globals.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.globals.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.globals.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.globals.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.globals.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.globals.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.globals.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.globals.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.globals.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.globals.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.globals.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.globals.replay_path));

    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("rdpproxy"),          std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string("/var/tmp/wab/recorded/rdp"),
                                                        std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string("/var/wab/recorded/rdp"),
                                                        std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),
//        std::string("C:\\Program Files\\Microsoft Visual Studio\\Common\\MSDev98\\Bin\\MSDEV.EXE"),
                                                        std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.input);
    // log
    BOOST_CHECK_EQUAL(2,                                ini.globals.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(7,                                ini.globals.client.performance_flags_default);
    BOOST_CHECK_EQUAL(1,                                ini.globals.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.globals.client.tls_fallback_legacy);
}

BOOST_AUTO_TEST_CASE(TestMultiple)
{
    // test we can read a config file with a global section
    std::stringstream oss(
    "[globals]\n"
    "bitmap_cache=no\n"
    "bitmap_compression=TRuE\n"
    "port=3390\n"
    "encryptionLevel=low\n"
    "enable_file_encryption=False\n"
    "enable_tls=False\n"
    "listen_address=0.0.0.0\n"
    "certificate_password=redemption\n"
    "enable_ip_transparent=False\n"
    "shell_working_directory=%HOMEDRIVE%%HOMEPATH%\n"
    "\n"
    );

    Inifile ini(oss);

    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.movie_path));
    BOOST_CHECK_EQUAL(std::string("flv"),               std::string(ini.globals.codec_id));
    BOOST_CHECK_EQUAL(std::string("medium"),            std::string(ini.globals.video_quality));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.auth_user));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.host));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_device));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_user));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.globals.auth_channel, "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(false,                            ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(3390,                             ini.globals.port);
    BOOST_CHECK_EQUAL(0,                                ini.globals.encryptionLevel);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.globals.authip));
    BOOST_CHECK_EQUAL(3350,                             ini.globals.authport);
    BOOST_CHECK_EQUAL(false,                            ini.globals.nomouse);
    BOOST_CHECK_EQUAL(false,                            ini.globals.notimestamp);
    BOOST_CHECK_EQUAL(false,                            ini.globals.autovalidate);
    BOOST_CHECK_EQUAL(std::string("/tmp/rdpproxy/"),    std::string(ini.globals.dynamic_conf_path));

    BOOST_CHECK_EQUAL(1,                                ini.globals.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.globals.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.globals.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.globals.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.globals.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.globals.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.globals.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.globals.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.globals.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.globals.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.globals.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.globals.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.globals.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.globals.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.globals.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.globals.replay_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("0.0.0.0"),           std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("redemption"),        std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string(PNG_PATH),            std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string(WRM_PATH),            std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string("%HOMEDRIVE%%HOMEPATH%"),
                                                        std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.input);
    // log
    BOOST_CHECK_EQUAL(2,                                ini.globals.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.globals.client.tls_fallback_legacy);


    // see we can change configuration using parse without default setting of existing ini
    std::stringstream oss2(
    "[globals]\n"
    "bitmap_compression=no\n"
    "enable_file_encryption=yes\n"
    "enable_tls=yes\n"
    "listen_address=192.168.1.1\n"
    "certificate_password=\n"
    "enable_ip_transparent=yes\n"
    "[debug]\n"
    "log_type=encryptedfile\n"
    );
    ini.cparse(oss2);

    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.movie_path));
    BOOST_CHECK_EQUAL(std::string("flv"),               std::string(ini.globals.codec_id));
    BOOST_CHECK_EQUAL(std::string("medium"),            std::string(ini.globals.video_quality));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.auth_user));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.host));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_device));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_user));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.globals.auth_channel, "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(false,                            ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(false,                            ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(3390,                             ini.globals.port);
    BOOST_CHECK_EQUAL(0,                                ini.globals.encryptionLevel);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.globals.authip));
    BOOST_CHECK_EQUAL(3350,                             ini.globals.authport);
    BOOST_CHECK_EQUAL(false,                            ini.globals.nomouse);
    BOOST_CHECK_EQUAL(false,                            ini.globals.notimestamp);
    BOOST_CHECK_EQUAL(false,                            ini.globals.autovalidate);
    BOOST_CHECK_EQUAL(std::string("/tmp/rdpproxy/"),    std::string(ini.globals.dynamic_conf_path));

    BOOST_CHECK_EQUAL(1,                                ini.globals.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.globals.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.globals.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.globals.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.globals.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.globals.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.globals.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.globals.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.globals.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.globals.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.globals.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.globals.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.globals.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.globals.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.globals.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.globals.replay_path));

    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("192.168.1.1"),       std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string(PNG_PATH),            std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string(WRM_PATH),            std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string("%HOMEDRIVE%%HOMEPATH%"),
                                                        std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.input);
    // log
    BOOST_CHECK_EQUAL(4,                                ini.globals.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.globals.client.tls_fallback_legacy);
}

BOOST_AUTO_TEST_CASE(TestNewConf)
{
    // new behavior:
    // init() load default values from main configuration file
    // - options with multiple occurences get the last value
    // - unrecognized lines are ignored
    // - every characters following # are ignored until end of line (comments)
    Inifile ini;

    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.movie_path));
    BOOST_CHECK_EQUAL(std::string("flv"),               std::string(ini.globals.codec_id));
    BOOST_CHECK_EQUAL(std::string("medium"),            std::string(ini.globals.video_quality));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.auth_user));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.host));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_device));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_user));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.globals.auth_channel, "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(3389,                             ini.globals.port);
    BOOST_CHECK_EQUAL(0,                                ini.globals.encryptionLevel);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.globals.authip));
    BOOST_CHECK_EQUAL(3350,                             ini.globals.authport);
    BOOST_CHECK_EQUAL(false,                            ini.globals.nomouse);
    BOOST_CHECK_EQUAL(false,                            ini.globals.notimestamp);
    BOOST_CHECK_EQUAL(false,                            ini.globals.autovalidate);
    BOOST_CHECK_EQUAL(std::string("/tmp/rdpproxy/"),    std::string(ini.globals.dynamic_conf_path));

    BOOST_CHECK_EQUAL(1,                                ini.globals.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.globals.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.globals.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.globals.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.globals.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.globals.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.globals.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.globals.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.globals.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.globals.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.globals.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.globals.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.globals.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.globals.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.globals.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.globals.replay_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("0.0.0.0"),           std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("inquisition"),       std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string(PNG_PATH),            std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string(WRM_PATH),            std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.input);
    // log
    BOOST_CHECK_EQUAL(2,                                ini.globals.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.globals.client.tls_fallback_legacy);


    std::stringstream ifs2(
    "# Here we put global values\n"
    "[globals]\n"
    "bitmap_compression=no # here we have a comment to end of line\n"
    "# below we have lines with syntax errors, but they are just ignored\n"
    "yyy\n"
    "zzz\n"
    "# unknwon keys are also ignored\n"
    "yyy=1\n"
    );

    ini.cparse(ifs2);

    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.movie_path));
    BOOST_CHECK_EQUAL(std::string("flv"),               std::string(ini.globals.codec_id));
    BOOST_CHECK_EQUAL(std::string("medium"),            std::string(ini.globals.video_quality));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.auth_user));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.host));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_device));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_user));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.globals.auth_channel, "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(false,                            ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(3389,                             ini.globals.port);
    BOOST_CHECK_EQUAL(0,                                ini.globals.encryptionLevel);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.globals.authip));
    BOOST_CHECK_EQUAL(3350,                             ini.globals.authport);
    BOOST_CHECK_EQUAL(false,                            ini.globals.nomouse);
    BOOST_CHECK_EQUAL(false,                            ini.globals.notimestamp);
    BOOST_CHECK_EQUAL(false,                            ini.globals.autovalidate);
    BOOST_CHECK_EQUAL(std::string("/tmp/rdpproxy/"),    std::string(ini.globals.dynamic_conf_path));

    BOOST_CHECK_EQUAL(1,                                ini.globals.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.globals.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.globals.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.globals.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.globals.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.globals.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.globals.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.globals.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.globals.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.globals.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.globals.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.globals.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.globals.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.globals.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.globals.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.globals.replay_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("0.0.0.0"),           std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("inquisition"),       std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string(PNG_PATH),            std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string(WRM_PATH),            std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.input);

    BOOST_CHECK_EQUAL(2,                                ini.globals.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.globals.client.tls_fallback_legacy);


    // back to default values
    ini.init();

    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.globals.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.movie_path));
    BOOST_CHECK_EQUAL(std::string("flv"),               std::string(ini.globals.codec_id));
    BOOST_CHECK_EQUAL(std::string("medium"),            std::string(ini.globals.video_quality));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.auth_user));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.host));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_device));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.target_user));
    BOOST_CHECK_EQUAL(0,                                memcmp(ini.globals.auth_channel, "\0\0\0\0\0\0\0\0", 8));

    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_cache);
    BOOST_CHECK_EQUAL(true,                             ini.globals.bitmap_compression);
    BOOST_CHECK_EQUAL(3389,                             ini.globals.port);
    BOOST_CHECK_EQUAL(0,                                ini.globals.encryptionLevel);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.globals.authip));
    BOOST_CHECK_EQUAL(3350,                             ini.globals.authport);
    BOOST_CHECK_EQUAL(false,                            ini.globals.nomouse);
    BOOST_CHECK_EQUAL(false,                            ini.globals.notimestamp);
    BOOST_CHECK_EQUAL(false,                            ini.globals.autovalidate);
    BOOST_CHECK_EQUAL(std::string("/tmp/rdpproxy/"),    std::string(ini.globals.dynamic_conf_path));

    BOOST_CHECK_EQUAL(1,                                ini.globals.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.globals.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.globals.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.globals.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.globals.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.globals.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.globals.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.globals.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.globals.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.globals.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.globals.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.globals.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.globals.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.globals.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.globals.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.globals.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.globals.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.globals.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.globals.replay_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("0.0.0.0"),           std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("inquisition"),       std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string(PNG_PATH),            std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string(WRM_PATH),            std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.globals.debug.input);
    // log
    BOOST_CHECK_EQUAL(2,                                ini.globals.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.globals.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.globals.client.tls_fallback_legacy);
}

BOOST_AUTO_TEST_CASE(TestConfigTools)
{
    BOOST_CHECK_EQUAL(0,    long_from_cstr("0"));
    BOOST_CHECK_EQUAL(0,    long_from_cstr("0x"));

    BOOST_CHECK_EQUAL(3,    long_from_cstr("3"));
    BOOST_CHECK_EQUAL(3,    long_from_cstr("0x3"));

    BOOST_CHECK_EQUAL(7,    long_from_cstr("0x00000007"));
    BOOST_CHECK_EQUAL(7,    long_from_cstr("0x0000000000000007"));
    BOOST_CHECK_EQUAL(7,    long_from_cstr("0x0007    "));

    BOOST_CHECK_EQUAL(1357, long_from_cstr("1357"));
    BOOST_CHECK_EQUAL(4951, long_from_cstr("0x1357"));

    BOOST_CHECK_EQUAL(10,   long_from_cstr("0x0A"));
    BOOST_CHECK_EQUAL(10,   long_from_cstr("0x0a"));

    BOOST_CHECK_EQUAL(0,    long_from_cstr("0x0000000I"));
    BOOST_CHECK_EQUAL(0,    long_from_cstr("I"));

    BOOST_CHECK_EQUAL(0,    level_from_cstr("LoW"));
    BOOST_CHECK_EQUAL(1,    level_from_cstr("mEdIuM"));
    BOOST_CHECK_EQUAL(2,    level_from_cstr("High"));

    BOOST_CHECK_EQUAL(0,    logtype_from_cstr(""));
    BOOST_CHECK_EQUAL(0,    logtype_from_cstr("null"));
    BOOST_CHECK_EQUAL(0,    logtype_from_cstr("NULL"));
    BOOST_CHECK_EQUAL(0,    logtype_from_cstr("unknown"));
    BOOST_CHECK_EQUAL(1,    logtype_from_cstr("print"));
    BOOST_CHECK_EQUAL(1,    logtype_from_cstr("Print"));
    BOOST_CHECK_EQUAL(2,    logtype_from_cstr("syslog"));
    BOOST_CHECK_EQUAL(2,    logtype_from_cstr("SYSLOG"));
    BOOST_CHECK_EQUAL(3,    logtype_from_cstr("file"));
    BOOST_CHECK_EQUAL(3,    logtype_from_cstr("FiLe"));
    BOOST_CHECK_EQUAL(4,    logtype_from_cstr("encryptedfile"));
    BOOST_CHECK_EQUAL(4,    logtype_from_cstr("EncryptedFile"));
}

//BOOST_AUTO_TEST_CASE(TestAuthentificationKeywordRecognition)
//{
//    BOOST_CHECK_EQUAL(AUTHID_UNKNOWN, authid_from_string("unknown"));
//    BOOST_CHECK_EQUAL(AUTHID_TARGET_USER, authid_from_string(STRAUTHID_TARGET_USER));
//    BOOST_CHECK_EQUAL(AUTHID_TARGET_PASSWORD, authid_from_string(STRAUTHID_TARGET_PASSWORD));
//    BOOST_CHECK_EQUAL(AUTHID_HOST, authid_from_string(STRAUTHID_HOST));
//    BOOST_CHECK_EQUAL(AUTHID_PASSWORD, authid_from_string(STRAUTHID_PASSWORD));
//    BOOST_CHECK_EQUAL(AUTHID_AUTH_USER, authid_from_string(STRAUTHID_AUTH_USER));
//    BOOST_CHECK_EQUAL(AUTHID_TARGET_DEVICE, authid_from_string(STRAUTHID_TARGET_DEVICE));
//    BOOST_CHECK_EQUAL(AUTHID_TARGET_PORT, authid_from_string(STRAUTHID_TARGET_PORT));
//    BOOST_CHECK_EQUAL(AUTHID_TARGET_PROTOCOL, authid_from_string(STRAUTHID_TARGET_PROTOCOL));
//    BOOST_CHECK_EQUAL(AUTHID_REJECTED, authid_from_string(STRAUTHID_REJECTED));
//    BOOST_CHECK_EQUAL(AUTHID_OPT_MOVIE, authid_from_string(STRAUTHID_OPT_MOVIE));
//    BOOST_CHECK_EQUAL(AUTHID_OPT_MOVIE_PATH, authid_from_string(STRAUTHID_OPT_MOVIE_PATH));
//    BOOST_CHECK_EQUAL(AUTHID_OPT_CLIPBOARD, authid_from_string(STRAUTHID_OPT_CLIPBOARD));
//    BOOST_CHECK_EQUAL(AUTHID_OPT_DEVICEREDIRECTION, authid_from_string(STRAUTHID_OPT_DEVICEREDIRECTION));
//    BOOST_CHECK_EQUAL(AUTHID_END_DATE_CNX, authid_from_string(STRAUTHID_END_DATE_CNX));
//    BOOST_CHECK_EQUAL(AUTHID_MESSAGE, authid_from_string(STRAUTHID_MESSAGE));
//    BOOST_CHECK_EQUAL(AUTHID_OPT_BITRATE, authid_from_string(STRAUTHID_OPT_BITRATE));
//    BOOST_CHECK_EQUAL(AUTHID_OPT_FRAMERATE, authid_from_string(STRAUTHID_OPT_FRAMERATE));
//    BOOST_CHECK_EQUAL(AUTHID_OPT_QSCALE, authid_from_string(STRAUTHID_OPT_QSCALE));
//    BOOST_CHECK_EQUAL(AUTHID_OPT_CODEC_ID, authid_from_string(STRAUTHID_OPT_CODEC_ID));
//    BOOST_CHECK_EQUAL(AUTHID_OPT_WIDTH, authid_from_string(STRAUTHID_OPT_WIDTH));
//    BOOST_CHECK_EQUAL(AUTHID_OPT_HEIGHT, authid_from_string(STRAUTHID_OPT_HEIGHT));
//    BOOST_CHECK_EQUAL(AUTHID_OPT_BPP, authid_from_string(STRAUTHID_OPT_BPP));
//    BOOST_CHECK_EQUAL(AUTHID_PROXY_TYPE, authid_from_string(STRAUTHID_PROXY_TYPE));
//    BOOST_CHECK_EQUAL(AUTHID_AUTHENTICATED, authid_from_string(STRAUTHID_AUTHENTICATED));
//    BOOST_CHECK_EQUAL(AUTHID_SELECTOR, authid_from_string(STRAUTHID_SELECTOR));
//    BOOST_CHECK_EQUAL(AUTHID_KEEPALIVE, authid_from_string(STRAUTHID_KEEPALIVE));
//    BOOST_CHECK_EQUAL(AUTHID_UNKNOWN, authid_from_string("8899676"));
//    BOOST_CHECK_EQUAL(AUTHID_DISPLAY_MESSAGE, authid_from_string(STRAUTHID_DISPLAY_MESSAGE));
//    BOOST_CHECK_EQUAL(AUTHID_ACCEPT_MESSAGE, authid_from_string(STRAUTHID_ACCEPT_MESSAGE));
//    BOOST_CHECK_EQUAL(AUTHID_AUTH_ERROR_MESSAGE, authid_from_string(STRAUTHID_AUTH_ERROR_MESSAGE));
//    BOOST_CHECK_EQUAL(AUTHID_PROXY_TYPE, authid_from_string(STRAUTHID_PROXY_TYPE));
//    BOOST_CHECK_EQUAL(AUTHID_TRANS_BUTTON_OK, authid_from_string(STRAUTHID_TRANS_BUTTON_OK));
//    BOOST_CHECK_EQUAL(AUTHID_TRANS_BUTTON_CANCEL, authid_from_string(STRAUTHID_TRANS_BUTTON_CANCEL));
//    BOOST_CHECK_EQUAL(AUTHID_TRANS_BUTTON_HELP, authid_from_string(STRAUTHID_TRANS_BUTTON_HELP));
//    BOOST_CHECK_EQUAL(AUTHID_TRANS_BUTTON_CLOSE, authid_from_string(STRAUTHID_TRANS_BUTTON_CLOSE));
//    BOOST_CHECK_EQUAL(AUTHID_TRANS_BUTTON_REFUSED, authid_from_string(STRAUTHID_TRANS_BUTTON_REFUSED));
//    BOOST_CHECK_EQUAL(AUTHID_TRANS_LOGIN, authid_from_string(STRAUTHID_TRANS_LOGIN));
//    BOOST_CHECK_EQUAL(AUTHID_TRANS_USERNAME, authid_from_string(STRAUTHID_TRANS_USERNAME));
//    BOOST_CHECK_EQUAL(AUTHID_TRANS_PASSWORD, authid_from_string(STRAUTHID_TRANS_PASSWORD));
//    BOOST_CHECK_EQUAL(AUTHID_TRANS_TARGET, authid_from_string(STRAUTHID_TRANS_TARGET));
//    BOOST_CHECK_EQUAL(AUTHID_TRANS_DIAGNOSTIC, authid_from_string(STRAUTHID_TRANS_DIAGNOSTIC));
//    BOOST_CHECK_EQUAL(AUTHID_TRANS_CONNECTION_CLOSED, authid_from_string(STRAUTHID_TRANS_CONNECTION_CLOSED));
//    BOOST_CHECK_EQUAL(AUTHID_MODE_CONSOLE, authid_from_string(STRAUTHID_MODE_CONSOLE));
//    BOOST_CHECK_EQUAL(AUTHID_VIDEO_QUALITY, authid_from_string(STRAUTHID_VIDEO_QUALITY));
//    BOOST_CHECK_EQUAL(AUTHID_TIMEZONE, authid_from_string(STRAUTHID_TIMEZONE));
//}
