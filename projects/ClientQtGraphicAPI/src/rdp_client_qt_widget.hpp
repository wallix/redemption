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

#include <vector>

#include "core/RDP/clipboard.hpp"
#include "core/FSCC/FileInformation.hpp"
#include "core/RDP/channels/rdpdr.hpp"
#include "core/RDPEA/audio_output.hpp"
#include "core/RDP/MonitorLayoutPDU.hpp"
#include "core/channel_list.hpp"

#include "mod/rdp/rdp.hpp"

#include "rdp_client_graphic_api/front_qt_rdp_graphic_api.hpp"


#define _SHARE_PATH "/share"
#define CB_FILE_TEMP_PATH "/clipboard_temp"
#define KEY_SETTING_PATH "/config/keySetting.config"
#define USER_CONF_PATH "/config/userConfig.config"


#define _WINDOWS_TICK 10000000
#define _SEC_TO_UNIX_EPOCH 11644473600LL



class Front_RDP_Qt_API : public FrontQtRDPGraphicAPI
{

private:
    class ClipboardServerChannelDataSender : public VirtualChannelDataSender
    {
    public:
        mod_api        * _callback;

        ClipboardServerChannelDataSender() = default;


        void operator()(uint32_t total_length, uint32_t flags, const uint8_t* chunk_data, uint32_t chunk_data_length) override {
            //std::cout << "operator()  server " << (int)flags  << std::endl;
            InStream chunk(chunk_data, chunk_data_length);
            this->_callback->send_to_mod_channel(channel_names::cliprdr, chunk, total_length, flags);
        }
    };

    class ClipboardClientChannelDataSender : public VirtualChannelDataSender
    {
    public:
        FrontAPI            * _front;
        CHANNELS::ChannelDef  _channel;

        ClipboardClientChannelDataSender() = default;


        void operator()(uint32_t total_length, uint32_t flags, const uint8_t* chunk_data, uint32_t chunk_data_length) override {
            //std::cout << "operator()  client " << (int)flags  << std::endl;

            this->_front->send_to_channel(this->_channel, chunk_data, total_length, chunk_data_length, flags);
        }
    };

public:

    enum : int {
        BUTTON_HEIGHT = 20
    };

    enum : int {
        MAX_MONITOR_COUNT = GCC::UserData::CSMonitor::MAX_MONITOR_COUNT / 4
    };

    ClipboardServerChannelDataSender _to_server_sender;
    ClipboardClientChannelDataSender _to_client_sender;
    ClipboardVirtualChannel          clipboard_channel;
    int                  _monitorCount;
    Rect                 _screen_dimensions[MAX_MONITOR_COUNT];
    int                  _current_screen_index;
    bool                 _recv_disconnect_ultimatum;
    bool                 enable_shared_clipboard;
    bool                 enable_shared_virtual_disk;

    const std::string    CB_TEMP_DIR;
    std::string          SHARE_DIR;
    const std::string    USER_CONF_DIR;

    struct ModRDPParamsData
    {
        bool enable_tls                      = false;
        bool enable_nla                      = false;
        bool enable_sound                    = false;
    } modRDPParamsData;

    struct ClipbrdFormatsList{

        enum : uint16_t {
              CF_QT_CLIENT_FILEGROUPDESCRIPTORW = 48025
            , CF_QT_CLIENT_FILECONTENTS         = 48026
        };

        enum : int {
              CLIPBRD_FORMAT_COUNT = 5
        };

        const std::string FILECONTENTS;
        const std::string FILEGROUPDESCRIPTORW;
        uint32_t          IDs[CLIPBRD_FORMAT_COUNT];
        std::string       names[CLIPBRD_FORMAT_COUNT];
        int index = 0;
        const double      ARBITRARY_SCALE;  //  module MetaFilePic resolution, value=40 is
                                            //  empirically close to original resolution.

        ClipbrdFormatsList()
          : FILECONTENTS(
              "F\0i\0l\0e\0C\0o\0n\0t\0e\0n\0t\0s\0\0\0"
            , 26)
          , FILEGROUPDESCRIPTORW(
              "F\0i\0l\0e\0G\0r\0o\0u\0p\0D\0e\0s\0c\0r\0i\0p\0t\0o\0r\0W\0\0\0"
            , 42)
          , ARBITRARY_SCALE(40)
        {}

        void add_format(uint32_t ID, const std::string & name) {
            if (index < CLIPBRD_FORMAT_COUNT) {
                IDs[index]   = ID;
                names[index] = name;
                index++;
            }
        }

    } clipbrdFormatsList;

    struct CB_FilesList {
        struct CB_in_Files {
            int         size;
            std::string name;
        };
        uint32_t                 cItems = 0;
        uint32_t                 lindexToRequest = 0;
        int                      streamIDToRequest = 0;
        std::vector<CB_in_Files> itemslist;
        int                      lindex = 0;

    }  _cb_filesList;

    struct CB_Buffers {
        std::unique_ptr<uint8_t[]>  data = nullptr;
        size_t size = 0;
        size_t sizeTotal = 0;
        int    pic_width = 0;
        int    pic_height = 0;
        int    pic_bpp = 0;

    } _cb_buffers;


    int current_user_profil;
    struct UserProfil {
        int id;
        std::string name;

        UserProfil(int id, const char * name)
          : id(id)
          , name(name) {}
    };
    std::vector<UserProfil> userProfils;

