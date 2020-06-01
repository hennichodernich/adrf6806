CC      = gcc
CFLAGS  = -Wall -I/usr/include/libftdi1
LDFLAGS = -lm -lwiringPi
FTDI_LDFLAGS = -lm -lftdi1
DUMMY_LDFLAGS = -lm

all: adrf6806 write_reg
dummy: adrf6806_dummy
ftdi: adrf6806_ftdi write_reg_ftdi
spidev: adrf6806_spidev	


adrf6806: adrf6806.o rpi_threewire.o frap.o
	$(CC) $(CFLAGS) -o adrf6806 adrf6806.o rpi_threewire.o frap.o $(LDFLAGS)

adrf6806_dummy: adrf6806.o dummy_threewire.o frap.o
	$(CC) $(CFLAGS) -o adrf6806_dummy adrf6806.o dummy_threewire.o frap.o $(DUMMY_LDFLAGS)

adrf6806_ftdi: adrf6806.o ftdi_threewire.o frap.o
	$(CC) $(CFLAGS) -o adrf6806_ftdi adrf6806.o ftdi_threewire.o frap.o $(FTDI_LDFLAGS)

adrf6806_spidev: adrf6806.o spidev_threewire.o frap.o
	$(CC) $(CFLAGS) -o adrf6806_spidev adrf6806.o spidev_threewire.o frap.o $(DUMMY_LDFLAGS)

adrf6806.o: adrf6806.c
	$(CC) $(CFLAGS) -c $<

write_reg: write_reg.o rpi_threewire.o
	$(CC) $(CFLAGS) -o write_reg write_reg.o rpi_threewire.o $(LDFLAGS)

write_reg_ftdi: write_reg.o ftdi_threewire.o
	$(CC) $(CFLAGS) -o write_reg_ftdi write_reg.o ftdi_threewire.o $(FTDI_LDFLAGS)

write_reg.o: write_reg.c
	$(CC) $(CFLAGS) -c $<

frap.o: frap.c
	$(CC) $(CFLAGS) -c $<


rpi_threewire.o: rpi_threewire.c
	$(CC) $(CFLAGS) -c $<

dummy_threewire.o: dummy_threewire.c
	$(CC) $(CFLAGS) -c $<

ftdi_threewire.o: ftdi_threewire.c
	$(CC) $(CFLAGS) -c $<

spidev_threewire.o: spidev_threewire.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -Rf adrf6806 adrf6806.o adrf6806_dummy adrf6806_ftdi write_reg.o write_reg write_reg_ftdi rpi_threewire.o dummy_threewire.o ftdi_threewire.o frap.o spidev_threewire.o
