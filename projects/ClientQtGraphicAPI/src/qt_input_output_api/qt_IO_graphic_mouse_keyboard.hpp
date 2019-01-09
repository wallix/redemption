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

#include "qt_graphics_components/qt_progress_bar_window.hpp"
#include "qt_graphics_components/qt_options_window.hpp"
#include "qt_graphics_components/qt_screen_window.hpp"
#include "qt_graphics_components/qt_form_window.hpp"

#include <QtGui/QBitmap>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QRgb>
#include <QtGui/QWheelEvent>
// #include <QtGui/QWindowsCEStyle>

#if REDEMPTION_QT_VERSION == 4
#   include <QtCore/QUrl>
#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtGui/name>
#else
#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtWidgets/name>
#endif

#include REDEMPTION_QT_INCLUDE_WIDGET(QApplication)
#include REDEMPTION_QT_INCLUDE_WIDGET(QDesktopWidget)

#undef REDEMPTION_QT_INCLUDE_WIDGET



class QtIOGraphicMouseKeyboard : public ClientRemoteAppGraphicAPI
{

public:
    int                  mod_bpp;
    QtForm             * form;
    QtScreen           * screen;
    QPixmap              cache;
    ProgressBarWindow  * bar;
    QPainter             painter;
    QImage cursor_image;
    std::map<uint32_t, RemoteAppQtScreen *> remote_app_screen_map;
    //     QPixmap            * trans_cache;;
    std::vector<QPixmap> balises;

    bool is_pre_loading;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONSTRUCTOR
    //------------------------

    QtIOGraphicMouseKeyboard(ClientCallback * controller, ClientRedemptionConfig * config)
      : ClientRemoteAppGraphicAPI(controller, config, QApplication::desktop()->width(), QApplication::desktop()->height())
      , mod_bpp(24)
      , form(nullptr)
      , screen(nullptr)
      , bar(nullptr)
      , is_pre_loading(false)
    {
        this->form = new QtForm(this->config, this->controller);
    }

    virtual void draw(const RDP::RAIL::NewOrExistingWindow & ) override {}
    virtual void draw(const RDP::RAIL::WindowIcon & ) override {}
    virtual void draw(const RDP::RAIL::CachedIcon & ) override {}
    virtual void draw(const RDP::RAIL::DeletedWindow & ) override {}
    virtual void draw(const RDP::RAIL::NewOrExistingNotificationIcons & ) override {}
    virtual void draw(const RDP::RAIL::DeletedNotificationIcons & ) override {}
    virtual void draw(const RDP::RAIL::ActivelyMonitoredDesktop & ) override {}
    virtual void draw(const RDP::RAIL::NonMonitoredDesktop & ) override {}



    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //----------------------------------
    // MAIN WINDOW MANAGEMENT FUNCTIONS
    //----------------------------------

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

    void reset_cache(const int w,  const int h) {

        LOG(LOG_INFO, "reset_cache w=%d h=%d", w, h);
        if (w == 0 || h == 0) {
            return;
        }
        if (this->painter.isActive()) {
            this->painter.end();
        }

        this->cache = QPixmap(w, h);

        if (!(this->cache.isNull())) {
            this->painter.begin(&(this->cache));
        }

        this->painter.fillRect(0, 0, w, h, Qt::black);
    }

    void create_screen() {
        QPixmap * map = &(this->cache);
        this->screen = new RDPQtScreen(&(this->config->windowsData), this->controller, map, this->config->is_spanning, this->config->target_IP);
    }

    void create_replay_screen() {
        QPixmap * map = &(this->cache);

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
        this->remote_app_screen_map.insert(std::pair<uint32_t, RemoteAppQtScreen *>(id, nullptr));
        this->remote_app_screen_map[id] = new RemoteAppQtScreen(&(this->config->windowsData), this->controller, w, h, x, y, &(this->cache));
        LOG(LOG_INFO, "create_remote_app_screen 2");
    }

    void show_screen(uint32_t id) override {
        if (this->form) {
            this->form->hide();
            if (this->remote_app_screen_map[id]) {
                this->remote_app_screen_map[id]->show();
            }
        }
    }

    void move_screen(uint32_t id, int x, int y) override {
        if (this->remote_app_screen_map[id]) {
            this->remote_app_screen_map[id]->move(x, y);
        }
    }

    void set_screen_size(uint32_t id, int x, int y) override {
        if (this->remote_app_screen_map[id]) {
            this->remote_app_screen_map[id]->setFixedSize(x, y);
        }
    }

    void set_pixmap_shift(uint32_t id, int x, int y) override {
        if (this->remote_app_screen_map[id]) {
            this->remote_app_screen_map[id]->x_pixmap_shift = x;
            this->remote_app_screen_map[id]->y_pixmap_shift = y;
        }
    }

    int get_visible_width(uint32_t id) override {
        return this->remote_app_screen_map[id]->width();
    }

    int get_visible_height(uint32_t id) override {
        return this->remote_app_screen_map[id]->height();
    }

    int get_mem_width(uint32_t id) override {
        return this->remote_app_screen_map[id]->_width;
    }

    int get_mem_height(uint32_t id) override {
        return this->remote_app_screen_map[id]->_height;
    }

    void set_mem_size(uint32_t id, int w, int h) override {
        this->remote_app_screen_map[id]->_width = w;
        this->remote_app_screen_map[id]->_height = h;
    }

//     void dropScreen(uint32_t id) override {
//         if (this->remote_app_screen_map[id] != nullptr) {
//             this->remote_app_screen_map[id]->disconnection();
//             this->remote_app_screen_map[id] = nullptr;
//         }
//
//         std::map<uint32_t, RemoteAppQtScreen *>::iterator it = this->remote_app_screen_map.find(id);
//         this->remote_app_screen_map.erase (it);
//     }

