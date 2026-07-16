// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include "TouchButton.h"
#include <TFT_eSPI.h>

/** Touch button that renders a centred pre-loaded RGB565 icon within its bounds.
 *  Use BMPLoader::rawLoad() to populate the icon buffer before constructing. */
class IconButton : public TouchButton
{
public:

  /// @param borderColor  Colour of the button border. Defaults to black.
  IconButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
             const uint16_t* iconData, uint16_t iconW, uint16_t iconH,
             TFT_eSPI& tft, uint16_t bgColor, bool clearBg = false) :
    TouchButton(x, y, w, h),
    mTFT(tft),
    mIconData(iconData),
    mIconW(iconW),
    mIconH(iconH),
    mBgColor(bgColor),
    mClearBg(clearBg) {}

  /// Draws the background, border, and icon centred within the button bounds.
  void draw()
  {
    if (mClearBg)
      clear();

    if (mIconData)
    {
      const uint16_t iconX = mX + (mWidth  - mIconW) / 2;
      const uint16_t iconY = mY + (mHeight - mIconH) / 2;
      mTFT.pushImage(iconX, iconY, mIconW, mIconH, mIconData);
    }
  }

  /// Fills the button area with the background colour.
  void clear()
  {
    mTFT.fillRect(mX, mY, mWidth, mHeight, mBgColor);
  }

  // Swap the icon at runtime (e.g. active/inactive states).
  void setIcon(const uint16_t* iconData, uint16_t iconW, uint16_t iconH)
  {
    mIconData = iconData;
    mIconW    = iconW;
    mIconH    = iconH;
  }

private:
  TFT_eSPI&       mTFT;
  const uint16_t* mIconData;
  uint16_t        mIconW;
  uint16_t        mIconH;
  uint16_t        mBgColor;
  bool            mClearBg;
};
