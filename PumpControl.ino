#include <LiquidCrystal.h>

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
#define PN0 2 // This is the MuC
#define PN1 4 // This is also the MuC
#define PN2 3 // This is the MuFuK

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

volatile boolean up;
volatile boolean turned;
volatile boolean pushed = true;


// Custom characters for LCD
byte canUl[8] = {
  B00001,
  B00010,
  B00100,
  B00101,
  B00011,
  B00001,
  B01100,
};
byte canUr[8] = {
  B10000,
  B01000,
  B11100,
  B11110,
  B11111,
  B11111,
  B11111,
};
byte canLl[8] = {
  B00111,
  B10000,
  B00000,
  B01000,
  B00000,
  B00100,
  B10000,
};
byte canLr[8] = {
  B11110,
  B01100,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};

byte flowerUl[8] = {
  B00000,
  B00000,
  B00111,
  B01100,
  B11000,
  B10010,
  B10000,
};
byte flowerUr[8] = {
  B00000,
  B00000,
  B11100,
  B00110,
  B00011,
  B01001,
  B00001,
};
byte flowerLl[8] = {
  B10000,
  B10010,
  B11001,
  B01100,
  B00111,
  B00000,
  B00000,
};
byte flowerLr[8] = {
  B00001,
  B01001,
  B10011,
  B00110,
  B11100,
  B00000,
  B00000,
};



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

const unsigned long lastPumpDtMin = (unsigned long) (1 * 60 * 1000L);   // the minimal intervall between two pumping actions
const int pumpDtMax = (int) (5 * 1000);                                 // the maximal pumping action duration
const unsigned long brightTime = 1 * 60 * 1000L;                        // the time the LCD is lit after button / rotation knob action
const int dt = 50;                                                      // ms delay time of loop
const int sensThrMax = 100;
const int sensThrMin = 0;

void loop() {
  static int sensThr = 50;
  static int sensorValueRaw = 0;        // value read from the pot
  static int sensorValueMapped = 0;
  static unsigned long lastPumpCnt = (long) (lastPumpDtMin / dt) + 1;
  static int pumpDurCnt = 0;
  static bool pumping = false;
  static unsigned long lastPushedCnt = 0;

  // Updating the threshold via the encoder variable.
  if (turned)
    {
      if (up){
        if (sensThr > sensThrMin){
          sensThr--;
          delay(10);
        }
      }else{
         if (sensThr < sensThrMax){
          sensThr++;
          delay(10);
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
  printLcd(sensorValueMapped, sensThr, pumping);

  // Checking and setting the pump activity.
  if (sensorValueMapped < sensThr){
      // Too dry
      if (pumping){
        pumpDurCnt++;
        if (pumpDurCnt * dt > pumpDtMax){
          digitalWrite(PUMPPN, LOW);
          pumping = false;
        }
      }else{
        if (lastPumpCnt * dt > lastPumpDtMin){
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
      if (pumpDurCnt * dt > pumpDtMax){
        digitalWrite(PUMPPN, LOW);
        pumping = false;
      }
    }else{
      lastPumpCnt++;
    }
  }

  // LCD LED background light
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

  delay(dt);
}


void printLcd(int sensorValue, int threshold, bool pumping){
  static int lastSensorValue;
  static int lastThreshold;
  if (sensorValue == lastSensorValue && threshold == lastThreshold)
    return;
  lastSensorValue = sensorValue;
  lastThreshold = threshold;

  String thresholdBaseString = "Threshold: ";
  String thresholdString = thresholdBaseString + threshold;
  String sensorBaseString = "Sensor: ";
  String sensorString = sensorBaseString + sensorValue;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(thresholdString);
  lcd.setCursor(0, 1);
  lcd.print(sensorString);
  if (sensorValue < threshold){
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
