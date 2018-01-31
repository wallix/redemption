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
   Author(s): Clément Moroldo

*/

#pragma once


#include "utils/log.hpp"


#include "../../client_input_output_api.hpp"


#include <QtCore/QTimer>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QImage>

#if REDEMPTION_QT_VERSION == 4
#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtGui/name>
#else
#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtWidgets/name>
#endif

#include REDEMPTION_QT_INCLUDE_WIDGET(QApplication)
#include REDEMPTION_QT_INCLUDE_WIDGET(QDesktopWidget)
#include REDEMPTION_QT_INCLUDE_WIDGET(QPushButton)
#include REDEMPTION_QT_INCLUDE_WIDGET(QLabel)
#include REDEMPTION_QT_INCLUDE_WIDGET(QToolTip)

#undef REDEMPTION_QT_INCLUDE_WIDGET


class Screen_Qt : public QWidget
{

Q_OBJECT

public:
    ClientRedemptionIOAPI * _front;
    ClientInputMouseKeyboardAPI * impl_input;
    int            _width;
    int            _height;
    QPushButton    _buttonCtrlAltDel;
    QPushButton    _buttonRefresh;
    QPushButton    _buttonDisconnexion;
    QColor         _penColor;
    QPixmap      * _cache;
    QPainter       _cache_painter;
//     QPixmap        _trans_cache;
//     QPainter       _trans_cache_painter;
//     QPixmap        _match_pixmap;
    bool           _connexionLasted;
    QTimer         _timer;
    QTimer         _timer_replay;
    uint8_t        _screen_index;

    bool           _running;
    std::string    _movie_name;
    std::string    _movie_dir;

    enum : int {
        BUTTON_HEIGHT = 20,
        READING_PANEL = 40,
        READING_BAR_H = 12,
        BALISED_FRAME = 15
    };

    uchar cursor_data[Pointer::DATA_SIZE*4];
    int cursorHotx;
    int cursorHoty;
    bool mouse_out;

    timeval movie_time_start;
    timeval movie_time_pause;
    bool is_paused;
    time_t movie_time;

    QLabel movie_status;
    QLabel movie_timer_label;
    QLabel video_timer_label;

    int begin;
    const int reading_bar_len;
    QPixmap readding_bar;
    time_t current_time_movie;
    time_t real_time_record;

    std::vector<QPixmap*> balises;

    int x_pixmap_shift;
    int y_pixmap_shift;



