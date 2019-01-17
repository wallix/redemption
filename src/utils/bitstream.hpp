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

   bitstream object, used for input / output communication between
   entities
*/
#pragma once

#include <algorithm>
#include <cstdint>
#include <cassert>
#include <memory>

/**
 * @brief bitstream object
 */
class InBitStream {
public:
	/** Constructor
	 *	@param array data
	 *	@param size size of array
	 *	@param offset an offset to start at
	 */
	explicit InBitStream(const uint8_t *array, std::size_t size, std::size_t offset = 0)
	: end(array + size)
	, p(array + offset)
	, bitsLeft(8)
	{
	}

	/**
	 * shifts the bitstream by nbits
	 * @param nbits the size of the shift
	 */
	void shift(std::size_t nbits) {
		if (!nbits) {
			return;
        }

		while (p < end && nbits > 0) {
			std::size_t const b = std::min(nbits, bitsLeft);
			nbits -= b;
			bitsLeft -= b;
			if (bitsLeft == 0) {
				p++;
				bitsLeft = 8;
			}
		}

		assert(nbits == 0);
	}

	/**
	 * picks nbits in the bitstream
	 *
	 * @param nbits the number of bits to pick
	 * @return nbits of the stream
	 */
	uint32_t getBits(std::size_t nbits) {
		uint32_t ret = 0;

		while (p < end && nbits > 0) {
			std::size_t const b = std::min(nbits, bitsLeft);
			if (ret) {
				ret <<= b;
            }
			ret |= (*p >> (bitsLeft - b)) & ((1 << b) - 1);
			bitsLeft -= b;
			nbits -= b;

			if (bitsLeft == 0) {
				bitsLeft = 8;
				p++;
			}
		}

		assert(nbits == 0); // if not that means that we ask for more bits than available
		return ret;
	}

	/**
	 * peeks nbits in the bitstream but without modifying the bitstream
	 *
	 * @param nbits the number of bits to pick
	 * @return nbits of the stream
	 */
	uint32_t peekBits(std::size_t nbits) const {
		uint32_t ret = 0;
		const uint8_t *alias = p;
		std::size_t localBitsLeft = bitsLeft;

		while (alias < end && nbits > 0) {
			std::size_t const b = std::min(nbits, localBitsLeft);
			if (ret) {
				ret <<= b;
            }
			ret |= (*alias >> (localBitsLeft - b)) & ((1 << b) - 1);
			localBitsLeft -= b;
			nbits -= b;

			if (localBitsLeft == 0) {
				localBitsLeft = 8;
				alias++;
			}
		}

		if (nbits != 0) {
			ret <<= nbits; // do not assert here, as sometime the rlgr decoder read ahead too much bits and expect zeros
		}

		return ret;
	}

	/**
	 * @return the remaining bits in the stream
	 */
	std::size_t getRemainingLength() const {
		return bitsLeft + ((end - p) - 1) * 8;
	}

	/**
	 * @return if we're reached the end of stream
	 */
	bool isEos() const {
		return (p == end);
	}

private:
	const uint8_t *end, *p;
	std::size_t bitsLeft;
};
