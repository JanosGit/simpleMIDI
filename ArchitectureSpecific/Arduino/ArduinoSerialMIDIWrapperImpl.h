#ifndef ArduinoSerialMIDIWrapperImpl_h
#define ArduinoSerialMIDIWrapperImpl_h

#include "ArduinoSerialMIDIWrapperDef.h"
#include "HardwareSerial.h"
#include <simpleMIDI.h>

#ifndef SIMPLE_MIDI_ARDUINO_NO_SOFT_SERIAL
#include <SoftwareSerial.h>
#endif


class ArduinoSerialMIDIWrapper : public SimpleMIDI {

public:

    ArduinoSerialMIDIWrapper (HardwareSerial& selectedDevice) : serialInterface (selectedDevice), header (midiDataBuffer[0]) {

        serialInterfaceType = HardwareSerialInterface;

#ifdef SIMPLE_MIDI_PRE_C++11
        sendChannel = Channel1;
        receiveChannel = ChannelAny;
#endif

        numBytesInMidiDataBuffer = 0;
        numBytesToWaitFor = 0;

    }
#ifndef SIMPLE_MIDI_ARDUINO_NO_SOFT_SERIAL
    ArduinoSerialMIDIWrapper (SoftwareSerial& selectedDevice) : serialInterface (selectedDevice), header (midiDataBuffer[0]) {

        serialInterfaceType = SoftwareSerialInterface;

#ifdef SIMPLE_MIDI_PRE_C++11
        sendChannel = Channel1;
        receiveChannel = ChannelAny;
#endif

        numBytesInMidiDataBuffer = 0;
        numBytesToWaitFor = 0;

    }
#endif


    ~ArduinoSerialMIDIWrapper() {

        // This is a bit ugly, but I found no other way to do this.
        switch (serialInterfaceType) {
            case HardwareSerialInterface: {
                HardwareSerial *downcastedInterface = (HardwareSerial*)&serialInterface;
                downcastedInterface->end();
            }
                break;
#ifndef SIMPLE_MIDI_ARDUINO_NO_SOFT_SERIAL
            case SoftwareSerialInterface: {
                SoftwareSerial *downcastedInterface = (SoftwareSerial*)&serialInterface;
                downcastedInterface->end();
            }
                break;
#endif
        }
    }

    /**
     * Call this during setup. Before calling begin() no MIDI I/O will take place
     */
    void begin() {
        switch (serialInterfaceType) {
            case HardwareSerialInterface: {
                HardwareSerial *downcastedInterface = (HardwareSerial*)&serialInterface;
                downcastedInterface->begin (31250);
            }
                break;
/*
            case SoftwareSerialInterface: {
                SoftwareSerial *downcastedInterface = (SoftwareSerial*)&serialInterface;
                downcastedInterface->begin (31250);
            }
                break;
                */
        }
    }

