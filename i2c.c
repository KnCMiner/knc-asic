#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "i2c.h"

#ifdef _DEBUG
#	include <stdio.h>
#	define DEBUG(...) fprintf(stderr,"[DEBUG] --- " __VA_ARGS__)
#else
#	define DEBUG(...)
#endif

int i2c_connect(int adapter_nr)
{
	int fd;
	char filename[20];

	snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
	fd = open(filename, O_RDWR);
	DEBUG("Connected %s on fd %d\n",filename, fd);
	if(fd < 0)
		fprintf(stderr, "Error opening %s: %m\n", filename);
	return fd;
}

void i2c_disconnect(int fd)
{
	DEBUG("Disconnecting fd %d\n", fd);
	close(fd);
}

bool i2c_set_slave_device_addr(int fd, int addr)
{
	if (ioctl(fd, I2C_SLAVE, addr) < 0) {
		fprintf(stderr, "Error setting slave device address 0x%x: %m\n",addr);
		return false;
	}	
	return true;
}

int i2c_insist_read_byte(int bus, __u8 addr, useconds_t delay_us)
{
	int data;
	usleep(delay_us);
	data = i2c_smbus_read_byte_data(bus, addr);
	if ((0 > data) || (0xFF == data)) {
		usleep(delay_us);
		data = i2c_smbus_read_byte_data(bus, addr);
		if ((0 > data) || (0xFF == data)) {
			usleep(delay_us);
			data = i2c_smbus_read_byte_data(bus, addr);
			if ((0 > data) || (0xFF == data)) {
				fprintf(stderr, "i2c_smbus_read_byte_data failed: addr 0x%02X\n", addr);
				return -1;
			}
		}
	}
	return data;
}

int i2c_insist_read_word(int bus, __u8 addr, useconds_t delay_us)
{
	int data;
	usleep(delay_us);
	data = i2c_smbus_read_word_data(bus, addr);
	if ((0 > data) || (0xFFFF == data)) {
		usleep(delay_us);
		data = i2c_smbus_read_word_data(bus, addr);
		if ((0 > data) || (0xFFFF == data)) {
			usleep(delay_us);
			data = i2c_smbus_read_word_data(bus, addr);
			if ((0 > data) || (0xFFFF == data)) {
				fprintf(stderr, "i2c_smbus_read_word_data failed: addr 0x%02X\n", addr);
				return -1;
			}
		}
	}
	return data;
}
