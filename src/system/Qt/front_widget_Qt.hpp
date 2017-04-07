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
#include "../src/system/Qt/front_Qt.hpp"
#ifdef QT5
#include </usr/include/x86_64-linux-gnu/qt5/QtWidgets/QWidget>
#include </usr/include/x86_64-linux-gnu/qt5/QtWidgets/QLabel>
#include </usr/include/x86_64-linux-gnu/qt5/QtGui/QPainter>
#include </usr/include/x86_64-linux-gnu/qt5/QtGui/QColor>
#include </usr/include/x86_64-linux-gnu/qt5/QtWidgets/QDesktopWidget>
#include </usr/include/x86_64-linux-gnu/qt5/QtWidgets/QApplication>
#include </usr/include/x86_64-linux-gnu/qt5/QtGui/QMouseEvent>
#include </usr/include/x86_64-linux-gnu/qt5/QtGui/QWheelEvent>
#include </usr/include/x86_64-linux-gnu/qt5/QtCore/QSocketNotifier>
#include </usr/include/x86_64-linux-gnu/qt5/QtWidgets/QLineEdit>
#include </usr/include/x86_64-linux-gnu/qt5/QtWidgets/QFormLayout>
#include </usr/include/x86_64-linux-gnu/qt5/QtWidgets/QDialog>
#include </usr/include/x86_64-linux-gnu/qt5/QtWidgets/QPushButton>
#include </usr/include/x86_64-linux-gnu/qt5/QtGui/QClipboard>
#include </usr/include/x86_64-linux-gnu/qt5/QtWidgets/QTabWidget>
#include </usr/include/x86_64-linux-gnu/qt5/QtWidgets/QGridLayout>
#include </usr/include/x86_64-linux-gnu/qt5/QtWidgets/QComboBox>
#include </usr/include/x86_64-linux-gnu/qt5/QtWidgets/QCheckBox>
#include </usr/include/x86_64-linux-gnu/qt5/QtWidgets/QScrollArea>
#include </usr/include/x86_64-linux-gnu/qt5/QtWidgets/QTableWidget>
#include </usr/include/x86_64-linux-gnu/qt5/QtCore/QList>
#include </usr/include/x86_64-linux-gnu/qt5/QtCore/QStringList>
#include </usr/include/x86_64-linux-gnu/qt5/QtCore/QTimer>
#include </usr/include/x86_64-linux-gnu/qt5/QtCore/QMimeData>
#endif
#ifdef QT4
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
#endif

#define KEY_SETTING_PATH "keySetting.config"

#define METAFILE_CLIP_PIC_HEADERS_SIZE 130
#define META_DIBSTRETCHBLT_HEADER_SIZE 66
#define METAFILE_HEADER_SIZE           118

