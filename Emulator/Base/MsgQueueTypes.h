// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(MSG_TYPE)
{
    MSG_NONE = 0,
    
    // Message queue
    MSG_REGISTER,
    MSG_UNREGISTER,
    
    // Emulator state
    MSG_CONFIG,
    MSG_POWER_ON,
    MSG_POWER_OFF,
    MSG_RUN,
    MSG_PAUSE,
    MSG_STEP,
    MSG_RESET,
    MSG_HALT,
    MSG_WARP_ON,
    MSG_WARP_OFF,
    MSG_MUTE_ON,
    MSG_MUTE_OFF,
    
    // Scripting
    MSG_SCRIPT_DONE,
    MSG_SCRIPT_PAUSE,
    MSG_SCRIPT_ABORT,
    MSG_SCRIPT_WAKEUP,

    // ROMs
    MSG_BASIC_ROM_LOADED,
    MSG_CHAR_ROM_LOADED,
    MSG_KERNAL_ROM_LOADED,
    MSG_DRIVE_ROM_LOADED,
    MSG_ROM_MISSING,

    // CPU related messages
    MSG_CPU_OK,
    MSG_CPU_JAMMED,
    MSG_BREAKPOINT_REACHED,
    MSG_WATCHPOINT_REACHED,

    // VIC related messages
    MSG_PAL,
    MSG_NTSC,

    // IEC Bus
    MSG_IEC_BUS_BUSY,
    MSG_IEC_BUS_IDLE,
    
    // Floppy drives
    MSG_DRIVE_CONNECT,
    MSG_DRIVE_DISCONNECT,
    MSG_DRIVE_POWER_ON,
    MSG_DRIVE_POWER_OFF,
    MSG_DRIVE_POWER_SAVE_ON,
    MSG_DRIVE_POWER_SAVE_OFF,
    MSG_DRIVE_READ,
    MSG_DRIVE_WRITE,
    MSG_DRIVE_LED_ON,
    MSG_DRIVE_LED_OFF,
    MSG_DRIVE_MOTOR_ON,
    MSG_DRIVE_MOTOR_OFF,
    MSG_DRIVE_STEP,
    MSG_DISK_INSERT,
    MSG_DISK_EJECT,
    MSG_DISK_SAVED,
    MSG_DISK_UNSAVED,
    MSG_DISK_PROTECT,
    MSG_FILE_FLASHED,

    // Peripherals (Datasette)
    MSG_VC1530_CONNECT,
    MSG_VC1530_DISCONNECT,
    MSG_VC1530_TAPE,
    MSG_VC1530_PLAY,
    MSG_VC1530_MOTOR,
    MSG_VC1530_COUNTER,

    // Peripherals (Expansion port)
    MSG_CRT_ATTACHED,
    MSG_CRT_DETACHED,
    MSG_CART_SWITCH,

    // Peripherals (Keyboard)
    MSG_KB_AUTO_RELEASE,
    
    // Peripherals (Mouse)
    MSG_SHAKING,
    
    // Snapshots
    MSG_AUTO_SNAPSHOT_TAKEN,
    MSG_USER_SNAPSHOT_TAKEN,
    MSG_SNAPSHOT_RESTORED,
    
    // Screen recording
    MSG_RECORDING_STARTED,
    MSG_RECORDING_STOPPED,
    MSG_RECORDING_ABORTED,
    
    // Console
    MSG_CLOSE_CONSOLE,
	MSG_UPDATE_CONSOLE,
    
    // Debugging
    MSG_DMA_DEBUG_ON,
    MSG_DMA_DEBUG_OFF,
    
    MSG_COUNT
};
typedef MSG_TYPE MsgType;

#ifdef __cplusplus
struct MsgTypeEnum : util::Reflection<MsgType, MsgType> {
    
    static constexpr long minVal = 0;
    static constexpr long maxVal = MSG_DMA_DEBUG_OFF;
    static bool isValid(auto value) { return value >= minVal && value <= maxVal; }

