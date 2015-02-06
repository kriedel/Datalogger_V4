/* small color graphics LCD based on ILI9341 chip */
/* ili9341.h */
#include <avr/io.h>
#include "Lib/DataflashManager.h"
#include <LUFA/Drivers/Peripheral/Serial.h>

/* hardware definition. Data is in SPI */
#define CONF_ILI9341_CS_PIN        _BV(0)
#define CONF_ILI9341_DC_PIN        _BV(6)
//#define CONF_ILI9341_BACKLIGHT_PIN 0
#define CONF_ILI9341_RESET_PIN     _BV(7)

//TFT resolution 240*320
#define MIN_X	0
#define MIN_Y	0
#define MAX_X	239
#define MAX_Y	319

#define FONT_SPACE 6
#define FONT_X 8
#define FONT_Y 8

#define ILI9341_COLOR(red, green, blue)   ((unsigned int)( (( red >> 3 ) << 11 ) | (( green >> 2 ) << 5  ) |  ( blue  >> 3 ))) //5 red | 6 green | 5 blue

 #define BLACK                       ILI9341_COLOR(0x00, 0x00, 0x00)
 #define WHITE                       ILI9341_COLOR(0xFF, 0xFF, 0xFF)
 #define RED                         ILI9341_COLOR(0xFF, 0x00, 0x00)
 #define GREEN                       ILI9341_COLOR(0x00, 0xFF, 0x00)
 #define BLUE                        ILI9341_COLOR(0x00, 0x00, 0xFF)
 #define YELLOW                      ILI9341_COLOR(0xFF, 0xFF, 0x00)
 #define MAGENTA                     ILI9341_COLOR(0xFF, 0x00, 0xFF)
 #define CYAN                        ILI9341_COLOR(0x00, 0xFF, 0xFF)
 #define GRAY                        ILI9341_COLOR(0x80, 0x80, 0x40)
 #define SILVER                      ILI9341_COLOR(0xA0, 0xA0, 0x80)
 #define GOLD                        ILI9341_COLOR(0xA0, 0xA0, 0x40)

uint8_t* font;

 //Prototypes
void ili9341_select_chip(void);
void ili9341_deselect_chip(void);
void ili9341_select_command_mode(void);
void ili9341_select_data_mode(void);
void ili9341_send_byte(uint8_t data);
void ili9341_init(void);
void ili9341_interface_init(void);
void ili9341_reset_display(void);
void ili9341_controller_init_registers(void);
uint8_t ili9341_read_ID(void);
void lcdFillRGB(uint16_t color);
void ili9341_setCol(uint16_t x0,uint16_t x1);
void ili9341_setPage(uint16_t y0,uint16_t y1);
void ili9341_setXY(uint16_t x, uint16_t y);
void ili9341_send_word(uint16_t data);
void ili9341_send_byte(uint8_t data);
void ili9341_select_data_mode(void);
void ili9341_select_command_mode(void);
void ili9341_deselect_chip(void);
void ili9341_select_chip(void);
void ili9341_exit_standby(void);
void lcdDrawHLine(uint16_t x0, uint16_t y, uint16_t x1, uint16_t color);
void lcdDrawVLine(uint16_t x, uint16_t y0, uint16_t y1, uint16_t color);
void lcdDrawPixel(uint16_t poX, uint16_t poY, uint16_t color);
void lcdDrawBMP(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *bitmap);
int lcdDrawBMP16(uint16_t x, uint16_t y, const char *Name_BMP);
void lcdDrawRectangle(uint16_t poX, uint16_t poY, uint16_t length, uint16_t width, uint16_t color);
void lcdDrawCircle(int16_t poX, int16_t poY, int16_t r,uint16_t color);
void lcdFillCircle(int16_t poX, int16_t poY, int16_t r,uint16_t color);
void lcdFilledRec(uint16_t x, uint16_t y, uint16_t w, uint16_t h,uint16_t color);
uint16_t lcdDrawChar( uint8_t c, uint16_t char_x, uint16_t char_y, uint16_t foreground, uint16_t background);
void lcdDrawString(const char *string,uint16_t poX, uint16_t poY, uint16_t fgcolor,uint16_t bgcolor);
void lcdDrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color);
void lcdSetFont(unsigned char* f);
