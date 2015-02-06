/****************************************************************************
 Copyright:      Kai Riedel
 Author:         Kai Riedel
 Remarks:        AVR
 Version:        29.04.2011
 Description:    LCD Control functions
//------------------------------------------------------------------------------*/

#include "lcd.h"
#include <util/delay.h>

// Defines custom characters
char lcd_custom_char[3][8] = {
     {0x04,0x0E,0x15,0x04,0x04,0x04,0x04,0x04}, // Arrow up.
     {0x04,0x04,0x04,0x04,0x04,0x15,0x0E,0x04}, // Arrow down.
     {0x0C,0x12,0x12,0x0C,0x00,0x00,0x00,0x00}, // Degree sign.
};

//------------------------------------------------------------------------------
void lcd_init (void)
//------------------------------------------------------------------------------
{
	//Set Port Direction Register to Output for LCD
	LCD_Port_DDR |= LCD_DataOutput+(1<<LCD_RS | 1<<LCD_E);


	//Wait a short Time after Power on
	_delay_ms(1);
	lcd_write (0x28,0);	//Init in 4 Bit Mode
	_delay_ms(1);
	lcd_write (0x28,0);
	_delay_ms(2);
	lcd_write (0x0C,0);	//Display On
	_delay_ms(1);
	lcd_load_user_chars();
	_delay_ms(1);
	lcd_write (0x01,0);	//Display clear
	_delay_ms(1);
	lcd_pos (1,1);	 	//Set Home Position
	_delay_ms (1);
}
//------------------------------------------------------------------------------
void lcd_load_user_chars (void)
//------------------------------------------------------------------------------
{
    for (int j=0; j<3; j++)
    {
        /* 0X40 is the initial CGRAM address. Because each character needs a total amount of 8 memory
        locations, we increment addres in 8 units after each character.*/
        lcd_write(0x40+8*j, 0);
         _delay_ms(1);

        for (int i=0; i<8; i++)
        {
             lcd_write(lcd_custom_char[j][i],1); // write 8 bytes from character table
        }
        lcd_write(0x80,0);
    }
}

//------------------------------------------------------------------------------
void lcd_write (char Data,char CD)
//------------------------------------------------------------------------------
{
	_delay_us(100);

	//Soll ins Steuer- oder Datenregister (CD=0/1) geschrieben werden?
	if (CD == 0)
		{
		LCD_Port_Write &=~ (1<<LCD_RS); //RS = 0 Steuerregister
		//LCD_Port_Write = 0;
		}
		else
		{
		//LCD_Port_Write = 0x10;
		if (Data == 0x23) Data = 0x02; //replace character # with grad sign
		LCD_Port_Write |= (1<<LCD_RS); //RS = 1 Dataregister
		}


	//Schreiben der oberen 4Bit an das LCD Display
	LCD_Port_Write = (LCD_Port_Write&0xF0) + ((Data&0xF0)>>4); //Write Nibbel MSB
	LCD_Port_Write |= (1<<LCD_E);
	_delay_us(5);
	LCD_Port_Write &=~ (1<<LCD_E);

	//Schreiben der unteren 4Bit an das LCD Display
	LCD_Port_Write = (LCD_Port_Write&0xF0) + (Data&0x0F); //Write Nibbel LSB
	LCD_Port_Write |= (1<<LCD_E);
	_delay_us(5);
	LCD_Port_Write &=~ (1<<LCD_E);

	_delay_us(100);

}


//------------------------------------------------------------------------------
void lcd_print_value (long anzeige, char *einheit, char vorz, uint Zeile, uint Spalte, uint vk, uint nk)
//------------------------------------------------------------------------------
{
	char buffer[16];

	ltoa_format(buffer,anzeige,vk,nk,vorz);
	lcd_pos (Zeile, Spalte);
	lcd_print_str (buffer);
	lcd_print_str (einheit);
}

//------------------------------------------------------------------------------
void lcd_print_str (char *Buffer)
//------------------------------------------------------------------------------
{
	while (*Buffer != 0)
		{
		lcd_write (*Buffer++,1);
		};
};

//------------------------------------------------------------------------------
void lcd_clear (void)
//------------------------------------------------------------------------------
{
	lcd_write (1,0); //Clear Display
	_delay_ms (1);
	lcd_pos (1,1);	 //Set Home Position
	_delay_ms (1);
}

//------------------------------------------------------------------------------
void lcd_pos (uint Zeile, uint Spalte)
//------------------------------------------------------------------------------

{
   if (Zeile < 3) lcd_write ((0x80 + Spalte-1 + 0x40*(Zeile-1)),0); //Set Position row 1+2
   else lcd_write ((0x80 + Spalte+19 + 0x40*(Zeile-3)),0); //Set Position row 3+4
   _delay_ms(1);
}


//------------------------------------------------------------------------------
int ltoa_format (char *erg, long zahl, uint vk, uint nk,  char vorz )
//------------------------------------------------------------------------------
{ // Out-String, input long, pre-decimal digits, decimal digits, sign '+' or ' '
   long temp;
   int  i;
   i = vk + nk + 1;                   	// string length
   erg[i--] = 0;                      	// string end
   if ( zahl == 0 )						// special case input = 0
   {
      while( i >= 0 &&
	       ( zahl > 0 || i+2*( nk != 0 ) >= vk) ) // vk contain sign
	   	{
	      if (i==vk&&nk!=0) erg[i--]='.';  // decimal point, if nk is executed
	      else erg[i--] = '0'; // detach digit, value = 0
	  	}
      if ( i >= 0 ) erg[i--] = vorz;      	// write sign
	}
   else
   {
      if ( zahl < 0 )
      {
      vorz  = '-';                     // sign = '-'
      zahl *=  -1;                     // calculate further with positive value
      }
      while( i >= 0 &&
	       ( zahl > 0 || i+2*( nk != 0 ) > vk) ) // vk contain sign
      {
      if (i==vk&&nk!=0) erg[i--]='.';  // decimal point, if nk is executed
      else
      {
      temp     =  zahl / 10;           	// integer division
      erg[i--] = (zahl - temp*10) + 48; // detach digit, assign ASCII-value
      zahl     =  temp;                	// for next pass reduce digit
      }
   	  }
   if ( i >= 0 ) erg[i--] = vorz;      	// write sign
   }
   while( i >= 0 ) erg[i--] = ' ';     	// fill begin with spaces
   return  vk + nk + 1;                	// return string length
}
