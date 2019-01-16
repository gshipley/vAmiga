// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

DMAController::DMAController()
{
    setDescription("DMAController");
}

DMAController::~DMAController()
{
    debug(2, "Destroying DMAController[%p]\n", this);
}

void
DMAController::_powerOn()
{
    
}
void
DMAController::_powerOff()
{
    
}
void
DMAController::_reset()
{
    
}
void
DMAController::_ping()
{
    
}
void
DMAController::_dump()
{
    
}

void
DMAController::fakeSomething()
{
    xCounter++;
    if (xCounter > Denise::HPIXELS) {
        xCounter = 0;
        yCounter++;
        
        if (yCounter > Denise::VPIXELS) {
            yCounter = 0;
            amiga->denise.fakeFrame();
        }
    }
}