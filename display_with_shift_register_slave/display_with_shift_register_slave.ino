#include <TimeLib.h>
#include <Wire.h>
#include "SevSegShift.h"

#define SHIFT_PIN_DS   2 /* Data input PIN */
#define SHIFT_PIN_STCP 3 /* Shift Register Storage PIN */
#define SHIFT_PIN_SHCP 4 /* Shift Register Shift PIN */

#define COMMAND_SET_TIME       0
#define COMMAND_SET_BRIGHTNESS 1
#define COMMAND_SETTINGS_MODE  2

#define COMNAND_GET_TIME       0

bool commandReady{false};
byte commandArray[255] = {0};
unsigned int brightness_{0};
//Instantiate a seven segment controller object (with Shift Register functionality)
SevSegShift sevsegshift(
                  SHIFT_PIN_DS, 
                  SHIFT_PIN_SHCP, 
                  SHIFT_PIN_STCP, 
                  1, /* number of shift registers */
                  true /* Digits are connected to Arduino directly */
                );

void setupDisplay() {
  setTime(0,0,0,0,0,0);
  byte numDigits = 4;
  byte digitPins[] = {5, 6, 7, 8};
  byte segmentPins[] = {6, 0, 2, 4, 5, 7, 1, 3};
  bool resistorsOnSegments = false;
  bool updateWithDelays = false;
  bool leadingZeros = true;
  byte hardwareConfig = COMMON_CATHODE;
  bool disableDecPoint = false;

  sevsegshift.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros, disableDecPoint);
  sevsegshift.setBrightness(20);
  sevsegshift.setNumber(0, 2);
}

void setup() {
  Serial.begin(9600);
  Serial.println("This is the start of the program");
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent); // register event
  setupDisplay();
}

void processCommand() {
  switch(commandArray[0]) {
    case COMMAND_SET_TIME:
      time_t t;
      t = (unsigned long)commandArray[1] <<  24 | 
          (unsigned long)commandArray[2] <<  16 | 
          (unsigned long)commandArray[3] <<  8  | 
          (unsigned long)commandArray[4];
      sevsegshift.setNumber(t, 2);
      break;
    case COMMAND_SET_BRIGHTNESS:
      //unsigned int brightness = (unsigned int)commandArray[1];
      sevsegshift.setBrightness(commandArray[1]);
      break;
  }
  commandReady = false;
}

void loop() {
  sevsegshift.refreshDisplay();
  if (commandReady) {
    processCommand();
  }
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  if (!commandReady) {
    unsigned int index = 0;
    while(Wire.available()) {
      // Wire.read() reads one byte of data
      commandArray[index++] = Wire.read();
    }
    commandReady = true;
  }
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  time_t t = now();
  byte data[4] = {t >> 24, t >> 16, t >> 8, t};
  Wire.write(data, 4); // respond with message of 4 bytes, the current time
                       // as expected by master
}