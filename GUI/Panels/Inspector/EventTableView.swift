// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class EventTableView: NSTableView {

    @IBOutlet weak var inspector: Inspector!

    var c64: EmulatorProxy { return inspector.c64 }

    var slotInfo = [vc64.EventSlotInfo?](repeating: nil, count: vc64.EventSlot._COUNT.rawValue)

    override func awakeFromNib() {

        delegate = self
        dataSource = self
        target = self
    }

    private func cache() {
        for row in 0 ..< vc64.EventSlot._COUNT.rawValue {
            slotInfo[row] = c64.getEventSlotInfo(row)
        }
    }

    func refresh(count: Int = 0, full: Bool = false) {

        cache()
        reloadData()
    }
}

extension EventTableView: NSTableViewDataSource {

    func numberOfRows(in tableView: NSTableView) -> Int {

        return vc64.EventSlot._COUNT.rawValue
    }

    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {

        guard let info = slotInfo[row] else { return nil }

        let willTrigger = (info.trigger != INT64_MAX)

        switch tableColumn?.identifier.rawValue {

        case "slot":
            return info.slot.description
        case "event":
            return String(cString: info.eventName)
        case "trigger":
            if willTrigger {
                return String(format: "%lld", info.trigger)
            } else {
                return "none"
            }
        case "frame":
            if !willTrigger {
                return ""
            } else if info.frameRel < 0 {
                return "previous"
            } else if info.frameRel == 0 {
                return "current"
            } else {
                return "current + \(info.frameRel)"
            }
        case "vpos":
            if willTrigger {
                if inspector.hex {
                    return String(format: "%02X", info.vpos)
                } else {
                    return info.vpos
                }
            } else {
                return ""
            }
        case "hpos":
            if willTrigger {
                if inspector.hex {
                    return String(format: "%02X", info.hpos)
                } else {
                    return info.hpos
                }
            } else {
                return ""
            }
        case "remark":
            if !willTrigger {
                return ""
            } else if info.triggerRel == 0 {
                return "due immediately"
            } else {
                return "due in \(info.triggerRel) cycles"
            }
        default:        return "???"
        }
    }
}

extension EventTableView: NSTableViewDelegate {

    func tableView(_ tableView: NSTableView, willDisplayCell cell: Any, for tableColumn: NSTableColumn?, row: Int) {

        if let cell = cell as? NSTextFieldCell {
            if tableColumn?.identifier.rawValue != "slot" {
                if c64.getEventSlotInfo(row).trigger == INT64_MAX {
                    cell.textColor = .secondaryLabelColor
                    return
                }
            }

            cell.textColor = .textColor
        }
    }
}
