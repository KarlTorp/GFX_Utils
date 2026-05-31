// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include <TFT_eSPI.h>

/** Horizontal fill bar with a border. Tracks fill width to redraw only the changed delta. */
class ProgressBar
{
public:

  /// Defines the bar region and colours. borderColor defaults to black.
  ProgressBar(TFT_eSPI& tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
              uint16_t fillColor, uint16_t bgColor, uint16_t borderColor = 0x0000) :
    mTFT(tft), mX(x), mY(y), mWidth(w), mHeight(h),
    mFillColor(fillColor), mBgColor(bgColor), mBorderColor(borderColor),
    mPrevFillW(0) {}

  // Draw the empty bar. Call once on first render.
  void draw()
  {
    mTFT.fillRect(mX, mY, mWidth, mHeight, mBgColor);
    mTFT.drawRect(mX, mY, mWidth, mHeight, mBorderColor);
    mPrevFillW = 0;
  }

  // Update fill level (0–100). Only redraws the delta — no full redraw flicker.
  void setPercent(uint8_t percent)
  {
    if (percent > 100) percent = 100;

    const uint16_t innerW  = mWidth - 2;
    const uint16_t fillW   = ((uint32_t)percent * innerW) / 100;

    if (fillW > mPrevFillW)
    {
      mTFT.fillRect(mX + 1 + mPrevFillW, mY + 1, fillW - mPrevFillW, mHeight - 2, mFillColor);
    }
    else if (fillW < mPrevFillW)
    {
      mTFT.fillRect(mX + 1 + fillW, mY + 1, mPrevFillW - fillW, mHeight - 2, mBgColor);
    }

    mPrevFillW = fillW;
  }

  void setFillColor(uint16_t color)   { mFillColor = color; }
  void setBgColor(uint16_t color)     { mBgColor = color; }
  void setBorderColor(uint16_t color) { mBorderColor = color; }

private:
  TFT_eSPI& mTFT;
  uint16_t mX;
  uint16_t mY;
  uint16_t mWidth;
  uint16_t mHeight;
  uint16_t mFillColor;
  uint16_t mBgColor;
  uint16_t mBorderColor;
  uint16_t mPrevFillW;
};
