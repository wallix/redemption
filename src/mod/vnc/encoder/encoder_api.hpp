/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.
h
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2018
   Author(s): Christophe Grosjean

   Vnc encoder module for RAW bitmaps
*/

#pragma once

#include <functional>
#include <type_traits>

#include <cassert>


class Buf64k;
namespace gdi
{
    class GraphicApi;
}


namespace VNC
{
    namespace Encoder
    {
        enum class EncoderState
        {
            Ready,
            NeedMoreData,
            Exit
        };

        struct Encoder
        {
            Encoder() = default;

            Encoder(Encoder&&) = default;
            Encoder(Encoder const&) = delete;

            template<class Consumer, class = std::enable_if_t<!std::is_same_v<Consumer, Encoder>>>
            explicit Encoder(Consumer&& consumer)
            : consumer(std::forward<Consumer>(consumer))
            {}

            Encoder& operator=(Encoder&&) = default;
            Encoder& operator=(Encoder const&) = delete;

            Encoder& operator=(decltype(nullptr)) noexcept
            {
                this->consumer = nullptr;
                return *this;
            }

            EncoderState operator()(Buf64k& buf, gdi::GraphicApi& gd)
            {
                assert(bool(this->consumer));
                return this->consumer(buf, gd);
            }

            explicit operator bool() const noexcept
            {
                return bool(this->consumer);
            }

        private:
            std::function<EncoderState(Buf64k&, gdi::GraphicApi&)> consumer;
        };
    }  // namespace Encoder
} // namespace VNC
