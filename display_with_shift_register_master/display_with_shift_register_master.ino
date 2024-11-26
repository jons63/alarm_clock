#include <TimeLib.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// For DFPlayer
SoftwareSerial softSerial(/*rx =*/10, /*tx =*/11);
#define FPSerial softSerial
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

unsigned long lastButtonPressTime = 0;
// Wait time in ms between two button presses
const int kTimeout = 500;

#define SETTINGS 2
#define ACTION   3
#define ALARM    4

enum class Mode : unsigned int {
  Normal = 0,
  TimeMinutes = 1,
  TimeHours = 2,
  AlarmMinutes = 3,
  AlarmHours = 4,
  Brightness = 5,
};

time_t time_{now()};

Mode mode_{Mode::Normal};
unsigned int brightness_{20};
struct Alarm {
  time_t alarm_time{0};
  bool alarm_active{false};
  time_t alarm_off;
};

Alarm alarm_;
void SendTime(time_t t);
void SendBrightness(byte brightness);

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

  myDFPlayer.volume(15);  //Set volume value. From 0 to 30
}

class Button {
  public:
  Button(unsigned int pin, void (*func)()) : pin_{pin}, func_{func} {};
  void operator()()  {
    if (!time_) {
      int new_state = digitalRead(pin_);
      if (new_state != last_state_ && new_state == LOW) {
        last_state_ = new_state;
        func_();
        time_ = millis();
      } else if (new_state != last_state_ && new_state == HIGH) {
        last_state_ = new_state;
      }
    } else if (millis() - time_ > kTimeout) {
      time_ = 0;
    }
  };

  private:
    unsigned long time_{0};
    int last_state_{HIGH};
    unsigned int pin_;
    void (*func_)();
};

void ActionFunction() {
  switch (mode_) {
    case Mode::TimeMinutes:
      time_ += 1;
      setTime(time_);
      SendTime(time_);
      break;
    case Mode::TimeHours:
      time_ += 60;
      setTime(time_);
      SendTime(time_);
      break;
    case Mode::AlarmMinutes:
      alarm_.alarm_time += 1;
      SendTime(alarm_.alarm_time);
      break;
    case Mode::AlarmHours:
      alarm_.alarm_time += 60;
      SendTime(alarm_.alarm_time);
      break;
    case Mode::Brightness:
      if (brightness_ >= 100) {
        brightness_ = 0;
      } else {
        brightness_ += 10;
      }
      SendBrightness(brightness_);
      break;
    default:
      break;
  }
}

void ModeFunction() {
  switch (mode_) {
    case Mode::Normal:
      SendTime(time_);
      mode_ = Mode::TimeMinutes;
      break;
    case Mode::TimeMinutes:
      SendTime(time_);
      mode_ = Mode::TimeHours;
      break;
    case Mode::TimeHours:
      SendTime(alarm_.alarm_time);
      mode_ = Mode::AlarmMinutes;
      break;
    case Mode::AlarmMinutes:
      SendTime(alarm_.alarm_time);
      mode_ = Mode::AlarmHours;
      break;
    case Mode::AlarmHours:
      SendTime(time_);
      mode_ = Mode::Brightness;
      break;
    case Mode::Brightness:
      mode_ = Mode::Normal;
      break;
  }
  if (mode_ == Mode::Normal) {
    
  } else if (mode_ == Mode::TimeMinutes) {
    
  } else if (mode_ == Mode::TimeHours) {
    
  }else if (mode_ == Mode::Brightness) {
    
  }
  Serial.print("ModeFunction, mode:");
  Serial.println((int)mode_);
}

Button btn(SETTINGS, &ModeFunction);
Button btn2(ACTION, &ActionFunction);

void setup(){
  Serial.begin(9600);
  Wire.begin();
  pinMode(SETTINGS, INPUT_PULLUP);
  pinMode(ACTION, INPUT_PULLUP); 
  pinMode(ALARM, INPUT_PULLUP); 
  setupAudio();
  Serial.println("This is the start of the program");
}

void CheckInput(){
  btn();
  btn2();
}

// time_t == unsinged long == 4 bytes == 32 bits
void SendTime(time_t t) {
  int s = second(t);
  int m = minute(t);
  m = m * 100;
  t = m+s;
  Wire.beginTransmission(8); 
  Wire.write(0);
  Wire.write(t>>24);
  Wire.write(t>>16);
  Wire.write(t>>8);
  Wire.write(t);
  Wire.endTransmission();  
}

void SendBrightness(byte brightness) {
  Wire.beginTransmission(8); 
  Wire.write(1);
  Wire.write(brightness);
  Wire.endTransmission();  
}

void loop(){
  time_t t = now();
  if ((mode_ == Mode::Normal) && (t - time_ > 0)) {
    SendTime(t);
    time_ = t;
  }

  if (alarm_.alarm_active && digitalRead(ALARM) == HIGH) {
    alarm_.alarm_active = false;
    myDFPlayer.pause();
    alarm_.alarm_off = now();
    Serial.println("Alarm is now inactive");
  }

  if (minute(time_) == minute(alarm_.alarm_time) &&
      minute(now()) > minute(alarm_.alarm_off)) {
    if (digitalRead(ALARM) == LOW && !alarm_.alarm_active) {
      alarm_.alarm_active = true;
      myDFPlayer.loop(2);
      Serial.println("Alarm is now active");
    } 
  } 

  if (alarm_.alarm_active && minute(now())-minute(alarm_.alarm_time) >= 1) {
      alarm_.alarm_active = false;
      myDFPlayer.pause();
      Serial.println("Alarm is now inactive");
  }

  CheckInput();
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
