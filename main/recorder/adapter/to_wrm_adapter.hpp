/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(__MAIN_RECORDER_ADAPTER_TO_WRM_ADAPTER_HPP__)
#define __MAIN_RECORDER_ADAPTER_TO_WRM_ADAPTER_HPP__

#include "recorder/adapter.hpp"
#include "recorder/wrm_recorder_option.hpp"
#include "recorder/to_wrm.hpp"
#include "recorder/to_one_wrm.hpp"

class ToWrmAdapter
: public RecorderAdapter
{
    RecorderOption& _option;

public:
    ToWrmAdapter(RecorderOption& option)
    : _option(option)
    {}

    virtual void operator()(WRMRecorder& recorder, const char* outfile)
    {
        const char * metaname = this->_option.metaname.empty() ? 0 : this->_option.metaname.c_str();

        const unsigned char * key = 0;
        const unsigned char * iv = 0;
        if (this->_option.out_crypt_mode)
        {
            if (this->_option.out_crypt_key.size)
                key = this->_option.out_crypt_key.data;
            if (this->_option.out_crypt_iv.size)
                iv = this->_option.out_crypt_iv.data;
        }

        if (this->_option.cat_wrm) {
            to_one_wrm(recorder, outfile,
                       this->_option.range.left.time,
                       this->_option.range.right.time,
                       metaname,
                       this->_option.out_crypt_mode,
                       key, iv
                      );
        }
        if (!this->_option.cat_wrm) {
            to_wrm(recorder, outfile,
                   this->_option.range.left.time,
                   this->_option.range.right.time,
                   this->_option.time.time,
                   this->_option.frame,
                   this->_option.screenshot_start,
                   this->_option.screenshot_wrm,
                   metaname,
                   this->_option.out_crypt_mode,
                   key, iv
                  );
        }
    }
};

#endif
