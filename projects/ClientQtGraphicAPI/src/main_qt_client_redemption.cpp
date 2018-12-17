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
   Copyright (C) Wallix 2010-2016
   Author(s): Clément Moroldo
*/


#include "utils/log.hpp"
#include "utils/fixed_random.hpp"
#include "utils/genrandom.hpp"
#include "utils/genfstat.hpp"
#include "utils/netutils.hpp"
#include "utils/sugar/algostring.hpp"

#include "acl/auth_api.hpp"

#include "core/RDP/RDPDrawable.hpp"
#include "core/channel_list.hpp"
#include "core/channel_names.hpp"

#include "mod/internal/replay_mod.hpp"
#include "mod/rdp/new_mod_rdp.hpp"
#include "mod/vnc/new_mod_vnc.hpp"

#include "transport/crypto_transport.hpp"
#include "transport/recorder_transport.hpp"
#include "transport/replay_transport.hpp"
#include "transport/socket_transport.hpp"

#include "capture/wrm_capture.hpp"

// #include "client_redemption/client_channel_managers/client_cliprdr_channel.hpp"
// #include "client_redemption/client_channel_managers/client_rdpdr_channel.hpp"
// #include "client_redemption/client_channel_managers/client_rdpsnd_channel.hpp"
// #include "client_redemption/client_channel_managers/client_remoteapp_channel.hpp"

#include "client_redemption/client_config/client_redemption_config.hpp"

#include "client_redemption/client_input_output_api/client_keymap_api.hpp"
#include "client_redemption/client_input_output_api/client_socket_api.hpp"

#include "client_redemption/client_redemption_api.hpp"

#include "client_redemption/mod_wrapper/client_callback.hpp"
#include "client_redemption/mod_wrapper/client_channel_mod.hpp"

#include "configs/config.hpp"
#include "front/execute_events.hpp"

#include "core/session_reactor.hpp"
#include "utils/set_exception_handler_pretty_message.hpp"

#include "client_redemption/client_redemption.hpp"

#include "qt_input_output_api/qt_output_sound.hpp"
#include "qt_input_output_api/qt_input_output_clipboard.hpp"
#include "qt_input_output_api/qt_IO_graphic_mouse_keyboard.hpp"
#include "qt_input_output_api/qt_input_socket.hpp"
#include "qt_input_output_api/IO_disk.hpp"
#include "qt_input_output_api/keymaps/qt_client_rdp_keylayout.hpp"



class ClientRedemptionQt : public ClientRedemption
{

    // io API
//     ClientOutputGraphicAPI      * graphic_api;
//     ClientIOClipboardAPI        * io_clipboard_api;
//     ClientOutputSoundAPI        * output_sound_api;
//     ClientInputSocketAPI        * socket_listener;
//     ClientKeyLayoutAPI          * keylayout_api;
//     ClientIODiskAPI             * io_disk_api;





public:
    ClientRedemptionQt(SessionReactor & session_reactor,
                     ClientRedemptionConfig & config,
                     ClientOutputGraphicAPI * graphic_api,
                     ClientIOClipboardAPI * io_clipboard_api,
                     ClientOutputSoundAPI * output_sound_api,
                     ClientInputSocketAPI * socket_listener,
                     ClientKeyLayoutAPI * keylayout_api,
                     ClientIODiskAPI * io_disk_api)
            :ClientRedemption(session_reactor, config, graphic_api, io_clipboard_api, output_sound_api, socket_listener, keylayout_api, io_disk_api)
//         : config(config)
//         , client_sck(-1)
//         , _callback(this, keylayout_api)
//         , session_reactor(session_reactor)
//         , graphic_api(graphic_api)
//         , io_clipboard_api(io_clipboard_api)
//         , output_sound_api (output_sound_api)
//         , socket_listener (socket_listener)
//         , keylayout_api(keylayout_api)
//         , io_disk_api(io_disk_api)
//         , close_box_extra_message_ref("Close")
//         , client_execute(session_reactor, *(this), this->config.info.window_list_caps, false)
//         , clientRDPSNDChannel(this->config.verbose, &(this->channel_mod), this->output_sound_api, this->config.rDPSoundConfig)
//         , clientCLIPRDRChannel(this->config.verbose, &(this->channel_mod), this->io_clipboard_api, this->config.rDPClipboardConfig)
//         , clientRDPDRChannel(this->config.verbose, &(this->channel_mod), this->io_disk_api, this->config.rDPDiskConfig)
//         , clientRemoteAppChannel(this->config.verbose, &(this->_callback), &(this->channel_mod), this->graphic_api)
//         , start_win_session_time(tvtime())
//         , secondary_connection_finished(false)
//         , primary_connection_finished(false)
//         , local_IP("unknow_local_IP")
    {

        this->cmd_launch_conn();
    }



    void listen_to_socket(const std::string& ip, const std::string& name, const std::string& pwd, const int port) {
        if (this->config.connected) {

            if (this->socket_listener) {

                if (this->socket_listener->start_to_listen(this->client_sck, this->_callback.get_mod())) {

                    this->start_wab_session_time = tvtime();

                    if (this->config.mod_state != ClientRedemptionConfig::MOD_RDP_REMOTE_APP) {
                        if (this->graphic_api) {
                            this->graphic_api->show_screen();
                        }
                    }

                    this->config.writeAccoundData(ip, name, pwd, port);
                }
            }
        }
    }

    virtual void connect(const std::string& ip, const std::string& name, const std::string& pwd, const int port) override {
        ClientRedemption::connect(ip, name, pwd, port);

        if (this->config.connected) {
            this->listen_to_socket(ip, name, pwd, port);
        }
    }
};



int main(int argc, char** argv)
{
    set_exception_handler_pretty_message();

    SessionReactor session_reactor;

    QApplication app(argc, argv);

    QtIOGraphicMouseKeyboard graphic_control_qt_obj;
    QWidget * qwidget_parent = graphic_control_qt_obj.get_static_qwidget();
    QtInputOutputClipboard clipboard_api_obj(qwidget_parent);
    QtOutputSound sound_api_obj(qwidget_parent);
    IODisk ioDisk_api_obj;
    QtInputSocket socket_api_obj(session_reactor, qwidget_parent);
    QtClientRDPKeyLayout keylayout_obj;

    ClientOutputGraphicAPI * graphic_qt    = &graphic_control_qt_obj;
    ClientIOClipboardAPI   * clipboard_api = &clipboard_api_obj;
    ClientOutputSoundAPI   * sound_api     = &sound_api_obj;
    ClientInputSocketAPI   * socket_api    = &socket_api_obj;
    ClientKeyLayoutAPI     * keylayout_api = &keylayout_obj;
    ClientIODiskAPI        * ioDisk_api    = &ioDisk_api_obj;

    RDPVerbose verbose = /*to_verbose_flags(0x0)*/RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump;

    ClientRedemptionConfig config(const_cast<const char**>(argv), argc, verbose, CLIENT_REDEMPTION_MAIN_PATH);

    sound_api->set_path(config.SOUND_TEMP_DIR);

    ClientRedemptionQt client_qt(session_reactor, config /*const_cast<char const**>(argv), argc, verbose*/
                              , graphic_qt
                              , clipboard_api
                              , sound_api
                              , socket_api
                              , keylayout_api
                              , ioDisk_api);

    app.exec();
}
