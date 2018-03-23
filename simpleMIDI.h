//
//  simpleMIDI.h
//
//
//  Created by Janos Buttgereit on 22.05.17.
//
//

#ifndef simpleMIDI_h
#define simpleMIDI_h



#include "ArchitectureSpecific/ArchitectureSpecific.h"

// Abstract base class for all architecture specific implementations
class SimpleMIDI {
public:
    
    static const bool NoteOn = true;
    static const bool NoteOff = false;
    
    //=========================================================================
    // All possible MIDI commands, the standard describes
    // 4 Bit commands
    static const uint8_t NoteOffCmd =               0b1000; // 2 data Bytes
    static const uint8_t NoteOnCmd =                0b1001; // 2 data Bytes
    static const uint8_t PolyphonicAftertouchCmd =  0b1010; // 2 data Bytes
    static const uint8_t ControlChangeCmd =         0b1011; // 2 data Bytes
    static const uint8_t ProgrammChangeCmd =        0b1100; // 1 data Byte
    static const uint8_t MonophonicAftertouchCmd =  0b1101; // 1 data Byte
    static const uint8_t PitchBendCmd =             0b1110; // 2 data Bytes
    // --> a command with a form of 0b110xxxxx will have only one data byte
    // 8 Bit commands
    static const char SysExBegin =               0b11110000; // x data Bytes
    static const uint8_t MIDITimecodeQuarterFrame = 0b11110001; // 1 data Byte
    static const uint8_t SongPositionPointerCmd =   0b11110010; // 2 data Bytes
    static const uint8_t SongSelectCmd =            0b11110011; // 1 data Byte
    //  0b11110100 undefined
    //  0b11110101 undefined
    static const uint8_t TuneRequest =              0b11110110; // 0 data Bytes
    static const char SysExEnd =                 0b11110111; // 0 data Bytes
    static const uint8_t ClockTickCmd =             0b11111000; // 0 data Bytes
    //  0b11111001 undefined
    static const uint8_t StartCmd =                 0b11111010; // 0 data Bytes
    static const uint8_t ContinueCmd =              0b11111011; // 0 data Bytes
    static const uint8_t StopCmd =                  0b11111100; // 0 data Bytes
    //  0b11111101 undefined
    static const uint8_t ActiveSense =              0b11111110; // 0 data Bytes
    static const uint8_t MIDIReset =                0b11111111;
    
    
    
    enum Channel : uint8_t {
        Channel1 = 0,
        Channel2 = 1,
        Channel3 = 2,
        Channel4 = 3,
        Channel5 = 4,
        Channel6 = 5,
        Channel7 = 6,
        Channel8 = 7,
        Channel9 = 8,
        Channel10 = 9,
        Channel11 = 10,
        Channel12 = 11,
        Channel13 = 12,
        Channel14 = 13,
        Channel15 = 14,
        Channel16 = 15,
        ChannelAny = 16
    };
    
    
    // ----------- These member functions are implemented by the architecture specific implementations ------------
    virtual ~SimpleMIDI() {};
    
    enum RetValue : int8_t {
        NoErrorCheckingForSpeedReasons = 1,
        Success = 0,
        FirstArgumentOutOfRange = -1,
        SecondArgumentOutOfRange = -2,
        ThirdArgumentOutOfRange = -3,
        FourthArgumentOutOfRange = -4,
        MissingSysExStart = -5,
        MissingSysExEnd = -6
    };
    
    // send MIDI Messages
    virtual RetValue sendNote (uint8_t note, uint8_t velocity, bool onOff) = 0;
    virtual RetValue sendNote (uint8_t note, uint8_t velocity, bool onOff, Channel channel) = 0;
    virtual RetValue sendAftertouchEvent (uint8_t note, uint8_t velocity) = 0;
    virtual RetValue sendAftertouchEvent (uint8_t note, uint8_t velocity, Channel channel) = 0;
    virtual RetValue sendControlChange (uint8_t control, uint8_t value) = 0;
    virtual RetValue sendControlChange (uint8_t control, uint8_t value, Channel channel) = 0;
    virtual RetValue sendProgramChange (uint8_t program) = 0;
    virtual RetValue sendProgramChange (uint8_t program, Channel channel) = 0;
    virtual RetValue sendPitchBend (int16_t pitch) = 0;
    // !! SysEx Messages must be framed by SYSEX_BEGIN and SYSEX_END
    virtual RetValue sendSysEx (const char *sysExBuffer, uint16_t length) = 0;
    virtual RetValue sendMIDITimecodeQuarterFrame (uint8_t quarterFrame) = 0;
    virtual RetValue sendMIDISongPositionPointer (uint16_t positionInBeats) = 0;
    virtual RetValue sendSongSelect (uint8_t songToSelect) = 0;
    virtual void sendTuneRequest() = 0;
    virtual void sendMIDIClockTick() = 0;
    virtual void sendMIDIStart() = 0;
    virtual void sendMIDIStop() = 0;
    virtual void sendMIDIContinue() = 0;
    virtual void sendActiveSense() = 0;
    virtual void sendReset() = 0;

