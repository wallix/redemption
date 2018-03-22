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


#include <stdio.h>
#include <stdlib.h>


#include "utils/log.hpp"
#include "core/RDP/MonitorLayoutPDU.hpp"
#include "core/channel_list.hpp"



#include "../../client_input_output_api.hpp"
#include "../keymaps/qt_scancode_keymap.hpp"
#include "qt_options_window.hpp"



#include <QtGui/QPainter>
#include <QtGui/QKeyEvent>

#if REDEMPTION_QT_VERSION == 4
#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtGui/name>
#else
#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtWidgets/name>
#endif

#include REDEMPTION_QT_INCLUDE_WIDGET(QApplication)
#include REDEMPTION_QT_INCLUDE_WIDGET(QDesktopWidget)
#include REDEMPTION_QT_INCLUDE_WIDGET(QCheckBox)
#include REDEMPTION_QT_INCLUDE_WIDGET(QComboBox)
#include REDEMPTION_QT_INCLUDE_WIDGET(QPushButton)
#include REDEMPTION_QT_INCLUDE_WIDGET(QLabel)
#include REDEMPTION_QT_INCLUDE_WIDGET(QFormLayout)
#include REDEMPTION_QT_INCLUDE_WIDGET(QLineEdit)
#include REDEMPTION_QT_INCLUDE_WIDGET(QFileDialog)
#include REDEMPTION_QT_INCLUDE_WIDGET(QComboBox)
#include REDEMPTION_QT_INCLUDE_WIDGET(QDialog)
#include REDEMPTION_QT_INCLUDE_WIDGET(QGridLayout)
#include REDEMPTION_QT_INCLUDE_WIDGET(QTabWidget)
#include REDEMPTION_QT_INCLUDE_WIDGET(QTableWidget)
#include REDEMPTION_QT_INCLUDE_WIDGET(QToolTip)
#include REDEMPTION_QT_INCLUDE_WIDGET(QWidget)
#include REDEMPTION_QT_INCLUDE_WIDGET(QScrollArea)

#undef REDEMPTION_QT_INCLUDE_WIDGET



#include <vector>





class FormReplay : public QWidget
{

Q_OBJECT

public:
    ClientInputMouseKeyboardAPI * controllers;

    QFormLayout lay;
    QPushButton buttonReplay;


    FormReplay(ClientInputMouseKeyboardAPI * controllers, QWidget * parent)
    : QWidget(parent)
    , controllers(controllers)
    , lay(this)
    , buttonReplay("Select a mwrm file", this)
    {
        QRect rectReplay(QPoint(10, 10), QSize(220, 24));
        this->buttonReplay.setToolTip(this->buttonReplay.text());
        this->buttonReplay.setGeometry(rectReplay);
        this->buttonReplay.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->buttonReplay)   , SIGNAL (pressed()),  this, SLOT (replayPressed()));
        this->buttonReplay.setFocusPolicy(Qt::StrongFocus);
        this->buttonReplay.setAutoDefault(true);
    }

private Q_SLOTS:
    void replayPressed() {
        QString filePath("");
        filePath = QFileDialog::getOpenFileName(this, tr("Open a Movie"),
                                                this->controllers->client->REPLAY_DIR.c_str(),
                                                tr("Movie Files(*.mwrm)"));
        std::string str_movie_path(filePath.toStdString());

        auto const last_delimiter_it = std::find(str_movie_path.rbegin(), str_movie_path.rend(), '/');
        int pos = str_movie_path.size() - (last_delimiter_it - str_movie_path.rbegin());

        std::string const movie_name = (last_delimiter_it == str_movie_path.rend())
        ? str_movie_path
        : str_movie_path.substr(str_movie_path.size() - (last_delimiter_it - str_movie_path.rbegin()));

        std::string const movie_dir = str_movie_path.substr(0, pos);

        this->controllers->client->_movie_name = movie_name;
        this->controllers->client->_movie_dir = movie_dir;
        this->controllers->client->replay(movie_dir, movie_name);
    }

};


