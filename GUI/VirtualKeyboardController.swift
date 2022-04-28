// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class VirtualKeyboardController: DialogController {

    var keyboard: KeyboardProxy { return c64.keyboard }
    
    @IBOutlet weak var caseSelector: NSSegmentedControl!

    // Array holding a reference to the view of each key
    var keyView = Array(repeating: nil as NSButton?, count: 66)

    // Array holding a reference to the image of each key
    var keyImage = Array(repeating: nil as NSImage?, count: 66)

    // Image cache for keys that are currently pressed
    var pressedKeyImage = Array(repeating: nil as NSImage?, count: 66)

    // Currently set key modifiers
    var modifiers: Modifier = []
        
    // Indicates if lower case or upper case characters should be displayed
    var lowercase: Bool { return caseSelector.selectedSegment == 1 }
    
    /* Indicates if the window should be closed when a key is pressed. If the
     * virtual keyboard is opened as a sheet, this variable is set to true. If
     * it is opened as a seperate window, it is set to false.
     */
    var autoClose = true

    /*
    static func make(parent: MyController) -> VirtualKeyboardController? {

        return make(parent: parent, nibName: NSNib.Name("VirtualKeyboard"))
    }
    */

    func showSheet() {
        
        autoClose = true
        super.showSheet()
    }
    
    func showWindow() {
        
        autoClose = false
        super.showWindow(self)
    }
    
    override func windowDidLoad() {
        
        track()
        updateImageCache()
        refresh()
    }

    override func sheetWillShow() {
    
        track()
        
        // Collect references to all buttons
        for tag in 0 ... 65 {
            keyView[tag] = window!.contentView!.viewWithTag(tag) as? NSButton
        }
    }

    override func sheetDidShow() {

        track()
        refresh()
    }

    func windowDidBecomeMain(_ notification: Notification) {
        
        track()
        refresh()
    }

    func refresh() {
                
        // Only proceed if the keyboard is visible
        if window == nil || !window!.isVisible { return }
        
        var newModifiers: Modifier = []
        
        if keyboard.leftShiftIsPressed() { newModifiers.insert(.shift) }
        if keyboard.rightShiftIsPressed() { newModifiers.insert(.shift) }
        if keyboard.shiftLockIsPressed() { newModifiers.insert(.shift) }
        if keyboard.controlIsPressed() { newModifiers.insert(.control) }
        if keyboard.commodoreIsPressed() { newModifiers.insert(.commodore) }
        if lowercase { newModifiers.insert(.lowercase) }
                
        // Update images if the modifier flags have changed
        if modifiers != newModifiers {
            modifiers = newModifiers
            updateImageCache()
        }
        
        for nr in 0 ... 65 {
                        
            if c64.keyboard.keyIsPressed(nr) {
                keyView[nr]!.image = pressedKeyImage[nr]
            } else {
                keyView[nr]!.image = keyImage[nr]
            }
        }
    }
    
    func updateImageCache() {
        
        for nr in 0 ... 65 {
            
            let keycap = C64Key.lookupKeycap(for: nr, modifier: modifiers)!
            keyImage[nr] = keycap.image
            pressedKeyImage[nr] = keycap.image?.copy() as? NSImage
            pressedKeyImage[nr]?.pressed()
        }
    }
        
    func pressKey(nr: Int) {
        
        track()
        
        c64.keyboard.pressKey(nr)
        refresh()

        if autoClose {
            c64.keyboard.scheduleKeyReleaseAll(2)
            cancelAction(self)
        } else {
            c64.keyboard.scheduleKeyRelease(nr, delay: 2)
        }
    }
    
    func holdKey(nr: Int) {
        
        c64.keyboard.toggleKey(nr)
        refresh()
    }
    
    override func mouseDown(with event: NSEvent) {
        
        track()
        
        // If opened as a sheet, close if the user clicked inside unsued area
        if autoClose { cancelAction(self) }
    }

    @IBAction func pressVirtualKey(_ sender: NSButton!) {
        
        // Not used at the moment
    }

    @IBAction func caseSelectorAction(_ sender: NSSegmentedControl!) {
        
        // let segment = sender.selectedSegment
        // track("segment = \(segment)")
        refresh()
    }
}

// Subclass of NSButton for the keys in the virtual keyboard
class KeycapButton: NSButton {
    
    override func mouseDown(with event: NSEvent) {
        
        if let controller = window?.delegate as? VirtualKeyboardController {
            
            controller.pressKey(nr: self.tag)
        }
    }

    override func mouseUp(with event: NSEvent) {
        
        if let controller = window?.delegate as? VirtualKeyboardController {
            
            track()
            controller.refresh()
        }
    }

    override func rightMouseDown(with event: NSEvent) {
    
        if let controller = window?.delegate as? VirtualKeyboardController {
            
            controller.holdKey(nr: self.tag)
        }
    }
}
