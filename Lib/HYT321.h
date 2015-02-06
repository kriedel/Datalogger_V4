/****************************************************************************
 Copyright:      Kai Riedel based on LUFA Library
 Author:         Kai Riedel
 Remarks:        AVR AT90USB1287
 Version:        15.05.2011
 Description:    Temperature Data Logger with radio sensors
//------------------------------------------------------------------------------*/
#ifndef _HYT321_H_
#define _HYT321_H_

	/* Includes: */
		#include <avr/io.h>

		#include <LUFA/Drivers/Peripheral/TWI.h>

	/* Macros: */
		#define HYT321_ADDRESS_READ   0b01010001
		#define HYT321_ADDRESS_WRITE  0b01010000

    /* Type Defines: */
        typedef struct
		{
			union
			{
				struct
				{
					unsigned int HumidityMSB     : 6;  // 6 Bits [0..5]
					unsigned int Status          : 2;  // 2 Bits [6..7]
                } Fields;

				uint8_t IntVal;
			} Byte1;

            union
			{
				struct
				{
				    unsigned int NotUsed         : 2; // 2 Bits [0..1]
					unsigned int TemperatureLSB  : 6; // 6 Bits [2..7]
                } Fields;

				uint8_t IntVal;
			} Byte2;

            uint8_t HumidityLSB;
            uint8_t TemperatureMSB ;

		} HYT321_DataRegs_t;

	/* Function Prototypes: */
		void HYT321_GetData(int8_t* const Humidity,
		                    int8_t* const Temperature);

#endif

