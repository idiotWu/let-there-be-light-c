#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

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
  int remainItem;

  Tile maze[MAZE_SIZE][MAZE_SIZE];

  bool idle;
  bool cleared;
  Direction keyPressed;

  double visibleRadius;
  struct {
    double x;
    double y;
    int state;
    Direction direction;
  } player;

  ClientRect ortho;
  ClientRect viewport;
} State;

extern State GameState;

#endif
