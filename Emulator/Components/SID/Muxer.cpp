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
#include "Muxer.h"
#include "Emulator.h"
#include "IOUtils.h"

#include <algorithm>
#include <cmath>

namespace vc64 {

Muxer::Muxer(C64 &ref) : SubComponent(ref)
{        
    subComponents = std::vector<CoreComponent *> {
        
        &resid[0],
        &resid[1],
        &resid[2],
        &resid[3],
        &fastsid[0],
        &fastsid[1],
        &fastsid[2],
        &fastsid[3]
    };

    for (int i = 0; i < 4; i++) {
        resid[i].setClockFrequency(PAL_CLOCK_FREQUENCY);
        fastsid[i].setClockFrequency(PAL_CLOCK_FREQUENCY);
    }
}

void
Muxer::operator << (SerResetter &worker)
{
    serialize(worker);
    
    if (isHardResetter(worker)) clearStats();
    clear();
}

void
Muxer::clear()
{
    debug(AUDBUF_DEBUG, "clear()\n");
    
    // Wipe out the ringbuffer
    stream.lock();
    stream.wipeOut();
    stream.alignWritePtr();
    stream.unlock();
}

SIDConfig
Muxer::getDefaultConfig()
{
    SIDConfig defaults;
    
    defaults.revision = MOS_8580;
    defaults.powerSave = false;
    defaults.enabled = 1;
    defaults.address[0] = 0xD400;
    defaults.address[1] = 0xD420;
    defaults.address[2] = 0xD440;
    defaults.address[3] = 0xD460;
    defaults.filter = true;
    defaults.engine = SIDENGINE_RESID;
    defaults.sampling = SAMPLING_INTERPOLATE;
    defaults.volL = 50;
    defaults.volR = 50;
    
    for (isize i = 0; i < 4; i++) {
        defaults.vol[i] = 400;
        defaults.pan[i] = 0;
    }
    
    return defaults;
}

void
Muxer::resetConfig()
{
    assert(isPoweredOff());
    auto &defaults = emulator.defaults;

    std::vector <Option> options = {

        OPT_SID_REVISION,
        OPT_SID_FILTER,
        OPT_SID_ENGINE,
        OPT_SID_SAMPLING,
        OPT_AUDVOLL,
        OPT_AUDVOLR
    };

    for (auto &option : options) {
        setConfigItem(option, defaults.get(option));
    }

    std::vector <Option> moreOptions = {

        OPT_SID_ENABLE,
        OPT_SID_ADDRESS,
        OPT_AUDVOL,
        OPT_AUDPAN
    };

    for (auto &option : moreOptions) {
        for (isize i = 0; i < 4; i++) {
            setConfigItem(option, i, defaults.get(option, i));
        }
    }
}

i64
Muxer::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_SID_REVISION:
            return config.revision;
            
        case OPT_SID_POWER_SAVE:
            return config.powerSave;
            
        case OPT_SID_FILTER:
            return config.filter;
            
        case OPT_SID_ENGINE:
            return config.engine;
            
        case OPT_SID_SAMPLING:
            return config.sampling;
            
        case OPT_AUDVOLL:
            return config.volL;

        case OPT_AUDVOLR:
            return config.volR;
            
        default:
            fatalError;
    }
}

i64
Muxer::getConfigItem(Option option, long id) const
{
    
    switch (option) {
            
        case OPT_SID_ENABLE:
            return GET_BIT(config.enabled, id);
            
        case OPT_SID_ADDRESS:
            return config.address[id];
            
        case OPT_AUDVOL:
            return config.vol[id];

        case OPT_AUDPAN:
            return config.pan[id];

        default:
            fatalError;
    }
}

