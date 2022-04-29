// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(CRTMODE)
{
    CRTMODE_16K,
    CRTMODE_8K,
    CRTMODE_ULTIMAX,
    CRTMODE_OFF
};
typedef CRTMODE CRTMode;

#ifdef __cplusplus
struct CRTModeEnum : util::Reflection<CRTModeEnum, CRTMode> {
    
	static constexpr long minVal = 0;
    static constexpr long maxVal = CRTMODE_OFF;
    static bool isValid(auto value) { return value >= minVal && value <= maxVal; }
    
    static const char *prefix() { return "CRTMODE"; }
    static const char *key(CRTMode value)
    {
        switch (value) {
                
            case CRTMODE_16K:      return "16K";
            case CRTMODE_8K:       return "8K";
            case CRTMODE_ULTIMAX:  return "ULTIMAX";
            case CRTMODE_OFF:      return "OFF";
        }
        return "???";
    }
};
#endif
