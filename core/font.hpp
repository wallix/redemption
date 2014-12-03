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

#ifndef _REDEMPTION_CORE_FONT_HPP_
#define _REDEMPTION_CORE_FONT_HPP_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include "make_unique.hpp"
#include <bits/posix1_lim.h>

#include "log.hpp"
#include "stream.hpp"
#include "bitfu.hpp"

//##############################################################################
struct FontChar
//##############################################################################
{
    int       offset = 0;   // leading whistespace before char
    int       baseline = 0; // real -height (probably unused for now)
    int       width = 0;    // width of glyph actually containing pixels
    int       height = 0;   // height of glyph (in pixels)
    int       incby = 0;    // width of glyph (in pixels) including leading and trailing whitespaces
    std::unique_ptr<uint8_t[]> data;

    FontChar(int offset, int baseline, int width, int height, int incby)
        : offset(offset)
        , baseline(baseline)
        , width(width)
        , height(height)
        , incby(incby)
        , data(std::make_unique<uint8_t[]>(this->datasize()))
    {
    }

    FontChar() = default;

    FontChar(FontChar && other) = default;
    FontChar & operator=(FontChar &&) = default;
    void * operator new (size_t) = delete;

    FontChar(const FontChar & other)
        : offset(other.offset)
        , baseline(other.baseline)
        , width(other.width)
        , height(other.height)
        , incby(other.incby)
        , data(std::make_unique<uint8_t[]>(other.datasize()))
    {
        memcpy(this->data.get(), other.data.get(), other.datasize());
    }

    explicit operator bool () const noexcept {
        return bool(this->data);
    }

    //==============================================================================
    inline int datasize() const noexcept
    //==============================================================================
    {
        return align4(nbbytes(this->width) * this->height);
    }

    /* compare the two font items returns 1 if they match */
    //==============================================================================
    int item_compare(FontChar & glyph, bool ignore_incby = true) noexcept
    //==============================================================================
    {
        bool result =
               glyph
            && (this->offset == glyph.offset)
            && (this->baseline == glyph.baseline)
            && (this->width == glyph.width)
            && (this->height == glyph.height)
            && (ignore_incby || (this->incby == glyph.incby))
            && (0 == memcmp(this->data.get(), glyph.data.get(), glyph.datasize()));

        if (result && ignore_incby)
        {
            if ((this->incby < 0) && (glyph.incby >= 0))
            {
                this->incby = glyph.incby;
            }
            else if ((this->incby >= 0) && (glyph.incby < 0))
            {
                glyph.incby = this->incby;
            }
        }

        return result;
    }
}; // END STRUCT - FontChar


TODO(" NUM_GLYPHS is misleading it's actually number of glyph in font. Using it to set size of a static array is quite dangerous as we shouldn't have to change code whenever we change font file.")


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


