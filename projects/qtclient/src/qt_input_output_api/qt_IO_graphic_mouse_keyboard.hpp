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
   Copyright (C) Wallix 2010-2013
   Author(s): Clément Moroldo, Jonathan Poelen, Christophe Grosjean, David Fort
*/



#pragma once

#include "utils/log.hpp"
#include "core/RDP/orders/RDPSurfaceCommands.hpp"
#include "core/RDP/rdp_pointer.hpp"

#include "qt_input_output_api/graphics.hpp"
#include "qt_graphics_components/qt_progress_bar_window.hpp"
#include "qt_graphics_components/qt_screen_window.hpp"
#include "qt_graphics_components/qt_form_window.hpp"

#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QPainter>

#include "client_redemption/pointer_to_rgba8888.hpp"

#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>


class QtIOGraphicMouseKeyboard : public ClientRemoteAppGraphicAPI
{
    QtForm             * form;
    QtScreen           * screen;
    ProgressBarWindow  * bar;

    std::map<uint32_t, RemoteAppQtScreen *> remote_app_screen_map;
    std::vector<QPixmap> balises;

    struct Graphics : qtclient::Graphics
    {
        QtIOGraphicMouseKeyboard & self;

        Graphics(QtIOGraphicMouseKeyboard & self) noexcept : self(self) {}

        void begin_update() override {

            ++this->update_counter;
        }

        void end_update() override {
            assert(this->update_counter);
            --this->update_counter;
            if (this->update_counter != 0){
                return;
            }

            if (this->self.config->mod_state == ClientRedemptionConfig::MOD_RDP_REMOTE_APP) {
                for (auto& p : this->self.remote_app_screen_map) {
                    if (p.second) {
                        p.second->update_view();
                    }
                }
            } else {
                if (this->self.screen != nullptr) {
                    this->self.screen->update_view();
                }
            }
        }

        void new_pointer(gdi::CachePointerIndex cache_idx, const RdpPointerView & cursor) override
        {
            if (!cache_idx.is_predefined_pointer()) {
                this->pointer_cache[cache_idx.cache_index()] = cursor;
            }
        }

        void cached_pointer(gdi::CachePointerIndex cache_idx) override
        {
            auto const& pointer = cache_idx.is_predefined_pointer()
                ? predefined_pointer_to_pointer(cache_idx.as_predefined_pointer())
                : pointer_cache[cache_idx.cache_index()];

            auto hotspot = pointer.get_hotspot();
            auto rgba_cursor = redclient::pointer_to_rgba8888(pointer);
            QImage cursor_image(
                rgba_cursor.data(),
                int(rgba_cursor.width),
                int(rgba_cursor.height),
                int(rgba_cursor.bytes_per_line()),
                QImage::Format_RGBA8888);
            QCursor cursor(QPixmap::fromImage(cursor_image), hotspot.x, hotspot.x);

            if (this->self.config->mod_state == ClientRedemptionConfig::MOD_RDP_REMOTE_APP) {
                for (auto && p : this->self.remote_app_screen_map) {
                    if (p.second) {
                        p.second->setCursor(cursor);
                    }
                }
            }
            else if (this->self.screen) {
                this->self.screen->setCursor(cursor);
            }
        }

    private:
        int update_counter = 0;
        std::array<RdpPointer, gdi::CachePointerIndex::MAX_POINTER_COUNT> pointer_cache;
    };

    Graphics graphics;

public:
    QtIOGraphicMouseKeyboard(ClientCallback * controller, ClientRedemptionConfig * config)
    : ClientRemoteAppGraphicAPI(controller, config, QApplication::desktop()->width(), QApplication::desktop()->height())
    , form(new QtForm(this->config, this->controller))
    , screen(nullptr)
    , bar(nullptr)
    , graphics(*this)
    {
    }

    gdi::GraphicApi & get_graphics() noexcept
    {
        return this->graphics;
    }

    void show_screen() {
        if (this->form) {
            this->form->hide();
            if (this->screen) {
                this->screen->show();
            }
        }
    }

    void set_screen_size(int x, int y) {
        if (this->screen) {
            this->screen->setFixedSize(x, y);
        }
    }

    void update_screen() {
        if (this->screen) {
            this->screen->slotRepainMatch();
        }
    }

