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
#include <cstdint>

/**
 * @brief Run-Length Golomb-Rice compression implementation
 */
class Rlgr {
public:
	/** @brief RLGR mode */
	enum RlgrMode {
		RLGR1,
		RLGR3
	};

public:

	/**
	 * @param mode the RLGR mode
	 * @param pSrcData source bytes
	 * @param srcSize size of source
	 * @param pDstData destination buffer
	 * @param dstSize size of destination buffer
	 * @return if the operation was successful
	 */
	static bool decode(RlgrMode mode, const uint8_t *pSrcData, uint32_t srcSize, int16_t *pDstData, uint32_t dstSize);
};
