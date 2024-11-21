#include <TimeLib.h>
#include <Wire.h>

unsigned long lastButtonPressTime = 0;
// Wait time in ms between two button presses
const int kTimeout = 100;

class Button {
  public:
  Button(unsigned int pin, void (*func)()) : pin_{pin}, func_{func} {};
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
    func_();
  };

  private:
    unsigned long time_{0};
    int last_state_{HIGH};
    unsigned int pin_;
    void (*func_)();
};

void TestFunc() {
    Wire.requestFrom(8, 4);
    byte arr[5];
    unsigned int index = 0;
    while (Wire.available()) {
      arr[index++] = Wire.read();
    }
    time_t t = (unsigned long)arr[0] << 26 | 
               (unsigned long)arr[1] << 16 | 
               (unsigned long)arr[2] << 8 | 
               (unsigned long)arr[3];
    t += SECS_PER_MIN;
    arr[0] = 0;       // command type
    arr[1] = t >> 24; // data
    arr[2] = t >> 16; // data
    arr[3] = t >> 8;  // data
    arr[4] = t;       // data
    Wire.beginTransmission(8);
    Wire.write(arr, 5);
    Wire.endTransmission();
    Serial.println("Temp function");
}

Button btn(1, &TestFunc);

void setup(){
  Serial.begin(9600);
  Wire.begin();
  Serial.println("This is the start of the program");
}

void check_input(){
  btn();
}

unsigned long x = 0;

void loop(){
  // Write time to slave arduino
  
  Wire.beginTransmission(8); 
  Wire.write(0);
  Wire.write(x>>24);
  Wire.write(x>>16);
  Wire.write(x>>8);
  Wire.write(x);
  Wire.endTransmission();   
  x += 60;
  delay(2000);
  
  /*
  // Read time from slave arduino
  Wire.requestFrom(8, 4);    // request 6 bytes from peripheral device #8
  byte commandArray[255] = {0};
  unsigned int index = 0;
    while (Wire.available()) { // peripheral may send less than requested
      commandArray[index++] = Wire.read(); // receive a byte as character
    }
    time_t t = (unsigned long)commandArray[0] <<  24 | 
                 (unsigned long)commandArray[1] <<  16 | 
                 (unsigned long)commandArray[2] <<  8 | 
                 (unsigned long)commandArray[3];
    Serial.print(minute(t));
    Serial.print(":");
    Serial.println(second(t));
    delay(1000);
    */
}
