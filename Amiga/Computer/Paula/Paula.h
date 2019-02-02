// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _PAULA_INC
#define _PAULA_INC

#include "AudioUnit.h"

//
// THIS CLASS IS A STUB TO MAKE THE VISUAL PROTOTYPE WORK
//

class Paula : public HardwareComponent {
    
public:
    
    // Paula has been executed up to this clock cycle.
    uint64_t clock = 0;
    
    // Audio unit
    AudioUnit audioUnit;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    Paula();
    
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
    void _powerOn() override;
    /*
    void _powerOff() override;
    void _reset() override;
    void _ping() override;
    void _dump() override;
    */
    void _setWarp(bool value) override;
    
    //
    // FAKE METHODS FOR THE VISUAL PROTOTYPE (TEMPORARY)
    //
    
public:
    
    /*
    uint32_t getVolume() { return 42; }
    long bufferUnderflows() { return 42; }
    long bufferOverflows() { return 42; }
    double fillLevel() { return .5; }
     */
};

#endif