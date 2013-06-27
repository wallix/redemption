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

    BOOST_CHECK_EQUAL(true,                             ini.video.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.video.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(false,                            ini.globals.movie);
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

    BOOST_CHECK_EQUAL(1,                                ini.video.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.video.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.video.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.video.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.video.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.video.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.video.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.video.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.video.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.video.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.video.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.video.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.video.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.video.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.video.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.video.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.video.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.video.replay_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("0.0.0.0"),           std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("inquisition"),       std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string(PNG_PATH),            std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string(WRM_PATH),            std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_bitmap_update);

    BOOST_CHECK_EQUAL(0,                                ini.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.debug.input);
    // log file
    BOOST_CHECK_EQUAL(2,                                ini.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.client.tls_fallback_legacy);
    BOOST_CHECK_EQUAL(true,                             ini.client.clipboard);
    BOOST_CHECK_EQUAL(true,                             ini.client.device_redirection);

    BOOST_CHECK_EQUAL(std::string("OK"),                std::string(ini.translation.button_ok.c_str()));
    BOOST_CHECK_EQUAL(std::string("Cancel"),            std::string(ini.translation.button_cancel.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help"),              std::string(ini.translation.button_help.c_str()));
    BOOST_CHECK_EQUAL(std::string("Close"),             std::string(ini.translation.button_close.c_str()));
    BOOST_CHECK_EQUAL(std::string("Refused"),           std::string(ini.translation.button_refused.c_str()));
    BOOST_CHECK_EQUAL(std::string("login"),             std::string(ini.translation.login.c_str()));
    BOOST_CHECK_EQUAL(std::string("username"),          std::string(ini.translation.username.c_str()));
    BOOST_CHECK_EQUAL(std::string("password"),          std::string(ini.translation.password.c_str()));
    BOOST_CHECK_EQUAL(std::string("target"),            std::string(ini.translation.target.c_str()));
    BOOST_CHECK_EQUAL(std::string("diagnostic"),        std::string(ini.translation.diagnostic.c_str()));
    BOOST_CHECK_EQUAL(std::string("Connection closed"), std::string(ini.translation.connection_closed.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help message"),      std::string(ini.translation.help_message.c_str()));

    BOOST_CHECK_EQUAL(0,                                ini.context.selector_focus);

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.movie));

    BOOST_CHECK_EQUAL(40000,                            ini.context.opt_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.context.opt_framerate);
    BOOST_CHECK_EQUAL(15,                               ini.context.opt_qscale);

    BOOST_CHECK_EQUAL(false,                            ini.context.ask_opt_bpp);
    BOOST_CHECK_EQUAL(false,                            ini.context.ask_opt_height);
    BOOST_CHECK_EQUAL(false,                            ini.context.ask_opt_width);

    BOOST_CHECK_EQUAL(800,                              ini.context.opt_width);
    BOOST_CHECK_EQUAL(600,                              ini.context.opt_height);
    BOOST_CHECK_EQUAL(24,                               ini.context.opt_bpp);

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.auth_error_message.c_str()));

    BOOST_CHECK_EQUAL(false,                            ini.context.ask_selector);
    BOOST_CHECK_EQUAL(false,                            ini.context.ask_selector_current_page);
    BOOST_CHECK_EQUAL(false,                            ini.context.ask_selector_device_filter);
    BOOST_CHECK_EQUAL(false,                            ini.context.ask_selector_group_filter);
    BOOST_CHECK_EQUAL(false,                            ini.context.ask_selector_lines_per_page);

    BOOST_CHECK_EQUAL(false,                            ini.context.selector);
    BOOST_CHECK_EQUAL(1,                                ini.context.selector_current_page);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.selector_device_filter.c_str()));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.selector_group_filter.c_str()));
    BOOST_CHECK_EQUAL(20,                               ini.context.selector_lines_per_page);
    BOOST_CHECK_EQUAL(1,                                ini.context.selector_number_of_pages);

    BOOST_CHECK_EQUAL(true,                             ini.context.ask_target_device);
    BOOST_CHECK_EQUAL(true,                             ini.context.ask_target_password);
    BOOST_CHECK_EQUAL(true,                             ini.context.ask_target_port);
    BOOST_CHECK_EQUAL(true,                             ini.context.ask_target_protocol);
    BOOST_CHECK_EQUAL(true,                             ini.context.ask_target_user);

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.target_password.c_str()));
    BOOST_CHECK_EQUAL(3389,                             ini.context.target_port);
    BOOST_CHECK_EQUAL(std::string("RDP"),               std::string(ini.context.target_protocol.c_str()));

    BOOST_CHECK_EQUAL(false,                            ini.context.ask_host);
    BOOST_CHECK_EQUAL(false,                            ini.context.ask_target);

    BOOST_CHECK_EQUAL(true,                             ini.context.ask_auth_user);
    BOOST_CHECK_EQUAL(true,                             ini.context.ask_password);


    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.password.c_str()));

    BOOST_CHECK_EQUAL(false,                            ini.context.ask_authchannel_target);
    BOOST_CHECK_EQUAL(false,                            ini.context.ask_authchannel_result);

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.authchannel_answer.c_str()));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.authchannel_result.c_str()));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.authchannel_target.c_str()));

    BOOST_CHECK_EQUAL(false,                            ini.context.ask_accept_message);
    BOOST_CHECK_EQUAL(false,                            ini.context.ask_display_message);

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.message.c_str()));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.accept_message.c_str()));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.display_message.c_str()));

    BOOST_CHECK_EQUAL(std::string("Connection refused by authentifier."),
                                                        std::string(ini.context.rejected.c_str()));

    BOOST_CHECK_EQUAL(false,                            ini.context.authenticated);

    BOOST_CHECK_EQUAL(true,                             ini.context.ask_keepalive);
    BOOST_CHECK_EQUAL(false,                            ini.context.ask_proxy_type);

    BOOST_CHECK_EQUAL(false,                            ini.context.keepalive);
    BOOST_CHECK_EQUAL(std::string("RDP"),               std::string(ini.context.proxy_type.c_str()));

    BOOST_CHECK_EQUAL(false,                            ini.context.ask_trace_seal);

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.trace_seal.c_str()));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.session_id.c_str()));

    BOOST_CHECK_EQUAL(0,                                ini.context.end_date_cnx);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.end_time.c_str()));

    BOOST_CHECK_EQUAL(std::string("allow"),             std::string(ini.context.mode_console.c_str()));
    BOOST_CHECK_EQUAL(-3600,                            ini.context.timezone);

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.real_target_device.c_str()));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.authentication_challenge.c_str()));
}

