//
//  main.cpp
//  sendSomeMIDI
//
//  Created by Janos Buttgereit on 25.05.17.
//  Copyright © 2017 Janos Buttgereit. All rights reserved.
//

#include <iostream>
#include <iomanip>
#include "../../../../simpleMIDI.h"


// First of all, create a derived class and fill all receive-methods with your own code.
// Here we will just print out any received MIDI messages to the command line

class MyDerivedMIDIClass : public SimpleMIDI::PlatformSpecificImplementation {
    
public:
    // The constructor of SimpleMIDI is called with a pointer to the Hardware ressource to be used for the MIDI connection passed as an argument. You might set this to a fixed ressource or pass a value from the constructor of your derived class to the constructor of the SimpleMIDI base class as done here
    MyDerivedMIDIClass (SimpleMIDI::HardwareResource &h) : SimpleMIDI::PlatformSpecificImplementation (h) {
        // some constructor stuff goes here, if needed
    };
    
    
    // override all four "received..." member functions. Make sure to mark all of them with the override keyword
    void receivedNote (uint8_t note, uint8_t velocity, bool noteOn) override {
        if (noteOn) {
            std::cout << "Received note on " << (int)note
            << " with velocity " << (int)velocity
            << " from MIDI channel " << (getMostRecentSourceChannel() + 1)
            << std::endl;
        }
        else {
            std::cout << "Received note off " << (int)note
            << " with velocity " << (int)velocity
            << " from MIDI channel " << (getMostRecentSourceChannel() + 1)
            << std::endl;
            
        }
    };

    void receivedAftertouch (uint8_t note, uint8_t velocity) override {
        std::cout << "Received ";
        if (note == MonophonicAftertouch) {
            std::cout << "monophonic afterouch";
        }
        else {
            std::cout <<"polyphonic aftertouch from note " << (int)note;
        }
        std::cout << " with velocity " << (int)velocity << std::endl;
    }

    void receivedControlChange (uint8_t control, uint8_t value) override {
        std::cout << "Received control change from control "
                  << (int)control << " with value " << (int)value << std::endl;
    }

    void receivedProgramChange (uint8_t program) override {
        std::cout << "Received program change to program " << (int)program << std::endl;
    }

    // will print a sysEx interpreted as chars
    void receivedSysEx (const uint8_t *sysExBuffer, const uint16_t length) override {
        std::string sysExAsString ((char*)(sysExBuffer + 1), length - 2);
        std::cout << "Received Sys Ex with Content \"" << sysExAsString << "\"" << std::endl;
    }

    void receivedMIDITimecodeQuarterFrame (uint8_t quarterFrame) override {
        lcd.clear();
        lcd.print ("Timecode quarter");
        lcd.print ("frame #");
        lcd.print (quarterFrame);
    }

    void receivedMIDIClockTick() override {
        // just toggle the LED state
        digitalWrite (CLOCK_LED, !digitalRead (CLOCK_LED));
    }

    void receivedSongSelect (uint8_t selectedSong) override {
        lcd.clear();
        lcd.print ("Song select");
        lcd.setCursor (0, 1);
        lcd.print ("Song #");
        lcd.print (selectedSong);
    }

    void receivedMIDIStart() override {
        lcd.clear();
        lcd.print ("Start");
    }

    void receivedMIDIStop() override {
        lcd.clear();
        lcd.print ("Stop");
    }

    void receivedMIDIContinue() override {
        lcd.clear();
        lcd.print ("Continue");
    }

    void receivedSongPositionPointer (uint16_t positionInBeats) override {
        lcd.clear();
        lcd.print ("Songposition ptr");
        lcd.print ("Position: ");
        lcd.print (positionInBeats);
    }

    void receivedTuneRequest() override {
        lcd.clear();
        lcd.print ("Tune request");
    }

    void receivedActiveSense() override {
        // just toggle the led
        digitalWrite (ACTIVE_SENSE_LED, !digitalRead (ACTIVE_SENSE_LED));
    }

    void receivedMIDIReset() override {
        lcd.print("Reset ");
    }
};


int main() {
    
    
    // Seach for connected devices
    std::vector<SimpleMIDI::HardwareResource> connectedDevices = searchMIDIDevices();
    
    
    
    // How many MIDI interfaces are available? Print all names!
    const int connectedDevicesCount = connectedDevices.size();
    std::cout << "Detected devices: " << connectedDevicesCount << std::endl;
    for (int i = 0; i < connectedDevicesCount; i++){
        std::cout << "Device " << i << ": " << connectedDevices[i].deviceName << std::endl;
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
    
    
    
    // The class encapsulating all MIDI action
    MyDerivedMIDIClass midiInterface (connectedDevices[selectedDevice]);
    
    // Tell the midiInterface instance which device it should represent from now on
    // midiInterface.selectDevice (&connectedDevices[selectedDevice]);
    std::cout << "Connected to device " << connectedDevices[selectedDevice].deviceName << std::endl;
    
    
    
    // Set a MIDI channel to use for all following send-commands
    midiInterface.setSendChannel (SimpleMIDI::Channel10);
    // Listen to all incomming MIDI channels
    midiInterface.setReceiveChannel(SimpleMIDI::ChannelAny);
    

    
    //Send some random Data
    midiInterface.sendNote (0x23, 0x34, true);
    midiInterface.sendControlChange (0x12, 0x36);
    midiInterface.sendProgramChange (8);
    uint8_t testSysEx[12] = {SimpleMIDI::SysExBegin, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, SimpleMIDI::SysExEnd};
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

