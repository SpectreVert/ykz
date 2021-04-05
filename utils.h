/*
 * Created by Costa Bushnaq
 *
 * 05-04-2021 @ 17:57:10
 *
 * see LICENSE
*/

#ifndef UTIL_H
#define UTIL_H

#include <errno.h>
#include <stdarg.h>

void warn(char const*, ...);
void die(char const*, ...);

#endif /* UTIL_H */
