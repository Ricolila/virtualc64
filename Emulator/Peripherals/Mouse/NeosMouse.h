// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"

namespace vc64 {

class NeosMouse : public SubComponent {
    
    // Mouse position
    i64 mouseX;
    i64 mouseY;
    
    // Mouse button states
    bool leftButton;
    bool rightButton;
    
    // Dividers applied to raw coordinates in setXY()
    int dividerX = 512;
    int dividerY = 256;
    
    // Mouse movement in pixels per execution step
    i64 shiftX = 127;
    i64 shiftY = 127;
    
    // Mouse state. When the mouse switches to state 0, the current mouse
    // position is latched and the deltaX and deltaY are computed. After that,
    // the mouse cycles through the other states and writes the delta values
    // onto the control port, nibble by nibble.
    u8 state;

    // CPU cycle of the most recent trigger event
    i64 triggerCycle;
    
    // Latched mouse position
    i64 latchedX;
    i64 latchedY;
    
    // Value transmitted to the C64
    i8 deltaX;
    i8 deltaY;
    
    
    //
    // Initializing
    //
    
public:
    
    NeosMouse(C64 &ref) : SubComponent(ref) { }
    
    
    //
    // Methods from CoreObject
    //

private:
    
    const char *getDescription() const override { return "NeosMouse"; }

    
    //
    // Methods from CoreComponent
    //

private:
    
    void _reset(bool hard) override;

    template <class T> void serialize(T& worker) { }
    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    
    
    //
    // Accessing
    //
    
public:
    
    // Updates the button state
    void setLeftMouseButton(bool value) { leftButton = value; }
    void setRightMouseButton(bool value) { rightButton = value; }
    
    // Returns the pot bits as set by the mouse
    u8 readPotX() const;
    u8 readPotY() const;
    
    // Updates the control port bits (must be called before reading)
    void updateControlPort(i64 targetX, i64 targetY);
    
    // Returns the control port bits triggered by the mouse
    u8 readControlPort() const;

    // Triggers a state change
    void risingStrobe(i64 targetX, i64 targetY);
    void fallingStrobe(i64 targetX, i64 targetY);
    
private:
    
    // Latches the current mouse position and computed the transmission deltas
    void latchPosition(i64 targetX, i64 targetY);
};

}
