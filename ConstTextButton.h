// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include "TouchButton.h"
#include "GFXText.h"

/** Fixed-size square button with a centred text label and a rounded border. */
class ConstTextButton : public TouchButton, public GFXText
{
public:

  /// @param text     Label displayed on the button.
  /// @param bgColor  Fill colour for the button background.
  /// @param txtColor Colour of the text label.
  /// @param edgeColor Colour of the rounded border.
  ConstTextButton(uint16_t x, uint16_t y, const char* text, TFT_eSPI& tft, uint16_t bgColor, uint16_t txtColor, uint16_t edgeColor) :
    TouchButton(x, y, BTN_WIDTH, BTN_HEIGHT),
    GFXText(tft),
    mTxt(text),
    mBgColor(bgColor),
    mTxtColor(txtColor),
    mEdgeColor(edgeColor) {}

  /// Draws the rounded border and text label.
  void draw()
  {
    clear();
    mTFT.drawRoundRect(mX + BTN_EDGE_MARGIN, mY + BTN_EDGE_MARGIN, 70, 70, BTN_ROUND_SIZE, mEdgeColor);
    printTxt(mX + BTN_TEXT_MARGIN_X, mY + BTN_TEXT_MARGIN_Y, mTxt.c_str(), mTxtColor, 2);
  }

  /// Fills the button area with the background colour.
  void clear()
  {
    mTFT.fillRect(mX, mY, mWidth, mHeight, mBgColor);
  }

  static const uint16_t BTN_WIDTH = 80;
  static const uint16_t BTN_HEIGHT = 80;
  static const uint16_t BTN_TEXT_MARGIN_X = 10;
  static const uint16_t BTN_TEXT_MARGIN_Y = 33;
  static const uint16_t BTN_EDGE_MARGIN = 5;
  static const uint16_t BTN_ROUND_SIZE = 5;

private:
  String mTxt;
  uint16_t mBgColor;
  uint16_t mTxtColor;
  uint16_t mEdgeColor;
};
