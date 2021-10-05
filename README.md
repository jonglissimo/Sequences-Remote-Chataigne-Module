# Sequence Remote Chataigne Module

This is a Chataigne module that enables control of sequences via OSC.
It's OSC API is compatible with the included M5StickC Plus Arduino firmware.
This way an M5Stick can be used to control (play, pause, stop, go to previous cue) Chataigne sequences.

## Setup

1. Upload the firmware to the M5StickC Plus
    * Install M5Stick dependencies: https://docs.m5stack.com/en/arduino/arduino_development
    * Dependencies:
        * ESP-IDF - 1.0.6 - https://github.com/espressif/arduino-esp32
        * OSC - 1.3.5 - https://github.com/CNMAT/OSC
        * Change ssid and password at the top of the code to your wifi credentials
    * Upload firmware

2. Install module from Chataigne community manager
3. Create module 
4. Press "Detect Remotes", this should create the correct output to your remote

## OSC API

* Input Port: 12034
* Output Port: 12033 (going to Remote)
* Commands (input from remote)
    * /play: play selected sequence
    * /pause: pause selected sequence
    * /togglePlay: toggle play selected sequence
    * /stop: stop selected sequence
    * /stopAll: stop all sequences
    * /next: select next sequence index
    * /prev: select previous sequence index
    * /prevCue: go to previous cue of selected sequence (relative to current time)
    * /wassup (string: IP of remote): response from remote to broadcasted /yo from module

* Commands (going to remote):
    * /status (integer: sequence index, string: name of sequence, float: current time, boolean: isPlaying)
    * /yo (string: ip of chataigne computer): Broadcast search for remotes