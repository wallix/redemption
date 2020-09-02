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

#include "cxx/diagnostic.hpp"


#include <QtWidgets/QProgressBar>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>



class ProgressBarWindow : public QWidget {
    /* This window progess bar is used when loading a wrm video
     */

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Winconsistent-missing-override")
Q_OBJECT
REDEMPTION_DIAGNOSTIC_POP

    QProgressBar load_bar;

public:
    ProgressBarWindow(int maxVal/*, ClientRedemptionIOAPI * front*/)
        : QWidget()
        , load_bar(this)
    {
        this->setWindowTitle("Loading Movie");
        this->setAttribute(Qt::WA_DeleteOnClose);

        QRect rect(QPoint(0,0),QSize(600, 50));
        this->load_bar.setGeometry(rect);
        this->load_bar.setRange(0, maxVal);

        uint32_t centerW = 0;
        uint32_t centerH = 0;
        centerW = (QApplication::desktop()->width()/2)  - 300;
        centerH = (QApplication::desktop()->height()/2) - 25;
        this->move(centerW, centerH);

        this->show();
    }

    void setValue(int val)
    {
        this->load_bar.setValue(val);
        if (val >= this->load_bar.maximum()) {
            this->close();
        }
    }
};
