// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Script.h"
#include "C64.h"
#include "IOUtils.h"

#include <sstream>

bool
Script::isCompatible(const string &path)
{
    auto s = util::extractSuffix(path);
    return s == "ini" || s == "INI";
}

bool
Script::isCompatible(std::istream &stream)
{
    return true;
}

void
Script::execute(class C64 &c64)
{
    string s((char *)data, size);
    try { c64.retroShell.execScript(s); } catch (util::Exception &) { }
}