    struct KeyCustomDefinition {
        int qtKeyID  = 0;
        int scanCode = 0;
        int ASCII8   = 0;
        int extended = 0;

        KeyCustomDefinition(int qtKeyID, int scanCode, int ASCII8, int extended)
          : qtKeyID(qtKeyID)
          , scanCode(scanCode)
          , ASCII8(ASCII8)
          , extended(extended) {}
    };
    std::vector<KeyCustomDefinition> keyCustomDefinitions;




    Front_RDP_Qt_API( RDPVerbose verbose)
    : FrontQtRDPGraphicAPI(verbose)
     , clipboard_channel(&(this->_to_client_sender), &(this->_to_server_sender) ,*this , [](){
            NullReportMessage reportMessage;
            ClipboardVirtualChannel::Params params(reportMessage);

            params.exchanged_data_limit = ~decltype(params.exchanged_data_limit){};
            params.verbose = to_verbose_flags(0);

            params.clipboard_down_authorized = true;
            params.clipboard_up_authorized = true;
            params.clipboard_file_authorized = true;

            params.dont_log_data_into_syslog = true;
            params.dont_log_data_into_wrm = true;

//            params.client_use_long_format_names = true;

            return params;
        }())
    , _current_screen_index(0)
    , _recv_disconnect_ultimatum(false)
    , enable_shared_clipboard(false)
    , enable_shared_virtual_disk(false)
    , CB_TEMP_DIR(MAIN_DIR + std::string(CB_FILE_TEMP_PATH))
    , SHARE_DIR(MAIN_DIR + std::string(_SHARE_PATH))
    , USER_CONF_DIR(MAIN_DIR + std::string(USER_CONF_PATH))
    , clipbrdFormatsList()
    , _cb_filesList()
    , _cb_buffers()
    , current_user_profil(0)
    {
        this->_to_client_sender._front = this;
    }

    virtual void setClientInfo() = 0;
    virtual void setDefaultConfig() = 0;
    virtual void writeClientInfo() = 0;
    virtual void deleteCurrentProtile() = 0;

    // CHANNELS
    virtual void send_FormatListPDU(uint32_t const * formatIDs, const uint16_t ** formatListDataShortName, const std::size_t * size_names, std::size_t formatIDs_size) = 0;
    virtual void empty_buffer() = 0;
    virtual void emptyLocalBuffer() = 0;
    virtual void send_WaveConfirmPDU() = 0;

};



class DialogOptions_Qt : public QDialog
{

Q_OBJECT

public:
    Front_RDP_Qt_API   * _front;
    const int            _width;
    const int            _height;

    QWidget              _emptyPanel;
    QWidget            * _viewTab;
    QWidget            * _connectionTab;
    QWidget            * _servicesTab;
    QWidget            * _keyboardTab;

    QGridLayout        * _layout;
    QPushButton          _buttonSave;
    QPushButton          _buttonCancel;
    QPushButton        * _buttonDeleteKey;
    QPushButton        * _buttonAddKey;
    QPushButton          _buttonRestorConfig;
    QPushButton          _buttonDelConfProfil;
    QTabWidget         * _tabs;

    QComboBox            _bppComboBox;
    QComboBox            _resolutionComboBox;
    QCheckBox            _perfCheckBox;
    QCheckBox            _spanCheckBox;
    QCheckBox            _recordingCB;
    QCheckBox            _tlsBox;
    QCheckBox            _nlaBox;
    QCheckBox            _soundBox;
    QCheckBox            _clipboardCheckBox;
    QCheckBox            _shareCheckBox;

    QLineEdit            _sharePath;
    QPushButton          _buttonSharePath;

    QComboBox            _languageComboBox;
    QComboBox            profilComboBox;
    QLineEdit            profilLineEdit;
    QComboBox            _monitorCountComboBox;
    QComboBox            _captureSnapFreqComboBox;

    QFormLayout        * _layoutView;
    QFormLayout        * _layoutConnection;
    QFormLayout        * _layoutServices;
    QFormLayout        * _layoutKeyboard;

    QLabel               _labelBpp;
    QLabel               _labelResolution;
    QLabel               _labelPerf;
    QLabel               _labelSpan;
    QLabel               _labelLanguage;
    QLabel               _labelProfil;
    QLabel               _labelScreen;
    QLabel               _labelRecording;
    QLabel               _labelTls;
    QLabel               _labelNla;
    QLabel               _labelSound;
    QLabel               _labelCaptureFreq;
    QLabel               _labelClipboard;
    QLabel               _labelShare;
    QLabel               _labelSharePath;

    QTableWidget       * _tableKeySetting;
    const int            _columnNumber;
    const int            _tableKeySettingMaxHeight;


