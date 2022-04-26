// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* This class stores all items that are specific to an individual emulator
 * instance. Each instance keeps its own object of this class inside the
 * document controller.
 *
 * See class "Preferences" for shared settings.
 */

class Configuration {
    
    var parent: MyController!
    var c64: C64Proxy { return parent.c64 }
    var renderer: Renderer { return parent.renderer }
    var gamePadManager: GamePadManager { return parent.gamePadManager }
    var ressourceManager: RessourceManager { return renderer.ressourceManager }
    
    //
    // Hardware
    //
    
    var vicRevision: Int {
        get { return c64.getConfig(.VIC_REVISION) }
        set { c64.configure(.VIC_REVISION, value: newValue) }
    }

    var vicSpeed: Int {
        get { return c64.getConfig(.VIC_SPEED) }
        set { c64.configure(.VIC_SPEED, value: newValue) }
    }

    var vicGrayDotBug: Bool {
        get { return c64.getConfig(.GRAY_DOT_BUG) != 0 }
        set { c64.configure(.GRAY_DOT_BUG, enable: newValue) }
    }
    
    var vicHideSprites: Bool {
        get { return c64.getConfig(.HIDE_SPRITES) != 0 }
        set { c64.configure(.HIDE_SPRITES, enable: newValue) }
    }
        
    var vicCutLayers: Int {
        get { return c64.getConfig(.CUT_LAYERS) }
        set { c64.configure(.CUT_LAYERS, value: newValue) }
    }
    
    var vicCutOpacity: Int {
        get { return c64.getConfig(.CUT_OPACITY) }
        set { c64.configure(.CUT_OPACITY, value: newValue) }
    }
        
    var ciaRevision: Int {
        get { return c64.getConfig(.CIA_REVISION) }
        set { c64.configure(.CIA_REVISION, value: newValue) }
    }
    
    var ciaTimerBBug: Bool {
        get { return c64.getConfig(.TIMER_B_BUG) != 0}
        set { c64.configure(.TIMER_B_BUG, enable: newValue) }
    }
    
    var sidRevision: Int {
        get { return c64.getConfig(.SID_REVISION) }
        set { c64.configure(.SID_REVISION, value: newValue) }
    }
    
    var sidFilter: Bool {
        get { return c64.getConfig(.SID_FILTER) != 0 }
        set { c64.configure(.SID_FILTER, enable: newValue) }
    }
    
    var glueLogic: Int {
        get { return c64.getConfig(.GLUE_LOGIC) }
        set { c64.configure(.GLUE_LOGIC, value: newValue) }
    }

    var powerGrid: Int {
        get { return c64.getConfig(.POWER_GRID) }
        set { c64.configure(.POWER_GRID, value: newValue) }
    }

    var ramPattern: Int {
        get { return c64.getConfig(.RAM_PATTERN) }
        set { c64.configure(.RAM_PATTERN, value: newValue) }
    }
    
    //
    // Peripherals
    //
    
    var drive8Connected: Bool {
        get { return c64.getConfig(.DRV_CONNECT, drive: .DRIVE8) != 0 }
        set { c64.configure(.DRV_CONNECT, drive: .DRIVE8, enable: newValue ) }
    }
    
    var drive9Connected: Bool {
        get { return c64.getConfig(.DRV_CONNECT, drive: .DRIVE9) != 0 }
        set { c64.configure(.DRV_CONNECT, drive: .DRIVE9, enable: newValue ) }
    }

    var drive8AutoConf: Bool {
        get { return c64.getConfig(.DRV_AUTO_CONFIG, drive: .DRIVE8) != 0 }
        set { c64.configure(.DRV_AUTO_CONFIG, drive: .DRIVE8, enable: newValue ) }
    }
    
    var drive9AutoConf: Bool {
        get { return c64.getConfig(.DRV_AUTO_CONFIG, drive: .DRIVE9) != 0}
        set { c64.configure(.DRV_AUTO_CONFIG, drive: .DRIVE9, enable: newValue ) }
    }
    
    var drive8Type: Int {
        get { return c64.getConfig(.DRV_TYPE, drive: .DRIVE8) }
        set { c64.configure(.DRV_TYPE, drive: .DRIVE8, value: newValue )}
    }
    
    var drive9Type: Int {
        get { return c64.getConfig(.DRV_TYPE, drive: .DRIVE9) }
        set { c64.configure(.DRV_TYPE, drive: .DRIVE9, value: newValue )}
    }

    var drive8Ram: Int {
        get { return c64.getConfig(.DRV_RAM, drive: .DRIVE8) }
        set { c64.configure(.DRV_RAM, drive: .DRIVE8, value: newValue )}
    }