    /**
     * As there are not automatic callbacks on the Arduino, this has to be called in the loop() or on a timer interrupt
     * to fetch new midi bytes from the Serial buffer
     */
    void receive() {

        uint16_t numBytesAvailable = serialInterface.available();

        while (numBytesAvailable) {
            digitalWrite(6, LOW);

            if (numBytesToWaitFor == 0) {
                // waiting for the next message
                midiDataBuffer[0] = serialInterface.read();
                numBytesInMidiDataBuffer = 1;
                numBytesAvailable--;

                if (header >= 0b10000000){
                    // in this case it's really a midi header

                    if ((header & 0b11100000) == 0b11000000) {
                        // in this case its a 4-Bit command and one byte is expected
                        lastChannel = (SimpleMIDI::Channel)(header & 0b00001111);
                        uint8_t command = header >> 4;
                        if ((receiveChannel == ChannelAny) || (receiveChannel == lastChannel)) {
                            numBytesToWaitFor = 1;
                            switch (command) {

                                case ProgrammChangeCmd:
                                    eventHandler = programChangeEventHandler;
                                    break;

                                case MonophonicAftertouchCmd:
                                    eventHandler = monophonicAftertouchEventHandler;
                                    break;

                                default:
                                    // no valid command, just wait for the next header
                                    numBytesToWaitFor = 0;
                                    break;
                            }

                        }

                    }
                    else if ((header & 0b11110000) == 0b11110000) {
                        // in this case it's a 8-Bit command

                        switch (header) {

                            case MIDITimecodeQuarterFrame:
                                numBytesToWaitFor = 1;
                                eventHandler = timecodeQuarterFrameEventHandler;
                                break;

                            case SongPositionPointerCmd:
                                numBytesToWaitFor = 2;
                                eventHandler = songPositionPointerEventHandler;
                                break;

                            case SongSelectCmd:
                                numBytesToWaitFor = 1;
                                eventHandler = songSelectEventHandler;
                                break;

                            case SysExBegin:
                                // a negative numBytesToWaitFor will represent the number of sysex bytes in the buffer
                                numBytesToWaitFor = -1;
                                break;

                                // all other commands require no further bytes and are handled immediately
                            case TuneRequest:
                                receivedTuneRequest ();
                                break;

                            case ClockTickCmd:
                                receivedMIDIClockTick ();
                                break;

                            case StartCmd:
                                receivedMIDIStart ();
                                break;

                            case ContinueCmd:
                                receivedMIDIContinue ();
                                break;

                            case StopCmd:
                                receivedMIDIStop ();
                                break;

                            case ActiveSense:
                                receivedActiveSense ();
                                break;

                            case MIDIReset:
                                receivedMIDIReset ();
                                break;

                                // In all other cases just do nothing

                        }


                    }
                    else {
                        // in this case its a 4-Bit command and two bytes are expected
                        lastChannel = (SimpleMIDI::Channel)(header & 0b00001111);
                        uint8_t command = header >> 4;
                        if ((receiveChannel == ChannelAny) || (receiveChannel == lastChannel)) {
                            numBytesToWaitFor = 2;
                            switch (command) {
                                case NoteOffCmd:
                                    eventHandler = noteOffEventHandler;
                                    break;

                                case NoteOnCmd:
                                    eventHandler = noteOnEventHandler;
                                    break;

                                case PolyphonicAftertouchCmd:
                                    eventHandler = polyphonicAftertouchEventHandler;
                                    break;

                                case ControlChangeCmd:
                                    eventHandler = controlChangeEventHanlder;
                                    break;

                                case PitchBendCmd:
                                    eventHandler = pitchEventHandler;
                                    break;

                                default:
                                    // no valid command, just wait for the next header
                                    numBytesToWaitFor = 0;
                                    break;
                            }
                        }
                    }
                }
            }

                // Waiting for all kinds of 1 or 2 Byte messages
            else if (numBytesToWaitFor > 0) {

                midiDataBuffer[numBytesInMidiDataBuffer] = serialInterface.read();
                numBytesToWaitFor--;
                if (numBytesToWaitFor == 0) {
                    // when all bytes are read, invoke the event handler
                    eventHandler (this);
                    continue;
                }
                numBytesInMidiDataBuffer++;

                // now the packet should be ready to process
            }

                // waiting for a SysEx to complete. With each byte received the negative numBytesToWaitFor value
                // is decreased by one
            else {
                midiDataBuffer[-numBytesToWaitFor] = serialInterface.read();
                if (midiDataBuffer[-numBytesToWaitFor] == SysExEnd) {
                    // now the end of a SysEx was reached
                    numBytesToWaitFor--;
                    numBytesInMidiDataBuffer = -numBytesToWaitFor;
                    receivedSysEx (midiDataBuffer, numBytesInMidiDataBuffer);
                    numBytesToWaitFor = 0;
                    continue;
                }

                numBytesToWaitFor--;
                if (-numBytesToWaitFor == midiDataBufferSize) {
                    // the buffer is full, drop it and notify the receiver
                    droppedSysExBuffer();
                    numBytesToWaitFor = 0;
                }
            }

        }

        digitalWrite(6, HIGH);
    }


