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
   Copyright (C) Wallix 2010-2016
   Author(s): Clément Moroldo
*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"

//#define LOGPRINT

#include <string>
#include <unistd.h>
#include <QtGui/QRgb>
#include <QtGui/QRegion>
#include <QtGui/QBitmap>

#include "front_widget_Qt4.hpp"
#pragma GCC diagnostic pop

#include "core/channel_list.hpp"
#include "core/channel_names.hpp"



Front_Qt::Front_Qt(char* argv[], int argc, uint32_t verbose)
    : Front_Qt_API(false, false, verbose)
    , mod_bpp(24)
    , mod_palette(BGRPalette::classic_332())
    , _form(nullptr)
    , _mod_qt(nullptr)
    , _clipboard_qt(nullptr)
    , _timer(0)
    , _connected(false)
    , _monitorCountNegociated(false)
    , _clipboard_channel(&(this->_to_client_sender), &(this->_to_server_sender) ,*this , [](){
        ClipboardVirtualChannel::Params params;

        params.authentifier = nullptr;
        params.exchanged_data_limit = ~decltype(params.exchanged_data_limit){};
        params.verbose = 0xfffffff;

        params.clipboard_down_authorized = true;
        params.clipboard_up_authorized = true;
        params.clipboard_file_authorized = true;

        params.dont_log_data_into_syslog = true;
        params.dont_log_data_into_wrm = true;

        return params;
    }())
    , _keymap()
    , _ctrl_alt_delete(false)
    , _bufferRDPClipboardChannel(nullptr)
    , _bufferRDPClipboardChannelSize(0)
    , _bufferRDPClipboardChannelSizeTotal(0)
    , _bufferRDPCLipboardMetaFilePic_width(0)
    , _bufferRDPCLipboardMetaFilePic_height(0)
    , _clipbrdFormatsList()
    , _cItems(0)
    , _lindexToRequest(0)
    , _streamIDToRequest(0)
    , _waiting_for_data(false)
    , _lindex(0)
{
    if(this->setClientInfo()) {
        this->writeClientInfo();
    }

    const char * localIPtmp = "unknow_local_IP";
    this->_localIP       = localIPtmp;
    this->_nbTry         = 3;
    this->_retryDelay    = 1000;

    uint8_t commandIsValid(0);

    // TODO QCommandLineParser / program_options
    for (int i = 0; i <  argc - 1; i++) {

        std::string word(argv[i]);
        std::string arg(argv[i+1]);

        if (       word == "-n") {
            this->_userName = arg;
            commandIsValid += NAME_GOTTEN;
        } else if (word == "-pwd") {
            this->_pwd = arg;
            commandIsValid += PWD_GOTTEN;
        } else if (word == "-ip") {
            this->_targetIP = arg;
            commandIsValid += IP_GOTTEN;
        } else if (word == "-p") {
            std::string portStr(arg);
            this->_port = std::stoi(portStr);
            commandIsValid += PORT_GOTTEN;
        }
    }

    QDesktopWidget* desktop = QApplication::desktop();
    int screen_count(desktop->screenCount());
    std::cout << "screen_count = " << screen_count << std::endl;
    if (this->_monitorCount > screen_count) {
        this->_monitorCount = screen_count;
    }

    if (this->_span) {

        this->_info.width  = 0;
        this->_info.height = 0;

        for (int i = 0; i < this->_monitorCount; i++) {
            const QRect rect = desktop->screenGeometry(i);

            this->_info.cs_monitor.monitorDefArray[i].left   = this->_info.width;
            this->_info.width  += rect.width();
            if (this->_info.height < rect.height()) {
                this->_info.height = rect.height();
            }
            // 0;
            this->_info.cs_monitor.monitorDefArray[i].top    = rect.top();
            this->_info.cs_monitor.monitorDefArray[i].right  = this->_info.width + rect.width() - 1;
            this->_info.cs_monitor.monitorDefArray[i].bottom = rect.height() - 1 - BUTTON_HEIGHT;

            this->_info.cs_monitor.monitorDefArray[i].flags  = 0;

            this->_screen_dimensions[i].x   = 0;
            this->_screen_dimensions[i].y   = 0;
            this->_screen_dimensions[i].cx  = rect.width();
            this->_screen_dimensions[i].cy  = rect.height();
        }
        this->_info.cs_monitor.monitorDefArray[0].flags  = GCC::UserData::CSMonitor::TS_MONITOR_PRIMARY;
        this->_info.height -= BUTTON_HEIGHT;

        /*for (int i = 0; i < this->_monitorCount; i++) {
            const QRect rect = desktop->screenGeometry(i);

            this->_info.cs_monitor.monitorDefArray[i].top = rect.top() + this->_info.height - this->_info.cs_monitor.monitorDefArray[i].bottom;
            this->_screen_dimensions[i].y = rect.top()
        }*/
    }

    CHANNELS::ChannelDef channel { channel_names::cliprdr
                                 , GCC::UserData::CSNet::CHANNEL_OPTION_INITIALIZED |
                                   GCC::UserData::CSNet::CHANNEL_OPTION_COMPRESS |
                                   GCC::UserData::CSNet::CHANNEL_OPTION_SHOW_PROTOCOL
                                 , PDU_MAX_SIZE+1
                                 };

    this->_clipbrdFormatsList.add_format( ClipbrdFormatsList::CF_QT_CLIENT_FILECONTENTS
                                          , this->_clipbrdFormatsList.FILECONTENTS
                                          );
    this->_clipbrdFormatsList.add_format( ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW
                                          , this->_clipbrdFormatsList.FILEGROUPDESCRIPTORW
                                          );
    this->_clipbrdFormatsList.add_format( RDPECLIP::CF_UNICODETEXT
                                          , std::string("\0\0", 2)
                                          );
    this->_clipbrdFormatsList.add_format( RDPECLIP::CF_TEXT
                                          , std::string("\0\0", 2)
                                          );
    this->_clipbrdFormatsList.add_format( RDPECLIP::CF_METAFILEPICT
                                          , std::string("\0\0", 2)
                                          );

    this->_to_client_sender._channel = channel;
    this->_cl.push_back(channel);

    if (this->mod_bpp == this->_info.bpp) {
        this->mod_palette = BGRPalette::classic_332();
    }

    this->_qtRDPKeymap.setKeyboardLayout(this->_info.keylayout);
    this->_keymap.init_layout(this->_info.keylayout);

    // Windows and socket contrainer
    this->_form      = new Form_Qt(this);
    this->_mod_qt = new Mod_Qt(this, this->_form);
    this->_clipboard_qt = new ClipBoard_Qt(this, this->_form);

    std::cout << "cs_monitor count negociated. MonitorCount=" << this->_monitorCount << std::endl;
                    std::cout << "width=" <<  this->_info.width <<  " " << "height=" << this->_info.height <<  std::endl;

    if (commandIsValid == Front_Qt::COMMAND_VALID) {
        this->connect();

    } else {
        std::cout << "Argument(s) required to connect: ";
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

        this->disconnect("");
    }
}


bool Front_Qt::setClientInfo() {

    // default config
    this->_info.keylayout = 0x040C;// 0x40C FR, 0x409 USA
    this->_info.console_session = 0;
    this->_info.brush_cache_code = 0;
    this->_info.bpp = 24;
    this->_imageFormatRGB  = this->bpp_to_QFormat(this->_info.bpp, false);
    if (this->_info.bpp ==  32) {
        this->_imageFormatARGB = this->bpp_to_QFormat(this->_info.bpp, true);
    }
    this->_width  = 800;
    this->_height = 600;
    this->_info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
    this->_fps = 30;
    this->_info.cs_monitor.monitorCount = 1;

    // file config
    std::ifstream ifichier(USER_CONF_PATH);
    if(ifichier) {
        // get config from conf file
        std::string ligne;
        std::string delimiter = " ";

        while(std::getline(ifichier, ligne)) {
            auto pos(ligne.find(delimiter));
            std::string tag  = ligne.substr(0, pos);
            std::string info = ligne.substr(pos + delimiter.length(), ligne.length());

            if (tag.compare(std::string("keylayout")) == 0) {
                this->_info.keylayout = std::stoi(info);
            } else
            if (tag.compare(std::string("console_session")) == 0) {
                this->_info.console_session = std::stoi(info);
            } else
            if (tag.compare(std::string("brush_cache_code")) == 0) {
                this->_info.brush_cache_code = std::stoi(info);
            } else
            if (tag.compare(std::string("bpp")) == 0) {
                this->_info.bpp = std::stoi(info);
            } else
            if (tag.compare(std::string("width")) == 0) {
                this->_width      = std::stoi(info);
            } else
            if (tag.compare(std::string("height")) == 0) {
                this->_height     = std::stoi(info);
            } else
            if (tag.compare(std::string("rdp5_performanceflags")) == 0) {
                this->_info.rdp5_performanceflags = std::stoi(info);
            } else
            if (tag.compare(std::string("fps")) == 0) {
                this->_fps = std::stoi(info);
            } else
            if (tag.compare(std::string("monitorCount")) == 0) {
                this->_info.cs_monitor.monitorCount = std::stoi(info);
                this->_monitorCount                 = std::stoi(info);
            }
        }

        this->_info.width  = this->_width * this->_monitorCount;
        this->_info.height = this->_height;

        ifichier.close();

        this->_imageFormatRGB  = this->bpp_to_QFormat(this->_info.bpp, false);
        if (this->_info.bpp ==  32) {
            this->_imageFormatARGB = this->bpp_to_QFormat(this->_info.bpp, true);
        }

        return false;

    } else {

        this->_info.width  = this->_width * this->_monitorCount;
        this->_info.height = this->_height;

        return true;
    }
}

void Front_Qt::writeClientInfo() {
    std::ofstream ofichier(USER_CONF_PATH, std::ios::out | std::ios::trunc);
    if(ofichier) {

        ofichier << "User Info" << std::endl << std::endl;

        ofichier << "keylayout "             << this->_info.keylayout               << "\n";
        ofichier << "console_session "       << this->_info.console_session         << "\n";
        ofichier << "brush_cache_code "      << this->_info.brush_cache_code        << "\n";
        ofichier << "bpp "                   << this->_info.bpp                     << "\n";
        ofichier << "width "                 << this->_width                        << "\n";
        ofichier << "height "                << this->_height                       << "\n";
        ofichier << "rdp5_performanceflags " << this->_info.rdp5_performanceflags   << "\n";
        ofichier << "fps "                   << this->_fps                          << "\n";
        ofichier << "monitorCount "          << this->_info.cs_monitor.monitorCount << "\n";

    }
}

void Front_Qt::set_pointer(Pointer const & cursor) {
    if (cursor.pointer_type !=  0) {
        //std::cout <<  "cursor=" << int(cursor.pointer_type) <<  std::endl;
    }
}

Screen_Qt * Front_Qt::getMainScreen() {
    return this->_screen[0];
}

