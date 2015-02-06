/* small color graphics LCD based on ILI9341 chip */
/* ili9341.c */
/* 3V version! */

#include "ili9341.h"
#include "ili9341_regs.h"

#define ORIENTATION 1

#define LCD_WIDTH 320
#define LCD_HEIGHT 240

void ili9341_select_chip(void)
{
	PORTB &= ~CONF_ILI9341_CS_PIN;
}
void ili9341_deselect_chip(void)
{
	PORTB |= CONF_ILI9341_CS_PIN;
}
void ili9341_select_command_mode(void)
{
	PORTB &= ~CONF_ILI9341_DC_PIN;
}
void ili9341_select_data_mode(void)
{
	PORTB |= CONF_ILI9341_DC_PIN;
}
void ili9341_send_byte(uint8_t data)
{
    //ili9341_select_chip();
	SPI_TransferByte(data);
	//ili9341_deselect_chip();
}
void ili9341_send_word(uint16_t data)
{
	SPI_TransferByte(data >> 8);
	SPI_TransferByte(data );
}
uint8_t ili9341_read_byte(void)
{
	ili9341_select_chip();
	return SPI_TransferByte(0);
}
void ili9341_send_command(uint8_t command)
{
    ili9341_select_command_mode();
    ili9341_select_chip();
    ili9341_send_byte(command);
    ili9341_select_data_mode();
}
void ili9341_interface_init(void)
{
	DDRB |= CONF_ILI9341_RESET_PIN | CONF_ILI9341_DC_PIN | CONF_ILI9341_CS_PIN;
	ili9341_deselect_chip();
	SPI_TransferByte(0);
}
void ili9341_write_buffer(char *buffer, uint8_t len)
{
uint8_t count, data1; //, data2;
ili9341_select_data_mode();
ili9341_select_chip();
for(count=0;count<len;count++)
	{
		data1 = buffer[count]; //>>8;
		//data2 = buffer[count] & 0xFF;
		SPI_TransferByte( data1 );
		//SPI_MasterTransmit( data2 );
	}

//INT16U    data1 = 0;
//    INT8U   data2 = 0;
//    TFT_DC_HIGH;
//    TFT_CS_LOW;
//    INT8U count=0;
//    for(count=0;count<howmany;count++)
//    {
//        data1 = data[count]>>8;
//        data2 = data[count]&0xff;
//        SPI.transfer(data1);
//        SPI.transfer(data2);
//    }
//    TFT_CS_HIGH;

}


uint8_t ili9341_read_register(uint8_t addr, uint8_t xparam)
{
    uint8_t data;
	ili9341_send_command(0xD9); /* ext command                  */
	ili9341_send_byte(0x10+xparam); /* 0x11 is the first Parameter  */
	ili9341_select_command_mode();
	ili9341_select_chip();
	ili9341_send_byte(addr);
	ili9341_select_data_mode();
	data=ili9341_read_byte();
	ili9341_deselect_chip();
	return data;
}

//INT8U TFT::readID(void)
uint8_t ili9341_read_ID(void)
{
uint8_t i;
uint8_t data[3];
uint8_t id[3]= {0x00, 0x93, 0x41};
uint8_t ToF=0xFF;
    for(i=0;i<3;i++)
    {
        data[i]=ili9341_read_register(0xd3,i+1);
        if(data[i] != id[i])
        {
            ToF=0;
        }
    }

    if(!ToF)                                                            /* data!=ID                     */
    {
//        Serial.print("Read TFT ID failed, ID should be 0x09341, but read ID = 0x");
//        for(i=0;i<3;i++)
//        {
//            Serial.print(data[i],HEX);
//       }
//        Serial.println();
		while(1){}
	}
   return ToF;
}





/*
void ili9341_drawChar( unsigned char ascii, unsigned int poX, unsigned int poY,unsigned int size, unsigned int fgcolor)
{
    if((ascii>=32)&&(ascii<=127))
    {
        ;
    }
    else
    {
        ascii = '?'-32;
    }
    for (int i =0; i<FONT_X; i++ ) {
        unsigned char temp = pgm_read_byte(&simpleFont[ascii-0x20][i]);
        for(unsigned char f=0;f<8;f++)
        {
            if((temp>>f)&0x01)
            {
                //fillRectangle(poX+i*size, poY+f*size, size, size, fgcolor);
            }

        }

    }
}
*/


