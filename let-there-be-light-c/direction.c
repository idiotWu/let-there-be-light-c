#include <stdbool.h>

#include "direction.h"

bool isSingleDirection(Direction dir) {
  return (dir & dir - 1) == 0;
}

unsigned int extractDirections(Direction dir, Direction res[4]) {
  unsigned int count = 0;
  
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
