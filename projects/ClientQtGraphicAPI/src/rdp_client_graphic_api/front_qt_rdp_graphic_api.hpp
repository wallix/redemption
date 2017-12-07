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
   Author(s): Christophe Grosjean, Clément Moroldo
*/



#pragma once

#include "utils/log.hpp"

#ifndef Q_MOC_RUN
#include <stdio.h>
#include <openssl/ssl.h>
#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#include <boost/algorithm/string.hpp>

#include "core/report_message_api.hpp"
#include "acl/auth_api.hpp"
#include "utils/genfstat.hpp"
#include "core/front_api.hpp"
#include "mod/mod_api.hpp"
#include "mod/rdp/rdp_log.hpp"
#include "transport/socket_transport.hpp"
#include "keymaps/qt_scancode_keymap.hpp"
#include "core/client_info.hpp"
#include "mod/internal/replay_mod.hpp"
#include "configs/config.hpp"
#include "utils/bitmap.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "utils/netutils.hpp"
#include "utils/genrandom.hpp"
#include "keyboard/keymap2.hpp"
#include "transport/crypto_transport.hpp"

#include "capture/full_video_params.hpp"
#include "capture/video_params.hpp"
#include "capture/capture.hpp"
#include "core/RDP/MonitorLayoutPDU.hpp"
#include "core/channel_list.hpp"


#include "core/RDP/RDPDrawable.hpp"

#include <QtCore/QList>
#include <QtCore/QTimer>
#include <QtCore/QStringList>
#include <QtCore/QMimeData>
#include <QtCore/QSocketNotifier>
#include <QtGui/QBitmap>
#include <QtGui/QClipboard>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QRgb>
#include <QtGui/QWheelEvent>

#if REDEMPTION_QT_VERSION == 4
#   include <QtCore/QByteArray>
#   include <QtCore/QUrl>
#   include <QtCore/QtGlobal>

#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtGui/name>
#else
#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtWidgets/name>
#endif

#include REDEMPTION_QT_INCLUDE_WIDGET(QApplication)
#include REDEMPTION_QT_INCLUDE_WIDGET(QCheckBox)
#include REDEMPTION_QT_INCLUDE_WIDGET(QComboBox)
#include REDEMPTION_QT_INCLUDE_WIDGET(QDesktopWidget)
#include REDEMPTION_QT_INCLUDE_WIDGET(QDialog)
#include REDEMPTION_QT_INCLUDE_WIDGET(QFileDialog)
#include REDEMPTION_QT_INCLUDE_WIDGET(QFormLayout)
#include REDEMPTION_QT_INCLUDE_WIDGET(QGridLayout)
#include REDEMPTION_QT_INCLUDE_WIDGET(QLabel)
#include REDEMPTION_QT_INCLUDE_WIDGET(QLineEdit)
#include REDEMPTION_QT_INCLUDE_WIDGET(QProgressBar)
#include REDEMPTION_QT_INCLUDE_WIDGET(QPushButton)
#include REDEMPTION_QT_INCLUDE_WIDGET(QTabWidget)
#include REDEMPTION_QT_INCLUDE_WIDGET(QTableWidget)
#include REDEMPTION_QT_INCLUDE_WIDGET(QToolTip)
#include REDEMPTION_QT_INCLUDE_WIDGET(QWidget)

#undef REDEMPTION_QT_INCLUDE_WIDGET

#include <Phonon/AudioOutput>
#include <Phonon/MediaObject>

#endif

#define REPLAY_PATH "/replay"
#define LOGINS_PATH "/config/logins.config"
#define WINODW_CONF_PATH "/config/windows_config.config"

#ifndef MAIN_PATH
# error "undefined MAIN_PATH macro"
# define MAIN_PATH ""
#endif


// class DummyAuthentifier : public auth_api
// {
// public:
//     virtual void set_auth_channel_target(const char *) {}
//     virtual void set_auth_error_message(const char *) {}
//     virtual void report(const char * , const char *) {}
//     virtual void log4(bool , const char *, const char * = nullptr) {}
//     virtual void disconnect_target() {}
// };


class ProgressBarWindow;


class Front_Qt_API : public FrontAPI
{

public:
    RDPVerbose        verbose;
    ClientInfo        info;
    CryptoContext     cctx;
    std::string       user_name;
    std::string       user_password;
    std::string       target_IP;
    int               port;
    std::string       local_IP;
    int               fps = 0;


    int                        nbTry;
    int                        retryDelay;
    int                        delta_time;
    mod_api                  * mod;
    std::unique_ptr<ReplayMod> replay_mod;
    bool                 is_recording;
    bool                 is_replaying;
    bool                 connected;


    QImage::Format       imageFormatRGB;
    QImage::Format       imageFormatARGB;
    Qt_ScanCode_KeyMap   qtRDPKeymap;
    ProgressBarWindow  * bar;
    QPixmap            * cache;
    QPixmap            * cache_replay;
    SocketTransport    * socket;
    TimeSystem           timeSystem;
    NullAuthentifier    authentifier;
    NullReportMessage  reportMessage;
    bool                 is_spanning;
    Fstat fstat;


    const std::string    MAIN_DIR;
    const std::string    REPLAY_DIR;
    const std::string    USER_CONF_LOG;
    const std::string    WINDOWS_CONF;

    std::string _movie_name;
    std::string _movie_dir;

    bool wab_diag_question;
    int asked_color;



    struct WindowsData {
        int form_x = 0;
        int form_y = 0;
        int screen_x = 0;
        int screen_y = 0;

        bool no_data = true;

        Front_Qt_API * front;

        WindowsData(Front_Qt_API * front)
          : front(front)
        {}

        void open() {
            std::ifstream ifile(this->front->WINDOWS_CONF, std::ios::in);
            if (ifile) {
                this->no_data = false;

                std::string line;
                std::string delimiter = " ";

                while(std::getline(ifile, line)) {
                    auto pos(line.find(delimiter));
                    std::string tag  = line.substr(0, pos);
                    std::string info = line.substr(pos + delimiter.length(), line.length());

                    if (tag.compare(std::string("form_x")) == 0) {
                        this->form_x = std::stoi(info);
                    } else
                      if (tag.compare(std::string("form_y")) == 0) {
                        this->form_y = std::stoi(info);
                    } else
                    if (tag.compare(std::string("screen_x")) == 0) {
                        this->screen_x = std::stoi(info);
                    } else
                    if (tag.compare(std::string("screen_y")) == 0) {
                        this->screen_y = std::stoi(info);
                    }
                }

                ifile.close();
            }
        }

        void write() {
            std::ofstream ofile(this->front->WINDOWS_CONF, std::ios::out | std::ios::trunc);
            if (ofile) {

                ofile  << "form_x " << this->form_x <<  "\n";
                ofile  << "form_y " << this->form_y <<  "\n";
                ofile  << "screen_x " << this->screen_x <<  "\n";
                ofile  << "screen_y " << this->screen_y <<  "\n";

                ofile.close();
            }
        }

    } windowsData;


    Front_Qt_API( RDPVerbose verbose)
    : verbose(verbose)
    , info()
    , cctx()
    , port(0)
    , local_IP("unknow_local_IP")
    , nbTry(3)
    , retryDelay(1000)
    , delta_time(1000000)
    , mod(nullptr)
    , replay_mod(nullptr)
    , is_recording(false)
    , is_replaying(false)
    , connected(false)
    , qtRDPKeymap()
    , bar(nullptr)
    , cache(nullptr)
    , cache_replay(nullptr)
    , socket(nullptr)
    , is_spanning(false)
    , MAIN_DIR(MAIN_PATH)
    , REPLAY_DIR(MAIN_PATH REPLAY_PATH)
    , USER_CONF_LOG(MAIN_PATH LOGINS_PATH)
    , WINDOWS_CONF(MAIN_PATH WINODW_CONF_PATH)
    , wab_diag_question(false)
    , asked_color(0)
    , windowsData(this)
    {
        this->windowsData.open();
        std::fill(std::begin(this->info.order_caps.orderSupport), std::end(this->info.order_caps.orderSupport), 1);
        this->info.glyph_cache_caps.GlyphSupportLevel = GlyphCacheCaps::GLYPH_SUPPORT_FULL;
    }

    virtual void send_to_channel( const CHANNELS::ChannelDef & , uint8_t const *
                                , std::size_t , std::size_t , int ) override {}

    // CONTROLLER
    virtual void connexionReleased() = 0;
    virtual void closeFromScreen() = 0;
    virtual void RefreshPressed() = 0;
    virtual void CtrlAltDelPressed() = 0;
    virtual void CtrlAltDelReleased() = 0;
    virtual void disconnexionReleased() = 0;
    virtual void setMainScreenOnTopRelease() = 0;
    virtual void mousePressEvent(QMouseEvent *e, int screen_index) = 0;
    virtual void mouseReleaseEvent(QMouseEvent *e, int screen_index) = 0;
    virtual void keyPressEvent(QKeyEvent *e) = 0;
    virtual void keyReleaseEvent(QKeyEvent *e) = 0;
    virtual void wheelEvent(QWheelEvent *e) = 0;
    virtual bool eventFilter(QObject *obj, QEvent *e, int screen_index) = 0;
    virtual void disconnect(std::string const & txt) = 0;
    virtual void dropScreen() = 0;
    virtual bool is_no_win_data() = 0;
    virtual void writeWindowsConf() = 0;

    virtual void replay(std::string const & movie_dir, std::string const & movie_path) = 0;
    virtual bool load_replay_mod(std::string const & movie_dir, std::string const & movie_name, timeval begin_read, timeval end_read) = 0;
    virtual void delete_replay_mod() = 0;
    virtual void callback() = 0;

    virtual bool can_be_start_capture() override { return true; }

    virtual void options() {
        LOG(LOG_WARNING, "No options window implemented yet. Virtual function \"void options()\" must be override.");
    }

    virtual mod_api * init_mod() = 0;

};


class ProgressBarWindow : public QWidget {

Q_OBJECT

public:
    QProgressBar load_bar;
    Front_Qt_API * front;

    ProgressBarWindow(int maxVal, Front_Qt_API * front)
        : QWidget()
        , load_bar(this)
        , front(front)
    {
        this->setWindowTitle("Loading Movie");
        this->setAttribute(Qt::WA_DeleteOnClose);

        QRect rect(QPoint(0,0),QSize(600, 50));
        this->load_bar.setGeometry(rect);
        this->load_bar.setRange(0, maxVal);

        uint32_t centerW = 0;
        uint32_t centerH = 0;
        QDesktopWidget* desktop = QApplication::desktop();
        centerW = (desktop->width()/2)  - 300;
        centerH = (desktop->height()/2) - 25;
        this->move(centerW, centerH);

        this->show();
    }

    ~ProgressBarWindow() {}



