// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _DENISE_T_INC
#define _DENISE_T_INC

//
// Enumerations
//

typedef enum : long
{
    DENISE_8362R8, // OCS
    DENISE_8373    // ECS (not supported yet)
}
DeniseRevision;

inline bool isDeniseRevision(long value)
{
    return value >= 0 && value <= DENISE_8373;
}

inline const char *DeniseRevisionName(DeniseRevision type)
{
    assert(isDeniseRevision(type));

    switch (type) {
        case DENISE_8362R8: return "DENISE_8362R8";
        case DENISE_8373:   return "DENISE_8373";
        default:            return "???";
    }
}

typedef enum : long
{
    COLOR_PALETTE = 0,
    BLACK_WHITE_PALETTE,
    PAPER_WHITE_PALETTE,
    GREEN_PALETTE,
    AMBER_PALETTE,
    SEPIA_PALETTE
}
Palette;

inline bool isPalette(long value) {
    return value >= COLOR_PALETTE && value <= SEPIA_PALETTE;
}

typedef enum
{
    MODE_SPF = 0, // Single-playfield mode
    MODE_DPF,     // Dual-playfield mode
    MODE_HAM      // Hold-and-modify mode
}
DrawingMode;

inline bool isDrawingMode(long value) {
    return value >= MODE_SPF && value <= MODE_HAM;
}


//
// Structures
//

typedef struct
{
    int32_t *data;
    bool longFrame;
    bool interlace;
}
ScreenBuffer;

typedef struct
{
    uint16_t pos;
    uint16_t ctl;
    uint32_t ptr;
    int16_t hstrt;
    int16_t vstrt;
    int16_t vstop;
    bool attach;
}
SpriteInfo;

typedef struct
{
    // Emulated chip model
    DeniseRevision revision;

    // Turns sprite emulation on or off
    bool emulateSprites;

    // Checks for sprite-sprite collisions
    bool clxSprSpr;

    // Checks for sprite-playfield collisions
    bool clxSprPlf;

    // Checks for playfield-playfield collisions
    bool clxPlfPlf;
}
DeniseConfig;

typedef struct
{
    uint16_t bplcon0;
    uint16_t bplcon1;
    uint16_t bplcon2;
    int16_t bpu;
    uint16_t bpldat[6];

    uint16_t diwstrt;
    uint16_t diwstop;
    int16_t diwHstrt;
    int16_t diwHstop;
    int16_t diwVstrt;
    int16_t diwVstop;

    uint16_t joydat[2];
    uint16_t clxdat;

    uint16_t colorReg[32];
    uint32_t color[32];
    
    SpriteInfo sprite[8];
}
DeniseInfo;

typedef struct
{
    long spriteLines;
}
DeniseStats;

#endif
