// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include <SdFat.h>
#include <TFT_eSPI.h>


class BMPLoader
{
public:

  BMPLoader(TFT_eSPI& tft, SdFat& sd);

  enum class mode_t
  {
    color,
    grayscale,
    shade,
  };

  void bmpDraw(const char *filename, uint8_t x, uint16_t y, mode_t mode = mode_t::color);

  // Fast BMP draw: one sequential SD pass + single pushImage call.
  // Allocates a full-image RGB565 buffer on heap (~w*h*2 bytes).
  // Falls back to bmpDraw() if malloc fails.
  void bmpDrawBuffered(const char *filename, uint8_t x, uint16_t y, mode_t mode = mode_t::color);

  void setServiceCallback(void (*serviceCallback)()) {
    mServiceCallback = serviceCallback;
  }

  struct bmpHeader {
    bool valid;
    int w;
    int h;
    uint32_t imageoffset;
  };

  TFT_eSPI& getTFT() { return mTFT; }

protected:
  BMPLoader::bmpHeader readBmpHeader();
  uint32_t extract32(uint8_t *data);
  uint16_t extract16(uint8_t *data);

  TFT_eSPI& mTFT;
  SdFat& mSd;
  SdBaseFile mFile;
  void (*mServiceCallback)() = nullptr;

};