    /**
     * Sends out a raw buffer of bytes over the MIDI output. The caller must gurantee that this is a valid
     * MIDI command.
     */
    virtual void sendRawMIDIBuffer (uint8_t *bytesToSend, int length) = 0;
    

    /**
     * Send an NRPN message with a high resolution 14 bit value. Parameter and value will be split
     * into MSB and LSB parts. Valid range for both parameter and value are 0 to 16383.
     */
    RetValue sendHiResNRPN (uint16_t parameter, uint16_t value) {
        if (parameter > 0b0011111111111111)
            return FirstArgumentOutOfRange;
        if (value > 0b0011111111111111)
            return SecondArgumentOutOfRange;

        sendHiResNRPN (parameter >> 7, parameter & 0b01111111, value >> 7, value & 0b01111111);

        return Success;
    }

    /**
     * Send an NRPN message with a high resolution 14 bit value. The parameter is passed as MSB and LSB
     * seperately with a range from 0 to 127 for both arguments while the value is passed as a single
     * 14 Bit value with a range from 0 to 16383.
     */
    RetValue sendHiResNRPN (uint8_t parameterMSB, uint8_t parameterLSB, uint16_t value) {
        if (value > 0b0011111111111111)
            return ThirdArgumentOutOfRange;

        return sendHiResNRPN (parameterMSB, parameterLSB, value >> 7, value & 0b01111111);
    }

    /**
     * Send an NRPN message with a high resolution 14 bit value. The parameter and value are passed as
     * MSB and LSB seperately with a range from 0 to 127 for each argument.
     */
    RetValue sendHiResNRPN (uint8_t parameterMSB, uint8_t parameterLSB, uint8_t valueMSB, uint8_t valueLSB) {
        if (parameterMSB > 0b01111111)
            return FirstArgumentOutOfRange;
        if (parameterLSB > 0b01111111)
            return SecondArgumentOutOfRange;
        if (valueMSB > 0b01111111)
            return ThirdArgumentOutOfRange;
        if (valueLSB > 0b01111111)
            return FourthArgumentOutOfRange;

        uint8_t sequence[12] = {ControlChangeCmd, 99, parameterMSB, ControlChangeCmd, 98, parameterLSB,
                                ControlChangeCmd, 6, valueMSB, ControlChangeCmd, 38, valueLSB};

        sendRawMIDIBuffer (sequence, 12);

        return Success;
    }

    /**
     * Send an NRPN message with a low resolution 7 bit value. The parameter is passed as a 14 Bit value
     * with a range from 0 - 16383 while the value is an 8 bit value with a range from 0 - 127.
     */
    RetValue sendLoResNRPN (uint16_t parameter, uint8_t value) {
        if (parameter > 0b0011111111111111)
            return FirstArgumentOutOfRange;

        RetValue r = sendLoResNRPN (parameter >> 7, parameter & 0b01111111, value);

        if (r == ThirdArgumentOutOfRange)
            return SecondArgumentOutOfRange;

        return Success;
    }

    /**
     * Send an NRPN message with a low resolution 7 bit value. The parameter is passed as MSB and LSB
     * separately. The valid range for all three arguments is 0 - 127.
     */
    RetValue sendLoResNRPN (uint8_t parameterMSB, uint8_t parameterLSB, uint8_t value) {
        if (parameterMSB > 0b01111111)
            return FirstArgumentOutOfRange;
        if (parameterLSB > 0b01111111)
            return SecondArgumentOutOfRange;
        if (value > 0b01111111)
            return ThirdArgumentOutOfRange;

        uint8_t sequence[9] = {ControlChangeCmd, 99, parameterMSB, ControlChangeCmd, 98, parameterLSB,
                               ControlChangeCmd, 6, value};

        sendRawMIDIBuffer (sequence, 9);

        return Success;
    }

    /**
     * Sets the channel all outgoing messages that use a channel (note, aftertouch, control change, program change
     * and pitch bend) use if they are called without a channel argument.
     * @return  true if the channel was inside the valid range, false otherwise
     */
    bool setSendChannel (Channel sendChannel) {
        if (sendChannel > Channel16) {
            return false;
        }
        this->sendChannel = sendChannel;
        return true;
    };
    
