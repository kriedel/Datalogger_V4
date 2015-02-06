/*
             LUFA Library
     Copyright (C) Dean Camera, 2014.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2014  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the TemperatureDataLogger project. This file contains the main tasks of
 *  the project and is responsible for the initial application hardware configuration.
 */

#include "TempDataLogger.h"
#include "Lib/Arial12x12.h"
#include "Lib/Arial24x23.h"
#include "Lib/Arial26x28.h"
//#include "Lib/font_big.h"
//#include "Lib/Federation29x20.h"
//#include "Lib/Federation45x31.h"
#include "Lib/Verdana43x36.h"
#include "Lib/wiznet_logo.h"

 /** Define LCD Modul: 2 -> 2*16 Char, 4 -> 4*20 Char, 3 -> ILI9341 */
#define LCD 3


/** LUFA Mass Storage Class driver interface configuration and state information. This structure is
 *  passed to all Mass Storage Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MS_Device_t Disk_MS_Interface =
	{
		.Config =
			{
				.InterfaceNumber           = INTERFACE_ID_MassStorage,
				.DataINEndpoint            =
					{
						.Address           = MASS_STORAGE_IN_EPADDR,
						.Size              = MASS_STORAGE_IO_EPSIZE,
						.Banks             = 1,
					},
				.DataOUTEndpoint           =
					{
						.Address           = MASS_STORAGE_OUT_EPADDR,
						.Size              = MASS_STORAGE_IO_EPSIZE,
						.Banks             = 1,
					},
				.TotalLUNs                 = 1,
			},
	};

/** Buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevHIDReportBuffer[GENERIC_REPORT_SIZE];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Generic_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_HID,
				.ReportINEndpoint             =
					{
						.Address              = GENERIC_IN_EPADDR,
						.Size                 = GENERIC_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevHIDReportBuffer),
			},
	};

/** Non-volatile Logging Interval value in EEPROM, stored as a number of 500ms ticks */
uint16_t EEMEM LoggingInterval500MS_EEPROM = DEFAULT_LOG_INTERVAL;

/** Non-volatile Web Logging Interval value in EEPROM, stored as a number of 500ms ticks */
uint16_t EEMEM WebLoggingInterval500MS_EEPROM = WEB_LOGGING_INTERVAL;

/** SRAM Logging Interval value fetched from EEPROM, stored as a number of 500ms ticks */
uint16_t LoggingInterval500MS_SRAM;

/** SRAM Web Logging Interval value fetched from EEPROM, stored as a number of 500ms ticks */
uint16_t WebLoggingInterval500MS_SRAM;

/** store the day of actual logfile */
uint8_t oldday;

/** store name or location sensor 1 */
char Sensor1Name_SRAM[9];
char EEMEM Sensor1Name_EEPROM[8] = DEFAULT_SENSOR1_NAME;
int8_t Sensor1Correction_SRAM;
int8_t Sensor1PreviousValue;
int8_t Sensor1MinValue=0;
int8_t Sensor1MaxValue;
bool Sensor1Received=false;
uint8_t EEMEM Sensor1Correction_EEPROM;

/** store name or location and value of sensor 2 */
char Sensor2Name_SRAM[9];
char EEMEM Sensor2Name_EEPROM[8] = DEFAULT_SENSOR2_NAME;
int8_t Sensor2Value=0;
int8_t Sensor2PreviousValue;
int8_t Sensor2Correction_SRAM;
int8_t Sensor2MinValue;
int8_t Sensor2MaxValue;
bool Sensor2Received=false;
uint8_t EEMEM Sensor2Correction_EEPROM;

/** store name or location and value of sensor 3 */
char Sensor3Name_SRAM[9];
char EEMEM Sensor3Name_EEPROM[8] = DEFAULT_SENSOR3_NAME;
int8_t Sensor3Value=0;
int8_t Sensor3PreviousValue;
int8_t Sensor3Correction_SRAM;
int8_t Sensor3MinValue;
int8_t Sensor3MaxValue;
bool Sensor3Received=false;
uint8_t EEMEM Sensor3Correction_EEPROM;

/** store name or location and value of sensor 4 */
char Sensor4Name_SRAM[9];
char EEMEM Sensor4Name_EEPROM[8] = DEFAULT_SENSOR4_NAME;
int8_t Sensor4Value=0;
int8_t Sensor4PreviousValue;
int8_t Sensor4Correction_SRAM;
int8_t Sensor4MinValue;
int8_t Sensor4MaxValue;
bool Sensor4Received=false;
uint8_t EEMEM Sensor4Correction_EEPROM;

/** Total number of 500ms logging ticks elapsed since the last log value was recorded */
uint16_t CurrentLoggingTicks;

/** From radio modul received data */
uint8_t DataReceived;

/** radio telegram content and assigned variables */
char recbuf[10];
uint8_t bufferposition;
uint16_t rf12_crc;
uint16_t rf12_crc_calculate;
uint8_t rec_started;

/** radio timeout counters */
uint16_t Sensor2Timeout;
uint16_t Sensor3Timeout;

/** logfile successful created */
bool LogfileCreateSuccess;

/** receiver restart */
bool ReceiverRestart;

/** free memory on device */
uint32_t FreeMemory;

/** days left for data logging */
uint16_t DaysLeft_SRAM;
uint16_t EEMEM DaysLeft_EEPROM;

/** FAT Fs structure to hold the internal state of the FAT driver for the Dataflash contents. */
FATFS DiskFATState;

/** FAT Fs structure to hold a FAT file handle for the log data write destination. */
FIL TempLogFile;

/** counter for web enable */
uint16_t WebEnableCounter;
char firstdatastreamID[3] = "0";

/** measure values */
int8_t Temperature=0;
int8_t Humidity=0;
int8_t HumidityPreviousValue;

/** Flag for webclient activ */
bool Webclient=false;


/** define actual frame to display: actual values or trend graphic */
uint8_t FrameCounter;

/** store sensor values for trend graphic */
int8_t Sensor1TempValues[300];
int8_t Sensor1HumditiyValues[300];
int8_t Sensor2Values[300];
int8_t Sensor3Values[300];
int8_t Sensor4Values[300];

/** store actual time value to determine array index in trend value array */
uint32_t ActualTime;

//uint8_t rx_position;
//char RXBuffer[15];

/** ISR to manage the reception of data from the serial port */
/*ISR(USART1_RX_vect, ISR_BLOCK)
{
	RXBuffer[rx_position++] = UDR1;                       // reveive 15 characters from UART
	if (rx_position > sizeof RXBuffer-1)
    {
        rx_position = 0;
        lcd_pos(2,1);
        lcd_print_str(RXBuffer);                           // display answer from webserver

        _delay_ms(5000);
        Serial_ShutDown();                                 // disable uart
        POWER_ENABLE_Port_Write &= ~(1<<POWER_ENABLE);     // disable web module LDO
    }
}*/


