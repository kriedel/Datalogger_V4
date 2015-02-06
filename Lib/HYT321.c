/****************************************************************************
 Copyright:      Kai Riedel based on LUFA Library
 Author:         Kai Riedel
 Remarks:        AVR AT90USB1287
 Version:        15.05.2011
 Description:    Temperature Data Logger with radio sensors
//------------------------------------------------------------------------------*/

#include "HYT321.h"

void HYT321_GetData(int8_t* const Humidity,
                    int8_t* const Temperature)
{
    uint32_t Calc_Humidity = 0;
    int32_t Calc_Temperature = 0;
    HYT321_DataRegs_t CurrentHYTData;
    CurrentHYTData.TemperatureMSB = 0;
    CurrentHYTData.HumidityLSB = 0;

    if (TWI_StartTransmission(HYT321_ADDRESS_WRITE | TWI_ADDRESS_WRITE, 25)== TWI_ERROR_NoError)    // Measurement Request
    {
        TWI_StopTransmission();
    }

    if (TWI_StartTransmission(HYT321_ADDRESS_READ | TWI_ADDRESS_READ, 25)== TWI_ERROR_NoError)      // Data Fetcb
	{

	    TWI_ReceiveByte(&CurrentHYTData.Byte1.IntVal, false);
	    TWI_ReceiveByte(&CurrentHYTData.HumidityLSB, false);
		TWI_ReceiveByte(&CurrentHYTData.TemperatureMSB, false);
		TWI_ReceiveByte(&CurrentHYTData.Byte2.IntVal, true);

		TWI_StopTransmission();
	}

	Calc_Humidity = ((uint32_t) CurrentHYTData.Byte1.Fields.HumidityMSB << 8) + CurrentHYTData.HumidityLSB;
	Calc_Humidity = (Calc_Humidity * 100) / 16384;

	Calc_Humidity = (Calc_Humidity * 106) + 156;        // Correct values based on result of climate chamber test
	Calc_Humidity /= 100;

	Calc_Temperature = ((int32_t) CurrentHYTData.TemperatureMSB << 6) + CurrentHYTData.Byte2.Fields.TemperatureLSB;
	Calc_Temperature = (Calc_Temperature * 165) / 16384 - 40;

    *Humidity = (int8_t) Calc_Humidity;
	*Temperature = (int8_t) Calc_Temperature;
}



