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
    QPainter*  _p;
    QPicture*  _pi;
    int        _width;
    int        _height;


    QColor u32_to_qcolor(uint32_t color){
        uint8_t b(color >> 16);
        uint8_t g(color >> 8);
        uint8_t r(color);
        return {r, g, b};
    }

    
public:
    
    RDPQWidget(int width, int height) : QWidget(), _label(this), _width(width), _height(height) {
        
        this->setFixedSize(width, height);
        QSize size(sizeHint());
        QDesktopWidget* desktop = QApplication::desktop();
        int centerW = (desktop->width()/2)  - (size.width()/2);
        int centerH = (desktop->height()/2) - (size.height()/2);
        this->move(centerW, centerH);
        
       
        this->reInitView();
    }
    
    
    void draw(const RDPOpaqueRect       & cmd, const Rect & clip) {
        Rect rect(cmd.rect.intersect(clip));
        this->_p->fillRect(rect.x, rect.y, rect.cx, rect.cy, u32_to_qcolor(cmd.color));
    }
    
    
    void draw(const RDPLineTo           & cmd, const Rect & clip) {
        // TO DO clipping
        this->_p->setPen(QPen(u32_to_qcolor(cmd.back_color), 1));
        this->_p->drawLine(cmd.startx, cmd.starty, cmd.endx, cmd.endy);
    }
    
    
    void flush() {
        this->_p->end();
        this->_label.setPicture(*(this->_pi));
        this->show();
    }
    
    
    void reInitView() {
        this->_pi = new QPicture();
        this->_p = new QPainter(this->_pi);
        this->_p->setRenderHint(QPainter::Antialiasing);
        this->_p->fillRect(0, 0, this->_width, this->_height, Qt::white);
    }
    
    
    ~RDPQWidget() {}
    
};

#endif
