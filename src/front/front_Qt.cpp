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

#include "../src/front/front_widget_Qt.hpp"



Front_Qt::Front_Qt(const char* argv[] = {}, int argc = 0, uint32_t verbose = 0)
    : Front_Qt_API(false, false, verbose)
    , mod_bpp(24)
    , mod_palette(BGRPalette::classic_332())
    , _form(nullptr)
    , _screen(nullptr)
    , _connector(nullptr)  
    , _timer(0)    
    , _connected(false)
    , _keymap() 
    , _ctrl_alt_delete(false)
    , _qtRDPKeymap() 
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    this->_info.keylayout = 0x040C;// 0x40C FR, 0x409 USA
    this->_info.console_session = 0;
    this->_info.brush_cache_code = 0;
    this->_info.bpp = 24;
    this->_info.width = 800;
    this->_info.height = 600;
    this->_info.rdp5_performanceflags = PERF_DISABLE_WALLPAPER;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    char * localIPtmp = "unknow_local_IP"; //"10.10.43.46";
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
    this->_localIP       = localIPtmp;
    this->_nbTry         = 3;
    this->_retryDelay    = 1000;
    
    uint8_t commandIsValid(0);
    
    for (int i = 0; i <  argc - 1; i++) {

        std::string word(argv[i]);
        std::string arg(argv[i+1]);
        
        if (word == "-n") {
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
    
    if (this->mod_bpp == 8) {
        this->mod_palette = BGRPalette::classic_332();
    }
    this->_qtRDPKeymap.setKeyboardLayout(this->_info.keylayout);
    
    this->_qtRDPKeymap.setCustomASCIIcode(0x152, 0xB2); // squared
    this->_qtRDPKeymap.setCustomASCIIcode(0x39c, 0xB5); // µ
    this->_keymap.init_layout(this->_info.keylayout);


    // Winodws and socket contrainer
    this->_screen    = new Screen_Qt(this);
    this->_form      = new Form_Qt(this);
    this->_connector = new Connector_Qt(this, this->_form);

    if (commandIsValid == Front_Qt::COMMAND_VALID) {
        
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
        
        this->disconnect("");
    }
}


Front_Qt::~Front_Qt() {}


    
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

//------------------------
//      CONTROLLERS
//------------------------
    
void Front_Qt::disconnexionReleased(){
    this->_form->setCursor(Qt::WaitCursor);
    this->_screen->setCursor(Qt::WaitCursor);
    this->disconnect("");
}
    
    
void Front_Qt::closeFromScreen() {
    if (this->_form != nullptr && this->_connected) {
        this->_form->close();
    }
}


void Front_Qt::closeFromForm() {
    if (this->_screen != nullptr && !(this->_connected)) {
        this->_screen->close();
    }
}
    
    
void Front_Qt::connect() {
    this->updateForm(false);

    if (this->_connector->connect()) {

        this->_form->hide(); 
        this->_screen->setUpdate();
        this->_screen->show();

        this->_connector->listen();

        this->_connected = true;
    } 
    
    this->_form->setCursor(Qt::ArrowCursor);
    this->_screen->setCursor(Qt::ArrowCursor);
    this->updateForm(true);;
}


void Front_Qt::disconnect(std::string txt) {
    
    if (this->_connector != nullptr) {
        this->_connector->drop_connexion();
    }
    
    this->_screen->hide();
    
    this->_form->set_IPField(this->_targetIP);
    this->_form->set_portField(this->_port);
    this->_form->set_PWDField(this->_pwd);
    this->_form->set_userNameField(this->_userName);
    this->_form->set_ErrorMsg(txt);
    this->_form->show();
    
    this->_form->setCursor(Qt::ArrowCursor);
    this->_screen->setCursor(Qt::ArrowCursor);
    this->_connected = false;
}

    
void Front_Qt::connexionReleased(){
    this->_form->setCursor(Qt::WaitCursor);
    this->_screen->setCursor(Qt::WaitCursor);
    this->_userName =  this->_form->get_userNameField();
    this->_targetIP =  this->_form->get_IPField();
    this->_pwd      =  this->_form->get_PWDField();
    this->_port     =  this->_form->get_portField();
    this->connect();
}


void Front_Qt::updateForm(bool enable){
    this->_form->setEnable(enable);
    this->_form->update();
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

//---------------------------------------
//   GRAPHIC FUNCTIONS (factorization)
//---------------------------------------
    
void Front_Qt::draw_bmp(const Rect & drect, const Bitmap & bitmap, bool invert) {
    const int16_t mincx = std::min<int16_t>(bitmap.cx(), std::min<int16_t>(this->_info.width - drect.x, drect.cx));
    const int16_t mincy = 1;

    if (mincx <= 0) {
        return;
    }        

    int rowYCoord(drect.y + drect.cy-1);
    int rowsize(bitmap.line_size()); //Bpp

    const unsigned char * row = bitmap.data();

    QImage::Format format(this->bpp_to_QFormat(bitmap.bpp())); //bpp

    for (size_t k = 0 ; k < drect.cy; k++) {
        
        QImage qbitmap(row, mincx, mincy, format);
        if (invert) {
            qbitmap.invertPixels();
        }
        QRect trect(drect.x, rowYCoord, mincx, mincy);
        this->_screen->paintCache().drawImage(trect, qbitmap.rgbSwapped());

        row += rowsize;
        rowYCoord--;
    }
    this->_screen->repaint();
}


void Front_Qt::draw_RDPScrBlt(int srcx, int srcy, const Rect & drect, bool invert) {
    QImage qbitmap(this->_screen->getCache()->toImage().copy(srcx, srcy, drect.cx, drect.cy));
    if (invert) {
        qbitmap.invertPixels();
    }
    const QRect trect(drect.x, drect.y, drect.cx, drect.cy);
    this->_screen->paintCache().drawImage(trect, qbitmap);
    this->_screen->repaint();
}


QColor Front_Qt::u32_to_qcolor(uint32_t color){
    uint8_t b(color >> 16);
    uint8_t g(color >> 8);
    uint8_t r(color);
    return {r, g, b};
}
    
    
QImage::Format Front_Qt::bpp_to_QFormat(int bpp) {
    QImage::Format format(QImage::Format_RGB16);
    switch (bpp) {
        case 15: format = QImage::Format_RGB555; break;
        case 16: format = QImage::Format_RGB16;  break;
        case 24: format = QImage::Format_RGB888; break;
        case 32: format = QImage::Format_RGB32;  break;
        default : break;
    }
    return format;
}
    
    

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    
//-----------------------------
//      DRAWING FUNCTIONS 
//-----------------------------
    
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
    this->_screen->paintCache().fillRect(rect.x, rect.y, rect.cx, rect.cy, this->u32_to_qcolor(new_cmd24.color));
    this->_screen->repaint();
}


void Front_Qt::draw(const RDPBitmapData & bitmap_data, const uint8_t * data,
    size_t size, const Bitmap & bmp) {
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
    const Rect clipRect(0, 0, this->_info.width, this->_info.height);
    const Rect rect = rectBmp.intersect(clipRect);
    
    this->draw_bmp(rect, bmp, false);
}


void Front_Qt::draw(const RDPLineTo & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    //std::cout << "RDPLineTo" << std::endl; 
    RDPLineTo new_cmd24 = cmd;
    new_cmd24.back_color = color_decode_opaquerect(cmd.back_color, this->mod_bpp, this->mod_palette);
    new_cmd24.pen.color  = color_decode_opaquerect(cmd.pen.color,  this->mod_bpp, this->mod_palette);

    // TO DO clipping
    this->_screen->setPenColor(this->u32_to_qcolor(new_cmd24.back_color));
    this->_screen->paintCache().drawLine(new_cmd24.startx, new_cmd24.starty, new_cmd24.endx, new_cmd24.endy);
    this->_screen->repaint();
}


void Front_Qt::draw(const RDPScrBlt & cmd, const Rect & clip) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }

    //std::cout << "RDPScrBlt" << std::endl;
    
    const Rect drect = clip.intersect(this->_info.width, this->_info.height).intersect(cmd.rect);
    if (drect.isempty()){ return; }

    const signed int deltax = cmd.srcx - cmd.rect.x;
    const signed int deltay = cmd.srcy - cmd.rect.y;
    int srcx(drect.x + deltax);
    int srcy(drect.y + deltay);
    
    switch (cmd.rop) {
            // +------+-------------------------------+
            // | 0x00 | ROP: 0x00000042 (BLACKNESS)   |
            // |      | RPN: 0                        |
            // +------+-------------------------------+
        case 0x00:
            this->_screen->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
            this->_screen->repaint();
            break;
            // +------+-------------------------------+
            // | 0x11 | ROP: 0x001100A6 (NOTSRCERASE) |
            // |      | RPN: DSon                     |
            // +------+-------------------------------+
        case 0x11:  // TODO
                        
            //this->scr_blt_op<Ops::Op_0x11>(srcx, srcy, drect);
            break;
            // +------+-------------------------------+
            // | 0x22 | ROP: 0x00220326               |
            // |      | RPN: DSna                     |
            // +------+-------------------------------+
        case 0x22:  // TODO
                        
            //this->scr_blt_op<Ops::Op_0x22>(srcx, srcy, drect);
            break;
            // +------+-------------------------------+
            // | 0x33 | ROP: 0x00330008 (NOTSRCCOPY)  |
            // |      | RPN: Sn                       |
            // +------+-------------------------------+
        case 0x33:  // TODO
                        
            //this->scr_blt_op<Ops::Op_0x33>(srcx, srcy, drect);
            break;
            // +------+-------------------------------+
            // | 0x44 | ROP: 0x00440328 (SRCERASE)    |
            // |      | RPN: SDna                     |
            // +------+-------------------------------+
        case 0x44:  // TODO
                       
            //this->scr_blt_op<Ops::Op_0x44>(srcx, srcy, drect);
            break;
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
        case 0x66:  // TODO
                        
            //this->scr_blt_op<Ops::Op_0x66>(srcx, srcy, drect);
            break;
            // +------+-------------------------------+
            // | 0x77 | ROP: 0x007700E6               |
            // |      | RPN: DSan                     |
            // +------+-------------------------------+
        case 0x77:  // TODO
                        
            //this->scr_blt_op<Ops::Op_0x77>(srcx, srcy, drect);
            break;
            // +------+-------------------------------+
            // | 0x88 | ROP: 0x008800C6 (SRCAND)      |
            // |      | RPN: DSa                      |
            // +------+-------------------------------+
        case 0x88:  // TODO
                        
            //this->scr_blt_op<Ops::Op_0x88>(srcx, srcy, drect);
            break;
            // +------+-------------------------------+
            // | 0x99 | ROP: 0x00990066               |
            // |      | RPN: DSxn                     |
            // +------+-------------------------------+
        case 0x99:   // TODO
                        
            //this->scr_blt_op<Ops::Op_0x99>(srcx, srcy, drect);
            break;
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
        case 0xBB:  // TODO
                        
            //this->scr_blt_op<Ops::Op_0xBB>(srcx, srcy, drect);
            break;
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
        case 0xDD:  // TODO
                        
            //this->scr_blt_op<Ops::Op_0xDD>(srcx, srcy, drect);
            break;
            // +------+-------------------------------+
            // | 0xEE | ROP: 0x00EE0086 (SRCPAINT)    |
            // |      | RPN: DSo                      |
            // +------+-------------------------------+
        case 0xEE:  // TODO
                        
            break;
            // +------+-------------------------------+
            // | 0xFF | ROP: 0x00FF0062 (WHITENESS)   |
            // |      | RPN: 1                        |
            // +------+-------------------------------+
        case 0xFF:
            this->_screen->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::white);
            this->_screen->repaint();
            break;
        default:
            // should not happen
            break;
    }
}
    

