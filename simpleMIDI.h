//
//  simpleMIDI.h
//  
//
//  Created by Janos Buttgereit on 22.05.17.
//
//

#ifndef simpleMIDI_h
#define simpleMIDI_h

/* To use simpleMIDI do the following
 
- Roll out your own MIDI class, inheriting from public SimpleMIDI. The version fitting your architecture is selected automatically.
- Override the member functions handling received data and fill them with your code.
 Example:
 
 class MyMIDI : public simpleMIDI {
 public:
    void receivedNote (uint8_t note, uint8_t velocity, bool noteOn) override {
        if (noteOn) {
            std::cout << "Received note on " << note << " with velocity " << velocity << std::endl;
        }
        else {
            std::cout << "Received note off " << note << " with velocity " << velocity << std::endl;
        }
    };
 
    void receivedControlChange (uint8_t control, uint8_t value) override {
        // do something with your control change
        // ...
    };
 
    void receivedProgramChange (uint8_t program) override {
        // do something with your program change
        // ...
    };
 
    void receivedSysEx (uint8_t *sysExBuffer, uint8_t length) override {
        // consider copying the sysExBuffer content to another memory location here
        // as it goes out of scope as soon as this function returns
    };
 
 private:
    int myPrivateMemberVariable; //whatever you will need it for...
 }
 
 
 */

#include "ArchitectureSpecific/ArchitectureSpecific.h"

// Abstract base class for all architecture specific implementations
class SimpleMIDI {
    public:

    static const bool NoteOn = true;
    static const bool NoteOff = false;

    // todo: change all to hex values
    // 4 Bit commands
    static const uint8_t NoteOnCmd = 0b1001;
    static const uint8_t NoteOffCmd = 0b1000;
    static const uint8_t MonophonicAftertouchCmd = 0xD;
    static const uint8_t PolyphonicAftertouchCmd = 0xA;
    static const uint8_t ControlChangeCmd = 0b1011;
    static const uint8_t ProgrammChangeCmd = 0b1100;
    static const uint8_t PitchBendCmd = 0xE;
    // 8 Bit commands
    static const uint8_t SysExBegin = 0b11110000;
    static const uint8_t SysExEnd = 0b11110111;
    static const uint8_t ClockTickCmd = 0xF8;
    static const uint8_t StartCmd = 0xFA;
    static const uint8_t StopCmd = 0xFC;
    static const uint8_t ContinueCmd = 0xFB;
    static const uint8_t SongPositionPointerCmd = 0xF2;


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
    // set send and receive channels
    virtual int setSendChannel (Channel sendChannel) = 0;
    virtual int setReceiveChannel (Channel receiveChannel) = 0;
    // send MIDI Messages
    virtual int sendNote (uint8_t note, uint8_t velocity, bool onOff) = 0;
    virtual int sendAftertouchEvent (uint8_t note, uint8_t velocity) = 0;
    virtual int sendControlChange (uint8_t control, uint8_t value) = 0;
    virtual int sendProgramChange (uint8_t program) = 0;
    virtual int sendPitchBend (int16_t pitch) = 0;
    // !! SysEx Messages must be framed by SYSEX_BEGIN and SYSEX_END
    virtual int sendSysEx(const uint8_t *sysExBuffer, uint16_t length) = 0;
    virtual void sendMIDIClockTick() = 0;
    virtual void sendMIDIStart() = 0;
    virtual void sendMIDIStop() = 0;
    virtual void sendMIDIContinue() = 0;
    virtual int sendMIDISongPositionPointer (uint16_t positionInBeats) = 0;
    
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
    virtual void receivedSysEx (const uint8_t *sysExBuffer, const uint16_t length) {};

    /**
     * Gets called when a MIDI clock tick was received. Usually there are twenty-four ticks per quarter note.
     * The application has to override this function if it wants to handle MIDI clock ticks.
     */
    virtual void receivedMIDIClockTick() {};

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
    
    // ---------- If simpleMIDI::ChannelAny is used as the receive channel, these functions will be called --------
    // ---------- As you see, they just call the standard receive functions and store the channel they came from. -
    // ---------- The user can obtain them by calling getMostRecentSourceChannel(), however, the user -------------
    // ---------- might also override these functions and catch the channel directly ------------------------------
    void receivedNoteWithChannel (uint8_t note, uint8_t velocity, bool onOff, Channel channel) {
        lastChannel = channel;
        receivedNote (note, velocity, onOff);
    }
    void receivedAftertouchWithChannel (uint8_t note, uint8_t velocity, Channel channel) {
        lastChannel = channel;
        receivedAftertouch(note, velocity);
    }
    void receivedControlChangeWithChannel (uint8_t control, uint8_t value, Channel channel) {
        lastChannel = channel;
        receivedControlChange (control, value);
    }
    void receivedProgramChangeWithChannel (uint8_t programm, Channel channel){
        lastChannel = channel;
        receivedProgramChange (programm);
    }
    void receivedPitchBendWithChannel (int16_t pitch, Channel channel) {
        lastChannel = channel;
        receivedPitchBend (pitch);
    }
    
    const Channel getMostRecentSourceChannel() {
        return lastChannel;
    }

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
    typedef CoreMIDIDeviceRessource HardwareRessource;
    typedef CoreMIDIWrapperDef PlatformSpecificImplementation;
#endif
protected:
    Channel lastChannel;
    
};

#ifdef SIMPLE_MIDI_MULTITHREADED
#include <thread>
#include <functional>
#include <mutex>
#include <chrono>
/**
 * A class that launches a timer that automatically sends out MIDI clock ticks on a regular basis
 */
class MIDIClockGenerator {

public:
    /**
     * Pass the midiConnection you want to use to send out the clock ticks to initialize the class
     * in an RAII style.
     */
    MIDIClockGenerator (SimpleMIDI &midiConnectionToAttachTo) : midiConnection (midiConnectionToAttachTo) {
        nextTick = std::chrono::system_clock::now ();
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
    void setIntervall (int intervallInMilliseconds, bool withStartCommand = false, bool withContinueCommand = false) {

        // aquire the lock before changing the interval if the timer is already running
        if (timerIsActive)
            timerThreadMutex.lock ();

        interval = std::chrono::milliseconds (intervallInMilliseconds);

        if (withStartCommand)
            midiConnection.sendMIDIStart();
        if (withContinueCommand)
            midiConnection.sendMIDIContinue();

        timerIsActive = true;

        //releasing the mutex will stop the thread
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
    std::chrono::milliseconds interval;
    std::chrono::system_clock::time_point nextTick;

    void timerThreadWork() {
        // launch an endless loop
        while (true) {
            std::this_thread::sleep_until (nextTick);
            nextTick = std::chrono::system_clock::now () + interval;

            // If the timer has been stopped, it will be sleeping at this
            // point until the mutex was released
            timerThreadMutex.lock ();
            if (timerThreadShouldExit)
                break;

            midiConnection.sendMIDIClockTick();
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
#include "Arduino/SerialMIDIWrapper.h"
    typedef SerialMIDIWrapper SimpleMIDI;

#else
#error "Unknown Plattfom for SimpleMIDI"

#endif

#endif /* simpleMIDI_h */
