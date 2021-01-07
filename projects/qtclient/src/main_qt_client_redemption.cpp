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

#include "utils/set_exception_handler_pretty_message.hpp"

#include "client_redemption/client_redemption.hpp"

#include "qt_input_output_api/qt_output_sound.hpp"
#include "qt_input_output_api/qt_input_output_clipboard.hpp"
#include "qt_input_output_api/qt_IO_graphic_mouse_keyboard.hpp"
#include "qt_input_output_api/qt_input_socket.hpp"
#include "qt_input_output_api/IO_disk.hpp"
#include "system/scoped_ssl_init.hpp"



class ClientRedemptionQt : public ClientRedemption
{

private:
    QtIOGraphicMouseKeyboard qt_graphic;
    QtOutputSound qt_sound;
    QtInputSocket qt_socket_listener;
    QtInputOutputClipboard qt_clipboard;
    IODisk ioDisk;

public:
    ClientRedemptionQt(
        EventContainer& events,
        ClientRedemptionConfig & config)
    : ClientRedemption(events, config)
    , qt_graphic(&this->_callback, &this->config)
    , qt_sound(this->config.SOUND_TEMP_DIR, this->qt_graphic.get_static_qwidget())
    , qt_socket_listener(this->qt_graphic.get_static_qwidget(), events)
    , qt_clipboard(&this->clientCLIPRDRChannel, this->config.CB_TEMP_DIR,
        this->qt_graphic.get_static_qwidget())
    {
        this->clientRDPSNDChannel.set_api(&this->qt_sound);
        this->clientCLIPRDRChannel.set_api(&this->qt_clipboard);
        this->clientRDPDRChannel.set_api(&this->ioDisk);
        this->clientRemoteAppChannel.set_api(&this->qt_graphic);

        this->qt_graphic.init_form();

        if (this->config.help_mode) {
            this->qt_graphic.close();
        } else {
            this->cmd_launch_conn();
        }
    }

    void session_update(timeval /*now*/, LogId /*id*/, KVLogList /*kv_list*/) override {}

    void possible_active_window_change() override {}

    void connect(const std::string& ip, const std::string& name, const std::string& pwd, const int port) override {
        if (this->config.mod_state != ClientRedemptionConfig::MOD_VNC) {
            if (this->config.is_spanning) {
                this->config.rdp_width  = this->qt_graphic.screen_max_width;
                this->config.rdp_height = this->qt_graphic.screen_max_height;

                this->config.modVNCParamsData.width = this->qt_graphic.screen_max_width;
                this->config.modVNCParamsData.height = this->qt_graphic.screen_max_height;
            }

            switch (this->config.mod_state) {
                case ClientRedemptionConfig::MOD_RDP:
                    this->config.info.screen_info.width  = this->config.rdp_width;
                    this->config.info.screen_info.height = this->config.rdp_height;
                    break;

                case ClientRedemptionConfig::MOD_VNC:
                    this->config.info.screen_info.width  = this->config.modVNCParamsData.width;
                    this->config.info.screen_info.height = this->config.modVNCParamsData.height;
                    break;

                default: break;
            }

            if (this->config.mod_state != ClientRedemptionConfig::MOD_RDP_REMOTE_APP) {

                this->qt_graphic.reset_cache(this->config.info.screen_info.width, this->config.info.screen_info.height);
                this->qt_graphic.create_screen();

            } else {
                this->qt_graphic.reset_cache(this->qt_graphic.screen_max_width, this->qt_graphic.screen_max_height);
            }
        }

        this->events.set_current_time(tvtime());

        ClientRedemption::connect(ip, name, pwd, port);

        if (this->config.connected) {
            if (auto* mod = this->_callback.get_mod()) {
                auto action = [this](bool is_timeout){
                    this->events.set_current_time(tvtime());
                    try {
                        auto fn = [is_timeout](int /*fd*/){ return !is_timeout; };
                        this->events.execute_events(fn, 0);
                    } catch (const Error & e) {
                        const std::string errorMsg = str_concat('[', this->config.target_IP, "] lost: pipe broken");
                        LOG(LOG_ERR, "%s: %s", errorMsg, e.errmsg());
                        std::string labelErrorMsg = str_concat("<font color='Red'>", errorMsg, "</font>");
                        this->disconnect(labelErrorMsg, true);
                    }
                };

                this->qt_socket_listener.start_to_listen(
                    this->client_sck,
                    [action]{ action(false); },
                    [action]{ action(true); });

                this->start_wab_session_time = tvtime();

                if (this->config.mod_state != ClientRedemptionConfig::MOD_RDP_REMOTE_APP) {
                    this->qt_graphic.show_screen();
                }
            }
        }
    }

