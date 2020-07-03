// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef NEOSMOUSE_H
#define NEOSMOUSE_H

#include "HardwareComponent.h"

class NeosMouse : public HardwareComponent {
    
    //! @brief    Mouse position
    int64_t mouseX;
    int64_t mouseY;
    
    //! @brief    Mouse button states
    bool leftButton;
    bool rightButton;
    
    //! @brief    Dividers applied to raw coordinates in setXY()
    int dividerX = 512;
    int dividerY = 256;
    
    //! @brief    Mouse movement in pixels per execution step
    int64_t shiftX = 127;
    int64_t shiftY = 127;
    
    //! @brief    Mouse state
    /*! @details  When the mouse switches to state 0, the current mouse
     *            position is latched and the deltaX and deltaY are computed.
     *            After that, the mouse cycles through the other states and
     *            writes the delta values onto the control port, nibble by nibble.
     */
    uint8_t state;

    //! @brief    CPU cycle of the most recent trigger event
    u64 triggerCycle;
    
    //! @brief    Latched horizontal mouse position
    int64_t latchedX;
    
    //! @brief    Latched vertical mouse position
    int64_t latchedY;
    
    //! @brief    The least signifanct value is transmitted to the C64
    int8_t deltaX;
    
    //! @brief    The least signifanct value is transmitted to the C64
    int8_t deltaY;
    
public:
    
    //! @brief    Constructor
    NeosMouse();
    
    //! @brief    Destructor
    ~NeosMouse();
    
    //! @brief    Methods from HardwareComponent class
    void reset();
    
    //! @brief   Updates the button state
    void setLeftMouseButton(bool value) { leftButton = value; }
    void setRightMouseButton(bool value) { rightButton = value; }
    
    //! @brief   Returns the pot X bits as set by the mouse
    uint8_t readPotX();
    
    //! @brief   Returns the pot Y bits as set by the mouse
    uint8_t readPotY();
    
    //! @brief   Returns the control port bits triggered by the mouse
    uint8_t readControlPort(int64_t targetX, int64_t targetY);
    
    /*! @brief   Execution function
     *  @details Shifts mouseX and mouseY smoothly towards targetX and targetX.
     */
    // void execute(int64_t targetX, int64_t targetY);
    
    //! @brief    Triggers a state change (rising edge on control port line)
    void risingStrobe(int portNr, int64_t targetX, int64_t targetY);
    
    //! @brief    Triggers a state change (falling edge on control port line)
    void fallingStrobe(int portNr, int64_t targetX, int64_t targetY);
    
private:
    
    //! @brief  Latches the current mouse position and computed deltas
    void latchPosition(int64_t targetX, int64_t targetY);
};

#endif
