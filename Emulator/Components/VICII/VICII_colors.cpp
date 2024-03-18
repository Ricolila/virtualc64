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
#include "VICII.h"
#include "C64.h"

#include <algorithm>
#include <cmath>

namespace vc64 {

void
VICII::updatePalette()
{
    for (isize i = 0; i < 16; i++) {
        rgbaTable[i] = monitor.getColor(i);
    }
}

}