struct AccountData {
    std::string title;
    std::string IP;
    std::string name;
    std::string pwd;
    int port = 0;
    int options_profil = 0;
    int index = -1;
};



class FormTabAPI : public QWidget
{
public:
     int account_index_to_drop;

    FormTabAPI(QWidget * parent)
      : QWidget(parent)
      , account_index_to_drop(-1)
      {}

    virtual void targetPicked(int ) {}
    virtual void drop_account() {}
    virtual ~FormTabAPI() = default;
};



class ConnectionFormQt  : public QWidget
{

Q_OBJECT

public:
    uint8_t protocol_type;

    QFormLayout line_edit_layout;

    FormTabAPI * main_panel;

    QComboBox  _IPCombobox;

    QLineEdit _IPField;
    QLineEdit _userNameField;
    QLineEdit _PWDField;
    QLineEdit _portField;

    QLabel    _IPLabel;
    QLabel    _userNameLabel;
    QLabel    _PWDLabel;
    QLabel    _portLabel;


    ConnectionFormQt(FormTabAPI * main_panel, uint8_t protocol_type, QWidget * parent)
      : QWidget(parent)
      , protocol_type(protocol_type)
      , line_edit_layout(this)
      , main_panel(main_panel)
      , _IPCombobox(this)
      , _IPField("", this)
      , _userNameField("", this)
      , _PWDField("", this)
      , _portField("", this)
      , _IPLabel(      QString("IP server :"), this)
      , _userNameLabel(QString("User name : "), this)
      , _PWDLabel(     QString("Password :  "), this)
      , _portLabel(    QString("Port :      "), this)
    {
        this->setFixedSize(240, 160);
        this->_IPCombobox.setFixedWidth(140);
        this->_IPCombobox.setLineEdit(&(this->_IPField));
        this->QObject::connect(&(this->_IPCombobox), SIGNAL(currentIndexChanged(int)) , this, SLOT(targetPicked(int)));

        this->setLayout(&(this->line_edit_layout));

        this->_PWDField.setEchoMode(QLineEdit::Password);
        this->_PWDField.setInputMethodHints(Qt::ImhHiddenText | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase);
        this->_userNameField.setFixedWidth(140);
        this->_PWDField.setFixedWidth(140);
        this->_portField.setFixedWidth(140);

        this->line_edit_layout.addRow(&(this->_IPLabel)      , &(this->_IPCombobox));
        this->line_edit_layout.addRow(&(this->_userNameLabel), &(this->_userNameField));
        this->line_edit_layout.addRow(&(this->_PWDLabel)     , &(this->_PWDField));
        this->line_edit_layout.addRow(&(this->_portLabel)    , &(this->_portField));

        if (this->protocol_type == ClientRedemptionIOAPI::MOD_VNC) {
            this->_userNameField.hide();
            this->_userNameLabel.hide();
        }
    }

private Q_SLOTS:
    void targetPicked(int index) {
        if (this->main_panel) {
            this->main_panel->targetPicked(index);
        }
    }
};



class IconAccountQt :  public QWidget
{

Q_OBJECT

public:
    FormTabAPI * main_tab;
    const AccountData accountData;
    QPixmap pixmap;
    QRect drop_rect;



    IconAccountQt(FormTabAPI * main_tab, const AccountData & accountData, QWidget * parent)
      : QWidget(parent)
      , main_tab(main_tab)
      , accountData(accountData)
      , pixmap(137, 50)
      , drop_rect(24, 95, 240, 160)
    {
        this->setFixedSize(137, 50);
    }

