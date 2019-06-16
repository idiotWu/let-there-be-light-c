#ifndef MAZE_H
#define MAZE_H

#include <stddef.h>

#include "config.h"
#include "tile.h"

size_t initMaze(size_t spawnerCount,
                size_t minDistance,
                size_t maxDistance,
                Tile tiles[MAZE_SIZE][MAZE_SIZE]);

#endif
