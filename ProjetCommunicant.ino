
#include <Wire.h>
#include "rgb_lcd.h"
#include <Keyboard.h>
#include "AceButton.h"

using namespace ace_button;

// var Ecran LCD
rgb_lcd lcd;
const int colorR = 255;
const int colorG = 0;
const int colorB = 0;

//Fleche bas
byte down[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b11111,
  0b01110,
  0b00100
};

//Fleche haut
byte up[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};

//signe Egal
byte eq[8] = {
  0b11111,
  0b11111,
  0b00000,
  0b11111,
  0b11111,
  0b00000,
  0b11111,
  0b11111
};

// Var IR
const byte IR_PIN = 4;

// Var Ultrason
const byte TRIGGER_PIN = 5; // Broche TRIGGER
const byte ECHO_PIN = 6;   // Broche ECHO
const unsigned long MEASURE_TIMEOUT = 25000UL; // 25ms = ~8m à 340m/s
/* Vitesse du son dans l'air en mm/us */
const float SOUND_SPEED = 340.0 / 1000;

// Var buttonMiser
ButtonConfig buttonConfigMiser;
const int btnMiser = 12;
AceButton buttonMiser(btnMiser);

//Var btn check
ButtonConfig buttonConfig;
const int btnCheck = 8;
AceButton buttonCheck(btnCheck);

//Var miser
const int pinA = 11;
const int pinB = 10;
int pinALast;

int aVal;
boolean bCW;



// Var logique
unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long previousMillisClear = 0;
const long interval = 450;           // interval at which to blink (milliseconds)



static char valString[20];
int oldIr = 0;
int compteurFold = 3;
int n = LOW;
int encoder0Pos = 0;
int lastencoder0Pos = 0;
int encoder0PinALast = LOW;

void setup()
{


  Serial.begin(115200);

  // LCD
  lcd.begin(16, 2); // set up the LCD's number of columns and rows:
  lcd.setRGB(colorR, colorG, colorB);  // Set color lcd
  lcd.print("  ...INIT...");// Print a message to the LCD.



  // create new character
  lcd.createChar(0, down);
  lcd.createChar(1, up);
  lcd.createChar(2, eq);


  //Keyboard.begin();

  //Btn check
  pinMode(btnCheck, INPUT_PULLUP);

  buttonConfig.setEventHandler(handleEvent);
  buttonConfig.setFeature(ButtonConfig::kFeatureClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonCheck.setButtonConfig(&buttonConfig);

  // Ultrason
  pinMode(TRIGGER_PIN, OUTPUT);
  digitalWrite(TRIGGER_PIN, LOW); // La broche TRIGGER doit être à LOW au repos
  pinMode(ECHO_PIN, INPUT);


  // Raise
  pinMode(pinA, INPUT);
  pinMode(pinB, INPUT);
  pinALast = digitalRead(pinA);


  //Button Miser
  pinMode(btnMiser, INPUT_PULLUP);
  buttonConfigMiser.setEventHandler(handleEvent2);
  buttonConfigMiser.setFeature(ButtonConfig::kFeatureClick);
  buttonConfigMiser.setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfigMiser.setFeature(ButtonConfig::kFeatureLongPress);
  buttonMiser.setButtonConfig(&buttonConfigMiser);

  // IR
  pinMode(IR_PIN, INPUT);

  delay(2500);
}


// Efface l'affichage
void clear(int ligne) {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisClear >= 500) {
    previousMillisClear = currentMillis;


  }
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, ligne);
    lcd.print(" ");
  }

}

void changeColor(int a) {

  lcd.setCursor(0, 0);
  if (a < 40  && a > 0) {
    lcd.setRGB(255, 255, 255);
    seCoucher();
  } else {
    compteurFold = 3;
    lcd.setRGB(0, 0, 0);
    lcd.print("Poker - En Attente");
    clear(0);
  }
}