Front_Qt::~Front_Qt() {
    this->empty_buffer();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//------------------------
//      CONTROLLERS
//------------------------

void Front_Qt::disconnexionReleased(){
    this->dropScreen();
    this->disconnect("");
}

void Front_Qt::dropScreen() {
    for (auto screen : this->_screen) {
        if (screen != nullptr) {
            screen->errorConnexion();
            screen->close();
            screen = nullptr;
        }
    }
}

void Front_Qt::closeFromScreen(int screen_index) {
    int i = 0;
    for (auto screen : this->_screen) {
        if (screen != nullptr && i != screen_index) {
            screen->errorConnexion();
            screen->close();
            screen = nullptr;
        }
        i++;
    }

    if (this->_form != nullptr && this->_connected) {
        this->_form->close();
    }
}

bool Front_Qt::connect() {

    if (this->_mod_qt->connect()) {
        if (!this->_span) {
            this->_screen_dimensions[0].cx = this->_info.width;
            this->_screen_dimensions[0].cy = this->_info.height;
        }
        this->_connected = true;
        this->_form->hide();


        this->_screen[0] = new Screen_Qt(this);
        //this->_screen[0]->show();
        //this->_cache = new QPixmap(this->_info.width, this->_info.height);

        if (!this->_monitorCountNegociated) {
            for (int i = 1; i < this->_monitorCount; i++) {
                this->_screen[i] = new Screen_Qt(this, i);
                this->_screen[i]->show();
            }
            //this->_screen[0]->activateWindow();
            this->_monitorCountNegociated = true;
        }

        this->_screen[0]->show();

        this->_mod_qt->listen();

        return true;
    }

    return false;
}

void Front_Qt::disconnect(std::string const & error) {

    if (this->_mod_qt != nullptr) {
        this->_mod_qt->drop_connexion();
    }
    this->_monitorCountNegociated = false;

    this->_form->set_IPField(this->_targetIP);
    this->_form->set_portField(this->_port);
    this->_form->set_PWDField(this->_pwd);
    this->_form->set_userNameField(this->_userName);
    this->_form->set_ErrorMsg(error);
    this->_form->show();

    this->_connected = false;
}

bool Front_Qt::connexionReleased(){
    this->_form->setCursor(Qt::WaitCursor);
    this->_userName =  this->_form->get_userNameField();
    this->_targetIP =  this->_form->get_IPField();
    this->_pwd      =  this->_form->get_PWDField();
    this->_port     =  this->_form->get_portField();

    bool res(false);
    if (!this->_targetIP.empty()){
        res = this->connect();
    }
    this->_form->setCursor(Qt::ArrowCursor);

    return res;
}

void Front_Qt::mousePressEvent(QMouseEvent *e, int screen_index) {
    if (this->_callback != nullptr) {
        int flag(0);
        switch (e->button()) {
            case 1: flag = MOUSE_FLAG_BUTTON1; break;
            case 2: flag = MOUSE_FLAG_BUTTON2; break;
            case 4: flag = MOUSE_FLAG_BUTTON4; break;
            default: break;
        }
        //std::cout << "mousePressed " << e->x() << " " <<  e->y() << std::endl;
        this->_callback->rdp_input_mouse(flag | MOUSE_FLAG_DOWN, e->x() + this->_width *screen_index, e->y(), &(this->_keymap));
    }
}

void Front_Qt::mouseReleaseEvent(QMouseEvent *e, int screen_index) {
    if (this->_callback != nullptr) {
        int flag(0);
        switch (e->button()) {

            case 1: flag = MOUSE_FLAG_BUTTON1; break;
            case 2: flag = MOUSE_FLAG_BUTTON2; break;
            case 4: flag = MOUSE_FLAG_BUTTON4; break;
            default: break;
        }
        //std::cout << "mouseRelease" << std::endl;
        this->_callback->rdp_input_mouse(flag, e->x() + this->_width *screen_index, e->y(), &(this->_keymap));
    }
}

void Front_Qt::keyPressEvent(QKeyEvent *e) {
    this->_qtRDPKeymap.keyEvent(0     , e);
    if (this->_qtRDPKeymap.scanCode != 0) {
        //std::cout << "keyPressed " << int(this->_qtRDPKeymap.scanCode) <<  std::endl;
        this->send_rdp_scanCode(this->_qtRDPKeymap.scanCode, this->_qtRDPKeymap.flag);

    }
}

void Front_Qt::keyReleaseEvent(QKeyEvent *e) {
    this->_qtRDPKeymap.keyEvent(KBD_FLAG_UP, e);
    if (this->_qtRDPKeymap.scanCode != 0) {
        this->send_rdp_scanCode(this->_qtRDPKeymap.scanCode, this->_qtRDPKeymap.flag);
    }
}

void Front_Qt::wheelEvent(QWheelEvent *e) {
    //std::cout << "wheel " << " delta=" << e->delta() << std::endl;
    int flag(MOUSE_FLAG_HWHEEL);
    if (e->delta() < 0) {
        flag = flag | MOUSE_FLAG_WHEEL_NEGATIVE;
    }
    if (this->_callback != nullptr) {
        //this->_callback->rdp_input_mouse(flag, e->x(), e->y(), &(this->_keymap));
    }
}

bool Front_Qt::eventFilter(QObject *, QEvent *e, int screen_index)  {
    if (e->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);
        //std::cout << "MouseMove " <<  mouseEvent->x() << " " <<  mouseEvent->y()<< std::endl;
        int x = mouseEvent->x() + this->_width *screen_index;
        int y = mouseEvent->y();

        if (x < 0) {
            x = 0;
        }
        if (y < 0) {
            y = 0;
        }

        if (this->_callback != nullptr) {
            this->_callback->rdp_input_mouse(MOUSE_FLAG_MOVE, x, y, &(this->_keymap));
        }
    }
    return false;
}

void Front_Qt::connexionPressed() {}

void Front_Qt::RefreshPressed() {
    Rect rect(0, 0, this->_info.width * this->_monitorCount, this->_info.height);
    this->_callback->rdp_input_invalidate(rect);
}

void Front_Qt::RefreshReleased() {}

void Front_Qt::CtrlAltDelPressed() {
    int flag = Keymap2::KBDFLAGS_EXTENDED;

    this->send_rdp_scanCode(KBD_SCANCODE_ALTGR , flag);     // ALT
    this->send_rdp_scanCode(KBD_SCANCODE_CTRL  , flag);     // CTRL
    this->send_rdp_scanCode(KBD_SCANCODE_DELETE, flag);  // DELETE
}

void Front_Qt::CtrlAltDelReleased() {
    int flag = Keymap2::KBDFLAGS_EXTENDED | KBD_FLAG_UP;

    this->send_rdp_scanCode(KBD_SCANCODE_ALTGR , flag);     // ALT
    this->send_rdp_scanCode(KBD_SCANCODE_CTRL  , flag);     // CTRL
    this->send_rdp_scanCode(KBD_SCANCODE_DELETE, flag);  // DELETE
}

void Front_Qt::disconnexionPressed() {}

void Front_Qt::send_rdp_scanCode(int keyCode, int flag) {
    Keymap2::DecodedKeys decoded_keys = this->_keymap.event(flag, keyCode, this->_ctrl_alt_delete);
    switch (decoded_keys.count)
    {
    case 2:
        if (this->_decoded_data.has_room(sizeof(uint32_t))) {
            this->_decoded_data.out_uint32_le(decoded_keys.uchars[0]);
        }
        if (this->_decoded_data.has_room(sizeof(uint32_t))) {
            this->_decoded_data.out_uint32_le(decoded_keys.uchars[1]);
        }
        break;
    case 1:
        if (this->_decoded_data.has_room(sizeof(uint32_t))) {
            this->_decoded_data.out_uint32_le(decoded_keys.uchars[0]);
        }
        break;
    default:
    case 0:
        break;
    }
    if (this->_callback != nullptr) {
        this->_callback->rdp_input_scancode(keyCode, 0, flag, this->_timer, &(this->_keymap));
    }
}

