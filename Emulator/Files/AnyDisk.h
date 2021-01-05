// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#if 0

#pragma once

#include "AnyCollection.h"

class AnyDisk : public AnyCollection {
    
protected:
    
    // File pointer. An offset into the data range of the selected track.
    long tFp = -1;
    
    // End of file position. This value equals the last valid offset plus 1.
    long tEof = -1;
    
    
    //
    // Constructing
    //
    
public:
    
    AnyDisk() : AnyCollection() { }
    
    const char *getDescription() override { return "AnyDisk"; }
    
    //
    // Selecting tracks or halftracks
    //
    
    // Returns the number of halftracks of the represented disk
    virtual int numberOfHalftracks() { return 0; }
    virtual int numberOfTracks() { return (numberOfHalftracks() + 1) / 2; }
    
    // Selects the active track (all track related methods work on this track)
    virtual void selectHalftrack(Halftrack ht) { };
    virtual void selectTrack(Track t) { selectHalftrack(2 * t - 1); }
        
    
    //
    // Reading data from a track
    //
    
    // Returns the size of the selected haltrack in bytes
    virtual usize getSizeOfHalftrack() { return 0; }
    virtual usize getSizeOfTrack() { return getSizeOfHalftrack(); }
                                                
    /* Moves the file pointer to the specified offset. seek(0) returns to the
     * beginning of the selected track.
     */
    virtual void seekHalftrack(long offset) { }
    virtual void seekTrack(long offset) { seekHalftrack(offset); }
    
    /* Reads a byte from the selected track. Returns EOF (-1) if all bytes have
     * been read in.
     */
    virtual int readHalftrack();
    virtual int readTrack() { return readHalftrack(); }
    
    // Reads multiple bytes (1 .. 85) in form of a hex dump string
    // virtual const char *readHalftrackHex(usize num);
    // virtual const char *readTrackHex(usize num) { return readHalftrackHex(num); }
    
    // Copies the selected track into the specified buffer
    virtual void copyHalftrack(u8 *buffer, usize offset = 0);
    virtual void copyTrack(u8 *buffer, usize offset = 0) { copyHalftrack(buffer, offset); }
};

#endif
