#ifndef CONFIG_FTDI_H
#define CONFIG_FTDI_H

#define DEADBUG

#ifdef DEADBUG

#define CS 0		//TXD, DBUS0
#define SCLK 2		//RTS, DBUS2
#define SDIO 4		//DTR, DBUS4

#else

#define CS 4		
#define SCLK 2	
#define SDIO 0

#endif

#define FTDI_INTERFACE INTERFACE_B

#endif // CONFIG_FTDI_H
