// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "C64.h"

SIDBridge::SIDBridge(C64 &ref) : C64Component(ref)
{
	setDescription("SIDBridge");
        
    subComponents = vector<HardwareComponent *> {
        
        &resid,
        &fastsid
    };
    
    // Register snapshot items
    SnapshotItem items[] = {
        
        // Configuration items
        { &config.engine,   sizeof(config.engine),  KEEP_ON_RESET },
        { &config.filter,   sizeof(config.filter),  KEEP_ON_RESET },

        // Internal state
        { &cycles,          sizeof(cycles),         CLEAR_ON_RESET },
        { NULL,             0,                      0 }};
    
    registerSnapshotItems(items, sizeof(items));
    
    config.engine = ENGINE_RESID;
}

void
SIDBridge::setRevision(SIDRevision rev)
{
    assert(isSIDRevision(rev));
    
    config.revision = rev;
    
    resid.setRevision(rev);
    fastsid.setRevision(rev);
}

void
SIDBridge::setFilter(bool value)
{
    config.filter = value;
    
    resid.setAudioFilter(value);
    fastsid.setAudioFilter(value);
}

void
SIDBridge::setEngine(SIDEngine value)
{
    config.engine = value;
}

void
SIDBridge::setSamplingMethod(SamplingMethod method)
{
    config.sampling = method;
    
    // Option is ReSID only
    resid.setSamplingMethod(method);
}

double
SIDBridge::getSampleRate()
{
    switch (config.engine) {
            
        case ENGINE_FASTSID: return (double)fastsid.getSampleRate();
        case ENGINE_RESID:   return resid.getSampleRate();
            
        default:
            assert(false);
            return 0;
    }
}

void
SIDBridge::setSampleRate(double rate)
{
    debug(SID_DEBUG, "Changing sample rate from %f to %f\n", getSampleRate(), rate);
    
    resid.setSampleRate(rate);
    fastsid.setSampleRate((u32)rate);
}

u32
SIDBridge::getClockFrequency()
{
    switch (config.engine) {
            
        case ENGINE_FASTSID: return fastsid.getClockFrequency();
        case ENGINE_RESID:   return resid.getClockFrequency();
            
        default:
            assert(false);
            return 0;
    }
}

void
SIDBridge::_run()
{
    clearRingbuffer();
}

void
SIDBridge::_pause()
{
    clearRingbuffer();
}

void
SIDBridge::_reset()
{
    // Clear snapshot items marked with 'CLEAR_ON_RESET'
     if (snapshotItems != NULL)
         for (unsigned i = 0; snapshotItems[i].data != NULL; i++)
             if (snapshotItems[i].flags & CLEAR_ON_RESET)
                 memset(snapshotItems[i].data, 0, snapshotItems[i].size);

    clearRingbuffer();
    resid._reset();
    fastsid._reset();
    
    volume = 100000;
    targetVolume = 100000;
}

void 
SIDBridge::_dump()
{
    msg("ReSID:\n");
    msg("------\n");
    _dump(resid.getInfo());

    msg("FastSID:\n");
    msg("--------\n");
    msg("    Chip model: %d (%s)\n", sidRevisionName(fastsid.getRevision()));
    msg(" Sampling rate: %d\n", fastsid.getSampleRate());
    msg(" CPU frequency: %d\n", fastsid.getClockFrequency());
    msg("Emulate filter: %s\n", fastsid.getAudioFilter() ? "yes" : "no");
    msg("\n");
    _dump(fastsid.getInfo());
}