    void disconnect(std::string const & error, bool pipe_broken) override {
        this->qt_graphic.dropScreen();
        this->qt_socket_listener.disconnect();
        ClientRedemption::disconnect(error, pipe_broken);
        this->qt_graphic.init_form();
    }

    void close() override {
        this->qt_graphic.close();
        this->disconnect("", false);
    }

    void set_error_msg(const std::string & error) override {
        ClientRedemption::set_error_msg(error);
        this->qt_graphic.set_ErrorMsg(error);
    }

    void set_pointer(uint16_t cache_idx, Pointer const& cursor, SetPointerMode mode) override {
        this->qt_graphic.get_graphics().set_pointer(cache_idx, cursor, mode);
    }

    bool init_mod() override {
        switch (this->config.mod_state) {
            case ClientRedemptionConfig::MOD_RDP_REMOTE_APP:
                this->config.info.screen_info.width = this->qt_graphic.screen_max_width;
                this->config.info.screen_info.height = this->qt_graphic.screen_max_height;
                break;
            default: break;
        }
        return ClientRedemption::init_mod();
    }

    ResizeResult server_resize(ScreenInfo screen_server) override {
        if (bool(this->config.verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "server_resize to (%d, %u, %u)",
            screen_server.width, screen_server.height, screen_server.bpp);
        }
        return this->qt_graphic.server_resize(screen_server);
    }

    void begin_update() override {
        if ((this->config.connected || this->config.is_replaying)) {
            this->qt_graphic.get_graphics().begin_update();
        }
        ClientRedemption::begin_update();
    }

    void end_update() override {
        if ((this->config.connected || this->config.is_replaying)) {
            this->qt_graphic.get_graphics().end_update();
        }
        ClientRedemption::end_update();
    }

    void print_wrm_graphic_stat(const std::string & movie_path) override {
        this->config.is_pre_loading = true;
        // this->qt_graphic.pre_load_movie(movie_path, this->replay_mod->get_wrm_version() == WrmVersion::v2);
        this->config.is_pre_loading = false;
        this->qt_graphic.stop_replay();
        this->qt_graphic.show_screen();

        ClientRedemption::print_wrm_graphic_stat(movie_path);
    }

    virtual void instant_replay_client(int begin, int last_balised) override {
        this->qt_graphic.draw_frame(last_balised);
        ClientRedemption::instant_replay_client(begin, last_balised);
        this->qt_graphic.update_screen();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //-----------------------------
    //       DRAW FUNCTIONS
    //-----------------------------

    using gdi::GraphicApi::draw;

    void draw(const RDPPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->qt_graphic.get_graphics().draw(cmd, clip, color_ctx);
        ClientRedemption::draw(cmd, clip, color_ctx);
    }

    void draw(const RDPOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->qt_graphic.get_graphics().draw(cmd, clip, color_ctx);
        ClientRedemption::draw(cmd, clip, color_ctx);
    }

    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
        this->qt_graphic.get_graphics().draw(bitmap_data, bmp);
        ClientRedemption::draw(bitmap_data, bmp);
    }

    void draw(const RDPLineTo & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->qt_graphic.get_graphics().draw(cmd, clip, color_ctx);
        ClientRedemption::draw(cmd, clip, color_ctx);
    }

    void draw(const RDPScrBlt & cmd, Rect clip) override {
        this->qt_graphic.get_graphics().draw(cmd, clip);
        ClientRedemption::draw(cmd, clip);
    }

    void draw(const RDPMemBlt & cmd, Rect clip, const Bitmap & bitmap) override {
        this->qt_graphic.get_graphics().draw(cmd, clip, bitmap);
        ClientRedemption::draw(cmd, clip, bitmap);
    }

