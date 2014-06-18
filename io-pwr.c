/*
 * KnCMiner IO Board power control
 *
 * Copyright (c) 2014  ORSoC AB
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "tps65217.h"
#include "i2c.h"

static int opt_verbose = 0;

static void print_usage(const char *prog)
{
	printf("Usage: %s [-v] [command,..]\n", prog);
	puts("  -v --verbose  Verbose operation\n"
	     "  init          Initialize I/O power\n"
	);
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "verbose",  0, 0, 'v' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "v", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'v':
			opt_verbose = 1;
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

static int io_pwr_init(void)
{
	int i2c_bus = i2c_connect(TPS65217_BUS_NUM);

	i2c_set_slave_device_addr(i2c_bus, TPS65217_BUS_ADDRESS);
	if (opt_verbose) fprintf(stderr, "Testing TPS65217\n");
	if(!test_tps65217(i2c_bus)) {
		fprintf(stderr, "TPS65217 TEST failure\n");
		return 1;
	}

	/* Try to configure DC(/DC for three times */
	if (opt_verbose) fprintf(stderr, "Configuring TPS65217\n");
	if(!configure_tps65217(i2c_bus)) {
	if (opt_verbose) fprintf(stderr, "Configuring TPS65217 try 2\n");
	if(!configure_tps65217(i2c_bus)) {
	if (opt_verbose) fprintf(stderr, "Configuring TPS65217 try 3\n");
	if(!configure_tps65217(i2c_bus)) {
		fprintf(stderr, "DC/DC converter configuration failed!\n");
		return 1;
	} } }

	i2c_disconnect(i2c_bus);
	if (opt_verbose) fprintf(stderr, "TPS65217 successfully initialized\n");

	return 0;
}

int main(int argc, char *argv[])
{
	parse_opts(argc, argv);

	if (optind >= argc) {
		print_usage(argv[0]);
		return 0;
	}

	if (strcmp(argv[optind], "init") == 0)
		return io_pwr_init();

	fprintf(stderr, "Unknown command '%s'\n", argv[optind]);
	return 1;
}
