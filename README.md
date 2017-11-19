# simpleMIDI
A cross platform capable class for easy midi handling. It should target computer- and microcontroller (mainly Arduino/Wiring based) targets, especially to develop APIs for midi-controlled devices that could be used on computers as well as on microcontrollers.

I'm working on this from time to time, when there is some spare time.

At the moment, nearly all MIDI functions work for Apple's CoreMIDI and Arduino targets - take a look at the issues for known bugs. Next steps will be some bug fixing, detailled instructions and an implementation that uses a linux Serial interface for making it work with the Raspberry Pi's uart port.

If there are any Windows or Linux guys out there, that wanted to help with a Windows or Linux implementation, just let me know!

Among others, a main goal of this project is to form the basis of [kpapi](https://github.com/JanosGit/kpapi), a cross-plattform solution to abstract all functionality of a Kemper Profiling Amp connected via MIDI.