    /**
     * This gets called if a SysEx was received that didn't fit into the midiDataBuffer and therefore was dropped.
     * It's only goal is to notify the receiver that a message was dropped
     */
    virtual void droppedSysExBuffer() {};

    // had to remove all "override" statements due to old c++ compiler compatibility

    inline RetValue sendNote (uint8_t note, uint8_t velocity, bool onOff) {
        return sendNote (note, velocity, onOff, sendChannel);
    };

    RetValue sendNote (uint8_t note, uint8_t velocity, bool onOff, Channel channel) { //override
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

        serialInterface.write (dataToSend, 3);

        return NoErrorCheckingForSpeedReasons;
    };

    inline RetValue sendAftertouchEvent (uint8_t note, uint8_t velocity) {
        return sendAftertouchEvent (note, velocity, sendChannel);
    };

    RetValue sendAftertouchEvent (uint8_t note, uint8_t velocity, Channel channel) { //override

        // check if it is a monophonic aftertouch
        if (note == MonophonicAftertouch) {
            uint8_t dataToSend[2];
            dataToSend[0] = MonophonicAftertouchCmd << 4 | channel;
            dataToSend[1] = velocity;
            serialInterface.write (dataToSend, 2);
        }

        else {
            uint8_t dataToSend[3];
            dataToSend[0] = PolyphonicAftertouchCmd << 4 | channel;
            dataToSend[1] = note;
            dataToSend[2] = velocity;
            serialInterface.write (dataToSend, 3);
        }

        return NoErrorCheckingForSpeedReasons;
    };

    inline RetValue sendControlChange (uint8_t control, uint8_t value) {
        return sendControlChange (control, value, sendChannel);
    };

    RetValue sendControlChange (uint8_t control, uint8_t value, Channel channel) { // override

        //Create MIDI Data
        uint8_t dataToSend[3];
        dataToSend[0] = ControlChangeCmd << 4 | channel;
        dataToSend[1] = control;
        dataToSend[2] = value;

        //Send MIDI Data
        serialInterface.write (dataToSend, 3);

        return NoErrorCheckingForSpeedReasons;
    };

    inline RetValue sendProgramChange (uint8_t program) {
        return sendProgramChange (program, sendChannel);
    };

    RetValue sendProgramChange (uint8_t program, Channel channel) { // override

        //Create MIDI Data
        uint8_t dataToSend[2];
        dataToSend[0] = ProgrammChangeCmd << 4 | channel;
        dataToSend[1] = program;

        //Send MIDI Data
        serialInterface.write (dataToSend, 2);

        return NoErrorCheckingForSpeedReasons;
    };

    // todo: check if this works as expected
    RetValue sendPitchBend (int16_t pitch) { // override

        // the input value is biased arround 0, so an offset has to be applied to get it into the range
        // from 0 - 2^14 as the "wire format" defines
        int16_t offset = 1 << 13;

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

        serialInterface.write (dataToSend, 3);

        return NoErrorCheckingForSpeedReasons;
    };

    //SysEx Messages must be framed by SYSEX_BEGIN and SYSEX_END
    RetValue sendSysEx (const uint8_t *sysExBuffer, uint16_t length) { // override
        if (sysExBuffer[0] == SysExBegin) {
            if (sysExBuffer[length - 1] == SysExEnd) {
                serialInterface.write (sysExBuffer, length);
                return Success;
            }
            return MissingSysExEnd;
        }
        return MissingSysExStart;
    };

    RetValue sendMIDITimecodeQuarterFrame (uint8_t quarterFrame) { // override

        //Create MIDI Data
        uint8_t dataToSend[2];
        dataToSend[0] = MIDITimecodeQuarterFrame;
        dataToSend[1] = quarterFrame;

        //Send MIDI Data
        serialInterface.write (dataToSend, 2);

        return NoErrorCheckingForSpeedReasons;
    };

    RetValue sendMIDISongPositionPointer (uint16_t positionInBeats) { // override

        // lower 7 bit
        const uint16_t lsb = positionInBeats & 0x7F;
        // upper 7 bit
        const uint16_t msb = positionInBeats >> 7;

        uint8_t dataToSend[3];
        dataToSend[0] = SongPositionPointerCmd;
        dataToSend[1] = lsb;
        dataToSend[2] = msb;

        serialInterface.write (dataToSend, 3);

        return NoErrorCheckingForSpeedReasons;
    };