    DialogOptions_Qt(Front_RDP_Qt_API * front, QWidget * parent)
        : QDialog(parent)
        , _front(front)
        , _width(400)
        , _height(350)
        , _emptyPanel(this)
        , _viewTab(nullptr)
        , _connectionTab(nullptr)
        , _servicesTab(nullptr)
        , _keyboardTab(nullptr)
        , _layout(nullptr)
        , _buttonSave("Save", this)
        , _buttonCancel("Cancel", this)
        , _buttonDeleteKey(nullptr)
        , _buttonAddKey(nullptr)
        , _buttonRestorConfig("Default configuration", this)
        , _buttonDelConfProfil("Delete current Profile", this)
        , _tabs(nullptr)
        , _bppComboBox(this)
        , _resolutionComboBox(this)
        , _perfCheckBox(this)
        , _spanCheckBox(this)
        , _recordingCB(this)
        , _tlsBox(this)
        , _nlaBox(this)
        , _soundBox(this)
        , _clipboardCheckBox(this)
        , _shareCheckBox(this)
        , _sharePath(this->_front->SHARE_DIR.c_str(), this)
        , _buttonSharePath("Select a Directory", this)
        , _languageComboBox(this)
        , profilComboBox(this)
        , profilLineEdit("", this)
        , _monitorCountComboBox(this)
        , _captureSnapFreqComboBox(this)
        , _layoutView(nullptr)
        , _layoutConnection(nullptr)
        , _layoutKeyboard(nullptr)
        , _labelBpp("Color depth :", this)
        , _labelResolution("Resolution :", this)
        , _labelPerf("Disable wallaper :", this)
        , _labelSpan("Span screen :", this)
        , _labelLanguage("Keyboard Language :", this)
        , _labelProfil("Options Profil:", this)
        , _labelScreen("Screen :", this)
        , _labelRecording("Record movie :", this)
        , _labelTls("TLS :", this)
        , _labelNla("NLA :", this)
        , _labelSound("Sound :",  this)
        , _labelCaptureFreq("Capture per second :", this)
        , _labelClipboard("Shared Clipboard :", this)
        , _labelShare("Shared Virtual Disk :", this)
        , _labelSharePath("Shared Path :", this)
        , _tableKeySetting(nullptr)
        , _columnNumber(4)
        , _tableKeySettingMaxHeight((20*6)+11)
    {
        this->setWindowTitle("Options");
        this->setAttribute(Qt::WA_DeleteOnClose);
        this->setFixedSize(this->_width, this->_height);
        this->setModal(true);

        this->_front->setClientInfo();

        this->_layout = new QGridLayout(this);


        // Tab options
        this->_viewTab = new QWidget(this);
        this->_connectionTab = new QWidget(this);
        this->_servicesTab = new QWidget(this);
        this->_keyboardTab = new QWidget(this);
        this->_tabs = new QTabWidget(this);



        // Connection config
        const QString strConnection("General");
        this->_layoutConnection = new QFormLayout(this->_connectionTab);

        this->profilComboBox.setLineEdit(&(this->profilLineEdit));
        for (size_t i = 0; i < this->_front->userProfils.size(); i++) {
            this->profilComboBox.addItem(this->_front->userProfils[i].name.c_str(), this->_front->userProfils[i].id);
        }
        this->profilComboBox.setStyleSheet("combobox-popup: 0;");
        this->_layoutConnection->addRow(&(this->_labelProfil), &(this->profilComboBox));
        this->QObject::connect(&(this->profilComboBox), SIGNAL(currentIndexChanged(int)), this, SLOT(changeProfil(int)));


        this->_buttonRestorConfig.setFixedSize(160, 20);
        this->_buttonRestorConfig.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonRestorConfig) , SIGNAL (pressed()) , this, SLOT (restoreConfig()));

        this->_buttonDelConfProfil.setFixedSize(160, 20);
        this->_buttonDelConfProfil.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonDelConfProfil) , SIGNAL (pressed()) , this, SLOT (deleteCurrentProtile()));


        this->_layoutConnection->addRow(&(this->_buttonRestorConfig), &(this->_buttonDelConfProfil));

        this->_layoutConnection->addRow(&(this->_labelRecording), &(this->_recordingCB));
        this->QObject::connect(&(this->_recordingCB), SIGNAL(stateChanged(int)), this, SLOT(recordingCheckChange(int)));

        this->_captureSnapFreqComboBox.addItem("1"   , 1000000);
        this->_captureSnapFreqComboBox.addItem("10"   , 1000000/10);
        this->_captureSnapFreqComboBox.addItem("20"   , 1000000/20);
        this->_captureSnapFreqComboBox.addItem("40"   , 1000000/40);
        this->_captureSnapFreqComboBox.addItem("60"   , 1000000/60);
        this->_captureSnapFreqComboBox.setStyleSheet("combobox-popup: 0;");
        this->_layoutConnection->addRow(&(this->_labelCaptureFreq), &(this->_captureSnapFreqComboBox));
        this->_captureSnapFreqComboBox.setEnabled(this->_front->is_recording);


        this->_tlsBox.setCheckState(Qt::Unchecked);
        this->_layoutConnection->addRow(&(this->_labelTls), &(this->_tlsBox));

        this->_nlaBox.setCheckState(Qt::Unchecked);
        this->_layoutConnection->addRow(&(this->_labelNla), &(this->_nlaBox));

        this->_soundBox.setCheckState(Qt::Unchecked);
        this->_layoutConnection->addRow(&(this->_labelSound), &(this->_soundBox));

        this->_connectionTab->setLayout(this->_layoutConnection);
        this->_tabs->addTab(this->_connectionTab, strConnection);


        // VIEW TAB
        const QString strView("View");
        this->_layoutView = new QFormLayout(this->_viewTab);

        this->_bppComboBox.addItem("15", 15);
        this->_bppComboBox.addItem("16", 16);
        this->_bppComboBox.addItem("24", 24);
        this->_bppComboBox.setStyleSheet("combobox-popup: 0;");
        this->_layoutView->addRow(&(this->_labelBpp), &(this->_bppComboBox));

        this->_resolutionComboBox.addItem( "640 * 480", 640);
        this->_resolutionComboBox.addItem( "800 * 600", 800);
        this->_resolutionComboBox.addItem("1024 * 768", 1024);
        this->_resolutionComboBox.addItem("1600 * 900", 1600);
        this->_resolutionComboBox.setStyleSheet("combobox-popup: 0;");
        this->_layoutView->addRow(&(this->_labelResolution), &(this->_resolutionComboBox));


        this->_spanCheckBox.setCheckState(Qt::Unchecked);
        this->_layoutView->addRow(&(this->_labelSpan), &(this->_spanCheckBox));
        this->QObject::connect(&(this->_spanCheckBox), SIGNAL(stateChanged(int)), this, SLOT(spanCheckChange(int)));


        for (int i = 1; i <= Front_RDP_Qt_API::MAX_MONITOR_COUNT; i++) {
            this->_monitorCountComboBox.addItem(std::to_string(i).c_str(), i);
        }
        this->_monitorCountComboBox.setStyleSheet("combobox-popup: 0;");
        this->_layoutView->addRow(&(this->_labelScreen), &(this->_monitorCountComboBox));
        this->QObject::connect(&(this->_monitorCountComboBox), SIGNAL(currentIndexChanged(int)), this, SLOT(monitorCountkChange(int)));


        this->_layoutView->addRow(&(this->_labelPerf), &(this->_perfCheckBox));

        this->_viewTab->setLayout(this->_layoutView);
        this->_tabs->addTab(this->_viewTab, strView);


        // Services tab
        const QString strServices("Services");
        this->_layoutServices = new QFormLayout(this->_servicesTab);

        this->_clipboardCheckBox.setCheckState(Qt::Unchecked);
        this->_layoutServices->addRow(&(this->_labelClipboard), &(this->_clipboardCheckBox));

        this->_shareCheckBox.setCheckState(Qt::Unchecked);
        this->QObject::connect(&(this->_shareCheckBox), SIGNAL(stateChanged(int)), this, SLOT(setEnableSharePath(int)));
        this->_layoutServices->addRow(&(this->_labelShare), &(this->_shareCheckBox));

        this->_sharePath.setEnabled(this->_front->enable_shared_virtual_disk);
        this->_layoutServices->addRow(&(this->_labelSharePath), &(this->_sharePath));

        QRect rectPath(QPoint(190, 226),QSize(180, 24));
        this->_buttonSharePath.setGeometry(rectPath);
        this->_buttonSharePath.setCursor(Qt::PointingHandCursor);
        this->QObject::connect(&(this->_buttonSharePath) , SIGNAL (pressed()) , this, SLOT (dirPathPressed()));
        QLabel dirButtonLabel("", this);
        this->_layoutServices->addRow(&(dirButtonLabel), &(this->_buttonSharePath));

        this->_servicesTab->setLayout(this->_layoutServices);
        this->_tabs->addTab(this->_servicesTab, strServices);


        // Keyboard tab
        const QString strKeyboard("Keyboard");
        this->_layoutKeyboard = new QFormLayout(this->_keyboardTab);

        for (int i = 0; i < KEYLAYOUTS_LIST_SIZE; i++) {
            this->_languageComboBox.addItem(keylayoutsList[i]->locale_name, keylayoutsList[i]->LCID);
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


        for (size_t i = 0; i < this->_front->keyCustomDefinitions.size(); i++) {
            this->addRow();
            this->setRowValues(this->_front->keyCustomDefinitions[i].qtKeyID,
                               this->_front->keyCustomDefinitions[i].scanCode,
                               this->_front->keyCustomDefinitions[i].ASCII8,
                               this->_front->keyCustomDefinitions[i].extended);

        }


//         std::ifstream ifichier(this->_front->MAIN_DIR + std::string(KEY_SETTING_PATH), std::ios::in);
//         if(ifichier) {
//
//             std::string ligne;
//             std::string delimiter = " ";
//
//             while(getline(ifichier, ligne)) {
//
//                 int pos(ligne.find(delimiter));
//
//                 if (strcmp(ligne.substr(0, pos).c_str(), "-") == 0) {
//
//                     ligne = ligne.substr(pos + delimiter.length(), ligne.length());
//                     pos = ligne.find(delimiter);
//
//                     int qtKeyID  = std::stoi(ligne.substr(0, pos));
//                     ligne = ligne.substr(pos + delimiter.length(), ligne.length());
//                     pos = ligne.find(delimiter);
//
//                     int scanCode = std::stoi(ligne.substr(0, pos));
//                     ligne = ligne.substr(pos + delimiter.length(), ligne.length());
//                     pos = ligne.find(delimiter);
//
//                     int ASCII8   = std::stoi(ligne.substr(0, pos));
//                     ligne = ligne.substr(pos + delimiter.length(), ligne.length());
//                     pos = ligne.find(delimiter);
//
//                     int extended = std::stoi(ligne.substr(0, pos));
//
//                     this->_front->qtRDPKeymap.setCustomKeyCode(qtKeyID, scanCode, ASCII8, extended);
//
//                     this->addRow();
//                     this->setRowValues(qtKeyID, scanCode, ASCII8, extended);
//                 }
//             }
//
//             ifichier.close();
//         }
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


        this->setConfigValues();

        QDesktopWidget* desktop = QApplication::desktop();
        int centerW = (desktop->width()/2)  - (this->_width/2);
        int centerH = (desktop->height()/2) - (this->_height/2);
        this->move(centerW, centerH);

        this->show();
    }

    ~DialogOptions_Qt() {}


