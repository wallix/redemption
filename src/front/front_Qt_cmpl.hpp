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

#ifndef FRONT_QT_CMPL_HPP
#define FRONT_QT_CMPL_HPP

//#include <algorithm>
#include <string> 
#include <unistd.h>
/*
#include <signal.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "listen.hpp"
#include "parse_ip_conntrack.hpp"
*/

#include "socket_transport.hpp"
#include "rdp/rdp.hpp"
#include "../src/front/front_Qt.hpp"


Front_Qt::Front_Qt(char* argv[] = {}, int argc = 0, uint32_t verbose = 0)
    : QWidget(), FrontAPI(false, false)
    , verbose(verbose)
    , info()
    , mod_bpp(24)
    , mod_palette(BGRPalette::no_init())
    , _label(this)
    , _picture() 
    , _pen() 
    , _painter()
    , _form(this)
    , _userNameLabel(QString("User name : "), &(this->_form))         
    , _IPLabel(QString      ("IP serveur :"), &(this->_form))   
    , _PWDLabel(QString  ("Password :  "), &(this->_form))   
    , _portLabel(QString    ("Port :      "), &(this->_form))  
    , _formLayout(&(this->_form))
    , _sckRead(nullptr)
    , _callback(nullptr)
    , _sck(nullptr)      
    , _port(0)  
    , _keymap() 
    , _ctrl_alt_delete(false)
    , _timer(0)
    , _buttonCtrlAltDel("CTRL + ALT + DELETE", this) 
    , _buttonRefresh("Refresh", this)
    , _buttonConnexion("Connexion", this)
    , _buttonDisconnexion("Disconnexion", this)
    , _userNameField("", this)
    , _IPField("", this)
    , _PWDField("", this)
    , _portField("", this)
    , _qtRDPKeymap(0x040C+0x80000000) 
    , _mouseFlag(0)
    {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        info.keylayout = 0x040C;// 0x40C FR, 0x409 USA
        info.console_session = 0;
        info.brush_cache_code = 0;
        info.bpp = 24;
        info.width = 800;
        info.height = 600;
        info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        char * localIPtmp = "10.10.43.46";
        /*union
        {
            struct sockaddr s;
            struct sockaddr_storage ss;
            struct sockaddr_in s4;
            struct sockaddr_in6 s6;
        } localAddress;
        socklen_t addressLength = sizeof(localAddress);
        int sck = 0;
        if (-1 == getsockname(sck, &localAddress.s, &addressLength)){
            std::cout << "Error, local adress not found." << std::endl;
            _exit(1);
        }
        strcpy(localIPtmp, inet_ntoa(localAddress.s4.sin_addr));*/
        this->_localIPtmp2   = localIPtmp;
        this->_nbTry         = 3;
        this->_retryDelay    = 1000;
        
        uint8_t commandIsValid(0);
        
        for (int i = 0; i <  argc - 1; i++) {

            std::string word(argv[i]);
            std::string arg(argv[i+1]);
            
            if (word == "-n") {
                this->_userName = arg;
                commandIsValid += Front_Qt::NAME_GOTTEN;
            } else if (word == "-pwd") {
                this->_pwd = arg; 
                commandIsValid += Front_Qt::PWD_GOTTEN;
            } else if (word == "-ip") {
                this->_targetIP = arg;
                commandIsValid += Front_Qt::IP_GOTTEN;
            } else if (word == "-p") {
                std::string portStr(arg);
                this->_port = std::stoi(portStr);  
                commandIsValid += Front_Qt::PORT_GOTTEN;
            }
        }
        
        if (this->mod_bpp == 8) {
            this->mod_palette = BGRPalette::classic_332();
        }
        
        int customKeys[2][2]
        { 
            { 0x152, 0xB2 }, //œ or square
            { 0x39c, 0xB5 }, // µ
        };
        this->_qtRDPKeymap.setCustomNoExtendedKeylayoutApplied(customKeys, 2);
        this->_keymap.init_layout(info.keylayout);

        QSize size(sizeHint());
        QDesktopWidget* desktop = QApplication::desktop();
        int centerW = (desktop->width()/2)  - (size.width()/2);
        int centerH = (desktop->height()/2) - (size.height()/2);
        this->move(centerW, centerH);   
        
        this->_painter.setRenderHint(QPainter::Antialiasing);
        this->_pen.setWidth(1);
        this->_painter.setPen(this->_pen);
        this->_painter.fillRect(0, 0, this->info.width, this->info.height, Qt::white);
        this->_label.setMouseTracking(true);
        this->_label.installEventFilter(this);
        this->setAttribute(Qt::WA_NoSystemBackground, true);
        
        QRect rectCtrlAltDel(QPoint(0, this->info.height+1),QSize(this->info.width/3, 20));
        this->initButton(this->_buttonCtrlAltDel, "CTRL + ALT + DELETE", rectCtrlAltDel);
        this->QObject::connect(&(this->_buttonCtrlAltDel)  , SIGNAL (pressed()),  this, SLOT (CtrlAltDelPressed()));
        this->QObject::connect(&(this->_buttonCtrlAltDel)  , SIGNAL (released()), this, SLOT (CtrlAltDelReleased()));

        QRect rectRefresh(QPoint(this->info.width/3, this->info.height+1),QSize(this->info.width/3, 20));
        this->initButton(this->_buttonRefresh, "Refresh", rectRefresh);
        this->QObject::connect(&(this->_buttonRefresh)     , SIGNAL (pressed()),  this, SLOT (RefreshPressed()));
        this->QObject::connect(&(this->_buttonRefresh)     , SIGNAL (released()), this, SLOT (RefreshReleased()));
        
        QRect rectDisconnexion(QPoint(((this->info.width/3)*2), this->info.height+1),QSize(this->info.width-((this->info.width/3)*2), 20));
        this->initButton(this->_buttonDisconnexion, "Disconnexion", rectDisconnexion);
        this->QObject::connect(&(this->_buttonDisconnexion), SIGNAL (pressed()),  this, SLOT (disconnexionPressed()));
        this->QObject::connect(&(this->_buttonDisconnexion), SIGNAL (released()), this, SLOT (disconnexionRelease()));
        
        QRect rectConnexion(QPoint(280, 256), QSize(110, 24)); 
        this->initButton(this->_buttonConnexion, "Connexion", rectConnexion);
        this->QObject::connect(&(this->_buttonConnexion)   , SIGNAL (pressed()),  this, SLOT (connexionPressed()));
        this->QObject::connect(&(this->_buttonConnexion)   , SIGNAL (released()), this, SLOT (connexionRelease()));
        
        this->_PWDField.setEchoMode(QLineEdit::Password);
        this->_PWDField.setInputMethodHints(Qt::ImhHiddenText| Qt::ImhNoPredictiveText|Qt::ImhNoAutoUppercase);
        this->_form.setFixedSize(400, 200);
        this->_formLayout.addRow(&(this->_IPLabel)      , &(this->_IPField));
        this->_formLayout.addRow(&(this->_userNameLabel), &(this->_userNameField));
        this->_formLayout.addRow(&(this->_PWDLabel)     , &(this->_PWDField));
        this->_formLayout.addRow(&(this->_portLabel)    , &(this->_portField));
        this->_form.setLayout(&(this->_formLayout));
            
        this->setFocusPolicy(Qt::StrongFocus);
   
        if (commandIsValid == Front_Qt::COMMAND_VALID) {
            
            std::cout << "command valid" << std::endl;
            
            this->connect();
            
        } else {
            std::cout << "missing argument(s) (" << (int)commandIsValid << "): ";
            if (!(commandIsValid & Front_Qt::NAME_GOTTEN)) {
                std::cout << "-n [user_name] ";
            } 
            if (!(commandIsValid & Front_Qt::PWD_GOTTEN)) {
                std::cout << "-pwd [password] ";
            } 
            if (!(commandIsValid & Front_Qt::IP_GOTTEN)) {
                std::cout << "-ip [ip_serveur] ";
            } 
            if (!(commandIsValid & Front_Qt::PORT_GOTTEN)) {
                std::cout << "-p [port] ";
            }
            std::cout << std::endl;
            
            this->disconnect();
        }
    }
    
    
    Front_Qt::~Front_Qt() {
        if (this->_sckRead != nullptr) {
            delete (this->_sckRead);
        }
        if (this->_callback != nullptr) {
            delete (this->_callback);
        }
        if (this->_sck != nullptr) {
            delete (this->_sck);
        }
    }
    
    
    void Front_Qt::initButton(QPushButton & button, const char * str, QRect & rect) {
        button.setToolTip(QString(str));
        button.setGeometry(rect);
        button.setCursor(Qt::PointingHandCursor);
        button.setFocusPolicy(Qt::NoFocus);
    }
    
    
    void Front_Qt::connect() {
        
        const char * name(this->_userName.c_str());      
        const char * pwd(this->_pwd.c_str()); 
        const char * targetIP(this->_targetIP.c_str());         
        const char * localIP(this->_localIPtmp2.c_str());
        
        std::cout << name << " " << pwd << " " << targetIP << " " << this->_port << std::endl;
        bool changeView(false);
        
        int client_sck = ip_connect(targetIP, this->_port, this->_nbTry, this->_retryDelay, this->verbose);
        
        if (client_sck > 0) {
            try {
                
                std::string error_message;
                this->_sck = new SocketTransport( name
                                                , client_sck
                                                , targetIP
                                                , this->_port
                                                , this->verbose
                                                , &error_message
                                                );
                
                
                changeView = true;
                
            } catch (const std::exception & e) {
                std::cout << "Can not connect to [" << targetIP <<  "]." << std::endl;
                this->disconnect();
            }
        } else {
            std::cout << "Can not connect to [" << targetIP <<  "]." << std::endl;
            this->disconnect();
        }
        
        if (changeView) {
            
            Inifile ini;
            ModRDPParams mod_rdp_params( name
                                        , pwd
                                        , targetIP
                                        , localIP
                                        , 2
                                        , 0
                                        );  
            mod_rdp_params.device_id                       = "device_id";
            mod_rdp_params.enable_tls                      = false;
            mod_rdp_params.enable_nla                      = false;
            //mod_rdp_params.enable_krb                      = false;
            //mod_rdp_params.enable_clipboard                = true;
            mod_rdp_params.enable_fastpath                 = false;
            mod_rdp_params.enable_mem3blt                  = false;
            mod_rdp_params.enable_bitmap_update            = true;
            mod_rdp_params.enable_new_pointer              = false;
            //mod_rdp_params.rdp_compression                 = 0;
            //mod_rdp_params.error_message                   = nullptr;
            //mod_rdp_params.disconnect_on_logon_user_change = false;
            //mod_rdp_params.open_session_timeout            = 0;
            //mod_rdp_params.certificate_change_action       = 0;
            //mod_rdp_params.extra_orders                    = "";
            mod_rdp_params.server_redirection_support        = true;
                
            // set view  
            this->_form.hide(); 
            this->_buttonConnexion.hide();
            this->_label.sizeHint();
            this->setFixedSize(this->info.width, this->info.height+20);
            this->setAttribute(Qt::WA_NoSystemBackground, true);
            this->reInitView();
            this->_painter.fillRect(0, 0, this->info.width, this->info.height, Qt::white);
            this->flush();
            this->_buttonCtrlAltDel.show();
            this->_buttonRefresh.show();
            this->_buttonDisconnexion.show();
            
            LCGRandom gen(0); // To always get the same client random, in tests
            this->_callback = new mod_rdp(*(this->_sck), *(this), info, ini.get_ref<cfg::mod_rdp::redir_info>(), gen, mod_rdp_params);
            this->_sckRead = new QSocketNotifier(client_sck, QSocketNotifier::Read, this);
            this->QObject::connect(this->_sckRead, SIGNAL(activated(int)), this, SLOT(call_Draw()));
        }    
        
        this->setCursor(Qt::ArrowCursor);
    }
    
    
    void Front_Qt::disconnect() {
        
        if (this->_sckRead != nullptr) {
            delete (this->_sckRead);
            this->_sckRead = nullptr;
        }
        if (this->_callback != nullptr) {
            delete (this->_callback);
            this->_callback = nullptr;
        }
        if (this->_sck != nullptr) {
            delete (this->_sck);
            this->_sck = nullptr;
        }
        
        this->setFixedSize(400, 300);
        this->_painter.fillRect(0, 0, 400, 300, Qt::white);
        this->flush();
        this->setAttribute(Qt::WA_NoSystemBackground, false);
        
        this->_userNameField.clear();
        this->_IPField.clear();
        this->_PWDField.clear();
        this->_portField.clear();
        this->_userNameField.insert(QString(this->_userName.c_str()));
        this->_IPField.insert(QString(this->_targetIP.c_str()));
        this->_PWDField.insert(QString(this->_pwd.c_str()));
        this->_portField.insert(QString(std::to_string(this->_port).c_str() ));
        
        this->_form.show();
        this->_buttonCtrlAltDel.hide();
        this->_buttonRefresh.hide();
        this->_buttonDisconnexion.hide();
        this->_buttonConnexion.show();
        
        this->setCursor(Qt::ArrowCursor);
    }


#endif