    void setValue(int val) {
        this->load_bar.setValue(val);
        if (val >= this->load_bar.maximum()) {
            this->close();
        }
    }
};


class Mod_Qt : public QObject
{

Q_OBJECT

public:
    Front_Qt_API              * _front;
    QSocketNotifier           * _sckRead;
    mod_api                   * _callback;
    SocketTransport           * _sck;
    int                         _client_sck;
    std::string error_message;

    QTimer timer;


    Mod_Qt(Front_Qt_API * front, QWidget * parent)
        : QObject(parent)
        , _front(front)
        , _sckRead(nullptr)
        , _callback(nullptr)
        , _sck(nullptr)
        , _client_sck(0)
        , timer(this)
    {}

    ~Mod_Qt() {
        this->disconnect(true);
    }


    void disconnect(bool is_pipe_ok) {

        if (this->_sckRead != nullptr) {
            delete (this->_sckRead);
            this->_sckRead = nullptr;
        }

        if (this->_callback != nullptr) {
            TimeSystem timeobj;
            if (is_pipe_ok) {
                this->_callback->disconnect(timeobj.get_time().tv_sec);
            }
//             delete (this->_callback);
            this->_callback = nullptr;
            this->_front->mod = nullptr;
        }

        if (this->_sck != nullptr) {
            delete (this->_sck);
            this->_sck = nullptr;
            LOG(LOG_INFO, "Disconnected from [%s].", this->_front->target_IP.c_str());
        }
    }

    bool connect() {
        const char * name(this->_front->user_name.c_str());
        const char * targetIP(this->_front->target_IP.c_str());
        const std::string errorMsg("Cannot connect to [" + this->_front->target_IP +  "].");

        unique_fd client_sck = ip_connect(targetIP, this->_front->port, this->_front->nbTry, this->_front->retryDelay);
        this->_client_sck = client_sck.fd();

        if (this->_client_sck > 0) {
            try {

                this->_sck = new SocketTransport( name
                                                , std::move(client_sck)
                                                , targetIP
                                                , this->_front->port
                                                , std::chrono::milliseconds(1000)
                                                , to_verbose_flags(0)
                                                //, SocketTransport::Verbose::dump
                                                , &this->error_message
                                                );

                this->_front->socket = this->_sck;
                LOG(LOG_INFO, "Connected to [%s].", targetIP);

            } catch (const std::exception &) {
                std::string windowErrorMsg(errorMsg+" Socket error.");
                LOG(LOG_WARNING, "%s", windowErrorMsg.c_str());
                this->_front->disconnect("<font color='Red'>"+windowErrorMsg+"</font>");
                return false;
            }

        } else {
            std::string windowErrorMsg(errorMsg+" Invalid ip or port.");
            LOG(LOG_WARNING, "%s", windowErrorMsg.c_str());
            this->_front->disconnect("<font color='Red'>"+windowErrorMsg+"</font>");
            return false;
        }

        return true;
    }

    bool listen() {

        this->_callback = this->_front->init_mod();

        if (this->_callback !=  nullptr) {
            this->_sckRead = new QSocketNotifier(this->_client_sck, QSocketNotifier::Read, this);
            this->QObject::connect(this->_sckRead,   SIGNAL(activated(int)), this,  SLOT(call_draw_event_data()));
            this->QObject::connect(&(this->timer),   SIGNAL(timeout()), this,  SLOT(call_draw_event_timer()));
            if (this->_callback) {
                if (this->_callback->get_event().is_trigger_time_set()) {
                    struct timeval now = tvtime();
                    int time_to_wake = (this->_callback->get_event().get_trigger_time().tv_usec - now.tv_usec) / 1000
                    + (this->_callback->get_event().get_trigger_time().tv_sec - now.tv_sec) * 1000;
//                     this->_callback->get_event().reset_trigger_time();

                    if (time_to_wake < 0) {
                        this->timer.stop();
                    } else {

                        this->timer.start( time_to_wake );
                    }
                }
            } else {
                const std::string errorMsg("Error: Mod Initialization failed.");
                std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");
                this->_front->dropScreen();
                this->_front->disconnect(labelErrorMsg);
                return false;
            }

        } else {
            const std::string errorMsg("Error: Mod Initialization failed.");
            std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");
            this->_front->dropScreen();
            this->_front->disconnect(labelErrorMsg);
            return false;
        }
        return true;
    }



public Q_SLOTS:
    void call_draw_event_data() {
        this->_callback->get_event().set_waked_up_by_time(false);
        this->call_draw_event();
    }

    void call_draw_event_timer() {
        this->_callback->get_event().set_waked_up_by_time(true);
        this->call_draw_event();
    }

    void call_draw_event() {
        if (this->_front->mod) {

            this->_front->callback();

            if (this->_callback) {
                if (this->_callback->get_event().is_trigger_time_set()) {
                    struct timeval now = tvtime();
                    int time_to_wake = ((this->_callback->get_event().get_trigger_time().tv_usec - now.tv_usec) / 1000)
                    + ((this->_callback->get_event().get_trigger_time().tv_sec - now.tv_sec) * 1000);

                    this->_callback->get_event().reset_trigger_time();

                    if (time_to_wake < 0) {
                        this->timer.stop();
//                         LOG(LOG_INFO, "time_to_wake = %d", time_to_wake);
//                         this->timer.start( 40 );
                    } else {
                        this->timer.start( time_to_wake );
                    }
                } else {
                    this->timer.stop();
                }
            }
        }
    }


};



class Form_Qt : public QWidget
{

Q_OBJECT

public:
    enum : int {
        MAX_ACCOUNT_DATA = 15
    };

    Front_Qt_API       * _front;
    const int            _width;
    const int            _height;
    QFormLayout          _formLayout;
    QComboBox            _IPCombobox;
    int                  _accountNB;
    QLineEdit            _IPField;
    QLineEdit            _userNameField;
    QLineEdit            _PWDField;
    QLineEdit            _portField;
    QLabel               _IPLabel;
    QLabel               _userNameLabel;
    QLabel               _PWDLabel;
    QLabel               _portLabel;
    QLabel               _errorLabel;
    QCheckBox            _pwdCheckBox;
    QPushButton          _buttonConnexion;
    QPushButton          _buttonOptions;
    QPushButton          _buttonReplay;
    //QCompleter         * _completer;
    struct AccountData {
        std::string title;
        std::string IP;
        std::string name;
        std::string pwd;
        int port;
    }                    _accountData[MAX_ACCOUNT_DATA];
    bool                 _pwdCheckBoxChecked;
    int                  _lastTargetIndex;


    Form_Qt(Front_Qt_API * front)
        : QWidget()
        , _front(front)
        , _width(400)
        , _height(300)
        , _formLayout(this)
        , _IPCombobox(this)
        , _accountNB(0)
        , _IPField("", this)
        , _userNameField("", this)
        , _PWDField("", this)
        , _portField("", this)
        , _IPLabel(      QString("IP server :"), this)
        , _userNameLabel(QString("User name : "), this)
        , _PWDLabel(     QString("Password :  "), this)
        , _portLabel(    QString("Port :      "), this)
        , _errorLabel(   QString(""            ), this)
        , _pwdCheckBox(QString("Save password."), this)
        , _buttonConnexion("Connection", this)
        , _buttonOptions("Options", this)
        , _buttonReplay("Replay", this)
        , _pwdCheckBoxChecked(false)
        , _lastTargetIndex(0)
    {
        this->setWindowTitle("Remote Desktop Player");
        this->setAttribute(Qt::WA_DeleteOnClose);
        this->setFixedSize(this->_width, this->_height);

        this->setAccountData();

        if (this->_pwdCheckBoxChecked) {
            this->_pwdCheckBox.setCheckState(Qt::Checked);
        }
        //this->_IPField.setCompleter(this->_completer);
        this->_IPCombobox.setLineEdit(&(this->_IPField));
        //this->_IPCombobox.setCompleter(this->_completer);
        this->QObject::connect(&(this->_IPCombobox), SIGNAL(currentIndexChanged(int)) , this, SLOT(targetPicked(int)));

        this->_PWDField.setEchoMode(QLineEdit::Password);
        this->_PWDField.setInputMethodHints(Qt::ImhHiddenText | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase);
        this->_formLayout.addRow(&(this->_IPLabel)      , &(this->_IPCombobox));
        this->_formLayout.addRow(&(this->_userNameLabel), &(this->_userNameField));
        this->_formLayout.addRow(&(this->_PWDLabel)     , &(this->_PWDField));
        this->_formLayout.addRow(&(this->_portLabel)    , &(this->_portField));
        this->_formLayout.addRow(&(this->_pwdCheckBox));
        this->_formLayout.addRow(&(this->_errorLabel));
        this->setLayout(&(this->_formLayout));

        QRect rectReplay(QPoint(10, 226), QSize(110, 24));
        this->_buttonReplay.setToolTip(this->_buttonReplay.text());
        this->_buttonReplay.setGeometry(rectReplay);
        this->_buttonReplay.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonReplay)   , SIGNAL (pressed()),  this, SLOT (replayPressed()));
        this->_buttonReplay.setFocusPolicy(Qt::StrongFocus);
        this->_buttonReplay.setAutoDefault(true);

        QRect rectConnexion(QPoint(280, 256), QSize(110, 24));
        this->_buttonConnexion.setToolTip(this->_buttonConnexion.text());
        this->_buttonConnexion.setGeometry(rectConnexion);
        this->_buttonConnexion.setCursor(Qt::PointingHandCursor);
        //this->QObject::connect(&(this->_buttonConnexion)   , SIGNAL (pressed()),  this, SLOT (connexionPressed()));
        this->QObject::connect(&(this->_buttonConnexion)   , SIGNAL (released()), this, SLOT (connexionReleased()));
        this->_buttonConnexion.setFocusPolicy(Qt::StrongFocus);
        this->_buttonConnexion.setAutoDefault(true);

        QRect rectOptions(QPoint(10, 256), QSize(110, 24));
        this->_buttonOptions.setToolTip(this->_buttonOptions.text());
        this->_buttonOptions.setGeometry(rectOptions);
        this->_buttonOptions.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonOptions)     , SIGNAL (pressed()),  this, SLOT (optionsPressed()));
        this->QObject::connect(&(this->_buttonOptions)     , SIGNAL (released()), this, SLOT (optionsReleased()));
        this->_buttonOptions.setFocusPolicy(Qt::StrongFocus);


        if (this->_front->is_no_win_data()) {
            QDesktopWidget* desktop = QApplication::desktop();
            this->_front->windowsData.form_x = (desktop->width()/2)  - (this->_width/2);
            this->_front->windowsData.form_y = (desktop->height()/2) - (this->_height/2);

        }
        this->move(this->_front->windowsData.form_x, this->_front->windowsData.form_y);
    }

    ~Form_Qt() {
        QPoint points = this->mapToGlobal({0, 0});
        this->_front->windowsData.form_x = points.x()-1;
        this->_front->windowsData.form_y = points.y()-39;
        this->_front->writeWindowsConf();
    }

    void setAccountData() {
        this->_accountNB = 0;
        std::ifstream ifichier(this->_front->USER_CONF_LOG, std::ios::in);

        if (ifichier) {
            int accountNB(0);
            std::string ligne;
            const std::string delimiter = " ";

            while(std::getline(ifichier, ligne)) {

                auto pos(ligne.find(delimiter));
                std::string tag  = ligne.substr(0, pos);
                std::string info = ligne.substr(pos + delimiter.length(), ligne.length());

                if (tag.compare(std::string("save_pwd")) == 0) {
                    if (info.compare(std::string("true")) == 0) {
                        this->_pwdCheckBoxChecked = true;
                    } else {
                        this->_pwdCheckBoxChecked = false;
                    }
                } else
                if (tag.compare(std::string("last_target")) == 0) {
                    this->_lastTargetIndex = std::stoi(info);
                } else
                if (tag.compare(std::string("title")) == 0) {
                    this->_accountData[accountNB].title = info;
                } else
                if (tag.compare(std::string("IP")) == 0) {
                    this->_accountData[accountNB].IP = info;
                } else
                if (tag.compare(std::string("name")) == 0) {
                    this->_accountData[accountNB].name = info;
                } else
                if (tag.compare(std::string("pwd")) == 0) {
                    this->_accountData[accountNB].pwd = info;
                } else
                if (tag.compare(std::string("port")) == 0) {
                    this->_accountData[accountNB].port = std::stoi(info);
                    accountNB++;
                    if (accountNB == MAX_ACCOUNT_DATA) {
                        this->_accountNB = MAX_ACCOUNT_DATA;
                        accountNB = 0;
                    }
                }
            }

            if (this->_accountNB < MAX_ACCOUNT_DATA) {
                this->_accountNB = accountNB;
            }

            this->_IPCombobox.clear();
            this->_IPCombobox.addItem(QString(""), 0);
            QStringList stringList;
            for (int i = 0; i < this->_accountNB; i++) {
                std::string title(this->_accountData[i].IP + std::string(" - ")+ this->_accountData[i].name);
                this->_IPCombobox.addItem(QString(title.c_str()), i+1);
                stringList << title.c_str();
            }
            //this->_completer = new QCompleter(stringList, this);
        }
     }

    void set_ErrorMsg(std::string str) {
        this->_errorLabel.clear();
        this->_errorLabel.setText(QString(str.c_str()));
    }

    void set_IPField(std::string str) {
        this->_IPField.clear();
        this->_IPField.insert(QString(str.c_str()));
    }

    void set_userNameField(std::string str) {
        this->_userNameField.clear();
        this->_userNameField.insert(QString(str.c_str()));
    }

    void set_PWDField(std::string str) {
        this->_PWDField.clear();
        this->_PWDField.insert(QString(str.c_str()));
    }

    void set_portField(int str) {
        this->_portField.clear();
        if (str == 0) {
            this->_portField.insert(QString(""));
        } else {
            this->_portField.insert(QString(std::to_string(str).c_str()));
        }
    }

    std::string get_IPField() {
        std::string delimiter(" - ");
        std::string ip_field_content = this->_IPField.text().toStdString();
        auto pos(ip_field_content.find(delimiter));
        std::string IP  = ip_field_content.substr(0, pos);
        return IP;
    }

    std::string get_userNameField() {
        if (this->_userNameField.text().toStdString() !=  std::string(""))
            return this->_userNameField.text().toStdString();

        return std::string(" ");
    }

    std::string get_PWDField() {
        if (this->_PWDField.text().toStdString() !=  std::string(""))
            return this->_PWDField.text().toStdString();

        return std::string(" ");
    }

    int get_portField() {
        return this->_portField.text().toInt();
    }

    void keyPressEvent(QKeyEvent *e) {
        if (e->key() == Qt::Key_Enter) {
            this->connexionReleased();
        }
    }


