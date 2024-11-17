#include <TimeLib.h>
#include <Wire.h>
#include "SevSegShift.h"

#define SHIFT_PIN_DS   2 /* Data input PIN */
#define SHIFT_PIN_STCP 3 /* Shift Register Storage PIN */
#define SHIFT_PIN_SHCP 4 /* Shift Register Shift PIN */

bool commandReady{false};
unsigned int commandArray[255] = {0};
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
}

void setup() {
  Serial.begin(9600);
  Serial.println("This is the start of the program");
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(receiveEvent); // register event
  setupDisplay();
}

void display() {
  time_t t = now();
  int m = minute(t);
  m = m * 100;
  int s = second(t);  

  sevsegshift.setNumber(m+s, 2);
}

void processCommand() {

}

void loop() {
  display();  
  sevsegshift.refreshDisplay();
  if (commandReady) {
    processCommand();
  }
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  unsigned int index = 0;
  while(Wire.available()) {
    commandArray[index++] = Wire.read();
  }
  commandReady = true;
}