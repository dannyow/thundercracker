/* -*- mode: C; c-basic-offset: 4; intent-tabs-mode: nil -*-
 *
 * Thundercracker firmware
 *
 * Copyright <c> 2012 Sifteo, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "flash_map.h"
#include "tasks.h"
#include <algorithm>


void FlashMapBlock::erase() const
{
    STATIC_ASSERT(FlashDevice::ERASE_BLOCK_SIZE <= BLOCK_SIZE);
    STATIC_ASSERT((BLOCK_SIZE % FlashDevice::ERASE_BLOCK_SIZE) == 0);

    unsigned I = address();
    unsigned E = I + BLOCK_SIZE;

    for (; I != E; I += FlashDevice::ERASE_BLOCK_SIZE) {
        ASSERT(I < E);

        // This is currently the only operation that's allowed to take so
        // long to complete that we need to reset the watchdog explicitly for it!
        Tasks::resetWatchdog();
        FlashDevice::eraseBlock(I);
    }

    // Must take place after erasing the flash device, for debug-only verify checks
    FlashBlock::invalidate(I, E, FlashBlock::F_KNOWN_ERASED);
}

bool FlashMapSpan::flashAddrToOffset(FlashAddr flashAddr, ByteOffset &byteOffset) const
{
    if (!map)
        return false;

    // Split the flash address
    uint32_t allocBlock = flashAddr / FlashMapBlock::BLOCK_SIZE;
    uint32_t allocOffset = flashAddr & FlashMapBlock::BLOCK_MASK;
    int32_t result = allocOffset - firstByte();

    // We have no direct index for this, so do a linear search in our map
    for (unsigned i = 0; i < arraysize(map->blocks); i++) {
        /*
         * The block may or may not be valid. If it's part of the in-use portion
         * of the map, that would be a problem. If not, it's perfectly legal and
         * we shouldn't even be reading the map contents in that case.
         *
         * So, we order our tests such that we do the range test before the block
         * index test. If the range test fails, we can give up our search
         * immediately.
         */
        if (result >= 0) {
            if (!offsetIsValid(result))
                break;

            if (map->blocks[i].index() == allocBlock) {
                byteOffset = result;
                return true;
            }
        }
        result += FlashMapBlock::BLOCK_SIZE;
    }

    // No match
    return false;
}

bool FlashMapSpan::copyBytes(FlashBlockRef &ref, ByteOffset byteOffset, uint8_t *dest, uint32_t length) const
{
    while (length) {
        PhysAddr srcPA;
        uint32_t chunk = length;

        if (!getBytes(ref, byteOffset, srcPA, chunk))
            return false;

        memcpy(dest, srcPA, chunk);
        dest += chunk;
        byteOffset += chunk;
        length -= chunk;
    }

    return true;
}

bool FlashMapSpan::copyBytesUncached(ByteOffset byteOffset, uint8_t *dest, uint32_t length) const
{
    while (length) {
        /*
         * Note: Even though we are bypassing the cache, we still split on
         *       cache block boundaries, since that's also the alignment
         *       guaranteed by a FlashMapSpan.
         */

        ByteOffset blockPart = byteOffset & ~(ByteOffset)FlashBlock::BLOCK_MASK;
        ByteOffset bytePart = byteOffset & FlashBlock::BLOCK_MASK;
        uint32_t maxLength = FlashBlock::BLOCK_SIZE - bytePart;
        uint32_t chunk = MIN(length, maxLength);

        FlashAddr fa;
        if (!offsetToFlashAddr(blockPart, fa))
            return false;

        FlashDevice::read(fa + bytePart, dest, chunk);

        byteOffset += chunk;
        dest += chunk;
        length -= chunk;
    }

    return true;
}
