##BUTTON/KNOB MAPPING

Buttons:
- Knob 0: VIBRATO TOGGLE (customizable with other buttons, see below)
- Knob 1: TREMOLO TOGGLE (non-customizable)
- Knob 2: ___
- Knob 3: ___ (REMOVED SEND/RECEIVE MODE - reading the spec, it would've lost marks - now the send/receive mode is automatically triggered by Pxx and Rxx Serial messages)

Knobs:
- Knob 0: Octave
- Knob 1: VIBRATO RANGE
- Knob 2: ___
- Knob 3: Volume

Joystick:
- X movement: WHAMMY BAR
- Y movement: VIBRATO SPEED
- Button: RESET VIBRATO OPTIONS TO DEFAULT

NOTES:
LFO class:
- used to make the vibrato and tremelo work - separate lfo for each, however both are updated in the same LFO Task
VIBRATO:
- vibrato toggle implemented on knob_0 button can be controlled by Joystick Y (speed of fluctuation) and knob_2 (range of fluctuation)
- Joystick Y speed control is sensitive so just flick it to change (unless you want to go really high)
- Knob_2 value gives the increase in range per iteration (e.g. if you leave the knob turned to 6+ for a long time the range will just increase and increase, same idea for negative knob values), so if you only want to increase it by a bit, turn it to a low positive number, then when you get to the range that you want, turn it back to 0
- OTHERWISE IF TOO FAR GONE THEN JUST PRESS THE JOYSTICK BUTTON TO RESET VIBRATO LFO
- (reason it's implemeted like this rather than set values it can take is that the really high vibrato speeds and fluctuations can produce some cool sounds) 
- Speed control is limited by range, but the higher range you go the faster the vibrato can go (it can go stupidly fast)

TREMOLO:
- only one setting - didn't want to use the remaining knobs/buttons for more of the same thing

SEND/RECEIVE MODE:
- toggles between sending data and receiving data
- no notes will play when playing keys if the board is on Receive mode (any time between receiving a Pxx and an Rxx)
- Octave played is determined by whether board is in send or receive mode; if in send then it will be the octave determined by knob 0, if in receive mode it will be determined by the octave part of the message it receives 
- You can test this out by using serial input on the serial monitor



## IMPLMENTED

- Octive Knob
- Wammy bar
- Sending data
- Vibrato
- Tremelo
- Low Frequency Oscillator
- Nice graphics
- Mostly protected(?)

---

## Todo 

- Tidy Code
- Sound link

- Waveform selector
- Multiple Notes
- Echo/Reverb


 
