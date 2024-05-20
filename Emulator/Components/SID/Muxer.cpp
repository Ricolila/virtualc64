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

AudioPort Muxer::stream;

Muxer::Muxer(C64 &ref) : SubComponent(ref)
{
    subComponents = std::vector<CoreComponent *> {

        &sid[0],
        &sid[1],
        &sid[2],
        &sid[3]
    };
}

void
Muxer::_reset(bool hard)
{
    if (hard) clearStats();
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

i64
Muxer::getOption(Option option) const
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
            
        case OPT_AUD_VOL_L:
            return config.volL;

        case OPT_AUD_VOL_R:
            return config.volR;
            
        default:
            fatalError;
    }
}

void
Muxer::setOption(Option option, i64 value)
{
    bool wasMuted = isMuted();

    switch (option) {

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
                sid[0].setRevision(SIDRevision(value));
                sid[1].setRevision(SIDRevision(value));
                sid[2].setRevision(SIDRevision(value));
                sid[3].setRevision(SIDRevision(value));
            }
            return;
        }
        case OPT_SID_FILTER:
        {
            {   SUSPENDED
                
                config.filter = bool(value);
                sid[0].setAudioFilter(bool(value));
                sid[1].setAudioFilter(bool(value));
                sid[2].setAudioFilter(bool(value));
                sid[3].setAudioFilter(bool(value));
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
                sid[0].setSamplingMethod(SamplingMethod(value));
                sid[1].setSamplingMethod(SamplingMethod(value));
                sid[2].setSamplingMethod(SamplingMethod(value));
                sid[3].setSamplingMethod(SamplingMethod(value));
            }
            return;
        }
        case OPT_AUD_VOL_L:
            
            config.volL = std::clamp(value, 0LL, 100LL);
            volL.set(powf((float)config.volL / 50, 1.4f));
            
            if (wasMuted != isMuted()) {
                msgQueue.put(MSG_MUTE, isMuted());
            }
            return;
            
        case OPT_AUD_VOL_R:

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

bool
Muxer::isMuted() const
{
    if (config.volL == 0 && config.volR == 0) return true;
    
    return
    sid[0].config.vol == 0 &&
    sid[1].config.vol == 0 &&
    sid[2].config.vol == 0 &&
    sid[3].config.vol == 0;
}

u32
Muxer::getClockFrequency()
{
    assert(sid[0].getClockFrequency() == sid[1].getClockFrequency());
    assert(sid[0].getClockFrequency() == sid[2].getClockFrequency());
    assert(sid[0].getClockFrequency() == sid[3].getClockFrequency());

    return sid[0].getClockFrequency();
}

void
Muxer::setClockFrequency(u32 frequency)
{
    trace(SID_DEBUG, "Setting clock frequency to %d\n", frequency);

    cpuFrequency = frequency;

    sid[0].setClockFrequency(frequency);
    sid[1].setClockFrequency(frequency);
    sid[2].setClockFrequency(frequency);
    sid[3].setClockFrequency(frequency);
}

double
Muxer::getSampleRate() const
{
    assert(sid[0].getSampleRate() == sid[1].getSampleRate());
    assert(sid[0].getSampleRate() == sid[2].getSampleRate());
    assert(sid[0].getSampleRate() == sid[3].getSampleRate());

    return sid[0].getSampleRate();
}

void
Muxer::setSampleRate(double rate)
{
    if (sampleRate != rate) {

        trace(SID_DEBUG, "Setting sample rate to %f\n", rate);

        sampleRate = rate;
        sid[0].setSampleRate(rate);
        sid[1].setSampleRate(rate);
        sid[2].setSampleRate(rate);
        sid[3].setSampleRate(rate);
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
    // Wipe out existing audio samples
    if (volL.current == 0.0) clear();

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
        
        dumpConfig(os);
    }
}

MuxerStats
Muxer::getStats()
{
    auto streamStats = stream.getStats();

    stats.fillLevel = streamStats.fillLevel;
    stats.bufferUnderflows = streamStats.bufferUnderflows;
    stats.bufferOverflows = streamStats.bufferOverflows;

    return stats;
}

SIDInfo
Muxer::getInfo(isize nr)
{
    assert(nr < 4);
    
    SIDInfo info = { };
    
    switch (config.engine) {
            
        case SIDENGINE_RESID:   info = sid[nr].resid.getInfo(); break;

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
            
        case SIDENGINE_RESID:   return sid[nr].resid.getVoiceInfo(voice);

        default:
            fatalError;
    }
}

