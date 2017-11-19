//
//  main.cpp
//  sendSomeMIDI
//
//  Created by Janos Buttgereit on 25.05.17.
//  Copyright © 2017 Janos Buttgereit. All rights reserved.
//

#include <iostream>
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
    void receivedSysEx (const char *sysExBuffer, const uint16_t length) override {
        std::string sysExAsString (sysExBuffer + 1, length - 2);
        std::cout << "Received Sys Ex with Content \"" << sysExAsString << "\"" << std::endl;
    }

    void receivedMIDITimecodeQuarterFrame (uint8_t quarterFrame) override {
        std::cout << "Received timecode quarter frame " << (int)quarterFrame << std::endl;
    }

    void receivedSongSelect (uint8_t selectedSong) override {
        std::cout << "Received song select " << (int)selectedSong << std::endl;
    }

    void receivedMIDIStart() override {
        std::cout << "Received start" << std::endl;
    }

    void receivedMIDIStop() override {
        std::cout << "Received stop" << std::endl;
    }

    void receivedMIDIContinue() override {
        std::cout << "Received continue" << std::endl;
    }

    void receivedSongPositionPointer (uint16_t positionInBeats) override {
        std::cout << "Received song position pointer at position " << (int)positionInBeats << std::endl;
    }

    void receivedTuneRequest() override {
        std::cout << "Received tune request" << std::endl;
    }

    void receivedActiveSense() override {
        std::cout << "|A| ";
    }

    void receivedMIDIReset() override {
        std::cout << "Received reset" << std::endl;
    }
};

void sendTestSequence(MyDerivedMIDIClass &midiInterface);

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
    

    // Setup a MIDI clock generator
    MIDIClockGenerator midiClockGenerator (midiInterface);
    midiClockGenerator.setIntervall (500, true);

    std::this_thread::sleep_for (std::chrono::seconds (2));
    //Send some random Data
    sendTestSequence (midiInterface);
    
    //wait for incomming MIDI data or the users exit command
    bool running = true;
    while (running) {
        std::string exitCmd;
        std::cin >> exitCmd;
        if(exitCmd.compare("x") == 0) {
            running = false;
        }
        else if(exitCmd.compare ("t") == 0) {
            sendTestSequence (midiInterface);
        }
    }
}


void sendTestSequence(MyDerivedMIDIClass &midiInterface) {

    midiInterface.sendNote (105, 101, SimpleMIDI::NoteOn);
    std::this_thread::sleep_for (std::chrono::seconds (2));

    midiInterface.sendNote (83, 0, SimpleMIDI::NoteOff);
    std::this_thread::sleep_for (std::chrono::seconds (2));

    midiInterface.sendControlChange (5, 18);
    std::this_thread::sleep_for (std::chrono::seconds (2));

    midiInterface.sendAftertouchEvent (74, 12);
    std::this_thread::sleep_for (std::chrono::seconds (2));

    midiInterface.sendAftertouchEvent (SimpleMIDI::MonophonicAftertouch, 63);
    std::this_thread::sleep_for (std::chrono::seconds (2));

    midiInterface.sendProgramChange (1);
    std::this_thread::sleep_for (std::chrono::seconds (2));

    midiInterface.sendPitchBend (-859);
    std::this_thread::sleep_for (std::chrono::seconds (2));

    char sysExMsg[18] = " Hi from your PC!";
    sysExMsg[0] = SimpleMIDI::SysExBegin;
    sysExMsg[17] = SimpleMIDI::SysExEnd;
    midiInterface.sendSysEx (sysExMsg, 18);
    std::this_thread::sleep_for (std::chrono::seconds (2));

    midiInterface.sendMIDITimecodeQuarterFrame (55);
    std::this_thread::sleep_for (std::chrono::seconds (2));

    midiInterface.sendMIDISongPositionPointer (1099);
    std::this_thread::sleep_for (std::chrono::seconds (2));

    midiInterface.sendSongSelect (92);
    std::this_thread::sleep_for (std::chrono::seconds (2));

    midiInterface.sendTuneRequest();
    std::this_thread::sleep_for (std::chrono::seconds (2));

    midiInterface.sendMIDIStart();
    std::this_thread::sleep_for (std::chrono::seconds (2));

    midiInterface.sendMIDIStop();
    std::this_thread::sleep_for (std::chrono::seconds (2));

    midiInterface.sendMIDIContinue();
    std::this_thread::sleep_for (std::chrono::seconds (2));

    midiInterface.sendActiveSense();
    std::this_thread::sleep_for (std::chrono::seconds (2));

    midiInterface.sendReset ();
}
