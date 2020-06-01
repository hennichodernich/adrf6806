#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "threewire.h"
#include "config_spidev.h"

static void pabort(const char *s)
{
	perror(s);
	exit(-1);
}

int threewire_init(t_spipintriple *spipins)
{
    const char *devname = SPI_DEVNAME;
    static int fd;
    int ret;
    static uint32_t mode = SPI_MODE;
    static uint8_t bits = 8;
    static uint32_t speed = SPI_SPEED;

    fd = open(devname, O_RDWR);
    if (fd < 0)
	pabort("can't open device");
    spipins->context = &fd;

    /*
     * spi mode
    */
    ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
    if (ret == -1)
	pabort("can't set spi mode");

    ret = ioctl(fd, SPI_IOC_RD_MODE32, &mode);
    if (ret == -1)
	pabort("can't get spi mode");

    /*
     * bits per word
     */
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
	pabort("can't set bits per word");

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
	pabort("can't get bits per word");

    /*
     * max speed hz
     */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
 	pabort("can't set max speed hz");

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
  	pabort("can't get max speed hz");

    /*
    printf("spi mode: 0x%x\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
    */
    return 0;
};

void threewire_clearpins(t_spipintriple spipins)
{
	return;
};

int threewire_read16(t_spipintriple spipins, uint8_t addr)
{
    return 0;
};

void threewire_write16(t_spipintriple spipins, uint8_t addr, uint16_t data)
{
    uint8_t rx[4]={0,0,0,0};
    uint8_t tx[4];
    int ret;

    struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)tx,
	.rx_buf = (unsigned long)rx,
	.len = 3,
	.delay_usecs = SPI_DELAY,
	.speed_hz = SPI_SPEED,
	.bits_per_word = 8,
    };

    tx[0] = addr;
    tx[1] = (data>>8) & 0xff;
    tx[2] = data & 0xff;

    
    ret = ioctl(*((int *)spipins.context), SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
	pabort("can't send spi message");

};

void threewire_writeraw24(t_spipintriple spipins, uint32_t data)
{
    uint8_t rx[4]={0,0,0,0};
    uint8_t tx[4];
    int ret;

    struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)tx,
	.rx_buf = (unsigned long)rx,
	.len = 3,
	.delay_usecs = SPI_DELAY,
	.speed_hz = SPI_SPEED,
	.bits_per_word = 8,
    };

    tx[0] = (data>>16) & 0xff;
    tx[1] = (data>>8) & 0xff;
    tx[2] = data & 0xff;

    ret = ioctl(*((int *)spipins.context), SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
	pabort("can't send spi message");
};

int threewire_close(t_spipintriple spipins)
{
    close(*((int *)spipins.context));
    return(0);
}