    static time_t get_movie_time_length(char const * mwrm_filename)
    {
        // TODO RZ: Support encrypted recorded file.
        CryptoContext cctx;
        Fstat fsats;
        InCryptoTransport trans(cctx, InCryptoTransport::EncryptionMode::NotEncrypted, fsats);
        MwrmReader mwrm_reader(trans);
        MetaLine meta_line;

        time_t start_time = 0;
        time_t stop_time = 0;

        trans.open(mwrm_filename);
        mwrm_reader.read_meta_headers();

        Transport::Read read_stat = mwrm_reader.read_meta_line(meta_line);
        if (read_stat == Transport::Read::Ok) {
            start_time = meta_line.start_time;
            stop_time = meta_line.stop_time;
            while (read_stat == Transport::Read::Ok) {
                stop_time = meta_line.stop_time;
                read_stat = mwrm_reader.read_meta_line(meta_line);
            }
        }

        return stop_time - start_time;
    }

public:
    Screen_Qt (ClientRedemptionIOAPI * front, ClientInputMouseKeyboardAPI * impl_input, std::string const & movie_dir, std::string const & movie_name, QPixmap * cache)
        : QWidget()
        , _front(front)
        , impl_input(impl_input)
        , _width(this->_front->info.width)
        , _height(this->_front->info.height)
        , _buttonCtrlAltDel("Play", this)
        , _buttonRefresh("Stop", this)
        , _buttonDisconnexion("Close", this)
        , _penColor(Qt::black)
        , _cache(cache)
        , _cache_painter(this->_cache)
//         , _trans_cache(this->_width, this->_height)
//         , _trans_cache_painter(&(this->_trans_cache))
//         , _match_pixmap(this->_width+2, this->_height+2)
        , _connexionLasted(false)
        , _timer_replay(this)
        , _screen_index(0)
        , _running(false)
        , _movie_name(movie_name)
        , _movie_dir(movie_dir)
        , cursorHotx(0)
        , cursorHoty(0)
        , mouse_out(false)
        , is_paused(false)
        , movie_time(get_movie_time_length(this->_front->replay_mod->get_mwrm_path().c_str()))
        , movie_status( QString("  Stop"), this)
        , movie_timer_label(" ", this)
        , video_timer_label(" ", this)
        , begin(0)
        , reading_bar_len(this->_width - 60)
        , readding_bar(this->reading_bar_len+10, READING_BAR_H)
        , current_time_movie(0)
        , real_time_record(this->_front->replay_mod->get_real_time_movie_begin())
    {
        std::string title = "Remote Desktop Player " + this->_movie_name;
        this->setWindowTitle(QString(title.c_str()));
        this->setAttribute(Qt::WA_DeleteOnClose);
//         this->_trans_cache.fill(Qt::transparent);
        this->paintCache().fillRect(0, 0, this->_width, this->_height, {0, 0, 0});

        if (this->_front->is_spanning) {
            this->setWindowState(Qt::WindowFullScreen);
        } else {
            this->setFixedSize(this->_width+2, this->_height + BUTTON_HEIGHT+READING_PANEL);
        }

        QPainter painter(&(this->readding_bar));
        painter.fillRect(0, 0, this->reading_bar_len+12, READING_BAR_H, this->palette().color(QWidget::backgroundRole()));

        QRect rectCtrlAltDel(QPoint(0, this->_height+READING_PANEL+1),QSize(this->_width/3, BUTTON_HEIGHT));
        this->_buttonCtrlAltDel.setToolTip(this->_buttonCtrlAltDel.text());
        this->_buttonCtrlAltDel.setGeometry(rectCtrlAltDel);
        this->_buttonCtrlAltDel.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonCtrlAltDel)     , SIGNAL (pressed()),  this, SLOT (playPressed()));
        this->_buttonCtrlAltDel.setFocusPolicy(Qt::NoFocus);

        QRect rectRefresh(QPoint(this->_width/3, this->_height+READING_PANEL+1),QSize(this->_width/3, BUTTON_HEIGHT));
        this->_buttonRefresh.setToolTip(this->_buttonRefresh.text());
        this->_buttonRefresh.setGeometry(rectRefresh);
        this->_buttonRefresh.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonRefresh), SIGNAL (pressed()), this, SLOT (stopRelease()));
        this->_buttonRefresh.setFocusPolicy(Qt::NoFocus);

        QRect rectDisconnexion(QPoint(((this->_width/3)*2), this->_height+READING_PANEL+1),QSize(this->_width-((this->_width/3)*2), BUTTON_HEIGHT));
        this->_buttonDisconnexion.setToolTip(this->_buttonDisconnexion.text());
        this->_buttonDisconnexion.setGeometry(rectDisconnexion);
        this->_buttonDisconnexion.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonDisconnexion), SIGNAL (released()), this, SLOT (closeReplay()));
        this->_buttonDisconnexion.setFocusPolicy(Qt::NoFocus);

        QRect rectMovieStatus(QPoint(0, this->_height+1),QSize(44, BUTTON_HEIGHT));
        this->movie_status.setGeometry(rectMovieStatus);
        this->movie_status.setFocusPolicy(Qt::NoFocus);

        QRect rectMovieTimer(QPoint(0, this->_height+21),QSize(280, BUTTON_HEIGHT));
        this->movie_timer_label.setGeometry(rectMovieTimer);
        this->movie_timer_label.setFocusPolicy(Qt::NoFocus);

        QRect rectVideoTimer(QPoint(this->_width-306, this->_height+21),QSize(300, BUTTON_HEIGHT));
        this->video_timer_label.setGeometry(rectVideoTimer);
        this->video_timer_label.setFocusPolicy(Qt::NoFocus);
        this->video_timer_label.setAlignment(Qt::AlignRight);

        std::string data(ctime(&(this->real_time_record)));
        QString movie_real_time = QString("  ") + QString(data.c_str());
        this->movie_timer_label.setText(movie_real_time);

        this->show_video_real_time_hms();

        QPen pen(Qt::black, 1);
        painter.setPen(pen);
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.addRoundedRect(QRectF(6, 0, this->reading_bar_len, READING_BAR_H), 6, 6);

        painter.fillPath(path, QColor(Qt::black));
        painter.drawPath(path);
        this->slotRepaint();

        if (this->_front->is_spanning) {
            this->move(0, 0);
        } else {
            if (this->_front->is_no_win_data()) {
                QDesktopWidget* desktop = QApplication::desktop();
                this->_front->windowsData.screen_x = (desktop->width()/2)  - (this->_width/2);
                this->_front->windowsData.screen_y = (desktop->height()/2) - (this->_height/2);
            }
            this->move(this->_front->windowsData.screen_x, this->_front->windowsData.screen_y);
        }

        this->QObject::connect(&(this->_timer_replay), SIGNAL (timeout()),  this, SLOT (playReplay()));

        this->setFocusPolicy(Qt::StrongFocus);

    }





    Screen_Qt (ClientRedemptionIOAPI * front, ClientInputMouseKeyboardAPI * impl_input, QPixmap * cache)
        : QWidget()
        , _front(front)
        , impl_input(impl_input)
        , _width(this->_front->info.width)
        , _height(this->_front->info.height)
        , _buttonCtrlAltDel("CTRL + ALT + DELETE", this)
        , _buttonRefresh("Refresh", this)
        , _buttonDisconnexion("Disconnection", this)
        , _penColor(Qt::black)
        , _cache(cache)
        , _cache_painter(this->_cache)