/** ISR to handle the 500ms ticks for sampling and data logging */
ISR(TIMER1_COMPA_vect, ISR_BLOCK)
{
	uint8_t LEDMask = LEDs_GetLEDs();
	uint16_t ArrayIndex;

	char LineBuffer[30];

    TimeDate_t CurrentTimeDate;
    /* Signal a 500ms tick has elapsed to the RTC */
    RTC_Tick500ms();

    RTC_GetTimeDate(&CurrentTimeDate);

    ActualTime = (uint32_t)CurrentTimeDate.Hour*3600 + (uint32_t)CurrentTimeDate.Minute*60
        + CurrentTimeDate.Second;
    /* key pressed? */
    if (!(MAX_Port_Read & (1<<MAXSWITCH)))
    {
       #if (LCD==3)
       FrameCounter++;

       if (FrameCounter == 1)
       {
           DrawFrame2();
           lcdDrawString("Sensor 1, Temp.:", 120, 15, GREEN, BLACK);
       }

       if (FrameCounter == 2)
       {
           DrawFrame3();
           lcdDrawString("Sensor 1, Hum.:", 120, 15, GREEN, BLACK);
       }

       if (FrameCounter == 3)
       {
           DrawFrame4();
           lcdDrawString("Sensor 2, Temp.:", 120, 15, YELLOW, BLACK);
       }

       if (FrameCounter == 4)
       {
           DrawFrame4();
           lcdDrawString("Sensor 3, Temp.:", 120, 15, MAGENTA, BLACK);
       }

       if (FrameCounter == 5)
       {
           DrawFrame4();
           lcdDrawString("Sensor 4, Temp.:", 120, 15, CYAN, BLACK);
        }

       if (FrameCounter == 6)
       {
           FrameCounter = 0;
           DrawFrame1();
           Sensor1PreviousValue = 0;
           Sensor1Received = false;

           lcdSetFont((uint8_t*) Arial28x28);
           sprintf (LineBuffer, "%d~C", Sensor2Value);
           if (Sensor2Received) lcdDrawString(LineBuffer,20,170,YELLOW,BLACK);

           lcdSetFont((uint8_t*) Arial28x28);
           sprintf (LineBuffer, "%d~C", Sensor3Value);
           if (Sensor3Received) lcdDrawString(LineBuffer,130,170,MAGENTA,BLACK);

           lcdSetFont((uint8_t*) Arial28x28);
           sprintf (LineBuffer, "%d~C", Sensor4Value);
           if (Sensor4Received) lcdDrawString(LineBuffer,235,170,CYAN,BLACK);
       }
       #endif
       #if (LCD==2)
       lcd_init();                     // clear display because interferences could cause lcd fault
       lcd_print_str(Sensor1Name_SRAM);
       lcd_pos(2,1);
       sprintf (LineBuffer, "D:%3d I:%3d L:%3d", DaysLeft_SRAM, LoggingInterval500MS_SRAM/2,
               (LoggingInterval500MS_SRAM-CurrentLoggingTicks)/2);
       lcd_print_str(LineBuffer);

       if (Sensor1Received)
       {
        lcd_print_value (Sensor1MinValue, "#C/", '+', 1, 8, 2, 0);
        lcd_print_value (Sensor1MaxValue, "#C", '+', 1, 14, 2, 0);
        }
       #endif
       #if (LCD==4)
       lcd_init();                     // clear display because interferences could cause lcd fault
       lcd_print_str(Sensor1Name_SRAM);
       lcd_pos(4,1);
       lcd_pos(2,1);
       sprintf (LineBuffer, "D:%3d I:%3d L:%3d", DaysLeft_SRAM, LoggingInterval500MS_SRAM/2,
               (LoggingInterval500MS_SRAM-CurrentLoggingTicks)/2);
       lcd_print_str(LineBuffer);

       if (Sensor1Received)
       {
        lcd_print_value (Sensor1MinValue, "#C/", '+', 1,10, 2, 0);
        lcd_print_value (Sensor1MaxValue, "#C", '+', 1, 16, 2, 0);

       {
           lcd_pos(2,1);
           lcd_print_str(Sensor2Name_SRAM);
           lcd_pos(3,1);
           lcd_print_str(Sensor3Name_SRAM);

           if (Sensor2Received)
           {
            lcd_print_value (Sensor2MinValue, "#C/", '+',  2, 10, 2, 0);
            lcd_print_value (Sensor2MaxValue, "#C", '+',  2, 16, 2, 0);
           }
           if (Sensor3Received)
           {
            lcd_print_value (Sensor3MinValue, "#C/", '+', 3, 10, 2, 0);
            lcd_print_value (Sensor3MaxValue, "#C", '+', 3, 16, 2, 0);
            }
       }
       }
       #endif
       #if (LCD==3)
       #else
       uint8_t Timer=0;
        while (!(MAX_Port_Read & (1<<MAXSWITCH)))      // wait until key is released
        {
           _delay_ms(100);
           if (Timer++>30)                              // clear min/max store after 3 seconds
           {
              Sensor1Received = false;
              Sensor2Received = false;
              Sensor3Received = false;
              lcd_pos(LCD,1);
              lcd_print_str("Clear Min/Max Store");
           }
        }
       #endif

       #if (LCD==2)
        lcd_print_value (Sensor1PreviousValue, "#C      ", '+', 1, 10, 2, 0);
        lcd_print_value (Sensor2Value, "#C      ", '+', 2, 10, 2, 0);
      #endif
      #if (LCD==4)
        lcd_print_value (Sensor1PreviousValue, "#C      ", '+', 1, 10, 2, 0);
        lcd_print_value (Sensor2Value, "#C      ", '+', 2, 10, 2, 0);
        lcd_print_value (Sensor3Value, "#C      ", '+', 3, 10, 2, 0);
      #endif
    }


    /* Write date, time to lcd */

    #if (LCD==2)
    sprintf(LineBuffer, "%02d%02d%02d %02d:%02d:%02d ", CurrentTimeDate.Day, CurrentTimeDate.Month,
            CurrentTimeDate.Year, CurrentTimeDate.Hour, CurrentTimeDate.Minute, CurrentTimeDate.Second);
    lcd_pos(2,1);
    lcd_print_str(Sensor2Name_SRAM);
    //lcd_print_str(LineBuffer);
    #endif
    #if (LCD==4)
    sprintf(LineBuffer, "%02d.%02d.%02d,%02d:%02d:%02d  ", CurrentTimeDate.Day, CurrentTimeDate.Month,
            CurrentTimeDate.Year, CurrentTimeDate.Hour, CurrentTimeDate.Minute, CurrentTimeDate.Second);
    lcd_pos(4,1);
    lcd_print_str(LineBuffer);
    lcd_pos(2,1);
    lcd_print_str(Sensor2Name_SRAM);
    lcd_pos(3,1);
    lcd_print_str(Sensor3Name_SRAM);
    #endif
    #if (LCD==3)
    if (FrameCounter == 0)
    {
        lcdSetFont((uint8_t*) Arial24x23);
        sprintf(LineBuffer, "%02d.%02d.%02d  %02d:%02d", CurrentTimeDate.Day, CurrentTimeDate.Month,
            CurrentTimeDate.Year, CurrentTimeDate.Hour, CurrentTimeDate.Minute);
        if (CurrentTimeDate.Second == 0 || !Sensor1Received)
        lcdDrawString(LineBuffer,15,10,GRAY,BLACK);
    }

    #else
    lcd_pos(1,1);
    lcd_print_str(Sensor1Name_SRAM);
    #endif

    //Temperature = Temperature_GetTemperature()+Sensor1Correction_SRAM;  // if use of local NTC
    HYT321_GetData(&Humidity, &Temperature);
    Temperature += Sensor1Correction_SRAM;


    #if (LCD==2)
    {
        lcd_print_value (Temperature, "#C  ", '+', 1, 8, 2, 0);
        lcd_print_value (Humidity, "% ", '/', 1, 13, 2, 0);
    }
    #endif
    #if (LCD==4)
    lcd_print_value (Temperature, "#C", '+', 1, 10, 2, 0);
    lcd_print_value (Humidity, "% ", '/', 1, 15, 2, 0);
    lcd_pos (1, 20);
    if (Temperature > Sensor1PreviousValue + 1) lcd_write (0, 1);
    else if (Temperature < Sensor1PreviousValue - 1) lcd_write (1, 1);
    else lcd_print_str ("~");
    #endif
    #if (LCD==3)
      if (FrameCounter == 0)        //display actual measure values
      {

        lcdSetFont((uint8_t*) Neu42x35);

        if ((Temperature != Sensor1PreviousValue)||(Humidity != HumidityPreviousValue))
        {
            sprintf (LineBuffer, "%d~C %d%%", Temperature, Humidity);
            lcdDrawString(LineBuffer,60,50,GREEN,BLACK);
            lcdDrawVLine(318,0,120,SILVER); //redraw right frame line
            lcdDrawVLine(1,0,120,SILVER); //redraw right frame line

        }
       }

       if (FrameCounter == 1)
       {
          for (ArrayIndex=0; ArrayIndex<299; ArrayIndex++)
          {
            if ((Sensor1TempValues[ArrayIndex]!=127) & (Sensor1TempValues[ArrayIndex+1]!=127))
                lcdDrawLine (ArrayIndex+10,220-Sensor1TempValues[ArrayIndex]*4,ArrayIndex+11,220-Sensor1TempValues[ArrayIndex+1]*4,GREEN);
          }
          lcdSetFont((uint8_t*) Arial12x12);      //display actual measure values

          sprintf (LineBuffer, "%d~C ", Temperature);
          lcdDrawString(LineBuffer,255,15,GREEN,BLACK);

       }

        if (FrameCounter == 2)
       {
          for (ArrayIndex=0; ArrayIndex<299; ArrayIndex++)
          {
            if ((Sensor1HumditiyValues[ArrayIndex]!=127) & (Sensor1HumditiyValues[ArrayIndex+1]!=127))
                lcdDrawLine (ArrayIndex+10,220-Sensor1HumditiyValues[ArrayIndex]*2,ArrayIndex+11,220-Sensor1HumditiyValues[ArrayIndex+1]*2,GREEN);
           }
          lcdSetFont((uint8_t*) Arial12x12);      //display actual measure values

          sprintf (LineBuffer, "%d%% ", Humidity);
          lcdDrawString(LineBuffer,255,15,GREEN,BLACK);
       }

       if (FrameCounter == 3)
       {
          for (ArrayIndex=0; ArrayIndex<299; ArrayIndex++)
          {
            if ((Sensor2Values[ArrayIndex]!=127) & (Sensor2Values[ArrayIndex+1]!=127))
                lcdDrawLine (ArrayIndex+10,140-Sensor2Values[ArrayIndex]*2,ArrayIndex+11,140-Sensor2Values[ArrayIndex+1]*2,YELLOW);
            }

          lcdSetFont((uint8_t*) Arial12x12);      //display actual measure values

          sprintf (LineBuffer, "%d~C ", Sensor2Value);
          if (Sensor2Received) lcdDrawString(LineBuffer,255,15,YELLOW,BLACK);
       }

       if (FrameCounter == 4)
       {
          for (ArrayIndex=0; ArrayIndex<299; ArrayIndex++)
          {
            if ((Sensor3Values[ArrayIndex]!=127) & (Sensor3Values[ArrayIndex+1]!=127))
                lcdDrawLine (ArrayIndex+10,140-Sensor3Values[ArrayIndex]*2,ArrayIndex+11,140-Sensor3Values[ArrayIndex+1]*2,MAGENTA);
            }
          lcdSetFont((uint8_t*) Arial12x12);      //display actual measure values

          sprintf (LineBuffer, "%d~C ", Sensor3Value);
          if (Sensor3Received) lcdDrawString(LineBuffer,255,15,MAGENTA,BLACK);
       }

        if (FrameCounter == 5)
        {
          for (ArrayIndex=0; ArrayIndex<299; ArrayIndex++)
          {
            if ((Sensor4Values[ArrayIndex]!=127) & (Sensor4Values[ArrayIndex+1]!=127))
                lcdDrawLine (ArrayIndex+10,140-Sensor4Values[ArrayIndex]*2,ArrayIndex+11,140-Sensor4Values[ArrayIndex+1]*2,CYAN);
            }
          lcdSetFont((uint8_t*) Arial12x12);      //display actual measure values

          sprintf (LineBuffer, "%d~C ", Sensor4Value);
          if (Sensor4Received) lcdDrawString(LineBuffer,255,15,CYAN,BLACK);
        }

       if (ActualTime % 288 == 0)          //store values for trend graphic in arrays
        {
            ArrayIndex = ActualTime / 288;
            Sensor1TempValues[ArrayIndex] = Temperature;
            Sensor1HumditiyValues[ArrayIndex] = Humidity;
            if (Sensor2Received) Sensor2Values[ArrayIndex] = Sensor2Value;
            if (Sensor3Received) Sensor3Values[ArrayIndex] = Sensor3Value;
            if (Sensor4Received) Sensor4Values[ArrayIndex] = Sensor4Value;
        }

    #endif

    if (Sensor1Received)
    {
        if (Temperature < Sensor1MinValue) Sensor1MinValue = Temperature;
        if (Temperature > Sensor1MaxValue) Sensor1MaxValue = Temperature;
    }
    else
    {
       Sensor1MinValue = Temperature;
       Sensor1MaxValue = Temperature;
       Sensor1Received = true;
    }

    Sensor1PreviousValue = Temperature;
    HumidityPreviousValue = Humidity;

    // optional check sensor timeout interval
    #if (LCD==4)
    {
        Sensor2Timeout++;
        Sensor3Timeout++;

        if (Sensor2Timeout>2400)			// no Sensor2 values received in 20 minutes
        {
            Sensor2Value=0;
            lcd_pos(2,10);
            lcd_print_str("---#C      ");
        }

        if (Sensor3Timeout>2400)			// no Sensor3 values received in 20 minutes
        {
            Sensor3Value=0;
            lcd_pos(3,10);
            lcd_print_str("---#C      ");
        }

        if (((Sensor2Timeout > 600) | (Sensor3Timeout > 600)) & !ReceiverRestart) // no sensor values after 5 minutes
        {
            Disable_receiver();             // restart receiver
            RFM12_Init();			        // init RFM12 radio modul
            Enable_receiver();
            ReceiverRestart = true;         // only one receiver restartd
            Sensor2Timeout = 0;
            Sensor3Timeout = 0;
        }

    }
    #endif


    //if(Sensor2Received&Sensor3Received)         // transmit values to webserver only if sensor values received, optional
    {

        if(WebEnableCounter++ == WebLoggingInterval500MS_SRAM)
        {
           // POWER_ENABLE_Port_Write |= (1<<POWER_ENABLE);      // activate web module LDO
            writeAddress(0x00, 0x01, 0x01, 1);	 // Socket Command  Register -> OPEN
            writeAddress(0x00, 0x01, 0x04, 1);	 // Socket  Command  Register -> CONNECT
            /*temp = readAddress(0x00, 0x03, 1);   // Socket Status Register -> 0x13
            sprintf (LineBuffer, "Status: %X", temp);
            Serial_SendString(LineBuffer);
            Serial_SendString("\n");*/
            Webclient=true;
        }

        if(Webclient & (WebEnableCounter == 1))
        {
            writeAddress(0x00, 0x01, 0x08, 1);	 // Socket Command  Register -> DISCONNECT
            writeAddress(0x00, 0x01, 0x10, 1);	 // Socket Command  Register -> CLOSE
           /* temp = readAddress(0x00, 0x03, 1);   // Socket Status Register -> 0x13
            sprintf (LineBuffer, "Status: %X", temp);
            Serial_SendString(LineBuffer);
            Serial_SendString("\n");*/
            Webclient=false;
        }
       /* if(WebEnableCounter == 30)
        {
            Serial_Disable();                                 // disable uart after 15 seconds
            POWER_ENABLE_Port_Write &= ~(1<<POWER_ENABLE);     // disable web module LDO
        }*/

        if(WebEnableCounter == WebLoggingInterval500MS_SRAM+3)   // transmit values to webserver xively 0,5 seconds after activation of module
        {


            SendToWeb();
            WebEnableCounter=0;        // reset weblogging counter
        }
    }

    //getData(LineBuffer, 50, 5);
    //Serial_SendString(LineBuffer);
    //sendToServer(sizeof(LineBuffer),LineBuffer, 1);

	/* Check to see if the logging interval has expired */


	if (CurrentLoggingTicks++ < LoggingInterval500MS_SRAM)
	  return;

	/* Reset log tick counter to prepare for next logging interval */
	CurrentLoggingTicks = 0;

	LEDs_SetAllLEDs(LEDMASK_USB_BUSY);

	/* Only log when not connected to a USB host */
	if (!(LOCAL_Port_Read & (1<<LOCAL)))
	{
		uint16_t BytesWritten;
		TimeDate_t CurrentTimeDate;
        RTC_GetTimeDate(&CurrentTimeDate);

        if ((CurrentTimeDate.Day != oldday) & (LogfileCreateSuccess==true)) 	/* new day -> new logfile */
		{
			LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
			/* Close the log file */
			CloseLogFile();
			DaysLeft_SRAM--;			                         // decrement day counter
			eeprom_update_word(&DaysLeft_EEPROM, DaysLeft_SRAM); // update eeprom value

			_delay_ms(250);
			/* Mount and open new log file on the Dataflash FAT partition */
			OpenLogFile();
			_delay_ms(250);

			#if (LCD==3)
            {
                uint16_t i;

                for (i=0; i<300; i++) Sensor1TempValues[i]=127; // clear arrays for trend values
                for (i=0; i<300; i++) Sensor1HumditiyValues[i]=127; // clear arrays for trend values
                for (i=0; i<300; i++) Sensor2Values[i]=127; // clear arrays for trend values
                for (i=0; i<300; i++) Sensor3Values[i]=127; // clear arrays for trend values
                for (i=0; i<300; i++) Sensor4Values[i]=127; // clear arrays for trend values
            }
            #endif
		}
		if ((LogfileCreateSuccess==true))
		{
			BytesWritten = sprintf(LineBuffer, "%02d:%02d:%02d,%d,%d,%d,%d,%d\r\n", CurrentTimeDate.Hour,
                          CurrentTimeDate.Minute, CurrentTimeDate.Second,
			Temperature, Humidity, Sensor2Value, Sensor3Value, Sensor4Value);

			f_write(&TempLogFile, LineBuffer, BytesWritten, &BytesWritten);
			f_sync(&TempLogFile);
		}

       	LEDs_SetAllLEDs(LEDS_NO_LEDS);

       	/* write tendency sign to lcd */
       	#if (LCD==4)
       	{
          lcd_pos (1, 20);
  	   	  if (Temperature > Sensor1PreviousValue + 1) lcd_write (0, 1);
  		    else if (Temperature < Sensor1PreviousValue - 1) lcd_write (1, 1);
          else lcd_print_str ("~");
          Sensor1PreviousValue = Temperature;

          lcd_pos(2,20);
          if (Sensor2Value > Sensor2PreviousValue) lcd_write (0, 1);
          else if (Sensor2Value < Sensor2PreviousValue) lcd_write (1, 1);
          else lcd_print_str ("~");
          Sensor2PreviousValue = Sensor2Value;

          lcd_pos(3,20);
          if (Sensor3Value > Sensor3PreviousValue) lcd_write (0, 1);
          else if (Sensor3Value < Sensor3PreviousValue) lcd_write (1, 1);
          else lcd_print_str ("~");
          Sensor3PreviousValue = Sensor3Value;

          LEDs_SetAllLEDs(LEDS_NO_LEDS);
        }
        #endif
	}
	else LEDs_SetAllLEDs(LEDMask);
}

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	/* Mount and open the log file on the Dataflash FAT partition */
	OpenLogFile();

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

	for (;;)
	{
       if (!(LOCAL_Port_Read & (1<<LOCAL)))    // mode: temperature logging
		{
		    if (FrameCounter == 0)
            {
                RFM12_Port_Write &=~ (1<<NSEL); //NSEL = 0, chip select -> activ
                if (RFM12_Port_Read&(1<<SDO)) RadioReceive();
            }
		}
		else
		{
		MS_Device_USBTask(&Disk_MS_Interface);      // USB mode -> connection to pc
        HID_Device_USBTask(&Generic_HID_Interface);
        USB_USBTask();
		}
    if (!Webclient) Webcontrol();
    }
}