void Front_Qt::setMainScreenOnTopRelease() {
    this->_screen[0]->activateWindow();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------
//   GRAPHIC FUNCTIONS (factorization)
//---------------------------------------

void Front_Qt::draw_MemBlt(const Rect & drect, const Bitmap & bitmap, bool invert, int srcx, int srcy) {
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
    this->_screen[0]->paintCache().drawImage(trect, qbitmap);

    /*if (invert) {
        this->_screen[0]->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::red);
    }*/
}


void Front_Qt::draw_RDPScrBlt(int srcx, int srcy, const Rect & drect, bool invert) {
    QImage qbitmap(this->_screen[0]->getCache()->toImage().copy(srcx, srcy, drect.cx, drect.cy));
    if (invert) {
        qbitmap.invertPixels();
    }
    const QRect trect(drect.x, drect.y, drect.cx, drect.cy);
    this->_screen[0]->paintCache().drawImage(trect, qbitmap);
}


QColor Front_Qt::u32_to_qcolor(uint32_t color){
    uint8_t b(color >> 16);
    uint8_t g(color >> 8);
    uint8_t r(color);
    return {r, g, b};
}


QImage::Format Front_Qt::bpp_to_QFormat(int bpp, bool alpha) {
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



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------
//       DRAW FUNCTIONS
//-----------------------------

void Front_Qt::draw(const RDPPatBlt & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    //std::cout << "RDPPatBlt " << std::hex << static_cast<int>(cmd.rop) << std::endl;
    RDPPatBlt new_cmd24 = cmd;
    new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
    new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
    const Rect rect = clip.intersect(this->_info.width * this->_monitorCount, this->_info.height).intersect(cmd.rect);

    if (cmd.brush.style == 0x03 && (cmd.rop == 0xF0 || cmd.rop == 0x5A)) { // external
        enum { BackColor, ForeColor };
        QColor backColor = this->u32_to_qcolor(new_cmd24.back_color);
        QColor foreColor = this->u32_to_qcolor(new_cmd24.fore_color);

        switch (cmd.rop) {

            // +------+-------------------------------+
            // | 0x5A | ROP: 0x005A0049 (PATINVERT)   |
            // |      | RPN: DPx                      |
            // +------+-------------------------------+
            case 0x5A:
                {
                    QBrush brush(backColor, Qt::Dense4Pattern);
                    this->_screen[0]->paintCache().setBrush(brush);
                    this->_screen[0]->paintCache().setCompositionMode(QPainter::RasterOp_SourceXorDestination);
                    this->_screen[0]->paintCache().drawRect(rect.x, rect.y, rect.cx, rect.cy);
                    this->_screen[0]->paintCache().setBrush(Qt::SolidPattern);
                    this->_screen[0]->paintCache().setCompositionMode(QPainter::CompositionMode_SourceOver);
                }
                break;

            // +------+-------------------------------+
            // | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
            // |      | RPN: P                        |
            // +------+-------------------------------+
            case 0xF0:
                {   this->_screen[0]->paintCache().setPen(Qt::NoPen);
                    this->_screen[0]->paintCache().fillRect(rect.x, rect.y, rect.cx, rect.cy, backColor);
                    QBrush brush(foreColor, Qt::Dense4Pattern);
                    this->_screen[0]->paintCache().setBrush(brush);
                    this->_screen[0]->paintCache().drawRect(rect.x, rect.y, rect.cx, rect.cy);
                    this->_screen[0]->paintCache().setBrush(Qt::SolidPattern);
                }
                break;
            default:
                std::cout << "RDPPatBlt brush_style = 03 " << static_cast<int>(cmd.rop) << std::endl;
                break;
        }

    } else {
         switch (cmd.rop) {

                // +------+-------------------------------+
                // | 0x00 | ROP: 0x00000042 (BLACKNESS)   |
                // |      | RPN: 0                        |
                // +------+-------------------------------+
            case 0x00: // blackness
                this->_screen[0]->paintCache().drawRect(rect.x, rect.y, rect.cx, rect.cy);
                break;
                // +------+-------------------------------+
                // | 0x05 | ROP: 0x000500A9               |
                // |      | RPN: DPon                     |
                // +------+-------------------------------+


                // +------+-------------------------------+
                // | 0x0F | ROP: 0x000F0001               |
                // |      | RPN: Pn                       |
                // +------+-------------------------------+

                // +------+-------------------------------+
                // | 0x50 | ROP: 0x00500325               |
                // |      | RPN: PDna                     |
                // +------+-------------------------------+

                // +------+-------------------------------+
                // | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
                // |      | RPN: Dn                       |
                // +------+-------------------------------+
            case 0x55: // inversion
                //this->invert_color(rect);
                break;
                // +------+-------------------------------+
                // | 0x5A | ROP: 0x005A0049 (PATINVERT)   |
                // |      | RPN: DPx                      |
                // +------+-------------------------------+
            case 0x5A:
                this->_screen[0]->paintCache().setCompositionMode(QPainter::RasterOp_SourceXorDestination);
                this->_screen[0]->paintCache().drawRect(rect.x, rect.y, rect.cx, rect.cy);
                this->_screen[0]->paintCache().setCompositionMode(QPainter::CompositionMode_SourceOver);
                break;
                // +------+-------------------------------+
                // | 0x5F | ROP: 0x005F00E9               |
                // |      | RPN: DPan                     |
                // +------+-------------------------------+

                // +------+-------------------------------+
                // | 0xA0 | ROP: 0x00A000C9               |
                // |      | RPN: DPa                      |
                // +------+-------------------------------+

                // +------+-------------------------------+
                // | 0xA5 | ROP: 0x00A50065               |
                // |      | RPN: PDxn                     |
                // +------+-------------------------------+

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

                // +------+-------------------------------+
                // | 0xF0 | ROP: 0x00F00021 (PATCOPY)     |
                // |      | RPN: P                        |
                // +------+-------------------------------+

                // +------+-------------------------------+
                // | 0xF5 | ROP: 0x00F50225               |
                // |      | RPN: PDno                     |
                // +------+-------------------------------+

                // +------+-------------------------------+
                // | 0xFA | ROP: 0x00FA0089               |
                // |      | RPN: DPo                      |
                // +------+-------------------------------+

                // +------+-------------------------------+
                // | 0xFF | ROP: 0x00FF0062 (WHITENESS)   |
                // |      | RPN: 1                        |
                // +------+-------------------------------+
            case 0xFF: // whiteness
                this->_screen[0]->paintCache().drawRect(rect.x, rect.y, rect.cx, rect.cy);
                break;
            default:
                std::cout << "RDPPatBlt " << int(cmd.rop) << std::endl;
                break;
        }
    }

}


void Front_Qt::draw(const RDPOpaqueRect & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    //std::cout << "RDPOpaqueRect" << std::endl;
    RDPOpaqueRect new_cmd24 = cmd;
    new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);
    Rect rect(new_cmd24.rect.intersect(clip));
    this->_screen[0]->paintCache().fillRect(rect.x, rect.y, rect.cx, rect.cy, this->u32_to_qcolor(new_cmd24.color));
}


void Front_Qt::draw(const RDPBitmapData & bitmap_data, const Bitmap & bmp) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        bitmap_data.log(LOG_INFO, "FakeFront");
        LOG(LOG_INFO, "========================================\n");
    }
    //std::cout << "RDPBitmapData" << std::endl;
    if (!bmp.is_valid()){
        return;
    }

    Rect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top,
                            (bitmap_data.dest_right - bitmap_data.dest_left + 1),
                            (bitmap_data.dest_bottom - bitmap_data.dest_top + 1));
    const Rect clipRect(0, 0, this->_info.width * this->_monitorCount, this->_info.height);
    const Rect drect = rectBmp.intersect(clipRect);

    const int16_t mincx = std::min<int16_t>(bmp.cx(), std::min<int16_t>(this->_info.width - drect.x, drect.cx));
    const int16_t mincy = std::min<int16_t>(bmp.cy(), std::min<int16_t>(this->_info.height - drect.y, drect.cy));;

    if (mincx <= 0 || mincy <= 0) {
        return;
    }

    int rowYCoord(drect.y + drect.cy - 1);

    QImage::Format format(this->bpp_to_QFormat(bmp.bpp(), false)); //bpp
    QImage qbitmap(bmp.data(), mincx, mincy, bmp.line_size(), format);

    if (bmp.bpp() == 24) {
        qbitmap = qbitmap.rgbSwapped();
    }

    if (bmp.bpp() != this->_info.bpp) {
        qbitmap = qbitmap.convertToFormat(this->_imageFormatRGB);
    }

    for (size_t k = 0 ; k < drect.cy; k++) {

        QImage image(qbitmap.constScanLine(k), mincx, 1, qbitmap.format());
        QRect trect(drect.x, rowYCoord, mincx, 1);
        this->_screen[0]->paintCache().drawImage(trect, image);
        rowYCoord--;
    }
}


void Front_Qt::draw(const RDPLineTo & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    //std::cout << "RDPLineTo" << std::endl;
    RDPLineTo new_cmd24 = cmd;
    new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, 24, this->mod_palette);
    new_cmd24.pen.color  = color_decode_opaquerect(cmd.pen.color,  24, this->mod_palette);

    // TO DO clipping
    this->_screen[0]->setPenColor(this->u32_to_qcolor(new_cmd24.back_color));
    this->_screen[0]->paintCache().drawLine(new_cmd24.startx, new_cmd24.starty, new_cmd24.endx, new_cmd24.endy);
}


void Front_Qt::draw(const RDPScrBlt & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    //std::cout << "RDPScrBlt" << std::endl;

    const Rect drect = clip.intersect(this->_info.width * this->_monitorCount, this->_info.height).intersect(cmd.rect);
    if (drect.isempty()) {
        return;
    }

    int srcx(drect.x + cmd.srcx - cmd.rect.x);
    int srcy(drect.y + cmd.srcy - cmd.rect.y);

    switch (cmd.rop) {
            // +------+-------------------------------+
            // | 0x00 | ROP: 0x00000042 (BLACKNESS)   |
            // |      | RPN: 0                        |
            // +------+-------------------------------+
        case 0x00: this->_screen[0]->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
            break;
            // +------+-------------------------------+
            // | 0x11 | ROP: 0x001100A6 (NOTSRCERASE) |
            // |      | RPN: DSon                     |
            // +------+-------------------------------+

            // +------+-------------------------------+
            // | 0x22 | ROP: 0x00220326               |
            // |      | RPN: DSna                     |
            // +------+-------------------------------+

            // +------+-------------------------------+
            // | 0x33 | ROP: 0x00330008 (NOTSRCCOPY)  |
            // |      | RPN: Sn                       |
            // +------+-------------------------------+

            // +------+-------------------------------+
            // | 0x44 | ROP: 0x00440328 (SRCERASE)    |
            // |      | RPN: SDna                     |
            // +------+-------------------------------+

            // +------+-------------------------------+
            // | 0x55 | ROP: 0x00550009 (DSTINVERT)   |
            // |      | RPN: Dn                       |
            // +------+-------------------------------+
        case 0x55: this->draw_RDPScrBlt(srcx, srcy, drect, true);
            break;
            // +------+-------------------------------+
            // | 0x66 | ROP: 0x00660046 (SRCINVERT)   |
            // |      | RPN: DSx                      |
            // +------+-------------------------------+

            // +------+-------------------------------+
            // | 0x77 | ROP: 0x007700E6               |
            // |      | RPN: DSan                     |
            // +------+-------------------------------+

            // +------+-------------------------------+
            // | 0x88 | ROP: 0x008800C6 (SRCAND)      |
            // |      | RPN: DSa                      |
            // +------+-------------------------------+

            // +------+-------------------------------+
            // | 0x99 | ROP: 0x00990066               |
            // |      | RPN: DSxn                     |
            // +------+-------------------------------+
;
            // +------+-------------------------------+
            // | 0xAA | ROP: 0x00AA0029               |
            // |      | RPN: D                        |
            // +------+-------------------------------+
        case 0xAA: // nothing to change
            break;
            // +------+-------------------------------+
            // | 0xBB | ROP: 0x00BB0226 (MERGEPAINT)  |
            // |      | RPN: DSno                     |
            // +------+-------------------------------+

            // +------+-------------------------------+
            // | 0xCC | ROP: 0x00CC0020 (SRCCOPY)     |
            // |      | RPN: S                        |
            // +------+-------------------------------+*/
        case 0xCC: this->draw_RDPScrBlt(srcx, srcy, drect, false);
            break;
            // +------+-------------------------------+
            // | 0xDD | ROP: 0x00DD0228               |
            // |      | RPN: SDno                     |
            // +------+-------------------------------+

            // +------+-------------------------------+
            // | 0xEE | ROP: 0x00EE0086 (SRCPAINT)    |
            // |      | RPN: DSo                      |
            // +------+-------------------------------+

            // +------+-------------------------------+
            // | 0xFF | ROP: 0x00FF0062 (WHITENESS)   |
            // |      | RPN: 1                        |
            // +------+-------------------------------+
        case 0xFF:
            this->_screen[0]->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::white);
            break;
        default: std::cout << "RDPScrBlt (" << std::hex << static_cast<int>(cmd.rop) << ")" << std::endl;
            break;
    }
}