void
Muxer::setConfigItem(Option option, i64 value)
{
    bool wasMuted = isMuted();

    switch (option) {

        case OPT_HOST_SAMPLE_RATE:
        {
            setSampleRate(double(value));
            return;
        }
        case OPT_SID_POWER_SAVE:
        {
            {   SUSPENDED

                config.powerSave = bool(value);
            }
            return;
        }
        case OPT_SID_REVISION:
        {
            if (!SIDRevisionEnum::isValid(value)) {
                throw VC64Error(ERROR_OPT_INVARG, SIDRevisionEnum::keyList());
            }
            
            {   SUSPENDED
                
                config.revision = SIDRevision(value);
                for (int i = 0; i < 4; i++) {
                    resid[i].setRevision(SIDRevision(value));
                    fastsid[i].setRevision(SIDRevision(value));
                }
            }
            return;
        }
        case OPT_SID_FILTER:
        {
            {   SUSPENDED
                
                config.filter = bool(value);
                for (int i = 0; i < 4; i++) {
                    resid[i].setAudioFilter(bool(value));
                    fastsid[i].setAudioFilter(bool(value));
                }
            }
            return;
        }
        case OPT_SID_ENGINE:
        {
            if (!SIDEngineEnum::isValid(value)) {
                throw VC64Error(ERROR_OPT_INVARG, SIDEngineEnum::keyList());
            }

            {   SUSPENDED

                config.engine = SIDEngine(value);
            }
            return;
        }
        case OPT_SID_SAMPLING:
        {
            if (!SamplingMethodEnum::isValid(value)) {
                throw VC64Error(ERROR_OPT_INVARG, SamplingMethodEnum::keyList());
            }

            {   SUSPENDED
                
                config.sampling = SamplingMethod(value);
                for (int i = 0; i < 4; i++) {
                    resid[i].setSamplingMethod(SamplingMethod(value));
                    // Note: fastSID has no such option
                }
            }
            return;
        }
        case OPT_AUDVOLL:
            
            config.volL = std::clamp(value, 0LL, 100LL);
            volL.set(powf((float)config.volL / 50, 1.4f));
            
            if (wasMuted != isMuted()) {
                msgQueue.put(MSG_MUTE, isMuted());
            }
            return;
            
        case OPT_AUDVOLR:

            config.volR = std::clamp(value, 0LL, 100LL);
            volR.set(powf((float)config.volR / 50, 1.4f));

            if (wasMuted != isMuted()) {
                msgQueue.put(MSG_MUTE, isMuted());
            }
            return;
            
        default:
            fatalError;
    }
}

void
Muxer::setConfigItem(Option option, long id, i64 value)
{
    bool wasMuted = isMuted();

    switch (option) {

        case OPT_SID_ENABLE:
        {
            assert(id >= 0 && id <= 3);

            if (id == 0 && value == false) {
                warn("SID 0 can't be disabled\n");
                return;
            }
            
            if (!!GET_BIT(config.enabled, id) == value) {
                return;
            }
            
            {   SUSPENDED
                
                REPLACE_BIT(config.enabled, id, value);
                clearSampleBuffer(id);
                
                for (int i = 0; i < 4; i++) {
                    resid[i].reset(true);
                    fastsid[i].reset(true);
                }
            }
            return;
        }
        case OPT_SID_ADDRESS:
        {
            assert(id >= 0 && id <= 3);

            if (id == 0 && value != 0xD400) {
                warn("SID 0 can't be remapped\n");
                return;
            }

            if (value < 0xD400 || value > 0xD7E0 || (value & 0x1F)) {
                throw VC64Error(ERROR_OPT_INVARG, "D400, D420 ... D7E0");
            }

            if (config.address[id] == value) {
                return;
            }
            
            {   SUSPENDED
                
                config.address[id] = (u16)value;
                clearSampleBuffer(id);
            }
            return;
        }
        case OPT_AUDVOL:
            
            assert(id >= 0 && id <= 3);

            config.vol[id] = std::clamp(value, 0LL, 100LL);
            vol[id] = powf((float)config.vol[id] / 100, 1.4f) * 0.000025f;
#ifdef __EMSCRIPTEN__
            vol[id] *= 0.15f;
#endif
            if (wasMuted != isMuted()) {
                msgQueue.put(MSG_MUTE, isMuted());
            }

            return;
            
        case OPT_AUDPAN:
            
            assert(id >= 0 && id <= 3);

            config.pan[id] = value;
            pan[id] = float(0.5 * (sin(config.pan[id] * M_PI / 200.0) + 1));
            return;

        default:
            fatalError;
    }
}