    void reset_cache(int w, int h)
    {
        LOG(LOG_INFO, "reset_cache w=%d h=%d", w, h);
        this->graphics.resize(w, h);
    }

    void create_screen() {
        QPixmap * map = &this->graphics.get_pixmap();
        this->screen = new RDPQtScreen(&this->config->windowsData, this->controller, map, this->config->is_spanning, this->config->target_IP);
    }

    void create_replay_screen() {
        QPixmap * map = &this->graphics.get_pixmap();

        this->screen = new ReplayQtScreen(this->controller, map, ClientConfig::get_movie_time_length(this->config->_movie_full_path.c_str()), 0, &(this->config->windowsData), this->config->_movie_name);
    }

    QWidget * get_static_qwidget() {
        return this->form;
    }

    void dropScreen() {
        if (this->screen != nullptr) {
            this->screen->disconnection();
            this->screen = nullptr;
        }
    }

    void draw_image(QPoint const& point, QImage& image)
    {
        this->graphics.get_painter().drawImage(point, image);
    }

    void draw_frame(int frame_index) {
        if (this->config->is_replaying) {
            this->graphics.get_painter().drawPixmap(
                QPoint(0, 0), this->balises[frame_index],
                QRect(0, 0,
                    this->graphics.get_pixmap().width(),
                    this->graphics.get_pixmap().height()
                )
            );
        }
    }



    void readError(std::string const & movie_path) {
        const std::string errorMsg("Cannot read movie \""+movie_path+ "\".");
        LOG(LOG_INFO, "%s", errorMsg.c_str());
        std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");

        if (this->form) {
            this->form->set_ErrorMsg(labelErrorMsg);
        }
    }

    void set_ErrorMsg(std::string const & error_msg) {
        if (this->form) {
            this->form->set_ErrorMsg(error_msg);
        }
    }

    void init_form() {
        if (this->form) {
            this->form->init_form();
            if (this->config->mod_state != ClientRedemptionConfig::MOD_RDP_REPLAY) {

                this->form->set_IPField(this->config->target_IP);
                this->form->set_portField(this->config->port);
                this->form->set_PWDField(this->config->user_password);
                this->form->set_userNameField(this->config->user_name);
            }

            this->form->show();
        }
    }



    /////////////////////////////////////////////////////////////////////////////////////////////////
    //  REMOTE APP FUNCTIONS
    //////////////////////////

    void create_remote_app_screen(uint32_t id, int w, int h, int x, int y) override {
        LOG(LOG_INFO, "create_remote_app_screen 1");
        this->remote_app_screen_map.emplace(id, new RemoteAppQtScreen(&this->config->windowsData, this->controller, w, h, x, y, &this->graphics.get_pixmap()));
        LOG(LOG_INFO, "create_remote_app_screen 2");
    }

private:
    RemoteAppQtScreen* find_remote_app(uint32_t id)
    {
        auto it = this->remote_app_screen_map.find(id);
        return it == this->remote_app_screen_map.end() ? nullptr : it->second;
    }

public:
    void show_screen(uint32_t id) override {
        if (this->form) {
            this->form->hide();
            if (auto* screen = this->find_remote_app(id)) {
                screen->show();
            }
        }
    }

    void move_screen(uint32_t id, int x, int y) override {
        if (auto* screen = this->find_remote_app(id)) {
            screen->move(x, y);
        }
    }

    void set_screen_size(uint32_t id, int x, int y) override {
        if (auto* screen = this->find_remote_app(id)) {
            screen->setFixedSize(x, y);
        }
    }

    void set_pixmap_shift(uint32_t id, int x, int y) override {
        if (auto* screen = this->find_remote_app(id)) {
            screen->x_pixmap_shift = x;
            screen->y_pixmap_shift = y;
        }
    }

    int get_visible_width(uint32_t id) override {
        if (auto* screen = this->find_remote_app(id)) {
            return screen->width();
        }
        return -1;
    }

    int get_visible_height(uint32_t id) override {
        if (auto* screen = this->find_remote_app(id)) {
            return screen->height();
        }
        return -1;
    }

    int get_mem_width(uint32_t id) override {
        if (auto* screen = this->find_remote_app(id)) {
            return screen->_width;
        }
        return -1;
    }

    int get_mem_height(uint32_t id) override {
        if (auto* screen = this->find_remote_app(id)) {
            return screen->_height;
        }
        return -1;
    }