private:
    void setConfigValues() {
        int indexProfil = this->profilComboBox.findData(this->_front->current_user_profil);
        if ( indexProfil != -1 ) {
            this->profilComboBox.setCurrentIndex(indexProfil);
        }

        int indexBpp = this->_bppComboBox.findData(this->_front->info.bpp);
        if ( indexBpp != -1 ) {
            this->_bppComboBox.setCurrentIndex(indexBpp);
        }

        int indexResolution = this->_resolutionComboBox.findData(this->_front->info.width);
        if ( indexResolution != -1 ) {
            this->_resolutionComboBox.setCurrentIndex(indexResolution);
        }

        this->_monitorCountComboBox.setCurrentIndex(this->_front->info.cs_monitor.monitorCount-1);

        if (this->_front->is_spanning) {
            this->_spanCheckBox.setCheckState(Qt::Checked);
        } else {
            this->_spanCheckBox.setCheckState(Qt::Unchecked);
        }

        if (this->_front->info.rdp5_performanceflags == PERF_DISABLE_WALLPAPER) {
            this->_perfCheckBox.setCheckState(Qt::Checked);
        }

        int indexLanguage = this->_languageComboBox.findData(this->_front->info.keylayout);
        if ( indexLanguage != -1 ) {
            this->_languageComboBox.setCurrentIndex(indexLanguage);
        }

        int indexCaptureFreq = this->_captureSnapFreqComboBox.findData(this->_front->delta_time);
        if ( indexCaptureFreq != -1 ) {
            this->_captureSnapFreqComboBox.setCurrentIndex(indexCaptureFreq);
        }

        if (this->_front->is_recording) {
            this->_recordingCB.setCheckState(Qt::Checked);
        } else {
            this->_recordingCB.setCheckState(Qt::Unchecked);
        }

        if (this->_front->modRDPParamsData.enable_tls) {
            this->_tlsBox.setCheckState(Qt::Checked);
        } else {
            this->_tlsBox.setCheckState(Qt::Unchecked);
        }

        if (this->_front->modRDPParamsData.enable_nla) {
            this->_nlaBox.setCheckState(Qt::Checked);
        } else {
            this->_nlaBox.setCheckState(Qt::Unchecked);
        }

        if (this->_front->modRDPParamsData.enable_sound) {
            this->_soundBox.setCheckState(Qt::Checked);
        } else {
            this->_soundBox.setCheckState(Qt::Unchecked);
        }

        if (this->_front->enable_shared_clipboard) {
            this->_clipboardCheckBox.setCheckState(Qt::Checked);
        } else {
            this->_clipboardCheckBox.setCheckState(Qt::Unchecked);
        }

        if (this->_front->enable_shared_virtual_disk) {
            this->_shareCheckBox.setCheckState(Qt::Checked);
        } else {
            this->_shareCheckBox.setCheckState(Qt::Unchecked);
        }
    }

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
    void deleteCurrentProtile() {
        if (this->profilComboBox.currentIndex() != 0) {
            this->_front->deleteCurrentProtile();
            this->profilComboBox.removeItem(this->_front->current_user_profil);
            this->changeProfil(0);
        }
    }

    void restoreConfig() {
        this->_front->setDefaultConfig();
        this->setConfigValues();
    }

    void changeProfil(int index) {
        this->_front->current_user_profil = this->profilComboBox.itemData(index).toInt();
        this->_front->setClientInfo();
        this->setConfigValues();
    }

    void setEnableSharePath(int value) {
        this->_sharePath.setEnabled(value);
        this->_buttonSharePath.setEnabled(value);
    }

    void dirPathPressed() {
        QString filePath("");
        filePath = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                     this->_front->SHARE_DIR.c_str(),
                                                     QFileDialog::ShowDirsOnly |
                                                     QFileDialog::DontResolveSymlinks);
        std::string str_share_path(filePath.toStdString());
        this->_sharePath.setText(filePath);
    }

    void savePressed() {}

    void saveReleased() {

        bool new_profil = true;
        std::string text_profil = this->profilComboBox.currentText().toStdString();
        for (size_t i = 0; i < this->_front->userProfils.size(); i++) {
            if (this->_front->userProfils[i].name == text_profil) {
                new_profil = false;
            }
        }

        if (new_profil) {
            this->_front->userProfils.push_back({int(this->_front->userProfils.size()), text_profil.c_str()});
            this->_front->current_user_profil = this->_front->userProfils.size()-1;
        } else {
             this->_front->current_user_profil = this->profilComboBox.currentIndex();
        }

        this->_front->info.bpp = this->_bppComboBox.currentText().toInt();
        this->_front->imageFormatRGB  = this->_front->bpp_to_QFormat(this->_front->info.bpp, false);
        this->_front->imageFormatARGB = this->_front->bpp_to_QFormat(this->_front->info.bpp, true);
        std::string delimiter = " * ";
        std::string resolution( this->_resolutionComboBox.currentText().toStdString());
        int pos(resolution.find(delimiter));
        this->_front->info.width  = std::stoi(resolution.substr(0, pos));
        this->_front->info.height = std::stoi(resolution.substr(pos + delimiter.length(), resolution.length()));
        if (this->_perfCheckBox.isChecked()) {
            this->_front->info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
        } else {
            this->_front->info.rdp5_performanceflags = 0;
        }
        if (this->_spanCheckBox.isChecked()) {
            this->_front->is_spanning = true;
        } else {
            this->_front->is_spanning = false;
        }
        if (this->_recordingCB.isChecked()) {
            this->_front->is_recording = true;
        } else {
            this->_front->is_recording = false;
        }
        if (this->_tlsBox.isChecked()) {
            this->_front->modRDPParamsData.enable_tls = true;
        } else {
            this->_front->modRDPParamsData.enable_tls = false;
        }
        if (this->_nlaBox.isChecked()) {
            this->_front->modRDPParamsData.enable_nla = true;
        } else {
            this->_front->modRDPParamsData.enable_nla = false;
        }
        if (this->_soundBox.isChecked()) {
            this->_front->modRDPParamsData.enable_sound = true;
        } else {
            this->_front->modRDPParamsData.enable_sound = false;
        }
        this->_front->info.keylayout = this->_languageComboBox.itemData(this->_languageComboBox.currentIndex()).toInt();
        this->_front->info.cs_monitor.monitorCount = this->_monitorCountComboBox.itemData(this->_monitorCountComboBox.currentIndex()).toInt();
        this->_front->_monitorCount = this->_front->info.cs_monitor.monitorCount;
        this->_front->info.width   = this->_front->info.width * this->_front->_monitorCount;
        this->_front->info.height  = this->_front->info.height;
        this->_front->delta_time   = this->_captureSnapFreqComboBox.itemData(this->_captureSnapFreqComboBox.currentIndex()).toInt();

        if (this->_clipboardCheckBox.isChecked()) {
            this->_front->enable_shared_clipboard = true;
        } else {
            this->_front->enable_shared_clipboard = false;
        }

        if (this->_shareCheckBox.isChecked()) {
            this->_front->enable_shared_virtual_disk = true;
        } else {
            this->_front->enable_shared_virtual_disk = false;
        }

        this->_front->SHARE_DIR = this->_sharePath.text().toStdString();

        this->_front->writeClientInfo();

        remove((this->_front->MAIN_DIR + std::string(KEY_SETTING_PATH)).c_str());
        this->_front->qtRDPKeymap.clearCustomKeyCode();

        std::ofstream ofichier(this->_front->MAIN_DIR + std::string(KEY_SETTING_PATH), std::ios::out | std::ios::trunc);
        if(ofichier) {

            ofichier << "Key Setting" << std::endl << std::endl;

            const int row_count = this->_tableKeySetting->rowCount();

            for (int i = 0; i < row_count; i++) {
                int qtKeyID(0);
                if (!(this->_tableKeySetting->item(i, 0)->text().isEmpty())) {
                    qtKeyID = this->_tableKeySetting->item(i, 0)->text().toInt();
                }

                if (qtKeyID != 0) {
                    int scanCode(0);
                    if (!(this->_tableKeySetting->item(i, 0)->text().isEmpty())) {
                        scanCode = this->_tableKeySetting->item(i, 1)->text().toInt();
                    }
                    int ASCII8(0);
                    if (!(this->_tableKeySetting->item(i, 0)->text().isEmpty())) {
                        ASCII8 = this->_tableKeySetting->item(i, 2)->text().toInt();
                    }
                    int extended(static_cast<QComboBox*>(this->_tableKeySetting->cellWidget(i, 3))->currentIndex());

                    this->_front->qtRDPKeymap.setCustomKeyCode(qtKeyID, scanCode, ASCII8, extended);

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

        QTableWidgetItem * item1 = new QTableWidgetItem;
        item1->setText("");
        this->_tableKeySetting->setItem(rowNumber, 0, item1);

        QTableWidgetItem * item2 = new QTableWidgetItem;
        item2->setText("");
        this->_tableKeySetting->setItem(rowNumber, 1, item2);

        QTableWidgetItem * item3 = new QTableWidgetItem;
        item3->setText("");
        this->_tableKeySetting->setItem(rowNumber, 2, item3);


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

    void spanCheckChange(int state) {
        if (state == Qt::Unchecked) {
            this->_monitorCountComboBox.setCurrentIndex(0);
        }
    }

    void recordingCheckChange(int state) {
        if (state == Qt::Unchecked) {
            this->_captureSnapFreqComboBox.setEnabled(false);
        } else {
            this->_captureSnapFreqComboBox.setEnabled(true);
        }
    }

    void monitorCountkChange(int index) {
        if (index != 0) {
            this->_spanCheckBox.setCheckState(Qt::Checked);
        }
    }

};



class ClipBoard_Qt : public QObject
{

    Q_OBJECT

public:
    Front_RDP_Qt_API                  * _front;
    QClipboard                * _clipboard;
    bool                        _local_clipboard_stream;
    size_t                      _cliboard_data_length;
    std::unique_ptr<uint8_t[]>  _chunk;
    QImage                    * _bufferImage;
    uint16_t                    _bufferTypeID;
    std::string                 _bufferTypeLongName;
    int                         _cItems;
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



    ClipBoard_Qt(Front_RDP_Qt_API * front, QWidget * parent)
        : QObject(parent)
        , _front(front)
        , _clipboard(nullptr)
        , _local_clipboard_stream(true)
        , _cliboard_data_length(0)
        , _bufferImage(nullptr)
        , _bufferTypeID(0)
        , _bufferTypeLongName("")
        , _cItems(0)
    {
        this->clean_CB_temp_dir();
        this->_clipboard = QApplication::clipboard();
        this->QObject::connect(this->_clipboard, SIGNAL(dataChanged()),  this, SLOT(mem_clipboard()));
    }

    void write_clipboard_temp_file(std::string fileName, const uint8_t * data, size_t data_len) {
        std::string filePath(this->_front->CB_TEMP_DIR + std::string("/") + fileName);
        std::string filePath_mem(filePath);
        this->_temp_files_list.push_back(filePath_mem);
        std::ofstream oFile(filePath, std::ios::out | std::ios::binary | std::ios::app);
        if(oFile.is_open()) {
            oFile.write(reinterpret_cast<const char *>(data), data_len);
            oFile.close();
        }
    }

    void setClipboard_files(std::vector<Front_RDP_Qt_API::CB_FilesList::CB_in_Files> items_list) {

        /*QClipboard *cb = QApplication::clipboard();
        QMimeData* newMimeData = new QMimeData();
        const QMimeData* oldMimeData = cb->mimeData();
        QStringList ll = oldMimeData->formats();
        for (int i = 0; i < ll.size(); i++) {
            newMimeData->setData(ll[i], oldMimeData->data(ll[i]));
        }
        QList<QUrl> list;

        const std::string delimiter = "\n";
        uint32_t pos = 0;
        std::string str = items_list[0].name;
        while (pos <= str.size()) {
            pos = str.find(delimiter);
            std::string path = str.substr(0, pos);
            str = str.substr(pos+1, str.size());
            QString fileName(path.c_str());
            newMimeData->setText(fileName);
            list.append(QUrl::fromLocalFile(fileName));
            QByteArray gnomeFormat = QByteArray("copy\n").append(QUrl::fromLocalFile(fileName).toEncoded());
            newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);
        }

        newMimeData->setUrls(list);
        cb->setMimeData(newMimeData);*/


        QClipboard *cb = QApplication::clipboard();
        QMimeData* newMimeData = new QMimeData();
        const QMimeData* oldMimeData = cb->mimeData();
        QStringList ll = oldMimeData->formats();
        for (int i = 0; i < ll.size(); i++) {
            newMimeData->setData(ll[i], oldMimeData->data(ll[i]));
        }

        QByteArray gnomeFormat = QByteArray("copy\n");

        for (size_t i = 0; i < items_list.size(); i++) {

            std::string path(this->_front->CB_TEMP_DIR + std::string("/") + items_list[i].name);
            //std::cout <<  path <<  std::endl;
            QString qpath(path.c_str());

            //qDebug() << "QUrl" << QUrl::fromLocalFile(qpath);

            gnomeFormat.append(QUrl::fromLocalFile(qpath).toEncoded());
        }

        newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);
        cb->setMimeData(newMimeData);
    }

    void setClipboard_text(std::string & str) {
        this->_clipboard->setText(QString::fromUtf8(str.c_str()), QClipboard::Clipboard);
    }

    void setClipboard_image(const QImage & image) {               // Paste image to client
        this->_clipboard->setImage(image, QClipboard::Clipboard);
    }

    void clean_CB_temp_dir() {
        DIR *theFolder = opendir(this->_front->CB_TEMP_DIR.c_str());
        struct dirent *next_file;

        while ( (next_file = readdir(theFolder)) != NULL )
        {
            std::string filepath(this->_front->CB_TEMP_DIR + std::string("/") + std::string(next_file->d_name));
            remove(filepath.c_str());
        }
        closedir(theFolder);
    }

    void emptyBuffer() {
        this->_bufferTypeID = 0;
        this->_cliboard_data_length = 0;

        this->clean_CB_temp_dir();

        this->_temp_files_list.clear();
        for (size_t i = 0; i < _items_list.size(); i++) {
            delete(this->_items_list[i]);
        }
        this->_items_list.clear();
    }

    void send_FormatListPDU() {
        uint32_t formatIDs[]                  = { this->_bufferTypeID };
        const uint16_t * formatListDataShortName[] = { reinterpret_cast<const uint16_t *>(this->_bufferTypeLongName.data())};
        const size_t size_names[] = {this->_bufferTypeLongName.size()};

        this->_front->send_FormatListPDU(formatIDs, formatListDataShortName, size_names, 1);
    }


public Q_SLOTS:

    void mem_clipboard() {
        if (this->_front->mod != nullptr && this->_local_clipboard_stream) {
            const QMimeData * mimeData = this->_clipboard->mimeData();
            mimeData->hasImage();

            if (mimeData->hasImage()){
            //==================
            //    IMAGE COPY
            //==================
                this->emptyBuffer();

                this->_bufferTypeID = RDPECLIP::CF_METAFILEPICT;
                this->_bufferTypeLongName = std::string("\0\0", 2);
                QImage bufferImageTmp(this->_clipboard->image());
                if (bufferImageTmp.depth() > 24) {
                    bufferImageTmp = bufferImageTmp.convertToFormat(QImage::Format_RGB888);
                    bufferImageTmp = bufferImageTmp.rgbSwapped();
                }
                this->_bufferImage = new QImage(bufferImageTmp);

                this->_cliboard_data_length = this->_bufferImage->byteCount();

                this->_chunk = std::make_unique<uint8_t[]>(this->_cliboard_data_length + RDPECLIP::FormatDataResponsePDU_MetaFilePic::Ender::SIZE);
                for (int i  = 0; i < this->_bufferImage->byteCount(); i++) {
                    this->_chunk[i] = this->_bufferImage->bits()[i];
                }
                RDPECLIP::FormatDataResponsePDU_MetaFilePic::Ender ender;
                ender.emit(this->_chunk.get(), this->_cliboard_data_length);

                this->send_FormatListPDU();
            //==========================================================================



            } else if (mimeData->hasText()){                //  File or Text copy

                QString qstr = this->_clipboard->text(QClipboard::Clipboard);
                if (qstr.size() > 0) {
                    this->emptyBuffer();
                    std::string str(qstr.toUtf8().constData());

                    if (str.at(0) == '/') {
                //==================
                //    FILE COPY
                //==================
                        this->_bufferTypeID       = Front_RDP_Qt_API::ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW;
                        this->_bufferTypeLongName = this->_front->clipbrdFormatsList.FILEGROUPDESCRIPTORW;

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
                                int UTF16nameSize = ::UTF8toUTF16_CrLf(
                                    reinterpret_cast<const uint8_t *>(file->nameUTF8.c_str())
                                , UTF16nameData
                                , UTF8nameSize);
                                file->name = std::string(reinterpret_cast<char *>(UTF16nameData), UTF16nameSize);
                                this->_cItems++;
                                this->_items_list.push_back(file);

                            } else {
                                LOG(LOG_WARNING, "Path \"%s\" not found.", path.c_str());
                            }
                        }

                        this->send_FormatListPDU();
                //==========================================================================



                    } else {
                //==================
                //    TEXT COPY
                //==================
                        this->_bufferTypeID = RDPECLIP::CF_UNICODETEXT;
                        this->_bufferTypeLongName = std::string("\0\0", 2);

                        size_t size( ( str.length() * 4) + 2 );

                        this->_chunk = std::make_unique<uint8_t[]>(size);

                        // UTF8toUTF16_CrLf for linux install
                        this->_cliboard_data_length = ::UTF8toUTF16_CrLf(reinterpret_cast<const uint8_t *>(str.c_str()), this->_chunk.get(), size);

                        RDPECLIP::FormatDataResponsePDU_Text::Ender ender;
                        ender.emit(this->_chunk.get(), this->_cliboard_data_length);

                        this->_cliboard_data_length += RDPECLIP::FormatDataResponsePDU_Text::Ender::SIZE;

                        this->send_FormatListPDU();
                //==========================================================================
                    }
                }
            }
        }
    }

};