void ili9341_controller_init_registers(void)
{
    uint8_t i=0, TFTDriver=0;
    for(i=0;i<3;i++)
	{
		TFTDriver = ili9341_read_ID();
	}
	_delay_ms(10);

	ili9341_send_command(ILI9341_CMD_SOFTWARE_RESET);
	_delay_ms(5);

    ili9341_send_command(ILI9341_CMD_DISPLAY_OFF);
    _delay_ms(5);

	ili9341_send_command(ILI9341_CMD_SLEEP_OUT);
	ili9341_deselect_chip();
    _delay_ms(5);

	ili9341_send_command(ILI9341_CMD_POWER_CONTROL_B);
    ili9341_send_byte(0x00);
    ili9341_send_byte(0x83); //8B
    ili9341_send_byte(0X30);

    ili9341_send_command(ILI9341_CMD_POWER_SEQUENCE_CONTROL);
    ili9341_send_byte(0x64); //67
    ili9341_send_byte(0x03);
    ili9341_send_byte(0X12);
    ili9341_send_byte(0X81);

    ili9341_send_command(ILI9341_CMD_DRIVER_TIMING_CONTROL_A);
    ili9341_send_byte(0x85);
    ili9341_send_byte(0x01); //10
    ili9341_send_byte(0x79); //7A

    ili9341_send_command(ILI9341_CMD_POWER_ON_SEQ_CONTROL);
    ili9341_send_byte(0x39);
    ili9341_send_byte(0x2C);
    ili9341_send_byte(0x00);
    ili9341_send_byte(0x34);
    ili9341_send_byte(0x02);

    ili9341_send_command(ILI9341_CMD_PUMP_RATIO_CONTROL);
    ili9341_send_byte(0x20);

    ili9341_send_command(ILI9341_CMD_DRIVER_TIMING_CONTROL_B);
    ili9341_send_byte(0x00);
    ili9341_send_byte(0x00);

    ili9341_send_command(ILI9341_CMD_POWER_CONTROL_1 );
    ili9341_send_byte(0x26); //1B

    ili9341_send_command(ILI9341_CMD_POWER_CONTROL_2);
    ili9341_send_byte(0x11); //10

    ili9341_send_command(ILI9341_CMD_VCOM_CONTROL_1);
    ili9341_send_byte(0x35); //3F
    ili9341_send_byte(0x3E); //3C

    ili9341_send_command(ILI9341_CMD_VCOM_CONTROL_2);
    ili9341_send_byte(0xBE); //B7

    ili9341_send_command(ILI9341_CMD_MEMORY_ACCESS_CONTROL);
    //ili9341_send_byte(0x28);

    switch (ORIENTATION) {
        case 0:
            ili9341_send_byte(0x48);
            break;
        case 1:
            ili9341_send_byte(0x28);
            break;
        case 2:
            ili9341_send_byte(0x88);
            break;
        case 3:
            ili9341_send_byte(0xE8);
            break;
    }


    ili9341_send_command(ILI9341_CMD_COLMOD_PIXEL_FORMAT_SET);
    ili9341_send_byte(0x55);

    ili9341_send_command(ILI9341_CMD_FRAME_RATE_CONTROL_NORMAL);
    ili9341_send_byte(0x00);
    ili9341_send_byte(0x1B);

    ili9341_send_command(ILI9341_CMD_DISPLAY_FUNCTION_CONTROL);	// Display Function Control
    ili9341_send_byte(0x0A);
    ili9341_send_byte(0x82);
    ili9341_send_byte(0x27);
    ili9341_send_byte(0x00);

    ili9341_send_command(ILI9341_CMD_ENABLE_3_GAMMA_CONTROL);	// 3Gamma Function Disable
    ili9341_send_byte(0x08);

    ili9341_send_command(ILI9341_CMD_GAMMA_SET); 				// Gamma curve selected
    ili9341_send_byte(0x01);

    ili9341_send_command(ILI9341_CMD_POSITIVE_GAMMA_CORRECTION);	// Set Gamma
    ili9341_send_byte(0x1F);
    ili9341_send_byte(0x1A);
    ili9341_send_byte(0x18);
    ili9341_send_byte(0x0A);
    ili9341_send_byte(0x0F);
    ili9341_send_byte(0x06);
    ili9341_send_byte(0x45);
    ili9341_send_byte(0X87);
    ili9341_send_byte(0x32);
    ili9341_send_byte(0x0A);
    ili9341_send_byte(0x07);
    ili9341_send_byte(0x02);
    ili9341_send_byte(0x07);
    ili9341_send_byte(0x05);
    ili9341_send_byte(0x00);

    ili9341_send_command(ILI9341_CMD_NEGATIVE_GAMMA_CORRECTION);	// Set Gamma
    ili9341_send_byte(0x00);
    ili9341_send_byte(0x25);
    ili9341_send_byte(0x27);
    ili9341_send_byte(0x05);
    ili9341_send_byte(0x10);
    ili9341_send_byte(0x09);
    ili9341_send_byte(0x3A);
    ili9341_send_byte(0x78);
    ili9341_send_byte(0x4D);
    ili9341_send_byte(0x05);
    ili9341_send_byte(0x18);
    ili9341_send_byte(0x0D);
    ili9341_send_byte(0x38);
    ili9341_send_byte(0x3A);
    ili9341_send_byte(0x1F);

    ili9341_setCol( 0, LCD_WIDTH);
    ili9341_setPage(0, LCD_HEIGHT);

    ili9341_send_command(ILI9341_CMD_ENTRY_MODE_SET);	// Set Gamma
    ili9341_send_byte(0x07);
}

