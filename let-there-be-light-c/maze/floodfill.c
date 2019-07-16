#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#include "maze/tile.h"
#include "maze/floodfill.h"
#include "util/list.h"
#include "util/util.h"

static void mark(int x, int y, FloodState* state) {
  if (state->depthMap[y][x] != FLOOD_DEPTH_UNVISITED) {
    return;
  }

  vec2i* frontier = malloc(sizeof(vec2i));
  frontier->x = x;
  frontier->y = y;

  Node* node = createNode();
  node->data = frontier;

  listAppend(state->frontiers, node);
  state->depthMap[y][x] = state->currentDepth;
  state->pathLength++;
}

void floodForward(FloodState* state) {
  if (state->finished) {
    return;
  }

  state->currentDepth++;

  ListIterator it = createListIterator(state->frontiers);

  while (!it.done) {
    Node* head = it.next(&it);
    vec2i* frontier = head->data;

    int x = frontier->x;
    int y = frontier->y;

    // up
    if (y < MAZE_SIZE - 1 && state->tiles[y + 1][x] & TILE_OPEN) {
      mark(x, y + 1, state);
    }
    // down
    if (y > 0 && state->tiles[y - 1][x] & TILE_OPEN) {
      mark(x, y - 1, state);
    }

    // right
    if (x < MAZE_SIZE - 1 && state->tiles[y][x + 1] & TILE_OPEN) {
      mark(x + 1, y, state);
    }
    // left
    if (x > 0 && state->tiles[y][x - 1] & TILE_OPEN) {
      mark(x - 1, y, state);
    }

    // remove the head frontier from list
    listDelete(state->frontiers, head);
  }

  state->finished = (state->frontiers->count == 0);
}

FloodState* floodGenerate(const Tile tiles[MAZE_SIZE][MAZE_SIZE],
                          int startX,
                          int startY) {
  FloodState* state = malloc(sizeof(FloodState));

  state->frontiers = createList();
  state->tiles = tiles;
  state->pathLength = 0;
  state->currentDepth = 0;
  state->finished = false;

  memset(state->depthMap, FLOOD_DEPTH_UNVISITED, sizeof(state->depthMap));

  mark(startX, startY, state);

  return state;
}

FloodState* floodFill(Tile tiles[MAZE_SIZE][MAZE_SIZE],
                      int x,
                      int y) {
  FloodState* state = floodGenerate(tiles, x, y);

  while (state->frontiers->count) {
    floodForward(state);
  }

  return state;
}

void floodDestory(FloodState* state) {
  listDestory(state->frontiers);
  free(state);
}