void seCoucher() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;



    if (compteurFold == -1 ) {
      lcd.setRGB(0, 250, 0);
      clear(0);
      lcd.setCursor(0, 0);

      Keyboard.write(194);

      lcd.print("Done !");
      delay(3000);
      clear(0);
    }
    if (compteurFold >= 0) {
      lcd.setCursor(0, 0);
      lcd.print("Fold in :");
      lcd.setCursor(10, 0);
      lcd.print(compteurFold);
    }


    compteurFold -= 1;

  }
}

void loop()
{

  buttonCheck.check();
  buttonMiser.check();
  abtnRotary.check();
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  /* 2. Mesure le temps entre l'envoi de l'impulsion ultrasonique et son écho (si il existe) */
  long measure = pulseIn(ECHO_PIN, HIGH, MEASURE_TIMEOUT);

  /* 3. Calcul la distance à partir du temps mesuré */
  float distance_mm = measure / 2.0 * SOUND_SPEED;

  //Serial.println(distance_mm / 10.0, 2);
  // set the cursor to column 0, line 1@
  // (note: line 1 is the second row, since counting begins with 0):
  //lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  //lcd.print(millis()/1000);
  n = digitalRead(pinA);
  if ((encoder0PinALast == LOW) && (n == HIGH)) {
    if (digitalRead(pinB) == LOW) {
      encoder0Pos--;
    } else {
      encoder0Pos++;
    }
    Serial.print (encoder0Pos);
    Serial.print ("/");
    if (lastencoder0Pos < encoder0Pos) {
      lcd.setCursor(8, 1);
      lcd.write((unsigned char)0);
      Keyboard.write(217);
    } else {
      lcd.setCursor(8, 1);
      lcd.write(1);
      Keyboard.write(218);
    }
  }
  encoder0PinALast = n;
  lastencoder0Pos = encoder0Pos;


  int ir = digitalRead(IR_PIN);

  if (ir == 1) {
    if (oldIr == 0) {
      clear(0);
      clear(1);
    }
    compteurFold = 3;

    int buttonStateMiser = digitalRead(btnMiser);
    int buttonStateCheck = digitalRead(btnCheck);
    int buttonStateAllIn = digitalRead(btnRotary);
    lcd.setRGB(255, 255, 255);
    lcd.setCursor(0, 0);
    lcd.print("Poker");
  } else {
    //clear(0);
    if (oldIr == 1) {
      clear(0);
      clear(1);
    }
    int nearest = distance_mm / 10.0;
    changeColor(nearest);
    dtostrf(nearest, 7, 2, valString);
    strcat(valString, "cm");
    lcd.setCursor(0, 1);
    lcd.print(valString);


    lcd.setCursor(7, 1);
    //lcd.print(ir);
    //Serial.println(ir);
  }

  oldIr = ir;

  delay(1);
}


// The event handler for the button.
void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventClicked:
      Keyboard.write(195);
      Serial.println("Click <== fonctionne pour double click");
      lcd.setCursor(0, 1);
      lcd.write("Call / Follow");
      lcd.setRGB(0, 255, 255);
      delay(1500);
      clear(1);
      lcd.setCursor(8, 1);
      lcd.write("Done !");
      lcd.setRGB(0, 255, 0);
      delay(500);
      clear(1);
      break;
    case AceButton::kEventDoubleClicked:
      Serial.println("Double Click");
      break;
  }
}

void handleEvent2(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  switch (eventType) {
    case AceButton::kEventClicked:
      /*Serial.println("Pressed");
      Keyboard.write(196);*/
      break;
    case AceButton::kEventDoubleClicked:
      Serial.println("Pressed twice");
      break;
    case AceButton::kEventLongPressed:
      Serial.println("Looong presse");
      for (int i = 0; i < 250; i++) {
        Keyboard.write(218);
      }
      lcd.setCursor(0, 1);
      lcd.write("ALL IN BABY !");
      lcd.setRGB(0, 255, 255);
      delay(2500);
      break;
     case AceButton::kEventReleased:
      clear(1);
      Serial.println("Pressed");
      Keyboard.write(196);
      lcd.setCursor(8, 1);
      lcd.write("Done !");
      lcd.setRGB(0, 255, 0);
      delay(1000);
      clear(1);
      break;
  }

}



/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
