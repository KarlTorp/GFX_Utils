// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include <TFT_eSPI.h>

/** Mixin base that provides a printTxt() helper via a shared TFT_eSPI reference.
 *  Not intended to be instantiated directly. */
class GFXText
{
protected:
  GFXText(TFT_eSPI& tft) : mTFT(tft) {}

  /// Renders null-terminated text at (x, y) with the given RGB565 colour and font size.
  void printTxt(int16_t x, int16_t y, const char* txt, uint16_t color, uint8_t size)
  {
    mTFT.setCursor(x, y);
    mTFT.setTextColor(color);
    mTFT.setTextSize(size);
    mTFT.print(txt);
  }

  TFT_eSPI& mTFT;
};
