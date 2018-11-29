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
  Copyright (C) Wallix 2018
  Author(s): David Fort

  Run-Length Golomb-Rice compression
*/
#include "core/RDP/rlgr.hpp"
#include "utils/bitstream.hpp"

#include <cstring>

/* Constants used in RLGR1/RLGR3 algorithm */
#define KPMAX    (80)    /* max value for kp or krp */
#define LSGR    (3)    /* shift count to convert kp to k */
#define UP_GR    (4)    /* increase in kp after a zero run in RL mode */
#define DN_GR    (6)    /* decrease in kp after a nonzero symbol in RL mode */
#define UQ_GR    (3)    /* increase in kp after nonzero symbol in GR mode */
#define DQ_GR    (3)    /* decrease in kp after zero symbol in GR mode */

static inline uint32_t lzcnt_s(uint32_t x)
{
    uint32_t y;
    int n = 32;

    if (!x) {
        return 32;
    }

    y = x >> 16;
    if (y != 0) {
        n = n - 16;
        x = y;
    }

    y = x >>  8;
    if (y != 0) {
        n = n - 8;
        x = y;
    }

    y = x >>  4;
    if (y != 0) {
        n = n - 4;
        x = y;
    }

    y = x >>  2;
    if (y != 0) {
        n = n - 2;
        x = y;
    }

    y = x >>  1;
    if (y != 0) {
        return n - 2;
    }

    return n - x;
}

