#ifndef FLOODFILL_H
#define FLOODFILL_H

#include <stdbool.h>
#include <stddef.h>

#include "config.h"

#include "tile.h"
#include "util/list.h"

#define FLOOD_DEPTH_UNVISITED  -1

typedef struct FloodState {
  List* frontiers;
  const Tile (*tiles)[MAZE_SIZE];
  int depthMap[MAZE_SIZE][MAZE_SIZE];
  int pathLength;
  int currentDepth;
  bool finished;
} FloodState;

void floodForward(FloodState* state);

FloodState* floodGenerate(const Tile tiles[MAZE_SIZE][MAZE_SIZE],
                          int x, int y);

FloodState* floodFill(Tile tiles[MAZE_SIZE][MAZE_SIZE],
                      int x, int y);

void floodDestory(FloodState* state);

#endif