void Front_Qt::draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bitmap) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    //std::cout << "RDPMemBlt (" << std::hex << static_cast<int>(cmd.rop) << ")" << std::endl;
    const Rect& drect = clip.intersect(cmd.rect);
    if (drect.isempty()){
        return ;
    }

    switch (cmd.rop) {

        case 0x00: this->_screen[0]->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
        break;

        case 0x22:
        {
            const uint16_t mincx = std::min<int16_t>(bitmap.cx(), std::min<int16_t>(this->_info.width - drect.x, drect.cx));
            const uint16_t mincy = std::min<int16_t>(bitmap.cy(), std::min<int16_t>(this->_info.height - drect.y, drect.cy));

            if (mincx <= 0 || mincy <= 0) {
                return;
            }

            int rowYCoord(drect.y + drect.cy-1);

            QImage::Format format(this->bpp_to_QFormat(bitmap.bpp(), false)); //bpp
            QImage srcBitmap(bitmap.data(), mincx, mincy, bitmap.line_size(), format);
            QImage dstBitmap(this->_screen[0]->getCache()->toImage().copy(drect.x, drect.y, mincx, mincy));

            if (bitmap.bpp() == 24) {
                srcBitmap = srcBitmap.rgbSwapped();
            }

            if (bitmap.bpp() != this->_info.bpp) {
                srcBitmap = srcBitmap.convertToFormat(this->_imageFormatRGB);
            }
            dstBitmap = dstBitmap.convertToFormat(srcBitmap.format());

            int indice(mincy-1);

            std::unique_ptr<uchar[]> data = std::make_unique<uchar[]>(srcBitmap.bytesPerLine());

            for (size_t k = 0 ; k < mincy; k++) {

                const uchar * srcData = srcBitmap.constScanLine(k);
                const uchar * dstData = dstBitmap.constScanLine(indice - k);

                for (int i = 0; i < srcBitmap.bytesPerLine(); i++) {
                    data[i] = ~(srcData[i]) & dstData[i];
                }

                QImage image(data.get(), mincx, 1, srcBitmap.format());
                QRect trect(drect.x, rowYCoord, mincx, 1);
                this->_screen[0]->paintCache().drawImage(trect, image);

                rowYCoord--;
            }
        }
        break;

        case 0x33:this->draw_MemBlt(drect, bitmap, true, cmd.srcx + (drect.x - cmd.rect.x), cmd.srcy + (drect.y - cmd.rect.y));
        break;

        case 0x55:
        {
            const uint16_t mincx = std::min<int16_t>(bitmap.cx(), std::min<int16_t>(this->_info.width - drect.x, drect.cx));
            const uint16_t mincy = std::min<int16_t>(bitmap.cy(), std::min<int16_t>(this->_info.height - drect.y, drect.cy));

            if (mincx <= 0 || mincy <= 0) {
                return;
            }

            int rowYCoord(drect.y + drect.cy-1);

            QImage dstBitmap(this->_screen[0]->getCache()->toImage().copy(drect.x, drect.y, mincx, mincy));

            int indice(mincy-1);

            std::unique_ptr<uchar[]> data = std::make_unique<uchar[]>(dstBitmap.bytesPerLine());

            for (size_t k = 0 ; k < mincy; k++) {

                const uchar * dstData = dstBitmap.constScanLine(indice - k);

                for (int i = 0; i < dstBitmap.bytesPerLine(); i++) {
                    data[i] = ~(dstData[i]);
                }

                QImage image(data.get(), mincx, 1, dstBitmap.format());

                QRect trect(drect.x, rowYCoord, mincx, 1);
                this->_screen[0]->paintCache().drawImage(trect, image);

                rowYCoord--;
            }
        }
        break;

        case 0x66:
        {
            const uint16_t mincx = std::min<int16_t>(bitmap.cx(), std::min<int16_t>(this->_info.width - drect.x, drect.cx));
            const uint16_t mincy = std::min<int16_t>(bitmap.cy(), std::min<int16_t>(this->_info.height - drect.y, drect.cy));

            if (mincx <= 0 || mincy <= 0) {
                return;
            }

            int rowYCoord(drect.y + drect.cy-1);

            QImage::Format format(this->bpp_to_QFormat(bitmap.bpp(), false)); //bpp
            QImage srcBitmap(bitmap.data(), mincx, mincy, bitmap.line_size(), format);
            QImage dstBitmap(this->_screen[0]->getCache()->toImage().copy(drect.x, drect.y, mincx, mincy));

            if (bitmap.bpp() == 24) {
                srcBitmap = srcBitmap.rgbSwapped();
            }

            if (bitmap.bpp() != this->_info.bpp) {
                srcBitmap = srcBitmap.convertToFormat(this->_imageFormatRGB);
            }
            dstBitmap = dstBitmap.convertToFormat(srcBitmap.format());

            int indice(mincy-1);

            std::unique_ptr<uchar[]> data = std::make_unique<uchar[]>(srcBitmap.bytesPerLine());

            for (size_t k = 0 ; k < mincy; k++) {

                const uchar * srcData = srcBitmap.constScanLine(k);
                const uchar * dstData = dstBitmap.constScanLine(indice - k);

                for (int i = 0; i < srcBitmap.bytesPerLine(); i++) {
                    data[i] = srcData[i] ^ dstData[i];
                }

                QImage image(data.get(), mincx, 1, dstBitmap.format());

                QRect trect(drect.x, rowYCoord, mincx, 1);
                this->_screen[0]->paintCache().drawImage(trect, image);

                rowYCoord--;
            }
        }
        break;

        case 0x99:  // nothing to change
        break;

        case 0xCC: this->draw_MemBlt(drect, bitmap, false, cmd.srcx + (drect.x - cmd.rect.x), cmd.srcy + (drect.y - cmd.rect.y));
        break;

        case 0xEE:
        {
            const uint16_t mincx = std::min<int16_t>(bitmap.cx(), std::min<int16_t>(this->_info.width - drect.x, drect.cx));
            const uint16_t mincy = std::min<int16_t>(bitmap.cy(), std::min<int16_t>(this->_info.height - drect.y, drect.cy));

            if (mincx <= 0 || mincy <= 0) {
                return;
            }

            int rowYCoord(drect.y + drect.cy-1);

            QImage::Format format(this->bpp_to_QFormat(bitmap.bpp(), false)); //bpp
            QImage srcBitmap(bitmap.data(), mincx, mincy, bitmap.line_size(), format);
            QImage dstBitmap(this->_screen[0]->getCache()->toImage().copy(drect.x, drect.y, mincx, mincy));

            if (bitmap.bpp() == 24) {
                srcBitmap = srcBitmap.rgbSwapped();
            }

            if (bitmap.bpp() != this->_info.bpp) {
                srcBitmap = srcBitmap.convertToFormat(this->_imageFormatRGB);
            }
            dstBitmap = dstBitmap.convertToFormat(srcBitmap.format());

            int indice(mincy-1);

            std::unique_ptr<uchar[]> data = std::make_unique<uchar[]>(srcBitmap.bytesPerLine());

            for (size_t k = 0 ; k < mincy; k++) {

                const uchar * srcData = srcBitmap.constScanLine(k);
                const uchar * dstData = dstBitmap.constScanLine(indice - k);

                for (int i = 0; i < srcBitmap.bytesPerLine(); i++) {
                    data[i] = srcData[i] | dstData[i];
                }

                QImage image(data.get(), mincx, 1, srcBitmap.format());

                QRect trect(drect.x, rowYCoord, mincx, 1);
                this->_screen[0]->paintCache().drawImage(trect, image);

                rowYCoord--;
            }
        }
        break;

        case 0x88:
        {
            const uint16_t mincx = std::min<int16_t>(bitmap.cx(), std::min<int16_t>(this->_info.width - drect.x, drect.cx));
            const uint16_t mincy = std::min<int16_t>(bitmap.cy(), std::min<int16_t>(this->_info.height - drect.y, drect.cy));

            if (mincx <= 0 || mincy <= 0) {
                return;
            }

            int rowYCoord(drect.y + drect.cy-1);

            QImage::Format format(this->bpp_to_QFormat(bitmap.bpp(), false)); //bpp
            QImage srcBitmap(bitmap.data(), mincx, mincy, bitmap.line_size(), format);
            QImage dstBitmap(this->_screen[0]->getCache()->toImage().copy(drect.x, drect.y, mincx, mincy));

            if (bitmap.bpp() == 24) {
                srcBitmap = srcBitmap.rgbSwapped();
            }

            if (bitmap.bpp() != this->_info.bpp) {
                srcBitmap = srcBitmap.convertToFormat(this->_imageFormatRGB);
            }
            dstBitmap = dstBitmap.convertToFormat(srcBitmap.format());

            int indice(mincy-1);

            std::unique_ptr<uchar[]> data = std::make_unique<uchar[]>(srcBitmap.bytesPerLine());

            for (size_t k = 0 ; k < mincy; k++) {

                const uchar * srcData = srcBitmap.constScanLine(k);
                const uchar * dstData = dstBitmap.constScanLine(indice - k);

                for (int i = 0; i < srcBitmap.bytesPerLine(); i++) {
                    data[i] = srcData[i] & dstData[i];
                }

                QImage image(data.get(), mincx, 1, srcBitmap.format());

                QRect trect(drect.x, rowYCoord, mincx, 1);
                this->_screen[0]->paintCache().drawImage(trect, image);

                rowYCoord--;
            }
        }
        break;

        case 0xFF: this->_screen[0]->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::white);
        break;

        default: std::cout << "RDPMemBlt (" << std::hex << static_cast<int>(cmd.rop) << ")" << std::endl;
        break;
    }
}


void Front_Qt::draw(const RDPMem3Blt & cmd, const Rect & clip, const Bitmap & bitmap) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    //std::cout << "RDPMem3Blt " << std::hex << int(cmd.rop) << std::dec <<  std::endl;
    const Rect& drect = clip.intersect(cmd.rect);
    if (drect.isempty()){
        return ;
    }

    switch (cmd.rop) {
        case 0xB8:
            {
                const uint16_t mincx = std::min<int16_t>(bitmap.cx(), std::min<int16_t>(this->_info.width  - drect.x, drect.cx));
                const uint16_t mincy = std::min<int16_t>(bitmap.cy(), std::min<int16_t>(this->_info.height - drect.y, drect.cy));

                if (mincx <= 0 || mincy <= 0) {
                    return;
                }
                uint32_t fore_color24 = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
                const QColor fore(this->u32_to_qcolor(fore_color24));
                const uint8_t r(fore.red());
                const uint8_t g(fore.green());
                const uint8_t b(fore.blue());

                int rowYCoord(drect.y + drect.cy-1);
                const QImage::Format format(this->bpp_to_QFormat(bitmap.bpp(), true));

                QImage dstBitmap(this->_screen[0]->getCache()->toImage().copy(drect.x, drect.y, mincx, mincy));
                QImage srcBitmap(bitmap.data(), mincx, mincy, bitmap.line_size(), format);
                srcBitmap = srcBitmap.convertToFormat(QImage::Format_RGB888);
                dstBitmap = dstBitmap.convertToFormat(QImage::Format_RGB888);

                const size_t rowsize(srcBitmap.bytesPerLine());
                std::unique_ptr<uchar[]> data = std::make_unique<uchar[]>(rowsize);


                for (size_t k = 1 ; k < drect.cy; k++) {

                    const uchar * srcData = srcBitmap.constScanLine(k);
                    const uchar * dstData = dstBitmap.constScanLine(mincy - k);

                    for (size_t x = 0; x < rowsize-2; x += 3) {
                        data[x+0] = ((dstData[x+0] ^ r) & srcData[x+0]) ^ r;
                        data[x+1] = ((dstData[x+1] ^ g) & srcData[x+1]) ^ g;
                        data[x+2] = ((dstData[x+2] ^ b) & srcData[x+2]) ^ b;
                    }

                    QImage image(data.get(), mincx, 1, srcBitmap.format());
                    if (image.depth() != this->_info.bpp) {
                        image = image.convertToFormat(this->_imageFormatRGB);
                    }
                    QRect trect(drect.x, rowYCoord, mincx, 1);
                    this->_screen[0]->paintCache().drawImage(trect, image);
                    rowYCoord--;
                }
            }
        break;

        default: std::cout << "RDPMem3Blt (" << std::hex << int(cmd.rop) << ")" << std::endl;
        break;
    }
}


void Front_Qt::draw(const RDPDestBlt & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    //std::cout << "RDPDestBlt " <<  int(cmd.rop) <<  std::endl;

    const Rect drect = clip.intersect(this->_info.width * this->_monitorCount, this->_info.height).intersect(cmd.rect);

    switch (cmd.rop) {
        case 0x00: // blackness
            this->_screen[0]->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
            break;
        case 0x55: // inversion
            this->draw_RDPScrBlt(drect.x, drect.y, drect, true);
            break;
        case 0xAA: // change nothing
            break;
        case 0xFF: // whiteness
            this->_screen[0]->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::white);
            break;
        default: std::cout << "RDPDestBlt (" << std::hex << static_cast<int>(cmd.rop) << ")" << std::endl;
            break;
    }
}

void Front_Qt::draw(const RDPMultiDstBlt & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "RDPMultiDstBlt" << std::endl;
}

void Front_Qt::draw(const RDPMultiOpaqueRect & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "RDPMultiOpaqueRect" << std::endl;
}

void Front_Qt::draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "RDPMultiPatBlt" << std::endl;
}

void Front_Qt::draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "RDPMultiScrBlt" << std::endl;
}