bool
Muxer::isMuted() const
{
    if (config.volL == 0 && config.volR == 0) return true;
    
    return
    config.vol[0] == 0 &&
    config.vol[1] == 0 &&
    config.vol[2] == 0 &&
    config.vol[3] == 0;
}

u32
Muxer::getClockFrequency()
{
    u32 result = resid[0].getClockFrequency();
    
    for (int i = 0; i < 4; i++) {
        assert(resid[i].getClockFrequency() == result);
        assert(fastsid[i].getClockFrequency() == result);
    }
    
    return result;
}

void
Muxer::setClockFrequency(u32 frequency)
{
    trace(SID_DEBUG, "Setting clock frequency to %d\n", frequency);

    cpuFrequency = frequency;

    for (int i = 0; i < 4; i++) {
        resid[i].setClockFrequency(frequency);
        fastsid[i].setClockFrequency(frequency);
    }
}

double
Muxer::getSampleRate() const
{
    double result = resid[0].getSampleRate();
    
    for (int i = 0; i < 4; i++) {
        if (resid[i].getSampleRate() != result) {
            warn("%f != %f\n", resid[i].getSampleRate(), result);
        }
        if (fastsid[i].getSampleRate() != result) {
            warn("%f != %f\n", fastsid[i].getSampleRate(), result);
        }
        assert(resid[i].getSampleRate() == result);
        assert(fastsid[i].getSampleRate() == result);
    }
    
    return result;
}

void
Muxer::setSampleRate(double rate)
{
    trace(true, "Setting sample rate to %f\n", rate);

    sampleRate = rate;
    
    for (int i = 0; i < 4; i++) {
        resid[i].setSampleRate(rate);
        fastsid[i].setSampleRate(rate);
    }
}

void 
Muxer::operator << (SerReader &worker)
{
    serialize(worker);

    for (isize i = 0; i < 4; i++) sidStream[i].clear(0);
}

void
Muxer::_run()
{
    if (volL.current == 0.0) {
        debug(AUDBUF_DEBUG, "Ramping up from 0\n");
        clear();
    }
    rampUp();
}

void
Muxer::_pause()
{
    rampDown();
}

void
Muxer::_warpOn()
{
    /* Warping has the unavoidable drawback that audio playback gets out of
     * sync. To cope with it, we ramp down the volume when warping is switched
     * on and fade in smoothly when it is switched off.
     */
    rampDown();
}

void
Muxer::_warpOff()
{
    rampUp();
    clear();
}

void
Muxer::_dump(Category category, std::ostream& os) const
{
    using namespace util;
    
    if (category == Category::Config) {
        
        os << tab("Chip revision");
        os << SIDRevisionEnum::key(config.revision) << std::endl;
        os << tab("Power save mode");
        os << bol(config.powerSave, "during warp", "never") << std::endl;
        os << tab("Enable mask");
        os << dec(config.enabled) << std::endl;
        os << tab("1st extra SID");
        os << hex(config.address[1]) << std::endl;
        os << tab("2nd extra SID");
        os << hex(config.address[2]) << std::endl;
        os << tab("3rd extra SID");
        os << hex(config.address[3]) << std::endl;
        os << tab("Filter");
        os << bol(config.filter) << std::endl;
        os << tab("Engine");
        os << SIDEngineEnum::key(config.engine) << std::endl;
        os << tab("Sampling");
        os << SamplingMethodEnum::key(config.sampling) << std::endl;
        os << tab("Volume 1");
        os << config.vol[0] << std::endl;
        os << tab("Volume 2");
        os << config.vol[1] << std::endl;
        os << tab("Volume 3");
        os << config.vol[2] << std::endl;
        os << tab("Volume 4");
        os << config.vol[3] << std::endl;
        os << tab("Volume L");
        os << config.volL << std::endl;
        os << tab("Volume R");
        os << config.volR << std::endl;

    } else {

        _dump(category, os, 0);
    }
}

