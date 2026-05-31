// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include "TouchButton.h"
#include "GFXText.h"

/** Square checkbox with a two-line tick mark when checked and an inline text label to the right. */
class CheckBox : public TouchButton, public GFXText
{
public:

  /// @param boxColor  Colour used for the box outline and tick mark.
  CheckBox(uint16_t x, uint16_t y, const char* text, TFT_eSPI& tft,
           uint16_t bgColor, uint16_t txtColor, uint16_t boxColor) :
    TouchButton(x, y, BOX_SIZE, BOX_SIZE),
    GFXText(tft),
    mTxt(text),
    mBgColor(bgColor),
    mTxtColor(txtColor),
    mBoxColor(boxColor),
    mState(false) {}

  /// Draws the box outline, tick mark (when checked), and the text label.
  void draw()
  {
    clear();
    mTFT.drawRect(mX, mY, BOX_SIZE, BOX_SIZE, mBoxColor);

    if (mState)
    {
      const uint16_t midX = mX + BOX_SIZE / 2;
      const uint16_t midY = mY + BOX_SIZE - 4;
      mTFT.drawLine(mX + 3, mY + BOX_SIZE / 2,     midX,              midY,     mBoxColor);
      mTFT.drawLine(mX + 4, mY + BOX_SIZE / 2,     midX,              midY - 1, mBoxColor);
      mTFT.drawLine(midX,   midY,                   mX + BOX_SIZE - 3, mY + 4,   mBoxColor);
      mTFT.drawLine(midX,   midY - 1,               mX + BOX_SIZE - 3, mY + 3,   mBoxColor);
    }

    printTxt(mX + BOX_SIZE + LABEL_GAP, mY + (BOX_SIZE - 8) / 2, mTxt.c_str(), mTxtColor, 1);
  }

  /// Fills the box area with the background colour.
  void clear()
  {
    mTFT.fillRect(mX, mY, BOX_SIZE, BOX_SIZE, mBgColor);
  }

  /// Toggles the state if (x, y) hits the box and returns true when a change occurred.
  bool stateChanged(uint16_t x, uint16_t y)
  {
    if (isPressed(x, y)) {
      mState = !mState;
      return true;
    }
    return false;
  }

  void setState(bool state) { mState = state; }
  bool getState() const     { return mState; }

  static const uint16_t BOX_SIZE  = 20;
  static const uint16_t LABEL_GAP = 6;

private:
  String   mTxt;
  uint16_t mBgColor;
  uint16_t mTxtColor;
  uint16_t mBoxColor;
  bool     mState;
};