BOOST_AUTO_TEST_CASE(TestConfigDefaultEmpty)
{
    // default config
    Inifile ini;
    char    buffer[128];

    BOOST_CHECK_EQUAL(true,                             ini.video.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.video.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(false,                            ini.globals.movie);
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

    BOOST_CHECK_EQUAL(1,                                ini.video.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.video.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.video.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.video.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.video.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.video.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.video.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.video.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.video.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.video.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.video.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.video.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.video.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.video.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.video.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.video.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.video.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.video.replay_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("0.0.0.0"),           std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("inquisition"),       std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string(PNG_PATH),            std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string(WRM_PATH),            std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_bitmap_update);

    BOOST_CHECK_EQUAL(0,                                ini.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.debug.input);
    // log file
    BOOST_CHECK_EQUAL(2,                                ini.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.client.tls_fallback_legacy);
    BOOST_CHECK_EQUAL(true,                             ini.client.clipboard);
    BOOST_CHECK_EQUAL(true,                             ini.client.device_redirection);


    BOOST_CHECK_EQUAL(std::string("OK"),                std::string(ini.context_get_value(AUTHID_TRANS_BUTTON_OK,         buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Cancel"),            std::string(ini.context_get_value(AUTHID_TRANS_BUTTON_CANCEL,     buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Help"),              std::string(ini.context_get_value(AUTHID_TRANS_BUTTON_HELP,       buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Close"),             std::string(ini.context_get_value(AUTHID_TRANS_BUTTON_CLOSE,      buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Refused"),           std::string(ini.context_get_value(AUTHID_TRANS_BUTTON_REFUSED,    buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("login"),             std::string(ini.context_get_value(AUTHID_TRANS_LOGIN,             buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("username"),          std::string(ini.context_get_value(AUTHID_TRANS_USERNAME,          buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("password"),          std::string(ini.context_get_value(AUTHID_TRANS_PASSWORD,          buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("target"),            std::string(ini.context_get_value(AUTHID_TRANS_TARGET,            buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("diagnostic"),        std::string(ini.context_get_value(AUTHID_TRANS_DIAGNOSTIC,        buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Connection closed"), std::string(ini.context_get_value(AUTHID_TRANS_CONNECTION_CLOSED, buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Help message"),      std::string(ini.context_get_value(AUTHID_TRANS_HELP_MESSAGE,      buffer, sizeof(buffer))));

    BOOST_CHECK_EQUAL(0,                                ini.context.selector_focus);

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context.movie));

    BOOST_CHECK_EQUAL(std::string("40000"),             std::string(ini.context_get_value(AUTHID_OPT_BITRATE,   buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("5"),                 std::string(ini.context_get_value(AUTHID_OPT_FRAMERATE, buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("15"),                std::string(ini.context_get_value(AUTHID_OPT_QSCALE,    buffer, sizeof(buffer))));

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_OPT_BPP));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_OPT_HEIGHT));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_OPT_WIDTH));

    BOOST_CHECK_EQUAL(std::string("800"),               std::string(ini.context_get_value(AUTHID_OPT_WIDTH,  buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("600"),               std::string(ini.context_get_value(AUTHID_OPT_HEIGHT, buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("24"),                std::string(ini.context_get_value(AUTHID_OPT_BPP,    buffer, sizeof(buffer))));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_AUTH_ERROR_MESSAGE, buffer, sizeof(buffer))));

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_SELECTOR));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_SELECTOR_CURRENT_PAGE));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_SELECTOR_DEVICE_FILTER));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_SELECTOR_GROUP_FILTER));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_SELECTOR_LINES_PER_PAGE));

    BOOST_CHECK_EQUAL(std::string("False"),             std::string(ini.context_get_value(AUTHID_SELECTOR,                 buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("1"),                 std::string(ini.context_get_value(AUTHID_SELECTOR_CURRENT_PAGE,    buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_SELECTOR_DEVICE_FILTER,   buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_SELECTOR_GROUP_FILTER,    buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("20"),                std::string(ini.context_get_value(AUTHID_SELECTOR_LINES_PER_PAGE,  buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("1"),                 std::string(ini.context_get_value(AUTHID_SELECTOR_NUMBER_OF_PAGES, buffer, sizeof(buffer))));

    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_TARGET_DEVICE));
    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_TARGET_PASSWORD));
    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_TARGET_PORT));
    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_TARGET_PROTOCOL));
    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_TARGET_USER));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_TARGET_DEVICE,   buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_TARGET_PASSWORD, buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_TARGET_PORT,     buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_TARGET_PROTOCOL, buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_TARGET_USER,     buffer, sizeof(buffer))));

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_HOST));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_TARGET));

    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_AUTH_USER));
    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_PASSWORD));


    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_AUTH_USER, buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_PASSWORD,  buffer, sizeof(buffer))));

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_AUTHCHANNEL_TARGET));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_AUTHCHANNEL_RESULT));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_AUTHCHANNEL_ANSWER, buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_AUTHCHANNEL_RESULT, buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_AUTHCHANNEL_TARGET, buffer, sizeof(buffer))));

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_ACCEPT_MESSAGE));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_DISPLAY_MESSAGE));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_MESSAGE,         buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_ACCEPT_MESSAGE,  buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_DISPLAY_MESSAGE, buffer, sizeof(buffer))));

    BOOST_CHECK_EQUAL(std::string("Connection refused by authentifier."),
                                                        std::string(ini.context_get_value(AUTHID_REJECTED, buffer, sizeof(buffer))));


    BOOST_CHECK_EQUAL(std::string("False"),             std::string(ini.context_get_value(AUTHID_AUTHENTICATED, buffer, sizeof(buffer))));

    BOOST_CHECK_EQUAL(false,                            ini.context_get_bool(AUTHID_AUTHENTICATED));


    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_KEEPALIVE));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_PROXY_TYPE));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_KEEPALIVE,  buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("RDP"),               std::string(ini.context_get_value(AUTHID_PROXY_TYPE, buffer, sizeof(buffer))));


    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_TRACE_SEAL));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_TRACE_SEAL, buffer, sizeof(buffer))));


    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_SESSION_ID, buffer, sizeof(buffer))));


    BOOST_CHECK_EQUAL(std::string("0"),                 std::string(ini.context_get_value(AUTHID_END_DATE_CNX, buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_END_TIME,     buffer, sizeof(buffer))));

    BOOST_CHECK_EQUAL(std::string("allow"),             std::string(ini.context_get_value(AUTHID_MODE_CONSOLE, buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("-3600"),             std::string(ini.context_get_value(AUTHID_TIMEZONE,     buffer, sizeof(buffer))));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_REAL_TARGET_DEVICE, buffer, sizeof(buffer))));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.context_get_value(AUTHID_AUTHENTICATION_CHALLENGE, buffer, sizeof(buffer))));
}