void
SIDBridge::_dump(SIDInfo info)
{
    u8 ft = info.filterType;
    msg("        Volume: %d\n", info.volume);
    msg("   Filter type: %s\n",
        (ft == FASTSID_LOW_PASS) ? "LOW PASS" :
        (ft == FASTSID_HIGH_PASS) ? "HIGH PASS" :
        (ft == FASTSID_BAND_PASS) ? "BAND PASS" : "NONE");
    msg("Filter cut off: %d\n\n", info.filterCutoff);
    msg("Filter resonance: %d\n\n", info.filterResonance);
    msg("Filter enable bits: %d\n\n", info.filterEnableBits);

    for (unsigned i = 0; i < 3; i++) {
        VoiceInfo vinfo = getVoiceInfo(i);
        u8 wf = vinfo.waveform;
        msg("Voice %d:       Frequency: %d\n", i, vinfo.frequency);
        msg("             Pulse width: %d\n", vinfo.pulseWidth);
        msg("                Waveform: %s\n",
            (wf == FASTSID_NOISE) ? "NOISE" :
            (wf == FASTSID_PULSE) ? "PULSE" :
            (wf == FASTSID_SAW) ? "SAW" :
            (wf == FASTSID_TRIANGLE) ? "TRIANGLE" : "NONE");
        msg("         Ring modulation: %s\n", vinfo.ringMod ? "yes" : "no");
        msg("               Hard sync: %s\n", vinfo.hardSync ? "yes" : "no");
        msg("             Attack rate: %d\n", vinfo.attackRate);
        msg("              Decay rate: %d\n", vinfo.decayRate);
        msg("            Sustain rate: %d\n", vinfo.sustainRate);
        msg("            Release rate: %d\n", vinfo.releaseRate);
    }
}

void
SIDBridge::_setWarp(bool enable)
{
    if (enable) {
        
        // Warping has the unavoidable drawback that audio playback gets out of
        // sync. To cope with this issue, we ramp down the volume when warping
        // is switched on and fade in smoothly when it is switched off.
        sid.rampDown();
        
    } else {
        
        sid.rampUp();
        sid.alignWritePtr();
    }
}

SIDInfo
SIDBridge::getInfo()
{
    SIDInfo info;
    
    switch (config.engine) {
            
        case ENGINE_FASTSID: info = fastsid.getInfo(); break;
        case ENGINE_RESID:   info = resid.getInfo(); break;
    }
    
    info.potX = mouse.readPotX();
    info.potY = mouse.readPotY();
    
    return info;
}

VoiceInfo
SIDBridge::getVoiceInfo(unsigned voice)
{
    VoiceInfo info;
    
    switch (config.engine) {
            
        case ENGINE_FASTSID: info = fastsid.getVoiceInfo(voice); break;
        case ENGINE_RESID:   info = resid.getVoiceInfo(voice); break;
    }
    
    return info;
}

u8 
SIDBridge::peek(u16 addr)
{
    assert(addr <= 0x1F);
    
    // Get SID up to date
    executeUntil(cpu.cycle);
    
    if (addr == 0x19) {
        return mouse.readPotX();
    }
    if (addr == 0x1A) {
        return mouse.readPotY();
    }
    
    switch (config.engine) {
            
        case ENGINE_FASTSID: return fastsid.peek(addr);
        case ENGINE_RESID:   return resid.peek(addr);
            
        default:
            assert(false);
            return 0;
    }
}

u8
SIDBridge::spypeek(u16 addr)
{
    assert(addr <= 0x1F);
    return peek(addr);
}

void 
SIDBridge::poke(u16 addr, u8 value)
{
    // Get SID up to date
    executeUntil(cpu.cycle);

    // Keep both SID implementations up to date
    resid.poke(addr, value);
    fastsid.poke(addr, value);
    
    // Run ReSID for at least one cycle to make pipelined writes work
    if (config.engine != ENGINE_RESID) resid.clock();
}

void
SIDBridge::executeUntil(u64 targetCycle)
{
    u64 missingCycles = targetCycle - cycles;
    
    if (missingCycles > PAL_CYCLES_PER_SECOND) {
        debug(SID_DEBUG, "Far too many SID cycles are missing.\n");
        missingCycles = PAL_CYCLES_PER_SECOND;
    }
    
    execute(missingCycles);
    cycles = targetCycle;
}

void
SIDBridge::execute(u64 numCycles)
{
    if (numCycles == 0)
        return;
    
    switch (config.engine) {
            
        case ENGINE_FASTSID: fastsid.execute(numCycles); break;
        case ENGINE_RESID:   resid.execute(numCycles); break;
            
        default:
            assert(false);
    }
}

void
SIDBridge::clearRingbuffer()
{
    // Reset ringbuffer contents
    for (unsigned i = 0; i < bufferSize; i++) {
        ringBuffer[i] = 0.0f;
    }
    
    // Put the write pointer ahead of the read pointer
    alignWritePtr();
}

