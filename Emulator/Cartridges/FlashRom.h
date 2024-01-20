// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SubComponent.h"
#include "CartridgeTypes.h"

using namespace vc64;

/* This class implements a Flash Rom module of type Am29F040B. Flash Roms
 * of this type are used, e.g., by the EasyFlash cartridge. The implementation
 * is based on the following ressources:
 *
 *       29F040.pdf : Data sheet published by AMD
 *   flash040core.c : Part of the VICE emulator
 */
class FlashRom : public SubComponent {
            
    // Number of sectors in this Flash Rom
    static const isize numSectors = 8;
    
    // Size of a single sector in bytes (64 KB)
    static const isize sectorSize = 0x10000;
    
    // Total size of the Flash Rom in bytes (512 KB)
    static const isize romSize = 0x80000;

    // Current Flash Rom state
    FlashState state;

    // State taken after an operations has been completed
    FlashState baseState;
    
    // Flash Rom data
    u8 *rom = nullptr;
    
    
    //
    // Class methods
    //

public:

    // Checks whether the provided number is a valid bank number
    static bool isBankNumber(isize bank) { return bank < 64; }
    
    // Converts a Flash Rom state to a string
    static const char *getStateAsString(FlashState state);
    
    
    //
    // Constructing and serializing
    //
    
public:
    
    FlashRom(C64 &ref);
    ~FlashRom();
    
    
    //
    // Methods from CoreObject
    //

private:
    
    const char *getDescription() const override { return "FlashRom"; }
    void _dump(Category category, std::ostream& os) const override;

    
    //
    // Methods from CoreComponent
    //

private:

    void _reset(bool hard) override;

    template <class T>
    void applyToPersistentItems(T& worker)
    {

    }
    
    template <class T>
    void serialize(T& worker)
    {
        if (util::isResetter(worker)) return;

        worker

        << state
        << baseState;
    }
    
    isize _size() override { return [&](){COMPUTE_SNAPSHOT_SIZE}() + romSize; }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    isize didLoadFromBuffer(const u8 *buffer) override;
    isize didSaveToBuffer(u8 *buffer) override;

    
    //
    // Loading banks
    //
    
public:
    
    /* Loads an 8 KB chunk of Rom data from a buffer. This method is used when
     * loading the contents from a CRT file.
     */
    void loadBank(isize bank, u8 *data);
        
 
    //
    // Accessing memory
    //
    
public:
    
    u8 peek(u32 addr);
    u8 peek(isize bank, u16 addr) {
        assert(isBankNumber(bank)); return peek((u32)bank * 0x2000 + addr); }
    
    u8 spypeek(u32 addr) const;
    u8 spypeek(isize bank, u16 addr) const {
        assert(isBankNumber(bank)); return spypeek((u32)bank * 0x2000 + addr); }
    
    void poke(u32 addr, u8 value);
    void poke(isize bank, u16 addr, u8 value) {
        assert(isBankNumber(bank)); poke((u32)bank * 0x2000 + addr, value); }
    
    
    //
    // Performing flash operations
    //
    
    // Checks if addr serves as the first command address
    bool firstCommandAddr(u32 addr) { return (addr & 0x7FF) == 0x555; }

    // Checks if addr serves as the second command address
    bool secondCommandAddr(u32 addr) { return (addr & 0x7FF) == 0x2AA; }

    // Performs a "Byte Program" operation
    bool doByteProgram(u32 addr, u8 value);
    
    // Convenience wrapper with bank,offset addressing
    bool doByteProgram(isize bank, u16 addr, u8 value) {
        assert(isBankNumber(bank)); return doByteProgram((u32)bank * 0x2000 + addr, value); }
    
    // Performs a "Sector Erase" operation
    void doSectorErase(u32 addr);
    
    // Convenience wrapper with bank,offset addressing
    void doSectorErase(isize bank, u16 addr) {
        assert(isBankNumber(bank)); doSectorErase((u32)bank * 0x2000 + addr); }
    
    // Performs a "Chip Erase" operation
    void doChipErase();
};
