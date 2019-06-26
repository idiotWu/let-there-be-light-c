#include <stdbool.h>

#include "config.h"

#include "state.h"
#include "util/list.h"
#include "maze/direction.h"

State GameState;

void initGameState(void) {
  // TODO: replace next
  GameState.currentScene = SCENE_NONE;

  GameState.level = 0;
  GameState.remainItem = 0;

  GameState.openTiles = NULL;
  GameState.pathLength = 0;

  GameState.paused = false;
  GameState.keyPressed = DIR_NONE;

  GameState.visibleRadius = 0.0;
  GameState.lastVisibleRadius = 0.0;
  GameState.player.idle = true;
  GameState.player.spoiled = false;
  GameState.player.spriteState = 0;
  
  GameState.enemies = createList();
}
