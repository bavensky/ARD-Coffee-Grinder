#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include<CountUpDownTimer.h>

LiquidCrystal_I2C lcd(0x3f, 16, 2);

CountUpDownTimer T(DOWN); // time count down

#define swSetting 3
#define swActive 4

#define debounceS 20
#define holdTimeS 500
#define debounceA 20
#define holdTimeA 500

byte Act_state = 0;
byte Set_State = 0;
byte modeLoop = 0;

boolean ignoreUpS = false;
boolean ignoreUpA = false;
boolean fact = false;
boolean count = false;

int addressMill = 8;// define address Millis eeprom is 8
int addressSec = 9; // define address Seccond eeprom is 9
int sec, mill, millCount;

int buttonLastS = 0;
int buttonLastA = 0;
//unsigned long _millis, _micro;

unsigned long btnDnTimeS;
unsigned long btnUpTimeS;
unsigned long btnDnTimeA;
unsigned long btnUpTimeA;
unsigned long prvTimeS = 0;

/*****      Function set time     *****/
void settime()  {
  Set_State = digitalRead(swSetting);
  Act_state = digitalRead(swActive);

  if (Set_State == LOW && buttonLastS == HIGH && (millis() - btnUpTimeS) > long(debounceS)) {
    btnDnTimeS = millis();
  }
  if (Set_State == HIGH && buttonLastS == LOW && (millis() - btnDnTimeS) > long(debounceS)) {
    ignoreUpS = false;
    btnUpTimeS = millis();
  }
  if (Set_State == LOW && (millis() - btnDnTimeS) > long(holdTimeS)) {
    ignoreUpS = true;
    btnDnTimeS = millis();
    sec += 1;
  }
  buttonLastS = Set_State;

  if (Act_state == LOW && buttonLastA == HIGH && (millis() - btnUpTimeA) > long(debounceA)) {
    btnDnTimeA = millis();
  }
  if (Act_state == HIGH && buttonLastA == LOW && (millis() - btnDnTimeA) > long(debounceA)) {
    ignoreUpA = false;
    btnUpTimeA = millis();
  }
  if (Act_state == LOW && (millis() - btnDnTimeA) > long(holdTimeA)) {
    ignoreUpA = true;
    btnDnTimeA = millis();
    sec -= 1;
  }
  buttonLastA = Act_state;

  if (Set_State == 0)  {
    delay(200);
    if (digitalRead(swSetting) == HIGH && ignoreUpS == false)  {
      mill += 1;
    }
  }

  if (Act_state == 0)  {
    delay(200);
    if (digitalRead(swActive) == HIGH && ignoreUpA == false)  {
      mill -= 1;
    }
  }

  if (mill >= 10)  {
    sec += 1;
    mill = 0;
  }

  if (sec <= 0 && mill <= 0)  {
    sec = 0;
    mill = 0;
  }

  if (mill < 0)  {
    sec -= 1;
    mill = 9;
  }

  if (Act_state == 0)  {
    if (Set_State == 0)  {
      EEPROM.write(addressSec, sec);
      EEPROM.write(addressMill, mill);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SET Time Finish ");
      modeLoop = 0;
      delay(2000);
    }
  }

  lcd.setCursor(0, 0);
  lcd.print(" Coffee grinder ");
  lcd.setCursor(0, 1);
  lcd.print("SET Time : ");
  if (sec < 10)  {
    lcd.print("0");
  }
  lcd.print(sec);
  lcd.print(".");
  lcd.print(mill);
  lcd.print("S  ");

  Serial.print("SET Time = ");
  if (sec < 10)  {
    Serial.print("0");
  }
  Serial.print(sec);
  Serial.print(".");
  Serial.print(mill);
  Serial.println(" S");
}

