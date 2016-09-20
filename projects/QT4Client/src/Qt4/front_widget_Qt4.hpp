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

//#define LOGPRINT

#include "transport/socket_transport.hpp"
#include "mod/rdp/rdp.hpp"

#include "front_Qt4.hpp"

#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QDesktopWidget>
#include <QtGui/QApplication>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtCore/QSocketNotifier>
#include <QtGui/QLineEdit>
#include <QtGui/QFormLayout>
#include <QtGui/QDialog>
#include <QtGui/QPushButton>
#include <QtGui/QClipboard>
#include <QtGui/QTabWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QScrollArea>
#include <QtGui/QTableWidget>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtCore/QMimeData>
#include <QtCore/QUrl>
#include <QtGui/QCompleter>


#define KEY_SETTING_PATH "keySetting.config"
#define LOGINS_PATH "logins.config"



class DialogOptions_Qt : public QDialog
{

Q_OBJECT

public:
    Front_Qt_API       * _front;
    const int            _width;
    const int            _height;
    QWidget              _emptyPanel;
    QWidget            * _viewTab;
    QWidget            * _keyboardTab;
    QGridLayout        * _layout;
    QPushButton          _buttonSave;
    QPushButton          _buttonCancel;
    QPushButton        * _buttonDeleteKey;
    QPushButton        * _buttonAddKey;
    QTabWidget         * _tabs;
    QComboBox            _bppComboBox;
    QComboBox            _resolutionComboBox;
    QCheckBox            _perfCheckBox;
    QComboBox            _languageComboBox;
    QComboBox            _fpsComboBox;
    QComboBox            _monitorCountComboBox;
    QFormLayout        * _layoutView;
    QFormLayout        * _layoutKeyboard;
    QLabel               _labelBpp;
    QLabel               _labelResolution;
    QLabel               _labelPerf;
    QLabel               _labelLanguage;
    QLabel               _labelFps;
    QLabel               _labelScreen;
    QTableWidget       * _tableKeySetting;
    const int            _columnNumber;
    const int            _tableKeySettingMaxHeight;


