#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

double randomBetween(double min, double max);
int randomInt(int lower, int upper);
void swap(void* a, void* b, size_t bytes);
void shuffle(void* array, size_t length, size_t bytesPerElement);

double min(double a, double b);
double max(double a, double b);
int sign(double num);

#endif
