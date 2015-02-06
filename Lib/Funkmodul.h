/****************************************************************************
 Copyright:      Kai Riedel
 Author:         Kai Riedel
 Remarks:        AVR
 Version:        29.04.2011
 Description:    RFM12 Control functions
//------------------------------------------------------------------------------*/

#ifndef _FUNKMODUL_H_
 #define _FUNKMODUL_H_

#include <avr/io.h>
#include <util/delay.h>

// define shortcuts
#define uint  unsigned int
#define uchar unsigned char

uint Din,n;
uchar Checksum;

union {
       struct {
          unsigned char data;
          unsigned char flags;
        } byte;
		unsigned int word;
   } U1input;

//Prototypes
//RFM12_SDI --> PORTx.0 | RFM12_NSEL --> PORTx.1 | RFM12_SDO --> PORTx.2 | RFM12_SCK-->PORTx.3
extern uint Spi16(uint Dout);
extern uint Wait_rfm12(uint Timeout);
extern void RFM12_Init (void);
extern uint Send_rfm12string(uchar *outstring);
extern void Enable_receiver(void);
extern void Disable_receiver(void);
extern void RestartFifoFill_receiver(void);

//Definitionen für RFM12 Modul
#define RFM12_Port_DDR			DDRA	//Port an dem das RFM12 angeschlossen wurde
#define RFM12_Port_Write		PORTA
#define RFM12_Port_Read			PINA

#define SDI     0   	//Pin für SDI
#define NSEL    1       //Pin für NSEL
#define SDO 	2 		//Pin für SDO
#define SCK 	3 		//Pin für SCK
#define VDD     4       //Pin für VDD - RFM12

#define STX 0x00
#define ETX 0x03
#define CR  0x0d
#define LF  0x0a

#define RFM12_TIMEOUT 1000

#endif //_FUNKMODUL_H_
