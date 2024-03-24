#include <DFRobotDFPlayerMini.h>
#include <TimeLib.h>
#include <SoftwareSerial.h>

#include "SevSeg.h"

SevSeg sevseg; 
SoftwareSerial softSerial(/*rx =*/10, /*tx =*/11);
#define FPSerial softSerial
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

int val = 0;
unsigned long lastButtonPressTime = 0;
// Wait time in ms between two button presses
const int kTimeout = 100;

#define ADD 0
#define SETTINGS 1

enum class AddState : unsigned int {
  Idle = 0,
  Add_Ongoing = 1,
};

enum class Mode : unsigned int {
  Normal = 0,
  Settings = 1,
};

AddState state = AddState::Idle;
Mode mode = Mode::Normal;

class Button {
  public:
  Button(unsigned int pin) : pin_{pin} {};
  virtual void normal_action();
  virtual void settings_action();
  void operator()()  {
    if (!time_) {
      int new_state = digitalRead(pin_);
      if (new_state != last_state_ && new_state == LOW) {
        last_state_ = new_state;
        action();
        time_ = millis();
      } else if (new_state != last_state_ && new_state == HIGH) {
        last_state_ = new_state;
      }
    } else if (millis() - time_ > kTimeout) {
      time_ = 0;
    }
  };

  void action() {
    if (mode == Mode::Normal) {
      normal_action();
    } else if (mode == Mode::Settings) {
      settings_action();
    }
  };

  private:
    unsigned long time_{0};
    int last_state_{HIGH};
    unsigned int pin_;
};

class ToggleButton : public Button {
  public:
  ToggleButton(unsigned int pin) : Button(pin) {};
  void normal_action() override {
    Serial.println("Normal");
    sevseg.setNumber(0, 2);
    mode = Mode::Settings;
  }

  void settings_action() override {
    Serial.println("Settings");
    mode = Mode::Normal;
  }
};

ToggleButton btn(1);



void setupLogic() {
  pinMode(ADD, INPUT_PULLUP);
  pinMode(SETTINGS, INPUT_PULLUP);
}

void setupDisplay() {
  setTime(0,0,0,0,0,0);
  byte numDigits = 4;
  byte digitPins[] = {10, 11, 12, 13};
  byte segmentPins[] = {9, 2, 3, 5, 6, 8, 7, 4};
  bool resistorsOnSegments = false;
  bool updateWithDelays = false;
  bool leadingZeros = true;
  byte hardwareConfig = COMMON_CATHODE;

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
  sevseg.setBrightness(90);
}

void setupAudio() {
  FPSerial.begin(9600);

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */true)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.volume(10);  //Set volume value. From 0 to 30
  myDFPlayer.play(2);  //Play the first mp3
}

void setup(){
  Serial.begin(9600);
  Serial.println("This is the start of the program");

  setupDisplay();

  //setupAudio();
}

bool logic() {
  if(lastButtonPressTime) {
    if(millis() - lastButtonPressTime > kTimeout){
      lastButtonPressTime = 0;
    } else {
      return false;
    }
  }
  if (digitalRead(ADD) && state == AddState::Idle) {
    state = AddState::Add_Ongoing;
    lastButtonPressTime = millis();
    return true;
  } else if(!digitalRead(ADD) && state == AddState::Add_Ongoing) {
    state = AddState::Idle;
  } else {
    return false;
  }
}

void display() {
  time_t t = now();
  int m = minute(t);
  m = m * 100;
  int s = second(t);  

  sevseg.setNumber(m+s, 2);
  /*
  if (logic()) {
    val +=1;
    sevseg.setNumber(val, 2);
  }
  */
}

void audio() {
  static unsigned long timer = millis();

  if (millis() - timer > 3000) {
    timer = millis();
    //myDFPlayer.next();  //Play next mp3 every 3 second.
  }

  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
}

void settings() {

}

void check_input(){
  btn();
}

void loop(){
  check_input();
  if (mode == Mode::Normal) {
  display();  
  //audio();
  } else if (mode == Mode::Settings) {
    //settings();
  }

  sevseg.refreshDisplay(); 
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
