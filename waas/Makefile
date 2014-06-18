CFLAGS=-W -Wall -O2 -Werror

CFLAGS +=  -I..

LDLIBS += -lrt

all: waas

waas: waas.o jsmn.o ../i2c.o ../knc-transport-spi.o ../logging.o libpmbus.o asic.o eeprom.o

clean:
	rm -f waas *.o *~