    static const char *prefix() { return "MSG"; }
    static const char *key(MsgType value)
    {
        switch (value) {
                
            case MSG_NONE:                 return "NONE";
                
            case MSG_REGISTER:             return "MSG_REGISTER";
            case MSG_UNREGISTER:           return "MSG_UNREGISTER";

            case MSG_CONFIG:               return "CONFIG";
            case MSG_POWER_ON:             return "POWER_ON";
            case MSG_POWER_OFF:            return "POWER_OFF";
            case MSG_RUN:                  return "RUN";
            case MSG_PAUSE:                return "PAUSE";
            case MSG_RESET:                return "RESET";
            case MSG_SCRIPT_DONE:          return "SCRIPT_DONE";
            case MSG_SCRIPT_PAUSE:         return "SCRIPT_PAUSE";
            case MSG_SCRIPT_ABORT:         return "SCRIPT_ABORT";
            case MSG_SCRIPT_WAKEUP:        return "MSG_SCRIPT_WAKEUP";
            case MSG_HALT:                 return "HALT";
            case MSG_WARP_ON:              return "WARP_ON";
            case MSG_WARP_OFF:             return "WARP_OFF";
            case MSG_MUTE_ON:              return "MUTE_ON";
            case MSG_MUTE_OFF:             return "MUTE_OFF";
                
            case MSG_BASIC_ROM_LOADED:     return "BASIC_ROM_LOADED";
            case MSG_CHAR_ROM_LOADED:      return "CHAR_ROM_LOADED";
            case MSG_KERNAL_ROM_LOADED:    return "KERNAL_ROM_LOADED";
            case MSG_DRIVE_ROM_LOADED:     return "DRIVE_ROM_LOADED";
            case MSG_ROM_MISSING:          return "ROM_MISSING";
                
            case MSG_CPU_OK:               return "CPU_OK";
            case MSG_CPU_JAMMED:           return "CPU_JAMMED";
            case MSG_BREAKPOINT_REACHED:   return "BREAKPOINT_REACHED";
            case MSG_WATCHPOINT_REACHED:   return "WATCHPOINT_REACHED";
                
            case MSG_PAL:                  return "PAL";
            case MSG_NTSC:                 return "NTSC";
                
            case MSG_IEC_BUS_BUSY:         return "IEC_BUS_BUSY";
            case MSG_IEC_BUS_IDLE:         return "IEC_BUS_IDLE";
                
            case MSG_DRIVE_CONNECT:        return "DRIVE_CONNECT";
            case MSG_DRIVE_DISCONNECT:     return "DRIVE_DISCONNECT";
            case MSG_DRIVE_POWER_ON:       return "DRIVE_POWER_ON";
            case MSG_DRIVE_POWER_OFF:      return "DRIVE_POWER_OFF";
            case MSG_DRIVE_POWER_SAVE_ON:  return "DRIVE_POWER_SAVE_ON";
            case MSG_DRIVE_POWER_SAVE_OFF: return "DRIVE_POWER_SAVE_OFF";
            case MSG_DRIVE_READ:           return "DRIVE_READ";
            case MSG_DRIVE_WRITE:          return "DRIVE_WRITE";
            case MSG_DRIVE_LED_ON:         return "DRIVE_LED_ON";
            case MSG_DRIVE_LED_OFF:        return "DRIVE_LED_OFF";
            case MSG_DRIVE_MOTOR_ON:       return "DRIVE_MOTOR_ON";
            case MSG_DRIVE_MOTOR_OFF:      return "DRIVE_MOTOR_OFF";
            case MSG_DRIVE_STEP:           return "DRIVE_STEP";
            case MSG_DISK_INSERT:          return "DISK_INSERT";
            case MSG_DISK_EJECT:           return "DISK_EJECT";
            case MSG_DISK_SAVED:           return "DISK_SAVED";
            case MSG_DISK_UNSAVED:         return "DISK_UNSAVED";
            case MSG_DISK_PROTECT:         return "DISK_PROTECT";
            case MSG_FILE_FLASHED:         return "FILE_FLASHED";

            case MSG_VC1530_CONNECT:       return "VC1530_CONNECT";
            case MSG_VC1530_DISCONNECT:    return "VC1530_DISCONNECT";
            case MSG_VC1530_TAPE:          return "VC1530_TAPE";
            case MSG_VC1530_PLAY:          return "VC1530_PLAY";
            case MSG_VC1530_MOTOR:         return "VC1530_MOTOR";
            case MSG_VC1530_COUNTER:       return "VC1530_COUNTER";
                
            case MSG_CRT_ATTACHED:         return "CRT_ATTACHED";
            case MSG_CRT_DETACHED:         return "CRT_DETACHED";
            case MSG_CART_SWITCH:          return "CART_SWITCH";
                
            case MSG_KB_AUTO_RELEASE:      return "KB_AUTO_RELEASE";

            case MSG_SHAKING:              return "SHAKING";
                
            case MSG_AUTO_SNAPSHOT_TAKEN:  return "AUTO_SNAPSHOT_TAKEN";
            case MSG_USER_SNAPSHOT_TAKEN:  return "USER_SNAPSHOT_TAKEN";
            case MSG_SNAPSHOT_RESTORED:    return "SNAPSHOT_RESTORED";
                
            case MSG_RECORDING_STARTED:    return "MSG_RECORDING_STARTED";
            case MSG_RECORDING_STOPPED:    return "MSG_RECORDING_STOPPED";
            case MSG_RECORDING_ABORTED:    return "MSG_RECORDING_ABORTED";
                
            case MSG_CLOSE_CONSOLE:        return "CLOSE_CONSOLE";
			case MSG_UPDATE_CONSOLE:        return "UPDATE_CONSOLE";
                
            case MSG_DMA_DEBUG_ON:         return "DMA_DEBUG_ON";
            case MSG_DMA_DEBUG_OFF:        return "DMA_DEBUG_OFF";
                
            case MSG_COUNT:                return "???";
        }
        return "???";
    }
};
#endif


//
// Structures
//

typedef struct
{
    MsgType type;

    /* The payload of a message consists of up to four (signed) 32-bit values.
     * We avoid the usage of 64-bit types inside this structure to make it
     * easily processable by JavaScript (web ports).
     */
    i32 data1;
    i32 data2;
    i32 data3;
    i32 data4;
}
Message;


//
// Signatures
//

typedef void Callback(const void *, long, i32, i32, i32, i32);
