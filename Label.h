// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include "GFXText.h"

/** Single-line text label that optionally clears its area before each write. */
class Label : public GFXText
{
public:

  /// @param length    Pixel width of the clear area used to erase previous text.
  /// @param txt_size  TFT_eSPI font size (1 = small, 2 = medium, …).
  Label(TFT_eSPI& tft, uint16_t x, uint16_t y, uint16_t length, uint16_t txt_size, uint16_t txtColor, uint16_t bgColor = 0xFFFF) :
    GFXText(tft), mX(x), mY(y), mWidth(length), mSize(txt_size), mTxtColor(txtColor), mBgColor(bgColor) {}

  /// Writes text in the stored colour. Clears the label area first when clear is true.
  void write(const char* text, bool clear = true)
  {
    int textHeight = 16;
    if (clear)
    {
      if (mSize == 1)
        textHeight = 10;
      mTFT.fillRect(mX, mY, mWidth, textHeight, mBgColor);
    }
    printTxt(mX + LABEL_TEXT_MARGIN_X, mY + LABEL_TEXT_MARGIN_Y, text, mTxtColor, mSize);
  }

  /// Writes text with a one-off colour override. Clears the label area first when clear is true.
  void write(const char* text, uint16_t color, bool clear = true)
  {
    int textHeight = 16;
    if (clear)
    {
      if (mSize == 1)
        textHeight = 10;
      mTFT.fillRect(mX, mY, mWidth, textHeight, mBgColor);
    }
    printTxt(mX + LABEL_TEXT_MARGIN_X, mY + LABEL_TEXT_MARGIN_Y, text, color, mSize);
  }

  void setTextColor(uint16_t color) { mTxtColor = color; }
  void setBackgroundColor(uint16_t color) { mBgColor = color; }
  void setTextSize(uint16_t size) { mSize = size; }
  void setPosition(uint16_t x, uint16_t y) { mX = x; mY = y; }

  static const uint16_t LABEL_TEXT_MARGIN_X = 0;
  static const uint16_t LABEL_TEXT_MARGIN_Y = 0;

private:
  uint16_t mX;
  uint16_t mY;
  uint16_t mWidth;
  uint16_t mSize;
  uint16_t mBgColor;
  uint16_t mTxtColor;
};
