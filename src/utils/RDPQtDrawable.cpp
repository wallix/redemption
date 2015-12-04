/*
 * 
 * */

#include <QLabel>
#include <QPicture>
#include <QPainter>
#include <QColor>
#include <QDesktopWidget>

#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDPQtDrawable.hpp"



RDPQtDrawable::RDPQtDrawable(int width, int height) : QWidget(){

    this->setFixedSize(width, height);
    QSize size(sizeHint());
    QDesktopWidget* desktop = QApplication::desktop();
    int centerW = (desktop->width()/2) - (size.width()/2);
    int centerH = (desktop->height()/2) - (size.height()/2);
    this->move(centerW, centerH);

    this->_label = new QLabel(this);
    QPicture pi;
    this->_p = new QPainter(&pi);
    this->_p->setRenderHint(QPainter::Antialiasing);
    this->_p->fillRect(0, 0, width, height, Qt::white);
}

QColor u32_to_qcolor(uint32_t color){
    uint8_t b(color >> 16);
    uint8_t g(color >> 8);
    uint8_t r(color);
    return {r, g, b};
}


void RDPQtDrawable::draw(const RDPOpaqueRect & cmd, const Rect & clip) override{
    Rect rect = cmd.rect.intersect(clip);
    this->_p->fillRect(rect.x, rect.y, rect.cx, rect.cy, u32_to_qcolor(cmd.color));
}



void RDPQtDrawable::draw(const RDPLineTo     & cmd, const Rect & clip) override{
    
/*TO DO
    this->_p->setPen(QPen(u32_to_qcolor(cmd.color), 1));
    this->_p->drawLine(x1, y1, x2, y2);*/
}



void RDPQtDrawable::flush() override {
    this->_p->end();
    this->_label->setPicture(pi);
}


RDPQtDrawable::~RDPQtDrawable(){
    delete (this->_label);
    delete (this->_p);
}
