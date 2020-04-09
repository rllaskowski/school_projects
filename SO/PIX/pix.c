#include <stdio.h>
#include <stdlib.h>

#include "pix.h"

#define MAX_DIGITS 10000

uint32_t digits[MAX_DIGITS];

void pixtime(uint64_t clock_tick) {
  fprintf(stderr, "%016lX\n", clock_tick);
}

int main(int argz, char *args[]) {
    if (argz > 1) {
      const uint32_t N = atoi(args[1]);
      uint64_t digit_pos = 0;
      
      if (N < MAX_DIGITS) {
        for (unsigned i = 1; i <= N; ++i) {
            pix(digits, &digit_pos, N);

            printf("%08X", digits[i-1]);
            
            if (i % 8 == 0) {
                printf("\n");
            }
        }
      }
    }

    return 0;
}