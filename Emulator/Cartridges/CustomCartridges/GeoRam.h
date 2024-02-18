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

class GeoRAM : public Cartridge {
    
    CartridgeTraits traits = {
 
        .type       = CRT_GEO_RAM,
        .title      = "GeoRam",
        .battery    = true
    };

    virtual const CartridgeTraits &getTraits() const override { return traits; }

private:
    
    // Selected RAM bank
    u8 bank = 0;
    
    // Selected page inside the selected RAM bank
    u8 page = 0;
    
    
    //
    // Initializing
    //

public:
    
    GeoRAM(C64 &ref) : Cartridge(ref) { };
    GeoRAM(C64 &ref, isize kb);
        

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
        if (isResetter(worker)) return;

        worker

        << bank
        << page;
    }

    void operator << (SerResetter &worker) override { Cartridge::operator<<(worker); serialize(worker); }
    void operator << (SerChecker &worker) override { Cartridge::operator<<(worker); serialize(worker); }
    void operator << (SerCounter &worker) override { Cartridge::operator<<(worker); serialize(worker); }
    void operator << (SerReader &worker) override { Cartridge::operator<<(worker); serialize(worker); }
    void operator << (SerWriter &worker) override { Cartridge::operator<<(worker); serialize(worker); }


    //
    // Accessing cartridge memory
    //
    
public:

    u8 peekIO1(u16 addr) override;
    u8 spypeekIO1(u16 addr) const override;
    u8 peekIO2(u16 addr) override;
    u8 spypeekIO2(u16 addr) const override;
    void pokeIO1(u16 addr, u8 value) override;
    void pokeIO2(u16 addr, u8 value) override;
    
private:
    
    // Maps an address to the proper position in cartridge RAM
    isize offset(u8 addr) const;
};
