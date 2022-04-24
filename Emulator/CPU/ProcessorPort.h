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

/* The MOS 6510 CPU contains a processor port register and a data direction
 * register. The latter indicates if a port pin is configured as input or
 * output. In the C64, the register serves multiple pursposes. Firstly, it is
 * used for bank switching, i.e. it decides if ROM, RAM, or IO is visible in
 * certain memory regions. Secondly, it is used to communicate with the
 * datasette.
 */
class ProcessorPort : public SubComponent {
   
    // The processor port register
    u8 port;
    
    // The processor port data direction register
    u8 direction;
    
    /* Clock cycle when floating bit values reach zero.
     * Bit 3, 6, and 7 of the processor need special attention. When the
     * direction of these bits is changed from output to input, there will be
     * no external signal driving them. As a result, these bits will be in a
     * floating state and act as capacitors. They will discharge slowly and
     * eventually reach zero. These variables are used to indicate when the
     * zero level is reached. All three variables are queried in readPort() and
     * comply to the following rules:
     *
     *    dischargeCycleBit > current cycle => bit reads as 1
     *                                         (if configured as input)
     *    otherwise                         => bit reads as 0
     *                                         (if configured as input)
     */
    u64 dischargeCycleBit3;
    u64 dischargeCycleBit6;
    u64 dischargeCycleBit7;

    
    //
    // Initializing
    //
    
public:
    
    ProcessorPort(C64 &ref) : SubComponent(ref) { }
    
    
    //
    // Methods from C64Object
    //

private:
    
    const char *getDescription() const override { return "ProcessorPort"; }
    void _dump(dump::Category category, std::ostream& os) const override;

    
    //
    // Methods from C64Component
    //

private:
    
    void _reset(bool hard) override;

    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }
    
    template <class T>
    void applyToResetItems(T& worker, bool hard = true)
    {
        worker
        
        << port
        << direction
        << dischargeCycleBit3
        << dischargeCycleBit6
        << dischargeCycleBit7;
    }
    
    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    u64 _checksum() override { COMPUTE_SNAPSHOT_CHECKSUM }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    
    
    //
    // Accessing the port registers
    //
    
public:
    
    u8 read() const;
    u8 readDirection() const;

    void write(u8 value);
    void writeDirection(u8 value);

    bool getLoram() const { return read() & 0x1; }
    bool getHiram() const { return read() & 0x2; }
    bool getCharen() const { return read() & 0x4; }
};
