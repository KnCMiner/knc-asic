#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "knc-asic.h"

#include "knc-transport.h"

#include "logging.h"

#define UNUSED __attribute__((unused))

struct knc_die_info die_info;

#define chip_version (die_info.version)

static void detect_chip(void *ctx, int channel, int die)
{
	if (chip_version != KNC_VERSION_UNKNOWN)
		return;

	if (knc_detect_die(ctx, channel, die, &die_info) != 0) {
		printf("ERROR: No asic found\n");
		exit(1);
	}
	chip_version = die_info.version;
}

static void do_info(void *ctx, int channel, int die, UNUSED int argc, UNUSED char **args)
{
	detect_chip(ctx, channel, die);

	if (knc_detect_die(ctx, channel, die, &die_info) != 0) {
		printf("ERROR: No asic found\n");
		exit(1);
	}

	printf("Version: %d\n", die_info.version);
	printf("Cores: %d\n", die_info.cores);
	if (die_info.pll_power_down >= 0) printf("PLL power: %s\n", die_info.pll_power_down ? "DOWN" : "UP");
	if (die_info.pll_locked >= 0)     printf("PLL status: %s\n", die_info.pll_locked ? "GOOD" : "BAD");
	if (die_info.hash_reset_n >= 0)   printf("HASH reset: %s\n", die_info.hash_reset_n ? "NORMAL" : "RESET");
	if (die_info.pll_reset_n >= 0)    printf("PLL reset: %s\n", die_info.pll_reset_n ? "NORMAL" : "RESET");
	printf("Want work: ");
	int core;
	for (core = 0; core < die_info.cores; core++) {
		putchar(die_info.want_work[core] == 0 ? '.' :
		        die_info.want_work[core] == 1 ? '*' :
		        '?');
	}
	putchar('\n');
	chip_version = die_info.version;
}

static int hex_decode(uint8_t *dst, const char *src, size_t max_len)
{
	size_t len = 0;
	int dir = 1;
	memset(dst, 0, max_len);
	if (strncmp(src, "0x", 2) == 0 || strncmp(src, "0X", 2) == 0) {
		src += 2;
		dst = dst + max_len - 1;
		dir = -1;
	}
	while (*src && len < max_len) {
		char octet[3] = {'0','0', 0};
		octet[1] = *src++;
		if (*src) {
			octet[0] = octet[1];
			octet[1] = *src++;
		}
		*dst = strtoul(octet, NULL, 16);
		dst += dir;
		len++;
	}
	return len;
}

static void handle_report(uint8_t *response)
{
	struct knc_report report;
	int nonces = 0;
	knc_decode_report(response, &report, chip_version);

	switch(chip_version) {
	case KNC_VERSION_JUPITER:
		nonces = 1;
		printf("Next    : %s\n", report.next_state ? "LOADED" : "FREE");
		printf("Current : 0x%x\n", report.active_slot);
		break;
	case KNC_VERSION_NEPTUNE:
		nonces = 5;
		printf("Next    : 0x%x %s\n", report.next_slot, report.next_state ? "LOADED" : "FREE");
		printf("Current : 0x%x %s\n", report.active_slot, report.state ? "HASHING" : "IDLE");
		break;
	case KNC_VERSION_UNKNOWN:
		break; /* To keep GCC happy */
	}
	printf("Progress: 0x%08x\n", report.progress);
	int n;
	for (n = 0; n < nonces; n++) {
		printf("Nonce %d : 0x%x %08x\n", n, report.nonce[n].slot, report.nonce[n].nonce);
	}
}

