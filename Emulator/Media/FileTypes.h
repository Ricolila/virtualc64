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
/// @file

#pragma once

#include "Aliases.h"
#include "Reflection.h"

//
// Enumerations
//

enum_long(FILETYPE)
{
    FILETYPE_UNKNOWN = 0,
    FILETYPE_SNAPSHOT,
    FILETYPE_SCRIPT,
    FILETYPE_CRT,
    FILETYPE_T64,
    FILETYPE_P00,
    FILETYPE_PRG,
    FILETYPE_FOLDER,
    FILETYPE_D64,
    FILETYPE_G64,
    FILETYPE_TAP,
    FILETYPE_BASIC_ROM,
    FILETYPE_CHAR_ROM,
    FILETYPE_KERNAL_ROM,
    FILETYPE_VC1541_ROM
};
typedef FILETYPE FileType;

#ifdef __cplusplus
struct FileTypeEnum : util::Reflection<FileTypeEnum, FileType> {
    
	static constexpr long minVal = 0;
    static constexpr long maxVal = FILETYPE_VC1541_ROM;
    static bool isValid(auto value) { return value >= minVal && value <= maxVal; }
    
    static const char *prefix() { return "FILETYPE"; }
    static const char *key(FileType value)
    {
        switch (value) {
                
            case FILETYPE_UNKNOWN:    return "UNKNOWN";
            case FILETYPE_SNAPSHOT:   return "SNAPSHOT";
            case FILETYPE_SCRIPT:     return "SCRIPT";
            case FILETYPE_CRT:        return "CRT";
            case FILETYPE_T64:        return "T64";
            case FILETYPE_PRG:        return "PRG";
            case FILETYPE_FOLDER:     return "FOLDER";
            case FILETYPE_P00:        return "P00";
            case FILETYPE_D64:        return "D64";
            case FILETYPE_G64:        return "G64";
            case FILETYPE_TAP:        return "TAP";
            case FILETYPE_BASIC_ROM:  return "BASIC_ROM";
            case FILETYPE_CHAR_ROM:   return "CHAR_ROM";
            case FILETYPE_KERNAL_ROM: return "KERNAL_ROM";
            case FILETYPE_VC1541_ROM: return "VC1541_ROM";
        }
        return "???";
    }
};
#endif

enum_long(TAP_VERSION)
{
    TAP_VERSION_ORIGINAL,
    TAP_VERSION_ADVANCED
};
typedef TAP_VERSION TAPVersion;

#ifdef __cplusplus
struct TAPVersionEnum : util::Reflection<TAPVersionEnum, TAPVersion> {
    
	static constexpr long minVal = 0;
    static constexpr long maxVal = TAP_VERSION_ADVANCED;
    static bool isValid(auto value) { return value >= minVal && value <= maxVal; }
    
    static const char *prefix() { return "TAP_VERSION"; }
    static const char *key(TAPVersion value)
    {
        switch (value) {
                
            case TAP_VERSION_ORIGINAL:  return "ORIGINAL";
            case TAP_VERSION_ADVANCED:  return "ADVANCED";
            default:                    return "???";
        }
    }
};
#endif

enum_long(FILE_ERROR)
{
    ERR_FILE_OK,
    
    // General
    ERR_FILE_NOT_FOUND,
    ERR_INVALID_TYPE,
    ERR_CANT_READ,
    ERR_CANT_WRITE,
    ERR_OUT_OF_MEMORY,
    ERR_UNKNOWN,
    
    // Snapshots
    ERR_UNSUPPORTED_SNAPSHOT
};
typedef FILE_ERROR FileError;

enum_long(ROM_IDENTIFIER)
{
    ROM_MISSING,
    ROM_UNKNOWN,
    
    // Basic Roms
    BASIC_COMMODORE,

    // Character Roms
    CHAR_COMMODORE,
    CHAR_SWEDISH_C2D007,
    CHAR_SWEDISH,
    CHAR_SPANISH_C64C,
    CHAR_DANISH,
    CHAR_ATARI800,
    CHAR_MSX,
    CHAR_ZX_SPECTRUM,
    CHAR_AMSTRAD_CPC,
    CHAR_A500_TOPAZ_BROKEN,
    CHAR_A500_TOPAZ_V2,
    CHAR_A1200_TOPAZ_BROKEN,
    CHAR_A1200_TOPAZ_V2,
    CHAR_TELETEXT,
    CHAR_MEGA65,
    CHAR_PXLFONT_V23,

    // Kernal Roms
    KERNAL_1ST,
    KERNAL_2ST,
    KERNAL_3ST,
    KERNAL_DANISH_3RD,
    KERNAL_SX64,
    KERNAL_SX64_SCAND,
    KERNAL_4064,
    KERNAL_64_JIFFY,
    KERNAL_SX64_JIFFY,
    KERNAL_TURBO_ROM,
    KERNAL_DATEL_V32,
    KERNAL_EXOS_V3,
    KERNAL_TURBO_TAPE,
    KERNEL_TURBO_250,
    KERNAL_64ER_V3,
    KERNAL_SPEEDDOS_PLUS,
    KERNAL_SPEEDDOS_27,
    KERNAL_DOLPHIN_10,
    KERNAL_DOLPHIN_20_1,
    KERNAL_DOLPHIN_20_1_MA,
    KERNAL_DOLPHIN_20_2,
    KERNAL_DOLPHIN_20_3,
    KERNAL_DOLPHIN_20_SLVDR,
    KERNAL_DOLPHIN_30,
    KERNAL_DOLPHIN_JIFFY,
    KERNEL_NORLAND_30,
    KERNEL_SD2IEC,
    KERNAL_JAFFY,

    // VC1541 Roms
    VC1541C_01,
    VC1541C_02,
    VC1541_II_1987,
    VC1541_II_NEWTRONIC,
    VC1541_II_RELOC_PATCH,
    VC1541_II_JIFFY,
    VC1541_II_JIFFY_V600,
    VC1541_64ER_V3,
    VC1541_SPEEDDOS_PLUS,
    VC1541_SPEEDDOS_27,
    VC1541_DOLPHIN_20,
    VC1541_DOLPHIN_20_SLVDR,
    VC1541_DOLPHIN_30,
    VC1541_DOLPHIN_30_SLVDR,

    ROM_IDENTIFIER_COUNT
};
typedef ROM_IDENTIFIER RomIdentifier;