float
SIDBridge::readData()
{
    // Read sound sample
    float value = ringBuffer[readPtr];
    
    // Adjust volume
    if (volume != targetVolume) {
        if (volume < targetVolume) {
            volume += MIN(volumeDelta, targetVolume - volume);
        } else {
            volume -= MIN(volumeDelta, volume - targetVolume);
        }
    }
    // float divider = 75000.0f; // useReSID ? 100000.0f : 150000.0f;
    float divider = 40000.0f;
    value = (volume <= 0) ? 0.0f : value * (float)volume / divider;
    
    // Advance read pointer
    advanceReadPtr();
    
    return value;
}

float
SIDBridge::ringbufferData(size_t offset)
{
    return ringBuffer[(readPtr + offset) % bufferSize];
}

void
SIDBridge::readMonoSamples(float *target, size_t n)
{
    // Check for buffer underflow
    if (samplesInBuffer() < n) {
        handleBufferUnderflow();
    }
    
    // Read samples
    for (size_t i = 0; i < n; i++) {
        float value = readData();
        target[i] = value;
    }
}

void
SIDBridge::readStereoSamples(float *target1, float *target2, size_t n)
{
    // Check for buffer underflow
    if (samplesInBuffer() < n) {
        handleBufferUnderflow();
    }
    
    // Read samples
    for (unsigned i = 0; i < n; i++) {
        float value = readData();
        target1[i] = target2[i] = value;
    }
}

void
SIDBridge::readStereoSamplesInterleaved(float *target, size_t n)
{
    // Check for buffer underflow
    if (samplesInBuffer() < n) {
        handleBufferUnderflow();
    }
    
    // Read samples
    for (unsigned i = 0; i < n; i++) {
        float value = readData();
        target[i*2] = value;
        target[i*2+1] = value;
    }
}

void
SIDBridge::writeData(short *data, size_t count)
{
    // Check for buffer overflow
    if (bufferCapacity() < count) {
        handleBufferOverflow();
    }
    
    // Convert sound samples to floating point values and write into ringbuffer
    for (unsigned i = 0; i < count; i++) {
        ringBuffer[writePtr] = float(data[i]) * scale;
        advanceWritePtr();
    }
}

void
SIDBridge::handleBufferUnderflow()
{
    // There are two common scenarios in which buffer underflows occur:
    //
    // (1) The consumer runs slightly faster than the producer.
    // (2) The producer is halted or not startet yet.
    
    debug(SID_DEBUG, "RINGBUFFER UNDERFLOW (r: %ld w: %ld)\n", readPtr, writePtr);

    // Determine the elapsed seconds since the last pointer adjustment.
    u64 now = mach_absolute_time();
    double elapsedTime = (double)(now - lastAlignment) / 1000000000.0;
    lastAlignment = now;

    // Adjust the sample rate, if condition (1) holds.
    if (elapsedTime > 10.0) {
        
        bufferUnderflows++;
        
        // Increase the sample rate based on what we've measured.
        int offPerSecond = (int)(samplesAhead / elapsedTime);
        setSampleRate(getSampleRate() + offPerSecond);
    }

    // Reset the write pointer
    alignWritePtr();
}

void
SIDBridge::handleBufferOverflow()
{
    // There are two common scenarios in which buffer overflows occur:
    //
    // (1) The consumer runs slightly slower than the producer.
    // (2) The consumer is halted or not startet yet.
    
    debug(SID_DEBUG, "RINGBUFFER OVERFLOW (r: %ld w: %ld)\n", readPtr, writePtr);
    
    // Determine the elapsed seconds since the last pointer adjustment.
    u64 now = mach_absolute_time();
    double elapsedTime = (double)(now - lastAlignment) / 1000000000.0;
    lastAlignment = now;
    
    // Adjust the sample rate, if condition (1) holds.
    if (elapsedTime > 10.0) {
        
        bufferOverflows++;
        
        // Decrease the sample rate based on what we've measured.
        int offPerSecond = (int)(samplesAhead / elapsedTime);
        setSampleRate(getSampleRate() - offPerSecond);
    }
    
    // Reset the write pointer
    alignWritePtr();
}
