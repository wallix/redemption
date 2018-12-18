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

private:
    QtIOGraphicMouseKeyboard qt_graphic;
    QtOutputSound qt_sound;
    QtInputSocket qt_socket_listener;
    QtInputOutputClipboard qt_clipboard;
    QtClientRDPKeyLayout qt_rdp_keylayout;
    IODisk ioDisk;

public:
    ClientRedemptionQt(SessionReactor & session_reactor,
                       ClientRedemptionConfig & config)
            :ClientRedemption(session_reactor, config, &qt_graphic)
            , qt_sound(qt_graphic.get_static_qwidget())
            , qt_socket_listener(session_reactor, qt_graphic.get_static_qwidget())
            , qt_clipboard(qt_graphic.get_static_qwidget())
    {
        this->qt_socket_listener.set_client(this);
        this->qt_graphic.set_drawn_client(&(this->_callback), &(this->config));

        this->qt_sound.set_path(this->config.SOUND_TEMP_DIR);
        this->clientRDPSNDChannel.set_api(&(this->qt_sound));

        this->qt_clipboard.set_path(this->config.CB_TEMP_DIR);
        this->qt_clipboard.set_channel(&(this->clientCLIPRDRChannel));
        this->clientCLIPRDRChannel.set_api(&(this->qt_clipboard));

        this->clientRDPDRChannel.set_api(&(this->ioDisk));

        this->_callback.set_rdp_keyLayout_api(&(this->qt_rdp_keylayout));

        this->cmd_launch_conn();

        this->graphic_api->init_form();
    }

    void connect(const std::string& ip, const std::string& name, const std::string& pwd, const int port) override {
        ClientRedemption::connect(ip, name, pwd, port);

        if (this->config.connected) {

            if (this->qt_socket_listener.start_to_listen(this->client_sck, this->_callback.get_mod())) {

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

    void disconnect(std::string const & error, bool pipe_broken) override {
        this->qt_socket_listener.disconnect();
        ClientRedemption::disconnect(error, pipe_broken);
        this->graphic_api->init_form();
    }

    void update_keylayout() override {
        this->qt_rdp_keylayout.update_keylayout(this->config.info.keylayout);

        this->qt_rdp_keylayout.clearCustomKeyCode();
        for (KeyCustomDefinition& key : this->config.keyCustomDefinitions) {
            this->qt_rdp_keylayout.setCustomKeyCode(key.qtKeyID, key.scanCode, key.ASCII8, key.extended);
        }
        ClientRedemption::update_keylayout();
    }

    void closeFromGUI() override {
        this->graphic_api->closeFromGUI();
    }

    void set_error_msg(const std::string & error) {
        ClientRedemption::set_error_msg(error);
        this->graphic_api->set_ErrorMsg(error);
    }

    void set_pointer(Pointer const & cursor) override {
        this->graphic_api->set_pointer(cursor);
    }
};


int main(int argc, char** argv)
{
    set_exception_handler_pretty_message();

    SessionReactor session_reactor;

    QApplication app(argc, argv);

    RDPVerbose verbose = to_verbose_flags(0x0)/*RDPVerbose::rdpsnd*/;
    ClientRedemptionConfig config(const_cast<const char**>(argv), argc, verbose, CLIENT_REDEMPTION_MAIN_PATH);

    ClientRedemptionQt client_qt(session_reactor, config);

    app.exec();
}