    var drive9Ram: Int {
        get { return c64.getConfig(.DRV_RAM, drive: .DRIVE9) }
        set { c64.configure(.DRV_RAM, drive: .DRIVE9, value: newValue )}
    }

    var drive8ParCable: Int {
        get { return c64.getConfig(.DRV_PARCABLE, drive: .DRIVE8) }
        set { c64.configure(.DRV_PARCABLE, drive: .DRIVE8, value: newValue ) }
    }

    var drive9ParCable: Int {
        get { return c64.getConfig(.DRV_PARCABLE, drive: .DRIVE9) }
        set { c64.configure(.DRV_PARCABLE, drive: .DRIVE9, value: newValue ) }
    }
    
    var drive8PowerSwitch: Bool {
        get { return c64.getConfig(.DRV_POWER_SWITCH, drive: .DRIVE8) != 0 }
        set { c64.configure(.DRV_POWER_SWITCH, drive: .DRIVE8, enable: newValue )}
    }
    
    var drive9PowerSwitch: Bool {
        get { return c64.getConfig(.DRV_POWER_SWITCH, drive: .DRIVE9) != 0 }
        set { c64.configure(.DRV_POWER_SWITCH, drive: .DRIVE9, enable: newValue )}
    }

    var blankDiskFormat = PeripheralsDefaults.std.blankDiskFormat
    var blankDiskFormatIntValue: Int {
        get { return Int(blankDiskFormat.rawValue) }
        set { blankDiskFormat = DOSType(rawValue: newValue) ?? .NODOS }
    }

    var gameDevice1 = PeripheralsDefaults.std.gameDevice1 {
        didSet {
            
            // Try to connect the device (may disconnect the other device)
            gamePadManager.connect(slot: gameDevice1, port: 1)
            gamePadManager.listDevices()
            
            // Avoid double mappings
            if gameDevice1 != -1 && gameDevice1 == gameDevice2 {
                gameDevice2 = -1
            }
            
            parent.toolbar.validateVisibleItems()
        }
    }
    
    var gameDevice2 = PeripheralsDefaults.std.gameDevice2 {
        didSet {
            
            // Try to connect the device (may disconnect the other device)
            gamePadManager.connect(slot: gameDevice2, port: 2)
            gamePadManager.listDevices()
            
            // Avoid double mappings
            if gameDevice2 != -1 && gameDevice2 == gameDevice1 {
                gameDevice1 = -1
            }
            
            parent.toolbar.validateVisibleItems()
        }
    }
    
    var mouseModel: Int {
        get { return c64.getConfig(.MOUSE_MODEL, id: 1) }
        set { c64.configure(.MOUSE_MODEL, value: newValue) }
    }
    
    //
    // Compatibility
    //

    var drivePowerSave: Bool {
        get { return c64.getConfig(.DRV_POWER_SAVE, id: 8) != 0 }
        set { c64.configure(.DRV_POWER_SAVE, enable: newValue) }
    }

    var viciiPowerSave: Bool {
        get { return c64.getConfig(.VIC_POWER_SAVE) != 0 }
        set { c64.configure(.VIC_POWER_SAVE, enable: newValue) }
    }

    var sidPowerSave: Bool {
        get { return c64.getConfig(.SID_POWER_SAVE) != 0 }
        set { c64.configure(.SID_POWER_SAVE, enable: newValue) }
    }

    var ssCollisions: Bool {
        get { return c64.getConfig(.SS_COLLISIONS) != 0 }
        set { c64.configure(.SS_COLLISIONS, enable: newValue) }
    }

    var sbCollisions: Bool {
        get { return c64.getConfig(.SB_COLLISIONS) != 0 }
        set { c64.configure(.SB_COLLISIONS, enable: newValue) }
    }
        
    //
    // Audio
    //
    
