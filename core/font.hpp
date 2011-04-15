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
   Author(s): Christophe Grosjean, Javier Caverni
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
#include "altoco.hpp"

struct FontChar {
    int offset;  // leading whistespace before char
    int baseline; // really -height (probably unused for now)
    int width; // with of glyph acctually containing pixels
    int height; // height of glyph in pixel
    int incby; // width of glyph in pixel including leading and trailing whitespaces
    uint8_t* data;
    FontChar(){
        this->offset = 0;
        this->baseline = 0;
        this->width = 0;
        this->height = 0;
        this->incby = 0;
        this->data = 0;
    }
    ~FontChar(){
    }
    inline int datasize(){
        int nb_bytes_width = nbbytes(this->width);
        // nb total bytes rounded up to next multiple of 4
        return (this->height * nb_bytes_width + 3) & ~3;
    }

    /* compare the two font items returns 1 if they match */
    int item_compare(struct FontChar* glyph)
    {
        return glyph
            && (this->offset == glyph->offset)
            && (this->baseline == glyph->baseline)
            && (this->width == glyph->width)
            && (this->height == glyph->height)
            && (0 == memcmp(this->data, glyph->data, glyph->datasize()));
    }

};


#warning NUM_FONTS is misleading it's actually number of glyph in font. Using it to set size of a static array is quite dangerous as we shouldn't have to change code whenever we change font file.

#define NUM_FONTS 0x4e00
#define DEFAULT_FONT_NAME "sans-10.fv1"

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
struct Font {
    struct FontChar font_items[NUM_FONTS];
    char name[32];
    int size;
    int style;
    Font(const char * file_path){
        int fd;
        int b;
        int index;
        int datasize;
        int file_size;

        try{
            if (access(file_path, F_OK)) {
                LOG(LOG_ERR,
                    "create: error font file [%s] does not exist\n",
                    file_path);
                throw 1;
            }
            struct stat st;

            if (stat(file_path, &st)) {
                LOG(LOG_ERR, "create: can't stat file [%s]\n", file_path);
                throw 2;
            }
            if (st.st_size < 1) {
                LOG(LOG_ERR, "create: empty font file [%s]\n", file_path);
                throw 3;
            }
            file_size = st.st_size;
            #warning stream allocated stream here is much too large, we could (should) read fonton the fly without storing whole file in buffer. Doinf that is quite insane.
            Stream stream(file_size + 1024);
            if (-1 == (fd = open(file_path, O_RDONLY))){
                LOG(LOG_ERR,
                    "create: "
                        "can't open font file [%s] for reading\n", file_path);
                throw 4;
            }

            long size_to_read = file_size;
            stream.end = stream.data;
            for (;;){
                b = read(fd, stream.end, (size_to_read<SSIZE_MAX)?size_to_read:SSIZE_MAX);
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

            stream.skip_uint8(4);
            memcpy(this->name, stream.in_uint8p(32), 32);
            this->size = stream.in_uint16_le();
            this->style = stream.in_uint16_le();
            stream.skip_uint8(8);

    #warning we can do something much cooler using C++ facilities and moving glyph building code to FontChar. Only problem : clean error management using exceptions implies a real exception object in FontChar. We will do that later.
            index = 32; // we start at space, no glyph for chars below 32
            while (stream.check_rem(16)) {
                struct FontChar& glyph = this->font_items[index];
#warning height of actual pixels contained in the char (got from clip box of the char)
                glyph.width = stream.in_sint16_le();
#warning head of a clipping box containing that char
                glyph.height = stream.in_sint16_le();
#warning baseline is always -height (seen from the code of fontdump) looks strange. It means that baseline is probably not used in current code.
                glyph.baseline = stream.in_sint16_le();
#warning offset is set to leading spaces of the font
                glyph.offset = stream.in_sint16_le();
                glyph.incby = stream.in_sint16_le();
                stream.skip_uint8(6);
                datasize = glyph.datasize();
                if (datasize < 0 || datasize > 512) {
                    /* shouldn't happen, implies broken font file*/
                    LOG(LOG_ERR,
                        "Error loading font %s. Wrong size for glyph %d"
                        "width %d height %d \n", file_path, index,
                        glyph.width, glyph.height);
                    // one glyph is broken but we continue with other glyphs
                }
                if (!stream.check_rem(datasize)) {
                    LOG(LOG_ERR, "Error loading font %s:"
                        " not enough data for definition of glyph %d"
                        " (expected %d, got %d)\n", file_path, index,
                        datasize, stream.free_size());
                        throw 6;
                        // we stop loading font here, we are at end of file
                }
                glyph.data = new uint8_t[datasize];
                memcpy(glyph.data, stream.in_uint8p(datasize), datasize);
                index++;
            }
        }
        catch (...){
        }
        return;
    }

    /*****************************************************************************/
    /* free the font and all the items */
    ~Font()
    {
        for (int i = 0; i < NUM_FONTS; i++) {
            // delete data only if glyph have been defined
            if (this->font_items[i].data){
                delete [] this->font_items[i].data;
            }
        }
    }

};

#endif