void Front_Qt::draw(const RDPGlyphIndex & cmd, const Rect & clip, const GlyphCache & gly_cache) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    Rect screen_rect = clip.intersect(this->_info.width, this->_info.height);
    if (screen_rect.isempty()){
        return ;
    }

    Rect const clipped_glyph_fragment_rect = cmd.bk.intersect(screen_rect);
    if (clipped_glyph_fragment_rect.isempty()) {
        return;
    }
    //std::cout << "RDPGlyphIndex " << std::endl;

    // set a background color
    /*{
        Rect ajusted = cmd.f_op_redundant ? cmd.bk : cmd.op;
        if ((ajusted.cx > 1) && (ajusted.cy > 1)) {
            ajusted.cy--;
            ajusted.intersect(screen_rect);
            this->_screen[0]->paintCache().fillRect(ajusted.x, ajusted.y, ajusted.cx, ajusted.cy, this->u32_to_qcolor(cmd.fore_color));
        }
    } */

    bool has_delta_bytes = (!cmd.ui_charinc && !(cmd.fl_accel & 0x20));

    const QColor color = this->u32_to_qcolor(cmd.back_color);
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
                REDASSERT(fc);
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
                                this->_screen[0]->paintCache().fillRect(x + fc.offset + xx, y + fc.baseline + yy, 1, 1, color);
                            }
                            fc_bit_mask >>= 1;
                        }
                        fc_data++;
                    }
                }
            }
        } else {
            std::cout << "RDPGlyphIndex " << gly_cache.glyphs << std::endl;
        }
    }
    //this->draw_VariableBytes(cmd.data, cmd.data_len, has_delta_bytes,
        //draw_pos, offset_y, color, cmd.bk.x + offset_x, cmd.bk.y,
        //clipped_glyph_fragment_rect, cmd.cache_id, gly_cache);
}

void Front_Qt::draw(const RDPPolygonSC & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "RDPPolygonSC" << std::endl;

    /*RDPPolygonSC new_cmd24 = cmd;
    new_cmd24.BrushColor  = color_decode_opaquerect(cmd.BrushColor,  this->mod_bpp, this->mod_palette);*/
    //this->gd.draw(new_cmd24, clip);
}

void Front_Qt::draw(const RDPPolygonCB & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "RDPPolygonCB" << std::endl;

    /*RDPPolygonCB new_cmd24 = cmd;
    new_cmd24.foreColor  = color_decode_opaquerect(cmd.foreColor,  this->mod_bpp, this->mod_palette);
    new_cmd24.backColor  = color_decode_opaquerect(cmd.backColor,  this->mod_bpp, this->mod_palette);*/
    //this->gd.draw(new_cmd24, clip);
}

void Front_Qt::draw(const RDPPolyline & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "RDPPolyline" << std::endl;

    /*RDPPolyline new_cmd24 = cmd;
    new_cmd24.PenColor  = color_decode_opaquerect(cmd.PenColor,  this->mod_bpp, this->mod_palette);*/
    //this->gd.draw(new_cmd24, clip);
}

void Front_Qt::draw(const RDPEllipseSC & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "RDPEllipseSC" << std::endl;

    /*RDPEllipseSC new_cmd24 = cmd;
    new_cmd24.color = color_decode_opaquerect(cmd.color, this->mod_bpp, this->mod_palette);*/
    //this->gd.draw(new_cmd24, clip);
}

void Front_Qt::draw(const RDPEllipseCB & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "RDPEllipseCB" << std::endl;
/*
    RDPEllipseCB new_cmd24 = cmd;
    new_cmd24.fore_color = color_decode_opaquerect(cmd.fore_color, this->mod_bpp, this->mod_palette);
    new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);*/
    //this->gd.draw(new_cmd24, clip);
}

void Front_Qt::draw(const RDP::FrameMarker & order) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        order.log(LOG_INFO);
        LOG(LOG_INFO, "========================================\n");
    }

    //std::cout << "FrameMarker" << std::endl;
    //this->gd.draw(order);
}

void Front_Qt::draw(const RDP::RAIL::NewOrExistingWindow & order) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        order.log(LOG_INFO);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "NewOrExistingWindow" << std::endl;
    //this->gd.draw(order);
}

void Front_Qt::draw(const RDP::RAIL::WindowIcon & order) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        order.log(LOG_INFO);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "WindowIcon" << std::endl;
    //this->gd.draw(order);
}

void Front_Qt::draw(const RDP::RAIL::CachedIcon & order) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        order.log(LOG_INFO);
        LOG(LOG_INFO, "========================================\n");
    }

        std::cout << "CachedIcon" << std::endl;

    //this->gd.draw(order);
}

void Front_Qt::draw(const RDP::RAIL::DeletedWindow & order) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        order.log(LOG_INFO);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "DeletedWindow" << std::endl;

    //this->gd.draw(order);
}

void Front_Qt::draw(const RDP::RAIL::NewOrExistingNotificationIcons & order) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        order.log(LOG_INFO);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "NewOrExistingNotificationIcons" << std::endl;
    //this->gd.draw(order);
}

void Front_Qt::draw(const RDP::RAIL::DeletedNotificationIcons & order) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        order.log(LOG_INFO);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "DeletedNotificationIcons" << std::endl;
    //this->gd.draw(order);
}

void Front_Qt::draw(const RDP::RAIL::ActivelyMonitoredDesktop & order) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        order.log(LOG_INFO);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "ActivelyMonitoredDesktop" << std::endl;
    //this->gd.draw(order);
}

void Front_Qt::draw(const RDP::RAIL::NonMonitoredDesktop & order) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        order.log(LOG_INFO);
        LOG(LOG_INFO, "========================================\n");
    }

    std::cout << "NonMonitoredDesktop" << std::endl;
    //this->gd.draw(order);
}

void Front_Qt::draw(const RDPColCache   & cmd) {
    std::cout <<  "RDPColCache " << cmd.cacheIndex << std::endl;
}

void Front_Qt::draw(const RDPBrushCache & ) {
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//------------------------------
// Serveur non drawing exchange
//------------------------------

int Front_Qt::server_resize(int width, int height, int bpp) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        LOG(LOG_INFO, "server_resize(width=%d, height=%d, bpp=%d", width, height, bpp);
        LOG(LOG_INFO, "========================================\n");
    }
    //std::cout << "resize serveur" << std::endl;
    this->mod_bpp = bpp;
    //this->_info.bpp = bpp;
    this->_info.width = width;
    this->_info.height = height;

    //this->_screen[0]->setUpdate();

    return 1;
}

void Front_Qt::update_pointer_position(uint16_t xPos, uint16_t yPos) {
    std::cout << "update_pointer_position " << int(xPos) << " " << int(yPos) << std::endl;
}

const CHANNELS::ChannelDefArray & Front_Qt::get_channel_list(void) const {
    return this->_cl;
}

