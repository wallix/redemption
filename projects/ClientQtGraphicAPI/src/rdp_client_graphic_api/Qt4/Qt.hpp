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
   Author(s): Christophe Grosjean, Clément Moroldo

*/


#include <QtGui/QImage>
#include <QtGui/QRgb>
#include <QtGui/QBitmap>
#include <QtGui/QColormap>
#include <QtGui/QPainter>

#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QColor>
#include <QtGui/QDesktopWidget>
#include <QtGui/QApplication>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtCore/QSocketNotifier>
#include <QtGui/QLineEdit>
#include <QtGui/QFormLayout>
#include <QtGui/QDialog>
#include <QtGui/QPushButton>
#include <QtGui/QClipboard>
#include <QtGui/QTabWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QScrollArea>
#include <QtGui/QTableWidget>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtCore/QMimeData>
#include <QtCore/QUrl>
#include <QtGui/QCompleter>
#include <QtGui/QFileDialog>
#include <QtCore/QThread>
#include <QtCore/QtGlobal>
#include <QtCore/QDebug>
//#include <QtCore/QFileSystemWatcher>
#include <QtCore/QTimer>
#include <QtGui/QToolTip>

