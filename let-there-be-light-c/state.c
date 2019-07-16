#include <stdbool.h>

#include "config.h"

#include "state.h"
#include "util/list.h"
#include "maze/direction.h"
#include "scene/scene.h"
#include "scene/title/title.h"

State GameState;

void resetGameState(void) {
  GameState.remainItem = 0;
  GameState.pathLength = 0;
  GameState.paused = false;
  GameState.visibleRadius = 0.0;
  GameState.lastVisibleRadius = 0.0;
  GameState.player.idle = true;
  GameState.player.spoiled = false;
  GameState.player.spriteState = 0;
}

void initGameState(void) {
  resetGameState();

  GameState.level.major = 1;
  GameState.level.minor = 1;
  GameState.unlockLimits = false;

  GameState.openTiles = NULL;
  GameState.enemies = createList();

  switchScene(titleScene);
}