void
Muxer::_dump(Category category, std::ostream& os, isize nr) const
{
    switch (config.engine) {
            
        case SIDENGINE_FASTSID: fastsid[nr].dump(category, os); break;
        case SIDENGINE_RESID:   resid[nr].dump(category, os); break;

        default:
            fatalError;
    }
}

SIDStats
Muxer::getStats()
{
    stats.fillLevel = stream.fillLevel();
    return stats;
}

void
Muxer::clearStats()
{
    memset(&stats, 0, sizeof(stats));
}

SIDInfo
Muxer::getInfo(isize nr)
{
    assert(nr < 4);
    
    SIDInfo info = { };
    
    switch (config.engine) {
            
        case SIDENGINE_FASTSID: info = fastsid[nr].getInfo(); break;
        case SIDENGINE_RESID:   info = resid[nr].getState(); break;

        default:
            fatalError;
    }
    
    info.potX = port1.mouse.readPotX() & port2.mouse.readPotX();
    info.potY = port1.mouse.readPotY() & port2.mouse.readPotY();
    
    return info;
}

VoiceInfo
Muxer::getVoiceInfo(isize nr, isize voice)
{
    assert(nr >= 0 && nr <= 3);

    switch (config.engine) {
            
        case SIDENGINE_FASTSID: return fastsid[nr].getVoiceInfo(voice);
        case SIDENGINE_RESID:   return resid[nr].getVoiceInfo(voice);

        default:
            fatalError;
    }
}

CoreComponent &
Muxer::getSID(isize nr)
{
    assert(nr >= 0 && nr <= 3);
    
    switch (config.engine) {
            
        case SIDENGINE_FASTSID: return fastsid[nr];
        case SIDENGINE_RESID:   return resid[nr];

        default:
            fatalError;
    }
}

void
Muxer::rampUp()
{
    trace(AUDBUF_DEBUG, "rampUp()\n");

    const isize steps = 20000;
    volL.fadeIn(steps);
    volR.fadeIn(steps);
    
    ignoreNextUnderOrOverflow();
}

void
Muxer::rampUpFromZero()
{
    trace(AUDBUF_DEBUG, "rampUpFromZero()\n");

    volL.current = 0;
    volR.current = 0;

    rampUp();
}

void
Muxer::rampDown()
{
    trace(AUDBUF_DEBUG, "rampDown()\n");

    const isize steps = 2000;
    volL.fadeOut(steps);
    volR.fadeOut(steps);
    
    ignoreNextUnderOrOverflow();
}

isize
Muxer::mappedSID(u16 addr) const
{
    addr &= 0xFFE0;
    
    if (isEnabled(1) && addr == config.address[1]) return 1;
    if (isEnabled(2) && addr == config.address[2]) return 2;
    if (isEnabled(3) && addr == config.address[3]) return 3;

    return 0;
}

u8 
Muxer::peek(u16 addr)
{
    // Get SIDs up to date
    executeUntil(cpu.clock);

    // Select the target SID
    isize sidNr = config.enabled > 1 ? mappedSID(addr) : 0;

    addr &= 0x1F;

    if (sidNr == 0) {
        
        if (addr == 0x19) {
            
            port1.updatePotX();
            port2.updatePotX();
            return readPotX();
        }
        if (addr == 0x1A) {

            port1.updatePotY();
            port2.updatePotY();
            return readPotY();
        }
    }
    
    switch (config.engine) {
            
        case SIDENGINE_FASTSID: return fastsid[sidNr].peek(addr);
        case SIDENGINE_RESID:   return resid[sidNr].peek(addr);
            
        default:
            fatalError;
    }
}

u8
Muxer::spypeek(u16 addr) const
{
    // Select the target SID
    isize sidNr = config.enabled > 1 ? mappedSID(addr) : 0;

    addr &= 0x1F;

    if (sidNr == 0) {
        if (addr == 0x19) { return port1.readPotX() & port2.readPotX(); }
        if (addr == 0x1A) { return port1.readPotY() & port2.readPotY(); }
    }

    /* At the moment, only FastSID allows us to peek into the SID registers
     * without causing side effects. Hence, we get the return value from there,
     * regardless of the selected SID engine.
     */
    return fastsid[sidNr].spypeek(addr);
}

