/****************************************************************************
 Copyright:      Kai Riedel
 Author:         Kai Riedel
 Remarks:        MSP430F2012 (eZ430)
 Version:        03.07.2014
 Description:    Firmware for radio sensor to temperature data logger
//------------------------------------------------------------------------------*/


#include "io430.h"
#include "intrinsics.h"
//#include "stdio.h"      // by use of printf

#define Nsel P1OUT_bit.P1 
#define Sdi P1OUT_bit.P0
#define Sdo P1IN_bit.P2
#define Sck P1OUT_bit.P3
#define Ntc P1OUT_bit.P7

#define TIMEOUT 1000
#define SENSOR 'A'      // sensor number
//#define TEMP_MIN_TEMP -21
//#define TEMP_MAX_TEMP 98
#define TEMP_MIN_TEMP -40
#define TEMP_MAX_TEMP 79
#define TEMP_TABLE_SIZE 120
#define PACKET_LEN      4

unsigned int i;
unsigned int transmit_timer=1000;
unsigned int rf12_crc;         // running crc value
char buffer [4];
long IntDegC;
long temp;

/*static const unsigned int Temperature_Lookup[] = {
   0x3B4, 0x3B0, 0x3AB, 0x3A6, 0x3A0, 0x39A, 0x394, 0x38E, 0x388, 0x381, 0x37A, 0x373,
   0x36B, 0x363, 0x35B, 0x353, 0x34A, 0x341, 0x338, 0x32F, 0x325, 0x31B, 0x311, 0x307,
   0x2FC, 0x2F1, 0x2E6, 0x2DB, 0x2D0, 0x2C4, 0x2B8, 0x2AC, 0x2A0, 0x294, 0x288, 0x27C,
   0x26F, 0x263, 0x256, 0x24A, 0x23D, 0x231, 0x225, 0x218, 0x20C, 0x200, 0x1F3, 0x1E7,
   0x1DB, 0x1CF, 0x1C4, 0x1B8, 0x1AC, 0x1A1, 0x196, 0x18B, 0x180, 0x176, 0x16B, 0x161,
   0x157, 0x14D, 0x144, 0x13A, 0x131, 0x128, 0x11F, 0x117, 0x10F, 0x106, 0x0FE, 0x0F7,
   0x0EF, 0x0E8, 0x0E1, 0x0DA, 0x0D3, 0x0CD, 0x0C7, 0x0C0, 0x0BA, 0x0B5, 0x0AF, 0x0AA,
   0x0A4, 0x09F, 0x09A, 0x096, 0x091, 0x08C, 0x088, 0x084, 0x080, 0x07C, 0x078, 0x074,
   0x071, 0x06D, 0x06A, 0x067, 0x064, 0x061, 0x05E, 0x05B, 0x058, 0x055, 0x053, 0x050,
   0x04E, 0x04C, 0x049, 0x047, 0x045, 0x043, 0x041, 0x03F, 0x03D, 0x03C, 0x03A, 0x038
   };*/

static const unsigned int Temperature_Lookup[] = {
  0x3E9, 0x3E7, 0x3E5, 0x3E4, 0x3E2, 0x3DF, 0x3DD, 0x3DB, 0x3D8, 0x3D6, 0x3D3,
  0x3D0, 0x3CD, 0x3C9, 0x3C6, 0x3C2, 0x3BE, 0x3BA, 0x3B6, 0x3B2, 0x3AD, 0x3A8,
  0x3A3, 0x39D, 0x398, 0x392, 0x38C, 0x385, 0x37F, 0x378, 0x371, 0x36A, 0x362,
  0x35A, 0x352, 0x34A, 0x341, 0x338, 0x32F, 0x326, 0x31C, 0x312, 0x308, 0x2FE,
  0x2F4, 0x2E9, 0x2DE, 0x2D3, 0x2C8, 0x2BD, 0x2B1, 0x2A6, 0x29A, 0x28F, 0x283,
  0x277, 0x26B, 0x25F, 0x253, 0x247, 0x23B, 0x22F, 0x223, 0x217, 0x20B, 0x1FF,
  0x1F3, 0x1E8, 0x1DC, 0x1D1, 0x1C5, 0x1BA, 0x1AF, 0x1A4, 0x199, 0x18F, 0x184,
  0x17A, 0x170, 0x166, 0x15C, 0x153, 0x14A, 0x140, 0x137, 0x12F, 0x126, 0x11E,
  0x116, 0x10E, 0x106, 0x0FE, 0x0F7, 0x0F0, 0x0E9, 0x0E2, 0x0DB, 0x0D5, 0x0CF,
  0x0C9, 0x0C3, 0x0BD, 0x0B7, 0x0B2, 0x0AD, 0x0A8, 0x0A3, 0x09E, 0x099, 0x095,
  0x090, 0x08C, 0x088, 0x084, 0x080, 0x07C, 0x079, 0x075, 0x072, 0x06E, 0x06B
   };