void Webcontrol()
{
    uint8_t temp;
    uint32_t temp2;
    char LineBuffer[16];

    cli();                                  // disable all interrupts

    temp = readAddress(0x00, 0x02, 5);      // Socket  Interrupt Register -> 0xX4 -> RECV

		if ( temp& 1<<2 )                   //Socket receive interrupt ?
        {
            getData(LineBuffer, 16, 5);
            //Serial_SendString(LineBuffer);
            // sendData(150,LineBuffer, 5);
           if ((LineBuffer[0] == 'G') & (LineBuffer[1] == 'E') & (LineBuffer[2] == 'T'))
           {
              //if (LineBuffer[8] =='E') LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
              //if (LineBuffer[8] =='A') LEDs_SetAllLEDs(LEDS_NO_LEDS);
              if (LineBuffer[6] =='L')
              {
                 LoggingInterval500MS_SRAM = atoi(&LineBuffer[8])*2;		 // convert in integer
                 if (LoggingInterval500MS_SRAM<20) LoggingInterval500MS_SRAM=20; // minimal value
                 eeprom_update_word(&LoggingInterval500MS_EEPROM, LoggingInterval500MS_SRAM);
                 f_getfree (0, (DWORD*)&FreeMemory,(FATFS**)&DiskFATState);
                 temp2 = (512 * FreeMemory) / (86400 * 26 * 2 / LoggingInterval500MS_SRAM);//26 data bytes, 86400 seconds = 1 day
                 DaysLeft_SRAM = (uint32_t)temp2;
                 eeprom_update_word(&DaysLeft_EEPROM, DaysLeft_SRAM);
              }

              if (LineBuffer[6] =='W')
              {
                 WebLoggingInterval500MS_SRAM = atoi(&LineBuffer[8])*2;		 // convert in integer
                 if (WebLoggingInterval500MS_SRAM<20) WebLoggingInterval500MS_SRAM=20; // minimal value
                 eeprom_update_word(&WebLoggingInterval500MS_EEPROM, WebLoggingInterval500MS_SRAM);
                 WebEnableCounter=0;
              }

              if (LineBuffer[6] =='A')
              {
                 Sensor1Correction_SRAM = atoi(&LineBuffer[8]);		 // convert in integer
                 eeprom_update_byte(&Sensor1Correction_EEPROM, Sensor1Correction_SRAM);
              }
              if (LineBuffer[6] =='B')
              {
                 Sensor2Correction_SRAM = atoi(&LineBuffer[8]);		 // convert in integer
                 eeprom_update_byte(&Sensor2Correction_EEPROM, Sensor2Correction_SRAM);
              }
              if (LineBuffer[6] =='C')
              {
                 Sensor3Correction_SRAM = atoi(&LineBuffer[8]);		 // convert in integer
                 eeprom_update_byte(&Sensor3Correction_EEPROM, Sensor3Correction_SRAM);
              }

              if (LineBuffer[6] =='D')
              {
                 Sensor4Correction_SRAM = atoi(&LineBuffer[8]);		 // convert in integer
                 eeprom_update_byte(&Sensor4Correction_EEPROM, Sensor4Correction_SRAM);
              }

              Webserver();
        }

           writeAddress(0x00, 0x02, 0x04, 5);	 // Clear Socket RECV Interrupt
           writeAddress(0x00, 0x01, 0x08, 5);	 // Socket Command  Register -> DISCON

           //Serial_SendString("Disconnect\n");
        }

        if ( temp& 1<<1 )                       //Socket DISCON interrupt ?
        {
            writeAddress(0x00, 0x02, 0x02, 5);	 // Clear Socket RECV Interrupt
            writeAddress(0x00, 0x01, 0x10, 5);	 // Socket Command  Register -> CLOSE
            writeAddress(0x00, 0x01, 0x01, 5);	 // Socket Command  Register -> OPEN
            writeAddress(0x00, 0x01, 0x02, 5);	 // Socket Command  Register -> LISTEN
           // Serial_SendString("Listen\n");
         }

         if ( temp& 1<<3 )                       //Socket DISCON interrupt ?
        {
           // Serial_SendString("Timeout\n");
         }
    sei();                                          // enable all interrupts
}