u8
Muxer::readPotX() const
{
    u8 result = 0xFF;

    if (GET_BIT(cia1.getPA(), 7) == 0) result &= port1.readPotX();
    if (GET_BIT(cia1.getPA(), 6) == 0) result &= port2.readPotX();

    return result;
}

u8
Muxer::readPotY() const
{
    u8 result = 0xFF;

    if (GET_BIT(cia1.getPA(), 7) == 0) result &= port1.readPotY();
    if (GET_BIT(cia1.getPA(), 6) == 0) result &= port2.readPotY();

    return result;
}

void 
Muxer::poke(u16 addr, u8 value)
{
    trace(SIDREG_DEBUG, "poke(%x,%x)\n", addr, value);
    
    // Get SID up to date
    executeUntil(cpu.clock);

    // Select the target SID
    isize sidNr = config.enabled > 1 ? mappedSID(addr) : 0;

    addr &= 0x1F;
    
    // Keep both SID implementations up to date
    resid[sidNr].poke(addr, value);
    fastsid[sidNr].poke(addr, value);
}

void
Muxer::executeUntil(Cycle targetCycle)
{
    assert(targetCycle >= cycles);
    
    // Skip sample synthesis in power-safe mode
    if (volL.current == 0 && volR.current == 0 && config.powerSave) {

        /* https://sourceforge.net/p/vice-emu/bugs/1374/
         *
         * Due to a bug in reSID, pending register writes are dropped if we
         * skip sample synthesis if SAMPLE_FAST and MOS8580 are selected both.
         * As a workaround, we ignore the power-saving setting in this case.
         */
        if (config.revision != MOS_8580 || config.sampling != SAMPLING_FAST) {

            cycles = targetCycle;
            return;
        }
    }
    
    isize missingCycles  = isize(targetCycle - cycles);
    isize consumedCycles = executeCycles(missingCycles);

    cycles += consumedCycles;
    
    debug(SID_EXEC,
          "target: %lld missing: %ld consumed: %ld reached: %lld still missing: %lld\n",
          targetCycle, missingCycles, consumedCycles, cycles, targetCycle - cycles);
}

isize
Muxer::executeCycles(isize numCycles)
{
    isize numSamples;
    
    // Run reSID for at least one cycle to make pipelined writes work
    if (numCycles == 0) {
        
        numCycles = 1;
        debug(SID_EXEC, "Running SIDs for an extra cycle\n");
    }
    
    switch (config.engine) {
            
        case SIDENGINE_FASTSID:

            // Run the primary SID (which is always enabled)
            numSamples = fastsid[0].executeCycles(numCycles, sidStream[0]);
            
            // Run all other SIDS (if any)
            if (config.enabled > 1) {
                for (isize i = 1; i < 4; i++) {
                    if (isEnabled(i)) {
                        isize numSamples2 = fastsid[i].executeCycles(numCycles, sidStream[i]);
                        numSamples = std::min(numSamples, numSamples2);
                    }
                }
            }
            break;

        case SIDENGINE_RESID:

            // Run the primary SID (which is always enabled)
            numSamples = resid[0].executeCycles(numCycles, sidStream[0]);
            
            // Run all other SIDS (if any)
            if (config.enabled > 1) {
                for (isize i = 1; i < 4; i++) {
                    if (isEnabled(i)) {
                        isize numSamples2 = resid[i].executeCycles(numCycles, sidStream[i]);
                        numSamples = std::min(numSamples, numSamples2);
                    }
                }
            }
            break;

        default:
            fatalError;
    }
    
    // Produce the final stereo stream
    (config.enabled > 1) ? mixMultiSID(numSamples) : mixSingleSID(numSamples);
    
    return numCycles;
}