void Front_Qt::send_to_channel( const CHANNELS::ChannelDef & channel, uint8_t const * data, size_t , size_t chunk_size, int flags) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        LOG(LOG_INFO, "send_to_channel");
        LOG(LOG_INFO, "========================================\n");
    }

    const CHANNELS::ChannelDef * mod_channel = this->_cl.get_by_name(channel.name);
    if (!mod_channel) {
        return;
    }

    if (!strcmp(channel.name, channel_names::cliprdr)) {
        std::unique_ptr<AsynchronousTask> out_asynchronous_task;

        std::cout << std::dec;

        InStream chunk(data, chunk_size);

        InStream chunk_series = chunk.clone();

        if (!chunk.in_check_rem(2  /*msgType(2)*/ )) {
            LOG(LOG_ERR,
                "ClipboardVirtualChannel::process_client_message: "
                    "Truncated msgType, need=2 remains=%zu",
                chunk.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        const uint16_t server_message_type = chunk.in_uint16_le();

       if (!this->_waiting_for_data) {
            switch (server_message_type) {
                case RDPECLIP::CB_CLIP_CAPS:
                    if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                        LOG(LOG_INFO,
                            "ClipboardVirtualChannel::process_client_message: "
                                "Clipboard Capabilities PDU");
                    }
                    std::cout << "server >> Clipboard Capabilities PDU" << std::endl;

                break;

                case RDPECLIP::CB_MONITOR_READY:
                    if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                        LOG(LOG_INFO,
                            "ClipboardVirtualChannel::process_server_message: "
                                "Monitor Ready PDU");
                    }
                    std::cout << "server >> Monitor Ready PDU" << std::endl;

                    {
                        RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu(1, RDPECLIP::GeneralCapabilitySet::size());
                        RDPECLIP::GeneralCapabilitySet general_cap_set(RDPECLIP::CB_CAPS_VERSION_2, RDPECLIP::CB_STREAM_FILECLIP_ENABLED | RDPECLIP::CB_USE_LONG_FORMAT_NAMES | RDPECLIP::CB_FILECLIP_NO_FILE_PATHS);
                        StaticOutStream<1024> out_stream;
                        clipboard_caps_pdu.emit(out_stream);
                        general_cap_set.emit(out_stream);

                        const uint32_t total_length = out_stream.get_offset();
                        InStream chunk(out_stream.get_data(), total_length);

                        this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                            , chunk
                                                            , total_length
                                                            , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST
                                                              |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                            );
                        std::cout << "client >> Clipboard Capabilities PDU" << std::endl;

                        this->_monitorCount = this->_info.cs_monitor.monitorCount;

                        std::cout << "cs_monitor count negociated. MonitorCount=" << this->_monitorCount << std::endl;
                        std::cout << "width=" <<  this->_info.width <<  " " << "height=" << this->_info.height <<  std::endl;

                        /*this->_info.width  = (this->_width * this->_monitorCount);

                        if (!this->_monitorCountNegociated) {
                            for (int i = this->_monitorCount - 1; i >= 1; i--) {
                                this->_screen[i] = new Screen_Qt(this, i);
                                this->_screen[i]->show();
                            }
                            this->_screen[0]->activateWindow();
                            this->_monitorCountNegociated = true;

                        }*/
                        this->_monitorCountNegociated = true;
                    }
                    {
                        this->send_FormatListPDU(this->_clipbrdFormatsList.IDs, this->_clipbrdFormatsList.names, ClipbrdFormatsList::CLIPBRD_FORMAT_COUNT, true);

                    }

                break;

                case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
                    if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                        LOG(LOG_INFO,
                            "ClipboardVirtualChannel::process_server_message: "
                                "Format List Response PDU");
                    }

                    std::cout << "server >> Format List Response PDU";
                    if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                        std::cout << " FAILED" <<  std::endl;
                    } else {
                        std::cout <<  std::endl;
                    }
                break;

                case RDPECLIP::CB_FORMAT_LIST:
                    if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                        LOG(LOG_INFO,
                            "ClipboardVirtualChannel::process_server_message: "
                                "Format List PDU");
                    }

                    {
                        std::cout << "server >> Format List PDU";
                        if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                            std::cout << " FAILED" <<  std::endl;
                        } else {
                            std::cout <<  std::endl;

                            int formatAvailable = chunk.in_uint32_le();

                            bool isSharedFormat = false;

                            std::cout << " Format ID nb = " << formatAvailable << ", Requested Format = \"" << this->_requestedFormatName << "\"" << std::endl;

                            while (formatAvailable > 0) {
                                uint32_t formatID = chunk.in_uint32_le();
                                formatAvailable -=  4;

                                uint16_t utf16_string[120];
                                int k(0);
                                bool isEndString = false;
                                while (!isEndString) {
                                    u_int16_t bit(chunk.in_uint16_le());
                                    if (bit == 0) {
                                        isEndString = true;
                                    }
                                    utf16_string[k] = bit;
                                    k++;
                                    formatAvailable -=  2;
                                }
                                this->_requestedFormatName = std::string(reinterpret_cast<const char*>(utf16_string), k*2);

                                for (int j = 0; j < ClipbrdFormatsList::CLIPBRD_FORMAT_COUNT && !isSharedFormat; j++) {
                                    if (this->_clipbrdFormatsList.IDs[j] == formatID) {
                                        this->_requestedFormatId = formatID;
                                        isSharedFormat = true;
                                        std::cout <<  " pick!";
                                        formatAvailable = 0;
                                    }
                                }

                                if (this->_requestedFormatName == this->_clipbrdFormatsList.FILEGROUPDESCRIPTORW && !isSharedFormat) {
                                    this->_requestedFormatId = formatID;
                                    isSharedFormat = true;
                                    std::cout <<  " pick!";
                                    formatAvailable = 0;
                                }

                                std::cout << std::endl;
                            }

                            RDPECLIP::FormatListResponsePDU formatListResponsePDU(true);
                            StaticOutStream<256> out_stream;
                            formatListResponsePDU.emit(out_stream);
                            InStream chunk(out_stream.get_data(), out_stream.get_offset());

                            this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                                , chunk
                                                                , out_stream.get_offset()
                                                                , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                  CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                );
                            std::cout << "client >> Format List Response PDU" << std::endl;


                            RDPECLIP::LockClipboardDataPDU lockClipboardDataPDU;
                            StaticOutStream<32> out_stream_lock;
                            lockClipboardDataPDU.emit(out_stream_lock, 0);
                            InStream chunk_lock(out_stream_lock.get_data(), out_stream_lock.get_offset());

                            this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                                , chunk_lock
                                                                , out_stream_lock.get_offset()
                                                                , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                  CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                );
                            std::cout << "client >> Lock Clipboard Data PDU" << std::endl;


                            RDPECLIP::FormatDataRequestPDU formatDataRequestPDU(this->_requestedFormatId);
                            StaticOutStream<256> out_streamRequest;
                            formatDataRequestPDU.emit(out_streamRequest);
                            InStream chunkRequest(out_streamRequest.get_data(), out_streamRequest.get_offset());

                            this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                                , chunkRequest
                                                                , out_streamRequest.get_offset()
                                                                , CHANNELS::CHANNEL_FLAG_LAST  |
                                                                  CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                );
                            std::cout << "client >> Format Data Request PDU" << std::endl;
                        }
                    }
                break;

                case RDPECLIP::CB_LOCK_CLIPDATA:
                    std::cout << "server >> Lock Clipboard Data PDU" << std::endl;
                break;

                case RDPECLIP::CB_UNLOCK_CLIPDATA:
                    std::cout << "server >> Unlock Clipboard Data PDU" << std::endl;
                break;

                case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                    if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                        LOG(LOG_INFO,
                            "ClipboardVirtualChannel::process_server_message: "
                                "Format Data Response PDU");
                    }

                    if(this->_requestedFormatName == this->_clipbrdFormatsList.FILEGROUPDESCRIPTORW) {
                        this->_requestedFormatId = ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW;
                    }

                    if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                        std::cout << "server >> Format Data Response PDU";
                        if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                            std::cout << " FAILED" <<  std::endl;
                        } else {
                            std::cout <<  std::endl;
                            this->process_server_clipboard_indata(flags, chunk);
                        }
                    }
                break;

                case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                {

                    if (this->verbose & MODRDP_LOGLEVEL_CLIPRDR) {
                        LOG(LOG_INFO,
                            "ClipboardVirtualChannel::process_server_message: "
                                "Format Data Request PDU");
                    }

                    std::cout << "server >> Format Data Request PDU";
                    if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                        std::cout << " FAILED" <<  std::endl;
                    } else {
                        std::cout <<  std::endl;

                        chunk.in_skip_bytes(4);
                        int first_part_data_size(0);
                        uint32_t total_length(this->_clipboard_qt->_cliboard_data_length + PDU_HEADER_SIZE);
                        StaticOutStream<PDU_MAX_SIZE> out_stream_first_part;
                        RDPECLIP::FormatDataResponsePDU formatDataResponsePDU(true);

                        if (this->_clipboard_qt->_bufferTypeID == chunk.in_uint32_le()) {



                            std::cout << "client >> Format Data Response PDU" << std::endl;

                            switch(this->_clipboard_qt->_bufferTypeID) {

                                case RDPECLIP::CF_METAFILEPICT:
                                {
                                    first_part_data_size = this->_clipboard_qt->_cliboard_data_length;
                                    if (first_part_data_size > PASTE_PIC_CONTENT_SIZE) {
                                        first_part_data_size = PASTE_PIC_CONTENT_SIZE;
                                    }
                                    total_length += RDPECLIP::METAFILE_HEADERS_SIZE;

                                    formatDataResponsePDU.emit_metaFilePic( out_stream_first_part
                                                                          , this->_clipboard_qt->_cliboard_data_length
                                                                          , this->_clipboard_qt->_bufferImage->width()
                                                                          , this->_clipboard_qt->_bufferImage->height()
                                                                          , this->_clipboard_qt->_bufferImage->depth()
                                                                          , this->_clipbrdFormatsList.ARBITRARY_SCALE
                                                                          );

                                    this->process_client_clipboard_outdata( total_length
                                                                          , out_stream_first_part
                                                                          , first_part_data_size
                                                                          , this->_clipboard_qt->_bufferImage->bits()
                                                                          );
                                }
                                break;

                                case RDPECLIP::CF_TEXT:
                                case RDPECLIP::CF_UNICODETEXT:
                                {
                                    first_part_data_size = this->_clipboard_qt->_cliboard_data_length;
                                    if (first_part_data_size > PASTE_TEXT_CONTENT_SIZE) {
                                        first_part_data_size = PASTE_TEXT_CONTENT_SIZE;
                                    }
                                    total_length += 2;
                                    formatDataResponsePDU.emit_text( out_stream_first_part
                                                                   , this->_clipboard_qt->_cliboard_data_length
                                                                   );

                                    this->process_client_clipboard_outdata( total_length
                                                                          , out_stream_first_part
                                                                          , first_part_data_size
                                                                          , this->_clipboard_qt->_chunk.get()
                                                                          );
                                }
                                break;

                                case ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW:
                                {
                                    int data_sent(0);
                                    first_part_data_size = PDU_HEADER_SIZE + 4;
                                    total_length = (RDPECLIP::FileDescriptor::size() * this->_clipboard_qt->_cItems) + 8 + PDU_HEADER_SIZE;
                                    int flag_first(CHANNELS::CHANNEL_FLAG_FIRST |CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);
                                    ClipBoard_Qt::CB_out_File * file = this->_clipboard_qt->_items_list[0];
                                    formatDataResponsePDU.emit_fileList( out_stream_first_part
                                                                       , this->_clipboard_qt->_cItems
                                                                       , file->name
                                                                       , file->size
                                                                       );

                                    if (this->_clipboard_qt->_cItems == 1) {
                                        flag_first = flag_first | CHANNELS::CHANNEL_FLAG_LAST;
                                        out_stream_first_part.out_uint32_le(0);
                                        data_sent += 4;
                                    }
                                    InStream chunk_first_part( out_stream_first_part.get_data()
                                                             , out_stream_first_part.get_offset()
                                                             );

                                    this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                                        , chunk_first_part
                                                                        , total_length
                                                                        , flag_first
                                                                        );
                                    data_sent += first_part_data_size + RDPECLIP::FileDescriptor::size();

                                    std::cout << "client >> Data PDU  " << data_sent << " / " << total_length << std::endl;

                                    RDPECLIP::FileDescriptor fd;
                                    for (int i = 1; i < this->_clipboard_qt->_cItems; i++) {

                                        StaticOutStream<PDU_MAX_SIZE> out_stream_next_part;
                                        file = this->_clipboard_qt->_items_list[i];
                                        int flag_next(CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL);

                                        fd.flags          = RDPECLIP::FD_SHOWPROGRESSUI |
                                                            RDPECLIP::FD_FILESIZE       |
                                                            RDPECLIP::FD_WRITESTIME     |
                                                            RDPECLIP::FD_ATTRIBUTES;
                                        fd.fileAttributes = RDPECLIP::FILE_ATTRIBUTES_ARCHIVE;
                                        fd.lastWriteTime  = RDPECLIP::TIME64_FILE_LIST;
                                        fd.fileSizeHigh   = file->size >> 32;
                                        fd.fileSizeLow    = file->size;

                                        fd.file_name      = file->nameUTF8;
                                        fd.emit(out_stream_next_part);

                                        if (i == this->_clipboard_qt->_cItems - 1) {
                                            flag_next = flag_next | CHANNELS::CHANNEL_FLAG_LAST;
                                            out_stream_next_part.out_uint32_le(0);
                                            data_sent += 4;
                                        }

                                        InStream chunk_next_part( out_stream_next_part.get_data()
                                                                , out_stream_next_part.get_offset()
                                                                );

                                        this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                                            , chunk_next_part
                                                                            , total_length
                                                                            , flag_next
                                                                            );
                                        data_sent += RDPECLIP::FileDescriptor::size();
                                        std::cout << "client >> Data PDU  " << data_sent << " / " << total_length << std::endl;
                                    }
                                }
                                break;

                                default:
                                    std::cout <<  "unknow buffer format ID " << int(this->_clipboard_qt->_bufferTypeID) << std::endl;
                                break;
                            }
                        }
                    }
                }
                break;

                case RDPECLIP::CB_FILECONTENTS_REQUEST:
                    std::cout << "server >> File Contents Resquest PDU";
                    if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                        std::cout << " FAILED" <<  std::endl;
                    } else {

                        chunk.in_skip_bytes(4);                 // data_len
                        int streamID(chunk.in_uint32_le());
                        int lindex(chunk.in_uint32_le());

                        RDPECLIP::FileContentsResponse fileContentsResponse(true);

                        switch (chunk.in_uint32_le()) {         // flag

                            case RDPECLIP::FILECONTENTS_SIZE :
                            {
                                std::cout << " SIZE streamID=" << streamID << " lindex=" << lindex <<  std::endl;
                                StaticOutStream<32> out_stream;

                                fileContentsResponse.emit_size( out_stream
                                                              , streamID
                                                              , this->_clipboard_qt->_items_list[lindex]->size
                                                              );

                                InStream chunk_to_send(out_stream.get_data(), out_stream.get_offset());
                                this->_callback->send_to_mod_channel(channel_names::cliprdr
                                                                    , chunk_to_send
                                                                    , out_stream.get_offset()
                                                                    , CHANNELS::CHANNEL_FLAG_LAST |
                                                                      CHANNELS::CHANNEL_FLAG_FIRST |  CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                                    );

                                std::cout << "client >> File Contents Response PDU SIZE streamID=" << streamID << " lindex=" << lindex << std::endl;
                            }
                            break;

                            case RDPECLIP::FILECONTENTS_RANGE :
                            {
                                std::cout << " RANGE streamID=" << streamID << " lindex=" << lindex <<  std::endl;
                                StaticOutStream<PDU_MAX_SIZE> out_stream_first_part;
                                this->_clipboard_qt->_cliboard_data_length = this->_clipboard_qt->_items_list[lindex]->size;
                                int total_length(this->_clipboard_qt->_items_list[lindex]->size + 12);
                                int first_part_data_size(this->_clipboard_qt->_items_list[lindex]->size);
                                first_part_data_size = this->_clipboard_qt->_items_list[lindex]->size;
                                if (first_part_data_size > PDU_MAX_SIZE - 12) {
                                    first_part_data_size = PDU_MAX_SIZE - 12;
                                }
                                fileContentsResponse.emit_range( out_stream_first_part
                                                               , streamID
                                                               , this->_clipboard_qt->_items_list[lindex]->size
                                                               );

                                std::cout << "client >> File Contents Response PDU RANGE streamID=" << streamID << " lindex=" << lindex << std::endl;

                                this->process_client_clipboard_outdata( total_length
                                                                      , out_stream_first_part
                                                                      , first_part_data_size
                                                                      , reinterpret_cast<uint8_t *>(
                                                                        this->_clipboard_qt->_items_list[lindex]->chunk)
                                                                      );
                            }
                            break;
                        }
                    }
                break;

                case RDPECLIP::CB_FILECONTENTS_RESPONSE:
                    if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                        std::cout << "server >> File Contents Response PDU";
                        if (chunk.in_uint16_le() == RDPECLIP::CB_RESPONSE_FAIL) {
                            std::cout << " FAILED" <<  std::endl;
                        } else {
                            std::cout <<  std::endl;

                            if(this->_requestedFormatName == this->_clipbrdFormatsList.FILEGROUPDESCRIPTORW) {
                                this->_requestedFormatId = ClipbrdFormatsList::CF_QT_CLIENT_FILECONTENTS;
                                this->process_server_clipboard_indata(flags, chunk);
                            }
                        }
                    }
                break;

                default:
                    std::cout << "process sever next part PDU data" <<  std::endl;
                    this->process_server_clipboard_indata(flags, chunk_series);

                break;
            }

        } else {
            std::cout << "process sever next part PDU data" <<  std::endl;
            this->process_server_clipboard_indata(flags, chunk_series);
        }
    }
}


