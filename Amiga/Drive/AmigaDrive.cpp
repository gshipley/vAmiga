// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

/*
AmigaDrive::AmigaDrive()
{
    setDescription("Drive");
}
*/

AmigaDrive::AmigaDrive(unsigned nr)
{
    assert(nr == 0 /* df0 */ || nr == 1 /* df1 */);
    
    this->nr = nr;
    setDescription(nr == 0 ? "Df0" : "Df1");
}

void
AmigaDrive::_powerOn()
{
    
}

void
AmigaDrive::_powerOff()
{
    
}

void
AmigaDrive::_reset()
{
    
}

void
AmigaDrive::_ping()
{
    debug("AmigaDrive::_ping()\n");

    amiga->putMessage(isConnected() ?
                      MSG_DRIVE_CONNECT : MSG_DRIVE_DISCONNECT, nr);
    amiga->putMessage(hasDisk() ?
                      MSG_DRIVE_DISK_INSERT : MSG_DRIVE_DISK_EJECT, nr);
    amiga->putMessage(hasWriteProtectedDisk() ?
                      MSG_DRIVE_DISK_PROTECTED : MSG_DRIVE_DISK_UNPROTECTED, nr);
    amiga->putMessage(hasModifiedDisk() ?
                      MSG_DRIVE_DISK_UNSAVED : MSG_DRIVE_DISK_SAVED, nr);
}

void
AmigaDrive::_dump()
{
    msg("Has disk: %s\n", hasDisk() ? "yes" : "no");
}

void
AmigaDrive::setConnected(bool value)
{
    if (connected != value) {
        
        connected = value;
        amiga->putMessage(connected ? MSG_DRIVE_CONNECT : MSG_DRIVE_DISCONNECT, nr);
    }
}

void
AmigaDrive::toggleConnected()
{
    setConnected(!isConnected());
}

void
AmigaDrive::toggleUnsafed()
{
    if (disk) {
        disk->modified = !disk->modified;
        if (disk->modified) {
            amiga->putMessage(MSG_DRIVE_DISK_UNSAVED);
        } else {
            amiga->putMessage(MSG_DRIVE_DISK_SAVED);
        }
    }
}

bool
AmigaDrive::hasWriteProtectedDisk()
{
    return hasDisk() ? disk->isWriteProtected() : false;
}

void
AmigaDrive::toggleWriteProtection()
{
    if (disk) {
        
        if (disk->isWriteProtected()) {
            
            disk->setWriteProtection(false);
            amiga->putMessage(MSG_DRIVE_DISK_UNPROTECTED);
            
        } else {
            
            disk->setWriteProtection(true);
            amiga->putMessage(MSG_DRIVE_DISK_PROTECTED);
        }
    }
}

void
AmigaDrive::ejectDisk()
{
    if (disk) {
        delete disk;
        disk = NULL;
        amiga->putMessage(MSG_DRIVE_DISK_EJECT, nr);
    }
}

void
AmigaDrive::insertDisk(AmigaDisk *newDisk)
{
    if (newDisk) {
        
        if (isConnected()) {
            ejectDisk();
            disk = newDisk;
            amiga->putMessage(MSG_DRIVE_DISK_INSERT, nr);
        }
    }
}

void
AmigaDrive::insertDisk(ADFFile *file)
{
    if (file) {
        
        // Convert ADF file into a disk
        // AmigaDisk = new AmigaDisk::makeWithFile(ADFFile *file)
        
        AmigaDisk *newDisk = new AmigaDisk();
        insertDisk(newDisk);
    }
}