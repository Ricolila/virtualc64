// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Datasette.h"
#include "C64.h"

namespace vc64 {

util::Time
Pulse::delay() const
{
    return util::Time((i64)cycles * 1000000000 / PAL_CLOCK_FREQUENCY);
}

Datasette::~Datasette()
{
    dealloc();
}

void
Datasette::alloc(isize capacity)
{
    dealloc();
    
    if (capacity) {
        pulses = new (std::nothrow) Pulse[capacity];
        size = capacity;
    }
}

void
Datasette::dealloc()
{
    if (pulses) {
        delete[] pulses;
        pulses = nullptr;
        size = 0;
    }
}

void
Datasette::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
}

void
Datasette::_dump(Category category, std::ostream& os) const
{
    using namespace util;

    if (category == Category::Config) {

        os << tab("Model") << DatasetteModelEnum::key(config.model) << std::endl;
        os << tab("Connected") << bol(config.connected) << std::endl;
    }

    if (category == Category::State) {
        
        os << tab("TAP type");
        os << dec(type) << std::endl;
        os << tab("Pulse count");
        os << dec(size) << std::endl;

        os << std::endl;

        os << tab("Head position");
        os << dec(head) << std::endl;
        os << tab("Play key");
        os << bol(playKey, "pressed", "released") << std::endl;
        os << tab("Motor");
        os << bol(motor, "on", "off") << std::endl;
        os << tab("nextRisingEdge");
        os << dec(nextRisingEdge) << std::endl;
        os << tab("nextFallingEdge");
        os << dec(nextFallingEdge) << std::endl;
    }
}

isize
Datasette::_size()
{
    util::SerCounter counter;
    
    applyToPersistentItems(counter);
    applyToResetItems(counter);
    
    counter << size;
    for (isize i = 0; i < size; i++) counter << pulses[i].cycles;

    return counter.count;
}

isize
Datasette::didLoadFromBuffer(const u8 *buffer)
{
    util::SerReader reader(buffer);

    // Free previously allocated memory
    dealloc();

    // Load size
    reader << size;

    // Make sure a corrupted value won't steal all memory
    if (size > 0x8FFFF) { size = 0; }

    // Create a new pulse buffer
    alloc(size);

    // Load pulses from buffer
    for (isize i = 0; i < size; i++) reader << pulses[i].cycles;

    return (isize)(reader.ptr - buffer);
}

isize
Datasette::didSaveToBuffer(u8 *buffer)
{
    util::SerWriter writer(buffer);

    // Save size
    writer << size;
    
    // Save pulses to buffer
    for (isize i = 0; i < size; i++) writer << pulses[i].cycles;

    return (isize)(writer.ptr - buffer);
}

void
Datasette::resetConfig()
{
    assert(isPoweredOff());
    auto &defaults = c64.defaults;

    std::vector <Option> options = {

        OPT_DAT_MODEL,
        OPT_DAT_CONNECT
    };

    for (auto &option : options) {
        setConfigItem(option, defaults.get(option));
    }
}

i64
Datasette::getConfigItem(Option option) const
{
    switch (option) {

        case OPT_DAT_MODEL:     return config.model;
        case OPT_DAT_CONNECT:   return config.connected;

        default:
            fatalError;
    }
}

void
Datasette::setConfigItem(Option option, i64 value)
{
    switch (option) {

        case OPT_DAT_MODEL:

            config.model = DatasetteModel(value);
            return;

        case OPT_DAT_CONNECT:

            config.connected = bool(value);
            scheduleNextDatEvent();
            msgQueue.put(value ? MSG_VC1530_CONNECT : MSG_VC1530_DISCONNECT);
            return;

        default:
            return;
    }
}

util::Time
Datasette::tapeDuration(isize pos)
{
    util::Time result;
    
    for (isize i = 0; i < pos && i < size; i++) {
        result += pulses[i].delay();
    }
    
    return result;
}

void
Datasette::insertTape(TAPFile &file)
{
    {   SUSPENDED

        // Allocate pulse buffer
        isize numPulses = file.numPulses();
        alloc(numPulses);
        
        debug(TAP_DEBUG, "Inserting tape (%ld pulses)...\n", numPulses);
        
        // Read pulses
        file.seek(0);
        for (isize i = 0; i < numPulses; i++) {
            
            pulses[i].cycles = (i32)file.read();
            assert(pulses[i].cycles != -1);
        }
        
        // Rewind the tape
        rewind();
        
        // Inform the GUI
        msgQueue.put(MSG_VC1530_TAPE, 1);
    }
}