    DialogOptions_Qt(Front_Qt_API * front, QWidget * parent)
        : QDialog(parent)
        , _front(front)
        , _width(400)
        , _height(350)
        , _emptyPanel(this)
        , _viewTab(nullptr)
        , _keyboardTab(nullptr)
        , _layout(nullptr)
        , _buttonSave("Save", this)
        , _buttonCancel("Cancel", this)
        , _buttonDeleteKey(nullptr)
        , _buttonAddKey(nullptr)
        , _tabs(nullptr)
        , _bppComboBox(this)
        , _resolutionComboBox(this)
        , _perfCheckBox(this)
        , _languageComboBox(this)
        , _fpsComboBox(this)
        , _layoutView(nullptr)
        , _layoutKeyboard(nullptr)
        , _labelBpp("Color depth :", this)
        , _labelResolution("Resolution :", this)
        , _labelPerf("Disable wallaper :", this)
        , _labelLanguage("Keyboard Language :", this)
        , _labelFps("Refresh per second :", this)
        , _labelScreen("Screen :", this)
        , _tableKeySetting(nullptr)
        , _columnNumber(4)
        , _tableKeySettingMaxHeight((20*6)+11)
    {
        this->setWindowTitle("Options");
        this->setAttribute(Qt::WA_DeleteOnClose);
        this->setFixedSize(this->_width, this->_height);
        this->setModal(true);

        this->_layout = new QGridLayout(this);


        // Tab options
        this->_viewTab = new QWidget(this);
        this->_keyboardTab = new QWidget(this);
        this->_tabs = new QTabWidget(this);


        // View tab
        const QString strView("View");
        this->_layoutView = new QFormLayout(this->_viewTab);


        //this->_bppComboBox.addItem("32", 32);
        this->_bppComboBox.addItem("24", 24);
        this->_bppComboBox.addItem("16", 16);
        this->_bppComboBox.addItem("15", 15);
        int indexBpp = this->_bppComboBox.findData(this->_front->_info.bpp);
        if ( indexBpp != -1 ) {
            this->_bppComboBox.setCurrentIndex(indexBpp);
        }
        this->_bppComboBox.setStyleSheet("combobox-popup: 0;");
        this->_layoutView->addRow(&(this->_labelBpp), &(this->_bppComboBox));


        this->_resolutionComboBox.addItem( "640 * 480", 640);
        this->_resolutionComboBox.addItem( "800 * 600", 800);
        this->_resolutionComboBox.addItem("1024 * 768", 1024);
        this->_resolutionComboBox.addItem("1600 * 900", 1600);
        int indexResolution = this->_resolutionComboBox.findData(this->_front->_width);
        if ( indexResolution != -1 ) {
            this->_resolutionComboBox.setCurrentIndex(indexResolution);
        }
        this->_resolutionComboBox.setStyleSheet("combobox-popup: 0;");
        this->_layoutView->addRow(&(this->_labelResolution), &(this->_resolutionComboBox));


        this->_fpsComboBox.addItem("20", 20);
        this->_fpsComboBox.addItem("30", 30);
        this->_fpsComboBox.addItem("40", 40);
        this->_fpsComboBox.addItem("50", 50);
        this->_fpsComboBox.addItem("60", 60);
        int indexFps = this->_fpsComboBox.findData(this->_front->_fps);
        if ( indexFps != -1 ) {
            this->_fpsComboBox.setCurrentIndex(indexFps);
        }
        this->_fpsComboBox.setStyleSheet("combobox-popup: 0;");
        this->_layoutView->addRow(&(this->_labelFps), &(this->_fpsComboBox));


        for (int i = 1; i <= Front_Qt::MAX_MONITOR_COUNT; i++) {
            this->_monitorCountComboBox.addItem(std::to_string(i).c_str(), i);
        }
        int indexMonitorCount = this->_monitorCountComboBox.findData(this->_front->_info.cs_monitor.monitorCount);
        if ( indexFps != -1 ) {
            this->_monitorCountComboBox.setCurrentIndex(indexMonitorCount);
        }
        this->_monitorCountComboBox.setStyleSheet("combobox-popup: 0;");
        this->_layoutView->addRow(&(this->_labelScreen), &(this->_monitorCountComboBox));


        if (this->_front->_info.rdp5_performanceflags == PERF_DISABLE_WALLPAPER) {
            this->_perfCheckBox.setCheckState(Qt::Checked);
        }
        this->_layoutView->addRow(&(this->_labelPerf), &(this->_perfCheckBox));


        this->_viewTab->setLayout(this->_layoutView);

        this->_tabs->addTab(this->_viewTab, strView);



        // Keyboard tab
        const QString strKeyboard("Keyboard");
        this->_layoutKeyboard = new QFormLayout(this->_keyboardTab);

        for (int i = 0; i < KEYLAYOUTS_LIST_SIZE; i++) {
            this->_languageComboBox.addItem(keylayoutsList[i]->locale_name, keylayoutsList[i]->LCID);
        }
        int indexLanguage = this->_languageComboBox.findData(this->_front->_info.keylayout);
        if ( indexLanguage != -1 ) {
            this->_languageComboBox.setCurrentIndex(indexLanguage);
        }
        this->_languageComboBox.setStyleSheet("combobox-popup: 0;");
        this->_layoutKeyboard->addRow(new QLabel("", this));
        this->_layoutKeyboard->addRow(&(this->_labelLanguage), &(this->_languageComboBox));

        this->_tableKeySetting = new QTableWidget(0, this->_columnNumber, this);
        QList<QString> columnTitles;
        columnTitles << "Qt key ID" << "Scan Code" << "ASCII8" << "Extended";
        this->_tableKeySetting->setHorizontalHeaderLabels({columnTitles});
        this->_tableKeySetting->setColumnWidth(0 ,85);
        this->_tableKeySetting->setColumnWidth(1 ,84);
        this->_tableKeySetting->setColumnWidth(2 ,84);
        this->_tableKeySetting->setColumnWidth(3 ,74);

        std::ifstream ifichier(KEY_SETTING_PATH, std::ios::in);
        if(ifichier) {

            std::string ligne;
            std::string delimiter = " ";

            while(getline(ifichier, ligne)) {

                int pos(ligne.find(delimiter));

                if (strcmp(ligne.substr(0, pos).c_str(), "-") == 0) {

                    ligne = ligne.substr(pos + delimiter.length(), ligne.length());
                    pos = ligne.find(delimiter);

                    int qtKeyID  = std::stoi(ligne.substr(0, pos));
                    ligne = ligne.substr(pos + delimiter.length(), ligne.length());
                    pos = ligne.find(delimiter);

                    int scanCode = std::stoi(ligne.substr(0, pos));
                    ligne = ligne.substr(pos + delimiter.length(), ligne.length());
                    pos = ligne.find(delimiter);

                    int ASCII8   = std::stoi(ligne.substr(0, pos));
                    ligne = ligne.substr(pos + delimiter.length(), ligne.length());
                    pos = ligne.find(delimiter);

                    int extended = std::stoi(ligne.substr(0, pos));

                    this->_front->_qtRDPKeymap.setCustomKeyCode(qtKeyID, scanCode, ASCII8, extended);

                    this->addRow();
                    this->setRowValues(qtKeyID, scanCode, ASCII8, extended);
                }
            }
        }
        this->addRow();


        this->_layoutKeyboard->addRow(this->_tableKeySetting);

        this->_keyboardTab->setLayout(this->_layoutKeyboard);


        this->_tabs->addTab(this->_keyboardTab, strKeyboard);

        this->_buttonAddKey = new QPushButton("Add row", this->_keyboardTab);
        QRect rectAddKey(QPoint(110, 226),QSize(70, 24));
        this->_buttonAddKey->setToolTip(this->_buttonAddKey->text());
        this->_buttonAddKey->setGeometry(rectAddKey);
        this->_buttonAddKey->setCursor(Qt::PointingHandCursor);
        this->QObject::connect(this->_buttonAddKey    , SIGNAL (pressed()) , this, SLOT (addRow()));

        this->_buttonDeleteKey = new QPushButton("Delete selected row", this->_keyboardTab);
        QRect rectDeleteKey(QPoint(190, 226),QSize(180, 24));
        this->_buttonDeleteKey->setToolTip(this->_buttonDeleteKey->text());
        this->_buttonDeleteKey->setGeometry(rectDeleteKey);
        this->_buttonDeleteKey->setCursor(Qt::PointingHandCursor);
        this->QObject::connect(this->_buttonDeleteKey , SIGNAL (pressed()) , this, SLOT (deletePressed()));
        this->QObject::connect(this->_buttonDeleteKey , SIGNAL (released()), this, SLOT (deleteReleased()));

        this->_layout->addWidget(this->_tabs, 0, 0, 9, 4);


        // Buttons
        this->_layout->addWidget(&(this->_emptyPanel), 11, 0, 1, 2);

        this->_buttonSave.setToolTip(this->_buttonSave.text());
        this->_buttonSave.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonSave)   , SIGNAL (pressed()),  this, SLOT (savePressed()));
        this->QObject::connect(&(this->_buttonSave)   , SIGNAL (released()), this, SLOT (saveReleased()));
        this->_buttonSave.setFocusPolicy(Qt::StrongFocus);
        this->_layout->addWidget(&(this->_buttonSave), 11, 2);

        this->_buttonCancel.setToolTip(this->_buttonCancel.text());
        this->_buttonCancel.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonCancel) , SIGNAL (pressed()),  this, SLOT (cancelPressed()));
        this->QObject::connect(&(this->_buttonCancel) , SIGNAL (released()), this, SLOT (cancelReleased()));
        this->_buttonCancel.setFocusPolicy(Qt::StrongFocus);
        this->_layout->addWidget(&(this->_buttonCancel), 11, 3);


        this->setLayout(this->_layout);

        QDesktopWidget* desktop = QApplication::desktop();
        int centerW = (desktop->width()/2)  - (this->_width/2);
        int centerH = (desktop->height()/2) - (this->_height/2);
        this->move(centerW, centerH);

        this->show();
    }

    ~DialogOptions_Qt() {}