    void draw_account() {
        QPen                 pen;
        QPainter             painter(&(this->pixmap));
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(Qt::black);
        painter.setPen(pen);
        painter.fillRect(0, 0, this->width(), this->height(), Qt::white);
        painter.fillRect(0, 0, this->width(), this->height(), Qt::transparent);
        painter.drawRoundedRect(2, 2, this->height()-5, this->height()-5, 4, 4);

        QFont font = painter.font();
        font.setPixelSize(10);
        painter.setFont(font);

        QString qip(this->accountData.IP.c_str());
        QString qname(this->accountData.name.c_str());
        painter.drawText(QPoint(this->height()+6, 20), qip);
        painter.drawText(QPoint(this->height()+6, 30), qname);

        pen.setBrush(QColor(0xFF, 0x8C, 0x00));
        painter.setPen(pen);
        painter.drawRoundedRect(0, 0, this->width()-1, this->height()-1, 4, 4);

        painter.end();

        this->repaint();
    }

    void enterEvent(QEvent *ev) override {
        Q_UNUSED(ev);
        QPen                 pen;
        QPainter             painter(&(this->pixmap));
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(QColor(0xFF, 0x8C, 0x00));
        painter.setPen(pen);
        painter.drawRoundedRect(0, 0, this->width()-1, this->height()-1, 4, 4);
        painter.end();
        this->repaint();
    }

    void leaveEvent(QEvent *ev) override {
        Q_UNUSED(ev);
        QPen                 pen;
        QPainter             painter(&(this->pixmap));
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(Qt::white);
        painter.setPen(pen);
        painter.drawRoundedRect(0, 0, this->width()-1, this->height()-1, 4, 4);
        painter.end();
        this->repaint();
    }

    void mousePressEvent(QMouseEvent *e) override {

        switch (e->button()) {
            case Qt::LeftButton:
                if (this->main_tab) {

                    this->main_tab->account_index_to_drop = this->accountData.index;
                    QImage image(this->pixmap.toImage().convertToFormat(QImage::Format_ARGB32));
                    QPixmap map = QPixmap::fromImage(image);

                    QCursor qcursor(map, 10, 10);

                    this->main_tab->setCursor(qcursor);
                }
                break;

            default:
                break;
        }
    }

    void mouseReleaseEvent(QMouseEvent *e) override {

        switch (e->button()) {
            case Qt::LeftButton:
                if (this->main_tab) {
                    if (this->drop_rect.contains(QPoint(e->globalX() - this->main_tab->nativeParentWidget()->x(), e->globalY() - this->main_tab->nativeParentWidget()->y()))) {
                        this->main_tab->drop_account();
                    }
                    this->main_tab->account_index_to_drop = -1;
                    this->main_tab->setCursor(Qt::ArrowCursor);
                }
                break;

            default:
                break;
        }
    }

    void paintEvent(QPaintEvent * event) override {
        Q_UNUSED(event);
        QPen                 pen;
        QPainter             painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(Qt::black);
        painter.setPen(pen);

        painter.drawPixmap(QPoint(0, 0), this->pixmap, QRect(0, 0, this->width(), this->height()));

        painter.end();
    }

};



class AccountPanelQt : public QWidget
{

Q_OBJECT

public:
    IconAccountQt * icons[15];
    QFormLayout lay;
    const AccountData * accountData;
    const int nb_account;


    AccountPanelQt(FormTabAPI * main_tab, const AccountData * accountData, int nb_account, QWidget * parent)
      : QWidget(parent)
      , lay(this)
      , accountData(accountData)
      , nb_account(nb_account < 15 ?  nb_account : 15)
    {
        this->setMinimumHeight(160);

        for (int i = 0; i < this->nb_account; i++) {
            this->icons[i] = new IconAccountQt(main_tab, this->accountData[i], this);
            this->icons[i]->draw_account();
            this->lay.addRow(this->icons[i]);
        }

        this->setLayout(&(this->lay));
    }

