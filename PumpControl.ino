#include <LiquidCrystal.h>
#include "includeFile.h"
// LCD
#define RS 12
#define EN 11
#define D4 8
#define D5 7
#define D6 6
#define D7 5
#define LEDPN 10
// Sensors and pump
#define PUMPPN 9
#define ANINPN A0
#define MESVPN 13
// Button and rotation encoder
#define PN0 2 // Muc
#define PN1 4 // Muc
#define PN2 3 // MuFuK
#define DBNC 20 //Debounce time

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

volatile boolean up;
volatile boolean turned;
volatile boolean pushed = true;

// Interrupt Service Routine for a change to encoder pin
void isr ()
{
 if (digitalRead (PN0))
   up = digitalRead (PN1);
 else
   up = !digitalRead (PN1);
 turned = true;
}

void isp (){
  if (digitalRead(PN2))
    pushed = true;
}

void setup() {
  lcd.begin(16, 2);
  lcd.createChar(0,canUl);
  lcd.createChar(1,canUr);
  lcd.createChar(2,canLl);
  lcd.createChar(3,canLr);
  lcd.createChar(4,flowerUl);
  lcd.createChar(5,flowerUr);
  lcd.createChar(6,flowerLl);
  lcd.createChar(7,flowerLr);

  pinMode(PUMPPN, OUTPUT);
  pinMode(MESVPN, OUTPUT);

  // Rotation encoder
  digitalWrite (PN0, HIGH);     // enable pull-ups
  digitalWrite (PN1, HIGH);
  attachInterrupt (digitalPinToInterrupt(PN0), isr, CHANGE);

  // Push button for LCD background
  digitalWrite(PN2, HIGH);
  digitalWrite(LEDPN, HIGH);
  attachInterrupt (digitalPinToInterrupt(PN2), isp, RISING);
}

const unsigned long brightTime = 1 * 60 * 1000L;                        // the time the LCD is lit after button / rotation knob action
const unsigned int dt = 50;                                             // ms delay time of loop
const unsigned int sensThrMax = 100;
const unsigned int sensThrMin = 0;
const unsigned long pumpDtMax = (1 * 60 * 1000L);
const unsigned int pumpDtMin = (int) (5 * 1000);
const unsigned long interPumpDtMax = (2L * 24L * 60L * 60L * 1000L);
const unsigned long interPumpDtMin = (1 * 60 * 60 * 1000L);

