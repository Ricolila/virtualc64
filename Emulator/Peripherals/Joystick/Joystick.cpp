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

#include "config.h"
#include "Joystick.h"
#include "Emulator.h"
#include "IOUtils.h"

namespace vc64 {

Joystick::Joystick(C64& ref, ControlPort& pref) : SubComponent(ref), port(pref)
{
};

const char *
Joystick::getDescription() const
{
    return port.nr == PORT_1 ? "Joystick1" : "Joystick2";
}

void
Joystick::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    // Discard any active joystick movements
    button = false;
    axisX = 0;
    axisY = 0;
}

void
Joystick::resetConfig()
{
    assert(isPoweredOff());
    auto &defaults = emulator.defaults;

    std::vector <Option> options = {

        OPT_AUTOFIRE,
        OPT_AUTOFIRE_BULLETS,
        OPT_AUTOFIRE_DELAY
    };

    for (auto &option : options) {
        setConfigItem(option, defaults.get(option));
    }
}

i64
Joystick::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_AUTOFIRE:          return (i64)config.autofire;
        case OPT_AUTOFIRE_BULLETS:  return (i64)config.autofireBullets;
        case OPT_AUTOFIRE_DELAY:    return (i64)config.autofireDelay;
            
        default:
            fatalError;
    }
}

void
Joystick::setConfigItem(Option option, i64 value)
{
    switch (option) {
            
        case OPT_AUTOFIRE:
            
            config.autofire = bool(value);
            
            // Release button immediately if autofire-mode is switches off
            if (value == false) button = false;
            return;

        case OPT_AUTOFIRE_BULLETS:
            
            config.autofireBullets = isize(value);
            
            // Update the bullet counter if we're currently firing
            if (bulletCounter > 0) reload();
            return;

        case OPT_AUTOFIRE_DELAY:
            
            config.autofireDelay = isize(value);
            return;

        default:
            fatalError;
    }
}

void
Joystick::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::Config) {
        
        os << tab("Joystick nr") << dec(port.nr) << std::endl;
        os << tab("Auto fire") << bol(config.autofire) << std::endl;
        os << tab("Auto fire bullets") << dec(config.autofireBullets) << std::endl;
        os << tab("Auto fire delay") << dec(config.autofireDelay) << std::endl;
    }

    if (category == Category::State) {
        
        os << tab("Joystick nr") << dec(port.nr) << std::endl;
        os << tab("Button") << bol(button) << std::endl;
        os << tab("X axis") << dec(axisX) << std::endl;
        os << tab("Y axis") << dec(axisY) << std::endl;
    }
}

void 
Joystick::newserialize(util::SerReader &worker)
{
    serialize(worker);

    // Discard any active joystick movements
    button = false;
    axisX = 0;
    axisY = 0;
}

void
Joystick::reload()
{
    bulletCounter = (config.autofireBullets < 0) ? INT64_MAX : config.autofireBullets;
}

void
Joystick::scheduleNextShot()
{
    nextAutofireFrame = c64.frame + config.autofireDelay;
}

u8
Joystick::getControlPort() const
{
    u8 result = 0xFF;
    
    if (axisY == -1) CLR_BIT(result, 0);
    if (axisY ==  1) CLR_BIT(result, 1);
    if (axisX == -1) CLR_BIT(result, 2);
    if (axisX ==  1) CLR_BIT(result, 3);
    if (button)      CLR_BIT(result, 4);

    return result;
}

void
Joystick::trigger(GamePadAction event)
{
    debug(JOY_DEBUG, "Port %ld: %s\n", port.nr, GamePadActionEnum::key(event));
    
    switch (event) {

        case PULL_UP:    axisY = -1; break;
        case PULL_DOWN:  axisY =  1; break;
        case PULL_LEFT:  axisX = -1; break;
        case PULL_RIGHT: axisX =  1; break;
        case RELEASE_X:  axisX =  0; break;
        case RELEASE_Y:  axisY =  0; break;
        case RELEASE_XY: axisX = axisY = 0; break;
            
        case PRESS_FIRE:
            
            if (config.autofire) {
                if (bulletCounter) {
                    
                    // Cease fire
                    bulletCounter = 0;
                    button = false;
                    
                } else {

                    // Load magazine
                    button = true;
                    reload();
                    scheduleNextShot();
                }
                
            } else {
                button = true;
            }
            break;

        case RELEASE_FIRE:
            
            if (!config.autofire) button = false;
            break;

        default:
            fatalError;
    }
    
    port.device = CPDEVICE_JOYSTICK;
}

void
Joystick::execute()
{
    // Only proceed if auto fire is enabled
    if (!config.autofire || config.autofireDelay < 0) return;

    // Only proceed if a trigger frame has been reached
    if ((i64)c64.frame != nextAutofireFrame) return;
    
    // Only proceed if there are bullets left
    if (bulletCounter == 0) return;
    
    if (button) {
        button = false;
        bulletCounter--;
    } else {
        button = true;
    }
    scheduleNextShot();
}

}