void Webserver()
{
    char LineBuffer[1490];
    char LineBuffer2[50];

    TimeDate_t CurrentTimeDate;
    /* Signal a 500ms tick has elapsed to the RTC */
    RTC_Tick500ms();

    RTC_GetTimeDate(&CurrentTimeDate);

    //HYT321_GetData(&Humidity, &Temperature);
    //Temperature += Sensor1Correction_SRAM;
   	strcpy(LineBuffer,"HTTP/1.1 200 OK\r\n");
    strcat(LineBuffer,"Connection: Close\r\n");
    strcat(LineBuffer,"Content-Type: text/html\r\n\r\n");
    strcat(LineBuffer,"<!DOCTYPE HTML>\r\n");
    strcat(LineBuffer,"<html><head>\r\n");
    strcat(LineBuffer,"<title>WIZ550 Datalogger</title></head>\r\n");
    strcat(LineBuffer,"<body>\r\n");
    strcat(LineBuffer,"<H3><font face='Verdana'>Datalogging Parameter and Configuration</H3><hr><br>\r\n");
    strcat(LineBuffer,"Actual System Date: ");
    sprintf(LineBuffer2, "%02d.%02d.%02d\n", CurrentTimeDate.Day, CurrentTimeDate.Month, CurrentTimeDate.Year);
    strcat(LineBuffer,LineBuffer2);
    strcat(LineBuffer,"<p>Actual System Time: ");
    sprintf(LineBuffer2, "%02d:%02d:%02d\n", CurrentTimeDate.Hour, CurrentTimeDate.Minute, CurrentTimeDate.Second);
    strcat(LineBuffer,LineBuffer2);
    strcat(LineBuffer,"<p>Actual Temperature Inside:");
    sprintf(LineBuffer2, "%3d °C\n", Temperature);
    strcat(LineBuffer,LineBuffer2);
    strcat(LineBuffer,"<p>Actual Humidity Inside: ");
    sprintf(LineBuffer2, "%3d %%\n", Humidity);
    strcat(LineBuffer,LineBuffer2);
    strcat(LineBuffer,"<p>Temperature Sensor 2: ");
    sprintf(LineBuffer2, "%3d °C\n", Sensor2Value);
    strcat(LineBuffer,LineBuffer2);
    strcat(LineBuffer,"<p>Temperature Sensor 3: ");
    sprintf(LineBuffer2, "%3d °C\n", Sensor3Value);
    strcat(LineBuffer,LineBuffer2);
    strcat(LineBuffer,"<p>Temperature Sensor 4: ");
    sprintf(LineBuffer2, "%3d °C\n", Sensor4Value);
    strcat(LineBuffer,LineBuffer2);
    sprintf (LineBuffer2, "<p>Left Days for Logging: %d\n<hr><br>", DaysLeft_SRAM);
    strcat(LineBuffer,LineBuffer2);
    strcat(LineBuffer,"<table border=0 cellpadding=0 cellspacing=4>\n");
    strcat(LineBuffer,"<tr><td width=25%>Internal Logging Interval [s]:</td>\n<form method=get><td><input type=text size=8 maxlength=3 name=L value=");
    sprintf (LineBuffer2, "%d> </td></form></tr>\n", LoggingInterval500MS_SRAM/2);
    strcat(LineBuffer,LineBuffer2);
    strcat(LineBuffer,"<tr><td width=25%>Web Logging Interval [s]:</td>\n<form method=get><td><input type=text size=8 maxlength=3 name=W value=");
    sprintf (LineBuffer2, "%d> </td></form></tr>\n", WebLoggingInterval500MS_SRAM/2);
    strcat(LineBuffer,LineBuffer2);
    strcat(LineBuffer,"<tr><td>Sensor 1 Correction [°C]:</td>\n<form method=get><td><input type=text size=8 maxlength=2 name=A value=");
    sprintf (LineBuffer2, "%d> </td></form></tr>\n", Sensor1Correction_SRAM);
    strcat(LineBuffer,LineBuffer2);
    strcat(LineBuffer,"<tr><td>Sensor 2 Correction [°C]:</td>\n<form method=get><td><input type=text size=8 maxlength=2 name=B value=");
    sprintf (LineBuffer2, "%d> </td></form></tr>\n", Sensor2Correction_SRAM);
    strcat(LineBuffer,LineBuffer2);
    strcat(LineBuffer,"<tr><td>Sensor 3 Correction [°C]:</td>\n<form method=get><td><input type=text size=8 maxlength=2 name=C value=");
    sprintf (LineBuffer2, "%d> </td></form></tr>\n", Sensor3Correction_SRAM);
    strcat(LineBuffer,LineBuffer2);
    strcat(LineBuffer,"<tr><td>Sensor 4 Correction [°C]:</td>\n<form method=get><td><input type=text size=8 maxlength=2 name=D value=");
    sprintf (LineBuffer2, "%d> </td></form></tr>\n", Sensor4Correction_SRAM);
    strcat(LineBuffer,LineBuffer2);

   // strcat(LineBuffer,"<form method=get><input type=submit name=3 value='AUS'></form>");
    strcat(LineBuffer,"</table><br><hr><br>Datalogger Values on Xively: ");
    strcat(LineBuffer, "<a href=""https://xively.com/feeds/126888"">https://xively.com/feeds/126888</a>");
    strcat(LineBuffer,"\r\n\r\n</body></html>\r\n");


    sendData(1490, LineBuffer, 5);
   // Serial_SendString(LineBuffer);
}

