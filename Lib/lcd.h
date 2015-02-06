/****************************************************************************
 Copyright:      Kai Riedel
 Author:         Kai Riedel
 Remarks:        AVR
 Version:        31.03.2011
 Description:    LCD Control functions
//------------------------------------------------------------------------------*/

#ifndef _LCD_H_
 #define _LCD_H_

#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

// define shortcuts
#define uint  unsigned int
#define uchar unsigned char


//Prototypes
extern void lcd_write (char,char);
extern void lcd_init (void);
extern void lcd_clear (void);
extern void lcd_pos (uint Zeile, uint Spalte);
extern void lcd_print_str (char *Buffer);
extern void lcd_load_user_chars (void);
extern int ltoa_format (char *erg, long zahl, uint vk, uint nk,  char vorz );
extern void lcd_print_value (long anzeige, char *einheit, char vorz, uint Zeile, uint Spalte, uint vk, uint nk);


//LCD_D0 - LCD_D3 connect to GND
//Im 4Bit Mode LCD_D4-->PORTx.0 ........ LCD_D7-->PORTx.3
//LCD_RS --> PORTx.4 | LCD_RW --> GND | LCD_E --> PORTx.6 | PORTx.7-->NotConnect

#define LCD_Port_DDR			DDRC	//Port an dem das Display angeschlossen wurde
#define LCD_Port_Write			PORTC
#define LCD_Port_Read			PINC

#define LCD_RS					4 		//Pin für RS
#define LCD_E					6 		//Pin für Enable

#define LCD_DataOutput			0x0f

#endif //_LCD_H_

