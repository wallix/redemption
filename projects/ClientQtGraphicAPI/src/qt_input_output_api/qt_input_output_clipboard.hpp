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


#include <fstream>
#include <vector>


#include "utils/log.hpp"
#include "core/RDP/clipboard.hpp"
#include "utils/fileutils.hpp"

#include "client_redemption/client_config/client_redemption_config.hpp"
#include "client_redemption/client_channels/client_cliprdr_channel.hpp"

#include <QtCore/QMimeData>
#include <QtGui/QClipboard>
#include <QtCore/QUrl>

#if REDEMPTION_QT_VERSION == 4
#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtGui/name>
#else
#   define REDEMPTION_QT_INCLUDE_WIDGET(name) <QtWidgets/name>
#endif


#include REDEMPTION_QT_INCLUDE_WIDGET(QWidget)
#include REDEMPTION_QT_INCLUDE_WIDGET(QApplication)
#include REDEMPTION_QT_INCLUDE_WIDGET(QDesktopWidget)

#undef REDEMPTION_QT_INCLUDE_WIDGET


struct TextEnder {
    enum : uint32_t {
        SIZE = 2
    };

    void emit (uint8_t * chunk, size_t data_len) {
        chunk[data_len + 1] = 0;
        chunk[data_len + 2] = 0;
    }
};

class QtInputOutputClipboard : public QObject, public ClientIOClipboardAPI
{
REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Winconsistent-missing-override")
Q_OBJECT
REDEMPTION_DIAGNOSTIC_POP

public:

    enum : int {
        FILEGROUPDESCRIPTORW_BUFFER_TYPE = 0,
        IMAGE_BUFFER_TYPE                = 1,
        TEXT_BUFFER_TYPE                 = 2
    };

    uint16_t    _bufferTypeID = 0;
    int         _bufferTypeNameIndex = 0;
    bool        _local_clipboard_stream = true;
    size_t      _cliboard_data_length = 0;
    int         _cItems = 0;

    std::string tmp_path;

    enum : uint16_t {
          CF_QT_CLIENT_FILEGROUPDESCRIPTORW = 48025
        , CF_QT_CLIENT_FILECONTENTS         = 48026
    };

    QClipboard                * _clipboard;
    std::unique_ptr<uint8_t[]>  _chunk;
    QImage                      _bufferImage;

    bool is_listenning = false;


    struct CB_out_File {
        uint64_t     size;
        std::string  name;
        std::string  nameUTF8;
        char * chunk;

        CB_out_File(uint64_t size)
          : size(size)
        {}

        ~CB_out_File() {
            delete[] (this->chunk);
        }
    };

    std::vector<CB_out_File*>  _items_list;
    std::vector<std::string>  _temp_files_list;

    ClientCLIPRDRChannel * channel;



    QtInputOutputClipboard(ClientCLIPRDRChannel * channel, const std::string & path, QWidget * parent)
        : QObject(parent), ClientIOClipboardAPI()
        , tmp_path(path)
        , _clipboard(QApplication::clipboard())
        , channel(channel)
    {
        this->QObject::connect(this->_clipboard, SIGNAL(dataChanged()),  this, SLOT(mem_clipboard()));
    }

    void write_clipboard_temp_file(std::string const& fileName, const uint8_t * data, size_t data_len) override {
        std::string filePath(this->tmp_path + "/" + fileName);
        std::string filePath_mem(filePath);
        this->_temp_files_list.push_back(filePath_mem);

        std::ofstream oFile(filePath, std::ios::binary | std::ios::app);

        if(oFile.is_open()) {
            oFile.write(reinterpret_cast<const char *>(data), data_len);
            oFile.close();
        }
    }

    void set_local_clipboard_stream(bool val) override {
        this->_local_clipboard_stream = val;
    }

    uint16_t get_buffer_type_id() override {
        return this->_bufferTypeID;
    }

    size_t get_cliboard_data_length() override {
        return this->_cliboard_data_length;
    }