/** CRC16 calculate routine*/
/**Polynomial: x^16 + x^15 + x^2 + 1 (0xa001)
Initial value: 0xffff*/
uint16_t crc16_update(uint16_t crc, uint8_t a)
{
    uint8_t i;

    crc ^= a;
    for (i = 0; i < 8; ++i)
    {
        if (crc & 1)
            crc = (crc >> 1) ^ 0xa001;
        else
            crc = (crc >> 1);
    }

    return crc;
}

/** Receive telegram from radio modul*/
void RadioReceive(void)
{
    #if (LCD==3)
    char LineBuffer[10];
    #endif

    cli();                                          // disable all interrupts
   	DataReceived = (uchar) Spi16(0xb000);           // receiver FIFO read command

	if ((bufferposition>recbuf[0]+2)&rec_started)	// all bytes received
    {
		rf12_crc_calculate = recbuf[6];
		rf12_crc_calculate = (rf12_crc_calculate << 8) + recbuf[5];

		if (rf12_crc == rf12_crc_calculate) 		// Checksum OK -> display new value
		{
		   recbuf[bufferposition-2] = '\0';			// set end of string, delete checksum
			if (recbuf[1]=='A')					    // Sensor A
			{
			        Sensor2Value = atoi(&recbuf[2]);		 // convert in integer
			        if (Sensor2Value != 0)
                    {
                        Sensor2Value -= 50;                       // check if conversion is ok, correct value with offset
                        Sensor2Value += Sensor2Correction_SRAM;  // correct value if conversion ok
                    }
			        else Sensor2Value = Sensor2PreviousValue;   // wrong values received

			        if (recbuf[2] == 'X')                         // display battery warning message
			        {
			            #if (LCD==3)
                        lcdSetFont((uint8_t*) Arial28x28);
			            lcdDrawString("BAT",20,170,YELLOW,BLACK);
			            #else
			            lcd_pos(2,10);
			            lcd_print_str("BATTERY    ");
			            #endif
			        }
                    else
                    {
                        #if (LCD==3)

                        lcdSetFont((uint8_t*) Arial28x28);
                        sprintf (LineBuffer, "%d~C", Sensor2Value);
                        lcdDrawString(LineBuffer,20,170,YELLOW,BLACK);
                        #else
                        lcd_print_value (Sensor2Value, "#C     ", '+', 2, 10, 2, 0);
                        #endif
                    }

              if (Sensor2Received)
              {
                  if (Sensor2Value < Sensor2MinValue) Sensor2MinValue = Sensor2Value;
                  if (Sensor2Value > Sensor2MaxValue) Sensor2MaxValue = Sensor2Value;
              }
              else
              {
                  Sensor2MinValue = Sensor2Value;
                  Sensor2MaxValue = Sensor2Value;
                  Sensor2Received = true;
              }

             Sensor2Timeout = 0;								 // reset timeout
			}
			if (recbuf[1]=='B')					                    // Sensor B
			{
			        Sensor3Value = atoi(&recbuf[2]);		 		// convert in integer
			        if (Sensor3Value != 0)
                    {
                        Sensor3Value -= 50;                         // check if conversion is ok, correct value with offset
                        Sensor3Value += Sensor3Correction_SRAM; 	// correct value if conversion ok
                    }
                    else Sensor3Value = Sensor3PreviousValue;       // // wrong values received

                    if (recbuf[2] == 'X')                         // display battery warning message
			        {
			            #if (LCD==3)
                        lcdSetFont((uint8_t*) Arial28x28);
			            lcdDrawString("BAT",125,170,MAGENTA,BLACK);
			            #else
			            lcd_pos(3,10);
			            lcd_print_str("BATTERY    ");
			            #endif
			        }
                    else
                    {
                        #if (LCD==3)
                        lcdSetFont((uint8_t*) Arial28x28);
                        sprintf (LineBuffer, "%d~C", Sensor3Value);
                        lcdDrawString(LineBuffer,130,170,MAGENTA,BLACK);
                        #else
                        lcd_print_value (Sensor3Value, "#C     ", '+', 3, 10, 2, 0);
                        #endif
                    }
              if (Sensor3Received)
              {
                  if (Sensor3Value < Sensor3MinValue) Sensor3MinValue = Sensor3Value;
                  if (Sensor3Value > Sensor3MaxValue) Sensor3MaxValue = Sensor3Value;
              }
              else
              {
                  Sensor3MinValue = Sensor3Value;
                  Sensor3MaxValue = Sensor3Value;
                  Sensor3Received = true;
              }
              Sensor3Timeout = 0;								 // reset timeout
            }
            if (recbuf[1]=='C')					    // Sensor C
			{
			        Sensor4Value = atoi(&recbuf[2]);		 // convert in integer
			        if (Sensor4Value != 0)
                    {
                        Sensor4Value -= 50;                       // check if conversion is ok, correct value with offset
                        Sensor4Value += Sensor4Correction_SRAM;  // correct value if conversion ok
                    }
			        else Sensor4Value = Sensor4PreviousValue;   // wrong values received

			        if (recbuf[2] == 'X')                         // display battery warning message
			        {
			            #if (LCD==3)
                        lcdSetFont((uint8_t*) Arial28x28);
			            lcdDrawString("BAT",240,170,CYAN,BLACK);
			            #else
			            lcd_pos(2,10);
			            lcd_print_str("BATTERY    ");
			            #endif
			        }
                    else
                    {
                        #if (LCD==3)

                        lcdSetFont((uint8_t*) Arial28x28);
                        sprintf (LineBuffer, "%d~C", Sensor4Value);
                        lcdDrawString(LineBuffer,235,170,CYAN,BLACK);
                        #else
                        lcd_print_value (Sensor2Value, "#C     ", '+', 2, 10, 2, 0);
                        #endif
                    }

              if (Sensor4Received)
              {
                  if (Sensor4Value < Sensor4MinValue) Sensor4MinValue = Sensor4Value;
                  if (Sensor4Value > Sensor4MaxValue) Sensor4MaxValue = Sensor4Value;
              }
              else
              {
                  Sensor4MinValue = Sensor4Value;
                  Sensor4MaxValue = Sensor4Value;
                  Sensor4Received = true;
              }

             Sensor3Timeout = 0;								 // reset timeout
			}
		}
        RestartFifoFill_receiver();
		rec_started = 0;
		bufferposition = 0;
    }
    else
    {
		if (DataReceived == STX) 						// STX -> start of telegram -> save following characters
		{
			rec_started = 1;
			rf12_crc  = 0xFFFF;							// crc initial value
 			rf12_crc  = crc16_update(rf12_crc, 0xD4); 	// group ID for crc16
      		rf12_crc  = crc16_update(rf12_crc, 0x00);
       	}
		else if ((rec_started) && (bufferposition < 10))	// save telegram content, n -> next array position
		{
		recbuf[bufferposition] = DataReceived;
		if (bufferposition <= recbuf[0]) rf12_crc =  crc16_update(rf12_crc, DataReceived);
		bufferposition++;
		}
		else if (bufferposition>=10)					// over 10 characters received -> reject telegram
		{
        RestartFifoFill_receiver();
		rec_started = 0;
		bufferposition = 0;
		}
	}
	sei();                                          // enable all interrupts
}

