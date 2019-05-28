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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "capture/wrm_chunk_type.hpp"
#include "transport/mwrm_reader.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"

#include "red_emscripten/bind.hpp"
#include "red_emscripten/em_asm.hpp"


struct WrmPlayer
{
    WrmPlayer(emscripten::val callbacks, std::string data) noexcept
    : callbacks(std::move(callbacks))
    , data(std::move(data))
    {}

    bool next_order() noexcept
    {
        if (!this->chunk.count) {
            constexpr std::size_t header_size = 8;

            InStream header(this->data_remaining());
            if (header.in_remain() < header_size) {
                return false;
            }

            this->chunk.type = safe_cast<WrmChunkType>(header.in_uint16_le());
            this->chunk.size = header.in_uint32_le();
            this->chunk.count = header.in_uint16_le();

            if (header.in_remain() < this->chunk.size) {
                return false;
            }
        }

        return true;
    }

    int interpret_order() noexcept
    {
        RED_EM_ASM({
            console.log("RdpClient: " + $0 + "/" + $1 + " " + $2 + " " + $3);
        }, this->offset, this->data.size(), this->chunk.count, this->chunk.size);
        --this->chunk.count;
        this->offset += this->chunk.size;
        return int(this->chunk.type) + 1;
    }

private:
    const_bytes_view data_remaining() const noexcept
    {
        return const_bytes_view(this->data).array_from_offset(this->offset);
    }

    struct Chunk
    {
        uint32_t size;
        WrmChunkType type;
        uint16_t count = 0;
    };

    Chunk chunk;
    emscripten::val callbacks;
    std::string data;
    std::size_t offset = 0;
};

EMSCRIPTEN_BINDINGS(player)
{
    redjs::class_<WrmPlayer>("WrmPlayer")
        .constructor<emscripten::val, std::string>()
        .function("next", &WrmPlayer::next_order)
        .function("interpret", &WrmPlayer::interpret_order)
    ;
}
