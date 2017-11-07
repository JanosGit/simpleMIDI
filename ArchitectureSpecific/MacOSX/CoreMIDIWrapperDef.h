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


// Returns all coreMIDI devices currently available
const std::vector<CoreMIDIDeviceRessource> searchMIDIDevices () {
    std::vector<CoreMIDIDeviceRessource> allDevices;
    allDevices.clear ();

    int availableDeviceCount = (int) MIDIGetNumberOfDevices ();

    //Iterate through all devices
    int i;
    for (i = 0; i < availableDeviceCount; i++) {
        CoreMIDIDeviceRessource d;
        d.deviceRef = MIDIGetDevice (i);

        //Check if device is online
        int offline = 0;
        MIDIObjectGetIntegerProperty (d.deviceRef, kMIDIPropertyOffline, &offline);

        if (!offline) {
            //Get name of device
            CFStringRef deviceNameCFString;
            if (noErr != MIDIObjectGetStringProperty (d.deviceRef, kMIDIPropertyName, &deviceNameCFString)) {
                d.deviceName = std::string ("Error getting name of MIDI device") + std::to_string (i);
            } else {
                // convert CFString to std::string
                char *nameCStrPointer;

                //get char pointer
                nameCStrPointer = (char *) CFStringGetCStringPtr (deviceNameCFString, kCFStringEncodingASCII);

                // If a nullpointer is returned, no direct char buffer is available. A call to CFStringGetCString does the conversion
                if (nameCStrPointer == NULL) {
                    long int length = CFStringGetLength (deviceNameCFString);
                    char nameCStrBuffer[length];
                    CFStringGetCString (deviceNameCFString, nameCStrBuffer, length, kCFStringEncodingASCII);
                    d.deviceName = std::string (nameCStrBuffer);
                } else {
                    d.deviceName = std::string (nameCStrPointer);
                }
            };
            allDevices.push_back (d);
        };
    };
    return allDevices;
}

#endif /* coreMidiWrapper_h */
