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


typedef enum : uint8_t {
    MIDIChannel1 = 0,
    MIDIChannel2 = 1,
    MIDIChannel3 = 2,
    MIDIChannel4 = 3,
    MIDIChannel5 = 4,
    MIDIChannel6 = 5,
    MIDIChannel7 = 6,
    MIDIChannel8 = 7,
    MIDIChannel9 = 8,
    MIDIChannel10 = 9,
    MIDIChannel11 = 10,
    MIDIChannel12 = 11,
    MIDIChannel13 = 12,
    MIDIChannel14 = 13,
    MIDIChannel15 = 14,
    MIDIChannel16 = 15,
    MIDIChannelAny = 16
} MIDIChannel_t;


// Abstract base class for all architecture specific implementations
class SimpleMIDIAbstractBaseClass {
public:
// ----------- These member functions are implemented by the architecture specific implementations ------------
    virtual ~SimpleMIDIAbstractBaseClass() {};
    // set send and receive channels
    virtual int setSendChannel (MIDIChannel_t sendChannel) = 0;
    virtual int setReceiveChannel (MIDIChannel_t receiveChannel) = 0;
    // send MIDI Messages
    virtual int sendNote (uint8_t note, uint8_t velocity, bool onOff) = 0;
    virtual int sendControlChange (uint8_t control, uint8_t value) = 0;
    virtual int sendProgramChange (uint8_t program) = 0;
    // !! SysEx Messages must be framed by SYSEX_BEGIN and SYSEX_END
    virtual int sendSysEx(uint8_t *sysExBuffer, uint32_t length) = 0;
    
// ----------- These member functions handling incomming data are needed to be implemented by the user --------
// ----------- They are called from the architecture specific implementation if incomming data is available ---
    
    virtual void receivedNote (uint8_t note, uint8_t velocity, bool onOff) = 0;
    virtual void receivedControlChange (uint8_t control, uint8_t value) = 0;
    virtual void receivedProgrammChange (uint8_t programm) = 0;
    // !! The sysExBuffer probably goes out of scope when the member function returns
    virtual void receivedSysEx (uint8_t *sysExBuffer, uint8_t length) = 0;

};

#include "ArchitectureSpecific/ArchitectureSpecific.h"

#endif /* simpleMIDI_h */
