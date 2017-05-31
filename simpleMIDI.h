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


// Constants
namespace simpleMIDI{
    
    const bool NoteOn = true;
    const bool NoteOff = false;
    
    const uint8_t NoteOnCmd = 0b1001;
    const uint8_t NoteOffCmd = 0b1000;
    const uint8_t ControlChangeCmd = 0b1011;
    const uint8_t ProgrammChangeCmd = 0b1100;
    const uint8_t SysExBegin = 0b11110000;
    const uint8_t SysExEnd = 0b11110111;
    
    
    typedef enum : uint8_t {
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
    } Channel;
}


// Abstract base class for all architecture specific implementations
class SimpleMIDIAbstractBaseClass {
    public:
    // ----------- These member functions are implemented by the architecture specific implementations ------------
    virtual ~SimpleMIDIAbstractBaseClass() {};
    // set send and receive channels
    virtual int setSendChannel (simpleMIDI::Channel sendChannel) = 0;
    virtual int setReceiveChannel (simpleMIDI::Channel receiveChannel) = 0;
    // send MIDI Messages
    virtual int sendNote (uint8_t note, uint8_t velocity, bool onOff) = 0;
    virtual int sendControlChange (uint8_t control, uint8_t value) = 0;
    virtual int sendProgramChange (uint8_t program) = 0;
    // !! SysEx Messages must be framed by SYSEX_BEGIN and SYSEX_END
    virtual int sendSysEx(const uint8_t *sysExBuffer, uint16_t length) = 0;
    
    // ----------- These member functions handling incomming data are needed to be implemented by the user --------
    // ----------- They are called from the architecture specific implementation if incomming data is available ---
    
    virtual void receivedNote (uint8_t note, uint8_t velocity, bool onOff) = 0;
    virtual void receivedControlChange (uint8_t control, uint8_t value) = 0;
    virtual void receivedProgrammChange (uint8_t programm) = 0;
    // !! The sysExBuffer probably goes out of scope when the member function returns
    virtual void receivedSysEx (const uint8_t *sysExBuffer, const uint16_t length) = 0;
    
    // ---------- If simpleMIDI::ChannelAny is used as the receive channel, these functions will be called --------
    // ---------- As you see, they just call the standard receive functions and store the channel they came from. -
    // ---------- The user can obtain them by calling getMostRecentSourceChannel(), however, the user -------------
    // ---------- might also override these functions and catch the channel directly ------------------------------
    void receivedNoteWithChannel (uint8_t note, uint8_t velocity, bool onOff, simpleMIDI::Channel channel) {
        lastChannel = channel;
        receivedNote (note, velocity, onOff);
    }
    void receivedControlChangeWithChannel (uint8_t control, uint8_t value, simpleMIDI::Channel channel) {
        lastChannel = channel;
        receivedControlChange (control, value);
    }
    void receivedProgrammChangeWithChannel (uint8_t programm, simpleMIDI::Channel channel){
        lastChannel = channel;
        receivedProgrammChange(programm);
    }
    
    const simpleMIDI::Channel getMostRecentSourceChannel() {
        return lastChannel;
    }
    
    protected:
    simpleMIDI::Channel lastChannel;
    
};

#include "ArchitectureSpecific/ArchitectureSpecific.h"



#endif /* simpleMIDI_h */
