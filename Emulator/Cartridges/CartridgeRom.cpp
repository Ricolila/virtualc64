// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "CartridgeRom.h"

CartridgeRom::CartridgeRom(C64 &ref) : C64Component(ref)
{
    setDescription("CartridgeRom");
    
    // Register snapshot items
    SnapshotItem items[] = {
        
        // Internal state
        { &size,        sizeof(size),        KEEP_ON_RESET },
        { &loadAddress, sizeof(loadAddress), KEEP_ON_RESET },
        { NULL,         0,                   0 }};
    
    registerSnapshotItems(items, sizeof(items));
}

CartridgeRom::CartridgeRom(C64 &ref, u16 size, u16 loadAddress, const u8 *buffer) : CartridgeRom(ref)
{
    this->size = size;
    this->loadAddress = loadAddress;
    rom = new u8[size];
    if (buffer) {
        memcpy(rom, buffer, size);
    }
}

CartridgeRom::~CartridgeRom()
{
    assert(rom != NULL);
    delete[] rom;
}

void
CartridgeRom::_reset()
{
    // Clear snapshot items marked with 'CLEAR_ON_RESET'
    if (snapshotItems != NULL)
        for (unsigned i = 0; snapshotItems[i].data != NULL; i++)
            if (snapshotItems[i].flags & CLEAR_ON_RESET)
                memset(snapshotItems[i].data, 0, snapshotItems[i].size);
}

size_t
CartridgeRom::_size()
{
    SerCounter counter;
    applyToPersistentItems(counter);
    applyToResetItems(counter);
    
    return size + counter.count;
}

size_t
CartridgeRom::_load(u8 *buffer)
{
    SerReader reader(buffer);
    applyToPersistentItems(reader);
    applyToResetItems(reader);
    
    // Delete the old packet and create a new one with the proper size
    if (rom) delete[] rom;
    rom = new u8[size];
    
    // Read packet data
    for (int i = 0; i < size; i++) rom[i] = read8(reader.ptr);

    debug(SNP_DEBUG, "Recreated from %d bytes\n", reader.ptr - buffer); \
    return reader.ptr - buffer;
}

size_t
CartridgeRom::_save(u8 *buffer)
{
    SerWriter writer(buffer);
    applyToPersistentItems(writer);
    applyToResetItems(writer);

    // Write packet data
    for (int i = 0; i < size; i++) write8(writer.ptr, rom[i]);

    debug(SNP_DEBUG, "Serialized to %d bytes\n", writer.ptr - buffer);
    return writer.ptr - buffer;
}


size_t
CartridgeRom::oldStateSize()
{
    return HardwareComponent::oldStateSize() + size;
}

void
CartridgeRom::oldDidLoadFromBuffer(u8 **buffer)
{
    if (rom) delete[] rom;
    rom = new u8[size];
    
    readBlock(buffer, rom, size);
}

void
CartridgeRom::oldDidSaveToBuffer(u8 **buffer)
{
    writeBlock(buffer, rom, size);
}

bool
CartridgeRom::mapsToL() {
    assert(rom != NULL);
    return loadAddress == 0x8000 && size <= 0x2000;
}

bool
CartridgeRom::mapsToLH() {
    assert(rom != NULL);
    return loadAddress == 0x8000 && size > 0x2000;
}

bool
CartridgeRom::mapsToH() {
    assert(rom != NULL);
    return loadAddress == 0xA000 || loadAddress == 0xE000;
}

u8
CartridgeRom::peek(u16 addr)
{
    assert(addr < size);
    return rom[addr];
}