/* font */
//##############################################################################
struct Font
//##############################################################################
{
#ifndef DEFAULT_FONT_NAME
#define DEFAULT_FONT_NAME "dejavu_14.fv1"
#endif

    enum {
           NUM_GLYPHS = 0x4e00
    };

    FontChar font_items[NUM_GLYPHS];
    char name[32];
    int size;
    int style;


    // Constructor
    // Params :
    //    - file_path : path to the font definition file (*.fv1)
    //==============================================================================
    Font(const char * file_path) {
    //==============================================================================
        int fd;
        int b;
         // we start at space, no glyph for chars below 32
        int file_size;

        LOG(LOG_INFO, "Reading font file %s", file_path);
        // RAZ of font chars table

        // Does font definition file exist and is it accessible ?
        if (access(file_path, F_OK)) {
            LOG(LOG_ERR,
                "create: error font file [%s] does not exist\n",
                file_path);
            goto ErrorReadingFontFile;
        }

        // Retrieves system stats about the file
        struct stat st;
        if (stat(file_path, &st)) {
            LOG(LOG_ERR, "create: can't stat file [%s]\n", file_path);
            goto ErrorReadingFontFile;
        }
        // Is file empty ?
        if (st.st_size < 1) {
            LOG(LOG_ERR, "create: empty font file [%s]\n", file_path);
            goto ErrorReadingFontFile;
        }

        // Allocate a buffer to read the whole file into
        file_size = st.st_size;

        if (-1 == (fd = open(file_path, O_RDONLY))){
            LOG(LOG_ERR, "create: can't open font file [%s] for reading\n", file_path);
            goto ErrorReadingFontFile;
        }

        {
            long size_to_read = file_size;
            BStream stream(8192);

            // Read header
            // -----------
            while ((b = read(fd, stream.end, std::min<uint32_t>(size_to_read, 8192))) < 0){
                if (b >= 0){
                    break;
                }
                if ((errno == EAGAIN)||(errno == EINTR)){
                    continue;
                }
                LOG(LOG_ERR,"create: error reading font file [%s] error: %s\n", file_path, strerror(errno));
                goto ErrorReadingFontFile;
            }
            stream.end += b;
            size_to_read -= b;
            if (size_to_read == 0){
                close(fd);
                fd = -1;
            }

            // Extract font info from the buffer
            //----------------------------------
            stream.in_skip_bytes(4);                       // >>> 4 bytes for FNT1 (dropped)
            stream.in_copy_bytes(this->name, 32);          // >>> 32 bytes for Font Name
            this->size = stream.in_uint16_le();            // >>> 2 bytes for Font Size
            LOG(LOG_INFO, "font name <%s> size <%u>", this->name, this->size);
            this->style = stream.in_uint16_le();           // >>> 2 bytes for Font Style
            stream.in_skip_bytes(8);                       // >>> 8 bytes for PAD (dropped)

            // Extract each character glyph
            for (int index = 32; index < NUM_GLYPHS ; index++) {
                unsigned remaining = stream.in_remain();
                if (remaining < 1024){
                    if (size_to_read > 0){
                        TODO("Create a pack_left function in stream to do this");
                        //-----------------------------------------------------
                        memmove(stream.get_data(), stream.p, remaining);
                        stream.p = stream.get_data();
                        stream.end = stream.p + remaining;
                        //-----------------------------------------------------
                        while ((b = read(fd, stream.end, std::min<uint32_t>(size_to_read, 8192 - remaining))) < 0){
                            if (b >= 0){
                                break;
                            }
                            if ((errno == EAGAIN)||(errno == EINTR)){
                                continue;
                            }
                            LOG(LOG_ERR,"create: error reading font file [%s] error: %s\n", file_path, strerror(errno));
                            goto ErrorReadingFontFile;
                        }
                        stream.end += b;
                        size_to_read -= b;
                        if (size_to_read == 0){
                            close(fd);
                            fd = -1;
                        }
                    }
                    // no more remaining glyphs in file
                    if (!stream.in_check_rem(1)){
                        LOG(LOG_INFO, "Font file %s defines glyphs up to %u", file_path, index);
                        break;
                    }
                    if (!stream.in_check_rem(16)){
                        LOG(LOG_WARNING, "Font file %s defines glyphs up to %u, file looks broken", file_path, index);
                        break;
                    }
                }

//                LOG(LOG_INFO, "Reading definition for glyph %u", index);
                int width = stream.in_sint16_le(); // >>> 2 bytes for glyph width
                int height = stream.in_sint16_le(); // >>> 2 bytes for glyph height

    TODO(" baseline is always -height (seen from the code of fontdump) looks strange. It means that baseline is probably not used in current code.");

                int baseline = stream.in_sint16_le(); // >>> 2 bytes for glyph baseline
                int offset = stream.in_sint16_le(); // >>> 2 bytes for glyph offset
                int incby = stream.in_sint16_le(); // >>> 2 bytes for glyph incby
                stream.in_skip_bytes(6); // >>> 6 bytes for PAD (dropped)
                this->font_items[index] = FontChar(offset, baseline, width, height, incby);

                // Check if glyph data size make sense
                unsigned datasize = this->font_items[index].datasize();
                if (datasize > 512) { // shouldn't happen, implies broken font file
                    LOG(LOG_WARNING,
                        "Error loading font %s. Wrong size for glyph %d"
                        "width %d height %d \n", file_path, index,
                        this->font_items[index].width,
                        this->font_items[index].height);
                    // one glyph is broken but we continue with other glyphs
                    continue;
                }

                // Read the data only if there is enough space left in buffer
                if (!stream.in_check_rem(datasize)) {
                    LOG(LOG_ERR
                       , "Error loading font %s: not enough data for definition of glyph %d (expected %d, got %d)\n"
                        , file_path, index, datasize, static_cast<unsigned>(stream.in_remain())
                       );
                    goto ErrorReadingFontFile;
                }

                // >>> <datasize> bytes for glyph data (bitmap)
                stream.in_copy_bytes(this->font_items[index].data.get(), datasize);
            }
        }
        return;
ErrorReadingFontFile:
        LOG(LOG_ERR, "Error reading font definition file %s, exiting proxy",  file_path);
        exit(-1);
        return;
    }

    bool glyph_defined(uint32_t charnum) const
    {
        if ((charnum < 32)||(charnum >= NUM_GLYPHS)){
            return false;
        }
        return bool(this->font_items[charnum]);
    }
}; // END STRUCT - Font

#endif
