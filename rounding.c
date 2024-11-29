#define _GNU_SOURCE

#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

size_t round_up(size_t size) {
  size_t return_size = 16;
  while (return_size < size) {
    return_size *= 2;
  }
  return return_size;
}

int main() {
  int sizes[10];

  sizes[0] = round_up(7);
  sizes[1] = round_up(16);
  sizes[2] = round_up(30);
  sizes[3] = round_up(50);
  sizes[4] = round_up(7);
  sizes[5] = round_up(100);
  sizes[6] = round_up(1000);
  sizes[7] = round_up(2047);
  sizes[8] = round_up(1999);
  sizes[9] = round_up(511);

  for (int i = 0; i < 10; i++) {
    printf("Size: %d\n", sizes[i]);
  }

  printf("%lu\n", sizeof(size_t));
}
