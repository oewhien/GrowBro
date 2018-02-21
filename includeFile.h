
enum MUFUKSTATE{
  thrSel = 0,
  pumpDtSel = 1,
  wateringDtSel = 2
};

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
