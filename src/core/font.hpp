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
              Raphael Zhou
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Font header file
*/


#pragma once

#include <cstring>
#include <cerrno>

#include <vector>
#include <algorithm>
#include "utils/bitfu.hpp"
#include "utils/sugar/non_null_ptr.hpp"
#include "utils/sugar/make_unique.hpp"

struct FontChar
{
    int16_t   offset = 0;   // leading whistespace before char
    int16_t   baseline = 0; // real -height (probably unused for now)
    uint16_t  width = 0;    // width of glyph actually containing pixels
    uint16_t  height = 0;   // height of glyph (in pixels)
    int16_t   incby = 0;    // width of glyph (in pixels) including leading and trailing whitespaces
    std::unique_ptr<uint8_t[]> data; // PERF mini_vector<32>

    // TODO data really aligned ?
    FontChar(std::unique_ptr<uint8_t[]> data, int16_t offset, int16_t baseline, uint16_t width, uint16_t height, int16_t incby)
        : offset{offset}
        , baseline{baseline}
        , width{width}
        , height{height}
        , incby{incby}
        , data{std::move(data)}
    {
    }

    FontChar(int16_t offset, int16_t baseline, uint16_t width, uint16_t height, int16_t incby)
        : offset{offset}
        , baseline{baseline}
        , width{width}
        , height{height}
        , incby{incby}
        , data{std::make_unique<uint8_t[]>(this->datasize())}
    {
    }

    FontChar() = default;

    FontChar(FontChar && other) = default;
    FontChar(FontChar const & other) = delete;
    FontChar & operator=(FontChar &&) = default;
    FontChar & operator=(FontChar const &) = delete;

    void * operator new (size_t) = delete;

    FontChar clone() const {
        auto ptr = std::make_unique<uint8_t[]>(this->datasize());
        memcpy(ptr.get(), this->data.get(), this->datasize());
        return FontChar(std::move(ptr), this->offset, this->baseline, this->width, this->height, this->incby);
    }

    explicit operator bool () const noexcept {
        return bool(this->data);
    }

    uint16_t datasize() const noexcept
    {
        return align4(nbbytes(this->width) * this->height);
    }

    /* compare the two font items returns true if they match */
    bool item_compare(FontChar const & glyph) const noexcept
    {
        return glyph
            && (this->offset == glyph.offset)
            && (this->baseline == glyph.baseline)
            && (this->width == glyph.width)
            && (this->height == glyph.height)
            && (0 == memcmp(this->data.get(), glyph.data.get(), glyph.datasize()));
    }

    //void show() {
    //          uint8_t   fc_bit_mask        = 128;
    //    const uint8_t * fc_data            = this->data.get();
    //    const bool      skip_padding_pixel = (this->width % 8);
    //
    //    for (int y = 0; y < this->height; y++)
    //    {
    //        for (int x = 0; x < this->width; x++)
    //        {
    //            if (fc_bit_mask & (*fc_data)) {
    //                printf("X");
    //            }
    //            else {
    //                printf(".");
    //            }
    //
    //            fc_bit_mask >>= 1;
    //            if (!fc_bit_mask)
    //            {
    //                fc_data++;
    //                fc_bit_mask = 128;
    //            }
    //        }
    //
    //        if (skip_padding_pixel) {
    //            fc_data++;
    //            fc_bit_mask = 128;
    //            printf("_");
    //        }
    //        printf("\n");
    //    }
    //    printf("
    //}
}; // END STRUCT - FontChar


// TODO NUM_GLYPHS is misleading it's actually number of glyph in font. Using it to set size of a static array is quite dangerous as we shouldn't have to change code whenever we change font file.


/*
  The fv1 files contain
  Font File Header (just one)
    FNT1       4 bytes
    Font Name  32 bytes
    Font Size  2 bytes
    Font Style 2 bytes
    Pad        8 bytes
  Font Data (repeat for each glyph)
    Width      2 bytes
    Height     2 bytes
    Baseline   2 bytes
    Offset     2 bytes
    Incby      2 bytes
    Pad        6 bytes
    Glyph Data var, see FONT_DATASIZE macro
*/

