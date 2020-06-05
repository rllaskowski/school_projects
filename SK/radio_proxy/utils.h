/** @file
 * Util functions interface
 *
 * @author Robert Laskowski <robert.laskowski0@gmail.com>
 * @copyright Robert Laskowski
 * @date 30.05.2020
 */


#ifndef _UTILS_H_
#define _UTILS_H_

uint32_t min(uint32_t a, uint32_t b);

bool parse_ui(char *ui_str, unsigned *res);

void str_lower(char *str);

int64_t to_msec(struct timeval *t);

#endif	/* _UTILS_H_ */