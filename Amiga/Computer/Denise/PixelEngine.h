// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _COLORIZER_INC
#define _COLORIZER_INC

#include "AmigaComponent.h"
#include "ChangeRecorder.h"

class PixelEngine : public AmigaComponent {

    friend class DmaDebugger;
    
public:

    // RGBA colors used to visualize the HBLANK and VBLANK area in the debugger
    static const int32_t rgbaHBlank = 0x00444444;
    static const int32_t rgbaVBlank = 0x00444444;

private:

    //
    // Screen buffers
    //

    /* We keep four frame buffers, two for storing long frames and
     * another two for storing short frames. The short frame buffers are only
     * used in interlace mode. At each point in time, one of the two buffers
     * is the "working buffer" and the other one the "stable buffer". All
     * drawing functions write to the working buffers, only. The GPU reads from
     * the stable buffers, only. Once a frame has been completed, the working
     * buffer and the stable buffer are switched.
     */
    ScreenBuffer longFrame[2];
    ScreenBuffer shortFrame[2];

    // Pointers to the working buffers
    ScreenBuffer *workingLongFrame = &longFrame[0];
    ScreenBuffer *workingShortFrame = &shortFrame[0];

    // Pointers to the stable buffers
    ScreenBuffer *stableLongFrame = &longFrame[0];
    ScreenBuffer *stableShortFrame = &shortFrame[0];

    // Pointer to the frame buffer Denise is currently working on
    ScreenBuffer *frameBuffer = &longFrame[0];

    // Buffer storing background noise (random black and white pixels)
    int32_t *noise;

    //
    // Color management
    //

    // The 32 Amiga color registers
    uint16_t colreg[32];

    // RGBA values for all possible 4096 Amiga colors
    uint32_t rgba[4096];

    /* The color register values translated to RGBA
     * Note that the number of elements exceeds the number of color registers:
     *  0 .. 31: RGBA values of the 32 color registers.
     * 32 .. 63: RGBA values of the 32 color registers in halfbright mode.
     * 64 .. 71: Additional colors used for debugging
     */
    static const int rgbaIndexCnt = 32 + 32 + 8;
    uint32_t indexedRgba[rgbaIndexCnt];

    // Color adjustment parameters
    Palette palette = COLOR_PALETTE;
    double brightness = 50.0;
    double contrast = 100.0;
    double saturation = 1.25;


    // The current drawing mode
    DrawingMode mode;
    

    //
    // Register change history buffer
    //

public:

    // Color register history
    ChangeRecorder<128> colRegChanges; 

    //
    // Constructing and destructing
    //
    
public:
    
    PixelEngine(Amiga& ref);
    ~PixelEngine();


    //
    // Iterating over snapshot items
    //

    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & colRegChanges
        & colreg
        & mode;
    }


    //
    // Methods from HardwareComponent
    //
    
private:

    void _powerOn() override;
    void _reset() override;
    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(uint8_t *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(uint8_t *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Configuring the color palette
    //
    
public:
    
    Palette getPalette() { return palette; }
    void setPalette(Palette p);
    
    double getBrightness() { return brightness; }
    void setBrightness(double value);
    
    double getSaturation() { return saturation; }
    void setSaturation(double value);
    
    double getContrast() { return contrast; }
    void setContrast(double value);


    //
    // Accessing color registers
    //

public:

    // Performs a consistency check for debugging.
    static bool isRgbaIndex(int nr) { return nr < rgbaIndexCnt; }
    
    // Changes one of the 32 Amiga color registers.
    void setColor(int reg, uint16_t value);

    // Returns a color value in Amiga format or RGBA format
    uint16_t getColor(int nr) { assert(nr < 32); return colreg[nr]; }
    uint32_t getRGBA(int nr) { assert(nr < 32); return indexedRgba[nr]; }

    // Returns sprite color in Amiga format or RGBA format
    uint16_t getSpriteColor(int s, int nr) { assert(s < 8); return getColor(16 + nr + 2 * (s & 6)); }
    uint32_t getSpriteRGBA(int s, int nr) { return rgba[getSpriteColor(s,nr)]; }


    //
    // Using the color lookup table
    //

private:

    // Updates the entire RGBA lookup table
    void updateRGBA();

    // Adjusts the RGBA value according to the selected color parameters
    void adjustRGB(uint8_t &r, uint8_t &g, uint8_t &b);


    //
    // Working with frame buffers
    //

private:

    // Return true if buffer points to a long frame screen buffer
    bool isLongFrame(ScreenBuffer *buf);

    // Return true if buffer points to a short frame screen buffer
    bool isShortFrame(ScreenBuffer *buf);

public:

    // Returns the stable frame buffer for long frames
    ScreenBuffer getStableLongFrame();

    // Returns the stable frame buffer for short frames
    ScreenBuffer getStableShortFrame();

    // Returns a pointer to randon noise
    int32_t *getNoise();

    // Returns the frame buffer address of a certain pixel in the current line
    int *pixelAddr(int pixel);

    // Called after each line in the VBLANK area
    void endOfVBlankLine();

    // Called after each frame to switch the frame buffers
    void beginOfFrame(bool interlace);


    //
    // Working with recorded register changes
    //

public:

    // Applies a register change
    void applyRegisterChange(const Change &change);


    //
    // Synthesizing pixels
    //

public:

    /* Colorizes a rasterline.
     * This function implements the last stage in the emulator's graphics
     * pipelile. It translates a line of color register indices into a line
     * of RGBA values in GPU format.
     */
    void colorize(int line);

private:

    void colorize(int *dst, int from, int to);
    void colorizeHAM(int *dst, int from, int to, uint16_t& ham);

};

#endif