void ili9341_reset_display(void)
{
	PORTB |= CONF_ILI9341_RESET_PIN;
	_delay_ms(5);
	PORTB &= ~CONF_ILI9341_RESET_PIN;
	_delay_ms(5);
	PORTB |= CONF_ILI9341_RESET_PIN;
	_delay_ms(5);
}
void ili9341_exit_standby(void)
{
    ili9341_send_command(ILI9341_CMD_SLEEP_OUT);
    _delay_ms(100);
    ili9341_send_command(ILI9341_CMD_DISPLAY_ON);
    _delay_ms(100);
}
void ili9341_init(void)
{
    /* Initialize the communication interface */
    ili9341_interface_init();

    /* Harware Reset the display */
    ili9341_reset_display();

    /* Write all the controller registers with correct values */
    ili9341_controller_init_registers();
	    /* Send commands to exit standby mode */
    ili9341_exit_standby();
	ili9341_deselect_chip();

}



void ili9341_setCol(uint16_t x0,uint16_t x1)
{
    ili9341_send_command(0x2A);                                                      /* Column Command address       */
    ili9341_send_word(x0);
    ili9341_send_word(x1);
}

void ili9341_setPage(uint16_t y0,uint16_t y1)
{
    ili9341_send_command(0x2B);                                                      /* Column Command address       */
    ili9341_send_word(y0);
    ili9341_send_word(y1);
}

void ili9341_setXY(uint16_t x, uint16_t y)
{
    ili9341_setCol(x, x);
    ili9341_setPage(y, y);
}

void lcdSetFont(uint8_t* f)
{
    font = f;
}

/* ********************** common subs ***************** */

void lcdDrawPixel(uint16_t poX, uint16_t poY, uint16_t color)
{
    ili9341_setXY(poX, poY);
	ili9341_send_command(0x2c);
    ili9341_send_word(color);
    ili9341_deselect_chip();
}
/* ******** same as drawpixel, but bigger ***** */
void lcdFilledRec(uint16_t x, uint16_t y, uint16_t w, uint16_t h,uint16_t color)
{
uint32_t i;
uint32_t n=(uint32_t) w*h;
uint16_t x1=x+w-1;
uint16_t y1=y+h-1;

    ili9341_setCol( x, x1);
    ili9341_setPage(y, y1);
    ili9341_send_command(0x2c);
	for(i=0;i<n;i++) { ili9341_send_word(color); }
	ili9341_deselect_chip();
}
/* ********** Clear screen ************ */
void lcdFillRGB(uint16_t color)
{
	lcdFilledRec(0,0,LCD_WIDTH,LCD_HEIGHT,color);
}

