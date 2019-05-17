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
  const Tile (*tiles)[MAP_SIZE];
  bool visited[MAP_SIZE][MAP_SIZE];
  size_t pathLength;
} FloodState;

FloodState* floodGenerate(const Tile tiles[MAP_SIZE][MAP_SIZE],
                          size_t x,
                          size_t y);

FloodState* floodFill(Tile tiles[MAP_SIZE][MAP_SIZE],
                      size_t x,
                      size_t y);

void floodDestory(FloodState* state);

#endif
