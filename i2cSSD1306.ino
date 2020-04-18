// >> https://www.kollino.de/arduino/oled-display-mit-ssd1306-chipsatz-via-i2c-an-arduino-anschliessen/
#include <avr/pgmspace.h>
// Fonts for number-display
#include "ssd1306_numfonts.h"
// communication commands for ssd1306
#include "ssd1306_device.h"
// >> Temperature Sensor - DS18B20 Tutorial: https://draeger-it.blog/arduino-lektion-48-temperatursensor-ds18b20/?cn-reloaded=1
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

// for Arduino
//#include <Wire.h> 
// for Attiny
#include <TinyWireM.h>  
#define Wire TinyWireM 

#define TEMPERATURE_PIN 4
// Display temp. only if in range
#define MIN_TEMPERATURE 15
#define MAX_TEMPERATURE 50

OneWire oneWire(TEMPERATURE_PIN);
DallasTemperature sensors(&oneWire);

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
ISR(WDT_vect) {}  // if this isn't executed -> leads to flickering of oled after update - why?


int yOffset=0;
int commaWidth = 7;
byte digitStartX = 16;
float previousTemp = 0.0;

/*    based on: https://github.com/ex-punctis/SSD1306_OLED_HW_I2C/blob/master/SSD1306_OLED_HW_I2C.c & 
 *    https://github.com/datacute/Tiny4kOLED/blob/master/src/Tiny4kOLED_common.h  */
void initDisplay()
{
  Wire.begin();
//  Wire.setClock(400000L);
  Wire.beginTransmission(I2C_ADDRESS);
  while (!Wire.endTransmission()==0x00)delay(10);

  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(SSD1306_CMD);

  for (uint8_t i = 0; i < sizeof(oled_init_128x64new)/sizeof(oled_init_128x64new[0]); i++) {
    Wire.write(pgm_read_byte(&oled_init_128x64new[i]));
  }

  Wire.endTransmission(); 
}

void sendCmd(byte cmd)
{
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(SSD1306_CMD);
  Wire.write(cmd);
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
  sendCmd(FLOOD_DISPLAY);
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

// Draw temperature on oled
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
    // extract + display 2 digit + 1 fraction digit from value
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
    sendCmd(0xAE);  // switch display off
    else if (previousTemp<MIN_TEMPERATURE && currentTemp>=MIN_TEMPERATURE)
      sendCmd(0xAF);  // switch display on
    
  if (currentTemp>=MIN_TEMPERATURE)
    drawFloat(currentTemp);
  
  previousTemp=currentTemp;
  pwrDown(CYCLE_TIME);
}
