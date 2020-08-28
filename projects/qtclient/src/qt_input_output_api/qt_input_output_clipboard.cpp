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

#include "qt_input_output_api/qt_input_output_clipboard.hpp"
#include "client_redemption/client_channels/client_cliprdr_channel.hpp"

#include <QtCore/QMimeData>
#include <QtGui/QClipboard>
#include <QtCore/QUrl>

#include "redemption_qt_include_widget.hpp"
#include "core/RDP/clipboard.hpp"
#include "utils/fileutils.hpp"

#include REDEMPTION_QT_INCLUDE_WIDGET(QApplication)
#include REDEMPTION_QT_INCLUDE_WIDGET(QWidget)

#include <fstream>

#include <sys/types.h>
#include <dirent.h>


QtInputOutputClipboard::QtInputOutputClipboard(ClientCLIPRDRChannel * channel, const std::string & path, QWidget * parent)
: QObject(parent)
, tmp_path(path)
, _clipboard(QApplication::clipboard())
, channel(channel)
{
    this->QObject::connect(this->_clipboard, SIGNAL(dataChanged()),  this, SLOT(mem_clipboard()));
}

void QtInputOutputClipboard::write_clipboard_temp_file(std::string const& fileName, bytes_view data)
{
    std::string filePath(this->tmp_path + "/" + fileName);
    std::string filePath_mem(filePath);
    this->_temp_files_list.push_back(filePath_mem);

    std::ofstream oFile(filePath, std::ios::binary | std::ios::app);

    if(oFile.is_open()) {
        oFile.write(data.as_charp(), data.size());
        oFile.close();
    }
}

void QtInputOutputClipboard::setClipboard_files(std::string const& name)
{  //std::vector<Front_RDP_Qt_API::CB_FilesList::CB_in_Files> items_list) {

    /*QClipboard *cb = QApplication::clipboard();
    QMimeData* newMimeData = new QMimeData();
    const QMimeData* oldMimeData = cb->mimeData();
    QStringList ll = oldMimeData->formats();
    for (int i = 0; i < ll.size(); i++) {
        newMimeData->setData(ll[i], oldMimeData->data(ll[i]));
    }
    QList<QUrl> list;

    const std::string delimiter = "\n";
    uint32_t pos = 0;
    std::string str = items_list[0].name;
    while (pos <= str.size()) {
        pos = str.find(delimiter);
        std::string path = str.substr(0, pos);
        str = str.substr(pos+1, str.size());
        QString fileName(path.c_str());
        newMimeData->setText(fileName);
        list.append(QUrl::fromLocalFile(fileName));
        QByteArray gnomeFormat = QByteArray("copy\n").append(QUrl::fromLocalFile(fileName).toEncoded());
        newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);
    }

    newMimeData->setUrls(list);
    cb->setMimeData(newMimeData);*/


//         QClipboard *cb = QApplication::clipboard();
//         QMimeData* newMimeData = new QMimeData();
//         const QMimeData* oldMimeData = cb->mimeData();
//         QStringList ll = oldMimeData->formats();
//         for (int i = 0; i < ll.size(); i++) {
//             newMimeData->setData(ll[i], oldMimeData->data(ll[i]));
//         }

//         QByteArray gnomeFormat = QByteArray("copy\n");

//         for (size_t i = 0; i < items_list.size(); i++) {

        QClipboard *cb = QApplication::clipboard();
        QMimeData* newMimeData = new QMimeData();
        const QMimeData* oldMimeData = cb->mimeData();
        QStringList ll = oldMimeData->formats();
        for (int i = 0; i < ll.size(); i++) {
            newMimeData->setData(ll[i], oldMimeData->data(ll[i]));
        }

        QByteArray gnomeFormat = QByteArray("copy\n");

        std::string path(this->tmp_path + "/" + name);
        //std::cout <<  path <<  std::endl;
        QString qpath(path.c_str());

        //qDebug() << "QUrl" << QUrl::fromLocalFile(qpath);

        gnomeFormat.append(QUrl::fromLocalFile(qpath).toEncoded());

        newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);
        cb->setMimeData(newMimeData);
//         }

//         newMimeData->setData("x-special/gnome-copied-files", gnomeFormat);
//         cb->setMimeData(newMimeData);
}

void QtInputOutputClipboard::setClipboard_text(std::string const& str)
{
    this->_clipboard->setText(QString::fromUtf8(str.c_str()), QClipboard::Clipboard);
}

// Paste image to client
void QtInputOutputClipboard::setClipboard_image(ImageView const& image)
{
    QImage qtimage(image.data(), image.width(), image.height(),
        this->bpp_to_QFormat(image.bits_per_pixel(), false));

    QImage imageSwapped(qtimage.rgbSwapped().mirrored(false, true));
    //image.mirrored(false, true);
    this->_clipboard->setImage(imageSwapped, QClipboard::Clipboard);
}

void QtInputOutputClipboard::clean_CB_temp_dir()
{
    DIR *theFolder = opendir(this->tmp_path.c_str());

    if (theFolder) {
        struct dirent *next_file;

        while ((next_file = readdir(theFolder)))
        {
            std::string filepath(this->tmp_path + std::string("/") + next_file->d_name);
            remove(filepath.c_str());
        }
        closedir(theFolder);
    }
}