CoreComponent &
Muxer::getSID(isize nr)
{
    assert(nr >= 0 && nr <= 3);
    
    switch (config.engine) {
            
        case SIDENGINE_RESID:   return sid[nr].resid;

        default:
            fatalError;
    }
}

void
Muxer::rampUp()
{
    trace(AUDVOL_DEBUG, "rampUp()\n");

    if (emulator.isPaused()) {

        trace(AUDVOL_DEBUG, "rampUp canceled (emulator pauses)\n");
        return;
    }
    if (emulator.isWarping()) {

        trace(AUDVOL_DEBUG, "rampUp canceled (emulator warps)\n");
        return;
    }

    const isize steps = 20000;
    volL.fadeIn(steps);
    volR.fadeIn(steps);
    
    ignoreNextUnderOrOverflow();
}

void
Muxer::rampDown()
{
    trace(AUDVOL_DEBUG, "rampDown()\n");

    const isize steps = 2000;
    volL.fadeOut(steps);
    volR.fadeOut(steps);
    
    ignoreNextUnderOrOverflow();
}

isize
Muxer::mappedSID(u16 addr) const
{
    addr &= 0xFFE0;
    
    if (isEnabled(1) && addr == sid[1].config.address) return 1;
    if (isEnabled(2) && addr == sid[2].config.address) return 2;
    if (isEnabled(3) && addr == sid[3].config.address) return 3;

    return 0;
}

u8 
Muxer::peek(u16 addr)
{
    // Select the target SID
    isize sidNr = mappedSID(addr);

    // Get the target SID up to date
    sid[sidNr].executeUntil(cpu.clock, sidStream[sidNr]);

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
            
        case SIDENGINE_RESID:   return sid[sidNr].resid.peek(addr);

        default:
            fatalError;
    }
}

u8
Muxer::spypeek(u16 addr) const
{
    // Select the target SID
    isize sidNr = mappedSID(addr);

    addr &= 0x1F;

    if (sidNr == 0) {

        if (addr == 0x19) { return port1.readPotX() & port2.readPotX(); }
        if (addr == 0x1A) { return port1.readPotY() & port2.readPotY(); }
    }

    return sid[sidNr].spypeek(addr);
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

    // Select the target SID
    isize sidNr = mappedSID(addr);

    // Get the target SID up to date
    sid[sidNr].executeUntil(cpu.clock, sidStream[sidNr]);

    // Write the register
    sid[sidNr].poke(addr, value);
}

void 
Muxer::beginFrame()
{
    setSampleRate(host.getOption(OPT_HOST_SAMPLE_RATE)); 
}

void 
Muxer::endFrame()
{
    // Execute all remaining SID cycles
    muxer.executeUntil(cpu.clock);

    if (stream.isActive(this)) {

        auto s0 = sidStream[0].count();
        auto s1 = sidStream[1].count();
        auto s2 = sidStream[2].count();
        auto s3 = sidStream[3].count();

        auto numSamples = s0;
        if (s1) numSamples = std::min(numSamples, s1);
        if (s2) numSamples = std::min(numSamples, s2);
        if (s3) numSamples = std::min(numSamples, s3);

        if (isEnabled(1) || isEnabled(2) || isEnabled(3)) {
            stream.mixMultiSID(numSamples);
        } else {
            stream.mixSingleSID(numSamples);
        }

    } else {

        // Trash all generated samples
        sidStream[0].clear();
        sidStream[1].clear();
        sidStream[2].clear();
        sidStream[3].clear();
    }
}

void
Muxer::executeUntil(Cycle targetCycle)
{
    sid[0].executeUntil(targetCycle, sidStream[0]);
    if (isEnabled(1)) sid[1].executeUntil(targetCycle, sidStream[1]);
    if (isEnabled(2)) sid[2].executeUntil(targetCycle, sidStream[2]);
    if (isEnabled(3)) sid[3].executeUntil(targetCycle, sidStream[3]);
}

bool
Muxer::powerSave() const
{
    if (volL.current == 0 && volR.current == 0 && config.powerSave) {

        /* https://sourceforge.net/p/vice-emu/bugs/1374/
         *
         * Due to a bug in reSID, pending register writes are dropped if we
         * skip sample synthesis if SAMPLE_FAST and MOS8580 are selected both.
         * As a workaround, we ignore the power-saving setting in this case.
         */
        return config.revision != MOS_8580 || config.sampling != SAMPLING_FAST;
    }

    return false;
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
Muxer::ignoreNextUnderOrOverflow()
{
    lastAlignment = util::Time::now();
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