/** Opens the log file on the Dataflash's FAT formatted partition according to the current date */
void OpenLogFile(void)
{
	char LogFileName[12];

	/* Get the current date for the filename as "X_DDMMYY.txt", X --> Sensor number */
	TimeDate_t CurrentTimeDate;
	RTC_GetTimeDate(&CurrentTimeDate);
	oldday=CurrentTimeDate.Day;
	sprintf(LogFileName, "%02d%02d%02d.txt", CurrentTimeDate.Day, CurrentTimeDate.Month, CurrentTimeDate.Year);

	f_mount(0, &DiskFATState);
	f_open(&TempLogFile, LogFileName, FA_OPEN_ALWAYS | FA_WRITE);
	f_lseek(&TempLogFile, TempLogFile.fsize);

	if (!(LOCAL_Port_Read & (1<<LOCAL)))
	{
		LogfileCreateSuccess = true;

		if (DaysLeft_SRAM > 5)
		{
			LogfileCreateSuccess = true;
		}
		else
		{
			LogfileCreateSuccess = false;
			CloseLogFile();
			#if (LCD==3)
			lcdSetFont((uint8_t*) Arial28x28);
			lcdDrawString("Memory full",10,110,RED,BLACK);
			#else
			lcd_pos(4,1);
			lcd_print_str("Memory full...      ");
			_delay_ms(2000);
			#endif
		}
	}
	else
	{
		f_getfree (0, (DWORD*)&FreeMemory,(FATFS**)&DiskFATState);
	}
}

/** Closes the open data log file on the Dataflash's FAT formatted partition */
void CloseLogFile(void)
{
	/* Sync any data waiting to be written, unmount the storage device */
	f_sync(&TempLogFile);
	f_close(&TempLogFile);
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
    char LineBuffer[30];


    #if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
    #endif

	/* Hardware Initialization */
	LEDs_Init();
	Dataflash_Init();
	USB_Init();
	TWI_Init(TWI_BIT_PRESCALE_4, TWI_BITLENGTH_FROM_FREQ(4, 50000));
	RTC_Init();

	Serial_Init(9600, 0);   // initialize uart for debugging

    Init_W5500();

    RFM12_Init();			// init RFM12 radio modul

    #if (LCD==3)
    {
        FRESULT Result;

        ili9341_init();
        lcdFillRGB(BLACK);

        f_mount(0, &DiskFATState);
        //Result=lcdDrawBMP16(0,0,"test.bmp");      // display picture from flash
        //sprintf(LineBuffer, "%d\n", Result);
        //Serial_SendString(LineBuffer);
        //_delay_ms(2000);
        //lcdFillRGB(BLACK);

        //graphic examples for lcd test
        /*lcdDrawVLine(120,50, 100, CYAN);
        lcdDrawHLine(50,100,120, YELLOW);
        lcdFillCircle(50,50,10,CYAN);
        lcdDrawRectangle(20,20,30,30,MAGENTA);
        lcdFilledRec(180,180,20,20,BLUE);


        double s;
        uint16_t i;

        for (i=0; i<320; i++) {
            s =20 * sin((long double) i / 10 );
            lcdDrawPixel(i,100 + (int)s ,RED);
        }
        lcdSetFont((uint8_t*) Arial12x12);
        lcdDrawChar(50,50,50,WHITE,BLACK);
        lcdDrawString("Hallo Kai...",5,210,WHITE,BLACK);
        lcdSetFont((uint8_t*) Arial28x28);
        lcdDrawString("Hallo Kai...",5,110,GREEN,BLACK);
        lcdSetFont((uint8_t*) Arial24x23);
        lcdDrawString("Hallo Kai...",5,150,GOLD,BLACK);*/

        lcdSetFont((uint8_t*) Neu42x35);
        lcdDrawString("Datalogger",40,80,MAGENTA,BLACK);
        lcdDrawString(" WZ1056",40,120,MAGENTA,BLACK);
        _delay_ms(2000);

       /* lcdFillRGB(WHITE);
        lcdSetFont((uint8_t*) Arial24x23);
        lcdDrawString("powered by",85,50,RED,WHITE);

        lcdDrawBMP(85,90,150,50,logo);
        _delay_ms(2000);*/
        lcdFillRGB(BLACK);
    }
    #else
    {
        lcd_init();
    }
    #endif

    HYT321_GetData(&Humidity, &Temperature);

    // send start value to webserver
    SendToWeb();

	MAX_Port_DDR &= MAX_DataInput;      //connection of max/min switch
	MAX_Port_Write |= (1<<MAXSWITCH);

    //POWER_ENABLE_Port_Write &= ~(1<<POWER_ENABLE);      //disable Wiznet module, optional
    //POWER_ENABLE_Port_DDR |= POWER_ENABLE_DataOutput;

   	//use if flash of AT90USBKey is corrupted
    /*WIZNET_FL1_Port_Write |= (1<<WIZNET_FL1);      //disable Wiznet module, cs -> 1
    WIZNET_FL1_Port_DDR |= WIZNET_FL1_Enable;
    WIZNET_FL2_Port_Write |= (1<<WIZNET_FL2);      //disable Wiznet module, cs -> 1
    WIZNET_FL2_Port_DDR |= WIZNET_FL2_Enable;

	WIZNET_FL1_Port_Write &= ~(1<<WIZNET_FL1);         //ensable Wiznet module, cs -> 0
    uint8_t r1 = SPI_TransferByte(0xC7);
    r1 = SPI_TransferByte(0x94);
    r1 = SPI_TransferByte(0x80);
    r1 = SPI_TransferByte(0x9A);
    _delay_ms(10000);
    WIZNET_FL1_Port_Write |= (1<<WIZNET_FL1);      //disable Wiznet module, cs -> 1

    WIZNET_FL2_Port_Write &= ~(1<<WIZNET_FL2);         //ensable Wiznet module, cs -> 0
    r1 = SPI_TransferByte(0xC7);
    r1 = SPI_TransferByte(0x94);
    r1 = SPI_TransferByte(0x80);
    r1 = SPI_TransferByte(0x9A);
    _delay_ms(10000);
    WIZNET_FL2_Port_Write |= (1<<WIZNET_FL2);      //disable Wiznet module, cs -> 1
    */
	DDRE |= 0x80;                                   //disable UVCON - Q1 at AT90USBKey Board
	PORTE &= 0x7F;

   	/* Check if the Dataflash is working, abort if not */
	if (!(DataflashManager_CheckDataflashOperation()))
	{
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		for(;;);
	}

	GetCorrectionValues();			//get correction values from eeprom

    /* Clear Dataflash sector protections, if enabled */
	DataflashManager_ResetDataflashProtections();


	/* Only if not connected to pc usb interface */
	if (!(LOCAL_Port_Read & (1<<LOCAL)))
	{
		#if (LCD==2)
        {
            lcd_print_str("Datalogger V1.3");
            lcd_pos (2,1);
            lcd_print_str("Kai Riedel, 2014");
            _delay_ms(1000);
            lcd_clear();
            lcd_print_str(Sensor1Name_SRAM);
        }
        #endif
        #if (LCD==4)
        {
            lcd_print_str("Datalogger V1.3");
            lcd_pos (2,1);
            lcd_print_str("Kai Riedel, 2014");

            lcd_pos(3,1);
            sprintf (LineBuffer, "Days: %d", DaysLeft_SRAM);
            lcd_print_str(LineBuffer);

            lcd_pos(4,1);
            sprintf (LineBuffer, "Interval: %ds", LoggingInterval500MS_SRAM/2);
            lcd_print_str(LineBuffer);

            _delay_ms(1000);
            lcd_clear();
            lcd_print_str(Sensor1Name_SRAM);
        }
        #endif
        #if (LCD==3)
        {
            lcdSetFont((uint8_t*) Arial28x28);
            sprintf (LineBuffer, "Days: %d", DaysLeft_SRAM);
            lcdDrawString(LineBuffer,5,50,MAGENTA,BLACK);

            sprintf (LineBuffer, "Interval: %ds", LoggingInterval500MS_SRAM/2);
            lcdDrawString(LineBuffer,5,100,MAGENTA,BLACK);

            sprintf (LineBuffer, "Webinterval: %ds", WebLoggingInterval500MS_SRAM/2);
            lcdDrawString(LineBuffer,5,150,MAGENTA,BLACK);

            _delay_ms(2000);
        }
        #endif

		//Temperature = Temperature_GetTemperature()+Sensor1Correction_SRAM; // if use of local NTC

		HYT321_GetData(&Humidity, &Temperature);

        #if (LCD==2)
        {
            lcd_print_value (Temperature+Sensor1Correction_SRAM, "#C", '+', 1, 8, 2, 0);
            lcd_print_value (Humidity, "% ", '/', 1, 13, 2, 0);
            lcd_pos(2,1);
            lcd_print_str(Sensor2Name_SRAM);
            lcd_pos(2,10);
            lcd_print_str("---#C  ");
        }
        #endif
        #if (LCD==4)
        {
            lcd_print_value (Temperature+Sensor1Correction_SRAM, "#C", '+', 1, 10, 2, 0);
            lcd_print_value (Humidity, "% ", '/', 1, 15, 2, 0);

            lcd_pos(2,1);
            lcd_print_str(Sensor2Name_SRAM);
            lcd_pos(2,10);
            lcd_print_str("---#C  ");

            lcd_pos(3,1);
            lcd_print_str(Sensor3Name_SRAM);
            lcd_pos(3,10);
            lcd_print_str("---#C  ");
        }
        #endif
        #if (LCD==3)
        {
            uint16_t i;

            DrawFrame1 ();

            lcdSetFont((uint8_t*) Neu42x35);
            sprintf (LineBuffer, "%d~C %d%%", Temperature+Sensor1Correction_SRAM,Humidity);
            lcdDrawString(LineBuffer,60,50,GREEN,BLACK);
            lcdDrawVLine(318,0,120,SILVER); //redraw right frame line
            lcdDrawVLine(1,0,120,SILVER); //redraw right frame line
            for (i=0; i<300; i++) Sensor1TempValues[i]=127; // clear arrays for trend values
            for (i=0; i<300; i++) Sensor1HumditiyValues[i]=127; // clear arrays for trend values
            for (i=0; i<300; i++) Sensor2Values[i]=127; // clear arrays for trend values
            for (i=0; i<300; i++) Sensor3Values[i]=127; // clear arrays for trend values
            for (i=0; i<300; i++) Sensor4Values[i]=127; // clear arrays for trend values

        }
        #endif

		Enable_receiver();		// enable radio receiver

        //Serial_Init(9600, 0);   // initialize uart for debugging

      	/* 500ms logging interval timer configuration */
        OCR1A   = (((F_CPU / 256) / 2) - 1);
        TCCR1B  = (1 << WGM12) | (1 << CS12);
        TIMSK1  = (1 << OCIE1A); // enable Timer 1 interrupt
		}
}


