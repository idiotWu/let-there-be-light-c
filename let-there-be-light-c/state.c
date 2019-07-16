#include <stdbool.h>
#include <stdlib.h>

#include "config.h"
#include "state.h"

#include "util/list.h"
#include "maze/direction.h"
#include "scene/scene.h"
#include "scene/title/title.h"

State GameState;

void initGameState(void) {
  GameState.level.major = 1;
  GameState.level.minor = 1;

  GameState.paused = false;
  GameState.unlockLimits = false;

  GameState.remainItem = 0;
  GameState.pathLength = 0;

  GameState.visibleRadius = 0.0;
  GameState.lastVisibleRadius = 0.0;

  GameState.player.idle = true;
  GameState.player.spoiled = false;
  GameState.player.spriteState = 0;

  if (GameState.openTiles) {
    free(GameState.openTiles);
    GameState.openTiles = NULL;
  }

  if (GameState.enemies) {
    listClear(GameState.enemies);
  } else {
    GameState.enemies = createList();
  }
}