    var sidEnable1: Bool {
        get { return c64.getConfig(.SID_ENABLE, id: 1) != 0 }
        set { c64.configure(.SID_ENABLE, id: 1, enable: newValue) }
    }
    var sidEnable2: Bool {
        get { return c64.getConfig(.SID_ENABLE, id: 2) != 0 }
        set { c64.configure(.SID_ENABLE, id: 2, enable: newValue) }
    }
    var sidEnable3: Bool {
        get { return c64.getConfig(.SID_ENABLE, id: 3) != 0 }
        set { c64.configure(.SID_ENABLE, id: 3, enable: newValue) }
    }
    var sidAddress1: Int {
        get { return c64.getConfig(.SID_ADDRESS, id: 1) }
        set { c64.configure(.SID_ADDRESS, id: 1, value: newValue) }
    }
    var sidAddress2: Int {
        get { return c64.getConfig(.SID_ADDRESS, id: 2) }
        set { c64.configure(.SID_ADDRESS, id: 2, value: newValue) }
    }
    var sidAddress3: Int {
        get { return c64.getConfig(.SID_ADDRESS, id: 3) }
        set { c64.configure(.SID_ADDRESS, id: 3, value: newValue) }
    }
    var sidEngine: Int {
        get { return c64.getConfig(.SID_ENGINE) }
        set { c64.configure(.SID_ENGINE, value: newValue) }
    }
    var sidSampling: Int {
        get { return c64.getConfig(.SID_SAMPLING) }
        set { c64.configure(.SID_SAMPLING, value: newValue) }
    }
    var vol0: Int {
        get { return c64.getConfig(.AUDVOL, id: 0) }
        set { c64.configure(.AUDVOL, id: 0, value: newValue) }
    }
    var vol1: Int {
        get { return c64.getConfig(.AUDVOL, id: 1) }
        set { c64.configure(.AUDVOL, id: 1, value: newValue) }
    }
    var vol2: Int {
        get { return c64.getConfig(.AUDVOL, id: 2) }
        set { c64.configure(.AUDVOL, id: 2, value: newValue) }
    }
    var vol3: Int {
        get { return c64.getConfig(.AUDVOL, id: 3) }
        set { c64.configure(.AUDVOL, id: 3, value: newValue) }
    }
    var pan0: Int {
        get { return c64.getConfig(.AUDPAN, id: 0) }
        set { c64.configure(.AUDPAN, id: 0, value: newValue) }
    }
    var pan1: Int {
        get { return c64.getConfig(.AUDPAN, id: 1) }
        set { c64.configure(.AUDPAN, id: 1, value: newValue) }
    }
    var pan2: Int {
        get { return c64.getConfig(.AUDPAN, id: 2) }
        set { c64.configure(.AUDPAN, id: 2, value: newValue) }
    }
    var pan3: Int {
        get { return c64.getConfig(.AUDPAN, id: 3) }
        set { c64.configure(.AUDPAN, id: 3, value: newValue) }
    }
    var volL: Int {
        get { return c64.getConfig(.AUDVOLL) }
        set { c64.configure(.AUDVOLL, value: newValue) }
    }
    var volR: Int {
        get { return c64.getConfig(.AUDVOLR) }
        set { c64.configure(.AUDVOLR, value: newValue) }
    }
    var stepVolume: Int {
        get { return c64.getConfig(.DRV_STEP_VOL, drive: .DRIVE8) }
        set { c64.configure(.DRV_STEP_VOL, value: newValue) }
    }
    var insertVolume: Int {
        get { return c64.getConfig(.DRV_INSERT_VOL, drive: .DRIVE8) }
        set { c64.configure(.DRV_INSERT_VOL, value: newValue) }
    }
    var ejectVolume: Int {
        get { return c64.getConfig(.DRV_EJECT_VOL, drive: .DRIVE8) }
        set { c64.configure(.DRV_EJECT_VOL, value: newValue) }
    }
    var drive8Pan: Int {
        get { return c64.getConfig(.DRV_PAN, drive: .DRIVE8) }
        set { c64.configure(.DRV_PAN, drive: .DRIVE8, value: newValue) }
    }
    var drive9Pan: Int {
        get { return c64.getConfig(.DRV_PAN, drive: .DRIVE9) }
        set { c64.configure(.DRV_PAN, drive: .DRIVE9, value: newValue) }
    }

    //
    // Video
    //
    
