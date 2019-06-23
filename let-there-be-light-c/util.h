#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

#define UNUSED(x) (void)(x)

#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
  #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

typedef struct vec2i {
  int x;
  int y;
} vec2i;

typedef struct vec2d {
  double x;
  double y;
} vec2d;

double randomBetween(double min, double max);
int randomInt(int lower, int upper);
void swap(void* a, void* b, size_t bytes);
void shuffle(void* array, size_t length, size_t bytesPerElement);

int sign(double num);
double clamp(double val, double min, double max);

#endif