void Front_Qt::draw(const RDPMemBlt & cmd, const Rect & clip, const Bitmap & bitmap) {
    if (this->verbose > 10) {
        LOG(LOG_INFO, "--------- FRONT ------------------------");
        cmd.log(LOG_INFO, clip);
        LOG(LOG_INFO, "========================================\n");
    }
    //std::cout << "RDPMemBlt" << std::endl;
    const Rect& drect = clip.intersect(cmd.rect);
    if (drect.isempty()){
        return ;
    }
    //std::cout << (int) cmd.rop << std::endl;
    switch (cmd.rop) {

        case 0x00:
            this->_screen->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::black);
            this->_screen->repaint();
            break;
            
        case 0x11: // nothing to change 
            break;
            
        case 0x22: // nothing to change;
            break;
            
        case 0x33: // nothing to change
            break;
            
        case 0x44: // nothing to change
            break;
            
        case 0x55: this->draw_bmp(drect, bitmap, true);
            break;
            
        case 0x66: // nothing to change
            break;
            
        case 0x77: // nothing to change;
            break;
            
        case 0x88: // nothing to change
            break;
            
        case 0x99:  // nothing to change
            break;
            
        case 0xAA: // nothing to change
            break;
            
        case 0xBB: // nothing to change
            break;
            
        case 0xCC: this->draw_bmp(drect, bitmap, false);
            break;
            
        case 0xDD: // nothing to change
            break;
            
        case 0xEE: this->draw_bmp(drect, bitmap, false);
            break;
            
        case 0xFF:
            this->_screen->paintCache().fillRect(drect.x, drect.y, drect.cx, drect.cy, Qt::white);
            this->_screen->repaint();
            break;
        default:
            // should not happen
            break;
    }
}

/*
///////////////////////////////
// APPLICATION 
int main(int argc, char** argv){
    
    //" -name QA\\administrateur -pwd 'S3cur3!1nux' -ip 10.10.46.88 -p 3389";

    QApplication app(argc, argv);

    int verbose = 511;

    Front_Qt front(argv, argc, verbose);
    
    
    app.exec();
   
}*/