#ifndef DEFAULT_FONT_NAME
#define DEFAULT_FONT_NAME "dejavu_14.fv1"
#endif

struct Font
{
    Font() = default;

    /// \param file_path  path to the font definition file (*.fv1)
    explicit Font(const char * file_path)
    {
        this->load_from_file(file_path);
        this->font_items.shrink_to_fit();
        if (auto item = this->glyph_at('?')) {
            this->unknown_item = item;
        }
    }

    bool is_loaded() const
    {
        return this->font_items.size();
    }

    uint16_t size() const noexcept {
        return this->size_;
    }

    uint16_t style() const noexcept {
        return this->style_;
    }

    char const * name() const noexcept {
        return this->name_;
    }

    bool glyph_defined(uint32_t charnum) const
    {
        return (size_t(charnum - 32u) < this->font_items.size())
            && bool(this->font_items[charnum - 32u]);
    }

    FontChar const & glyph_or_unknown(uint32_t charnum) const
    {
        return this->glyph_defined(charnum)
             ? this->font_items[charnum - 32u]
             : *this->unknown_item;
    }

    FontChar const * glyph_at(uint32_t charnum) const
    {
        return this->glyph_defined(charnum)
             ? &this->font_items[charnum - 32u]
             : nullptr;
    }

    FontChar const & unknown_glyph() const
    {
        return *this->unknown_item;
    }

private:
    static FontChar const & default_unknown_glyph()
    {
        static FontChar item(
            std::unique_ptr<uint8_t[]>(
                new uint8_t[16]{0, 0, 0, 0x70, 0x88, 0x08, 0x10, 0x20, 0x20, 0x00, 0x20, 0x20, 0, 0, 0, 0}
            ), 1, -15, 5, 15, 7
        );
        return item;
    }

    void load_from_file(const char * file_path);

    std::vector<FontChar> font_items;
    non_null_ptr<FontChar const> unknown_item = &default_unknown_glyph();
    uint16_t size_ = 0;
    uint16_t style_ = 0;
    char name_[32] {};
};


#include "utils/sugar/unique_fd.hpp"
#include "utils/stream.hpp"
#include "utils/log.hpp"

#include "cxx/cxx.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