    /**
     * Returns the channel that all outgoing messages are sent to if no channel is provided with the call.
     */
    Channel getSendChannel() {
        
        return sendChannel;
    }
    
    
    /**
     * Sets the channel all incomming messages that use a channel (note, aftertouch, control change, program change
     * and pitch bend) react to. If you set a particular receive channel here, messages from a different channel
     * will simply be ignored and won't invoke any callback. If you want to react to messages from all channels
     * set it to ChannelAny. This is also the default value. In this case a call to getMostRecentSourceChannel will
     * return the channel of the last message that came in.
     *
     * @return  true if the channel was inside the valid range, false otherwise
     *
     * @see getMostRecentSourceChannel
     */
    bool setReceiveChannel (Channel receiveChannel) {
        
        if (receiveChannel <= ChannelAny) {
            this->receiveChannel = receiveChannel;
            if (receiveChannel != ChannelAny) {
                lastChannel = receiveChannel;
            }
            return true;
        }
        return false;
    };
    
    /**
     * Returns the channel that's currently used for receiving. In case of ChannelAny getMostRecentSourceChannel() could
     * be helpful.
     *
     * @see getMostRecentSourceChannel
     */
    Channel getReceiveChannel() {
        
        return receiveChannel;
    }
    
    /**
     * Returns the channel the last incomming message that used a channel (note, aftertouch, control change, program change
     * and pitch bend) was sent from. The main usage of this function is when you want to determine where a message came
     * from when the receive channel is set to ChannelAny. If you need this information, it's a good idea to call this
     * right at the beginning of the receivedXYZ() callback.
     *
     * @see getReceiveChannel
     */
    Channel getMostRecentSourceChannel() {
        return lastChannel;
    }
    
    // ----------- These member functions handling incomming data are needed to be implemented by the user --------
    // ----------- They are called from the architecture specific implementation if incomming data is available ---
    
    /**
     * Gets called when a note on the specified MIDI Channel was received. The application has to override this
     * function if it wants to handle MIDI notes.
     * @param note      Number of the note that was played in the range from 0 - 127
     * @param velocity  Velocity in the range from 0 - 127
     * @param onOff     True if a note on was received, false if a note off was received
     *
     * @see SimpleMIDI::valueToFloat, SimpleMIDI::valueToDouble
     */
    virtual void receivedNote (uint8_t note, uint8_t velocity, bool onOff){};
    
    /**
     * Gets called when a Aftertouch event on the specified MIDI Channel was received. The application has to override
     * this function if it wants to handle Aftertouch events. If it is a monophonic aftertouch event, the note value
     * will be MonophonicAftertouch (=255)
     * @param note      Number of the note that this aftertouch event refers to in the range from 0 - 127 or
     *                  MonophonicAftertouch (=255) if it is a monophonic aftertouch event
     * @param velocity  elocity in the range from 0 - 127
     */
    virtual void receivedAftertouch (uint8_t note, uint8_t velocity){};
    
    /**
     * This value indicates, that the Aftertouch event was no polyphonic Aftertouch event, related to a special
     * note but a monophonic aftertouch event
     */
    static const uint8_t MonophonicAftertouch = 255;
    
    /**
     * Gets called when a Control Change on the specified MIDI Channel was received. The application has to override
     * this function if it wants to handle Control Change messages.
     * @param control   Number of the controller in the range from 0 - 127
     * @param value     Value of the controller in the range from 0 - 127
     *
     * @see SimpleMIDI::valueToFloat, SimpleMIDI::valueToDouble
     */
    virtual void receivedControlChange (uint8_t control, uint8_t value) {};
    
    /**
     * Gets called when a Programm Change on the specified MIDI Channel was received. The application has to override
     * this function if it wants to handle Program Change messages.
     * @param program   Number of the programm in the range from 0 - 127
     */
    virtual void receivedProgramChange (uint8_t program) {};
    
    /**
     * Gets called when a Pitch Bend on the specified MIDI Channel was received. The application has to override
     * this function if it wants to handle Pitch Bend messages.
     * @param pitch     Amount of pitch bend in the range from -8192 - 8192, with 0 = no pitch bend, 8192 = maximum
     *                  pitch up, -8192 maximum pitch down
     */
    virtual void receivedPitchBend (int16_t pitch) {};
    
