CFLAGS=-g -O0 -W -Wall -Werror

LDLIBS=-lz

BINARIES = asic knc-serial io-pwr

.PHONY: waas

all: $(BINARIES) waas

asic: knc-asic.o knc-transport-spi.o logging.o

io-pwr: i2c.o tps65217.o

waas:
	make -C waas

clean:
	rm -f $(BINARIES) *.o *~
	make -C waas clean
