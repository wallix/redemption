/*
 * 
 * */
#ifndef QTIMERPARENT_HPP
#define QTIMERPARENT_HPP

#include "RDP/RDPQtDrawable.hpp"

#include <QObject>
#include <QtCore/QTimer>



class QTimerParent : public QObject {
    
Q_OBJECT
    

private:
    QTimer         timer;
    int            signalCount;
    int            maxSignals;
    mod_api *      mod;
    RDPQtDrawable* rdpQtDrawable;
       
    
public:
    QTimerParent (RDPQtDrawable* rdpQtDrawable, int msec, int maxSignals, mod_api * mod) : QObject(), timer(this), signalCount(0), mod(mod), maxSignals(maxSignals), rdpQtDrawable(rdpQtDrawable){
        connect(&timer, SIGNAL(timeout()), this, SLOT(countTimeout()));
        this->timer.start(msec);
    }
        
        
public Q_SLOTS:
    void countTimeout() {
        this->signalCount++;
        if (this->signalCount > maxSignals){
            this->timer.stop();
            //this->rdpQtDrawable->flush();//
            
        } else {
            this->rdpQtDrawable->reInitView();
            this->mod->draw_event(time(nullptr));
            this->rdpQtDrawable->flush();
        }
    }
};

#endif