    void paintEvent(QPaintEvent * event) override {
        Q_UNUSED(event);
        QPen                 pen;
        QPainter             painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        pen.setWidth(1);
        pen.setBrush(Qt::black);
        painter.setPen(pen);
        painter.fillRect(0, 0, this->width(), this->height(), Qt::white);
        painter.end();
    }

};



class FormTabQt : public FormTabAPI
{

Q_OBJECT

public:
    enum : int {
        MAX_ACCOUNT_DATA = 15
    };

    uint8_t protocol_type;
    ClientRedemptionIOAPI       * _front;
    ClientInputMouseKeyboardAPI * controllers;
    const int            _width;
    const int            _height;

    QGridLayout         grid_layout;
    int                  _accountNB;

    ConnectionFormQt     line_edit_panel;

    QLabel               _errorLabel;
    QCheckBox            _pwdCheckBox;

    AccountPanelQt  *   account_panel;

    QPushButton          _buttonConnexion;
    QPushButton          _buttonOptions;

    QtOptions options;

    QScrollArea scroller;

    AccountData          _accountData[MAX_ACCOUNT_DATA];
    bool                 _pwdCheckBoxChecked;
    int                  _lastTargetIndex;



    FormTabQt(ClientInputMouseKeyboardAPI * controllers, ClientRedemptionIOAPI  * front, uint8_t protocol_type, QWidget * parent)
        : FormTabAPI(parent)
        , protocol_type(protocol_type)
        , _front(front)
        , controllers(controllers)
        , _width(400)
        , _height(600)
        , grid_layout(this)
        , _accountNB(0)
        , line_edit_panel(this, protocol_type, this)
        , _errorLabel(   QString(""            ), this)
        , _pwdCheckBox(QString("Save password."), this)
        , _buttonConnexion("Connection", this)
        , _buttonOptions("Options", this)
        , options(front, protocol_type, this)
        , scroller(this)
        , _pwdCheckBoxChecked(false)
        , _lastTargetIndex(0)
    {
        this->setAccountData();

        this->grid_layout.addWidget(&(this->line_edit_panel), 0, 0, 1, 2);

        this->account_panel = new AccountPanelQt(this, this->_accountData, this->_accountNB, this);
        this->scroller.setFixedSize(170,  160);
        this->scroller.setStyleSheet("/*background-color: #FFFFFF;*/ border: 1px solid #FFFFFF;"
            "border-bottom-color: #FF8C00;");
        this->scroller.setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        this->scroller.setWidget(this->account_panel);
        this->grid_layout.addWidget(&(this->scroller), 0, 2, 1, 2);

        this->grid_layout.addWidget(&(this->_pwdCheckBox), 1, 0, 1, 2);
        this->_errorLabel.setFixedHeight(this->_errorLabel.height()+10);
        this->grid_layout.addWidget(&(this->_errorLabel), 2, 0, 1, 4);

        this->_buttonConnexion.setToolTip(this->_buttonConnexion.text());
        this->_buttonConnexion.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonConnexion)   , SIGNAL (released()), this, SLOT (connexionReleased()));
        this->_buttonConnexion.setFocusPolicy(Qt::StrongFocus);
        this->_buttonConnexion.setAutoDefault(true);
        this->grid_layout.addWidget(&(this->_buttonConnexion), 3, 2, 1, 2);

        this->_buttonOptions.setToolTip(this->_buttonOptions.text());
        this->_buttonOptions.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonOptions)     , SIGNAL (released()), this, SLOT (optionsReleased()));
        this->_buttonOptions.setFocusPolicy(Qt::StrongFocus);
        this->grid_layout.addWidget(&(this->_buttonOptions), 3, 0, 1, 1);

        this->grid_layout.addWidget(&(this->options), 4, 0, 1, 2);
        this->options.hide();

        this->setLayout(&(this->grid_layout));
    }


    void setAccountData() {
        if (this->controllers->client) {
            this->_accountNB = 0;
            std::ifstream ifichier(this->controllers->client->USER_CONF_LOG, std::ios::in);

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
                            this->_pwdCheckBox.setCheckState(Qt::Checked);
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
                    if (tag.compare(std::string("options_profil")) == 0) {
                        this->_accountData[accountNB].options_profil = std::stoi(info);
                        this->_accountData[accountNB].index = accountNB+1;
                        accountNB++;
                        if (accountNB == MAX_ACCOUNT_DATA) {
                            this->_accountNB = MAX_ACCOUNT_DATA;
                            accountNB = 0;
                        }
                    } else
                    if (tag.compare(std::string("port")) == 0) {
                        this->_accountData[accountNB].port = std::stoi(info);
                    }
                }

                if (this->_accountNB < MAX_ACCOUNT_DATA) {
                    this->_accountNB = accountNB;
                }

                this->line_edit_panel._IPCombobox.clear();
                this->line_edit_panel._IPCombobox.addItem(QString(""), 0);

                QStringList stringList;

                for (int i = 0; i < this->_accountNB; i++) {
                    std::string title(this->_accountData[i].IP + std::string(" - ")+ this->_accountData[i].name);
                    this->line_edit_panel._IPCombobox.addItem(QString(title.c_str()), i+1);
                    stringList << title.c_str();
                }
                //this->_completer = new QCompleter(stringList, this);
            }else {
                LOG(LOG_INFO, "can't open login config file");
            }
        }
    }

    void set_ErrorMsg(std::string str) {
        this->_errorLabel.clear();
        this->_errorLabel.setText(QString(str.c_str()));
    }

    void set_IPField(std::string str) {
        this->line_edit_panel._IPField.clear();
        this->line_edit_panel._IPField.insert(QString(str.c_str()));
    }

    void set_userNameField(std::string str) {
        this->line_edit_panel._userNameField.clear();
        this->line_edit_panel._userNameField.insert(QString(str.c_str()));
    }

    void set_PWDField(std::string str) {
        this->line_edit_panel._PWDField.clear();
        this->line_edit_panel._PWDField.insert(QString(str.c_str()));
    }

    void set_portField(int str) {
        this->line_edit_panel._portField.clear();
        if (str == 0) {
            this->line_edit_panel._portField.insert(QString(""));
        } else {
            this->line_edit_panel._portField.insert(QString(std::to_string(str).c_str()));
        }
    }

    std::string get_IPField() {
        std::string delimiter(" - ");
        std::string ip_field_content = this->line_edit_panel._IPField.text().toStdString();
        auto pos(ip_field_content.find(delimiter));
        std::string IP  = ip_field_content.substr(0, pos);
        return IP;
    }

    std::string get_userNameField() {
        if (this->line_edit_panel._userNameField.text().toStdString() !=  std::string(""))
            return this->line_edit_panel._userNameField.text().toStdString();

        return std::string(" ");
    }

    std::string get_PWDField() {
        if (this->line_edit_panel._PWDField.text().toStdString() !=  std::string(""))
            return this->line_edit_panel._PWDField.text().toStdString();

        return std::string(" ");
    }

    int get_portField() {
        return this->line_edit_panel._portField.text().toInt();
    }

    void keyPressEvent(QKeyEvent *e) override {
        if (e->key() == Qt::Key_Enter) {
            this->connexionReleased();
        }
    }

    void drop_account() override {
        if (this->account_index_to_drop !=  -1) {
            this->targetPicked(this->account_index_to_drop);
            this->account_index_to_drop = -1;
        }
    }

    void targetPicked(int index) override {
        if (index < 0) {
            return;
        }
        if (index >=  16 ) {
             this->connexionReleased();
             return;
        }
        if (index == 0) {
            this->line_edit_panel._IPField.clear();
            this->line_edit_panel._userNameField.clear();
            this->line_edit_panel._PWDField.clear();
            this->line_edit_panel._portField.clear();

        } else {
            index--;
            this->set_IPField(this->_accountData[index].IP);
            this->set_userNameField(this->_accountData[index].name);
            this->set_PWDField(this->_accountData[index].pwd);
            this->set_portField(this->_accountData[index].port);

            this->controllers->client->current_user_profil = this->_accountData[index].options_profil;
        }

        this->_buttonConnexion.setFocus();
    }

