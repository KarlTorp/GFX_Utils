// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#include "BMPLoader.h"

#define BUFFPIXEL 32

#ifdef ESP8266
#define LCDBUFFERSIZE 15
#else
//#define LCDBUFFERSIZE 31
#define LCDBUFFERSIZE 240
#endif

uint8_t sdbuffer[BUFFPIXEL*6];   // pixel buffer (R+G+B per pixel)
uint16_t lcdbuffer[LCDBUFFERSIZE];   // pixel out buffer (16-bit per pixel)

BMPLoader::BMPLoader(TFT_eSPI& tft, SdFat& sd) :
mTFT(tft),
mSd(sd)
{

}

inline uint32_t BMPLoader::extract32(uint8_t *data)
{
  return data[0] | data[1] << 8 | data[2] << 16 | data[3] << 24;
}

inline uint16_t BMPLoader::extract16(uint8_t *data)
{
  return data[0] | data[1] << 8;
}

BMPLoader::bmpHeader BMPLoader::readBmpHeader()
{
  BMPLoader::bmpHeader header;
  header.valid = false;

  uint8_t buffer[34];
  mFile.read((uint8_t *) buffer, sizeof(buffer));

  if (buffer[0] == 'B' && buffer[1] == 'M')   // BMP Signature.
  {
    header.imageoffset = extract32(&buffer[10]);
    header.w = extract32(&buffer[18]);
    header.h = extract32(&buffer[22]);
    uint16_t planes = extract16(&buffer[26]);
    uint16_t depth = extract16(&buffer[28]);
    uint32_t compressed = extract32(&buffer[30]);
    if (planes == 1 && depth == 24 && compressed == 0) {
      header.valid = true;
    }
  }

  return header;
}

void BMPLoader::bmpDraw(const char *filename, uint8_t x, uint16_t y, mode_t mode)
{
  uint32_t rowSize;                  // Not always = bmpWidth; may have padding
  uint8_t buffidx = sizeof(sdbuffer);   // Current position in sdbuffer
  boolean goodBmp = false;              // Set to true on valid header parse
  boolean flip = true;                  // BMP is stored bottom-to-top
  int w, h, row, col;
  uint8_t r, g, b;
  uint32_t pos = 0, startTime = millis();

  if ((x >= mTFT.width()) || (y >= mTFT.height()))
    return;

  // Open requested file on SD card
  if (!(mFile = mSd.open(filename, O_READ))) {
    return;
  }

  uint8_t lcdidx = 0;
  BMPLoader::bmpHeader header = readBmpHeader();

  // Parse BMP header
  if (header.valid) {   // 0 = uncompressed
    goodBmp = true;     // Supported BMP format -- proceed!
    rowSize = (header.w * 3 + 3) & ~3;

    if (header.h < 0) {
      header.h = -header.h;
      flip = false;
    }

    w = header.w;
    h = header.h;
    if ((x + w - 1) >= mTFT.width())
      w = mTFT.width() - x;
    if ((y + h - 1) >= mTFT.height())
      h = mTFT.height() - y;

    mTFT.setAddrWindow(x, y, w, h);

    for (row = 0; row < h; row++) {   // For each scanline...
      if (flip)   // Bitmap is stored bottom-to-top order (normal BMP)
        pos = header.imageoffset + (header.h - 1 - row) * rowSize;
      else   // Bitmap is stored top-to-bottom
        pos = header.imageoffset + row * rowSize;

      if (mFile.curPosition() != pos) {   // Need seek?
        mFile.seekSet(pos);
        buffidx = sizeof(sdbuffer);   // Force buffer reload
      }

      for (col = 0; col < w; col++) {        // For each pixel...
        if (buffidx >= sizeof(sdbuffer)) {   // Indeed
          mFile.read(sdbuffer, sizeof(sdbuffer));
          buffidx = 0;   // Set index to beginning
        }
        b = sdbuffer[buffidx++];
        g = sdbuffer[buffidx++];
        r = sdbuffer[buffidx++];

        switch(mode)
        {
          case mode_t::color:
            break;
          case mode_t::grayscale:
          {
            //const uint8_t gray = 0.2989 * r+ 0.5870 * g+ 0.1140 * b;
            const uint8_t gray = (r * 77 + g * 150 + b * 29) >> 8;
            r = gray;
            g = gray;
            b = gray;
          }
            break;
          case mode_t::shade:
            if(r != 0xFF || g != 0xFF || b != 0xFF)
            {
              r = 0;
              g = 0;
              b = 0;
            }
            break;
        }

        lcdbuffer[lcdidx++] = mTFT.color565(r, g, b);
        if (lcdidx == LCDBUFFERSIZE) {
          mTFT.pushColors(lcdbuffer, lcdidx, true);
          lcdidx = 0;
          if (mServiceCallback) {
            mServiceCallback();
          }
        }
      }   // end pixel

    }   // end scanline

    if (lcdidx > 0) {
      mTFT.pushColors(lcdbuffer, lcdidx, true);
    }
  }
  mFile.close();
}

