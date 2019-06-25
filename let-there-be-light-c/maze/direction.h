#ifndef DIRECTION_H
#define DIRECTION_H

#include <stdbool.h>
#include <stddef.h>

#include "util/util.h"

typedef enum Direction {
  DIR_NONE  = 0,      /* 0000 */
  DIR_UP    = 1 << 0, /* 0001 */
  DIR_RIGHT = 1 << 1, /* 0010 */
  DIR_DOWN  = 1 << 2, /* 0100 */
  DIR_LEFT  = 1 << 3, /* 1000 */

  DIR_VERTICAL   = DIR_UP | DIR_DOWN,
  DIR_HORIZONTAL = DIR_LEFT | DIR_RIGHT,
  DIR_ALL        = DIR_UP | DIR_RIGHT | DIR_DOWN | DIR_LEFT
} Direction;

bool isSingleDirection(Direction dir);
int extractDirections(Direction dir, Direction res[4]);
vec2i directionToDelta(Direction dir);
Direction deltaToDirection(vec2i delta);

#endif
