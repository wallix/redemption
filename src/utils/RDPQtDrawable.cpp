/*
 * 
 * */


#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDP/orders/RDPOrdersPrimaryLineTo.hpp"
#include "text_metrics.hpp"

#include "../core/RDP/RDPQtDrawable.hpp"



RDPQtDrawable::RDPQtDrawable(int width, int height) : QWidget(){
    
    this->_timer = new QTimer;
    QObject::connect(_timer, SIGNAL(timeout()), this, SLOT(update()));
    this->_signalCount = 0;
    
    this->_width = width;
    this->_height = height;
    this->setFixedSize(width, height);
    QSize size(sizeHint());
    QDesktopWidget* desktop = QApplication::desktop();
    int centerW = (desktop->width()/2)  - (size.width()/2);
    int centerH = (desktop->height()/2) - (size.height()/2);
    this->move(centerW, centerH);
    
    this->_label = new QLabel(this);
    this->reInit();
}


QColor u32_to_qcolor(uint32_t color){
    uint8_t b(color >> 16);
    uint8_t g(color >> 8);
    uint8_t r(color);
    return {r, g, b};
}


void RDPQtDrawable::draw(const RDPOpaqueRect & cmd, const Rect & clip) {
    
    Rect rect(cmd.rect.intersect(clip));
    this->_p->fillRect(rect.x, rect.y, rect.cx, rect.cy, u32_to_qcolor(cmd.color));
}


void RDPQtDrawable::draw(const RDPLineTo     & cmd, const Rect & clip) {
    
    // TO DO clipping
    this->_p->setPen(QPen(u32_to_qcolor(cmd.back_color), 1));
    this->_p->drawLine(cmd.startx, cmd.starty, cmd.endx, cmd.endy);
}


void RDPQtDrawable::flush() {
    this->_p->end();
    this->_label->setPicture(*_pi);
}


void RDPQtDrawable::reInit(){
    this->_pi = new QPicture();
    this->_p = new QPainter(_pi);
    this->_p->setRenderHint(QPainter::Antialiasing);
    this->_p->fillRect(0, 0, _width, _height, Qt::white);
}


void RDPQtDrawable::stop(){
    this->_timer->stop();
}


void RDPQtDrawable::update(){
    this->reInit();
        
    this->_mod->draw_event(time(nullptr));
        
    this->flush();
    this->show();
    
    this->_signalCount++;
    if (this->_signalCount >= this->_maxSignals){
        this->stop();
    }
}


void RDPQtDrawable::setModAndStart(mod_api * mod, int maxSignals){
    this->_maxSignals = maxSignals;
    this->_mod = mod;
    this->_timer->start(1000);
}


RDPQtDrawable::~RDPQtDrawable(){
    delete (this->_label);
    delete (this->_p);
    delete (this->_pi);
    delete (this->_timer);
}



