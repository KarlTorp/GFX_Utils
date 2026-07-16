// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

// Loads pre-converted raw RGB565 images (see tools/convert_to_raw.py in the
// Pokedex_Pages project) with optional grayscale/shade recoloring. Separate
// from BMPLoader, which handles on-the-fly BMP decoding - the two file
// formats and drawing strategies don't share meaningful implementation, so
// they're kept as distinct classes rather than one class doing both jobs.

#include <SdFat.h>
#include <TFT_eSPI.h>


class RawLoader
{
public:

  RawLoader(TFT_eSPI& tft, SdFat& sd);

  enum class mode_t
  {
    color,
    grayscale,
    shade,
  };

  // Draw a pre-converted raw RGB565 file (see tools/convert_to_raw.py).
  // File format: 4-byte header (uint16_t w LE, uint16_t h LE)
  //              followed by w*h big-endian RGB565 pixels, top-to-bottom.
  // mode_t::color streams pixels through unmodified; grayscale/shade recolor
  // each chunk in place before it's sent to the display.
  void rawDraw(const char *filename, uint8_t x, uint16_t y, mode_t mode = mode_t::color);

  // Like rawDraw but uses DMA double-buffering: reads the next SD chunk
  // while the previous chunk is being DMA'd to the display in parallel.
  // Requires tft.initDMA() in setup(). Falls back to rawDraw() if malloc fails.
  void rawDrawDMA(const char *filename, uint8_t x, uint16_t y, mode_t mode = mode_t::color);

  // Load a raw file into a caller-provided buffer without displaying it.
  // Call rawDisplayBuffer() afterwards to show one or more pre-loaded images.
  // Returns true on success. buf must hold at least w*h uint16_t values.
  // outW and outH receive the image dimensions from the file header.
  bool rawLoad(const char *filename, uint16_t *buf, uint32_t bufPixels,
               uint16_t &outW, uint16_t &outH);

  // Display a buffer previously filled by rawLoad() using a single DMA transfer.
  void rawDisplayBuffer(uint8_t x, uint16_t y, uint16_t w, uint16_t h,
                        uint16_t *buf);

  void setServiceCallback(void (*serviceCallback)()) {
    mServiceCallback = serviceCallback;
  }

  TFT_eSPI& getTFT() { return mTFT; }

protected:
  // Recolors count pixels in buf for grayscale/shade. Pixels are stored
  // byte-swapped relative to their true RGB565 value (see RawLoader.cpp) -
  // this un-swaps, recolors, and swaps back in place. Not called for color mode.
  void applyMode(uint16_t *buf, uint32_t count, mode_t mode);

  TFT_eSPI& mTFT;
  SdFat& mSd;
  SdBaseFile mFile;
  void (*mServiceCallback)() = nullptr;

};
