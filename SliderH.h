// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include <cstdint>

/** Touch logic for a horizontal slider. No rendering — pair with GFXSliderH for display output. */
class SliderH
{
public:

  /// Defines the slider region. marginX extends the hit area beyond each end on the X axis.
  SliderH(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t marginX) :
    mX(x), mY(y), mWidth(w), mHeight(h), mMarginX(marginX) {}

  /// Returns true if (x, y) is within the slider area including the touch margin.
  bool isPressed(uint16_t x, uint16_t y) const
  {
    if (x >= (mX - mMarginX) && x <= (mX + mWidth + mMarginX) && y >= mY && y <= (mY + mHeight))
      return true;
    return false;
  }

  /// Maps the x coordinate to a percentage (0–100). Returns 0 if outside the slider area.
  float getPercent(uint16_t x, uint16_t y) const
  {
    float percent = 0.0f;
    if (isPressed(x, y))
      percent = ((float)(x - mX) / (float)(mWidth)) * 100.0f;

    if (percent < 0.0f)
      percent = 0.0f;

    if (percent > 100.0f)
      percent = 100.0f;

    return percent;
  }

protected:
  uint16_t mX;
  uint16_t mY;
  uint16_t mWidth;
  uint16_t mHeight;
  uint16_t mMarginX;
};