void
Muxer::mixSingleSID(isize numSamples)
{    
    stream.lock();
    
    // Check for buffer overflow
    if (stream.free() < numSamples) {
        handleBufferOverflow();
    }
    
    debug(SID_EXEC, "vol0: %f pan0: %f volL: %f volR: %f\n",
          vol[0], pan[0], volL.current, volR.current);

    // Convert sound samples to floating point values and write into ringbuffer
    for (isize i = 0; i < numSamples; i++) {
        
        // Read SID sample from ring buffer
        float ch0 = (float)sidStream[0].read() * vol[0];
        
        // Compute left and right channel output
        float l = ch0 * (1 - pan[0]);
        float r = ch0 * pan[0];

        // Apply master volume
        l *= volL.current;
        r *= volR.current;
        
        // Apply ear protection
        assert(abs(l) < 1.0);
        assert(abs(r) < 1.0);
        
        stream.write(SamplePair { l, r } );
    }
    stream.unlock();
}

void
Muxer::mixMultiSID(isize numSamples)
{
    stream.lock();
    
    // Check for buffer overflow
    if (stream.free() < numSamples) {
        handleBufferOverflow();
    }
    
    debug(SID_EXEC, "vol0: %f pan0: %f volL: %f volR: %f\n",
          vol[0], pan[0], volL.current, volR.current);

    // Convert sound samples to floating point values and write into ringbuffer
    for (isize i = 0; i < numSamples; i++) {
        
        float ch0, ch1, ch2, ch3, l, r;
        
        ch0 = (float)sidStream[0].read()  * vol[0];
        ch1 = (float)sidStream[1].read(0) * vol[1];
        ch2 = (float)sidStream[2].read(0) * vol[2];
        ch3 = (float)sidStream[3].read(0) * vol[3];

        // Compute left channel output
        l =
        ch0 * (1 - pan[0]) + ch1 * (1 - pan[1]) +
        ch2 * (1 - pan[2]) + ch3 * (1 - pan[3]);

        // Compute right channel output
        r =
        ch0 * pan[0] + ch1 * pan[1] +
        ch2 * pan[2] + ch3 * pan[3];

        // Apply master volume
        l *= volL.current;
        r *= volR.current;
        
        // Apply ear protection
        assert(abs(l) < 1.0);
        assert(abs(r) < 1.0);
        
        stream.write(SamplePair { l, r } );
    }
    stream.unlock();
}

void
Muxer::clearSampleBuffers()
{
    for (int i = 0; i < 4; i++) clearSampleBuffer(i);
}

void
Muxer::clearSampleBuffer(long nr)
{
    sidStream[nr].clear(0);
}

void
Muxer::ringbufferData(isize offset, float *left, float *right)
{
    const SamplePair &pair = stream.current((int)offset);
    *left = pair.left;
    *right = pair.right;
}

void
Muxer::handleBufferUnderflow()
{
    // There are two common scenarios in which buffer underflows occur:
    //
    // (1) The consumer runs slightly faster than the producer.
    // (2) The producer is halted or not startet yet.
    
    trace(AUDBUF_DEBUG, "BUFFER UNDERFLOW (r: %ld w: %ld)\n", stream.r, stream.w);

    // Reset the write pointer
    stream.alignWritePtr();
    
    // Determine the elapsed seconds since the last pointer adjustment
    auto elapsedTime = util::Time::now() - lastAlignment;
    lastAlignment = util::Time::now();
    
    // Adjust the sample rate, if condition (1) holds
    if (elapsedTime.asSeconds() > 10.0) {

        stats.bufferUnderflows++;
        
        // Increase the sample rate based on what we've measured
        isize offPerSecond = (isize)(stream.count() / elapsedTime.asSeconds());
        setSampleRate(getSampleRate() + offPerSecond);
    }
}

