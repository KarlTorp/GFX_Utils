// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

// draw() loads every cell from SD first, then pushes them all to the display
// in a tight loop, so the grid appears all at once instead of painting
// left-to-right.

#include "TouchButton.h"
#include "RawLoader.h"

#define GRID_COLS 3
#define GRID_ROWS 3
#define MAX_PATH_LENGTH 30

/** Touch button grid that renders pre-converted raw RGB565 images */
class RawButtonGrid
{
public:
    RawButtonGrid(uint16_t x, uint16_t y, uint16_t btnWidth, uint16_t btnHeight,
                    RawLoader& loader, uint16_t bgColor = TFT_WHITE) :
        mLoader(loader),
        mX(x),
        mY(y),
        mBtnWidth(btnWidth),
        mBtnHeight(btnHeight),
        mBgColor(bgColor)
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
        static const int CELLS = GRID_ROWS * GRID_COLS;
        uint16_t* thumb[CELLS] = {};
        uint16_t  tw[CELLS] = {};
        uint16_t  th[CELLS] = {};
        const uint32_t bufPixels = (uint32_t)mBtnWidth * mBtnHeight;

        // Load phase: read every non-empty cell from SD before drawing anything.
        for (int i = 0; i < CELLS; ++i) {
            if (mIconPaths[i][0] == '\0') continue;

            thumb[i] = (uint16_t*)malloc(bufPixels * sizeof(uint16_t));
            if (!thumb[i]) continue;

            if (!mLoader.rawLoad(mIconPaths[i], thumb[i], bufPixels, tw[i], th[i])) {
                free(thumb[i]);
                thumb[i] = nullptr;
            }
        }

        // Display phase: push every loaded cell in one rapid sequence, so the
        // whole grid appears simultaneously instead of staggering in.
        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                const int i = r * GRID_COLS + c;
                const uint16_t iconX = mX + c * mBtnWidth;
                const uint16_t iconY = mY + r * mBtnHeight;

                if (thumb[i]) {
                    // The icon always fills the cell exactly, so there's no
                    // need to clear it first.
                    mLoader.rawDisplayBuffer(iconX, iconY, tw[i], th[i], thumb[i]);
                    free(thumb[i]);
                } else {
                    // No icon for this cell (blank slot, or load failed) -
                    // clear it so a stale icon from the previous page doesn't linger.
                    clearButtonArea(iconX, iconY);
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
    RawLoader& mLoader;
    char mIconPaths[GRID_ROWS * GRID_COLS][MAX_PATH_LENGTH];
    uint16_t mX;
    uint16_t mY;
    uint16_t mBtnWidth;
    uint16_t mBtnHeight;
    uint16_t mIconW;
    uint16_t mIconH;
    uint16_t mBgColor;
};
