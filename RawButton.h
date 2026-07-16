// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include "TouchButton.h"
#include "RawLoader.h"

/** Touch button that renders pre-converted raw RGB565 images */
class RawButton : public TouchButton
{
public:

  /// @param borderColor  Colour of the button border. Defaults to black.
  RawButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
             const char* iconPath, uint16_t iconW, uint16_t iconH,
             RawLoader& loader, uint16_t bgColor = TFT_WHITE, bool clearBg = false) :
    TouchButton(x, y, w, h),
    mLoader(loader),
    mIconPath(iconPath),
    mIconW(iconW),
    mIconH(iconH),
    mBgColor(bgColor),
    mClearBg(clearBg)
  {}

  /// Draws the background, border, and icon centred within the button bounds.
  void draw()
  {
    if (mClearBg)
      clear();

    const uint16_t iconX = mX + (mWidth  - mIconW) / 2;
    const uint16_t iconY = mY + (mHeight - mIconH) / 2;
    mLoader.rawDrawDMA(mIconPath, iconX, iconY, mMode);
  }

  /// Fills the button area with the background colour.
  void clear()
  {
    mLoader.getTFT().fillRect(mX, mY, mWidth, mHeight, mBgColor);
  }

  void blink(uint16_t delayMs)
  {
    clear();
    delay(delayMs);
    draw();
  }

  // Swap the icon at runtime (e.g. active/inactive states).
  void setIcon(const char* iconPath, uint16_t iconW, uint16_t iconH, RawLoader::mode_t mode = RawLoader::mode_t::color)
  {
    mIconPath = iconPath;
    mIconW    = iconW;
    mIconH    = iconH;
    mMode     = mode;
  }

  void setMode(RawLoader::mode_t mode)
  {
    mMode = mode;
  }

private:
  RawLoader&      mLoader;
  const char*     mIconPath;
  uint16_t        mIconW;
  uint16_t        mIconH;
  uint16_t        mBgColor;
  bool            mClearBg;
  RawLoader::mode_t         mMode;
};
