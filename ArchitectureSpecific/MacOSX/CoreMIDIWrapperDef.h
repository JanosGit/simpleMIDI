//
//  coreMIDIWrapper.h
//
//  Created by Janos Buttgereit on 30.10.16.
//  Copyright © 2016 Janos Buttgereit. All rights reserved.
//

#ifndef coreMidiWrapper_h
#define coreMidiWrapper_h

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>

#include <CoreMIDI/CoreMIDI.h>

class CoreMIDIWrapper;


//Groups a MIDIDeviceRef with its name
typedef struct {
    std::string deviceName;
    MIDIDeviceRef deviceRef;
} CoreMIDIDeviceRessource;

#endif /* coreMidiWrapper_h */
