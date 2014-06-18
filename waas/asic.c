#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "asic.h"
#include "eeprom.h"

static const char brdtypestr_4GE[] = "AG";
static const char brdtypestr_8GE[] = "BG";
static const char brdtypestr_ERICSSON[] = "BE";
static const char brdtypestr_NEPTUNE_ERICSSON[] = "NE";

const char *get_str_from_board_type(brd_type_t brd_type, bool neptune)
{
	if (neptune)
		return brdtypestr_NEPTUNE_ERICSSON;

	switch (brd_type) {
	case ASIC_BOARD_8GE:
		return brdtypestr_8GE;
	case ASIC_BOARD_ERICSSON:
		return brdtypestr_ERICSSON;
	case ASIC_BOARD_4GE:
	default:
		return brdtypestr_4GE;
	}
}

brd_type_t asic_boardtype_from_serial(char *serial)
{
	switch (serial[1]) {
	case '1':
	case '2':
	case 'S':
		return ASIC_BOARD_4GE;
		break;
	case 'G':
		return ASIC_BOARD_8GE;
		break;
	case 'E':
		return ASIC_BOARD_ERICSSON;
		break;
	default:
		return ASIC_BOARD_REVISION_UNDEFINED;
		break;
	}
}

static bool device_is_Neptune_from_serial(char *serial)
{
	return ('N' == serial[0]);
}

static brd_type_t asic_boardtype_from_eeprom(uint8_t eeprom_revision)
{
	switch (eeprom_revision) {
	case ASIC_BOARD_4GE:
	case ASIC_BOARD_8GE:
	case ASIC_BOARD_ERICSSON:
		return eeprom_revision;
	default:
		return ASIC_BOARD_REVISION_UNDEFINED;
	}
}

unsigned int asic_init_boards(asic_board_t** boards)
{
	int idx;
	unsigned int good_boards = 0;
	for (idx = 0; idx < MAX_ASICS; ++idx) {
		asic_board_t * board = (asic_board_t*)calloc(1, sizeof(asic_board_t));
		board->id = idx;
		if (asic_init_board(board, ASIC_BOARD_REVISION_UNDEFINED, false) && board->enabled) {
			++good_boards;
		}
		boards[idx] = board;
	}
	return good_boards;
}

void asic_release_boards(asic_board_t** boards)
{
	int idx;
	for (idx = 0; idx < MAX_ASICS; ++idx) {
		asic_board_t * board = boards[idx];
		asic_release_board(board);
		free(board);
	}
}

bool asic_board_read_info(asic_board_t *board)
{
	struct eeprom_neptune data;

	board->enabled = read_eeprom(board->id, &data);

	board->type = ASIC_BOARD_REVISION_UNDEFINED;

	if (!board->enabled) {
		fprintf(stderr,
			"ASIC board #%d is non-functional: Bad EEPROM data\n",
			board->id);
		return false;
	}

	if ('X' == data.SN[sizeof(data.SN) - 1]) {
		fprintf(stderr,
			"ASIC board #%d is marked with TEST_FAIL flag\n",
			board->id);
	}
	strncpy((char *)board->serial_num, (char *)data.SN, sizeof(board->serial_num));
	board->serial_num[sizeof(board->serial_num) - 1] = '\0';

	board->type = asic_boardtype_from_eeprom(data.board_revision);
	if (ASIC_BOARD_REVISION_UNDEFINED == board->type)
		board->type = asic_boardtype_from_serial((char *)board->serial_num);
	board->neptune = device_is_Neptune_from_serial((char *)board->serial_num);
	return true;	
}

bool asic_init_board(asic_board_t *board, brd_type_t brd_type, bool neptune)
{
	if ((brd_type != ASIC_BOARD_4GE) && (brd_type != ASIC_BOARD_8GE) && (brd_type != ASIC_BOARD_ERICSSON)) {
		asic_board_read_info(board);
	} else {
		board->neptune = neptune;
		board->type = brd_type;
		memset(board->serial_num, 0, sizeof(board->serial_num));
		board->enabled = true;
	}
	if (board->enabled) {
		printf("ASIC board #%d: sn = %s type = %s\n", board->id,
		       board->serial_num, get_str_from_board_type(board->type, board->neptune));

		board->num_dcdc = 0;
		board->i2c_bus = i2c_connect(FIRST_ASIC_I2C_BUS + board->id);
	}
	return true;
}

void asic_release_board(asic_board_t* board)
{
	i2c_disconnect(board->i2c_bus);
	board->i2c_bus = -1;
}

bool dcdc_is_ok(asic_board_t *board, int dcdc)
{
	if ((0 > dcdc) || (MAX_DCDC_DEVICES <= dcdc))
		return false;

	switch (board->type) {
	case ASIC_BOARD_8GE:
	case ASIC_BOARD_ERICSSON:
		return true;
	case ASIC_BOARD_4GE:
	default:
		if ((0 == dcdc) || (2 == dcdc) || (4 == dcdc) || (7 == dcdc))
			return true;
	}

	return false;
}

uint8_t get_primary_dcdc_addr_for_die(int die)
{
	switch (die) {
	case 0:
		return DCDC_ADDR(0);
	case 1:
		return DCDC_ADDR(2);
	case 2:
		return DCDC_ADDR(4);
	case 3:
		return DCDC_ADDR(7);
	default:
		return DCDC_ADDR(0);
	}
}