void active() {
  if (sec > 0)  {
    T.SetTimer(0, 0, sec);
    T.StartTimer();
    count = true;
  }
  if (sec == 0 && millCount > 0) {
    millCount--;
    lcd.setCursor(0, 0);
    lcd.print(" Coffee grinder ");
    lcd.setCursor(0, 1);
    lcd.print("Count down ");
    if (sec < 10)
      lcd.print("0");
    lcd.print(sec);
    lcd.print(".");
    lcd.print(millCount);
    lcd.print("S  ");
  }
  if (sec == 0 && millCount <= 0)  {
    Serial.println("END millCount");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" Coffee grinder ");
    lcd.setCursor(0, 1);
    lcd.print("     Finish     ");
    delay(2000);
    count = false;
    modeLoop = 0;
  }

  while (count == true) {
    Set_State = digitalRead(swSetting);
    if (Set_State == 0)  {
      delay(200);
      count = false;
      modeLoop = 0;
    }

    Act_state = digitalRead(swActive);
    if (Act_state == 0)  {
      delay(200);
      fact = true;
    }

    while (fact == true)  {
      Serial.println("WHILE ---");
      lcd.setCursor(0, 1);
      lcd.print("    Pause ");
      Act_state = digitalRead(swActive);
      if (Act_state == 0)  {
        delay(200);
        fact = false;
      }
      Set_State = digitalRead(swSetting);
      if (Set_State == 0)  {
        delay(200);
        modeLoop = 0;
        fact = false;
      }
    }
    
    T.Timer(); // run the timer
    lcd.setCursor(0, 0);
    lcd.print(" Coffee grinder ");
    lcd.setCursor(0, 1);
    lcd.print("Count down ");
    if (T.ShowSeconds() < 10)
      lcd.print("0");
    lcd.print(T.ShowSeconds());
    lcd.print(".");
    if (T.ShowSeconds() > 0 )  {
      lcd.print((T.ShowMilliSeconds() / 100) * (-1) % 10);
    }
    if (T.ShowSeconds() <= 0)  {
      lcd.print(millCount--);
    }
    lcd.print("S  ");

    if (T.ShowSeconds() <= 0 && millCount <= 0)  {
      Serial.println("END Seconds");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" Coffee grinder ");
      lcd.setCursor(0, 1);
      lcd.print("     Finish     ");
      delay(2000);
      count = false;
      modeLoop = 0;
    }
  }
}

/*****      Main Code     *****/
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(swActive, INPUT_PULLUP);
  pinMode(swSetting, INPUT_PULLUP);

  lcd.begin();
  lcd.backlight();

  //  T.SetTimer(0, 1, 0);
  //  T.StartTimer();

  mill = EEPROM.read(addressMill);
  sec = EEPROM.read(addressSec);

  modeLoop = 0; // default loop
}

void loop() {

  /***   Main Code   ***/


  //  micros()

  Set_State = digitalRead(swSetting);
  if (Set_State == LOW && buttonLastS == HIGH && (millis() - btnUpTimeS) > long(debounceS)) {
    btnDnTimeS = millis();
  }
  if (Set_State == HIGH && buttonLastS == LOW && (millis() - btnDnTimeS) > long(debounceS)) {
    ignoreUpS = false;
    btnUpTimeS = millis();
  }
  if (Set_State == LOW && (millis() - btnDnTimeS) > long(holdTimeS)) {
    ignoreUpS = true;
    btnDnTimeS = millis();
    lcd.clear();
    modeLoop = 1;
  }
  buttonLastS = Set_State;

  Act_state = digitalRead(swActive);
  if (Act_state == 0)  {
    delay(200);
    lcd.clear();
    millCount = mill;
    modeLoop = 2;
  }

  while (modeLoop == 1) {
    settime();
  }
  while (modeLoop == 2) {
    active();
  }

  Serial.println("Main Loop");
  lcd.setCursor(0, 0);
  lcd.print(" Coffee grinder ");
  lcd.setCursor(0, 1);
  lcd.print("Count down ");
  if (sec < 10)  {
    lcd.print("0");
  }
  lcd.print(sec);
  lcd.print(".");
  lcd.print(mill);
  lcd.print("S  ");
}