    /**
     * Gets called when a SysEx was received. The application has to override this function if it wants to handle
     * SysEx messages. The sysExBuffer will go out of scope when the function returns, so make sure to copy its
     * content to an own buffer if needed!
     * @param sysExBuffer A pointer to the buffer containing the received bytes
     * @param length      The number of bytes received
     */
    virtual void receivedSysEx (const char *sysExBuffer, const uint16_t length) {};
    
    /**
     * Gets called when a MIDI timecode quarter frame was received. The application has to override this function if
     * it wants to handle MIDI timecode messages.
     * @param quarterFrame todo: what is a quarter frame? ;)
     */
    virtual void receivedMIDITimecodeQuarterFrame (uint8_t quarterFrame) {};
    
    /**
     * Gets called when a MIDI clock tick was received. Usually there are twenty-four ticks per quarter note.
     * The application has to override this function if it wants to handle MIDI clock ticks.
     */
    virtual void receivedMIDIClockTick() {};
    
    /**
     * Gets called when a Song Select command was received. The application has to override this function if
     * it wants to handle Song Select commands.
     * @param selectedSong The number of the selected song
     */
    virtual void receivedSongSelect (uint8_t selectedSong) {};
    
    /**
     * Gets called when a MIDI start event was received. The application has to override this function if it wants
     * to handle MIDI start events.
     */
    virtual void receivedMIDIStart(){};
    
    /**
     * Gets called when a MIDI stop event was received. The application has to override this function if it wants
     * to handle MIDI stop events.
     */
    virtual void receivedMIDIStop(){};
    
    /**
     * Gets called when a MIDI continue event was received. The application has to override this function if it
     * wants to handle MIDI continue events.
     */
    virtual void receivedMIDIContinue(){};
    
    /**
     * Gets called when a MIDI song-position-pointer event was received. The application has to override this
     * function if it wants to handle MIDI song position pointer events.
     */
    virtual void receivedSongPositionPointer (uint16_t positionInBeats){};
    
    
    virtual void receivedTuneRequest() {};
    
    
    virtual void receivedActiveSense() {};
    
    
    virtual void receivedMIDIReset() {};
    
    /**
     * Gets called when a MIDI command was received that is not one of the commands described above. The
     * application has to override this function if it wants to handle any other standard or non-standard
     * MIDI commands and has to provide its own parsing mechanism for the message received. The dataBuffer will
     * go out of scope when the function returns, so make sure to copy its content to an own buffer if needed!
     * @param dataBuffer        Buffer holding the unknown MIDI message, beginning with the MIDI command/header
     *                          byte, followed by the data bytes
     * @param numBytesAvailable Size of the dataBuffer
     */
    virtual void receivedUnknownCommand (uint8_t *dataBuffer, int numBytesAvailable){};
    
    /**
     * Handy helper function to convert a MIDI value ranging from 0 - 127 (e.g. controller value, velocity)
     * to a normalized single precision float value, ranging from 0.0 - 1.0
     */
    static float valueToFloat (uint8_t uint8Value) {
        return uint8Value / 127.0;
    };
    
    /**
     * Handy helper function to convert a MIDI value ranging from 0 - 127 (e.g. controller value, velocity)
     * to a normalized double precision float value, ranging from 0.0 - 1.0
     */
    static double valueToDouble (uint8_t uint8Value) {
        return uint8Value / 127.0;
    };
    
    /**
     * Handy helper function to convert a normalized single precision float value ranging from 0.0 - 1.0
     * (e.g. controller value, velocity) to a MIDI value, ranging from 0 - 127. If the value passed is
     * greater than 1.0, the output will be clipped to 127
     */
    static uint8_t valueToUint8 (float floatValue) {
        if (floatValue > 1.0) {
            return 127;
        }
        return floatValue * 127;
    }
    
    /**
     * Handy helper function to convert a normalized double precision float value ranging from 0.0 - 1.0
     * (e.g. controller value, velocity) to a MIDI value, ranging from 0 - 127. If the value passed is
     * greater than 1.0, the output will be clipped to 127
     */
    static uint8_t valueToUint8 (double doubleValue) {
        if (doubleValue > 1.0) {
            return 127;
        }
        return doubleValue * 127;
    }
    
    
#ifdef SIMPLE_MIDI_MAC
    typedef CoreMIDIDeviceRessource HardwareResource;
    typedef CoreMIDIWrapper PlatformSpecificImplementation;
#elif defined SIMPLE_MIDI_ARDUINO
    typedef ArduinoSerialMIDIWrapper PlatformSpecificImplementation;
    typedef ArduinoSerialMIDIWrapper ForArduino; // an alternative, easier to understand declaration for Arduino users
#endif
    
    
protected:
    Channel lastChannel;
    
#ifdef SIMPLE_MIDI_PRE_C++11
    // in this case these will be initialized in the derived class' constructor
    Channel sendChannel;
    Channel receiveChannel;
#else
    Channel sendChannel = Channel1;
    Channel receiveChannel = ChannelAny;
#endif
    
};