    int get_citems_number() override {
        return this->_cItems;
    }

    void setClipboard_files(std::string const& name) override {  //std::vector<Front_RDP_Qt_API::CB_FilesList::CB_in_Files> items_list) {

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

    void setClipboard_text(std::string const& str) override {
        this->_clipboard->setText(QString::fromUtf8(str.c_str()), QClipboard::Clipboard);
    }

    void setClipboard_image(const uint8_t * data, const int image_width, const int image_height, const BitsPerPixel bpp) override {               // Paste image to client
        QImage image(data,
                     image_width,
                     image_height,
                     this->bpp_to_QFormat(bpp, false)
                    );

        QImage imageSwapped(image.rgbSwapped().mirrored(false, true));
        //image.mirrored(false, true);
        this->_clipboard->setImage(imageSwapped, QClipboard::Clipboard);
    }

    void clean_CB_temp_dir() {
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

    void emptyBuffer() override {
        this->_bufferTypeID = 0;
        this->_cliboard_data_length = 0;

        this->clean_CB_temp_dir();

        this->_temp_files_list.clear();
        for (size_t i = 0; i < _items_list.size(); i++) {
            delete(this->_items_list[i]);
        }
        this->_items_list.clear();
    }



public Q_SLOTS:

    void mem_clipboard() {
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
                std::string utf8_text = qstr.toUtf8().constData();

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
                        const std::string delimiter = "\n";
                        this->_cItems = 0;
                        uint32_t pos = 0;
                        while (pos <= str.size()) {
                            pos = str.find(delimiter);
                            std::string path = str.substr(0, pos);
                            str = str.substr(pos+1, str.size());

                            // double slash
                            uint32_t posSlash(0);
                            std::string slash = "/";
                            bool stillSlash = true;
                            while (stillSlash) {
                                posSlash = path.find(slash, posSlash);
                                if (posSlash < path.size()) {
                                    path = path.substr(0, posSlash) + "//" + path.substr(posSlash+1, path.size());
                                    posSlash += 2;
                                } else {
                                    path = path.substr(0, path.size());
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
                                file->name = std::string(reinterpret_cast<char *>(UTF16nameData), UTF16nameSizeReal);
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

    ConstImageDataView get_image() override
    {
        return ConstImageDataView(
            this->_chunk.get(),
            this->_bufferImage.width(),
            this->_bufferImage.height(),
            this->_bufferImage.width(),
            ConstImageDataView::BitsPerPixel(this->_bufferImage.depth()),
            ConstImageDataView::Storage::TopToBottom
        );
    }

    uint8_t * get_text() override {
        return this->_chunk.get();
    }

    std::string get_file_item_name(int index) override {
        return this->_items_list[index]->nameUTF8;
    }

    array_view_char get_file_item(int index) override {
        return {this->_items_list[index]->chunk, this->_items_list[index]->size};
    }

    QImage::Format bpp_to_QFormat(BitsPerPixel bpp, bool alpha) {
        QImage::Format format(QImage::Format_RGB16);

        if (alpha) {

            switch (bpp) {
                case BitsPerPixel{15}: format = QImage::Format_ARGB4444_Premultiplied; break;
                case BitsPerPixel{16}: format = QImage::Format_ARGB4444_Premultiplied; break;
                case BitsPerPixel{24}: format = QImage::Format_ARGB8565_Premultiplied; break;
                case BitsPerPixel{32}: format = QImage::Format_ARGB32_Premultiplied;   break;
                default : break;
            }
        } else {

            switch (bpp) {
                case BitsPerPixel{15}: format = QImage::Format_RGB555; break;
                case BitsPerPixel{16}: format = QImage::Format_RGB16;  break;
                case BitsPerPixel{24}: format = QImage::Format_RGB888; break;
                case BitsPerPixel{32}: format = QImage::Format_RGB32;  break;
                default : break;
            }
        }

        return format;
    }

};

