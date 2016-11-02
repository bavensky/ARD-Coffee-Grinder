#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include<CountUpDownTimer.h>

LiquidCrystal_I2C lcd(0x3f, 16, 2);

CountUpDownTimer T(DOWN); // time count down

#define swSetting 3
#define swActive 4

#define debounceS 50
#define holdTimeS 1000
#define debounceA 50
#define holdTimeA 1000
#define RELAY 12

byte Act_state = 0;
byte Set_State = 0;
byte modeLoop = 0;

boolean ignoreUpS = false;
boolean ignoreUpA = false;
boolean fact = false;
boolean count = false;
boolean count_mill = false;

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
  if (mill < 0)  {
    sec -= 1;
    mill = 9;
  }

  if (Act_state == 0)  {
    if (Set_State == 0)  {
      if (sec <= 0) {
        sec = 0;
      }
      if (mill <= 0)  {
        mill = 0;
      }
      EEPROM.write(addressSec, sec);
      EEPROM.write(addressMill, mill);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" SET Time Finish ");
      lcd.setCursor(0, 1);
      lcd.print("   ");
      if (sec < 10) lcd.print("0");
      lcd.print(sec);
      lcd.print(".");
      lcd.print(mill);
      lcd.print("0 Sec.   ");
      modeLoop = 0;
      delay(2000);
      lcd.clear();
    }
  }

  lcd.setCursor(0, 0);
  lcd.print("   ");
  if (sec < 10)  lcd.print("0");
  if (sec <= 0) {
    lcd.print("0");
  } else {
    lcd.print(sec);
  }
  lcd.print(".");
  if (mill <= 0)  {
    lcd.print("0");
  } else {
    lcd.print(mill);
  }
  lcd.print("0");
  lcd.print(" Sec.   ");
  lcd.setCursor(0, 1);
  lcd.print(" v=- ^=+ v^=Set ");
}

void active() {
  digitalWrite(RELAY, HIGH);
  if (sec > 1 && millCount >= 0)  {
    T.SetTimer(0, 0, sec);
    count = true;
    lcd.home();
  } else if (sec <= 1 && millCount >= 0) {
    count_mill = true;
    lcd.setCursor(0, 0);
    lcd.print("   00.");
    lcd.print(millCount--);
    lcd.print(millCount--);
    lcd.print(" Sec.   ");
    lcd.setCursor(0, 1);
    lcd.print("v=Pause  ^=Reset");
  }

  if (sec <= 1 && millCount <= 0 && count_mill == true )  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   00.00 Sec.   ");
    lcd.setCursor(0, 1);
    lcd.print("    Finish      ");
    digitalWrite(RELAY, LOW);
    delay(1000);
    lcd.clear();
    count = false;
    count_mill == false;
    millCount = mill;
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
      digitalWrite(RELAY, LOW);
      lcd.setCursor(0, 1);
      lcd.print("     Pause      ");
      Act_state = digitalRead(swActive);
      if (Act_state == 0)  {
        delay(200);
        digitalWrite(RELAY, HIGH);
        fact = false;
      }
      Set_State = digitalRead(swSetting);
      if (Set_State == 0)  {
        delay(200);
        modeLoop = 0;
        count = false;
        fact = false;
      }
    }
    T.StartTimer();
    T.Timer(); // run the timer

    lcd.setCursor(0, 0);
    lcd.print("   ");
    if (T.ShowSeconds() < 10)
      lcd.print("0");
    lcd.print(T.ShowSeconds());
    lcd.print(".");
    if (T.ShowSeconds() > 0 && millCount <= 0)  {
      millCount = 9;
    }
    lcd.print(millCount--);
    lcd.print(millCount--);
    lcd.print(" Sec.   ");

    lcd.setCursor(0, 1);
    lcd.print("v=Pause  ^=Reset");

    if (T.ShowSeconds() <= 0 && millCount <= 0)  {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("   00.00 Sec.   ");
      lcd.setCursor(0, 1);
      lcd.print("     Finish     ");
      digitalWrite(RELAY, LOW);
      delay(1000);
      lcd.clear();
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
  pinMode(RELAY, OUTPUT);

  lcd.begin();
  lcd.backlight();

  mill = EEPROM.read(addressMill);
  sec = EEPROM.read(addressSec);

  modeLoop = 0; // default loop
}

void loop() {
  digitalWrite(RELAY, LOW);
  
  /***   Main Code   ***/
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
    delay(1000);
    lcd.clear();
    millCount = mill;
    if (sec == 1) millCount = 9;
    modeLoop = 2;
  }

  while (modeLoop == 1) {
    settime();
  }
  while (modeLoop == 2) {
    active();
  }

  lcd.setCursor(0, 0);
  lcd.print("   ");
  if (sec < 10)  lcd.print("0");
  lcd.print(sec);
  lcd.print(".");
  lcd.print(mill);
  if (mill < 10) lcd.print("0");
  lcd.print(" Sec.   ");
  lcd.setCursor(0, 1);
  lcd.print(" v=Start  ^=Set ");

}