    var palette: Int {
        get { return c64.getConfig(.PALETTE) }
        set { c64.configure(.PALETTE, value: newValue) }
    }
    var brightness: Int {
        get { return c64.getConfig(.BRIGHTNESS) }
        set { c64.configure(.BRIGHTNESS, value: newValue) }
    }
    var contrast: Int {
        get { return c64.getConfig(.CONTRAST) }
        set { c64.configure(.CONTRAST, value: newValue) }
    }
    var saturation: Int {
        get { c64.getConfig(.SATURATION) }
        set { c64.configure(.SATURATION, value: newValue) }
    }
    var hCenter = VideoDefaults.tft.hCenter {
        didSet { renderer.canvas.updateTextureRect() }
    }
    var vCenter = VideoDefaults.tft.vCenter {
        didSet { renderer.canvas.updateTextureRect() }
    }
    var hZoom = VideoDefaults.tft.hZoom {
        didSet { renderer.canvas.updateTextureRect() }
    }
    var vZoom = VideoDefaults.tft.vZoom {
        didSet { renderer.canvas.updateTextureRect() }
    }
    var upscaler = VideoDefaults.tft.upscaler {
        didSet {
            if !ressourceManager.selectUpscaler(upscaler) { upscaler = oldValue }
        }
    }
    var blur = VideoDefaults.tft.blur {
        didSet { renderer.shaderOptions.blur = blur }
    }
    var blurRadius = VideoDefaults.tft.blurRadius {
        didSet { renderer.shaderOptions.blurRadius = blurRadius }
    }
    var bloom = VideoDefaults.tft.bloom {
        didSet {
            renderer.shaderOptions.bloom = Int32(bloom)
            if !ressourceManager.selectBloomFilter(bloom) { bloom = oldValue }
        }
    }
    var bloomRadiusR = VideoDefaults.tft.bloomRadiusR {
        didSet { renderer.shaderOptions.bloomRadiusR = bloomRadiusR }
    }
    var bloomRadiusG = VideoDefaults.tft.bloomRadiusG {
        didSet { renderer.shaderOptions.bloomRadiusG = bloomRadiusG }
    }
    var bloomRadiusB = VideoDefaults.tft.bloomRadiusB {
        didSet { renderer.shaderOptions.bloomRadiusB = bloomRadiusB }
    }
    var bloomBrightness = VideoDefaults.tft.bloomBrightness {
        didSet { renderer.shaderOptions.bloomBrightness = bloomBrightness }
    }
    var bloomWeight = VideoDefaults.tft.bloomWeight {
        didSet { renderer.shaderOptions.bloomWeight = bloomWeight }
    }
    var dotMask = VideoDefaults.tft.dotMask {
        didSet {
            renderer.shaderOptions.dotMask = Int32(dotMask)
            ressourceManager.buildDotMasks()
            if !ressourceManager.selectDotMask(dotMask) { dotMask = oldValue }
        }
    }
    var dotMaskBrightness = VideoDefaults.tft.dotMaskBrightness {
        didSet {
            renderer.shaderOptions.dotMaskBrightness = dotMaskBrightness
            ressourceManager.buildDotMasks()
        }
    }
    var scanlines = VideoDefaults.tft.scanlines {
        didSet {
            renderer.shaderOptions.scanlines = Int32(scanlines)
            if !ressourceManager.selectScanlineFilter(scanlines) { scanlines = oldValue }
        }
    }
    var scanlineBrightness = VideoDefaults.tft.scanlineBrightness {
        didSet { renderer.shaderOptions.scanlineBrightness = scanlineBrightness }
    }
    var scanlineWeight = VideoDefaults.tft.scanlineWeight {
        didSet { renderer.shaderOptions.scanlineWeight = scanlineWeight }
    }
    var disalignment = VideoDefaults.tft.disalignment {
        didSet { renderer.shaderOptions.disalignment = disalignment }
    }
    var disalignmentH = VideoDefaults.tft.disalignmentH {
        didSet { renderer.shaderOptions.disalignmentH = disalignmentH }
    }
    var disalignmentV = VideoDefaults.tft.disalignmentV {
        didSet { renderer.shaderOptions.disalignmentV = disalignmentV }
    }
    
    init(with controller: MyController) { parent = controller }
    
    //
    // Roms
    //
    
    func loadRomUserDefaults() {
        
        func load(_ url: URL?, type: FileType) {
            
            if url != nil {
                if let file = try? Proxy.make(url: url!) as RomFileProxy {
                    if file.type == type { c64.loadRom(file) }
                }
            }
        }
        
        c64.suspend()
        load(UserDefaults.basicRomUrl, type: .BASIC_ROM)
        load(UserDefaults.charRomUrl, type: .CHAR_ROM)
        load(UserDefaults.kernalRomUrl, type: .KERNAL_ROM)
        load(UserDefaults.vc1541RomUrl, type: .VC1541_ROM)
        c64.resume()
    }
    
    func saveRomUserDefaults() {
        
        track()
        var url: URL?
        
        func save(_ type: RomType) throws {
            
            if url == nil { throw VC64Error(ErrorCode.FILE_CANT_WRITE) }
            try? FileManager.default.removeItem(at: url!)
            try c64.saveRom(type, url: url!)
        }
        
        c64.suspend()
        
        do {
            url = UserDefaults.basicRomUrl;  try save(.BASIC)
            url = UserDefaults.charRomUrl;   try save(.CHAR)
            url = UserDefaults.kernalRomUrl; try save(.KERNAL)
            url = UserDefaults.vc1541RomUrl; try save(.VC1541)
            
        } catch {
            if error is VC64Error && url != nil {
                VC64Error.warning("Failed to save Roms",
                                  "Can't write to file \(url!.path)")
            }
            if error is VC64Error && url == nil {
                VC64Error.warning("Failed to save Roms",
                                  "Unable to access the application defaults folder")
            }
        }
        
        c64.resume()
    }
    
    //
    // Hardware
    //
    
