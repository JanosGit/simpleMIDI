#ifndef ArduinoSerialMIDIWrapperDef_h
#define ArduinoSerialMIDIWrapperDef_h

#include "Arduino.h"

#ifdef ENERGIA_ARCH_MSP430
#define SIMPLE_MIDI_ARDUINO_NO_SOFT_SERIAL
#define SIMPLE_MIDI_PRE_C++11
#endif

#ifdef ENERGIA_ARCH_TIVAC
#define SIMPLE_MIDI_ARDUINO_NO_SOFT_SERIAL
#endif

// on arduino, a simple Serial interface is used for MIDI I/O
typedef Stream ArduinoSerialMIDIDeviceRessource;

class ArduinoSerialMIDIWrapper;

#endif
