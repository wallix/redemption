/*
 * 
 * */
#ifndef RDPQWIDGET_HPP
#define RDPQWIDGET_HPP

#include <iostream>
#include <stdint.h>

#include "orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "orders/RDPOrdersPrimaryEllipseCB.hpp"
#include "orders/RDPOrdersPrimaryScrBlt.hpp"
#include "orders/RDPOrdersPrimaryMultiDstBlt.hpp"
#include "orders/RDPOrdersPrimaryMultiOpaqueRect.hpp"
#include "orders/RDPOrdersPrimaryDestBlt.hpp"
#include "orders/RDPOrdersPrimaryMultiPatBlt.hpp"
#include "orders/RDPOrdersPrimaryMultiScrBlt.hpp"
#include "orders/RDPOrdersPrimaryPatBlt.hpp"
#include "orders/RDPOrdersPrimaryMemBlt.hpp"
#include "orders/RDPOrdersPrimaryMem3Blt.hpp"
#include "orders/RDPOrdersPrimaryLineTo.hpp"
#include "orders/RDPOrdersPrimaryGlyphIndex.hpp"
#include "orders/RDPOrdersPrimaryPolyline.hpp"
#include "orders/RDPOrdersPrimaryPolygonCB.hpp"
#include "orders/RDPOrdersPrimaryPolygonSC.hpp"
#include "orders/RDPOrdersSecondaryFrameMarker.hpp"
#include "orders/RDPOrdersPrimaryEllipseSC.hpp"
#include "orders/RDPOrdersSecondaryGlyphCache.hpp"
#include "orders/AlternateSecondaryWindowing.hpp"

#include "caches/glyphcache.hpp"
#include "capabilities/glyphcache.hpp"
#include "bitmapupdate.hpp"
#include "../src/utils/bitmap.hpp"
#include "mod_api.hpp"

#include <QtGui/QWidget>
#include <QtGui/QPicture>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QDesktopWidget>
#include <QtGui/QApplication>
#include <QImage>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>


class RDPQWidget : public QWidget 
{

Q_OBJECT


private:
    QLabel    _label;
    QPainter* _painter;
    QPicture  _picture;
    int       _width;
    int       _height;
    QPen      _pen;


