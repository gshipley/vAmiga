// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

class TraceTableView: NSTableView {

    @IBOutlet weak var inspector: Inspector!

    // Data caches
    var instrInRow: [Int: DisassembledInstr] = [:]
    var numRows = 0

    override func awakeFromNib() {
        
        dataSource = self
        target = self
    }

    func cache() {

        numRows = amiga?.cpu.loggedInstructions() ?? 0

        for i in 0 ..< numRows {
            instrInRow[i] = amiga?.cpu.getLoggedInstrInfo(i)
        }
    }

    func refresh(count: Int) {

        // Update display cache
        cache()

        // Refresh display with cached values
        reloadData()        
    }
}

extension TraceTableView: NSTableViewDataSource {
    
    func numberOfRows(in tableView: NSTableView) -> Int {

        return numRows
    }
    
    func tableView(_ tableView: NSTableView, objectValueFor tableColumn: NSTableColumn?, row: Int) -> Any? {
        
        if var info = instrInRow[row] {
            
            switch tableColumn?.identifier.rawValue {
                
            case "addr":
                return String(cString: &info.addr.0)
            case "flags":
                return String(cString: &info.sr.0)
            case "instr":
                return String(cString: &info.instr.0)
            default:
                return "???"
            }
        }
        return "??"
    }
}
