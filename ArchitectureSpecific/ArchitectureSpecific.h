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
#define SIMPLE_MIDI_MAC
#define SIMPLE_MIDI_MULTITHREADED
#include "MacOSX/CoreMIDIWrapperDef.h"

#elif defined _WIN32 //Windows specific implementation
#define SIMPLE_MIDI_WINDOWS
#define SIMPLE_MIDI_MULTITHREADED
#elif defined ARDUINO //Arduino specific implementation
#define SIMPLE_MIDI_ARDUINO
#endif

#endif /* ArchitectureSpecific_h */
