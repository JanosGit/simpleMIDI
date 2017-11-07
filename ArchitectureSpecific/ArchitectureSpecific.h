//
//  ArchitectureSpecific.h
//  
//
//  Created by Janos Buttgereit on 22.05.17.
//
//

#ifndef ArchitectureSpecific_h
#define ArchitectureSpecific_h

#ifdef SIMPLE_MIDI_TTY
#define SIMPLE_MIDI_MULTITHREAD

#elif defined ARDUINO //Arduino specific implementation
#define SIMPLE_MIDI_ARDUINO
#include "Arduino/ArduinoSerialMIDIWrapperDef.h"
    
#elif defined __APPLE__ //OSX specific implementation
#define SIMPLE_MIDI_MAC
#define SIMPLE_MIDI_MULTITHREADED
#import "MacOSX/CoreMIDIWrapperDef.h"

#elif defined _WIN32 //Windows specific implementation
#define SIMPLE_MIDI_WINDOWS
#define SIMPLE_MIDI_MULTITHREADED
#endif

#endif /* ArchitectureSpecific_h */
