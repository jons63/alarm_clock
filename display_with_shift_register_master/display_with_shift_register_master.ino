#include <TimeLib.h>
#include <Wire.h>

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
    alarm_.alarm_off = now();
    Serial.println("Alarm is now inactive");
  }

  if (minute(time_) == minute(alarm_.alarm_time) &&
      minute(now()) > minute(alarm_.alarm_off)) {
    if (digitalRead(ALARM) == LOW && !alarm_.alarm_active) {
      alarm_.alarm_active = true;
      Serial.println("Alarm is now active");
    } 
  } 

  if (alarm_.alarm_active && minute(now())-minute(alarm_.alarm_time) >= 1) {
      alarm_.alarm_active = false;
      Serial.println("Alarm is now inactive");
  }

  CheckInput();
}
