#ifndef STATE_H
#define STATE_H

#include "config.h"
#include "tile.h"
#include "direction.h"

typedef struct State {
  int level;
  double visibleRadius;
  Tile map[MAZE_SIZE][MAZE_SIZE];
  struct {
    double x;
    double y;
    Direction direction;
  } player;

  struct {
    double top;
    double right;
    double bottom;
    double left;
  } ortho;
} State;

extern State GameState;

#endif