BOOST_AUTO_TEST_CASE(TestConfigDefault)
{
    // test we can read a config file with a global section
    std::stringstream oss("");

    Inifile ini(oss);

    BOOST_CHECK_EQUAL(true,                             ini.video.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.video.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(false,                            ini.globals.movie);
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

    BOOST_CHECK_EQUAL(1,                                ini.video.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.video.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.video.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.video.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.video.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.video.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.video.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.video.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.video.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.video.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.video.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.video.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.video.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.video.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.video.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.video.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.video.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.video.replay_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("0.0.0.0"),           std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("inquisition"),       std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string(PNG_PATH),            std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string(WRM_PATH),            std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_bitmap_update);

    BOOST_CHECK_EQUAL(0,                                ini.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.debug.input);
    // log
    BOOST_CHECK_EQUAL(2,                                ini.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.client.tls_fallback_legacy);
    BOOST_CHECK_EQUAL(true,                             ini.client.clipboard);
    BOOST_CHECK_EQUAL(true,                             ini.client.device_redirection);

    BOOST_CHECK_EQUAL(std::string("OK"),                std::string(ini.translation.button_ok.c_str()));
    BOOST_CHECK_EQUAL(std::string("Cancel"),            std::string(ini.translation.button_cancel.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help"),              std::string(ini.translation.button_help.c_str()));
    BOOST_CHECK_EQUAL(std::string("Close"),             std::string(ini.translation.button_close.c_str()));
    BOOST_CHECK_EQUAL(std::string("Refused"),           std::string(ini.translation.button_refused.c_str()));
    BOOST_CHECK_EQUAL(std::string("login"),             std::string(ini.translation.login.c_str()));
    BOOST_CHECK_EQUAL(std::string("username"),          std::string(ini.translation.username.c_str()));
    BOOST_CHECK_EQUAL(std::string("password"),          std::string(ini.translation.password.c_str()));
    BOOST_CHECK_EQUAL(std::string("target"),            std::string(ini.translation.target.c_str()));
    BOOST_CHECK_EQUAL(std::string("diagnostic"),        std::string(ini.translation.diagnostic.c_str()));
    BOOST_CHECK_EQUAL(std::string("Connection closed"), std::string(ini.translation.connection_closed.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help message"),      std::string(ini.translation.help_message.c_str()));

    BOOST_CHECK_EQUAL(40000,                            ini.context.opt_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.context.opt_framerate);
    BOOST_CHECK_EQUAL(15,                               ini.context.opt_qscale);
    BOOST_CHECK_EQUAL(800,                              ini.context.opt_width);
    BOOST_CHECK_EQUAL(600,                              ini.context.opt_height);
    BOOST_CHECK_EQUAL(24,                               ini.context.opt_bpp);
}

BOOST_AUTO_TEST_CASE(TestConfig1)
{
    // test we can read a config file with a global section
    std::stringstream oss(
    "[globals]\n"
    "movie=yes\n"
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
    "alternate_shell=C:\\\\WINDOWS\\\\NOTEPAD.EXE\n"
    "shell_working_directory=C:\\\\WINDOWS\\\\\n"
    "enable_bitmap_update=true\n"
    "\n"
    "[client]\n"
    "ignore_logon_password=yes\n"
    "performance_flags_default=0x00000007\n"
    "performance_flags_force_present=0x1\n"
    "performance_flags_force_not_present=0x0\n"
    "tls_fallback_legacy=yes\n"
    "clipboard=no\n"
    "device_redirection=no\n"
    "[debug]\n"
    "log_type=file\n"
    "log_file_path=/var/log/redemption.log\n"
    "[translation]\n"
    "button_cancel=Annuler\n"
    "\n"
    );

    Inifile ini(oss);

    BOOST_CHECK_EQUAL(true,                             ini.video.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.video.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(true,                             ini.globals.movie);
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

    BOOST_CHECK_EQUAL(1,                                ini.video.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.video.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.video.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.video.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.video.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.video.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.video.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.video.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.video.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.video.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.video.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.video.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.video.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.video.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.video.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.video.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.video.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.video.replay_path));

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

    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_bitmap_update);

    BOOST_CHECK_EQUAL(0,                                ini.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.debug.input);
    // log
    BOOST_CHECK_EQUAL(3,                                ini.debug.log_type);
    BOOST_CHECK_EQUAL(std::string("/var/log/redemption.log"),
                                                        std::string(ini.debug.log_file_path));

    BOOST_CHECK_EQUAL(true,                             ini.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(7,                                ini.client.performance_flags_default);
    BOOST_CHECK_EQUAL(1,                                ini.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(true,                             ini.client.tls_fallback_legacy);
    BOOST_CHECK_EQUAL(false,                            ini.client.clipboard);
    BOOST_CHECK_EQUAL(false,                            ini.client.device_redirection);

    BOOST_CHECK_EQUAL(std::string("OK"),                std::string(ini.translation.button_ok.c_str()));
    BOOST_CHECK_EQUAL(std::string("Annuler"),           std::string(ini.translation.button_cancel.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help"),              std::string(ini.translation.button_help.c_str()));
    BOOST_CHECK_EQUAL(std::string("Close"),             std::string(ini.translation.button_close.c_str()));
    BOOST_CHECK_EQUAL(std::string("Refused"),           std::string(ini.translation.button_refused.c_str()));
    BOOST_CHECK_EQUAL(std::string("login"),             std::string(ini.translation.login.c_str()));
    BOOST_CHECK_EQUAL(std::string("username"),          std::string(ini.translation.username.c_str()));
    BOOST_CHECK_EQUAL(std::string("password"),          std::string(ini.translation.password.c_str()));
    BOOST_CHECK_EQUAL(std::string("target"),            std::string(ini.translation.target.c_str()));
    BOOST_CHECK_EQUAL(std::string("diagnostic"),        std::string(ini.translation.diagnostic.c_str()));
    BOOST_CHECK_EQUAL(std::string("Connection closed"), std::string(ini.translation.connection_closed.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help message"),      std::string(ini.translation.help_message.c_str()));

    BOOST_CHECK_EQUAL(40000,                            ini.context.opt_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.context.opt_framerate);
    BOOST_CHECK_EQUAL(15,                               ini.context.opt_qscale);
    BOOST_CHECK_EQUAL(800,                              ini.context.opt_width);
    BOOST_CHECK_EQUAL(600,                              ini.context.opt_height);
    BOOST_CHECK_EQUAL(24,                               ini.context.opt_bpp);
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
    "enable_bitmap_update=no\n"
    "[client]\n"
    "performance_flags_default=7\n"
    "performance_flags_force_present=1\n"
    "performance_flags_force_not_present=0\n"
    "[translation]\n"
    "connection_closed=Connexion\\ fermée\n"
    "\n"
    );

    Inifile ini(oss);

    BOOST_CHECK_EQUAL(true,                             ini.video.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.video.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(false,                            ini.globals.movie);
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

    BOOST_CHECK_EQUAL(1,                                ini.video.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.video.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.video.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.video.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.video.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.video.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.video.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.video.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.video.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.video.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.video.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.video.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.video.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.video.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.video.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.video.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.video.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.video.replay_path));

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

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_bitmap_update);

    BOOST_CHECK_EQUAL(0,                                ini.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.debug.input);
    // log
    BOOST_CHECK_EQUAL(2,                                ini.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(7,                                ini.client.performance_flags_default);
    BOOST_CHECK_EQUAL(1,                                ini.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.client.tls_fallback_legacy);
    BOOST_CHECK_EQUAL(true,                             ini.client.clipboard);
    BOOST_CHECK_EQUAL(true,                             ini.client.device_redirection);

    BOOST_CHECK_EQUAL(std::string("OK"),                std::string(ini.translation.button_ok.c_str()));
    BOOST_CHECK_EQUAL(std::string("Cancel"),            std::string(ini.translation.button_cancel.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help"),              std::string(ini.translation.button_help.c_str()));
    BOOST_CHECK_EQUAL(std::string("Close"),             std::string(ini.translation.button_close.c_str()));
    BOOST_CHECK_EQUAL(std::string("Refused"),           std::string(ini.translation.button_refused.c_str()));
    BOOST_CHECK_EQUAL(std::string("login"),             std::string(ini.translation.login.c_str()));
    BOOST_CHECK_EQUAL(std::string("username"),          std::string(ini.translation.username.c_str()));
    BOOST_CHECK_EQUAL(std::string("password"),          std::string(ini.translation.password.c_str()));
    BOOST_CHECK_EQUAL(std::string("target"),            std::string(ini.translation.target.c_str()));
    BOOST_CHECK_EQUAL(std::string("diagnostic"),        std::string(ini.translation.diagnostic.c_str()));
    BOOST_CHECK_EQUAL(std::string("Connexion fermée"),  std::string(ini.translation.connection_closed.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help message"),      std::string(ini.translation.help_message.c_str()));

    BOOST_CHECK_EQUAL(40000,                            ini.context.opt_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.context.opt_framerate);
    BOOST_CHECK_EQUAL(15,                               ini.context.opt_qscale);
    BOOST_CHECK_EQUAL(800,                              ini.context.opt_width);
    BOOST_CHECK_EQUAL(600,                              ini.context.opt_height);
    BOOST_CHECK_EQUAL(24,                               ini.context.opt_bpp);
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
    "alternate_shell=C:\\\\Program\\ Files\\\\Microsoft\\ Visual\\ Studio\\\\Common\\\\MSDev98\\\\Bin\\\\MSDEV.EXE\n"
    "shell_working_directory=\n"
    "[client]\n"
    "performance_flags_default=07\n"
    "performance_flags_force_present=1\n"
    "performance_flags_force_not_present=0x\n"
    "\n"
    );

    Inifile ini(oss);

    BOOST_CHECK_EQUAL(true,                             ini.video.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.video.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(false,                            ini.globals.movie);
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

    BOOST_CHECK_EQUAL(1,                                ini.video.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.video.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.video.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.video.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.video.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.video.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.video.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.video.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.video.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.video.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.video.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.video.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.video.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.video.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.video.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.video.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.video.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.video.replay_path));

    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("rdpproxy"),          std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string("/var/tmp/wab/recorded/rdp"),
                                                        std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string("/var/wab/recorded/rdp"),
                                                        std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(
        std::string("C:\\Program Files\\Microsoft Visual Studio\\Common\\MSDev98\\Bin\\MSDEV.EXE"),
                                                        std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_bitmap_update);

    BOOST_CHECK_EQUAL(0,                                ini.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.debug.input);
    // log
    BOOST_CHECK_EQUAL(2,                                ini.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(7,                                ini.client.performance_flags_default);
    BOOST_CHECK_EQUAL(1,                                ini.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.client.tls_fallback_legacy);
    BOOST_CHECK_EQUAL(true,                             ini.client.clipboard);
    BOOST_CHECK_EQUAL(true,                             ini.client.device_redirection);

    BOOST_CHECK_EQUAL(std::string("OK"),                std::string(ini.translation.button_ok.c_str()));
    BOOST_CHECK_EQUAL(std::string("Cancel"),            std::string(ini.translation.button_cancel.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help"),              std::string(ini.translation.button_help.c_str()));
    BOOST_CHECK_EQUAL(std::string("Close"),             std::string(ini.translation.button_close.c_str()));
    BOOST_CHECK_EQUAL(std::string("Refused"),           std::string(ini.translation.button_refused.c_str()));
    BOOST_CHECK_EQUAL(std::string("login"),             std::string(ini.translation.login.c_str()));
    BOOST_CHECK_EQUAL(std::string("username"),          std::string(ini.translation.username.c_str()));
    BOOST_CHECK_EQUAL(std::string("password"),          std::string(ini.translation.password.c_str()));
    BOOST_CHECK_EQUAL(std::string("target"),            std::string(ini.translation.target.c_str()));
    BOOST_CHECK_EQUAL(std::string("diagnostic"),        std::string(ini.translation.diagnostic.c_str()));
    BOOST_CHECK_EQUAL(std::string("Connection closed"), std::string(ini.translation.connection_closed.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help message"),      std::string(ini.translation.help_message.c_str()));

    BOOST_CHECK_EQUAL(40000,                            ini.context.opt_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.context.opt_framerate);
    BOOST_CHECK_EQUAL(15,                               ini.context.opt_qscale);
    BOOST_CHECK_EQUAL(800,                              ini.context.opt_width);
    BOOST_CHECK_EQUAL(600,                              ini.context.opt_height);
    BOOST_CHECK_EQUAL(24,                               ini.context.opt_bpp);
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

    BOOST_CHECK_EQUAL(true,                             ini.video.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.video.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(false,                            ini.globals.movie);
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

    BOOST_CHECK_EQUAL(1,                                ini.video.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.video.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.video.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.video.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.video.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.video.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.video.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.video.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.video.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.video.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.video.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.video.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.video.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.video.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.video.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.video.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.video.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.video.replay_path));

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

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_bitmap_update);

    BOOST_CHECK_EQUAL(0,                                ini.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.debug.input);
    // log
    BOOST_CHECK_EQUAL(2,                                ini.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.client.tls_fallback_legacy);
    BOOST_CHECK_EQUAL(true,                             ini.client.clipboard);
    BOOST_CHECK_EQUAL(true,                             ini.client.device_redirection);

    BOOST_CHECK_EQUAL(std::string("OK"),                std::string(ini.translation.button_ok.c_str()));
    BOOST_CHECK_EQUAL(std::string("Cancel"),            std::string(ini.translation.button_cancel.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help"),              std::string(ini.translation.button_help.c_str()));
    BOOST_CHECK_EQUAL(std::string("Close"),             std::string(ini.translation.button_close.c_str()));
    BOOST_CHECK_EQUAL(std::string("Refused"),           std::string(ini.translation.button_refused.c_str()));
    BOOST_CHECK_EQUAL(std::string("login"),             std::string(ini.translation.login.c_str()));
    BOOST_CHECK_EQUAL(std::string("username"),          std::string(ini.translation.username.c_str()));
    BOOST_CHECK_EQUAL(std::string("password"),          std::string(ini.translation.password.c_str()));
    BOOST_CHECK_EQUAL(std::string("target"),            std::string(ini.translation.target.c_str()));
    BOOST_CHECK_EQUAL(std::string("diagnostic"),        std::string(ini.translation.diagnostic.c_str()));
    BOOST_CHECK_EQUAL(std::string("Connection closed"), std::string(ini.translation.connection_closed.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help message"),      std::string(ini.translation.help_message.c_str()));

    BOOST_CHECK_EQUAL(40000,                            ini.context.opt_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.context.opt_framerate);
    BOOST_CHECK_EQUAL(15,                               ini.context.opt_qscale);
    BOOST_CHECK_EQUAL(800,                              ini.context.opt_width);
    BOOST_CHECK_EQUAL(600,                              ini.context.opt_height);
    BOOST_CHECK_EQUAL(24,                               ini.context.opt_bpp);


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

    BOOST_CHECK_EQUAL(true,                             ini.video.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.video.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(false,                            ini.globals.movie);
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

    BOOST_CHECK_EQUAL(1,                                ini.video.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.video.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.video.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.video.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.video.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.video.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.video.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.video.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.video.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.video.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.video.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.video.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.video.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.video.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.video.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.video.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.video.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.video.replay_path));

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

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_bitmap_update);

    BOOST_CHECK_EQUAL(0,                                ini.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.debug.input);
    // log
    BOOST_CHECK_EQUAL(4,                                ini.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.client.tls_fallback_legacy);
    BOOST_CHECK_EQUAL(true,                             ini.client.clipboard);
    BOOST_CHECK_EQUAL(true,                             ini.client.device_redirection);

    BOOST_CHECK_EQUAL(std::string("OK"),                std::string(ini.translation.button_ok.c_str()));
    BOOST_CHECK_EQUAL(std::string("Cancel"),            std::string(ini.translation.button_cancel.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help"),              std::string(ini.translation.button_help.c_str()));
    BOOST_CHECK_EQUAL(std::string("Close"),             std::string(ini.translation.button_close.c_str()));
    BOOST_CHECK_EQUAL(std::string("Refused"),           std::string(ini.translation.button_refused.c_str()));
    BOOST_CHECK_EQUAL(std::string("login"),             std::string(ini.translation.login.c_str()));
    BOOST_CHECK_EQUAL(std::string("username"),          std::string(ini.translation.username.c_str()));
    BOOST_CHECK_EQUAL(std::string("password"),          std::string(ini.translation.password.c_str()));
    BOOST_CHECK_EQUAL(std::string("target"),            std::string(ini.translation.target.c_str()));
    BOOST_CHECK_EQUAL(std::string("diagnostic"),        std::string(ini.translation.diagnostic.c_str()));
    BOOST_CHECK_EQUAL(std::string("Connection closed"), std::string(ini.translation.connection_closed.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help message"),      std::string(ini.translation.help_message.c_str()));

    BOOST_CHECK_EQUAL(40000,                            ini.context.opt_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.context.opt_framerate);
    BOOST_CHECK_EQUAL(15,                               ini.context.opt_qscale);
    BOOST_CHECK_EQUAL(800,                              ini.context.opt_width);
    BOOST_CHECK_EQUAL(600,                              ini.context.opt_height);
    BOOST_CHECK_EQUAL(24,                               ini.context.opt_bpp);
}

BOOST_AUTO_TEST_CASE(TestNewConf)
{
    // new behavior:
    // init() load default values from main configuration file
    // - options with multiple occurences get the last value
    // - unrecognized lines are ignored
    // - every characters following # are ignored until end of line (comments)
    Inifile ini;

    BOOST_CHECK_EQUAL(true,                             ini.video.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.video.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(false,                            ini.globals.movie);
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

    BOOST_CHECK_EQUAL(1,                                ini.video.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.video.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.video.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.video.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.video.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.video.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.video.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.video.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.video.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.video.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.video.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.video.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.video.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.video.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.video.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.video.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.video.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.video.replay_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("0.0.0.0"),           std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("inquisition"),       std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string(PNG_PATH),            std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string(WRM_PATH),            std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_bitmap_update);

    BOOST_CHECK_EQUAL(0,                                ini.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.debug.input);
    // log
    BOOST_CHECK_EQUAL(2,                                ini.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.client.tls_fallback_legacy);
    BOOST_CHECK_EQUAL(true,                             ini.client.clipboard);
    BOOST_CHECK_EQUAL(true,                             ini.client.device_redirection);

    BOOST_CHECK_EQUAL(std::string("OK"),                std::string(ini.translation.button_ok.c_str()));
    BOOST_CHECK_EQUAL(std::string("Cancel"),            std::string(ini.translation.button_cancel.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help"),              std::string(ini.translation.button_help.c_str()));
    BOOST_CHECK_EQUAL(std::string("Close"),             std::string(ini.translation.button_close.c_str()));
    BOOST_CHECK_EQUAL(std::string("Refused"),           std::string(ini.translation.button_refused.c_str()));
    BOOST_CHECK_EQUAL(std::string("login"),             std::string(ini.translation.login.c_str()));
    BOOST_CHECK_EQUAL(std::string("username"),          std::string(ini.translation.username.c_str()));
    BOOST_CHECK_EQUAL(std::string("password"),          std::string(ini.translation.password.c_str()));
    BOOST_CHECK_EQUAL(std::string("target"),            std::string(ini.translation.target.c_str()));
    BOOST_CHECK_EQUAL(std::string("diagnostic"),        std::string(ini.translation.diagnostic.c_str()));
    BOOST_CHECK_EQUAL(std::string("Connection closed"), std::string(ini.translation.connection_closed.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help message"),      std::string(ini.translation.help_message.c_str()));

    BOOST_CHECK_EQUAL(40000,                            ini.context.opt_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.context.opt_framerate);
    BOOST_CHECK_EQUAL(15,                               ini.context.opt_qscale);
    BOOST_CHECK_EQUAL(800,                              ini.context.opt_width);
    BOOST_CHECK_EQUAL(600,                              ini.context.opt_height);
    BOOST_CHECK_EQUAL(24,                               ini.context.opt_bpp);


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

    BOOST_CHECK_EQUAL(true,                             ini.video.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.video.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(false,                            ini.globals.movie);
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

    BOOST_CHECK_EQUAL(1,                                ini.video.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.video.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.video.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.video.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.video.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.video.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.video.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.video.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.video.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.video.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.video.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.video.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.video.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.video.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.video.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.video.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.video.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.video.replay_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("0.0.0.0"),           std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("inquisition"),       std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string(PNG_PATH),            std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string(WRM_PATH),            std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_bitmap_update);

    BOOST_CHECK_EQUAL(0,                                ini.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.debug.input);

    BOOST_CHECK_EQUAL(2,                                ini.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.client.tls_fallback_legacy);
    BOOST_CHECK_EQUAL(true,                             ini.client.clipboard);
    BOOST_CHECK_EQUAL(true,                             ini.client.device_redirection);

    BOOST_CHECK_EQUAL(std::string("OK"),                std::string(ini.translation.button_ok.c_str()));
    BOOST_CHECK_EQUAL(std::string("Cancel"),            std::string(ini.translation.button_cancel.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help"),              std::string(ini.translation.button_help.c_str()));
    BOOST_CHECK_EQUAL(std::string("Close"),             std::string(ini.translation.button_close.c_str()));
    BOOST_CHECK_EQUAL(std::string("Refused"),           std::string(ini.translation.button_refused.c_str()));
    BOOST_CHECK_EQUAL(std::string("login"),             std::string(ini.translation.login.c_str()));
    BOOST_CHECK_EQUAL(std::string("username"),          std::string(ini.translation.username.c_str()));
    BOOST_CHECK_EQUAL(std::string("password"),          std::string(ini.translation.password.c_str()));
    BOOST_CHECK_EQUAL(std::string("target"),            std::string(ini.translation.target.c_str()));
    BOOST_CHECK_EQUAL(std::string("diagnostic"),        std::string(ini.translation.diagnostic.c_str()));
    BOOST_CHECK_EQUAL(std::string("Connection closed"), std::string(ini.translation.connection_closed.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help message"),      std::string(ini.translation.help_message.c_str()));

    BOOST_CHECK_EQUAL(40000,                            ini.context.opt_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.context.opt_framerate);
    BOOST_CHECK_EQUAL(15,                               ini.context.opt_qscale);
    BOOST_CHECK_EQUAL(800,                              ini.context.opt_width);
    BOOST_CHECK_EQUAL(600,                              ini.context.opt_height);
    BOOST_CHECK_EQUAL(24,                               ini.context.opt_bpp);


    // back to default values
    ini.init();

    BOOST_CHECK_EQUAL(true,                             ini.video.capture_png);
    BOOST_CHECK_EQUAL(true,                             ini.video.capture_wrm);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_flv);
    BOOST_CHECK_EQUAL(false,                            ini.video.capture_ocr);
    BOOST_CHECK_EQUAL(false,                            ini.globals.capture_chunk);
    BOOST_CHECK_EQUAL(false,                            ini.globals.movie);
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

    BOOST_CHECK_EQUAL(1,                                ini.video.capture_flags);
    BOOST_CHECK_EQUAL(3000,                             ini.video.png_interval);
    BOOST_CHECK_EQUAL(40,                               ini.video.frame_interval);
    BOOST_CHECK_EQUAL(600,                              ini.video.break_interval);
    BOOST_CHECK_EQUAL(600000000l,                       ini.globals.flv_break_interval);
    BOOST_CHECK_EQUAL(1000000L,                         ini.globals.flv_frame_interval);
    BOOST_CHECK_EQUAL(100,                              ini.video.ocr_interval);

    BOOST_CHECK_EQUAL(3,                                ini.video.png_limit);

    BOOST_CHECK_EQUAL(20000,                            ini.video.l_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.l_framerate);
    BOOST_CHECK_EQUAL(480,                              ini.video.l_height);
    BOOST_CHECK_EQUAL(640,                              ini.video.l_width);
    BOOST_CHECK_EQUAL(25,                               ini.video.l_qscale);

    BOOST_CHECK_EQUAL(40000,                            ini.video.m_bitrate);
    BOOST_CHECK_EQUAL(1,                                ini.video.m_framerate);
    BOOST_CHECK_EQUAL(768,                              ini.video.m_height);
    BOOST_CHECK_EQUAL(1024,                             ini.video.m_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.m_qscale);

    BOOST_CHECK_EQUAL(200000,                           ini.video.h_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.video.h_framerate);
    BOOST_CHECK_EQUAL(1024,                             ini.video.h_height);
    BOOST_CHECK_EQUAL(1280,                             ini.video.h_width);
    BOOST_CHECK_EQUAL(15,                               ini.video.h_qscale);

    BOOST_CHECK_EQUAL(30,                               ini.globals.max_tick);
    BOOST_CHECK_EQUAL(30,                               ini.globals.keepalive_grace_delay);

    BOOST_CHECK_EQUAL(false,                            ini.globals.internal_domain);
    BOOST_CHECK_EQUAL(std::string("/tmp/"),             std::string(ini.video.replay_path));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_file_encryption);
    BOOST_CHECK_EQUAL(true,                             ini.globals.enable_tls);
    BOOST_CHECK_EQUAL(std::string("0.0.0.0"),           std::string(ini.globals.listen_address));
    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_ip_transparent);
    BOOST_CHECK_EQUAL(std::string("inquisition"),       std::string(ini.globals.certificate_password));

    BOOST_CHECK_EQUAL(std::string(PNG_PATH),            std::string(ini.globals.png_path));
    BOOST_CHECK_EQUAL(std::string(WRM_PATH),            std::string(ini.globals.wrm_path));

    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.alternate_shell));
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.globals.shell_working_directory));

    BOOST_CHECK_EQUAL(false,                            ini.globals.enable_bitmap_update);

    BOOST_CHECK_EQUAL(0,                                ini.debug.x224);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mcs);
    BOOST_CHECK_EQUAL(0,                                ini.debug.sec);
    BOOST_CHECK_EQUAL(0,                                ini.debug.rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.primary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.secondary_orders);
    BOOST_CHECK_EQUAL(0,                                ini.debug.bitmap);
    BOOST_CHECK_EQUAL(0,                                ini.debug.capture);
    BOOST_CHECK_EQUAL(0,                                ini.debug.auth);
    BOOST_CHECK_EQUAL(0,                                ini.debug.session);
    BOOST_CHECK_EQUAL(0,                                ini.debug.front);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_rdp);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_vnc);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_int);
    BOOST_CHECK_EQUAL(0,                                ini.debug.mod_xup);
    BOOST_CHECK_EQUAL(0,                                ini.debug.widget);
    BOOST_CHECK_EQUAL(0,                                ini.debug.input);
    // log
    BOOST_CHECK_EQUAL(2,                                ini.debug.log_type);
    BOOST_CHECK_EQUAL(std::string(""),                  std::string(ini.debug.log_file_path));

    BOOST_CHECK_EQUAL(false,                            ini.client.ignore_logon_password);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_default);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_present);
    BOOST_CHECK_EQUAL(0,                                ini.client.performance_flags_force_not_present);
    BOOST_CHECK_EQUAL(false,                            ini.client.tls_fallback_legacy);
    BOOST_CHECK_EQUAL(true,                             ini.client.clipboard);
    BOOST_CHECK_EQUAL(true,                             ini.client.device_redirection);

    BOOST_CHECK_EQUAL(std::string("OK"),                std::string(ini.translation.button_ok.c_str()));
    BOOST_CHECK_EQUAL(std::string("Cancel"),            std::string(ini.translation.button_cancel.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help"),              std::string(ini.translation.button_help.c_str()));
    BOOST_CHECK_EQUAL(std::string("Close"),             std::string(ini.translation.button_close.c_str()));
    BOOST_CHECK_EQUAL(std::string("Refused"),           std::string(ini.translation.button_refused.c_str()));
    BOOST_CHECK_EQUAL(std::string("login"),             std::string(ini.translation.login.c_str()));
    BOOST_CHECK_EQUAL(std::string("username"),          std::string(ini.translation.username.c_str()));
    BOOST_CHECK_EQUAL(std::string("password"),          std::string(ini.translation.password.c_str()));
    BOOST_CHECK_EQUAL(std::string("target"),            std::string(ini.translation.target.c_str()));
    BOOST_CHECK_EQUAL(std::string("diagnostic"),        std::string(ini.translation.diagnostic.c_str()));
    BOOST_CHECK_EQUAL(std::string("Connection closed"), std::string(ini.translation.connection_closed.c_str()));
    BOOST_CHECK_EQUAL(std::string("Help message"),      std::string(ini.translation.help_message.c_str()));

    BOOST_CHECK_EQUAL(40000,                            ini.context.opt_bitrate);
    BOOST_CHECK_EQUAL(5,                                ini.context.opt_framerate);
    BOOST_CHECK_EQUAL(15,                               ini.context.opt_qscale);
    BOOST_CHECK_EQUAL(800,                              ini.context.opt_width);
    BOOST_CHECK_EQUAL(600,                              ini.context.opt_height);
    BOOST_CHECK_EQUAL(24,                               ini.context.opt_bpp);
}

BOOST_AUTO_TEST_CASE(TestConfigTools)
{
    BOOST_CHECK_EQUAL(0,        ulong_from_cstr("0"));
    BOOST_CHECK_EQUAL(0,        ulong_from_cstr("0x"));

    BOOST_CHECK_EQUAL(3,        ulong_from_cstr("3"));
    BOOST_CHECK_EQUAL(3,        ulong_from_cstr("0x3"));

    BOOST_CHECK_EQUAL(7,        ulong_from_cstr("0x00000007"));
    BOOST_CHECK_EQUAL(7,        ulong_from_cstr("0x0000000000000007"));
    BOOST_CHECK_EQUAL(7,        ulong_from_cstr("0x0007    "));

    BOOST_CHECK_EQUAL(1357,     ulong_from_cstr("1357"));
    BOOST_CHECK_EQUAL(4951,     ulong_from_cstr("0x1357"));

    BOOST_CHECK_EQUAL(10,       ulong_from_cstr("0x0A"));
    BOOST_CHECK_EQUAL(10,       ulong_from_cstr("0x0a"));

    BOOST_CHECK_EQUAL(0,        ulong_from_cstr("0x0000000I"));
    BOOST_CHECK_EQUAL(0,        ulong_from_cstr("I"));

    BOOST_CHECK_EQUAL(0,        level_from_cstr("LoW"));
    BOOST_CHECK_EQUAL(1,        level_from_cstr("mEdIuM"));
    BOOST_CHECK_EQUAL(2,        level_from_cstr("High"));

    BOOST_CHECK_EQUAL(0,        logtype_from_cstr(""));
    BOOST_CHECK_EQUAL(0,        logtype_from_cstr("null"));
    BOOST_CHECK_EQUAL(0,        logtype_from_cstr("NULL"));
    BOOST_CHECK_EQUAL(0,        logtype_from_cstr("unknown"));
    BOOST_CHECK_EQUAL(1,        logtype_from_cstr("print"));
    BOOST_CHECK_EQUAL(1,        logtype_from_cstr("Print"));
    BOOST_CHECK_EQUAL(2,        logtype_from_cstr("syslog"));
    BOOST_CHECK_EQUAL(2,        logtype_from_cstr("SYSLOG"));
    BOOST_CHECK_EQUAL(3,        logtype_from_cstr("file"));
    BOOST_CHECK_EQUAL(3,        logtype_from_cstr("FiLe"));
    BOOST_CHECK_EQUAL(4,        logtype_from_cstr("encryptedfile"));
    BOOST_CHECK_EQUAL(4,        logtype_from_cstr("EncryptedFile"));

    BOOST_CHECK_EQUAL(3600,     _long_from_cstr("3600"));
    BOOST_CHECK_EQUAL(0,        _long_from_cstr("0"));
    BOOST_CHECK_EQUAL(0,        _long_from_cstr(""));
    BOOST_CHECK_EQUAL(-3600,    _long_from_cstr("-3600"));
}

BOOST_AUTO_TEST_CASE(TestContextSetValue)
{
    Inifile ini;
    char    buffer[128];

    // Translation
    ini.context_set_value(AUTHID_TRANS_BUTTON_OK,       "Ok");
    ini.context_set_value(AUTHID_TRANS_BUTTON_CANCEL,   "Annuler");
    ini.context_set_value(AUTHID_TRANS_BUTTON_HELP,     "Aide");
    ini.context_set_value(AUTHID_TRANS_BUTTON_CLOSE,    "Fermer");
    ini.context_set_value(AUTHID_TRANS_BUTTON_REFUSED,  "Refusé");
    ini.context_set_value(AUTHID_TRANS_LOGIN,           "Compte");
    ini.context_set_value(AUTHID_TRANS_USERNAME,        "Nom de l'utilisateur");
    ini.context_set_value(AUTHID_TRANS_PASSWORD,        "Mot de passe");
    ini.context_set_value(AUTHID_TRANS_TARGET,          "Cible");
    ini.context_set_value(AUTHID_TRANS_DIAGNOSTIC,      "diagnostique");
    ini.context_set_value(AUTHID_TRANS_CONNECTION_CLOSED,
                                                        "Connexion fermée");
    ini.context_set_value(AUTHID_TRANS_HELP_MESSAGE,    "Message d'aide");

    BOOST_CHECK_EQUAL(std::string("Ok"),                std::string(ini.context_get_value(AUTHID_TRANS_BUTTON_OK,         buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Annuler"),           std::string(ini.context_get_value(AUTHID_TRANS_BUTTON_CANCEL,     buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Aide"),              std::string(ini.context_get_value(AUTHID_TRANS_BUTTON_HELP,       buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Fermer"),            std::string(ini.context_get_value(AUTHID_TRANS_BUTTON_CLOSE,      buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Refusé"),            std::string(ini.context_get_value(AUTHID_TRANS_BUTTON_REFUSED,    buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Compte"),            std::string(ini.context_get_value(AUTHID_TRANS_LOGIN,             buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Nom de l'utilisateur"),
                                                        std::string(ini.context_get_value(AUTHID_TRANS_USERNAME,          buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Mot de passe"),      std::string(ini.context_get_value(AUTHID_TRANS_PASSWORD,          buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Cible"),             std::string(ini.context_get_value(AUTHID_TRANS_TARGET,            buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("diagnostique"),      std::string(ini.context_get_value(AUTHID_TRANS_DIAGNOSTIC,        buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Connexion fermée"),  std::string(ini.context_get_value(AUTHID_TRANS_CONNECTION_CLOSED, buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Message d'aide"),    std::string(ini.context_get_value(AUTHID_TRANS_HELP_MESSAGE,      buffer, sizeof(buffer))));


    // bitrate, framerate, qscale
    ini.context_set_value(AUTHID_OPT_BITRATE,           "80000");
    ini.context_set_value(AUTHID_OPT_FRAMERATE,         "6");
    ini.context_set_value(AUTHID_OPT_QSCALE,            "16");

    BOOST_CHECK_EQUAL(80000,                            ini.context.opt_bitrate);
    BOOST_CHECK_EQUAL(6,                                ini.context.opt_framerate);
    BOOST_CHECK_EQUAL(16,                               ini.context.opt_qscale);

    BOOST_CHECK_EQUAL(std::string("80000"),             std::string(ini.context_get_value(AUTHID_OPT_BITRATE,   buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("6"),                 std::string(ini.context_get_value(AUTHID_OPT_FRAMERATE, buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("16"),                std::string(ini.context_get_value(AUTHID_OPT_QSCALE,    buffer, sizeof(buffer))));


    // bpp, height, width
    ini.context_ask(AUTHID_OPT_BPP);
    ini.context_ask(AUTHID_OPT_HEIGHT);
    ini.context_ask(AUTHID_OPT_WIDTH);

    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_OPT_BPP));
    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_OPT_HEIGHT));
    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_OPT_WIDTH));

    ini.context_set_value(AUTHID_OPT_BPP,               "16");
    ini.context_set_value(AUTHID_OPT_HEIGHT,            "1024");
    ini.context_set_value(AUTHID_OPT_WIDTH,             "1280");

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_OPT_BPP));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_OPT_HEIGHT));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_OPT_WIDTH));

    BOOST_CHECK_EQUAL(1280,                             ini.context.opt_width);
    BOOST_CHECK_EQUAL(1024,                             ini.context.opt_height);
    BOOST_CHECK_EQUAL(16,                               ini.context.opt_bpp);

    BOOST_CHECK_EQUAL(std::string("1280"),              std::string(ini.context_get_value(AUTHID_OPT_WIDTH,  buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("1024"),              std::string(ini.context_get_value(AUTHID_OPT_HEIGHT, buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("16"),                std::string(ini.context_get_value(AUTHID_OPT_BPP,    buffer, sizeof(buffer))));


    ini.context_set_value(AUTHID_AUTH_ERROR_MESSAGE,    "Message d'erreur.");

    BOOST_CHECK_EQUAL(std::string("Message d'erreur."), std::string(ini.context.auth_error_message.c_str()));

    BOOST_CHECK_EQUAL(std::string("Message d'erreur."), std::string(ini.context_get_value(AUTHID_AUTH_ERROR_MESSAGE, buffer, sizeof(buffer))));


    // selector, ...
    ini.context_ask(AUTHID_SELECTOR);
    ini.context_ask(AUTHID_SELECTOR_CURRENT_PAGE);
    ini.context_ask(AUTHID_SELECTOR_DEVICE_FILTER);
    ini.context_ask(AUTHID_SELECTOR_GROUP_FILTER);
    ini.context_ask(AUTHID_SELECTOR_LINES_PER_PAGE);

    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_SELECTOR));
    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_SELECTOR_CURRENT_PAGE));
    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_SELECTOR_DEVICE_FILTER));
    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_SELECTOR_GROUP_FILTER));
    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_SELECTOR_LINES_PER_PAGE));

    ini.context_set_value(AUTHID_SELECTOR,                  "True");
    ini.context_set_value(AUTHID_SELECTOR_CURRENT_PAGE,     "2");
    ini.context_set_value(AUTHID_SELECTOR_DEVICE_FILTER,    "Windows");
    ini.context_set_value(AUTHID_SELECTOR_GROUP_FILTER,     "RDP");
    ini.context_set_value(AUTHID_SELECTOR_LINES_PER_PAGE,   "25");
    ini.context_set_value(AUTHID_SELECTOR_NUMBER_OF_PAGES,  "2");

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_SELECTOR));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_SELECTOR_CURRENT_PAGE));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_SELECTOR_DEVICE_FILTER));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_SELECTOR_GROUP_FILTER));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_SELECTOR_LINES_PER_PAGE));

    BOOST_CHECK_EQUAL(true,                             ini.context.selector);
    BOOST_CHECK_EQUAL(2,                                ini.context.selector_current_page);
    BOOST_CHECK_EQUAL(std::string("Windows"),           std::string(ini.context.selector_device_filter.c_str()));
    BOOST_CHECK_EQUAL(std::string("RDP"),               std::string(ini.context.selector_group_filter.c_str()));
    BOOST_CHECK_EQUAL(25,                               ini.context.selector_lines_per_page);
    BOOST_CHECK_EQUAL(2,                                ini.context.selector_number_of_pages);

    BOOST_CHECK_EQUAL(std::string("True"),              std::string(ini.context_get_value(AUTHID_SELECTOR,                 buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("2"),                 std::string(ini.context_get_value(AUTHID_SELECTOR_CURRENT_PAGE,    buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("Windows"),           std::string(ini.context_get_value(AUTHID_SELECTOR_DEVICE_FILTER,   buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("RDP"),               std::string(ini.context_get_value(AUTHID_SELECTOR_GROUP_FILTER,    buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("25"),                std::string(ini.context_get_value(AUTHID_SELECTOR_LINES_PER_PAGE,  buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("2"),                 std::string(ini.context_get_value(AUTHID_SELECTOR_NUMBER_OF_PAGES, buffer, sizeof(buffer))));

    BOOST_CHECK_EQUAL(true,                             ini.context_get_bool(AUTHID_SELECTOR));


    // target_xxxx
    ini.context_set_value(AUTHID_TARGET_DEVICE,         "127.0.0.1");
    ini.context_set_value(AUTHID_TARGET_PASSWORD,       "12345678");
    ini.context_set_value(AUTHID_TARGET_PORT,           "3390");
    ini.context_set_value(AUTHID_TARGET_PROTOCOL,       "RDP");
    ini.context_set_value(AUTHID_TARGET_USER,           "admin");

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_TARGET_DEVICE));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_TARGET_PASSWORD));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_TARGET_PORT));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_TARGET_PROTOCOL));
    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_TARGET_USER));

    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.globals.target_device));
    BOOST_CHECK_EQUAL(std::string("12345678"),          std::string(ini.context.target_password.c_str()));
    BOOST_CHECK_EQUAL(3390,                             ini.context.target_port);
    BOOST_CHECK_EQUAL(std::string("RDP"),               std::string(ini.context.target_protocol.c_str()));
    BOOST_CHECK_EQUAL(std::string("admin"),             std::string(ini.globals.target_user));

    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.context_get_value(AUTHID_TARGET_DEVICE,   buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("12345678"),          std::string(ini.context_get_value(AUTHID_TARGET_PASSWORD, buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("3390"),              std::string(ini.context_get_value(AUTHID_TARGET_PORT,     buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("RDP"),               std::string(ini.context_get_value(AUTHID_TARGET_PROTOCOL, buffer, sizeof(buffer))));
    BOOST_CHECK_EQUAL(std::string("admin"),             std::string(ini.context_get_value(AUTHID_TARGET_USER,     buffer, sizeof(buffer))));


    // host
    ini.context_ask(AUTHID_HOST);

    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_HOST));

    ini.context_set_value(AUTHID_HOST,                  "127.0.0.1");

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_HOST));

    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.globals.host));

    BOOST_CHECK_EQUAL(std::string("127.0.0.1"),         std::string(ini.context_get_value(AUTHID_HOST, buffer, sizeof(buffer))));


    // target
    ini.context_ask(AUTHID_TARGET);

    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_TARGET));

    ini.context_set_value(AUTHID_TARGET,                "192.168.0.1");

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_TARGET));

    BOOST_CHECK_EQUAL(std::string("192.168.0.1"),       std::string(ini.globals.target));

    BOOST_CHECK_EQUAL(std::string("192.168.0.1"),       std::string(ini.context_get_value(AUTHID_TARGET, buffer, sizeof(buffer))));


    // auth_user
    ini.context_set_value(AUTHID_AUTH_USER,             "admin");

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_AUTH_USER));

    BOOST_CHECK_EQUAL(std::string("admin"),             std::string(ini.globals.auth_user));

    BOOST_CHECK_EQUAL(std::string("admin"),             std::string(ini.context_get_value(AUTHID_AUTH_USER, buffer, sizeof(buffer))));


    // password
    ini.context_ask(AUTHID_PASSWORD);

    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_PASSWORD));

    ini.context_set_value(AUTHID_PASSWORD,              "12345678");

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_PASSWORD));

    BOOST_CHECK_EQUAL(std::string("12345678"),          std::string(ini.context.password.c_str()));

    BOOST_CHECK_EQUAL(std::string("12345678"),          std::string(ini.context_get_value(AUTHID_PASSWORD, buffer, sizeof(buffer))));


    // answer
    ini.context_set_value(AUTHID_AUTHCHANNEL_ANSWER,    "answer");

    BOOST_CHECK_EQUAL(std::string("answer"),            std::string(ini.context.authchannel_answer.c_str()));

    BOOST_CHECK_EQUAL(std::string("answer"),            std::string(ini.context_get_value(AUTHID_AUTHCHANNEL_ANSWER, buffer, sizeof(buffer))));


    // authchannel_target
    ini.context_ask(AUTHID_AUTHCHANNEL_TARGET);

    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_AUTHCHANNEL_TARGET));

    ini.context_set_value(AUTHID_AUTHCHANNEL_TARGET,    "target");

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_AUTHCHANNEL_TARGET));

    BOOST_CHECK_EQUAL(std::string("target"),            std::string(ini.context.authchannel_target.c_str()));

    BOOST_CHECK_EQUAL(std::string("target"),            std::string(ini.context_get_value(AUTHID_AUTHCHANNEL_TARGET, buffer, sizeof(buffer))));


    // authchannel_result
    ini.context_ask(AUTHID_AUTHCHANNEL_RESULT);

    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_AUTHCHANNEL_RESULT));

    ini.context_set_value(AUTHID_AUTHCHANNEL_RESULT,    "result");

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_AUTHCHANNEL_RESULT));

    BOOST_CHECK_EQUAL(std::string("result"),            std::string(ini.context.authchannel_result.c_str()));

    BOOST_CHECK_EQUAL(std::string("result"),            std::string(ini.context_get_value(AUTHID_AUTHCHANNEL_RESULT, buffer, sizeof(buffer))));


    // message
    ini.context_set_value(AUTHID_MESSAGE,               "message");

    BOOST_CHECK_EQUAL(std::string("message"),           std::string(ini.context.message.c_str()));

    BOOST_CHECK_EQUAL(std::string("message"),           std::string(ini.context_get_value(AUTHID_MESSAGE, buffer, sizeof(buffer))));


    // accept_message
    ini.context_ask(AUTHID_ACCEPT_MESSAGE);

    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_ACCEPT_MESSAGE));

    ini.context_set_value(AUTHID_ACCEPT_MESSAGE,        "accept_message");

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_ACCEPT_MESSAGE));

    BOOST_CHECK_EQUAL(std::string("accept_message"),    std::string(ini.context.accept_message.c_str()));

    BOOST_CHECK_EQUAL(std::string("accept_message"),    std::string(ini.context_get_value(AUTHID_ACCEPT_MESSAGE, buffer, sizeof(buffer))));


    // display_message
    ini.context_ask(AUTHID_DISPLAY_MESSAGE);

    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_DISPLAY_MESSAGE));

    ini.context_set_value(AUTHID_DISPLAY_MESSAGE,       "display_message");

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_DISPLAY_MESSAGE));

    BOOST_CHECK_EQUAL(std::string("display_message"),   std::string(ini.context.display_message.c_str()));

    BOOST_CHECK_EQUAL(std::string("display_message"),   std::string(ini.context_get_value(AUTHID_DISPLAY_MESSAGE, buffer, sizeof(buffer))));


    // rejected
    ini.context_set_value(AUTHID_REJECTED,              "rejected");

    BOOST_CHECK_EQUAL(std::string("rejected"),          std::string(ini.context.rejected.c_str()));

    BOOST_CHECK_EQUAL(std::string("rejected"),          std::string(ini.context_get_value(AUTHID_REJECTED, buffer, sizeof(buffer))));


    // authenticated
    ini.context_set_value(AUTHID_AUTHENTICATED,         "True");

    BOOST_CHECK_EQUAL(true,                             ini.context.authenticated);

    BOOST_CHECK_EQUAL(true,                             ini.context_get_bool(AUTHID_AUTHENTICATED));

    BOOST_CHECK_EQUAL(std::string("True"),              std::string(ini.context_get_value(AUTHID_AUTHENTICATED, buffer, sizeof(buffer))));


    // keepalive
    ini.context_set_value(AUTHID_KEEPALIVE,             "True");

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_KEEPALIVE));

    BOOST_CHECK_EQUAL(true,                             ini.context.keepalive);

    BOOST_CHECK_EQUAL(true,                             ini.context_get_bool(AUTHID_KEEPALIVE));

    BOOST_CHECK_EQUAL(std::string("True"),              std::string(ini.context_get_value(AUTHID_KEEPALIVE, buffer, sizeof(buffer))));


    // proxy_type
    ini.context_ask(AUTHID_PROXY_TYPE);

    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_PROXY_TYPE));

    ini.context_set_value(AUTHID_PROXY_TYPE,            "VNC");

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_PROXY_TYPE));

    BOOST_CHECK_EQUAL(std::string("VNC"),               std::string(ini.context.proxy_type.c_str()));

    BOOST_CHECK_EQUAL(std::string("VNC"),               std::string(ini.context_get_value(AUTHID_PROXY_TYPE, buffer, sizeof(buffer))));


    // trace_seal
    ini.context_ask(AUTHID_TRACE_SEAL);

    BOOST_CHECK_EQUAL(true,                             ini.context_is_asked(AUTHID_TRACE_SEAL));

    ini.context_set_value(AUTHID_TRACE_SEAL,            "trace_seal");

    BOOST_CHECK_EQUAL(false,                            ini.context_is_asked(AUTHID_TRACE_SEAL));

    BOOST_CHECK_EQUAL(std::string("trace_seal"),        std::string(ini.context.trace_seal.c_str()));

    BOOST_CHECK_EQUAL(std::string("trace_seal"),        std::string(ini.context_get_value(AUTHID_TRACE_SEAL, buffer, sizeof(buffer))));


    // session_id
    ini.context_set_value(AUTHID_SESSION_ID,            "0123456789");

    BOOST_CHECK_EQUAL(std::string("0123456789"),        std::string(ini.context.session_id.c_str()));

    BOOST_CHECK_EQUAL(std::string("0123456789"),        std::string(ini.context_get_value(AUTHID_SESSION_ID, buffer, sizeof(buffer))));


    // end_date_cnx
    ini.context_set_value(AUTHID_END_DATE_CNX,          "12345678");

    BOOST_CHECK_EQUAL(12345678,                         ini.context.end_date_cnx);

    BOOST_CHECK_EQUAL(std::string("12345678"),          std::string(ini.context_get_value(AUTHID_END_DATE_CNX, buffer, sizeof(buffer))));


    // end_time
    ini.context_set_value(AUTHID_END_TIME,              "end_time");

    BOOST_CHECK_EQUAL(std::string("end_time"),          std::string(ini.context.end_time.c_str()));

    BOOST_CHECK_EQUAL(std::string("end_time"),          std::string(ini.context_get_value(AUTHID_END_TIME, buffer, sizeof(buffer))));


    // mode_console
    ini.context_set_value(AUTHID_MODE_CONSOLE,          "deny");

    BOOST_CHECK_EQUAL(std::string("deny"),              std::string(ini.context.mode_console.c_str()));

    BOOST_CHECK_EQUAL(std::string("deny"),              std::string(ini.context_get_value(AUTHID_MODE_CONSOLE, buffer, sizeof(buffer))));


    // timezone
    ini.context_set_value(AUTHID_TIMEZONE,              "-7200");

    BOOST_CHECK_EQUAL(-7200,                            ini.context.timezone);

    BOOST_CHECK_EQUAL(std::string("-7200"),             std::string(ini.context_get_value(AUTHID_TIMEZONE, buffer, sizeof(buffer))));


    // real_target_device
    ini.context_set_value(AUTHID_REAL_TARGET_DEVICE,    "10.0.0.1");

    BOOST_CHECK_EQUAL(std::string("10.0.0.1"),          std::string(ini.context.real_target_device.c_str()));

    BOOST_CHECK_EQUAL(std::string("10.0.0.1"),          std::string(ini.context_get_value(AUTHID_REAL_TARGET_DEVICE, buffer, sizeof(buffer))));


    // authentication_challenge
    ini.context_set_value(AUTHID_AUTHENTICATION_CHALLENGE,     "authentication_challenge");

    BOOST_CHECK_EQUAL(std::string("authentication_challenge"), std::string(ini.context.authentication_challenge.c_str()));

    BOOST_CHECK_EQUAL(std::string("authentication_challenge"), std::string(ini.context_get_value(AUTHID_AUTHENTICATION_CHALLENGE, buffer, sizeof(buffer))));
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
