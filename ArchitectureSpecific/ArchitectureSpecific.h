//
//  ArchitectureSpecific.h
//  
//
//  Created by Janos Buttgereit on 22.05.17.
//
//

#ifndef ArchitectureSpecific_h
#define ArchitectureSpecific_h
    
#ifdef __APPLE__ //OSX specific implementation
#import "MacOSX/CoreMIDIWrapper.h"
    typedef CoreMIDIWrapper SimpleMIDI;
namespace simpleMIDI {
    typedef CoreMIDIDeviceRessource HardwareRessource;
}

    
#elif defined _WIN32 //Windows specific implementation
    // todo: Implement Windows specific MIDI Class
#error "Currently no Windows specific implementation of SimpleMIDI
    
#elif defined ARDUINO //Arduino specific implementation
#include "Arduino/SerialMIDIWrapper.h"
    typedef SerialMIDIWrapper SimpleMIDI;
    
#else
#error "Unknown Plattfom for SimpleMIDI"
    
#endif





#endif /* ArchitectureSpecific_h */