static void do_setwork(void *ctx, int channel, int die, UNUSED int argc, char **args)
{
	uint8_t midstate[8*4];
	uint8_t data[16*4+3*4];
	int request_length = 4 + 1 + 6*4 + 3*4 + 8*4;
	uint8_t request[request_length];
	int response_length = 1 + 1 + (1 + 4) * 5;
	uint8_t response[response_length];
	struct work work = {
		.midstate = midstate,
		.data = data
	};

	int core = strtoul(*args++, NULL, 0);
	int slot = strtoul(*args++, NULL, 0);
	int clean = strtoul(*args++, NULL, 0);
	memset(data, 0, sizeof(data));
	hex_decode(midstate, *args++, sizeof(midstate));
	hex_decode(data+16*4, *args++, sizeof(data)-16*4);

	detect_chip(ctx, channel, die);

	switch(chip_version) {
	case KNC_VERSION_JUPITER:
		if (clean) {
			/* Double halt to get rid of any previous queued work */
			request_length = knc_prepare_jupiter_halt(request, die, core);
			knc_syncronous_transfer(ctx, channel, request_length, request, 0, NULL);
			knc_syncronous_transfer(ctx, channel, request_length, request, 0, NULL);
		}
		request_length = knc_prepare_jupiter_setwork(request, die, core, slot, &work);
		knc_syncronous_transfer(ctx, channel, request_length, request, 1, response);
		if (response[0] == 0x7f)
			applog(LOG_ERR, "KnC %d-%d: Core disabled", channel, die);
		break;
	case KNC_VERSION_NEPTUNE:
		request_length = knc_prepare_neptune_setwork(request, die, core, slot, &work, clean);
		int status = knc_syncronous_transfer(ctx, channel, request_length, request, response_length, response);
		if (status != KNC_ACCEPTED) {
			if (response[0] == 0x7f) {
				applog(LOG_ERR, "KnC %d-%d: Core disabled", channel, die);
				return;
			}
			if (status & KNC_ERR_MASK) {
				applog(LOG_ERR, "KnC %d-%d: Failed to set work state (%x)", channel, die, status);
				return;
			}
			if (!(status & KNC_ERR_MASK)) {
				/* !KNC_ERRMASK */
				applog(LOG_ERR, "KnC %d-%d: Core busy", channel, die, status);
			}
		}
		handle_report(response);
		break;
	case KNC_VERSION_UNKNOWN:
		break; /* To keep GCC happy */
	}
}

static void do_report(void *ctx, int channel, int die, UNUSED int argc, char **args)
{
	int core = strtoul(*args++, NULL, 0);
	uint8_t request[4];
	int request_length;
	int response_length = 1 + 1 + (1 + 4) * 5;
	uint8_t response[response_length];
	int status;

	detect_chip(ctx, channel, die);

	request_length = knc_prepare_report(request, die, core);

	switch(chip_version) {
	case KNC_VERSION_JUPITER:
		response_length = 1 + 1 + (1 + 4);
		knc_syncronous_transfer(ctx, channel, request_length, request, response_length, response);
		break;
	case KNC_VERSION_NEPTUNE:
		status = knc_syncronous_transfer(ctx, channel, request_length, request, response_length, response);
		if (status) {
			applog(LOG_ERR, "KnC %d-%d: Failed (%x)", channel, die, status);
			return;
		}
	case KNC_VERSION_UNKNOWN:
		break; /* To keep GCC happy */
	}
	handle_report(response);
}

static void do_halt(void *ctx, int channel, int die, UNUSED int argc, char **args)
{
	int core = strtoul(*args++, NULL, 0);
	int request_length = 4 + 1 + 6*4 + 3*4 + 8*4;
	uint8_t request[request_length];
	int status;

	detect_chip(ctx, channel, die);

	switch(chip_version) {
	case KNC_VERSION_JUPITER:
		request_length = knc_prepare_jupiter_setwork(request, die, core, 0, NULL);
		knc_syncronous_transfer(ctx, channel, request_length, request, 0, NULL);
		request_length = knc_prepare_jupiter_halt(request, die, core);
		knc_syncronous_transfer(ctx, channel, request_length, request, 0, NULL);
		request_length = knc_prepare_jupiter_halt(request, die, core);
		knc_syncronous_transfer(ctx, channel, request_length, request, 0, NULL);
		break;
	case KNC_VERSION_NEPTUNE:
		request_length = knc_prepare_neptune_halt(request, die, core);
		status = knc_syncronous_transfer(ctx, channel, request_length, request, 0, NULL);
		if (status) {
			applog(LOG_ERR, "KnC %d-%d: Failed (%x)", channel, die, status);
			return;
		}
	case KNC_VERSION_UNKNOWN:
		break; /* To keep GCC happy */
	}
}

static void do_raw(void *ctx, int channel, int die, UNUSED int argc, char **args)
{
	uint8_t response[256];
	uint8_t request[256];

	int response_length = strtoul(*args++, NULL, 0);
	int request_length = hex_decode(request, *args++, sizeof(request));

	int status = knc_syncronous_transfer(ctx, channel, request_length, request, response_length, response);
	applog(LOG_ERR, "KnC %d-%d: STATUS=%x\n", channel, die, status);
}

