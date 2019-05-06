// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_DRIVE_INC
#define _AMIGA_DRIVE_INC

#include "HardwareComponent.h"
#include "Disk.h"

class ADFFile;



class Drive : public HardwareComponent {
    
    friend class DiskController;
    
private:
    
    // Drive number (0 = df0, 1 = df1, 2 = df2, 3 = df3)
    long nr = 0;
    
    // The type of this drive
    DriveType type = DRIVE_35_DD;
    
    /* Acceleration factor of this drive
     * This value equals the number of words that get transfered into memory
     * during a single disk DMA cycle. This value must be 1 to emulate a real
     * Amiga. If it set to, e.g., 2, the drive loads twice as fast.
     * A negative value indicates a turbo drive for which the exect value of
     * the acceleration factor has no meaning.
     */
    uint16_t speed = 1;
    
    /* The drive type identification code
     * Each drive identifies itself by a 32 bit identification code that is
     * transmitted via the DRVRDY signal in a special identification mode. The
     * identification mode is activated by switching the drive motor on and
     * off.
     */
    // DriveIdCode id = DRIVE_ID_35DD;

    // Indicates if the identification mode is active.
    // bool idMode;
    
    // Position of the currently transmitted identification bit
    uint8_t idCount;

    // Value of the currently transmitted identification bit
    bool idBit;

    
    /* The latched MTR bit (motor control bit)
     * Each drive latches the motor signal at the time it is selected (i.e.,
     * when the SELx line is pulled down to 0). The disk drive motor stays in
     * this state until the drive is selected agail. This bit also controls the
     * activity light on the front of the disk drive.
     */
    // bool mtr;
    
    /* Indicates if the motor is running at full speed
     * On a real drive, it can take up to one half second (500ms) until the
     * drive runs at full speed. We don't emulate  accurate timing here and
     * set the variable to true once the drive motor is switched on.
     */
    bool motor;
    
    /* Disk change status
     * This variable controls the /CHNG bit in the CIA A PRA register. Note
     * that the variable only changes its value under certain circumstances.
     * If a head movement pulse is sent and no disk is inserted, the variable
     * is set to false (which is also the reset value). It becomes true when
     * a disk is ejected.
     */
    bool dskchange;
    
    // The currently selected disk head
    // DEPRECATED
    // bool side;

    // A copy of the DSKLEN register
    uint8_t dsklen;
    
    // A copy of the PRB register of CIA B
    uint8_t prb;
    
    // The current drive head location
    struct {
        uint8_t side;
        uint8_t cylinder;
        uint16_t offset;
    } head;
    
    /* History buffer storing the most recently visited tracks.
     * The buffer is used to detect the polling head movements that are issued
     * by track disc device to detect a newly inserted disk.
     */
    uint64_t cylinderHistory;
    
public:
    
    // The currently inserted disk (if any)
    Disk *disk = NULL;
    
    
    //
    // Constructing and destructing
    //
    
public:
    
    Drive(unsigned nr);
    
    
    //
    // Methods from HardwareComponent
    //
    
private:
    
     void _powerOn() override;
     void _powerOff() override;
     void _reset() override;
     void _ping() override;
     void _dump() override;
    
    
public:
    
    //
    // Accessing device properties
    //
    
    // Returns the device number (0 = df0, 1 = df1, 2 = df2, 3 = df3).
    long getNr() { return nr; }
    
    // Returns the drive type.
    DriveType getType() { return type; }

    // Sets the drive type.
    void setType(DriveType t);
    
    // Returns the accleration factor.
    uint16_t getSpeed() { return speed; }

    // Sets the accleration factor.
    void setSpeed(uint16_t value);

    // Indicates whether this drive is a turbo drive.
    bool isTurboDrive() { return speed > 128; }

    // Returns the drive identification code.
    uint32_t getDriveId();
    
    
    //
    // Handling the drive status register flags
    //
    
    // Returns true if this drive is currently selected
    inline bool isSelected() { return (prb & (0b1000 << nr)) == 0; }
    
    // Returns true if this drive is pushing data onto the data lines
    bool isDataSource();
    
    
    uint8_t driveStatusFlags();
    

    //
    // Operating the drive
    //
    
    // Turns the drive motor on or off
    void setMotor(bool value);
    void switchMotorOn() { setMotor(true); }
    void switchMotorOff() { setMotor(false); }
    
    // Selects the active drive head (0 = lower, 1 = upper)
    void selectSide(int side);

    // Reads a byte at current drive head position
    uint8_t readHead();

    // Writes a byte at current drive head position
    void writeHead(uint8_t value);

    // Emulate a disk rotation (moves head to the next byte)
    void rotate();

    // Rotates the disk to the next sync mark
    void findSyncMark();

    // Moves the drive head (0 = inwards, 1 = outwards)
    void moveHead(int dir);
    
    
    //
    // Handling disks
    //


    bool hasDisk() { return disk != NULL; }
    bool hasModifiedDisk() { return disk ? disk->isModified() : false; }
    void setModifiedDisk(bool value) { if (disk) disk->setModified(value); }
    
    bool hasWriteEnabledDisk();
    bool hasWriteProtectedDisk();
    void setWriteProtection(bool value); 
    void toggleWriteProtection();
    
    void ejectDisk();
    void insertDisk(Disk *disk);
    void insertDisk(ADFFile *file);
    
    /* Latches the MTR bit
     * TODO: Quote from HRM?
     */
    // void latchMTR(bool value);
    
    
    //
    // Register delegation methods
    //
    
    // Write handler for the PRB register of CIA B
    void PRBdidChange(uint8_t oldValue, uint8_t newValue);
};

#endif
