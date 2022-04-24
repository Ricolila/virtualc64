// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

import Cocoa

var myAppDelegate: MyAppDelegate { return NSApp.delegate as! MyAppDelegate }

@NSApplicationMain
@objc public class MyAppDelegate: NSObject, NSApplicationDelegate {
           
    @IBOutlet weak var drive8Menu: NSMenuItem!
    @IBOutlet weak var drive9Menu: NSMenuItem!

    // Replace the old document controller by instantiating a custom controller
    let myDocumentController = MyDocumentController()
    
    // Preferences
    var pref: Preferences!
    var prefController: PreferencesController?
    
    // Information provider for connected HID devices
    var database = DeviceDatabase()
    
    // The list of recently inserted media URLs
    var recentlyInsertedDiskURLs: [URL] = []
    
    // The list of recently exported media URLs
    var recentlyExportedDisk8URLs: [URL] = []
    var recentlyExportedDisk9URLs: [URL] = []
    var recentlyInsertedTapeURLs: [URL] = []
    var recentlyAttachedCartridgeURLs: [URL] = []

    override init() {
        
        super.init()
        pref = Preferences()
    }
    
    public func applicationDidFinishLaunching(_ aNotification: Notification) {
        
        track()        
    }
    
    public func applicationWillTerminate(_ aNotification: Notification) {
        
        track()
    }
    
    //
    // Handling the lists of recently used URLs
    //
    
    func noteRecentlyUsedURL(_ url: URL, to list: inout [URL], size: Int) {
        
        if !list.contains(url) {
            
            // Shorten the list if it is too large
            if list.count == size { list.remove(at: size - 1) }
            
            // Add new item at the beginning
            list.insert(url, at: 0)
        }
    }
    
    func getRecentlyUsedURL(_ pos: Int, from list: [URL]) -> URL? {
        return (pos < list.count) ? list[pos] : nil
    }
    
    func noteNewRecentlyInsertedDiskURL(_ url: URL) {
        noteRecentlyUsedURL(url, to: &recentlyInsertedDiskURLs, size: 10)
    }
    
    func getRecentlyInsertedDiskURL(_ pos: Int) -> URL? {
        return getRecentlyUsedURL(pos, from: recentlyInsertedDiskURLs)
    }
    
    func noteNewRecentlyExportedDiskURL(_ url: URL, drive: DriveID) {
        
        precondition(drive == .DRIVE8 || drive == .DRIVE9)
        
        if drive == .DRIVE8 {
            noteRecentlyUsedURL(url, to: &recentlyExportedDisk8URLs, size: 1)
        } else {
            noteRecentlyUsedURL(url, to: &recentlyExportedDisk9URLs, size: 1)
        }
    }
    
    func getRecentlyExportedDiskURL(_ pos: Int, drive: DriveID) -> URL? {
        
        precondition(drive == .DRIVE8 || drive == .DRIVE9)

        if drive == .DRIVE8 {
            return getRecentlyUsedURL(pos, from: recentlyExportedDisk8URLs)
        } else {
            return getRecentlyUsedURL(pos, from: recentlyExportedDisk9URLs)
        }
    }
    
    func clearRecentlyExportedDiskURLs(drive: DriveID) {
        
        precondition(drive == .DRIVE8 || drive == .DRIVE9)

        if drive == .DRIVE8 {
            recentlyExportedDisk8URLs = []
        } else {
            recentlyExportedDisk9URLs = []
        }
    }
    
    func noteNewRecentlyInsertedTapeURL(_ url: URL) {
        noteRecentlyUsedURL(url, to: &recentlyInsertedTapeURLs, size: 10)
    }
    
    func getRecentlyInsertedTapeURL(_ pos: Int) -> URL? {
        return getRecentlyUsedURL(pos, from: recentlyInsertedTapeURLs)
    }
    
    func noteNewRecentlyAtachedCartridgeURL(_ url: URL) {
        noteRecentlyUsedURL(url, to: &recentlyAttachedCartridgeURLs, size: 10)
    }
    
    func getRecentlyAtachedCartridgeURL(_ pos: Int) -> URL? {
        return getRecentlyUsedURL(pos, from: recentlyAttachedCartridgeURLs)
    }
    
    func noteNewRecentlyUsedURL(_ url: URL) {
        
        switch url.pathExtension.uppercased() {
            
        case "D64", "T64", "G64", "PRG", "P00":
            noteNewRecentlyInsertedDiskURL(url)
            
        case "TAP":
            noteNewRecentlyInsertedTapeURL(url)
            
        case "CRT":
            noteNewRecentlyAtachedCartridgeURL(url)
            
        default:
            break
        }
    }
}

//
// Personal delegation methods
//

extension MyAppDelegate {
    
    var documents: [MyDocument] {
        return NSDocumentController.shared.documents as? [MyDocument] ?? []
    }
    var windows: [NSWindow] {
        return documents.compactMap({ $0.windowForSheet })
    }
    var controllers: [MyController] {
        return documents.compactMap({ $0.windowForSheet?.windowController as? MyController })
    }
    var proxies: [C64Proxy] {
        return documents.map({ $0.c64 })
    }
    
    func windowDidBecomeMain(_ window: NSWindow) {
        
        for c in controllers {
            
            if c.window == window {
                
                // Start playback
                if !c.macAudio!.isRunning {
                    c.macAudio!.startPlayback()
                    if !c.c64.warpMode { c.c64.sid.rampUpFromZero() }
                }
                
                // Update the visibility of all drive menus
                c.hideOrShowDriveMenus()
                
            } else {
                
                // Stop playback
                if c.macAudio!.isRunning {
                    c.macAudio!.stopPlayback()
                    c.c64.sid.rampDown()
                }
            }
        }
    }
    
    // Callen when a HID device has been added
    func deviceAdded() {
        prefController?.refresh()
    }
    
    // Callen when a HID device has been removed
    func deviceRemoved() {
        prefController?.refresh()
    }

    // Callen when a HID device has been pulled
    func devicePulled(events: [GamePadAction]) {
        prefController?.refreshDeviceEvents(events: events)
    }
}