signed char Temperature_GetTemperature(unsigned int Temp_ADC)
{
	if (Temp_ADC > Temperature_Lookup[0])
	  return TEMP_MIN_TEMP;

	for (char Index = 0; Index < TEMP_TABLE_SIZE; Index++)
	{
		if (Temp_ADC > Temperature_Lookup[Index])
		  return (Index + TEMP_MIN_TEMP);
	}

	return TEMP_MAX_TEMP;
}


unsigned int Spi16(unsigned int Dout)
{

  unsigned int Nspi, Dspi;
  unsigned int Dsdo,D;

  Nsel = 0;
  Dsdo = 0;
  D= Dout;
  for (Nspi = 1;Nspi < 17;Nspi++)
  {
    Dspi = D & 0x8000;
    if (Dspi == 0) Sdi = 0;
	else Sdi = 1;
    D = D  << 1;
    Dsdo = Dsdo  << 1;
	if (Sdo) Dsdo +=1;
    Sck = 1;
    __delay_cycles(5);
    Sck = 0;
  }
  Nsel = 1;
  return Dsdo;
}

void RFM12_Init (void)
{
  Nsel = 1;
  Sck = 0;
  Spi16(0x80e7);      //El , Ef , 11.5pf, 868 MHz band
  Spi16(0x82d9);      //!er , !ebb , Et , Es , Ex , !eb , !ew , Dc
  //Spi16(0xa67c);      // 868,3 MHz
  Spi16(0xa640);      // 868,0 MHz
  Spi16(0xc647);      // 4.8kbps
  //Spi16(0xc691);      // 2.4kbps
  Spi16(0x94a4);      // Vdi , Fast , 134 kHz , 0dbm , -79dbm
  Spi16(0xc2ac);      // Al , !ml , Dig , Dqd4
  Spi16(0xca81);      // Fifo8 , Sync , !ff , Dr
  Spi16(0xc483);      // @pwr , No Rstric , !st , !fi , Oe , En
  Spi16(0x9850);      // ±90 kHz , power - 0 dB (maximum)
  Spi16(0xe000);
  Spi16(0xc800);
  Spi16(0xc000);      // 1 MHz , 2.2V
  Spi16(0x0000);      // request status (dummy)
}

unsigned int Wait_rfm12( unsigned int Timeout)
{
  unsigned int T;
  
  Nsel = 0;
  T = 0;
  do
  {
     T += 1;
     __delay_cycles(100);
  }
  while ((Sdo == 0) && (T < Timeout));
  if (T < Timeout) return 1; else return 0;
}

