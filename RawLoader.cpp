// Copyright (c) 2026 Karl Kristian Dyrholm Torp

#include "RawLoader.h"

// Chunk size (pixels) for rawDraw streaming — matches SD 512-byte block size.
#define RAW_CHUNK_PIXELS 256

RawLoader::RawLoader(TFT_eSPI& tft, SdFat& sd) :
mTFT(tft),
mSd(sd)
{
}

// ---------------------------------------------------------------------------
// applyMode
// ---------------------------------------------------------------------------
// Raw files store big-endian RGB565 (display-native, matching how
// pushColors/pushPixelsDMA send bytes as-is when _swapBytes is disabled).
// Read into a uint16_t buffer on a little-endian MCU, each value is
// byte-swapped relative to its true RGB565 number - so recoloring needs to
// un-swap, unpack the 5/6/5 fields, apply the same luma/shade formulas
// BMPLoader::bmpDraw uses, repack, and swap back before the pixels go out
// over SPI/DMA.
// ---------------------------------------------------------------------------
void RawLoader::applyMode(uint16_t *buf, uint32_t count, mode_t mode)
{
  for (uint32_t i = 0; i < count; ++i) {
    const uint16_t v = (buf[i] << 8) | (buf[i] >> 8);   // un-swap to true RGB565

    const uint8_t r5 = (v >> 11) & 0x1F;
    const uint8_t g6 = (v >> 5) & 0x3F;
    const uint8_t b5 = v & 0x1F;
    const uint8_t r8 = (r5 << 3) | (r5 >> 2);
    const uint8_t g8 = (g6 << 2) | (g6 >> 4);
    const uint8_t b8 = (b5 << 3) | (b5 >> 2);

    uint16_t out;
    if (mode == mode_t::grayscale) {
      const uint8_t gray = (r8 * 77u + g8 * 150u + b8 * 29u) >> 8;
      out = mTFT.color565(gray, gray, gray);
    } else {   // shade: pure white stays white, everything else goes black
      const bool white = (r8 == 0xFF && g8 == 0xFF && b8 == 0xFF);
      out = white ? mTFT.color565(0xFF, 0xFF, 0xFF) : mTFT.color565(0, 0, 0);
    }

    buf[i] = (out << 8) | (out >> 8);   // swap back for the wire
  }
}

// ---------------------------------------------------------------------------
// rawDraw
// ---------------------------------------------------------------------------
// Loads a pre-converted .raw file produced by tools/convert_to_raw.py.
// File layout:
//   Bytes 0-1 : image width  (uint16_t, little-endian)
//   Bytes 2-3 : image height (uint16_t, little-endian)
//   Bytes 4.. : w*h big-endian RGB565 pixels, top-to-bottom, left-to-right
//
// No BGR->RGB565 conversion. Streams RAW_CHUNK_PIXELS pixels per SD read
// and pushes them directly to the display.
// ---------------------------------------------------------------------------
void RawLoader::rawDraw(const char *filename, uint8_t x, uint16_t y, mode_t mode)
{
  if (!(mFile = mSd.open(filename, O_READ))) return;

  uint8_t hdr[4];
  mFile.read(hdr, sizeof(hdr));
  const uint16_t w = (uint16_t)hdr[0] | ((uint16_t)hdr[1] << 8);
  const uint16_t h = (uint16_t)hdr[2] | ((uint16_t)hdr[3] << 8);

  if (w == 0 || h == 0 || (x + w) > (uint16_t)mTFT.width() || (y + h) > (uint16_t)mTFT.height()) {
    mFile.close();
    return;
  }

  mTFT.setAddrWindow(x, y, w, h);

  uint16_t chunkBuf[RAW_CHUNK_PIXELS];
  uint32_t remaining = (uint32_t)w * h;

  while (remaining > 0) {
    const uint32_t count = remaining < RAW_CHUNK_PIXELS ? remaining : RAW_CHUNK_PIXELS;
    mFile.read((uint8_t *)chunkBuf, count * 2);

    if (mode != mode_t::color) {
      applyMode(chunkBuf, count, mode);
    }

    // Data is stored big-endian (display-native) by the converter — no swap.
    mTFT.pushColors(chunkBuf, count, false);
    remaining -= count;

    if (mServiceCallback) mServiceCallback();
  }

  mFile.close();
}

// ---------------------------------------------------------------------------
// rawDrawDMA
// ---------------------------------------------------------------------------
// Double-buffered DMA version of rawDraw.
// While DMA streams buffer[A] to the TFT over HSPI, the CPU simultaneously
// fills buffer[B] from the SD card over VSPI — true hardware parallelism.
// The two SPI buses must be separate (VSPI for SD, HSPI for TFT), which is
// the default TFT_eSPI / SdFat configuration on ESP32.
// ---------------------------------------------------------------------------
#define DMA_CHUNK_PIXELS 8192  // 16 KB per buffer — 6 SD transactions for 215x215

