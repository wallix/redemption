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

#undef REDEMPTION_QT_INCLUDE_WIDGET


class Form_Qt : public QWidget
{

Q_OBJECT

public:
    enum : int {
        MAX_ACCOUNT_DATA = 15
    };

    ClientInputMouseKeyboardAPI       * controllers;
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
        int options_profil = 0;
    }                    _accountData[MAX_ACCOUNT_DATA];
    bool                 _pwdCheckBoxChecked;
    int                  _lastTargetIndex;


    Form_Qt(ClientInputMouseKeyboardAPI * controllers)
        : QWidget()
        , controllers(controllers)
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
        this->setWindowTitle("ReDemPtion Client");
        this->setAttribute(Qt::WA_DeleteOnClose);
        this->setFixedSize(this->_width, this->_height);

//         this->setAccountData();

//         if (this->_pwdCheckBoxChecked) {
//             this->_pwdCheckBox.setCheckState(Qt::Checked);
//         }
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
        this->QObject::connect(&(this->_buttonConnexion)   , SIGNAL (released()), this, SLOT (connexionReleased()));
        this->_buttonConnexion.setFocusPolicy(Qt::StrongFocus);
        this->_buttonConnexion.setAutoDefault(true);

        QRect rectOptions(QPoint(10, 256), QSize(110, 24));
        this->_buttonOptions.setToolTip(this->_buttonOptions.text());
        this->_buttonOptions.setGeometry(rectOptions);
        this->_buttonOptions.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonOptions)     , SIGNAL (released()), this, SLOT (optionsReleased()));
        this->_buttonOptions.setFocusPolicy(Qt::StrongFocus);
    }

    ~Form_Qt() {
        QPoint points = this->mapToGlobal({0, 0});
        this->controllers->client->windowsData.form_x = points.x()-1;
        this->controllers->client->windowsData.form_y = points.y()-39;
        this->controllers->client->writeWindowsConf();
    }

    void init_form() {
        if (this->controllers->client->is_no_win_data()) {
            QDesktopWidget* desktop = QApplication::desktop();
            this->controllers->client->windowsData.form_x = (desktop->width()/2)  - (this->_width/2);
            this->controllers->client->windowsData.form_y = (desktop->height()/2) - (this->_height/2);
        }
        this->move(this->controllers->client->windowsData.form_x, this->controllers->client->windowsData.form_y);
        this->setAccountData();

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

                this->_IPCombobox.clear();
                this->_IPCombobox.addItem(QString(""), 0);

                QStringList stringList;

                for (int i = 0; i < this->_accountNB; i++) {
                    std::string title(this->_accountData[i].IP + std::string(" - ")+ this->_accountData[i].name);
                    this->_IPCombobox.addItem(QString(title.c_str()), i+1);
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

    void keyPressEvent(QKeyEvent *e) override {
        if (e->key() == Qt::Key_Enter) {
            this->connexionReleased();
        }
    }


private Q_SLOTS:
    void targetPicked(int index) {
        if (index < 0) {
            return;
        }
        if (index >=  16 ) {
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

            this->controllers->client->current_user_profil = this->_accountData[index].options_profil;
        }

        this->_buttonConnexion.setFocus();
    }

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

    void connexionReleased() {

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