//         , _trans_cache(this->_width, this->_height)
//         , _match_pixmap(this->_width, this->_height)
        , _connexionLasted(false)
        , _screen_index(0)
        , _running(false)
        , cursorHotx(0)
        , cursorHoty(0)
        , mouse_out(false)
        , is_paused(false)
        , movie_time(0)
        , begin(0)
        , reading_bar_len(this->_width - 60)
        , readding_bar(this->reading_bar_len+12, READING_BAR_H)
        , current_time_movie(0)
        , real_time_record(0)
    {
        this->setMouseTracking(true);
        this->installEventFilter(this);
        this->setAttribute(Qt::WA_DeleteOnClose);
        std::string title = "Remote Desktop Player connected to [" + this->_front->target_IP +  "].";
        this->setWindowTitle(QString(title.c_str()));
//         this->_trans_cache.fill(Qt::transparent);

        if (this->_front->is_spanning) {
            this->setWindowState(Qt::WindowFullScreen);
        } else {
            this->setFixedSize(this->_width, this->_height + BUTTON_HEIGHT);
        }

        QPainter painter(&(this->readding_bar));
        painter.fillRect(0, 0, this->reading_bar_len+12, READING_BAR_H, this->palette().color(QWidget::backgroundRole()));

        QRect rectCtrlAltDel(QPoint(0, this->_height+1),QSize(this->_width/3, BUTTON_HEIGHT));
        this->_buttonCtrlAltDel.setToolTip(this->_buttonCtrlAltDel.text());
        this->_buttonCtrlAltDel.setGeometry(rectCtrlAltDel);
        this->_buttonCtrlAltDel.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonCtrlAltDel)  , SIGNAL (pressed()),  this, SLOT (CtrlAltDelPressed()));
        this->QObject::connect(&(this->_buttonCtrlAltDel)  , SIGNAL (released()), this, SLOT (CtrlAltDelReleased()));;
        this->_buttonCtrlAltDel.setFocusPolicy(Qt::NoFocus);

        QRect rectRefresh(QPoint(this->_width/3, this->_height+1),QSize(this->_width/3, BUTTON_HEIGHT));
        this->_buttonRefresh.setToolTip(this->_buttonRefresh.text());
        this->_buttonRefresh.setGeometry(rectRefresh);
        this->_buttonRefresh.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonRefresh)     , SIGNAL (pressed()),  this, SLOT (RefreshPressed()));
        this->_buttonRefresh.setFocusPolicy(Qt::NoFocus);

        QRect rectDisconnexion(QPoint(((this->_width/3)*2), this->_height+1),QSize(this->_width-((this->_width/3)*2), BUTTON_HEIGHT));
        this->_buttonDisconnexion.setToolTip(this->_buttonDisconnexion.text());
        this->_buttonDisconnexion.setGeometry(rectDisconnexion);
        this->_buttonDisconnexion.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonDisconnexion), SIGNAL (released()), this, SLOT (disconnexionRelease()));
        this->_buttonDisconnexion.setFocusPolicy(Qt::NoFocus);

        if (this->_front->is_spanning) {
            this->move(0, 0);
        } else {
            if (this->_front->is_no_win_data()) {
                QDesktopWidget* desktop = QApplication::desktop();
                this->_front->windowsData.screen_x = (desktop->width()/2)  - (this->_width/2);
                this->_front->windowsData.screen_y = (desktop->height()/2) - (this->_height/2);
            }
            this->move(this->_front->windowsData.screen_x, this->_front->windowsData.screen_y);
        }

        this->setFocusPolicy(Qt::StrongFocus);
    }

    Screen_Qt (ClientRedemptionIOAPI * front, ClientInputMouseKeyboardAPI * impl_input, int width, int height, int x, int y, QPixmap * cache)
        : QWidget()
        , _front(front)
        , impl_input(impl_input)
        , _width(width)
        , _height(height)
        , _penColor(Qt::black)
        , _cache(cache/*this->_front->screen_max_width, this->_front->screen_max_height*/)
        , _cache_painter(this->_cache)
