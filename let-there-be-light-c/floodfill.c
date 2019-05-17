#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "floodfill.h"
#include "list.h"
#include "tile.h"
#include "config.h"

static void mark(size_t x, size_t y, FloodState* state) {
  if (state->visited[y][x]) {
    return;
  }
  
  Frontier* f = malloc(sizeof(Frontier));
  f->x = x;
  f->y = y;
  
  FrontierNode* node = (FrontierNode*)createNode();
  node->data = f;
  
  listAppend((List*)state->frontiers, (Node*)node);
  state->visited[y][x] = true;
  state->pathLength++;
}

void floodForward(FloodState* state) {
  size_t batched = state->frontiers->count;
  
  while (batched--) {
    FrontierNode* head = state->frontiers->head;
    size_t x = head->data->x;
    size_t y = head->data->y;
    
    // up
    if (y > 0 && state->tiles[y - 1][x] & TILE_OPEN) {
      mark(x, y - 1, state);
    }
    // down
    if (y < MAP_SIZE - 1 && state->tiles[y + 1][x] & TILE_OPEN) {
      mark(x, y + 1, state);
    }
    
    // left
    if (x > 0 && state->tiles[y][x - 1] & TILE_OPEN) {
      mark(x - 1, y, state);
    }
    // right
    if (x < MAP_SIZE - 1 && state->tiles[y][x + 1] & TILE_OPEN) {
      mark(x + 1, y, state);
    }
    
    // remove the frontier from list
    listDelete((List*)state->frontiers, (Node*)head);
  }
}

FloodState* floodGenerate(const Tile tiles[MAP_SIZE][MAP_SIZE], size_t x, size_t y) {
  FloodState* state = calloc(1, sizeof(FloodState));
  
  state->frontiers = (FrontierList*)createList();
  state->tiles = tiles;
  state->pathLength = 0;
  
  mark(x, y, state);
  
  return state;
}

FloodState* floodFill(Tile tiles[MAP_SIZE][MAP_SIZE], size_t x, size_t y) {
  FloodState* state = floodGenerate(tiles, x, y);
  
  while (state->frontiers->count) {
    floodForward(state);
  }
  
  return state;
}

void floodDestory(FloodState* state) {
  listDestory((List*)state->frontiers);
  free(state);
}
