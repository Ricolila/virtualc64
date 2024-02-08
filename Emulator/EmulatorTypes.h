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

#include "Aliases.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(OPT)
{
    // Host
    OPT_HOST_REFRESH_RATE,
    OPT_HOST_SAMPLE_RATE,
    OPT_HOST_FRAMEBUF_WIDTH,
    OPT_HOST_FRAMEBUF_HEIGHT,

    // C64
    OPT_WARP_BOOT,
    OPT_WARP_MODE,
    OPT_VSYNC,
    OPT_TIME_LAPSE,
    OPT_RUN_AHEAD,

    // VICII
    OPT_VIC_REVISION,
    OPT_PALETTE,
    OPT_BRIGHTNESS,
    OPT_CONTRAST,
    OPT_SATURATION,
    OPT_GRAY_DOT_BUG,
    OPT_VIC_POWER_SAVE,

    // Sprite debugger
    OPT_HIDE_SPRITES,
    OPT_CUT_LAYERS,
    OPT_CUT_OPACITY,
    OPT_SS_COLLISIONS,
    OPT_SB_COLLISIONS,

    // DMA Debugger
    OPT_DMA_DEBUG_ENABLE,
    OPT_DMA_DEBUG_MODE,
    OPT_DMA_DEBUG_OPACITY,
    OPT_DMA_DEBUG_CHANNEL,
    OPT_DMA_DEBUG_COLOR,

    // Power supply
    OPT_POWER_GRID,

    // Logic board
    OPT_GLUE_LOGIC,

    // CIA
    OPT_CIA_REVISION,
    OPT_TIMER_B_BUG,

    // SID
    OPT_SID_ENABLE,
    OPT_SID_ADDRESS,
    OPT_SID_REVISION,
    OPT_SID_FILTER,
    OPT_SID_POWER_SAVE,
    OPT_SID_ENGINE,
    OPT_SID_SAMPLING,
    OPT_AUDPAN,
    OPT_AUDVOL,
    OPT_AUDVOLL,
    OPT_AUDVOLR,

    // Memory
    OPT_RAM_PATTERN,
    OPT_SAVE_ROMS,

    // Drive
    OPT_DRV_AUTO_CONFIG,
    OPT_DRV_TYPE,
    OPT_DRV_RAM,
    OPT_DRV_PARCABLE,
    OPT_DRV_CONNECT,
    OPT_DRV_POWER_SWITCH,
    OPT_DRV_POWER_SAVE,
    OPT_DRV_EJECT_DELAY,
    OPT_DRV_SWAP_DELAY,
    OPT_DRV_INSERT_DELAY,
    OPT_DRV_PAN,
    OPT_DRV_POWER_VOL,
    OPT_DRV_STEP_VOL,
    OPT_DRV_INSERT_VOL,
    OPT_DRV_EJECT_VOL,

    // Datasette
    OPT_DAT_MODEL,
    OPT_DAT_CONNECT,

    // Mouse
    OPT_MOUSE_MODEL,
    OPT_SHAKE_DETECTION,
    OPT_MOUSE_VELOCITY,

    // Joystick
    OPT_AUTOFIRE,
    OPT_AUTOFIRE_BULLETS,
    OPT_AUTOFIRE_DELAY,

    OPT_COUNT
};
typedef OPT Option;

#ifdef __cplusplus
struct OptionEnum : util::Reflection<OptionEnum, Option> {

    static constexpr long minVal = 0;
    static constexpr long maxVal = OPT_AUTOFIRE_DELAY;
    static bool isValid(auto value) { return value >= minVal && value <= maxVal; }