private:
    void setRowValues(int qtKeyID, int scanCode, int ASCII8, int extended) {
        int row(this->_tableKeySetting->rowCount() - 1);

        QTableWidgetItem * item1 = new QTableWidgetItem;
        item1->setText(std::to_string(qtKeyID).c_str());
        this->_tableKeySetting->setItem(row, 0, item1);

        QTableWidgetItem * item2 = new QTableWidgetItem;
        item2->setText(std::to_string(scanCode).c_str());
        this->_tableKeySetting->setItem(row, 1, item2);

        QTableWidgetItem * item3 = new QTableWidgetItem;
        item3->setText(std::to_string(ASCII8).c_str());
        this->_tableKeySetting->setItem(row, 2, item3);

        static_cast<QComboBox*>(this->_tableKeySetting->cellWidget(row, 3))->setCurrentIndex(extended);
    }

    void updateKeySetting() {
        int tableKeySettingHeight((20*(this->_tableKeySetting->rowCount()+1))+11);
        if (tableKeySettingHeight > this->_tableKeySettingMaxHeight) {
            tableKeySettingHeight = this->_tableKeySettingMaxHeight;
        }
        this->_tableKeySetting->setFixedSize((80*this->_columnNumber)+40, tableKeySettingHeight);
        if (this->_tableKeySetting->rowCount() > 5) {
            this->_tableKeySetting->setColumnWidth(3 ,74);
        } else {
            this->_tableKeySetting->setColumnWidth(3 ,87);
        }

        this->update();
    }


