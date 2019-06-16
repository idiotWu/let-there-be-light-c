#ifndef FLOODFILL_H
#define FLOODFILL_H

#include <stdbool.h>
#include <stddef.h>

#include "config.h"
#include "list.h"
#include "tile.h"

typedef struct Frontier {
  size_t x;
  size_t y;
} Frontier;

defNode(FrontierNode, Frontier);
defList(FrontierList, FrontierNode);

typedef struct FloodState {
  FrontierList* frontiers;
  const Tile (*tiles)[MAZE_SIZE];
  bool visited[MAZE_SIZE][MAZE_SIZE];
  size_t pathLength;
} FloodState;

FloodState* floodGenerate(const Tile tiles[MAZE_SIZE][MAZE_SIZE],
                          size_t x,
                          size_t y);

FloodState* floodFill(Tile tiles[MAZE_SIZE][MAZE_SIZE],
                      size_t x,
                      size_t y);

void floodDestory(FloodState* state);

#endif