void DrawFrame1 (void)
{
    lcdFillRGB(BLACK);

    lcdDrawRectangle(1,1,318,239,SILVER);
    lcdDrawHLine(1,120,318,SILVER);
    lcdDrawVLine(106,120,239,SILVER);
    lcdDrawVLine(212,120,239,SILVER);

    lcdSetFont((uint8_t*) Arial24x23);
    lcdDrawChar('1',10, 140, SILVER, BLACK);
    lcdDrawChar('2',115, 140, SILVER, BLACK);
    lcdDrawChar('3',220, 140, SILVER, BLACK);
    lcdDrawString("IN",5,50,SILVER,BLACK);

    lcdSetFont((uint8_t*) Arial12x12);
    lcdDrawString(Sensor2Name_SRAM,20,215,YELLOW,BLACK);
    lcdDrawString(Sensor3Name_SRAM,125,215,MAGENTA,BLACK);
    lcdDrawString(Sensor4Name_SRAM,235,215,CYAN,BLACK);
}

void DrawFrame2 (void)
{
    lcdFillRGB(BLACK);

    lcdDrawRectangle(10,20,300,200,YELLOW);
    lcdDrawHLine(10,60,310,SILVER);
    lcdDrawHLine(10,100,310,SILVER);
    lcdDrawHLine(10,140,310,SILVER);
    lcdDrawHLine(10,180,310,SILVER);
    lcdDrawVLine(60,20,220,SILVER);
    lcdDrawVLine(110,20,220,SILVER);
    lcdDrawVLine(160,20,220,SILVER);
    lcdDrawVLine(210,20,220,SILVER);
    lcdDrawVLine(260,20,220,SILVER);

    lcdSetFont((uint8_t*) Arial12x12);

    lcdDrawString("0",5, 220, WHITE, BLACK);
    lcdDrawString("10",0, 175, WHITE, BLACK);
    lcdDrawString("20",0, 135, WHITE, BLACK);
    lcdDrawString("30",0, 95, WHITE, BLACK);
    lcdDrawString("40",0, 55, WHITE, BLACK);
    lcdDrawString("50 Grad",0, 15, WHITE, BLACK);

    lcdDrawString("4",55, 220, WHITE, BLACK);
    lcdDrawString("8",105, 220, WHITE, BLACK);
    lcdDrawString("12",155, 220, WHITE, BLACK);
    lcdDrawString("16",205, 220, WHITE, BLACK);
    lcdDrawString("20",255, 220, WHITE, BLACK);
    lcdDrawString("24",290, 220, WHITE, BLACK);
}

void DrawFrame3 (void)
{
    lcdFillRGB(BLACK);

    lcdDrawRectangle(10,20,300,200,YELLOW);
    lcdDrawHLine(10,60,310,SILVER);
    lcdDrawHLine(10,100,310,SILVER);
    lcdDrawHLine(10,140,310,SILVER);
    lcdDrawHLine(10,180,310,SILVER);
    lcdDrawVLine(60,20,220,SILVER);
    lcdDrawVLine(110,20,220,SILVER);
    lcdDrawVLine(160,20,220,SILVER);
    lcdDrawVLine(210,20,220,SILVER);
    lcdDrawVLine(260,20,220,SILVER);

    lcdSetFont((uint8_t*) Arial12x12);

    lcdDrawString("0",5, 220, WHITE, BLACK);
    lcdDrawString("20",0, 175, WHITE, BLACK);
    lcdDrawString("40",0, 135, WHITE, BLACK);
    lcdDrawString("60",0, 95, WHITE, BLACK);
    lcdDrawString("80",0, 55, WHITE, BLACK);
    lcdDrawString("100 %",0, 15, WHITE, BLACK);

    lcdDrawString("4",55, 220, WHITE, BLACK);
    lcdDrawString("8",105, 220, WHITE, BLACK);
    lcdDrawString("12",155, 220, WHITE, BLACK);
    lcdDrawString("16",205, 220, WHITE, BLACK);
    lcdDrawString("20",255, 220, WHITE, BLACK);
    lcdDrawString("24",290, 220, WHITE, BLACK);
}

void DrawFrame4 (void)
{
    lcdFillRGB(BLACK);

    lcdDrawRectangle(10,20,300,200,YELLOW);
    lcdDrawHLine(10,60,310,SILVER);
    lcdDrawHLine(10,100,310,SILVER);
    lcdDrawHLine(10,140,310,SILVER);
    lcdDrawHLine(10,180,310,SILVER);
    lcdDrawVLine(60,20,220,SILVER);
    lcdDrawVLine(110,20,220,SILVER);
    lcdDrawVLine(160,20,220,SILVER);
    lcdDrawVLine(210,20,220,SILVER);
    lcdDrawVLine(260,20,220,SILVER);

    lcdSetFont((uint8_t*) Arial12x12);

    lcdDrawString("-40",5, 220, WHITE, BLACK);
    lcdDrawString("-20",0, 175, WHITE, BLACK);
    lcdDrawString("0",0, 135, WHITE, BLACK);
    lcdDrawString("20",0, 95, WHITE, BLACK);
    lcdDrawString("40",0, 55, WHITE, BLACK);
    lcdDrawString("60 Grad",0, 15, WHITE, BLACK);

    lcdDrawString("4",55, 220, WHITE, BLACK);
    lcdDrawString("8",105, 220, WHITE, BLACK);
    lcdDrawString("12",155, 220, WHITE, BLACK);
    lcdDrawString("16",205, 220, WHITE, BLACK);
    lcdDrawString("20",255, 220, WHITE, BLACK);
    lcdDrawString("24",290, 220, WHITE, BLACK);
}