private Q_SLOTS:
    void targetPicked(int index) {
        if (index >=  16) {
             this->connexionReleased();
             return;
        }
        if (index == 0) {
            this->_IPField.clear();
            this->_userNameField.clear();
            this->_PWDField.clear();
            this->_portField.clear();

        } else {
            index--;
            this->set_IPField(this->_accountData[index].IP);
            this->set_userNameField(this->_accountData[index].name);
            this->set_PWDField(this->_accountData[index].pwd);
            this->set_portField(this->_accountData[index].port);
        }

        this->_buttonConnexion.setFocus();
    }

    void replayPressed() {
        QString filePath("");
        filePath = QFileDialog::getOpenFileName(this, tr("Open a Movie"),
                                                this->_front->REPLAY_DIR.c_str(),
                                                tr("Movie Files(*.mwrm)"));
        std::string str_movie_path(filePath.toStdString());

        auto const last_delimiter_it = std::find(str_movie_path.rbegin(), str_movie_path.rend(), '/');
        int pos = str_movie_path.size() - (last_delimiter_it - str_movie_path.rbegin());

        std::string const movie_name = (last_delimiter_it == str_movie_path.rend())
        ? str_movie_path
        : str_movie_path.substr(str_movie_path.size() - (last_delimiter_it - str_movie_path.rbegin()));

        std::string const movie_dir = str_movie_path.substr(0, pos);

        this->_front->_movie_name = movie_name;
        this->_front->_movie_dir = movie_dir;
        this->_front->replay(movie_dir, movie_name);
    }

    void connexionReleased() {

        this->_front->connexionReleased();
        if (this->_front->connected && this->_front->mod !=  nullptr) {
            bool alreadySet = false;
            this->_pwdCheckBoxChecked = this->_pwdCheckBox.isChecked();

            std::string title(this->get_IPField() + std::string(" - ")+ this->get_userNameField());

            for (int i = 0; i < this->_accountNB; i++) {

                if (this->_accountData[i].title.compare(title) == 0) {
                    alreadySet = true;
                    this->_lastTargetIndex = i;
                    this->_accountData[i].pwd  = this->get_PWDField();
                    this->_accountData[i].port = this->get_portField();
                }
            }
            if (!alreadySet && (this->_accountNB < MAX_ACCOUNT_DATA)) {
                this->_accountData[this->_accountNB].title = title;
                this->_accountData[this->_accountNB].IP    = this->get_IPField();
                this->_accountData[this->_accountNB].name  = this->get_userNameField();
                this->_accountData[this->_accountNB].pwd   = this->get_PWDField();
                this->_accountData[this->_accountNB].port  = this->get_portField();
                this->_accountNB++;

                if (this->_accountNB > MAX_ACCOUNT_DATA) {
                    this->_accountNB = MAX_ACCOUNT_DATA;
                }
                this->_lastTargetIndex = this->_accountNB;
            }

            std::ofstream ofichier(this->_front->USER_CONF_LOG, std::ios::out | std::ios::trunc);
            if(ofichier) {

                if (this->_pwdCheckBoxChecked) {
                    ofichier << "save_pwd true" << "\n";
                } else {
                    ofichier << "save_pwd false" << "\n";
                }
                ofichier << "last_target " <<  this->_lastTargetIndex << "\n";

                ofichier << "\n";

                for (int i = 0; i < this->_accountNB; i++) {
                    ofichier << "title " << this->_accountData[i].title << "\n";
                    ofichier << "IP "    << this->_accountData[i].IP    << "\n";
                    ofichier << "name "  << this->_accountData[i].name  << "\n";
                    if (this->_pwdCheckBoxChecked) {
                        ofichier << "pwd " << this->_accountData[i].pwd << "\n";
                    } else {
                        ofichier << "pwd " << "\n";
                    }
                    ofichier << "port " << this->_accountData[i].port << "\n";
                    ofichier << "\n";
                }
            }
        }
    }

    void optionsPressed() {}

    void optionsReleased() {
        this->_front->options();
    }
};




class Screen_Qt : public QWidget
{

Q_OBJECT

public:
    Front_Qt_API  * _front;
    QPushButton     _buttonCtrlAltDel;
    QPushButton     _buttonRefresh;
    QPushButton     _buttonDisconnexion;
    QColor          _penColor;
    QPixmap      * _cache;
    QPainter       _cache_painter;
    QPixmap      * _trans_cache;
    QPainter       _trans_cache_painter;
    const int      _width;
    const int      _height;
    QPixmap        _match_pixmap;
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
        BALISED_FRAME = 30
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

private:
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
    Screen_Qt (Front_Qt_API * front, QPixmap * cache, std::string const & movie_dir, std::string const & movie_name, QPixmap * trans_cache)
        : QWidget()
        , _front(front)
        , _buttonCtrlAltDel("Play", this)
        , _buttonRefresh("Stop", this)
        , _buttonDisconnexion("Close", this)
        , _penColor(Qt::black)
        , _cache(cache)
        , _cache_painter(this->_cache)
        , _trans_cache(trans_cache)
        , _trans_cache_painter(this->_trans_cache)
        , _width(this->_front->info.width)
        , _height(this->_front->info.height)
        , _match_pixmap(this->_width+2, this->_height+2)
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


    void pre_load_movie() {

        long int movie_length = this->movie_time;
        long int endin_frame = 0;
        int i = 0;

        while (endin_frame < movie_length) {
            //timeval end_fram_time = {long int(endin_frame), 0};
            this->_front->replay_mod->instant_play_client(std::chrono::microseconds(endin_frame*1000000));

            this->balises.push_back(nullptr);
            this->balises[i] = new QPixmap(*(this->_cache));
            endin_frame += BALISED_FRAME;
            i++;
            if (this->_front->bar) {
                this->_front->bar->setValue(endin_frame);
            }
        }

        this->_front->load_replay_mod(this->_movie_dir, this->_movie_name, {this->begin, 0}, {0, 0});
    }



    Screen_Qt (Front_Qt_API * front, QPixmap * cache, QPixmap * trans_cache)
        : QWidget()
        , _front(front)
        , _buttonCtrlAltDel("CTRL + ALT + DELETE", this)
        , _buttonRefresh("Refresh", this)
        , _buttonDisconnexion("Disconnection", this)
        , _penColor(Qt::black)
        , _cache(cache)
        , _cache_painter(this->_cache)
        , _trans_cache(trans_cache)
        , _width(this->_front->info.width)
        , _height(this->_front->info.height)
        , _match_pixmap(this->_width, this->_height)
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

