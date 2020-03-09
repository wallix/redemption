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
   Copyright (C) Wallix 2018
   Author(s): David Fort
   Based on FreeRDP Copyright (C) 2011 Vic Lee

   DWT (Discrete Wavelet Transform) functions
*/
#include "core/RDP/dwt.hpp"

static void rfx_dwt_2d_decode_block(int16_t *buffer, int16_t *idwt, int subband_width)
{
    int total_width = subband_width*2;

    /* Inverse DWT in horizontal direction, results in 2 sub-bands in L, H order in tmp buffer idwt. */
    /* The 4 sub-bands are stored in HL(0), LH(1), HH(2), LL(3) order. */
    /* The lower part L uses LL(3) and HL(0). */
    /* The higher part H uses LH(1) and HH(2). */

    int16_t* ll = buffer + subband_width * subband_width * 3;
    int16_t* hl = buffer;
    int16_t* l_dst = idwt;

    int16_t* lh = buffer + subband_width * subband_width;
    int16_t* hh = buffer + subband_width * subband_width * 2;
    int16_t* h_dst = idwt + subband_width * subband_width * 2;

    int n;

    for (int y = 0; y < subband_width; y++)
    {
        /* Even coefficients */
        l_dst[0] = ll[0] - ((hl[0] + hl[0] + 1) >> 1);
        h_dst[0] = lh[0] - ((hh[0] + hh[0] + 1) >> 1);
        for (n = 1; n < subband_width; n++)
        {
            int x = n*2;
            l_dst[x] = ll[n] - ((hl[n-1] + hl[n] + 1) >> 1);
            h_dst[x] = lh[n] - ((hh[n-1] + hh[n] + 1) >> 1);
        }

        /* Odd coefficients */
        for (n = 0; n < subband_width-1; n++)
        {
            int x = n*2;
            l_dst[x + 1] = (hl[n]*2) + ((l_dst[x] + l_dst[x + 2]) >> 1);
            h_dst[x + 1] = (hh[n]*2) + ((h_dst[x] + h_dst[x + 2]) >> 1);
        }
        int x = n*2;
        l_dst[x + 1] = (hl[n]*2) + (l_dst[x]);
        h_dst[x + 1] = (hh[n]*2) + (h_dst[x]);

        ll += subband_width;
        hl += subband_width;
        l_dst += total_width;

        lh += subband_width;
        hh += subband_width;
        h_dst += total_width;
    }

    /* Inverse DWT in vertical direction, results are stored in original buffer. */
    for (int x = 0; x < total_width; x++)
    {
        int16_t* dst;
        int16_t* l;
        int16_t* h;

        /* Even coefficients */
        for (n = 0; n < subband_width; n++)
        {
            int y = n*2;
            dst = buffer + y * total_width + x;
            l = idwt + n * total_width + x;
            h = l + subband_width * total_width;
            dst[0] = *l - (((n > 0 ? *(h - total_width) : *h) + (*h) + 1) >> 1);
        }

        /* Odd coefficients */
        for (n = 0; n < subband_width; n++)
        {
            int y = n*2;
            dst = buffer + y * total_width + x;
            l = idwt + n * total_width + x;
            h = l + subband_width * total_width;
            dst[total_width] = (*h*2) + ((dst[0] + dst[n < subband_width - 1 ? 2 * total_width : 0]) >> 1);
        }
    }
}

void Dwt::decode_2d(int16_t *buffer, int16_t *dwt_buffer) {
    rfx_dwt_2d_decode_block(&buffer[3840], dwt_buffer, 8);
    rfx_dwt_2d_decode_block(&buffer[3072], dwt_buffer, 16);
    rfx_dwt_2d_decode_block(&buffer[0], dwt_buffer, 32);
}