public Q_SLOTS:
    void savePressed() {}

    void saveReleased() {;
        this->_front->_info.bpp = this->_bppComboBox.currentText().toInt();
        this->_front->_imageFormatRGB  = this->_front->bpp_to_QFormat(this->_front->_info.bpp, false);
        this->_front->_imageFormatARGB = this->_front->bpp_to_QFormat(this->_front->_info.bpp, true);
        std::string delimiter = " * ";
        std::string resolution( this->_resolutionComboBox.currentText().toStdString());
        int pos(resolution.find(delimiter));
        this->_front->_width  = std::stoi(resolution.substr(0, pos));
        this->_front->_height = std::stoi(resolution.substr(pos + delimiter.length(), resolution.length()));
        this->_front->_fps = this->_fpsComboBox.currentText().toInt();
        if (this->_perfCheckBox.isChecked()) {
            this->_front->_info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
        } else {
            this->_front->_info.rdp5_performanceflags = 0;
        }
        this->_front->_info.keylayout = this->_languageComboBox.itemData(this->_languageComboBox.currentIndex()).toInt();
        this->_front->_info.cs_monitor.monitorCount = this->_monitorCountComboBox.itemData(this->_monitorCountComboBox.currentIndex()).toInt();
        this->_front->_monitorCount = this->_front->_info.cs_monitor.monitorCount;
        this->_front->_info.width   = this->_front->_width * this->_front->_monitorCount;
        this->_front->_info.height  = this->_front->_height;

        this->_front->writeClientInfo();


        remove(KEY_SETTING_PATH);
        this->_front->_qtRDPKeymap.clearCustomKeyCod();

        std::ofstream ofichier(KEY_SETTING_PATH, std::ios::out | std::ios::trunc);
        if(ofichier) {

            ofichier << "Key Setting" << std::endl << std::endl;

            for (int i = 0; i < this->_tableKeySetting->rowCount(); i++) {

                int qtKeyID(0);
                if (this->_tableKeySetting->item(i, 0)) {
                    qtKeyID = this->_tableKeySetting->item(i, 0)->text().toInt();
                }

                if (qtKeyID != 0) {
                    int scanCode(0);
                    if (this->_tableKeySetting->item(i, 0)) {
                        scanCode = this->_tableKeySetting->item(i, 1)->text().toInt();
                    }
                    int ASCII8(0);
                    if (this->_tableKeySetting->item(i, 0)) {
                        ASCII8 = this->_tableKeySetting->item(i, 2)->text().toInt();
                    }
                    int extended(static_cast<QComboBox*>(this->_tableKeySetting->cellWidget(i, 3))->currentIndex());

                    this->_front->_qtRDPKeymap.setCustomKeyCode(qtKeyID, scanCode, ASCII8, extended);

                    ofichier << "- ";
                    ofichier << qtKeyID  << " ";
                    ofichier << scanCode << " ";
                    ofichier << ASCII8   << " ";
                    ofichier << extended << std::endl;
                }
            }
            ofichier.close();
        }

        this->close();
    }

    void cancelPressed() {}

    void cancelReleased() {
        this->close();
    }

    void addRow() {
        int rowNumber(this->_tableKeySetting->rowCount());
        this->_tableKeySetting->insertRow(rowNumber);
        this->_tableKeySetting->setRowHeight(rowNumber ,20);
        QComboBox * combo = new QComboBox(this->_tableKeySetting);
        combo->addItem("No" , 0);
        combo->addItem("Yes", 1);
        this->_tableKeySetting->setCellWidget(rowNumber, 3, combo);

        this->updateKeySetting();
    }

    void deletePressed() {
       QModelIndexList indexes = this->_tableKeySetting->selectionModel()->selection().indexes();
       for (int i = 0; i < indexes.count(); ++i) {
           QModelIndex index = indexes.at(i);
           this->_tableKeySetting->removeRow(index.row());
       }

       if (this->_tableKeySetting->rowCount() == 0) {
           this->addRow();
       }

       this->updateKeySetting();
    }

    void deleteReleased() {}

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
    QCompleter         * _completer;
    struct {
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

        QRect rectConnexion(QPoint(280, 256), QSize(110, 24));
        this->_buttonConnexion.setToolTip(this->_buttonConnexion.text());
        this->_buttonConnexion.setGeometry(rectConnexion);
        this->_buttonConnexion.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonConnexion)   , SIGNAL (pressed()),  this, SLOT (connexionPressed()));
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
        std::ifstream ifichier(LOGINS_PATH, std::ios::in);
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

    void connexionPressed() {
        this->_front->connexionPressed();
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

            std::ofstream ofichier(LOGINS_PATH, std::ios::out | std::ios::trunc);
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
        new DialogOptions_Qt(this->_front, this);
    }
};



class Screen_Qt : public QWidget
{

Q_OBJECT

public:
    Front_Qt_API       * _front;
    QPushButton          _buttonCtrlAltDel;
    QPushButton          _buttonRefresh;
    QPushButton          _buttonDisconnexion;
    QColor               _penColor;
    QPixmap            * _cache;
    //QGraphicsScene       _scene;
    //QGraphicsView        _view;
    QPainter             _cache_painter;
    const int            _width;
    const int            _height;
    bool                 _connexionLasted;
    const int            _buttonHeight;
    QTimer               _timer;
    uint8_t              _screen_index;


