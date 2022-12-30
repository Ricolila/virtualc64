// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "C64.h"

void
GeoRAM::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    if (!getBattery()) {
        trace(CRT_DEBUG, "Erasing GeoRAM\n");
        eraseRAM(0);
    } else {
        trace(CRT_DEBUG, "Preserving GeoRAM\n");
    }
}

void
GeoRAM::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    Cartridge::_dump(category, os);
    os << std::endl;

    if (category == Category::Debug) {

        os << tab("Bank");
        os << dec(bank) << std::endl;
        os << tab("Page");
        os << dec(page) << std::endl;
    }
}

u8
GeoRAM::peekIO1(u16 addr)
{
    assert(addr >= 0xDE00 && addr <= 0xDEFF);
    return peekRAM((u16)offset(addr & 0xFF));
}

u8
GeoRAM::spypeekIO1(u16 addr) const
{
    assert(addr >= 0xDE00 && addr <= 0xDEFF);
    return peekRAM((u16)offset(addr & 0xFF));
}

u8
GeoRAM::peekIO2(u16 addr)
{
    return 0;
}

u8
GeoRAM::spypeekIO2(u16 addr) const
{
    return 0;
}

void
GeoRAM::pokeIO1(u16 addr, u8 value)
{
    assert(addr >= 0xDE00 && addr <= 0xDEFF);
    pokeRAM((u16)offset(addr & 0xFF), value);
}

void
GeoRAM::pokeIO2(u16 addr, u8 value)
{
    if (addr & 1) {
        bank = value; // Bank select
    } else {
        page = value; // Page select
    }
}

isize
GeoRAM::offset(u8 addr) const
{
    /* From VICE:
     * "The GeoRAM is a banked memory system. It uses the registers at
     *  $dffe and $dfff to determine what part of the GeoRAM memory should
     *  be mapped to $de00-$deff.
     *  The register at $dfff selects which 16k block to map, and $dffe
     *  selects a 256-byte page in that block. Since there are only 64
     *  256-byte pages inside of 16k, the value in $dffe ranges from 0 to 63."
     */
    
    isize bankOffset = (bank * 16384) % getRamCapacity();
    isize pageOffset = (page & 0x3F) * 256;
    return bankOffset + pageOffset + addr;
}
