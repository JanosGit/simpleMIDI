//
//  coreMIDIWrapper.h
//
//  Created by Janos Buttgereit on 30.10.16.
//  Copyright © 2016 Janos Buttgereit. All rights reserved.
//

#ifndef coreMidiWrapper_h
#define coreMidiWrapper_h

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>

#include <CoreMIDI/CoreMIDI.h>

using namespace simpleMIDI;

class CoreMIDIWrapper;


//Groups a MIDIDeviceRef with its name
typedef struct{
    CFStringRef deviceName;
    MIDIDeviceRef deviceRef;
} MIDIDeviceInfo;






//Converts the CFString Pointer from a MIDIDeviceInfo Struct into a C++ String
const std::string deviceNameString(const MIDIDeviceInfo *mdi){
    long int length = CFStringGetLength (mdi->deviceName);
    char *nameCStrPointer;
    
    //get char pointer
    nameCStrPointer = (char*)CFStringGetCStringPtr (mdi->deviceName, kCFStringEncodingASCII);
    
    // If a nullpointer is returned, no direct char buffer is available. A call to CFStringGetCString does the conversion
    if (nameCStrPointer == NULL){
        char nameCStrBuffer[length];
        CFStringGetCString(mdi->deviceName, nameCStrBuffer, length, kCFStringEncodingASCII);
        const std::string nameString (nameCStrBuffer);
        return nameString;
    }
    else {
        const std::string nameString(nameCStrPointer);
        return nameString;
    }
}




// Returns all coreMIDI devices currently available
const std::vector<MIDIDeviceInfo> searchMIDIDevices(){
    std::vector<MIDIDeviceInfo> allDevices;
    allDevices.clear();
    
    int availableDeviceCount = (int)MIDIGetNumberOfDevices();
    
    //Iterate through all devices
    int i;
    for (i = 0; i < availableDeviceCount; i++){
        MIDIDeviceInfo d;
        d.deviceRef = MIDIGetDevice(i);
        d.deviceName = nil;
        
        //Check if device is online
        int offline = 0;
        MIDIObjectGetIntegerProperty (d.deviceRef, kMIDIPropertyOffline, &offline);
        
        if (!offline){
            //Get name of device
            if (noErr != MIDIObjectGetStringProperty (d.deviceRef, kMIDIPropertyName, &d.deviceName)){
                d.deviceName = CFStringCreateWithFormat (NULL, NULL, CFSTR("Error getting name of MIDI device # %d"), i);
            };
            allDevices.push_back (d);
        };
    };
    return allDevices;
}





class CoreMIDIWrapper : public SimpleMIDIAbstractBaseClass {
    
public:
    
    ~CoreMIDIWrapper() override {};
    
    //Setup Routines
    int selectDevice(const MIDIDeviceInfo *selectedDevice){
        entity = MIDIDeviceGetEntity(selectedDevice->deviceRef, 0);
        source = MIDIEntityGetSource(entity, 0);
        destination = MIDIEntityGetDestination(entity, 0);
        MIDIClientCreate (CFSTR("client"), NULL, NULL, &client);
        MIDIOutputPortCreate (client, CFSTR("Out"), &outputPort);
        MIDIInputPortCreate (client, CFSTR("In"), readProc, NULL, &inputPort);
        
        // The refCon pointer passed to MIDIPortConnectSource will be passed to the
        // readProc function every time it is called from this special input port.
        // Passing "this" as the refCon makes it possible for the static readProc function
        // to call the correspondind member functions of the class instance that created
        // the connection
        void *myRefCon = this;
        MIDIPortConnectSource (inputPort, source, myRefCon);
        return 0;
    };
    
    
    int setSendChannel (Channel_t sendChannel) override {
        if (sendChannel > Channel16){
            return 1;
        }
        this->sendChannel = sendChannel;
        return 0;
    };
    
    // todo: implement this feature
    int setReceiveChannel (Channel_t receiveChannel) override {
        return 0;
    };
    
