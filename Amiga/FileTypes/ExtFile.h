// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _EXTFILE_INC
#define _EXTFILE_INC

#include "AmigaFile.h"

class ExtFile : public AmigaFile {

private:

    // Accepted header signatures
    static const uint8_t magicBytes1[];
    static const uint8_t magicBytes2[];

public:

    //
    // Class methods
    //

    // Returns true iff buffer contains an Extended Rom image
    static bool isExtBuffer(const uint8_t *buffer, size_t length);

    // Returns true iff path points to a Extended Rom file
    static bool isExtFile(const char *path);


    //
    // Creating and destructing
    //

    ExtFile();

    // Factory methods
    static ExtFile *makeWithBuffer(const uint8_t *buffer, size_t length);
    static ExtFile *makeWithFile(const char *path);


    //
    // Methods from AmigaFile
    //

    AmigaFileType fileType() override { return FILETYPE_KICK_ROM; }
    const char *typeAsString() override { return "Extended Rom"; }
    bool bufferHasSameType(const uint8_t *buffer, size_t length) override {
        return isExtBuffer(buffer, length); }
    bool fileHasSameType(const char *path) override { return isExtFile(path); }
    bool readFromBuffer(const uint8_t *buffer, size_t length) override;

};

#endif
