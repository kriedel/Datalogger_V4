/****************************************************************************
 Copyright:      Kai Riedel
 Author:         Kai Riedel
 Remarks:        AVR
 Version:        21.04.2014
 Description:    Wiznet WIZ5500 functions
//------------------------------------------------------------------------------*/

#include "W5500.h"

// Network Information
uint8_t  mac[6] = {0x00, 0x08, 0xDC, 0x1D, 0x26, 0xDB};    // MAC Address
//uint8_t  ip[4] = {192, 168, 1, 105};                       // IP Address
uint8_t  ip[4] = {172, 21, 16, 203};                       // IP Address
//uint8_t  gw[4] = {192, 168, 1, 1};                         // Gateway Address
uint8_t  gw[4] = {172, 21, 1, 11};                         // Gateway Address
uint8_t  subnet[4] = {255, 255, 255, 0};                   // SubnetMask Address
//uint8_t  serverip[4] = {172, 21, 16, 183};               // Destination IP Address for PC as HTTP Client
uint8_t  serverip[4] = {64, 94, 18, 120};               // Destination IP Address for HTTP Client Xively
//uint8_t  serverip[4] = {192, 168, 1, 83};               // Destination IP Address for PC as HTTP Client
uint8_t destport[2] = {0, 80};                             // Destination Port number for HTTP Client
uint8_t  clientport[2] = {0, 80};                     // Client Port 80


uint8_t writeAddress(uint8_t high, uint8_t low, uint8_t value, uint8_t block)
{
	uint8_t temp;
	uint8_t r1, r2, r3, r4;

	WIZNET_CS_Port_Write &= ~(1<<WIZNET_CS);        // ensable Wiznet module, cs -> 0
	r1 = SPI_TransferByte(high);                    // address phase
	r2 = SPI_TransferByte(low);
	r3 = SPI_TransferByte(0x04|(block<<3));         // control phase
	r4 = SPI_TransferByte(value);                   // data phase
	WIZNET_CS_Port_Write |= (1<<WIZNET_CS);         // disable Wiznet module, cs -> 1
	if(r1 == 0 && r2==1 && r3==2 && r4 == 3) temp = 1;
	else temp = 0;

	return temp;

}

uint8_t readAddress(uint8_t high, uint8_t low, uint8_t block)
{
	uint8_t temp;
	uint8_t r1, r2, r3;

	WIZNET_CS_Port_Write &= ~(1<<WIZNET_CS);         // ensable Wiznet module, cs -> 0
	r1 = SPI_TransferByte(high);                     // address phase
	r2 = SPI_TransferByte(low);
	r3 = SPI_TransferByte(0x00|(block<<3));          // control phase
	temp = SPI_TransferByte(0xFF);                   // data phase
	WIZNET_CS_Port_Write |= (1<<WIZNET_CS);          // disable Wiznet module, cs -> 1
	return temp;

}

//Send data to socket
uint8_t sendData(uint16_t datlen, char datatosend[],uint8_t socket)
{

	uint8_t q1, q2;
	uint16_t sentcount = 0;
	printf("%s\n", datatosend);

	q1 = readAddress(0x00, 0x24, socket);
	q2 = readAddress(0x00, 0x25, socket);

	uint16_t wrAdd = 0;
	uint16_t offset;
	uint8_t wrAddHigh = q1;
	uint8_t wrAddLow = 	q2;

	offset = (((q1<<8) | (q2)));

	while(sentcount < datlen)
	{
		wrAdd = (offset & 0x07ff);
		wrAddHigh = ((wrAdd & 0xff00)>>8);
		wrAddLow = ((wrAdd & 0x00ff));
		writeAddress(wrAddHigh, wrAddLow, datatosend[sentcount],socket+1); // Socket TX Buffer
		sentcount++;
		offset++;
	}

	writeAddress(0x00, 0x24, (offset & 0xff00)>>8, socket);
	writeAddress(0x00, 0x25, (offset & 0x00ff), socket);

	//Send
	q1 = writeAddress(0x00, 0x01, 0x20,socket);	 // Socket Command  Register -> SEND

    return q1;
}