#define PASTE_TEXT_FIRST_PART_SIZE   1592 // = 1600 - 8
#define PASTE_PIC_FIRST_PART_SIZE    1462 // = 1600 - METAFILEPICT_HEADER_SIZE - 8
#define PDU_MAX_SIZE                 1600




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
    QFormLayout        * _layoutView;
    QFormLayout        * _layoutKeyboard;
    QLabel               _labelBpp;
    QLabel               _labelResolution;
    QLabel               _labelPerf;
    QLabel               _labelLanguage;
    QLabel               _labelFps;
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

        this->_bppComboBox.addItem("32", 32);
        this->_bppComboBox.addItem("24", 24);
        this->_bppComboBox.addItem("16", 16);
        this->_bppComboBox.addItem("15", 15);
        int indexBpp = this->_bppComboBox.findData(this->_front->_info.bpp);
        if ( indexBpp != -1 ) {
            this->_bppComboBox.setCurrentIndex(indexBpp);
        }
        this->_bppComboBox.setStyleSheet("combobox-popup: 0;");
        this->_layoutView->addRow(new QLabel("", this));
        this->_layoutView->addRow(&(this->_labelBpp), &(this->_bppComboBox));

        this->_resolutionComboBox.addItem( "640 * 480", 640);
        this->_resolutionComboBox.addItem( "800 * 600", 800);
        this->_resolutionComboBox.addItem("1024 * 768", 1024);
        this->_resolutionComboBox.addItem("1600 * 900", 1600);
        int indexResolution = this->_resolutionComboBox.findData(this->_front->_info.width);
        if ( indexResolution != -1 ) {
            this->_resolutionComboBox.setCurrentIndex(indexResolution);
        }
        this->_resolutionComboBox.setStyleSheet("combobox-popup: 0;");
        this->_layoutView->addRow(new QLabel("", this));
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
        this->_layoutView->addRow(new QLabel("", this));
        this->_layoutView->addRow(&(this->_labelFps), &(this->_fpsComboBox));

        if (this->_front->_info.rdp5_performanceflags == PERF_DISABLE_WALLPAPER) {
            this->_perfCheckBox.setCheckState(Qt::Checked);
        }
        this->_layoutView->addRow(new QLabel("", this));
        this->_layoutView->addRow(&(this->_labelPerf), &(this->_perfCheckBox));

        this->_viewTab->setLayout(this->_layoutView);

        this->_tabs->addTab(this->_viewTab, strView);


        // Keyboard tab
        const QString strKeyboard("Keyboard");
        this->_layoutKeyboard = new QFormLayout(this->_keyboardTab);

        for (int i = 0; i < Qt_ScanCode_KeyMap::KEYLAYOUTS_LIST_SIZE; i++) {
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
;
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

    void saveReleased() {
        //this->_front->_info.console_session = 0;
        //this->_front->_info.brush_cache_code = 0;
        this->_front->_info.bpp = this->_bppComboBox.currentText().toInt();
        this->_front->_imageFormatRGB  = this->_front->bpp_to_QFormat(this->_front->_info.bpp, false);
        this->_front->_imageFormatARGB = this->_front->bpp_to_QFormat(this->_front->_info.bpp, true);
        std::string delimiter = " * ";
        std::string resolution( this->_resolutionComboBox.currentText().toStdString());
        int pos(resolution.find(delimiter));
        this->_front->_info.width  = std::stoi(resolution.substr(0, pos));
        this->_front->_info.height = std::stoi(resolution.substr(pos + delimiter.length(), resolution.length()));
        this->_front->_fps = this->_fpsComboBox.currentText().toInt();
        if (this->_perfCheckBox.isChecked()) {
            this->_front->_info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
        } else {
            this->_front->_info.rdp5_performanceflags = 0;
        }
        this->_front->_info.keylayout = this->_languageComboBox.itemData(this->_languageComboBox.currentIndex()).toInt();
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
    Front_Qt_API       * _front;
    const int            _width;
    const int            _height;
    QFormLayout          _formLayout;
    QLineEdit            _IPField;
    QLineEdit            _userNameField;
    QLineEdit            _PWDField;
    QLineEdit            _portField;
    QLabel               _IPLabel;
    QLabel               _userNameLabel;
    QLabel               _PWDLabel;
    QLabel               _portLabel;
    QLabel               _errorLabel;
    QPushButton          _buttonConnexion;
    QPushButton          _buttonOptions;


    Form_Qt(Front_Qt_API * front)
        : QWidget()
        , _front(front)
        , _width(400)
        , _height(300)
        , _formLayout(this)
        , _IPField("", this)
        , _userNameField("", this)
        , _PWDField("", this)
        , _portField("", this)
        , _IPLabel(      QString("IP serveur :"), this)
        , _userNameLabel(QString("User name : "), this)
        , _PWDLabel(     QString("Password :  "), this)
        , _portLabel(    QString("Port :      "), this)
        , _errorLabel(   QString(""            ), this)
        , _buttonConnexion("Connection", this)
        , _buttonOptions("Options", this)
    {
        this->setWindowTitle("Remote Desktop Player");
        this->setAttribute(Qt::WA_DeleteOnClose);
        this->setFixedSize(this->_width, this->_height);

        this->_PWDField.setEchoMode(QLineEdit::Password);
        this->_PWDField.setInputMethodHints(Qt::ImhHiddenText | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase);
        this->_formLayout.addRow(&(this->_IPLabel)      , &(this->_IPField));
        this->_formLayout.addRow(&(this->_userNameLabel), &(this->_userNameField));
        this->_formLayout.addRow(&(this->_PWDLabel)     , &(this->_PWDField));
        this->_formLayout.addRow(&(this->_portLabel)    , &(this->_portField));
        this->_formLayout.addRow(&(this->_errorLabel));
        this->setLayout(&(this->_formLayout));

        QRect rectConnexion(QPoint(280, 256), QSize(110, 24));
        this->_buttonConnexion.setToolTip(this->_buttonConnexion.text());
        this->_buttonConnexion.setGeometry(rectConnexion);
        this->_buttonConnexion.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonConnexion)   , SIGNAL (pressed()),  this, SLOT (connexionPressed()));
        this->QObject::connect(&(this->_buttonConnexion)   , SIGNAL (released()), this, SLOT (connexionReleased()));
        this->_buttonConnexion.setFocusPolicy(Qt::StrongFocus);

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
        return this->_IPField.text().toStdString();
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
    void connexionPressed() {
        this->_front->connexionPressed();
    }

    void connexionReleased() {
        this->_front->connexionReleased();
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
    QPixmap              _cache;
    //QGraphicsScene       _scene;
    //QGraphicsView        _view;
    QPainter             _cache_painter;
    const int            _width;
    const int            _height;
    bool                 _connexionLasted;
    const int            _buttonHeight;
    QTimer               _timer;


    Screen_Qt (Front_Qt_API * front)
    : QWidget()
    , _front(front)
    , _buttonCtrlAltDel("CTRL + ALT + DELETE", this)
    , _buttonRefresh("Refresh", this)
    , _buttonDisconnexion("Disconnection", this)
    , _penColor(Qt::black)
    , _cache(this->_front->_info.width, this->_front->_info.height)
    //, _scene(0, 0, this->_front->_info.width, this->_front->_info.height, this)
    //, _view(&(this->_scene), this)
    , _cache_painter(&(this->_cache))
    , _width(this->_front->_info.width)
    , _height(this->_front->_info.height)
    , _connexionLasted(false)
    , _buttonHeight(20)
    , _timer(this)
    {
        this->setMouseTracking(true);
        this->installEventFilter(this);
        this->setAttribute(Qt::WA_DeleteOnClose);
        std::string title = "Remote Desktop Player connected to [" + this->_front->_targetIP +  "].";
        this->setWindowTitle(QString(title.c_str()));

        this->setFixedSize(this->_width, this->_height + this->_buttonHeight);
        this->_cache_painter.fillRect(0, 0, this->_width, this->_height, QColor(0, 0, 0, 0));

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
        int centerW = (desktop->width()/2)  - (this->_width/2);
        int centerH = (desktop->height()/2) - ((this->_height+20)/2);
        this->move(centerW, centerH);

        this->QObject::connect(&(this->_timer), SIGNAL (timeout()),  this, SLOT (slotRepaint()));
        this->_timer.start(1000/this->_front->_fps);

        this->setFocusPolicy(Qt::StrongFocus);
    }

    ~Screen_Qt() {
        if (!this->_connexionLasted) {
            this->_front->closeFromScreen();
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
        painter.drawPixmap(0, 0, this->_cache);
        painter.end();
    }

    QPixmap * getCache() {
        return &(this->_cache);
    }

    void setPenColor(QColor color) {
        this->_penColor = color;
    }


private:
    void mousePressEvent(QMouseEvent *e) {
        this->_front->mousePressEvent(e);
    }

    void mouseReleaseEvent(QMouseEvent *e) {
        this->_front->mouseReleaseEvent(e);
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
        this->_front->eventFilter(obj, e);
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
};



class Connector_Qt : public QObject
{

Q_OBJECT

public:
    Front_Qt_API    * _front;
    QSocketNotifier * _sckRead;
    mod_api         * _callback;
    SocketTransport * _sck;
    int               _client_sck;
    QClipboard      * _clipboard;
    bool              _local_clipboard_stream;
    size_t            _length;
    uint8_t         * _chunk;
    QImage          * _bufferImage;
    uint16_t          _bufferTypeID;


    Connector_Qt(Front_Qt_API * front, QWidget * parent)
    : QObject(parent)
    , _front(front)
    , _sckRead(nullptr)
    , _callback(nullptr)
    , _sck(nullptr)
    , _client_sck(0)
    , _clipboard(nullptr)
    , _local_clipboard_stream(true)
    , _length(0)
    , _chunk(nullptr)
    , _bufferImage(nullptr)
    , _bufferTypeID(0)
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
            this->_callback->send_disconnect_ultimatum();
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
        const char * targetIP(this->_front->_targetIP.c_str());
        const std::string errorMsg("Cannot connect to [" + this->_front->_targetIP +  "].");

        //std::cout << name << " " << this->_front->_pwd << " " << this->_front->_targetIP.c_str() << " " << this->_front->_port << std::endl;

        this->_client_sck = ip_connect(targetIP, this->_front->_port, this->_front->_nbTry, this->_front->_retryDelay, {});

        if (this->_client_sck > 0) {
            try {
                std::string error_message;
                const char * name(this->_front->_userName.c_str());
                this->_sck = new SocketTransport( name
                                                , this->_client_sck
                                                , targetIP
                                                , this->_front->_port
                                                , this->_front->verbose
                                                // TODO error_message life time is too short
                                                , &error_message
                                                );
                std::cout << "Connected to [" << targetIP <<  "]." << std::endl;
                return true;

            } catch (const std::exception & e) {
                std::cout << errorMsg << std::endl;
                std::string windowErrorMsg("<font color='Red'>"+errorMsg+"</font>");
                this->_front->disconnect(windowErrorMsg);
                return false;
            }
        } else {
            std::cout << errorMsg << std::endl;
            std::string windowErrorMsg("<font color='Red'>"+errorMsg+"</font>");
            this->_front->disconnect(windowErrorMsg);
            return false;
        }
    }

    void listen() {
        const char * name(this->_front->_userName.c_str());
        const char * pwd(this->_front->_pwd.c_str());
        const char * targetIP(this->_front->_targetIP.c_str());
        const char * localIP(this->_front->_localIP.c_str());

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
        mod_rdp_params.enable_fastpath                 = false;
        mod_rdp_params.enable_mem3blt                  = true;
        mod_rdp_params.enable_bitmap_update            = true;
        mod_rdp_params.enable_new_pointer              = true;
        mod_rdp_params.server_redirection_support      = true;
        std::string allow_channels = "*";
        mod_rdp_params.allow_channels                  = &allow_channels;

        LCGRandom gen(0); // To always get the same client random, in tests

        try {
            this->_callback = new mod_rdp(*(this->_sck), *(this->_front), this->_front->_info, ini.get_ref<cfg::mod_rdp::redir_info>(), gen, mod_rdp_params);
            this->_front->_to_server_sender._callback = this->_callback;
            this->_front->_callback = this->_callback;
            this->_sckRead = new QSocketNotifier(this->_client_sck, QSocketNotifier::Read, this);
            this->QObject::connect(this->_sckRead,   SIGNAL(activated(int)), this,  SLOT(call_Draw()));

        } catch (const Error & e) {
            const std::string errorMsg("Error: connexion to [" + this->_front->_targetIP +  "] is closed.");
            std::cout << errorMsg << std::endl;
            std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");
            this->_front->dropScreen();
            this->_front->disconnect(labelErrorMsg);
        }
    }

    void setClipboard(const std::string & str) {
        this->_clipboard->setText(QString::fromUtf8(str.c_str()), QClipboard::Clipboard);
    }

    void setClipboard(const QImage & image) {
        this->_clipboard->setImage(image, QClipboard::Clipboard);
    }

    void emptyBuffer() {
        this->_bufferTypeID = 0;
        this->_length = 0;
        if (this->_chunk != nullptr) {
            delete (this->_chunk);
            this->_chunk = nullptr;
        }
    }

    void send_FormatListPDU() {
        uint32_t formatIDs[]                  = {this->_bufferTypeID};
        std::string formatListDataShortName[] = {std::string("")};
        this->_front->send_FormatListPDU(formatIDs, formatListDataShortName, 1);
    }


public Q_SLOTS:
    void call_Draw() {
        this->_front->call_Draw();
    }

    void mem_clipboard() {
        if (this->_callback != nullptr && this->_local_clipboard_stream) {
            const QMimeData * mimeData = this->_clipboard->mimeData();

            if (!mimeData->hasUrls()) {
                if (mimeData->hasImage()){

                    this->emptyBuffer();

                    this->_bufferTypeID = RDPECLIP::CF_METAFILEPICT;
                    this->_bufferImage = new QImage(this->_clipboard->image());
                    this->_length = this->_bufferImage->byteCount();

                    this->send_FormatListPDU();

                } else if (mimeData->hasText()){

                    this->emptyBuffer();
                    this->_bufferTypeID = RDPECLIP::CF_UNICODETEXT;
                    int cmptCR(0);
                    std::string str(std::string(this->_clipboard->text(QClipboard::Clipboard).toUtf8().constData()) + std::string(" "));
                    std::string tmp(str);
                    int pos(tmp.find("\n"));

                    while (pos != -1) {
                        cmptCR++;
                        tmp = tmp.substr(pos+2, tmp.length());
                        pos = tmp.find("\n"); // for linux install
                    }
                    size_t size((str.length() + cmptCR*2) * 4);

                    this->_chunk  = new uint8_t[size];
                    this->_length = ::UTF8toUTF16_CrLf(reinterpret_cast<const uint8_t *>(str.c_str()), this->_chunk, size);  // UTF8toUTF16_CrLf for linux install

                    this->send_FormatListPDU();
                }
            }
        }
    }

};


