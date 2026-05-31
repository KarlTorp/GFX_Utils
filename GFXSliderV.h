// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include "SliderV.h"
#include <TFT_eSPI.h>

/** Rendered vertical slider. Tracks the previous thumb position to erase cleanly on each update. */
class GFXSliderV : public SliderV
{
public:

  /// Defines the slider region and stores the TFT reference for rendering.
  GFXSliderV(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t marginY, TFT_eSPI& tft) :
    SliderV(x, y, w, h, marginY), mTFT(tft), mPrevY(0) {}

  /// Moves the thumb to absolute pixel y, clamped to the slider bounds.
  void draw(uint16_t y)
  {
    uint16_t newY = y;
    if (y < mY)
      newY = mY;
    if (y > mY + mHeight)
      newY = mY + mHeight;

    const uint16_t xLine = mX + mWidth / 2;
    mTFT.fillRect(mX, mPrevY, mWidth, 10, 0xFFFF);
    mTFT.drawLine(xLine, mY, xLine, mY + mHeight, 0x0000);
    mTFT.fillRect(mX, newY - 5, mWidth, 10, 0x001F);
    mPrevY = newY - 5;
  }

  /// Moves the thumb to the position corresponding to perc (0–100).
  void drawPercent(uint16_t perc)
  {
    const uint16_t xLine = mX + mWidth / 2;
    const uint16_t y = (perc * mHeight) / 100 + mY - 5;
    mTFT.fillRect(mX, mPrevY, mWidth, 10, 0xFFFF);
    mTFT.drawLine(xLine, mY, xLine, mY + mHeight, 0x0000);
    mTFT.fillRect(mX, y, mWidth, 10, 0x001F);
    mPrevY = y;
  }

private:
  TFT_eSPI& mTFT;
  uint16_t mPrevY;
};
