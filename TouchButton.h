// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include <cstdint>
#include "Arduino.h"

/** Base class for a rectangular touchscreen-hittable area. Extend to add rendering. */
class TouchButton
{
public:

  /// Defines the hit area at (x, y) with the given width and height.
  TouchButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h, unsigned long repressionDelayMs = 500) :
    mX(x), mY(y), mWidth(w), mHeight(h), mRepressionDelay(repressionDelayMs) {}

  /// Returns true if the coordinate (x, y) falls within the button bounds.
  virtual bool isPressed(uint16_t x, uint16_t y)
  {
    if (mRepressionDelay > 0)
    {
      unsigned long currentTime = millis();
      if (currentTime - mLastPressTime < mRepressionDelay)
        return false; // Still in repression delay
      mLastPressTime = currentTime; // Update last press time
    }

    if (x >= mX && x < (mX + mWidth) && y >= mY && y < (mY + mHeight))
      return true;
    else
      return false;
  }

  virtual void setRepressionDelay(unsigned long delayMs) { mRepressionDelay = delayMs; }

protected:
  uint16_t mX;
  uint16_t mY;
  uint16_t mWidth;
  uint16_t mHeight;
  unsigned long mLastPressTime = 0;
  unsigned long mRepressionDelay = 0;
};
