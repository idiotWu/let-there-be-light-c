#ifndef STATE_H
#define STATE_H

#include "config.h"
#include "tile.h"
#include "direction.h"

typedef struct ClientRect {
  double top;
  double right;
  double bottom;
  double left;
  double width;
  double height;
} ClientRect;

typedef struct State {
  int level;
  double visibleRadius;
  Tile map[MAZE_SIZE][MAZE_SIZE];
  struct {
    double x;
    double y;
    Direction direction;
  } player;

  ClientRect ortho;
  ClientRect viewport;
} State;

extern State GameState;

#endif
