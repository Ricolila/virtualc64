// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// swiftlint:disable colon

import Carbon.HIToolbox

//
// Convenience extensions to UserDefaults
//

extension UserDefaults {
    
    // Registers an item of generic type 'Encodable'
    func register<T: Encodable>(encodableItem item: T, forKey key: String) {
        
        if let data = try? PropertyListEncoder().encode(item) {
            register(defaults: [key: data])
        }
    }

    // Encodes an item of generic type 'Encodable'
    func encode<T: Encodable>(_ item: T, forKey key: String) {
        
        if let encoded = try? PropertyListEncoder().encode(item) {
            set(encoded, forKey: key)
        } else {
            track("Failed to encode \(key)")
        }
    }
    
    // Decodes an item of generic type 'Decodable'
    func decode<T: Decodable>(_ item: inout T, forKey key: String) {
        
        if let data = data(forKey: key) {
            if let decoded = try? PropertyListDecoder().decode(T.self, from: data) {
                item = decoded
            } else {
                track("Failed to decode \(key)")
            }
        }
    }
}

//
// User defaults (all)
//

extension UserDefaults {
    
    static func registerUserDefaults() {
        
        track()
        
        registerGeneralUserDefaults()
        registerControlsUserDefaults()
        registerDevicesUserDefaults()
        registerKeyboardUserDefaults()
        
        registerHardwareUserDefaults()
        registerPeripheralsUserDefaults()
        registerCompatibilityUserDefaults()
        registerAudioUserDefaults()
        registerVideoUserDefaults()
        registerRomUserDefaults()
    }
}

extension MyController {
    
    func loadUserDefaults() {
        
        track()
        
        c64.suspend()
        
        pref.loadGeneralUserDefaults()
        pref.loadControlsUserDefaults()
        pref.loadDevicesUserDefaults()
        pref.loadKeyboardUserDefaults()
        
        config.loadRomUserDefaults()
        config.loadHardwareUserDefaults()
        config.loadPeripheralsUserDefaults()
        config.loadCompatibilityUserDefaults()
        config.loadAudioUserDefaults()
        config.loadVideoUserDefaults()

        c64.resume()
    }
    
    func loadUserDefaults(url: URL, prefixes: [String]) {
        
        if let fileContents = NSDictionary(contentsOf: url) {
            
            if let dict = fileContents as? [String: Any] {
                
                let filteredDict = dict.filter { prefixes.contains(where: $0.0.hasPrefix) }
                
                let defaults = UserDefaults.standard
                defaults.setValuesForKeys(filteredDict)
                
                loadUserDefaults()
            }
        }
    }
    
    func saveUserDefaults(url: URL, prefixes: [String]) {
        
        track()
        
        let dict = UserDefaults.standard.dictionaryRepresentation()
        let filteredDict = dict.filter { prefixes.contains(where: $0.0.hasPrefix) }
        let nsDict = NSDictionary(dictionary: filteredDict)
        nsDict.write(to: url, atomically: true)
    }
}

//
// User defaults (Emulator)
//

struct Keys {
    
    struct Gen {
                
        // Snapshots and screenshots
        static let autoSnapshots          = "VC64_GEN_AutoSnapshots"
        static let autoSnapshotInterval   = "VC64_GEN_ScreenshotInterval"
        static let screenshotSource       = "VC64_GEN_ScreenshotSource"
        static let screenshotTarget       = "VC64_GEN_ScreenshotTarget"
        
        // Screen captures
        static let ffmpegPath             = "VC64_GEN_ffmpegPath"
        static let captureSource          = "VC64_GEN_Source"
        static let bitRate                = "VC64_GEN_BitRate"
        static let aspectX                = "VC64_GEN_AspectX"
        static let aspectY                = "VC64_GEN_AspectY"

        // Fullscreen
        static let keepAspectRatio        = "VC64_GEN_FullscreenKeepAspectRatio"
        static let exitOnEsc              = "VC64_GEN_FullscreenExitOnEsc"
                
        // Warp mode
        static let warpMode               = "VC64_GEN_WarpMode"
        
        // Miscellaneous
        static let driveEjectUnasked      = "VC64_GEN_EjectUnasked"
        static let pauseInBackground      = "VC64_GEN_PauseInBackground"
        static let closeWithoutAsking     = "VC64_GEN_CloseWithoutAsking"
    }
}

struct GeneralDefaults {
        
    // Snapshots and Screenshots
    let autoSnapshots: Bool
    let autoSnapshotInterval: Int
    let screenshotSource: Int
    let screenshotTarget: NSBitmapImageRep.FileType
    
    // Captures
    let captureSource: Int
    let bitRate: Int
    let aspectX: Int
    let aspectY: Int
    
    // Fullscreen
    let keepAspectRatio: Bool
    let exitOnEsc: Bool
    
    // Warp mode
    let warpMode: WarpMode
    
    // Miscellaneous
    let ejectWithoutAsking: Bool
    let closeWithoutAsking: Bool
    let pauseInBackground: Bool

    //
    // Schemes
    //
    
    static let std = GeneralDefaults(
                
        autoSnapshots: false,
        autoSnapshotInterval: 20,
        screenshotSource: 0,
        screenshotTarget: .png,
        
        captureSource: 0,
        bitRate: 2048,
        aspectX: 768,
        aspectY: 702,
        
        keepAspectRatio: false,
        exitOnEsc: true,
        
        warpMode: .auto,
        
        ejectWithoutAsking: false,
        closeWithoutAsking: false,
        pauseInBackground: false
    )
}