//Get data from socket RX Buffer
uint16_t getData(char buffer[], uint8_t maxLength, uint8_t socket)
{

    //char LineBuffer[16];

	//get data
	uint16_t i = 0;
	uint8_t inter0;



	inter0 = readAddress(0x00, 0x02, socket);

	if(inter0 & 1<<2)                           //Socket interrupt ?
	{

		//sprintf(LineBuffer,"Data Received\n");
		//Serial_TxString(LineBuffer);
        //Serial_TxString("\n");

		uint8_t dataSize1;
		uint8_t dataSize2;

		dataSize1 = readAddress(0x00, 0x26, socket);
		dataSize2 = readAddress(0x00, 0x27, socket);


		//sprintf(LineBuffer, "%d %d\n", dataSize1, dataSize2);
       //Serial_TxString(LineBuffer);
        //Serial_TxString("\n");

		uint8_t pointer1;
		uint8_t pointer2;

		pointer1 = readAddress(0x00, 0x28, socket);
		pointer2 = readAddress(0x00, 0x29, socket);

		uint16_t length = ((dataSize1<<8) | dataSize2);

		while(i < length && i < maxLength)
		{

			if((pointer1%8) + ((i+pointer2)>>8) < 0x08)
			{
				buffer[i] = readAddress((pointer1%8) + ((i+pointer2)>>8), i+pointer2, socket+2);
			}
			else
			{
				buffer[i] = readAddress((pointer1%8) + ((i+pointer2)>>8) - 0x08, i+pointer2, socket+2);
			}

			i++;
		}
		if (maxLength < length)
		{
			writeAddress(0x00, 0x28, (pointer1 + ((i+pointer2)>>8)), socket);
			writeAddress(0x00, 0x29, (uint8_t) i+pointer2, socket);
		}
		else
		{
			writeAddress(0x00, 0x28, (dataSize1+pointer1 + ((dataSize2+pointer2)>>8)), socket);
			writeAddress(0x00, 0x29, (uint8_t) dataSize2+pointer2, socket);
		}

		//Recv command
		writeAddress(0x00, 0x01, 0x40, socket); // Socket Command  Register -> RECV
		//writeAddress(0x00, 0x01, 0x10, socket);	 // Socket Command  Register -> CLOSE
		//writeAddress(0x00, 0x01, 0x01, 5);	 // Socket Command  Register -> OPEN
        //writeAddress(0x00, 0x01, 0x02, 5);	 // Socket Command  Register -> LISTEN

	}
	buffer[i] = '\0';

	return i;

}