    ~Screen_Qt() {
        QPoint points = this->mapToGlobal({0, 0});
        this->_front->windowsData.screen_x = points.x()-1;
        this->_front->windowsData.screen_y = points.y()-39;
        this->_front->writeWindowsConf();
        if (!this->_connexionLasted) {
            this->_front->closeFromScreen();
        }
        this->balises.clear();
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

    QPainter & paintTransCache() {
        return this->_trans_cache_painter;
    }

    void paintEvent(QPaintEvent * event) {
        Q_UNUSED(event);

        QPen                 pen;
        QPainter             painter;
        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(this->_penColor);
        painter.setPen(pen);
        painter.drawPixmap(QPoint(0, 0), this->_match_pixmap, QRect(0, 0, this->_width, this->_height));
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

    bool event(QEvent *event) {
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
    void mousePressEvent(QMouseEvent *e) {
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

            this->_front->mousePressEvent(e, 0);
        }
    }

    void mouseReleaseEvent(QMouseEvent *e) {
        this->_front->mouseReleaseEvent(e, 0);
    }

    void keyPressEvent(QKeyEvent *e) {
        this->_front->keyPressEvent(e);
    }

    void keyReleaseEvent(QKeyEvent *e) {
        this->_front->keyReleaseEvent(e);
    }

    void wheelEvent(QWheelEvent *e) {
        this->_front->wheelEvent(e);
    }

    void enterEvent(QEvent *event) {
        Q_UNUSED(event);
        //this->update_current_cursor();
        //this->_front->_current_screen_index =  this->_screen_index;
    }

    bool eventFilter(QObject *obj, QEvent *e) {
        this->_front->eventFilter(obj, e, 0);
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
        QPainter match_painter(&(this->_match_pixmap));
        match_painter.drawPixmap(QPoint(0, 0), *(this->_cache), QRect(0, 0, this->_width, this->_height));
        //match_painter.drawPixmap(QPoint(0, 0), *(this->_trans_cache), QRect(0, 0, this->_width, this->_height));
        this->repaint();
    }

    void slotRepainMatch() {
        QPainter match_painter(&(this->_match_pixmap));
        match_painter.drawPixmap(QPoint(0, 0), *(this->_cache), QRect(0, 0, this->_width, this->_height));
        match_painter.drawPixmap(QPoint(0, 0), *(this->_trans_cache), QRect(0, 0, this->_width, this->_height));
        this->repaint();
    }

    void RefreshPressed() {
        this->_front->RefreshPressed();
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
        this->_front->disconnexionReleased();
    }

    void setMainScreenOnTopRelease() {
        this->_front->setMainScreenOnTopRelease();
    }
};



class FrontQtRDPGraphicAPI : public Front_Qt_API
{
    struct Snapshoter : gdi::CaptureApi
    {
        FrontQtRDPGraphicAPI & front;

        Snapshoter(FrontQtRDPGraphicAPI & front) : front(front) {}

        Microseconds periodic_snapshot(
            const timeval& /*now*/, int cursor_x, int cursor_y, bool /*ignore_frame_in_timeval*/
        ) override {
            this->front.update_pointer_position(cursor_x, cursor_y);
            std::chrono::microseconds res(1);
            return res;
        }
    };
    Snapshoter snapshoter;

public:

    // Graphic members
    int                  mod_bpp;
    BGRPalette           mod_palette;
    Form_Qt            * form;
    Screen_Qt          * screen;
    Mod_Qt             * mod_qt;
    QPixmap            * cache;
    QPixmap            * trans_cache;
    gdi::GraphicApi    * graph_capture;

    struct MouseData {
        QImage cursor_image;
        uint16_t x = 0;
        uint16_t y = 0;
    } mouse_data;

    // Connexion socket members
    int                  _timer;
     std::unique_ptr<Capture>  capture;
    Font                 _font;
    std::string          _error;

    // Keyboard Controllers members
    Keymap2              keymap;
    bool                 ctrl_alt_delete; // currently not used and always false
    StaticOutStream<256> decoded_data;    // currently not initialised
    uint8_t              keyboardMods;

    CHANNELS::ChannelDefArray   cl;

    bool is_pipe_ok;

    bool remoteapp;


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONSTRUCTOR
    //------------------------

    FrontQtRDPGraphicAPI( RDPVerbose verbose)
      : Front_Qt_API(verbose)
      , snapshoter(*this)
      , mod_bpp(24)
      , mod_palette(BGRPalette::classic_332())
      , form(nullptr)
      , screen(nullptr)
      , mod_qt(nullptr)
      , cache(nullptr)
      , trans_cache(nullptr)
      , graph_capture(nullptr)
      , _timer(0)
      , _error("error")
      , keymap()
      , ctrl_alt_delete(false)
      , is_pipe_ok(true)
      , remoteapp(false)
    {
        SSL_load_error_strings();
        SSL_library_init();

        // Windows and socket contrainer
        this->mod_qt = new Mod_Qt(this, this->form);
        this->form = new Form_Qt(this);

        if (this->mod_bpp == this->info.bpp) {
            this->mod_palette = BGRPalette::classic_332();
        }

        this->info.width  = 800;
        this->info.height = 600;
        this->info.keylayout = 0x040C;// 0x40C FR, 0x409 USA
        this->info.console_session = 0;
        this->info.brush_cache_code = 0;
        this->info.bpp = 24;
        this->imageFormatRGB  = this->bpp_to_QFormat(this->info.bpp, false);
        if (this->info.bpp ==  32) {
            this->imageFormatARGB = this->bpp_to_QFormat(this->info.bpp, true);
        }
        this->info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
        this->info.cs_monitor.monitorCount = 1;

        this->qtRDPKeymap.setKeyboardLayout(this->info.keylayout);
        this->keymap.init_layout(this->info.keylayout);

        this->disconnect("");
    }

    virtual bool must_be_stop_capture() override {
//         this->is_pipe_ok = false;
//         if (this->capture) {
//             this->capture.reset(nullptr);
//             this->graph_capture = nullptr;
//             this->is_recording = false;
//
//             return true;
//         }
        return false;
    }

    virtual void begin_update() override {

        if (bool(this->verbose & RDPVerbose::graphics)) {
           LOG(LOG_INFO, "--------- FRONT ------------------------");
           LOG(LOG_INFO, "begin_update");
           LOG(LOG_INFO, "========================================\n");
        }

        if ((this->connected || this->is_replaying) && this->screen != nullptr) {
            if (this->is_recording && !this->is_replaying && this->screen != nullptr) {
                this->graph_capture->begin_update();
                struct timeval time;
                gettimeofday(&time, nullptr);
                this->capture.get()->periodic_snapshot(time, this->mouse_data.x, this->mouse_data.y, false);
            }
        }
    }

    virtual void end_update() override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
           LOG(LOG_INFO, "--------- FRONT ------------------------");
           LOG(LOG_INFO, "end_update");
           LOG(LOG_INFO, "========================================\n");
        }

        if ((this->connected || this->is_replaying) && this->screen != nullptr) {
            this->screen->update_view();

            if (this->is_recording && !this->is_replaying && this->screen != nullptr) {
                this->graph_capture->end_update();
                struct timeval time;
                gettimeofday(&time, nullptr);
                this->capture.get()->periodic_snapshot(time, this->mouse_data.x, this->mouse_data.y, false);
            }
        }
    }

    virtual void update_pointer_position(uint16_t xPos, uint16_t yPos) override {

        if (this->is_replaying) {
            this->trans_cache->fill(Qt::transparent);
            QRect nrect(xPos, yPos, this->mouse_data.cursor_image.width(), this->mouse_data.cursor_image.height());

            this->screen->paintTransCache().drawImage(nrect, this->mouse_data.cursor_image);
        }
    }

    virtual ResizeResult server_resize(int width, int height, int bpp) override{
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d)", width, height, bpp);
            LOG(LOG_INFO, "========================================\n");
        }

        if (this->remoteapp) {
            return ResizeResult::remoteapp;
        }

        if (width == 0 || height == 0) {
            return ResizeResult::fail;
        }

        if ((this->connected || this->is_replaying) && this->screen != nullptr) {
            this->info.bpp = bpp;
            this->imageFormatRGB  = this->bpp_to_QFormat(this->info.bpp, false);

            if (this->info.width != width || this->info.height != height) {
                this->info.width = width;
                this->info.height = height;
                if (this->screen) {
                    this->screen->disconnection();
                    this->dropScreen();
                }
                this->cache = new QPixmap(this->info.width, this->info.height);

                if (this->is_replaying) {
                    this->screen = new Screen_Qt(this, this->cache, this->_movie_dir, this->_movie_name, this->trans_cache);

                } else {
                    this->trans_cache = new QPixmap(this->info.width, this->info.height);
                    this->trans_cache->fill(Qt::transparent);
                    this->screen = new Screen_Qt(this, this->cache, this->trans_cache);
                }

                this->screen->show();
            }
        }


        return ResizeResult::instant_done;
    }

    virtual void set_pointer(Pointer const & cursor) override {

        QImage image_data(cursor.data, cursor.width, cursor.height, this->bpp_to_QFormat(24, false));
        QImage image_mask(cursor.mask, cursor.width, cursor.height, QImage::Format_Mono);

        if (cursor.mask[0x48] == 0xFF &&
            cursor.mask[0x49] == 0xFF &&
            cursor.mask[0x4A] == 0xFF &&
            cursor.mask[0x4B] == 0xFF) {

            image_mask = image_data.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            image_data.invertPixels();

        } else {
            image_mask.invertPixels();
        }

        image_data = image_data.mirrored(false, true).convertToFormat(QImage::Format_ARGB32_Premultiplied);
        image_mask = image_mask.mirrored(false, true).convertToFormat(QImage::Format_ARGB32_Premultiplied);

        const uchar * data_data = image_data.bits();
        const uchar * mask_data = image_mask.bits();

        uint8_t data[Pointer::DATA_SIZE*4];

        for (int i = 0; i < Pointer::DATA_SIZE; i += 4) {
            data[i  ] = data_data[i+2];
            data[i+1] = data_data[i+1];
            data[i+2] = data_data[i  ];
            data[i+3] = mask_data[i+0];
        }

        if (this->is_replaying) {
            this->mouse_data.cursor_image = QImage(static_cast<uchar *>(data), cursor.x, cursor.y, QImage::Format_ARGB32_Premultiplied);

        } else {
            this->screen->set_mem_cursor(static_cast<uchar *>(data), cursor.x, cursor.y);

            if (this->is_recording && !this->is_replaying) {
                this->graph_capture->set_pointer(cursor);
                struct timeval time;
                gettimeofday(&time, nullptr);
                this->capture.get()->periodic_snapshot(time, this->mouse_data.x, this->mouse_data.y, false);
            }
        }
    }

    bool load_replay_mod(std::string const & movie_dir, std::string const & movie_name, timeval begin_read, timeval end_read) override {
         try {
            this->replay_mod.reset(new ReplayMod( *this
                                                , movie_dir.c_str() //(this->REPLAY_DIR + "/").c_str()
                                                , movie_name.c_str()
                                                , 0             //this->info.width
                                                , 0             //this->info.height
                                                , this->_error
                                                , this->_font
                                                , true
                                                , begin_read
                                                , end_read
                                                , Screen_Qt::BALISED_FRAME
                                                //, FileToGraphic::Verbose::rdp_orders
                                                , to_verbose_flags(0)
                                                ));

            //this->replay_mod->add_consumer(nullptr, &this->snapshoter, nullptr, nullptr, nullptr);

            return true;

        } catch (const Error & err) {
            LOG(LOG_ERR, "new ReplayMod error %s", err.errmsg());
        }

        if (this->replay_mod == nullptr) {
            this->dropScreen();
            this->readError(movie_name);
            this->form->show();
        }
        return false;
    }

    void delete_replay_mod() override {
        this->replay_mod.reset();
    }

    virtual bool is_no_win_data() {
        return this->windowsData.no_data;
    }

    virtual void writeWindowsConf() {
        this->windowsData.write();
    }

    void answer_question(int color) {
        QImage image = this->cache->toImage();

        QRgb asked(color);

        QRgb top_left  = image.pixel(0, 0);
        QRgb top_right = image.pixel(this->info.width-1, 0);
        QRgb bot_left  = image.pixel(0, this->info.height-1);
        QRgb bot_right = image.pixel(this->info.width-1, this->info.height-1);

        //LOG(LOG_INFO, "         top_left = 0x%04x top_right = 0x%04x bot_left = 0x%04x bot_right = 0x%04x asked_color = 0x%04x, top_left, top_right, bot_left, bot_right, asked);

        if        (top_left == asked) {
            this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, 0, 0, &(this->keymap));
            this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1, 0, 0, &(this->keymap));
            this->wab_diag_question = false;
