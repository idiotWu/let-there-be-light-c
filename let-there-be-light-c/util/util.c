/**
 * @file
 * @brief ユーティリティ
 */
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "util/util.h"

double randomBetween(double lower, double upper) {
  static int randInitFlag = 0;

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

// clamps the given value between [min, max]
double clamp(double val, double min, double max) {
  if (val < min) {
    return min;
  } else if (val > max) {
    return max;
  }

  return val;
}

double distance(double x1, double y1, double x2, double y2) {
  return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

void noop(void) {}