#ifdef SIMPLE_MIDI_MULTITHREADED
#include <thread>
#include <functional>
#include <mutex>
#include <chrono>
/**
 * A class that launches a timer that automatically sends out MIDI clock ticks on a regular basis. If you need
 * high precision better roll out your own version based on some high precision timer.
 */
class MIDIClockGenerator {
    
public:
    /**
     * Pass the midiConnection you want to use to send out the clock ticks to initialize the class
     * in an RAII style.
     */
    MIDIClockGenerator (SimpleMIDI &midiConnectionToAttachTo) : midiConnection (midiConnectionToAttachTo) {
        //nextTick = std::chrono::system_clock::now ();
        nextTick = std::chrono::steady_clock::now ();
        // lock the mutex before launching the thread so that it blocks until it was started
        timerThreadMutex.lock ();
        timerThread = new std::thread (std::bind (&MIDIClockGenerator::timerThreadWork, this));
    }
    
    ~MIDIClockGenerator () {
        timerThreadShouldExit = true;
        
        // if the timer thread is sleeping because the mutex blocks it, wake it up so that it can leave
        if (!timerIsActive) {
            timerThreadMutex.unlock ();
        }
        
        timerThread->join ();
        delete timerThread;
    }
    
    /**
     * Sets the intervall between the clock ticks and starts the timer if not already running.
     * @param intervallInMilliseconds   Intervall between the clock ticks in milliseconds. Keep in mind that
     *                                  the standard expects four ticks per quarter note
     * @param withStartCommand          Sends a MIDI start command before starting the continous clock tick if true
     * @param withContinueCommand       Sends a MIDI continue command before starting the continous clock tick if true
     */
    void setIntervall (uint64_t quarterNoteIntervallInMilliseconds, bool withStartCommand = false, bool withContinueCommand = false) {
        // convert to nanoseconds for higher precision
        quarterNoteIntervallInMilliseconds *= 1000000;
        // convert to the corresponding midi clock tick intervall
        quarterNoteIntervallInMilliseconds /= 24;
        
        // aquire the lock before changing the interval if the timer is already running
        if (timerIsActive)
            timerThreadMutex.lock ();
        
        interval = std::chrono::nanoseconds (quarterNoteIntervallInMilliseconds);
        
        if (withStartCommand)
            midiConnection.sendMIDIStart();
        if (withContinueCommand)
            midiConnection.sendMIDIContinue();
        
        timerIsActive = true;
        
        //releasing the mutex will start the thread
        timerThreadMutex.unlock ();
    }
    
    /**
     * Stops the continous clock ticks
     * @param withStopCommand           Sends a MIDI stop command after stopping the last clock tick
     */
    void stop (bool withStopCommand = false) {
        
        // aquire the lock, so that the timer thread will sleep.
        timerThreadMutex.lock ();
        timerIsActive = false;
        
        if (withStopCommand)
            midiConnection.sendMIDIStop();
    }
    
private:
    SimpleMIDI &midiConnection;
    std::thread *timerThread = nullptr;
    std::mutex timerThreadMutex;
    bool timerIsActive = false;
    bool timerThreadShouldExit = false;
    std::chrono::nanoseconds interval;
    std::chrono::steady_clock::time_point nextTick;
    
    void timerThreadWork() {
        // launch an endless loop
        while (true) {
            std::this_thread::sleep_until (nextTick);
            nextTick = std::chrono::steady_clock::now () + interval;
            
            // If the timer has been stopped, it will be sleeping at this
            // point until the mutex was released
            timerThreadMutex.lock ();
            if (timerThreadShouldExit)
                break;
            
            midiConnection.sendMIDIClockTick();
            timerThreadMutex.unlock ();
        }
    }
};


#endif



#ifdef SIMPLE_MIDI_MAC
#import "ArchitectureSpecific/MacOSX/CoreMIDIWrapperImpl.h"

#elif defined SIMPLE_MIDI_WINDOWS
// todo: Implement Windows specific MIDI Class
#error "Currently no Windows specific implementation of SimpleMIDI"

#elif defined SIMPLE_MIDI_ARDUINO
#include "ArchitectureSpecific/Arduino/ArduinoSerialMIDIWrapperImpl.h"

#else
#error "Unknown Plattfom for SimpleMIDI"

#endif

#endif /* simpleMIDI_h */