void Front_Qt::show_out_stream(int flags, OutStream & chunk, size_t length) {
    uint8_t * data = chunk.get_data();

    std::cout <<  std::hex << "flag=0x" << flags << " total_length=" << std::dec << int(length) <<  std::hex <<  std::endl;
    std::cout << "\"";
    for (size_t i = 0; i < length; i++) {
        int byte(data[i]);
        if ((i % 16) == 0 && i != 0) {
            std::cout << "\"" << std::endl << "\"";
        }

        std::cout << "\\x";
        if (byte < 0x10) {
            std::cout << "0";
        }
        std::cout  <<  byte;
    }
    std::cout << "\"" << std::dec << std::endl;
}

void Front_Qt::show_in_stream(int flags, InStream & chunk, size_t length) {
    std::cout <<  std::hex << "flag=0x" << flags << " total_length=" << std::dec << int(length) <<  std::hex <<  std::endl;
    std::cout << "\"";
    for (size_t i = 0; i < length; i++) {
        int byte(chunk.in_uint8());
        if ((i % 16) == 0 && i != 0) {
            std::cout << "\"" << std::endl << "\"";
        }

        std::cout << "\\x";
        if (byte < 0x10) {
            std::cout << "0";
        }
        std::cout  <<  byte;
    }
    std::cout << "\"" << std::dec << std::endl;
}

void Front_Qt::process_server_clipboard_indata(int flags, InStream & chunk) {

    // 3.1.5.2.2 Processing of Virtual Channel PDU

    // The Virtual Channel PDU is received by both the client and the server. Its structure
    // and fields are specified in section 2.2.6.1.

    // If Enhanced RDP Security (section 5.4) is in effect, the External Security Protocol (section 5.4.5)
    // being used to secure the connection MUST be used to decrypt and verify the integrity of the entire
    // PDU prior to any processing taking place.

    // The embedded length fields within the tpktHeader ([T123] section 8) and mcsPdu ([T125] section 7, parts
    // 7 and 10) fields MUST be examined for consistency with the received data. If there is any discrepancy,
    // the connection SHOULD be dropped.

    // The mcsPdu field encapsulates either an MCS Send Data Request PDU (if the PDU is being sent from client
    // to server) or an MCS Send Data Indication PDU (if the PDU is being sent from server to client). In both
    // of these cases, the embedded channelId field MUST contain the server-assigned virtual channel ID. This
    // ID MUST be used to route the data in the virtualChannelData field to the appropriate virtual channel
    // endpoint after decryption of the PDU and any necessary decompression of the payload has been conducted.

    // The conditions mandating the presence of the securityHeader field, as well as the type of Security
    // Header structure present in this field, are explained in section 2.2.6.1. If the securityHeader field is
    // present, the embedded flags field MUST be examined for the presence of the SEC_ENCRYPT (0x0008) flag
    // (section 2.2.8.1.1.2.1), and, if it is present, the data following the securityHeader field MUST be
    // verified and decrypted using the methods and techniques specified in section 5.3.6. If the MAC signature
    // is incorrect, or the data cannot be decrypted correctly, the connection SHOULD be dropped.

    // If the data in the virtualChannelData field is compressed, then the data MUST be decompressed using
    // the techniques detailed in section 3.1.8.3 (the Virtual Channel PDU compression flags are specified
    // in section 2.2.6.1.1).

    // If the embedded flags field of the channelPduHeader field (the Channel PDU Header structure is specified
    // in section 2.2.6.1.1) does not contain the CHANNEL_FLAG_FIRST (0x00000001) flag or CHANNEL_FLAG_LAST
    // (0x00000002) flag, and the data is not part of a chunked sequence (that is, a start chunk has not been
    // received), then the data in the virtualChannelData field can be dispatched to the appropriate virtual
    // channel endpoint (no reassembly is required by the endpoint). If the CHANNEL_FLAG_SHOW_PROTOCOL
    // (0x00000010) flag is specified in the Channel PDU Header, then the channelPduHeader field MUST also
    // be dispatched to the virtual channel endpoint.

    // If the virtual channel data is part of a sequence of chunks, then the instructions in section 3.1.5.2.2.1
    //MUST be followed to reassemble the stream.

    if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
        this->_waiting_for_data = true;
        this->_bufferRDPClipboardChannelSizeTotal = chunk.in_uint32_le();
        this->_bufferRDPClipboardChannel = std::make_unique<uint8_t[]>(this->_bufferRDPClipboardChannelSizeTotal);
    }

    switch (this->_requestedFormatId) {

        case RDPECLIP::CF_UNICODETEXT:
        case RDPECLIP::CF_TEXT:
            this->send_to_clipboard_Buffer(chunk);
            if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                this->send_textBuffer_to_clipboard();
            }
        break;

        case RDPECLIP::CF_METAFILEPICT:

            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {

                RDPECLIP::MetaFilePicDescriptor mfpd;
                mfpd.receive(chunk);

                this->_bufferRDPCLipboardMetaFilePic_height = mfpd.height;
                this->_bufferRDPCLipboardMetaFilePic_width  = mfpd.width;
                this->_bufferRDPClipboardMetaFilePicBPP     = mfpd.bpp;
                this->_bufferRDPClipboardChannelSizeTotal   = mfpd.imageSize;
                this->_bufferRDPClipboardChannel = std::make_unique<uint8_t[]>(this->_bufferRDPClipboardChannelSizeTotal);
            }

            this->send_to_clipboard_Buffer(chunk);

            if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                this->send_imageBuffer_to_clipboard();
            }
        break;

        case ClipbrdFormatsList::CF_QT_CLIENT_FILEGROUPDESCRIPTORW:

            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                this->_bufferRDPClipboardChannelSizeTotal -= 4;
                this->_cItems = chunk.in_uint32_le();
                this->_lindexToRequest = 0;
                this->_clipboard_qt->emptyBuffer();
                this->_items_list.clear();
            }

            this->send_to_clipboard_Buffer(chunk);

            if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                InStream stream(this->_bufferRDPClipboardChannel.get(), this->_bufferRDPClipboardChannelSizeTotal);

                RDPECLIP::FileDescriptor fd;

                for (int i = 0; i < this->_cItems; i++) {
                    fd.receive(stream);
                    CB_in_Files file;
                    file.size = fd.file_size();
                    file.name = fd.fileName();
                    this->_items_list.push_back(file);
                }

                RDPECLIP::FileContentsRequestPDU fileContentsRequest(true);
                StaticOutStream<32> out_streamRequest;
                fileContentsRequest.emit(out_streamRequest
                                        , this->_streamIDToRequest
                                        , RDPECLIP::FILECONTENTS_RANGE
                                        , this->_lindexToRequest
                                        , this->_items_list[this->_lindexToRequest].size
                                        );
                const uint32_t total_length_FormatDataRequestPDU = out_streamRequest.get_offset();

                InStream chunkRequest(out_streamRequest.get_data(), total_length_FormatDataRequestPDU);

                this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                    , chunkRequest
                                                    , total_length_FormatDataRequestPDU
                                                    , CHANNELS::CHANNEL_FLAG_LAST  |
                                                      CHANNELS::CHANNEL_FLAG_FIRST |
                                                      CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                    );
                std::cout << "client >> File Contents Resquest PDU FILECONTENTS_RANGE streamID=" << this->_streamIDToRequest << " lindex=" << this->_lindexToRequest << std::endl;

                this->empty_buffer();
            }
        break;

        case ClipbrdFormatsList::CF_QT_CLIENT_FILECONTENTS:

            if (flags & CHANNELS::CHANNEL_FLAG_FIRST) {
                this->_bufferRDPClipboardChannelSizeTotal = this->_items_list[this->_lindexToRequest].size;
                this->_streamIDToRequest = chunk.in_uint32_le();
                this->_bufferRDPClipboardChannel = std::make_unique<uint8_t[]>(this->_items_list[this->_lindexToRequest].size);
            }

            this->send_to_clipboard_Buffer(chunk);

            if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                this->_waiting_for_data = false;

                 this->_clipboard_qt->write_clipboard_temp_file( this->_items_list[this->_lindexToRequest].name
                                                               , this->_bufferRDPClipboardChannel.get()
                                                               , this->_items_list[this->_lindexToRequest].size
                                                               );
                this->_lindexToRequest++;

                if (this->_lindexToRequest >= this->_cItems) {

                    this->_clipboard_qt->_local_clipboard_stream = false;
                    this->_clipboard_qt->setClipboard_files(this->_items_list);
                    this->_clipboard_qt->_local_clipboard_stream = true;

                    RDPECLIP::UnlockClipboardDataPDU unlockClipboardDataPDU;
                    StaticOutStream<32> out_stream_unlock;
                    unlockClipboardDataPDU.emit(out_stream_unlock, 0);
                    InStream chunk_unlock(out_stream_unlock.get_data(), out_stream_unlock.get_offset());

                    this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                        , chunk_unlock
                                                        , out_stream_unlock.get_offset()
                                                        , CHANNELS::CHANNEL_FLAG_LAST  | CHANNELS::CHANNEL_FLAG_FIRST |
                                                          CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                        );
                    std::cout << "client >> Unlock Clipboard Data PDU" << std::endl;

                } else {
                    this->_streamIDToRequest++;
                    RDPECLIP::FileContentsRequestPDU fileContentsRequest(true);
                    StaticOutStream<32> out_streamRequest;
                    fileContentsRequest.emit(out_streamRequest, this->_streamIDToRequest, RDPECLIP::FILECONTENTS_RANGE, this->_lindexToRequest, this->_items_list[this->_lindexToRequest].size);
                    const uint32_t total_length_FormatDataRequestPDU = out_streamRequest.get_offset();

                    InStream chunkRequest(out_streamRequest.get_data(), total_length_FormatDataRequestPDU);

                    this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                        , chunkRequest
                                                        , total_length_FormatDataRequestPDU
                                                        , CHANNELS::CHANNEL_FLAG_LAST  |
                                                          CHANNELS::CHANNEL_FLAG_FIRST |
                                                          CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                        );

                    std::cout << "client >> File Contents Resquest PDU FILECONTENTS_RANGE streamID=" << this->_streamIDToRequest << " lindex=" << this->_lindexToRequest << std::endl;
                }

                this->empty_buffer();
            }
        break;

        default:
            if (strcmp(this->_requestedFormatName.c_str(), RDPECLIP::get_format_short_name(RDPECLIP::SF_TEXT_HTML)) == 0) {
                this->send_to_clipboard_Buffer(chunk);

                if (flags & CHANNELS::CHANNEL_FLAG_LAST) {
                    this->send_textBuffer_to_clipboard();
                }

            }  else {

                std::cout << " Format not recognized (" << static_cast<int>(this->_requestedFormatId) << ")" <<  std::endl;
            }

        break;
    }
}

