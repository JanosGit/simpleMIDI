//
//  main.cpp
//  sendSomeMIDI
//
//  Created by Janos Buttgereit on 25.05.17.
//  Copyright © 2017 Janos Buttgereit. All rights reserved.
//

#include <iostream>
#include <iomanip>
#include "../../simpleMIDI.h"


// First of all, create a derived class and fill all receive-methods with your own code.
// Here we will just print out any received MIDI messages to the command line

class MyDerivedMIDIClass : public SimpleMIDI {
    
    // override all four "received..." member functions. Make sure to mark all of them with the override keyword
    public:
    void receivedNote (uint8_t note, uint8_t velocity, bool noteOn) override {
        if (noteOn) {
            std::cout << "Received note on  0x"
                      << std::hex << std::setfill('0') << std::setw(2) << (int)note
                      << " with velocity 0x"
                      << std::hex << std::setfill('0') << std::setw(2) << (int)velocity
                      << std::endl;
        }
        else {
            std::cout << "Received note off 0x"
            << std::hex << std::setfill('0') << std::setw(2) << (int)note
            << " with velocity 0x"
            << std::hex << std::setfill('0') << std::setw(2) << (int)velocity
            << std::endl;

        }
    };
        
    void receivedControlChange (uint8_t control, uint8_t value) override {
        std::cout << "Received control change 0x"
                  << std::hex << std::setfill('0') << std::setw(2) << (int)control
                  << " with value 0x"
                  << std::hex << std::setfill('0') << std::setw(2) << (int)value
                  << std::endl;
    };
            
    void receivedProgrammChange (uint8_t programm) override {
        std::cout << "Received programm change to programm " << (int)programm << std::endl;
    };
            
    void receivedSysEx (uint8_t *sysExBuffer, uint8_t length) override {
        std::cout << "Received SysEx with content";
        for (int i = 0; i < length; i++) {
            std::cout << " 0x" << std::hex << std::setfill('0') << std::setw(2) << (int)sysExBuffer[i];
        }
        std::cout << std::endl;
    };
};


int main() {
    
    // The class encapsulating all MIDI action
    MyDerivedMIDIClass midiInterface;
    
    // Seach for connected
    const std::vector<MIDIDeviceInfo> connectedDevices = searchMIDIDevices();
    
    // How many MIDI interfaces are available? Print all names!
    const int connectedDevicesCount = connectedDevices.size();
    std::cout << "Detected devices: " << connectedDevicesCount << std::endl;
    for (int i = 0; i < connectedDevicesCount; i++){
        std::cout << "Device " << i << ": " << deviceNameString (&connectedDevices[i]) << std::endl;
    }
    
    // Ask the user to select a device
    std::cout << "\nChose device by index" << std::endl;
    int selectedDevice = connectedDevicesCount;
    while (selectedDevice > (connectedDevicesCount - 1)) {
        std::cin >> selectedDevice;
        if((selectedDevice >= connectedDevicesCount) || (selectedDevice < 0)){
            std::cout << "No device with index " << selectedDevice << " available" << std::endl;
            selectedDevice = connectedDevicesCount;
        };
    };
    // Tell the midiInterface instance which device it should represent from now on
    midiInterface.selectDevice (&connectedDevices[selectedDevice]);
    std::cout << "Connected to device " << deviceNameString (&connectedDevices[selectedDevice]) << std::endl;
    
    // Set a MIDI Channel to use for all following send-commands
    midiInterface.setSendChannel (simpleMIDI::Channel10);
    
    //Send some random Data
    midiInterface.sendNote (0x23, 0x34, true);
    midiInterface.sendControlChange (0x12, 0x36);
    midiInterface.sendProgramChange (8);
    uint8_t testSysEx[12] = {simpleMIDI::SysExBegin, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, simpleMIDI::SysExEnd};
    midiInterface.sendSysEx (testSysEx, 12);
    
    //wait for incomming MIDI data or the users exit command
    bool running = true;
    while (running) {
        std::string exitCmd;
        std::cin >> exitCmd;
        if(exitCmd.compare("x") == 0){
            running = false;
        }
    }
}
