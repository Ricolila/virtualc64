// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// This FILE is dual-licensed. You are free to choose between:
//
//     - The GNU General Public License v3 (or any later version)
//     - The Mozilla Public License v2
//
// SPDX-License-Identifier: GPL-3.0-or-later OR MPL-2.0
// -----------------------------------------------------------------------------

#pragma once

#include "Concurrency.h"
#include "RingBuffer.h"
#include "Volume.h"

namespace vc64 {

typedef util::RingBuffer<short, 2048> SampleStream;

typedef struct { float left; float right; } SamplePair;

class StereoStream : public util::RingBuffer < SamplePair, 12288 > {
    
    // Mutex for synchronizing read / write accesses
    util::ReentrantMutex mutex;


    //
    // Synchronizing access
    //

public:

    // Locks or unlocks the mutex
    void lock() { mutex.lock(); }
    void unlock() { mutex.unlock(); }

    // Initializes the ring buffer with zeroes
    void wipeOut() { this->clear(SamplePair {0,0} ); }
    
    // Adds a sample to the ring buffer
    void add(float l, float r) { this->write(SamplePair {l,r} ); }

    // Puts the write pointer somewhat ahead of the read pointer
    void alignWritePtr();

    
    //
    // Copying data
    //
    
    /* Copies n audio samples into a memory buffer. These functions mark the
     * final step in the audio pipeline. They are used to copy the generated
     * sound samples into the buffers of the native sound device.
     */
    void copyMono(float *buffer, isize n, Volume &volL, Volume &volR);
    void copyStereo(float *left, float *right, isize n, Volume &volL, Volume &volR);
    void copyInterleaved(float *buffer, isize n, Volume &volL, Volume &volR);
};

}