private Q_SLOTS:
    void connexionReleased() {

        this->_front->mod_state = this->protocol_type;

        QPoint points = this->mapToGlobal({0, 0});
        this->controllers->client->windowsData.form_x = points.x()-14;
        this->controllers->client->windowsData.form_y = points.y()-85;
        this->controllers->client->writeWindowsConf();

        this->controllers->connexionReleased();

        if (this->controllers->client->connected && this->controllers->client->mod !=  nullptr) {
            bool alreadySet = false;
            this->_pwdCheckBoxChecked = this->_pwdCheckBox.isChecked();

            std::string title(this->get_IPField() + std::string(" - ")+ this->get_userNameField());

            for (int i = 0; i < this->_accountNB; i++) {
                if (this->_accountData[i].title.compare(title) == 0) {
                    alreadySet = true;
                    this->_lastTargetIndex = i;
                    this->_accountData[i].pwd  = this->get_PWDField();
                    this->_accountData[i].port = this->get_portField();
                    this->_accountData[i].options_profil  = this->controllers->client->current_user_profil;
                }
            }

            if (!alreadySet && (this->_accountNB < MAX_ACCOUNT_DATA)) {
                this->_accountData[this->_accountNB].title = title;
                this->_accountData[this->_accountNB].IP    = this->get_IPField();
                this->_accountData[this->_accountNB].name  = this->get_userNameField();
                this->_accountData[this->_accountNB].pwd   = this->get_PWDField();
                this->_accountData[this->_accountNB].port  = this->get_portField();
                this->_accountData[this->_accountNB].options_profil  = this->controllers->client->current_user_profil;
                this->_accountNB++;

                if (this->_accountNB > MAX_ACCOUNT_DATA) {
                    this->_accountNB = MAX_ACCOUNT_DATA;
                }
                this->_lastTargetIndex = this->_accountNB;
            }

            std::ofstream ofichier(this->controllers->client->USER_CONF_LOG, std::ios::out | std::ios::trunc);
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
                    ofichier << "options_profil " << this->_accountData[i].options_profil << "\n";
                    ofichier << "\n";
                }
                ofichier.close();
            }
        }
    }

    void optionsReleased() {
        this->controllers->open_options();
    }
};



