// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#pragma once

// draw() prefers to buffer every cell and push them all at once (so the grid
// appears in one burst instead of painting left-to-right), but falls back to a
// single reusable buffer, one cell at a time, when the heap can't hold them all
// - see the comment on draw() for why. Either way every cell is drawn.

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
            mIconModes[i] = RawLoader::mode_t::color;
        }
    }

    void setIconPath(int index, const char* path) {
        if (index >= 0 && index < GRID_ROWS * GRID_COLS) {
            strncpy(mIconPaths[index], path, MAX_PATH_LENGTH - 1);
            mIconPaths[index][MAX_PATH_LENGTH - 1] = '\0'; // Ensure null termination
            mIconModes[index] = RawLoader::mode_t::color; // reset; override via setIconMode
        }
    }

    // Optional per-cell recolor (e.g. grayscale for "not yet caught"). Call
    // after setIconPath for that cell, since setIconPath resets the mode.
    void setIconMode(int index, RawLoader::mode_t mode) {
        if (index >= 0 && index < GRID_ROWS * GRID_COLS) {
            mIconModes[index] = mode;
        }
    }

    void clearButtonArea(uint16_t x, uint16_t y) {
        mLoader.getTFT().fillRect(x, y, mBtnWidth, mBtnHeight, mBgColor);
    }

    void clearGridArea() {
        mLoader.getTFT().fillRect(mX, mY, GRID_COLS * mBtnWidth, GRID_ROWS * mBtnHeight, mBgColor);
    }

    // Draws the grid, preferring to hold every cell in RAM at once (so it
    // appears in a single burst) but degrading to one cell at a time when the
    // heap can't spare that much.
    //
    // Nine 80x80 cells is ~112KB held simultaneously, and on the device (heap
    // already carrying the encounter sprite buffers) the last one or two of
    // those allocations fail under fragmentation, which would leave cells 8 and
    // 9 blank. So we *try* the all-at-once path by actually allocating every
    // buffer up front - the only reliable test of whether they fit, since a
    // free-heap check ignores fragmentation - and if any allocation fails we
    // hand it all back and fall back to a single reusable ~12.8KB buffer. The
    // fast path allocates but does no SD reads, so the fallback wastes none.
    void draw() {
        static const int CELLS = GRID_ROWS * GRID_COLS;
        const uint32_t bufPixels = (uint32_t)mBtnWidth * mBtnHeight;

        uint16_t* thumb[CELLS] = {};
        bool haveAll = true;
        for (int i = 0; i < CELLS && haveAll; ++i) {
            if (mIconPaths[i][0] == '\0') continue;   // empty slot needs no buffer
            thumb[i] = (uint16_t*)malloc(bufPixels * sizeof(uint16_t));
            if (!thumb[i]) haveAll = false;
        }

        if (haveAll) {
            drawBatched(thumb, bufPixels);
        } else {
            for (int i = 0; i < CELLS; ++i) free(thumb[i]);   // free(nullptr) is a no-op
            drawSequential(bufPixels);
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
    // Fast path: every non-empty cell already has a buffer. Load each from SD,
    // then push them all in one rapid sequence so the grid appears at once.
    // Frees each buffer as it is drawn.
    void drawBatched(uint16_t** thumb, uint32_t bufPixels) {
        static const int CELLS = GRID_ROWS * GRID_COLS;
        uint16_t tw[CELLS] = {};
        uint16_t th[CELLS] = {};

        for (int i = 0; i < CELLS; ++i) {
            if (thumb[i] && !mLoader.rawLoad(mIconPaths[i], thumb[i], bufPixels, tw[i], th[i])) {
                free(thumb[i]);
                thumb[i] = nullptr;   // load failed - draw it as an empty cell
            }
        }

        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                const int i = r * GRID_COLS + c;
                const uint16_t iconX = mX + c * mBtnWidth;
                const uint16_t iconY = mY + r * mBtnHeight;
                if (thumb[i]) {
                    // The icon fills the cell exactly, so no pre-clear is needed.
                    mLoader.rawDisplayBuffer(iconX, iconY, tw[i], th[i], thumb[i], mIconModes[i]);
                    free(thumb[i]);
                } else {
                    clearButtonArea(iconX, iconY);
                }
            }
        }
    }

    // Fallback: one reusable buffer, a cell at a time. Slower to paint, but its
    // single ~12.8KB allocation fits where nine at once don't.
    void drawSequential(uint32_t bufPixels) {
        uint16_t* thumb = (uint16_t*)malloc(bufPixels * sizeof(uint16_t));

        for (int r = 0; r < GRID_ROWS; ++r) {
            for (int c = 0; c < GRID_COLS; ++c) {
                const int i = r * GRID_COLS + c;
                const uint16_t iconX = mX + c * mBtnWidth;
                const uint16_t iconY = mY + r * mBtnHeight;

                uint16_t tw = 0, th = 0;
                if (thumb && mIconPaths[i][0] != '\0' &&
                    mLoader.rawLoad(mIconPaths[i], thumb, bufPixels, tw, th)) {
                    mLoader.rawDisplayBuffer(iconX, iconY, tw, th, thumb, mIconModes[i]);
                } else {
                    // Empty slot, load failure, or no buffer - clear it so a
                    // stale icon from the previous page doesn't linger.
                    clearButtonArea(iconX, iconY);
                }
            }
        }

        free(thumb);
    }

    RawLoader& mLoader;
    char mIconPaths[GRID_ROWS * GRID_COLS][MAX_PATH_LENGTH];
    RawLoader::mode_t mIconModes[GRID_ROWS * GRID_COLS];
    uint16_t mX;
    uint16_t mY;
    uint16_t mBtnWidth;
    uint16_t mBtnHeight;
    uint16_t mIconW;
    uint16_t mIconH;
    uint16_t mBgColor;
};