    Screen_Qt (Front_Qt_API * front, int screen_index)
        : QWidget()
        , _front(front)
        , _buttonCtrlAltDel("CTRL + ALT + DELETE", this)
        , _buttonRefresh("Refresh", this)
        , _buttonDisconnexion("Disconnection", this)
        , _penColor(Qt::black)
        , _cache(front->getMainScreen()->_cache)
        //, _cache_painter(&(this->_cache))
        , _width(this->_front->_width)
        , _height(this->_front->_height)
        , _connexionLasted(false)
        , _buttonHeight(20)
        , _timer(this)
        , _screen_index(screen_index)
    {
        this->setMouseTracking(true);
        this->installEventFilter(this);
        this->setAttribute(Qt::WA_DeleteOnClose);
        std::string screen_index_str = std::to_string(int(this->_screen_index));
        std::string title = "Remote Desktop Player connected to [" + this->_front->_targetIP +  "]. " + screen_index_str;
        this->setWindowTitle(QString(title.c_str()));

        this->setFixedSize(this->_width, this->_height + this->_buttonHeight);

        QRect rectCtrlAltDel(QPoint(0, this->_height+1),QSize(this->_width/3, this->_buttonHeight));
        this->_buttonCtrlAltDel.setToolTip(this->_buttonCtrlAltDel.text());
        this->_buttonCtrlAltDel.setGeometry(rectCtrlAltDel);
        this->_buttonCtrlAltDel.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonCtrlAltDel)  , SIGNAL (pressed()),  this, SLOT (CtrlAltDelPressed()));
        this->QObject::connect(&(this->_buttonCtrlAltDel)  , SIGNAL (released()), this, SLOT (CtrlAltDelReleased()));
        this->_buttonCtrlAltDel.setFocusPolicy(Qt::NoFocus);

        QRect rectRefresh(QPoint(this->_width/3, this->_height+1),QSize(this->_width/3, this->_buttonHeight));
        this->_buttonRefresh.setToolTip(this->_buttonRefresh.text());
        this->_buttonRefresh.setGeometry(rectRefresh);
        this->_buttonRefresh.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonRefresh)     , SIGNAL (pressed()),  this, SLOT (RefreshPressed()));
        this->QObject::connect(&(this->_buttonRefresh)     , SIGNAL (released()), this, SLOT (RefreshReleased()));
        this->_buttonRefresh.setFocusPolicy(Qt::NoFocus);

        QRect rectDisconnexion(QPoint(((this->_width/3)*2), this->_height+1),QSize(this->_width-((this->_width/3)*2), this->_buttonHeight));
        this->_buttonDisconnexion.setToolTip(this->_buttonDisconnexion.text());
        this->_buttonDisconnexion.setGeometry(rectDisconnexion);
        this->_buttonDisconnexion.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonDisconnexion), SIGNAL (pressed()),  this, SLOT (disconnexionPressed()));
        this->QObject::connect(&(this->_buttonDisconnexion), SIGNAL (released()), this, SLOT (disconnexionRelease()));
        this->_buttonDisconnexion.setFocusPolicy(Qt::NoFocus);

        QDesktopWidget* desktop = QApplication::desktop();
        int shift(10 * this->_screen_index);
        uint32_t centerW = (desktop->width()/2)  - (this->_width/2);
        uint32_t centerH = (desktop->height()/2) - ((this->_height+this->_buttonHeight)/2);
        this->move(centerW + shift, centerH + shift);

        this->QObject::connect(&(this->_timer), SIGNAL (timeout()),  this, SLOT (slotRepaint()));
        this->_timer.start(1000/this->_front->_fps);

        this->setFocusPolicy(Qt::StrongFocus);
    }


    Screen_Qt (Front_Qt_API * front)
        : QWidget()
        , _front(front)
        , _buttonCtrlAltDel("CTRL + ALT + DELETE", this)
        , _buttonRefresh("Refresh", this)
        , _buttonDisconnexion("Disconnection", this)
        , _penColor(Qt::black)
        , _cache(new QPixmap(this->_front->_info.width*front->_monitorCount, this->_front->_info.height))
        , _cache_painter(this->_cache)
        , _width(this->_front->_width)
        , _height(this->_front->_height)
        , _connexionLasted(false)
        , _buttonHeight(20)
        , _timer(this)
        , _screen_index(0)
    {
        this->setMouseTracking(true);
        this->installEventFilter(this);
        this->setAttribute(Qt::WA_DeleteOnClose);
        std::string title = "Remote Desktop Player connected to [" + this->_front->_targetIP +  "].";
        this->setWindowTitle(QString(title.c_str()));

        this->_cache_painter.fillRect(0, 0, this->_width * this->_front->_monitorCount, this->_height, QColor(127, 127, 127, 0));

        this->setFixedSize(this->_width, this->_height + this->_buttonHeight);

        QRect rectCtrlAltDel(QPoint(0, this->_height+1),QSize(this->_width/3, this->_buttonHeight));
        this->_buttonCtrlAltDel.setToolTip(this->_buttonCtrlAltDel.text());
        this->_buttonCtrlAltDel.setGeometry(rectCtrlAltDel);
        this->_buttonCtrlAltDel.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonCtrlAltDel)  , SIGNAL (pressed()),  this, SLOT (CtrlAltDelPressed()));
        this->QObject::connect(&(this->_buttonCtrlAltDel)  , SIGNAL (released()), this, SLOT (CtrlAltDelReleased()));
        this->_buttonCtrlAltDel.setFocusPolicy(Qt::NoFocus);

        QRect rectRefresh(QPoint(this->_width/3, this->_height+1),QSize(this->_width/3, this->_buttonHeight));
        this->_buttonRefresh.setToolTip(this->_buttonRefresh.text());
        this->_buttonRefresh.setGeometry(rectRefresh);
        this->_buttonRefresh.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonRefresh)     , SIGNAL (pressed()),  this, SLOT (RefreshPressed()));
        this->QObject::connect(&(this->_buttonRefresh)     , SIGNAL (released()), this, SLOT (RefreshReleased()));
        this->_buttonRefresh.setFocusPolicy(Qt::NoFocus);

        QRect rectDisconnexion(QPoint(((this->_width/3)*2), this->_height+1),QSize(this->_width-((this->_width/3)*2), this->_buttonHeight));
        this->_buttonDisconnexion.setToolTip(this->_buttonDisconnexion.text());
        this->_buttonDisconnexion.setGeometry(rectDisconnexion);
        this->_buttonDisconnexion.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonDisconnexion), SIGNAL (pressed()),  this, SLOT (disconnexionPressed()));
        this->QObject::connect(&(this->_buttonDisconnexion), SIGNAL (released()), this, SLOT (disconnexionRelease()));
        this->_buttonDisconnexion.setFocusPolicy(Qt::NoFocus);

        QDesktopWidget* desktop = QApplication::desktop();
        uint32_t centerW = (desktop->width()/2)  - (this->_width/2);
        uint32_t centerH = (desktop->height()/2) - ((this->_height+20)/2);
        this->move(centerW, centerH);

        this->QObject::connect(&(this->_timer), SIGNAL (timeout()),  this, SLOT (slotRepaint()));
        this->_timer.start(1000/this->_front->_fps);

        this->setFocusPolicy(Qt::StrongFocus);
    }

    ~Screen_Qt() {
        if (!this->_connexionLasted) {
            this->_front->closeFromScreen(this->_screen_index);
        }
    }

    void errorConnexion() {
        this->_connexionLasted = true;
    }

    QPainter & paintCache() {
        return this->_cache_painter;
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
        painter.drawPixmap(QPoint(0, 0), *(this->_cache), QRect(this->_width * this->_screen_index, 0, this->_width, this->_height));
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
        this->_front->mousePressEvent(e, this->_screen_index);
    }

    void mouseReleaseEvent(QMouseEvent *e) {
        this->_front->mouseReleaseEvent(e, this->_screen_index);
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

    bool eventFilter(QObject *obj, QEvent *e) {
        this->_front->eventFilter(obj, e, this->_screen_index);
        return false;
    }


private Q_SLOTS:
    void slotRepaint() {
        this->repaint();
    }

    void RefreshPressed() {
        this->_front->RefreshPressed();
    }

    void RefreshReleased() {
        this->_front->RefreshReleased();
    }

    void CtrlAltDelPressed() {
        this->_front->CtrlAltDelPressed();
    }

    void CtrlAltDelReleased() {
        this->_front->CtrlAltDelReleased();
    }

    void disconnexionPressed() {
        this->_front->disconnexionPressed();
    }

    void disconnexionRelease(){
        this->_front->disconnexionReleased();
    }

    void setMainScreenOnTopPressed() {

    }

    void setMainScreenOnTopRelease() {
        this->_front->setMainScreenOnTopRelease();
    }
};



