/****************************************************************************
 Copyright:      Kai Riedel based on LUFA Library
 Author:         Kai Riedel
 Remarks:        AVR AT90USB1287
 Version:        24.11.2013
 Description:    Temperature Data Logger with radio sensors
//------------------------------------------------------------------------------*/

/*
             LUFA Library
     Copyright (C) Dean Camera, 2010.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2010  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
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
 *  Header file for TempDataLogger.c.
 */

#ifndef _TEMP_DATALOGGER_H_
#define _TEMP_DATALOGGER_H_

	/* Includes: */
		#include <avr/io.h>
		#include <avr/wdt.h>
		#include <avr/power.h>
		#include <avr/interrupt.h>
		#include <stdio.h>
		#include <string.h>

		#include <util/delay.h>

		#include "Descriptors.h"

       	#include "Lib/SCSI.h"
       	#include "Lib/DataflashManager.h"
		#include "Lib/FATFs/ff.h"
		#include "Lib/RTC.h"
		#include "Lib/HYT321.h"
		#include "Lib/lcd.h"
		#include "Lib/Funkmodul.h"
        #include "Lib/W5500.h"
        #include "Lib/ili9341.h"
        #include "Config/AppConfig.h"

		#include <LUFA/Drivers/Board/LEDs.h>
		#include <LUFA/Drivers/Board/Temperature.h>
		#include <LUFA/Drivers/Peripheral/ADC.h>
        #include <LUFA/Drivers/Peripheral/Serial.h>
		#include <LUFA/Drivers/USB/USB.h>
		#include <LUFA/Platform/Platform.h>

	/* Macros: */
		/** LED mask for the library LED driver, to indicate that the USB interface is not ready. */
		#define LEDMASK_USB_NOTREADY      LEDS_LED1

		/** LED mask for the library LED driver, to indicate that the USB interface is enumerating. */
		#define LEDMASK_USB_ENUMERATING  (LEDS_LED2 | LEDS_LED3)

		/** LED mask for the library LED driver, to indicate that the USB interface is ready. */
		#define LEDMASK_USB_READY        (LEDS_LED2 | LEDS_LED4)

		/** LED mask for the library LED driver, to indicate that an error has occurred in the USB interface. */
		#define LEDMASK_USB_ERROR        (LEDS_LED1 | LEDS_LED3)

		/** LED mask for the library LED driver, to indicate that the USB interface is busy. */
		#define LEDMASK_USB_BUSY          LEDS_LED2

		/** Filename for the log data when written to the dataflash FAT filesystem. */
		#define LOG_FILENAME             "TEMPLOG.txt"

		/** Default log interval when the EEPROM is blank, in 500ms ticks. */
		#define DEFAULT_LOG_INTERVAL     20

		/** Indicates if the disk is write protected or not. */
		#define DISK_READ_ONLY           false

		/** Default sensor1 name when the EEPROM is blank. */
		#define DEFAULT_SENSOR1_NAME     "Sensor1"

		/** Default sensor2 name when the EEPROM is blank. */
		#define DEFAULT_SENSOR2_NAME     "Sensor2"

		/** Default sensor3 name when the EEPROM is blank. */
		#define DEFAULT_SENSOR3_NAME     "Sensor3"

		/** Default sensor3 name when the EEPROM is blank. */
		#define DEFAULT_SENSOR4_NAME     "Sensor4"

        /** Define Port for max/min display switch -> PB5*/
		#define MAX_Port_DDR			DDRB
		#define MAX_Port_Read			PINB
		#define MAX_Port_Write		    PORTB
		#define MAX_DataInput           0xDF
		#define MAXSWITCH 5

		/** Define Port for local connection or usb */
		#define LOCAL_Port_DDR          DDRC
		#define LOCAL_Port_Read         PINC
		#define LOCAL_DataInput         0x7F
		#define LOCAL 7

        /** Define Chip select for Dataflash, use only if flash is corrupted */
        /*#define WIZNET_FL1_Port_DDR DDRE
        #define WIZNET_FL1_Port_Write PORTE
        #define WIZNET_FL1_Enable 0x01
        #define WIZNET_FL1 0

        #define WIZNET_FL2_Port_DDR DDRE
        #define WIZNET_FL2_Port_Write PORTE
        #define WIZNET_FL2_Enable 0x02
        #define WIZNET_FL2 1
        */


        /** Define parameters for cosm datalogging */
        //#define API_KEY "t5oM-cXVCGkP-Rbb3m8xa8Avwc-SAKxJV0l1bVUvaEdoTT0g" // Xively API key two channels
        #define API_KEY "mZ0aRPpG70uZqprYE9oOA8hD3BBVNwX0PBe4BUpIS1shyZ2J" // Xively API key four channels
        //#define FEED_ID "116164" // Xively feed ID two channels
        #define FEED_ID "126888" // Xively feed ID four channels

        //ThingSpeak Server: 184.106.153.149, raw tcp, match string: POST
        //Xively Server: old 216.52.233.121, new 64.94.18.120, match string: PUT
        //#define writeAPIKey "HKAFWXK0MVKBI8CD" // ThingSpeak API key Office
        //#define writeAPIKey "63GNEHB17QDE685X" // ThingSpeak API key Home
        //#define FEED_ID "126888" // Xively feed ID Haus

        /** Define Power Enable for Wiznet module WIZ107SR -> PB4 */
        #define POWER_ENABLE_Port_DDR	DDRB
        #define POWER_ENABLE_Port_Write PORTB
        #define POWER_ENABLE_DataOutput 0x10
        #define POWER_ENABLE 4

        /** Define Web logging interval = (time in seconds * 2) */
        //#define WEB_LOGGING_INTERVAL 1200       // interval 10 minutes
        #define WEB_LOGGING_INTERVAL 120      // interval 60 seconds

	/* Type Defines: */
		typedef struct
		{
			TimeDate_t TimeDate;
			uint16_t LogInterval500MS;

			uint8_t Sensor1Correction;
			uint8_t Sensor2Correction;
			uint8_t Sensor3Correction;
			uint8_t Sensor4Correction;

			uint32_t FreeMemory;
			uint16_t DaysLeft;

			uint16_t WebLogInterval500MS;
		} Device_Report_t;


/*	typedef struct
		{
			TimeDate_t TimeDate;
			uint8_t    LogInterval500MS;
		} Device_Report_t;*/

	/* Function Prototypes: */
		void SetupHardware(void);
		void OpenLogFile(void);
		void CloseLogFile(void);

		void EVENT_USB_Device_Connect(void);
		void EVENT_USB_Device_Disconnect(void);
		void EVENT_USB_Device_ConfigurationChanged(void);
		void EVENT_USB_Device_ControlRequest(void);


		bool CALLBACK_MS_Device_SCSICommandReceived(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo);
		bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		                                         uint8_t* const ReportID,
                                                 const uint8_t ReportType,
		                                         void* ReportData,
		                                         uint16_t* const ReportSize);
		void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		                                          const uint8_t ReportID,
		                                          const uint8_t ReportType,
		                                          const void* ReportData,
		                                          const uint16_t ReportSize);

		void RadioReceive(void);
		void GetCorrectionValues(void);
        void DrawFrame1 (void);
        void DrawFrame2 (void);
        void DrawFrame3 (void);
        void DrawFrame4 (void);
		void Webserver(void);
		void Webcontrol(void);
		void SendToWeb (void);
        uint16_t crc16_update(uint16_t crc, uint8_t a);

#endif