// ---------------------------------------------------------------------------
// bmpDrawBuffered
// ---------------------------------------------------------------------------
// Key difference from bmpDraw:
//   - Seeks to imageoffset ONCE, then reads every BMP row sequentially.
//     (Standard bmpDraw seeks backward per row because BMP is bottom-to-top.)
//   - Converts each row into the correct display position of a heap buffer.
//   - Sends the whole image with a single pushImage() call.
// RAM cost: w * h * 2 bytes  (~92 KB for 215x215). Falls back to bmpDraw()
// if malloc fails (e.g., fragmented heap).
// ---------------------------------------------------------------------------
void BMPLoader::bmpDrawBuffered(const char *filename, uint8_t x, uint16_t y, mode_t mode)
{
  if (!(mFile = mSd.open(filename, O_READ))) return;

  BMPLoader::bmpHeader header = readBmpHeader();
  if (!header.valid) {
    mFile.close();
    return;
  }

  bool flip = true;
  if (header.h < 0) {
    header.h = -header.h;
    flip = false;
  }

  int w = header.w;
  int h = header.h;
  if ((x + w - 1) >= mTFT.width())  w = mTFT.width() - x;
  if ((y + h - 1) >= mTFT.height()) h = mTFT.height() - y;

  // rowSize includes 4-byte padding required by the BMP format.
  const uint32_t rowSize = ((uint32_t)header.w * 3 + 3) & ~3u;

  uint16_t *imgBuf = (uint16_t *)malloc((uint32_t)w * h * sizeof(uint16_t));
  uint8_t  *rowBuf = (uint8_t  *)malloc(rowSize);

  if (!imgBuf || !rowBuf) {
    // Not enough contiguous heap — fall back to the row-by-row method.
    free(imgBuf);
    free(rowBuf);
    mFile.close();
    bmpDraw(filename, x, y, mode);
    return;
  }

  // One sequential pass: seek to pixel data once, then read forward.
  mFile.seekSet(header.imageoffset);

  for (int fileRow = 0; fileRow < h; fileRow++) {
    mFile.read(rowBuf, rowSize);

    // BMP bottom-to-top: file row 0 = bottom of image = last display row.
    const int displayRow = flip ? (h - 1 - fileRow) : fileRow;
    uint16_t *dest = imgBuf + (uint32_t)displayRow * w;

    for (int col = 0; col < w; col++) {
      uint8_t b = rowBuf[col * 3];
      uint8_t g = rowBuf[col * 3 + 1];
      uint8_t r = rowBuf[col * 3 + 2];

      switch (mode) {
        case mode_t::grayscale: {
          const uint8_t gray = (r * 77u + g * 150u + b * 29u) >> 8;
          r = g = b = gray;
          break;
        }
        case mode_t::shade:
          if (r != 0xFF || g != 0xFF || b != 0xFF) r = g = b = 0;
          break;
        default:
          break;
      }
      dest[col] = mTFT.color565(r, g, b);
    }

    if (mServiceCallback) mServiceCallback();
  }

  free(rowBuf);
  mTFT.setAddrWindow(x, y, w, h);
  mTFT.pushColors(imgBuf, (uint32_t)w * h, true);
  free(imgBuf);
  mFile.close();
}