void loop() {
  static unsigned int sensThr = 50;
  static unsigned long pumpDt = (int) (10 * 1000);                           // the maximal pumping action duration
  static unsigned long interPumpDt = (unsigned long) (2 * 60 * 60 * 1000L);      // the minimal intervall between two pumping actions
  static unsigned int sensorValueRaw = 0;                                   // value read from the pot
  static unsigned int sensorValueMapped = 0;
  static unsigned long lastPumpCnt = (long) (interPumpDt / dt) + 1;
  static unsigned int pumpDurCnt = 0;
  static bool pumping = false;
  static unsigned long lastPushedCnt = 0;
  static MUFUKSTATE mufState;


  // MuFuK: LCD LED background light and value selection
  if (pushed){
    delay(DBNC);   // debounce
    digitalWrite(LEDPN, HIGH);
    pushed = false;
    if (lastPushedCnt * dt < brightTime){
      mufState = MUFUKSTATE((((int) mufState) + 1) % 3);    // Is that really okay?
    }
    lastPushedCnt = 0;
    }else{
      lastPushedCnt++;
      if (lastPushedCnt * dt >= brightTime){
        digitalWrite(LEDPN, LOW);
        mufState = MUFUKSTATE(0);
      }
    }

  // MuC:Updating the threshold via the encoder variable.
  if (turned)
    {
      if (up){
        switch(mufState){
          case thrSel:
            if (sensThr > sensThrMin)
              sensThr--;
            break;
            case pumpDtSel:
              if (pumpDt > pumpDtMin)
                pumpDt -= 1000;
              break;
            case wateringDtSel:
              if (interPumpDt > interPumpDtMin)
                interPumpDt -= 3600000L;
              break;
        }
        delay(DBNC);     // debounce
      }else{
        switch(mufState){
          case thrSel:
            if (sensThr < sensThrMax)
              sensThr++;
            break;
           case pumpDtSel:
            if (pumpDt < pumpDtMax)
              pumpDt+= 1000;
            break;
            case wateringDtSel:
              if (interPumpDt < interPumpDtMax)
                interPumpDt += 3600000L;
              break;

          delay(DBNC);   // debounce
         }
      }
    turned = false;
    digitalWrite(LEDPN, HIGH);
    pushed = false;
    lastPushedCnt = 0;
   }

  // Measuring the conductivity and display the value and the threshold.
  digitalWrite(MESVPN, HIGH);
  delay(1);
  sensorValueRaw = analogRead(ANINPN);
  delay(1);
  digitalWrite(MESVPN, LOW);
  sensorValueMapped = map(sensorValueRaw, 0, 1023, sensThrMin, sensThrMax);


  switch(mufState){
    case thrSel:
      printLcd(sensorValueMapped, sensThr, sensorValueMapped, sensThr, pumping, mufState);
      break;
    case pumpDtSel:
      printLcd(sensorValueMapped, sensThr, 0, pumpDt, pumping, mufState);
      break;
    case wateringDtSel:
      printLcd(sensorValueMapped, sensThr, 0, interPumpDt, pumping, mufState);
      break;
  }

  // Checking and setting the pump activity.
  if (sensorValueMapped < sensThr){
      // Too dry
      if (pumping){
        pumpDurCnt++;
        if (pumpDurCnt * dt > pumpDt){
          digitalWrite(PUMPPN, LOW);
          pumping = false;
        }
      }else{
        if (lastPumpCnt * dt > interPumpDt){
          lastPumpCnt = 0;
          pumpDurCnt = 0;
          digitalWrite(PUMPPN, HIGH);
          pumping = true;
        }
        lastPumpCnt++;
    }
  }else{
    // not too dry
    if (pumping){
      pumpDurCnt++;
      if (pumpDurCnt * dt > pumpDt){
        digitalWrite(PUMPPN, LOW);
        pumping = false;
      }
    }else{
      lastPumpCnt++;
    }
  }

<<<<<<< HEAD
=======
  // This is where the MuFuK (MultiFunctionalKnob) will change the menu to
  // adjust settings with the MuC (MultiCrementor) like threshold, watering time
  // and the interval between two watering attempts
  if (pushed){
    digitalWrite(LEDPN, HIGH);
    pushed = false;
    lastPushedCnt = 0;
  }else{
    lastPushedCnt++;
    if (lastPushedCnt * dt >= brightTime){
      digitalWrite(LEDPN, LOW);
    }
  }

>>>>>>> 39c0ed92ec42842858100bfce01c3920f41b8df4
  delay(dt);
}



void printLcd(int isHum, int humThr, int isVal, unsigned long editVal, bool pumping, MUFUKSTATE state){
  static int lastIsVal;
  static int lastThr;
  static MUFUKSTATE lastState;
  if (isVal == lastIsVal && editVal == lastThr && lastState == state)
    return;

  lastIsVal = isVal;
  lastThr = editVal;
  lastState = state;

  String topRowBaseStr = "";
  String lowRowBaseStr = "";
  String topRowStr = "";
  String lowRowStr = "";
  switch (state){
    case thrSel:
      topRowBaseStr = "Threshold: ";
      topRowStr = topRowBaseStr + editVal;
      lowRowBaseStr = "Sensor: ";
      lowRowStr = lowRowBaseStr + isVal;
      break;
    case pumpDtSel:
      topRowStr = "Pump duration: ";
      lowRowStr = lowRowBaseStr + (editVal/1000L) + " s";
      break;
    case wateringDtSel:
      topRowStr = "Pump interval ";
      lowRowStr = lowRowBaseStr + (editVal/(3600000L)) + " h";
      break;
  }


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(topRowStr);
  lcd.setCursor(0, 1);
  lcd.print(lowRowStr);
  if (isHum < humThr){
    lcd.setCursor(14, 0);
    lcd.write(byte(0));
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
    lcd.setCursor(14, 1);
    lcd.write(byte(2));
    lcd.setCursor(15, 1);
    lcd.write(byte(3));
  }else{
    lcd.setCursor(14, 0);
    lcd.write(byte(4));
    lcd.setCursor(15, 0);
    lcd.write(byte(5));
    lcd.setCursor(14, 1);
    lcd.write(byte(6));
    lcd.setCursor(15, 1);
    lcd.write(byte(7));
  }

}
