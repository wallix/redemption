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
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Dominique Lafages,
              Raphael Zhou, Jonathan Poelen
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Font header file
*/

#include "core/font.hpp"
#include "utils/sugar/unique_fd.hpp"
#include "utils/stream.hpp"
#include "utils/log.hpp"

#include <algorithm>

#include <cerrno>
#include <cstring>

namespace
{
    // height aligned of 4 bytes
    constexpr uint8_t unknown_glyph_data[12]{
        0b01111'000,
        0b10001'000,
        0b00001'000,
        0b00011'000,
        0b00110'000,
        0b00100'000,
        0b00100'000,
        0b00000'000,
        0b00100'000,
        0b00100'000,
    };
}  // namespace

FontCharView Font::default_unknown_glyph() noexcept
{
    return FontCharView{2, 0, 7, 5, 10, unknown_glyph_data};
}

/*
- the RBF1 file always begins by the label "RBF1"
- Police global informations are :
    * name (32 bytes) (ex : Deja Vu Sans)
    * size (2 bytes)
    * style (2 bytes) (always '1')
    * max height (4 bytes)
    * number of glyph (4 bytes)
    * total data len (4 bytes)
- Individual glyph informations are :
    * value (4 bytes)
    * offsetx (2 bytes)
    * offsety (2 bytes)
    * abcA (left space)
    * abcB (glyph width)
    * abcC (roght space)
    * cx (2 bytes)
    * cy (2 bytes)
    * data (the bitmap representing the sketch of the glyph, one bit by pixel, 0 for
      background, 1 for foreground) (aligned of 4 btyes)
*/

void Font::load_from_file(char const * file_path)
{
    // TODO Temporary disabling font to avoid useless messages in watchdog
    //LOG(LOG_INFO, "Reading font file %s", file_path);
    // RAZ of font chars table

    this->max_height_ = default_unknown_glyph().offsety + default_unknown_glyph().height + 1;

    unique_fd const ufd{open(file_path, O_RDONLY)};
    if (!ufd) {
        LOG(LOG_ERR, "Font: can't open font file [%s] for reading: %s",
            file_path, strerror(errno));
        return ;
    }

    auto read = [&ufd](byte_ptr buf, size_t len){
        ssize_t r;
        do  {
            r = ::read(ufd.fd(), buf.to_u8p(), len);
        } while (r == -1 && errno == EINTR);
        return r;
    };

    enum class [[nodiscard]] Read { eof, error, ok };

    size_t const stream_buf_sz = 8192;
    char stream_buf[stream_buf_sz];
    InStream stream(stream_buf, 0);

    auto prepare_stream = [&](size_t const len, int index) -> Read {
        assert(len < stream_buf_sz / 2);
        size_t const remaining = stream.in_remain();
        if (remaining < len) {
            memmove(stream_buf, stream.get_current(), remaining);
            ssize_t b = read(stream_buf + remaining, stream_buf_sz - remaining);
            if (b == 0) {
                return Read::eof;
            }
            if (b < 0) {
                LOG(LOG_ERR, "Font: error reading font file [%s] error: %s",
                    file_path, strerror(errno));
                return Read::error;
            }
            size_t const new_size = remaining + b;
            if (new_size < len) {
                LOG(LOG_WARNING, "Font: file %s defines glyphs up to %d, file looks broken",
                    file_path, index < 0 ? index : index+32);
                return Read::error;
            }
            stream = InStream(stream_buf, new_size);
        }
        return Read::ok;
    };


    uint32_t number_of_glyph;
    uint32_t total_data_len;

    // Read header
    {
        size_t const header_size = 4 + 32 + 2 + 2 + 4 + 4;

        switch (prepare_stream(header_size, -1)) {
            case Read::error:
            case Read::eof:
                return ;
            case Read::ok:
                break;
        }

        if (strncmp(byte_ptr(stream_buf).to_charp(), "RBF1", 4)) {
            LOG(LOG_ERR,"Font: bad magic number ('%.*s', expected 'RBF1'). Please, update font file", 4, stream_buf);
            return ;
        }

        stream.in_skip_bytes(4);
        stream.in_copy_bytes(this->name_, 32);
        this->name_[31] = 0;
        this->size_ = stream.in_uint16_le();
        // TODO temporary disabled to avoid warning in watchdog, see other TODO above to reenable later
        // LOG(LOG_INFO, "font name <%s> size <%u>", this->name, this->size);
        this->style_ = stream.in_uint16_le();
        this->max_height_ = stream.in_uint16_le();
        number_of_glyph = stream.in_uint32_le();
        total_data_len = stream.in_uint32_le();

        // LOG(LOG_DEBUG,
        //     "name: '%s'  size: %d  style: %d  nbglyph: %d  total_data: %d",
        //     this->name_, this->size_, this->style_, number_of_glyph, total_data_len);
    }

    this->data_glyphs = std::make_unique<uint8_t[]>(total_data_len);
    this->font_items.reserve(number_of_glyph);

    auto* data = this->data_glyphs.get();

    // Extract each character glyph
    Read status = Read::eof;
    for (uint32_t index{}; index < number_of_glyph; ++index) {
        // Read header
        size_t const description_size = 4 + 2 + 2 + 2 + 2 + 2 + 2 + 2;
        if (Read::ok != (status = prepare_stream(description_size, index))) {
            break;
        }

        auto const value = stream.in_uint32_le(); (void)value;
        auto const offsetx = stream.in_sint16_le();
        auto const offsety = stream.in_sint16_le();
        auto const abcA = stream.in_sint16_le();
        auto const abcB = stream.in_uint16_le();
        auto const abcC = stream.in_sint16_le();
        auto const width = stream.in_uint16_le();
        auto const height = stream.in_uint16_le();

        auto const data_size = align4(nbbytes(width) * height);

        // LOG(LOG_DEBUG,
        //     "value: %d  offsetx: %d  offsety: %d  left: %d  display_width: %d  "
        //     "right: %d  width: %d  height: %d  data_size: %d",
        //     value, offsetx, offsety, left, display_width, right, width, height, data_size);

        if (size_t(width * height) > stream_buf_sz
         || this->data_glyphs.get() + total_data_len - data < data_size
        ) {
            LOG(LOG_ERR, "Font: error reading font file [%s at glyph %u]:"
                " width(%u)*height(%u) too large (total_data_len = %u)",
                file_path, index, width, height, total_data_len);
            return;
        }

        if (Read::ok != (status = prepare_stream(data_size, index))) {
            break;
        }

        stream.in_copy_bytes(data, data_size);

        this->font_items.emplace_back(
            std::max(offsetx + abcA, 1), offsety, std::max(abcB + abcC, width+0),
            width, height, data);
        data += data_size;
    }

    if (status == Read::ok && (stream.in_remain() || 0 != read(stream_buf, 1))) {
        LOG(LOG_ERR, "Font: error loading font %s. Bad size", file_path);
    }
}
