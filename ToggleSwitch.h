// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include "GFXText.h"
#include "TouchButton.h"

/** Rounded on/off switch. Fills with an active or inactive colour and shows a text label. */
class ToggleSwitch : public TouchButton, public GFXText
{
public:

  /// Default active colour is green, inactive is red. Override with setColors().
  ToggleSwitch(uint16_t x, uint16_t y, const char* text, TFT_eSPI& tft, uint16_t bgColor, uint16_t txtColor, uint16_t edgeColor) :
    TouchButton(x, y, BTN_WIDTH, BTN_HEIGHT),
    GFXText(tft),
    mTxt(text),
    mBgColor(bgColor),
    mTxtColor(txtColor),
    mEdgeColor(edgeColor)
  {
    mActiveColor = mTFT.color565(0, 200, 0);
    mInactiveColor = mTFT.color565(200, 0, 0);
  }

  /// Overrides the default active (green) and inactive (red) fill colours.
  void setColors(uint16_t activeColor, uint16_t inactiveColor)
  {
    mActiveColor = activeColor;
    mInactiveColor = inactiveColor;
  }

  /// Redraws the switch in its current state with the appropriate fill colour and label.
  void draw()
  {
    clear();
    if (mState) {
      mTFT.fillRoundRect(mX, mY, BTN_WIDTH, BTN_HEIGHT, BTN_ROUND_SIZE, mActiveColor);
    } else {
      mTFT.fillRoundRect(mX, mY, BTN_WIDTH, BTN_HEIGHT, BTN_ROUND_SIZE, mInactiveColor);
    }
    mTFT.drawRoundRect(mX, mY, BTN_WIDTH, BTN_HEIGHT, BTN_ROUND_SIZE, mEdgeColor);
    printTxt(mX + BTN_TEXT_MARGIN_X, mY + BTN_TEXT_MARGIN_Y, mTxt.c_str(), mTxtColor, 2);
  }

  /// Fills the switch area with the background colour.
  void clear()
  {
    mTFT.fillRoundRect(mX, mY, mWidth, mHeight, BTN_ROUND_SIZE, mBgColor);
  }

  /// Flips the state if (x, y) hits the button and returns true when a change occurred.
  bool stateChanged(uint16_t x, uint16_t y)
  {
    if (isPressed(x, y)) {
      setState(!getState());
      return true;
    }
    return false;
  }

  void setState(bool state) { mState = state; }

  bool getState() const { return mState; }

  static const uint16_t BTN_WIDTH = 70;
  static const uint16_t BTN_HEIGHT = 26;
  static const uint16_t BTN_TEXT_MARGIN_X = 10;
  static const uint16_t BTN_TEXT_MARGIN_Y = 5;
  static const uint16_t BTN_ROUND_SIZE = 13;

private:
  String mTxt;
  uint16_t mBgColor;
  uint16_t mTxtColor;
  uint16_t mEdgeColor;
  uint16_t mActiveColor;
  uint16_t mInactiveColor;
  bool mState = false;
};
