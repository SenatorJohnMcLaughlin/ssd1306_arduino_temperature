// >> https://www.kollino.de/arduino/oled-display-mit-ssd1306-chipsatz-via-i2c-an-arduino-anschliessen/
#include <avr/pgmspace.h>

// >> DS18B20 Tutorial: https://draeger-it.blog/arduino-lektion-48-temperatursensor-ds18b20/?cn-reloaded=1
#include <OneWire.h>
#include <DallasTemperature.h>


// Sleep-Mode: http://shelvin.de/arduino-in-den-sleep_mode_pwr_down-schlaf-modus-setzen/
// Additional: https://www.hackster.io/Itverkx/sleep-at-tiny-371f04
#include <avr/sleep.h> 
#ifndef cbi
  #define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
  #define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


//#include <Wire.h>
#include <TinyWireM.h>
#define Wire TinyWireM 

//Attiny i2c address:  https://bitbucket.org/tinusaur/ssd1306xled/src/default/ssd1306xled/ssd1306xled.h
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
#define COLUMN_WIDTH 16
#define DISPLAY_CONTRAST_VALUE 255    // 0-255

#define TEMPERATURE_PIN 4
#define MIN_TEMPERATURE 15
#define MAX_TEMPERATURE 50

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);

int yOffset=0;
int commaWidth = 7;
byte digitStartX = 16;
float previousTemp = 0.0;

// => https://www.hackster.io/Itverkx/sleep-at-tiny-371f04
#define CYCLE_TIME 4 // => 4*4=16sec; 15*4 = 60sec
// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
int ii = 8; // => 4 seconds

// => https://www.hackster.io/Itverkx/sleep-at-tiny-371f04
void watchdogOn() {
  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;
  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
  // switch Analog to Digitalconverter OFF
  cbi(ADCSRA,ADEN);                    
}

// SleepLen depending on set Timer Prescaler
void pwrDown(int sleepLen) {
//  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // den tiefsten Schlaf auswählen PWR_DOWN
  for(int i=0; i < sleepLen; i++) {
    sleep_enable(); // sleep mode einschalten
    sleep_mode(); // in den sleep mode gehen
    sleep_disable(); // sleep mode ausschalten nach dem Erwachen
  }
//  sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON
}
ISR(WDT_vect) {}

static const byte num_one[] PROGMEM = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  56, 124,  124,  124,  124,  124,  124,  252,  252,  252,  252,  248,  248,  240,  224,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  255,  255,  255,  255,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  255,  255,  255,  255,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  255,  255,  255,  255,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  255,  255,  255,  255,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  31, 63, 63, 63, 63, 31, 0,  0,  0,  0,  0,  0,  0,  0
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
  0,  0,  0,  28, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 126,  126,  254,  254,  252,  252,  248,  240,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  255,  255,  255,  255,  255,  0,
0,  0,  0,  0,  0,  0,  0,  0,  224,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  240,  248,  248,  252,  255,  255,  63, 31, 7,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,  3,  7,  255,  255,  255,  254,  248,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  128,  255,  255,  255,  255,  255,  0,
0,  0,  0,  28, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 63, 63, 63, 63, 31, 31, 15, 7,  0

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


// https://github.com/datacute/Tiny4kOLED/blob/master/src/Tiny4kOLED_common.h
static const uint8_t tiny4koled_init_128x64new [] PROGMEM = {
  0x20, 0x00,   // Set Memory Addressing Mode; 00=Horizontal Addressing Mode; 01=Vertical Addressing Mode;
  0xB0,         // Set Page Start Address for Page Addressing Mode, 0-7
  0xA8, 0x3F,   // Set multiplex ratio(1 to 64)
  0xA4,         // Output RAM to Display; // 0xA4=Output follows RAM content; 0xA5,Output ignores RAM content
  0xD3,0x00,    // Set display offset. 00 = no offset
  0xD5,0xF0,    // --set display clock divide ratio/oscillator frequency 
//  0xD9,0x22,    // Set pre-charge period
//  0xDA,0x00,    // Set com pins hardware configuration
//  0xDB,0x20,  // --set vcomh // 0x20,0.77xVcc
  0xA6, // Set display mode. A6=Normal; A7=Inverse
  
  0xC8,         // Set COM Output Scan Direction
//  0x00,  // ---set low column address
//  0x10,  // ---set high column address
//  0x40,  // ---set start line address

  0xA1,         // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
  0x8D, 0x14    // Set DC-DC enable 7.5V (We can't see the screen without the charge pump on)
//  0xAF          // Display on
};

// based on: https://github.com/ex-punctis/SSD1306_OLED_HW_I2C/blob/master/SSD1306_OLED_HW_I2C.c
void initDisplay()
{
  Wire.begin();
//  Wire.setClock(400000L);
  Wire.beginTransmission(I2C_ADDRESS);
  while (!Wire.endTransmission()==0x00)delay(10);

  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(SSD1306_CMD);

  for (uint8_t i = 0; i < sizeof(tiny4koled_init_128x64new)/sizeof(tiny4koled_init_128x64new[0]); i++) {
    Wire.write(pgm_read_byte(&tiny4koled_init_128x64new[i]));
  }

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

void sendCmd(byte cmd)
{
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(SSD1306_CMD);
  Wire.write(cmd);
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
   for (int k=0;k<NUMBER_WIDTH/COLUMN_WIDTH;k++)
   { 
     setOledCursor(x+k*COLUMN_WIDTH,y+j);
     Wire.beginTransmission(I2C_ADDRESS);
     Wire.write(SSD1306_DATA);
      for (int i=0;i<COLUMN_WIDTH;i++)
     {
       Wire.write((byte)buffer[i+k*COLUMN_WIDTH+j*NUMBER_WIDTH]);
     }
     Wire.endTransmission(); 
    }
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

void drawLineX(byte x ,byte y)
{
 setOledCursor(x,y);
 Wire.beginTransmission(I2C_ADDRESS);
 Wire.write(SSD1306_DATA);

 //Wire.write(B10111101);

  for (int i=0;i<8;i++)
    Wire.write(B10100011);
  for (int i=0;i<8;i++)
    Wire.write(B11111111);
  
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
  if (celsius>MAX_TEMPERATURE)return 0.0;
  return celsius;
}

void setup() {
  watchdogOn(); // Watchdog timer einschalten.
  initDisplay();
  delay(20);
  changeContrast(DISPLAY_CONTRAST_VALUE);
  clearScreen();
}


void loop() {
  // put your main code here, to run repeatedly:
  float currentTemp = getTemp();
  // Only show temp. if in reasonable range
  if (previousTemp>=MIN_TEMPERATURE && currentTemp<MIN_TEMPERATURE)
    sendCmd(0xAE);
    else if (previousTemp<MIN_TEMPERATURE && currentTemp>=MIN_TEMPERATURE)
      sendCmd(0xAF);
    
  if (currentTemp>=MIN_TEMPERATURE)
    drawFloat(currentTemp);
  
  previousTemp=currentTemp;
  pwrDown(CYCLE_TIME);
}