void
Muxer::handleBufferOverflow()
{
    // There are two common scenarios in which buffer overflows occur:
    //
    // (1) The consumer runs slightly slower than the producer
    // (2) The consumer is halted or not startet yet
    
    trace(AUDBUF_DEBUG, "BUFFER OVERFLOW (r: %ld w: %ld)\n", stream.r, stream.w);

    // Reset the write pointer
    stream.alignWritePtr();
    
    // Determine the number of elapsed seconds since the last adjustment
    auto elapsedTime = util::Time::now() - lastAlignment;
    lastAlignment = util::Time::now();
    trace(AUDBUF_DEBUG, "elapsedTime: %f\n", elapsedTime.asSeconds());
    
    // Adjust the sample rate, if condition (1) holds
    if (elapsedTime.asSeconds() > 10.0) {
        
        stats.bufferOverflows++;
        
        // Decrease the sample rate based on what we've measured
        isize offPerSecond = (isize)(stream.count() / elapsedTime.asSeconds());
        double newSampleRate = getSampleRate() - offPerSecond;

        trace(AUDBUF_DEBUG, "Changing sample rate to %f\n", newSampleRate);
        setSampleRate(newSampleRate);
    }
}

void
Muxer::ignoreNextUnderOrOverflow()
{
    lastAlignment = util::Time::now();
}

void
Muxer::copyMono(float *target, isize n)
{
    if (recorder.isRecording()) {
        for (isize i = 0; i < n; i++) target[i] = 0.0;
        return;
    }
    
    stream.lock();
    
    // Check for a buffer underflow
    if (stream.count() < n) handleBufferUnderflow();

    // Copy sound samples
    stream.copyMono(target, n, volL, volR);
    
    stream.unlock();
}

void
Muxer::copyStereo(float *target1, float *target2, isize n)
{
    if (recorder.isRecording()) {
        for (isize i = 0; i < n; i++) target1[i] = target2[i] = 0.0;
        return;
    }

    stream.lock();
    
    // Check for a buffer underflow
    if (stream.count() < n) handleBufferUnderflow();

    // Copy sound samples
    stream.copyStereo(target1, target2, n, volL, volR);
    
    stream.unlock();
}

void
Muxer::copyInterleaved(float *target, isize n)
{
    if (recorder.isRecording()) {
        for (isize i = 0; i < n; i++) target[i] = 0.0;
        return;
    }

    stream.lock();
    
    // Check for a buffer underflow
    if (stream.count() < n) handleBufferUnderflow();

    // Read sound samples
    stream.copyInterleaved(target, n, volL, volR);
    
    stream.unlock();
}

float
Muxer::draw(u32 *buffer, isize width, isize height,
            float maxAmp, u32 color, isize sid) const
{
    auto samples = new float[width][2];
    isize hheight = height / 2;
    float newMaxAmp = 0.001f, dw;

    // Gather data
    switch (sid) {

        case 0: case 1: case 2: case 3:

            dw = sidStream[sid].cap() / float(width);

            for (isize w = 0; w < width; w++) {

                auto sample = sidStream[sid].current(isize(w * dw));
                samples[w][0] = float(abs(sample));
                samples[w][1] = float(abs(sample));
            }
            break;

        default:

            dw = stream.cap() / float(width);

            for (isize w = 0; w < width; w++) {

                auto sample = stream.current(isize(w * dw));
                samples[w][0] = abs(sample.left);
                samples[w][1] = abs(sample.right);
            }
            break;
    }

    // Clear buffer
    for (isize i = 0; i < width * height; i++) buffer[i] = 0;

    // Draw waveform
    for (isize w = 0; w < width; w++) {

        u32 *ptr = buffer + width * hheight + w;

        if (samples[w][0] == 0 && samples[w][1] == 0) {

            // Draw some noise to make it look sexy
            *ptr = color;
            if (rand() % 2) *(ptr + width) = color;
            if (rand() % 2) *(ptr - width) = color;

        } else {

            // Remember the highest amplitude
            if (samples[w][0] > newMaxAmp) newMaxAmp = samples[w][0];
            if (samples[w][1] > newMaxAmp) newMaxAmp = samples[w][1];

            // Scale the sample
            isize scaledL = std::min(isize(samples[w][0] * hheight / maxAmp), hheight);
            isize scaledR = std::min(isize(samples[w][1] * hheight / maxAmp), hheight);

            // Draw vertical lines
            for (isize j = 0; j < scaledL; j++) *(ptr - j * width) = color;
            for (isize j = 0; j < scaledR; j++) *(ptr + j * width) = color;
        }
    }

    delete[] samples;
    return newMaxAmp;
}

}