class Form_Qt : public QWidget
{

Q_OBJECT

public:
    ClientInputMouseKeyboardAPI * controllers;
    const int _width;
    const int _height;

    const int _long_height;

    QFormLayout main_layout;
    QTabWidget  tabs;

    FormTabQt  RDP_tab;
    FormTabQt  VNC_tab;
    FormReplay       replay_tab;

    bool is_option_open;
    bool is_closing;



    Form_Qt(ClientInputMouseKeyboardAPI * controllers, ClientRedemptionIOAPI  * front)
        : QWidget()
        , controllers(controllers)
        , _width(460)
        , _height(360)
        , _long_height(700)
        , main_layout(this)
        , tabs(this)
        , RDP_tab(controllers, front, ClientRedemptionIOAPI::MOD_RDP, this)
        , VNC_tab(controllers, front, ClientRedemptionIOAPI::MOD_VNC, this)
        , replay_tab(controllers, this)
        , is_option_open(false)
        , is_closing(false)
    {
        this->setWindowTitle("ReDemPtion Client");
        this->setAttribute(Qt::WA_DeleteOnClose);
        this->setFixedSize(this->_width, this->_height);

        QString qss =
            "QTabBar::tab:selected {"
            "    background: #FFFFFF; "
            "    border: 1px solid #C4C4C3;"
            "    border-top: 3px solid #FF8C00; "
            "    border-bottom-color: #FFFFFF; "
            "    border-top-left-radius: 4px;"
            "    border-top-right-radius: 4px;"
//             "    min-width: 8ex;"
            "    padding: 2px;"
            "}"
//             "QTabWidget::pane { /* The tab widget frame */"
//             "    border: 1px solid #C4C4C3;"
//             "    border-top-color: #FFFFFF; "
//             "    background: #C4F4C3; "
//             "}"
        ;

        this->tabs.setStyleSheet( this->tabs.styleSheet().append(qss));
        this->QObject::connect(&(this->tabs)   , SIGNAL(currentChanged(int)), this, SLOT (tab_changed(int)));

        const QString RDP_title("  RDP  ");
        this->tabs.addTab(&(this->RDP_tab), RDP_title);

        const QString VNC_title("  VNC  ");
        this->tabs.addTab(&(this->VNC_tab), VNC_title);

        const QString replay_title(" Replay ");
        this->tabs.addTab(&(this->replay_tab), replay_title);

        this->main_layout.addRow(&(this->tabs));
        this->setLayout(&(this->main_layout));
    }

