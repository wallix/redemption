/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2014
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_UTILS_WRITE_SPEC_CPP
#define REDEMPTION_UTILS_WRITE_SPEC_CPP

#include "write_config_spec.hpp"
#include "config.hpp"

#include <limits>
#include <type_traits>


namespace {
    void write_config_spec_var(std::ostream & os, const char * name, bool x) {
        os << "\t" << name << " = boolean(default=" << x << ")\n";
    }
    void write_config_spec_var(std::ostream & os, const char * name, FieldObserver::BoolField const & x) {
        os << "\t" << name << " = boolean(default=" << x.get() << ")\n";
    }

    template<class T>
    typename std::enable_if<std::is_integral<T>::value>::type
    write_config_spec_var(std::ostream & os, const char * name, T x) {
        os << "\t" << name << " = integer(min="
           << std::numeric_limits<T>::min() << ",max="
           << std::numeric_limits<T>::max() << ", default="
           << x << ")\n";
    }
//     void write_config_spec_var(std::ostream & os, const char * name, FieldObserver::UnsignedField const & x) {
//         write_config_spec_var(os, name, x.get());
//     }
//     void write_config_spec_var(std::ostream & os, const char * name, FieldObserver::SignedField const & x) {
//         write_config_spec_var(os, name, x.get());
//     }

    template<std::size_t N>
    void write_config_spec_var(std::ostream & os, const char * name, Inifile::StaticString<N> const & x) {
        os << "\t" << name << " = string(max=" << (N-1) << ", default='" << x.c_str() << "')\n";
    }
    template<std::size_t N>
    void write_config_spec_var(std::ostream & os, const char * name, Inifile::StaticPath<N> const & x) {
        os << "\t" << name << " = string(max=" << (N-1) << ", default='" << x.c_str() << "')\n";
    }
    void write_config_spec_var(std::ostream & os, const char * name, redemption::string const & x) {
        os << "\t" << name << " = string(default='" << x.c_str() << "')\n";
    }
    void write_config_spec_var(std::ostream & os, const char * name, FieldObserver::StringField const & x) {
        write_config_spec_var(os, name, x.get());
    }
}

