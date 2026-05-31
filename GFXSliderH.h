// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include "SliderH.h"
#include <TFT_eSPI.h>

/** Rendered horizontal slider. Tracks the previous thumb position to erase cleanly on each update. */
class GFXSliderH : public SliderH
{
public:

  /// Defines the slider region and stores the TFT reference for rendering.
  GFXSliderH(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t marginX, TFT_eSPI& tft) :
    SliderH(x, y, w, h, marginX), mTFT(tft), mPrevX(0) {}

  /// Moves the thumb to absolute pixel x, clamped to the slider bounds.
  void draw(uint16_t x)
  {
    uint16_t newX = x;
    if (x < mX)
      newX = mX;
    if (x > mX + mWidth)
      newX = mX + mWidth;

    const uint16_t yLine = mY + mHeight / 2;
    mTFT.fillRect(mPrevX, mY, 10, mHeight, 0xFFFF);
    mTFT.drawLine(mX, yLine, mX + mWidth, yLine, 0x0000);
    mTFT.fillRect(newX - 5, mY, 10, mHeight, 0x001F);
    mPrevX = newX - 5;
  }

  /// Moves the thumb to the position corresponding to perc (0–100).
  void drawPercent(uint16_t perc)
  {
    const uint16_t yLine = mY + mHeight / 2;
    const uint16_t x = (perc * mWidth) / 100 + mX - 5;
    mTFT.fillRect(mPrevX, mY, 10, mHeight, 0xFFFF);
    mTFT.drawLine(mX, yLine, mX + mWidth, yLine, 0x0000);
    mTFT.fillRect(x, mY, 10, mHeight, 0x001F);
    mPrevX = x;
  }

private:
    TFT_eSPI& mTFT;
    uint16_t mPrevX;
};
