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
#include "SID.h"
#include "Emulator.h"

namespace vc64 {

void
SID::cacheInfo(SIDInfo &info) const
{
    {   SYNCHRONIZED

        reSID::SID::State state = resid.sid->read_state();
        u8 *reg = (u8 *)state.sid_register;

        info.volume = reg[0x18] & 0xF;
        info.filterModeBits = reg[0x18] & 0xF0;
        info.filterType = reg[0x18] & 0x70;
        info.filterCutoff = u16(reg[0x16] << 3 | (reg[0x15] & 0x07));
        info.filterResonance = reg[0x17] >> 4;
        info.filterEnableBits = reg[0x17] & 0x0F;

        info.potX = port1.mouse.readPotX() & port2.mouse.readPotX();
        info.potY = port1.mouse.readPotY() & port2.mouse.readPotY();
        
        for (isize i = 0; i < 3; i++, reg += 7) {

            for (isize j = 0; j < 7; j++) info.voice[i].reg[j] = reg[j];
            info.voice[i].frequency = HI_LO(reg[0x1], reg[0x0]);
            info.voice[i].pulseWidth = u16((reg[0x3] & 0xF) << 8 | reg[0x02]);
            info.voice[i].waveform = reg[0x4] & 0xF0;
            info.voice[i].ringMod = (reg[0x4] & 0x4) != 0;
            info.voice[i].hardSync = (reg[0x4] & 0x2) != 0;
            info.voice[i].gateBit = (reg[0x4] & 0x1) != 0;
            info.voice[i].testBit = (reg[0x4] & 0x8) != 0;
            info.voice[i].attackRate = reg[0x5] >> 4;
            info.voice[i].decayRate = reg[0x5] & 0xF;
            info.voice[i].sustainRate = reg[0x6] >> 4;
            info.voice[i].releaseRate = reg[0x6] & 0xF;
        }
    }
}

void
SID::resetConfig()
{
    Configurable::resetConfig(emulator.defaults, objid);
}

i64
SID::getOption(Option option) const
{

    switch (option) {

        case OPT_SID_ENABLE:    return config.enabled;
        case OPT_SID_ADDRESS:   return config.address;
        case OPT_SID_REVISION:  return config.revision;
        case OPT_SID_FILTER:    return config.filter;
        case OPT_SID_ENGINE:    return config.engine;
        case OPT_SID_SAMPLING:  return config.sampling;
        case OPT_AUD_VOL:       return config.vol;
        case OPT_AUD_PAN:       return config.pan;

        default:
            fatalError;
    }
}

i64
SID::getFallback(Option opt) const
{
    return emulator.defaults.getFallback(opt, objid);
}

void
SID::setOption(Option option, i64 value)
{
    bool wasMuted = c64.sidBridge.isMuted();

    switch (option) {

        case OPT_SID_ENABLE:
        {
            if (objid == 0 && value == false) {
                warn("SID 0 can't be disabled\n");
                return;
            }

            if (config.enabled == value) {
                return;
            }

            {   SUSPENDED

                config.enabled = value;
                c64.sidBridge.clearSampleBuffer(objid);
                c64.sidBridge.hardReset();
            }
            return;
        }

        case OPT_SID_ADDRESS:
        {
            if (objid == 0 && value != 0xD400) {
                warn("SID 0 can't be remapped\n");
                return;
            }

            if (value < 0xD400 || value > 0xD7E0 || (value & 0x1F)) {
                throw VC64Error(ERROR_OPT_INVARG, "D400, D420 ... D7E0");
            }

            if (config.address == value) {
                return;
            }

            {   SUSPENDED

                config.address = (u16)value;
                sidBridge.clearSampleBuffer(objid);
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
                setRevision(SIDRevision(value));
            }
            return;
        }

        case OPT_SID_FILTER:
        {
            {   SUSPENDED

                config.filter = bool(value);
                setAudioFilter(bool(value));
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
                setSamplingMethod(SamplingMethod(value));
            }
            return;
        }

        case OPT_AUD_VOL:
        {
            config.vol = std::clamp(value, 0LL, 100LL);
            vol = powf((float)config.vol / 100, 1.4f) * 0.000025f;
            if (emscripten) vol *= 0.15f;

            if (wasMuted != sidBridge.isMuted()) {
                msgQueue.put(MSG_MUTE, sidBridge.isMuted());
            }
            return;
        }

        case OPT_AUD_PAN:
        {
            config.pan = value;
            pan = float(0.5 * (sin(config.pan * M_PI / 200.0) + 1));
            return;
        }

        default:
            fatalError;
    }
}

}