    func loadHardwareDefaults(_ defaults: HardwareDefaults) {
        
        c64.suspend()
        
        vicRevision = defaults.vicRevision.rawValue
        vicSpeed = defaults.vicSpeed.rawValue
        vicGrayDotBug = defaults.vicGrayDotBug
        
        ciaRevision = defaults.ciaRevision.rawValue
        ciaTimerBBug = defaults.ciaTimerBBug
        
        sidRevision = defaults.sidRevision.rawValue
        sidFilter = defaults.sidFilter
        
        sidEnable1 = defaults.sidEnable1
        sidEnable2 = defaults.sidEnable2
        sidEnable3 = defaults.sidEnable3
        sidAddress1 = defaults.sidAddress1
        sidAddress2 = defaults.sidAddress2
        sidAddress3 = defaults.sidAddress3
        
        glueLogic = defaults.glueLogic.rawValue
        powerGrid = defaults.powerGrid.rawValue
        
        ramPattern = defaults.ramPattern.rawValue
        
        c64.resume()
    }
    
    func loadHardwareUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        c64.suspend()
        
        vicRevision = defaults.integer(forKey: Keys.Hwd.vicRevision)
        vicSpeed = defaults.integer(forKey: Keys.Hwd.vicSpeed)
        vicGrayDotBug = defaults.bool(forKey: Keys.Hwd.vicGrayDotBug)
        
        ciaRevision = defaults.integer(forKey: Keys.Hwd.ciaRevision)
        ciaTimerBBug = defaults.bool(forKey: Keys.Hwd.ciaTimerBBug)
        
        sidRevision = defaults.integer(forKey: Keys.Hwd.sidRevision)
        sidFilter = defaults.bool(forKey: Keys.Hwd.sidFilter)
        sidEnable1 = defaults.bool(forKey: Keys.Hwd.sidEnable1)
        sidEnable2 = defaults.bool(forKey: Keys.Hwd.sidEnable2)
        sidEnable3 = defaults.bool(forKey: Keys.Hwd.sidEnable3)
        sidAddress1 = defaults.integer(forKey: Keys.Hwd.sidAddress1)
        sidAddress2 = defaults.integer(forKey: Keys.Hwd.sidAddress2)
        sidAddress3 = defaults.integer(forKey: Keys.Hwd.sidAddress3)
        
        glueLogic = defaults.integer(forKey: Keys.Hwd.glueLogic)
        powerGrid = defaults.integer(forKey: Keys.Hwd.powerGrid)
        
        ramPattern = defaults.integer(forKey: Keys.Hwd.ramPattern)
        
