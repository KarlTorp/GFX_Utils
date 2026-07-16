// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include <TFT_eSPI.h>
#include <cstring>

/** Manages a fixed-size pixel buffer for a sub-region of the screen.
 *  create() samples from a larger source image; draw() pushes the buffer to the display. */
class PixelBuffer
{
public:

  /// Defines the screen region and the external buffer to write into.
  PixelBuffer(TFT_eSPI& tft, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t* buffer) :
    mTFT(tft), mX(x), mY(y), mWidth(width), mHeight(height), mBuffer(buffer) {}

  ~PixelBuffer() = default;

  /// Extracts the stored region from the source image img of dimensions width x height.
  void create(const uint16_t *img, uint16_t width, uint16_t height)
  {
    for (int r = 0; r < mHeight; ++r) {
      int srcY = mY + r;
      for (int c = 0; c < mWidth; ++c) {
        int srcX = mX + c;
        uint16_t val = 0;
        if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height) {
          val = img[srcY * width + srcX];
        }
        mBuffer[r * mWidth + c] = val;
      }
    }
  }

  /// Pushes the buffer to the display at the stored position using pushImage.
  void draw()
  {
    if (mBuffer) {
      mTFT.pushImage(mX, mY, mWidth, mHeight, mBuffer);
    }
  }

private:
  TFT_eSPI& mTFT;
  uint16_t mX;
  uint16_t mY;
  uint16_t mWidth;
  uint16_t mHeight;
  uint16_t* mBuffer = nullptr;
};