bool Rlgr::decode(RlgrMode mode, const uint8_t *pSrcData, uint32_t srcSize, int16_t *pDstData, uint32_t dstSize)
{
    int run;
    int cnt;
    int size;
    int nbits;
    unsigned int offset;
    int16_t mag;
    uint16_t code;
    uint32_t sign;
    uint32_t nIdx;
    uint32_t val1;
    uint32_t val2;
    int16_t* pOutput;

    int k = 1;
    int kp = k << LSGR;

    int kr = 1;
    int krp = kr << LSGR;

    if ((mode != RLGR1) && (mode != RLGR3)) {
        mode = RLGR1;
    }

    if (!pSrcData || !srcSize) { // TODO: asserts
        return false;
    }

    InBitStream bs(pSrcData, srcSize);

    if (!pDstData || !dstSize) {
        return false;
    }

    pOutput = pDstData;

    while ((bs.getRemainingLength() > 0) && ((pOutput - pDstData) < dstSize)) {
        if (k) {
            /* Run-Length (RL) Mode */
            run = 0;

            /* count number of leading 0s */
            cnt = lzcnt_s(bs.peekBits(32));
            nbits = bs.getRemainingLength();
            if (cnt > nbits) {
                cnt = nbits;
            }

            int vk = cnt;

            while ((cnt == 32) && (bs.getRemainingLength() >= 32)) {
                bs.shift(32);

                cnt = lzcnt_s(bs.peekBits(32));
                nbits = bs.getRemainingLength();

                if (cnt > nbits) {
                    cnt = nbits;
                }

                vk += cnt;
            }

            bs.shift(vk % 32);

            if (bs.getRemainingLength() < 1) {
                break;
            }

            bs.shift(1);

            while (vk--) {
                run += (1 << k); /* add (1 << k) to run length */

                /* update k, kp params */

                kp += UP_GR;

                if (kp > KPMAX) {
                    kp = KPMAX;
                }

                k = kp >> LSGR;
            }

            /* next k bits contain run length remainder */
            if (bs.getRemainingLength() < k) {
                break;
            }
            run += bs.getBits(k);

            /* read sign bit */
            if (bs.getRemainingLength() < 1) {
                break;
            }
            sign = bs.getBits(1) ? 1 : 0;

            /* count number of leading 1s */
            cnt = lzcnt_s(~bs.peekBits(32));
            nbits = bs.getRemainingLength();

            if (cnt > nbits) {
                cnt = nbits;
            }

            vk = cnt;

            while ((cnt == 32) && (bs.getRemainingLength() >= 32)) {
                bs.shift(32);

                cnt = lzcnt_s(~(bs.peekBits(32)));

                nbits = bs.getRemainingLength();

                if (cnt > nbits) {
                    cnt = nbits;
                }

                vk += cnt;
            }

            bs.shift(vk % 32);

            if (bs.getRemainingLength() < 1) {
                break;
            }

            bs.shift(1);

            /* next kr bits contain code remainder */
            if (bs.getRemainingLength() < kr) {
                break;
            }
            code = bs.getBits(kr);

            /* add (vk << kr) to code */
            code |= (vk << kr);

            if (!vk) {
                /* update kr, krp params */
                krp -= 2;

                if (krp < 0) {
                    krp = 0;
                }

                kr = krp >> LSGR;
            } else if (vk != 1)    {
                /* update kr, krp params */
                krp += vk;

                if (krp > KPMAX) {
                    krp = KPMAX;
                }

                kr = krp >> LSGR;
            }

            /* update k, kp params */
            kp -= DN_GR;

            if (kp < 0) {
                kp = 0;
            }

            k = kp >> LSGR;

            /* compute magnitude from code */
            if (sign) {
                mag = (static_cast<int16_t>(code + 1)) * -1;
            } else {
                mag = static_cast<int16_t>(code + 1);
            }

            /* write to output stream */

            offset = static_cast<int>(pOutput - pDstData);
            size = run;

            if ((offset + size) > dstSize) {
                size = dstSize - offset;
            }

            if (size) {
                std::memset(pOutput, 0, size * sizeof(int16_t));
                pOutput += size;
            }

            if ((pOutput - pDstData) < dstSize)    {
                *pOutput = mag;
                pOutput++;
            }
        } else {
            /* Golomb-Rice (GR) Mode */

            /* count number of leading 1s */
            cnt = lzcnt_s(~bs.peekBits(32));
            nbits = bs.getRemainingLength();

            if (cnt > nbits) {
                cnt = nbits;
            }

            int vk = cnt;

            while ((cnt == 32) && (bs.getRemainingLength() >= 32)) {
                bs.shift(32);

                cnt = lzcnt_s(~bs.peekBits(32));
                nbits = bs.getRemainingLength();

                if (cnt > nbits) {
                    cnt = nbits;
                }

                vk += cnt;
            }

            bs.shift(vk % 32);

            if (bs.getRemainingLength() < 1) {
                break;
            }
            bs.shift(1);

            /* next kr bits contain code remainder */
            if (bs.getRemainingLength() < kr) {
                break;
            }
            code = bs.getBits(kr);

            /* add (vk << kr) to code */
            code |= (vk << kr);

            if (!vk) {
                /* update kr, krp params */
                krp -= 2;

                if (krp < 0) {
                    krp = 0;
                }

                kr = krp >> LSGR;
            } else if (vk != 1)    {
                /* update kr, krp params */
                krp += vk;

                if (krp > KPMAX) {
                    krp = KPMAX;
                }

                kr = krp >> LSGR;
            }

            if (mode == RLGR1) { /* RLGR1 */
                if (!code) {
                    /* update k, kp params */
                    kp += UQ_GR;

                    if (kp > KPMAX) {
                        kp = KPMAX;
                    }

                    k = kp >> LSGR;

                    mag = 0;
                } else {
                    /* update k, kp params */
                    kp -= DQ_GR;

                    if (kp < 0) {
                        kp = 0;
                    }

                    k = kp >> LSGR;

                    /*
                     * code = 2 * mag - sign
                     * sign + code = 2 * mag
                     */

                    if (code & 1) {
                        mag = static_cast<int16_t>((code + 1) >> 1) * -1;
                    } else {
                        mag = static_cast<int16_t>(code >> 1);
                    }
                }

                if ((pOutput - pDstData) < dstSize) {
                    *pOutput = mag;
                    pOutput++;
                }
            } else if (mode == RLGR3) { /* RLGR3 */
                nIdx = 0;

                if (code) {
                    mag = static_cast<uint32_t>(code);
                    nIdx = 32 - lzcnt_s(mag);
                }

                if (bs.getRemainingLength() < nIdx) {
                    break;
                }
                val1 = bs.getBits(nIdx);

                val2 = code - val1;

                if (val1 && val2) {
                    /* update k, kp params */
                    kp -= (2 * DQ_GR);

                    if (kp < 0) {
                        kp = 0;
                    }

                    k = kp >> LSGR;
                } else if (!val1 && !val2) {
                    /* update k, kp params */
                    kp += (2 * UQ_GR);

                    if (kp > KPMAX) {
                        kp = KPMAX;
                    }

                    k = kp >> LSGR;
                }

                if (val1 & 1) {
                    mag = (static_cast<int16_t>((val1 + 1) >> 1)) * -1;
                } else {
                    mag = static_cast<int16_t>(val1 >> 1);
                }

                if ((pOutput - pDstData) < dstSize)    {
                    *pOutput = mag;
                    pOutput++;
                }

                if (val2 & 1) {
                    mag = (static_cast<int16_t>((val2 + 1) >> 1)) * -1;
                } else {
                    mag = static_cast<int16_t>(val2 >> 1);
                }

                if ((pOutput - pDstData) < dstSize)    {
                    *pOutput = mag;
                    pOutput++;
                }
            }
        }
    }

    offset = static_cast<int>(pOutput - pDstData);
    if (offset < dstSize) {
        size = dstSize - offset;
        memset(pOutput, 0, size * 2);
        pOutput += size;
    }

    offset = static_cast<int>(pOutput - pDstData);
    return offset == dstSize;
}