/***********Line*************************************/
void lcdDrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t color)
{
    //WindowMax();
    int16_t   dx = 0, dy = 0;
    int16_t   dx_sym = 0, dy_sym = 0;
    int16_t   dx_x2 = 0, dy_x2 = 0;
    int16_t   di = 0;

    dx = x1-x0;
    dy = y1-y0;

    if (dx == 0) {        /* vertical line */
        if (y1 > y0) lcdDrawVLine(x0,y0,y1,color);
        else lcdDrawVLine(x0,y1,y0,color);
        return;
    }

    if (dx > 0) {
        dx_sym = 1;
    } else {
        dx_sym = -1;
    }
    if (dy == 0) {        /* horizontal line */
        if (x1 > x0) lcdDrawHLine(x0,y0,x1,color);
        else  lcdDrawHLine(x1,y0,x0,color);
        return;
    }

    if (dy > 0) {
        dy_sym = 1;
    } else {
        dy_sym = -1;
    }

    dx = dx_sym*dx;
    dy = dy_sym*dy;

    dx_x2 = dx*2;
    dy_x2 = dy*2;

    if (dx >= dy) {
        di = dy_x2 - dx;
        while (x0 != x1) {

            lcdDrawPixel(x0, y0, color);
            x0 += dx_sym;
            if (di<0) {
                di += dy_x2;
            } else {
                di += dy_x2 - dx_x2;
                y0 += dy_sym;
            }
        }
        lcdDrawPixel(x0, y0, color);
    } else {
        di = dx_x2 - dy;
        while (y0 != y1) {
            lcdDrawPixel(x0, y0, color);
            y0 += dy_sym;
            if (di < 0) {
                di += dx_x2;
            } else {
                di += dx_x2 - dy_x2;
                x0 += dx_sym;
            }
        }
        lcdDrawPixel(x0, y0, color);
    }
    return;
}

/***********H Line*************************************/
void lcdDrawHLine(uint16_t x0, uint16_t y, uint16_t x1, uint16_t color)
{
  // Allows for slightly better performance than setting individual pixels
  uint16_t x, pixels;

  if (x1 < x0)
  {
    // Switch x1 and x0
    x = x1;
    x1 = x0;
    x0 = x;
  }

  // Check limits
  if (x1 >= LCD_WIDTH)
  {
    x1 = LCD_WIDTH ;
  }
  if (x0 >= LCD_WIDTH)
  {
    x0 = LCD_WIDTH;
  }
    ili9341_setCol( x0, x1);
    ili9341_setPage(y, y);
    ili9341_send_command(0x2c);

   for (pixels = 0; pixels < x1 - x0 ; pixels++)
  {
    ili9341_send_word(color);
  }
    ili9341_deselect_chip();
}
/***********V Line*************************************/
void lcdDrawVLine(uint16_t x, uint16_t y0, uint16_t y1, uint16_t color)
{
 // Allows for slightly better performance than setting individual pixels
  uint16_t y, pixels;

  if (y1 < y0)
  {
    // Switch y1 and y0
    y = y1;
    y1 = y0;
    y0 = y;
  }

  // Check limits
  if (y1 >= LCD_HEIGHT)
  {
    y1 = LCD_HEIGHT - 1;
  }
  if (y0 >= LCD_HEIGHT)
  {
    y0 = LCD_HEIGHT - 1;
  }

    ili9341_setCol( x, x);
    ili9341_setPage(y0, y1);
    ili9341_send_command(0x2c);

  for (pixels = 0; pixels < y1 - y0 ; pixels++)
  {
	ili9341_send_word(color);
  }
    ili9341_deselect_chip();
}

