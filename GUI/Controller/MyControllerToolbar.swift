// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

struct InputDevice {
    static let none = -1
    static let keyset1 = 0
    static let keyset2 = 1
    static let mouse = 2
    static let joystick1 = 3
    static let joystick2 = 4
}

extension MyController {

    func connectPort1(device: Int) {

        let cpd: ControlPortDevice =
            device == InputDevice.none ? CPD_NONE :
                device == InputDevice.mouse ? CPD_MOUSE : CPD_JOYSTICK

        amiga.controlPort1.connect(cpd)
        inputDevice1 = device
        UserDefaults.standard.set(device, forKey: Keys.inputDevice1)
    }

    func connectPort2(device: Int) {

        let cpd: ControlPortDevice =
            device == InputDevice.none ? CPD_NONE :
                device == InputDevice.mouse ? CPD_MOUSE : CPD_JOYSTICK

        amiga.controlPort2.connect(cpd)
        inputDevice2 = device
        UserDefaults.standard.set(device, forKey: Keys.inputDevice2)
    }

    /*
    func connectPort(port: Int, device: Int) {

        assert(port == 1 || port == 2)

        let cpd: ControlPortDevice =
            device == InputDevice.none ? CPD_NONE :
                device == InputDevice.mouse ? CPD_MOUSE : CPD_JOYSTICK

        port == 1 ? amiga.controlPort1.connect(cpd) : amiga.controlPort2.connect(cpd)
    }
    */

    @IBAction func port1Action(_ sender: NSPopUpButton) {
        
        setPort1(sender.selectedTag())
    }
 
    func setPort1(_ value: Int) {

        let newDevice1 = value
        var newDevice2 = inputDevice2

        // Avoid double mappings
        if newDevice1 == newDevice2 { newDevice2 = InputDevice.none }

        // Update ports
        if newDevice1 != inputDevice1 { connectPort1(device: newDevice1) }
        if newDevice2 != inputDevice2 { connectPort2(device: newDevice2) }

        toolbar.validateVisibleItems()
    }
    
    @IBAction func port2Action(_ sender: NSPopUpButton) {
        
        setPort2(sender.selectedTag())
    }
    
    func setPort2(_ value: Int) {
        
        var newDevice1 = inputDevice1
        let newDevice2 = value

        // Avoid double mappings
        if newDevice1 == newDevice2 { newDevice1 = InputDevice.none }

        // Update ports
        if newDevice1 != inputDevice1 { connectPort1(device: newDevice1) }
        if newDevice2 != inputDevice2 { connectPort2(device: newDevice2) }

        toolbar.validateVisibleItems()
    }
    
    @IBAction func preferencesAction(_ sender: Any!) {
        
        openPreferences()
    }

    @IBAction func inspectAction(_ sender: NSSegmentedControl) {

        switch sender.selectedSegment {

        case 0: inspectorAction(sender)
        case 1: monitorAction(sender)

        default: assert(false)
        }
    }
    
    @IBAction func snapshotAction(_ sender: NSSegmentedControl) {
        
        switch sender.selectedSegment {
        
        case 0: // Rewind

            track("Rewind")
            if !amiga.restoreLatestAutoSnapshot() {
                NSSound.beep()
            }
        
        case 1: // Take

            track("Snap")
            amiga.takeUserSnapshot()
            
        case 2: // Restore
            
            track("Restore")
            if !amiga.restoreLatestUserSnapshot() {
                NSSound.beep()
            }

        case 3: // Browse
            
            track("Browse")
            let nibName = NSNib.Name("SnapshotDialog")
            let controller = SnapshotDialog.init(windowNibName: nibName)
            controller.showSheet()

        default:
            assert(false)
        }
    }
    
    @IBAction func keyboardAction(_ sender: Any!) {
        
        // Open the virtual keyboard as a sheet
        virtualKeyboardSheet = VirtualKeyboardController.make()
        virtualKeyboardSheet?.showSheet()
    }

    @IBAction func snapshotsAction(_ sender: Any!) {
        
        let nibName = NSNib.Name("SnapshotDialog")
        let controller = SnapshotDialog.init(windowNibName: nibName)
        controller.showSheet()
    }

    @IBAction func restoreLatestAutoSnapshotAction(_ sender: Any!) {
        
        if amiga.restoreLatestAutoSnapshot() {
            amiga.deleteAutoSnapshot(0)
            renderer.snapToFront()
        }
    }

    @IBAction func restoreLatestUserSnapshotAction(_ sender: Any!) {
        
        if amiga.restoreLatestUserSnapshot() {
            renderer.snapToFront()
        }
    }
    
    @IBAction func printDocument(_ sender: Any!) {
    
        guard let window = mydocument?.windowForSheet else { return }

        // Printing properties
        let printInfo = mydocument!.printInfo
        printInfo.horizontalPagination = .fit
        printInfo.isHorizontallyCentered = true
        printInfo.verticalPagination = .fit
        printInfo.isVerticallyCentered = true
        printInfo.orientation = .landscape
        printInfo.leftMargin = 32.0
        printInfo.rightMargin = 32.0
        printInfo.topMargin = 32.0
        printInfo.bottomMargin = 32.0

        // Image view
        let paper = printInfo.paperSize
        let image = renderer.screenshot()
        let printRect = NSRect.init(x: 0, y: 0, width: paper.width, height: paper.height)
        let imageView = NSImageView.init(frame: printRect)
        imageView.image = image
        imageView.imageScaling = .scaleAxesIndependently
    
        // Print image
        let printOperation = NSPrintOperation.init(view: imageView, printInfo: printInfo)
        printOperation.runModal(for: window,
                                delegate: nil,
                                didRun: nil,
                                contextInfo: nil)
    }
    
}
