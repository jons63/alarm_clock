# Alarm clock with mp3 song support
This project spawned when my sister was looking to switch out her phone for an alarm clock.
She said that she easily wakes up in the morning so the normal alarm sounds are to harsh and
she would instead like to have nature sound like the ones usually available in wake up light
alarm clocks. I did a modest effort in trying to find what she wanted but when I finally
found an alarm clock that had nature sounds and no light the sound was horrible.
And now im here after thinking "how hard can it be to make an alarm clock that sounds decent?".

## The display
Any seven segment display with four digits should work for an alarm clock. I went with the largest available one.

Below is the wiring for the display + one button for incrementing the value shown.

![display](res/alarm_clock_bb.png)

## Display with shift register
A seven segment display require a lot of pins to operate, twelve for a display with four digits. To get around this I use a shift register to only use seven pins on the arduino instead of twelve.
![display with shift register](res/display_with_shift_register.png)
But even when using a shift register the amount of digital pins on one Arduino Uno is not enough when adding the MP3 player and buttons. Also, when adding logic to handle button presses and the alarm functionality the update frequence of the display gets low enough for it to be visibly flickering. To combat this I will add another Arduino Uno that will only refresh the display and the second arduino will handle the logic. They will use i2c communication to change what time should be displayed and other functoin that might be needed.

## Dual Arduino Uno with i2c communication. 
i2c is an easy communication interface with good Arduino support from the Wire library. I copied a simple example of i2c communication between two Arduinos to test it out.
![Minimal Arduino i2c example](res/i2c_example.png)

