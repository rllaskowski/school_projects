/** @file
 * Util functions implementation.
 *
 * @author Robert Laskowski <robert.laskowski0@gmail.com>
 * @copyright Robert Laskowski
 * @date 30.05.2020
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

#include "utils.h"

uint32_t min(uint32_t a, uint32_t b) {
	if (a < b) {
		return a;
	} else {
		return b;
	}
}

bool parse_ui(char *ui_str, unsigned *res) {
	if (!ui_str || strlen(ui_str) == 0 || ui_str[0] == '-' ) {
		return false;
	}

	char *end;

	errno = 0;
	unsigned long parsed = strtoul(ui_str, &end, 10);

	if (errno == ERANGE || errno == EINVAL ||
		*end != 0 || parsed > UINT32_MAX) {
		return false;
	}

	*res = parsed;
	return true;
}

void str_lower(char *str) {
	while (*str) {
		(*str) = (char)tolower((int)(*str));
		++str;
	}
}

int64_t to_msec(struct timeval *t) {
	return (int64_t)t->tv_sec*1000+t->tv_usec/1000;
}

void logd(const char *fmt, ...) {
#ifdef DEBUG
	va_list fmt_args;
	va_start(fmt_args, fmt);
	vfprintf(stderr, fmt, fmt_args);
	va_end(fmt_args);
	fprintf(stderr, "\n");
#endif
}