    RetValue sendSongSelect (uint8_t songToSelect) { // override

        //Create MIDI Data
        uint8_t dataToSend[2];
        dataToSend[0] = SongSelectCmd;
        dataToSend[1] = songToSelect;

        //Send MIDI Data
        serialInterface.write (dataToSend, 2);

        return NoErrorCheckingForSpeedReasons;
    };

    void sendTuneRequest() { // override
        serialInterface.write (TuneRequest);
    };

    void sendMIDIClockTick() { // override
        serialInterface.write (ClockTickCmd);
    };

    void sendMIDIStart() { // override
        serialInterface.write (StartCmd);
    };

    void sendMIDIStop() { // override
        serialInterface.write (StopCmd);
    };

    void sendMIDIContinue() { // override
        serialInterface.write (ContinueCmd);
    };

    void sendActiveSense() { // override
        serialInterface.write (ActiveSense);
    };

    void sendReset() { // override
        serialInterface.write (MIDIReset);
    }

private:

    // The Serial interface used for physical MIDI I/O
    ArduinoSerialMIDIDeviceRessource &serialInterface;
    enum InterfaceType : uint8_t {
        HardwareSerialInterface,
        SoftwareSerialInterface
    };
    InterfaceType serialInterfaceType;

    // Everything needed to handle the incoming data
    static const int midiDataBufferSize = 256;
    uint8_t midiDataBuffer[midiDataBufferSize];
    uint8_t numBytesInMidiDataBuffer;
    const uint8_t &header;
    int8_t numBytesToWaitFor;

    // When a midi command is received, the header is parsed to find out which kind of
    // midi message came in. If the message still needs additional bytes to finish, but
    // the parsing shouldn't be repeated when all bytes are there to find the appropriate
    // handler member function, a function pointer to one of these event handlers is stored
    // to directly invoke the handler function when the message is complete
    typedef void (*EventHandler)(ArduinoSerialMIDIWrapper*);
    EventHandler eventHandler;

    static void noteOnEventHandler (ArduinoSerialMIDIWrapper *ref) {
        ref->receivedNote (ref->midiDataBuffer[1], ref->midiDataBuffer[2], NoteOn);
    };

    static void noteOffEventHandler (ArduinoSerialMIDIWrapper *ref) {
        ref->receivedNote (ref->midiDataBuffer[1], ref->midiDataBuffer[2], NoteOff);
    };

    static void polyphonicAftertouchEventHandler (ArduinoSerialMIDIWrapper *ref) {
        ref->receivedAftertouch (ref->midiDataBuffer[0], ref->midiDataBuffer[1]);
    };

    static void monophonicAftertouchEventHandler (ArduinoSerialMIDIWrapper *ref) {
        ref->receivedAftertouch (ref->midiDataBuffer[0], MonophonicAftertouch);
    };

    static void controlChangeEventHanlder (ArduinoSerialMIDIWrapper *ref) {
        ref->receivedControlChange (ref->midiDataBuffer[0], ref->midiDataBuffer[1]);
    }

    static void programChangeEventHandler (ArduinoSerialMIDIWrapper *ref) {
        ref->receivedProgramChange (ref->midiDataBuffer[0]);
    }

    static void pitchEventHandler (ArduinoSerialMIDIWrapper *ref) {
        // todo: implement
    }

    static void timecodeQuarterFrameEventHandler (ArduinoSerialMIDIWrapper *ref) {
        ref->receivedMIDITimecodeQuarterFrame (ref->midiDataBuffer[0]);
    }

    static void songSelectEventHandler (ArduinoSerialMIDIWrapper *ref) {
        ref->receivedSongSelect (ref->midiDataBuffer[0]);
    }

    static void songPositionPointerEventHandler (ArduinoSerialMIDIWrapper *ref) {
        // todo: implement
    }

};


#endif
