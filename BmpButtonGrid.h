// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

#include "TouchButton.h"
#include "BMPLoader.h"

#define GRID_COLS 3
#define GRID_ROWS 3
#define MAX_PATH_LENGTH 30

/** Touch button that renders bmp images */
class BmpButtonGrid
{
public:
    BmpButtonGrid(uint16_t x, uint16_t y, uint16_t btnWidth, uint16_t btnHeight,
                    BMPLoader& loader, uint16_t bgColor = TFT_WHITE, bool clearBg = false) :
        mLoader(loader),
        mX(x),
        mY(y),
        mBtnWidth(btnWidth),
        mBtnHeight(btnHeight),
        mBgColor(bgColor),
        mClearBg(clearBg)
    {
        // Initialize icon paths to nullptr
        for (int i = 0; i < GRID_ROWS * GRID_COLS; ++i) {
            mIconPaths[i][0] = '\0'; // Set first character to null terminator
        }
    }
    
    void setIconPath(int index, const char* path) {
        if (index >= 0 && index < GRID_ROWS * GRID_COLS) {
            strncpy(mIconPaths[index], path, MAX_PATH_LENGTH - 1);
            mIconPaths[index][MAX_PATH_LENGTH - 1] = '\0'; // Ensure null termination
        }
    }

    void clearButtonArea(uint16_t x, uint16_t y) {
        mLoader.getTFT().fillRect(x, y, mBtnWidth, mBtnHeight, mBgColor);
    }

    void clearGridArea() {
        mLoader.getTFT().fillRect(mX, mY, GRID_COLS * mBtnWidth, GRID_ROWS * mBtnHeight, mBgColor);
    }
    
    void draw() {
        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                const char* iconPath = mIconPaths[r * GRID_COLS + c];
                if (iconPath) {
                const uint16_t iconX = mX + c * mBtnWidth + (mBtnWidth - mBtnWidth) / 2;
                const uint16_t iconY = mY + r * mBtnHeight + (mBtnHeight - mBtnHeight) / 2;
                if (mClearBg) {
                    clearButtonArea(iconX, iconY);
                }
                mLoader.bmpDrawBuffered(iconPath, iconX, iconY);
                }
            }
        }
    }

    bool isPressed(uint16_t x, uint16_t y) {
        if (x >= mX && x < (mX + GRID_COLS * mBtnWidth) && y >= mY && y < (mY + GRID_ROWS * mBtnHeight)) {
            return true;
        }
        return false;
    }

    int16_t getPressedButtonIndex(uint16_t x, uint16_t y) {
        if (isPressed(x, y)) {
            int col = (x - mX) / mBtnWidth;
            int row = (y - mY) / mBtnHeight;
            return row * GRID_COLS + col;
        }
        return -1; // No button pressed
    }

private:
    BMPLoader& mLoader;
    char mIconPaths[GRID_ROWS * GRID_COLS][MAX_PATH_LENGTH];
    uint16_t mX;
    uint16_t mY;
    uint16_t mBtnWidth;
    uint16_t mBtnHeight;
    uint16_t mIconW;
    uint16_t mIconH;
    uint16_t mBgColor;
    bool mClearBg;
};