void write_config_spec(std::ostream & os, Inifile const & ini)
{
    const char * strlvls[] {"low","medium","high"};

    #define WRITE_VAR(section, name) write_config_spec_var(os, #name, ini.section.name)

    os << "[globals]\n";
    {
        //WRITE_VAR(globals, capture_chunk);

        //WRITE_VAR(globals, auth_user);
        //WRITE_VAR(globals, host);
        //WRITE_VAR(globals, target);
        //WRITE_VAR(globals, target_device);
        //WRITE_VAR(globals, target_user);
        //WRITE_VAR(globals, target_application);

        WRITE_VAR(globals, bitmap_cache);
        WRITE_VAR(globals, port);
        WRITE_VAR(globals, nomouse);
        WRITE_VAR(globals, notimestamp);
        os << "\tencryptionLevel = option('low','medium','high', default='" << strlvls[ini.globals.encryptionLevel] << "')\n";
        os << "\tauthip = ip_addr(default='" << ini.globals.authip << "')\n";
        WRITE_VAR(globals, authport);
        WRITE_VAR(globals, autovalidate);

        WRITE_VAR(globals, max_tick);
        WRITE_VAR(globals, keepalive_grace_delay);
        WRITE_VAR(globals, close_timeout);

        WRITE_VAR(globals, auth_channel);
        WRITE_VAR(globals, enable_file_encryption);
        os << "\tlisten_address = ip_addr(default='" << ini.globals.listen_address << "')\n";
        WRITE_VAR(globals, enable_ip_transparent);
        WRITE_VAR(globals, certificate_password);

        WRITE_VAR(globals, png_path);
        WRITE_VAR(globals, wrm_path);

        WRITE_VAR(globals, alternate_shell);
        WRITE_VAR(globals, shell_working_directory);

        WRITE_VAR(globals, codec_id);
        WRITE_VAR(globals, movie);
        WRITE_VAR(globals, movie_path);
        os << "\tvideo_quality = option('low','medium','high', default='" << ini.globals.video_quality.get_cstr() << "')\n";
        WRITE_VAR(globals, enable_bitmap_update);
        WRITE_VAR(globals, enable_close_box);
        WRITE_VAR(globals, enable_osd);

        //WRITE_VAR(globals, flv_break_interval);
        //WRITE_VAR(globals, flv_frame_interval);

        WRITE_VAR(globals, persistent_path);
    }

    os << "[client]\n";
    {
        WRITE_VAR(client, ignore_logon_password);

        WRITE_VAR(client, performance_flags_default);
        WRITE_VAR(client, performance_flags_force_present);
        WRITE_VAR(client, performance_flags_force_not_present);

        WRITE_VAR(client, tls_fallback_legacy);
        WRITE_VAR(client, tls_support);
        WRITE_VAR(client, bogus_neg_request);

        WRITE_VAR(client, device_redirection);

        WRITE_VAR(client, disable_tsk_switch_shortcuts);

        os << "\trdp_compression = option('0','1','2','3','4', default='" << ini.client.rdp_compression << "')\n";

        os << "\tmax_color_depth = option(8,15,16,24, default='" << ini.client.max_color_depth << "')\n";

        WRITE_VAR(client, persistent_disk_bitmap_cache);
        WRITE_VAR(client, cache_waiting_list);
        WRITE_VAR(client, persist_bitmap_cache_on_disk);

        WRITE_VAR(client, bitmap_compression);

        os << "\tallow_channels = string_list(default='" << ini.client.allow_channels.c_str() << "')\n";
        os << "\tdeny_channels = string_list(default='" << ini.client.deny_channels.c_str() << "')\n";
    }


    os << "[mod_rdp]\n";
    {
        os << "\trdp_compression = option('0','1','2','3','4', default='" << ini.mod_rdp.rdp_compression << "')\n";

        WRITE_VAR(mod_rdp, disconnect_on_logon_user_change);

        WRITE_VAR(mod_rdp, open_session_timeout);

        os << "\tcertificate_change_action = option('0','1', default='" << ini.mod_rdp.certificate_change_action << "')\n";

        WRITE_VAR(mod_rdp, extra_orders);

        WRITE_VAR(mod_rdp, enable_nla);

        WRITE_VAR(mod_rdp, enable_kerberos);

        WRITE_VAR(mod_rdp, persistent_disk_bitmap_cache);
        WRITE_VAR(mod_rdp, cache_waiting_list);
        WRITE_VAR(mod_rdp, persist_bitmap_cache_on_disk);

        os << "\tallow_channels = string_list(default='" << ini.mod_rdp.allow_channels.get_cstr() << "')\n";
        os << "\tdeny_channels = string_list(default='" << ini.mod_rdp.deny_channels.get_cstr() << "')\n";
    }


    os << "[mod_vnc]\n";
    {
        WRITE_VAR(mod_vnc, clipboard);
        WRITE_VAR(mod_vnc, encodings);
        WRITE_VAR(mod_vnc, allow_authentification_retries);
    }


    os << "[video]\n";
    {
        os << "\tcapture_flags = integer(min=0,max=16, default=" << ini.video.capture_flags << ")\n";
        WRITE_VAR(video, capture_png);
        WRITE_VAR(video, capture_wrm);
        WRITE_VAR(video, capture_flv);
        WRITE_VAR(video, capture_ocr);

        WRITE_VAR(video, ocr_interval);
        WRITE_VAR(video, ocr_on_title_bar_only);
        os << "\tocr_max_unrecog_char_rate = integer(min=0,max=100, default=" << ini.video.ocr_max_unrecog_char_rate << ")\n";

        WRITE_VAR(video, png_interval);
        WRITE_VAR(video, capture_groupid);
        WRITE_VAR(video, frame_interval);
        WRITE_VAR(video, break_interval);
        WRITE_VAR(video, png_limit);
        WRITE_VAR(video, replay_path);

        WRITE_VAR(video, l_bitrate);
        WRITE_VAR(video, l_framerate);
        WRITE_VAR(video, l_height);
        WRITE_VAR(video, l_width);
        WRITE_VAR(video, l_qscale);

        WRITE_VAR(video, m_bitrate);
        WRITE_VAR(video, m_framerate);
        WRITE_VAR(video, m_height);
        WRITE_VAR(video, m_width);
        WRITE_VAR(video, m_qscale);

        WRITE_VAR(video, h_bitrate);
        WRITE_VAR(video, h_framerate);
        WRITE_VAR(video, h_height);
        WRITE_VAR(video, h_width);
        WRITE_VAR(video, h_qscale);

        WRITE_VAR(video, hash_path);
        WRITE_VAR(video, record_tmp_path);
        WRITE_VAR(video, record_path);

        WRITE_VAR(video, inactivity_pause);
        WRITE_VAR(video, inactivity_timeout);

        os << "\tdisable_keyboard_log = integer(min=0,max=7, default=" << ini.video.disable_keyboard_log.get() << ")\n";

        os << "\twrm_color_depth_selection_strategy = integer(min=0,max=1, default=" << ini.video.wrm_color_depth_selection_strategy << ")\n";

        os << "\twrm_compression_algorithm = integer(min=0,max=3, default=" << ini.video.wrm_compression_algorithm << ")\n";
    }


    os << "[crypto]\n";
    {
        auto tohex = [](std::ostream & os, const char * k) {
            int c;
            for (const char * e = k + 32; k != e; ++k) {
                c = (*k >> 4);
                c += (c > 9) ? 'A' - 10 : '0';
                os << char(c);
                c = (*k & 0xf);
                c += (c > 9) ? 'A' - 10 : '0';
                os << char(c);
            }
        };
        os << "\tkey0 = string(min=64,max=64, default='"; tohex(os, ini.crypto.key0.c_str()); os << "')\n";
        os << "\tkey1 = string(min=64,max=64, default='"; tohex(os, ini.crypto.key1.c_str()); os << "')\n";
    }


    os << "[debug]\n";
    {
        WRITE_VAR(debug, x224);
        WRITE_VAR(debug, mcs);
        WRITE_VAR(debug, sec);
        WRITE_VAR(debug, rdp);
        WRITE_VAR(debug, primary_orders);
        WRITE_VAR(debug, secondary_orders);
        WRITE_VAR(debug, bitmap);
        WRITE_VAR(debug, capture);
        WRITE_VAR(debug, auth);
        WRITE_VAR(debug, session);
        WRITE_VAR(debug, front);
        WRITE_VAR(debug, mod_rdp);
        WRITE_VAR(debug, mod_vnc);
        WRITE_VAR(debug, mod_int);
        WRITE_VAR(debug, mod_xup);
        WRITE_VAR(debug, widget);
        WRITE_VAR(debug, input);
        WRITE_VAR(debug, password);
        WRITE_VAR(debug, compression);
        WRITE_VAR(debug, cache);
        WRITE_VAR(debug, bitmap_update);
        WRITE_VAR(debug, performance);

        WRITE_VAR(debug, pass_dialog_box);
    }


    os << "[translation]\n";
    {
        WRITE_VAR(translation, button_ok);
        WRITE_VAR(translation, button_cancel);
        WRITE_VAR(translation, button_help);
        WRITE_VAR(translation, button_close);
        WRITE_VAR(translation, button_refused);
        WRITE_VAR(translation, login);
        WRITE_VAR(translation, username);
        WRITE_VAR(translation, password);
        WRITE_VAR(translation, target);
        WRITE_VAR(translation, diagnostic);
        WRITE_VAR(translation, connection_closed);
        WRITE_VAR(translation, help_message);
        WRITE_VAR(translation, manager_close_cnx);

        os << "\tlanguage = option('fr','en', default='" << ini.translation.language.get_cstr() << "')\n";
    }


    //os << "[context]\n";
    //{
        //WRITE_VAR(context, movie);

        //WRITE_VAR(context, opt_bitrate);
        //WRITE_VAR(context, opt_framerate);
        //WRITE_VAR(context, opt_qscale);

        //WRITE_VAR(context, opt_bpp);
        //WRITE_VAR(context, opt_height);
        //WRITE_VAR(context, opt_width);

        //WRITE_VAR(context, authchannel_answer);
        //WRITE_VAR(context, authchannel_result);
        //WRITE_VAR(context, authchannel_target);

        //WRITE_VAR(context, message);
        //WRITE_VAR(context, pattern_kill);
        //WRITE_VAR(context, pattern_notify);

        //WRITE_VAR(context, end_date_cnx);
        //WRITE_VAR(context, end_time);

        //os << "\tmode_console = option('allow','force','forbid')\n";
        //WRITE_VAR(context, timezone);
    //}

    #undef WRITE_VAR
}

#endif
