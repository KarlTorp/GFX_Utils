# GFX_Utils

C++ classes for various touch-based user interface components that utilize TFT_eSPI.

## Image loading: BMP vs. raw

The library has two independent families of classes for drawing images from
an SD card, both built directly on TFT_eSPI and the [SdFat](https://github.com/greiman/SdFat)
library (`#include <SdFat.h>`) — there's no filesystem-abstraction layer;
`BMPLoader`/`RawLoader` open files via a `SdFat&` you pass in and manage
their own `SdBaseFile` handle internally.

| | `BMPLoader` / `BmpButton` / `BmpButtonGrid` | `RawLoader` / `RawButton` / `RawButtonGrid` |
|---|---|---|
| Source format | Standard 24-bit BMP | Pre-converted raw RGB565 (see below) |
| Decode cost | Per-pixel BGR→RGB565 conversion at draw time | None — pixels are already display-native |
| Speed | Baseline | Significantly faster (no per-pixel work, optional DMA) |
| Color modes | `color` / `grayscale` / `shade` | `color` / `grayscale` / `shade` |
| Use when | You want to draw ordinary `.bmp` files directly, no build step | You can afford a one-time image-conversion step and want the fastest possible draw |

They're kept as separate classes rather than one class doing both jobs,
since the two file formats and drawing strategies don't share meaningful
implementation. Pick whichever family fits a given project; both are
maintained.

### Raw file format

Produced by `tools/convert_to_raw.py`:

```
Offset  Size   Description
0       2      uint16_t width  (little-endian)
2       2      uint16_t height (little-endian)
4       w*h*2  RGB565 pixels, big-endian, top-to-bottom, left-to-right
```

Big-endian pixel storage matches what the display itself expects, so
`RawLoader::rawDraw()`/`rawDrawDMA()` stream bytes straight from the SD card
to the TFT with no per-pixel conversion and no byte-swap.

### Hardware requirements for `rawDrawDMA()` / `RawButton` / `RawButtonGrid`

- Call `tft.initDMA()` once in `setup()` before any raw DMA draw — `pushPixelsDMA()`
  silently no-ops without it.
- For the double-buffered parallelism `rawDrawDMA()` relies on (reading the
  next SD chunk while the previous one streams to the display), the TFT and
  SD card need to be on **separate SPI peripherals** (e.g. TFT on HSPI via
  `TFT_eSPI`'s `USE_HSPI_PORT`, SD on a separately-instantiated `SPIClass(VSPI)`).
  It still works correctly if they share a bus, just without the parallelism.
- `rawDraw()` (non-DMA) and `rawLoad()`/`rawDisplayBuffer()` have no such
  requirement and work on any setup.

### Converting images

```bash
cd tools
python3 -m venv env          # one-time setup
./env/bin/pip install -r requirements.txt

# Single file
./env/bin/python3 convert_to_raw.py photo.bmp photo.raw

# Every BMP/PNG/JPG in a folder (batch), mirrored into an output folder
./env/bin/python3 convert_to_raw.py images/ sd_card/rawImg/

# Resize while converting (e.g. to exactly 80x80 icons)
./env/bin/python3 convert_to_raw.py photo.bmp photo.raw --size 80 80
```

Pixel packing is vectorized with numpy, so converting a few thousand images
(a full SD card's worth of icons and sprites) takes seconds rather than
minutes — see the script's own docstring for details.

### API summary

```cpp
SdFat sd;
sd.begin(...);                      // however you mount your SD card

RawLoader loader(tft, sd);
loader.setServiceCallback(yield);   // called periodically during long transfers

// Single image, simplest form (no DMA):
loader.rawDraw("/icons/1.raw", x, y);

// Single image, DMA double-buffered (fastest, needs tft.initDMA()):
loader.rawDrawDMA("/icons/1.raw", x, y);

// Grayscale/shade recoloring (e.g. a "disabled" icon state):
loader.rawDrawDMA("/icons/1.raw", x, y, RawLoader::mode_t::grayscale);

// Load into a buffer without displaying, to batch multiple images:
uint16_t buf[80 * 80];
uint16_t w, h;
if (loader.rawLoad("/icons/1.raw", buf, 80 * 80, w, h)) {
    loader.rawDisplayBuffer(x, y, w, h, buf);
}
```

`RawButton` and `RawButtonGrid` wrap this into ready-made touch UI components
(same shape as `BmpButton`/`BmpButtonGrid`, but drawing via `RawLoader`).
`RawButtonGrid::draw()` in particular loads every cell from SD first, then
displays them all in one rapid sequence — the whole grid appears at once
instead of painting left-to-right.

If your project already wraps `SdFat` in its own class for mounting/CSV
access/etc. rather than using it bare, just expose the underlying `SdFat`
instance (e.g. a `getSdFat()` accessor returning `SdFat&`) and pass that
through to `BMPLoader`/`RawLoader` — they only need the reference, not
ownership, so both your wrapper and the loader operate on the same
filesystem instance.