void lcdDrawBMP(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *bitmap)
{
    uint16_t  j;
    int16_t padd;
    uint16_t *bitmap_ptr = (uint16_t *)bitmap;
    uint16_t pix_temp;
    uint16_t i;

    // the lines are padded to multiple of 4 bytes in a bitmap
    padd = -1;
    do {
        padd ++;
    } while (2*(w + padd)%4 != 0);

    ili9341_setCol( x, x+w-1);
    ili9341_setPage(y, y+h-1);

    bitmap_ptr += ((h - 1)* (w + padd));
    ili9341_send_command(0x2c);

    for (j = 0; j < h; j++) {         //Lines
        for (i = 0; i < w; i++) {     // one line
              pix_temp = pgm_read_word(bitmap_ptr);
              ili9341_send_byte(pix_temp >> 8);
              ili9341_send_byte(pix_temp);
              bitmap_ptr++;
              //bitmap_ptr++;
       }
        bitmap_ptr -= 2*w;
        bitmap_ptr -= padd;
    }
    ili9341_deselect_chip();

    ili9341_setCol( 0, LCD_WIDTH);  // window max
    ili9341_setPage(0, LCD_HEIGHT);
}
/* ************** copy bitmap from filesystem (RGB, 16bit) **********
bitmap format: 16 bit R5 G6 B5
use Gimp to create / load , save as BMP, option 16 bit R5 G6 B5 copy to internal file system or SD card  */
int lcdDrawBMP16(uint16_t x, uint16_t y, const char *Name_BMP)
{

#define OffsetPixelWidth    18
#define OffsetPixelHeigh    22
#define OffsetFileSize      34
#define OffsetPixData       10
#define OffsetBPP           28

    char filename[50];
    uint16_t BytesRead;
    unsigned char BMP_Header[54];
    uint16_t BPP_t;
    uint32_t off, i, start_data, PixelWidth, PixelHeigh;
    int16_t padd,j;
    //
    uint16_t *line;
    /** FAT Fs structure to hold a FAT file handle for the log data write destination. */
    FIL Image;
    FRESULT Result;

    // get the filename
    i=0;
    while (*Name_BMP!='\0') {
        filename[i++]=*Name_BMP++;
    }
    filename[i] = 0;

    Result = f_open(&Image, (const char *)&filename[0], FA_READ);

    if (Result != FR_OK) {
        return(0);                  // error file open
    }

    BytesRead = 54;
    Result = f_read(&Image, &BMP_Header[0],BytesRead,&BytesRead);      // get the BMP Header

    if (BMP_Header[0] != 0x42 || BMP_Header[1] != 0x4D) {  // check magic byte
        f_close (&Image);
        return(-1);     // error no BMP file
    }

    BPP_t = BMP_Header[OffsetBPP] + (BMP_Header[OffsetBPP + 1] << 8);
    if (BPP_t != 0x0010) {
        f_close (&Image);
        return(-2);     // error no 16 bit BMP
    }

    PixelHeigh = BMP_Header[OffsetPixelHeigh] + (BMP_Header[OffsetPixelHeigh + 1] << 8) + (BMP_Header[OffsetPixelHeigh + 2] << 16) + (BMP_Header[OffsetPixelHeigh + 3] << 24);
    PixelWidth = BMP_Header[OffsetPixelWidth] + (BMP_Header[OffsetPixelWidth + 1] << 8) + (BMP_Header[OffsetPixelWidth + 2] << 16) + (BMP_Header[OffsetPixelWidth + 3] << 24);

    if (PixelHeigh > LCD_HEIGHT + y || PixelWidth > LCD_WIDTH + x) {
        f_close (&Image);
        return(-3);      // to big
    }

    start_data = BMP_Header[OffsetPixData] + (BMP_Header[OffsetPixData + 1] << 8) + (BMP_Header[OffsetPixData + 2] << 16) + (BMP_Header[OffsetPixData + 3] << 24);

    line = (uint16_t *) malloc (2 * PixelWidth); // we need a buffer for a line
    if (line == NULL) {
        return(-4);         // error no memory
    }

    // the bmp lines are padded to multiple of 4 bytes
    padd = -1;
    do {
        padd ++;
    } while ((PixelWidth * 2 + padd)%4 != 0);


    ili9341_setCol( x, x+PixelWidth-1);
    ili9341_setPage(y, y+PixelHeigh-1);

    ili9341_send_command(0x2c); // send pixel

    for (j = PixelHeigh - 1; j >= 0; j--)
    {               //Lines bottom up
        off = j * (PixelWidth  * 2 + padd) + start_data;   // start of line

        ili9341_deselect_chip();                // flash is on the same spi bus as lcd controller
        Result= f_lseek(&Image, off);
        Result = f_read(&Image, line, PixelWidth * 2, &BytesRead);   // read a line - slow
        ili9341_select_chip();

        for (i = 0; i < PixelWidth     ; i++)           // copy pixel data to TFT
        {
            ili9341_send_byte(line[i] >> 8);
            ili9341_send_byte(line[i]);
        }

     }

    ili9341_deselect_chip();
    f_close (&Image);
    ili9341_setCol( 0, LCD_WIDTH);  // window max
    ili9341_setPage(0, LCD_HEIGHT);
    return(1);
}



