OPTFLAGS=-O2
CFLAGS=-g $(OPTFLAGS) -W -Wall -Werror -I. $(DEFINES)

LDLIBS=

BINARIES = asic knc-serial io-pwr knc-led RPi_gpio_pud

.PHONY: waas raspberry beaglebone backplane all

default: beaglebone

raspberry: DEFINES+=-DCONTROLLER_BOARD_RPI
beaglebone: DEFINES+=-DCONTROLLER_BOARD_BBB
backplane: DEFINES+=-DCONTROLLER_BOARD_BACKPLANE

raspberry: all
beaglebone: all
backplane: all

all: $(BINARIES) waas

asic: knc-asic.o knc-spimux.o knc-transport-spimux.o logging.o

knc-led: knc-asic.o knc-spimux.o knc-transport-spimux.o logging.o

io-pwr: i2c.o tps65217.o

waas:
	$(MAKE) -C waas DEFINES="$(DEFINES)"

clean:
	rm -f $(BINARIES) *.o *~
	make -C waas clean
