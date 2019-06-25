#include <stdbool.h>

#include "direction.h"
#include "util/util.h"

bool isSingleDirection(Direction dir) {
  return (dir & dir - 1) == 0;
}

int extractDirections(Direction dir, Direction res[4]) {
  int count = 0;

  if (dir & DIR_UP) {
    res[count++] = DIR_UP;
  }

  if (dir & DIR_RIGHT) {
    res[count++] = DIR_RIGHT;
  }

  if (dir & DIR_DOWN) {
    res[count++] = DIR_DOWN;
  }

  if (dir & DIR_LEFT) {
    res[count++] = DIR_LEFT;
  }

  return count;
}

vec2i directionToDelta(Direction dir) {
  vec2i delta = { 0, 0 };

  switch (dir) {
    case DIR_UP:
      delta.x = 0;
      delta.y = 1;
      break;
    case DIR_DOWN:
      delta.x = 0;
      delta.y = -1;
      break;

    case DIR_RIGHT:
      delta.x = 1;
      delta.y = 0;
      break;

    case DIR_LEFT:
      delta.x = -1;
      delta.y = 0;
      break;

    default:
      break;
  }

  return delta;
}

Direction deltaToDirection(vec2i delta) {
  Direction dir = DIR_NONE;

  if (delta.x == 1 && delta.y == 0) {
    return DIR_RIGHT;
  }
  if (delta.x == -1 && delta.y == 0) {
    return DIR_LEFT;
  }

  if (delta.x == 0 && delta.y == 1) {
    return DIR_UP;
  }
  if (delta.x == 0 && delta.y == -1) {
    return DIR_DOWN;
  }

  return dir;
}