    void draw(const RDPMem3Blt & cmd, Rect clip, gdi::ColorCtx color_ctx, const Bitmap & bitmap) override {
        this->qt_graphic.get_graphics().draw(cmd, clip, color_ctx, bitmap);
        ClientRedemption::draw(cmd, clip, color_ctx, bitmap);
    }

    void draw(const RDPDstBlt & cmd, Rect clip) override {
        this->qt_graphic.get_graphics().draw(cmd, clip);
        ClientRedemption::draw(cmd, clip);
    }

    void draw(const RDPMultiDstBlt & cmd, Rect clip) override {
        this->qt_graphic.get_graphics().draw(cmd, clip);
        ClientRedemption::draw(cmd, clip);
    }

    void draw(const RDPMultiOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->qt_graphic.get_graphics().draw(cmd, clip, color_ctx);
        ClientRedemption::draw(cmd, clip, color_ctx);
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->qt_graphic.get_graphics().draw(cmd, clip, color_ctx);
        ClientRedemption::draw(cmd, clip, color_ctx);
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) override {
        this->qt_graphic.get_graphics().draw(cmd, clip);
        ClientRedemption::draw(cmd, clip);
    }

    void draw(const RDPGlyphIndex & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) override {
        this->qt_graphic.get_graphics().draw(cmd, clip, color_ctx, gly_cache);
        ClientRedemption::draw(cmd, clip, color_ctx, gly_cache);
    }

    void draw(const RDPPolygonSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->qt_graphic.get_graphics().draw(cmd, clip, color_ctx);
        ClientRedemption::draw(cmd, clip, color_ctx);
    }

    void draw(const RDPPolygonCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->qt_graphic.get_graphics().draw(cmd, clip, color_ctx);
        ClientRedemption::draw(cmd, clip, color_ctx);
    }

    void draw(const RDPPolyline & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->qt_graphic.get_graphics().draw(cmd, clip, color_ctx);
        ClientRedemption::draw(cmd, clip, color_ctx);
    }

    void draw(const RDPEllipseSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->qt_graphic.get_graphics().draw(cmd, clip, color_ctx);
        ClientRedemption::draw(cmd, clip, color_ctx);
    }

    void draw(const RDPEllipseCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        this->qt_graphic.get_graphics().draw(cmd, clip, color_ctx);
        ClientRedemption::draw(cmd, clip, color_ctx);
    }

    void draw(const RDP::FrameMarker& order) override {
        this->qt_graphic.get_graphics().draw(order);
        ClientRedemption::draw(order);
    }

    void draw(RDPSetSurfaceCommand const & /*cmd*/) override {
    }

    void draw(RDPSetSurfaceCommand const & cmd, RDPSurfaceContent const & content) override {
        LOG(LOG_INFO, "DEFAULT: RDPSetSurfaceCommand(x=%d y=%d width=%d(%d) height=%d(%d))", cmd.destRect.x, cmd.destRect.y,
                        cmd.width, content.width, cmd.height, cmd.height);

        ClientRedemption::draw(cmd, content);

        QImage img(content.data, content.width, cmd.height, QImage::Format_RGBA8888);
        img = img.copy(QRect(0, 0, cmd.width, cmd.height));

#if 0
        static int frameNo = 0;
        frameNo++;
        img.save(QString("/tmp/img%1.png").arg(frameNo), "PNG", 100);
#endif

        this->qt_graphic.draw_image(QPoint(cmd.destRect.x, cmd.destRect.y), img);
    }
};


int main(int argc, char** argv)
{
    set_exception_handler_pretty_message();

    Inifile ini;
    EventContainer events;

    QApplication app(argc, argv);

    RDPVerbose verbose = RDPVerbose::none; //RDPVerbose::cliprdr | RDPVerbose::cliprdr_dump;
    ClientRedemptionConfig config(verbose, CLIENT_REDEMPTION_MAIN_PATH);
    ClientConfig::set_config(argc, const_cast<const char**>(argv), config);

    ScopedSslInit scoped_init;

    ClientRedemptionQt client_qt(events, config);

    app.exec();
}