//Polynomial: x^16 + x^15 + x^2 + 1 (0xa001)
//Initial value: 0xffff
unsigned int crc16_update(unsigned int crc, unsigned char a)
{
    unsigned char i;

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

unsigned int Send_rfm12string(char *outstring)
{
  unsigned int D;
 
  D = Spi16(0x0000);                      // request status (dummy)
  if (Wait_rfm12(TIMEOUT) == 0) return 0;
  D = Spi16(0xb8aa);
  if (Wait_rfm12(TIMEOUT) == 0) return 0;
  D = Spi16(0xb8aa);
  if (Wait_rfm12(TIMEOUT) == 0) return 0;
  D = Spi16(0xb8aa);
  if (Wait_rfm12(TIMEOUT) == 0) return 0;
  D = Spi16(0xb82d);
  if (Wait_rfm12(TIMEOUT) == 0) return 0;
  D = Spi16(0xb8d4);
  if (Wait_rfm12(TIMEOUT) == 0) return 0;
  D = Spi16(0xb800);
  if (Wait_rfm12(TIMEOUT) == 0) return 0;
  D = Spi16(0xb800 + PACKET_LEN);          // data length
  if (Wait_rfm12(TIMEOUT) == 0) return 0;
  D = Spi16(0xB800 + SENSOR);              // Send Sensornumber
  while (*outstring != '\0')
  {  
    if (Wait_rfm12(TIMEOUT) == 0) return 0;
    D = 0xB800 + *outstring;
    rf12_crc = crc16_update(rf12_crc, *outstring);	
    outstring++;
    D = Spi16(D);
  }
  if (Wait_rfm12(TIMEOUT) == 0) return 0;
  D= 0xB800 + (rf12_crc & 0xFF);
  D = Spi16(D);                         //Send Checksum Low
  if (Wait_rfm12(TIMEOUT) == 0) return 0;
  D= 0xB800 + (rf12_crc >> 8);
  D = Spi16(D);                         //Send Checksum High
  if (Wait_rfm12(TIMEOUT) == 0) return 0;
  D = Spi16(0xb8aa);
  if (Wait_rfm12(TIMEOUT) == 0) return 0;
  D = Spi16(0xb8aa);
  if (Wait_rfm12(TIMEOUT) == 0) return 0;
  return 1;
}

int ltoa_format (char *erg, long zahl, unsigned int vk, unsigned int nk)
{ // Out-String, input long, pre-decimal digits, decimal digits, sign '+' or ' ' 
  char vorz = '+'; 
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

void Timer (void)
{
    ADC10CTL0 &= ~ENC;                      // Disable conversion
    
    ADC10CTL1 = INCH_5 + ADC10DIV_3;         // ADC Channel A5, ADC10CLK/4, ADC Clock=ADC10OSC
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + REFOUT; //64 x ADC10CLKs, Uref=1,5V, ADC enabled
    __delay_cycles (200);
    ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
    while (ADC10CTL1 & ADC10BUSY);          // ADC10BUSY?
    transmit_timer = 9*ADC10MEM+500;        // store ADC value -> time: ~5s...90s
    
    ADC10CTL0=0;                            // Disable ADC
    ADC10CTL1=0;
    ADC10CTL0=0;
}

void Transmit(void)
{
    Ntc = 1;                                // enable NTC sensor  
  
    __delay_cycles(5000);                  // wait 3 ms
    
    //ADC10CTL1 = INCH_10 + ADC10DIV_3;         // Internal Temp Sensor ADC10CLK/4, ADC Clock=ADC10OSC
    //ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON; //64 x ADC10CLKs, Uref=1,5V, ADC enabled
    
    // external temperature sensor (NTC)
    ADC10CTL1 = INCH_6 + ADC10DIV_3;         // ADC Channel A6, ADC10CLK/4, ADC Clock=ADC10OSC
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + REFON + ADC10ON; //64 x ADC10CLKs, Uref=VCC, ADC enabled
    ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
    while (ADC10CTL1 & ADC10BUSY);          // ADC10BUSY?
    temp = ADC10MEM;                        // store ADC value
      
    Ntc = 0;                                // disable NTC sensor
    
    IntDegC = Temperature_GetTemperature (temp); // calculate temperature, external NTC
    IntDegC += 50;                               // offset for transmission -> no value=zero
   
    //IntDegC = ((temp - 673) * 423) / 1024;// Calculate temperature value, internal sensor
    //IntDegC += 50;                               // offset for transmission -> no value=zero
    
     ADC10CTL0 &= ~ENC;                      // Disable conversion
        
     ADC10CTL1 = INCH_11;                    // Select A11 -> AVcc/2
     ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON; //64 x ADC10CLKs, Uref=1,5V, ADC enabled
     __delay_cycles (100);                  // wait 100 µs
     ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
     while (ADC10CTL1 & ADC10BUSY);          // ADC10BUSY?
     
      Spi16(0x8239);                         // enable RFM12   
      __delay_cycles(5000);                  // wait 3 ms after RFM12 enabled
     
     if (ADC10MEM < 0x311)                   // ADC10MEM = A11 < 0.65 (VCC < 2,3V) with Uref=1,5V
     {
        Send_rfm12string("X");               // Send Battery warning message
     }
     else 
      {
       ltoa_format(buffer, IntDegC, 2, 0);
       rf12_crc = 0xFFFF;                        // crc initial value
       rf12_crc = crc16_update(rf12_crc, 0xD4); // group ID for crc16
       rf12_crc = crc16_update(rf12_crc, 0x00); 
       rf12_crc = crc16_update(rf12_crc, PACKET_LEN); 
       rf12_crc = crc16_update(rf12_crc, SENSOR); 
       Send_rfm12string(buffer);                // send first telegram
       }
     
    //printf ("%d\n", IntDegC);             // use as control in terminal window
    //printf ("\n");
    //printf (buffer);    
    
    Spi16(0x8201);                          // set RFM12 in Sleep mode
    Sdi = 0;
     
    ADC10CTL0=0;                            // Disable ADC
    ADC10CTL1=0;
    ADC10CTL0=0;
   
    BCSCTL3 |= LFXT1S_2;                   // LFXT1 = VLO
    IFG1 &= ~OFIFG;                        // Clear OSCFault flag
    BCSCTL2 |= SELM_3 + DIVM_3;            // MCLK = LFXT1/8
 }


void main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;

  BCSCTL1 = CALBC1_1MHZ;      // Set the calibrated clock value
  DCOCTL = CALDCO_1MHZ;       // Set DCO step and modulation

  //P1DIR = 0xEB;              // P1.0, P1.1, P1.3, P1.5, P1.6, P1.7 outputs, P1.2, P1.4 inputs, internal sensor
  P1DIR = 0x8B;              // P1.0, P1.1, P1.3, P1.4, P1.7 outputs, P1.2, P1.5, P1.6 inputs
  P1REN = 0x04;              // P1.2 with pull up
  P1OUT = 0x00;              // All P1.x reset
  P1SEL = 0x00;              // select P1.x I/O functions 
  
  for (i = 10000; i > 0; i--);                 // power on delay
 
  Timer();                                    // get timer value for transmit 
     
  RFM12_Init ();
    
  while (1)
  {
    __bic_SR_register(SCG1 + SCG0);          // Start DCO
    BCSCTL2 = 0x00;                          // clock default values 
    
    Transmit();                               // measure temperature, send one telegram
        
    __bis_SR_register(SCG1 + SCG0);           // Stop DCO -> low power mode
    
    for (i = transmit_timer; i > 0; i--);     // Delay 10000x -> ca. 90s with VLO = 12 kHz
   }
}
