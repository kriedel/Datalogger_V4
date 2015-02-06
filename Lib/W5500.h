/****************************************************************************
 Copyright:      Kai Riedel
 Author:         Kai Riedel
 Remarks:        AVR
 Version:        21.04.2014
 Description:    Wiznet WIZ5500 functions
//------------------------------------------------------------------------------*/

#ifndef _W5500_H_
 #define _W5500_H_

#include <avr/io.h>
#include "Lib/DataflashManager.h"
#include <LUFA/Drivers/Peripheral/Serial.h>

/** Define Chip Select for Wiznet module WIZ550IO -> PB0 */
#define WIZNET_CS_Port_DDR DDRE
#define WIZNET_CS_Port_Write PORTE
#define WIZNET_CS_Enable 0x10
#define WIZNET_CS 4

//Prototypes
uint8_t writeAddress(uint8_t high, uint8_t low, uint8_t value, uint8_t block);
uint8_t readAddress(uint8_t high, uint8_t low, uint8_t block);
uint8_t sendData(uint16_t datlen, char *datatosend,uint8_t socket);
uint16_t getData(char buffer[], uint8_t maxLength, uint8_t socket);
void Init_W5500(void);
void sendToServer(uint16_t datlen, char data[], uint8_t socket);

#endif //_W5500_H_