class Connector_Qt : public QObject
{

Q_OBJECT

public:
    Front_Qt                  * _front;
    QSocketNotifier           * _sckRead;
    mod_rdp                   * _callback;
    SocketTransport           * _sck;
    int                         _client_sck;
    QClipboard                * _clipboard;
    bool                        _local_clipboard_stream;
    size_t                      _cliboard_data_length;
    std::unique_ptr<uint8_t[]>  _chunk;
    QImage                    * _bufferImage;
    uint16_t                    _bufferTypeID;
    std::string                 _bufferTypeLongName;
    int                         _cItems;
    TimeSystem                  _timeSystem;
    struct CB_out_File {
        uint64_t     size;
        std::string  name;
        std::string  nameUTF8;
        char *       chunk;

        CB_out_File()
          : size(0)
          , name("")
          , nameUTF8("")
          , chunk(nullptr)
        {}

        ~CB_out_File() {
            delete[] (chunk);
        }
    };
    std::vector<CB_out_File *>  _items_list;
    std::vector<std::string>    _temp_files_list;


    Connector_Qt(Front_Qt * front, QWidget * parent)
        : QObject(parent)
        , _front(front)
        , _sckRead(nullptr)
        , _callback(nullptr)
        , _sck(nullptr)
        , _client_sck(0)
        , _clipboard(nullptr)
        , _local_clipboard_stream(true)
        , _cliboard_data_length(0)
        , _bufferImage(nullptr)
        , _bufferTypeID(0)
        , _bufferTypeLongName("")
        , _cItems(0)
    {
        this->_clipboard = QApplication::clipboard();
        this->QObject::connect(this->_clipboard, SIGNAL(dataChanged()),  this, SLOT(mem_clipboard()));
    }

    ~Connector_Qt() {
        this->drop_connexion();
    }

    void drop_connexion() {
        this->emptyBuffer();

        if (this->_callback != nullptr) {
            static_cast<mod_api*>(this->_callback)->disconnect();
            delete (this->_callback);
            this->_callback = nullptr;
            this->_front->_callback = nullptr;
        }
        if (this->_sckRead != nullptr) {
            delete (this->_sckRead);
            this->_sckRead = nullptr;
        }
        if (this->_sck != nullptr) {
            delete (this->_sck);
            this->_sck = nullptr;
            std::cout << "Disconnected from [" << this->_front->_targetIP << "]." << std::endl;
        }
    }

