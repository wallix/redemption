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
   Author(s): Clément Moroldo, David Fort
*/

#pragma once

#include "utils/log.hpp"


#include "client_redemption/client_redemption_api.hpp"




class ClientIOClipboardAPI : public ClientIO {

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

public:
    ClientIOClipboardAPI() = default;

    virtual ~ClientIOClipboardAPI() = default;

    // control state
    virtual void emptyBuffer() = 0;

    void set_local_clipboard_stream(bool val) {
        this->_local_clipboard_stream = val;
    }

    bool get_local_clipboard_stream() {
        return this->_local_clipboard_stream;
    }

    //  set distant clipboard data
    virtual void setClipboard_text(std::string const& str) = 0;
    virtual void setClipboard_image(const uint8_t * data, const int image_width, const int image_height, const int bpp) = 0;
    virtual void setClipboard_files(std::string const& name) = 0;
    virtual void write_clipboard_temp_file(std::string const& fileName, const uint8_t * data, size_t data_len) = 0;


    //  get local clipboard data
    uint16_t get_buffer_type_id() {
        return this->_bufferTypeID;
    }

    size_t get_cliboard_data_length() {
        return _cliboard_data_length;
    }

    int get_buffer_type_long_name() {
        return this->_bufferTypeNameIndex;
    }

    virtual ConstImageDataView get_image()
    {
        return ConstImageDataView(
            byte_ptr_cast(""),
            0, 0, 0, ConstImageDataView::BitsPerPixel{},
            ConstImageDataView::Storage::TopToBottom
        );
    }

    // TODO should be `array_view_const_u8 (get_text + get_cliboard_data_length)`
    virtual uint8_t * get_text() = 0;

    // files data (file index to identify a file among a files group descriptor)
    virtual std::string get_file_item_name(int index) {(void)index; return {};}

    // TODO should be `array_view_const_char get_file_item_size(int index)`
    virtual array_view_char get_file_item(int index) { (void)index; return {}; }

    int get_citems_number() {
        return this->_cItems;
    }
};
