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

#include "Aliases.h"
#include "Serialization.h"

#pragma once

namespace vc64 {

/* An object of this class stores a single volume value and provides the means
 * to emulate a fading effect. Fading is utilized to avoid cracking noises if,
 * e.g., the emulator is put in pause mode.
 */
template <typename T> struct AudioVolume : util::Serializable {

    // Current volume
    T current = 1.0;

    // Value of 'current' if no fading takes place
    T normal = 1.0;

    // Target value pipe (used to modulate the volume)
    T target[2] = { 1.0, 1.0 };
    T delta[2] = { 1.0, 1.0 };

    // Serializing
    template <class W>
    void operator<<(W& worker)
    {
        worker
        
        << current
        << normal
        << target
        << delta;
    }
    
    // Setter and getter
    T get() const { return current; }
    void set(T value) { current = normal = target[0] = value; }
    
    // Returns true if the volume is currently fading in or out
    bool isFading() const { return current != target[0]; }

    // Initiates a fading effect
    void fadeIn(isize steps) {
        
        target[0] = normal;
        target[1] = normal;
        delta[0]  = normal / steps;
        delta[1]  = normal / steps;
    }
    void fadeOut(isize steps) {
        
        target[0] = 0;
        target[1] = 0;
        delta[0]  = normal / steps;
        delta[1]  = normal / steps;
    }
    void fadeOutTemporarily(int steps1, int steps2) {
        
        target[0] = 0;
        target[1] = normal;
        delta[0]  = normal / steps1;
        delta[0]  = normal / steps2;
    }

    // Shifts the current volume towards the target volume
    void shift() {
        
        if (current == target[0]) return;
        
        if (current < target[0]) {
            if ((current += delta[0]) < target[0]) return;
        } else {
            if ((current -= delta[0]) > target[0]) return;
        }
        
        current = target[0];
        target[0] = target[1];
        delta[0] = delta[1];
    }
};

typedef AudioVolume<float> Volume;

}
