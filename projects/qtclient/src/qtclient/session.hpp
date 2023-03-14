/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "qtclient/widget/screen_widget.hpp"


class QPixmap;
class RdpInput;

namespace qtclient
{

class RegularSession
{

};

class Session : public ScreenWidget
{
public:
    Session(RdpInput* rdp_input, QPixmap* pixmap);

    void setPixmap(QPixmap* pixmap) noexcept
    {
        _pixmap = pixmap;
    }

    void setRdpInput(RdpInput* rdp_input) noexcept
    {
        _rdp_input = rdp_input;
    }

    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

    void paintEvent(QPaintEvent* event) override;

private:
    RdpInput* _rdp_input;
    QPixmap* _pixmap;
};

}
