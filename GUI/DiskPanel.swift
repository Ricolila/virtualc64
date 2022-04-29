// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

extension NSTextStorage {

    var fullRange: NSRange { return NSRange(location: 0, length: self.length) }
    
    func addAttr(_ name: NSAttributedString.Key, value: Any, range: NSRange?) {
        
        if let intersectedRange = range?.intersection(fullRange) {
            self.addAttribute(name, value: value, range: intersectedRange)
        }
    }

    func remAttr(_ name: NSAttributedString.Key, range: NSRange?) {
        
        if let intersectedRange = range?.intersection(fullRange) {
            self.removeAttribute(name, range: intersectedRange)
        }
    }
}

extension Inspector {
    
    var halftrack: Halftrack? {
        if isHalftrackNumber(selectedHalftrack) {
            return Halftrack(selectedHalftrack)
        } else {
            return nil
        }
    }
    
    var sector: Sector? {
        if isSectorNumber(selectedSector) {
            return Sector(selectedSector)
        } else {
            return nil
        }
    }
    
    func analyzeDisk() {
    
        if let disk = drive.disk {

            track("Analyzing disk")
            analyzer = DiskAnalyzerProxy(disk: disk)

        } else {
            track("Can't analyze disk (no disk present)")
            analyzer = nil
        }
    }
    
    func refreshDisk(count: Int = 0, full: Bool = false) {
        
        if full {
                        
            if analyzer == nil {
                selectedHalftrack = -1
                selectedSector = -1
                drvNoDiskText.isHidden = false
            } else {
                drvNoDiskText.isHidden = true
            }
                
            // Refresh sub views
            drvDiskDataView.hex = hex
            drvTrackTableView.refresh(count: count, full: full)
            drvSectorTableView.refresh(count: count, full: full)
            drvDiskDataView.refresh(count: count, full: full)
        }
    }
    
    func setSelectedDrive(_ id: DriveID) {
        
        if selectedDrive == id { return }
        selectedDrive = id
        drvDriveSel.selectSegment(withTag: id == .DRIVE8 ? 0 : 1)
        
        selectedHalftrack = -1
        selectedSector = -1
        
        // Reanalyze the disk
        analyzeDisk()
        
        // Force sub views to update
        drvSectorTableView.isDirty = true
        drvDiskDataView.dataIsDirty = true
        drvDiskDataView.sectionMarksAreDirty = true
        fullRefresh()
    }
    
    func setRawGcr(_ value: Bool) {
    
        if rawGcr == value { return }
        rawGcr = value
        drvGcrBytesSel.selectSegment(withTag: value ? 0 : 1)

        // Force sub views to update
        drvDiskDataView.dataIsDirty = true
        drvDiskDataView.sectionMarksAreDirty = true
        fullRefresh()
    }
    
    func setSelectedHalftrack(_ nr: Int) {

        if selectedHalftrack == nr { return }
        selectedHalftrack = nr
        
        if halftrack != nil && analyzer != nil {
                                                
            // Warn if this track contains errors
            let trackIsValid = analyzer!.numErrors(halftrack!) == 0
            drvWarningText.isHidden = trackIsValid
            drvWarningButton.isHidden = trackIsValid
            
            // Force sub views to update
            drvSectorTableView.isDirty = true
            drvDiskDataView.dataIsDirty = true
            drvDiskDataView.sectionMarksAreDirty = true
        }
        
        fullRefresh()
    }

    func setSelectedTrack(_ nr: Int) {

        setSelectedHalftrack(2 * nr - 1)
    }

    func setSelectedSector(_ nr: Int) {
            
        if selectedSector == nr { return }
        selectedSector = nr

        // Blend GCR / Byte selector in or out
        drvGcrBytesSel.isHidden = selectedSector < 0

        // Force the data view to update
        if rawGcr {
            drvDiskDataView.sectionMarksAreDirty = true
        } else {
            drvDiskDataView.dataIsDirty = true
        }
        fullRefresh()
    }

    @IBAction func drvDriveAction(_ sender: NSSegmentedControl!) {
        
        setSelectedDrive(sender.selectedSegment == 0 ? .DRIVE8 : .DRIVE9)
    }

    @IBAction func drvGcrBytesAction(_ sender: NSSegmentedControl!) {
        
        setRawGcr(sender.selectedSegment == 0)
    }
    
    @IBAction func drvHalftracksAction(_ sender: NSButton!) {
        
        track("Halftracks \(sender.state)")
        drvTrackTableView.showHalftracks = sender.state == .on
        fullRefresh()
    }
    
    @IBAction func drvWarningAction(_ sender: NSButton!) {
        
        track()
        
        let nibName = NSNib.Name("DiskErrors")
        let controller = DiskErrorController(windowNibName: nibName)
        controller.parent = self
        
        window!.beginSheet(controller.window!, completionHandler: { result in
            if result == NSApplication.ModalResponse.OK {
                controller.cleanup()
            }
        })
    }    
}