    ~Form_Qt() {
        QPoint points = this->mapToGlobal({0, 0});
        this->controllers->client->windowsData.form_x = points.x()-1;
        this->controllers->client->windowsData.form_y = points.y()-39;
        this->controllers->client->writeWindowsConf();
        this->is_closing = true;
    }


    FormTabQt * get_current_tab() {
        switch (this->tabs.currentIndex()) {
            case 0: return &(this->RDP_tab);
            case 1: return &(this->VNC_tab);
            default: return nullptr;
        }
    }


    void set_ErrorMsg(std::string str) {
        this->get_current_tab()->set_ErrorMsg(str);
    }

    void set_IPField(std::string str) {
         this->get_current_tab()->set_IPField(str);
    }

    void set_userNameField(std::string str) {
        this->get_current_tab()->set_userNameField(str);
    }

    void set_PWDField(std::string str) {
        this->get_current_tab()->set_PWDField(str);
    }

    void set_portField(int str) {
        this->get_current_tab()->set_portField(str);
    }

    std::string get_IPField() {
        return this->get_current_tab()->get_IPField();
    }

    std::string get_userNameField() {
        return this->get_current_tab()->get_userNameField();
    }

    std::string get_PWDField() {
        return this->get_current_tab()->get_PWDField();
    }

    int get_portField() {
        return this->get_current_tab()->get_portField();
    }

    void init_form() {
        if (this->controllers->client->is_no_win_data()) {
            QDesktopWidget* desktop = QApplication::desktop();
            this->controllers->client->windowsData.form_x = (desktop->width()/2)  - (this->_width/2);
            this->controllers->client->windowsData.form_y = (desktop->height()/2) - (this->_height/2);
        }
        this->move(this->controllers->client->windowsData.form_x, this->controllers->client->windowsData.form_y);
        this->get_current_tab()->setAccountData();
    }

    void options() {
        if (this->is_option_open) {
            this->get_current_tab()->options.hide();
            this->get_current_tab()->_buttonOptions.setText("Options v");
            this->VNC_tab.options.hide();
            this->VNC_tab._buttonOptions.setText("Options v");
            this->setFixedHeight(this->_height);
            this->is_option_open = false;
        } else {
            this->setFixedHeight(this->_long_height);
            this->get_current_tab()->options.show();
            this->get_current_tab()->_buttonOptions.setText("Options ^");
            this->VNC_tab.options.show();
            this->VNC_tab._buttonOptions.setText("Options ^");
            this->is_option_open = true;
        }
    }

private Q_SLOTS:
    void tab_changed(int) {
        this->setFixedHeight(this->_height);
        if (!this->is_closing) {
            this->get_current_tab()->options.hide();
        }
    }

};