void QtInputOutputClipboard::emptyBuffer()
{
    this->_bufferTypeID = 0;
    this->_cliboard_data_length = 0;

    this->clean_CB_temp_dir();

    std::destroy(this->_items_list.begin(), this->_items_list.end());
    this->_items_list.clear();
}

void QtInputOutputClipboard::mem_clipboard()
{
    if (this->_local_clipboard_stream) {
        const QMimeData * mimeData = this->_clipboard->mimeData();
        mimeData->hasImage();

        if (mimeData->hasImage()){
        //==================
        //    IMAGE COPY
        //==================
            this->emptyBuffer();

            this->_bufferTypeID = RDPECLIP::CF_METAFILEPICT;
            this->_bufferTypeNameIndex = IMAGE_BUFFER_TYPE;

            QImage bufferImageTmp(this->_clipboard->image());
            if (bufferImageTmp.depth() > 24) {
                bufferImageTmp = bufferImageTmp.convertToFormat(QImage::Format_RGB888);
                bufferImageTmp = bufferImageTmp.rgbSwapped();
            }
            this->_bufferImage = bufferImageTmp;

            this->_cliboard_data_length = this->_bufferImage.byteCount();

            this->_chunk = std::make_unique<uint8_t[]>(this->_cliboard_data_length + RDPECLIP::FormatDataResponsePDU_MetaFilePic::Ender::SIZE);
            for (int i  = 0; i < this->_bufferImage.byteCount(); i++) {
                this->_chunk.get()[i] = this->_bufferImage.bits()[i];
            }
            RDPECLIP::FormatDataResponsePDU_MetaFilePic::Ender ender;
            ender.emit(this->_chunk.get(), this->_cliboard_data_length);

            this->channel->send_FormatListPDU();
        //==========================================================================



        } else if (mimeData->hasText()){                //  File or Text copy

            QString qstr = this->_clipboard->text(QClipboard::Clipboard);

            if (qstr.size() > 0) {
                this->emptyBuffer();
                std::string str(qstr.toUtf8().constData());

                if (str.at(0) == '/') {
            //==================
            //    FILE COPY
            //==================
                    this->_bufferTypeID       = CF_QT_CLIENT_FILEGROUPDESCRIPTORW;
                    this->_bufferTypeNameIndex = FILEGROUPDESCRIPTORW_BUFFER_TYPE;

                    // retrieve each path
                    this->_cItems = 0;
                    uint32_t pos = 0;
                    while (pos <= str.size()) {
                        pos = str.find('\n');
                        std::string path = str.substr(0, pos);
                        str = str.substr(pos+1, str.size());

                        // double slash
                        uint32_t posSlash(0);
                        bool stillSlash = true;
                        while (stillSlash) {
                            posSlash = path.find('/', posSlash);
                            if (posSlash < path.size()) {
                                path = path.substr(0, posSlash) + "//" + path.substr(posSlash+1, path.size());
                                posSlash += 2;
                            } else {
                                stillSlash = false;
                            }
                        }

                        // get file data
                        uint64_t size(::filesize(path.c_str()));
                        std::ifstream iFile(path.c_str(), std::ios::in | std::ios::binary);
                        if (iFile.is_open()) {

                            CB_out_File * file = new CB_out_File(size);

                            file->chunk = new char[file->size];

                            iFile.read(file->chunk, file->size);
                            iFile.close();

                            int posName(path.size()-1);
                            bool lookForName = true;
                            while (posName >= 0 && lookForName) {
                                if (path.at(posName) == '/') {
                                    lookForName = false;
                                }
                                posName--;
                            }
                            file->nameUTF8 = path.substr(posName+2, path.size());
                            //std::string name = file->nameUTF8;
                            int UTF16nameSize(file->nameUTF8.size() *2);
                            if (UTF16nameSize > 520) {
                                UTF16nameSize = 520;
                            }
                            uint8_t UTF16nameData[520];
                            int UTF16nameSizeReal = ::UTF8toUTF16_CrLf(file->nameUTF8, UTF16nameData, UTF16nameSize);
                            file->name = std::string(char_ptr_cast(UTF16nameData), UTF16nameSizeReal);
                            this->_cItems++;
                            this->_items_list.push_back(file);

                        } else {
                            LOG(LOG_WARNING, "Path \"%s\" not found.", path.c_str());
                        }
                    }

                    this->channel->send_FormatListPDU();
            //==========================================================================



                } else {
            //==================
            //    TEXT COPY
            //==================
                    this->_bufferTypeID = RDPECLIP::CF_UNICODETEXT;
                    //this->_bufferTypeNameIndex = TEXT_BUFFER_TYPE;

                    size_t size( ( str.length() * 4) + 2 );

                    this->_chunk = std::make_unique<uint8_t[]>(size);

                    // UTF8toUTF16_CrLf for linux install
                    this->_cliboard_data_length = ::UTF8toUTF16_CrLf(str, this->_chunk.get(), size);

                    TextEnder ender;
                    ender.emit(this->_chunk.get(), this->_cliboard_data_length);

                    this->_cliboard_data_length += TextEnder::SIZE;

                    this->channel->send_FormatListPDU();
            //==========================================================================
                }
            }
        }
    }
}
