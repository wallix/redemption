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

   Use (implemented) basic RDP orders to draw some known test pattern

*/

#if !defined(__TEST_INTERNAL_HPP__)
#define __TEST_INTERNAL_HPP__

struct test_internal_mod : public internal_mod {
    test_internal_mod(
        wait_obj * event,
        int (& keys)[256], int & key_flags, Keymap * &keymap,
        ModContext & context, Front & front, Session * session):
            internal_mod(keys, key_flags, keymap, front)
    {
        this->event = event;

        uint8_t raw3[] = {
        0xda, 0xde, 0xdb, 0xde, 0xda, 0xde, 0xda, 0xde, 0xda, 0xde, 0xdb, 0xde, 0xda, 0xde, 0xda, 0xde,
        0xda, 0xde, 0xba, 0xde, 0x37, 0xc6, 0x38, 0xce, 0x1d, 0x99, 0x1d, 0x99, 0x1d, 0x99, 0x00, 0x00,
        0xda, 0xde, 0xda, 0xde, 0xda, 0xde, 0xda, 0xde, 0xda, 0xde, 0xda, 0xde, 0xda, 0xde, 0xda, 0xde,
        0xda, 0xde, 0xda, 0xde, 0x37, 0xce, 0x17, 0xc6, 0x1d, 0x99, 0x1d, 0x99, 0x1d, 0x99, 0x00, 0x00,
        };

        //    Bitmap bmp(16, 48, 1);
        //    bmp.copy_upsidedown(raw, 48);

        Bitmap bmp(16, 15, 2);
        bmp.copy(raw3);

        Bitmap & bitmap = bmp;

        RGBPalette palette332;
        /* rgb332 palette */
        for (int bindex = 0; bindex < 4; bindex++) {
            for (int gindex = 0; gindex < 8; gindex++) {
                for (int rindex = 0; rindex < 8; rindex++) {
                    palette332[(rindex << 5) | (gindex << 2) | bindex] =
                    (RGBcolor)(
                    // r1 r2 r2 r1 r2 r3 r1 r2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
                        (((rindex<<5)|(rindex<<2)|(rindex>>1))<<16)
                    // 0 0 0 0 0 0 0 0 g1 g2 g3 g1 g2 g3 g1 g2 0 0 0 0 0 0 0 0
                       | (((gindex<<5)|(gindex<<2)|(gindex>>1))<< 8)
                    // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 b1 b2 b1 b2 b1 b2 b1 b2
                       | ((bindex<<6)|(bindex<<4)|(bindex<<2)|(bindex)));
                }
            }
        }

        const uint16_t width = bitmap.cx;
        const uint16_t height = bitmap.cy;
        const uint8_t * src = bitmap.data_co;
        const uint8_t in_bpp = bitmap.bpp;
        const uint8_t out_bpp = front.orders->rdp_layer->client_info.bpp;

        uint8_t * bmpdata = (uint8_t*)malloc(width * height * nbbytes(out_bpp));
        uint8_t * dest = bmpdata;
        for (int i = 0; i < width * height; i++) {
            uint32_t pixel = color_decode(in_bytes_le(nbbytes(in_bpp), src),
                                          in_bpp,
                                          palette332);
            uint32_t target_pixel = color_encode(pixel, out_bpp, palette332);
            printf("target_pixel = %.6x pixel = %.6x out_bpp=%d nbbout=%d in_bpp=%d nbbin=%d\n",
                target_pixel, pixel, out_bpp, nbbytes(out_bpp), in_bpp, nbbytes(in_bpp));

            target_pixel = 0xFFFFFF & target_pixel;
            out_bytes_le(dest, nbbytes(out_bpp), target_pixel);
            src += nbbytes(in_bpp);
            dest += nbbytes(out_bpp);
        }

        front.begin_update();
    //    front.send_bitmap_front(Rect(31, 572, 48, 1), Rect(0, 0, 48, 1), bmpdata, 0, Rect(0, 0, 1024, 768));
        front.send_bitmap_front(Rect(31, 572, 15, 2), Rect(0, 0, 15, 2), bmpdata, 0, Rect(0, 0, 1024, 768));
        front.end_update();
    }

    virtual ~test_internal_mod()
    {
    }

    // event from front (keyboard or mouse event)
    virtual int mod_event(int msg, long x, long y, long param4, long param5)
    {
        return 0;
    }

    // event from back en (draw event from remote or internal server)
    // returns module continuation status, 0 if module want to continue
    // non 0 if it wants to stop (to run another module)
    virtual int mod_signal()
    {
        return 0;
    }

};

#endif
