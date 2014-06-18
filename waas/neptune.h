#ifndef __NEPTUNE_H_
#define __NEPTUNE_H_

#ifndef PATH_MAX
#define	PATH_MAX	4096
#endif

#define	ARRAY_SIZE(a)	((int)(sizeof(a) / sizeof(a[0])))

/* Hardware specs for the Neptune device */

#define	ERICSSON_I2C_WORKAROUND_DELAY_us	100
#define	ERICSSON_SAFE_BIG_DELAY			100000

/* Max number of ASIC boards */
#define	MAX_ASICS		6

/* SPI bus to the control FPGA */
#define SPI_BUS			1

/* GPIO */

/* P8.8 = gpio2_3 = nCONFIG */
#define	nCONFIG_FILE		"/sys/class/gpio/gpio67/value"
/* P8.7 = gpio2_2 = CONF_DONE */
#define	CONFIG_DONE_FILE	"/sys/class/gpio/gpio66/value"
/* P8.9 = gpio2_5 = nSTATUS */
#define	nSTATUS_FILE		"/sys/class/gpio/gpio69/value"

/* I2C busses */
#define	FPGA_I2C_BUS_NUM	2

/* Debug info */
#ifdef	DEBUG_INFO
#include <stdio.h>
#define PRINT_DEBUG(...) fprintf(stderr, "[DEBUG] --- " __VA_ARGS__)
#else
#define PRINT_DEBUG(...)
#endif

#endif /* __NEPTUNE_H_ */
