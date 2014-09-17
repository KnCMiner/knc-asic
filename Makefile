CFLAGS=-g -O0 -W -Wall -Werror -I.

LDLIBS=-lz

BINARIES = asic knc-serial io-pwr knc-led RPi_gpio_pud

.PHONY: waas

all: $(BINARIES) waas

asic: knc-asic.o knc-spimux.o knc-transport-spimux.o logging.o

knc-led: knc-asic.o knc-spimux.o knc-transport-spimux.o logging.o

io-pwr: i2c.o tps65217.o

waas:
	make -C waas

clean:
	rm -f $(BINARIES) *.o *~
	make -C waas clean