//             LOG(LOG_INFO, "CLIENT >> answer_question top_left");

        } else if (top_right == asked) {
            this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, this->info.width-1, 0, &(this->keymap));
            this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1, this->info.width-1, 0, &(this->keymap));
            this->wab_diag_question = false;
//             LOG(LOG_INFO, "CLIENT >> answer_question top_right");

        } else if (bot_left == asked) {
            this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, 0, this->info.height-1, &(this->keymap));
            this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1, 0, this->info.height-1, &(this->keymap));
            this->wab_diag_question = false;
//             LOG(LOG_INFO, "CLIENT >> answer_question bot_left");

        } else if (bot_right == asked) {
            this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN, this->info.width-1, this->info.height-1, &(this->keymap));
            this->mod->rdp_input_mouse(MOUSE_FLAG_BUTTON1, this->info.width-1, this->info.height-1, &(this->keymap));
            this->wab_diag_question = false;
//             LOG(LOG_INFO, "CLIENT >> answer_question bot_right");

        }
    }


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

        if (uint8_t(this->info.bpp) != color_ctx.depth().to_bpp()) {
            BGRColor d = color_decode(color, color_ctx);
            color      = color_encode(d, uint8_t(this->info.bpp));
        }

        BGRColor bgr = color_decode(color, this->info.bpp, this->mod_palette);

        return {bgr.red(), bgr.green(), bgr.blue()};
    }

    template<class Op>
    void draw_memblt_op(const Rect & drect, const Bitmap & bitmap) {
        const uint16_t mincx = std::min<int16_t>(bitmap.cx(), std::min<int16_t>(this->info.width - drect.x, drect.cx));
        const uint16_t mincy = std::min<int16_t>(bitmap.cy(), std::min<int16_t>(this->info.height - drect.y, drect.cy));

        if (mincx <= 0 || mincy <= 0) {
            return;
        }

        QImage::Format format(this->bpp_to_QFormat(bitmap.bpp(), false)); //bpp
        QImage srcBitmap(bitmap.data(), mincx, mincy, bitmap.line_size(), format);
        QImage dstBitmap(this->screen->getCache()->toImage().copy(drect.x, drect.y, mincx, mincy));

        if (bitmap.bpp() == 24) {
            srcBitmap = srcBitmap.rgbSwapped();
        }

        if (bitmap.bpp() != this->info.bpp) {
            srcBitmap = srcBitmap.convertToFormat(this->imageFormatRGB);
        }
        dstBitmap = dstBitmap.convertToFormat(srcBitmap.format());

        QImage srcBitmapMirrored = srcBitmap.mirrored(false, true);

        uchar data[1600*900*3];

        //std::unique_ptr<uchar[]> data = std::make_unique<uchar[]>(srcBitmapMirrored.bytesPerLine() * drect.cy);

        const uchar * srcData = srcBitmapMirrored.constBits();
        const uchar * dstData = dstBitmap.constBits();

        int data_len = bitmap.line_size() * mincy;
        Op op;
        for (int i = 0; i < data_len; i++) {
            data[i] = op.op(srcData[i], dstData[i]);
        }

        QImage image(data, mincx, mincy, srcBitmapMirrored.format());
        QRect trect(drect.x, drect.y, mincx, mincy);

        if (this->connected || this->is_replaying) {
             this->screen->paintCache().drawImage(trect, image);
        }
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

        if (bitmap.bpp() == 24) {
            qbitmap = qbitmap.rgbSwapped();
        }

        const QRect trect(drect.x, drect.y, drect.cx, drect.cy);
        if (this->connected || this->is_replaying) {
             this->screen->paintCache().drawImage(trect, qbitmap);
        }
    }


    void draw_RDPScrBlt(int srcx, int srcy, const Rect & drect, bool invert) {
        QImage qbitmap(this->screen->getCache()->toImage().copy(srcx, srcy, drect.cx, drect.cy));
        if (invert) {
            qbitmap.invertPixels();
        }
        const QRect trect(drect.x, drect.y, drect.cx, drect.cy);
        if (this->screen) {
            this->screen->paintCache().drawImage(trect, qbitmap);
        }
    }

