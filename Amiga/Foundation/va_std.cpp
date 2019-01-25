// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "va_std.h"

bool
matchingFileHeader(const char *path, const uint8_t *header, size_t length)
{
    assert(path != NULL);
    assert(header != NULL);
    
    bool result = true;
    FILE *file;
    
    if ((file = fopen(path, "r")) == NULL)
        return false;
    
    for (unsigned i = 0; i < length; i++) {
        int c = fgetc(file);
        if (c != (int)header[i]) {
            result = false;
            break;
        }
    }
    
    fclose(file);
    return result;
}


bool matchingBufferHeader(const uint8_t *buffer, const uint8_t *header, size_t length)
{
    assert(buffer != NULL);
    assert(header != NULL);
    
    for (unsigned i = 0; i < length; i++) {
        if (header[i] != buffer[i])
            return false;
    }
    
    return true;
}



bool releaseBuild()
{
#ifdef NDEBUG
    return true;
#else
    return false;
#endif
}