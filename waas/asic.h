#ifndef ASIC_H_
#define ASIC_H_

#include <stdint.h>
#include <stdbool.h>

#include "i2c.h"

#define	ARRAY_SIZE(a)		((int)(sizeof(a) / sizeof(a[0])))

#define	FIRST_ASIC_I2C_BUS	2

/* Hardware specs for the Neptune device */
#define	KNC_MAX_DCDC_DEVICES	(2 * KNC_MAX_DIES_PER_ASIC)

#define	ERICSSON_I2C_WORKAROUND_DELAY_us	100
#define	ERICSSON_SAFE_BIG_DELAY			100000

/* These enum values must be sorted in ascending order.
 * When iniitc detects "device type" it peeks the highest value of all boards.
 */
typedef enum brd_type_enum {
	ASIC_BOARD_REVISION_UNDEFINED = 0,
	ASIC_BOARD_4GE,
	ASIC_BOARD_8GE,
	ASIC_BOARD_ERICSSON
} brd_type_t;

typedef enum dcdc_mfrid_enum {
	MFRID_ERROR 	= -1,
	MFRID_UNDEFINED	= 0,
	MFRID_GE,
	MFRID_ERICSSON,
} dcdc_mfrid_t;

#define	DCDC_BASE_ADDR	0x10
#define	DCDC_ADDR(dcdc)	(DCDC_BASE_ADDR + (dcdc))
static inline void dcdc_set_i2c_address(int i2c_bus, int dcdc)
{
	i2c_set_slave_device_addr(i2c_bus, DCDC_ADDR(dcdc));
}
static inline dcdc_mfrid_t mfrid_from_board_type(brd_type_t brd_type)
{
	switch (brd_type) {
	case ASIC_BOARD_REVISION_UNDEFINED:
		return MFRID_UNDEFINED;
	case ASIC_BOARD_4GE:
	case ASIC_BOARD_8GE:
		return MFRID_GE;
	case ASIC_BOARD_ERICSSON:
		return MFRID_ERICSSON;
	}
	return MFRID_ERROR;
}

typedef struct asic_board {
	int id;
	bool enabled;
	brd_type_t type;
	bool neptune;
	uint8_t serial_num[32];	 /* One EEPROM page */
	int i2c_bus;
	int num_dcdc;
} asic_board_t;

brd_type_t asic_boardtype_from_serial(char *serial);
const char *get_str_from_board_type(brd_type_t brd_type, bool neptune);
bool asic_board_read_info(asic_board_t *board);
unsigned int asic_init_boards(asic_board_t **boards);
void asic_release_boards(asic_board_t **boards);
bool asic_init_board(asic_board_t *board, brd_type_t brd_type, bool neptune);
void asic_release_board(asic_board_t *board);

bool dcdc_is_ok(asic_board_t *board, int dcdc);
uint8_t get_primary_dcdc_addr_for_die(int die);

#endif /* ASIC_H_ */
