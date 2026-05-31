// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include <cstdint>

/** Base class for a rectangular touchscreen-hittable area. Extend to add rendering. */
class TouchButton
{
public:

  /// Defines the hit area at (x, y) with the given width and height.
  TouchButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h) :
    mX(x), mY(y), mWidth(w), mHeight(h) {}

  /// Returns true if the coordinate (x, y) falls within the button bounds.
  virtual bool isPressed(uint16_t x, uint16_t y) const
  {
    if (x >= mX && x < (mX + mWidth) && y >= mY && y < (mY + mHeight))
      return true;
    else
      return false;
  }

protected:
  uint16_t mX;
  uint16_t mY;
  uint16_t mWidth;
  uint16_t mHeight;
};