    bool connect() {
        const char * name(this->_front->_userName.c_str());
        const char * targetIP(this->_front->_targetIP.c_str());
        const std::string errorMsg("Cannot connect to [" + this->_front->_targetIP +  "].");

        this->_client_sck = ip_connect(targetIP, this->_front->_port, this->_front->_nbTry, this->_front->_retryDelay);

        if (this->_client_sck > 0) {
            try {
                std::string error_message;
                this->_sck = new SocketTransport( name
                                                , this->_client_sck
                                                , targetIP
                                                , this->_front->_port
                                                , this->_front->verbose
                                                , &error_message
                                                );
                std::cout << "Connected to [" << targetIP <<  "]." << std::endl;
                return true;

            } catch (const std::exception & e) {
                std::string windowErrorMsg(errorMsg+" Socket error.");
                std::cout << windowErrorMsg << std::endl;
                this->_front->disconnect("<font color='Red'>"+windowErrorMsg+"</font>");
                return false;
            }
        } else {
            std::string windowErrorMsg(errorMsg+" ip_connect error.");
            std::cout << windowErrorMsg << std::endl;
            this->_front->disconnect("<font color='Red'>"+windowErrorMsg+"</font>");
            return false;
        }
    }

    void listen() {
        const char * name(this->_front->_userName.c_str());
        const char * pwd(this->_front->_pwd.c_str());
        const char * targetIP(this->_front->_targetIP.c_str());
        const char * localIP(this->_front->_localIP.c_str());

        Inifile ini;
        //ini.set<cfg::debug::rdp>(MODRDP_LOGLEVEL_CLIPRDR);

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
        mod_rdp_params.enable_fastpath                 = false;
        mod_rdp_params.enable_mem3blt                  = true;
        mod_rdp_params.enable_bitmap_update            = true;
        mod_rdp_params.enable_new_pointer              = true;
        mod_rdp_params.server_redirection_support      = true;
        std::string allow_channels = "*";
        mod_rdp_params.allow_channels                  = &allow_channels;
        //mod_rdp_params.allow_using_multiple_monitors   = true;
        //mod_rdp_params.bogus_refresh_rect              = true;
        mod_rdp_params.verbose = MODRDP_LOGLEVEL_CLIPRDR;

        LCGRandom gen(0); // To always get the same client random, in tests

        try {
            this->_callback = new mod_rdp(*(this->_sck), *(this->_front), this->_front->_info, ini.get_ref<cfg::mod_rdp::redir_info>(), gen, this->_timeSystem, mod_rdp_params);
            this->_front->_to_server_sender._callback = this->_callback;
            this->_front->_callback = this->_callback;
            this->_sckRead = new QSocketNotifier(this->_client_sck, QSocketNotifier::Read, this);
            this->QObject::connect(this->_sckRead,   SIGNAL(activated(int)), this,  SLOT(call_Draw()));

            while (!this->_callback->is_up_and_running()) {
                this->_callback->draw_event(time(nullptr), *(this->_front));
            }

        } catch (const Error & e) {
            const std::string errorMsg("Error: connexion to [" + this->_front->_targetIP +  "] is closed.");
            std::cout << errorMsg << std::endl;
            std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");
            this->_front->dropScreen();
            this->_front->disconnect(labelErrorMsg);
        }
    }

    void write_clipboard_temp_file(std::string fileName, uint8_t * data, size_t data_len) {
        std::string filePath(this->_front->CB_TEMP_DIR + fileName);
        std::string filePath_mem(filePath);
        this->_temp_files_list.push_back(filePath_mem);
        std::ofstream oFile(filePath, std::ios::out | std::ios::binary);
        if(oFile.is_open()) {
            oFile.write(reinterpret_cast<char *>(data), data_len);
            oFile.close();
        }
    }

    void setClipboard_files(std::vector<Front_Qt::CB_in_Files> items_list) {
        QClipboard *cb = QApplication::clipboard();
        QMimeData* newMimeData = new QMimeData();
        const QMimeData* oldMimeData = cb->mimeData();
        QStringList ll = oldMimeData->formats();
        for (int i = 0; i < ll.size(); i++) {
            newMimeData->setData(ll[i], oldMimeData->data(ll[i]));
        }
        QList<QUrl> list;
        for (size_t i = 0; i < items_list.size(); i++) {
            std::string path(this->_front->CB_TEMP_DIR + items_list[i].name);
            std::cout << "path on clipboard " << path <<  std::endl;
            QString qpath(path.c_str());
            list.append(QUrl::fromLocalFile(qpath));
            QByteArray gnomeFormat = QByteArray("copy\n").append(QUrl::fromLocalFile(qpath).toEncoded());
            newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);
        }

        newMimeData->setUrls(list);
        cb->setMimeData(newMimeData);
    }

    void setClipboard_text(std::string & str) {
        this->_clipboard->setText(QString::fromUtf8(str.c_str()), QClipboard::Clipboard);
    }

    void setClipboard_image(const QImage & image) {               // Paste image to client
        this->_clipboard->setImage(image, QClipboard::Clipboard);
    }

    void emptyBuffer() {
        this->_bufferTypeID = 0;
        this->_cliboard_data_length = 0;
        for (size_t i = 0; i < _temp_files_list.size(); i++) {
            if (remove(this->_temp_files_list[i].c_str()) != 0) {
                std::cout <<  "error " << _temp_files_list[i] <<  std::endl;
            }
        }
        this->_temp_files_list.clear();
        for (size_t i = 0; i < _items_list.size(); i++) {
            delete(this->_items_list[i]);
        }
        this->_items_list.clear();
    }

    void send_FormatListPDU(bool isLongFormat) {
        uint32_t formatIDs[]                  = { this->_bufferTypeID };
        std::string formatListDataShortName[] = { this->_bufferTypeLongName };
        this->_front->send_FormatListPDU(formatIDs, formatListDataShortName, 1, isLongFormat);
    }