    void clear_remote_app_screen() override {
        for (std::map<uint32_t, RemoteAppQtScreen *>::iterator it=this->remote_app_screen_map.begin(); it!=this->remote_app_screen_map.end(); ++it) {
            if (it->second) {
                it->second->disconnection();
                it->second = nullptr;
            }
        }
        this->remote_app_screen_map.clear();

        if (this->form) {
            this->form->show();
        }
    }

    void begin_update() override {

        this->update_counter++;
    }

public:
    size_t update_counter = 0;

    void end_update() override {
        assert(this->update_counter);
        this->update_counter--;
        if (this->update_counter != 0){
            return;
        }

        if (this->config->mod_state == ClientRedemptionConfig::MOD_RDP_REMOTE_APP) {
            for (std::map<uint32_t, RemoteAppQtScreen *>::iterator it=this->remote_app_screen_map.begin(); it!=this->remote_app_screen_map.end(); ++it) {
                if (it->second) {
                    it->second->update_view();
                }
            }
        } else {
            if (this->screen != nullptr) {
                this->screen->update_view();
            }
        }
    }

    FrontAPI::ResizeResult server_resize(const int width, const int height, const BitsPerPixel bpp) {

        if (width == 0 || height == 0) {
            return FrontAPI::ResizeResult::fail;
        }

        switch (this->config->mod_state) {

            case ClientRedemptionConfig::MOD_RDP:
                if (this->config->info.screen_info.width == width && this->config->info.screen_info.height == height) {
                    return FrontAPI::ResizeResult::instant_done;
                }
                this->dropScreen();
                this->reset_cache(width, height);
                this->screen = new RDPQtScreen(&(this->config->windowsData), this->controller, &(this->cache), this->config->is_spanning, this->config->target_IP);
                this->screen->show();
                    break;

            case ClientRedemptionConfig::MOD_VNC:
                if (this->config->modVNCParamsData.width == width && this->config->modVNCParamsData.height == height) {
                    return FrontAPI::ResizeResult::instant_done;
                }
                this->config->modVNCParamsData.width = width;
                this->config->modVNCParamsData.height = height;
                this->dropScreen();
                this->reset_cache(width, height);
                this->screen = new RDPQtScreen(&(this->config->windowsData), this->controller, &(this->cache), this->config->is_spanning, this->config->target_IP);
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

                    this->reset_cache(width, height);

                    if (!this->config->is_pre_loading) {

                        this->screen = new ReplayQtScreen(this->controller, &(this->cache), ClientConfig::get_movie_time_length(this->config->_movie_full_path.c_str()), current_time_movie, &(this->config->windowsData), this->config->_movie_name);

                        this->screen->show();
                    }
                }
                return FrontAPI::ResizeResult::instant_done;
                    break;
        }

        this->config->info.screen_info.bpp = bpp;

