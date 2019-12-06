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

#include "utils/image_data_view.hpp"
#include "client_redemption/client_channels/client_cliprdr_channel.hpp"

#include <QtCore/QObject>
#include <QtGui/QImage>

#include <vector>
#include <memory>


class QWidget;
class QClipboard;
class ClientCLIPRDRChannel;


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
        // TODO unique_ptr / dynamic_array
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

    QtInputOutputClipboard(ClientCLIPRDRChannel * channel, const std::string & path, QWidget * parent);

    void write_clipboard_temp_file(std::string const& fileName, bytes_view data) override;

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

    void setClipboard_files(std::string const& name) override;

    void setClipboard_text(std::string const& str) override;

    // Paste image to client
    void setClipboard_image(ConstImageDataView const& image) override;

    void clean_CB_temp_dir();

    void emptyBuffer() override;


public Q_SLOTS:

    void mem_clipboard();

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

    array_view_const_u8 get_cliboard_text() override {
        return {this->_chunk.get(), this->_cliboard_data_length};
    }

    std::string get_file_item_name(int index) override {
        return this->_items_list[index]->nameUTF8;
    }

    array_view_const_char get_file_item(int index) override {
        return {this->_items_list[index]->chunk, this->_items_list[index]->size};
    }

    static QImage::Format bpp_to_QFormat(BitsPerPixel bpp, bool alpha)
    {
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