extension UserDefaults {

    static func registerGeneralUserDefaults() {
        
        let defaults = GeneralDefaults.std
        let dictionary: [String: Any] = [

            Keys.Gen.autoSnapshots: defaults.autoSnapshots,
            Keys.Gen.autoSnapshotInterval: defaults.autoSnapshotInterval,
            Keys.Gen.screenshotSource: defaults.screenshotSource,
            Keys.Gen.screenshotTarget: Int(defaults.screenshotTarget.rawValue),
            
            Keys.Gen.captureSource: defaults.captureSource,
            Keys.Gen.bitRate: defaults.bitRate,
            Keys.Gen.aspectX: defaults.aspectX,
            Keys.Gen.aspectY: defaults.aspectY,

            Keys.Gen.keepAspectRatio: defaults.keepAspectRatio,
            Keys.Gen.exitOnEsc: defaults.exitOnEsc,
            
            Keys.Gen.warpMode: Int(defaults.warpMode.rawValue),

            Keys.Gen.driveEjectUnasked: defaults.ejectWithoutAsking,
            Keys.Gen.pauseInBackground: defaults.pauseInBackground,
            Keys.Gen.closeWithoutAsking: defaults.closeWithoutAsking
        ]
        
        let userDefaults = UserDefaults.standard
        
        userDefaults.register(defaults: dictionary)
    }
    
    static func resetGeneralUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        let keys = [ Keys.Gen.autoSnapshots,
                     Keys.Gen.autoSnapshotInterval,
                     Keys.Gen.screenshotSource,
                     Keys.Gen.screenshotTarget,
                     
                     Keys.Gen.captureSource,
                     Keys.Gen.bitRate,
                     Keys.Gen.aspectX,
                     Keys.Gen.aspectY,

                     Keys.Gen.keepAspectRatio,
                     Keys.Gen.exitOnEsc,
                     
                     Keys.Gen.warpMode,
                     
                     Keys.Gen.driveEjectUnasked,
                     Keys.Gen.pauseInBackground,
                     Keys.Gen.closeWithoutAsking
        ]
        
        for key in keys { defaults.removeObject(forKey: key) }
    }
}
    
//
// User defaults (Controls)
//

extension Keys {
    
    struct Con {
        
        // Emulation keys
        static let mouseKeyMap           = "VC64_CON_MouseKeyMap"
        static let joyKeyMap1            = "VC64_CON_JoyKeyMap1"
        static let joyKeyMap2            = "VC64_CON_JoyKeyMap2"
        static let disconnectJoyKeys     = "VC64_CON_DisconnectKeys"
        
        // Joysticks
        static let autofire              = "VC64_CON_Autofire"
        static let autofireBullets       = "VC64_CON_AutofireBullets"
        static let autofireFrequency     = "VC64_CON_AutofireFrequency"
        
        // Mouse
        static let retainMouseKeyComb    = "VC64_CON_RetainMouseKeyComb"
        static let retainMouseWithKeys   = "VC64_CON_RetainMouseWithKeys"
        static let retainMouseByClick    = "VC64_CON_RetainMouseByClick"
        static let retainMouseByEntering = "VC64_CON_RetainMouseByEntering"
        static let releaseMouseKeyComb   = "VC64_CON_ReleaseMouseKeyComb"
        static let releaseMouseWithKeys  = "VC64_CON_ReleaseMouseWithKeys"
        static let releaseMouseByShaking = "VC64_CON_ReleaseMouseByShaking"
    }
}

struct ControlsDefaults {
    
    // Emulation keys
    let mouseKeyMap: [MacKey: Int]
    let joyKeyMap1: [MacKey: Int]
    let joyKeyMap2: [MacKey: Int]
    let disconnectJoyKeys: Bool
    
    // Joystick
    let autofire: Bool
    let autofireBullets: Int
    let autofireFrequency: Float
    
    // Mouse
    let retainMouseKeyComb: Int
    let retainMouseWithKeys: Bool
    let retainMouseByClick: Bool
    let retainMouseByEntering: Bool
    let releaseMouseKeyComb: Int
    let releaseMouseWithKeys: Bool
    let releaseMouseByShaking: Bool
    
    //
    // Schemes
    //
    
    static let stdKeyMap1 = [
        
        MacKey(keyCode: kVK_LeftArrow): GamePadAction.PULL_LEFT.rawValue,
        MacKey(keyCode: kVK_RightArrow): GamePadAction.PULL_RIGHT.rawValue,
        MacKey(keyCode: kVK_UpArrow): GamePadAction.PULL_UP.rawValue,
        MacKey(keyCode: kVK_DownArrow): GamePadAction.PULL_DOWN.rawValue,
        MacKey(keyCode: kVK_Space): GamePadAction.PRESS_FIRE.rawValue
    ]
    static let stdKeyMap2 = [
        
        MacKey(keyCode: kVK_ANSI_S): GamePadAction.PULL_LEFT.rawValue,
        MacKey(keyCode: kVK_ANSI_D): GamePadAction.PULL_RIGHT.rawValue,
        MacKey(keyCode: kVK_ANSI_E): GamePadAction.PULL_UP.rawValue,
        MacKey(keyCode: kVK_ANSI_X): GamePadAction.PULL_DOWN.rawValue,
        MacKey(keyCode: kVK_ANSI_C): GamePadAction.PRESS_FIRE.rawValue
    ]
    
