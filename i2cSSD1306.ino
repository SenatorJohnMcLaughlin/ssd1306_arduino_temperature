// >> https://www.kollino.de/arduino/oled-display-mit-ssd1306-chipsatz-via-i2c-an-arduino-anschliessen/
#include <Wire.h>
#include <avr/pgmspace.h>

// >> DS18B20 Tutorial: https://draeger-it.blog/arduino-lektion-48-temperatursensor-ds18b20/?cn-reloaded=1
#include <OneWire.h>
#include <DallasTemperature.h>

#define I2C_ADDRESS 0x3C

// >>  https://github.com/greiman/SSD1306Ascii/blob/master/src/SSD1306Ascii.h
#define SSD1306_CMD 0x00  // Command
#define SSD1306_DATA 0x40 // Data
#define SSD1306_DATA_BUFFER 2 // Data-buffered


#define DISPLAY_ON 0xAF
#define DISPLAY_OFF 0xAE
#define RESET_DISPLAY 0xA4
#define FLOOD_DISPLAY 0xA5
#define INVERT_DISPLAY 0xA7
#define REINVERT_DISPLAY 0xA6
#define SET_CONTRAST 0x81
#define SET_COLUMN 0x00
#define SET_ROW 0x00

#define NUMBER_HEIGHT  48
#define NUMBER_WIDTH 32
#define DISPLAY_CONTRAST_VALUE 255    // 0-255

#define TEMPERATURE_PIN 2
#define READ_INTERVALL_MS 3000

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);

int yOffset=0;
int commaWidth = 7;
byte digitStartX = 16;

static const byte num_one[] PROGMEM = {
    0,	0,	0,	0,	0,	0,	0,	0,	0,	56,	124,	124,	124,	124,	124,	124,	252,	252,	252,	252,	248,	248,	240,	224,	0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	255,	255,	255,	255,	255,	255,	0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	255,	255,	255,	255,	255,	255,	0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	255,	255,	255,	255,	255,	255,	0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	255,	255,	255,	255,	255,	255,	0,	0,	0,	0,	0,	0,	0,	0,
    0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	31,	63,	63,	63,	63,	31,	0,	0,	0,	0,	0,	0,	0,	0
};

static const byte num_two[] PROGMEM = {
0,  0,  0,  28, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 126,  126,  254,  254,  252,  252,  248,  240,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  255,  255,  255,  255,  255,  0,
0,  0,  0,  0,  128,  192,  192,  224,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  248,  248,  252,  127,  127,  63, 31, 7,  0,
0,  0,  0,  255,  255,  255,  255,  255,  3,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  255,  255,  255,  255,  255,  128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  15, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 30, 0
};

static const byte num_three[] PROGMEM  = {
  0,	0,	0,	28,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	126,	126,	254,	254,	252,	252,	248,	240,	0,
0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	255,	255,	255,	255,	255,	0,
0,	0,	0,	0,	0,	0,	0,	0,	224,	240,	240,	240,	240,	240,	240,	240,	240,	240,	240,	240,	240,	240,	240,	248,	248,	252,	255,	255,	63,	31,	7,	0,
0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	3,	3,	7,	255,	255,	255,	254,	248,	0,
0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	128,	255,	255,	255,	255,	255,	0,
0,	0,	0,	28,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	62,	63,	63,	63,	63,	31,	31,	15,	7,	0

};

static const byte num_four[] PROGMEM = {
  0,  0,  0,  248,  252,  252,  252,  248,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  248,  252,  252,  252,  248,  0,
0,  0,  0,  255,  255,  255,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  255,  255,  255,  255,  255,  0,
0,  0,  0,  127,  255,  255,  255,  255,  248,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  248,  248,  255,  255,  255,  255,  255,  0,
0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  3,  255,  255,  255,  255,  255,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  255,  255,  255,  255,  255,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  31, 63, 63, 63, 31, 0

};

static const byte num_five[] PROGMEM  = {
  0,  0,  0,  248,  252,  252,  252,  252,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  56, 0,
0,  0,  0,  255,  255,  255,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  127,  255,  255,  255,  255,  248,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  224,  224,  192,  0,  0,
0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  7,  255,  255,  255,  255,  255,  0,
0,  0,  0,  0,  128,  128,  128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  128,  255,  255,  255,  255,  255,  0,
0,  0,  0,  15, 31, 63, 63, 63, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 63, 63, 63, 31, 31, 15, 3,  0
};

static const byte num_six[] PROGMEM = {
  0,  0,  0,  192,  240,  248,  248,  252,  252,  252,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  56, 0,
0,  0,  0,  255,  255,  255,  255,  255,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  255,  255,  255,  255,  255,  252,  248,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  224,  224,  192,  0,  0,
0,  0,  0,  255,  255,  255,  255,  255,  7,  3,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  7,  255,  255,  255,  255,  255,  0,
0,  0,  0,  255,  255,  255,  255,  255,  128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  128,  255,  255,  255,  255,  255,  0,
0,  0,  0,  3,  15, 31, 31, 63, 63, 63, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 63, 63, 63, 31, 31, 15, 3,  0
};