//         , _trans_cache(this->_width, this->_height)
//         , _match_pixmap(cache->width(), cache->height())
        , _connexionLasted(false)
        , _screen_index(0)
        , _running(false)
        , cursorHotx(0)
        , cursorHoty(0)
        , mouse_out(false)
        , is_paused(false)
        , movie_time(0)
        , begin(0)
        , reading_bar_len(this->_width - 60)
        , readding_bar(this->reading_bar_len+12, READING_BAR_H)
        , current_time_movie(0)
        , real_time_record(0)
        , x_pixmap_shift(x)
        , y_pixmap_shift(y)
    {
        this->setMouseTracking(true);
        this->installEventFilter(this);
        this->setAttribute(Qt::WA_DeleteOnClose);
        std::string title = "Remote Desktop Player connected to [" + this->_front->target_IP +  "].";
        this->setWindowTitle(QString(title.c_str()));
        setWindowFlags( Qt::FramelessWindowHint );
        //this->setAttribute(Qt::WA_OutsideWSRange);
//         this->_trans_cache.fill(Qt::transparent);

        if (this->_front->is_spanning) {
            this->setWindowState(Qt::WindowFullScreen);
        } else {
            this->setFixedSize(this->_width, this->_height);
        }

        this->paintCache().fillRect(0, 0, this->_width, this->_height, Qt::red/*{0, 0, 0}*/);

//         QPainter painter(&(this->readding_bar));
//         painter.fillRect(0, 0, this->reading_bar_len+12, READING_BAR_H, this->palette().color(QWidget::backgroundRole()));

        this->move(this->x_pixmap_shift, this->y_pixmap_shift);

        this->setFocusPolicy(Qt::StrongFocus);
    }

    ~Screen_Qt() {

        QPoint points = this->mapToGlobal({0, 0});
        this->_front->windowsData.screen_x = points.x()-1;
        this->_front->windowsData.screen_y = points.y()-39;
        this->_front->writeWindowsConf();
        this->balises.clear();
        if (!this->_connexionLasted) {
            this->_front->closeFromScreen();
        }
    }

    void show_video_real_time() {

        struct timeval now = tvtime();
        time_t movie_time_tmp = this->current_time_movie;
        this->current_time_movie = now.tv_sec - this->movie_time_start.tv_sec + this->begin;

        if (this->current_time_movie > movie_time_tmp) {

            time_t current_real_time_record = real_time_record + this->current_time_movie;
            std::string data(ctime(&current_real_time_record));
            QString movie_real_time = QString("  ") + QString(data.c_str());
            this->movie_timer_label.setText(movie_real_time);

            this->show_video_real_time_hms();

            this->barRepaint(this->current_time_movie, QColor(Qt::green));
        }
    }

    QString toQStringData(time_t time) {
        int s = time;
        int h = s/3600;
        s = s % 3600;
        int m = s/60;
        s = s % 60;
        QString date_str;
        if (h) {
            date_str += QString(std::to_string(h).c_str()) + QString(":");
        }
        if (m < 10) {
            date_str += QString("0");
        }
        date_str += QString(std::to_string(m).c_str()) + QString(":");
        if (s < 10) {
            date_str += QString("0");
        }
        date_str += QString(std::to_string(s).c_str());

        return date_str;
    }

    void show_video_real_time_hms() {
        this->video_timer_label.setText( this->toQStringData(this->current_time_movie) + QString(" / ") + this->toQStringData(this->movie_time));
    }

    void set_mem_cursor(const uchar * data, int x, int y) {
        this->cursorHotx = x;
        this->cursorHoty = y;
        for (int i = 0; i < Pointer::DATA_SIZE*4; i++) {
            this->cursor_data[i] = data[i];
        }
        this->update_current_cursor();
    }

    void update_current_cursor() {
        QImage image(this->cursor_data, 32, 32, QImage::Format_ARGB32_Premultiplied);
        QPixmap map = QPixmap::fromImage(image);
        QCursor qcursor(map, this->cursorHotx, this->cursorHoty);

        this->setCursor(qcursor);
    }

    void update_view() {
        this->slotRepaint();
    }

    void disconnection() {
        this->_connexionLasted = true;
    }

    QPainter & paintCache() {
        return this->_cache_painter;
    }

