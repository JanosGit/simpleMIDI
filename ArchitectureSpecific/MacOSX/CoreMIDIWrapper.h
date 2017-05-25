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

// todo: change macros to const uint8_t
//------------------------------------------------------------------------
//Note On Off Argument
#define NOTE_ON 0
#define NOTE_OFF 1

//MIDI Command
#define NOTE_ON_CMD 0b1001
#define NOTE_OFF_CMD 0b1000
#define CC_CMD 0b1011
#define PC_CMD 0b1100
#define SYSEX_BEGIN 0b11110000
#define SYSEX_END 0b11110111
//------------------------------------------------------------------------

class CoreMIDIWrapper;

// todo: understand & use the contex-pointer coreMIDI uses to get rid of this ugly global variables
CoreMIDIWrapper *currentMidiIO;
uint8_t *sysExBuffer;
uint32_t sysExBufferIdx;
uint32_t sysExBufferSize;




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
    CoreMIDIWrapper(){
        // only needed for ugly readProc() solution
        currentMidiIO = this;
    };
    
    ~CoreMIDIWrapper() override {};
    
    //Setup Routines
    int selectDevice(const MIDIDeviceInfo *selectedDevice){
        device = *selectedDevice;
        entity = MIDIDeviceGetEntity(selectedDevice->deviceRef, 0);
        source = MIDIEntityGetSource(entity, 0);
        destination = MIDIEntityGetDestination(entity, 0);
        MIDIClientCreate (CFSTR("client"), NULL, NULL, &client);
        MIDIOutputPortCreate (client, CFSTR("Out"), &outputPort);
        MIDIInputPortCreate (client, CFSTR("In"), readProc, NULL, &inputPort);
        MIDIPortConnectSource (inputPort, source, NULL);
        return 0;
    };
    
    
    int setSendChannel (MIDIChannel_t sendChannel) override {
        if (sendChannel > MIDIChannel16){
            return 1;
        }
        this->sendChannel = sendChannel;
        return 0;
    };
    
    // todo: implement this feature
    int setReceiveChannel (MIDIChannel_t receiveChannel) override {
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
            dataToSend[0]= NOTE_ON_CMD<<4 | sendChannel;
            dataToSend[1]=note;
            dataToSend[2]=velocity;
        }
            else {
            dataToSend[0]= NOTE_OFF_CMD<<4 | sendChannel;
            dataToSend[1]=note;
            dataToSend[2]=velocity;
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
        dataToSend[0]=CC_CMD<<4 | sendChannel;
        dataToSend[1]=control;
        dataToSend[2]=value;
                
        //Send MIDI Data
        this->sendMidiBytes(dataToSend, 3);
                
        return 0;
    };
            
            
    int sendProgramChange(uint8_t program) override {
        //Check if values are 7Bit as the MIDI Standard requires
        if((program>>7)==1) return 1;
        
        //Create MIDI Data
        uint8_t dataToSend[2];
        dataToSend[0]=PC_CMD<<4 | sendChannel;
        dataToSend[1]=program;
                
        //Send MIDI Data
        this->sendMidiBytes(dataToSend, 2);
                
        return 0;
    };
            
            
    //SysEx Messages must be framed by SYSEX_BEGIN and SYSEX_END
    int sendSysEx(uint8_t *dataBuffer, uint32_t length) override {
        if(dataBuffer[0]==SYSEX_BEGIN){
            sendMidiBytes(dataBuffer, length);
            return 0;
        }
        return 1;
    };
    
    
protected:
    //MIDI device
    MIDIDeviceInfo device;
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
        MIDIPacket *packet = (MIDIPacket*)newPackets->packet;
        int packetCount = newPackets->numPackets;
        for (int k = 0; k < packetCount; k++) {
            Byte midiStatus = packet->data[0];
            Byte midiChannel= midiStatus & 0x0F;
            Byte midiCommand = midiStatus >> 4;
            
            if(midiCommand==NOTE_ON_CMD){
                currentMidiIO->receivedNote (packet->data[1], packet->data[2], true);
            }
            else if(midiCommand==NOTE_OFF_CMD){
                currentMidiIO->receivedNote (packet->data[1], packet->data[2], false);
            }
            else if (midiCommand==PC_CMD){
                currentMidiIO->receivedProgrammChange (packet->data[1]);
            }
            else if(midiCommand==CC_CMD){
                currentMidiIO->receivedControlChange (packet->data[1], packet->data[2]);
            }
            else{
                std::cout << "Received Unknown MIDI Command: " << std::bitset<8>(packet->data[0]) << " - " << std::bitset<8>(packet->data[1]) << " cmd=" << std::bitset<4>(midiCommand) << std::endl;
            }
            packet = MIDIPacketNext(packet);
        }
        
    }
    
};

#endif /* coreMidiWrapper_h */
