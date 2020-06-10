/** @file
 * Implementacja funkcji pomocniczych.
 *
 * @author Robert Laskowski <robert.laskowski0@gmail.com>
 * @copyright Robert Laskowski
 * @date 10.06.2020
 */


#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

uint8_t number_length(uint64_t number) {
	if (number == 0) {
		return 1;
	}

	uint8_t length = 0;

	while (number != 0) {
		++length;
		number /= 10;
	}

	return length;
}

struct termios start_raw_mode() {
	struct termios original;
	struct termios raw;

	tcgetattr(STDIN_FILENO, &original);

	raw = original;

	raw.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

	return original;
}

void set_term_mode(struct termios mode) {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &mode);
}

void scr_clear() {
	printf("\033[H\033[J");
}

void cursor_move(direction_m dir, uint32_t shift) {
	if (shift > 0) {
		printf("\033[%u%c", shift, 'A' + dir);
	}
}

control_m get_key() {
	int c;
	int last_c = 0;

	while (true) {
		if (last_c == 0) {
			c = getchar();
		} else {
			c = last_c;
		}
		last_c = 0;

		if (c == 'g' || c == 'G') {
			return KEY_G;
		} else if (c == 'c' || c == 'C') {
			return KEY_C;
		} else if (c == ' ') {
			return KEY_SPACE;
		} else if (c == '\033') {
			c = getchar();
			if (c == '[') {
				c = getchar();
				if ('A' <= c && c <= 'D') {
					return KEY_UP+(c-'A');
				}
			}
			last_c = c;
		} else if (c == 4) {
			return CTRL_D;
		}
	}
}

bool parse_ui(char *ui_str, uint32_t *res) {
	if (!ui_str || strlen(ui_str) == 0
		|| ui_str[0] == '-'  || ui_str[0] == '+') {
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

void cursor_clear() {
	printf("\033[H");
}