//     QPainter & paintTransCache() {
//         return this->_trans_cache_painter;
//     }

    void paintEvent(QPaintEvent * event) override {
        Q_UNUSED(event);

        QPen                 pen;
        QPainter             painter;
        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(this->_penColor);
        painter.setPen(pen);
        painter.drawPixmap(QPoint(0, 0), *(this->_cache), QRect(this->x_pixmap_shift, this->y_pixmap_shift, this->_width, this->_height));
        painter.drawPixmap(QPoint(52, this->_height+4), this->readding_bar, QRect(0, 0, this->reading_bar_len+10, READING_BAR_H));
        painter.end();
    }

    void barRepaint(int len, QColor color) {

        double read_len_tmp = (len * this->reading_bar_len) / this->movie_time;
        int read_len = int(read_len_tmp);
        if (read_len > this->reading_bar_len) {
            read_len = this->reading_bar_len;
        }

        QPainter painter(&(this->readding_bar));
        QPen pen(Qt::black, 1);
        painter.setPen(pen);
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.addRoundedRect(QRectF(6, 0, read_len, READING_BAR_H), 6, 6);

        painter.fillPath(path, color);
        painter.drawPath(path);

        this->slotRepaint();
    }

    QPixmap * getCache() {
        return this->_cache;
    }

    void setPenColor(QColor color) {
        this->_penColor = color;
    }

    bool event(QEvent *event) override {
        if (this->_front->is_replaying) {
            QHelpEvent *helpEvent = static_cast<QHelpEvent*>( event );
            QRect bar_zone(44, this->_height+4, this->reading_bar_len, READING_BAR_H);
            int x = helpEvent->pos().x();
            int y = helpEvent->pos().y();
            if (x > 44 && x < this->reading_bar_len  && y >  this->_height+2 && y < this->_height + 14) {
                int bar_len = this->reading_bar_len;
                int bar_pos = x - 44;
                double read_len_tmp = (bar_pos * this->movie_time) / bar_len;

                this->setToolTip(this->toQStringData(int(read_len_tmp)));
            } else {
                QToolTip::hideText();
            }
        }

        return QWidget::event( event );
    }


