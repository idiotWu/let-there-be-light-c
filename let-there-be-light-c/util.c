#include <stdlib.h>
#include <time.h>

#include "util.h"

static int randInitFlag = 0;

double randomBetween(double lower, double upper) {
  // init once
  if (!randInitFlag) {
    randInitFlag = 1;
    srand((unsigned int)time(NULL));
  }

  return (double)rand() / (double)RAND_MAX * (upper - lower) + lower;
}

int randomInt(int lower, int upper) {
  return (int)randomBetween((double)lower, (double)upper + 1.0);
}

void swap(void* a, void* b, size_t bytes) {
  char* _a = a;
  char* _b = b;

  while (bytes--) {
    char t = *_a;
    *_a++ = *_b;
    *_b++ = t;
  }
}

void shuffle(void* array, size_t length, size_t bytesPerElement) {
  while (length --> 1) {
    size_t index = randomInt(0, (int)length - 1);

    // cast `void*` to `char*` for arithmetic
    swap(
      (char*)array + index * bytesPerElement,
      (char*)array + length * bytesPerElement,
      bytesPerElement
    );
  }
}

int sign(double num) {
  if (num < 0) {
    return -1;
  }

  if (num > 0) {
    return 1;
  }

  return 0;
}
