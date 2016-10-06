#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include<CountUpDownTimer.h>

LiquidCrystal_I2C lcd(0x3f, 16, 2);
CountUpDownTimer T(DOWN);

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

int addressMill = 8;// define address millis eeprom is 8
int addressSec = 9; // define address Seccond eeprom is 9
int sec, mill;

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
      delay(3000);
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

  Act_state = digitalRead(swActive);

  T.Timer();
  //  Serial.print("Time count down = ");
  //  Serial.print(T.ShowMinutes());
  //  Serial.print(" : ");
  //  if (T.ShowSeconds() <= 9) {
  //    Serial.print("0");
  //  }
  //  Serial.print(T.ShowSeconds());
  //  Serial.print(" . ");
  //
  //  mill = ((millis() / 100) * (-1)) % 10;
  //  if (T.ShowSeconds() <= 0)
  //    mill = 0;
  //  Serial.print(mill);
  //  Serial.println(" S ");
  //
  //  if (T.ShowSeconds() <= 0 && mill <= 0)  {
  //    Serial.println(" END ");
  //    delay(2000);
  //    //    while (1);
  //  }

  if (T.TimeHasChanged() ) {
    Serial.print(T.ShowHours());
    Serial.print(":");
    Serial.print(T.ShowMinutes());
    Serial.print(":");
    Serial.print(T.ShowSeconds());
    Serial.print(":");
    Serial.print(T.ShowMilliSeconds());
    Serial.print(":");
    Serial.print(T.ShowMicroSeconds());
    Serial.print(":");
    Serial.println(T.ShowTotalSeconds());
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

  T.SetTimer(0, 1, 0);
  T.StartTimer();

  mill = EEPROM.read(addressMill);
  sec = EEPROM.read(addressSec);

  modeLoop = 0; // default loop
}

void loop() {

  /***   Main Code   ***/
  Act_state = digitalRead(swActive);
  //  if(Act_state == 0)  {
  //    active();
  //  }

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

  //  if (modeLoop >= 3) {
  //    modeLoop = 0;
  //  }

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
  
  while (modeLoop == 1) {
    settime();
  }
  while (modeLoop == 2) {
    active();
  }
}


