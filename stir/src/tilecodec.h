/* -*- mode: C; c-basic-offset: 4; intent-tabs-mode: nil -*-
 *
 * STIR -- Sifteo Tiled Image Reducer
 * Micah Elizabeth Scott <micah@misc.name>
 *
 * Copyright <c> 2011 Sifteo, Inc.
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

#ifndef _TILECODEC_H
#define _TILECODEC_H

#include "tile.h"
#include "logger.h"

namespace Stir {

/*
 * TileCodecLUT --
 *
 *    This is similar to TilePalette, but here instead of tracking the
 *    colors used in a particular tile, we'll track the state of the
 *    color LUT in the tile encoder/decoder.
 *
 *    This is used for actually encoding tiles, as well as for
 *    estimating the cost of encoding a particular tile using the
 *    current encoder state.
 */

struct TileCodecLUT {
    // Current state of the color table
    static const unsigned LUT_MAX = 16;
    RGB565 colors[LUT_MAX];

    TileCodecLUT();

    unsigned encode(const TilePalette &pal, uint16_t &newColors);
    unsigned encode(const TilePalette &pal);

    int findColor(RGB565 c, unsigned maxIndex = LUT_MAX - 1) const {
        // Is a particular color in the LUT? Return the index.
        for (unsigned i = 0; i <= maxIndex; i++)
            if (colors[i] == c && isEntryValid(i))
                return i;
        return -1;
    }

    bool isEntryValid(unsigned index) const {
        return 0 != (valid & (1 << index));
    }

    void makeEntryValid(unsigned index) {
        valid |= 1 << index;
    }

private:
    void bumpMRU(unsigned mruIndex, unsigned lutIndex) {
        for (;mruIndex < LUT_MAX - 1; mruIndex++)
            mru[mruIndex] = mru[mruIndex + 1];
        mru[mruIndex] = lutIndex;
    }

    TilePalette::ColorMode lastMode;
    uint32_t valid;         // Bitmap of which LUT entries can be relied on
    uint8_t mru[LUT_MAX];   // Newest entries at the end, oldest at the front.
};


/*
 * RLECodec4 --
 *
 *    A nybble-wise RLE codec. Nybbles are handy since the 8051 can
 *    quickly do 4-bit rotations, and 4 bits is a handy size for run
 *    length counts. The encoding is simple- every time two identical
 *    nybbles are repeated, a third nybble follows with a count of
 *    additional repeats.
 *
 *    Nybbles are stored in little-endian (least significant nybble first).
 */

class RLECodec4 {
 public:
    RLECodec4();

    void encode(uint8_t nybble, std::vector<uint8_t>& out);
    void flush(std::vector<uint8_t>& out);

 private:
    static const unsigned MAX_RUN = 17;
    
    uint8_t runNybble, bufferedNybble;
    bool isNybbleBuffered;
    unsigned runCount;

    void encodeNybble(uint8_t value, std::vector<uint8_t>& out);
    void encodeRun(std::vector<uint8_t>& out, bool terminal=false);
};


/*
 * FlashAddress --
 *
 *    Addresses in our cube's flash memory have some special
 *    properties and representations. All tiles are aligned on a
 *    tile-size multiple, and we also have large flash blocks (64K)
 *    which set our erase granularity.
 *
 *    The device firmware doesn't use linear addresses, instead it
 *    uses addresses that have been broken into three left-justified
 *    7-bit chunks. Due to the hardware conventions, these chunks are
 *    named "low", "lat1", and "lat2". The upper chunks are programmed
 *    into the two hardware latches.
 */

struct FlashAddress {
    uint32_t linear;

    static const unsigned TILE_SIZE = 128;
    
    FlashAddress(uint32_t addr)
        : linear(addr) {}

    FlashAddress(uint8_t lat2, uint8_t lat1, uint8_t low = 0)
        : linear( ((lat2 >> 1) << 14) |
                  ((lat1 >> 1) << 7 ) |
                  ((low  >> 1)      ) ) {} 

    uint8_t low() const {
        return linear << 1;
    }

    uint8_t lat1() const {
        return (linear >> 7) << 1;
    }

    uint8_t lat2() const {
        return (linear >> 14) << 1;
    }
};


/*
 * TileCodec --
 *
 *    A stateful compressor for streams of Tile data.  The encoded
 *    format is a "load stream", a sequence of opcodes that can be
 *    sent over the radio to the cube MCU in order to reproduce the
 *    original tile data in flash memory.
 */

class TileCodec {
 public:
    TileCodec(std::vector<uint8_t>& buffer);

    void encode(const TileRef tile);
    void flush();

    void dumpStatistics(Logger &log);

 private:
    std::vector<uint8_t>& out;
    std::vector<uint8_t> dataBuf;

    bool opIsBuffered;
    uint8_t opcodeBuf;
    unsigned tileCount;
    TileCodecLUT lut;
    RLECodec4 rle;
    unsigned paddedOutputMin;
    FlashAddress currentAddress;

    // Stats
    struct {
        unsigned opcodes;
        unsigned tiles;
        unsigned dataBytes;
    } stats[TilePalette::CM_COUNT];
    int statBucket;
    
    void newStatsTile(unsigned bucket);

    void reservePadding(unsigned bytes);

    void encodeOp(uint8_t op);
    void flushOp();

    void encodeLUT(uint16_t newColors);
    void encodeWord(uint16_t w);
    void encodeTileRLE4(const TileRef tile, unsigned bits);
    void encodeTileMasked16(const TileRef tile);
};

};  // namespace Stir

#endif