    QColor u32_to_qcolor(uint32_t color){
        uint8_t b(color >> 16);
        uint8_t g(color >> 8);
        uint8_t r(color);
        return {r, g, b};
    }

    
public:
    RDPQWidget(int width, int height) : QWidget(), _label(this), _width(width), _height(height), _picture(), _pen() {
        
        this->setFixedSize(width, height);
        QSize size(sizeHint());
        QDesktopWidget* desktop = QApplication::desktop();
        int centerW = (desktop->width()/2)  - (size.width()/2);
        int centerH = (desktop->height()/2) - (size.height()/2);
        this->move(centerW, centerH);
        
        setMouseTracking(true);
        
        this->_painter = new QPainter(&(this->_picture));
        this->_painter->setRenderHint(QPainter::Antialiasing);
        this->_painter->fillRect(0, 0, width, height, Qt::white);
        this->_pen.setWidth(1);
        this->_painter->setPen(this->_pen);
        this->setAttribute(Qt::WA_NoSystemBackground);
    }

    
    void flush() {
        this->_painter->end();
        this->_label.setPicture(this->_picture);
        this->show(); 
    }
    
    
    void reInitView() {
        this->_painter->begin(&(this->_picture));
        this->_painter->fillRect(0, 0, this->_width, this->_height, QColor(0, 0, 0, 0));
    }
    
    
    void draw(const RDPOpaqueRect       & cmd, const Rect & clip) {
        Rect rect(cmd.rect.intersect(clip));
        this->_painter->fillRect(rect.x, rect.y, rect.cx, rect.cy, u32_to_qcolor(cmd.color));
    }
    
    
    void draw(const RDPLineTo           & cmd, const Rect & clip) {
        // TO DO clipping
        this->_pen.setBrush(u32_to_qcolor(cmd.back_color));
        this->_painter->drawLine(cmd.startx, cmd.starty, cmd.endx, cmd.endy);
    }
    
    
    void draw(const RDPBitmapData & bitmap_data, const uint8_t * data, std::size_t size, const Bitmap & bmp) {
        
        if (!bmp.is_valid()){
            return;
        }
        if (bmp.cx() < 0 || bmp.cy() < 0) {
            return ;
        } 

        const QRect rectBmp( bitmap_data.dest_left, bitmap_data.dest_top, 
                             (bitmap_data.dest_right - bitmap_data.dest_left + 1), 
                             (bitmap_data.dest_bottom - bitmap_data.dest_top + 1));
        const QRect clipRect(0, 0, this->_width, this->_height);
        const QRect rect = rectBmp.intersected(clipRect);
            
        const int16_t mincx = std::min<int16_t>(bmp.cx(), std::min<int16_t>(this->_width - rect.x(), rect.width()));
        const int16_t mincy = 1;

        if (mincx <= 0 || mincy <= 0) {
            return;
        }        
        
        int rowYCoord(rect.y() + rect.height()-1);
        int rowsize(bmp.line_size()); //Bpp
      
        const uint8_t * row = bmp.data();
        
        QImage::Format format(QImage::Format_RGB16); //bpp
        if (bmp.bpp() == 16){
            format = QImage::Format_RGB16;
        }
        if (bmp.bpp() == 24){
            format = QImage::Format_RGB888;
        }
        if (bmp.bpp() == 32){
            format = QImage::Format_RGB32;
        }
        if (bmp.bpp() == 15){
            format = QImage::Format_RGB555;
        }
        
        for (size_t k = 0 ; k < bitmap_data.height; k++) {
            
            QImage qbitmap(const_cast<unsigned char*>(row), mincx, mincy, format);
            const QRect trect(rect.x(), rowYCoord, mincx, mincy);
            this->_painter->drawImage(trect, qbitmap);

            row += rowsize;
            rowYCoord--;
        }
    }
    
    
    ~RDPQWidget() {}
    
    void draw(const RDPDestBlt          & cmd, const Rect & clip) {}
    void draw(const RDPMultiDstBlt      & cmd, const Rect & clip) {}
    void draw(const RDPPatBlt           & cmd, const Rect & clip) {}
    void draw(const RDP::RDPMultiPatBlt & cmd, const Rect & clip) {}

    void draw(const RDPMultiOpaqueRect  & cmd, const Rect & clip) {}
    void draw(const RDPScrBlt           & cmd, const Rect & clip) {}
    void draw(const RDP::RDPMultiScrBlt & cmd, const Rect & clip) {}
    void draw(const RDPMemBlt           & cmd, const Rect & clip, const Bitmap & bmp) {}
    void draw(const RDPMem3Blt          & cmd, const Rect & clip, const Bitmap & bmp) {}

    void draw(const RDPGlyphIndex       & cmd, const Rect & clip, const GlyphCache * gly_cache) {}
    void draw(const RDPPolygonSC        & cmd, const Rect & clip) {}
    void draw(const RDPPolygonCB        & cmd, const Rect & clip) {}
    void draw(const RDPPolyline         & cmd, const Rect & clip) {}
    void draw(const RDPEllipseSC        & cmd, const Rect & clip) {}
    void draw(const RDPEllipseCB        & cmd, const Rect & clip) {}
    
    
    // CONTROLLER
    void mousePressEvent(QMouseEvent *e) {
        std::cout << "click   " << "x=" << e->x() << " y=" << e->y() << " button:" << e->button() << std::endl;
    }
    
    void mouseReleaseEvent(QMouseEvent *e) {
        std::cout << "release " << "x=" << e->x() << " y=" << e->y() << " button:" << e->button() << std::endl;
    }
    
    void keyPressEvent(QKeyEvent *e) {
        std::cout << "keyPressed " << e->text().toStdString()  << std::endl;
    }
};

#endif
