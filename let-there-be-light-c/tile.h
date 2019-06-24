#ifndef TILE_H
#define TILE_H

typedef enum Tile {
  TILE_WALL   = 1 << 0,
  TILE_SHADOW = 1 << 1,
  TILE_PATH   = 1 << 2,
  TILE_COIN   = 1 << 3,
  TILE_KERNEL = 1 << 4,

  TILE_SPOILED = 1 << 5,

  TILE_CLOSED = TILE_WALL | TILE_SHADOW,
  TILE_OPEN   = TILE_PATH | TILE_COIN | TILE_KERNEL,

  TILE_ITEM   = TILE_COIN | TILE_KERNEL
} Tile;

#endif
