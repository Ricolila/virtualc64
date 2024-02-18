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

#include "Cartridge.h"

class Epyx : public Cartridge {
    
    CartridgeTraits traits = {

        .type       = CRT_EPYX_FASTLOAD,
        .title      = "Epyx Fastload"
    };

    virtual const CartridgeTraits &getTraits() const override { return traits; }

    // Indicates when the capacitor discharges. The Epyx cartridge utilizes a
    // capacitor to switch the ROM on and off. During normal operation, the
    // capacitor charges slowly. When it is completely charged, the ROM gets
    // disabled. When the cartridge is attached, the capacitor is discharged
    // and the ROM visible. To avoid the ROM to be disabled, the cartridge can
    // either read from ROML or I/O space 1. Both operations discharge the
    // capacitor and keep the ROM alive.
    
    Cycle cycle = 0;
    
    
    //
    // Initializing
    //
    
public:
    
    Epyx(C64 &ref) : Cartridge(ref) { };


    //
    // Methods from CoreObject
    //

private:

    void _dump(Category category, std::ostream& os) const override;


    //
    // Methods from CoreComponent
    //
    
public:
    
    template <class T>
    void serialize(T& worker)
    {
        if (isSoftResetter(worker)) return;

        worker

        << cycle;
    }

    void operator << (SerResetter &worker) override;
    void operator << (SerChecker &worker) override { Cartridge::operator<<(worker); serialize(worker); }
    void operator << (SerCounter &worker) override { Cartridge::operator<<(worker); serialize(worker); }
    void operator << (SerReader &worker) override { Cartridge::operator<<(worker); serialize(worker); }
    void operator << (SerWriter &worker) override { Cartridge::operator<<(worker); serialize(worker); }

    
    //
    // Methods from Cartridge
    //
    
public:
    
    void resetCartConfig() override;
    u8 peekRomL(u16 addr) override;
    u8 spypeekRomL(u16 addr) const override;
    u8 peekIO1(u16 addr) override;
    u8 spypeekIO1(u16 addr) const override;
    u8 peekIO2(u16 addr) override;
    u8 spypeekIO2(u16 addr) const override ;
    void execute() override;

private:
    
    // Discharges the cartridge's capacitor
    void dischargeCapacitor();
};