    //Sending Data
    int sendNote (uint8_t note, uint8_t velocity, bool onOff) override {
        //Check if values are 7Bit as the MIDI Standard requires
        if((note>>7)==1) return 1;
        if((velocity>>7)==1) return 2;
        
        //Create MIDI Data
        uint8_t dataToSend[3];
        
        if(onOff == true){
            dataToSend[0] = NoteOnCmd << 4 | sendChannel;
            dataToSend[1] = note;
            dataToSend[2] = velocity;
        }
            else {
            dataToSend[0] = NoteOffCmd << 4 | sendChannel;
            dataToSend[1] = note;
            dataToSend[2] = velocity;
        }
            
        //Send MIDI Data
        this->sendMidiBytes(dataToSend, 3);
            
        return 0;
    };
    
    
    int sendControlChange(uint8_t control, uint8_t value) override {
        //Check if values are 7Bit as the MIDI Standard requires
        if((control>>7)==1) return 1;
        if((value>>7)==1) return 2;
                
        //Create MIDI Data
        uint8_t dataToSend[3];
        dataToSend[0] = ControlChangeCmd << 4 | sendChannel;
        dataToSend[1] = control;
        dataToSend[2] = value;
                
        //Send MIDI Data
        this->sendMidiBytes(dataToSend, 3);
                
        return 0;
    };
            
            
    int sendProgramChange(uint8_t program) override {
        //Check if values are 7Bit as the MIDI Standard requires
        if((program>>7)==1) return 1;
        
        //Create MIDI Data
        uint8_t dataToSend[2];
        dataToSend[0] = ProgrammChangeCmd << 4 | sendChannel;
        dataToSend[1] = program;
                
        //Send MIDI Data
        this->sendMidiBytes(dataToSend, 2);
                
        return 0;
    };
            
            
    //SysEx Messages must be framed by SYSEX_BEGIN and SYSEX_END
    int sendSysEx(uint8_t *dataBuffer, uint32_t length) override {
        if(dataBuffer[0] == SysExBegin){
            sendMidiBytes(dataBuffer, length);
            return 0;
        }
        return 1;
    };
    
    
protected:
    // Everything describing the "physical" MIDI device
    MIDIEntityRef entity;
    MIDIClientRef client;
    MIDIPortRef outputPort;
    MIDIPortRef inputPort;
    MIDIEndpointRef destination;
    MIDIEndpointRef source;
    
    //Everything needed for sending
    char buffer[1024];
    MIDIPacketList *pktList;
    MIDIPacket *pkt;
    uint8_t sendChannel;
            
    int sendMidiBytes(uint8_t *bytesToSend, int length) {
        //Initialize Packetlist
        pktList = (MIDIPacketList*)&buffer;
        pkt = MIDIPacketListInit (pktList);
        //Add bytes to send to list
        pkt = MIDIPacketListAdd (pktList, 1024, pkt, 0, length, bytesToSend);
        //Send list
        MIDISend (outputPort, destination, pktList);
        return 0;
    };
    
    
    
    // todo: same thing as above: Don't use the currentMidiIO pointer
    static void readProc(const MIDIPacketList *newPackets, void *refCon, void *connRefCon){
        
        // The connRefCon pointer is the pointer was handed to the MIDIPortConnectSource function call when setting up the connection.
        // As we passed a "this" pointer back then, this now can be used to call the member function of the right class Instance if there
        // are multiple connections established
        CoreMIDIWrapper *callbackDestination = (CoreMIDIWrapper*)connRefCon;

        MIDIPacket *packet = (MIDIPacket*)newPackets->packet;
        int packetCount = newPackets->numPackets;
        for (int k = 0; k < packetCount; k++) {
            Byte midiStatus = packet->data[0];
            Byte midiChannel= midiStatus & 0x0F;
            Byte midiCommand = midiStatus >> 4;
            
            if(midiCommand == NoteOnCmd){
                callbackDestination->receivedNote (packet->data[1], packet->data[2], NoteOn);
            }
            else if(midiCommand == NoteOffCmd){
                callbackDestination->receivedNote (packet->data[1], packet->data[2], NoteOff);
            }
            else if (midiCommand == ProgrammChangeCmd){
                callbackDestination->receivedProgrammChange (packet->data[1]);
            }
            else if(midiCommand == ControlChangeCmd){
                callbackDestination->receivedControlChange (packet->data[1], packet->data[2]);
            }
            else{
                std::cout << "Received Unknown MIDI Command: " << std::bitset<8>(packet->data[0]) << " - " << std::bitset<8>(packet->data[1]) << " cmd=" << std::bitset<4>(midiCommand) << std::endl;
            }
            packet = MIDIPacketNext(packet);
        }
        
    }
    
};

#endif /* coreMidiWrapper_h */
