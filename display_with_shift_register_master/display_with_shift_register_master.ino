#include <TimeLib.h>
#include <Wire.h>

unsigned long lastButtonPressTime = 0;
// Wait time in ms between two button presses
const int kTimeout = 500;

#define SETTINGS 2
#define ACTION   3

enum class Mode : unsigned int {
  Normal = 0,
  Settings = 1,
};

time_t t_{now()};
Mode mode_{Mode::Normal};

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
  if (mode_ == Mode::Settings) {
    t_ += 1;
    setTime(t_);
    SendTime(t_);
  }
}

void ModeFunction() {
  if (mode_ == Mode::Normal) {
    mode_ = Mode::Settings;
  } else {
    mode_ = Mode::Normal;
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

unsigned long x{0};


void loop(){
  time_t t = now();
  if ((mode_ == Mode::Normal) && (t - t_ > 0)) {
    SendTime(t);
    t_ = t;
  }
  
  CheckInput();
}