public Q_SLOTS:
    void call_Draw() {
        if (this->_front->_callback)
        this->_front->call_Draw();
    }

    void mem_clipboard() {
        if (this->_callback != nullptr && this->_local_clipboard_stream) {
            const QMimeData * mimeData = this->_clipboard->mimeData();

            if (mimeData->hasImage()){
            //==================
            //    IMAGE COPY
            //==================
                this->emptyBuffer();

                this->_bufferTypeID = RDPECLIP::CF_METAFILEPICT;
                this->_bufferTypeLongName = "";
                QImage bufferImageTmp(this->_clipboard->image());
                bufferImageTmp = bufferImageTmp.convertToFormat(QImage::Format_RGB888);
                bufferImageTmp = bufferImageTmp.rgbSwapped();
                this->_bufferImage = new QImage(bufferImageTmp);

                this->_cliboard_data_length = this->_bufferImage->byteCount();

                this->send_FormatListPDU(false);


            } else if (mimeData->hasText()){                //  File or Text copy

                this->emptyBuffer();
                std::string str(this->_clipboard->text(QClipboard::Clipboard).toUtf8().constData());

                if (str.at(0) == '/') {
                //==================
                //    FILE COPY
                //==================
                    this->_bufferTypeID       = Front_Qt::Clipbrd_formats_list::CF_QT_CLIENT_FILEGROUPDESCRIPTORW;
                    this->_bufferTypeLongName = this->_front->_clipbrd_formats_list.FILEGROUPDESCRIPTORW;

                    // retrieve each path
                    const std::string delimiter = "\n";
                    this->_cItems = 0;
                    uint32_t pos = 0;
                    while (pos <= str.size()) {
                        pos = str.find(delimiter);
                        std::string path = str.substr(0, pos);
                        str = str.substr(pos+1, str.size());

                        // double slash
                        uint32_t posSlash(0);
                        std::string slash = "/";
                        bool stillSlash = true;
                        while (stillSlash) {
                            posSlash = path.find(slash, posSlash);
                            if (posSlash < path.size()) {
                                path = path.substr(0, posSlash) + "//" + path.substr(posSlash+1, path.size());
                                posSlash += 2;
                            } else {
                                path = path.substr(0, path.size());
                                stillSlash = false;
                            }
                        }

                        // get file data
                        uint64_t size(::filesize(path.c_str()));
                        std::ifstream iFile(path.c_str(), std::ios::in | std::ios::binary);
                        if (iFile.is_open()) {

                            CB_out_File * file = new CB_out_File();
                            file->size  = size;
                            file->chunk = new char[file->size];
                            iFile.read(file->chunk, file->size);
                            iFile.close();

                            int posName(path.size()-1);
                            bool lookForName = true;
                            while (posName >= 0 && lookForName) {
                                if (path.at(posName) == '/') {
                                    lookForName = false;
                                }
                                posName--;
                            }
                            file->nameUTF8 = path.substr(posName+2, path.size());
                            //std::string name = file->nameUTF8;
                            int UTF8nameSize(file->nameUTF8.size() *2);
                            if (UTF8nameSize > 520) {
                                UTF8nameSize = 520;
                            }
                            uint8_t UTF16nameData[520];
                            int UTF16nameSize = ::UTF8toUTF16_CrLf(reinterpret_cast<const uint8_t *>(file->nameUTF8.c_str()), UTF16nameData, UTF8nameSize);
                            file->name = std::string(reinterpret_cast<char *>(UTF16nameData), UTF16nameSize);
                            this->_cItems++;
                            this->_items_list.push_back(file);

                        } else {
                            std::cout << "path \"" << path << "\" not found." <<  std::endl;
                        }
                    }

                    this->send_FormatListPDU(true);


                } else {
                //==================
                //    TEXT COPY
                //==================
                    this->_bufferTypeID = RDPECLIP::CF_UNICODETEXT;
                    this->_bufferTypeLongName = "";

                    int cmptCR(0);
                    std::string tmp(str);
                    int pos(tmp.find("\n"));

                    while (pos != -1) {
                        cmptCR++;
                        tmp = tmp.substr(pos+2, tmp.length());
                        pos = tmp.find("\n"); // for linux install
                    }

                    size_t size( (str.length() + cmptCR*2) * 4 );

                    this->_chunk  = std::make_unique<uint8_t[]>(size);
                    // UTF8toUTF16_CrLf for linux install
                    this->_cliboard_data_length = ::UTF8toUTF16_CrLf(reinterpret_cast<const uint8_t *>(str.c_str()), this->_chunk.get(), size) + 2;

                    this->send_FormatListPDU(false);

                }
            }
        }
    }


};


