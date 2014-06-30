#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "miner.h"
#include "knc-asic.h"
#include "knc-transport.h"
#include "logging.h"

int main(int argc, char **argv)
{
	void *ctx = knc_trnsp_new(0);
	char **args = &argv[1];
	
	if (argc > 1 && strcmp(*args, "-d") == 0)  {
		argc--;
		args++;
		debug_level = LOG_DEBUG;
	}
	if (argc != 4) {
		fprintf(stderr, "Usage: %s red green blue\n", argv[0]);
		exit(1);
	}

	uint8_t request[2], response[2];
	uint32_t red = strtoul(*args++, NULL, 0);
	uint32_t green = strtoul(*args++, NULL, 0);
	uint32_t blue = strtoul(*args++, NULL, 0);
	int len = knc_prepare_led(request, 0, sizeof(request), red, green, blue);
	knc_trnsp_transfer(ctx, request, response, len);

	knc_trnsp_free(ctx);
	
	return 0;
	
}
