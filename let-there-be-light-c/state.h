#ifndef STATE_H
#define STATE_H

#include <stdbool.h>

#include "config.h"

#include "scene/game/player.h"
#include "scene/game/enemy.h"

#include "scene/scene.h"
#include "util/util.h"
#include "util/list.h"
#include "maze/tile.h"

typedef struct ClientRect {
  double top;
  double right;
  double bottom;
  double left;
  double width;
  double height;
} ClientRect;

typedef struct State {
  Scene* scene;

  struct {
    int major;
    int minor;
  } level;

  int remainItem;

  Tile maze[MAZE_SIZE][MAZE_SIZE];
  int stepsFromPlayer[MAZE_SIZE][MAZE_SIZE];
  vec2i* openTiles;
  int pathLength;

  bool paused;

  double visibleRadius;
  double lastVisibleRadius;

  bool unlockLimits;

  Player player;
  List* enemies;

  ClientRect ortho;
  ClientRect viewport;
} State;

extern State GameState;

void resetGameState(void);
void initGameState(void);

#endif
