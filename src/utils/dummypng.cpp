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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

   This file is a dummy stub for PNG functions.
   Some tests have a dependency on some functions from libpng 
   (through inclusion of other inlined classes and functions)
   while never actually calling these. Depending on the compilation
   optimisation flags model the references to these functions 
   may or may not be stripped before linking. When they are not stripped
   we need these dummy stubs to link with.
   
*/

#include <stdlib.h>
#include <png.h>

#pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"

 void png_set_IHDR(png_structp png_ptr,
   png_infop info_ptr, png_uint_32 width, png_uint_32 height, int bit_depth,
   int color_type, int interlace_method, int compression_method,
   int filter_method)
{
    // this is a stub, if this function is called we should link with
    // the real libpng instead of the dummypng stub
    abort();
}

 void png_write_info(png_structp png_ptr, png_infop info_ptr)
{
    // this is a stub, if this function is called we should link with
    // the real libpng instead of the dummypng stub
    abort();
}

void  png_write_row(png_structp png_ptr, png_bytep row)
{
    // this is a stub, if this function is called we should link with
    // the real libpng instead of the dummypng stub
    abort();
}

void  png_destroy_write_struct(png_structpp png_ptr_ptr, png_infopp info_ptr_ptr)
{
    // this is a stub, if this function is called we should link with
    // the real libpng instead of the dummypng stub
    abort();
}

void  png_write_end(png_structp png_ptr, png_infop info_ptr)
{
    // this is a stub, if this function is called we should link with
    // the real libpng instead of the dummypng stub
    abort();
}

png_info*  png_create_info_struct(png_structp png_ptr)
{
    // this is a stub, if this function is called we should link with
    // the real libpng instead of the dummypng stub
    abort();
    return nullptr;
}

void  png_set_write_fn(png_structp png_ptr,
        png_voidp io_ptr, png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn)
{
    // this is a stub, if this function is called we should link with
    // the real libpng instead of the dummypng stub
    abort();
}


png_struct*  png_create_write_struct(png_const_charp user_png_ver, png_voidp error_ptr,
png_error_ptr error_fn, png_error_ptr warn_fn)
{
    // this is a stub, if this function is called we should link with
    // the real libpng instead of the dummypng stub
    abort();
    return nullptr;
}