/* ********draw simple frame (rectangle ) *********** */
void lcdDrawRectangle(uint16_t poX, uint16_t poY, uint16_t length, uint16_t width, uint16_t color)
{
    lcdDrawHLine(poX, poY, poX + length-1, color);
    lcdDrawHLine(poX, poY + width-1, poX+length, color);

    lcdDrawVLine(poX, poY, poY + width-1, color);
    lcdDrawVLine(poX + length-1, poY, poY+width-1, color);
    ili9341_deselect_chip();
}

/* yeap, round circule */
void lcdDrawCircle(int16_t poX, int16_t poY, int16_t r,uint16_t color)
{
    int x = -r, y = 0, err = 2-2*r, e2;
    do {
        lcdDrawPixel(poX-x, poY+y,color);
        lcdDrawPixel(poX+x, poY+y,color);
        lcdDrawPixel(poX+x, poY-y,color);
        lcdDrawPixel(poX-x, poY-y,color);
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);
    ili9341_deselect_chip();
}
void lcdFillCircle(int16_t poX, int16_t poY, int16_t r,uint16_t color)
{
    int x = -r, y = 0, err = 2-2*r, e2;
    do {

        lcdDrawVLine(poX-x, poY-y, poY-y+ 2*y, color);
        lcdDrawVLine(poX+x, poY-y, poY-y+ 2*y, color);

        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);
    ili9341_deselect_chip();
}

/* draw char or print text. */

uint16_t lcdDrawChar( uint8_t c, uint16_t char_x, uint16_t char_y, uint16_t foreground, uint16_t background)
{
    uint16_t hor,vert,offset,bpl,j,i,b;
    unsigned char* zeichen;
    unsigned char z,w;

    if ((c < 31) || (c > 127)) return 0;   // test char range

    // read font parameter from start of array
    offset = pgm_read_byte(&font[0]);                    // bytes / char
    hor = pgm_read_byte(&font[1]);                       // get hor size of font
    vert = pgm_read_byte(&font[2]);                      // get vert size of font
    bpl = pgm_read_byte(&font[3]);                       // bytes per line

   if (char_x + hor > LCD_WIDTH) {
        char_x = 0;
        char_y = char_y + vert;
        if (char_y >= LCD_HEIGHT - pgm_read_byte(&font[2])) {
            char_y = 0;
        }
    }

    ili9341_setCol( char_x, char_x+hor-1); // char box
    ili9341_setPage(char_y,char_y+vert-1);

    ili9341_send_command(0x2c);  // send pixel

    zeichen = &font[((c -32) * offset) + 4];            // start of char bitmap
    w = pgm_read_byte(&zeichen[0]);                     // width of actual char

    for (j=0; j<vert; j++) {  //  vert line
        for (i=0; i<hor; i++) {   //  horz line

            z =  pgm_read_byte(&zeichen[bpl * i + ((j & 0xF8) >> 3)+1]);

            b = 1 << (j & 0x07);
            if (( z & b ) == 0x00)
            {
               ili9341_send_byte(background >> 8);
               ili9341_send_byte(background & 0xff);
            }
            else
            {
                ili9341_send_byte(foreground >> 8);
                ili9341_send_byte(foreground & 0xff);
            }
        }
    }

    ili9341_deselect_chip();

    ili9341_setCol( 0, LCD_WIDTH);  // window max
    ili9341_setPage(0, LCD_HEIGHT);

    if ((w + 2) < hor) {                   // x offset to next char
        char_x += w + 2;
    } else char_x += hor;

    ili9341_deselect_chip();

    return char_x;
}

void lcdDrawString(const char *string,uint16_t poX, uint16_t poY, uint16_t fgcolor,uint16_t bgcolor)
{
    uint16_t w;

    while (*string!=0)
	{
       w=lcdDrawChar(*string++, poX, poY, fgcolor, bgcolor);
       if(w < LCD_WIDTH)
        {
            poX = w;                                     /* Move cursor right            */
        }
    }
}

/* end of file */
