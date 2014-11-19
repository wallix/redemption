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
  Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan

  Unit test to config.cpp file
  Using lib boost functions, some tests need to be added
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestConfig
#include <boost/test/auto_unit_test.hpp>

#include "config.hpp"
#include "write_config_spec.hpp"

BOOST_AUTO_TEST_CASE(TestConfigWriteSpec)
{
    Inifile ini;
    std::ostringstream os;
    write_config_spec(os, ini);
    BOOST_CHECK_EQUAL(os.str(),
    "[globals]\n"
    "\tbitmap_cache = boolean(default=1)\n"
    "\tport = integer(min=0,max=4294967295, default=3389)\n"
    "\tnomouse = boolean(default=0)\n"
    "\tnotimestamp = boolean(default=0)\n"
    "\tencryptionLevel = option('low','medium','high', default='low')\n"
    "\tauthip = ip_addr(default='127.0.0.1')\n"
    "\tauthport = integer(min=0,max=4294967295, default=3350)\n"
    "\tautovalidate = boolean(default=0)\n"
    "\tmax_tick = integer(min=0,max=4294967295, default=30)\n"
    "\tkeepalive_grace_delay = integer(min=0,max=4294967295, default=30)\n"
    "\tclose_timeout = integer(min=0,max=4294967295, default=600)\n"
    "\tauth_channel = string(max=7, default='')\n"
    "\tenable_file_encryption = boolean(default=0)\n"
    "\tlisten_address = ip_addr(default='0.0.0.0')\n"
    "\tenable_ip_transparent = boolean(default=0)\n"
    "\tcertificate_password = string(max=255, default='inquisition')\n"
    "\tpng_path = string(max=1023, default='/var/rdpproxy/tmp')\n"
    "\twrm_path = string(max=1023, default='/var/rdpproxy/recorded')\n"
    "\talternate_shell = string(default='')\n"
    "\tshell_working_directory = string(default='')\n"
    "\tcodec_id = string(default='flv')\n"
    "\tmovie = boolean(default=0)\n"
    "\tmovie_path = string(default='')\n"
    "\tvideo_quality = option('low','medium','high', default='medium')\n"
    "\tenable_bitmap_update = boolean(default=0)\n"
    "\tenable_close_box = boolean(default=1)\n"
    "\tenable_osd = boolean(default=1)\n"
    "\tpersistent_path = string(max=1023, default='/var/lib/redemption/cache/')\n"
    "[client]\n"
    "\tignore_logon_password = boolean(default=0)\n"
    "\tperformance_flags_default = integer(min=0,max=4294967295, default=0)\n"
    "\tperformance_flags_force_present = integer(min=0,max=4294967295, default=0)\n"
    "\tperformance_flags_force_not_present = integer(min=0,max=4294967295, default=0)\n"
    "\ttls_fallback_legacy = boolean(default=0)\n"
    "\ttls_support = boolean(default=1)\n"
    "\tbogus_neg_request = boolean(default=0)\n"
    "\tdevice_redirection = boolean(default=1)\n"
    "\tdisable_tsk_switch_shortcuts = boolean(default=0)\n"
    "\trdp_compression = option('0','1','2','3','4', default='0')\n"
    "\tmax_color_depth = option(8,15,16,24, default='24')\n"
    "\tpersistent_disk_bitmap_cache = boolean(default=0)\n"
    "\tcache_waiting_list = boolean(default=1)\n"
    "\tpersist_bitmap_cache_on_disk = boolean(default=0)\n"
    "\tbitmap_compression = boolean(default=1)\n"
    "\tallow_channels = string_list(default='*')\n"
    "\tdeny_channels = string_list(default='')\n"
    "[mod_rdp]\n"
    "\trdp_compression = option('0','1','2','3','4', default='0')\n"
    "\tdisconnect_on_logon_user_change = boolean(default=0)\n"
    "\topen_session_timeout = integer(min=0,max=4294967295, default=0)\n"
    "\tcertificate_change_action = option('0','1', default='0')\n"
    "\textra_orders = string(default='')\n"
    "\tenable_nla = boolean(default=1)\n"
    "\tenable_kerberos = boolean(default=0)\n"
    "\tpersistent_disk_bitmap_cache = boolean(default=0)\n"
    "\tcache_waiting_list = boolean(default=1)\n"
    "\tpersist_bitmap_cache_on_disk = boolean(default=0)\n"
    "\tallow_channels = string_list(default='*')\n"
    "\tdeny_channels = string_list(default='')\n"
    "[mod_vnc]\n"
    "\tclipboard = boolean(default=0)\n"
    "\tencodings = string(default='')\n"
    "\tallow_authentification_retries = boolean(default=0)\n"
    "[video]\n"
    "\tcapture_flags = integer(min=0,max=16, default=1)\n"
    "\tcapture_png = boolean(default=1)\n"
    "\tcapture_wrm = boolean(default=1)\n"
    "\tcapture_flv = boolean(default=0)\n"
    "\tcapture_ocr = boolean(default=0)\n"
    "\tocr_interval = integer(min=0,max=4294967295, default=100)\n"
    "\tocr_on_title_bar_only = boolean(default=0)\n"
    "\tocr_max_unrecog_char_rate = integer(min=0,max=100, default=40)\n"
    "\tpng_interval = integer(min=0,max=4294967295, default=3000)\n"
    "\tcapture_groupid = integer(min=0,max=4294967295, default=33)\n"
    "\tframe_interval = integer(min=0,max=4294967295, default=40)\n"
    "\tbreak_interval = integer(min=0,max=4294967295, default=600)\n"
    "\tpng_limit = integer(min=0,max=4294967295, default=3)\n"
    "\treplay_path = string(max=1023, default='/tmp/')\n"
    "\tl_bitrate = integer(min=0,max=4294967295, default=20000)\n"
    "\tl_framerate = integer(min=0,max=4294967295, default=5)\n"
    "\tl_height = integer(min=0,max=4294967295, default=480)\n"
    "\tl_width = integer(min=0,max=4294967295, default=640)\n"
    "\tl_qscale = integer(min=0,max=4294967295, default=25)\n"
    "\tm_bitrate = integer(min=0,max=4294967295, default=40000)\n"
    "\tm_framerate = integer(min=0,max=4294967295, default=5)\n"
    "\tm_height = integer(min=0,max=4294967295, default=768)\n"
    "\tm_width = integer(min=0,max=4294967295, default=1024)\n"
    "\tm_qscale = integer(min=0,max=4294967295, default=15)\n"
    "\th_bitrate = integer(min=0,max=4294967295, default=200000)\n"
    "\th_framerate = integer(min=0,max=4294967295, default=5)\n"
    "\th_height = integer(min=0,max=4294967295, default=1024)\n"
    "\th_width = integer(min=0,max=4294967295, default=1280)\n"
    "\th_qscale = integer(min=0,max=4294967295, default=15)\n"
    "\thash_path = string(max=1023, default='/var/rdpproxy/hash/')\n"
    "\trecord_tmp_path = string(max=1023, default='/var/rdpproxy/tmp/')\n"
    "\trecord_path = string(max=1023, default='/var/rdpproxy/recorded/')\n"
    "\tinactivity_pause = boolean(default=0)\n"
    "\tinactivity_timeout = integer(min=0,max=4294967295, default=300)\n"
    "\tdisable_keyboard_log = integer(min=0,max=7, default=0)\n"
    "\twrm_color_depth_selection_strategy = integer(min=0,max=1, default=0)\n"
    "\twrm_compression_algorithm = integer(min=0,max=3, default=0)\n"
    "[crypto]\n"
    "\tkey0 = string(min=64,max=64, default='000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F')\n"
    "\tkey1 = string(min=64,max=64, default='000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F')\n"
    "[debug]\n"
    "\tx224 = integer(min=0,max=4294967295, default=0)\n"
    "\tmcs = integer(min=0,max=4294967295, default=0)\n"
    "\tsec = integer(min=0,max=4294967295, default=0)\n"
    "\trdp = integer(min=0,max=4294967295, default=0)\n"
    "\tprimary_orders = integer(min=0,max=4294967295, default=0)\n"
    "\tsecondary_orders = integer(min=0,max=4294967295, default=0)\n"
    "\tbitmap = integer(min=0,max=4294967295, default=0)\n"
    "\tcapture = integer(min=0,max=4294967295, default=0)\n"
    "\tauth = integer(min=0,max=4294967295, default=0)\n"
    "\tsession = integer(min=0,max=4294967295, default=0)\n"
    "\tfront = integer(min=0,max=4294967295, default=0)\n"
    "\tmod_rdp = integer(min=0,max=4294967295, default=0)\n"
    "\tmod_vnc = integer(min=0,max=4294967295, default=0)\n"
    "\tmod_int = integer(min=0,max=4294967295, default=0)\n"
    "\tmod_xup = integer(min=0,max=4294967295, default=0)\n"
    "\twidget = integer(min=0,max=4294967295, default=0)\n"
    "\tinput = integer(min=0,max=4294967295, default=0)\n"
    "\tpassword = integer(min=0,max=4294967295, default=0)\n"
    "\tcompression = integer(min=0,max=4294967295, default=0)\n"
    "\tcache = integer(min=0,max=4294967295, default=0)\n"
    "\tbitmap_update = integer(min=0,max=4294967295, default=0)\n"
    "\tperformance = integer(min=0,max=4294967295, default=0)\n"
    "\tpass_dialog_box = integer(min=0,max=4294967295, default=0)\n"
    "[translation]\n"
    "\tbutton_ok = string(default='OK')\n"
    "\tbutton_cancel = string(default='Cancel')\n"
    "\tbutton_help = string(default='Help')\n"
    "\tbutton_close = string(default='Close')\n"
    "\tbutton_refused = string(default='Refused')\n"
    "\tlogin = string(default='Login')\n"
    "\tusername = string(default='Username')\n"
    "\tpassword = string(default='Password')\n"
    "\ttarget = string(default='Target')\n"
    "\tdiagnostic = string(default='Diagnostic')\n"
    "\tconnection_closed = string(default='Connection closed')\n"
    "\thelp_message = string(default='')\n"
    "\tmanager_close_cnx = string(default='Connection closed by manager')\n"
    "\tlanguage = option('fr','en', default='en')\n");
}