inline void Font::load_from_file(char const * file_path)
{
    enum { MAX_GLYPHS = 65 * 1024, NUM_GLYPHS = 0x4e00 };

    // TODO Temporary disabling font to avoid useless messages in watchdog
    //LOG(LOG_INFO, "Reading font file %s", file_path);
    // RAZ of font chars table

    // Does font definition file exist and is it accessible ?
    if (access(file_path, F_OK)) {
        LOG(LOG_ERR, "Font: error font file [%s] does not exist", file_path);
        return ;
    }

    // Retrieves system stats about the file
    struct stat st;
    if (stat(file_path, &st)) {
        LOG(LOG_ERR, "Font: can't stat file [%s]", file_path);
        return ;
    }

    using std::size_t;
    const size_t header_size = 4 + 32 + 2 + 2 + 8;

    // Is file empty ?
    if (st.st_size < 1 || size_t(st.st_size) < header_size) {
        LOG(LOG_ERR, "Font: empty font file [%s]", file_path);
        return ;
    }

    int fd;
    if (-1 == (fd = open(file_path, O_RDONLY))) {
        LOG(LOG_ERR, "Font: can't open font file [%s] for reading", file_path);
        return ;
    }
    unique_fd auto_close{fd};


    size_t const stream_buf_sz = 8192;
    char stream_buf[stream_buf_sz];
    ssize_t b;
    InStream stream(stream_buf, 0);

    auto read = [](int fd, void * buf, size_t len){
        ssize_t r;
        do  {
            r = ::read(fd, buf, len);
        } while (r == -1 && errno == EINTR);
        return r;
    };

    // Read header
    {
        size_t const header_size = 4 + 32 + 2 + 2 + 8;
        assert(stream_buf_sz >= header_size);
        b = read(fd, stream_buf, stream_buf_sz);

        if (size_t(b) < header_size) {
            LOG(LOG_ERR,"Font: error reading font file [%s] error: %s", file_path, strerror(errno));
            return ;
        }
        stream = InStream(stream_buf, b);

        // Extract font info from the buffer
        //----------------------------------
        stream.in_skip_bytes(4);                       // >>> 4 bytes for FNT1 (dropped)
        stream.in_copy_bytes(this->name_, 32);          // >>> 32 bytes for Font Name
        this->name_[31] = 0;
        this->size_ = stream.in_uint16_le();            // >>> 2 bytes for Font Size
        // TODO temporary disabled to avoid warning in watchdog, see other TODO above to reenable later
        // LOG(LOG_INFO, "font name <%s> size <%u>", this->name, this->size);
        this->style_ = stream.in_uint16_le();           // >>> 2 bytes for Font Style
        stream.in_skip_bytes(8);                       // >>> 8 bytes for PAD (dropped)
    }

    enum class Read { eof, error, ok };

    auto prepare_stream = [&](size_t const len, unsigned index) -> Read {
        assert(len);
        assert(len < stream_buf_sz / 2);
        size_t const remaining = stream.in_remain();
        if (remaining < len) {
            memmove(stream_buf, stream.get_current(), remaining);
            b = read(fd, stream_buf + remaining, stream_buf_sz - remaining);
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
                LOG(LOG_WARNING, "Font: file %s defines glyphs up to %u, file looks broken",
                    file_path, index+32u);
                return Read::error;
            }
            stream = InStream(stream_buf, new_size);
        }
        return Read::ok;
    };

    this->font_items.reserve(NUM_GLYPHS);

    // Extract each character glyph
    Read status = Read::eof;
    for (size_t index {}; index < MAX_GLYPHS ; ++index) {
        // Read header
        size_t const description_size = 16;
        switch ((status = prepare_stream(description_size, index))) {
            case Read::eof:
            case Read::error:
                return ;
            case Read::ok: {
                auto width = stream.in_uint16_le(); // >>> 2 bytes for glyph width
                auto height = stream.in_uint16_le(); // >>> 2 bytes for glyph height

                // TODO baseline is always -height (seen from the code of fontdump) looks strange. It means that baseline is probably not used in current code.
                auto baseline = stream.in_sint16_le(); // >>> 2 bytes for glyph baseline
                auto offset = stream.in_sint16_le(); // >>> 2 bytes for glyph offset
                auto incby = stream.in_sint16_le(); // >>> 2 bytes for glyph incby
                stream.in_skip_bytes(6); // >>> 6 bytes for PAD (dropped)
                this->font_items.emplace_back(offset, baseline, width, height, incby);
                FontChar & item = this->font_items.back();

                // Check if glyph data size make sense
                uint16_t datasize {item.datasize()};
                if (datasize > 512u) { // shouldn't happen, implies broken font file
                    LOG(LOG_ERR,
                        "Font: error loading font %s. Wrong size for glyph %zu"
                        "width %d height %d", file_path, index,
                        item.width, item.height);

                    item.data.reset();

                    size_t const remaining = stream.in_remain();
                    if (datasize > remaining) {
                        datasize -= remaining;
                        stream = InStream(stream_buf, 0);
                        if (static_cast<off_t>(-1) == lseek(fd, datasize, SEEK_CUR)) {
                            return ;
                        }
                    }
                    else {
                        stream.in_skip_bytes(datasize);
                    }

                    // one glyph is broken but we continue with other glyphs
                    continue;
                }

                switch ((status = prepare_stream(datasize, index))) {
                    case Read::error:
                        LOG(LOG_ERR,
                            "Font: error loading font %s: not enough data for definition"
                            " of glyph %zu (expected %u, got %zu)",
                            file_path, index, datasize, stream.in_remain()
                        );
                        REDEMPTION_CXX_FALLTHROUGH;
                    case Read::eof:
                        item.data.reset();
                        return ;
                    case Read::ok:
                        // >>> <datasize> bytes for glyph data (bitmap)
                        stream.in_copy_bytes(item.data.get(), datasize);
                        break;
                }
            } break;
        }
    }

    if (status == Read::ok && 0 == read(fd, stream_buf, stream_buf_sz)) {
        LOG(LOG_ERR, "Font: error loading font %s. Bad size", file_path);
    }
}
