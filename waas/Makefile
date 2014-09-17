OPTFLAGS=-O2
CFLAGS=-W -Wall $(OPTFLAGS) -Werror $(DEFINES)

CFLAGS +=  -I..

LDLIBS += -lrt

all: waas

waas: waas.o jsmn.o ../i2c.o ../knc-asic.o ../knc-spimux.o ../knc-transport-spimux.o ../logging.o libpmbus.o asic.o eeprom.o

clean:
	rm -f waas *.o *~
