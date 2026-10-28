#include <Ultrasonic.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include <Stepper.h>



int distance;
int previousDistance = 100;
int state = 0;
int eggCollected = 0;
const int stepsPerRevolution = 2048;
float stepPerangle = 5.66111111;

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27,16,2);
Ultrasonic ultrasonic(12, 13);
Stepper myStepper(stepsPerRevolution, 2, 4, 3, 5);


void setup() {
  //setup peripherals
  lcd.init();
  lcd.clear();
  lcd.backlight();
  Serial.begin(115200);
  myStepper.setSpeed(5);

  if (!rtc.begin()) {
    lcd.setCursor(0, 0);
    lcd.print("RTC Not Found!");
    Serial.println("Couldn't find RTC module.");
    while (1); 
  }

  //Pin for IR sensor
  pinMode(A0, INPUT);
}

void loop() {
  //State 0 waiting for ducks
  if(state == 0){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Collected ");
    lcd.print(eggCollected, DEC);
    lcd.print(" eggs");

    distance = ultrasonic.read();
    //Only collect if no duck is present
    if(previousDistance <= 15 && distance > previousDistance && distance > 50){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.println("chicken detected");
      state = 1;
      return;
    }
    previousDistance = distance;
    delay(100);

  }

  //State 1 collects eggs
  if(state == 1){
    previousDistance = 1000;
    TimeSpan delayS(0, 0, 0, 10);
    DateTime now = rtc.now();
    DateTime after = now+delayS;
    lcd.setCursor(0,1);
    lcd.println("Collect in ");
    
    while(now < after){
      // Serial.println("test2");
      TimeSpan different = after-now;
      int seconds = different.seconds();
      lcd.setCursor(11,1);
      lcd.print(seconds, DEC);
      if(seconds < 10)
        lcd.print(" ");
      delay(50);
      now = rtc.now();
    }

    myStepper.step(45*stepPerangle);
    int nowMilli = millis();
    int nextMilli = nowMilli + 5000;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Waiting for egg");
    
    bool foundEgg = false;
    //Non blocking delay for detecting eggs
    while(nowMilli < nextMilli){
      if(digitalRead(A0) == LOW){
        foundEgg = true;
        break;
      }
      nowMilli = millis();
    }
    lcd.clear();
    if(!foundEgg){
      Serial.print("Egg not found! -");
      lcd.setCursor(0,0);
      lcd.print("No egg found");
      delay(1000);
    }else{
      eggCollected++;
      Serial.print("Egg found! Total ");
      Serial.print(eggCollected);
      Serial.print(" -");
      lcd.setCursor(0,0);
      lcd.print("Egg found");
      delay(1000);
    }

    DateTime stamp = rtc.now();
    Serial.print(stamp.hour(), DEC);
    Serial.print(':');
    if (stamp.minute() < 10) Serial.print('0');
    Serial.print(stamp.minute(), DEC);
    Serial.print(':');
    if (stamp.second() < 10) Serial.print('0');
    Serial.print(stamp.second(), DEC);
    
    Serial.println();

    myStepper.step(-45*stepPerangle);
    state = 0;
  }
}
