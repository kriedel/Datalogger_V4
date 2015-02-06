/****************************************************************************
 Copyright:      Kai Riedel
 Author:         Kai Riedel
 Remarks:        AVR
 Version:        15.04.2011
 Description:    RFM12 Control functions
//------------------------------------------------------------------------------*/

#include "Funkmodul.h"

uint Spi16(uint Dout)
{

  uint Nspi, Dspi;
  uint Dsdo,D;

  RFM12_Port_Write &=~ (1<<NSEL); //NSEL = 0, chip select -> activ

  Dsdo = 0;
  D= Dout;
  for (Nspi = 1;Nspi < 17;Nspi++)
  {
    Dspi = D & 0x8000;
    if (Dspi == 0) RFM12_Port_Write &=~ (1<<SDI);
	else RFM12_Port_Write |= (1<<SDI);
    D = D  << 1;
    Dsdo = Dsdo  << 1;
	if (RFM12_Port_Read&(1<<SDO)) Dsdo +=1;
    RFM12_Port_Write |= (1<<SCK); //SCK = 1
    _delay_us (5);
    RFM12_Port_Write &=~ (1<<SCK); //SCK = 0
  }
   RFM12_Port_Write |= (1<<NSEL); //NSEL = 1
  return Dsdo;
}

void RFM12_Init (void)
{
  uint D;

  RFM12_Port_DDR |= (1<<SDI | 1<<NSEL | 1<<SCK | 1<<VDD); //set port directions

  RFM12_Port_Write |= (1<<VDD);  //enable VDD RFM12
  _delay_ms(1000);               //short wait until VDD is OK
  RFM12_Port_Write |= (1<<NSEL); //NSEL = 1
  RFM12_Port_Write &=~ (1<<SCK); //SCK = 0

  D = Spi16(0x80e7);      //El , Ef , 11.5pf, 868 MHz band
  D = Spi16(0x82d9);      // er , ebb , !Et , Es , Ex , !eb , !ew , Dc
 // D = Spi16(0xa67c);      // 868,3 MHz
  D = Spi16(0xa640);      // 868,0 MHz
  D = Spi16(0xc647);      // 4.8kbps
  D = Spi16(0x94A1);      // Vdi , Fast , 134 kHz , 0db , -103dbm, receiver bandwidth, lna gain, rssi treshold
  D = Spi16(0xc2a9);      // Al , !ml , Dig , Dqd1
  D = Spi16(0xca81);      // Fifo8 , Sync , !ff , Dr
  D = Spi16(0xc483);      // @pwr , No Rstric , !st , !fi , Oe , En
  //D = Spi16(0xc4F3);      // @pwr , No Rstric , !st , !fi , Oe , En
  D = Spi16(0x9850);      // 90 kHz , power - 0 dB (maximum)
  D = Spi16(0xe000);
  D = Spi16(0xc800);
  D = Spi16(0xc000);      // 1 MHz , 2.2V
  D = Spi16(0x0000);      // request status (dummy)

  //D = Spi16(0x8239);      			// start transmitter cw mode --> Test RFM12
}


uint Send_rfm12string(unsigned char *outstring)
{
  uint D;

  D = Spi16(0x8238);
  if (Wait_rfm12(RFM12_TIMEOUT) == 0) return 0;
  D = Spi16(0xb8aa);
  if (Wait_rfm12(RFM12_TIMEOUT) == 0) return 0;
  D = Spi16(0xb8aa);
  if (Wait_rfm12(RFM12_TIMEOUT) == 0) return 0;
  D = Spi16(0xb8aa);
  if (Wait_rfm12(RFM12_TIMEOUT) == 0) return 0;
  D = Spi16(0xb82d);
  if (Wait_rfm12(RFM12_TIMEOUT) == 0) return 0;
  D = Spi16(0xb8d4);
  if (Wait_rfm12(RFM12_TIMEOUT) == 0) return 0;
  D = Spi16(0xb802); //Send <STX>
  Checksum =0;
  while (*outstring != '\0')
  {
    if (Wait_rfm12(RFM12_TIMEOUT) == 0) return 0;
    D = 0xB800 + *outstring;
	Checksum += *outstring;
    outstring++;
    D = Spi16(D);
  }
  if (Wait_rfm12(RFM12_TIMEOUT) == 0) return 0;
  D= 0xB800 + Checksum;
  D = Spi16(D); //Send Checksum
  if (Wait_rfm12(RFM12_TIMEOUT) == 0) return 0;
  D = Spi16(0xb803); //Send <ETX>
  if (Wait_rfm12(RFM12_TIMEOUT) == 0) return 0;
  D = Spi16(0xb8aa);
  if (Wait_rfm12(RFM12_TIMEOUT) == 0) return 0;
  D = Spi16(0xb8aa);
  if (Wait_rfm12(RFM12_TIMEOUT) == 0) return 0;
  D = Spi16(0x8208);
  return 1;
}

uint Wait_rfm12(uint Timeout)
{

  uint T;

  RFM12_Port_Write &=~ (1<<NSEL); //NSEL = 0, chip select -> activ
  T = 0;
  {
     T += 1;
     _delay_us (100);
  }
  while (!(RFM12_Port_Read&(1<<SDO)) && (T < Timeout));
  if (T < Timeout) return 1; else return 0;
}

void Enable_receiver(void)
{
  RFM12_Port_Write &=~ (1<<NSEL); //NSEL = 0, chip select -> activ
  Spi16(0x82c8);
  Spi16(0xca83);
}

void Disable_receiver(void)
{
  //Spi16(0x8208);
  //Spi16(0xca81); // stop FIFO fill
  //RFM12_Port_Write |= (1<<NSEL); //NSEL = 1
  RFM12_Port_Write &=~ (1<<NSEL); //NSEL = 0, chip select -> activ
  RFM12_Port_Write &=~ (1<<SCK);  //SCK = 0
  RFM12_Port_Write &=~ (1<<SDI);  //SCK = 0
  RFM12_Port_Write &=~ (1<<VDD);  //VDD = 0 -> RFM12 offline
  _delay_ms(1000);
}

void RestartFifoFill_receiver(void)
{
  Spi16(0xca81); // stop FIFO fill
  _delay_ms(5);
  Spi16(0xca83); // restart FIFO synchron pattern recognition
}
