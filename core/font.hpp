/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Dominique Lafages
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Font header file

*/

#if !defined(__FONT_HPP__)
#define __FONT_HPP__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "log.hpp"
#include "stream.hpp"
#include <unistd.h>
#include <limits.h>
#include <bits/posix1_lim.h>
#include "bitfu.hpp"

//##############################################################################
struct FontChar
//##############################################################################
{
    int offset;   // leading whistespace before char
    int baseline; // real -height (probably unused for now)
    int width;    // width of glyph actually containing pixels
    int height;   // height of glyph (in pixels)
    int incby;    // width of glyph (in pixels) including leading and trailing whitespaces
    uint8_t * data;

    // Constructor
    //==============================================================================
    FontChar(int offset, int baseline, int width, int height, int incby)
    //==============================================================================
        : offset(   offset )
        , baseline( baseline )
        , width(    width )
        , height(   height )
        , incby(    incby )
        , data(     new uint8_t[ this->datasize() ] )
    //------------------------------------------------------------------------------
    {
    }

    // Destructor
    //==============================================================================
    ~FontChar(){
    //==============================================================================
       delete [] this->data;
    }

    //==============================================================================
    inline int datasize() const {
    //==============================================================================
        return align4(this->height * nbbytes(this->width));
    }

    /* compare the two font items returns 1 if they match */
    //==============================================================================
    int item_compare(struct FontChar* glyph)
    //==============================================================================
    {
        return glyph
            && (this->offset == glyph->offset)
            && (this->baseline == glyph->baseline)
            && (this->width == glyph->width)
            && (this->height == glyph->height)
            && (this->incby == glyph->incby)
            && (0 == memcmp(this->data, glyph->data, glyph->datasize()));
    }

}; // END STRUCT - FontChar


TODO(" NUM_FONTS is misleading it's actually number of glyph in font. Using it to set size of a static array is quite dangerous as we shouldn't have to change code whenever we change font file.")


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
#define DEFAULT_FONT_NAME "dejavu_14.fv1"

    enum {
           NUM_GLYPHS = 0x4e00
         };

    struct FontChar * font_items[NUM_GLYPHS];
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
        int index = 32;
        int file_size;

        LOG(LOG_INFO, "Reading font file %s", file_path);
        // RAZ of font chars table
        for (int i = 0; i < NUM_GLYPHS ; i++){
            font_items[i] = 0;
        }

        try{
            // Does font definition file exist and is it accessible ?
            if (access(file_path, F_OK)) {
                LOG(LOG_ERR,
                    "create: error font file [%s] does not exist\n",
                    file_path);
                throw 1;
            }

            // Retrieves system stats about the file
            struct stat st;
            if (stat(file_path, &st)) {
                LOG(LOG_ERR, "create: can't stat file [%s]\n", file_path);
                throw 2;
            }
            // Is file empty ?
            if (st.st_size < 1) {
                LOG(LOG_ERR, "create: empty font file [%s]\n", file_path);
                throw 3;
            }

            // Allocate a buffer to read the whole file into
            file_size = st.st_size;
            TODO(" stream allocated stream here is much too large  we could (should) read font on the fly without storing whole file in buffer. Doing that is quite insane.")
            BStream stream(file_size + 1024);

            if (-1 == (fd = open(file_path, O_RDONLY))){
                LOG(LOG_ERR,
                    "create: "
                        "can't open font file [%s] for reading\n", file_path);
                throw 4;
            }

            // Read the file into the buffer by slices of bytes, the size of each slices beeing at most equal to the size of a long int (limits::SSIZE_MAX)
            long size_to_read = file_size;
            stream.end = stream.data;

            for (;;){
                b = read(fd, stream.end, (size_to_read < SSIZE_MAX) ? size_to_read : SSIZE_MAX);
                if (b > 0) {
                    size_to_read -= b;
                    stream.end += b;
                    if (size_to_read == 0){
                        break;
                    }
                }
                else if (b == 0) {
                    break;
                }
                else if (b < 0) {
                    LOG(LOG_ERR,
                        "create:"
                        " error reading font file [%s]\n", file_path);
                    throw 5;
                }
            }
            close(fd);

            // Extract font info from the buffer
            //----------------------------------
            // >>> 4 bytes for FNT1 (dropped)
            stream.in_skip_bytes(4);

            // >>> 32 bytes for Font Name
            memcpy(this->name, stream.in_uint8p(32), 32);

            // >>> 2 bytes for Font Size
            this->size = stream.in_uint16_le();
            LOG(LOG_INFO, "font name <%s> size <%u>", this->name, this->size);

            // >>> 2 bytes for Font Style
            this->style = stream.in_uint16_le();

            // >>> 8 bytes for PAD (dropped)
            stream.in_skip_bytes(8);

    TODO(" we can do something much cooler using C++ facilities and moving glyph building code to FontChar. Only problem : clean error management using exceptions implies a real exception object in FontChar. We will do that later.")
            // Extract each character glyph
            while (stream.in_check_rem(16)) {
//                LOG(LOG_INFO, "Reading definition for glyph %u", index);

                // >>> 2 bytes for glyph width
                int width = stream.in_sint16_le();

                // >>> 2 bytes for glyph height
                int height = stream.in_sint16_le();

TODO(" baseline is always -height (seen from the code of fontdump) looks strange. It means that baseline is probably not used in current code.")

                // >>> 2 bytes for glyph baseline
                int baseline = stream.in_sint16_le();

                // >>> 2 bytes for glyph offset
                int offset = stream.in_sint16_le();

                // >>> 2 bytes for glyph incby
                int incby = stream.in_sint16_le();

                // >>> 6 bytes for PAD (dropped)
                stream.in_skip_bytes(6);

                this->font_items[index] = new FontChar(offset, baseline, width, height, incby);

                // Check if glyph data size makes sens
                int datasize = this->font_items[index]->datasize();

                if (datasize < 0 || datasize > 512) {
                    /* shouldn't happen, implies broken font file*/
                    LOG(LOG_ERR,
                        "Error loading font %s. Wrong size for glyph %d"
                        "width %d height %d \n", file_path, index,
                        this->font_items[index]->width,
                        this->font_items[index]->height);
                    // one glyph is broken but we continue with other glyphs
                }
                else {
                    // Read the data only if there is enough space left in buffer
                    if (!stream.in_check_rem(datasize)) {

                        LOG( LOG_ERR
                           , "Error loading font %s:"
                             " not enough data for definition of glyph %d"
                             " (expected %d, got %d)\n"
                           , file_path
                           , index
                           , datasize
                           , (unsigned)(stream.end - stream.p)
                           );

                        throw 6;
                        // we stop loading font here, we are at end of file
                    }

                    // >>> <datasize> bytes for glyph data (bitmap)
                    memcpy(this->font_items[index]->data, stream.in_uint8p(datasize), datasize);
                }
                index++;
            }
        }
        catch (...){
        }

        return;
    }

    // Destructor
    //==============================================================================
    ~Font()
    //==============================================================================
    {
    }

}; // END STRUCT - Font

#endif