static void do_freq(void *ctx, int channel, int die, UNUSED int argc, char **args)
{
	/* 4'op=2, 12'length, 4'bus, 4'die, 16'freq, many more clocks */
	int request_len = 4 + 12 + 16 + 4 + 4 + 16;
	int len = (request_len + 1000) / 8;
	uint8_t request[len];
	uint8_t response[len];

	uint32_t freq = strtoul(*args++, NULL, 0);

	if (freq > 1000000)
		freq = freq / 1000000;  // Assume Hz was given instead of MHz

	memset(request, 0, sizeof(request));
	request[0] = 2 << 4 | ((len * 8) >> 8);
	request[1] = (len * 8) >> 0;
	request[2] = ((channel+1) << 4)  | (die << 0);
	request[3] = (freq >> 8);
	request[4] = (freq >> 0);

	knc_trnsp_transfer(ctx, request, response, len);

	int i;
	for (i = request_len / 8; i < len-1; i++) {
		if (response[i] == 0xf1) {
			break;
		} else if (response[i] == 0xf0) {
			applog(LOG_DEBUG, "KnC %d-%d: Accepted FREQ=%d", channel, die, response[i+1]<<8 | response[i+2]);
			i+=2;
		}
	}
	if (response[i] == 0xf1)
		applog(LOG_INFO, "KnC %d-%d: Frequency change successful", channel, die);
	else
		applog(LOG_ERR, "KnC %d-%d: Frequency change FAILED!", channel, die);
}

static void do_status(void *ctx, int channel, UNUSED int die, UNUSED int argc, UNUSED char **args)
{
	/* 4'op=3, 3'channel, 9'x -> 32'revision, 8'board_type, 8'board_revision, 48'reserved, 1440'core_available (360' per die) */
	int request_len = 16;
	int len = (request_len + 32 + 8 + 8 + 48 + 360 * 4) / 8;
	uint8_t request[len];
	uint8_t response[len];

	memset(request, 0, sizeof(request));
	request[0] = 3 << 4 | (channel + 1) << 1;

	knc_trnsp_transfer(ctx, request, response, len);

}

struct knc_command {
	const char *name;
	const char *args;
	const char *description;
	int nargs;
	void (*handler)(void *ctx, int channel, int die, int nargs, char **args);
} knc_commands[] = {
	{"info", "", "ASIC version & info", 0, do_info},
	{"status", "", "ASIC status", 0, do_status},
	{"setwork", "core slot(1-15) clean(0/1) midstate data", "Set work vector", 5, do_setwork},
	{"report", "core", "Get nonce report", 1, do_report},
	{"halt", "core", "Halt core", 1, do_halt},
	{"freq", "frequency", "Set core frequency", 1, do_freq},
	{"raw", "response_length request_data", "Send raw ASIC request", 2, do_raw},
	{NULL, NULL, NULL, 0, NULL}
};

int main(int argc, char **argv)
{
	struct knc_command *cmd;
	void *ctx = knc_trnsp_new(0);
	int channel, die;
	char *command;
	char **args = &argv[1];
	
	for (;argc > 1 && *args[0] == '-'; argc--, args++) {
		if (strcmp(*args, "-n") == 0) {
			die_info.version = KNC_VERSION_NEPTUNE;
			die_info.cores = 360;
		}
		if (strcmp(*args, "-c") == 0 && argc > 1)  {
			die_info.cores = atoi(args[1]);
			argc--;
			args++;
		}
		if (strcmp(*args, "-j") == 0)
			chip_version = KNC_VERSION_JUPITER;
		if (strcmp(*args, "-d") == 0)
			debug_level = LOG_DEBUG;
	}
	if (argc < 4) {
		fprintf(stderr, "Usage: %s command arguments..\n", argv[0]);
		for (cmd = knc_commands; cmd->name; cmd++)
			fprintf(stderr, "  %s channel die %s\n	%s\n", cmd->name, cmd->args, cmd->description);
		exit(1);
	}

	command = *args++; argc--;
	if (isdigit(*command)) {
		channel = atoi(command);
	} else {
		channel = atoi(*args++); argc--;
	}
	die = atoi(*args++); argc--;
	if (isdigit(*command)) {
		command = *args++; argc--;
	}
	argc--;

	for (cmd = knc_commands; cmd->name; cmd++) {
		if (strcmp(cmd->name, command) == 0) {
			if (argc != cmd->nargs && cmd->nargs != -1) {
				fprintf(stderr, "ERROR: Invalid arguments");
				exit(1);
			}
			cmd->handler(ctx, channel, die, argc, args);
			goto done;

		}
	}

	knc_trnsp_free(ctx);

	fprintf(stderr, "ERROR: Unknown command %s\n", command);
	exit(1);

done:
	knc_trnsp_free(ctx);
	
	return 0;
	
}