private:
    void mousePressEvent(QMouseEvent *e) override {
        int x = e->x();
        int y = e->y();
        if (this->_front->is_replaying) {
            if (x > 44 && x < this->_width - 4  && y > this->_height+2 && y < this->_height + 14) {

                this->_timer_replay.stop();

                timeval now_stop = tvtime();

                int bar_click = x - 44;
                double read_len_tmp = (bar_click * this->movie_time) / this->reading_bar_len;
                this->begin = int(read_len_tmp);

                this->current_time_movie = this->begin;
                this->_running = true;
                this->is_paused = false;

                this->_cache_painter.fillRect(0, 0, this->_width, this->_height, Qt::black);
                this->_buttonCtrlAltDel.setText("Pause");
                this->movie_status.setText("  Play ");
                this->barRepaint(this->reading_bar_len, QColor(Qt::black));
                time_t current_real_time_record = real_time_record + this->current_time_movie;
                std::string data(ctime(&current_real_time_record));
                QString movie_real_time = QString("  ") + QString(data.c_str());
                this->movie_timer_label.setText(movie_real_time);
                this->show_video_real_time_hms();
                this->barRepaint(this->current_time_movie, QColor(Qt::green));
                this->slotRepainMatch();

                switch (this->_front->replay_mod->get_wrm_version()) {

                    case WrmVersion::v1:
                        if (this->_front->load_replay_mod(this->_movie_dir, this->_movie_name, {0, 0}, {0, 0})) {
                            this->_front->replay_mod->instant_play_client(std::chrono::microseconds(this->begin*1000000));
                            this->movie_time_start = tvtime();
                        }
                        break;

                    case WrmVersion::v2:
                    {
                        int last_balised = (this->begin/ BALISED_FRAME);
                        if (this->_front->load_replay_mod(this->_movie_dir, this->_movie_name, {last_balised * BALISED_FRAME, 0}, {0, 0})) {
                            this->_cache_painter.drawPixmap(QPoint(0, 0), *(this->balises[last_balised]), QRect(0, 0, this->_width, this->_height));
                            this->_front->replay_mod->instant_play_client(std::chrono::microseconds(this->begin*1000000));
                            this->slotRepainMatch();

                            this->movie_time_start = tvtime();
                            timeval waited_for_load = {this->movie_time_start.tv_sec - now_stop.tv_sec, this->movie_time_start.tv_usec - now_stop.tv_usec};
                            timeval wait_duration = {this->movie_time_start.tv_sec - this->begin - waited_for_load.tv_sec, this->movie_time_start.tv_usec - waited_for_load.tv_usec};
                            this->_front->replay_mod->set_wait_after_load_client(wait_duration);
                        }
                    }
                        break;

                }

                this->_timer_replay.start(4);
            }
        } else {

            int flag(0);
            switch (e->button()) {
                case Qt::LeftButton:  flag = MOUSE_FLAG_BUTTON1; break;
                case Qt::RightButton: flag = MOUSE_FLAG_BUTTON2; break;
                case Qt::MidButton:   flag = MOUSE_FLAG_BUTTON4; break;
                case Qt::XButton1:
                case Qt::XButton2:
                case Qt::NoButton:
                case Qt::MouseButtonMask:

                default: break;
            }

            int x = e->x();
            int y = e->y();

            if (this->_front->mod_state == ClientRedemptionIOAPI::MOD_RDP_REMOTE_APP) {
                QPoint mouseLoc = QCursor::pos();
                x = mouseLoc.x();
                y = mouseLoc.y();
            }

            this->_front->mouseButtonEvent(x, y, flag | MOUSE_FLAG_DOWN);
        }
    }

    void mouseReleaseEvent(QMouseEvent *e) override {
        int flag(0);
        switch (e->button()) {

            case Qt::LeftButton:  flag = MOUSE_FLAG_BUTTON1; break;
            case Qt::RightButton: flag = MOUSE_FLAG_BUTTON2; break;
            case Qt::MidButton:   flag = MOUSE_FLAG_BUTTON4; break;
            case Qt::XButton1:
            case Qt::XButton2:
            case Qt::NoButton:
            case Qt::MouseButtonMask:

            default: break;
        }

        int x = e->x();
        int y = e->y();

        if (this->_front->mod_state == ClientRedemptionIOAPI::MOD_RDP_REMOTE_APP) {
            QPoint mouseLoc = QCursor::pos();
            x = mouseLoc.x();
            y = mouseLoc.y();
        }

        this->_front->mouseButtonEvent(x, y, flag);
    }

    void keyPressEvent(QKeyEvent *e) override {
        this->impl_input->keyPressEvent(e->key(), e->text().toStdString()[0]);
    }

    void keyReleaseEvent(QKeyEvent *e) override {
        this->impl_input->keyReleaseEvent(e->key(), e->text().toStdString()[0]);
    }

    void wheelEvent(QWheelEvent *e) override {
//         int flag(MOUSE_FLAG_HWHEEL);
//         if (e->delta() < 0) {
//             flag = flag | MOUSE_FLAG_WHEEL_NEGATIVE;
//         }
        this->_front->wheelEvent(e->x(), e->y(), e->delta());
    }

    void enterEvent(QEvent *event) override {
        Q_UNUSED(event);
        //this->update_current_cursor();
        //this->_front->_current_screen_index =  this->_screen_index;
    }

    bool eventFilter(QObject *obj, QEvent *e) override {
        Q_UNUSED(obj);
        if (e->type() == QEvent::MouseMove)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);
            int x = std::max(0, mouseEvent->x());
            int y = std::max(0, mouseEvent->y());

