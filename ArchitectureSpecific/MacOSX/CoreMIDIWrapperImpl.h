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


class CoreMIDIWrapper : public SimpleMIDI {

public:


    CoreMIDIWrapper (CoreMIDIDeviceRessource &selectedDevice) {
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


    ~CoreMIDIWrapper () override {
    };




    //Sending Data
    inline RetValue sendNote (uint8_t note, uint8_t velocity, bool onOff) {
        return sendNote (note, velocity, onOff, sendChannel);
    };

    RetValue sendNote (uint8_t note, uint8_t velocity, bool onOff, Channel channel) override {
        //Check if values are 7Bit as the MIDI Standard requires
        if ((note >> 7) == 1)
            return FirstArgumentOutOfRange;
        if ((velocity >> 7) == 1)
            return SecondArgumentOutOfRange;

        //Create MIDI Data
        uint8_t dataToSend[3];

        if (onOff == NoteOn) {
            dataToSend[0] = NoteOnCmd << 4 | channel;
            dataToSend[1] = note;
            dataToSend[2] = velocity;
        } else {
            dataToSend[0] = NoteOffCmd << 4 | channel;
            dataToSend[1] = note;
            dataToSend[2] = velocity;
        }

        //Send MIDI Data
        this->sendMidiBytes (dataToSend, 3);

        return Success;
    };

    inline RetValue sendAftertouchEvent (uint8_t note, uint8_t velocity) {
        return sendAftertouchEvent (note, velocity, sendChannel);
    };

    RetValue sendAftertouchEvent (uint8_t note, uint8_t velocity, Channel channel) override {
        if ((velocity >> 7) == 1)
            return SecondArgumentOutOfRange;

        // check if it is a monophonic aftertouch
        if (note == MonophonicAftertouch) {
            uint8_t dataToSend[2];
            dataToSend[0] = MonophonicAftertouchCmd << 4 | channel;
            dataToSend[1] = velocity;
            this->sendMidiBytes (dataToSend, 2);
        }
            // if not, check if the note is inside the range
        else if ((note >> 7) == 1)
            return FirstArgumentOutOfRange;
        else {
            uint8_t dataToSend[3];
            dataToSend[0] = PolyphonicAftertouchCmd << 4 | channel;
            dataToSend[1] = note;
            dataToSend[2] = velocity;
            this->sendMidiBytes (dataToSend, 3);
        }

        return Success;
    };

    inline RetValue sendControlChange (uint8_t control, uint8_t value) {
        return sendControlChange (control, value, sendChannel);
    };

    RetValue sendControlChange (uint8_t control, uint8_t value, Channel channel) override {
        //Check if values are 7Bit as the MIDI Standard requires
        if ((control >> 7) == 1)
            return FirstArgumentOutOfRange;
        if ((value >> 7) == 1)
            return SecondArgumentOutOfRange;

        //Create MIDI Data
        uint8_t dataToSend[3];
        dataToSend[0] = ControlChangeCmd << 4 | channel;
        dataToSend[1] = control;
        dataToSend[2] = value;

        //Send MIDI Data
        this->sendMidiBytes (dataToSend, 3);

        return Success;
    };

    inline RetValue sendProgramChange (uint8_t program) {
        return sendProgramChange (program, sendChannel);
    };

    RetValue sendProgramChange (uint8_t program, Channel channel) override {
        //Check if values are 7Bit as the MIDI Standard requires
        if ((program >> 7) == 1)
            return FirstArgumentOutOfRange;

        //Create MIDI Data
        uint8_t dataToSend[2];
        dataToSend[0] = ProgrammChangeCmd << 4 | channel;
        dataToSend[1] = program;

        //Send MIDI Data
        this->sendMidiBytes (dataToSend, 2);

        return Success;
    };

    // todo: check if this works as expected
    RetValue sendPitchBend (int16_t pitch) override {

        // the input value is biased arround 0, so an offset has to be applied to get it into the range
        // from 0 - 2^14 as the "wire format" defines
        int16_t offset = 1 << 13;

        // check if input is in range
        if (pitch > offset)
            return SecondArgumentOutOfRange;
        if (pitch < -offset)
            return SecondArgumentOutOfRange;

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

        return Success;

    }

    //SysEx Messages must be framed by SYSEX_BEGIN and SYSEX_END
    RetValue sendSysEx (const char *sysExBuffer, uint16_t length) override {
        if (sysExBuffer[0] == SysExBegin) {
            if (sysExBuffer[length - 1] == SysExEnd) {
                sendMidiBytes ((uint8_t *) sysExBuffer, length);
                return Success;
            }
            return MissingSysExEnd;
        }
        return MissingSysExStart;
    };

    RetValue sendMIDITimecodeQuarterFrame (uint8_t quarterFrame) override {
        //Check if values are 7Bit as the MIDI Standard requires
        if ((quarterFrame >> 7) == 1)
            return FirstArgumentOutOfRange;

        //Create MIDI Data
        uint8_t dataToSend[2];
        dataToSend[0] = MIDITimecodeQuarterFrame;
        dataToSend[1] = quarterFrame;

        //Send MIDI Data
        this->sendMidiBytes (dataToSend, 2);

        return Success;
    };

    RetValue sendMIDISongPositionPointer (uint16_t positionInBeats) override {
        // positionInBeats has to be a 14 Bit int
        if ((positionInBeats >> 14) != 0) {
            return FirstArgumentOutOfRange;
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

        return Success;
    };

    RetValue sendSongSelect (uint8_t songToSelect) override {
        //Check if values are 7Bit as the MIDI Standard requires
        if ((songToSelect >> 7) == 1)
            return FirstArgumentOutOfRange;

        //Create MIDI Data
        uint8_t dataToSend[2];
        dataToSend[0] = SongSelectCmd;
        dataToSend[1] = songToSelect;

        //Send MIDI Data
        this->sendMidiBytes (dataToSend, 2);

        return Success;
    };

    void sendTuneRequest() override {
        uint8_t cmd = TuneRequest;
        sendMidiBytes (&cmd, 1);
    };

    void sendMIDIClockTick() override {
        uint8_t cmd = ClockTickCmd;
        this->sendMidiBytes (&cmd, 1);
    };

    void sendMIDIStart() override {
        uint8_t cmd = StartCmd;
        this->sendMidiBytes (&cmd, 1);
    };

    void sendMIDIStop() override {
        uint8_t cmd = StopCmd;
        this->sendMidiBytes (&cmd, 1);
    };

    void sendMIDIContinue() override {
        uint8_t cmd = ContinueCmd;
        this->sendMidiBytes (&cmd, 1);
    };

    void sendActiveSense() override {
        uint8_t cmd = ActiveSense;
        sendMidiBytes (&cmd, 1);
    };

    void sendReset() override {
        uint8_t cmd = MIDIReset;
        sendMidiBytes (&cmd, 1);
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

    std::vector<char> sysExReceiveBuffer;

    static void readProc (const MIDIPacketList *newPackets, void *refCon, void *connRefCon) {

        // The connRefCon pointer is the pointer was handed to the MIDIPortConnectSource function call when setting up the connection.
        // As we passed a "this" pointer back then, this now can be used to call the member function of the right class Instance if there
        // are multiple connections established
        CoreMIDIWrapper *callbackDestination = (CoreMIDIWrapper *) connRefCon;

        MIDIPacket *packet = (MIDIPacket *) newPackets->packet;
        int packetCount = newPackets->numPackets;
        for (int k = 0; k < packetCount; k++) {
            const uint8_t messageHeader = packet->data[0];

            // Process a SysEx if there is a SysExBegin header or if the sysExReceiveBuffer is still filled with unterminated data from the previous packet.
            if ((messageHeader == (uint8_t)SysExBegin) || (!callbackDestination->sysExReceiveBuffer.empty ())) {
                uint16_t currentPacketLength = packet->length;

                for (uint16_t i = 0; i < currentPacketLength; i++) {
                    callbackDestination->sysExReceiveBuffer.push_back ((char)packet->data[i]);

                    if (packet->data[i] == (uint8_t)SysExEnd) {
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
                    case MIDITimecodeQuarterFrame:
                        callbackDestination->receivedMIDITimecodeQuarterFrame (packet->data[1]);
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

                    case SongSelectCmd:
                        callbackDestination->receivedSongSelect (packet->data[1]);
                        break;

                    case TuneRequest:
                        callbackDestination->receivedTuneRequest();
                        break;

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

                    case ActiveSense:
                        callbackDestination->receivedActiveSense();
                        break;

                    case MIDIReset:
                        callbackDestination->receivedMIDIReset();
                        break;

                    default: {
                        // process 4 Bit commands with channel
                        uint8_t midiChannel = messageHeader & 0x0F;
                        uint8_t midiCommand = messageHeader >> 4;



                        if ((callbackDestination->receiveChannel == midiChannel) || (callbackDestination->receiveChannel == ChannelAny)) {

                            callbackDestination->lastChannel = (Channel)midiChannel;

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
