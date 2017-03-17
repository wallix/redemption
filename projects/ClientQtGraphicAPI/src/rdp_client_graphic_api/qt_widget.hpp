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

#define LOGPRINT
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
#include <vector>
#include <boost/algorithm/string.hpp>

#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/hdreg.h>

#include "core/RDP/caches/brushcache.hpp"
#include "core/RDP/capabilities/colcache.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryDestBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPatBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMemBlt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolyline.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "core/RDP/orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "core/RDP/orders/AlternateSecondaryWindowing.hpp"

#include "mod/rdp/rdp_log.hpp"
#include "core/RDP/pointer.hpp"
// #include "core/RDP/clipboard.hpp"
// #include "core/FSCC/FileInformation.hpp"
// #include "core/RDP/channels/rdpdr.hpp"
#include "core/RDP/MonitorLayoutPDU.hpp"
#include "core/front_api.hpp"
// #include "core/channel_list.hpp"
#include "mod/mod_api.hpp"
#include "mod/internal/replay_mod.hpp"
#include "utils/bitmap.hpp"
#include "core/RDP/caches/glyphcache.hpp"
#include "core/RDP/capabilities/cap_glyphcache.hpp"
#include "core/RDP/bitmapupdate.hpp"
#include "keyboard/keymap2.hpp"
#include "core/client_info.hpp"
#include "keymaps/qt_scancode_keymap.hpp"
#include "capture/capture.hpp"
#include "transport/socket_transport.hpp"

#include "Qt4/Qt.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic pop

#endif


#define USER_CONF_PATH "/config/userConfig.config"
#define CB_FILE_TEMP_PATH "/clipboard_temp"
#define REPLAY_PATH "/replay"
#define KEY_SETTING_PATH "/config/keySetting.config"
#define LOGINS_PATH "/config/logins.config"
#define _SHARE_PATH "/share"


struct DummyAuthentifier : public auth_api
{
public:
    virtual void set_auth_channel_target(const char *) {}
    virtual void set_auth_error_message(const char *) {}
    virtual void report(const char * , const char *) {}
    virtual void log4(bool , const char *, const char * = nullptr) {}
    virtual void disconnect_target() {}
};


class Front_Qt_API : public FrontAPI
{

public:
    RDPVerbose        verbose;
    ClientInfo        info;
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


    QImage::Format       imageFormatRGB;
    QImage::Format       imageFormatARGB;
    Qt_ScanCode_KeyMap   qtRDPKeymap;
    QPixmap            * cache;
    QPixmap            * cache_replay;
    SocketTransport    * socket;
    TimeSystem           timeSystem;
    DummyAuthentifier    authentifier;
    bool                 is_spanning;


    std::string    MAIN_DIR;
    std::string    CB_TEMP_DIR;
    std::string    USER_CONF_DIR;
    std::string    REPLAY_DIR;
    std::string    SHARE_DIR;
    std::string    USER_CONF_LOG;




    Front_Qt_API( RDPVerbose verbose)
    : FrontAPI(false, false)
    , verbose(verbose)
    , info()
    , port(0)
    , local_IP("unknow_local_IP")
    , nbTry(3)
    , retryDelay(1000)
    , delta_time(1000000)
    , mod(nullptr)
    , replay_mod(nullptr)
    , is_recording(false)
    , is_replaying(false)
    , qtRDPKeymap()
    , cache(nullptr)
    , cache_replay(nullptr)
    , socket(nullptr)
    , is_spanning(false)
    , MAIN_DIR(MAIN_PATH)
    , CB_TEMP_DIR(MAIN_DIR + std::string(CB_FILE_TEMP_PATH))
    , USER_CONF_DIR(MAIN_DIR + std::string(USER_CONF_PATH))
    , REPLAY_DIR(MAIN_DIR + std::string(REPLAY_PATH))
    , SHARE_DIR(MAIN_DIR + std::string(_SHARE_PATH))
    , USER_CONF_LOG(MAIN_DIR + std::string(LOGINS_PATH))

    {}

    virtual void send_to_channel( const CHANNELS::ChannelDef & , uint8_t const *
                                , std::size_t , std::size_t , int ) override {}

    // CONTROLLER
    virtual bool connexionReleased() = 0;
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

    virtual void replay(std::string const & movie_path) = 0;
    virtual void load_replay_mod(std::string const & movie_name) = 0;
    virtual void delete_replay_mod() = 0;
    virtual void callback() = 0;

    virtual bool can_be_start_capture() override { return true; }
    virtual bool must_be_stop_capture() override { return true; }