//     QColor u32_to_qcolor_r(RDPColor color) {
//         BGRColor_ bgr = color_decode(color, this->info.bpp, this->mod_palette);
//         return {bgr.blue(), bgr.green(), bgr.red()};
//     }


    QImage::Format bpp_to_QFormat(int bpp, bool alpha) {
        QImage::Format format(QImage::Format_RGB16);

        if (alpha) {

            switch (bpp) {
                case 15: format = QImage::Format_ARGB4444_Premultiplied; break;
                case 16: format = QImage::Format_ARGB4444_Premultiplied; break;
                case 24: format = QImage::Format_ARGB8565_Premultiplied; break;
                case 32: format = QImage::Format_ARGB32_Premultiplied;   break;
                default : break;
            }
        } else {

            switch (bpp) {
                case 15: format = QImage::Format_RGB555; break;
                case 16: format = QImage::Format_RGB16;  break;
                case 24: format = QImage::Format_RGB888; break;
                case 32: format = QImage::Format_RGB32;  break;
                default : break;
            }
        }

        return format;
    }

    void draw_RDPPatBlt(const Rect & rect, const QColor color, const QPainter::CompositionMode mode, const Qt::BrushStyle style) {
        QBrush brush(color, style);
        if (this->connected || this->is_replaying) {
            this->screen->paintCache().setBrush(brush);
            this->screen->paintCache().setCompositionMode(mode);
            this->screen->paintCache().drawRect(rect.x, rect.y, rect.cx, rect.cy);
            this->screen->paintCache().setCompositionMode(QPainter::CompositionMode_SourceOver);
            this->screen->paintCache().setBrush(Qt::SolidPattern);
        }
    }

    void draw_RDPPatBlt(const Rect & rect, const QPainter::CompositionMode mode) {
        if (this->connected || this->is_replaying) {
            this->screen->paintCache().setCompositionMode(mode);
            this->screen->paintCache().drawRect(rect.x, rect.y, rect.cx, rect.cy);
            this->screen->paintCache().setCompositionMode(QPainter::CompositionMode_SourceOver);
        }
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //-----------------------------
    //       DRAW FUNCTIONS
    //-----------------------------

    using Front_Qt_API::draw;

    void draw(const RDPPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        const Rect rect = clip.intersect(this->info.width, this->info.height).intersect(cmd.rect);

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
                        if (this->connected || this->is_replaying) {
                            this->screen->paintCache().setBrush(brush);
                            this->screen->paintCache().setCompositionMode(QPainter::RasterOp_SourceXorDestination);
                            this->screen->paintCache().drawRect(rect.x, rect.y, rect.cx, rect.cy);
                            this->screen->paintCache().setCompositionMode(QPainter::CompositionMode_SourceOver);
                            this->screen->paintCache().setBrush(Qt::SolidPattern);
                        }
                    }
                    break;

                // +------+-------------------------------+
                // | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
                // |      | RPN: P                        |
                // +------+-------------------------------+
                case 0xF0:
                    {
                        if (this->connected || this->is_replaying) {
                            QBrush brush(foreColor, Qt::Dense4Pattern);
                            this->screen->paintCache().setPen(Qt::NoPen);
                            this->screen->paintCache().fillRect(rect.x, rect.y, rect.cx, rect.cy, backColor);
                            this->screen->paintCache().setBrush(brush);
                            this->screen->paintCache().drawRect(rect.x, rect.y, rect.cx, rect.cy);
                            this->screen->paintCache().setBrush(Qt::SolidPattern);
                        }
                    }
                    break;
                default: LOG(LOG_WARNING, "RDPPatBlt brush_style = 0x03 rop = %x", cmd.rop);
                    break;
            }

        } else {
            switch (cmd.rop) {

                case 0x00: // blackness
                    if (this->connected || this->is_replaying) {
                        this->screen->paintCache().fillRect(rect.x, rect.y, rect.cx, rect.cy, Qt::black);
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
                    if (this->connected || this->is_replaying) {
                        this->screen->paintCache().setPen(Qt::NoPen);
                        this->screen->paintCache().fillRect(rect.x, rect.y, rect.cx, rect.cy, backColor);
                        this->screen->paintCache().drawRect(rect.x, rect.y, rect.cx, rect.cy);
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
                    if (this->connected || this->is_replaying) {
                        this->screen->paintCache().fillRect(rect.x, rect.y, rect.cx, rect.cy, Qt::white);
                    }
                    break;
                default: LOG(LOG_WARNING, "RDPPatBlt rop = %x", cmd.rop);
                    break;
            }
        }

        if (this->is_recording && !this->is_replaying) {
            this->graph_capture->draw(cmd, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->info.bpp), &this->mod_palette));
            struct timeval time;
            gettimeofday(&time, nullptr);
            this->capture.get()->periodic_snapshot(time, this->mouse_data.x, this->mouse_data.y, false);
        }

        if (this->wab_diag_question) {
            this->answer_question(this->asked_color);
        }
    }


    void draw(const RDPOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        if ((this->connected || this->is_replaying) && this->screen != nullptr) {
            QColor qcolor(this->u32_to_qcolor(cmd.color, color_ctx));
            Rect rect(cmd.rect.intersect(clip));

            this->screen->paintCache().fillRect(rect.x, rect.y, rect.cx, rect.cy, qcolor);

            if (this->is_recording && !this->is_replaying) {
                this->graph_capture->draw(cmd, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->info.bpp), &this->mod_palette));
                struct timeval time;
                gettimeofday(&time, nullptr);
                this->capture.get()->periodic_snapshot(time, this->mouse_data.x, this->mouse_data.y, false);
            }

            if (this->wab_diag_question) {
                this->answer_question(this->asked_color);
            }
        } else {
            if (this->connected || this->is_replaying) {
                LOG(LOG_INFO, "opaquerect is else and connected");
            } else {
                LOG(LOG_INFO, "opaquerect is else");
            }
        }
    }


    void draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            //bitmap_data.log(LOG_INFO, "RDPBitmapData");
            LOG(LOG_INFO, "RDPBitmapData");
            LOG(LOG_INFO, "========================================\n");
        }
        //std::cout << "RDPBitmapData" << std::endl;
        if (!bmp.is_valid()){
            return;
        }

        Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top,
                                (bitmap_data.dest_right - bitmap_data.dest_left + 1),
                                (bitmap_data.dest_bottom - bitmap_data.dest_top + 1));
        const Rect clipRect(0, 0, this->info.width, this->info.height);
        const Rect drect = rectBmp.intersect(clipRect);

        const int16_t mincx = std::min<int16_t>(bmp.cx(), std::min<int16_t>(this->info.width - drect.x, drect.cx));
        const int16_t mincy = std::min<int16_t>(bmp.cy(), std::min<int16_t>(this->info.height - drect.y, drect.cy));;

        if (mincx <= 0 || mincy <= 0) {
            return;
        }

        QImage::Format format(this->bpp_to_QFormat(bmp.bpp(), false)); //bpp
        QImage qbitmap(bmp.data(), mincx, mincy, bmp.line_size(), format);

        if (bmp.bpp() == 24) {
            qbitmap = qbitmap.rgbSwapped();
        }

        if (bmp.bpp() != this->info.bpp) {
            qbitmap = qbitmap.convertToFormat(this->imageFormatRGB);
            LOG(LOG_INFO, "RDPBitmapData convertToFormat");
        }

        qbitmap = qbitmap.mirrored(false, true);
        QRect trect(drect.x, drect.y, mincx, mincy);
        if (this->connected || this->is_replaying) {
            this->screen->paintCache().drawImage(trect, qbitmap);
        }

        if (this->is_recording && !this->is_replaying) {
            this->graph_capture->draw(bitmap_data, bmp);
            struct timeval time;
            gettimeofday(&time, nullptr);
            this->capture.get()->periodic_snapshot(time, this->mouse_data.x, this->mouse_data.y, false);
        }

        if (this->wab_diag_question) {
            this->answer_question(this->asked_color);
        }
    }


    void draw(const RDPLineTo & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        // TODO clipping
        if (this->connected || this->is_replaying) {
            this->screen->setPenColor(this->u32_to_qcolor(cmd.back_color, color_ctx));

            this->screen->paintCache().drawLine(cmd.startx, cmd.starty, cmd.endx, cmd.endy);
        }

        if (this->is_recording && !this->is_replaying) {
            this->graph_capture->draw(cmd, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->info.bpp), &this->mod_palette));
            struct timeval time;
            gettimeofday(&time, nullptr);
            this->capture.get()->periodic_snapshot(time, this->mouse_data.x, this->mouse_data.y, false);
        }

        if (this->wab_diag_question) {
            this->answer_question(this->asked_color);
        }
    }


    void draw(const RDPScrBlt & cmd, Rect clip) override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        //std::cout << "RDPScrBlt" << std::endl;

        const Rect drect = clip.intersect(this->info.width, this->info.height).intersect(cmd.rect);
        if (drect.isempty()) {
            return;
        }

        int srcx(drect.x + cmd.srcx - cmd.rect.x);
        int srcy(drect.y + cmd.srcy - cmd.rect.y);

        switch (cmd.rop) {

            case 0x00:
                if (this->connected || this->is_replaying) {
                    this->screen->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
                }
                break;

            case 0x55: this->draw_RDPScrBlt(srcx, srcy, drect, true);
                break;

            case 0xAA: // nothing to change
                break;

            case 0xCC: this->draw_RDPScrBlt(srcx, srcy, drect, false);
                break;

            case 0xFF:
                if (this->connected || this->is_replaying) {
                    this->screen->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::white);
                }
                break;
            default: LOG(LOG_WARNING, "DEFAULT: RDPScrBlt rop = %x", cmd.rop);
                break;
        }

        if (this->is_recording && !this->is_replaying) {
            this->graph_capture->draw(cmd, clip);
            struct timeval time;
            gettimeofday(&time, nullptr);
            this->capture.get()->periodic_snapshot(time, this->mouse_data.x, this->mouse_data.y, false);
        }

        if (this->wab_diag_question) {
            this->answer_question(this->asked_color);
        }
    }


    void draw(const RDPMemBlt & cmd, Rect clip, const Bitmap & bitmap) override {
         if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
         }
        //std::cout << "RDPMemBlt (" << std::hex << static_cast<int>(cmd.rop) << ")" <<  std::dec <<  std::endl;
        const Rect drect = clip.intersect(cmd.rect);
        if (drect.isempty()){
            return ;
        }

        switch (cmd.rop) {

            case 0x00:
                if (this->connected || this->is_replaying) {
                    this->screen->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
                }
                break;

            case 0x22: this->draw_memblt_op<Op_0x22>(drect, bitmap);
                break;

            case 0x33: this->draw_MemBlt(drect, bitmap, true, cmd.srcx + (drect.x - cmd.rect.x), cmd.srcy + (drect.y - cmd.rect.y));
                break;

            case 0x55:
                this->draw_memblt_op<Op_0x55>(drect, bitmap);
                break;

            case 0x66: this->draw_memblt_op<Op_0x66>(drect, bitmap);
                break;

            case 0x99:  this->draw_memblt_op<Op_0x99>(drect, bitmap);
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
                if (this->connected || this->is_replaying) {
                    this->screen->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::white);
                }
                break;

            default: LOG(LOG_WARNING, "DEFAULT: RDPMemBlt rop = %x", cmd.rop);
                break;
        }

        if (this->is_recording && !this->is_replaying) {
            this->graph_capture->draw(cmd, clip, bitmap);
            struct timeval time;
            gettimeofday(&time, nullptr);
            this->capture.get()->periodic_snapshot(time, this->mouse_data.x, this->mouse_data.y, false);
        }

        if (this->wab_diag_question) {
            this->answer_question(this->asked_color);
        }
    }


    void draw(const RDPMem3Blt & cmd, Rect clip, gdi::ColorCtx color_ctx, const Bitmap & bitmap) override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        const Rect drect = clip.intersect(cmd.rect);
        if (drect.isempty()){
            return ;
        }

        switch (cmd.rop) {
            case 0xB8:
            {
                const uint16_t mincx = std::min<int16_t>(bitmap.cx(), std::min<int16_t>(this->info.width  - drect.x, drect.cx));
                const uint16_t mincy = std::min<int16_t>(bitmap.cy(), std::min<int16_t>(this->info.height - drect.y, drect.cy));

                if (mincx <= 0 || mincy <= 0) {
                    return;
                }

                const QColor fore(this->u32_to_qcolor(cmd.fore_color, color_ctx));
                const uint8_t r(fore.red());
                const uint8_t g(fore.green());
                const uint8_t b(fore.blue());

                int rowYCoord(drect.y + drect.cy-1);
                const QImage::Format format(this->bpp_to_QFormat(bitmap.bpp(), true));

                QImage dstBitmap(this->screen->getCache()->toImage().copy(drect.x, drect.y, mincx, mincy));
                QImage srcBitmap(bitmap.data(), mincx, mincy, bitmap.line_size(), format);
                srcBitmap = srcBitmap.convertToFormat(QImage::Format_RGB888);
                dstBitmap = dstBitmap.convertToFormat(QImage::Format_RGB888);

                const size_t rowsize(srcBitmap.bytesPerLine());
                std::unique_ptr<uchar[]> data = std::make_unique<uchar[]>(rowsize);

                for (size_t k = 1 ; k < drect.cy; k++) {

                    const uchar * srcData = srcBitmap.constScanLine(k);
                    const uchar * dstData = dstBitmap.constScanLine(mincy - k);

                    for (size_t x = 0; x < rowsize-2; x += 3) {
                        data[x  ] = ((dstData[x  ] ^ r) & srcData[x  ]) ^ r;
                        data[x+1] = ((dstData[x+1] ^ g) & srcData[x+1]) ^ g;
                        data[x+2] = ((dstData[x+2] ^ b) & srcData[x+2]) ^ b;
                    }

                    QImage image(data.get(), mincx, 1, srcBitmap.format());
                    if (image.depth() != this->info.bpp) {
                        image = image.convertToFormat(this->imageFormatRGB);
                    }
                    QRect trect(drect.x, rowYCoord, mincx, 1);
                    if (this->connected || this->is_replaying) {
                        this->screen->paintCache().drawImage(trect, image);
                    }
                    rowYCoord--;
                }
            }
            break;

            default: LOG(LOG_WARNING, "DEFAULT: RDPMem3Blt rop = %x", cmd.rop);
            break;
        }

        if (this->is_recording && !this->is_replaying) {
            this->graph_capture->draw(cmd, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->info.bpp), &this->mod_palette), bitmap);
            struct timeval time;
            gettimeofday(&time, nullptr);
            this->capture.get()->periodic_snapshot(time, this->mouse_data.x, this->mouse_data.y, false);
        }

        if (this->wab_diag_question) {
            this->answer_question(this->asked_color);
        }
    }


    void draw(const RDPDestBlt & cmd, Rect clip) override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        const Rect drect = clip.intersect(this->info.width, this->info.height).intersect(cmd.rect);

        switch (cmd.rop) {
            case 0x00: // blackness
                if (this->connected || this->is_replaying) {
                    this->screen->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
                }
                break;
            case 0x55:                                         // inversion
                this->draw_RDPScrBlt(drect.x, drect.y, drect, true);
                break;
            case 0xAA: // change nothing
                break;
            case 0xFF: // whiteness
                if (this->connected || this->is_replaying) {
                    this->screen->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::white);
                }
                break;
            default: LOG(LOG_WARNING, "DEFAULT: RDPDestBlt rop = %x", cmd.rop);
                break;
        }

        if (this->is_recording && !this->is_replaying) {
            this->graph_capture->draw(cmd, clip);
            struct timeval time;
            gettimeofday(&time, nullptr);
            this->capture.get()->periodic_snapshot(time, this->mouse_data.x, this->mouse_data.y, false);
        }

        if (this->wab_diag_question) {
            this->answer_question(this->asked_color);
        }
    }

    void draw(const RDPMultiDstBlt & cmd, Rect clip) override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        LOG(LOG_WARNING, "DEFAULT: RDPMultiDstBlt");
    }

    void draw(const RDPMultiOpaqueRect & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }


        (void) color_ctx;
        LOG(LOG_WARNING, "DEFAULT: RDPMultiOpaqueRect");
    }

    void draw(const RDP::RDPMultiPatBlt & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        (void) color_ctx;
        LOG(LOG_WARNING, "DEFAULT: RDPMultiPatBlt");
    }

    void draw(const RDP::RDPMultiScrBlt & cmd, Rect clip) override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }

        LOG(LOG_WARNING, "DEFAULT: RDPMultiScrBlt");
    }

    void draw(const RDPGlyphIndex & cmd, Rect clip, gdi::ColorCtx color_ctx, const GlyphCache & gly_cache) override {
         if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
         }

        Rect screen_rect = clip.intersect(this->info.width, this->info.height);
        if (screen_rect.isempty()){
            return ;
        }

        Rect const clipped_glyph_fragment_rect = cmd.bk.intersect(screen_rect);
        if (clipped_glyph_fragment_rect.isempty()) {
            return;
        }
        //std::cout << "RDPGlyphIndex " << std::endl;

        // set a background color
        {
            Rect ajusted = cmd.f_op_redundant ? cmd.bk : cmd.op;
            if ((ajusted.cx > 1) && (ajusted.cy > 1)) {
                ajusted.cy--;
                ajusted = ajusted.intersect(screen_rect);
                this->screen->paintCache().fillRect(ajusted.x, ajusted.y, ajusted.cx, ajusted.cy, this->u32_to_qcolor(cmd.fore_color, color_ctx));
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
                                if (clip.contains_pt(x + fc.offset + xx, y + fc.baseline + yy)
                                && (fc_bit_mask & *fc_data))
                                {
                                    if (this->connected || this->is_replaying) {
                                        this->screen->paintCache().fillRect(x + fc.offset + xx, y + fc.baseline + yy, 1, 1, color);
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
        if (this->is_recording && !this->is_replaying) {
            this->graph_capture->draw(cmd, clip, gdi::ColorCtx(gdi::Depth::from_bpp(this->info.bpp), &this->mod_palette), gly_cache);
            struct timeval time;
            gettimeofday(&time, nullptr);
            this->capture.get()->periodic_snapshot(time, this->mouse_data.x, this->mouse_data.y, false);
        }
    }

    void draw(const RDPPolygonSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            //cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        (void) cmd;
        (void) clip;
        (void) color_ctx;
        LOG(LOG_WARNING, "DEFAULT: RDPPolygonSC");

        /*RDPPolygonSC new_cmd24 = cmd;
        new_cmd24.BrushColor  = color_decode_opaquerect(cmd.BrushColor,  this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPPolygonCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            //cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
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
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        (void) color_ctx;
        LOG(LOG_WARNING, "DEFAULT: RDPPolyline");
        /*RDPPolyline new_cmd24 = cmd;
        new_cmd24.PenColor  = color_decode_opaquerect(cmd.PenColor,  this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPEllipseSC & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
        (void) cmd;
        (void) clip;
        (void) color_ctx;
        LOG(LOG_WARNING, "DEFAULT: RDPEllipseSC");

        /*RDPEllipseSC new_cmd24 = cmd;
        new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);*/
        //this->gd.draw(new_cmd24, clip);
    }

    void draw(const RDPEllipseCB & cmd, Rect clip, gdi::ColorCtx color_ctx) override {
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            //cmd.log(LOG_INFO, clip);
            LOG(LOG_INFO, "========================================\n");
        }
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
        if (bool(this->verbose & RDPVerbose::graphics)) {
            LOG(LOG_INFO, "--------- FRONT ------------------------");
            //order.log(LOG_INFO);
            LOG(LOG_INFO, "========================================\n");
        }

        if (this->is_recording && !this->is_replaying) {
            this->graph_capture->draw(order);
            struct timeval time;
            gettimeofday(&time, nullptr);
            this->capture.get()->periodic_snapshot(time, this->mouse_data.x, this->mouse_data.y, false);
        }
        LOG(LOG_INFO, "DEFAULT: FrameMarker");
    }

    void draw(RDPNineGrid const & cmd, Rect clip, gdi::ColorCtx color_ctx, Bitmap const & bmp) override {
        LOG(LOG_INFO, "DEFAULT: RDPNineGrid");
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //------------------------
    //      CONTROLLERS
    //------------------------

    void mousePressEvent(QMouseEvent *e, int screen_shift) override {
        if (this->mod != nullptr) {
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

            this->mod->rdp_input_mouse(flag | MOUSE_FLAG_DOWN, e->x() + screen_shift, e->y(), &(this->keymap));
        }
    }

    void mouseReleaseEvent(QMouseEvent *e, int screen_shift) override {
        if (this->mod != nullptr) {
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

            this->mod->rdp_input_mouse(flag, e->x() + screen_shift, e->y(), &(this->keymap));
        }
    }

    void keyPressEvent(QKeyEvent *e) override {
        this->qtRDPKeymap.keyEvent(0     , e);
        if (this->qtRDPKeymap.scanCode != 0) {
            this->send_rdp_scanCode(this->qtRDPKeymap.scanCode, this->qtRDPKeymap.flag);
        }
    }

    void keyReleaseEvent(QKeyEvent *e) override {
        this->qtRDPKeymap.keyEvent(KBD_FLAG_UP, e);
        if (this->qtRDPKeymap.scanCode != 0) {
            this->send_rdp_scanCode(this->qtRDPKeymap.scanCode, this->qtRDPKeymap.flag);
        }
    }

    void wheelEvent(QWheelEvent *e) override {
        int flag(MOUSE_FLAG_HWHEEL);
        if (e->delta() < 0) {
            flag = flag | MOUSE_FLAG_WHEEL_NEGATIVE;
        }
        if (this->mod != nullptr) {
            //this->mod->rdp_input_mouse(flag, e->x(), e->y(), &(this->keymap));
        }
    }

    bool eventFilter(QObject *, QEvent *e, int screen_shift) override {
        if (e->type() == QEvent::MouseMove)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);
            int x = mouseEvent->x() + screen_shift;
            int y = mouseEvent->y();

            if (x < 0) {
                x = 0;
            }
            if (y < 0) {
                y = 0;
            }

//             if (y > this->info.height) {
//                 LOG(LOG_INFO, "eventFilter out");
//                 this->screen->mouse_out = true;
//             } else if (this->screen->mouse_out) {
//                 this->screen->update_current_cursor();
//                 this->screen->mouse_out = false;
//             }

            if (this->mod != nullptr && y < this->info.height) {
                this->mouse_data.x = x;
                this->mouse_data.y = y;
                this->mod->rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, &(this->keymap));
            }
        }
        return false;
    }

    void connexionReleased() override {
        this->form->setCursor(Qt::WaitCursor);
        this->user_name     = this->form->get_userNameField();
        this->target_IP     = this->form->get_IPField();
        this->user_password = this->form->get_PWDField();
        this->port          = this->form->get_portField();

        //bool res(false);
        if (!this->target_IP.empty()){
            this->connect();
        }
        this->form->setCursor(Qt::ArrowCursor);
        //return res;
    }

    void RefreshPressed() override {
        Rect rect(0, 0, this->info.width, this->info.height);
        this->mod->rdp_input_invalidate(rect);
    }

    void CtrlAltDelPressed() {
        int flag = Keymap2::KBDFLAGS_EXTENDED;

        this->send_rdp_scanCode(KBD_SCANCODE_ALTGR , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_CTRL  , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_DELETE, flag);
    }

    void CtrlAltDelReleased() {
        int flag = Keymap2::KBDFLAGS_EXTENDED | KBD_FLAG_UP;

        this->send_rdp_scanCode(KBD_SCANCODE_ALTGR , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_CTRL  , flag);
        this->send_rdp_scanCode(KBD_SCANCODE_DELETE, flag);
    }

    void disconnexionReleased() override{
        this->is_replaying = false;
        this->dropScreen();
        this->disconnect("");
        this->cache = nullptr;
        this->trans_cache = nullptr;
        this->capture = nullptr;
        this->graph_capture = nullptr;
    }

    void setMainScreenOnTopRelease() override {
        if (this->connected || this->is_replaying) {
            this->screen->activateWindow();
        }
    }

    void send_rdp_scanCode(int keyCode, int flag) {
        Keymap2::DecodedKeys decoded_keys = this->keymap.event(flag, keyCode, this->ctrl_alt_delete);
        switch (decoded_keys.count)
        {
        case 2:
            if (this->decoded_data.has_room(sizeof(uint32_t))) {
                this->decoded_data.out_uint32_le(decoded_keys.uchars[0]);
            }
            if (this->decoded_data.has_room(sizeof(uint32_t))) {
                this->decoded_data.out_uint32_le(decoded_keys.uchars[1]);
            }
            break;
        case 1:
            if (this->decoded_data.has_room(sizeof(uint32_t))) {
                this->decoded_data.out_uint32_le(decoded_keys.uchars[0]);
            }
            break;
        default:
        case 0:
            break;
        }
        if (this->mod != nullptr) {
            this->mod->rdp_input_scancode(keyCode, 0, flag, this->_timer, &(this->keymap));
        }
    }

    void closeFromScreen() override {

        if (this->screen != nullptr) {
            this->screen->disconnection();
            this->screen->close();
            this->screen = nullptr;
        }

        this->capture = nullptr;
        this->graph_capture = nullptr;

        if (this->form != nullptr && this->connected) {
            this->form->close();
        }
    }

    void dropScreen() override {
        if (this->screen != nullptr) {
            this->screen->disconnection();
            this->screen->close();
            this->screen = nullptr;
        }
    }

    void replay(std::string const & movie_dir_, std::string const & movie_path_) override {
        if (movie_path_.empty()) {
//             this->readError(movie_path_);
            return;
        }

        this->is_replaying = true;
        //this->setScreenDimension();
        if (this->load_replay_mod(movie_dir_, movie_path_, {0, 0}, {0, 0})) {
            this->info.width = this->replay_mod->get_dim().w;
            this->info.height = this->replay_mod->get_dim().h;
            this->cache_replay = new QPixmap(this->info.width, this->info.height);
            this->trans_cache = new QPixmap(this->info.width, this->info.height);
            this->trans_cache->fill(Qt::transparent);
            this->screen = new Screen_Qt(this, this->cache_replay, movie_dir_, movie_path_, this->trans_cache);
            //this->connected = true;
            this->form->hide();
            if (this->replay_mod->get_wrm_version() == WrmVersion::v2) {
                this->bar = new ProgressBarWindow(this->screen->movie_time, this);
                this->screen->pre_load_movie();
            }
            this->screen->show();
        }
    }

    void readError(std::string const & movie_path) {
        const std::string errorMsg("Cannot read movie \""+movie_path+ "\".");
        LOG(LOG_INFO, "%s", errorMsg.c_str());
        std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");

        this->form->set_ErrorMsg(labelErrorMsg);
    }


