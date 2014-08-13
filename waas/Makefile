CFLAGS=-W -Wall -O2 -Werror

CFLAGS +=  -I..

LDLIBS += -lrt -lz

all: waas

waas: waas.o jsmn.o ../i2c.o ../knc-asic.o ../knc-spimux.o ../knc-transport-spimux.o ../logging.o libpmbus.o asic.o eeprom.o

clean:
	rm -f waas *.o *~