        return FrontAPI::ResizeResult::instant_done;
    }

    void set_pointer(Pointer const & cursor) override {

        auto dimensions = cursor.get_dimensions();
        auto hotspot = cursor.get_hotspot();

        ARGB32Pointer vnccursor(cursor);
        const auto av_alpha_q = vnccursor.get_alpha_q();

        //::hexdump(av_alpha_q.data(), dimensions.width * dimensions.height, dimensions.width);

        // this->cursor_image is used when client is replaying
        this->cursor_image = QImage(av_alpha_q.data(), dimensions.width, dimensions.height, dimensions.width * 4, QImage::Format_ARGB32_Premultiplied);

        if (this->config->mod_state == ClientRedemptionConfig::MOD_RDP_REMOTE_APP) {
            for (std::map<uint32_t, RemoteAppQtScreen *>::iterator it=this->remote_app_screen_map.begin(); it!=this->remote_app_screen_map.end(); ++it) {
                if (it->second) {
                    it->second->setCursor(QCursor(QPixmap::fromImage(this->cursor_image), hotspot.x, hotspot.x));
                }
            }
        } else if (this->screen) {
            this->screen->setCursor(QCursor(QPixmap::fromImage(this->cursor_image), hotspot.x, hotspot.x));
        }
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
            this->controller->instant_play_client(std::chrono::microseconds(endin_frame*1000000));
            this->balises.push_back(this->cache);
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

//     void answer_question(int color) {
//         QImage image = this->cache.toImage();
//
//         QRgb asked(color);
//
//         QRgb top_left  = image.pixel(0, 0);
//         QRgb top_right = image.pixel(this->config->info.width-1, 0);
//         QRgb bot_left  = image.pixel(0, this->config->info.height-1);
//         QRgb bot_right = image.pixel(this->config->info.width-1, this->config->info.height-1);
//
//         //LOG(LOG_INFO, "         top_left = 0x%04x top_right = 0x%04x bot_left = 0x%04x bot_right = 0x%04x asked_color = 0x%04x, top_left, top_right, bot_left, bot_right, asked);
//
//         if        (top_left == asked) {
//             this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, 0, 0, &(this->keymap));
//             this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1, 0, 0, &(this->keymap));
//             this->wab_diag_question = false;
// //             LOG(LOG_INFO, "CLIENT >> answer_question top_left");
//
//         } else if (top_right == asked) {
//             this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, this->config->info.width-1, 0, &(this->keymap));
//             this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1, this->config->info.width-1, 0, &(this->keymap));
//             this->wab_diag_question = false;
// //             LOG(LOG_INFO, "CLIENT >> answer_question top_right");
//
//         } else if (bot_left == asked) {
//             this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, 0, this->config->info.height-1, &(this->keymap));
//             this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1, 0, this->config->info.height-1, &(this->keymap));
//             this->wab_diag_question = false;
// //             LOG(LOG_INFO, "CLIENT >> answer_question bot_left");
//
//         } else if (bot_right == asked) {
//             this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, this->config->info.width-1, this->config->info.height-1, &(this->keymap));
//             this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1, this->config->info.width-1, this->config->info.height-1, &(this->keymap));
//             this->wab_diag_question = false;
// //             LOG(LOG_INFO, "CLIENT >> answer_question bot_right");
//
//         }
//     }


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //-----------------------
    //   GRAPHIC FUNCTIONS
    //-----------------------

    struct Op_0x11 {
        uchar op(const uchar src, const uchar dst) const { // +------+-------------------------------+
             return ~(src | dst);                          // | 0x11 | ROP: 0x001100A6 (NOTSRCERASE) |
        }                                                  // |      | RPN: DSon                     |
    };                                                     // +------+-------------------------------+

    struct Op_0x22 {
        uchar op(const uchar src, const uchar dst) const { // +------+-------------------------------+
             return (~src & dst);                          // | 0x22 | ROP: 0x00220326               |
        }                                                  // |      | RPN: DSna                     |
    };                                                     // +------+-------------------------------+

    struct Op_0x33 {
        uchar op(const uchar src, const uchar) const {     // +------+-------------------------------+
             return (~src);                                // | 0x33 | ROP: 0x00330008 (NOTSRCCOPY)  |
        }                                                  // |      | RPN: Sn                       |
    };                                                     // +------+-------------------------------+

    struct Op_0x44 {
        uchar op(const uchar src, const uchar dst) const { // +------+-------------------------------+
            return (src & ~dst);                           // | 0x44 | ROP: 0x00440328 (SRCERASE)    |
        }                                                  // |      | RPN: SDna                     |
    };                                                     // +------+-------------------------------+

    struct Op_0x55 {
        uchar op(const uchar, const uchar dst) const {     // +------+-------------------------------+
             return (~dst);                                // | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
        }                                                  // |      | RPN: Dn                       |
    };                                                     // +------+-------------------------------+

    struct Op_0x66 {
        uchar op(const uchar src, const uchar dst) const { // +------+-------------------------------+
            return (src ^ dst);                            // | 0x66 | ROP: 0x00660046 (SRCINVERT)   |
        }                                                  // |      | RPN: DSx                      |
    };                                                     // +------+-------------------------------+

    struct Op_0x77 {
        uchar op(const uchar src, const uchar dst) const { // +------+-------------------------------+
             return ~(src & dst);                          // | 0x77 | ROP: 0x007700E6               |
        }                                                  // |      | RPN: DSan                     |
    };                                                     // +------+-------------------------------+

    struct Op_0x88 {
        uchar op(const uchar src, const uchar dst) const { // +------+-------------------------------+
            return (src & dst);                            // | 0x88 | ROP: 0x008800C6 (SRCAND)      |
        }                                                  // |      | RPN: DSa                      |
    };                                                     // +------+-------------------------------+

    struct Op_0x99 {
        uchar op(const uchar src, const uchar dst) const { // +------+-------------------------------+
            return ~(src ^ dst);                           // | 0x99 | ROP: 0x00990066               |
        }                                                  // |      | RPN: DSxn                     |
    };                                                     // +------+-------------------------------+

    struct Op_0xBB {
        uchar op(const uchar src, const uchar dst) const { // +------+-------------------------------+
            return (~src | dst);                           // | 0xBB | ROP: 0x00BB0226 (MERGEPAINT)  |
        }                                                  // |      | RPN: DSno                     |
    };                                                     // +------+-------------------------------+

    struct Op_0xDD {
        uchar op(const uchar src, const uchar dst) const { // +------+-------------------------------+
            return (src | ~dst);                           // | 0xDD | ROP: 0x00DD0228               |
        }                                                  // |      | RPN: SDno                     |
    };                                                     // +------+-------------------------------+

    struct Op_0xEE {
        uchar op(const uchar src, const uchar dst) const { // +------+-------------------------------+
            return (src | dst);                            // | 0xEE | ROP: 0x00EE0086 (SRCPAINT)    |
        }                                                  // |      | RPN: DSo                      |
    };                                                     // +------+-------------------------------+


    QColor u32_to_qcolor(RDPColor color, gdi::ColorCtx color_ctx) {

        if (gdi::Depth::from_bpp(this->config->info.screen_info.bpp) != color_ctx.depth()) {
            BGRColor d = color_decode(color, color_ctx);
            color      = color_encode(d, this->config->info.screen_info.bpp);
        }

        BGRColor bgr = color_decode(color, this->config->info.screen_info.bpp, this->config->mod_palette);

        return {bgr.red(), bgr.green(), bgr.blue()};
    }

    template<class Op>
    void draw_memblt_op(const Rect & drect, const Bitmap & bitmap) {

//         LOG(LOG_INFO, "draw_memblt_op bitmap.cx()=%u this->cache.width()=%d drect.x=%u drect.cx=%u", bitmap.cx(), this->cache.width(), drect.x, drect.cx);
//         LOG(LOG_INFO, "draw_memblt_op bitmap.cy()=%u this->cache.height()=%d drect.y=%u drect.cy=%u", bitmap.cy(), this->cache.height(), drect.y, drect.cy);
//         const uint16_t mincx = std::min<int16_t>(bitmap.cx(), /*std::min<int16_t>(this->cache.width() - drect.x,*/ drect.cx/*)*/);
//         const uint16_t mincy = std::min<int16_t>(bitmap.cy(), /*std::min<int16_t>(this->cache.height() - drect.y,*/ drect.cy/*)*/);
//
//         LOG(LOG_INFO, "draw_memblt_op mincx=%u mincy=%u", mincx, mincy);
//
//         if (mincx <= 0 || mincy <= 0) {
//             return;
//         }
//         if (this->screen) {
//             QImage::Format format(this->bpp_to_QFormat(bitmap.bpp(), false)); //bpp
//
//             LOG(LOG_INFO,"draw_memblt_op mincx=%u mincy=%u bitmap.line_size=%zu x=%d y=%d bpp=%u", mincx, mincy, bitmap.line_size(), drect.x, drect.y, bitmap.bpp());
//             QImage srcBitmap(bitmap.data(), mincx, mincy, bitmap.line_size(), format);
//             if (bitmap.bpp() == 24) {
//                 srcBitmap = srcBitmap.rgbSwapped();
//             }
//             if (bitmap.bpp() != this->config->info.bpp) {
//                 srcBitmap = srcBitmap.convertToFormat(this->bpp_to_QFormat(this->config->info.bpp, false));
//             }
//             srcBitmap = srcBitmap.mirrored(false, true);
//             const uchar * srcData = srcBitmap.constBits();
//
//             QImage dstBitmap(this->cache.toImage().copy(drect.x, drect.y, mincx, mincy));
//             dstBitmap = dstBitmap.convertToFormat(srcBitmap.format());
//             const uchar * dstData = dstBitmap.constBits();
//
//             //bitmap.line_size() * mincy;
//             int data_len = (bitmap.bpp() * dstBitmap.width() * dstBitmap.height());
//             LOG(LOG_INFO, "memblt data_len = %d dstBitmap.width()=%d dstBitmap.height()=%d", data_len, dstBitmap.width(),  dstBitmap.height());
//             if (data_len <= 0) {
//                 LOG(LOG_INFO, "memblt data_len null");
//                 return;
//             }
//
//
//             std::unique_ptr<uchar[]> data = std::make_unique<uchar[]>(data_len);
//
//             Op op;
//             for (int i = 0; i < data_len; i++) {
//                 LOG(LOG_INFO, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 1 i=%d", i);
//                 uchar dst_res = dstData[i];
//                 LOG(LOG_INFO, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 2 i =%d", i);
//                 uchar src_res = srcData[i];
//                 LOG(LOG_INFO, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 3 i =%d", i);
//                 uchar res = op.op(src_res, dst_res);
//                 LOG(LOG_INFO, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 4 i =%d", i);
//                 data[i] = res;
//                 LOG(LOG_INFO, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 5 i =%d", i);
//             }
//
//             QImage image(data.get(), mincx, mincy, srcBitmap.format());
//             QRect trect(drect.x, drect.y, mincx, mincy);

//             if (this->config->connected || this->config->is_replaying) {
//                 this->painter.drawImage(trect, image);
//                 //this->painter.fillRect(trect, Qt::red);
//             }
//         }
    }

    void draw_MemBlt(const Rect & drect, const Bitmap & bitmap, bool invert, int srcx, int srcy) {
        const int16_t mincx = bitmap.cx();
        const int16_t mincy = bitmap.cy();

        if (mincx <= 0 || mincy <= 0) {
            return;
        }

        const unsigned char * row = bitmap.data();

        QImage qbitmap(row, mincx, mincy, this->bpp_to_QFormat(bitmap.bpp(), false));
        qbitmap = qbitmap.mirrored(false, true);
        qbitmap = qbitmap.copy(srcx, srcy, drect.cx, drect.cy);

        if (invert) {
            qbitmap.invertPixels();
        }

        if (bitmap.bpp() == BitsPerPixel{24}) {
            qbitmap = qbitmap.rgbSwapped();
        }

        const QRect trect(drect.x, drect.y, drect.cx, drect.cy);
        if (this->config->connected || this->config->is_replaying) {

             this->painter.drawImage(trect, qbitmap);
        }
    }


    void draw_RDPScrBlt(int srcx, int srcy, const Rect & drect, bool invert) {
        QImage qbitmap(this->cache.toImage().copy(srcx, srcy, drect.cx, drect.cy));
        if (invert) {
            qbitmap.invertPixels();
        }
        const QRect trect(drect.x, drect.y, drect.cx, drect.cy);
        if (this->config->connected || this->config->is_replaying) {
            this->painter.drawImage(trect, qbitmap);
        }
    }

    // TODO duplicated
    QImage::Format bpp_to_QFormat(BitsPerPixel bpp, bool alpha) {
        QImage::Format format(QImage::Format_RGB16);

        if (alpha) {

            switch (bpp) {
                case BitsPerPixel{15}: format = QImage::Format_ARGB4444_Premultiplied; break;
                case BitsPerPixel{16}: format = QImage::Format_ARGB4444_Premultiplied; break;
                case BitsPerPixel{24}: format = QImage::Format_ARGB8565_Premultiplied; break;
                case BitsPerPixel{32}: format = QImage::Format_ARGB32_Premultiplied;   break;
                default : break;
            }
        } else {

            switch (bpp) {
                case BitsPerPixel{15}: format = QImage::Format_RGB555; break;
                case BitsPerPixel{16}: format = QImage::Format_RGB16;  break;
                case BitsPerPixel{24}: format = QImage::Format_RGB888; break;
                case BitsPerPixel{32}: format = QImage::Format_RGB32;  break;
                default : break;
            }
        }

        return format;
    }

    void draw_RDPPatBlt(const Rect & rect, const QColor color, const QPainter::CompositionMode mode, const Qt::BrushStyle style) {
        QBrush brush(color, style);
        if (this->config->connected || this->config->is_replaying) {
            this->painter.setBrush(brush);
            this->painter.setCompositionMode(mode);
            this->painter.drawRect(rect.x, rect.y, rect.cx, rect.cy);
            this->painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            this->painter.setBrush(Qt::SolidPattern);
        }
    }

    void draw_RDPPatBlt(const Rect & rect, const QPainter::CompositionMode mode) {
        if (this->config->connected || this->config->is_replaying) {
            this->painter.setCompositionMode(mode);
            this->painter.drawRect(rect.x, rect.y, rect.cx, rect.cy);
            this->painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        }
    }

    void draw_frame(int frame_index) {
        if (this->config->is_replaying) {
            this->painter.drawPixmap(QPoint(0, 0), this->balises[frame_index], QRect(0, 0, this->cache.width(), this->cache.height()));
        }
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //-----------------------------
    //       DRAW FUNCTIONS
    //-----------------------------

    //using ClientRedemptionAPI::draw;

    void draw(const RDPPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override {

        const Rect rect = clip.intersect(this->cache.width(), this->cache.height()).intersect(cmd.rect);
        // this->setClip(rect.x, rect.y, rect.cx, rect.cy);

        QColor backColor = this->u32_to_qcolor(cmd.back_color, color_ctx);
        QColor foreColor = this->u32_to_qcolor(cmd.fore_color, color_ctx);

        if (cmd.brush.style == 0x03 && (cmd.rop == 0xF0 || cmd.rop == 0x5A)) { // external

            switch (cmd.rop) {

                // +------+-------------------------------+
                // | 0x5A | ROP: 0x005A0049 (PATINVERT)   |
                // |      | RPN: DPx                      |
                // +------+-------------------------------+
                case 0x5A:
                    {
                        QBrush brush(backColor, Qt::Dense4Pattern);
                        if (this->config->connected || this->config->is_replaying) {
                            this->painter.setBrush(brush);
                            this->painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
                            this->painter.drawRect(rect.x, rect.y, rect.cx, rect.cy);
                            this->painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                            this->painter.setBrush(Qt::SolidPattern);
                        }
                    }
                    break;

                // +------+-------------------------------+
                // | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
                // |      | RPN: P                        |
                // +------+-------------------------------+
                case 0xF0:
                    {
                        if (this->config->connected || this->config->is_replaying) {
                            QBrush brush(foreColor, Qt::Dense4Pattern);
                            this->painter.setPen(Qt::NoPen);
                            this->painter.fillRect(rect.x, rect.y, rect.cx, rect.cy, backColor);
                            this->painter.setBrush(brush);
                            this->painter.drawRect(rect.x, rect.y, rect.cx, rect.cy);
                            this->painter.setBrush(Qt::SolidPattern);
                        }
                    }
                    break;
                default: LOG(LOG_WARNING, "RDPPatBlt brush_style = 0x03 rop = %x", cmd.rop);
                    break;
            }

        } else {
            switch (cmd.rop) {

                case 0x00: // blackness
                    if (this->config->connected || this->config->is_replaying) {
                        this->painter.fillRect(rect.x, rect.y, rect.cx, rect.cy, Qt::black);
                    }
                    break;
                    // +------+-------------------------------+
                    // | 0x05 | ROP: 0x000500A9               |
                    // |      | RPN: DPon                     |
                    // +------+-------------------------------+

                    // +------+-------------------------------+
                    // | 0x0F | ROP: 0x000F0001               |
                    // |      | RPN: Pn                       |
                    // +------+-------------------------------+
                case 0x0F:
                    this->draw_RDPPatBlt(rect, QPainter::RasterOp_NotSource);
                    break;
                    // +------+-------------------------------+
                    // | 0x50 | ROP: 0x00500325               |
                    // |      | RPN: PDna                     |
                    // +------+-------------------------------+
                case 0x50:
                    this->draw_RDPPatBlt(rect, QPainter::RasterOp_NotSourceAndNotDestination);
                    break;
                    // +------+-------------------------------+
                    // | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
                    // |      | RPN: Dn                       |
                    // +------+-------------------------------+
                /*case 0x55:
                    this->draw_RDPPatBlt(rect, QPainter::RasterOp_NotDestination);

                    break;*/
                    // +------+-------------------------------+
                    // | 0x5A | ROP: 0x005A0049 (PATINVERT)   |
                    // |      | RPN: DPx                      |
                    // +------+-------------------------------+
                case 0x5A:
                    this->draw_RDPPatBlt(rect, QPainter::RasterOp_SourceXorDestination);
                    break;
                    // +------+-------------------------------+
                    // | 0x5F | ROP: 0x005F00E9               |
                    // |      | RPN: DPan                     |
                    // +------+-------------------------------+

                    // +------+-------------------------------+
                    // | 0xA0 | ROP: 0x00A000C9               |
                    // |      | RPN: DPa                      |
                    // +------+-------------------------------+
                case 0xA0:
                    this->draw_RDPPatBlt(rect, QPainter::RasterOp_SourceAndDestination);
                    break;
                    // +------+-------------------------------+
                    // | 0xA5 | ROP: 0x00A50065               |
                    // |      | RPN: PDxn                     |
                    // +------+-------------------------------+
                /*case 0xA5:
                    // this->draw_RDPPatBlt(rect, QPainter::RasterOp_NotSourceXorNotDestination);
                    break;*/
                    // +------+-------------------------------+
                    // | 0xAA | ROP: 0x00AA0029               |
                    // |      | RPN: D                        |
                    // +------+-------------------------------+
                case 0xAA: // change nothing
                    break;
                    // +------+-------------------------------+
                    // | 0xAF | ROP: 0x00AF0229               |
                    // |      | RPN: DPno                     |
                    // +------+-------------------------------+
                /*case 0xAF:
                    //this->draw_RDPPatBlt(rect, QPainter::RasterOp_NotSourceOrDestination);
                    break;*/
                    // +------+-------------------------------+
                    // | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
                    // |      | RPN: P                        |
                    // +------+-------------------------------+
                case 0xF0:
                    if (this->config->connected || this->config->is_replaying) {
                        this->painter.setPen(Qt::NoPen);
                        this->painter.fillRect(rect.x, rect.y, rect.cx, rect.cy, backColor);
                        this->painter.drawRect(rect.x, rect.y, rect.cx, rect.cy);
                    }
                    break;
                    // +------+-------------------------------+
                    // | 0xF5 | ROP: 0x00F50225               |
                    // |      | RPN: PDno                     |
                    // +------+-------------------------------+
                //case 0xF5:
                    //this->draw_RDPPatBlt(rect, QPainter::RasterOp_SourceOrNotDestination);
                    //break;
                    // +------+-------------------------------+
                    // | 0xFA | ROP: 0x00FA0089               |
                    // |      | RPN: DPo                      |
                    // +------+-------------------------------+
                case 0xFA:
                    this->draw_RDPPatBlt(rect, QPainter::RasterOp_SourceOrDestination);
                    break;

                case 0xFF: // whiteness
                    if (this->config->connected || this->config->is_replaying) {
                        this->painter.fillRect(rect.x, rect.y, rect.cx, rect.cy, Qt::white);
                    }
                    break;
                default: LOG(LOG_WARNING, "RDPPatBlt rop = %x", cmd.rop);
                    break;
            }
        }
    }


    void draw(const RDPOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override {

        if (this->config->connected || this->config->is_replaying) {
            QColor qcolor(this->u32_to_qcolor(cmd.color, color_ctx));
            Rect rect(cmd.rect.intersect(clip));
            // this->setClip(rect.x, rect.y, rect.cx, rect.cy);

            this->painter.fillRect(rect.x, rect.y, rect.cx, rect.cy, qcolor);
        }
    }


    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
        if (!bmp.is_valid()){
            return;
        }

        Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top,
                                (bitmap_data.dest_right - bitmap_data.dest_left + 1),
                                (bitmap_data.dest_bottom - bitmap_data.dest_top + 1));
        const Rect clipRect(0, 0, this->cache.width(), this->cache.height());
        const Rect drect = rectBmp.intersect(clipRect);

        const int16_t mincx = std::min<int16_t>(bmp.cx(), std::min<int16_t>(this->cache.width() - drect.x, drect.cx));
        const int16_t mincy = std::min<int16_t>(bmp.cy(), std::min<int16_t>(this->cache.height() - drect.y, drect.cy));;

        if (mincx <= 0 || mincy <= 0) {
            return;
        }

        QImage::Format format(this->bpp_to_QFormat(bmp.bpp(), false)); //bpp
        QImage qbitmap(bmp.data(), mincx, mincy, bmp.line_size(), format);

        if (bmp.bpp() == BitsPerPixel{24}) {
            qbitmap = qbitmap.rgbSwapped();
        }

        if (bmp.bpp() != this->config->info.screen_info.bpp) {
            qbitmap = qbitmap.convertToFormat(this->bpp_to_QFormat(this->config->info.screen_info.bpp, false));
            //LOG(LOG_INFO, "RDPBitmapData convertToFormat");
        }

        qbitmap = qbitmap.mirrored(false, true);
        QRect trect(drect.x, drect.y, mincx, mincy);
        if (this->config->connected || this->config->is_replaying) {
            // this->setClip(trect.x(), trect.y(), trect.width(), trect.height());
            this->painter.drawImage(trect, qbitmap);
        }
    }


    void draw(const RDPLineTo & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        (void) clip;

        // TODO clipping
        if (this->config->connected || this->config->is_replaying) {
            this->screen->setPenColor(this->u32_to_qcolor(cmd.back_color, color_ctx));
            // this->setClip(clip.x, clip.y, clip.cx, clip.cy);
            this->painter.drawLine(cmd.startx, cmd.starty, cmd.endx, cmd.endy);
        }
    }


    void draw(const RDPScrBlt & cmd, Rect clip) override {

        //std::cout << "RDPScrBlt" << std::endl;

        const Rect drect = clip.intersect(this->cache.width(), this->cache.height()).intersect(cmd.rect);
        if (drect.isempty()) {
            return;
        }
        // // this->setClip(drect.x, drect.y, drect.cx, drect.cy);

        int srcx(drect.x + cmd.srcx - cmd.rect.x);
        int srcy(drect.y + cmd.srcy - cmd.rect.y);

        switch (cmd.rop) {

            case 0x00:
                if (this->config->connected || this->config->is_replaying) {
                    this->painter.fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
                }
                break;

            case 0x55: this->draw_RDPScrBlt(srcx, srcy, drect, true);
                break;

            case 0xAA: // nothing to change
                break;

            case 0xCC: this->draw_RDPScrBlt(srcx, srcy, drect, false);
                break;

            case 0xFF:
                if (this->config->connected || this->config->is_replaying) {
                    this->painter.fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::white);
                }
                break;
            default: LOG(LOG_WARNING, "DEFAULT: RDPScrBlt rop = %x", cmd.rop);
                break;
        }
    }


    void draw(const RDPMemBlt & cmd, Rect clip, const Bitmap & bitmap) override {
        //std::cout << "RDPMemBlt (" << std::hex << static_cast<int>(cmd.rop) << ")" <<  std::dec <<  std::endl;
        const Rect drect = clip.intersect(cmd.rect);
        if (drect.isempty()){
            return ;
        }

//         if (this->cache.width() <= 0 || this->cache.height() <=  0) {
//             return;
//         }

        // // this->setClip(drect.x, drect.y, drect.cx, drect.cy);

        switch (cmd.rop) {

            case 0x00:
                if (this->config->connected || this->config->is_replaying) {
                    this->painter.fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
                }
                break;

            case 0x22: this->draw_memblt_op<Op_0x22>(drect, bitmap);
                break;

            case 0x33: this->draw_MemBlt(drect, bitmap, true, cmd.srcx + (drect.x - cmd.rect.x), cmd.srcy + (drect.y - cmd.rect.y));
                break;

            case 0x55: this->draw_memblt_op<Op_0x55>(drect, bitmap);
                break;

            case 0x66: this->draw_memblt_op<Op_0x66>(drect, bitmap);
                break;

            case 0x99: this->draw_memblt_op<Op_0x99>(drect, bitmap);
                break;

            case 0xAA:  // nothing to change
                break;

            case 0xBB: this->draw_memblt_op<Op_0xBB>(drect, bitmap);
                break;

            case 0xCC: this->draw_MemBlt(drect, bitmap, false, cmd.srcx + (drect.x - cmd.rect.x), cmd.srcy + (drect.y - cmd.rect.y));
                break;

            case 0xEE: this->draw_memblt_op<Op_0xEE>(drect, bitmap);
                break;

            case 0x88: this->draw_memblt_op<Op_0x88>(drect, bitmap);
                break;

            case 0xFF:
                if (this->config->connected || this->config->is_replaying) {
                    this->painter.fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::white);
                }
                break;

            default: LOG(LOG_WARNING, "DEFAULT: RDPMemBlt rop = %x", cmd.rop);
                break;
        }

    }


    void draw(const RDPMem3Blt & cmd, Rect clip, gdi::ColorCtx color_ctx, const Bitmap & bitmap) override {
//         LOG(LOG_INFO, "RDPMem3Blt    !!!!!!!!!!!!!!!!!");
        const Rect drect = clip.intersect(cmd.rect);
        if (drect.isempty()){
            return ;
        }

        switch (cmd.rop) {
            case 0xB8:
            {
                const uint16_t mincx = std::min<int16_t>(bitmap.cx(), std::min<int16_t>(this->screen->_width - drect.x, drect.cx));
                const uint16_t mincy = std::min<int16_t>(bitmap.cy(), std::min<int16_t>(this->screen->_height - drect.y, drect.cy))-2;

                if (mincx <= 0 || mincy <= 0) {
                    LOG(LOG_INFO, "RDPMem3Blt    null");
                    return;
                }

                const QColor fore(this->u32_to_qcolor(cmd.fore_color, color_ctx));
                const uint8_t r(fore.red());
                const uint8_t g(fore.green());
                const uint8_t b(fore.blue());

                const QImage::Format format(this->bpp_to_QFormat(bitmap.bpp(), true));

                if (this->config->connected || this->config->is_replaying) {

                    QImage srcBitmap(bitmap.data(), mincx, mincy, bitmap.line_size(), format);
                    srcBitmap = srcBitmap.convertToFormat(QImage::Format_RGB888);
                    srcBitmap = srcBitmap.mirrored(false, true);
                    if (bitmap.bpp() == BitsPerPixel{24}) {
                        srcBitmap = srcBitmap.rgbSwapped();
                    }
                    const uchar * srcData = srcBitmap.bits();

                    QImage dstBitmap(this->cache.toImage().copy(drect.x, drect.y, mincx, mincy));
                    dstBitmap = dstBitmap.convertToFormat(QImage::Format_RGB888);
                    const uchar * dstData = dstBitmap.bits();

                    const int data_size(3*dstBitmap.width()*dstBitmap.height());
                    std::unique_ptr<uchar[]> data = std::make_unique<uchar[]>(data_size);

                    for (int k = 0 ; k < data_size-2; k += 3) {
                            data[k  ] = ((dstData[k  ] ^ r) & srcData[k  ]) ^ r;
                            data[k+1] = ((dstData[k+1] ^ g) & srcData[k+1]) ^ g;
                            data[k+2] = ((dstData[k+2] ^ b) & srcData[k+2]) ^ b;
                    }

                    QImage image(data.get(), mincx, mincy, srcBitmap.format());
                    if (image.depth() != safe_cast<int>(this->config->info.screen_info.bpp)) {
                        image = image.convertToFormat(this->bpp_to_QFormat(this->config->info.screen_info.bpp, false));
                    }
                    QRect trect(drect.x, drect.y, mincx, mincy-1);

                    this->painter.drawImage(trect, image);
                }
            }
            break;

            default: LOG(LOG_WARNING, "DEFAULT: RDPMem3Blt rop = %x", cmd.rop);
            break;
        }
    }


    void draw(const RDPDestBlt & cmd, Rect clip) override {

        const Rect drect = clip.intersect(this->cache.width(), this->cache.height()).intersect(cmd.rect);
        // // this->setClip(drect.x, drect.y, drect.cx, drect.cy);

        switch (cmd.rop) {
            case 0x00: // blackness
                if (this->config->connected || this->config->is_replaying) {
                    this->painter.fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
                }
                break;
            case 0x55:                                         // inversion
                this->draw_RDPScrBlt(drect.x, drect.y, drect, true);
                break;
            case 0xAA: // change nothing
                break;
            case 0xFF: // whiteness
                if (this->config->connected || this->config->is_replaying) {
                    this->painter.fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::white);
                }
                break;
            default: LOG(LOG_WARNING, "DEFAULT: RDPDestBlt rop = %x", cmd.rop);
                break;
        }
    }

    void draw(const RDPMultiDstBlt & cmd, Rect clip) override {
        (void) cmd;
        (void) clip;

        LOG(LOG_WARNING, "DEFAULT: RDPMultiDstBlt");
    }

    void draw(const RDPMultiOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        (void) cmd;
        (void) clip;
        (void) color_ctx;

        LOG(LOG_WARNING, "DEFAULT: RDPMultiOpaqueRect");
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        (void) color_ctx;
        (void) cmd;
        (void) clip;
        LOG(LOG_WARNING, "DEFAULT: RDPMultiPatBlt");
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) override {
        (void) cmd;
        (void) clip;

        LOG(LOG_WARNING, "DEFAULT: RDPMultiScrBlt");
    }

    void draw(const RDPGlyphIndex & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) override {

        Rect screen_rect = clip.intersect(this->cache.width(), this->cache.height());
        if (screen_rect.isempty()){
            return ;
        }
        // this->setClip(screen_rect.x, screen_rect.y, screen_rect.cx, screen_rect.cy);

        Rect const clipped_glyph_fragment_rect = cmd.bk.intersect(screen_rect);
        if (clipped_glyph_fragment_rect.isempty()) {
            return;
        }

        // set a background color
        {
            Rect ajusted = cmd.f_op_redundant ? cmd.bk : cmd.op;
            if ((ajusted.cx > 1) && (ajusted.cy > 1)) {
                ajusted.cy--;
                ajusted = ajusted.intersect(screen_rect);
                 if (this->config->connected || this->config->is_replaying) {
                    this->painter.fillRect(ajusted.x, ajusted.y, ajusted.cx, ajusted.cy, this->u32_to_qcolor(cmd.fore_color, color_ctx));
                 }
            }
        }

        bool has_delta_bytes = (!cmd.ui_charinc && !(cmd.fl_accel & 0x20));

        const QColor color = this->u32_to_qcolor(cmd.back_color, color_ctx);
        const int16_t offset_y = /*cmd.bk.cy - (*/cmd.glyph_y - cmd.bk.y/* + 1)*/;
        const int16_t offset_x = cmd.glyph_x - cmd.bk.x;

        uint16_t draw_pos = 0;

        InStream variable_bytes(cmd.data, cmd.data_len);

        //uint8_t const * fragment_begin_position = variable_bytes.get_current();

        while (variable_bytes.in_remain()) {
            uint8_t data = variable_bytes.in_uint8();

            if (data <= 0xFD) {
                FontChar const & fc = gly_cache.glyphs[cmd.cache_id][data].font_item;
                if (!fc)
                {
                    LOG( LOG_INFO
                        , "RDPDrawable::draw_VariableBytes: Unknown glyph, cacheId=%u cacheIndex=%u"
                        , cmd.cache_id, data);
                    assert(fc);
                }

                if (has_delta_bytes)
                {
                    data = variable_bytes.in_uint8();
                    if (data == 0x80)
                    {
                        draw_pos += variable_bytes.in_uint16_le();
                    }
                    else
                    {
                        draw_pos += data;
                    }
                }

                if (fc)
                {
                    const int16_t x = draw_pos + cmd.bk.x + offset_x;
                    const int16_t y = offset_y + cmd.bk.y;
                    if (Rect(0,0,0,0) != clip.intersect(Rect(x, y, fc.incby, fc.height))){

                        const uint8_t * fc_data            = fc.data.get();
                        for (int yy = 0 ; yy < fc.height; yy++)
                        {
                            uint8_t   fc_bit_mask        = 128;
                            for (int xx = 0 ; xx < fc.width; xx++)
                            {
                                if (!fc_bit_mask)
                                {
                                    fc_data++;
                                    fc_bit_mask = 128;
                                }
                                if (clip.contains_pt(x + fc.offsetx + xx, y + fc.offsety + yy)
                                && (fc_bit_mask & *fc_data))
                                {
                                    if (this->config->connected || this->config->is_replaying) {
                                        this->painter.fillRect(x + fc.offsetx + xx, y + fc.offsety + yy, 1, 1, color);
                                    }
                                }
                                fc_bit_mask >>= 1;
                            }
                            fc_data++;
                        }
                    }
                } else {
                    LOG(LOG_WARNING, "DEFAULT: RDPGlyphIndex glyph_cache unknow FontChar");
                }

                if (cmd.ui_charinc) {
                    draw_pos += cmd.ui_charinc;
                }

            } else {
                LOG(LOG_WARNING, "DEFAULT: RDPGlyphIndex glyph_cache 0xFD");
            }
        }
        //this->draw_VariableBytes(cmd.data, cmd.data_len, has_delta_bytes,
            //draw_pos, offset_y, color, cmd.bk.x + offset_x, cmd.bk.y,
            //clipped_glyph_fragment_rect, cmd.cache_id, gly_cache);
    }

    void draw(const RDPPolygonSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        (void) cmd;
        (void) clip;
        (void) color_ctx;
        LOG(LOG_WARNING, "DEFAULT: RDPPolygonSC");

        /*RDPPolygonSC new_cmd24 = cmd;
        new_cmd24.BrushColor  = color_decode_opaquerect(cmd.BrushColor,  this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPPolygonCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {

        (void) cmd;
        (void) clip;
        (void) color_ctx;
        LOG(LOG_WARNING, "DEFAULT: RDPPolygonCB");

        /*RDPPolygonCB new_cmd24 = cmd;
        new_cmd24.foreColor  = color_decode_opaquerect(cmd.foreColor,  this->mod_bpp, this->mod_palette);
        new_cmd24.backColor  = color_decode_opaquerect(cmd.backColor,  this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPPolyline & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        (void) clip;
        (void) cmd;
        (void) color_ctx;
        LOG(LOG_WARNING, "DEFAULT: RDPPolyline");
        /*RDPPolyline new_cmd24 = cmd;
        new_cmd24.PenColor  = color_decode_opaquerect(cmd.PenColor,  this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPEllipseSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {

        (void) cmd;
        (void) clip;
        (void) color_ctx;
        LOG(LOG_WARNING, "DEFAULT: RDPEllipseSC");

        /*RDPEllipseSC new_cmd24 = cmd;
        new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPEllipseCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {

        (void) cmd;
        (void) clip;
        (void) color_ctx;
        LOG(LOG_WARNING, "DEFAULT: RDPEllipseCB");
    /*
        RDPEllipseCB new_cmd24 = cmd;
        new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
        new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDP::FrameMarker & order) override {
        (void) order;
        LOG(LOG_INFO, "DEFAULT: FrameMarker");
    }

    void draw(RDPNineGrid const & /*cmd*/, Rect /*clip*/, gdi::ColorCtx /*color_ctx*/, Bitmap const & /*bmp*/) override {
        LOG(LOG_INFO, "DEFAULT: RDPNineGrid");
    }

//     using ClientOutputGraphicAPI::draw;


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONTROLLERS
    //------------------------

    void close() {

        if (this->form != nullptr) {
            this->form->close();
        }

//         this->controller->disconnect(false);
    }

};