    static let std = ControlsDefaults(
        
        mouseKeyMap: [:],
        joyKeyMap1: stdKeyMap1,
        joyKeyMap2: stdKeyMap2,
        disconnectJoyKeys: true,
        
        autofire: false,
        autofireBullets: -3,
        autofireFrequency: 2.5,
        
        retainMouseKeyComb: 0,
        retainMouseWithKeys: true,
        retainMouseByClick: true,
        retainMouseByEntering: false,
        releaseMouseKeyComb: 0,
        releaseMouseWithKeys: true,
        releaseMouseByShaking: true
    )
}

extension UserDefaults {
    
    static func registerControlsUserDefaults() {
        
        let defaults = ControlsDefaults.std
        let dictionary: [String: Any] = [

            // Joysticks
            Keys.Con.disconnectJoyKeys: defaults.disconnectJoyKeys,
            Keys.Con.autofire: defaults.autofire,
            Keys.Con.autofireBullets: defaults.autofireBullets,
            Keys.Con.autofireFrequency: defaults.autofireFrequency,
            
            // Mouse
            Keys.Con.retainMouseKeyComb: defaults.retainMouseKeyComb,
            Keys.Con.retainMouseWithKeys: defaults.retainMouseWithKeys,
            Keys.Con.retainMouseByClick: defaults.retainMouseByClick,
            Keys.Con.retainMouseByEntering: defaults.retainMouseByEntering,
            Keys.Con.releaseMouseKeyComb: defaults.releaseMouseKeyComb,
            Keys.Con.releaseMouseWithKeys: defaults.releaseMouseWithKeys,
            Keys.Con.releaseMouseByShaking: defaults.releaseMouseByShaking
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
        userDefaults.register(encodableItem: defaults.mouseKeyMap, forKey: Keys.Con.mouseKeyMap)
        userDefaults.register(encodableItem: defaults.joyKeyMap1, forKey: Keys.Con.joyKeyMap1)
        userDefaults.register(encodableItem: defaults.joyKeyMap2, forKey: Keys.Con.joyKeyMap2)
    }
    
    static func resetControlsUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        let keys = [ Keys.Con.mouseKeyMap,
                     Keys.Con.joyKeyMap1,
                     Keys.Con.joyKeyMap2,
                     Keys.Con.disconnectJoyKeys,
                     
                     Keys.Con.autofire,
                     Keys.Con.autofireBullets,
                     Keys.Con.autofireFrequency,
                     
                     Keys.Con.retainMouseKeyComb,
                     Keys.Con.retainMouseWithKeys,
                     Keys.Con.retainMouseByClick,
                     Keys.Con.retainMouseByEntering,
                     Keys.Con.releaseMouseKeyComb,
                     Keys.Con.releaseMouseWithKeys,
                     Keys.Con.releaseMouseByShaking ]
        
        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Devices)
//

extension Keys {
    
    struct Dev {

        static let schemes            = "VC64_DEV_Schemes"
    }
}

extension UserDefaults {
    
    static func registerDevicesUserDefaults() {

    }

}

//
// User defaults (Keyboard)
//

extension Keys {
    
    struct Kbd {
        
        // Key map
        static let keyMap            = "VC64KeyMap"
        static let mapKeysByPosition = "VC64MapKeysByPosition"
    }
}

struct KeyboardDefaults {
    
    // Key map
    let keyMap: [MacKey: C64Key]
    let mapKeysByPosition: Bool
    
    //
    // Schemes
    //
    
    static let symbolicMapping = KeyboardDefaults(
        
        keyMap: KeyboardController.standardKeyMap,
        mapKeysByPosition: false
    )

    static let positionalMapping = KeyboardDefaults(
        
        keyMap: KeyboardController.standardKeyMap,
        mapKeysByPosition: true
    )
}

extension UserDefaults {
    
    static func registerKeyboardUserDefaults() {
                
        let defaults = KeyboardDefaults.symbolicMapping
        let dictionary: [String: Any] = [
            
            Keys.Kbd.mapKeysByPosition: defaults.mapKeysByPosition
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
        userDefaults.register(encodableItem: defaults.keyMap, forKey: Keys.Kbd.keyMap)
    }
    
    static func resetKeyMapUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        let keys = [ Keys.Kbd.mapKeysByPosition,
                     Keys.Kbd.keyMap
        ]
        
        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Roms)
//

extension UserDefaults {
    
    static func romUrl(name: String) -> URL? {
        
        let folder = try? URL.appSupportFolder("Roms")
        return folder?.appendingPathComponent(name)
    }
    
    static var basicRomUrl:  URL? { return romUrl(name: "basic.bin") }
    static var charRomUrl:   URL? { return romUrl(name: "char.bin") }
    static var kernalRomUrl: URL? { return romUrl(name: "kernal.bin") }
    static var vc1541RomUrl: URL? { return romUrl(name: "vc1541.bin") }
    
    static func registerRomUserDefaults() {
        
    }

    static func resetRomUserDefaults() {
        
        // Delete previously saved Rom files
        let fm = FileManager.default
        
        if let url = basicRomUrl {
            track("Deleting Basic Rom")
            try? fm.removeItem(at: url)
        }
        if let url = charRomUrl {
            track("Deleting Character Rom")
            try? fm.removeItem(at: url)
        }
        if let url = kernalRomUrl {
            track("Deleting Kernal Rom")
            try? fm.removeItem(at: url)
        }
        if let url = vc1541RomUrl {
            track("Deleting Drive Rom")
            try? fm.removeItem(at: url)
        }
    }
}

//
// User defaults (Hardware)
//

extension Keys {
    
    struct Hwd {
        