void Init_W5500(void)
{
    //uint8_t temp1;
   // uint8_t temp2, temp3, temp4, temp5, temp6;
    //char LineBuffer[16];

    WIZNET_CS_Port_Write |= (1<<WIZNET_CS);      //disable Wiznet module, cs -> 1
    WIZNET_CS_Port_DDR |= WIZNET_CS_Enable;

  	writeAddress(0x00, 0x01, gw[0],0);          //gateway address
	writeAddress(0x00, 0x02, gw[1],0);
	writeAddress(0x00, 0x03, gw[2],0);
	writeAddress(0x00, 0x04, gw[3],0);
    writeAddress(0x00, 0x05, subnet[0],0);      //subnet address
	writeAddress(0x00, 0x06, subnet[1],0);
	writeAddress(0x00, 0x07, subnet[2],0);
	writeAddress(0x00, 0x08, subnet[3],0);
	writeAddress(0x00, 0x0F, ip[0],0);          //ip address
	writeAddress(0x00, 0x10, ip[1],0);
	writeAddress(0x00, 0x11, ip[2],0);
	writeAddress(0x00, 0x12, ip[3],0);
	writeAddress(0x00, 0x09, mac[0],0);
	writeAddress(0x00, 0x0A, mac[1],0);
	writeAddress(0x00, 0x0B, mac[2],0);
	writeAddress(0x00, 0x0C, mac[3],0);
	writeAddress(0x00, 0x0D, mac[4],0);
	writeAddress(0x00, 0x0E, mac[5],0);
	//Socket 0 Register
	writeAddress(0x00, 0x0C, serverip[0],1);    //server ip address
	writeAddress(0x00, 0x0D, serverip[1],1);
	writeAddress(0x00, 0x0E, serverip[2],1);
	writeAddress(0x00, 0x0F, serverip[3],1);
	writeAddress(0x00, 0x10, destport[0],1);    //server port
	writeAddress(0x00, 0x11, destport[1],1);
	writeAddress(0x00, 0x04, clientport[0],1);
	writeAddress(0x00, 0x05, clientport[1],1);  //client port socket 0

	writeAddress(0x00, 0x04, clientport[0],5);
	writeAddress(0x00, 0x05, clientport[1],5);  //client port socket 1

	/*temp1 = readAddress(0x00, 0x0F,0); // ip address
	temp2 = readAddress(0x00, 0x10,0);
	temp3 = readAddress(0x00, 0x11,0);
	temp4 = readAddress(0x00, 0x12,0);

  	sprintf (LineBuffer, "IP: %d.%d.%d.%d", temp1,temp2, temp3, temp4);
    Serial_SendString(LineBuffer);
    Serial_SendString("\n");

    temp1 = readAddress(0x00, 0x01,0); // gateway address
	temp2 = readAddress(0x00, 0x02,0);
	temp3 = readAddress(0x00, 0x03,0);
	temp4 = readAddress(0x00, 0x04,0);

    sprintf (LineBuffer, "Gateway: %d.%d.%d.%d", temp1,temp2, temp3, temp4);
    Serial_SendString(LineBuffer);
    Serial_SendString("\n");

    temp1 = readAddress(0x00, 0x09,0); // mac address
	temp2 = readAddress(0x00, 0x0A,0);
	temp3 = readAddress(0x00, 0x0B,0);
	temp4 = readAddress(0x00, 0x0C,0);
	temp5 = readAddress(0x00, 0x0D,0);
	temp6 = readAddress(0x00, 0x0E,0);

    sprintf (LineBuffer, "Mac: %X.%X.%X.%X.%X.%X", temp1,temp2, temp3, temp4, temp5, temp6);
    Serial_SendString(LineBuffer);
    Serial_SendString("\n");*/

    writeAddress(0x00, 0x00, 0x01,1);	 // Socket 0 Mode Register
   	writeAddress(0x00, 0x00, 0x01,5);	 // Socket 1 Mode Register
    writeAddress(0x00, 0x01, 0x01, 5);	 // Socket Command  Register -> OPEN
    writeAddress(0x00, 0x01, 0x02, 5);	 // Socket Command  Register -> LISTEN

    //_delay_ms(100);
    //Serial_Disable();

    //writeAddress(0x00, 0x2E, 0xF6,0);	 // Wiznet PowerDown Mode
    //writeAddress(0x00, 0x2E, 0x76,0);	 // Wiznet Reset

}


void sendToServer(uint16_t datlen, char data[], uint8_t socket)
{

    uint8_t temp1;
    //char LineBuffer[16];

    //Socket 0 --> Block Select Bits 1 in Control Phase

    writeAddress(0x00, 0x01, 0x01, socket);	 // Socket Command  Register -> OPEN

	/*temp1 = readAddress(0x00, 0x03, socket);   // Socket Status Register -> 0x13
	sprintf (LineBuffer, "Status: %X", temp1);
    Serial_SendString(LineBuffer);
    Serial_SendString("\n");*/

	writeAddress(0x00, 0x01, 0x04, socket);	 // Socket  Command  Register -> CONNECT
    _delay_ms(150);
   /* temp1 = readAddress(0x00, 0x03, socket);   // Socket Status Register -> 0x13
	sprintf (LineBuffer, "Status: %X", temp1);
    Serial_SendString(LineBuffer);
    Serial_SendString("\n");*/

	temp1 = sendData(datlen, data, socket);

    _delay_ms(250);                            // wait before disconnect

    writeAddress(0x00, 0x01, 0x08, socket);	 // Socket Command  Register -> DISCONNECT
	writeAddress(0x00, 0x01, 0x10, socket);	 // Socket Command  Register -> CLOSE
	/*temp1 = readAddress(0x00, 0x03, socket);   // Socket Status Register -> 0x13
	sprintf (LineBuffer, "Status: %X", temp1);
    Serial_SendString(LineBuffer);
    Serial_SendString("\n");*/
}
