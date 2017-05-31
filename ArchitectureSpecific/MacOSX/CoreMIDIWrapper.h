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
    std::string deviceName;
    MIDIDeviceRef deviceRef;
} CoreMIDIDeviceRessource;



// Returns all coreMIDI devices currently available
const std::vector<CoreMIDIDeviceRessource> searchMIDIDevices(){
    std::vector<CoreMIDIDeviceRessource> allDevices;
    allDevices.clear();
    
    int availableDeviceCount = (int)MIDIGetNumberOfDevices();
    
    //Iterate through all devices
    int i;
    for (i = 0; i < availableDeviceCount; i++){
        CoreMIDIDeviceRessource d;
        d.deviceRef = MIDIGetDevice(i);
        
        //Check if device is online
        int offline = 0;
        MIDIObjectGetIntegerProperty (d.deviceRef, kMIDIPropertyOffline, &offline);
        
        if (!offline){
            //Get name of device
            CFStringRef deviceNameCFString;
            if (noErr != MIDIObjectGetStringProperty (d.deviceRef, kMIDIPropertyName, &deviceNameCFString)){
                d.deviceName = std::string("Error getting name of MIDI device") + std::to_string(i);
            }
            else{
                // convert CFString to std::string
                char *nameCStrPointer;
                
                //get char pointer
                nameCStrPointer = (char*)CFStringGetCStringPtr (deviceNameCFString, kCFStringEncodingASCII);
                
                // If a nullpointer is returned, no direct char buffer is available. A call to CFStringGetCString does the conversion
                if (nameCStrPointer == NULL){
                    long int length = CFStringGetLength (deviceNameCFString);
                    char nameCStrBuffer[length];
                    CFStringGetCString(deviceNameCFString, nameCStrBuffer, length, kCFStringEncodingASCII);
                    d.deviceName = std::string(nameCStrBuffer);              }
                else {
                    d.deviceName = std::string(nameCStrPointer);
                }
            };
            allDevices.push_back (d);
        };
    };
    return allDevices;
}





class CoreMIDIWrapper : public SimpleMIDIAbstractBaseClass {
    
public:
    
    
    CoreMIDIWrapper (const CoreMIDIDeviceRessource *selectedDevice){
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
        
    };
    
    
    ~CoreMIDIWrapper() override {};
    
    
    int setSendChannel (Channel sendChannel) override {
        if (sendChannel > Channel16){
            return 1;
        }
        this->sendChannel = sendChannel;
        return 0;
    };
    
    
    int setReceiveChannel (Channel receiveChannel) override {
        
        if (receiveChannel <= ChannelAny){
            this-> receiveChannel = receiveChannel;
            if (receiveChannel != ChannelAny){
                lastChannel = receiveChannel;
            }
            return 0;
        }
        return 1;
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
            int sendSysEx(const uint8_t *sysExBuffer, uint16_t length) override {
                if(sysExBuffer[0] == SysExBegin){
                    sendMidiBytes((uint8_t*)sysExBuffer, length);
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
            Channel sendChannel = Channel1;
            Channel receiveChannel = ChannelAny;
            
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
            
            std::vector<uint8_t> sysExReceiveBuffer;
            
            static void readProc(const MIDIPacketList *newPackets, void *refCon, void *connRefCon){
                
                // The connRefCon pointer is the pointer was handed to the MIDIPortConnectSource function call when setting up the connection.
                // As we passed a "this" pointer back then, this now can be used to call the member function of the right class Instance if there
                // are multiple connections established
                CoreMIDIWrapper *callbackDestination = (CoreMIDIWrapper*)connRefCon;
                
                MIDIPacket *packet = (MIDIPacket*)newPackets->packet;
                int packetCount = newPackets->numPackets;
                for (int k = 0; k < packetCount; k++) {
                    const uint8_t messageHeader = packet->data[0];
                    
                    // Process a SysEx if there is a SysExBegin header or if the sysExReceiveBuffer is still filled with unterminated data from the previous packet.
                    if ((messageHeader == SysExBegin) || (! callbackDestination->sysExReceiveBuffer.empty())) {
                        uint16_t currentPacketLength = packet->length;
                        
                        for (uint16_t i = 0; i < currentPacketLength; i++){
                            callbackDestination->sysExReceiveBuffer.push_back (packet->data[i]);
                            
                            if (packet->data[i] == SysExEnd) {
                                callbackDestination->receivedSysEx (callbackDestination->sysExReceiveBuffer.data(), callbackDestination->sysExReceiveBuffer.size());
                                
                                // clear the sysExReceiveBuffer so that sysExReceiveBuffer.empty() will return true
                                callbackDestination->sysExReceiveBuffer.clear();
                                // stop the for loop
                                i = currentPacketLength;
                            }
                        }
                    }
                    else {
                        uint8_t midiChannel = messageHeader & 0x0F;
                        uint8_t midiCommand = messageHeader >> 4;
                        
                        
                        if (callbackDestination->receiveChannel == ChannelAny){
                            
                            switch (midiCommand) {
                                case NoteOnCmd:
                                    callbackDestination->receivedNoteWithChannel (packet->data[1], packet->data[2], NoteOn, (Channel)midiChannel);
                                    break;
                                    
                                case NoteOffCmd:
                                    callbackDestination->receivedNoteWithChannel (packet->data[1], packet->data[2], NoteOff, (Channel)midiChannel);
                                    break;
                                    
                                case ControlChangeCmd:
                                    callbackDestination->receivedControlChangeWithChannel (packet->data[1], packet->data[2], (Channel)midiChannel);
                                    break;
                                    
                                case ProgrammChangeCmd:
                                    callbackDestination->receivedProgrammChangeWithChannel (packet->data[1], (Channel)midiChannel);
                                    break;
                                    
                                default:
                                    std::cout << "Received Unknown MIDI Command: " << std::bitset<8>(packet->data[0]) << " - " << std::bitset<8>(packet->data[1]) << " cmd=" << std::bitset<4>(midiCommand) << std::endl;
                                    break;
                            }
                        }
                        else if (callbackDestination->receiveChannel == midiChannel){
                            
                            switch (midiCommand) {
                                case NoteOnCmd:
                                    callbackDestination->receivedNote (packet->data[1], packet->data[2], NoteOn);
                                    break;
                                    
                                case NoteOffCmd:
                                    callbackDestination->receivedNote (packet->data[1], packet->data[2], NoteOff);
                                    break;
                                    
                                case ControlChangeCmd:
                                    callbackDestination->receivedControlChange (packet->data[1], packet->data[2]);
                                    break;
                                    
                                case ProgrammChangeCmd:
                                    callbackDestination->receivedProgrammChange (packet->data[1]);
                                    break;
                                    
                                default:
                                    std::cout << "Received Unknown MIDI Command: " << std::bitset<8>(packet->data[0]) << " - " << std::bitset<8>(packet->data[1]) << " cmd=" << std::bitset<4>(midiCommand) << std::endl;
                                    break;
                            }
                        }
                        
                    }
                    
                    packet = MIDIPacketNext(packet);
                }
            }
            
            };
            
#endif /* coreMidiWrapper_h */