void Front_Qt::send_to_clipboard_Buffer(InStream & chunk) {

    // 3.1.5.2.2.1 Reassembly of Chunked Virtual Channel Data

    const size_t length_of_data_to_dump(chunk.in_remain());
    const size_t sum_buffer_and_data(this->_bufferRDPClipboardChannelSize + length_of_data_to_dump);
    const uint8_t * utf8_data = chunk.get_current();

    for (size_t i = 0; i < length_of_data_to_dump && i + this->_bufferRDPClipboardChannelSize < this->_bufferRDPClipboardChannelSizeTotal; i++) {
        this->_bufferRDPClipboardChannel[i + this->_bufferRDPClipboardChannelSize] = utf8_data[i];
    }

    this->_bufferRDPClipboardChannelSize = sum_buffer_and_data;
}

void Front_Qt::send_imageBuffer_to_clipboard() {

    QImage image(this->_bufferRDPClipboardChannel.get(),
                 this->_bufferRDPCLipboardMetaFilePic_width,
                 this->_bufferRDPCLipboardMetaFilePic_height,
                 this->bpp_to_QFormat(this->_bufferRDPClipboardMetaFilePicBPP, false));

    QImage imageSwapped(image.rgbSwapped().mirrored(false, true));

    this->_clipboard_qt->_local_clipboard_stream = false;
    this->_clipboard_qt->setClipboard_image(imageSwapped);
    this->_clipboard_qt->_local_clipboard_stream = true;

    this->empty_buffer();
}

void Front_Qt::send_textBuffer_to_clipboard() {
    std::unique_ptr<uint8_t[]> utf8_string = std::make_unique<uint8_t[]>(this->_bufferRDPClipboardChannelSizeTotal);
    size_t length_of_utf8_string = ::UTF16toUTF8(
        this->_bufferRDPClipboardChannel.get(), this->_bufferRDPClipboardChannelSizeTotal,
        utf8_string.get(), this->_bufferRDPClipboardChannelSizeTotal);
    std::string str(reinterpret_cast<const char*>(utf8_string.get()), length_of_utf8_string);

    this->_clipboard_qt->_local_clipboard_stream = false;
    this->_clipboard_qt->setClipboard_text(str);
    this->_clipboard_qt->_local_clipboard_stream = true;

    this->empty_buffer();
}

void Front_Qt::empty_buffer() {
    this->_bufferRDPClipboardMetaFilePicBPP     = 0;
    this->_bufferRDPClipboardChannelSizeTotal   = 0;
    this->_bufferRDPCLipboardMetaFilePic_width  = 0;
    this->_bufferRDPCLipboardMetaFilePic_height = 0;
    this->_bufferRDPClipboardChannelSize        = 0;
    this->_waiting_for_data = false;
}

void Front_Qt::emptyLocalBuffer() {
    this->_clipboard_qt->emptyBuffer();
}

void Front_Qt::send_FormatListPDU(uint32_t const * formatIDs, std::string const * formatListDataShortName, std::size_t formatIDs_size, bool isLong) {

    RDPECLIP::FormatListPDU format_list_pdu;
    StaticOutStream<1024> out_stream;
    if (isLong) {
        format_list_pdu.emit_long(out_stream, formatIDs, formatListDataShortName, formatIDs_size);
    } else {
        format_list_pdu.emit_short(out_stream, formatIDs, formatListDataShortName, formatIDs_size);
    }
    const uint32_t total_length = out_stream.get_offset();
    InStream chunk(out_stream.get_data(), out_stream.get_offset());

    this->_callback->send_to_mod_channel( channel_names::cliprdr
                                        , chunk
                                        , total_length
                                        , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST |
                                          CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                        );

    std::cout << "client >> Format List PDU" << std::endl;
}


void Front_Qt::process_client_clipboard_outdata(uint64_t total_length, OutStream & out_stream_first_part, int first_part_data_size,  uint8_t const * data){

    // 3.1.5.2.2.1 Reassembly of Chunked Virtual Channel Dat

    // Virtual channel data can span multiple Virtual Channel PDUs (section 3.1.5.2.1).
    // If this is the case, the embedded length field of the channelPduHeader field
    // (the Channel PDU Header structure is specified in section 2.2.6.1.1) specifies
    // the total length of the uncompressed virtual channel data spanned across all of
    // the associated Virtual Channel PDUs. This length is referred to as totalLength.
    // For example, assume that the virtual channel chunking size specified in the Virtual
    // Channel Capability Set (section 2.2.7.1.10) is 1,000 bytes and that 2,062 bytes need
    // to be transmitted on a given virtual channel. In this example,
    // the following sequence of Virtual Channel PDUs will be sent (only relevant fields are listed):

    //    Virtual Channel PDU 1:
    //    CHANNEL_PDU_HEADER::length = 2062 bytes
    //    CHANNEL_PDU_HEADER::flags = CHANNEL_FLAG_FIRST
    //    Actual virtual channel data is 1000 bytes (the chunking size).

    //    Virtual Channel PDU 2:
    //    CHANNEL_PDU_HEADER::length = 2062 bytes
    //    CHANNEL_PDU_HEADER::flags = 0
    //    Actual virtual channel data is 1000 bytes (the chunking size).

    //    Virtual Channel PDU 3:
    //    CHANNEL_PDU_HEADER::length = 2062 bytes
    //    CHANNEL_PDU_HEADER::flags = CHANNEL_FLAG_LAST
    //    Actual virtual channel data is 62 bytes.

    // The size of the virtual channel data in the last PDU (the data in the virtualChannelData field)
    // is determined by subtracting the offset of the virtualChannelData field in the encapsulating
    // Virtual Channel PDU from the total size specified in the tpktHeader field. This length is
    // referred to as chunkLength.

    // Upon receiving each Virtual Channel PDU, the server MUST dispatch the virtual channel data to
    // the appropriate virtual channel endpoint. The sequencing of the chunk (whether it is first,
    // intermediate, or last), totalLength, chunkLength, and the virtualChannelData fields MUST
    // be dispatched to the virtual channel endpoint so that the data can be correctly reassembled.
    // If the CHANNEL_FLAG_SHOW_PROTOCOL (0x00000010) flag is specified in the Channel PDU Header,
    // then the channelPduHeader field MUST also be dispatched to the virtual channel endpoint.

    // A reassembly buffer MUST be created by the virtual channel endpoint using the size specified
    // by totalLength when the first chunk is received. After the reassembly buffer has been created
    // the first chunk MUST be copied into the front of the buffer. Subsequent chunks MUST then be
    // copied into the reassembly buffer in the order in which they are received. Upon receiving the
    // last chunk of virtual channel data, the reassembled data is processed by the virtual channel endpoint.

    if (total_length > PDU_MAX_SIZE) {

        const int cmpt_PDU_part(total_length / PDU_MAX_SIZE);
        const int remains_PDU  (total_length % PDU_MAX_SIZE);
        int data_sent(0);

        // First Part
            out_stream_first_part.out_copy_bytes(data, first_part_data_size);
            data_sent += first_part_data_size;
            InStream chunk_first(out_stream_first_part.get_data(), out_stream_first_part.get_offset());

            this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                , chunk_first
                                                , total_length
                                                , CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );

            std::cout << "client >> Data PDU " << data_sent << " / " << this->_clipboard_qt->_cliboard_data_length << std::endl;


        for (int i = 0; i < cmpt_PDU_part - 1; i++) {

        // Next Part
            StaticOutStream<PDU_MAX_SIZE> out_stream_next_part;
            out_stream_next_part.out_copy_bytes(data + data_sent, PDU_MAX_SIZE);
            data_sent += PDU_MAX_SIZE;
            InStream chunk_next(out_stream_next_part.get_data(), out_stream_next_part.get_offset());

            this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                , chunk_next
                                                , total_length
                                                , CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );

            std::cout << "client >> Data PDU " << data_sent << " / " << this->_clipboard_qt->_cliboard_data_length << std::endl;

        }

        // Last part
            StaticOutStream<PDU_MAX_SIZE> out_stream_last_part;
            out_stream_last_part.out_copy_bytes(data + data_sent, remains_PDU);
            if (this->_clipboard_qt->_bufferTypeID == RDPECLIP::CF_METAFILEPICT) {
                out_stream_last_part.out_uint32_le(3);
                out_stream_last_part.out_uint16_le(0);
            }
            data_sent += remains_PDU;
            InStream chunk_last(out_stream_last_part.get_data(), out_stream_last_part.get_offset());

            this->_callback->send_to_mod_channel( channel_names::cliprdr
                                                , chunk_last
                                                , total_length
                                                , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                                );

            std::cout << "client >> Data PDU " << data_sent << " / " << this->_clipboard_qt->_cliboard_data_length << std::endl;


    } else {

        out_stream_first_part.out_copy_bytes(data, this->_clipboard_qt->_cliboard_data_length);
        if (this->_clipboard_qt->_bufferTypeID == RDPECLIP::CF_UNICODETEXT) {
            out_stream_first_part.out_uint16_le(0);
        }
        InStream chunk(out_stream_first_part.get_data(), out_stream_first_part.get_offset());

        this->_callback->send_to_mod_channel( channel_names::cliprdr
                                            , chunk
                                            , total_length
                                            , CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST |
                                              CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
                                            );

        std::cout << "client >> Data PDU  " << this->_clipboard_qt->_cliboard_data_length << " / " << this->_clipboard_qt->_cliboard_data_length << std::endl;
    }
}

void Front_Qt::begin_update() {
    //if (this->verbose > 10) {
    //    LOG(LOG_INFO, "--------- FRONT ------------------------");
    //    LOG(LOG_INFO, "begin_update");
    //    LOG(LOG_INFO, "========================================\n");
    //}
}

void Front_Qt::end_update() {
    //if (this->verbose > 10) {
    //    LOG(LOG_INFO, "--------- FRONT ------------------------");
    //    LOG(LOG_INFO, "end_update");
    //    LOG(LOG_INFO, "========================================\n");
    //}
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------
//    SOCKET EVENTS FUNCTIONS
//--------------------------------

void Front_Qt::call_Draw() {
    if (this->_callback != nullptr) {
        try {
            this->_callback->draw_event(time(nullptr), *(this));
        } catch (const Error & e) {
            this->dropScreen();
            const std::string errorMsg("Error: connexion to [" + this->_targetIP +  "] is closed.");
            std::cout << errorMsg << std::endl;
            std::string labelErrorMsg("<font color='Red'>"+errorMsg+"</font>");

            this->disconnect(labelErrorMsg);
        }
    }
}



///////////////////////////////
// APPLICATION
int main(int argc, char** argv){

    //" -name QA\\administrateur -pwd '' -ip 10.10.46.88 -p 3389";

    // sudo python ./sesman/sesmanlink/WABRDPAuthentifier

    // sudo nano /etc/rdpproxy/rdpproxy.ini

    // sudo bin/gcc-4.9.2/san/rdpproxy -nf

    //bjam -a client_rdp_Qt4 |& sed '/usr\/include\/qt4\|threading-multi\/src\/Qt4\/\|in expansion of macro .*Q_OBJECT\|Wzero/,/\^/d' && bin/gcc-4.9.2/release/threading-multi/client_rdp_Qt4 -n admin -pwd "$testmdp" -ip 10.10.40.22 -p 3389

    QApplication app(argc, argv);

    int verbose = MODRDP_LOGLEVEL_CLIPRDR;                               //0x04000000 | 0x40000000;

    Front_Qt front(argv, argc, verbose);


    app.exec();

    //  xfreerdp /u:x /p: /port:3389 /v:10.10.43.46 /multimon /monitors:2


}