static const byte num_seven[] PROGMEM  = {
0,  0,  0,  56, 124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  252,  252,  252,  252,  252,  120,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  192,  240,  252,  255,  255,  127,  31, 7,  1,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  192,  240,  252,  255,  255,  127,  31, 7,  1,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  192,  240,  252,  255,  255,  127,  31, 7,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  192,  240,  252,  255,  255,  127,  31, 7,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  24, 60, 63, 63, 63, 15, 7,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

static const byte num_eight[] PROGMEM  = {
  0,  0,  0,  192,  240,  248,  248,  252,  252,  252,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  252,  252,  252,  248,  248,  240,  192,  0,
0,  0,  0,  255,  255,  255,  255,  255,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  255,  255,  255,  255,  255,  0,
0,  0,  0,  15, 31, 191,  255,  255,  252,  248,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  248,  252,  255,  255,  191,  31, 15, 0,
0,  0,  0,  254,  255,  255,  255,  255,  7,  3,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  7,  255,  255,  255,  255,  254,  0,
0,  0,  0,  255,  255,  255,  255,  255,  128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  128,  255,  255,  255,  255,  255,  0,
0,  0,  0,  3,  15, 31, 31, 63, 63, 63, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 63, 63, 63, 31, 31, 15, 3,  0
};

static const byte num_nine[] PROGMEM = {
  0,  0,  0,  192,  240,  248,  248,  252,  252,  252,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  252,  252,  252,  248,  248,  240,  192,  0,
0,  0,  0,  255,  255,  255,  255,  255,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  255,  255,  255,  255,  255,  0,
0,  0,  0,  31, 127,  255,  255,  255,  252,  248,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  248,  252,  255,  255,  255,  255,  255,  0,
0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  7,  255,  255,  255,  255,  255,  0,
0,  0,  0,  0,  128,  128,  128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  128,  255,  255,  255,  255,  255,  0,
0,  0,  0,  3,  15, 31, 31, 63, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 63, 63, 63, 31, 31, 15, 3,  0
};

static const byte num_zero[] PROGMEM = {
  0,  0,  0,  192,  240,  248,  248,  252,  252,  252,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  124,  252,  252,  252,  248,  248,  240,  192,  0,
0,  0,  0,  255,  255,  255,  255,  255,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  255,  255,  255,  255,  255,  0,
0,  0,  0,  255,  255,  255,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  255,  255,  255,  255,  255,  0,
0,  0,  0,  255,  255,  255,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  255,  255,  255,  255,  255,  0,
0,  0,  0,  255,  255,  255,  255,  255,  128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  128,  255,  255,  255,  255,  255,  0,
0,  0,  0,  3,  15, 31, 31, 63, 63, 63, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 63, 63, 63, 31, 31, 15, 3,  0
};

const byte *const numbers[] PROGMEM = {num_zero,num_one,num_two,num_three,num_four,num_five,num_six,num_seven,num_eight,num_nine};
// based on: https://github.com/ex-punctis/SSD1306_OLED_HW_I2C/blob/master/SSD1306_OLED_HW_I2C.c
void initDisplay()
{
  Wire.begin();
  Wire.setClock(400000L);

  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(SSD1306_CMD);

  Wire.write(0xAE); // Display OFF
  
  Wire.write(0x20); // Set Memory Addressing Mode
  Wire.write(0x00); // 00=Horizontal Addressing Mode; 01=Vertical Addressing Mode;

  Wire.write(0xB0); // Set Page Start Address for Page Addressing Mode, 0-7

  Wire.write(0xC8); // Set COM Output Scan Direction
  

  Wire.write(0x00);  // ---set low column address
  Wire.write(0x10);  // ---set high column address
  Wire.write(0x40);  // ---set start line address
  
  Wire.write(0x81); // Set contrast control register
  Wire.write(0x00);
  
  Wire.write(0xA1); // Set Segment Re-map. A0=address mapped; A1=address 127 mapped. 
  
  Wire.write(0xA6);  // Set display mode. A6=Normal; A7=Inverse
  
  Wire.write(0xA8); // Set multiplex ratio(1 to 64)
  Wire.write(0x3F);

  
  Wire.write(0xA4); // Output RAM to Display; // 0xA4=Output follows RAM content; 0xA5,Output ignores RAM content
  
  Wire.write(0xD3); // Set display offset. 00 = no offset
  Wire.write(0x00);
  
  Wire.write(0xD5); // --set display clock divide ratio/oscillator frequency
  Wire.write(0xF0); // --set divide ratio
  
  Wire.write(0xD9); // Set pre-charge period
  Wire.write(0x22);
  
  Wire.write(0xDA);   // Set com pins hardware configuration
  Wire.write(0x12);
  
  Wire.write(0xDB);   // --set vcomh
  
  Wire.write(0x20);   // 0x20,0.77xVcc
  
  Wire.write(0x8D);   // Set DC-DC enable
  Wire.write(0x14);
  
  Wire.write(0xAF);   // Display ON in normal mode
  
  Wire.endTransmission(); 
}

void changeContrast(byte value)
{
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(SSD1306_CMD);
  Wire.write(SET_CONTRAST);
  Wire.write(value);
  Wire.endTransmission(); 
}

void floodScreen()
{
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(SSD1306_CMD);
  Wire.write(FLOOD_DISPLAY);
  Wire.endTransmission(); 
}

void setOledCursor(byte col, byte row)
{
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(SSD1306_CMD);
 // https://github.com/greiman/SSD1306Ascii/blob/master/src/SSD1306init.h
  // Column
  Wire.write(0x00 | (col & 0XF));
  Wire.write(0x10 | (col >> 4));
  // Row
  Wire.write(0XB0 | row);
  Wire.endTransmission(); 
}

void drawDots(byte x, byte y,byte value)
{
  setOledCursor(x,y);
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(SSD1306_DATA);
  Wire.write(value);
  Wire.endTransmission(); 
}
byte bVal;
byte buffer[192];
void drawNumber(byte x, byte y, int index )
{
  // Reading from PROGMEM: https://forum.arduino.cc/index.php?topic=560344.0 + https://www.arduino.cc/reference/tr/language/variables/utilities/progmem/
  memcpy_P(buffer, (byte *)pgm_read_word(&(numbers[index])), 192);
  for (int j=0;j<NUMBER_HEIGHT/8;j++)
  {
    setOledCursor(x,y+j);
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(SSD1306_DATA);
    for (int i=0;i<NUMBER_WIDTH;i++)
    {
      Wire.write((byte)buffer[i+j*NUMBER_WIDTH]);
    }
    Wire.endTransmission(); 
  }
}

void drawComma(byte x, byte y)
{
  setOledCursor(x+1,y+5);
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(SSD1306_DATA);

  Wire.write(B00111100);
  for (int i=0;i<commaWidth-4;i++)
    Wire.write(B01111110);
  Wire.write(B00111100);
  
  Wire.endTransmission(); 
}

void drawMinus(byte x, byte y)
{
  setOledCursor(x,y+3);
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(SSD1306_DATA);

  Wire.write(B00000110);
  for (int i=0;i<digitStartX-3;i++)
  {
    Wire.write(B00001111);
  }
  Wire.write(B00000110);
  
  Wire.endTransmission();
}

void drawDegree(byte x, byte y)
{
  setOledCursor(x+2,y);
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(SSD1306_DATA);

  Wire.write(B00111100);

  Wire.write(B00100100);
  Wire.write(B01000010);
  Wire.write(B01000010);
  Wire.write(B00100100);
  
  Wire.write(B00111100);
  
  Wire.endTransmission();
}

void clearScreen()
{
  byte fillVal = 0; //B10101010;
  byte numCols = 128;
  byte numRows = 8;
  setOledCursor(0,0);

  for (byte i=0;i<numCols;i++)
  {
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(SSD1306_DATA);
    for (byte j=0;j<numRows;j++)
    {  
      Wire.write(fillVal);
    }
    Wire.endTransmission(); 
  }
  setOledCursor(0,0);
}

void drawFloat(float value)
{
  drawDegree(digitStartX+32+32+commaWidth+32, yOffset);
  if (value>99)
  {
    drawNumber(digitStartX,0,0);
    drawNumber(digitStartX+32,0,0);
    drawNumber(digitStartX+32+32+5,0,0);
    return;
  }
  else
  {
    float absValue = sqrt(value*value);
    int firstDigit = absValue/10;
    int secondDigit = absValue-(firstDigit*10);
    int thrdDigit = absValue*10 - (floor(absValue))*10;
    if (firstDigit>0)
      drawNumber(digitStartX,yOffset,firstDigit);
    drawNumber(digitStartX+32,yOffset,secondDigit);
    drawComma(digitStartX+32+32,yOffset);
    drawNumber(digitStartX+32+32+commaWidth,yOffset,thrdDigit);
  }
  if (value<0)
    drawMinus(0,yOffset);

}

float getTemp()
{
  sensors.requestTemperatures(); 
  float celsius = sensors.getTempCByIndex(0);
  return celsius;
}

void setup() {
  Serial.begin(9600);
  initDisplay();
  changeContrast(DISPLAY_CONTRAST_VALUE);
  clearScreen();
}


void loop() {
  // put your main code here, to run repeatedly:
  drawFloat(getTemp());
  delay(READ_INTERVALL_MS);
}
