/*!
 * @header      FreezeFrame.h
 * @author      Dirk W. Hoffmann, www.dirkwhoffmann.de
 * @copyright   Dirk W. Hoffmann. All rights reserved.
 */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _FREEZEFRAME_INC
#define _FREEZEFRAME_INC

#include "Cartridge.h"

class FreezeFrame : public Cartridge {
    
public:
    
    using Cartridge::Cartridge;
    CartridgeType getCartridgeType() { return CRT_FREEZE_FRAME; }

    //
    //! @functiongroup Methods from VirtualComponent
    //

    void reset();

    //
    //! @functiongroup Methods from Cartridge
    //
    
    uint8_t peekIO1(uint16_t addr);
    uint8_t spypeekIO1(uint16_t addr) { return 0; }
    uint8_t peekIO2(uint16_t addr);
    uint8_t spypeekIO2(uint16_t addr) { return 0; }

    unsigned numButtons() { return 1; }
    const char *getButtonTitle(unsigned nr) { return (nr == 1) ? "Freeze" : NULL; }
    void pressButton(unsigned nr);
    void releaseButton(unsigned nr);
};

#endif