//             if (y > this->info.height) {
//                 LOG(LOG_INFO, "eventFilter out");
//                 this->screen->mouse_out = true;
//             } else if (this->screen->mouse_out) {
//                 this->screen->update_current_cursor();
//                 this->screen->mouse_out = false;
//             }

//             if (this->mod != nullptr && this->mouse_data.y < this->info.height) {
            if (this->_front->mod_state == ClientRedemptionIOAPI::MOD_RDP_REMOTE_APP) {
                QPoint mouseLoc = QCursor::pos();
                x = mouseLoc.x();
                y = mouseLoc.y();

            }

//                 this->mod->rdp_input_mouse(MOUSE_FLAG_MOVE, this->mouse_data.x, this->mouse_data.y, &(this->keymap));
//             }
            this->_front->mouseMouveEvent(std::max(0, x), std::max(0, y) );
        }

        return false;
    }


public Q_SLOTS:
    void playPressed() {
        if (this->_running) {
            this->movie_time_pause = tvtime();
            this->_running = false;
            this->is_paused = true;
            this->_buttonCtrlAltDel.setText("Play");
            this->movie_status.setText(" Pause");
            this->_timer_replay.stop();
        } else {
            this->_running = true;
            if (this->is_paused) {
                timeval pause_duration = tvtime();
                pause_duration = {pause_duration.tv_sec - this->movie_time_pause.tv_sec, pause_duration.tv_usec - this->movie_time_pause.tv_usec};
                this->movie_time_start.tv_sec += pause_duration.tv_sec;
                this->_front->replay_mod->set_pause(pause_duration);

                this->is_paused = false;
            } else {
                this->begin = 0;
                this->barRepaint(this->reading_bar_len, QColor(Qt::black));
                this->movie_time_start = tvtime();
                this->_front->replay_mod->set_sync();
            }
            this->_buttonCtrlAltDel.setText("Pause");
            this->movie_status.setText("  Play ");

            this->_timer_replay.start(1);
        }
    }

    void playReplay() {

        this->show_video_real_time();

        if (!this->_front->replay_mod->get_break_privplay_client()) {
            if (!this->_front->replay_mod->play_client()) {
                this->slotRepainMatch();
            }
        }

        if (this->current_time_movie >= this->movie_time) {

            this->show_video_real_time();
            this->_timer_replay.stop();
            this->begin = 0;
            this->movie_time_start = {0, 0};
            this->movie_time_pause = {0, 0};
            this->current_time_movie = 0;
            this->_buttonCtrlAltDel.setText("Replay");
            this->movie_status.setText("  Stop ");
            this->_running = false;
            this->is_paused = false;
            this->_front->load_replay_mod(this->_movie_dir, this->_movie_name, {0, 0}, {0, 0});
        }
    }

    void closeReplay() {
        this->_front->delete_replay_mod();
        this->_front->disconnexionReleased();
    }

    void slotRepaint() {
//         QPainter match_painter(&(this->_match_pixmap));
//         match_painter.drawPixmap(QPoint(0, 0), *(this->_cache), QRect(0, 0, this->_cache->width(), this->_cache->height()));
        //match_painter.drawPixmap(QPoint(0, 0), *(this->_trans_cache), QRect(0, 0, this->_width, this->_height));
        this->repaint();
    }

    void slotRepainMatch() {
//         QPainter match_painter(&(this->_match_pixmap));
//         match_painter.drawPixmap(QPoint(0, 0), *(this->_cache), QRect(0, 0, this->_width, this->_height));
        //match_painter.drawPixmap(QPoint(0, 0), this->_trans_cache, QRect(0, 0, this->_width, this->_height));
        this->repaint();
    }

    void RefreshPressed() {
        this->_front->refreshPressed();
    }

    void CtrlAltDelPressed() {
        this->_front->CtrlAltDelPressed();
    }

    void CtrlAltDelReleased() {
        this->_front->CtrlAltDelReleased();
    }

    void stopRelease() {
        this->_timer_replay.stop();

        this->movie_time_start = {0, 0};
        this->movie_time_pause = {0, 0};
        this->begin = 0;
        this->_running = false;
        this->is_paused = false;

        this->_buttonCtrlAltDel.setText("Replay");
        this->movie_status.setText("  Stop ");
        this->barRepaint(this->reading_bar_len, QColor(Qt::black));
        this->current_time_movie = 0;
        this->show_video_real_time_hms();

        if (this->_front->load_replay_mod(this->_movie_dir, this->_movie_name, {0, 0}, {0, 0})) {
            this->_cache_painter.fillRect(0, 0, this->_width, this->_height, Qt::black);
            this->slotRepainMatch();
        }
    }

    void disconnexionRelease(){
        this->paintCache().end();
        this->_front->disconnexionReleased();
    }

//     void setMainScreenOnTopRelease() {
//         this->_front->setMainScreenOnTopRelease();
//     }
};