        c64.resume()
    }
    
    func saveHardwareUserDefaults() {
        
        track()
        
        let defaults = UserDefaults.standard
        
        defaults.set(vicRevision, forKey: Keys.Hwd.vicRevision)
        defaults.set(vicSpeed, forKey: Keys.Hwd.vicSpeed)
        defaults.set(vicGrayDotBug, forKey: Keys.Hwd.vicGrayDotBug)
        
        defaults.set(ciaRevision, forKey: Keys.Hwd.ciaRevision)
        defaults.set(ciaTimerBBug, forKey: Keys.Hwd.ciaTimerBBug)
        
        defaults.set(sidRevision, forKey: Keys.Hwd.sidRevision)
        defaults.set(sidFilter, forKey: Keys.Hwd.sidFilter)
        defaults.set(sidEnable1, forKey: Keys.Hwd.sidEnable1)
        defaults.set(sidEnable2, forKey: Keys.Hwd.sidEnable2)
        defaults.set(sidEnable3, forKey: Keys.Hwd.sidEnable3)
        defaults.set(sidAddress1, forKey: Keys.Hwd.sidAddress1)
        defaults.set(sidAddress2, forKey: Keys.Hwd.sidAddress2)
        defaults.set(sidAddress3, forKey: Keys.Hwd.sidAddress3)
        
        defaults.set(glueLogic, forKey: Keys.Hwd.glueLogic)
        defaults.set(powerGrid, forKey: Keys.Hwd.powerGrid)

        defaults.set(ramPattern, forKey: Keys.Hwd.ramPattern)
    }
    
    //
    // Peripherals
    //
    
    func loadPeripheralsDefaults(_ defaults: PeripheralsDefaults) {
        
        c64.suspend()
        
        drive8Connected = defaults.driveConnect[0]
        drive8Type = defaults.driveModel[0].rawValue
        drive8Ram = defaults.driveRam[0].rawValue
        drive8ParCable = defaults.parCable[0].rawValue
        drive8AutoConf = defaults.driveAutoConf[0]
        drive9Connected = defaults.driveConnect[1]
        drive9Type = defaults.driveModel[1].rawValue
        drive9Ram = defaults.driveRam[1].rawValue
        drive9ParCable = defaults.parCable[1].rawValue
        drive9AutoConf = defaults.driveAutoConf[1]

        blankDiskFormat = defaults.blankDiskFormat
        
        gameDevice1 = defaults.gameDevice1
        gameDevice2 = defaults.gameDevice2
        
        mouseModel = defaults.mouseModel.rawValue
        
        c64.resume()
    }
    
    func loadPeripheralsUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        c64.suspend()
        
        drive8Connected = defaults.bool(forKey: Keys.Per.drive8Connect)
        drive8Type = defaults.integer(forKey: Keys.Per.drive8Model)
        drive8Ram = defaults.integer(forKey: Keys.Per.drive8Ram)
        drive8ParCable = defaults.integer(forKey: Keys.Per.drive8ParCable)
        drive8AutoConf = defaults.bool(forKey: Keys.Per.drive8AutoConf)
        drive9Connected = defaults.bool(forKey: Keys.Per.drive9Connect)
        drive9Type = defaults.integer(forKey: Keys.Per.drive9Model)
        drive9Ram = defaults.integer(forKey: Keys.Per.drive9Ram)
        drive9ParCable = defaults.integer(forKey: Keys.Per.drive9ParCable)
        drive9AutoConf = defaults.bool(forKey: Keys.Per.drive9AutoConf)

        blankDiskFormatIntValue = defaults.integer(forKey: Keys.Per.blankDiskFormat)
        
        gameDevice1 = defaults.integer(forKey: Keys.Per.gameDevice1)
        gameDevice2 = defaults.integer(forKey: Keys.Per.gameDevice2)
        
        mouseModel = defaults.integer(forKey: Keys.Per.mouseModel)
        
        c64.resume()
    }
    
    func savePeripheralsUserDefaults() {
                
        let defaults = UserDefaults.standard
        
        defaults.set(drive8Connected, forKey: Keys.Per.drive8Connect)
        defaults.set(drive8Type, forKey: Keys.Per.drive8Model)
        defaults.set(drive8Ram, forKey: Keys.Per.drive8Ram)
        defaults.set(drive8ParCable, forKey: Keys.Per.drive8ParCable)
        defaults.set(drive8AutoConf, forKey: Keys.Per.drive8AutoConf)
        defaults.set(drive9Connected, forKey: Keys.Per.drive9Connect)
        defaults.set(drive9Type, forKey: Keys.Per.drive9Model)
        defaults.set(drive9Ram, forKey: Keys.Per.drive9Ram)
        defaults.set(drive9ParCable, forKey: Keys.Per.drive9ParCable)
        defaults.set(drive9AutoConf, forKey: Keys.Per.drive9AutoConf)

        defaults.set(blankDiskFormatIntValue, forKey: Keys.Per.blankDiskFormat)
        
        defaults.set(gameDevice1, forKey: Keys.Per.gameDevice1)
        defaults.set(gameDevice2, forKey: Keys.Per.gameDevice2)
        
        defaults.set(mouseModel, forKey: Keys.Per.mouseModel)
    }
    
    //
    // Compatibility
    //
    
    func loadCompatibilityDefaults(_ defaults: CompatibilityDefaults) {
                 
        drivePowerSave = defaults.drivePowerSave
        viciiPowerSave = defaults.viciiPowerSave
        sidPowerSave = defaults.sidPowerSave
        
        ssCollisions = defaults.ssCollisions
        sbCollisions = defaults.sbCollisions
     }
    
    func loadCompatibilityUserDefaults() {
        
        let defaults = UserDefaults.standard

        drivePowerSave = defaults.bool(forKey: Keys.Com.drivePowerSave)
        viciiPowerSave = defaults.bool(forKey: Keys.Com.viciiPowerSave)
        sidPowerSave = defaults.bool(forKey: Keys.Com.sidPowerSave)
        
        ssCollisions = defaults.bool(forKey: Keys.Com.ssCollisions)
        sbCollisions = defaults.bool(forKey: Keys.Com.sbCollisions)
     }
    
    func saveCompatibilityUserDefaults() {
                
        let defaults = UserDefaults.standard
        
        defaults.set(drivePowerSave, forKey: Keys.Com.drivePowerSave)
        defaults.set(viciiPowerSave, forKey: Keys.Com.viciiPowerSave)
        defaults.set(sidPowerSave, forKey: Keys.Com.sidPowerSave)

        defaults.set(ssCollisions, forKey: Keys.Com.ssCollisions)
        defaults.set(sbCollisions, forKey: Keys.Com.sbCollisions)
    }
    
    //
    // Audio
    //
    
    func loadAudioDefaults(_ defaults: AudioDefaults) {
        
        c64.suspend()
        
        sidEngine = defaults.sidEngine.rawValue
        sidSampling = defaults.sidSampling.rawValue
        
        vol0 = defaults.vol0
        vol1 = defaults.vol1
        vol2 = defaults.vol2
        vol3 = defaults.vol3
        pan0 = defaults.pan0
        pan1 = defaults.pan1
        pan2 = defaults.pan2
        pan3 = defaults.pan3
        
        volL = defaults.volL
        volR = defaults.volR
        
        drive8Pan = defaults.drivePan[0]
        drive9Pan = defaults.drivePan[1]
        stepVolume = defaults.stepVolume
        insertVolume = defaults.insertVolume
        ejectVolume = defaults.ejectVolume
        
        c64.resume()
    }
    
    func loadAudioUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        c64.suspend()
        
        sidEngine = defaults.integer(forKey: Keys.Aud.sidEngine)
        sidSampling = defaults.integer(forKey: Keys.Aud.sidSampling)
        
        vol0 = defaults.integer(forKey: Keys.Aud.vol0)
        vol1 = defaults.integer(forKey: Keys.Aud.vol1)
        vol2 = defaults.integer(forKey: Keys.Aud.vol2)
        vol3 = defaults.integer(forKey: Keys.Aud.vol3)
        pan0 = defaults.integer(forKey: Keys.Aud.pan0)
        pan1 = defaults.integer(forKey: Keys.Aud.pan1)
        pan2 = defaults.integer(forKey: Keys.Aud.pan2)
        pan3 = defaults.integer(forKey: Keys.Aud.pan3)
        
        volL = defaults.integer(forKey: Keys.Aud.volL)
        volR = defaults.integer(forKey: Keys.Aud.volR)
        
        drive8Pan = defaults.integer(forKey: Keys.Aud.drive8Pan)
        drive9Pan = defaults.integer(forKey: Keys.Aud.drive9Pan)
        stepVolume = defaults.integer(forKey: Keys.Aud.stepVolume)
        insertVolume = defaults.integer(forKey: Keys.Aud.insertVolume)
        ejectVolume = defaults.integer(forKey: Keys.Aud.ejectVolume)

        c64.resume()
    }
    
    func saveAudioUserDefaults() {
        
        track()
        
        let defaults = UserDefaults.standard
        
        defaults.set(sidEngine, forKey: Keys.Aud.sidEngine)
        defaults.set(sidSampling, forKey: Keys.Aud.sidSampling)
        
        defaults.set(vol0, forKey: Keys.Aud.vol0)
        defaults.set(vol1, forKey: Keys.Aud.vol1)
        defaults.set(vol2, forKey: Keys.Aud.vol2)
        defaults.set(vol3, forKey: Keys.Aud.vol3)
        defaults.set(pan0, forKey: Keys.Aud.pan0)
        defaults.set(pan1, forKey: Keys.Aud.pan1)
        defaults.set(pan2, forKey: Keys.Aud.pan2)
        defaults.set(pan3, forKey: Keys.Aud.pan3)
        
        defaults.set(volL, forKey: Keys.Aud.volL)
        defaults.set(volR, forKey: Keys.Aud.volR)
        
        defaults.set(drive8Pan, forKey: Keys.Aud.drive8Pan)
        defaults.set(drive9Pan, forKey: Keys.Aud.drive9Pan)
        defaults.set(stepVolume, forKey: Keys.Aud.stepVolume)
        defaults.set(insertVolume, forKey: Keys.Aud.insertVolume)
        defaults.set(ejectVolume, forKey: Keys.Aud.ejectVolume)
    }
    
    //
    // Video
    //
    
    func loadColorDefaults(_ defaults: VideoDefaults) {
        
        c64.suspend()
        
        palette = defaults.palette.rawValue
        brightness = defaults.brightness
        contrast = defaults.contrast
        saturation = defaults.saturation
        
        c64.resume()
    }
    
    func loadGeometryDefaults(_ defaults: VideoDefaults) {
        
        hCenter = defaults.hCenter
        vCenter = defaults.vCenter
        hZoom = defaults.hZoom
        vZoom = defaults.vZoom
        
        renderer.canvas.updateTextureRect()
    }
    
    func loadShaderDefaults(_ defaults: VideoDefaults) {
        
        upscaler = defaults.upscaler
        
        blur = defaults.blur
        blurRadius = defaults.blurRadius
        
        bloom = defaults.bloom
        bloomRadiusR = defaults.bloomRadiusR
        bloomRadiusG = defaults.bloomRadiusG
        bloomRadiusB = defaults.bloomRadiusB
        bloomBrightness = defaults.bloomBrightness
        bloomWeight = defaults.bloomWeight
        dotMask = defaults.dotMask
        dotMaskBrightness = defaults.dotMaskBrightness
        scanlines = defaults.scanlines
        scanlineBrightness = defaults.scanlineBrightness
        scanlineWeight = defaults.scanlineWeight
        disalignment = defaults.disalignment
        disalignmentH = defaults.disalignmentH
        disalignment = defaults.disalignment
    }
    
    func loadVideoDefaults(_ defaults: VideoDefaults) {
        
        loadColorDefaults(defaults)
        loadGeometryDefaults(defaults)
        loadShaderDefaults(defaults)
    }
    
    func loadVideoUserDefaults() {
        
        let defaults = UserDefaults.standard
        
        c64.suspend()
        
        palette = defaults.integer(forKey: Keys.Vid.palette)
        brightness = defaults.integer(forKey: Keys.Vid.brightness)
        contrast = defaults.integer(forKey: Keys.Vid.contrast)
        saturation = defaults.integer(forKey: Keys.Vid.saturation)
        
        hCenter = defaults.float(forKey: Keys.Vid.hCenter)
        vCenter = defaults.float(forKey: Keys.Vid.vCenter)
        hZoom = defaults.float(forKey: Keys.Vid.hZoom)
        vZoom = defaults.float(forKey: Keys.Vid.vZoom)
        
        upscaler = defaults.integer(forKey: Keys.Vid.upscaler)
        
        bloom = defaults.integer(forKey: Keys.Vid.bloom)
        bloomRadiusR = defaults.float(forKey: Keys.Vid.bloomRadiusR)
        bloomRadiusG = defaults.float(forKey: Keys.Vid.bloomRadiusG)
        bloomRadiusB = defaults.float(forKey: Keys.Vid.bloomRadiusB)
        bloomBrightness = defaults.float(forKey: Keys.Vid.bloomBrightness)
        bloomWeight = defaults.float(forKey: Keys.Vid.bloomWeight)
        dotMask = defaults.integer(forKey: Keys.Vid.dotMask)
        dotMaskBrightness = defaults.float(forKey: Keys.Vid.dotMaskBrightness)
        scanlines = defaults.integer(forKey: Keys.Vid.scanlines)
        scanlineBrightness = defaults.float(forKey: Keys.Vid.scanlineBrightness)
        scanlineWeight = defaults.float(forKey: Keys.Vid.scanlineWeight)
        disalignment = Int32(defaults.integer(forKey: Keys.Vid.disalignment))
        disalignmentH = defaults.float(forKey: Keys.Vid.disalignmentH)
        disalignmentV = defaults.float(forKey: Keys.Vid.disalignmentV)
        
        renderer.canvas.updateTextureRect()
        
        c64.resume()
    }
    
    func saveVideoUserDefaults() {
        
        track()
        
        let defaults = UserDefaults.standard
        
        defaults.set(palette, forKey: Keys.Vid.palette)
        defaults.set(brightness, forKey: Keys.Vid.brightness)
        defaults.set(contrast, forKey: Keys.Vid.contrast)
        defaults.set(saturation, forKey: Keys.Vid.saturation)
        
        defaults.set(hCenter, forKey: Keys.Vid.hCenter)
        defaults.set(vCenter, forKey: Keys.Vid.vCenter)
        defaults.set(hZoom, forKey: Keys.Vid.hZoom)
        defaults.set(vZoom, forKey: Keys.Vid.vZoom)
        
        defaults.set(upscaler, forKey: Keys.Vid.upscaler)
        
        defaults.set(bloom, forKey: Keys.Vid.bloom)
        defaults.set(bloomRadiusR, forKey: Keys.Vid.bloomRadiusR)
        defaults.set(bloomRadiusG, forKey: Keys.Vid.bloomRadiusG)
        defaults.set(bloomRadiusB, forKey: Keys.Vid.bloomRadiusB)
        defaults.set(bloomBrightness, forKey: Keys.Vid.bloomBrightness)
        defaults.set(bloomWeight, forKey: Keys.Vid.bloomWeight)
        defaults.set(dotMask, forKey: Keys.Vid.dotMask)
        defaults.set(dotMaskBrightness, forKey: Keys.Vid.dotMaskBrightness)
        defaults.set(scanlines, forKey: Keys.Vid.scanlines)
        defaults.set(scanlineBrightness, forKey: Keys.Vid.scanlineBrightness)
        defaults.set(scanlineWeight, forKey: Keys.Vid.scanlineWeight)
        defaults.set(disalignment, forKey: Keys.Vid.disalignment)
        defaults.set(disalignmentH, forKey: Keys.Vid.disalignmentH)
        defaults.set(disalignmentV, forKey: Keys.Vid.disalignmentV)
    }
}