void
Datasette::ejectTape()
{
    // Only proceed if a tape is present
    if (!hasTape()) return;

    {   SUSPENDED
        
        debug(TAP_DEBUG, "Ejecting tape...\n");
        
        pressStop();
        rewind();
        dealloc();
        
        msgQueue.put(MSG_VC1530_TAPE, 0);
    }
}

void
Datasette::rewind(isize seconds)
{
    i64 old = (i64)counter.asSeconds();

    // Start at the beginning
    counter = util::Time(0);
    head = 0;
    
    // Fast forward to the requested position
    while (counter.asMilliseconds() < 1000 * seconds && head + 1 < size) {
        advanceHead();
    }
    
    // Inform the GUI
    if (old != (i64)counter.asSeconds()) {
        msgQueue.put(MSG_VC1530_COUNTER, (i64)counter.asSeconds());
    }
}

void
Datasette::advanceHead()
{
    assert(head < size);
    
    i64 old = (i64)counter.asSeconds();

    counter += pulses[head].delay();
    head++;
    
    // Inform the GUI
    if (old != (i64)counter.asSeconds()) {
        msgQueue.put(MSG_VC1530_COUNTER, (i64)counter.asSeconds());
    }
}

void
Datasette::pressPlay()
{
    debug(TAP_DEBUG, "pressPlay\n");

    // Only proceed if the device is connected
    if (!config.connected) return;

    // Only proceed if a tape is present
    if (!hasTape()) return;
    
    playKey = true;

    // Schedule the first pulse
    schedulePulse(head);
    advanceHead();

    // Update the execution event slot
    scheduleNextDatEvent();

    msgQueue.put(MSG_VC1530_PLAY, 1);
}

void
Datasette::pressStop()
{
    debug(TAP_DEBUG, "pressStop\n");

    playKey = false;
    motor = false;

    // Update the execution event slot
    scheduleNextDatEvent();

    msgQueue.put(MSG_VC1530_PLAY, 0);
}

void
Datasette::setMotor(bool value)
{
    if (motor != value) {

        // Only proceed if the device is connected
        if (!config.connected) return;

        motor = value;

        // Update the execution event slot
        scheduleNextDatEvent();

        /* When the motor is switched on or off, a MSG_VC1530_MOTOR message is
         * sent to the GUI. However, if we sent the message immediately, we
         * would risk to flood the message queue, because some C64 switch the
         * motor state insanely often. To cope with this situation, we set a
         * counter and let the vsync handler send the message once the counter
         * has timed out.
         */
        c64.scheduleRel<SLOT_MOT>(MSEC(200), motor ? MOT_START : MOT_STOP);
    }
}

void
Datasette::processMotEvent(EventID event)
{
    switch (event) {

        case MOT_START: msgQueue.put(MSG_VC1530_MOTOR, true);
        case MOT_STOP:  msgQueue.put(MSG_VC1530_MOTOR, false);

        default:
            break;
    }

    c64.cancel<SLOT_MOT>();
}

void
Datasette::processDatEvent(EventID event, isize cycles)
{
    assert(event == DAT_EXECUTE);

    for (isize i = 0; i < cycles; i++) {

        if (--nextRisingEdge == 0) {

            cia1.triggerRisingEdgeOnFlagPin();
        }

        if (--nextFallingEdge == 0) {

            cia1.triggerFallingEdgeOnFlagPin();

            if (head < size) {

                schedulePulse(head);
                advanceHead();

            } else {

                pressStop();
            }
        }
    }

    scheduleNextDatEvent();
}

void
Datasette::scheduleNextDatEvent()
{
    if (playKey && motor && hasTape() && config.connected) {

        // Call the execution handler every 16 cycles
        c64.scheduleRel<SLOT_DAT>(16, DAT_EXECUTE, 16);

    } else {

        c64.cancel<SLOT_DAT>();
    }
}

void
Datasette::schedulePulse(isize nr)
{
    assert(nr < size);
    
    // The VC1530 uses square waves with a 50% duty cycle
    nextRisingEdge = pulses[nr].cycles / 2;
    nextFallingEdge = pulses[nr].cycles;
}

}