    void set_mem_size(uint32_t id, int w, int h) override {
        if (auto* screen = this->find_remote_app(id)) {
            screen->_width = w;
            screen->_height = h;
        }
    }

    void clear_remote_app_screen() override {
        for (auto& p : this->remote_app_screen_map) {
            if (p.second) {
                p.second->disconnection();
                p.second = nullptr;
            }
        }
        this->remote_app_screen_map.clear();

        if (this->form) {
            this->form->show();
        }
    }

    FrontAPI::ResizeResult server_resize(ScreenInfo screen_server) {

        if (screen_server.width == 0 || screen_server.height == 0) {
            return FrontAPI::ResizeResult::fail;
        }

        switch (this->config->mod_state) {

            case ClientRedemptionConfig::MOD_RDP:
                if (this->config->info.screen_info.width == screen_server.width
                 && this->config->info.screen_info.height == screen_server.height) {
                    return FrontAPI::ResizeResult::instant_done;
                }
                this->dropScreen();
                this->reset_cache(screen_server.width, screen_server.height);
                this->screen = new RDPQtScreen(&this->config->windowsData, this->controller, &this->graphics.get_pixmap(), this->config->is_spanning, this->config->target_IP);
                this->screen->show();
                    break;

            case ClientRedemptionConfig::MOD_VNC:
                if (this->config->modVNCParamsData.width == screen_server.width
                 && this->config->modVNCParamsData.height == screen_server.height) {
                    return FrontAPI::ResizeResult::instant_done;
                }
                this->config->modVNCParamsData.width = screen_server.width;
                this->config->modVNCParamsData.height = screen_server.height;
                this->dropScreen();
                this->reset_cache(screen_server.width, screen_server.height);
                this->screen = new RDPQtScreen(&this->config->windowsData, this->controller, &this->graphics.get_pixmap(), this->config->is_spanning, this->config->target_IP);
                this->screen->show();
                    break;

            case ClientRedemptionConfig::MOD_RDP_REMOTE_APP:
                return FrontAPI::ResizeResult::remoteapp;
                    break;

            case ClientRedemptionConfig::MOD_RDP_REPLAY:
                if (!this->config->is_loading_replay_mod) {
                    time_t current_time_movie = 0;

                    if (!this->config->is_pre_loading) {
                        if (this->screen) {
                            current_time_movie = this->screen->get_current_time_movie();
                        }
                        this->dropScreen();
                    }

                    this->reset_cache(screen_server.width, screen_server.height);

                    if (!this->config->is_pre_loading) {

                        this->screen = new ReplayQtScreen(this->controller, &this->graphics.get_pixmap(), ClientConfig::get_movie_time_length(this->config->_movie_full_path.c_str()), current_time_movie, &(this->config->windowsData), this->config->_movie_name);

                        this->screen->show();
                    }
                }
                return FrontAPI::ResizeResult::instant_done;
                    break;
        }

        this->config->info.screen_info.bpp = screen_server.bpp;

        return FrontAPI::ResizeResult::instant_done;
    }

    long int init_replay(const std::string & movie_path, bool is_v2_wrm) {
        this->create_replay_screen();

        if (is_v2_wrm) {
            this->balises.clear();
            long int movie_length = ClientConfig::get_movie_time_length(movie_path.c_str());
            this->form->hide();

            if (movie_length > ClientRedemptionConfig::BALISED_FRAME) {
                this->bar = new ProgressBarWindow(movie_length);
                return movie_length;
            }
        }

        return 0;
    }


    bool pre_load_movie(const std::string & movie_path, bool is_v2_wrm) {

        long int endin_frame = 0;

        long int movie_length = this->init_replay(movie_path, is_v2_wrm);

        while (endin_frame < movie_length) {
            // TODO this->controller->instant_play_client(std::chrono::microseconds(endin_frame*1000000));
            this->balises.push_back(this->graphics.get_pixmap());
            endin_frame += ClientRedemptionConfig::BALISED_FRAME;
            if (this->bar) {
                this->bar->setValue(endin_frame);
            }
        }

        if (this->bar) {
            this->bar->close();
        }

        return (movie_length != 0);
    }

    void stop_replay() {
        if (this->screen) {
            this->screen->stopRelease();
        }
    }

    void close()
    {
        if (this->form != nullptr) {
            this->form->close();
        }

        // this->controller->disconnect(false);
    }
};