class Sound_Qt : public QObject
{

Q_OBJECT

    Phonon::MediaObject * media;
    Phonon::AudioOutput * audioOutput;

public:
    int wave_data_to_wait;

    uint32_t n_sample_per_sec;
    uint16_t bit_per_sample;
    uint16_t n_channels;
    uint16_t n_block_align;
    uint32_t bit_per_sec;

    bool last_PDU_is_WaveInfo;

    Front_RDP_Qt_API * front;

    std::string wave_file_to_write;

    int current_wav_index;
    int total_wav_files;




    Sound_Qt(QWidget * parent, Front_RDP_Qt_API * front)
      : QObject(parent)
      , media(nullptr)
      , audioOutput(nullptr)
      , wave_data_to_wait(0)
      , n_sample_per_sec(0)
      , bit_per_sample(0)
      , n_channels(0)
      , n_block_align(0)
      , bit_per_sec(0)
      , last_PDU_is_WaveInfo(false)
      , front(front)
      , current_wav_index(0)
      , total_wav_files(0)
    {
        this->media = new Phonon::MediaObject(this);
        this->audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
        Phonon::createPath(this->media, this->audioOutput);

        this->QObject::connect(this->media, SIGNAL (finished()),  this, SLOT (call_playback_over()));
    }

    void init(size_t raw_total_size) {

        this->total_wav_files++;

        this->wave_file_to_write = std::string("sound") + std::to_string(this->total_wav_files) +std::string(".wav");

        StaticOutStream<64> out_stream;
        out_stream.out_copy_bytes("RIFF", 4);
        out_stream.out_uint32_le(raw_total_size + 36);
        out_stream.out_copy_bytes("WAVEfmt ", 8);
        out_stream.out_uint32_le(16);
        out_stream.out_uint16_le(1);
        out_stream.out_uint16_le(this->n_channels);
        out_stream.out_uint32_le(this->n_sample_per_sec);
        out_stream.out_uint32_le(this->bit_per_sec);
        out_stream.out_uint16_le(this->n_block_align);
        out_stream.out_uint16_le(this->bit_per_sample);
        out_stream.out_copy_bytes("data", 4);
        out_stream.out_uint32_le(raw_total_size);

        std::ofstream file(this->wave_file_to_write.c_str(), std::ios::out| std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<const char *>(out_stream.get_data()), 44);
            file.close();
        }
    }

    void setData(const uint8_t * data, size_t size) {
        std::ofstream file(this->wave_file_to_write.c_str(), std::ios::app | std::ios::out| std::ios::binary);
        if (file) {
            file.write(reinterpret_cast<const char *>(data), size);
            file.close();
        }
    }

    void play() {
        if (this->media->state() == Phonon::StoppedState) {

            if (this->total_wav_files > this->current_wav_index) {

                this->current_wav_index++;
                std::string wav_file_name = std::string("sound") + std::to_string(this->current_wav_index) +std::string(".wav");

                Phonon::MediaSource sources(QUrl(wav_file_name.c_str()));
                this->media->setCurrentSource(sources);
                this->media->play();
            }
        }
    }

private Q_SLOTS:
    void call_playback_over() {
        std::string wav_file_name = std::string("sound") + std::to_string(this->current_wav_index) +std::string(".wav");
        remove(wav_file_name.c_str());

        this->play();
    }

};

