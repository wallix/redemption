/*
 * 
 * */
#ifndef RDPQWIDGET_HPP
#define RDPQWIDGET_HPP


#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "mod_api.hpp"

#include <QtGui/QWidget>
#include <QtGui/QPicture>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QDesktopWidget>
#include <QtGui/QApplication>
#include <QtCore/QTimer>


class RDPQWidget : public QWidget 
{

Q_OBJECT

private:
    
    QLabel     _label;
    QPainter* _painter;
    QPicture  _picture;
    int        _width;
    int        _height;


    QColor u32_to_qcolor(uint32_t color){
        uint8_t b(color >> 16);
        uint8_t g(color >> 8);
        uint8_t r(color);
        return {r, g, b};
    }

    
public:
    
    RDPQWidget(int width, int height) : QWidget(), _label(this), _width(width), _height(height), _picture() {
        
        this->setFixedSize(width, height);
        QSize size(sizeHint());
        QDesktopWidget* desktop = QApplication::desktop();
        int centerW = (desktop->width()/2)  - (size.width()/2);
        int centerH = (desktop->height()/2) - (size.height()/2);
        this->move(centerW, centerH);
        
        _painter = new QPainter(&(this->_picture));
        this->reInitView();
    }
    
    
    void draw(const RDPOpaqueRect       & cmd, const Rect & clip) {
        Rect rect(cmd.rect.intersect(clip));
        this->_painter->fillRect(rect.x, rect.y, rect.cx, rect.cy, u32_to_qcolor(cmd.color));
    }
    
    
    void draw(const RDPLineTo           & cmd, const Rect & clip) {
        // TO DO clipping
        this->_painter->setPen(QPen(u32_to_qcolor(cmd.back_color), 1));
        this->_painter->drawLine(cmd.startx, cmd.starty, cmd.endx, cmd.endy);
    }
    
    
    void flush() {
        this->_painter->end();
        this->_label.setPicture(this->_picture);
        this->show();
    }
    
    
    void reInitView() {
        this->_painter->restore();
        this->_painter->begin(&(this->_picture));
        this->_painter->setRenderHint(QPainter::Antialiasing);
        this->_painter->fillRect(0, 0, this->_width, this->_height, Qt::white);
    }
    
    
    ~RDPQWidget() {}
    
};

#endif