void SendToWeb (void)
{
    char LineBuffer[200];
	char LineBuffer2[12];

    strcpy(LineBuffer,"PUT /v2/feeds/");
    strcat(LineBuffer,FEED_ID);
    strcat(LineBuffer, ".csv HTTP/1.1\r\n");
    strcat(LineBuffer, "Host: api.xively.com\r\n");
    strcat(LineBuffer, "X-ApiKey: ");
    strcat(LineBuffer, API_KEY);
    strcat(LineBuffer, "\r\n");
    strcat(LineBuffer, "Content-Length: 30\r\n");
    strcat(LineBuffer, "Content-Type: text/csv\r\n");
    strcat(LineBuffer, "\r\n");

    strcat(LineBuffer, "0,");
    sprintf(LineBuffer2, "%3d\n", Temperature);
    strcat(LineBuffer, LineBuffer2);

    strcat(LineBuffer, "1,");
    sprintf(LineBuffer2, "%3d\n", Humidity);
    strcat(LineBuffer, LineBuffer2);

    strcat(LineBuffer, "2,");
    sprintf(LineBuffer2, "%3d\n", Sensor2Value);
    strcat(LineBuffer, LineBuffer2);

    strcat(LineBuffer, "3,");
    sprintf(LineBuffer2, "%3d\n", Sensor3Value);
    strcat(LineBuffer, LineBuffer2);

    strcat(LineBuffer, "4,");
    sprintf(LineBuffer2, "%3d\r\n", Sensor4Value);
    strcat(LineBuffer, LineBuffer2);

    sendData(198, LineBuffer, 1);

}
void GetCorrectionValues (void)
{
    /*	Fetch sensor names from EEPROM if not to USB connected*/
	/*eeprom_read_block(&Sensor1Name_SRAM,&Sensor1Name_EEPROM,8);
	if (Sensor1Name_SRAM[0] == 0xFF) sprintf(Sensor1Name_SRAM, DEFAULT_SENSOR1_NAME);

	eeprom_read_block(&Sensor2Name_SRAM,&Sensor2Name_EEPROM,8);
	if (Sensor2Name_SRAM[0] == 0xFF) sprintf(Sensor2Name_SRAM, DEFAULT_SENSOR2_NAME);

	eeprom_read_block(&Sensor3Name_SRAM,&Sensor3Name_EEPROM,8);
	if (Sensor3Name_SRAM[0] == 0xFF) sprintf(Sensor3Name_SRAM, DEFAULT_SENSOR3_NAME);*/
    /* Fetch logging interval from EEPROM */
	LoggingInterval500MS_SRAM = eeprom_read_word(&LoggingInterval500MS_EEPROM);

	/* Check if the logging interval is invalid (0xFF) indicating that the EEPROM is blank */
	if (LoggingInterval500MS_SRAM == 0xFF) LoggingInterval500MS_SRAM = DEFAULT_LOG_INTERVAL;

	/* Fetch web logging interval from EEPROM */
	WebLoggingInterval500MS_SRAM = eeprom_read_word(&WebLoggingInterval500MS_EEPROM);

	/* Check if the web logging interval is invalid (0xFF) indicating that the EEPROM is blank */
	if (WebLoggingInterval500MS_SRAM == 0xFF) WebLoggingInterval500MS_SRAM = WEB_LOGGING_INTERVAL;

	//LoggingInterval500MS_SRAM = DEFAULT_LOG_INTERVAL;

    sprintf(Sensor1Name_SRAM, DEFAULT_SENSOR1_NAME);
    sprintf(Sensor2Name_SRAM, DEFAULT_SENSOR2_NAME);
    sprintf(Sensor3Name_SRAM, DEFAULT_SENSOR3_NAME);
    sprintf(Sensor4Name_SRAM, DEFAULT_SENSOR4_NAME);


	/* Fetch correction values from EEPROM */
	Sensor1Correction_SRAM = eeprom_read_byte(&Sensor1Correction_EEPROM);
	Sensor2Correction_SRAM = eeprom_read_byte(&Sensor2Correction_EEPROM);
	Sensor3Correction_SRAM = eeprom_read_byte(&Sensor3Correction_EEPROM);
	Sensor4Correction_SRAM = eeprom_read_byte(&Sensor4Correction_EEPROM);

	/* Check if the correction values are invalid (0xFF) indicating that the EEPROM is blank */
	if (Sensor1Correction_SRAM == 0xFF) Sensor1Correction_SRAM = 0;
	if (Sensor2Correction_SRAM == 0xFF) Sensor2Correction_SRAM = 0;
	if (Sensor3Correction_SRAM == 0xFF) Sensor3Correction_SRAM = 0;
	if (Sensor4Correction_SRAM == 0xFF) Sensor4Correction_SRAM = 0;

	/* Fetch days left from EEPROM */
	DaysLeft_SRAM = eeprom_read_word(&DaysLeft_EEPROM);
	if (DaysLeft_SRAM == 0xFFFF) DaysLeft_SRAM = 50;
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
    TIMSK1  &= ~(1 << OCIE1A); // disable Timer 1 interrupt
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);

	/* Close the log file so that the host has exclusive filesystem access */
	CloseLogFile();

    #if(LCD==3)
    lcdFillRGB(BLACK);
    lcdSetFont((uint8_t*) Arial28x28);
	lcdDrawString("Connected to PC",5,50,GREEN,BLACK);
	#else
	lcd_clear();
	lcd_print_str("Connected to PC");
    #endif

}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);

	//SetupHardware();

	/* Mount and open the log file on the Dataflash FAT partition */
	OpenLogFile();

	//GlobalInterruptEnable();

}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Generic_HID_Interface);
	ConfigSuccess &= MS_Device_ConfigureEndpoints(&Disk_MS_Interface);

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	MS_Device_ProcessControlRequest(&Disk_MS_Interface);
	HID_Device_ProcessControlRequest(&Generic_HID_Interface);
}

/** Mass Storage class driver callback function the reception of SCSI commands from the host, which must be processed.
 *
 *  \param[in] MSInterfaceInfo  Pointer to the Mass Storage class interface configuration structure being referenced
 */
bool CALLBACK_MS_Device_SCSICommandReceived(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo)
{
	bool CommandSuccess;

	LEDs_SetAllLEDs(LEDMASK_USB_BUSY);
	CommandSuccess = SCSI_DecodeSCSICommand(MSInterfaceInfo);
	LEDs_SetAllLEDs(LEDMASK_USB_READY);

	return CommandSuccess;
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	Device_Report_t* ReportParams = (Device_Report_t*)ReportData;

    RTC_GetTimeDate(&ReportParams->TimeDate);

	ReportParams->LogInterval500MS = LoggingInterval500MS_SRAM;
	ReportParams->WebLogInterval500MS = WebLoggingInterval500MS_SRAM;

	ReportParams->Sensor1Correction = Sensor1Correction_SRAM;
	ReportParams->Sensor2Correction = Sensor2Correction_SRAM;
	ReportParams->Sensor3Correction = Sensor3Correction_SRAM;
	ReportParams->Sensor4Correction = Sensor4Correction_SRAM;

   	ReportParams->FreeMemory = FreeMemory;

    *ReportSize = sizeof(Device_Report_t);
	return true;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	Device_Report_t* ReportParams = (Device_Report_t*)ReportData;

	RTC_SetTimeDate(&ReportParams->TimeDate);

	/* If the logging interval has changed from its current value, write it to EEPROM */
	if (LoggingInterval500MS_SRAM != ReportParams->LogInterval500MS)
	{
		LoggingInterval500MS_SRAM = ReportParams->LogInterval500MS;
		eeprom_update_word(&LoggingInterval500MS_EEPROM, LoggingInterval500MS_SRAM);
	}

	if (WebLoggingInterval500MS_SRAM != ReportParams->WebLogInterval500MS)
	{
		WebLoggingInterval500MS_SRAM = ReportParams->WebLogInterval500MS;
		eeprom_update_word(&WebLoggingInterval500MS_EEPROM, WebLoggingInterval500MS_SRAM);
	}

	if (Sensor1Correction_SRAM != ReportParams->Sensor1Correction)
	{
		Sensor1Correction_SRAM = ReportParams->Sensor1Correction;
		eeprom_update_byte(&Sensor1Correction_EEPROM, Sensor1Correction_SRAM);
	}

	if (Sensor2Correction_SRAM != ReportParams->Sensor2Correction)
	{
		Sensor2Correction_SRAM = ReportParams->Sensor2Correction;
		eeprom_update_byte(&Sensor2Correction_EEPROM, Sensor2Correction_SRAM);
	}

	if (Sensor3Correction_SRAM != ReportParams->Sensor3Correction)
	{
		Sensor3Correction_SRAM = ReportParams->Sensor3Correction;
		eeprom_update_byte(&Sensor3Correction_EEPROM, Sensor3Correction_SRAM);
	}

	if (Sensor4Correction_SRAM != ReportParams->Sensor4Correction)
	{
		Sensor4Correction_SRAM = ReportParams->Sensor4Correction;
		eeprom_update_byte(&Sensor4Correction_EEPROM, Sensor4Correction_SRAM);
	}

	if (DaysLeft_SRAM  != ReportParams->DaysLeft)
	{
		DaysLeft_SRAM  = ReportParams->DaysLeft;
		eeprom_update_word(&DaysLeft_EEPROM, DaysLeft_SRAM);
	}
}

