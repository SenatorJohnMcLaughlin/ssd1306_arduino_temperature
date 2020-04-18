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


// https://github.com/datacute/Tiny4kOLED/blob/master/src/Tiny4kOLED_common.h
static const uint8_t oled_init_128x64new [] PROGMEM = {
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