    static const char *prefix() { return "OPT"; }
    static const char *key(Option value)
    {
        switch (value) {

            case OPT_HOST_REFRESH_RATE:     return "HOST_REFRESH_RATE";
            case OPT_HOST_SAMPLE_RATE:      return "HOST_SAMPLE_RATE";
            case OPT_HOST_FRAMEBUF_WIDTH:   return "HOST_FRAMEBUF_WIDTH";
            case OPT_HOST_FRAMEBUF_HEIGHT:  return "HOST_FRAMEBUF_HEIGHT";

            case OPT_WARP_BOOT:             return "WARP_BOOT";
            case OPT_WARP_MODE:             return "WARP_MODE";
            case OPT_VSYNC:                 return "VSYNC";
            case OPT_TIME_LAPSE:            return "TIME_LAPSE";
            case OPT_RUN_AHEAD:             return "RUN_AHEAD";

            case OPT_VIC_REVISION:          return "VIC_REVISION";
            case OPT_PALETTE:               return "PALETTE";
            case OPT_BRIGHTNESS:            return "BRIGHTNESS";
            case OPT_CONTRAST:              return "CONTRAST";
            case OPT_SATURATION:            return "SATURATION";
            case OPT_GRAY_DOT_BUG:          return "GRAY_DOT_BUG";
            case OPT_VIC_POWER_SAVE:        return "VIC_POWER_SAVE";

            case OPT_HIDE_SPRITES:          return "HIDE_SPRITES";
            case OPT_CUT_LAYERS:            return "CUT_LAYERS";
            case OPT_CUT_OPACITY:           return "CUT_OPACITY";
            case OPT_SS_COLLISIONS:         return "SS_COLLISIONS";
            case OPT_SB_COLLISIONS:         return "SB_COLLISIONS";

            case OPT_DMA_DEBUG_ENABLE:      return "DMA_DEBUG_ENABLE";
            case OPT_DMA_DEBUG_MODE:        return "DMA_DEBUG_MODE";
            case OPT_DMA_DEBUG_OPACITY:     return "DMA_DEBUG_OPACITY";
            case OPT_DMA_DEBUG_CHANNEL:     return "DMA_DEBUG_CHANNEL";
            case OPT_DMA_DEBUG_COLOR:       return "DMA_DEBUG_COLOR";

            case OPT_POWER_GRID:            return "POWER_GRID";

            case OPT_GLUE_LOGIC:            return "GLUE_LOGIC";

            case OPT_CIA_REVISION:          return "CIA_REVISION";
            case OPT_TIMER_B_BUG:           return "TIMER_B_BUG";

            case OPT_SID_ENABLE:            return "SID_ENABLE";
            case OPT_SID_ADDRESS:           return "SID_ADDRESS";
            case OPT_SID_REVISION:          return "SID_REVISION";
            case OPT_SID_FILTER:            return "SID_FILTER";
            case OPT_SID_POWER_SAVE:        return "SID_POWER_SAVE";
            case OPT_SID_ENGINE:            return "SID_ENGINE";
            case OPT_SID_SAMPLING:          return "SID_SAMPLING";
            case OPT_AUDPAN:                return "AUDPAN";
            case OPT_AUDVOL:                return "AUDVOL";
            case OPT_AUDVOLL:               return "AUDVOLL";
            case OPT_AUDVOLR:               return "AUDVOLR";

            case OPT_RAM_PATTERN:           return "RAM_PATTERN";
            case OPT_SAVE_ROMS:             return "SAVE_ROMS";

            case OPT_DRV_AUTO_CONFIG:       return "DRV_AUTO_CONFIG";
            case OPT_DRV_TYPE:              return "DRV_TYPE";
            case OPT_DRV_RAM:               return "OPT_DRV_RAM";
            case OPT_DRV_PARCABLE:          return "OPT_DRV_PARCABLE";
            case OPT_DRV_CONNECT:           return "DRV_CONNECT";
            case OPT_DRV_POWER_SWITCH:      return "DRV_POWER_SWITCH";
            case OPT_DRV_POWER_SAVE:        return "DRV_POWER_SAVE";
            case OPT_DRV_EJECT_DELAY:       return "DRV_EJECT_DELAY";
            case OPT_DRV_SWAP_DELAY:        return "DRV_SWAP_DELAY";
            case OPT_DRV_INSERT_DELAY:      return "DRV_INSERT_DELAY";
            case OPT_DRV_PAN:               return "DRV_PAN";
            case OPT_DRV_POWER_VOL:         return "DRV_POWER_VOL";
            case OPT_DRV_STEP_VOL:          return "DRV_STEP_VOL";
            case OPT_DRV_INSERT_VOL:        return "DRV_INSERT_VOL";
            case OPT_DRV_EJECT_VOL:         return "DRV_EJECT_VOL";

            case OPT_DAT_MODEL:             return "DAT_MODEL";
            case OPT_DAT_CONNECT:           return "DAT_CONNECT";

            case OPT_MOUSE_MODEL:           return "MOUSE_MODEL";
            case OPT_SHAKE_DETECTION:       return "SHAKE_DETECTION";
            case OPT_MOUSE_VELOCITY:        return "MOUSE_VELOCITY";

            case OPT_AUTOFIRE:              return "AUTOFIRE";
            case OPT_AUTOFIRE_BULLETS:      return "AUTOFIRE_BULLETS";
            case OPT_AUTOFIRE_DELAY:        return "AUTOFIRE_DELAY";

            case OPT_COUNT:                 return "???";
        }
        return "???";
    }
};
#endif

enum_long(WARP_MODE)
{
    WARP_AUTO,
    WARP_NEVER,
    WARP_ALWAYS
};
typedef WARP_MODE WarpMode;

#ifdef __cplusplus
struct WarpModeEnum : util::Reflection<WarpModeEnum, WarpMode>
{
    static constexpr long minVal = 0;
    static constexpr long maxVal = WARP_ALWAYS;
    static bool isValid(auto val) { return val >= minVal && val <= maxVal; }

    static const char *prefix() { return "WARP"; }
    static const char *key(WarpMode value)
    {
        switch (value) {

            case WARP_AUTO:     return "WARP_AUTO";
            case WARP_NEVER:    return "WARP_NEVER";
            case WARP_ALWAYS:   return "WARP_ALWAYS";
        }
        return "???";
    }
};
#endif


//
// Structures
//

typedef struct
{
    isize warpBoot;
    WarpMode warpMode;
    bool vsync;
    isize timeLapse;
    isize runAhead;
}
EmulatorConfig;
