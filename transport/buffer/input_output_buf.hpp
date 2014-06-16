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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_PUBLIC_TRANSPORT_BUFFER_INPUT_OUTPUT_BUF_HPP
#define REDEMPTION_PUBLIC_TRANSPORT_BUFFER_INPUT_OUTPUT_BUF_HPP

#include "dispatch_read_and_write.hpp"

#include <unistd.h>

namespace transbuf
{

    struct open_close_base
    {
        template<class Buf>
        static int init(Buf & buf) /*noexcept*/
        { return buf.init(); }

        template<class Buf>
        static int close(Buf & buf) /*noexcept*/
        { return buf.close(); }

        template<class Buf>
        static bool ready(const Buf & buf) /*noexcept*/
        { return buf.ready(); }
    };

    template<class Reader, class OpenClose = open_close_base>
    class input_buf
    : private OpenClose
    , public dispatch_read<input_buf<Reader, OpenClose> >
    {
        friend class dispatch_core;

        Reader buf;

    public:
        template<class T>
        input_buf(const T & params)
        : OpenClose(params)
        {}

        ~input_buf()
        {
            if (this->ready()) {
                this->close();
            }
        }

        int close() /*noexcept*/
        { return OpenClose::close(this->buf); }

        bool ready() const /*noexcept*/
        { return OpenClose::ready(this->buf); }

        OpenClose & impl()
        { return *this; }

        const OpenClose & impl() const
        { return *this; }

    protected:
        Reader & dispatch()
        { return this->buf; }

        int init() /*noexcept*/
        { return OpenClose::init(this->buf); }
    };

    template<class Writer, class OpenClose = open_close_base>
    class output_buf
    : private OpenClose
    , public dispatch_write<output_buf<Writer, OpenClose> >
    {
        friend class dispatch_core;

        Writer buf;

    public:
        template<class T>
        output_buf(const T & params)
        : OpenClose(params)
        {}

        ~output_buf()
        {
            if (this->ready()) {
                this->close();
            }
        }

        bool ready() const /*noexcept*/
        { return OpenClose::ready(this->buf); }

        int close() /*noexcept*/
        { return OpenClose::close(this->buf); }

        OpenClose & impl() /*noexcept*/
        { return *this; }

        const OpenClose & impl() const /*noexcept*/
        { return *this; }

    protected:
        Writer & dispatch() /*noexcept*/
        { return this->buf; }

        int init() /*noexcept*/
        { return OpenClose::init(this->buf); }
    };


    template<class Reader, class OpenClose = open_close_base>
    class reopen_input
    : public input_buf<Reader, OpenClose>
    {
        typedef input_buf<Reader, OpenClose> base_buf;

    public:
        reopen_input()
        {}

        template<class T>
        reopen_input(const T & params)
        : base_buf(params)
        {}

        ssize_t read(void * data, size_t len) /*noexcept*/
        {
            ssize_t res = this->base_buf::read(data, len);
            if (res < 0) {
                return res;
            }
            if (size_t(res) != len) {
                ssize_t res2 = res;
                do {
                    if (const ssize_t err = this->close()) {
                        return res;
                    }
                    reinterpret_cast<char*&>(data) += res2;
                    len -= res2;
                    res2 = this->base_buf::read(data, len);
                    if (res2 < 0) {
                        return res;
                    }
                    res += res2;
                } while (size_t(res2) != len);
            }
            return res;
        }
    };

}

#endif