void RawLoader::rawDrawDMA(const char *filename, uint8_t x, uint16_t y, mode_t mode)
{
  if (!(mFile = mSd.open(filename, O_READ))) return;

  uint8_t hdr[4];
  mFile.read(hdr, sizeof(hdr));
  const uint16_t w = (uint16_t)hdr[0] | ((uint16_t)hdr[1] << 8);
  const uint16_t h = (uint16_t)hdr[2] | ((uint16_t)hdr[3] << 8);

  if (w == 0 || h == 0 ||
      (x + w) > (uint16_t)mTFT.width() ||
      (y + h) > (uint16_t)mTFT.height()) {
    mFile.close();
    return;
  }

  // Two ping-pong buffers in DMA-capable heap (internal SRAM).
  uint16_t *buf[2];
  buf[0] = (uint16_t *)malloc(DMA_CHUNK_PIXELS * sizeof(uint16_t));
  buf[1] = (uint16_t *)malloc(DMA_CHUNK_PIXELS * sizeof(uint16_t));
  if (!buf[0] || !buf[1]) {
    free(buf[0]);
    free(buf[1]);
    mFile.close();
    rawDraw(filename, x, y, mode);  // graceful fallback (reopens file internally)
    return;
  }

  mTFT.startWrite();
  mTFT.setAddrWindow(x, y, w, h);

  uint32_t remaining  = (uint32_t)w * h;
  uint8_t  active     = 0;
  bool     hasPending = false;

  while (remaining > 0) {
    const uint32_t count = remaining < DMA_CHUNK_PIXELS ? remaining : DMA_CHUNK_PIXELS;

    // Read next chunk from SD into the active buffer.
    // This overlaps in time with the DMA transfer of the previous buffer
    // because SD (VSPI) and TFT DMA (HSPI) run on separate SPI peripherals.
    mFile.read((uint8_t *)buf[active], count * 2);

    if (mode != mode_t::color) {
      applyMode(buf[active], count, mode);
    }

    // Ensure the previous DMA transfer is done before handing DMA a new buffer.
    if (hasPending) mTFT.dmaWait();

    // Hand the freshly-read buffer to DMA — returns immediately.
    mTFT.pushPixelsDMA(buf[active], count);
    hasPending = true;

    active ^= 1;   // swap to the other buffer
    remaining -= count;

    if (mServiceCallback) mServiceCallback();
  }

  if (hasPending) mTFT.dmaWait();  // wait for the final chunk
  mTFT.endWrite();

  free(buf[0]);
  free(buf[1]);
  mFile.close();
}

// ---------------------------------------------------------------------------
// rawLoad
// ---------------------------------------------------------------------------
// Reads a raw file into a caller-provided buffer. Does NOT display anything.
// Use rawDisplayBuffer() to show the image after all thumbnails are loaded.
// Returns true on success. buf must be at least outW*outH uint16_t values.
// ---------------------------------------------------------------------------
bool RawLoader::rawLoad(const char *filename, uint16_t *buf, uint32_t bufPixels,
                         uint16_t &outW, uint16_t &outH)
{
  outW = outH = 0;
  if (!buf || !(mFile = mSd.open(filename, O_READ))) return false;

  uint8_t hdr[4];
  mFile.read(hdr, sizeof(hdr));
  outW = (uint16_t)hdr[0] | ((uint16_t)hdr[1] << 8);
  outH = (uint16_t)hdr[2] | ((uint16_t)hdr[3] << 8);

  if (outW == 0 || outH == 0 || (uint32_t)outW * outH > bufPixels) {
    mFile.close();
    outW = outH = 0;
    return false;
  }

  mFile.read((uint8_t *)buf, (uint32_t)outW * outH * 2);
  mFile.close();
  return true;
}

// ---------------------------------------------------------------------------
// rawDisplayBuffer
// ---------------------------------------------------------------------------
// Sends a buffer previously filled by rawLoad() to the display via DMA.
// ---------------------------------------------------------------------------
void RawLoader::rawDisplayBuffer(uint8_t x, uint16_t y, uint16_t w, uint16_t h,
                                  uint16_t *buf, mode_t mode)
{
  if (!buf || w == 0 || h == 0) return;
  if (mode != mode_t::color) {
    applyMode(buf, (uint32_t)w * h, mode);
  }
  mTFT.startWrite();
  mTFT.setAddrWindow(x, y, w, h);
  mTFT.pushPixelsDMA(buf, (uint32_t)w * h);
  mTFT.dmaWait();
  mTFT.endWrite();
}