        // VICII
        static let vicRevision    = "VC64_HW_VicRev"
        static let vicSpeed       = "VC64_HW_VicSpeed"
        static let vicGrayDotBug  = "VC64_HW_VicGrayDotBug"
        
        // CIAs
        static let ciaRevision    = "VC64_HW_CiaRev"
        static let ciaTimerBBug   = "VC64_HW_CiaTimerBBug"
        
        // SID
        static let sidRevision    = "VC64_HW_SidRev"
        static let sidFilter      = "VC64_HW_SidFilter"
        static let sidEnable1     = "VC64_HW_SidEnable1"
        static let sidEnable2     = "VC64_HW_SidEnable2"
        static let sidEnable3     = "VC64_HW_SidEnable3"
        static let sidAddress1    = "VC64_HW_SidAddress1"
        static let sidAddress2    = "VC64_HW_SidAddress2"
        static let sidAddress3    = "VC64_HW_SidAddress3"
        
        // Logic board and power supply
        static let glueLogic      = "VC64_HW_GlueLogic"
        static let powerGrid      = "VC64_HW_PowerGrid"
        
        // RAM
        static let ramPattern     = "VC64_HW_RamPattern"
    }
}

struct HardwareDefaults {
    
    var vicRevision: VICIIRevision
    var vicSpeed: VICIISpeed
    var vicGrayDotBug: Bool
    
    var ciaRevision: CIARevision
    var ciaTimerBBug: Bool
    
    var sidRevision: SIDRevision
    var sidFilter: Bool
    let sidEnable1: Bool
    let sidEnable2: Bool
    let sidEnable3: Bool
    let sidAddress1: Int
    let sidAddress2: Int
    let sidAddress3: Int
    
    var glueLogic: GlueLogic
    var powerGrid: PowerGrid
    
    var ramPattern: RamPattern
    
    //
    // Schemes
    //
    
    static let C64_PAL = HardwareDefaults(
        
        vicRevision:   .PAL_6569_R3,
        vicSpeed:      .NATIVE,
        vicGrayDotBug: false,
        
        ciaRevision:   .MOS_6526,
        ciaTimerBBug:  true,
        
        sidRevision:   .MOS_6581,
        sidFilter:     true,
        sidEnable1:    false,
        sidEnable2:    false,
        sidEnable3:    false,
        sidAddress1:   0xD420,
        sidAddress2:   0xD440,
        sidAddress3:   0xD460,
        
        glueLogic:     .DISCRETE,
        powerGrid:     .STABLE_50HZ,
        
        ramPattern:    .VICE
    )
    
    static let C64_II_PAL = HardwareDefaults(
        
        vicRevision:   .PAL_8565,
        vicSpeed:      .NATIVE,
        vicGrayDotBug: true,
        
        ciaRevision:   .MOS_8521,
        ciaTimerBBug:  false,
        
        sidRevision:   .MOS_8580,
        sidFilter:     true,
        sidEnable1:    false,
        sidEnable2:    false,
        sidEnable3:    false,
        sidAddress1:   0xD420,
        sidAddress2:   0xD440,
        sidAddress3:   0xD460,
        
        glueLogic:     .IC,
        powerGrid:     .STABLE_50HZ,

        ramPattern:    .VICE
    )
    
    static let C64_OLD_PAL = HardwareDefaults(
        
        vicRevision:   .PAL_6569_R1,
        vicSpeed:      .NATIVE,
        vicGrayDotBug: false,
        
        ciaRevision:   .MOS_6526,
        ciaTimerBBug:  true,
        
        sidRevision:   .MOS_6581,
        sidFilter:     true,
        sidEnable1:    false,
        sidEnable2:    false,
        sidEnable3:    false,
        sidAddress1:   0xD420,
        sidAddress2:   0xD440,
        sidAddress3:   0xD460,
        
        glueLogic:     .DISCRETE,
        powerGrid:     .STABLE_50HZ,

        ramPattern:    .VICE
    )

    static let C64_NTSC = HardwareDefaults(
        
        vicRevision:   .NTSC_6567,
        vicSpeed:      .NATIVE,
        vicGrayDotBug: false,
        
        ciaRevision:   .MOS_6526,
        ciaTimerBBug:  false,
        
        sidRevision:   .MOS_6581,
        sidFilter:     true,
        sidEnable1:    false,
        sidEnable2:    false,
        sidEnable3:    false,
        sidAddress1:   0xD420,
        sidAddress2:   0xD440,
        sidAddress3:   0xD460,
        
        glueLogic:     .DISCRETE,
        powerGrid:     .STABLE_60HZ,

        ramPattern:    .VICE
    )
    
    static let C64_II_NTSC = HardwareDefaults(
        
        vicRevision:   .NTSC_8562,
        vicSpeed:      .NATIVE,
        vicGrayDotBug: true,
        
        ciaRevision:   .MOS_8521,
        ciaTimerBBug:  true,
        
        sidRevision:   .MOS_8580,
        sidFilter:     true,
        sidEnable1:    false,
        sidEnable2:    false,
        sidEnable3:    false,
        sidAddress1:   0xD420,
        sidAddress2:   0xD440,
        sidAddress3:   0xD460,
        
        glueLogic:     .IC,
        powerGrid:     .STABLE_60HZ,

        ramPattern:    .VICE
    )
    