//     void start_capture() {
//         Inifile ini;
//             ini.set<cfg::video::capture_flags>(CaptureFlags::wrm | CaptureFlags::png);
//             ini.set<cfg::video::png_limit>(0);
//             ini.set<cfg::video::disable_keyboard_log>(KeyboardLogFlags::none);
//             ini.set<cfg::session_log::enable_session_log>(0);
//             ini.set<cfg::session_log::keyboard_input_masking_level>(KeyboardInputMaskingLevel::unmasked);
//             ini.set<cfg::context::pattern_kill>("");
//             ini.set<cfg::context::pattern_notify>("");
//             ini.set<cfg::debug::capture>(0xfffffff);
//             ini.set<cfg::video::capture_groupid>(1);
//             ini.set<cfg::video::record_tmp_path>(this->REPLAY_DIR);
//             ini.set<cfg::video::record_path>(this->REPLAY_DIR);
//             ini.set<cfg::video::hash_path>(this->REPLAY_DIR+std::string("/signatures"));
//             time_t now;
//             time(&now);
//             std::string data(ctime(&now));
//             std::string data_cut(data.c_str(), data.size()-1);
//             std::string name("-Replay");
//             std::string movie_name(data_cut+name);
//             ini.set<cfg::globals::movie_path>(movie_name.c_str());
//             ini.set<cfg::globals::trace_type>(TraceType::localfile);
//             ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);
//             ini.set<cfg::video::frame_interval>(std::chrono::duration<unsigned, std::ratio<1, 100>>(1));
//             ini.set<cfg::video::break_interval>(std::chrono::seconds(600));
//
//         UdevRandom gen;
//         CryptoContext cctx;
//         NullReportMessage * reportMessage  = nullptr;
//         struct timeval time;
//         gettimeofday(&time, nullptr);
//         PngParams png_params = {0, 0, std::chrono::milliseconds{60}, 100, 0, true, reportMessage, ini.get<cfg::video::record_tmp_path>().c_str(), "", 1};
//         FlvParams flv_params = flv_params_from_ini(this->info.width, this->info.height, ini);
//         OcrParams ocr_params = { ini.get<cfg::ocr::version>(),
//                                     static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>()),
//                                     ini.get<cfg::ocr::on_title_bar_only>(),
//                                     ini.get<cfg::ocr::max_unrecog_char_rate>(),
//                                     ini.get<cfg::ocr::interval>(),
//                                     0
//                                 };
//
//
//         std::string record_path = this->REPLAY_DIR.c_str() + std::string("/");
//
//
//
//         WrmParams wrmParams(
//                 this->info.bpp
// //                     , TraceType::localfile
//             , cctx
//             , gen
//             , this->fstat
//             , record_path.c_str()
//             , ini.get<cfg::video::hash_path>().c_str()
//             , movie_name.c_str()
//             , ini.get<cfg::video::capture_groupid>()
//             , std::chrono::duration<unsigned int, std::ratio<1l, 100l> >{60}
//             , ini.get<cfg::video::break_interval>()
//             , WrmCompressionAlgorithm::no_compression
//             , 0
//         );
//
//         PatternCheckerParams patternCheckerParams;
//         SequencedVideoParams sequenced_video_params;
//         FullVideoParams full_video_params;
//         MetaParams meta_params;
//         KbdLogParams kbd_log_params;
//
//         this->capture = std::make_unique<Capture>(true, wrmParams
//                                         , false, png_params
//                                         , false, patternCheckerParams
//                                         , false, ocr_params
//                                         , false, sequenced_video_params
//                                         , false, full_video_params
//                                         , false, meta_params
//                                         , false, kbd_log_params
//                                         , ""
//                                         , time
//                                         , this->info.width
//                                         , this->info.height
//                                         , ini.get<cfg::video::record_tmp_path>().c_str()
//                                         , ini.get<cfg::video::record_tmp_path>().c_str()
//                                         , 1
//                                         , flv_params
//                                         , false
//                                         , &(this->reportMessage)
//                                         , nullptr
//                                         , ""
//                                         , ""
//                                         , 0xfffffff
//                                         , false
//                                         , std::chrono::duration<long int>{60}
//                                         , false
//                                         , false
//                                         , false
//                                         , false
//                                         , false
//                                         , false
//                                         );
//
//         this->graph_capture = this->capture.get()->get_graphic_api();
//     }

    virtual void connect() {
        this->is_pipe_ok = true;
        if (this->mod_qt->connect()) {
            this->qtRDPKeymap.setKeyboardLayout(this->info.keylayout);
            this->cache = new QPixmap(this->info.width, this->info.height);
            this->trans_cache = new QPixmap(this->info.width, this->info.height);
            this->trans_cache->fill(Qt::transparent);

            this->screen = new Screen_Qt(this, this->cache, this->trans_cache);

            this->is_replaying = false;
            if (this->is_recording && !this->is_replaying) {

//                 this->start_capture();

                   Inifile ini;
                    ini.set<cfg::video::capture_flags>(CaptureFlags::wrm | CaptureFlags::png);
                    ini.set<cfg::video::png_limit>(0);
                    ini.set<cfg::video::disable_keyboard_log>(KeyboardLogFlags::none);
                    ini.set<cfg::session_log::enable_session_log>(0);
                    ini.set<cfg::session_log::keyboard_input_masking_level>(KeyboardInputMaskingLevel::unmasked);
                    ini.set<cfg::context::pattern_kill>("");
                    ini.set<cfg::context::pattern_notify>("");
                    ini.set<cfg::debug::capture>(0xfffffff);
                    ini.set<cfg::video::capture_groupid>(1);
                    ini.set<cfg::video::record_tmp_path>(this->REPLAY_DIR);
                    ini.set<cfg::video::record_path>(this->REPLAY_DIR);
                    ini.set<cfg::video::hash_path>(this->REPLAY_DIR+std::string("/signatures"));
                    time_t now;
                    time(&now);
                    std::string data(ctime(&now));
                    std::string data_cut(data.c_str(), data.size()-1);
                    std::string name("-Replay");
                    std::string movie_name(data_cut+name);
                    ini.set<cfg::globals::movie_path>(movie_name.c_str());
                    ini.set<cfg::globals::trace_type>(TraceType::localfile);
                    ini.set<cfg::video::wrm_compression_algorithm>(WrmCompressionAlgorithm::no_compression);
                    ini.set<cfg::video::frame_interval>(std::chrono::duration<unsigned, std::ratio<1, 100>>(1));
                    ini.set<cfg::video::break_interval>(std::chrono::seconds(600));

                UdevRandom gen;

                //NullReportMessage * reportMessage  = nullptr;
                struct timeval time;
                gettimeofday(&time, nullptr);
                PngParams png_params = {0, 0, std::chrono::milliseconds{60}, 100, true, this->info.remote_program, static_cast<bool>(ini.get<cfg::video::rt_display>())};
                VideoParams videoParams = {Level::high, this->info.width, this->info.height, 0, 0, 0, std::string(""), true, true, false, ini.get<cfg::video::break_interval>(), 0};
                OcrParams ocr_params = { ini.get<cfg::ocr::version>(),
                                            static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>()),
                                            ini.get<cfg::ocr::on_title_bar_only>(),
                                            ini.get<cfg::ocr::max_unrecog_char_rate>(),
                                            ini.get<cfg::ocr::interval>(),
                                            0
                                        };

                std::string record_path = this->REPLAY_DIR.c_str() + std::string("/");



                WrmParams wrmParams(
                      this->info.bpp
                    , this->cctx
                    , gen
                    , this->fstat
                    , ini.get<cfg::video::hash_path>().c_str()
                    , std::chrono::duration<unsigned int, std::ratio<1l, 100l> >{60}
                    , ini.get<cfg::video::break_interval>()
                    , WrmCompressionAlgorithm::no_compression
                    , 0
                );

                PatternParams patternCheckerParams;
                SequencedVideoParams sequenced_video_params;
                FullVideoParams full_video_params = { false };
                MetaParams meta_params;
                KbdLogParams kbd_log_params;

                CaptureParams captureParams;
                captureParams.now = tvtime();
                captureParams.basename = movie_name.c_str();
                captureParams.record_tmp_path = record_path.c_str();
                captureParams.record_path = record_path.c_str();
                captureParams.groupid = 0;
                captureParams.report_message = nullptr;

                DrawableParams drawableParams;
                drawableParams.width  = this->info.width;
                drawableParams.height = this->info.height;
                drawableParams.rdp_drawable = nullptr;

                this->capture = std::make_unique<Capture>(captureParams
                                                , drawableParams
                                                , true, wrmParams
                                                , false, png_params
                                                , false, patternCheckerParams
                                                , false, ocr_params
                                                , false, sequenced_video_params
                                                , false, full_video_params
                                                , false, meta_params
                                                , false, kbd_log_params
                                                , videoParams
                                                , nullptr
                                                , Rect(0, 0, 0, 0)
                                                );

                this->capture.get()->gd_drawable->width();

                this->graph_capture = this->capture.get()->get_graphic_api();
            }

            if (this->mod_qt->listen()) {
                this->form->hide();
                this->screen->show();
                this->connected = true;

            } else {
                this->connected = false;
            }
        }
    }

    void disconnect(std::string const & error) override {

        if (this->mod_qt != nullptr) {
            this->mod_qt->disconnect(true);
        }

        //this->must_be_stop_capture();

        this->form->set_IPField(this->target_IP);
        this->form->set_portField(this->port);
        this->form->set_PWDField(this->user_password);
        this->form->set_userNameField(this->user_name);
        this->form->set_ErrorMsg(error);
        this->form->show();

        this->connected = false;
    }

    virtual const CHANNELS::ChannelDefArray & get_channel_list(void) const override {
        return this->cl;
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //--------------------------------
    //    SOCKET EVENTS FUNCTIONS
    //--------------------------------

    virtual void callback() override {

        if (this->mod != nullptr) {
            try {
                this->mod->draw_event(time(nullptr), *(this));
                if (!this->is_pipe_ok) {
                    this->dropScreen();
                    const std::string errorMsg("Error: Connection to [" + this->target_IP +  "] is closed. Broken pipe.");
                    LOG(LOG_INFO, "%s", errorMsg.c_str());
                    std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");

                    this->mod_qt->disconnect(false);
                    this->disconnect(labelErrorMsg);
                }
            } catch (const Error & e) {
                this->dropScreen();
                const std::string errorMsg("Error: Connection to [" + this->target_IP +  "] is closed. Error "+ e.errmsg());
                LOG(LOG_INFO, "%s", errorMsg.c_str());
                std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");

                this->disconnect(labelErrorMsg);
            }
        }

    }


};

