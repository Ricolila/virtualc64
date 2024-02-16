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

#include <iostream>

namespace vc64 {

enum class Category
{
    BankMap,
    Config,
    Current,
    Debug,
    Defaults,
    Disk,
    Dma,
    Layout,
    Properties,
    Registers,
    Slots,
    State,
    Stats,
    Summary,
    Tod,
};

struct Void { };

class Dumpable {

public:

    virtual ~Dumpable() { }
    virtual void _dump(Category category, std::ostream& ss) const { }

    void dump(Category category, std::ostream& ss) const { _dump(category, ss); }
    void dump(Category category) const { dump(category, std::cout); }
};

template <typename T1, typename T2>
class Inspectable : public Dumpable {

protected:
    
    mutable T1 info;
    mutable T2 stats;

public:

    Inspectable() { }
    virtual ~Inspectable() { }
    void autoInspect() const { recordState(info); }

    T1 &getState() const {

        if (stateIsDirty()) recordState(info);
        return info;
    }

    T2 &getStats() const {

        if (statsIsDirty()) recordStats(stats);
        return stats;
    }

private:

    virtual bool stateIsDirty() const { return true; }
    virtual bool statsIsDirty() const { return true; }

    virtual void recordState(T1 &result) const { };
    virtual void recordStats(T2 &result) const { };
};

}