    static let C64_OLD_NTSC = HardwareDefaults(
        
        vicRevision:   .NTSC_6567_R56A,
        vicSpeed:      .NATIVE,
        vicGrayDotBug: false,
        
        ciaRevision:   .MOS_6526,
        ciaTimerBBug:  false,
        
        sidRevision:   .MOS_6581,
        sidFilter:     true,
        sidEnable1:    false,
        sidEnable2:    false,
        sidEnable3:    false,
        sidAddress1:   0xD420,
        sidAddress2:   0xD440,
        sidAddress3:   0xD460,
        
        glueLogic:     .DISCRETE,
        powerGrid:     .STABLE_60HZ,

        ramPattern:    .VICE
    )
}

extension UserDefaults {
    
    static func registerHardwareUserDefaults() {
        
        let defaults = HardwareDefaults.C64_PAL
        let dictionary: [String: Any] = [
            
            Keys.Hwd.vicRevision:   defaults.vicRevision.rawValue,
            Keys.Hwd.vicSpeed:      defaults.vicSpeed.rawValue,
            Keys.Hwd.vicGrayDotBug: defaults.vicGrayDotBug,
            
            Keys.Hwd.ciaRevision:   defaults.ciaRevision.rawValue,
            Keys.Hwd.ciaTimerBBug:  defaults.ciaTimerBBug,
            
            Keys.Hwd.sidRevision:   defaults.sidRevision.rawValue,
            Keys.Hwd.sidEnable1:    defaults.sidEnable1,
            Keys.Hwd.sidEnable2:    defaults.sidEnable2,
            Keys.Hwd.sidEnable3:    defaults.sidEnable3,
            Keys.Hwd.sidAddress1:   defaults.sidAddress1,
            Keys.Hwd.sidAddress2:   defaults.sidAddress2,
            Keys.Hwd.sidAddress3:   defaults.sidAddress3,

            Keys.Hwd.glueLogic:     defaults.glueLogic.rawValue,
            Keys.Hwd.powerGrid:     defaults.powerGrid.rawValue,

            Keys.Hwd.ramPattern:    defaults.ramPattern.rawValue
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }
    
    static func resetHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        let keys = [Keys.Hwd.vicRevision,
                    Keys.Hwd.vicSpeed,
                    Keys.Hwd.vicGrayDotBug,
                    
                    Keys.Hwd.ciaRevision,
                    Keys.Hwd.ciaTimerBBug,
                    
                    Keys.Hwd.sidRevision,
                    Keys.Hwd.sidEnable1,
                    Keys.Hwd.sidEnable2,
                    Keys.Hwd.sidEnable3,
                    Keys.Hwd.sidAddress1,
                    Keys.Hwd.sidAddress2,
                    Keys.Hwd.sidAddress3,
                    
                    Keys.Hwd.glueLogic,
                    Keys.Hwd.powerGrid,
                    
                    Keys.Hwd.ramPattern
        ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Peripherals)
//

extension Keys {
    
    struct Per {
                
        // Drive
        static let drive8Connect    = "VC64_PER_Drive8Connect"
        static let drive8AutoConf   = "VC64_PER_Drive8AutoConf"
        static let drive8Model      = "VC64_PER_Drive8Type"
        static let drive8Ram        = "VC64_PER_Drive8Ram"
        static let drive8ParCable   = "VC64_PER_Drive8ParCable"
        static let drive9Connect    = "VC64_PER_Drive9Connect"
        static let drive9AutoConf   = "VC64_PER_Drive9AutoConf"
        static let drive9Model      = "VC64_PER_Drive9Type"
        static let drive9Ram        = "VC64_PER_Drive9Ram"
        static let drive9ParCable   = "VC64_PER_Drive9ParCable"

        // Power saving
        static let drivePowerSave   = "VC64_PER_DrivePowerSave"
        
        // Disks
        static let blankDiskFormat  = "VC64_PER_BlankDiskFormat"

        // Ports
        static let gameDevice1      = "VC64_PER_ControlPort1"
        static let gameDevice2      = "VC64_PER_ControlPort2"
        
        // Mouse
        static let mouseModel       = "VC64_PER_MouseModel"
    }
}

struct PeripheralsDefaults {
    
    var driveConnect: [Bool]
    var driveAutoConf: [Bool]
    var driveModel: [DriveType]
    var driveRam: [DriveRam]
    var parCable: [ParCableType]
    var driveHibernate: Bool
    
    let blankDiskFormat: DOSType

    var gameDevice1: Int
    var gameDevice2: Int
    
    let mouseModel: MouseModel
    
    //
    // Schemes
    //
    
    static let std = PeripheralsDefaults(
        
        driveConnect:    [true, false],
        driveAutoConf:   [true, true],
        driveModel:      [.VC1541II, .VC1541II],
        driveRam:        [._NONE, ._NONE],
        parCable:        [.NONE, .NONE],
        driveHibernate:  true,
        blankDiskFormat: .CBM,
        gameDevice1:     -1,
        gameDevice2:     -1,
        
        mouseModel:      .C1350
    )
}

extension UserDefaults {
    
    static func registerPeripheralsUserDefaults() {
        
        let defaults = PeripheralsDefaults.std
        let dictionary: [String: Any] = [
                        
            Keys.Per.drive8Connect:   defaults.driveConnect[0],
            Keys.Per.drive8AutoConf:  defaults.driveAutoConf[0],
            Keys.Per.drive8Model:     defaults.driveModel[0].rawValue,
            Keys.Per.drive8Ram:       defaults.driveRam[0].rawValue,
            Keys.Per.drive8ParCable:  defaults.parCable[0].rawValue,
            Keys.Per.drive9Connect:   defaults.driveConnect[1],
            Keys.Per.drive9AutoConf:  defaults.driveAutoConf[1],
            Keys.Per.drive9Model:     defaults.driveModel[1].rawValue,
            Keys.Per.drive9Ram:       defaults.driveRam[1].rawValue,
            Keys.Per.drive9ParCable:  defaults.parCable[1].rawValue,
            Keys.Per.drivePowerSave:  defaults.driveHibernate,
            
            Keys.Per.blankDiskFormat: defaults.blankDiskFormat.rawValue,
            
            Keys.Per.gameDevice1:     defaults.gameDevice1,
            Keys.Per.gameDevice2:     defaults.gameDevice2,
            
            Keys.Per.mouseModel:      defaults.mouseModel.rawValue
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }
    
    static func resetPeripheralsUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        let keys = [Keys.Per.drive8Connect,
                    Keys.Per.drive8AutoConf,
                    Keys.Per.drive8Model,
                    Keys.Per.drive8Ram,
                    Keys.Per.drive8ParCable,
                    Keys.Per.drive9Connect,
                    Keys.Per.drive9AutoConf,
                    Keys.Per.drive9Model,
                    Keys.Per.drive9Ram,
                    Keys.Per.drive9ParCable,
                    Keys.Per.drivePowerSave,
                    
                    Keys.Per.blankDiskFormat,
                    
                    Keys.Per.gameDevice1,
                    Keys.Per.gameDevice2,
                    
                    Keys.Per.mouseModel
        ]

        for key in keys { defaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Compatibility)
//

extension Keys {
    
    struct Com {
        
        // Energy saving
        static let drivePowerSave = "VAMIGA_COM_DrivePowerSave"
        static let viciiPowerSave = "VAMIGA_COM_ViciiPowerSave"
        static let sidPowerSave = "VAMIGA_COM_SidPowerSave"
        
        // Collision checking
        static let ssCollisions = "VAMIGA_COM_SprSprCollisions"
        static let sbCollisions = "VAMIGA_COM_SprBgCollisions"
    }
}

struct CompatibilityDefaults {
    
    let drivePowerSave: Bool
    let viciiPowerSave: Bool
    let sidPowerSave: Bool

    let ssCollisions: Bool
    let sbCollisions: Bool

    //
    // Schemes
    //
    
    static let std = CompatibilityDefaults(
        
        drivePowerSave: true,
        viciiPowerSave: true,
        sidPowerSave: false,
        
        ssCollisions: true,
        sbCollisions: true
    )
    
    static let accurate = CompatibilityDefaults(
        
        drivePowerSave: false,
        viciiPowerSave: false,
        sidPowerSave: false,
        
        ssCollisions: true,
        sbCollisions: true
    )
    
    static let accelerated = CompatibilityDefaults(
        
        drivePowerSave: true,
        viciiPowerSave: true,
        sidPowerSave: true,
        
        ssCollisions: false,
        sbCollisions: false
    )
}

extension UserDefaults {

    static func registerCompatibilityUserDefaults() {

        let defaults = CompatibilityDefaults.std
        let dictionary: [String: Any] = [

            Keys.Com.drivePowerSave: defaults.drivePowerSave,
            Keys.Com.viciiPowerSave: defaults.viciiPowerSave,
            Keys.Com.sidPowerSave: defaults.sidPowerSave,

            Keys.Com.ssCollisions: defaults.ssCollisions,
            Keys.Com.sbCollisions: defaults.sbCollisions
        ]

        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }

    static func resetCompatibilityUserDefaults() {

        let userDefaults = UserDefaults.standard

        let keys = [ Keys.Com.drivePowerSave,
                     Keys.Com.viciiPowerSave,
                     Keys.Com.sidPowerSave,
                     
                     Keys.Com.ssCollisions,
                     Keys.Com.sbCollisions
        ]

        for key in keys { userDefaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Audio)
//

extension Keys {
    
    struct Aud {
        
        // Engine
        static let sidEngine          = "VC64_AUD_SidEngine"
        static let sidSampling        = "VC64_AUD_Sampling"
        
        // In
        static let vol0               = "VC64_AUD_Volume0"
        static let vol1               = "VC64_AUD_Volume1"
        static let vol2               = "VC64_AUD_Volume2"
        static let vol3               = "VC64_AUD_Volume3"
        static let pan0               = "VC64_AUD_Pan0"
        static let pan1               = "VC64_AUD_Pan1"
        static let pan2               = "VC64_AUD_Pan2"
        static let pan3               = "VC64_AUD_Pan3"
        
        // Out
        static let volL               = "VC64_AUD_VolumeL"
        static let volR               = "VC64_AUD_VolumeR"
        
        // Drive volumes
        static let stepVolume         = "VC64_AUD_StepVolume"
        static let insertVolume       = "VC64_AUD_InsertVolume"
        static let ejectVolume        = "VC64_AUD_EjectVolume"
        static let drive8Pan          = "VC64_AUD_Drive8Pan"
        static let drive9Pan          = "VC64_AUD_Drive9Pan"
    }
}

struct AudioDefaults {
    
    // Audio Engine
    let sidEngine: SIDEngine
    let sidSampling: SamplingMethod
    
    // In
    let vol0: Int
    let vol1: Int
    let vol2: Int
    let vol3: Int
    let pan0: Int
    let pan1: Int
    let pan2: Int
    let pan3: Int
    
    // Out
    let volL: Int
    let volR: Int
    
    // Drive
    var drivePan: [Int]
    var stepVolume: Int
    var insertVolume: Int
    var ejectVolume: Int
    
    //
    // Schemes
    //
    
    static let mono = AudioDefaults(
        
        sidEngine: .RESID,
        sidSampling: .INTERPOLATE,
        
        vol0: 400,
        vol1: 400,
        vol2: 400,
        vol3: 400,
        pan0: 0,
        pan1: 0,
        pan2: 0,
        pan3: 0,
        
        volL: 50,
        volR: 50,
        
        drivePan: [0, 0],
        stepVolume: 50,
        insertVolume: 50,
        ejectVolume: 50
    )
    
    static let stereo = AudioDefaults(
        
        sidEngine: .RESID,
        sidSampling: .INTERPOLATE,

        vol0: 400,
        vol1: 400,
        vol2: 400,
        vol3: 400,
        pan0: 150,
        pan1: 50,
        pan2: 150,
        pan3: 50,
        
        volL: 50,
        volR: 50,
        
        drivePan: [50, 150],
        stepVolume: 50,
        insertVolume: 50,
        ejectVolume: 50
    )
}

extension UserDefaults {

    static func registerAudioUserDefaults() {

        let defaults = AudioDefaults.mono
        let dictionary: [String: Any] = [

            Keys.Aud.sidEngine: Int(defaults.sidEngine.rawValue),
            Keys.Aud.sidSampling: Int(defaults.sidSampling.rawValue),

            Keys.Aud.vol0: defaults.vol0,
            Keys.Aud.vol1: defaults.vol1,
            Keys.Aud.vol2: defaults.vol2,
            Keys.Aud.vol3: defaults.vol3,
            Keys.Aud.pan0: defaults.pan0,
            Keys.Aud.pan1: defaults.pan1,
            Keys.Aud.pan2: defaults.pan2,
            Keys.Aud.pan3: defaults.pan3,
            
            Keys.Aud.volL: defaults.volL,
            Keys.Aud.volR: defaults.volR,

            Keys.Aud.drive8Pan: defaults.drivePan[0],
            Keys.Aud.drive9Pan: defaults.drivePan[1],
            Keys.Aud.stepVolume: defaults.stepVolume,
            Keys.Aud.insertVolume: defaults.insertVolume,
            Keys.Aud.ejectVolume: defaults.ejectVolume
        ]

        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }

    static func resetAudioUserDefaults() {

        let userDefaults = UserDefaults.standard
        
        let keys = [ Keys.Aud.sidEngine,
                     Keys.Aud.sidSampling,
                     
                     Keys.Aud.vol0,
                     Keys.Aud.vol1,
                     Keys.Aud.vol2,
                     Keys.Aud.vol3,
                     Keys.Aud.pan0,
                     Keys.Aud.pan1,
                     Keys.Aud.pan2,
                     Keys.Aud.pan3,
                     
                     Keys.Aud.volL,
                     Keys.Aud.volR,
        
                     Keys.Aud.drive8Pan,
                     Keys.Aud.drive9Pan,
                     Keys.Aud.stepVolume,
                     Keys.Aud.insertVolume,
                     Keys.Aud.ejectVolume
        ]

        for key in keys { userDefaults.removeObject(forKey: key) }
    }
}

//
// User defaults (Video)
//

extension Keys {
    
    struct Vid {
        
        // Colors
        static let palette            = "VC64_VID_Palette"
        static let brightness         = "VC64_VID_Brightness"
        static let contrast           = "VC64_VID_Contrast"
        static let saturation         = "VC64_VID_Saturation"
        
        // Geometry
        static let hCenter            = "VC64_VID_HCenter"
        static let vCenter            = "VC64_VID_VCenter"
        static let hZoom              = "VC64_VID_HZoom"
        static let vZoom              = "VC64_VID_VZoom"
        
        // Upscalers
        static let upscaler           = "VC64_VID_Upscaler"
        
        // Shader options
        static let blur               = "VC64_VID_Blur"
        static let blurRadius         = "VC64_VID_BlurRadius"
        static let bloom              = "VC64_VID_Bloom"
        static let bloomRadiusR       = "VC64_VID_BloonRadiusR"
        static let bloomRadiusG       = "VC64_VID_BloonRadiusG"
        static let bloomRadiusB       = "VC64_VID_BloonRadiusB"
        static let bloomBrightness    = "VC64_VID_BloomBrightness"
        static let bloomWeight        = "VC64_VID_BloomWeight"
        static let flicker            = "VC64_VID_Flicker"
        static let flickerWeight      = "VC64_VID_FlickerWeight"
        static let dotMask            = "VC64_VID_DotMask"
        static let dotMaskBrightness  = "VC64_VID_DotMaskBrightness"
        static let scanlines          = "VC64_VID_Scanlines"
        static let scanlineBrightness = "VC64_VID_ScanlineBrightness"
        static let scanlineWeight     = "VC64_VID_ScanlineWeight"
        static let disalignment       = "VC64_VID_Disalignment"
        static let disalignmentH      = "VC64_VID_DisalignmentH"
        static let disalignmentV      = "VC64_VID_DisalignmentV"
    }
}

struct VideoDefaults {
    
    // Colors
    let palette: Palette
    let brightness: Int
    let contrast: Int
    let saturation: Int
    
    // Geometry
    let hCenter: Float
    let vCenter: Float
    let hZoom: Float
    let vZoom: Float
    
    // Upscalers
    let upscaler: Int
    
    // Shader options
    let blur: Int32
    let blurRadius: Float
    let bloom: Int
    let bloomRadiusR: Float
    let bloomRadiusG: Float
    let bloomRadiusB: Float
    let bloomBrightness: Float
    let bloomWeight: Float
    let flicker: Int32
    let flickerWeight: Float
    let dotMask: Int
    let dotMaskBrightness: Float
    let scanlines: Int
    let scanlineBrightness: Float
    let scanlineWeight: Float
    let disalignment: Int32
    let disalignmentH: Float
    let disalignmentV: Float
    
    //
    // Schemes
    //
    
    // TFT monitor appearance
    static let tft = VideoDefaults(
        
        palette: .COLOR,
        brightness: 50,
        contrast: 100,
        saturation: 50,
        
        hCenter: 0,
        vCenter: 0,
        hZoom: 0,
        vZoom: 0.046,
        
        upscaler: 0,
        
        blur: 1,
        blurRadius: 0,
        bloom: 0,
        bloomRadiusR: 1.0,
        bloomRadiusG: 1.0,
        bloomRadiusB: 1.0,
        bloomBrightness: 0.4,
        bloomWeight: 1.21,
        flicker: 1,
        flickerWeight: 0.5,
        dotMask: 0,
        dotMaskBrightness: 0.7,
        scanlines: 0,
        scanlineBrightness: 0.55,
        scanlineWeight: 0.11,
        disalignment: 0,
        disalignmentH: 0.001,
        disalignmentV: 0.001
    )
    
    // CRT monitor appearance
    static let crt = VideoDefaults(
        
        palette: .COLOR,
        brightness: 50,
        contrast: 100,
        saturation: 50,
        
        hCenter: 0,
        vCenter: 0,
        hZoom: 0,
        vZoom: 0.046,
        
        upscaler: 0,
        
        blur: 1,
        blurRadius: 1.5,
        bloom: 1,
        bloomRadiusR: 1.0,
        bloomRadiusG: 1.0,
        bloomRadiusB: 1.0,
        bloomBrightness: 0.4,
        bloomWeight: 1.21,
        flicker: 1,
        flickerWeight: 0.5,
        dotMask: 1,
        dotMaskBrightness: 0.5,
        scanlines: 2,
        scanlineBrightness: 0.55,
        scanlineWeight: 0.11,
        disalignment: 0,
        disalignmentH: 0.001,
        disalignmentV: 0.001
    )
}

extension UserDefaults {
    
    static func registerVideoUserDefaults() {
        
        let defaults = VideoDefaults.tft
        let dictionary: [String: Any] = [
            
            Keys.Vid.palette: Int(defaults.palette.rawValue),
            Keys.Vid.brightness: defaults.brightness,
            Keys.Vid.contrast: defaults.contrast,
            Keys.Vid.saturation: defaults.saturation,
            
            Keys.Vid.hCenter: defaults.hCenter,
            Keys.Vid.vCenter: defaults.vCenter,
            Keys.Vid.hZoom: defaults.hZoom,
            Keys.Vid.vZoom: defaults.vZoom,
            
            Keys.Vid.upscaler: defaults.upscaler,
            
            Keys.Vid.blur: defaults.blur,
            Keys.Vid.blurRadius: defaults.blurRadius,
            Keys.Vid.bloom: defaults.bloom,
            Keys.Vid.bloomRadiusR: defaults.bloomRadiusR,
            Keys.Vid.bloomRadiusG: defaults.bloomRadiusG,
            Keys.Vid.bloomRadiusB: defaults.bloomRadiusB,
            Keys.Vid.bloomBrightness: defaults.bloomBrightness,
            Keys.Vid.bloomWeight: defaults.bloomWeight,
            Keys.Vid.flicker: defaults.flicker,
            Keys.Vid.flickerWeight: defaults.flickerWeight,
            Keys.Vid.dotMask: defaults.dotMask,
            Keys.Vid.dotMaskBrightness: defaults.dotMaskBrightness,
            Keys.Vid.scanlines: defaults.scanlines,
            Keys.Vid.scanlineBrightness: defaults.scanlineBrightness,
            Keys.Vid.scanlineWeight: defaults.scanlineWeight,
            Keys.Vid.disalignment: defaults.disalignment,
            Keys.Vid.disalignmentH: defaults.disalignmentH,
            Keys.Vid.disalignmentV: defaults.disalignmentV
        ]
        
        let userDefaults = UserDefaults.standard
        userDefaults.register(defaults: dictionary)
    }
    
    static func resetVideoUserDefaults() {
        
        let defaults = UserDefaults.standard
                
        let keys = [ Keys.Vid.palette,
                     Keys.Vid.brightness,
                     Keys.Vid.contrast,
                     Keys.Vid.saturation,
                     
                     Keys.Vid.hCenter,
                     Keys.Vid.vCenter,
                     Keys.Vid.hZoom,
                     Keys.Vid.vZoom,
                     
                     Keys.Vid.upscaler,
                     
                     Keys.Vid.blur,
                     Keys.Vid.blurRadius,
                     Keys.Vid.bloom,
                     Keys.Vid.bloomRadiusR,
                     Keys.Vid.bloomRadiusG,
                     Keys.Vid.bloomRadiusB,
                     Keys.Vid.bloomBrightness,
                     Keys.Vid.bloomWeight,
                     Keys.Vid.flicker,
                     Keys.Vid.flickerWeight,
                     Keys.Vid.dotMask,
                     Keys.Vid.dotMaskBrightness,
                     Keys.Vid.scanlines,
                     Keys.Vid.scanlineBrightness,
                     Keys.Vid.scanlineWeight,
                     Keys.Vid.disalignment,
                     Keys.Vid.disalignmentH,
                     Keys.Vid.disalignmentV
        ]
        
        for key in keys { defaults.removeObject(forKey: key) }
    }
}