    virtual void options() {
        LOG(LOG_WARNING, "No options window implemented yet.");
    };

    virtual mod_api * init_mod() = 0;

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
        this->disconnect();
    }


    void disconnect() {

        if (this->_sckRead != nullptr) {
            delete (this->_sckRead);
            this->_sckRead = nullptr;
        }

        if (this->_callback != nullptr) {
            TimeSystem timeobj;
            this->_callback->disconnect(timeobj.get_time().tv_sec);
            delete (this->_callback);
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

        this->_client_sck = ip_connect(targetIP, this->_front->port, this->_front->nbTry, this->_front->retryDelay);

        if (this->_client_sck > 0) {
            try {

                this->_sck = new SocketTransport( name
                                                , this->_client_sck
                                                , targetIP
                                                , this->_front->port
                                                , to_verbose_flags(0)
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
            std::string windowErrorMsg(errorMsg+" ip_connect error.");
            LOG(LOG_WARNING, "%s", windowErrorMsg.c_str());
            this->_front->disconnect("<font color='Red'>"+windowErrorMsg+"</font>");
            return false;
        }

        return true;
    }

    bool listen() {

        this->_callback = this->_front->init_mod();

//         struct      timeval time_mark = { 0, 50000 };
//         bool        run_session       = true;

        if (this->_callback !=  nullptr) {

//             while (run_session) {
//                 try {
//                     unsigned max = 0;
//                     fd_set   rfds;
//                     fd_set   wfds;
//
//                     io_fd_zero(rfds);
//                     io_fd_zero(wfds);
//                     struct timeval timeout = time_mark;
//
//                     this->_callback->get_event().wait_on_fd(this->_client_sck, rfds, max, timeout);
//
//                     int num = select(max + 1, &rfds, &wfds, nullptr, &timeout);
//
//                     LOG(LOG_INFO, "VNC CLIENT :: select num = %d\n", num);
//
//                     if (num < 0) {
//                         if (errno == EINTR) {
//                             continue;
//                         }
//
//                         LOG(LOG_INFO, "VNC CLIENT :: errno = %d\n", errno);
//                         break;
//                     }
//
//                     if (this->_callback->get_event().is_set(this->_client_sck, rfds)) {
//                         this->_callback->get_event().reset();
//                         this->_callback->draw_event(time(nullptr), *(this->_front));
//                     }
//
//                     if (this->_callback->is_up_and_running()) {
//                         run_session = false;
//                     }
//
//
//                 } catch (Error & e) {
//                     LOG(LOG_ERR, "VNC CLIENT :: Exception raised = %d!\n", e.id);
//                     run_session = false;
//                 };
//             }

//             while (!this->_callback->is_up_and_running()) {
//                 try {
//                     this->_callback->draw_event(time(nullptr), *(this->_front));
//                     LOG(LOG_INFO, "draw_event");
//
//                 } catch (const Error &) {
//                     const std::string errorMsg("Error: Failed during early negociations.");
//                     LOG(LOG_WARNING, "%s", errorMsg.c_str());
//                     std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");
//                     this->_front->dropScreen();
//                     this->_front->disconnect(labelErrorMsg);
//                     return false;
//                 }
//             }

            this->_sckRead = new QSocketNotifier(this->_client_sck, QSocketNotifier::Read, this);
            this->QObject::connect(this->_sckRead,   SIGNAL(activated(int)), this,  SLOT(call_draw_event()));

            this->QObject::connect(&(this->timer),   SIGNAL(timeout()), this,  SLOT(call_draw_event()));
            if (this->_callback->get_event().set_state) {
                struct timeval now = tvtime();
                int time_to_wake = (this->_callback->get_event().trigger_time.tv_usec - now.tv_usec) / 1000
                + (this->_callback->get_event().trigger_time.tv_sec - now.tv_sec) * 1000;

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

        return true;
    }


public Q_SLOTS:
    void call_draw_event() {
        if (this->_front->mod) {

            this->_front->callback();

            if (this->_callback->get_event().set_state) {
                struct timeval now = tvtime();
                int time_to_wake = ((this->_callback->get_event().trigger_time.tv_usec - now.tv_usec) / 1000)
                + ((this->_callback->get_event().trigger_time.tv_sec - now.tv_sec) * 1000);

                if (time_to_wake < 0) {
                    this->timer.stop();
                } else {
                    this->timer.start( time_to_wake );
                }
            } else {
                this->timer.stop();
            }
        }
    }



};







class Form_Qt : public QWidget
{

Q_OBJECT

public:
    enum : int {
        MAX_ACCOUNT_DATA = 10
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
    QCompleter         * _completer;
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
        , _IPLabel(      QString("IP serveur :"), this)
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

        QDesktopWidget* desktop = QApplication::desktop();
        int centerW = (desktop->width()/2)  - (this->_width/2);
        int centerH = (desktop->height()/2) - (this->_height/2);
        this->move(centerW, centerH);
    }

    ~Form_Qt() {}

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
            this->_completer = new QCompleter(stringList, this);
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
        return this->_userNameField.text().toStdString();
    }

    std::string get_PWDField() {
        return this->_PWDField.text().toStdString();
    }

    int get_portField() {
        return this->_portField.text().toInt();
    }



private Q_SLOTS:
    void targetPicked(int index) {
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
        this->_front->replay(str_movie_path);
    }

    void connexionReleased() {

        if (this->_front->connexionReleased()) {
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
            if (!alreadySet) {
                this->_accountData[this->_accountNB].title = title;
                this->_accountData[this->_accountNB].IP    = this->get_IPField();
                this->_accountData[this->_accountNB].name  = this->get_userNameField();
                this->_accountData[this->_accountNB].pwd   = this->get_PWDField();
                this->_accountData[this->_accountNB].port  = this->get_portField();
                this->_accountNB++;
                this->_lastTargetIndex = this->_accountNB;
                if (this->_accountNB > MAX_ACCOUNT_DATA) {
                    this->_accountNB = MAX_ACCOUNT_DATA;
                }
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
        //new DialogOptions_Qt(this->_front, this);
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
    int            _width;
    int            _height;
    QPixmap        _match_pixmap;
    bool           _connexionLasted;
    QTimer         _timer;
    QTimer         _timer_replay;
    uint8_t        _screen_index;

    bool           _running;
    std::string    _movie_name;

    enum : int {
        BUTTON_HEIGHT = 20
    };

    uchar cursor_data[Pointer::DATA_SIZE*4];
    int cursorHotx;
    int cursorHoty;
    bool mouse_out;

    Screen_Qt (Front_Qt_API * front, int screen_index, QPixmap * cache, QPixmap * trans_cache)
        : QWidget()
        , _front(front)
        , _buttonCtrlAltDel("CTRL + ALT + DELETE", this)
        , _buttonRefresh("Refresh", this)
        , _buttonDisconnexion("Disconnection", this)
        , _penColor(Qt::black)
        , _cache(cache)
        , _trans_cache(trans_cache)
        , _width(this->_front->info.width)
        , _height(this->_front->info.height)
        , _match_pixmap(this->_width, this->_height)
        , _connexionLasted(false)
        , _screen_index(screen_index)
        , _running(false)
        , cursorHotx(0)
        , cursorHoty(0)
        , mouse_out(false)
    {
        this->setMouseTracking(true);
        this->installEventFilter(this);
        this->setAttribute(Qt::WA_DeleteOnClose);
        std::string screen_index_str = std::to_string(int(this->_screen_index));
        std::string title = "Remote Desktop Player connected to [" + this->_front->target_IP +  "]. " + screen_index_str;
        this->setWindowTitle(QString(title.c_str()));

        if (this->_front->is_spanning) {
            this->setWindowState(Qt::WindowFullScreen);
            //this->_height -= 2*Front_Qt_API::BUTTON_HEIGHT;
        } else {
            this->setFixedSize(this->_width, this->_height + BUTTON_HEIGHT);
        }

        QDesktopWidget * desktop = QApplication::desktop();
        int shift(10 * this->_screen_index);
        uint32_t centerW = (desktop->width()/2)  - (this->_width/2);
        uint32_t centerH = (desktop->height()/2) - ((this->_height+BUTTON_HEIGHT)/2);
        if (this->_front->is_spanning) {
            centerW = 0;
            centerH = 0;
        }
        this->move(centerW + shift, centerH + shift);

        this->setFocusPolicy(Qt::StrongFocus);
    }

    Screen_Qt (Front_Qt_API * front, QPixmap * cache, std::string const & movie_path, QPixmap * trans_cache)
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
        , _match_pixmap(this->_width, this->_height)
        , _connexionLasted(false)
        , _timer_replay(this)
        , _screen_index(0)
        , _running(false)
        , _movie_name(movie_path)
        , cursorHotx(0)
        , cursorHoty(0)
        , mouse_out(false)
    {
        std::string title = "Remote Desktop Player " + this->_movie_name;
        this->setWindowTitle(QString(title.c_str()));
        this->setAttribute(Qt::WA_DeleteOnClose);
        this->paintCache().fillRect(0, 0, this->_width, this->_height, {0, 0, 0});

        if (this->_front->is_spanning) {
            this->setWindowState(Qt::WindowFullScreen);
        } else {
            this->setFixedSize(this->_width, this->_height + BUTTON_HEIGHT);
        }

        QRect rectCtrlAltDel(QPoint(0, this->_height+1),QSize(this->_width/3, BUTTON_HEIGHT));
        this->_buttonCtrlAltDel.setToolTip(this->_buttonCtrlAltDel.text());
        this->_buttonCtrlAltDel.setGeometry(rectCtrlAltDel);
        this->_buttonCtrlAltDel.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonCtrlAltDel)     , SIGNAL (pressed()),  this, SLOT (playPressed()));
        this->_buttonCtrlAltDel.setFocusPolicy(Qt::NoFocus);

        QRect rectRefresh(QPoint(this->_width/3, this->_height+1),QSize(this->_width/3, BUTTON_HEIGHT));
        this->_buttonRefresh.setToolTip(this->_buttonRefresh.text());
        this->_buttonRefresh.setGeometry(rectRefresh);
        this->_buttonRefresh.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonRefresh), SIGNAL (pressed()), this, SLOT (stopRelease()));
        this->_buttonRefresh.setFocusPolicy(Qt::NoFocus);

        QRect rectDisconnexion(QPoint(((this->_width/3)*2), this->_height+1),QSize(this->_width-((this->_width/3)*2), BUTTON_HEIGHT));
        this->_buttonDisconnexion.setToolTip(this->_buttonDisconnexion.text());
        this->_buttonDisconnexion.setGeometry(rectDisconnexion);
        this->_buttonDisconnexion.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonDisconnexion), SIGNAL (released()), this, SLOT (closeReplay()));
        this->_buttonDisconnexion.setFocusPolicy(Qt::NoFocus);

        uint32_t centerW = 0;
        uint32_t centerH = 0;
        if (!this->_front->is_spanning) {
            QDesktopWidget* desktop = QApplication::desktop();
            centerW = (desktop->width()/2)  - (this->_width/2);
            centerH = (desktop->height()/2) - ((this->_height+BUTTON_HEIGHT)/2);
        }
        this->move(centerW, centerH);

        this->QObject::connect(&(this->_timer_replay), SIGNAL (timeout()),  this, SLOT (playReplay()));

        this->setFocusPolicy(Qt::StrongFocus);
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

        uint32_t centerW = 0;
        uint32_t centerH = 0;
        if (!this->_front->is_spanning) {
            QDesktopWidget* desktop = QApplication::desktop();
            centerW = (desktop->width()/2)  - (this->_width/2);
            centerH = (desktop->height()/2) - ((this->_height+BUTTON_HEIGHT)/2);
        }
        this->move(centerW, centerH);

        this->setFocusPolicy(Qt::StrongFocus);
    }

    ~Screen_Qt() {
        if (!this->_connexionLasted) {
            this->_front->closeFromScreen();
        }
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
        painter.end();
    }

    QPixmap * getCache() {
        return this->_cache;
    }

    void setPenColor(QColor color) {
        this->_penColor = color;
    }


private:
    void mousePressEvent(QMouseEvent *e) {
        this->_front->mousePressEvent(e, 0);
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
            this->_running = false;
            this->_buttonCtrlAltDel.setText("Play");
            this->_timer_replay.stop();
        } else {
            this->_running = true;
            this->_buttonCtrlAltDel.setText("Pause");
            this->_timer_replay.start(1);
        }
    }

    void playReplay() {
        if (this->_front->replay_mod->play_qt()) {
            this->slotRepaint();
        }

        if (this->_front->replay_mod->get_break_privplay_qt()) {
            this->_timer_replay.stop();
            this->slotRepaint();
            this->_buttonCtrlAltDel.setText("Replay");
            this->_running = false;
            this->_front->load_replay_mod(this->_movie_name);
        }
    }

    void closeReplay() {
        this->_front->delete_replay_mod();
        this->_front->disconnexionReleased();
    }

    void slotRepaint() {

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
        this->_buttonCtrlAltDel.setText("Replay");
        this->_timer_replay.stop();
        this->_running = false;
        this->_front->load_replay_mod(this->_movie_name);
        this->_cache_painter.fillRect(0, 0, this->_width, this->_height, Qt::black);
        this->slotRepaint();
    }

    void disconnexionRelease(){
        this->_front->disconnexionReleased();
    }

    void setMainScreenOnTopRelease() {
        this->_front->setMainScreenOnTopRelease();
    }
};

