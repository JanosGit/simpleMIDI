//
//  coreMIDIWrapper.h
//
//  Created by Janos Buttgereit on 30.10.16.
//  Copyright © 2016 Janos Buttgereit. All rights reserved.
//

#ifndef coreMidiWrapper_mm
#define coreMidiWrapper_mm

#include "CoreMIDIWrapperDef.h"
#include "../../simpleMIDI.h"


class CoreMIDIWrapperDef : public SimpleMIDI {

public:


    CoreMIDIWrapperDef (const CoreMIDIDeviceRessource &selectedDevice) {
        entity = MIDIDeviceGetEntity (selectedDevice.deviceRef, 0);
        source = MIDIEntityGetSource (entity, 0);
        destination = MIDIEntityGetDestination (entity, 0);
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


    ~CoreMIDIWrapperDef () override {
    };


    int setSendChannel (Channel sendChannel) override {
        if (sendChannel > Channel16) {
            return 1;
        }
        this->sendChannel = sendChannel;
        return 0;
    };


    int setReceiveChannel (Channel receiveChannel) override {

        if (receiveChannel <= ChannelAny) {
            this->receiveChannel = receiveChannel;
            if (receiveChannel != ChannelAny) {
                lastChannel = receiveChannel;
            }
            return 0;
        }
        return 1;
    };

    //Sending Data
    int sendNote (uint8_t note, uint8_t velocity, bool onOff) override {
        //Check if values are 7Bit as the MIDI Standard requires
        if ((note >> 7) == 1)
            return 1;
        if ((velocity >> 7) == 1)
            return 2;

        //Create MIDI Data
        uint8_t dataToSend[3];

        if (onOff == true) {
            dataToSend[0] = NoteOnCmd << 4 | sendChannel;
            dataToSend[1] = note;
            dataToSend[2] = velocity;
        } else {
            dataToSend[0] = NoteOffCmd << 4 | sendChannel;
            dataToSend[1] = note;
            dataToSend[2] = velocity;
        }

        //Send MIDI Data
        this->sendMidiBytes (dataToSend, 3);

        return 0;
    };


    int sendAftertouchEvent (uint8_t note, uint8_t velocity) override {
        if ((velocity >> 7) == 1)
            return 2;

        // check if it is a monophonic aftertouch
        if (note == MonophonicAftertouch) {
            uint8_t dataToSend[2];
            dataToSend[0] = MonophonicAftertouchCmd << 4 | sendChannel;
            dataToSend[1] = velocity;
            this->sendMidiBytes (dataToSend, 2);
        }
            // if not, check if the note is inside the range
        else if ((note >> 7) == 1)
            return 1;
        else {
            uint8_t dataToSend[3];
            dataToSend[0] = PolyphonicAftertouchCmd << 4 | sendChannel;
            dataToSend[1] = note;
            dataToSend[2] = velocity;
            this->sendMidiBytes (dataToSend, 3);
        }

        return 0;
    }

    int sendControlChange (uint8_t control, uint8_t value) override {
        //Check if values are 7Bit as the MIDI Standard requires
        if ((control >> 7) == 1)
            return 1;
        if ((value >> 7) == 1)
            return 2;

        //Create MIDI Data
        uint8_t dataToSend[3];
        dataToSend[0] = ControlChangeCmd << 4 | sendChannel;
        dataToSend[1] = control;
        dataToSend[2] = value;

        //Send MIDI Data
        this->sendMidiBytes (dataToSend, 3);

        return 0;
    };


    int sendProgramChange (uint8_t program) override {
        //Check if values are 7Bit as the MIDI Standard requires
        if ((program >> 7) == 1)
            return 1;

        //Create MIDI Data
        uint8_t dataToSend[2];
        dataToSend[0] = ProgrammChangeCmd << 4 | sendChannel;
        dataToSend[1] = program;

        //Send MIDI Data
        this->sendMidiBytes (dataToSend, 2);

        return 0;
    };

    // todo: check if this works as expected
    int sendPitchBend (int16_t pitch) override {

        // the input value is biased arround 0, so an offset has to be applied to get it into the range
        // from 0 - 2^14 as the "wire format" defines
        int16_t offset = 1 << 13;

        // check if input is in range
        if (pitch > offset)
            return 1;
        if (pitch < -offset)
            return 2;

        // should be positive after this operation
        pitch += offset;

        // lower 7 bit
        const int16_t lsb = pitch & 0x7F;
        // upper 7 bit
        const int16_t msb = pitch >> 7;

        uint8_t dataToSend[3];
        dataToSend[0] = PitchBendCmd;
        dataToSend[1] = lsb;
        dataToSend[2] = msb;

        this->sendMidiBytes (dataToSend, 3);

        return 0;

    }

    //SysEx Messages must be framed by SYSEX_BEGIN and SYSEX_END
    int sendSysEx (const uint8_t *sysExBuffer, uint16_t length) override {
        if (sysExBuffer[0] == SysExBegin) {
            sendMidiBytes ((uint8_t *) sysExBuffer, length);
            return 0;
        }
        return 1;
    };

    void sendMIDIClockTick () override {
        uint8_t cmd = ClockTickCmd;
        this->sendMidiBytes (&cmd, 1);
    }

    void sendMIDIStart () override {

        uint8_t cmd = StartCmd;
        this->sendMidiBytes (&cmd, 1);
    }

    void sendMIDIStop () override {

        uint8_t cmd = StopCmd;
        this->sendMidiBytes (&cmd, 1);
    }

    void sendMIDIContinue () override {

        uint8_t cmd = ContinueCmd;
        this->sendMidiBytes (&cmd, 1);
    }

    int sendMIDISongPositionPointer (uint16_t positionInBeats) override {
        // positionInBeats has to be a 14 Bit int
        if ((positionInBeats >> 14) != 0) {
            return 1;
        }
        // lower 7 bit
        const uint16_t lsb = positionInBeats & 0x7F;
        // upper 7 bit
        const uint16_t msb = positionInBeats >> 7;

        uint8_t dataToSend[3];
        dataToSend[0] = SongPositionPointerCmd;
        dataToSend[1] = lsb;
        dataToSend[2] = msb;

        this->sendMidiBytes (dataToSend, 3);

        return 0;
    }

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

    int sendMidiBytes (uint8_t *bytesToSend, int length) {
        //Initialize Packetlist
        pktList = (MIDIPacketList *) &buffer;
        pkt = MIDIPacketListInit (pktList);
        //Add bytes to send to list
        pkt = MIDIPacketListAdd (pktList, 1024, pkt, 0, length, bytesToSend);
        //Send list
        MIDISend (outputPort, destination, pktList);
        return 0;
    };

    std::vector<uint8_t> sysExReceiveBuffer;

    static void readProc (const MIDIPacketList *newPackets, void *refCon, void *connRefCon) {

        // The connRefCon pointer is the pointer was handed to the MIDIPortConnectSource function call when setting up the connection.
        // As we passed a "this" pointer back then, this now can be used to call the member function of the right class Instance if there
        // are multiple connections established
        CoreMIDIWrapperDef *callbackDestination = (CoreMIDIWrapperDef *) connRefCon;

        MIDIPacket *packet = (MIDIPacket *) newPackets->packet;
        int packetCount = newPackets->numPackets;
        for (int k = 0; k < packetCount; k++) {
            const uint8_t messageHeader = packet->data[0];

            // Process a SysEx if there is a SysExBegin header or if the sysExReceiveBuffer is still filled with unterminated data from the previous packet.
            if ((messageHeader == SysExBegin) || (!callbackDestination->sysExReceiveBuffer.empty ())) {
                uint16_t currentPacketLength = packet->length;

                for (uint16_t i = 0; i < currentPacketLength; i++) {
                    callbackDestination->sysExReceiveBuffer.push_back (packet->data[i]);

                    if (packet->data[i] == SysExEnd) {
                        callbackDestination->receivedSysEx (callbackDestination->sysExReceiveBuffer.data (), callbackDestination->sysExReceiveBuffer.size ());

                        // clear the sysExReceiveBuffer so that sysExReceiveBuffer.empty() will return true
                        callbackDestination->sysExReceiveBuffer.clear ();
                        // stop the for loop
                        i = currentPacketLength;
                    }
                }
            } else {
                // process 8 Bit Commands
                switch (messageHeader) {
                    case ClockTickCmd:
                        callbackDestination->receivedMIDIClockTick ();
                        break;

                    case StartCmd:
                        callbackDestination->receivedMIDIStart ();
                        break;

                    case StopCmd:
                        callbackDestination->receivedMIDIStop ();
                        break;

                    case ContinueCmd:
                        callbackDestination->receivedMIDIContinue ();
                        break;

                    case SongPositionPointerCmd: {
                        // take the msb and shift it up
                        uint16_t positionInBeats = packet->data[2];
                        positionInBeats <<= 7;
                        // add the lsb
                        positionInBeats &= packet->data[1];
                        callbackDestination->receivedSongPositionPointer (positionInBeats);
                    }
                        break;

                    default: {
                        // process 4 Bit commands with channel
                        uint8_t midiChannel = messageHeader & 0x0F;
                        uint8_t midiCommand = messageHeader >> 4;


                        if (callbackDestination->receiveChannel == ChannelAny) {

                            switch (midiCommand) {
                                case NoteOnCmd:
                                    callbackDestination->receivedNoteWithChannel (packet->data[1], packet->data[2], NoteOn, (Channel) midiChannel);
                                    break;

                                case NoteOffCmd:
                                    callbackDestination->receivedNoteWithChannel (packet->data[1], packet->data[2], NoteOff, (Channel) midiChannel);
                                    break;

                                case PolyphonicAftertouchCmd:
                                    callbackDestination->receivedAftertouchWithChannel (packet->data[1], packet->data[2], (Channel) midiChannel);
                                    break;

                                case MonophonicAftertouchCmd:
                                    callbackDestination->receivedAftertouchWithChannel (MonophonicAftertouch, packet->data[1], (Channel) midiChannel);

                                case ControlChangeCmd:
                                    callbackDestination->receivedControlChangeWithChannel (packet->data[1], packet->data[2], (Channel) midiChannel);
                                    break;

                                case ProgrammChangeCmd:
                                    callbackDestination->receivedProgramChangeWithChannel (packet->data[1], (Channel) midiChannel);
                                    break;

                                case PitchBendCmd:
                                    callbackDestination->receivedPitchBendWithChannel (packet->data[1], (Channel) midiChannel);
                                    break;

                                default:
                                    callbackDestination->receivedUnknownCommand (packet->data, packet->length);
                                    break;
                            }
                        } else if (callbackDestination->receiveChannel == midiChannel) {

                            switch (midiCommand) {
                                case NoteOnCmd:
                                    callbackDestination->receivedNote (packet->data[1], packet->data[2], NoteOn);
                                    break;

                                case NoteOffCmd:
                                    callbackDestination->receivedNote (packet->data[1], packet->data[2], NoteOff);
                                    break;

                                case PolyphonicAftertouchCmd:
                                    callbackDestination->receivedAftertouch (packet->data[1], packet->data[2]);
                                    break;

                                case MonophonicAftertouchCmd:
                                    callbackDestination->receivedAftertouch (MonophonicAftertouch, packet->data[1]);

                                case ControlChangeCmd:
                                    callbackDestination->receivedControlChange (packet->data[1], packet->data[2]);
                                    break;

                                case ProgrammChangeCmd:
                                    callbackDestination->receivedProgramChange (packet->data[1]);
                                    break;

                                case PitchBendCmd:
                                    callbackDestination->receivedPitchBend (packet->data[1]);
                                    break;

                                default:
                                    callbackDestination->receivedUnknownCommand (packet->data, packet->length);
                                    break;
                            }
                        }
                    }
                        break;
                }
            }

            packet = MIDIPacketNext (packet);
        }
    }

};

#endif /* coreMidiWrapper_mm */
