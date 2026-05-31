// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include <cstdint>

/** Touch logic for a vertical slider. No rendering — pair with GFXSliderV for display output. */
class SliderV
{
public:

  /// Defines the slider region. marginY extends the hit area beyond each end on the Y axis.
  SliderV(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t marginY) :
    mX(x), mY(y), mWidth(w), mHeight(h), mMarginY(marginY) {}

  /// Returns true if (x, y) is within the slider area including the touch margin.
  bool isPressed(uint16_t x, uint16_t y) const
  {
    if (x >= mX && x <= (mX + mWidth) && y >= (mY - mMarginY) && y <= (mY + mHeight + mMarginY))
      return true;
    return false;
  }

  /// Maps the y coordinate to a percentage (0–100). Returns 0 if outside the slider area.
  float getPercent(uint16_t x, uint16_t y) const
  {
    float percent = 0.0f;
    if (isPressed(x, y))
      percent = ((float)(y - mY) / (float)(mHeight)) * 100.0f;

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
  uint16_t mMarginY;
};
