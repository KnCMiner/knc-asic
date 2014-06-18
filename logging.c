#include <stdarg.h>
#include <stdio.h>

#define UNUSED __attribute__((unused))

void applog(UNUSED int level, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	printf("\n");
	va_end(ap);
}
