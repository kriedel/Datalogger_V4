Datalogger_V4
=

Temperature and humidity data logging

The Radio Data Logger project is based on the LUFA TempDataLogger Demo project and published in Funkamateur 5/2012 (http://www.funkamateur.de/).

The project is a data logger for the current local temperature and humidity and additional two radio sensors for logging of distantly temperatures, writing the measured values to a file stored on the board's Dataflash in a FAT filesystem each time a specified interval elapses. When inserted into a PC, the datalogger will appear as a standard USB Mass Storage device with text files for each logging day, which contains the logged data. Files are named according to the current date of logging. A special PC software written in Visual C# allow the parametrization of the data logger (logging intervals, temperature correction etc.) and the analysis of the logging data (visualization as plot).

Main features:

* A DS1307 or compatible RTC IC is designed to be attached to the AVR's TWI bus, for the management of timestamps on the sampled data. This project will not function correctly if the RTC chip is omitted unless the DUMMY_RTC compile time token is specified.

* A HTY371 sensor from Hygrosens is connected to the AVR's TWI bus for the measurement of local temperature and humidity.

* A RFM12 radio modul from HopeRF is connected for receiving the signals from the external radio temperature sensors. Each radio sensor contains also a RFM12 modul and a eZ430-F2012 from TI with a MSP430F2012 as controller.

* A ethernet modul from Wiznet (WIZ550io) is connected to store data on a webserver in the cloud like Xively or Thingspeak. You can call up the data from everywhere. Data is displayed as a linechart for each logged value. In order to save energy the ethernet modul is only actived on demand from the controller.

* A LCD with ILI9341 controller (320 x 240 pixel, 65536 colors) display the measure values and the saved trends for the last day.

* Due to the host's need for exclusive access to the file system, the device will not log samples while connected to a host. For the logger to store data, the Dataflash must first be formatted by the host so that it contains a valid FAT file system.

* This project uses the FatFS library from ELM Chan (http://elm-chan.org/fsw/ff/00index_e.html) and the .NET HID device library LibHIDNet (http://sourceforge.net/projects/libhidnet/).

