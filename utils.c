/*
 * Created by Costa Bushnaq
 *
 * 05-04-2021 @ 17:58:23
*/

#include "utils.h"

#include <stdio.h>
#include <string.h>

void
err(char const* fmt, va_list ap)
{
	vfprintf(stderr, fmt, ap);

	if (fmt[0] && fmt[strlen(fmt) - 1] == ':')
	{
		fputc(' ', stderr);
		perror(0x0);
	}
	else
	{
		fputc('\n', stderr);
	}
}

void
warn(char const* fmt, ...)
{
	va_list ap;

	va_start(ap,  fmt);
	err(fmt, ap);
	va_end(ap);
}
